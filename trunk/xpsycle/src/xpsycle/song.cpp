///\file
///\brief implementation file for psycle::host::Song
#include "song.h"
#include "machine.h"
#include "internal_machines.h"
#include "sampler.h"
#include "xmsampler.h"
#include "plugin.h"
#include "psyfilter.h"
#include "datacompression.h"
#include "riff.h"
#include "ladspamachine.h"
#include "pluginfinder.h"
#include "prototypemachinefactory.h"
#include <cassert>
#include <algorithm>
#include <sstream>


namespace psycle {
  namespace host
  {

    Song::Song()
    {
      machineFactory_.setDeleteFlag(true);
      machineFactory_.registerMachine( 0, *(new Master(0,this)) );
      machineFactory_.registerMachine( 3, *(new Sampler(0,this)) );
      machineFactory_.registerMachine( 8, *(new Plugin(0,this)) );
      machineFactory_.registerMachine( 13, *(new DuplicatorMac(0,this)) );
      machineFactory_.registerMachine( 16, *(new LADSPAMachine( 0, this )) );      
      machineFactory_.registerMachine( 255, *(new Dummy(0,this)) );

      tracks_= MAX_TRACKS;
      _machineLock = false;
      Invalided = false;
      preview_vol = 0.25f;

      for(int i(0) ; i < MAX_MACHINES ; ++i) _pMachine[i] = 0;
      for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) _pInstrument[i] = new Instrument;

      clear();
    }

    Song::~Song()
    {
      DestroyAllMachines();
      DestroyAllInstruments();
    }


    void Song::clear( )
    {
      _machineLock = false;
      Invalided = false;
      preview_vol = 0.25f;


      tracks_= MAX_TRACKS;
      seqBus=0;

      name_    = "Untitled";
      author_  = "Unnamed";
      comment_ = "No Comments";

      /// gui stuff
      currentOctave=4;

      // General properties
      {
        bpm_ = 125.0f;
        m_LinesPerBeat=4;
      }
      // Clean up allocated machines.
      DestroyAllMachines(true);
      // Cleaning instruments
      DeleteInstruments();
      // Clear patterns
      patternSequence()->removeAll();
      // Clear sequence
      _sampCount=0;
      // Cleaning pattern allocation info
      for(int i(0) ; i < MAX_INSTRUMENTS; ++i) _pInstrument[i]->waveLength=0;
      for(int i(0) ; i < MAX_MACHINES ; ++i)
      {
        if (_pMachine[i]) delete _pMachine[i];
        _pMachine[i] = 0;
      }

      _trackArmedCount = 0;
      for(int i(0) ; i < MAX_TRACKS; ++i)
      {
        _trackMuted[i] = false;
        _trackArmed[i] = false;
      }
      machineSoloed = -1;
      _trackSoloed = -1;


      instSelected = 0;
      midiSelected = 0;
      auxcolSelected = 0;

      _saved=false;
      fileName = "Untitled.psy";

      CreateMachine(MACH_MASTER, 320, 200, "master", MASTER_INDEX);
    }

    const AbstractMachineFactory& Song::machineFactory() const {
      return machineFactory_;
    }


    Machine * Song::createMachine( const PluginFinder & finder, const PluginFinderKey & key, int x, int y ) {
      int fb = GetFreeBus();
      if ( key == PluginFinderKey::internalSampler() ) {
        // create internal Sampler
        CreateMachine(MACH_SAMPLER, x, y, "SAMPLER", fb);  
      } else 
        if ( finder.info( key ).type() == MACH_PLUGIN ) 
        {
          // create native Plugin
          CreateMachine( MACH_PLUGIN, x, y, key.dllPath(), fb );
        } else
          if ( finder.info( key ).type() == MACH_LADSPA ){
            // create ladspa plugin
            LADSPAMachine* plugin = new LADSPAMachine( fb, this );
            if  ( plugin->loadDll( key.dllPath(), key.index() ) ) {
              plugin->_x = x;
              plugin->_y = y;
              if( _pMachine[fb] )  DestroyMachine( fb );
              _pMachine[ fb ] = plugin;
            } else {				 
              delete plugin;
            }
          }
          return _pMachine[fb];
    }

    bool Song::CreateMachine(int type, int x, int y, std::string const & plugin_name, int index, int pluginIndex)
    {
      Machine * machine(0);
      switch (type)
      {
      case MACH_MASTER:
        if(_pMachine[MASTER_INDEX]) return false;
        index = MASTER_INDEX;
        machine = new Master(index, this);
        break;
      case MACH_SAMPLER:
        machine = new Sampler(index,this);
        break;
      case MACH_DUPLICATOR:
        machine = new DuplicatorMac(index, this);
        break;
      case MACH_DUMMY:
        machine = new Dummy(index, this);
        break;
      case MACH_PLUGIN:
        {
          Plugin* plugin = new Plugin( index, this );
          plugin->LoadDll( plugin_name );
          machine = plugin;
        }
        break;
      case MACH_LADSPA:
        {
          LADSPAMachine* plugin = new LADSPAMachine(index,this);
          machine = plugin;
          const char* pcLADSPAPath;
          pcLADSPAPath = std::getenv("LADSPA_PATH");
          if ( !pcLADSPAPath) {
#ifdef __unix__
            pcLADSPAPath = "/usr/lib/ladspa/";
#else
            pcLADSPAPath = "I:\\Archivos de Programa\\Multimedia\\Audacity\\Plug-Ins\\";
#endif
          }
          std::string path;
          if ( pcLADSPAPath ) path = pcLADSPAPath;
          plugin->loadDll( path + plugin_name, pluginIndex);
        }
        break;
      default:
        machine = new Dummy(index, this);
        break;
      }

      if(index < 0)
      {
        index =	GetFreeMachine();
        if(index < 0)
        {
          return false;
        }
      }

      if(_pMachine[index]) DestroyMachine(index);

      ///\todo init problem
      {
        if(machine->_type == MACH_VSTFX || machine->_type == MACH_VST )
        {

          // Do not call VST Init() function after Instance.
          machine->Machine::Init();
        }
        else machine->Init();
      }
      machine->_x = x;
      machine->_y = y;

      // Finally, activate the machine
      _pMachine[index] = machine;

      return true;
    }

    bool Song::InsertConnection(int src, int dst, float volume)
    {
      Machine *srcMac = _pMachine[src];
      Machine *dstMac = _pMachine[dst];
      if(!srcMac || !dstMac)
      {
        std::ostringstream s;
        s << "attempted to use a null machine pointer when connecting machines ids: src: " << src << ", dst: " << dst << std::endl;
        s << "src pointer is " << srcMac << ", dst pointer is: " << dstMac;
        //				loggers::warning(s.str());
        return false;
      }
      return srcMac->ConnectTo(*dstMac, 0, 0, volume);
    }

    int Song::FindBusFromIndex(int smac)
    {
      if(!_pMachine[smac]) return int(255);
      return smac;
    }


    void Song::DestroyAllMachines(bool write_locked)
    {
      _machineLock = true;
      for(int c(0); c < MAX_MACHINES; ++c)
      {
        if(_pMachine[c])
        {
          for(int j(c + 1); j < MAX_MACHINES; ++j)
          {
            if(_pMachine[c] == _pMachine[j])
            {
              ///\todo wtf? duplicate machine? could happen if loader messes up?
              {
                std::ostringstream s;
                s << c << " and " << j << " have duplicate pointers";
                report.emit(s.str(), "Song Delete Error");
              }
              _pMachine[j] = 0;
            }
          }
          DestroyMachine(c, write_locked);
        }
        _pMachine[c] = 0;
      }
      _machineLock = false;
    }


    int Song::GetFreeMachine()
    {
      int tmac(0);
      for(;;)
      {
        if(!_pMachine[tmac]) return tmac;
        if(tmac++ >= MAX_MACHINES) return int(-1); // that's why ids can't be unsigned :-(
      }
    }

    void Song::DestroyMachine(int mac, bool write_locked)
    {
      Machine *iMac = _pMachine[mac];
      Machine *iMac2;
      if(iMac)
      {
        // Deleting the connections to/from other machines
        for(int w=0; w<MAX_CONNECTIONS; w++)
        {
          // Checking In-Wires
          if(iMac->_inputCon[w])
          {
            if((iMac->_inputMachines[w] >= 0) && (iMac->_inputMachines[w] < MAX_MACHINES))
            {
              iMac2 = _pMachine[iMac->_inputMachines[w]];
              if(iMac2)
              {
                for(int x=0; x<MAX_CONNECTIONS; x++)
                {
                  if( iMac2->_connection[x] && iMac2->_outputMachines[x] == mac)
                  {
                    iMac2->_connection[x] = false;
                    iMac2->_outputMachines[x]=-1;
                    iMac2->_connectedOutputs--;
                    break;
                  }
                }
              }
            }
          }
          // Checking Out-Wires
          if(iMac->_connection[w])
          {
            if((iMac->_outputMachines[w] >= 0) && (iMac->_outputMachines[w] < MAX_MACHINES))
            {
              iMac2 = _pMachine[iMac->_outputMachines[w]];
              if(iMac2)
              {
                for(int x=0; x<MAX_CONNECTIONS; x++)
                {
                  if(iMac2->_inputCon[x] && iMac2->_inputMachines[x] == mac)
                  {
                    iMac2->_inputCon[x] = false;
                    iMac2->_inputMachines[x]=-1;
                    iMac2->_connectedInputs--;
                    break;
                  }
                }
              }
            }
          }
        }
      }
      if(mac == machineSoloed) machineSoloed = -1;
      // If it's a (Vst)Plugin, the destructor calls to release the underlying library
      zapObject(_pMachine[mac]);
    }


    int Song::GetFreeBus()
    {
      for(int c(0) ; c < MAX_BUSES ; ++c) if(!_pMachine[c]) return c;
      return -1; 
    }

    int Song::GetFreeFxBus()
    {
      for(int c(MAX_BUSES) ; c < MAX_BUSES * 2 ; ++c) if(!_pMachine[c]) return c;
      return -1; 
    }

    // IFF structure ripped by krokpitr
    // Current Code Extremely modified by [JAZ] ( RIFF based )
    // Advise: IFF files use Big Endian byte ordering. That's why I use
    // the following structure.
    //
    // typedef struct {
    //   unsigned char hihi;
    //   unsigned char hilo;
    //   unsigned char lohi;
    //   unsigned char lolo;
    // } ULONGINV;
    // 
    //
    /*
    ** IFF Riff Header
    ** ----------------

    char Id[4]			// "FORM"
    ULONGINV hlength	// of the data contained in the file (except Id and length)
    char type[4]		// "16SV" == 16bit . 8SVX == 8bit

    char name_Id[4]		// "NAME"
    ULONGINV hlength	// of the data contained in the header "NAME". It is 22 bytes
    char name[22]		// name of the sample.

    char vhdr_Id[4]		// "VHDR"
    ULONGINV hlength	// of the data contained in the header "VHDR". it is 20 bytes
    ULONGINV Samplength	// Lenght of the sample. It is in bytes, not in Samples.
    ULONGINV loopstart	// Start point for the loop. It is in bytes, not in Samples.
    ULONGINV loopLength	// Length of the loop (so loopEnd = loopstart+looplenght) In bytes.
    unsigned char unknown2[5]; //Always $20 $AB $01 $00 //
    unsigned char volumeHiByte;
    unsigned char volumeLoByte;
    unsigned char unknown3;

    char body_Id[4]		// "BODY"
    ULONGINV hlength	// of the data contained in the file. It is the sample length as well (in bytes)
    char *data			// the sample.

    */

    bool Song::IffAlloc(int instrument,const char * str)
    {
      if(instrument != PREV_WAV_INS)
      {
        Invalided = true;
        //				::Sleep(LOCK_LATENCY); ///< ???
      }
      RiffFile file;
      RiffChunkHeader hd;
      std::uint32_t data;
      endian::big::uint32_t tmp;
      int bits = 0;
      // opens the file and reads the "FORM" header.
      if(!file.Open(const_cast<char*>(str)))
      {
        Invalided = false;
        return false;
      }
      DeleteLayer(instrument);
      file.Read(data);
      if( data == file.FourCC("16SV")) bits = 16;
      else if(data == file.FourCC("8SVX")) bits = 8;
      file.Read(hd);
      if(hd._id == file.FourCC("NAME"))
      {
        file.ReadChunk(_pInstrument[instrument]->waveName, 22); _pInstrument[instrument]->waveName[21]=0;///\todo should be hd._size instead of "22", but it is incorrectly read.
        std::strncpy(_pInstrument[instrument]->_sName,str, 31);
        _pInstrument[instrument]->_sName[31]='\0';
        file.Read(hd);
      }
      if ( hd._id == file.FourCC("VHDR"))
      {
        std::uint32_t Datalen, ls, le;
        file.Read(tmp); Datalen = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
        file.Read(tmp); ls = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
        file.Read(tmp); le = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
        if(bits == 16)
        {
          Datalen >>= 1;
          ls >>= 1;
          le >>= 1;
        }
        _pInstrument[instrument]->waveLength=Datalen;
        if(ls != le)
        {
          _pInstrument[instrument]->waveLoopStart = ls;
          _pInstrument[instrument]->waveLoopEnd = ls + le;
          _pInstrument[instrument]->waveLoopType = true;
        }
        file.Skip(8); // Skipping unknown bytes (and volume on bytes 6&7)
        file.Read(hd);
      }
      if(hd._id == file.FourCC("BODY"))
      {
        std::int16_t * csamples;
        std::uint32_t const Datalen(_pInstrument[instrument]->waveLength);
        _pInstrument[instrument]->waveStereo = false;
        _pInstrument[instrument]->waveDataL = new std::int16_t[Datalen];
        csamples = _pInstrument[instrument]->waveDataL;
        if(bits == 16)
        {
          for(unsigned int smp(0) ; smp < Datalen; ++smp)
          {
            file.ReadChunk(&tmp, 2);
            *csamples = tmp.hilo * 256 + tmp.hihi;
            ++csamples;
          }
        }
        else
        {
          for(unsigned int smp(0) ; smp < Datalen; ++smp)
          {
            file.ReadChunk(&tmp, 1);
            *csamples = tmp.hihi * 256 + tmp.hihi;
            ++csamples;
          }
        }
      }
      file.Close();
      Invalided = false;
      return true;
    }

    bool Song::WavAlloc(int iInstr, bool bStereo, long iSamplesPerChan, const char * sName)
    {
      assert(iSamplesPerChan<(1<<30)); ///< Since in some places, signed values are used, we cannot use the whole range.
      DeleteLayer(iInstr);
      _pInstrument[iInstr]->waveDataL = new std::int16_t[iSamplesPerChan];
      if(bStereo)
      {	_pInstrument[iInstr]->waveDataR = new std::int16_t[iSamplesPerChan];
      _pInstrument[iInstr]->waveStereo = true;
      } else {
        _pInstrument[iInstr]->waveStereo = false;
      }
      _pInstrument[iInstr]->waveLength = iSamplesPerChan;
      std::strncpy(_pInstrument[iInstr]->waveName, sName, 31);
      _pInstrument[iInstr]->waveName[31] = '\0';
      std::strncpy(_pInstrument[iInstr]->_sName,sName,31);
      _pInstrument[iInstr]->_sName[31]='\0';
      return true;
    }

    bool Song::WavAlloc(int instrument,const char * Wavfile)
    { 
      assert(Wavfile != 0);
      WaveFile file;
      ExtRiffChunkHeader hd;
      // opens the file and read the format Header.
      DDCRET retcode(file.OpenForRead(Wavfile));
      if(retcode != DDC_SUCCESS) 
      {
        Invalided = false;
        return false; 
      }
      Invalided = true;
      //			::Sleep(LOCK_LATENCY); ///< ???
      // sample type	
      int st_type(file.NumChannels());
      int bits(file.BitsPerSample());
      long int Datalen(file.NumSamples());
      // Initializes the layer.
      WavAlloc(instrument, st_type == 2, Datalen, Wavfile);
      // Reading of Wave data.
      // We don't use the WaveFile "ReadSamples" functions, because there are two main differences:
      // We need to convert 8bits to 16bits, and stereo channels are in different arrays.
      std::int16_t * sampL(_pInstrument[instrument]->waveDataL);

      ///\todo use template code for all this semi-repetitive code.

      std::int32_t io; /// \todo why is this declared here?
      // mono
      if(st_type == 1)
      {
        std::uint8_t smp8;
        switch(bits)
        {
        case 8:
          for(io = 0 ; io < Datalen ; ++io)
          {
            file.Read(smp8);
            *sampL = (smp8 << 8) - 32768;
            ++sampL;
          }
          break;
        case 16:
          file.ReadData(sampL, Datalen);
          break;
        case 24:
          for(io = 0 ; io < Datalen ; ++io)
          {
            file.Read(smp8); ///\todo [bohan] is the lsb just discarded? [JosepMa]: yes. sampler only knows about 16bit samples
            file.ReadData(sampL, 1);
            ++sampL;
          }
          break;
        default:
          break; ///\todo should throw an exception
        }
      }
      // stereo
      else
      {
        std::int16_t *sampR(_pInstrument[instrument]->waveDataR);
        std::uint8_t smp8;
        switch(bits)
        {
        case 8:
          for(io = 0 ; io < Datalen ; ++io)
          {
            file.Read(smp8);
            *sampL = (smp8 << 8) - 32768;
            ++sampL;
            file.Read(smp8);
            *sampR = (smp8 << 8) - 32768;
            ++sampR;
          }
          break;
        case 16:
          for(io = 0 ; io < Datalen ; ++io)
          {
            file.ReadData(sampL, 1);
            file.ReadData(sampR, 1);
            ++sampL;
            ++sampR;
          }
          break;
        case 24:
          for(io = 0 ; io < Datalen ; ++io)
          {
            file.Read(smp8); ///\todo [bohan] is the lsb just discarded? [JosepMa]: yes. sampler only knows about 16bit samples
            file.ReadData(sampL, 1);
            ++sampL;
            file.Read(smp8); ///\todo [bohan] is the lsb just discarded? [JosepMa]: yes. sampler only knows about 16bit samples
            file.ReadData(sampR, 1);
            ++sampR;
          }
          break;
        default:
          break; ///\todo should throw an exception
        }
      }
      retcode = file.Read(hd);
      while(retcode == DDC_SUCCESS)
      {
        if(hd.ckID == FourCC("smpl"))
        {
          file.Skip(28);
          char pl;
          file.Read(pl);
          if(pl == 1)
          {
            file.Skip(15);
            std::uint32_t ls; file.Read(ls);
            std::uint32_t le; file.Read(le);
            _pInstrument[instrument]->waveLoopStart = ls;
            _pInstrument[instrument]->waveLoopEnd = le;
            // only for my bad sample collection
            if(!((ls <= 0) && (le >= Datalen - 1)))
            {
              _pInstrument[instrument]->waveLoopType = true;
            }
            else { ls = 0; le = 0;	}
          }
          file.Skip(9);
        }
        else if(hd.ckSize > 0)
          file.Skip(hd.ckSize);
        else
          file.Skip(1);
        retcode = file.Read(hd); ///\todo bloergh!
      }
      file.Close();
      Invalided = false;
      return true;
    }

    bool Song::load(const std::string & fileName)
    {
      PsyFilter filter;
      return filter.loadSong(fileName, *this);
    }

    bool Song::save(const std::string & fileName)
    {
      PsyFilter filter;
      return filter.saveSong(fileName, *this,4);
    }

    void Song::DoPreviews(int amount)
    {
      //todo do better.. use a vector<InstPreview*> or something instead
      if(wavprev.IsEnabled())
      {
        wavprev.Work(_pMachine[MASTER_INDEX]->_pSamplesL, _pMachine[MASTER_INDEX]->_pSamplesR, amount);
      }
      if(waved.IsEnabled())
      {
        waved.Work(_pMachine[MASTER_INDEX]->_pSamplesL, _pMachine[MASTER_INDEX]->_pSamplesR, amount);
      }
    }

    PatternSequence * Song::patternSequence( )
    {
      return &patternSequence_;
    }

    const PatternSequence & Song::patternSequence( ) const
    {
      return patternSequence_;
    }

    unsigned int Song::tracks( ) const
    {
      return tracks_;
    }

    void Song::setTracks( unsigned int trackCount )
    {
      tracks_ = trackCount;
    }

    void Song::setName( const std::string & name )
    {
      name_ = name;
    }

    const std::string & Song::name( ) const
    {
      return name_;
    }

    void Song::setAuthor( const std::string & author )
    {
      author_ = author;
    }

    const std::string & Song::author( ) const
    {
      return author_;
    }

    void Song::setComment( const std::string & comment )
    {
      comment_ = comment;
    }

    const std::string & Song::comment( ) const
    {
      return comment_;
    }

    void Song::setBpm( float bpm )
    {
      if (bpm > 0 && bpm < 1000)
        bpm_ = bpm;
    }

    float Song::bpm( ) const
    {
      return bpm_;
    }


    Machine* Song::LoadFileChunk(Song* pSong, RiffFile* pFile, int index, int version,bool fullopen)
    {
      // assume version 0 for now
      bool bDeleted(false);
      Machine* pMachine;
      MachineType type;//,oldtype;
      char dllName[256];
      pFile->Read(type);
      //oldtype=type;
      pFile->ReadString(dllName,256);
      switch (type)
      {
      case MACH_MASTER:
        if (pSong->_pMachine[MASTER_INDEX]) pMachine = pSong->_pMachine[MASTER_INDEX];
        else if ( !fullopen ) pMachine = new Dummy(index, pSong);
        else pMachine = new Master(index, pSong);
        break;
      case MACH_SAMPLER:
        if ( !fullopen ) pMachine = new Dummy(index, pSong);
        else pMachine = new Sampler(index, pSong );
        break;
      case MACH_XMSAMPLER:
        //if ( !fullopen ) 
        pMachine = new Dummy(index, pSong);
        type = MACH_DUMMY;
        //else pMachine = new XMSampler(index);
        break;
      case MACH_DUPLICATOR:
        if ( !fullopen ) pMachine = new Dummy(index, pSong);
        else pMachine = new DuplicatorMac(index, pSong);
        break;
      case MACH_PLUGIN:
        {
          if(!fullopen) pMachine = new Dummy(index, pSong);
          else 
          {
            Plugin * p;
            pMachine = p = new Plugin(index, pSong);
            if(!p->LoadDll(dllName))
            {
              pMachine = new Dummy(index, pSong);
              type = MACH_DUMMY;
              delete p;
              bDeleted = true;
            }
          }
        }
        break;
      default:
        std::cerr << "Please inform the devers about this message: unknown kind of machine while loading new file format" << std::endl;
        pMachine = new Dummy(index, pSong);
        break;
      }
      pMachine->Init();
      int temp;
      pMachine->_type = type;
      pFile->Read(pMachine->_bypass);
      pFile->Read(pMachine->_mute);
      pFile->Read(pMachine->_panning);
      pFile->Read(temp);
      pMachine->_x=temp;
      pFile->Read(temp);
      pMachine->_y=temp;
      pFile->Read(pMachine->_connectedInputs);
      pFile->Read(pMachine->_connectedOutputs);
      for(int i = 0; i < MAX_CONNECTIONS; i++)
      {
        pFile->Read(pMachine->_inputMachines[i]);
        pFile->Read(pMachine->_outputMachines[i]);
        pFile->Read(pMachine->_inputConVol[i]);
        pFile->Read(pMachine->_wireMultiplier[i]);
        pFile->Read(pMachine->_connection[i]);
        pFile->Read(pMachine->_inputCon[i]);
      }
      {
        std::vector<char> nametemp(128);
        pFile->ReadString(&nametemp[0], nametemp.size());
        pMachine->_editName.assign( nametemp.begin(), std::find(nametemp.begin(), nametemp.end(), 0));
      }
      if(bDeleted) pMachine->_editName += " (replaced)";
      if(!fullopen) return pMachine;
      if(!pMachine->LoadSpecificChunk(pFile,version))
      {
        {
          std::ostringstream s;
          s << "Missing or Corrupted Machine Specific Chunk " << dllName << std::endl << "Replacing with Dummy.";
          std::cerr << s.str() << std::endl;
          //MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
        }
        Machine* p = new Dummy(index, pSong);
        p->Init();
        p->_type=MACH_DUMMY;
        p->_mode=pMachine->_mode;
        p->_bypass=pMachine->_bypass;
        p->_mute=pMachine->_mute;
        p->_panning=pMachine->_panning;
        p->_x = pMachine->_x;
        p->_y = pMachine->_y;
        p->_connectedInputs=pMachine->_connectedInputs;							// number of Incoming connections
        p->_connectedOutputs=pMachine->_connectedOutputs;						// number of Outgoing connections
        for(int i = 0; i < MAX_CONNECTIONS; i++)
        {
          p->_inputMachines[i]=pMachine->_inputMachines[i];
          p->_outputMachines[i]=pMachine->_outputMachines[i];
          p->_inputConVol[i]=pMachine->_inputConVol[i];
          p->_wireMultiplier[i]=pMachine->_wireMultiplier[i];
          p->_connection[i]=pMachine->_connection[i];
          p->_inputCon[i]=pMachine->_inputCon[i];
        }
        pMachine->_editName += " (replaced)";
        p->_numPars = 0;
        delete pMachine;
        pMachine = p;
      }
      if(index < MAX_BUSES)
      {
        pMachine->_mode = MACHMODE_GENERATOR;
      }
      else if (index < MAX_BUSES*2)
      {
        pMachine->_mode = MACHMODE_FX;

      }
      else
      {
        pMachine->_mode = MACHMODE_MASTER;

      }
      pMachine->SetPan(pMachine->_panning);
      return pMachine;
    }
  }
}

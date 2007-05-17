///\file
///\brief implementation file for psy::core::Song
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
#include <cassert>
#include <sstream>
namespace psy
{
	namespace core
	{
		CoreSong::CoreSong(MachineCallbacks* callbacks)
		: machinecallbacks(callbacks)
		{
			tracks_= MAX_TRACKS; // FIXME: change to 'numOfTracks_'
			_machineLock = false;
			Invalided = false;
			for(int i(0) ; i < MAX_MACHINES ; ++i) _pMachine[i] = 0;
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) _pInstrument[i] = new Instrument;
			clear();
		}

		CoreSong::~CoreSong()
		{
			DestroyAllMachines();
			DestroyAllInstruments();
		}

		void CoreSong::clear()
		{
			_machineLock = false;
			Invalided = false;

			setTracks(MAX_TRACKS);
			seqBus=0;

			setName("Untitled");
			setAuthor("Unnamed");
			setComment("No Comments");

			// General properties
			{
				setBpm(125.0f);
				setLinesPerBeat(4);
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

			machineSoloed = -1;
			_trackSoloed = -1;

			_saved=false;
			fileName = "Untitled.psy";
			
			CreateMachine(MACH_MASTER, 320, 200, "master", MASTER_INDEX);

			_trackArmedCount = 0;
			for(int i(0) ; i < MAX_TRACKS; ++i)
			{
				_trackMuted[i] = false;
				_trackArmed[i] = false;
			}
			instSelected = 0;
			midiSelected = 0;
			auxcolSelected = 0;
		}
		
		UISong::UISong(MachineCallbacks* callbacks)
		: CoreSong(callbacks)
		{
		}

		Machine * CoreSong::createMachine(const PluginFinder & finder, const PluginFinderKey & key, int x, int y ) 
		{
			int fb; 
			if ( key == PluginFinderKey::internalSampler() ) {
				fb = GetFreeBus();
				CreateMachine(MACH_SAMPLER, x, y, "SAMPLER", fb );  
			} else if ( finder.info( key ).type() == MACH_PLUGIN ) 
			{
				if ( finder.info( key ).mode() == MACHMODE_FX ) {
				fb = GetFreeFxBus();
				} else fb = GetFreeBus();
				CreateMachine(MACH_PLUGIN, x, y, key.dllPath(), fb );
			} else if ( finder.info( key ).type() == MACH_LADSPA )
			{
				fb = GetFreeFxBus();
				LADSPAMachine* plugin = new LADSPAMachine( machinecallbacks, fb, this );
				if  ( plugin->loadDll( key.dllPath(), key.index() ) ) {
					plugin->SetPosX( x );
					plugin->SetPosY( y );
					if( _pMachine[fb] )  DestroyMachine( fb );
					_pMachine[ fb ] = plugin;
				} else {				 
					delete plugin;
				}
			}
			return _pMachine[fb];
		}

		Machine & CoreSong::CreateMachine(Machine::type_type type, int x, int y, std::string const & plugin_name ) throw(std::exception)
		{
			Machine::id_type const array_index(GetFreeMachine());
			if(array_index < 0) throw std::runtime_error("sorry, psycle doesn't dynamically allocate memory.");
			if(!CreateMachine(type, x, y, plugin_name, array_index))
			throw std::runtime_error("something bad happened while i was trying to create a machine, but i forgot what it was.");
			return *_pMachine[array_index];
		}

		bool CoreSong::CreateMachine(Machine::type_type type, int x, int y, std::string const & plugin_name, Machine::id_type index )
		{
			Machine * machine(0);
			switch (type)
			{
				case MACH_MASTER:
					if(_pMachine[MASTER_INDEX]) return false;
					index = MASTER_INDEX;
					machine = new Master(machinecallbacks, index, this);
					break;
				case MACH_SAMPLER:
					machine = new Sampler(machinecallbacks, index,this);
					break;
				case MACH_XMSAMPLER:
//					machine = new XMSampler(machinecallbacks, index);
					break;
				case MACH_DUPLICATOR:
					machine = new DuplicatorMac(machinecallbacks, index, this);
					break;
				case MACH_MIXER:
					machine = new Mixer(machinecallbacks, index, this);
					break;
				case MACH_LFO:
					machine = new LFO(machinecallbacks, index, this);
					break;
//				case MACH_AUTOMATOR:
//					machine = new Automator(machinecallbacks, index);
//					break;
				case MACH_DUMMY:
					machine = new Dummy(machinecallbacks, index, this);
					break;
				case MACH_PLUGIN:
					{
						Plugin* plugin = new Plugin( machinecallbacks, index, this );
						plugin->LoadDll( plugin_name );
						machine = plugin;
					}
					break;
/*				case MACH_LADSPA:
					{
						LADSPAMachine* plugin = new LADSPAMachine(machinecallbacks,index,this);
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
					break;*/
/*				case MACH_VST:
				case MACH_VSTFX:
					{
						vst::plugin * plugin(0);
						if (type == MACH_VST) machine = plugin = new vst::instrument(machinecallbacks,index);
						else if (type == MACH_VSTFX)	machine = plugin = new vst::fx(machinecallbacks,index);
						if(!CNewMachine::TestFilename(plugin_name)) ///\todo that's a call to the GUI stuff :-(
						{
							delete plugin;
							return false;
						}
						try
						{
							plugin->Instance(plugin_name);
						}
						catch(std::exception const & e)
						{
//							loggers::exception(e.what());
							delete plugin;
							return false;
						}
						catch(...)
						{
							delete plugin;
							return false;
						}
						break;
					}
*/
				default:
//					loggers::warning("failed to create requested machine type");
//					return false;
					machine = new Dummy(machinecallbacks, index, this);
					break;
			}

			if(index < 0)
			{
				index =	GetFreeMachine();
				if(index < 0)
				{
//					loggers::warning("no more machine slots");
					return false;
				}
			}

			if(_pMachine[index]) DestroyMachine(index);

			///\todo init problem
			{
				if(machine->type() == MACH_VSTFX || machine->type() == MACH_VST )
				{
					// Do not call VST Init() function after Instance.
					machine->Machine::Init();
				}
				else machine->Init();
			}
			machine->SetPosX(x);
			machine->SetPosY(y);

			// Finally, activate the machine
			_pMachine[index] = machine;
			
			return true;
		}

		Machine::id_type CoreSong::FindBusFromIndex(Machine::id_type smac)
		{
			if(!_pMachine[smac]) return Machine::id_type(255);
			return smac;
		}

		void CoreSong::DestroyAllMachines(bool write_locked)
		{
			_machineLock = true;
			for(Machine::id_type c(0); c < MAX_MACHINES; ++c)
			{
				if(_pMachine[c])
				{
					for(Machine::id_type j(c + 1); j < MAX_MACHINES; ++j)
					{
						if(_pMachine[c] == _pMachine[j])
						{
							///\todo wtf? duplicate machine? could happen if loader messes up?
							{
								std::ostringstream s;
								s << c << " and " << j << " have duplicate pointers";
								#if defined PSYCLE__CORE__SIGNALS
									report.emit(s.str(), "duplicate machine found");
								#endif
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

		Machine::id_type CoreSong::GetFreeMachine()
		{
			Machine::id_type tmac(0);
			for(;;)
			{
				if(!_pMachine[tmac]) return tmac;
				if(tmac++ >= MAX_MACHINES) return Machine::id_type(-1); // that's why ids can't be unsigned :-(
			}
		}

		bool CoreSong::InsertConnection(Machine::id_type src, Machine::id_type dst, float volume)
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
			return srcMac->ConnectTo(*dstMac, InPort::id_type(0), OutPort::id_type(0), volume);
		}

		int CoreSong::ChangeWireDestMac(Machine::id_type wiresource, Machine::id_type wiredest, Wire::id_type wireindex)
		{
			Wire::id_type w;
			float volume = 1.0f;
			if (_pMachine[wiresource])
			{
				Machine *dmac = _pMachine[_pMachine[wiresource]->_outputMachines[wireindex]];

				if (dmac)
				{
					w = dmac->FindInputWire(wiresource);
					dmac->GetWireVolume(w,volume);
					if (InsertConnection(wiresource, wiredest,volume)) ///\todo this needs to be checked. It wouldn't allow a machine with MAXCONNECTIONS to move any wire.
					{
						// delete the old wire
						_pMachine[wiresource]->_connection[wireindex] = false;
						_pMachine[wiresource]->_outputMachines[wireindex] = -1;
						_pMachine[wiresource]->_connectedOutputs--;

						dmac->_inputCon[w] = false;
						dmac->_inputMachines[w] = -1;
						dmac->_connectedInputs--;
					}
				}
			}
			return 0;
		}

		int CoreSong::ChangeWireSourceMac(Machine::id_type wiresource, Machine::id_type wiredest, Wire::id_type wireindex)
		{
			float volume = 1.0f;

			if (_pMachine[wiredest])
			{					
				Machine *smac = _pMachine[_pMachine[wiredest]->_inputMachines[wireindex]];

				if (smac)
				{
					_pMachine[wiredest]->GetWireVolume(wireindex,volume);
					if (InsertConnection(wiresource, wiredest,volume)) ///\todo this needs to be checked. It wouldn't allow a machine with MAXCONNECTIONS to move any wire.
					{
						// delete the old wire
						int wire = smac->FindOutputWire(wiredest);
						smac->_connection[wire] = false;
						smac->_outputMachines[wire] = 255;
						smac->_connectedOutputs--;

						_pMachine[wiredest]->_inputCon[wireindex] = false;
						_pMachine[wiredest]->_inputMachines[wireindex] = -1;
						_pMachine[wiredest]->_connectedInputs--;
					}
					/*
					else
					{
						MessageBox("Machine connection failed!","Error!", MB_ICONERROR);
					}
					*/
				}
			}
			return 0;
		}

		void CoreSong::DestroyMachine(Machine::id_type mac, bool write_locked)
		{
/*			#if !defined PSYCLE__CONFIGURATION__READ_WRITE_MUTEX
				#error PSYCLE__CONFIGURATION__READ_WRITE_MUTEX isn't defined anymore, please clean the code where this error is triggered.
			#else
				#if PSYCLE__CONFIGURATION__READ_WRITE_MUTEX // new implementation
					boost::read_write_mutex::scoped_write_lock lock(read_write_mutex(), !write_locked); // only lock if not already locked
				#else // original implementation
					CSingleLock lock(&door, true);
				#endif
			#endif*/
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

		int CoreSong::GetFreeBus()
		{
			for(int c(0) ; c < MAX_BUSES ; ++c) if(!_pMachine[c]) return c;
			return -1; 
		}

		int CoreSong::GetFreeFxBus()
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

		bool CoreSong::IffAlloc(Instrument::id_type instrument,const char * str)
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

		bool CoreSong::WavAlloc(Instrument::id_type iInstr, bool bStereo, long iSamplesPerChan, const char * sName)
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

		bool CoreSong::WavAlloc(Instrument::id_type instrument,const char * Wavfile)
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

		bool CoreSong::load(const std::string & fileName)
		{
			PsyFilter filter;
			return filter.loadSong(fileName, *this, machinecallbacks);
		}

		bool CoreSong::save(const std::string & fileName)
		{
			PsyFilter filter;
			return filter.saveSong(fileName, *this,4);
		}

		void CoreSong::DoPreviews(int amount)
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

		///\todo mfc+winapi->std
		bool CoreSong::CloneMac(Machine::id_type src, Machine::id_type dst)
		{
			// src has to be occupied and dst must be empty
			if (_pMachine[src] && _pMachine[dst])
			{
				return false;
			}
			if (_pMachine[dst])
			{
				int temp = src;
				src = dst;
				dst = temp;
			}
			if (!_pMachine[src])
			{
				return false;
			}
			// check to see both are same type
			if (((dst < MAX_BUSES) && (src >= MAX_BUSES))
				|| ((dst >= MAX_BUSES) && (src < MAX_BUSES)))
			{
				return false;
			}

			if ((src >= MAX_MACHINES-1) || (dst >= MAX_MACHINES-1))
			{
				return false;
			}

			// save our file

/*			boost::filesystem::path path(Global::configuration().GetSongDir(), boost::filesystem::native);
			path /= "psycle.tmp";

			boost::filesystem::remove(path);

			RiffFile file;
			if(!file.Create(path.string(), true)) return false;

			file.WriteChunk("MACD",4);
			std::uint32_t version = CURRENT_FILE_VERSION_MACD;
			file.Write(version);
			std::fpos_t pos = file.GetPos();
			std::uint32_t size = 0;
			file.Write(size);

			std::uint32_t index = dst; // index
			file.Write(index);

			_pMachine[src]->SaveFileChunk(&file);

			std::fpos_t pos2 = file.GetPos(); 
			size = pos2 - pos - sizeof size;
			file.Seek(pos);
			file.Write(size);
			file.Close();

			// now load it

			if(!file.Open(path.string()))
			{
				boost::filesystem::remove(path);
				return false;
			}

			char Header[5];
			file.ReadChunk(&Header, 4);
			Header[4] = 0;
			if (strcmp(Header,"MACD")==0)
			{
				file.Read(version);
				file.Read(size);
				if (version > CURRENT_FILE_VERSION_MACD)
				{
					// there is an error, this file is newer than this build of psycle
					file.Close();
					boost::filesystem::remove(path);
					return false;
				}
				else
				{
					file.Read(index);
					index = dst;
					if (index < MAX_MACHINES)
					{
						Machine::id_type id(index);
						// we had better load it
						DestroyMachine(id);
						_pMachine[index] = Machine::LoadFileChunk(&file,id,version);
					}
					else
					{
						file.Close();
						boost::filesystem::remove(path);
						return false;
					}
				}
			}
			else
			{
				file.Close();
				boost::filesystem::remove(path);
				return false;
			}
			file.Close();
			boost::filesystem::remove(path);

			_pMachine[dst]->SetPosX(_pMachine[dst]->GetPosX()+32);
			_pMachine[dst]->SetPosY(_pMachine[dst]->GetPosY()+8);

			// delete all connections

			_pMachine[dst]->_connectedInputs = 0;
			_pMachine[dst]->_connectedOutputs = 0;

			for (int i = 0; i < MAX_CONNECTIONS; i++)
			{
				if (_pMachine[dst]->_connection[i])
				{
					_pMachine[dst]->_connection[i] = false;
					_pMachine[dst]->_outputMachines[i] = -1;
				}

				if (_pMachine[dst]->_inputCon[i])
				{
					_pMachine[dst]->_inputCon[i] = false;
					_pMachine[dst]->_inputMachines[i] = -1;
				}
			}

			#if 1
			{
				std::stringstream s;
				s << _pMachine[dst]->_editName << " " << std::hex << dst << " (cloned from " << std::hex << src << ")";
				s >> _pMachine[dst]->_editName;
			}
			#else ///\todo rewrite this for std::string
				int number = 1;
				char buf[sizeof(_pMachine[dst]->_editName)+4];
				strcpy (buf,_pMachine[dst]->_editName);
				char* ps = strrchr(buf,' ');
				if (ps)
				{
					number = atoi(ps);
					if (number < 1)
					{
						number =1;
					}
					else
					{
						ps[0] = 0;
						ps = strchr(_pMachine[dst]->_editName,' ');
						ps[0] = 0;
					}
				}

				for (int i = 0; i < MAX_MACHINES-1; i++)
				{
					if (i!=dst)
					{
						if (_pMachine[i])
						{
							if (strcmp(_pMachine[i]->_editName,buf)==0)
							{
								number++;
								sprintf(buf,"%s %d",_pMachine[dst]->_editName.c_str(),number);
								i = -1;
							}
						}
					}
				}

				buf[sizeof(_pMachine[dst]->_editName)-1] = 0;
				strcpy(_pMachine[dst]->_editName,buf);
			#endif*/

			return true;
		}

		///\todo mfc+winapi->std
		bool CoreSong::CloneIns(Instrument::id_type src, Instrument::id_type dst)
		{
			// src has to be occupied and dst must be empty
		/*	if (!Global::song()._pInstrument[src]->Empty() && !Global::song()._pInstrument[dst]->Empty())
			{
				return false;
			}
			if (!Global::song()._pInstrument[dst]->Empty())
			{
				int temp = src;
				src = dst;
				dst = temp;
			}
			if (Global::song()._pInstrument[src]->Empty())
			{
				return false;
			}
			// ok now we get down to business
			// save our file

			CString filepath = Global::configuration().GetSongDir().c_str();
			filepath += "\\psycle.tmp";
			::DeleteFile(filepath);
			RiffFile file;
			if (!file.Create(filepath.GetBuffer(1), true))
			{
				return false;
			}

			file.WriteChunk("INSD",4);
			std::uint32_t version = CURRENT_FILE_VERSION_INSD;
			file.Write(version);
			std::fpos_t pos = file.GetPos();
			std::uint32_t size = 0;
			file.Write(size);

			std::uint32_t index = dst; // index
			file.Write(index);

			_pInstrument[src]->SaveFileChunk(&file);

			std::fpos_t pos2 = file.GetPos(); 
			size = pos2 - pos - sizeof size;
			file.Seek(pos);
			file.Write(size);

			file.Close();

			// now load it

			if (!file.Open(filepath.GetBuffer(1)))
			{
				DeleteFile(filepath);
				return false;
			}
			char Header[5];
			file.ReadChunk(&Header, 4);
			Header[4] = 0;

			if (strcmp(Header,"INSD")==0)
			{
				file.Read(version);
				file.Read(size);
				if (version > CURRENT_FILE_VERSION_INSD)
				{
					// there is an error, this file is newer than this build of psycle
					file.Close();
					DeleteFile(filepath);
					return false;
				}
				else
				{
					file.Read(index);
					index = dst;
					if (index < MAX_INSTRUMENTS)
					{
						// we had better load it
						_pInstrument[index]->LoadFileChunk(&file,version);
					}
					else
					{
						file.Close();
						DeleteFile(filepath);
						return false;
					}
				}
			}
			else
			{
				file.Close();
				DeleteFile(filepath);
				return false;
			}
			file.Close();
			DeleteFile(filepath);*/
			return true;
		}

		void /*Reset*/CoreSong::DeleteInstrument(Instrument::id_type id)
		{
			Invalided=true;
			_pInstrument[id]->Delete(); Invalided=false;
		}
	    
	    void /*Reset*/CoreSong::DeleteInstruments()
	    {
			Invalided=true;
			for(Instrument::id_type id(0) ; id < MAX_INSTRUMENTS ; ++id)
			_pInstrument[id]->Delete();
			Invalided=false;
		}
	    
	    void /*Delete*/CoreSong::DestroyAllInstruments()
	    {
			for(Instrument::id_type id(0) ; id < MAX_INSTRUMENTS ; ++id) {
				delete _pInstrument[id];
				_pInstrument[id] = 0;
			}
	    }
	    
	    void CoreSong::DeleteLayer(Instrument::id_type id)
	    {
			_pInstrument[id]->DeleteLayer();
		}
	
		void CoreSong::patternTweakSlide(int machine, int command, int value, int patternPosition, int track, int line)
		{
			///\todo reowkr for multitracking
			/*  bool bEditMode = true;

			// UNDO CODE MIDI PATTERN TWEAK
			if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
			if (value > 0xffff) value = 0xffff;// no else incase of neg overflow
			//if(viewMode == VMPattern && bEditMode)
			{ 
				// write effect
				const int ps = playOrder[patternPosition];
				int line = Global::pPlayer()->_lineCounter;
				unsigned char * toffset;
				if (Global::pPlayer()->_playing&&Global::pConfig()->_followSong)
				{
					if(_trackArmedCount)
					{
						//SelectNextTrack();
					}
					else if (!Global::pConfig()->_RecordUnarmed)
					{	
						return;
					}
					toffset = _ptrack(ps,track)+(line*MULTIPLY);
				}
				else
				{
					toffset = _ptrackline(ps, track, line);
				}

				// build entry
				PatternEntry *entry = (PatternEntry*) toffset;
				if (entry->_note >= 120)
				{
					if ((entry->_mach != machine) || (entry->_cmd != ((value>>8)&255)) || (entry->_parameter != (value&255)) || (entry->_inst != command) || ((entry->_note != cdefTweakM) && (entry->_note != cdefTweakE) && (entry->_note != cdefTweakS)))
					{
						//AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
						entry->_mach = machine;
						entry->_cmd = (value>>8)&255;
						entry->_parameter = value&255;
						entry->_inst = command;
						entry->_note = cdefTweakS;

						//NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
						//Repaint(DMData);
					}
				}
			}*/
		}

		void CoreSong::setTracks( unsigned int trackCount )
		{
			tracks_ = trackCount;
		}

		void CoreSong::setName( const std::string & name )
		{
			name_ = name;
		}

		void CoreSong::setAuthor( const std::string & author )
		{
			author_ = author;
		}

		void CoreSong::setComment( const std::string & comment )
		{
			comment_ = comment;
		}

		void CoreSong::setBpm( float bpm )
		{
			if (bpm > 0 && bpm < 1000) bpm_ = bpm;
		}

		void CoreSong::setLinesPerBeat(const unsigned int value)
		{
			if ( value < 1 ) linesPerBeat_ = 1;
			else if ( value > 31 ) linesPerBeat_ = 31;
			else linesPerBeat_ = value;
		}
	}
}

///\file
///\brief implementation file for psycle::host::Machine
#include "machine.h"
#include "song.h"
#include "dsp.h"
#include "configuration.h"
#include <algorithm>
#include "plugin_interface.h"
#include "player.h"

#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace psycle {
  namespace host {

    Machine::Machine( int type, int mode, int id, Song * song )
      :
    _type(type),
      _mode(mode),
      _macIndex(id),
      _bypass(false),
      _mute(false),
      _waitingForSound(false),
      _stopped(false),
      _worked(false),
      _audiorange(1.0f),
      _pSamplesL(0),
      _pSamplesR(0),
      _lVol(0),
      _rVol(0),
      _panning(0),
      _x(0),
      _y(0),
      _numPars(0),
      _nCols(1),
      _connectedInputs(0),
      numInPorts(0),
      numOutPorts(0),
      _connectedOutputs(0),
      TWSSamples(0),
      TWSActive(false),
      _volumeCounter(0.0f),
      _volumeDisplay(0),
      _volumeMaxDisplay(0),
      _volumeMaxCounterLife(0),
      _pScopeBufferL(0),
      _pScopeBufferR(0),
      _scopeBufferIndex(0),
      _scopePrevNumSamples(0),
      _editName("")

    {
      _outDry = 256;
      _pSong = song;
      _pSamplesL = new float[MAX_BUFFER_LENGTH];
      _pSamplesR = new float[MAX_BUFFER_LENGTH];
      // Clear machine buffer samples
      for (int c=0; c<MAX_BUFFER_LENGTH; c++)
      {
        _pSamplesL[c] = 0;
        _pSamplesR[c] = 0;
      }
      for (int c = 0; c<MAX_TRACKS; c++)
      {
        TriggerDelay[c].setCommand( 0 );
        TriggerDelayCounter[c]=0;
        RetriggerRate[c]=256;
        ArpeggioCount[c]=0;
      }
      for (int c = 0; c<MAX_TWS; c++)
      {
        TWSInst[c] = 0;
        TWSDelta[c] = 0;
        TWSCurrent[c] = 0;
        TWSDestination[c] = 0;
      }

      for (int i = 0; i<MAX_CONNECTIONS; i++)
      {
        _inputMachines[i]=-1;
        _outputMachines[i]=-1;
        _inputConVol[i]=0.0f;
        _wireMultiplier[i]=0.0f;
        _connection[i]=false;
        _inputCon[i]=false;
      }
    }

    Machine::~Machine() 
    {
      delete _pSamplesL;
      delete _pSamplesR;
    }

    void Machine::Init()
    {
      // Standard gear initalization    
      _mute = false;
      _stopped = false;
      _bypass = false;
      _waitingForSound = false;
      // Centering volume and panning
      SetPan(64);
      // Clearing connections
      for(int i=0; i<MAX_CONNECTIONS; i++)
      {
        _inputConVol[i] = 1.0f;
        _wireMultiplier[i] = 1.0f;
        _inputMachines[i]=-1;
        _outputMachines[i]=-1;
        _inputCon[i] = false;
        _connection[i] = false;
      }
      _connectedInputs = 0;
      _connectedOutputs = 0;
    }

    void Machine::SetPan(int newPan)
    {
      if (newPan < 0)
      {
        newPan = 0;
      }
      if (newPan > 128)
      {
        newPan = 128;
      }
      _rVol = newPan * 0.015625f;
      _lVol = 2.0f-_rVol;
      if (_lVol > 1.0f)
      {
        _lVol = 1.0f;
      }
      if (_rVol > 1.0f)
      {
        _rVol = 1.0f;
      }
      _panning = newPan;
    }

    void Machine::InitWireVolume(int type, int wire, float value)
    {
      if (type == MACH_VST || type == MACH_VSTFX  || type == MACH_LADSPA)
      {
        if (this->_type == MACH_VST || this->_type == MACH_VSTFX || this->_type == MACH_LADSPA) // VST to VST, no need to convert.
        {
          _inputConVol[wire] = value;
          _wireMultiplier[wire] = 1.0f;
        }
        else											// VST to native, multiply
        {
          _inputConVol[wire] = value * 32768.0f;
          _wireMultiplier[wire] = 0.000030517578125f; // what is it?
        }
      }
      else if (this->_type == MACH_VST || this->_type == MACH_VSTFX || this->_type == MACH_LADSPA ) // native to VST, divide.
      {
        _inputConVol[wire] = value * 0.000030517578125f; // what is it?
        _wireMultiplier[wire] = 32768.0f;
      }
      else												// native to native, no need to convert.
      {
        _inputConVol[wire] = value;
        _wireMultiplier[wire] = 1.0f;
      }	
      // The reason of the conversions in the case of MACH_VST is because VST's output wave data
      // in the range -1.0 to +1.0, while native and internal output at -32768.0 to +32768.0
      // Initially (when the format was made), Psycle did convert this in the "Work" function,
      // but since it already needs to multiply the output by inputConVol, I decided to remove
      // that extra conversion and use directly the volume to do so.
    }

    int Machine::FindInputWire(int id)
    {
      for(int c(0); c < MAX_CONNECTIONS; ++c)
        if(_inputCon[c])
          if(_inputMachines[c] == id)
            return c;
      return int(-1);
    }

    int Machine::FindOutputWire(int id)
    {
      for(int c(0); c < MAX_CONNECTIONS; ++c)
        if(_connection[c])
          if(_outputMachines[c] == id)
            return c;
      return int(-1);
    }


    bool Machine::SetDestWireVolume(int srcIndex, int WireIndex,float value)
    {
      // Get reference to the destination machine
      if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
      Machine *_pDstMachine = _pSong->_pMachine[_outputMachines[WireIndex]];
      if (_pDstMachine)
      {
        int c;
        if((c = _pDstMachine->FindInputWire(srcIndex)) != -1)
        {
          _pDstMachine->SetWireVolume(c,value);
          return true;
        }
      }
      return false;
    }

    bool Machine::GetDestWireVolume(int srcIndex, int WireIndex,float &value)
    {
      // Get reference to the destination machine
      if ((WireIndex > MAX_CONNECTIONS) || (!_connection[WireIndex])) return false;
      Machine *_pDstMachine = _pSong->_pMachine[_outputMachines[WireIndex]];
      if (_pDstMachine)
      {
        int c;
        if((c = _pDstMachine->FindInputWire(srcIndex)) != -1)
        {
          //float val;
          _pDstMachine->GetWireVolume(c,value);
          //value = f2i(val*256.0f);
          return true;
        }
      }
      return false;
    }

    void Machine::PreWork(int numSamples)
    {
      _worked = false;
      _waitingForSound= false;
      if (_pScopeBufferL && _pScopeBufferR)
      {
        float *pSamplesL = _pSamplesL;   
        float *pSamplesR = _pSamplesR;   
        int i = _scopePrevNumSamples;
        while (i > 0)   
        {   
          if (i+_scopeBufferIndex >= SCOPE_BUF_SIZE)   
          {   
            memcpy(&_pScopeBufferL[_scopeBufferIndex],pSamplesL,(SCOPE_BUF_SIZE-(_scopeBufferIndex)-1)*sizeof(float));
            memcpy(&_pScopeBufferR[_scopeBufferIndex],pSamplesR,(SCOPE_BUF_SIZE-(_scopeBufferIndex)-1)*sizeof(float));
            pSamplesL+=(SCOPE_BUF_SIZE-(_scopeBufferIndex)-1);
            pSamplesR+=(SCOPE_BUF_SIZE-(_scopeBufferIndex)-1);
            i -= (SCOPE_BUF_SIZE-(_scopeBufferIndex)-1);
            _scopeBufferIndex = 0;   
          }   
          else   
          {   
            memcpy(&_pScopeBufferL[_scopeBufferIndex],pSamplesL,i*sizeof(float));   
            memcpy(&_pScopeBufferR[_scopeBufferIndex],pSamplesR,i*sizeof(float));   
            _scopeBufferIndex += i;   
            i = 0;   
          }   
        } 
      }
      _scopePrevNumSamples=numSamples;
      dsp::Clear(_pSamplesL, numSamples);
      dsp::Clear(_pSamplesR, numSamples);    
    }

    /// Each machine is expected to produce its output in its own
    /// _pSamplesX buffers.
    void Machine::Work(int numSamples )
    {
      _waitingForSound=true;
      for (int i=0; i<MAX_CONNECTIONS; i++)
      {
        if (_inputCon[i])
        {
          Machine* pInMachine = _pSong->_pMachine[_inputMachines[i]];
          if (pInMachine)
          {
            if (!pInMachine->_worked && !pInMachine->_waitingForSound)
            { 
              {
                pInMachine->Work( numSamples );
              }              
              pInMachine->_waitingForSound = false;
            }
            if(!pInMachine->_stopped) _stopped = false;
            if(!_mute && !_stopped)
            {
              dsp::Add(pInMachine->_pSamplesL, _pSamplesL, numSamples, pInMachine->_lVol*_inputConVol[i]);
              dsp::Add(pInMachine->_pSamplesR, _pSamplesR, numSamples, pInMachine->_rVol*_inputConVol[i]);
            }
          }
        }
      }
      dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
      GenerateAudio( numSamples );
    }

    //Modified version of Machine::Work(). The only change is the removal of mixing inputs into one stream.
    void Machine::WorkNoMix( int numSamples )
    {
      _waitingForSound=true;
      for (int i=0; i<MAX_CONNECTIONS; i++)
      {
        if (_inputCon[i])
        {
          Machine* pInMachine = _pSong->_pMachine[_inputMachines[i]];
          if (pInMachine)
          {
            if (!pInMachine->_worked && !pInMachine->_waitingForSound)
            { 
              pInMachine->Work( numSamples );
              pInMachine->_waitingForSound = false;
            }
            if(!pInMachine->_stopped) _stopped = false;
          }
        }
      }
    }


    bool Machine::ConnectTo(Machine & dst_machine, int dstport, int outport, float volume)
    {
      if(dst_machine._mode == MACHMODE_GENERATOR)
      {
        std::ostringstream s;
        s << "attempted to use a generator as destination for wire" << this->id() << " -> " << dst_machine.id();
        //loggers::warning(s.str());
        std::cerr << s.str() << std::endl;
        return false;
      }

      // Get a free output slot on the source machine
      int freebus(-1);
      {
        bool error = false;
        for(int c(MAX_CONNECTIONS - 1); c >= 0; --c)
        {
          if(!_connection[c]) freebus = c;
          // Checking that there's not a slot to the dest. machine already
          else if(_outputMachines[c] == dst_machine.id()) error = true;
        }
        // lamely abandon
        if(freebus == -1 || error) return false;
      }

      // Get a free input slot on the destination machine
      int dfreebus(-1);
      {
        bool error = false;
        for(int c(MAX_CONNECTIONS - 1); c >= 0; --c)
        {
          if(!dst_machine._inputCon[c]) dfreebus = c;
          // Checking if the destination machine is connected with the source machine to avoid a loop.
          else if(dst_machine._outputMachines[c] == this->id()) error = true;
        }
        // lamely abandon
        if(dfreebus == -1 || error) return false;
      }

      // Calibrating in/out properties
      this->_outputMachines[freebus] = dst_machine.id();
      this->_connection[freebus] = true;
      this->_connectedOutputs++;
      dst_machine._inputMachines[dfreebus] = this->id();
      dst_machine._inputCon[dfreebus] = true;
      dst_machine._connectedInputs++;
      dst_machine.InitWireVolume(_type, dfreebus, volume);
      return true;
    }


    bool Machine::LoadSpecificChunk(RiffFile* pFile, int version)
    {
      std::uint32_t size;
      pFile->Read(size);
      std::uint32_t count;
      pFile->Read(count);
      for(std::uint32_t i(0); i < count; ++i)
      {
        std::uint32_t temp;
        pFile->Read(temp);
        SetParameter(i,temp);
      }
      pFile->Skip(size - sizeof count - count * sizeof(std::uint32_t));
      return true;
    };

   


    void Machine::SaveFileChunk(RiffFile* pFile)
    {
      pFile->Write(_type);
      SaveDllName(pFile);
      pFile->Write(_bypass);
      pFile->Write(_mute);
      pFile->Write(_panning);
      pFile->Write(_x);
      pFile->Write(_y);
      pFile->Write(_connectedInputs);
      pFile->Write(_connectedOutputs);
      for(int i = 0; i < MAX_CONNECTIONS; i++)
      {
        pFile->Write(_inputMachines[i]);
        pFile->Write(_outputMachines[i]);
        pFile->Write(_inputConVol[i]);
        pFile->Write(_wireMultiplier[i]);
        pFile->Write(_connection[i]);
        pFile->Write(_inputCon[i]);
      }
      pFile->WriteChunk(GetEditName().c_str(), GetEditName().length()+1);	//a max of 128 chars will be read on song load, but there's no real
      //reason to limit what gets saved here.. (is there?)
      SaveSpecificChunk(pFile);
    }

    void Machine::SaveSpecificChunk(RiffFile* pFile) 
    {
      std::uint32_t count = GetNumParams();
      std::uint32_t const size(sizeof count  + count * sizeof(std::uint32_t));
      pFile->Write(size);
      pFile->Write(count);
      for(unsigned int i = 0; i < count; i++)
      {
        std::uint32_t temp = GetParamValue(i);
        pFile->Write(temp);
      }
    }

    void Machine::SaveDllName(RiffFile* pFile)
    {
      char temp=0;
      pFile->Write(temp);
    }

    void Machine::AddEvent( double offset, int track, const PatternEvent & event )
    {
      workEvents.push_back( WorkEvent(offset,track,event));
    }

    WorkEvent::WorkEvent( )
    {
    }

    WorkEvent::WorkEvent( double beatOffset, int track, const PatternEvent & patternEvent )
      : offset_(beatOffset)
      , track_(track)
      , event_(patternEvent)
    {
    }

    const PatternEvent&  psycle::host::WorkEvent::event( ) const
    {
      return event_;
    }

    double WorkEvent::beatOffset() const
    {
      return offset_;
    }

    int WorkEvent::track() const
    {
      return track_;
    }

    int Machine::GenerateAudioInTicks(int startSample, int numsamples )
    {
      workEvents.clear();
      return 0;
    }

    int Machine::GenerateAudio( int numsamples )
    {
      const PlayerTimeInfo & timeInfo = Player::Instance()->timeInfo();
      //position [0.0-1.0] inside the current beat.
      const double positionInBeat = timeInfo.playBeatPos() - static_cast<int>(timeInfo.playBeatPos()); 
      //position [0.0-linesperbeat] converted to "Tick()" lines
      const double positionInLines = positionInBeat*Player::Instance()->timeInfo().linesPerBeat();
      //position in samples of the next "Tick()" Line
      int nextLineInSamples = static_cast<int>( (1.0-(positionInLines-static_cast<int>(positionInLines)))* timeInfo.samplesPerRow() );
      //Next event, initialized to "out of scope".
      int nextevent = numsamples+1;
      int previousline = nextLineInSamples;
      std::map<int,int>::iterator colsIt;

      // check for next event.
      if (!workEvents.empty())
      {
        WorkEvent & workEvent = workEvents.front();
        nextevent = static_cast<int>( workEvent.beatOffset() * timeInfo.samplesPerBeat() );
        // correcting rounding errors.
        if ( nextevent == nextLineInSamples+1 ) nextLineInSamples = nextevent;
      }
      int samplestoprocess = 0;
      int processedsamples = 0;
      for(;processedsamples<numsamples; processedsamples+=samplestoprocess)
      {
        if ( processedsamples == nextLineInSamples )
        {
          Tick( );
          previousline = nextLineInSamples;
          nextLineInSamples += static_cast<int>( timeInfo.samplesPerRow() ); 
        }


        while ( processedsamples == nextevent  )
        {
          if ( !workEvents.empty() ) {
            WorkEvent & workEvent = *workEvents.begin();
            ///\todo: beware of using more than MAX_TRACKS. "Stop()" resets the list, but until that, playColIndex keeps increasing.
            colsIt = playCol.find(workEvent.track());
            if ( colsIt == playCol.end() ) { playCol[workEvent.track()]=playColIndex++;  colsIt = playCol.find(workEvent.track()); }
            Tick(colsIt->second, workEvent.event() );
            workEvents.pop_front();
            if (!workEvents.empty())
            {
              WorkEvent & workEvent1 = *workEvents.begin();
              //	nextevent = (workEvent.beatOffset() - beatOffset) * Global::player().SamplesPerBeat();
              nextevent = static_cast<int>( workEvent1.beatOffset() * timeInfo.samplesPerBeat() );
            } else nextevent = numsamples+1;
          } else nextevent = numsamples+1;
        } 

        //minimum between remaining samples, next "Tick()" and next event
        samplestoprocess= std::min(numsamples,std::min(nextLineInSamples,nextevent))-processedsamples;
        //		samplestoprocess= std::min(numsamples,nextevent)-processedsamples;

        if ( (processedsamples !=0 && processedsamples+ samplestoprocess != numsamples) || samplestoprocess <= 0)
        {
          std::cout << "GenerateAudio:" << processedsamples << "-" << samplestoprocess << "-" << nextLineInSamples << "(" << previousline << ")" << "-" << nextevent << std::endl;
        }
        GenerateAudioInTicks( processedsamples, samplestoprocess );
      }
      // reallocate events remaining in the buffer, This happens when soundcard buffer is bigger than STREAM_SIZE (machine buffer).
      //	Since events are generated once per soundcard work(), events have to be reallocated for the next machine Work() call.
      reallocateRemainingEvents( numsamples/ timeInfo.samplesPerBeat() ); 

      return processedsamples;
    }

    Song * Machine::song( )
    {
      return _pSong;
    }

    void Machine::reallocateRemainingEvents(double beatOffset)
    {
      std::deque<WorkEvent>::iterator it = workEvents.begin();
      while(it != workEvents.end())
      {
        it->changeposition(it->beatOffset()-beatOffset);
        it++;
      }
    }

  }
}

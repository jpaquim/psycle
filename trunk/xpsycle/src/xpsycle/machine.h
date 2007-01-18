///\file
///\brief interface file for psycle::host::Machine
#pragma once
#include "patternevent.h"
#include "songstructs.h"
#include "dsp.h"
#include "helpers.h"
#include "constants.h"
#include "fileio.h"
#include "playertimeinfo.h"
#include "cstdint.h"
#include <stdexcept>
#include <vector>
#include <deque>
#include <cassert>
#include <map>

namespace psycle
{
  namespace host
  {

    class Machine; // forward declaration
    class Song;

    const int MAX_SCOPE_BANDS = 128;
    const int SCOPE_BUF_SIZE  = 4096;
    const int SCOPE_SPEC_SAMPLES = 256;

    struct CPoint {
      int x;
      int y;
    };

    enum MachineType
    {
      MACH_UNDEFINED = -1, //< :-(
      MACH_MASTER = 0,
      MACH_SINE = 1, //< for psycle old fileformat version 2
      MACH_DIST = 2, //< for psycle old fileformat version 2
      MACH_SAMPLER = 3,
      MACH_DELAY = 4, //< for psycle old fileformat version 2
      MACH_2PFILTER = 5, //< for psycle old fileformat version 2
      MACH_GAIN = 6, //< for psycle old fileformat version 2
      MACH_FLANGER = 7, //< for psycle old fileformat version 2
      MACH_PLUGIN = 8,
      MACH_VST = 9,
      MACH_VSTFX = 10,
      MACH_SCOPE = 11,
      MACH_XMSAMPLER = 12,
      MACH_DUPLICATOR = 13,
      MACH_MIXER = 14,			
      MACH_LFO = 15,
      MACH_LADSPA = 16,
      MACH_DUMMY = 255			
    };

    enum MachineMode
    {
      MACHMODE_UNDEFINED = -1, //< :-(
      MACHMODE_GENERATOR = 0,
      MACHMODE_FX = 1,
      MACHMODE_MASTER = 2,
    };

    class WorkEvent {
    public:
      WorkEvent();
      WorkEvent( double beatOffset, int track, const PatternEvent& patternEvent );
      const PatternEvent &  event() const;
      double beatOffset() const;
      int track() const;
      void changeposition(double beatOffset) { offset_ = beatOffset; }

    private:

      double offset_;
      int track_;
      PatternEvent event_;

    };

    /// Base class for "Machines", the audio producing elements.
    /// monolith
    class Machine
    {

   public:

      Machine(int type, int mode, int id, Song * song);
      virtual ~Machine();
      Song* song();

      virtual Machine* clone() const = 0;


    public:
      virtual int GenerateAudioInTicks(int startSample, int numsamples );
      virtual int GenerateAudio(int numsamples );
      virtual void AddEvent(double offset, int track, const PatternEvent & event);
      virtual void reallocateRemainingEvents(double beatOffset);

    protected:
      std::deque<WorkEvent> workEvents;
      std::map<int,int> playCol;
      int playColIndex;

    public:
      int _type;
      int type() const  { 
        return _type; 
      }
      virtual void InitWireVolume(int, int, float value);
      virtual int FindInputWire( int );
      virtual int FindOutputWire( int );
      
    public:

      int inline mode() const  { return _mode; }
      int _mode;
      int id() const  { return _macIndex; }
      int _macIndex;


 
      virtual bool ConnectTo(Machine & dst, int dstport = 0, int outport = 0, float volume = 1.0f);
    private:
      Song* _pSong;

    public:
      virtual void Init();
      virtual void PreWork(int numSamples);
      virtual void Work(int numSamples );
      virtual void WorkNoMix(int numSamples );
      virtual void Tick( ) {};
      virtual void Tick(int channel, const PatternEvent & data ) {}
      virtual void Stop() { playCol.clear(); playColIndex =0; }

      virtual void SaveDllName( RiffFile* pFile);
      virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
      
      virtual void SaveFileChunk(RiffFile * pFile);
      virtual void SaveSpecificChunk(RiffFile * pFile);
      /// Loader for psycle fileformat version 2.
      virtual bool LoadPsy2FileFormat(RiffFile* pFile);

    public:
      virtual void SetSampleRate(int hertz) {  
      };
      virtual void SetPan(int newpan);


      virtual float GetAudioRange() { return _audiorange; }

      virtual void GetWireVolume(int wire, float & result) 
      { 
        result = _inputConVol[wire] * _wireMultiplier[wire]; 
      }
      virtual void SetWireVolume(int wire, float value) { 
        _inputConVol[wire] = value / _wireMultiplier[wire]; 
      }
      virtual bool GetDestWireVolume( int src, int, float & result);
      virtual bool SetDestWireVolume( int src, int, float value);

      virtual std::string GetDllName() const { return "built-in"; };
      virtual std::string GetName() const = 0;
      virtual std::string const &GetEditName() { return _editName; }
      virtual void SetEditName(std::string editname) { _editName = editname; }

      std::string  _editName;

      virtual int GetNumCols() { return _nCols; };
      virtual int GetNumParams() { return _numPars; };
      virtual void GetParamName(int numparam, char * name) { name[0]='\0'; };
      virtual void GetParamRange(int numparam, int &minval, int &maxval) {minval=0; maxval=0; };
      virtual void GetParamValue(int numparam, char * parval) { parval[0]='\0'; };
      virtual int GetParamValue(int numparam) { return 0; };
      virtual bool SetParameter(int numparam, int value) { return false;}; 

    protected:
      void SetVolumeCounter(int numSamples);

    public:
      int numInPorts;
      int numOutPorts;
      bool _bypass;
      bool _mute;
      bool _waitingForSound;
      bool _stopped;
      bool _worked;
      float _audiorange;
      /// left data
      float *_pSamplesL;
      /// right data
      float *_pSamplesR;
      /// left chan volume
      float _lVol;
      /// right chan volume
      float _rVol;
      /// numerical value of panning.
      int _panning;
      int _x;
      int _y;
      int _numPars;
      int _nCols;

      /// number of Incoming connections
      int _connectedInputs;
      /// Incoming connections Machine numbers				
      int _inputMachines[MAX_CONNECTIONS];
      /// Incoming connections activated				
      bool _inputCon[MAX_CONNECTIONS];
      /// Incoming connections Machine volumes				
      float _inputConVol[MAX_CONNECTIONS];
      /// Value to multiply _inputConVol[] to have a 0.0...1.0 range				
      float _wireMultiplier[MAX_CONNECTIONS];

      /// number of Outgoing connections
      int _connectedOutputs;
      /// Outgoing connections Machine numbers
      int _outputMachines[MAX_CONNECTIONS];
      /// Outgoing connections activated				
      bool _connection[MAX_CONNECTIONS];

      /// The topleft point of a square where the wire triangle is centered when drawn. (Used to detect when to open the wire dialog)				
      CPoint _connectionPoint[MAX_CONNECTIONS];

      /// output peak level for DSP
      float _volumeCounter;					
      /// output peak level for display
      int _volumeDisplay;	
      /// output peak level for display
      int _volumeMaxDisplay;
      /// output peak level for display
      int _volumeMaxCounterLife;
      int _scopePrevNumSamples;
      int	_scopeBufferIndex;
      ///\todo doc
      float *_pScopeBufferL;
      ///\todo doc
      float *_pScopeBufferR;

      /// various player-related states
      PatternEvent TriggerDelay[MAX_TRACKS];				
      int TriggerDelayCounter[MAX_TRACKS];				
      int RetriggerRate[MAX_TRACKS];				
      int ArpeggioCount[MAX_TRACKS];
      bool TWSActive;				
      int TWSInst[MAX_TWS];
      int TWSSamples;				
      float TWSDelta[MAX_TWS];
      float TWSCurrent[MAX_TWS];
      float TWSDestination[MAX_TWS];

      float _lMax;
      float _rMax;
      int _outDry;
    };

    inline void Machine::SetVolumeCounter(int numSamples)
    {
      _volumeCounter = dsp::GetMaxVol(_pSamplesL, _pSamplesR, numSamples);
      if(_volumeCounter > 32768.0f) _volumeCounter = 32768.0f;
      int temp((f2i(fast_log2(_volumeCounter) * 78.0f * 4 / 14.0f) - (78 * 3)));// not 100% accurate, but looks as it sounds
      // prevent downward jerkiness
      if(temp > 97) temp = 97;
      else if (temp <0) temp=0;
      if(temp > _volumeDisplay) _volumeDisplay = temp;
      --_volumeDisplay;
    };
  }
}

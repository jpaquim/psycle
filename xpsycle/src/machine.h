/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
 *   natti@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef MACHINE_H
#define MACHINE_H


#include "dsp.h"
#include "song.h"
#include <stdexcept>


class DeSerializer;
class Serializer;


/**
@author Stefan
*/

class CPoint {
public:
   int x;
   int y;
};

#define CPUCOST_INIT(cost) long long cost = 0;

/*
long long cost; \
__asm rdtsc \
__asm mov cost, eax*/


class Machine{
public:
    Machine();

    virtual ~Machine();

    static Machine * LoadFileChunk(DeSerializer* pFile, int index, int version,bool fullopen=true);
    virtual bool LoadSpecificChunk(DeSerializer* pFile, int version);

    virtual void SaveFileChunk(Serializer * pFile);
    virtual void SaveSpecificChunk(Serializer* pFile);
    virtual void SaveDllName(Serializer* pFile);

    virtual void Init();
    virtual void PreWork(int numSamples);
    virtual void WorkNoMix(int numSamples);
    virtual void Work(int numSamples);
    virtual void Tick() {};
    virtual void Tick(int track, PatternEntry * pData) {};
    //virtual void Tick(int track, PatternEntry * pData) {};
    virtual void Stop() {};
    virtual void SetPan(int newpan);

    virtual char * GetName() = 0;
    virtual int GetNumParams() { return _numPars; };
    virtual int GetNumCols() { return _nCols; };
    virtual void GetParamName(int numparam, char * name) { name[0]='\0'; };
    virtual void GetParamRange(int numparam, int &minval, int &maxval) {minval=0; maxval=0; };
    virtual void GetParamValue(int numparam, char * parval) { parval[0]='\0'; };
    virtual int GetParamValue(int numparam) { return 0; };
    virtual void SetSampleRate(int sr) {};
    virtual bool SetParameter(int numparam, int value) { return false;};
    virtual bool Load(DeSerializer * pFile);

    virtual void GetWireVolume(int wireIndex, float &value) { value = _inputConVol[wireIndex] * _wireMultiplier[wireIndex]; };
    virtual void SetWireVolume(int wireIndex,float value) { _inputConVol[wireIndex] = value / _wireMultiplier[wireIndex]; };
    virtual bool GetDestWireVolume(int srcIndex, int WireIndex,float &value);
    virtual bool SetDestWireVolume(int srcIndex, int WireIndex,float value);
    virtual void InitWireVolume(MachineType mType,int wireIndex,float value);
    virtual int FindInputWire(int macIndex);
    virtual int FindOutputWire(int macIndex);

    int _macIndex;
    MachineType _type;
    MachineMode _mode;
    bool _bypass;
    bool _mute;
    bool _waitingForSound;
    bool _stopped;
    bool _worked;
    /// left data
    float *_pSamplesL;
    /// right data
    float *_pSamplesR;
    /// left chan volume
    float _lVol;
    /// right chan volume
    float _rVol;
    int _outDry;
    /// numerical value of panning.
    int _panning;
    int _x;
    int _y;
    char _editName[32];
    int _numPars;
    int _nCols;
    /// Incoming connections Machine number
    int _inputMachines[MAX_CONNECTIONS];	
    /// Outgoing connections Machine number
    int _outputMachines[MAX_CONNECTIONS];	
    /// Incoming connections Machine vol
    float _inputConVol[MAX_CONNECTIONS];	
    /// Value to multiply _inputConVol[] to have a 0.0...1.0 range
    float _wireMultiplier[MAX_CONNECTIONS];
    /// Outgoing connections activated
    bool _connection[MAX_CONNECTIONS];
    /// Incoming connections activated
    bool _inputCon[MAX_CONNECTIONS];
    /// number of Incoming connections
    int _numInputs;
    /// number of Outgoing connections
    int _numOutputs;
    PatternEntry TriggerDelay[MAX_TRACKS];
    int TriggerDelayCounter[MAX_TRACKS];
    int RetriggerRate[MAX_TRACKS];
    int ArpeggioCount[MAX_TRACKS];
    bool TWSActive;
    int TWSInst[MAX_TWS];
    int TWSSamples;
    float TWSDelta[MAX_TWS];
    float TWSCurrent[MAX_TWS];
    float TWSDestination[MAX_TWS];
    /// output peak level for DSP
    float _volumeCounter;
    /// output peak level for display
    int _volumeDisplay;	
    /// output peak level for display
    int _volumeMaxDisplay;
    /// output peak level for display
    int _volumeMaxCounterLife;
    unsigned long int _cpuCost;
    unsigned long int _wireCost;
    int _scopePrevNumSamples;
    int _scopeBufferIndex;
    float *_pScopeBufferL;
    float *_pScopeBufferR;
    /// The topleft point of a square where the wire triangle is centered when drawn. (Used to detect when to open the wire dialog)
    CPoint _connectionPoint[MAX_CONNECTIONS];

    protected:
       void SetVolumeCounter(int numSamples);
};

class DuplicatorMac : public Machine
{
  public:
    DuplicatorMac();
    DuplicatorMac(int index);
    virtual void Init(void);
    virtual void Tick( int channel,PatternEntry* pData);
    virtual void Work(int numSamples);
    virtual char* GetName(void) { return _psName; };
    virtual void GetParamName(int numparam,char *name);
    virtual void GetParamRange(int NUMPARSE,int &minval,int &maxval);
    virtual void GetParamValue(int numparam,char *parVal);
    virtual int GetParamValue(int numparam);
    virtual bool SetParameter(int numparam,int value);
    virtual bool LoadSpecificChunk(DeSerializer* pFile, int version);
    virtual void SaveSpecificChunk(Serializer* pFile);

    protected:
      short macOutput[8];
      short noteOffset[8];
      static char* _psName;
      bool bisTicking;
};


/// dummy machine.
class Dummy : public Machine
{
  public:
    Dummy(int index);
    virtual void Work(int numSamples);
    virtual char* GetName(void) { return _psName; };
    virtual bool LoadSpecificChunk(DeSerializer* pFile, int version);
    /// Marks that the Dummy was in fact a VST plugin that couldn't be loaded
    bool wasVST;
    protected:
       static char * _psName;
};

class Master : public Machine
{
  public:
    Master();
    Master(int index);
    virtual void Init(void);
    virtual void Work(int numSamples);
    virtual char* GetName(void) { return _psName; };

    //virtual bool Load(RiffFile * pFile);
    virtual bool LoadSpecificChunk(DeSerializer * pFile, int version);
    virtual void Master::SaveSpecificChunk(Serializer* pFile);

    /// this is for the VstHost
    double sampleCount;
    bool _clip;
    bool decreaseOnClip;
    static float* _pMasterSamples;
    int peaktime;
    float currentpeak;
    float _lMax;
    float _rMax;
    bool vuupdated;
    protected:
       static char* _psName;
    };

class Mixer : public Machine
{
   public:
   enum
   {
     mix=0,
     send0,
     sendmax=send0+MAX_CONNECTIONS
   };
   Mixer();
   Mixer(int index);
   virtual void Init(void);
   virtual void Work(int numSamples);
   void FxSend(int numSamples);
   void Mix(int numSamples);
   virtual char* GetName(void) { return _psName; };
   virtual int GetNumCols();
   virtual void GetParamName(int numparam,char *name);
   virtual void GetParamRange(int numparam, int &minval, int &maxval) { minval=0; maxval=100; };
   virtual void GetParamValue(int numparam,char *parVal);
   virtual int GetParamValue(int numparam);
   virtual bool SetParameter(int numparam,int value);
   virtual bool LoadSpecificChunk(DeSerializer* pFile, int version);
   virtual void SaveSpecificChunk(Serializer* pFile);

   protected:
     float _sendGrid[MAX_CONNECTIONS][MAX_CONNECTIONS+1]; // 12 inputs with 12 sends (+dry) each.  (0 -> dry, 1+ -> sends)
     /// Incoming send, Machine number
     int _send[MAX_CONNECTIONS];	
     /// Incoming send, connection volume
     float _sendVol[MAX_CONNECTIONS];	
     /// Value to multiply _sendVol[] to have a 0.0..1.0 range
     float _sendVolMulti[MAX_CONNECTIONS];
     /// Incoming connections activated
     bool _sendValid[MAX_CONNECTIONS];		

     static char* _psName;
};

inline void Machine::SetVolumeCounter(int numSamples)
{
  _volumeCounter = dsp::GetMaxVol(_pSamplesL, _pSamplesR, numSamples);
  if(_volumeCounter > 32768.0f) _volumeCounter = 32768.0f;
  int temp((f2i(fast_log2(_volumeCounter) * 78.0f * 4 / 14.0f) - (78 * 3)));// not 100% accurate, but looks as it sounds
  // prevent downward jerkiness
  if(temp > 97) temp = 97;
  if(temp > _volumeDisplay) _volumeDisplay = temp;
  --_volumeDisplay;
};


#endif

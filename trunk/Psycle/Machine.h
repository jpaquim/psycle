#ifndef _MACHINE_H
#define _MACHINE_H


#if defined(_WINAMP_PLUGIN_)
	#include <stdio.h>
#endif // defined(_WINAMP_PLUGIN_)

#include "SongStructs.h"
#include "Dsp.h"
#include "Helpers.h"
#include "Constants.h"
#include "FileIO.h"

#if !defined(_CYRIX_PROCESSOR_) && !defined(_WINAMP_PLUGIN_)
#define CPUCOST_INIT(cost)	\
	ULONG cost;				\
	__asm rdtsc				\
	__asm mov cost, eax
/*
#define CPUCOST_CALC(cost, numSamples)	\
	__asm {								\
	__asm rdtsc							\
	__asm sub eax, cost					\
	__asm mov cost, eax					\
	}									\
	cost = cost* Global::pConfig->_pOutputDriver->_samplesPerSec/ numSamples
//	cost = (cost*1000)/(numSamples*(Global::_cpuHz/Global::pConfig->_pOutputDriver->_samplesPerSec));
*/
#define CPUCOST_CALC(cost, numSamples)	\
	__asm rdtsc							\
	__asm sub eax, cost					\
	__asm mov cost, eax

#else

#define CPUCOST_INIT(cost)	\
	ULONG cost;				\
	cost = 0;

#define CPUCOST_CALC(cost, numSamples)	cost = 0;


#endif


class CIntMachParam			// Internal Machines' Parameters' Class.
{
public:
	char const *name;		// Short name: "Cutoff"
	int minValue;			// 0
	int maxValue;			// 127
};

class RiffFile;

class Machine
{
public:
	bool wasVST;  // THIS NEVER EVER GETS RESET TO FALSE, SO WHAT DOES IT DO?  IT IS ALWAYS TRUE
	int _macIndex;

	MachineType _type;
	MachineMode _mode;
	bool _bypass;
	bool _mute;
	bool _waitingForSound;

	bool _stopped;
	bool _worked;

	float *_pSamplesL;						// left data
	float *_pSamplesR;						// right data
	float _lVol;							// left chan volume
	float _rVol;							// right chan volume
	int _panning;							// numerical value of panning.
#if !defined(_WINAMP_PLUGIN_)
	float _volumeCounter;					// output peak level for DSP
	int _volumeDisplay;						// output peak level for display
	int _volumeMaxDisplay;					// output peak level for display
	int _volumeMaxCounterLife;				// output peak level for display
	unsigned long int _cpuCost;
	unsigned long int _wireCost;
	int	_scopeBufferIndex;
	float *_pScopeBufferL;
	float *_pScopeBufferR;
#endif // ndef _WINAMP_PLUGIN_

	int _x;
	int _y;
	char _editName[32];
	int _numPars;
	int _inputMachines[MAX_CONNECTIONS];	// Incoming connections Machine number
	int _outputMachines[MAX_CONNECTIONS];	// Outgoing connections Machine number
	float _inputConVol[MAX_CONNECTIONS];	// Incoming connections Machine vol
	float _wireMultiplier[MAX_CONNECTIONS];	// Value to multiply _inputConVol[] to have a 0.0...1.0 range
	bool _connection[MAX_CONNECTIONS];      // Outgoing connections activated
	bool _inputCon[MAX_CONNECTIONS];		// Incoming connections activated
	int _numInputs;							// number of Incoming connections
	int _numOutputs;						// number of Outgoing connections
#if !defined(_WINAMP_PLUGIN_)
	CPoint _connectionPoint[MAX_CONNECTIONS];// point for wire? 
#endif
	PatternEntry	TriggerDelay[MAX_TRACKS];
	int				TriggerDelayCounter[MAX_TRACKS];
	int				RetriggerRate[MAX_TRACKS];

	bool TWSActive;
	int TWSInst[MAX_TWS];
	int TWSSamples;
	float TWSDelta[MAX_TWS];
	float TWSCurrent[MAX_TWS];
	float TWSDestination[MAX_TWS];
	
	Machine();
	virtual ~Machine();

	virtual void Init(void);
	virtual void SetPan(int newpan);
	virtual bool SetDestWireVolume(int srcIndex, int WireIndex,float value);
	virtual void SetWireVolume(int wireIndex,float value)
	{
		_inputConVol[wireIndex] = value / _wireMultiplier[wireIndex];
	}
	virtual bool GetDestWireVolume(int srcIndex, int WireIndex,float &value);
	virtual void GetWireVolume(int wireIndex, float &value)
	{
		value = _inputConVol[wireIndex] * _wireMultiplier[wireIndex];
	}
	virtual void InitWireVolume(MachineType mType,int wireIndex,float value);
	virtual int FindInputWire(int macIndex);
	virtual int FindOutputWire(int macIndex);
	virtual void Tick(void) {};
	virtual void Tick(int track, PatternEntry* pData) {};
	virtual void PreWork(int numSamples);
	virtual void Work(int numSamples);
	virtual void Stop(void) {};
	virtual char* GetName(void) = 0;
	virtual int GetNumParams(void) { return _numPars; }
	virtual void GetParamName(int numparam,char* name) { name[0]='\0'; }
	virtual void GetParamValue(int numparam,char* parval) { parval[0]='\0'; };
	virtual int GetParamValue(int numparam) { return 0; };
	virtual bool SetParameter(int numparam,int value) { return false;}; 
	virtual bool Load(RiffFile* pFile);
	static Machine* LoadFileChunk(RiffFile* pFile, int index, int version);
	virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version) // versions can only be older than current or this won't get called 
	{
		UINT size;
		pFile->Read(&size,sizeof(size)); // size of this part params to load

		UINT count;

		pFile->Read(&count,sizeof(count)); // num params to load
		for (UINT i = 0; i < count; i++)
		{
			int temp;
			pFile->Read(&temp,sizeof(temp));
			SetParameter(i,temp);
		}

		pFile->Skip(size-sizeof(count)-(count*sizeof(int)));

		return TRUE;
	};
#if !defined(_WINAMP_PLUGIN_)
	void SaveFileChunk(RiffFile* pFile);
	virtual void SaveSpecificChunk(RiffFile* pFile) 
	{
		UINT count = GetNumParams();
		UINT size = sizeof(count)+(count*sizeof(int));
		pFile->Write(&size,sizeof(size));
		pFile->Write(&count,sizeof(count));
		for (UINT i = 0; i < count; i++)
		{
			int temp = GetParamValue(i);
			pFile->Write(&temp,sizeof(temp));
		}
	};
	virtual void SaveDllName(RiffFile* pFile) 
	{
		char temp=0;
		pFile->Write(&temp,1);
	};

protected:
	inline void SetVolumeCounter(int numSamples)
	{
		_volumeCounter = Dsp::GetMaxVol(_pSamplesL, _pSamplesR, numSamples);
		if (_volumeCounter > 32768.0f)
		{
			_volumeCounter = 32768.0f;
		}
		int temp = (f2i(fast_log2(_volumeCounter)*78.0f*4/14.0f) - (78*3));// not 100% accurate, but looks as it sounds
		// prevent downward jerkiness
		if (temp > 97)
		{
			temp = 97;
		}
		if (temp > _volumeDisplay)
		{
			_volumeDisplay = temp;
		}
		_volumeDisplay--;
	};
	/*
	inline void SetVolumeCounterAccurate(int numSamples)
	{
		_volumeCounter = Dsp::GetMaxVolAccurate(_pSamplesL, _pSamplesR, numSamples);
	};
	*/
#endif // ndef _WINAMP_PLUGIN_
};

class Dummy : public Machine
{
public:
	void Work(int numSamples);
	Dummy(int index);
	virtual char* GetName(void) { return _psName; };
	virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version) // versions can only be older than current or this won't get called 
	{
		UINT size;
		pFile->Read(&size,sizeof(size)); // size of this part params to load
		pFile->Skip(size);

		return TRUE;
	};

protected:
	static char* _psName;
};


class Master : public Machine
{
public:
	Master(int index);
	double sampleCount; // this is for the VstHost
	int _outDry;
	bool _clip;
	bool decreaseOnClip;
	static float* _pMasterSamples;
	virtual bool Load(RiffFile* pFile);
	virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version)
	{
		UINT size;
		pFile->Read(&size,sizeof(size)); // size of this part params to load

		pFile->Read(&_outDry,sizeof(_outDry));
		pFile->Read(&decreaseOnClip, sizeof(decreaseOnClip)); // numSubtracks

		return TRUE;
	};

#if !defined(_WINAMP_PLUGIN_)
//	int _LMAX;
//	int _RMAX;
	int peaktime;
	float currentpeak;
	float _lMax;
	float _rMax;
	bool vuupdated;

	virtual void SaveSpecificChunk(RiffFile* pFile) 
	{
		UINT size = sizeof(_outDry)+sizeof(decreaseOnClip);
		pFile->Write(&size,sizeof(size)); // size of this part params to load

		pFile->Write(&_outDry,sizeof(_outDry));
		pFile->Write(&decreaseOnClip, sizeof(decreaseOnClip)); 
	};

#endif // ndef _WINAMP_PLUGIN_

	Master();

	virtual void Init(void);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };

protected:
	static char* _psName;

};

#ifndef CONVERT_INTERNAL_MACHINES

class Gainer : public Machine
{
public:
	Gainer(int index);

	int _outWet;
	virtual void Tick(int channel, PatternEntry *pData);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual void GetParamName(int numparam,char* name)
	{
		if ( numparam < _numPars ) strcpy(name,pars[numparam].name);
		else strcpy(name,"Out of Range");
	}
	virtual void GetParamValue(int numparam,char* parval)
	{
		switch(numparam)
		{
			case 0: strcpy(parval,"None"); break;
			case 1: sprintf(parval,"%.2f%%",CValueMapper::Map_255_100(_outWet)); break;
			default: strcpy(parval,"Out Of Range"); break;
		}
	}
	virtual int GetParamValue(int numparam)
	{
		switch(numparam)
		{
			case 0: return -1;
			case 1: return _outWet;
			default:return -1;
		}
	}
	virtual bool SetParameter(int numparam,int value)
	{
		switch(numparam)
		{
			case 0: return false;
			case 1: if ( value >=0 && value <=1024) { _outWet = value; return true; }
					else return false;
			default:return false;
		}
	}

protected:
	static char* _psName;
	static CIntMachParam pars[];
};

class CGearPsychOsc;

class Sine : public Machine
{
public:
	unsigned char _sineSpeed;
	unsigned char _sineGlide;
	unsigned char _sineVolume; // Archaic Variable. (not used right now)
	unsigned char _sineLfoSpeed;
	unsigned char _sineLfoAmp;

	Sine(int index);

	virtual void Init(void);
	virtual void Tick(int channel, PatternEntry *pData);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version)
	{
		UINT size;
		pFile->Read(&size,sizeof(size)); // size of this part params to load

		UINT count;
		pFile->Read(&count,sizeof(count)); // num params to load
		for (UINT i = 0; i < count; i++)
		{
			int temp;
			pFile->Read(&temp,sizeof(temp));
			SetParameter(i,temp);
		}

		Update();
		return true;
	}
	virtual bool Load(RiffFile* pFile);
	virtual void GetParamName(int numparam,char* name)
	{
		if ( numparam < _numPars ) strcpy(name,pars[numparam].name);
		else strcpy(name,"Out of Range");
	}
	virtual void GetParamValue(int numparam,char* parval);
	virtual int GetParamValue(int numparam)
	{
		switch(numparam)
		{
			case 0: return -1;
			case 1: return _sineSpeed;
			case 2: return _sineGlide;
			case 3: return _sineLfoSpeed;
			case 4: return _sineLfoAmp;
			default:return -1;
		}
	}
	virtual bool SetParameter(int numparam,int value)
	{
		switch(numparam)
		{
			case 0: return false;
			case 1: if (value >=0 && value <=129 ) { _sineSpeed=value; Update(); return true; }
					else return false;
			case 2: if (value >=0 && value <=129 ) { _sineGlide=value; Update(); return true; }
					else return false;
			case 3: if (value >=0 && value <=129 ) { _sineLfoSpeed=value; Update(); return true; }
					else return false;
			case 4: if (value >=0 && value <=129 ) { _sineLfoAmp=value; Update(); return true; }
					else return false;
			default:return false;
		}
	}

	void Update(void);

protected:
	friend CGearPsychOsc;

	static char* _psName;
	static CIntMachParam pars[];

	float _speed;
	float _volume;
	float _osc;
	float _realSpeed;
	float _glide;
	float _lfoSpeed;
	float _lfoAmp;
	float _lfoOsc;
};

class Distortion : public Machine
{
public:
	int _posThreshold;
	int _posClamp;
	int _negThreshold;
	int _negClamp;

	Distortion(int index);

	virtual void Init(void);
	virtual void Tick(int channel, PatternEntry *pData);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual bool Load(RiffFile* pFile);
	virtual void GetParamName(int numparam,char* name)
	{
		if ( numparam < _numPars ) strcpy(name,pars[numparam].name);
		else strcpy(name,"Out of Range");
	}
	virtual void GetParamValue(int numparam,char* parval)
	{
		switch(numparam)
		{
			case 0: strcpy(parval,"None"); break;
			case 1:
				sprintf(parval, "%d ", _posThreshold);
				break;
			case 2:
				sprintf(parval,"%d",_posClamp);
				break;
			case 3:
				sprintf(parval, "%d", _negThreshold);
				break;
			case 4:
				sprintf(parval,"%d",_negClamp);
				break;
			default: strcpy(parval,"Out Of Range"); break;
		}
	}
	virtual int GetParamValue(int numparam)
	{
		switch(numparam)
		{
			case 0: return -1;
			case 1: return _posThreshold;
			case 2: return _posClamp;
			case 3: return _negThreshold;
			case 4: return _negClamp;
			default:return -1;
		}
	}
	virtual bool SetParameter(int numparam,int value)
	{
		switch(numparam)
		{
			case 0: return false;
			case 1: if (value >=0 && value <=128 ) { _posThreshold=value; return true; }
					else return false;
			case 2: if (value >=0 && value <=128 ) { _posClamp=value; return true; }
					else return false;
			case 3: if (value >=0 && value <=128 ) { _negThreshold=value; return true; }
					else return false;
			case 4: if (value >=0 && value <=128 ) { _negClamp=value; return true; }
					else return false;
			default:return false;
		}
	}

protected:
	static char* _psName;
	static CIntMachParam pars[];
};

class Delay : public Machine
{
public:
	int _timeL;
	int _timeR;
	int _counterL;
	int _counterR;
	int _delayedCounterL;
	int _delayedCounterR;
	int _feedbackL;
	int _feedbackR;
	int _outDry;
	int _outWet;

	Delay(int index);
	virtual ~Delay();

	virtual void Init(void);
	virtual void Tick(int channel,PatternEntry* pData);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version)
	{
		UINT size;
		pFile->Read(&size,sizeof(size)); // size of this part params to load

		UINT count;
		pFile->Read(&count,sizeof(count)); // num params to load
		for (UINT i = 0; i < count; i++)
		{
			int temp;
			pFile->Read(&temp,sizeof(temp));
			SetParameter(i,temp);
		}

		Update(_timeL,_timeR,_feedbackL,_feedbackR);
		return true;
	}
	virtual bool Load(RiffFile* pFile);
	virtual void GetParamName(int numparam,char* name)
	{
		if ( numparam < _numPars ) strcpy(name,pars[numparam].name);
		else strcpy(name,"Out of Range");
	}
	virtual void GetParamValue(int numparam,char* parval);
	virtual int GetParamValue(int numparam)
	{
		switch(numparam)
		{
			case 0: return -1;
			case 1: return _timeL;
			case 2: return _feedbackL;
			case 3: return _timeR;
			case 4: return _feedbackR;
			case 5: return _outDry;
			case 6: return _outWet;
			default:return -1;
		}
	}
	virtual bool SetParameter(int numparam,int value)
	{
		switch(numparam)
		{
			case 0: return false;
			case 1: if (value >=1 && value <=MAX_DELAY_BUFFER-1 )
					{	Update(value,_timeR,_feedbackL,_feedbackR); return true; }
					else return false;
			case 2: if (value >=-100 && value < 100 )
					{	Update(_timeL,_timeR,value,_feedbackR); return true; }
					else return false;
			case 3: if (value >=1 && value <=MAX_DELAY_BUFFER-1 )
					{	Update(_timeL,value,_feedbackL,_feedbackR); return true; }
					else return false;
			case 4: if (value >=-100 && value <=100 )
					{	Update(_timeL,_timeR,_feedbackL,value); return true; }
					else return false;
			case 5: if (value >=-256 && value <=256 ) { _outDry = value; return true; }
					else return false;
			case 6: if (value >=-256 && value <=256 ) { _outWet = value; return true; }
					else return false;
			default:return false;
		}
	}

	void Update(int delayL, int delayR, int feedbackL, int feedbackR);

protected:
	static char* _psName;
	static CIntMachParam pars[];

	float* _pBufferL;
	float* _pBufferR;
};

class Flanger : public Machine
{
public:
	int _time;
	int _feedbackL;
	int _feedbackR;
	int _lfoSpeed;
	int _lfoAmp;
	int _lfoPhase;
	int _outDry;
	int _outWet;
	bool useResample;

	Flanger(int index);
	~Flanger();

	virtual void Init(void);
	virtual void Tick(int channel, PatternEntry *pData);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version)
	{
		UINT size;
		pFile->Read(&size,sizeof(size)); // size of this part params to load

		UINT count;
		pFile->Read(&count,sizeof(count)); // num params to load
		for (UINT i = 0; i < count; i++)
		{
			int temp;
			pFile->Read(&temp,sizeof(temp));
			SetParameter(i,temp);
		}

		Update();
		return true;
	}
	virtual bool Load(RiffFile* pFile);
	virtual void GetParamName(int numparam,char* name)
	{
		if ( numparam < _numPars ) strcpy(name,pars[numparam].name);
		else strcpy(name,"Out of Range");
	}
	virtual void GetParamValue(int numparam,char* parval);
	virtual int GetParamValue(int numparam)
	{
		switch(numparam)
		{
			case 0: return useResample;
			case 1: return _time;
			case 2: return _lfoAmp;
			case 3: return _lfoSpeed;
			case 4: return _feedbackL;
			case 5: return _lfoPhase;
			case 6: return _feedbackR;
			case 7: return _outDry;
			case 8: return _outWet;
			default:return -1;
		}
	}
	virtual bool SetParameter(int numparam,int value)
	{
		switch(numparam)
		{
			case 0: if (value) {useResample= TRUE;}
				else {useResample = FALSE;}
				return true;
			case 1: if (value >=1 && value <=1024 ) { _time= value; return true; }
				    else return false;
			case 2: if (value >=1 && value <= 256 )	{	_lfoAmp= value; Update(); return true; }
				    else return false;
			case 3: if (value >=0 && value <=32768 ) { _lfoSpeed=value; Update(); return true; }
					else return false;
			case 4: if (value >=-100 && value <=100 ) { _feedbackL=value; return true; }
					else return false;
			case 5: if (value >=0 && value <=256 ) { _lfoPhase=value; Update(); return true; }
					else return false;
			case 6: if (value >=-100 && value <=100 ) { _feedbackR=value; return true; }
					else return false;
			case 7: if (value >=-256 && value <=256 ) { _outDry=value; return true; }
					else return false;
			case 8: if (value >=-256 && value <=256 ) { _outWet=value; return true; }
					else return false;
			default:return false;
		}
	}

	void Update(void);
	void SetPreset(int preset);

protected:
	static char* _psName;
	static CIntMachParam pars[];
	float* _pBufferL;
	float* _pBufferR;

	int _counter;

	double _dLfoSpeed;
	double _dLfoPos;
	float _fLfoAmp;
	float _fLfoPhase;
};

class Filter2p : public Machine
{
public:
	int _cutoff;
	int _resonance;
	int _lfoSpeed;
	int _lfoAmp;
	int _lfoPhase;
	int _filterMode;

	Filter2p(int index);

	virtual void Init(void);
	virtual void Tick(int channel, PatternEntry *pData);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version)
	{
		UINT size;
		pFile->Read(&size,sizeof(size)); // size of this part params to load

		UINT count;
		pFile->Read(&count,sizeof(count)); // num params to load

		for (UINT i = 0; i < count; i++)
		{
			int temp;
			pFile->Read(&temp,sizeof(temp));
			SetParameter(i,temp);
		}

		Update();
		return true;
	}
	virtual bool Load(RiffFile* pFile);
	virtual void GetParamName(int numparam,char* name)
	{
		if ( numparam < _numPars ) strcpy(name,pars[numparam].name);
		else strcpy(name,"Out of Range");
	}
	virtual void GetParamValue(int numparam,char* parval);
	virtual int GetParamValue(int numparam)
	{
		switch(numparam)
		{
			case 0: return -1;
			case 1: return _filterMode;
			case 2: return _cutoff;
			case 3: return _resonance;
			case 4: return _lfoSpeed;
			case 5: return _lfoAmp;
			case 6: return _lfoPhase;
			default:return -1;
		}
	}
	virtual bool SetParameter(int numparam,int value)
	{
		switch(numparam)
		{
			case 0: return false;
			case 1: if ( value ==0 || value == 1) { _filterMode = value; return true; }
				    else return false;
			case 2: if ( value >=0 && value <=256) { _cutoff = value; return true; }
				    else return false;
			case 3: if ( value >=0 && value <=256) { _resonance = value; return true; }
				    else return false;
			case 4: if ( value >=0 && value <=32768) { _lfoSpeed = value; return true; }
					else return false;
			case 5: if ( value >=0 && value <=256) { _lfoAmp = value; return true; }
				    else return false;
			case 6: if ( value >=0 && value <=256) { _lfoPhase = value; return true; }
				    else return false;
			default:return false;
		}
	}

	void Update(void);

protected:
	static char* _psName;
	static CIntMachParam pars[];
	float _bufferL0;
	float _bufferL1;
	float _bufferR0;
	float _bufferR1;
	float _fCutoff;
	float _fResonance;
	float _fLfoSpeed;
	float _fLfoAmp;
	float _fLfoPhase;
	float _fLfoDegree;

	float WorkL(float input, float f, float q, float fa, float fb);
	float WorkR(float input, float f, float q, float fa, float fb);
};

#endif

#endif

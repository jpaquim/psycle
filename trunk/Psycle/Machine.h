#ifndef _MACHINE_H
#define _MACHINE_H

#include "SongStructs.h"
#include "Dsp.h"
#include "Helpers.h"

#define MAX_DELAY_BUFFER		65536 // Dalay Delay and Flanger
#define OVERLAPTIME				128  // Sampler
#define STREAM_SIZE				256	// If changed, change "MAX_BUFFER_LENGTH" in machineinterface.h, if needed.

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


typedef enum
{
	MACH_MASTER = 0,
	MACH_SINE = 1,
	MACH_DIST = 2,
	MACH_SAMPLER = 3,
	MACH_DELAY = 4,
	MACH_2PFILTER = 5,
	MACH_GAIN = 6,
	MACH_FLANGER = 7,
	MACH_PLUGIN = 8,
	MACH_VST = 9,
	MACH_VSTFX = 10,
	MACH_DUMMY = 255
}
MachineType;

typedef enum
{
	MACHMODE_GENERATOR = 0,
	MACHMODE_FX = 1,
	MACHMODE_MASTER = 2,
	MACHMODE_PLUGIN = 3,
}
MachineMode;

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
	bool wasVST;

	MachineType _type;
	MachineMode _mode;
	bool _bypass;
	bool _mute;
	bool _waitingForSound;

	bool _stopped;
	bool _worked;

	int _outDry;
	int _outWet;
	float *_pSamplesL;						// left data
	float *_pSamplesR;						// right data
	float _lVol;							// left chan volume
	float _rVol;							// right chan volume
	int _panning;							// numerical value of panning.
	float _volumeMultiplier;				// Trick to avoid some extra multiplications.
#if !defined(_WINAMP_PLUGIN_)
	int _volumeCounter;						// output peak level.
	int _volumeDisplay;						// output peak level.
	int _volumeMaxDisplay;					// output peak level.
	int _volumeMaxCounterLife;				// output peak level.
	unsigned long int _cpuCost;
	unsigned long int _wireCost;
#endif // ndef _WINAMP_PLUGIN_

	int _x;
	int _y;
	char _editName[16];
	int _numPars;
	int _inputMachines[MAX_CONNECTIONS];	// Incoming connections Machine number
	int _outputMachines[MAX_CONNECTIONS];	// Outgoing connections Machine number
	float _inputConVol[MAX_CONNECTIONS];	// Incoming connections Machine vol
	bool _connection[MAX_CONNECTIONS];      // Outgoing connections activated
	bool _inputCon[MAX_CONNECTIONS];		// Incoming connections activated
	int _numInputs;							// number of Incoming connections
	int _numOutputs;						// number of Outgoing connections
	CPoint _connectionPoint[MAX_CONNECTIONS];

	Machine();
	virtual ~Machine();

	virtual void Init(void);
	virtual void SetPan(int newpan);
	virtual void SetWireVolume(int srcIndex, int WireIndex,int value);
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
#if !defined(_WINAMP_PLUGIN_)
	virtual bool Save(RiffFile* pFile);
protected:
	inline void SetVolumeCounter(int numSamples)
	{
		int newVolume = Dsp::GetMaxVol(_pSamplesL, _pSamplesR, numSamples);
		if (newVolume > 32768)
		{
			newVolume = 32768;
		}
		if (newVolume > _volumeCounter)
		{
			_volumeCounter = newVolume;
			int temp = (f2i(fast_log2(float(newVolume))*78.0f*4*2/14.0f) - (78*3*2));//*2;// not 100% accurate, but looks as it sounds
			// prevent downward jerkiness
			if (temp > 97*2)
			{
				temp = 97*2;
			}
			if (temp > _volumeDisplay)
			{
				_volumeDisplay = temp;
			}
		}
		_volumeCounter-=numSamples/4;
		_volumeDisplay-=2;
	};
	inline void SetVolumeCounterAccurate(int numSamples)
	{
		_volumeCounter = Dsp::GetMaxVolAccurate(_pSamplesL, _pSamplesR, numSamples);
	};
#endif // ndef _WINAMP_PLUGIN_

};

class Dummy : public Machine
{
public:
	void Work(int numSamples);
	Dummy();
	virtual char* GetName(void) { return _psName; };

protected:
	static char* _psName;
};


class Master : public Machine
{
public:
	bool _clip;
	bool decreaseOnClip;
	static float* _pMasterSamples;
#if !defined(_WINAMP_PLUGIN_)
//	int _LMAX;
//	int _RMAX;
	int peaktime;
	float currentpeak;
	float _lMax;
	float _rMax;
	bool vuupdated;
#endif // ndef _WINAMP_PLUGIN_

	Master();

	virtual void Init(void);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };

protected:
	static char* _psName;

};

class Gainer : public Machine
{
public:
	Gainer();

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
			case 1: sprintf(parval,"%.2f%%",(float)_outWet*0.390625f); break;
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

	Sine();

	virtual void Init(void);
	virtual void Tick(int channel, PatternEntry *pData);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual bool Load(RiffFile* pFile);
#if !defined(_WINAMP_PLUGIN_)
	virtual bool Save(RiffFile* pFile);
#endif // ndef _WINAMP_PLUGIN_
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
			case 1: if (value >=0 && value <=128 ) { _sineSpeed=value; Update(); return true; }
					else return false;
			case 2: if (value >=0 && value <=128 ) { _sineGlide=value; Update(); return true; }
					else return false;
			case 3: if (value >=0 && value <=128 ) { _sineLfoSpeed=value; Update(); return true; }
					else return false;
			case 4: if (value >=0 && value <=128 ) { _sineLfoAmp=value; Update(); return true; }
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

	Distortion();

	virtual void Init(void);
	virtual void Tick(int channel, PatternEntry *pData);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual bool Load(RiffFile* pFile);
#if !defined(_WINAMP_PLUGIN_)
	virtual bool Save(RiffFile* pFile);
#endif // ndef _WINAMP_PLUGIN_
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

	Delay();
	virtual ~Delay();

	virtual void Init(void);
	virtual void Tick(int channel,PatternEntry* pData);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual bool Load(RiffFile* pFile);
#if !defined(_WINAMP_PLUGIN_)
	virtual bool Save(RiffFile* pFile);
#endif // ndef _WINAMP_PLUGIN_
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
					{	Update(value,_feedbackL,_timeR,_feedbackR); return true; }
					else return false;
			case 2: if (value >=-100 && value < 100 )
					{	Update(value,_feedbackL,_timeR,_feedbackR); return true; }
					else return false;
			case 3: if (value >=1 && value <=MAX_DELAY_BUFFER-1 )
					{	Update(value,_feedbackL,_timeR,_feedbackR); return true; }
					else return false;
			case 4: if (value >=-100 && value <=100 )
					{	Update(value,_feedbackL,_timeR,_feedbackR); return true; }
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

	Flanger();
	~Flanger();

	virtual void Init(void);
	virtual void Tick(int channel, PatternEntry *pData);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual bool Load(RiffFile* pFile);
#if !defined(_WINAMP_PLUGIN_)
	virtual bool Save(RiffFile* pFile);
#endif // ndef _WINAMP_PLUGIN_
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
			case 0: return false;
			case 1: if (value >=1 && value <=1024 ) { _time= value; return true; }
				    else return false;
			case 2: if (value >=1 && value < 256 )	{	_lfoAmp= value; Update(); return true; }
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

	Filter2p();

	virtual void Init(void);
	virtual void Tick(int channel, PatternEntry *pData);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual bool Load(RiffFile* pFile);
#if !defined(_WINAMP_PLUGIN_)
	virtual bool Save(RiffFile* pFile);
#endif // ndef _WINAMP_PLUGIN_
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

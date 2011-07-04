// Psycle FX PLUGIN API
   
#ifndef __MACHINE_INTERFACE_H
#define __MACHINE_INTERFACE_H

#include <stdio.h>
#include <assert.h>

#define MI_VERSION				11		//Psycle 1.6.
#define SEQUENCER				1
#define EFFECT					0
#define GENERATOR				3
#define NOTE_MAX								119
#define NOTE_NO									120
#define NOTE_OFF								255

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

double const PI = 3.14159265358979323846;

#define MAX_BUFFER_LENGTH		256		// in number of samples (per channel)

// CMachineParameter flags
#define MPF_NULL				0
#define MPF_STATE				2	

class CMachineParameter
{
public:

	char const *Name;		// Short name: "Cutoff"
	char const *Description;// Longer description: "Cutoff Frequency (0-7f)"
	int MinValue;			// 0
	int MaxValue;			// 127
	int Flags;
	int DefValue;			// default value for params that have MPF_STATE flag set
};

class CMachineInfo
{
public:
	int Version;							// VERSION
	int Flags;								// Flags
	int numParameters;				
	CMachineParameter const **Parameters;
	char const *Name;						// "Rambo Delay"
	char const *ShortName;					// "Delay"
	char const *Author;						// "John Rambo"
	char const *Command;					// "About"
	int numCols;
};

class CFxCallback
{
public:
	virtual void MessBox(char* ptxt,char*caption,unsigned int type){}
	virtual int CallbackFunc(int cbkID,int par1,int par2,int par3){return 0;}
	virtual float *GetWaveLData(int inst,int wave){return 0;} // USELESS if you cannot get the length!
	virtual float *GetWaveRData(int inst,int wave){return 0;} // USELESS if you cannot get the length!
	virtual int GetTickLength(){return 2048;}
	virtual int GetSamplingRate(){return 44100;}
	virtual int GetBPM(){return 125;}
	virtual int GetTPB(){return 4;}

	// Don't get fooled by the above return values.
	// You get a pointer to a subclass of this one that returns the correct ones.
};

class CMachineInterface
{
public:
	virtual ~CMachineInterface() {}
	virtual void Init() {}
	virtual void SequencerTick() {}
	virtual void ParameterTweak(int par, int val) {}

	// Work function
	virtual void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks) {}
	
	virtual void Stop() {}

	// Export / Import
	virtual void PutData(byte* pData) {}
	virtual void GetData(byte* pData) {}
	virtual int GetDataSize() { return 0; }
	
	virtual void Command() {}

	virtual void MuteTrack(int const i) {}	// Not used (yet?)
	virtual bool IsTrackMuted(int const i) const { return false; } 	// Not used (yet?)

	virtual void MidiNote(int const channel, int const value, int const velocity) {} 	// Not used (yet?)
	virtual void Event(dword const data) {} 	// Not used (yet?)

	virtual bool DescribeValue(char* txt,int const param, int const value) { return false; }

	virtual bool PlayWave(int const wave, int const note, float const volume) { return false; } 	// Not used (prolly never)
	virtual void SeqTick(int channel, int note, int ins, int cmd, int val) {}
	
	virtual void StopWave() {} 	// Not used (prolly never)

public:
	// initialize these members in the constructor 
	int *Vals;
	
	// these members are initialized by the 
	// engine right after it calls CreateMachine()
	// don't touch them in the constructor

	CFxCallback *pCB;						/* Callback				*/
};


#define DLL_EXPORTS extern "C" { \
__declspec(dllexport) CMachineInfo const * __cdecl GetInfo() \
{ \
	return &MacInfo; \
} \
__declspec(dllexport) CMachineInterface * __cdecl CreateMachine() \
{ \
	return new mi; \
} \
} 
#endif

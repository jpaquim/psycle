#pragma once

#include "Machine.hpp"
namespace psycle
{
	namespace host
	{

/// dummy machine.
class Dummy : public Machine
{
public:
	Dummy(int index);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
	/// Marks that the Dummy was in fact a VST plugin that couldn't be loaded
	bool wasVST;
protected:
	static char * _psName;
};

class DuplicatorMac : public Machine
{
public:
	DuplicatorMac();
	DuplicatorMac(int index);
	virtual void Init(void);
	virtual void Tick();
	virtual void Tick( int channel,PatternEntry* pData);
	virtual void Stop();
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual void GetParamName(int numparam,char *name);
	virtual void GetParamRange(int numparam, int &minval, int &maxval);
	virtual void GetParamValue(int numparam,char *parVal);
	virtual int GetParamValue(int numparam);
	virtual bool SetParameter(int numparam,int value);
	virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
	virtual void SaveSpecificChunk(RiffFile * pFile);

protected:
	static const int NUMMACHINES=8;
	void AllocateVoice(int channel, int machine);
	void DeallocateVoice(int channel, int machine);
	short macOutput[NUMMACHINES];
	short noteOffset[NUMMACHINES];
	static char* _psName;
	bool bisTicking;
	// returns the allocated channel of the machine, for the channel (duplicator's channel) of this tick.
	int allocatedchans[MAX_TRACKS][NUMMACHINES];
	// indicates if the channel of the specified machine is in use or not
	bool availablechans[MAX_MACHINES][MAX_TRACKS];
};

//////////////////////////////////////////////////////////////////////////
/// mixer machine.
class Mixer : public Machine
{
public:
	enum
	{
		mix=0,
		send0,
		sendmax=send0+MAX_CONNECTIONS
	};
	enum 
	{
		collabels=0,
		chan1,
		chan2,
		chan3,
		chan4,
		chan5,
		chan6,
		chan7,
		chan8,
		chan9,
		chan10,
		chan11,
		chan12,
		return1,
		return2,
		return3,
		return4,
		return5,
		return6,
		return7,
		return8,
		return9,
		return10,
		return11,
		return12
	};
	Mixer();
	Mixer(int index);
	virtual ~Mixer() throw();
	virtual void Init(void);
	virtual void Tick( int channel,PatternEntry* pData);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	void FxSend(int numSamples);
	void Mix(int numSamples);
	std::string GetAudioInputName(int port);
	virtual int GetNumCols();
	virtual void GetParamName(int numparam,char *name);
	virtual void GetParamRange(int numparam, int &minval, int &maxval) { minval=0; maxval=100; };
	virtual void GetParamValue(int numparam,char *parVal);
	virtual int GetParamValue(int numparam);
	virtual bool SetParameter(int numparam,int value);
	virtual int GetAudioInputs() { return 24; };
	virtual int GetAudioOutputs() { return 1; };
	virtual int GetSend(int i){ assert(i<MAX_CONNECTIONS); return _send[i]; }
	virtual bool SendValid(int i) { assert(i<MAX_CONNECTIONS); return _send[i]; }
	virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
	virtual void SaveSpecificChunk(RiffFile * pFile);

	virtual float VuChan(int idx);
	virtual float VuSend(int idx);

protected:
	static char* _psName;

	///\todo hardcoded limits and wastes
	float _sendGrid[MAX_CONNECTIONS][MAX_CONNECTIONS+1]; // 12 inputs with 12 sends (+dry) each.  (0 -> dry, 1+ -> sends)
	/// Incoming send, Machine number
	///\todo hardcoded limits and wastes
	int _send[MAX_CONNECTIONS];	
	/// Incoming send, connection volume
	///\todo hardcoded limits and wastes
	float _sendVol[MAX_CONNECTIONS];	
	/// Value to multiply _sendVol[] to have a 0.0..1.0 range
	///\todo hardcoded limits and wastes
	float _sendVolMulti[MAX_CONNECTIONS];
	/// Incoming connections activated
	///\todo hardcoded limits and wastes
	bool _sendValid[MAX_CONNECTIONS];

#if 0 // more lightweight
	class send
	{
	private:
		float grid;
		/// Incoming send, Machine number
		Machine::id_type incoming;
		/// Incoming send, connection volume
		float volume;
		/// Value to multiply volume to have a 0.0..1.0 range
		float normalize;
	};
	std::vector<send> sends;
#endif

};

	}
}

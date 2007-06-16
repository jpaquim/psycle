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
	class InputChannel
	{
	public:
		InputChannel(){};
		InputChannel(int sends)
		{
			sends_.resize(sends);
		}
		InputChannel(const InputChannel &in)
		{
			for(unsigned int i=0; i<in.sends_.size(); ++i)
			{
				sends_.push_back(in.sends_[i]);
			}
			mix_ = in.mix_;
			mute_ = in.mute_;
			drymix_ = in.drymix_;
			wetmix_ = in.wetmix_;
		}
		inline float &Send(int i) { return sends_[i]; }
		inline const float &Send(int i) const { return sends_[i]; }
		inline float &Mix() { return mix_; }
		inline bool &Mute() { return mute_; }
		inline bool &DryMix() { return drymix_; }
		inline bool &WetMix() { return wetmix_; }

		void AddSend() { sends_.push_back(0); }
		void resize(int sends) { sends_.resize(sends); }
		void ExchangeSends(int send1,int send2)
		{
			float tmp = sends_[send1];
			sends_[send1] = sends_[send2];
			sends_[send2] = tmp;
		}

	protected:
		std::vector<float> sends_;
		float mix_;
		bool mute_;
		bool drymix_;
		bool wetmix_;
	};

	class ReturnChannel
	{
	public:
		ReturnChannel(){};
		ReturnChannel(int sends)
		{
			sends_.resize(sends);
		}
		ReturnChannel(const ReturnChannel &in)
		{
			for(unsigned int i=0; i<in.sends_.size(); ++i)
			{
				sends_.push_back(in.sends_[i]);
			}
			mute_ = in.mute_;
		}
		inline const bool Send(int i) const { return sends_[i]; }
		inline void Send(int i,bool value) { sends_[i]= value; }
		inline bool &Mute() { return mute_; }

		void AddSend() { sends_.push_back(false); }
		void resize(int sends) { sends_.resize(sends); }
		void ExchangeSends(int send1,int send2)
		{
			bool tmp = sends_[send1];
			sends_[send1] = sends_[send2];
			sends_[send2] = tmp;
		}

	protected:
		std::vector<bool> sends_;
		bool mute_;
	};


	class MixerGrid
	{
	public:
		MixerGrid(){};
		MixerGrid(int inputs, int sends)
		{
			for(int i=0; i<inputs; ++i)
			{
				inputs_.push_back(InputChannel(sends));
			}
		}
		MixerGrid(MixerGrid& copy)
		{
			for(int i=0; i<copy.numsends(); ++i)
			{
				inputs_.push_back(copy.inputs_[i]);
			}
		}

		inline InputChannel & Channel(int i) { return inputs_[i]; }
		inline const InputChannel & Channel(int i) const { return inputs_[i]; }

		int numinputs() const { }
		int numsends() const { }

		// other accessors, like at() ...

		void resize(int inputs, int sends)
		{
			inputs_.resize(inputs);
			for(int i = 0; i < inputs; ++i)
				Channel(i).resize(sends);
		}
		void ExchangeChans(int chan1,int chan2)
		{
			InputChannel tmp = inputs_[chan1];
			inputs_[chan1] = inputs_[chan2];
			inputs_[chan2] = tmp;
		}
		void ExchangeSends(int send1,int send2)
		{
			for (unsigned int i(0); i < inputs_.size(); ++i)
			{
				inputs_[i].ExchangeSends(send1,send2);
				returns_[i].ExchangeSends(send1,send2);
			}
		}
		// other member functions, like reserve()....

	private:
		std::vector<InputChannel> inputs_;
		std::vector<ReturnChannel> returns_;
		int allocatedinputs;
		int allocatedsends;
		bool *inputvalid;
		bool *sendvalid;
	};

	enum
	{
		mix=0,
		send0,
		sendmax=send0+MAX_CONNECTIONS,
		mute,
		solo,
		gain
	};
	enum 
	{
		collabels=0,
		colmastervol,
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
	virtual void GetWireVolume(int wireIndex, float &value){ value = GetWireVolume(wireIndex); }
	virtual float GetWireVolume(int wireIndex);
	virtual void SetWireVolume(int wireIndex,float value);
	virtual void InitWireVolume(MachineType mType,int wireIndex,float value);
	virtual int FindInputWire(int macIndex);
	virtual void DeleteInputWireIndex(int wireIndex);
	virtual char* GetName(void) { return _psName; };
	void FxSend(int numSamples);
	void Mix(int numSamples);
	std::string GetAudioInputName(int port);
	virtual int GetNumCols();
	virtual int InsertFx(Machine* mac);
	virtual void GetParamName(int numparam,char *name);
	virtual void GetParamRange(int numparam, int &minval, int &maxval)
	{	minval=0;  maxval= (numparam==0)?256:100; };
	virtual void GetParamValue(int numparam,char *parVal);
	virtual int GetParamValue(int numparam);
	virtual bool SetParameter(int numparam,int value);
	virtual bool GetSoloState(int column) { return column==_solocolumn; }
	virtual bool GetMuteState(int column) { return _mutestate[column]; }
	virtual void SetSoloState(int column,bool solo) { _solocolumn= solo?column:-1; }
	virtual void SetMuteState(int column,bool mute) { _mutestate[column]=mute; }
	virtual int GetAudioInputs() { return 24; };
	virtual int GetAudioOutputs() { return 1; };
	inline int GetSend(int i){ assert(i<MAX_CONNECTIONS); return _send[i]; }
	inline bool SendValid(int i) { assert(i<MAX_CONNECTIONS); return _sendValid[i]; }
	virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
	virtual void SaveSpecificChunk(RiffFile * pFile);

	virtual float VuChan(int idx);
	virtual float VuSend(int idx);

protected:
	static char* _psName;

	bool _wetmix;
	bool _drymix;
	int _masterVolume;
	int _masterGain;

	///\todo hardcoded limits and wastes




	float _sendGrid[MAX_CONNECTIONS][MAX_CONNECTIONS+1]; // 12 inputs with 12 sends (+dry+gain) each.  (0 -> dry, 1+ -> sends) 
	int _solocolumn;
	bool _mutestate[MAX_CONNECTIONS*2]; // inputs and returns.
	/// Incoming send, Machine number
	///\todo hardcoded limits and wastes
	int _send[MAX_CONNECTIONS];	
	/// Value to multiply _sendVol[] to have a 0.0..1.0 range
	///\todo hardcoded limits and wastes
	float _returnVolMulti[MAX_CONNECTIONS];
	/// Incoming connections activated
	///\todo hardcoded limits and wastes
	bool _sendValid[MAX_CONNECTIONS];

	// Internal variables
	float _outGain;
	float _returnVol[MAX_CONNECTIONS];	


#if 0 // more lightweight
	class send
	{
	private:
		int sendmachine;
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

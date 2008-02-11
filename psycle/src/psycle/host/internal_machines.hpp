// -*- mode:c++; indent-tabs-mode:t -*-
#pragma once

#include "Machine.hpp"
namespace psycle
{
	namespace host
	{
//////////////////////////////////////////////////////////////////////////
/// dummy machine.
class Dummy : public Machine
{
public:
	Dummy(int index);
	Dummy(Machine *mac);
	virtual void Work(int numSamples);
	virtual float GetAudioRange(){ return 32768.0f; }
	virtual char* GetName(void) { return _psName; }
	virtual bool LoadSpecificChunk(RiffFile* pFile, int version);

	/// Marks that the Dummy was in fact a VST plugin that couldn't be loaded
	bool wasVST;
protected:
	static char * _psName;
};
//////////////////////////////////////////////////////////////////////////
/// Duplicator machine.
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
	virtual float GetAudioRange(){ return 32768.0f; }
	virtual char* GetName(void) { return _psName; }
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
/// Audio Recorder
class AudioRecorder : public Machine
{
public:
	AudioRecorder();
	AudioRecorder(int index);
	virtual ~AudioRecorder();
	virtual void PreWork(int numSamples,bool clear) { Machine::PreWork(numSamples,false); }
	virtual void Init(void);
	virtual void Work(int numSamples);
	virtual float GetAudioRange(){ return 32768.0f; }
	virtual char* GetName(void) { return _psName; }
	virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
	virtual void SaveSpecificChunk(RiffFile * pFile);

	virtual void ChangePort(int newport);

	static char* _psName;

	int _captureidx;
	bool _initialized;
	float _gainvol;
	float* pleftorig;
	float* prightorig;
	
};

//////////////////////////////////////////////////////////////////////////
/// mixer machine.
class Mixer : public Machine
{
public:
	class InputChannel
	{
	public:
		InputChannel(){ Init(); }
		InputChannel(int sends){ Init(); sends_.resize(sends); }
		InputChannel(const InputChannel &in) { Copy(in); }
		inline void Init()
		{
			if (sends_.size() != 0) sends_.resize(0);
			volume_=1.0f;
			panning_=0.5f;
			drymix_=1.0f;
			mute_=false;
			dryonly_=false;
			wetonly_=false;
		}
		InputChannel& operator=(const InputChannel &in) { Copy(in); return *this; }
		void Copy(const InputChannel &in)
		{
			for(unsigned int i=0; i<in.sends_.size(); ++i)
			{
				sends_.push_back(in.sends_[i]);
			}
			volume_ = in.volume_;
			panning_ = in.panning_;
			drymix_ = in.drymix_;
			mute_ = in.mute_;
			dryonly_ = in.dryonly_;
			wetonly_ = in.wetonly_;
		}
		inline float &Send(int i) { return sends_[i]; }
		inline const float &Send(int i) const { return sends_[i]; }
		inline float &Volume() { return volume_; }
		inline float &Panning() { return panning_; }
		inline float &DryMix() { return drymix_; }
		inline bool &Mute() { return mute_; }
		inline bool &DryOnly() { return dryonly_; }
		inline bool &WetOnly() { return wetonly_; }

		void AddSend() { sends_.push_back(0); }
		void ResizeTo(int sends) { sends_.resize(sends); }
		void ExchangeSends(int send1,int send2)
		{
			float tmp = sends_[send1];
			sends_[send1] = sends_[send2];
			sends_[send2] = tmp;
		}

	protected:
		std::vector<float> sends_;
		float volume_;
		float panning_;
		float drymix_;
		bool mute_;
		bool dryonly_;
		bool wetonly_;
	};

	class MixerWire
	{
	public:
		MixerWire():machine_(-1),volume_(1.0f),normalize_(1.0f) {}
		MixerWire(int mac,float norm):machine_(mac),volume_(1.0f),normalize_(norm){}
		bool IsValid(){ return (machine_!=-1); }

		int machine_;
		float volume_;
		float normalize_;
	};

	class ReturnChannel
	{
	public:
		ReturnChannel(){Init();}
		ReturnChannel(int sends) { Init(); sends_.resize(sends); }
		ReturnChannel(const ReturnChannel &in) { Copy(in); }
		void Init()
		{
			if (sends_.size()!=0) sends_.resize(0);
			mastersend_=true;
			volume_=1.0f;
			panning_=0.5f;
			mute_=false;
		}
		ReturnChannel& operator=(const ReturnChannel& in) { Copy(in); return *this; }
		void Copy(const ReturnChannel& in)
		{
			wire_ = in.wire_;
			for(unsigned int i=0; i<in.sends_.size(); ++i)
			{
				sends_.push_back(in.sends_[i]);
			}
			mastersend_ = in.mastersend_;
			volume_ = in.volume_;
			panning_ = in.panning_;
			mute_ = in.mute_;
		}
		inline void Send(int i,bool value) { sends_[i]= value; }
		inline const bool Send(int i) const { return sends_[i]; }
		inline bool &MasterSend() { return mastersend_; }
		inline float &Volume() { return volume_; }
		inline float &Panning() { return panning_; }
		inline bool &Mute() { return mute_; }
		inline MixerWire &Wire() { return wire_; }
		
		bool IsValid() { return wire_.IsValid(); }

		void AddSend() { sends_.push_back(false); }
		void ResizeTo(int sends) { sends_.resize(sends); }
		void ExchangeSends(int send1,int send2)
		{
			bool tmp = sends_[send1];
			sends_[send1] = sends_[send2];
			sends_[send2] = tmp;
		}

	protected:
		MixerWire wire_;
		std::vector<bool> sends_;
		bool mastersend_;
		float volume_;
		float panning_;
		bool mute_;
	};

	class MasterChannel
	{
	public:
		MasterChannel(){Init();}
		inline void Init()
		{
			volume_=1.0f; drywetmix_=0.5f; gain_=1.0f;
		}
		inline float &Volume() { return volume_; }
		inline float &DryWetMix() { return drywetmix_; }
		inline float &Gain() { return gain_; }

	protected:
		float volume_;
		float drywetmix_;
		float gain_;
	};

	enum 
	{
		chan1=0,
		chanmax = chan1+MAX_CONNECTIONS,
		return1 = chanmax,
		returnmax = return1+MAX_CONNECTIONS,
		send1 = returnmax,
		sendmax = send1+MAX_CONNECTIONS
	};
	Mixer();
	Mixer(int index);
	virtual ~Mixer() throw();
	virtual void Init(void);
	virtual void Tick( int channel,PatternEntry* pData);
	virtual void Work(int numSamples);
	void FxSend(int numSamples);
	void Mix(int numSamples);
	virtual void GetWireVolume(int wireIndex, float &value){ value = GetWireVolume(wireIndex); }
	virtual float GetWireVolume(int wireIndex);
	virtual void SetWireVolume(int wireIndex,float value);
	virtual void InsertInputWireIndex(Song* pSong,int wireIndex,int srcmac,float wiremultiplier, float initialvol=1.0f);
	virtual int FindInputWire(int macIndex);
	virtual void NotifyNewSendtoMixer(Song* pSong,int callerMac,int senderMac);
	virtual int GetFreeInputWire(int slottype=0);
	virtual int GetInputSlotTypes() { return 2; }
	virtual void DeleteInputWireIndex(Song* pSong,int wireIndex);
	virtual void DeleteMixerSendFlag(Song* pSong,Machine* mac);
	virtual void SetMixerSendFlag(Song* pSong,Machine* mac);
	virtual void DeleteWires(Song* pSong);
	virtual float GetAudioRange(){ return 32768.0f; }
	std::string GetAudioInputName(int port);
	virtual int GetAudioInputs() { return 24; }
	virtual int GetAudioOutputs() { return 1; }
	virtual char* GetName(void) { return _psName; }
	virtual int GetNumCols();
	virtual void GetParamName(int numparam,char *name);
	virtual void GetParamRange(int numparam, int &minval, int &maxval);
	virtual void GetParamValue(int numparam,char *parVal);
	virtual int GetParamValue(int numparam);
	virtual bool SetParameter(int numparam,int value);
	virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
	virtual void SaveSpecificChunk(RiffFile * pFile);

	bool GetSoloState(int column) { return column==solocolumn_; }
	void SetSoloState(int column,bool solo)
	{
		if (solo ==false && column== solocolumn_)
			solocolumn_=-1;
		else solocolumn_=column;
	}
	float VuChan(int idx);
	float VuSend(int idx);

	inline InputChannel & Channel(int i) { return inputs_[i]; }
	inline const InputChannel & Channel(int i) const { return inputs_[i]; }
	inline ReturnChannel & Return(int i) { return returns_[i]; }
	inline const ReturnChannel & Return(int i) const { return returns_[i]; }
	inline MixerWire & Send(int i) { return sends_[i]; }
	inline const MixerWire & Send(int i) const { return sends_[i]; }
	inline MasterChannel & Master() { return master_; }
	inline int numinputs() const { return static_cast<int>(inputs_.size()); }
	inline int numreturns() const { return static_cast<int>(returns_.size()); }
	inline int numsends() const { return static_cast<int>(sends_.size()); }
	inline bool ChannelValid(int i) { assert (i<MAX_CONNECTIONS); return (i<numinputs() && _inputCon[i]); }
	inline bool ReturnValid(int i) { assert (i<MAX_CONNECTIONS); return (i<numreturns() && Return(i).IsValid()); }
	inline bool SendValid(int i) { assert (i<MAX_CONNECTIONS); return (i<numsends() && sends_[i].IsValid()); }

	void InsertChannel(int idx,InputChannel*input=0);
	void InsertReturn(int idx,ReturnChannel* retchan=0);
	void InsertReturn(int idx,MixerWire rwire)
	{
		InsertReturn(idx);
		Return(idx).Wire()=rwire;
	}
	void InsertSend(int idx,MixerWire swire);
	void DiscardChannel(int idx);
	void DiscardReturn(int idx);
	void DiscardSend(int idx);

	void ExchangeChans(int chann1,int chann2);
	void ExchangeReturns(int chann1,int chann2);
	void ExchangeSends(int send1,int send2);
	void ResizeTo(int inputs, int sends);

	void RecalcMaster();
	void RecalcReturn(int idx);
	void RecalcChannel(int idx);
	void RecalcSend(int chan,int send);


protected:
	static char* _psName;

	int solocolumn_;
	std::vector<InputChannel> inputs_;
	std::vector<ReturnChannel> returns_;
	std::vector<MixerWire> sends_;
	MasterChannel master_;

	// Arrays of precalculated volume values for the FxSend and Mix functions
	float _sendvolpl[MAX_CONNECTIONS][MAX_CONNECTIONS];
	float _sendvolpr[MAX_CONNECTIONS][MAX_CONNECTIONS];
	float mixvolpl[MAX_CONNECTIONS];
	float mixvolpr[MAX_CONNECTIONS];
	float mixvolretpl[MAX_CONNECTIONS];
	float mixvolretpr[MAX_CONNECTIONS];

};
	/// tweaks:
	/// [0x]:
	///  0 -> Master volume
	///  1..C -> Input volumes
	///  D -> master drywetmix.
	///  E -> master gain.
	///  F -> master pan.
	/// [1x..Cx]:
	///  0 -> input wet mix.
	///  1..C -> input send amout to the send x.
	///  D -> input drywetmix. ( 0 normal, 1 dryonly, 2 wetonly  3 mute)
	///  E -> input gain.
	///  F -> input panning.
	/// [Dx]:
	///  0 -> Solo channel.
	///  1..C -> return grid. // the return grid array grid represents: bit0 -> mute, bit 1..12 routing to send. bit 13 -> route to master
	/// [Ex]: 
	///  1..C -> return volumes
	/// [Fx]:
	///  1..C -> return panning
	}
}

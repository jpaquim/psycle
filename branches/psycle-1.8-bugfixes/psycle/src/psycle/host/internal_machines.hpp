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


class AudioRecorder : public Machine
{
public:
	AudioRecorder();
	AudioRecorder(int index);
	virtual ~AudioRecorder();
	virtual void Init(void);
	virtual void Work(int numSamples);
	virtual char* GetName(void) { return _psName; };
	virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
	virtual void SaveSpecificChunk(RiffFile * pFile);

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
	///\todo: variable initialization of all the classes.
	class InputChannel
	{
	public:
		InputChannel(){ Init(); };
		InputChannel(int sends)
		{
			Init();
			sends_.resize(sends);
		}
		InputChannel(const InputChannel &in)
		{
			Copy(in);
		}
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
		InputChannel& operator=(const InputChannel &in)
		{
			Copy(in);
			return *this;
		}
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
		///\todo: panning?
		std::vector<float> sends_;
		float volume_;
		float panning_;
		float drymix_;
		bool mute_;
		bool dryonly_;
		bool wetonly_;
	};

	class ReturnWire
	{
	public:
		ReturnWire():machine_(-1),volume_(1.0f),normalize_(1.0f) {}
		ReturnWire(int mac,float norm):machine_(mac),volume_(1.0f),normalize_(norm){}
		bool IsValid(){ return (machine_!=-1); }

		int machine_;
		float volume_;
		float normalize_;
	};

	class ReturnChannel
	{
	public:
		ReturnChannel(){Init();}
		ReturnChannel(int sends)
		{
			Init();
			sends_.resize(sends);
		}
		ReturnChannel(const ReturnChannel &in)
		{
			Copy(in);
		}
		void Init()
		{
			if (sends_.size()!=0) sends_.resize(0);
			volume_=1.0f;
			panning_=0.5f;
			mute_=false;
		}
		ReturnChannel& operator=(const ReturnChannel& in)
		{
			Copy(in);
			return *this;
		}
		void Copy(const ReturnChannel& in)
		{
			for(unsigned int i=0; i<in.sends_.size(); ++i)
			{
				sends_.push_back(in.sends_[i]);
			}
			wire_ = in.wire_;
			volume_ = in.volume_;
			panning_ = in.panning_;
			mute_ = in.mute_;
		}
		inline void Send(int i,bool value) { sends_[i]= value; }
		inline const bool Send(int i) const { return sends_[i]; }
		inline float &Volume() { return volume_; }
		inline float &Panning() { return panning_; }
		inline bool &Mute() { return mute_; }
		inline ReturnWire &Wire() { return wire_; }
		
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
		std::vector<bool> sends_;
		ReturnWire wire_;
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

	class Grid
	{
	public:
		Grid(){ Init();};
		Grid(int inputs, int sends)
		{
			for(int i=0; i<inputs; ++i)
			{
				inputs_.push_back(InputChannel(sends));
			}
			for(int i=0; i<sends; ++i)
			{
				returns_.push_back(ReturnChannel(sends));
			}
			master_.Init();
		}
		Grid(Grid& copy)
		{
			for(int i=0; i<copy.numinputs(); ++i)
			{
				inputs_.push_back(copy.inputs_[i]);
			}
			for(int i=0; i<copy.numreturns(); ++i)
			{
				returns_.push_back(copy.returns_[i]);
			}
		}
		inline void Init(bool* inputs=0)
		{
			if (inputs_.size() != 0) inputs_.resize(0);
			if (returns_.size() != 0) returns_.resize(0);
			master_.Init();
			inputvalid=inputs;
		}
		inline InputChannel & Channel(int i) { return inputs_[i]; }
		inline const InputChannel & Channel(int i) const { return inputs_[i]; }
		inline ReturnChannel & Return(int i) { return returns_[i]; }
		inline const ReturnChannel & Return(int i) const { return returns_[i]; }
		inline MasterChannel & Master() { return master_; }

		int numinputs() const { return inputs_.size(); }
		int numreturns() const { return returns_.size(); }
		int numsends() const { return returns_.size(); }

		// other accessors, like at() ...

		bool ChannelValid(int i) { assert (i<MAX_CONNECTIONS); return (i<numinputs() && inputvalid[i]); }
		bool ReturnValid(int i) { assert (i<MAX_CONNECTIONS); return (i<numreturns() && Return(i).IsValid()); }

		void InsertChannel(int idx,InputChannel*input=0)
		{
			assert(idx<MAX_CONNECTIONS);
			if ( idx >= numinputs()-1)
			{
				for(int i=numinputs()-1; i<idx; ++i)
				{
					inputs_.push_back(InputChannel(numsends()));
				}
				if (input) inputs_.push_back(*input);
				else inputs_.push_back(InputChannel(numsends()));
			}
			else if (input) inputs_[idx]=*input;
			///\todo: _inputCon and _inputMachines are updated by the Mixer class
		}
		void InsertReturn(int idx,ReturnChannel* retchan=0)
		{
			///\todo: return wire is updated by the Mixer class
			assert(idx<MAX_CONNECTIONS);
			if ( idx >= numreturns()-1)
			{
				for(int i=numreturns()-1; i<idx; ++i)
				{
					returns_.push_back(ReturnChannel(numsends()));
				}
				if (retchan) returns_.push_back(*retchan);
				else returns_.push_back(ReturnChannel(numsends()));
			}
			else if (retchan) returns_[idx]=*retchan;
			for(int i=0; i<numinputs(); ++i)
			{
				Channel(i).ResizeTo(numsends());
			}
			for(int i=0; i<numreturns(); ++i)
			{
				Return(i).ResizeTo(numsends());
			}
		}
		void InsertReturn(int idx,ReturnWire wire)
		{
			InsertReturn(idx);
			Return(idx).Wire()=wire;
		}
		void DiscardChannel(int idx)
		{
			///\todo: _inputCon and _inputMachines are updated by the Mixer class
			assert(idx<MAX_CONNECTIONS);
			if (idx!=numinputs()-1) return;
			int i;
			for (i = idx; i >= 0; i--)
			{
				if (inputvalid[i])
					break;
			}
			inputs_.resize(i+1);
		}
		void DiscardReturn(int idx)
		{
			///\todo: return wire is updated by the Mixer class
			assert(idx<MAX_CONNECTIONS);
			if (idx!=numreturns()-1) return;
			int i;
			for (i = idx; i >= 0; i--)
			{
				if (Return(i).IsValid())
					break;
			}
			returns_.resize(i+1);
		}

		void ExchangeChans(int chann1,int chann2)
		{
			InputChannel tmp = inputs_[chann1];
			inputs_[chann1] = inputs_[chann2];
			inputs_[chann2] = tmp;
			/// Exchange wires
		}
		void ExchangeReturns(int chann1,int chann2)
		{
			ReturnChannel tmp = returns_[chann1];
			returns_[chann1] = returns_[chann2];
			returns_[chann2] = tmp;
			ExchangeSends(chann1,chann2);
		}
		void ExchangeSends(int send1,int send2)
		{
			for (int i(0); i < numinputs(); ++i)
			{
				Channel(i).ExchangeSends(send1,send2);
			}
			for (int i(0); i < numreturns(); ++i)
			{
				Return(i).ExchangeSends(send1,send2);
			}
		}
/*		void ResizeTo(int inputs, int sends)
		{
			inputs_.resize(inputs);
			returns_.resize(sends);
			for(int i=0; i<numinputs(); ++i)
			{
				Channel(i).ResizeTo(numsends());
			}
			for(int i=0; i<numreturns(); ++i)
			{
				Return(i).ResizeTo(numsends());
			}
		}
*/		// other member functions, like reserve()....

	private:
		bool *inputvalid;
		std::vector<InputChannel> inputs_;
		std::vector<ReturnChannel> returns_;
		MasterChannel master_;
	};

	enum
	{
		mix=0,
		send1,
		sendmax=send1+MAX_CONNECTIONS,
		drymix,
		wetmix,
		mute,
		///\todo: remove solo and gain
		solo,
		gain

	};
	enum 
	{
		labelcol=0,
		mastervol,
		chan1,
		chanmax = chan1+MAX_CONNECTIONS,
		return1 = chan1+MAX_CONNECTIONS,
		returnmax = return1+MAX_CONNECTIONS,
		///\todo: remove collabels and colmaster
		collabels=0,
		colmastervol
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
	virtual void InitWireVolume(MachineType mType,int wireIndex,float value);
	virtual int InsertFx(Machine* mac);
	virtual int FindInputWire(int macIndex);
	virtual void DeleteInputWireIndex(int wireIndex);
	virtual void DeleteWires();
	std::string GetAudioInputName(int port);
	virtual int GetAudioInputs() { return 24; };
	virtual int GetAudioOutputs() { return 1; };
	virtual char* GetName(void) { return _psName; };
	virtual int GetNumCols();
	virtual void GetParamName(int numparam,char *name);
	virtual void GetParamRange(int numparam, int &minval, int &maxval);
	virtual void GetParamValue(int numparam,char *parVal);
	virtual int GetParamValue(int numparam);
	virtual bool SetParameter(int numparam,int value);
	virtual bool GetSoloState(int column) { return column==solocolumn_; }
	virtual bool GetMuteState(int column)
	{
		if (column<MAX_CONNECTIONS)
			return thegrid.Channel(column).Mute();
		else
			return thegrid.Return(column-MAX_CONNECTIONS).Mute();
	}
	virtual void SetSoloState(int column,bool solo)
	{
		if (solo ==false && column== solocolumn_)
			solocolumn_=-1;
		else solocolumn_=column;
	}
	virtual void SetMuteState(int column,bool mute)
	{
		if (column<MAX_CONNECTIONS)
			thegrid.Channel(column).Mute()=mute;
		else
			thegrid.Return(column-MAX_CONNECTIONS).Mute()=mute;
	}
	inline int GetSend(int i){ return thegrid.Return(i).Wire().machine_; }
	inline bool SendValid(int i) { return thegrid.ReturnValid(i); }
	virtual float VuChan(int idx);
	virtual float VuSend(int idx);

	virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
	virtual void SaveSpecificChunk(RiffFile * pFile);


protected:
	static char* _psName;

	int solocolumn_;
	Grid thegrid;

	bool _wetmix;
	bool _drymix;
	int _masterVolume;
	int _masterGain;

	///\todo hardcoded limits and wastes

	float _sendGrid[MAX_CONNECTIONS][MAX_CONNECTIONS+1]; // 12 inputs with 12 sends (+dry+gain) each.  (0 -> dry, 1+ -> sends) 
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

};
			/// tweaks:
			/// [0]:
			///  0 -> Master volume
			///  1..12 -> Input volumes
			///  13 -> master gain.
			///  14 -> master drywetmix.
			///  15 -> Solo channel.
			/// [1..12]:
			///  0 -> input wet mix.
			///  1..12 -> sends
			///  13, 14 -> dryonly, wetonly.
			///  15 -> mute.
			/// [13]:
			///  0 -> return 0 mute.
			///  1..12 -> input panning.
			///  13..15 -> returns 1..3 mute.
			/// [14]: 
			///  0 -> return 4 mute
			///  1..12 -> return volumes
			///  13..15 -> returns 5..7 mute
			/// [15]:
			///  0 -> return 8 mute
            ///  1..12 -> return panning
			///  13..15 -> returns 8..11 mute
	}
}

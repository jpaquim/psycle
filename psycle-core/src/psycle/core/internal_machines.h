// -*- mode:c++; indent-tabs-mode:t -*-
#pragma once

#include "machine.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// internal machines

namespace psy {
	namespace core {
		//////////////////////////////////////////////////////////////////////////
		/// dummy machine.
		class Dummy : public Machine
		{
		public:
			Dummy(MachineCallbacks* callbacks, id_type index, CoreSong* song);
			Dummy(Machine *mac);
			virtual ~Dummy() throw();
			virtual int GenerateAudio(int numSamples);
			virtual std::string GetName() const { return _psName; }
			virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
			/// Marks that the Dummy was in fact a VST plugin that couldn't be loaded
			bool wasVST;
		protected:
			static std::string _psName;
		};

		//////////////////////////////////////////////////////////////////////////
		/// note duplicator machine.
		class DuplicatorMac : public Machine
		{
		public:
			DuplicatorMac(MachineCallbacks* callbacks);
			DuplicatorMac(MachineCallbacks* callbacks, id_type index, CoreSong* song);
			virtual ~DuplicatorMac() throw();
			virtual void Init(void);
			virtual void Tick( int channel, const PatternEvent & pData );
			virtual void Stop();
			virtual void PreWork(int numSamples);
			virtual int GenerateAudio( int numSamples );
			virtual std::string GetName() const { return _psName; }
			virtual void GetParamName(int numparam,char *name) const;
			virtual void GetParamRange(int numparam,int &minval,int &maxval) const;
			virtual void GetParamValue(int numparam,char *parVal) const;
			virtual int GetParamValue(int numparam) const;
			virtual bool SetParameter(int numparam,int value);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile) const;

		protected:
			static const int NUM_MACHINES=8;
			void AllocateVoice(int channel, int machine);
			void DeallocateVoice(int channel, int machine);
		std::int16_t macOutput[NUM_MACHINES];
		std::int16_t noteOffset[NUM_MACHINES];
			static std::string _psName;
			bool bisTicking;
			// returns the allocated channel of the machine, for the channel (duplicator's channel) of this tick.
			int allocatedchans[MAX_TRACKS][NUM_MACHINES];
			// indicates if the channel of the specified machine is in use or not
			bool availablechans[MAX_MACHINES][MAX_TRACKS];
		};

		//////////////////////////////////////////////////////////////////////////
		/// master machine.
		class Master : public Machine
		{
		public:
			Master(MachineCallbacks* callbacks);
			Master(MachineCallbacks* callbacks, id_type index, CoreSong* song);
			virtual ~Master() throw();
			virtual void Init(void);
			virtual void Stop();
			virtual void Tick(int channel, const PatternEvent & data );
			virtual int GenerateAudio( int numSamples );
			virtual std::string GetName() const { return _psName; }
			/// Loader for psycle fileformat version 2.
			virtual bool LoadPsy2FileFormat(RiffFile* pFile);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile) const;

			/// this is for the VstHost
			double sampleCount;
			bool _clip;
			bool decreaseOnClip;
			static float* _pMasterSamples;
			int peaktime;
			float currentpeak;
			float _lMax;
			float _rMax;
			int _outDry;
			bool vuupdated;
		protected:
			static std::string _psName;
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
				inline const float &Volume() const { return volume_; }
				inline float &Panning() { return panning_; }
				inline const float &Panning() const { return panning_; }
				inline float &DryMix() { return drymix_; }
				inline const float &DryMix() const { return drymix_; }
				inline bool &Mute() { return mute_; }
				inline const bool &Mute() const { return mute_; }
				inline bool &DryOnly() { return dryonly_; }
				inline const bool &DryOnly() const { return dryonly_; }
				inline bool &WetOnly() { return wetonly_; }
				inline const bool &WetOnly() const { return wetonly_; }
		
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
				bool IsValid() const { return (machine_!=-1); }
		
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
				inline const bool &MasterSend() const{ return mastersend_; }
				inline float &Volume() { return volume_; }
				inline const float &Volume() const { return volume_; }
				inline float &Panning() { return panning_; }
				inline const float &Panning() const { return panning_; }
				inline bool &Mute() { return mute_; }
				inline const bool &Mute() const { return mute_; }
				inline MixerWire &Wire() { return wire_; }
				inline const MixerWire &Wire() const { return wire_; }
				
				bool IsValid() const { return wire_.IsValid(); }
		
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
				inline const float &Volume() const { return volume_; }
				inline float &DryWetMix() { return drywetmix_; }
				inline const float &DryWetMix() const { return drywetmix_; }
				inline float &Gain() { return gain_; }
				inline const float &Gain() const { return gain_; }
		
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

			Mixer(MachineCallbacks* callbacks);
			Mixer(MachineCallbacks* callbacks, id_type index, CoreSong* song);
			virtual ~Mixer() throw();
			virtual void Init(void);
			virtual void Tick( int channel, const PatternEvent & pData );
			virtual int GenerateAudio(int numSamples );
			void FxSend(int numSamples );
			void Mix(int numSamples);
			virtual void InsertInputWire(Machine& srcMac, Wire::id_type dstWire,InPort::id_type dstType, float initialVol=1.0f);
			virtual bool MoveWireSourceTo(Machine& srcMac, InPort::id_type dsttype, Wire::id_type dstwire, OutPort::id_type srctype = OutPort::id_type(0));
			virtual void GetWireVolume(Wire::id_type wireIndex, float &value) const { value = GetWireVolume(wireIndex); }
			virtual float GetWireVolume(Wire::id_type wireIndex) const ;
			virtual void SetWireVolume(Wire::id_type wireIndex,float value);
			virtual Wire::id_type FindInputWire(Machine::id_type macIndex) const;
			virtual Wire::id_type GetFreeInputWire(InPort::id_type slotType=InPort::id_type(0)) const;
			virtual void ExchangeInputWires(Wire::id_type first,Wire::id_type second, InPort::id_type firstType= InPort::id_type(0), InPort::id_type secondType = InPort::id_type(0));
			virtual void NotifyNewSendtoMixer(Machine& caller,Machine& senderMac);
			virtual void DeleteInputWire(Wire::id_type wireIndex, InPort::id_type dstType);
			virtual void DeleteWires();
			virtual std::string GetAudioInputName(Wire::id_type wire) const;
			virtual std::string GetPortInputName(InPort::id_type port) const;
			virtual int GetInPorts() const { return 2; }
			virtual int GetAudioInputs() const{ return GetInPorts() * MAX_CONNECTIONS; }
			virtual std::string GetName() const { return _psName; }
			virtual int GetNumCols() const;
			virtual void GetParamName(int numparam,char *name) const;
			virtual void GetParamRange(int numparam, int &minval, int &maxval) const;
			virtual void GetParamValue(int numparam,char *parVal) const;
			virtual int GetParamValue(int numparam) const;
			virtual bool SetParameter(int numparam,int value);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile) const;

			bool GetSoloState(int column) const { return column==solocolumn_; }
			void SetSoloState(int column,bool solo)
			{
				if (solo ==false && column== solocolumn_)
					solocolumn_=-1;
				else solocolumn_=column;
			}
			virtual float VuChan(Wire::id_type idx) const;
			virtual float VuSend(Wire::id_type idx) const;

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
			inline bool ChannelValid(int i) const { assert (i<MAX_CONNECTIONS); return (i<numinputs() && _inputCon[i]); }
			inline bool ReturnValid(int i) const { assert (i<MAX_CONNECTIONS); return (i<numreturns() && Return(i).IsValid()); }
			inline bool SendValid(int i) const { assert (i<MAX_CONNECTIONS); return (i<numsends() && sends_[i].IsValid()); }
		
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
			static std::string _psName;
		
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


		//////////////////////////////////////////////////////////////////////////
		/// LFO machine
		class LFO : public Machine
		{
		public:
			LFO(MachineCallbacks* callbacks);
		LFO(MachineCallbacks* callbacks, id_type index, CoreSong* song);
			virtual ~LFO() throw();
			virtual void Init(void);
			virtual void Tick( int channel, const PatternEvent & pData );
			virtual void PreWork(int numSamples);
			virtual int GenerateAudio( int numSamples );
			virtual std::string GetName() const { return _psName; }
			virtual void GetParamName(int numparam,char *name) const;
			virtual void GetParamRange(int numparam,int &minval,int &maxval) const;
			virtual void GetParamValue(int numparam,char *parVal) const;
			virtual int GetParamValue(int numparam) const;
			virtual bool SetParameter(int numparam,int value);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile) const;


			///\name constants
			///\{
			int const static LFO_SIZE = 2048;
			int const static MAX_PHASE = LFO_SIZE;
			int const static MAX_SPEED = 100000;
			int const static MAX_DEPTH = 100;
			int const static NUM_CHANS = 6;
			///\}

			struct lfo_types
			{
				enum lfo_type
				{
					sine, tri, saw, /*sawdown,*/ square, ///< depth ranges from -100% to 100% -- inverse saw is redundant
					num_lfos
				};
			};

			struct prms
			{
				/// our parameter indices
				enum prm
				{
					wave, speed,
					mac0,   mac1,   mac2,   mac3,   mac4,   mac5,
					prm0,   prm1,   prm2,   prm3,   prm4,   prm5,
					level0, level1, level2, level3, level4, level5,
					phase0, phase1, phase2, phase3, phase4, phase5,
					num_params
				};
			};

		protected:
			/// fills the lfo table based on the value of waveform
			virtual void FillTable();
			/// initializes data to start modulating the param given by 'which'
			virtual void ParamStart(int which);
			/// resets a parameter that's no longer being modulated
			virtual void ParamEnd(int which);

			//parameter settings
			
		std::int16_t waveform;
		std::int32_t lSpeed;
		std::int16_t macOutput[NUM_CHANS];
		std::int16_t paramOutput[NUM_CHANS];
		std::int32_t phase[NUM_CHANS];
		std::int32_t level[NUM_CHANS];

			//internal state vars
			
			/// position in our lfo
			float lfoPos;
			/// our lfo
			float waveTable[LFO_SIZE];
			/// value of knob when last seen-- used to compensate for outside changes
		std::int16_t prevVal[NUM_CHANS];
			/// where knob should be at lfo==0
		std::int32_t centerVal[NUM_CHANS];

			static std::string _psName;
			bool bisTicking;
		};
	}
}

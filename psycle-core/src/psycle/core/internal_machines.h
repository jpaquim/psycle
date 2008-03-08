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
			Mixer(MachineCallbacks* callbacks);
			Mixer(MachineCallbacks* callbacks, id_type index, CoreSong* song);
			virtual ~Mixer() throw();
			virtual void Init(void);
			virtual void Tick( int channel, const PatternEvent & pData );
			virtual int GenerateAudio(int numSamples );
			void FxSend(int numSamples );
			void Mix(int numSamples);
			virtual std::string GetName() const { return _psName; }
			virtual int GetNumCols() const;
			virtual void GetParamName(int numparam,char *name) const;
			virtual void GetParamRange(int /*numparam*/, int &minval, int &maxval) const { minval=0; maxval=100; }
			virtual void GetParamValue(int numparam,char *parVal) const;
			virtual int GetParamValue(int numparam) const;
			virtual bool SetParameter(int numparam,int value);
			virtual int GetAudioInputs() { return 24; }
			virtual int GetAudioOutputs() { return 1; }
			virtual std::string GetAudioInputName(InPort::id_type port);
			virtual std::string GetAutioOutputName(OutPort::id_type /*port*/) { std::string rettxt = "Stereo Output"; return rettxt; }
			virtual bool ConnectTo(Machine & dst, InPort::id_type dstport = InPort::id_type(0), OutPort::id_type outport = OutPort::id_type(0), float volume = 1.0f);
			virtual int GetSend(int i){ assert(i<MAX_CONNECTIONS); return _send[i]; }
			virtual bool SendValid(int i) { assert(i<MAX_CONNECTIONS); return _sendValid[i]; }
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile) const;

			virtual float VuChan(Wire::id_type idx);
			virtual float VuSend(Wire::id_type idx);

		protected:
			///\todo hardcoded limits and wastes
			float _sendGrid[MAX_CONNECTIONS][MAX_CONNECTIONS+1]; // 12 inputs with 12 sends (+dry) each.  (0 -> dry, 1+ -> sends)
			/// Incoming send, Machine number
			///\todo hardcoded limits and wastes
			id_type _send[MAX_CONNECTIONS];
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

			static std::string _psName;
		};

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

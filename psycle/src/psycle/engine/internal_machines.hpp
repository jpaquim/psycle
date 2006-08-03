#pragma once

#include "machine.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// internal machines
namespace psycle {
	namespace host {
		//////////////////////////////////////////////////////////////////////////
		/// dummy machine.
		class Dummy : public Machine
		{
		public:
			Dummy(id_type index);
			virtual ~Dummy() throw();
			static Machine* CreateFromType(Machine::id_type _id, std::string _dllname);
			virtual void Work(int numSamples);
			virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
		public:
			//\todo: to be removed... someday( Marks that the Dummy was in fact a VST plugin that couldn't be loaded)
			bool wasVST;
		};

		//////////////////////////////////////////////////////////////////////////
		/// note duplicator machine.
		class DuplicatorMac : public Machine
		{
		public:
			DuplicatorMac();
			DuplicatorMac(id_type index);
			virtual ~DuplicatorMac() throw();
			static Machine* CreateFromType(Machine::id_type _id, std::string _dllname);
			virtual void Init(void);
			virtual void Tick( int channel,PatternEntry* pData);
			virtual void Work(int numSamples);
			virtual void GetParamName(int numparam,char *name);
			virtual void GetParamRange(int NUMPARSE,int &minval,int &maxval);
			virtual void GetParamValue(int numparam,char *parVal);
			virtual int GetParamValue(int numparam);
			virtual bool SetParameter(int numparam,int value);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile);

		protected:
			short macOutput[8];
			short noteOffset[8];
			bool bisTicking;
		};

		//////////////////////////////////////////////////////////////////////////
		/// master machine.
		class Master : public Machine
		{
		public:
			Master();
			Master(id_type index);
			virtual ~Master() throw();
			static Machine* CreateFromType(Machine::id_type _id, std::string _dllname);
			virtual void Init(void);
			virtual void Stop();
			virtual void Tick(PatternEntry *pEntry);
			virtual void Work(int numSamples);
			virtual bool LoadOldFileFormat(RiffFile * pFile);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile);

			/// this is for the VstHost
			double sampleCount;
			int _outDry;
			bool _clip;
			bool decreaseOnClip;
			static float* _pMasterSamples;
			int peaktime;
			float currentpeak;
			float _lMax;
			float _rMax;
			bool vuupdated;
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
			Mixer(id_type index);
			virtual ~Mixer() throw();
			static Machine* CreateFromType(Machine::id_type _id, std::string _dllname);
			virtual void Init(void);
			virtual void Tick( int channel,PatternEntry* pData);
			virtual void Work(int numSamples);
			void FxSend(int numSamples);
			void Mix(int numSamples);
			virtual int GetNumCols();
			virtual void GetParamName(int numparam,char *name);
			virtual void GetParamRange(int numparam, int &minval, int &maxval) { minval=0; maxval=100; };
			virtual void GetParamValue(int numparam,char *parVal);
			virtual int GetParamValue(int numparam);
			virtual bool SetParameter(int numparam,int value);
			virtual int GetAudioInputs() { return 24; };
			virtual int GetAudioOutputs() { return 1; };
			virtual std::string GetAudioInputName(InPort::id_type port);
			virtual std::string GetAutioOutputName(OutPort::id_type port) { std::string rettxt = "Stereo Output"; return rettxt; };
			virtual bool ConnectTo(Machine & dst, InPort::id_type dstport = InPort::id_type(0), OutPort::id_type outport = OutPort::id_type(0), float volume = 1.0f);
			virtual int GetSend(int i){ assert(i<MAX_CONNECTIONS); return _send[i]; }
			virtual bool SendValid(int i) { assert(i<MAX_CONNECTIONS); return _send[i]; }
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile);

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

		};

		//////////////////////////////////////////////////////////////////////////
		/// LFO machine
		class LFO : public Machine
		{
		public:
			LFO();
			LFO(id_type index);
			virtual ~LFO() throw();
			static Machine* CreateFromType(Machine::id_type _id, std::string _dllname);
			virtual void Init(void);
			virtual void Tick( int channel,PatternEntry* pData);
			virtual void PreWork(int numSamples);
			virtual void Work(int numSamples);
			virtual void GetParamName(int numparam,char *name);
			virtual void GetParamRange(int numparam,int &minval,int &maxval);
			virtual void GetParamValue(int numparam,char *parVal);
			virtual int GetParamValue(int numparam);
			virtual bool SetParameter(int numparam,int value);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile);


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
					sine, tri, saw, /*sawdown,*/ square,	//depth ranges from -100% to 100% -- inverse saw is redundant
					num_lfos
				};
			};

			struct prms
			{
				/// our parameter indices
				enum prm
				{
					wave, speed,
					mac0,	mac1,	mac2,	mac3,	mac4,	mac5,
					prm0,	prm1,	prm2,	prm3,	prm4,	prm5,
					level0,	level1,	level2,	level3,	level4,	level5,
					phase0,	phase1,	phase2,	phase3,	phase4,	phase5,
					num_params
				};
			};

		protected:
			//protected member funcs
			virtual void FillTable();				//fills the lfo table based on the value of waveform
			virtual void ParamStart(int which);		//initializes data to start modulating the param given by 'which'
			virtual void ParamEnd(int which);		//resets a parameter that's no longer being modulated

			//parameter settings
			short waveform;	
			int lSpeed;
			short macOutput[NUM_CHANS];
			short paramOutput[NUM_CHANS];
			int phase[NUM_CHANS];
			int level[NUM_CHANS];

			//internal state vars
			float lfoPos;				//position in our lfo
			float waveTable[LFO_SIZE];	//our lfo
			int prevVal[NUM_CHANS];				//value of knob when last seen-- used to compensate for outside changes
			int centerVal[NUM_CHANS];			//where knob should be at lfo==0

			bool bisTicking;

		};


		//////////////////////////////////////////////////////////////////////////
		/// Automator
		class EnvelopeWindow;

		class Automator : public Machine
		{
		public:

			friend EnvelopeWindow;

			Automator();
			Automator(id_type index);
			virtual ~Automator() throw();
			static Machine* CreateFromType(Machine::id_type _id, std::string _dllname);
			virtual void Init(void);
			virtual void Tick( int channel,PatternEntry* pData);
			virtual void Tick();
			virtual void PreWork(int numSamples);
			virtual void Work(int numSamples);
			virtual void Stop();
			virtual void GetParamName(int numparam,char *name);
			virtual void GetParamRange(int numparam,int &minval,int &maxval);
			virtual void GetParamValue(int numparam,char *parVal);
			virtual int GetParamValue(int numparam);
			virtual bool SetParameter(int numparam,int value);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile);


			virtual bool IsDiscrete() { return bDiscrete; }
			virtual bool IsRelative() { return bRelative; }
			///\name constants
			///\{
			int const static DSTEP_SCALER = 100;

			int const static DLENGTH_MAX = 256;
			int const static CLENGTH_MAX = 60000;	//one minute(!)
			int const static DSTEP_MAX = 32 * DSTEP_SCALER;
			///\}

			struct prms
			{
				enum prm
				{
					discrete, relative,
					clength, dlength, dstep,
					num_params
				};
			};

			class Node
			{
			public:
				Node(int time_, float value_) : time(time_), value(value_) {}
				int time;
				float value;
				bool operator<(const Node &rhs) {return (time < rhs.time); }
				bool operator<(const int rhs)   {return (time < rhs); }
				operator int() {return this->time; }
			};

		private:

			class Track
			{
			public:
				Track(int mach_, int param_) : param(param_), mach(mach_), pos(0.0f) {}
				virtual ~Track() {}
				int const mach;
				int const param;
				//for relative mode-- where dest param should be when envelope is at 0
				int centerVal;
				//in relative mode, this is where dest param was last time we saw it
				//in absolute mode, this is the value of the parameter when the modulation began
				int prevVal;
				int minVal;
				int maxVal;
				float pos;			//pos takes on a double meaning, depending on the mode..
			};						//ms for continuous, and increments of dStepSize for discrete.


			//interprets a track's position based on the mode and corresponding envelope
			//returns in range [-1,1] for relative, and [0,1] for absolute
			float EvaluatePos(float pos);

			//data
			std::vector<float> dTable;		//discrete envelope table
			std::vector<Node> cTable;		//continuous envelope table


			std::vector<Track*> tracks;

			//length of discrete envelope, in increments of dStepSize
			int dLength;
			//ratio of discrete step to tick length
			float dStepSize;
			//length of continuous envelope, in -ms-
			int cLength;

			int sampsPerStep;
			int sampsPerMs;

			bool bDiscrete;			//true=discrete, false=continuous
			bool bRelative;			//true=relative, false=absolute

			//whether or not to put a parameter back the way we found it when we're done
			bool bResetWhenDone;

			bool bisTicking;

		};
	}
}

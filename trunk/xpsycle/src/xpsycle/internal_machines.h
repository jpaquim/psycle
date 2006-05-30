#pragma once

#include "machine.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// internal machines

//namespace psycle {
//	namespace host {

		/// dummy machine.
		class Dummy : public Machine
		{
		public:
			Dummy(id_type index);
			virtual ~Dummy() throw();
			virtual void Work(int numSamples);
			virtual std::string GetName() const { return _psName; };
			virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
			/// Marks that the Dummy was in fact a VST plugin that couldn't be loaded
			bool wasVST;
		protected:
			static std::string _psName;
		};

		/// note duplicator machine.
		class DuplicatorMac : public Machine
		{
		public:
			DuplicatorMac();
			DuplicatorMac(id_type index);
			virtual ~DuplicatorMac() throw();
			virtual void Init(void);
			virtual void Tick( int channel,PatternEntry* pData);
			virtual void Work(int numSamples);
			virtual std::string GetName() const { return _psName; };
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
			static std::string _psName;
			bool bisTicking;
		};


		/// master machine.
		class Master : public Machine
		{
		public:
			Master();
			Master(id_type index);
			virtual ~Master() throw();
			virtual void Init(void);
			virtual void Work(int numSamples);
			virtual std::string GetName() const { return _psName; };
//			virtual bool LoadOldFileFormat(RiffFile * pFile);
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
			bool vuupdated;
		protected:
			static std::string _psName;
		};

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
			virtual void Init(void);
			virtual void Tick( int channel,PatternEntry* pData);
			virtual void Work(int numSamples);
			void FxSend(int numSamples);
			void Mix(int numSamples);
			virtual std::string GetName() const { return _psName; };
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

			static std::string _psName;
		};


		/// LFO machine
		class LFO : public Machine
		{
		public:
			LFO();
			LFO(id_type index);
			virtual ~LFO() throw();
			virtual void Init(void);
			virtual void Tick( int channel,PatternEntry* pData);
			virtual void Work(int numSamples);
			virtual std::string GetName() const { return _psName; };
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
			int const static MAX_SPEED = 10000;
			int const static MAX_DEPTH = 100;
			int const static NUM_CHANS = 4;
			///\}

			struct lfo_types
			{
				enum lfo_type
				{
					sine=0, tri, sawup, sawdown, square
				};
			};

			struct prms
			{
				/// our parameter indices
				enum prm
				{
					wave=0, pwidth, speed,
					mac0,	mac1,	mac2,	mac3,
					prm0,	prm1,	prm2,	prm3,
					level0,	level1,	level2,	level3,
					phase0,	phase1,	phase2,	phase3,
					display,
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
			int	pWidth;
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


			static std::string _psName;
			bool bisTicking;

		};
//	}
//}

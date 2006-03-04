///\file
///\brief interface file for psycle::host::Machine
#pragma once
#include "SongStructs.hpp"
#include "Dsp.hpp"
#include "Helpers.hpp"
#include "Constants.hpp"
#include "FileIO.hpp"
#include <processor/fpu.hpp>
#include <stdexcept>
namespace psycle
{
	namespace host
	{
		class Machine; // forward declaration
		class RiffFile; // forward declaration

		/// Base class for exceptions thrown from plugins.
		class exception : public std::runtime_error
		{
			public:
				exception(std::string const & what) : std::runtime_error(what) {}
		};

		/// Classes derived from exception.
		namespace exceptions
		{
			/// Base class for exceptions caused by errors on library operation.
			class library_error : public exception
			{
				public:
					library_error(std::string const & what) : exception(what) {}
			};

			/// Classes derived from library.
			namespace library_errors
			{
				/// Exception caused by library loading failure.
				class loading_error : public library_error
				{
					public:
						loading_error(std::string const & what) : library_error(what) {}
				};

				/// Exception caused by symbol resolving failure in a library.
				class symbol_resolving_error : public library_error
				{
					public:
						symbol_resolving_error(std::string const & what) : library_error(what) {}
				};
			}

			/// Base class for exceptions caused by an error in a library function.
			class function_error : public exception
			{
				public:
					function_error(std::string const & what, std::exception const * const exception = 0) : host::exception(what), exception_(exception) {}
				public:
					std::exception const inline * const exception() const throw() { return exception_; }
				private:
					std::exception const * const        exception_;
			};
			
			///\relates function_error.
			namespace function_errors
			{
				namespace
				{
					template<typename e> const std::string string(e const & e) { std::ostringstream s; s << e; return s.str(); }
					template<> const std::string string<std::exception>(std::exception const & e) { return e.what(); }
					template<> const std::string string<void const *>(void const * const &) { return "Type of exception is unkown, cannot display any further information."; }
				}

				template<typename e> void rethrow(Machine & machine, std::string const & function, e const * const e, std::exception const * const standard) throw(function_error)
				{
					std::ostringstream s;
					s
						<< "Machine had an exception in function '" << function << "'." << std::endl
						<< typeid(*e).name() << std::endl
						<< string(*e);
					function_error const function_error(s.str(), standard);
					machine.crashed(function_error);
					throw function_error;
				}

				template<typename e> void inline rethrow(Machine & machine, std::string const & function, e const * const e = 0) throw(function_error)
				{
					rethrow(machine, function, e, 0);
				}

				template<> void inline rethrow<std::exception>(Machine & machine, std::string const & function, std::exception const * const e) throw(function_error)
				{
					rethrow(machine, function, e, e);
				}

				/// Exception caused by a bad returned value from a library function.
				class bad_returned_value : public function_error
				{
					public:
						bad_returned_value(std::string const & what) : function_error(what) {}
				};
			}
		}

		/// Internal Machines' Parameters' Class.
		class CIntMachParam			
		{
		public:
			/// Short name
			const char * name;		
			/// >= 0
			int minValue;
			/// <= 65535
			int maxValue;
		};

		/// well, what to say?
		class Machine
		{
			///\name crash handling
			///\{
				public:
					/// This function should be called when an exception was thrown from the machine.
					/// This will mark the machine as crashed, i.e. crashed() will return true,
					/// and it will be disabled.
					///\param e the exception that occured, converted to a std::exception if needed.
					void crashed(std::exception const & e) throw();

				public:
					/// Tells wether this machine has crashed.
					bool const inline & crashed() const throw() { return crashed_; }
				private:
					bool                crashed_;

				public:
					processor::fpu::exception_mask::type const inline & fpu_exception_mask() const throw() { return fpu_exception_mask_; }
					processor::fpu::exception_mask::type       inline & fpu_exception_mask()       throw() { return fpu_exception_mask_; }
				private:
					processor::fpu::exception_mask::type                fpu_exception_mask_;
			///\}

		public:
			static Machine * LoadFileChunk(RiffFile* pFile, int index, int version,bool fullopen=true);
			Machine();
			virtual ~Machine() throw();
			virtual void Init();
			virtual void PreWork(int numSamples);
			virtual void Work(int numSamples);
			virtual void WorkNoMix(int numSamples);
			virtual void Tick() {};
			virtual void Tick(int track, PatternEntry * pData) {};
			virtual void Stop() {};
			virtual void SetPan(int newpan);
			virtual void GetWireVolume(int wireIndex, float &value) { value = _inputConVol[wireIndex] * _wireMultiplier[wireIndex]; };
			virtual void SetWireVolume(int wireIndex,float value) { _inputConVol[wireIndex] = value / _wireMultiplier[wireIndex]; };
			virtual bool GetDestWireVolume(int srcIndex, int WireIndex,float &value);
			virtual bool SetDestWireVolume(int srcIndex, int WireIndex,float value);
			virtual void InitWireVolume(MachineType mType,int wireIndex,float value);
			virtual int FindInputWire(int macIndex);
			virtual int FindOutputWire(int macIndex);
			virtual const char * const GetDllName() const throw() { return "built-in"; };
			virtual char * GetName() = 0;
			virtual char * GetEditName() { return _editName; }
			virtual int GetNumParams() { return _numPars; };
			virtual int GetNumCols() { return _nCols; };
			virtual void GetParamName(int numparam, char * name) { name[0]='\0'; };
			virtual void GetParamRange(int numparam, int &minval, int &maxval) {minval=0; maxval=0; };
			virtual void GetParamValue(int numparam, char * parval) { parval[0]='\0'; };
			virtual int GetParamValue(int numparam) { return 0; };
			virtual bool SetParameter(int numparam, int value) { return false;}; 
			virtual void SetSampleRate(int sr) {};
			virtual bool Load(RiffFile * pFile);
			virtual bool LoadSpecificChunk(RiffFile* pFile, int version);

			virtual void SaveFileChunk(RiffFile * pFile);
			virtual void SaveSpecificChunk(RiffFile * pFile);
			virtual void SaveDllName(RiffFile * pFile);
		protected:
			void SetVolumeCounter(int numSamples);
			//void SetVolumeCounterAccurate(int numSamples);

		public:
			int _macIndex;
			MachineType _type;
			MachineMode _mode;
			bool _bypass;
			bool _mute;
			bool _waitingForSound;
			bool _stopped;
			bool _worked;
			/// left data
			float *_pSamplesL;
			/// right data
			float *_pSamplesR;						
			/// left chan volume
			float _lVol;							
			/// right chan volume
			float _rVol;							
			/// numerical value of panning.
			int _panning;							
			int _x;
			int _y;
			char _editName[32];
			int _numPars;
			int _nCols;
			/// Incoming connections Machine number
			int _inputMachines[MAX_CONNECTIONS];	
			/// Outgoing connections Machine number
			int _outputMachines[MAX_CONNECTIONS];	
			/// Incoming connections Machine vol
			float _inputConVol[MAX_CONNECTIONS];	
			/// Value to multiply _inputConVol[] to have a 0.0...1.0 range
			float _wireMultiplier[MAX_CONNECTIONS];	
			/// Outgoing connections activated
			bool _connection[MAX_CONNECTIONS];      
			/// Incoming connections activated
			bool _inputCon[MAX_CONNECTIONS];		
			/// number of Incoming connections
			int _numInputs;							
			/// number of Outgoing connections
			int _numOutputs;						
			PatternEntry TriggerDelay[MAX_TRACKS];
			int TriggerDelayCounter[MAX_TRACKS];
			int RetriggerRate[MAX_TRACKS];
			int ArpeggioCount[MAX_TRACKS];
			bool TWSActive;
			int TWSInst[MAX_TWS];
			int TWSSamples;
			float TWSDelta[MAX_TWS];
			float TWSCurrent[MAX_TWS];
			float TWSDestination[MAX_TWS];
			/// output peak level for DSP
			float _volumeCounter;					
			/// output peak level for display
			int _volumeDisplay;	
			/// output peak level for display
			int _volumeMaxDisplay;
			/// output peak level for display
			int _volumeMaxCounterLife;
			unsigned long int _cpuCost;
			unsigned long int _wireCost;
			int _scopePrevNumSamples;
			int	_scopeBufferIndex;
			float *_pScopeBufferL;
			float *_pScopeBufferR;
			/// The topleft point of a square where the wire triangle is centered when drawn. (Used to detect when to open the wire dialog)
			CPoint _connectionPoint[MAX_CONNECTIONS];
		};

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
			virtual void Tick( int channel,PatternEntry* pData);
			virtual void Work(int numSamples);
			virtual char* GetName(void) { return _psName; };
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
			static char* _psName;
			bool bisTicking;
		};


		/// master machine.
		class Master : public Machine
		{
		public:
			Master();
			Master(int index);
			virtual void Init(void);
			virtual void Work(int numSamples);
			virtual char* GetName(void) { return _psName; };
			virtual bool Load(RiffFile * pFile);
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
		protected:
			static char* _psName;
		};

		/// master machine.
		class Mixer : public Machine
		{
		public:
			enum
			{
				mix=0,
				send0,
				sendmax=send0+MAX_CONNECTIONS
			};
			Mixer();
			Mixer(int index);
			virtual void Init(void);
			virtual void Tick( int channel,PatternEntry* pData);
			virtual void Work(int numSamples);
			void FxSend(int numSamples);
			void Mix(int numSamples);
			virtual char* GetName(void) { return _psName; };
			virtual int GetNumCols();
			virtual void GetParamName(int numparam,char *name);
			virtual void GetParamRange(int numparam, int &minval, int &maxval) { minval=0; maxval=100; };
			virtual void GetParamValue(int numparam,char *parVal);
			virtual int GetParamValue(int numparam);
			virtual bool SetParameter(int numparam,int value);
			virtual int GetSend(int i){ ASSERT(i<MAX_CONNECTIONS); return _send[i]; }
			virtual bool SendValid(int i) { ASSERT(i<MAX_CONNECTIONS); return _send[i]; }
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile);
			

			virtual float VuChan(int idx);
			virtual float VuSend(int idx);
		protected:
			float _sendGrid[MAX_CONNECTIONS][MAX_CONNECTIONS+1]; // 12 inputs with 12 sends (+dry) each.  (0 -> dry, 1+ -> sends)
			/// Incoming send, Machine number
			int _send[MAX_CONNECTIONS];	
			/// Incoming send, connection volume
			float _sendVol[MAX_CONNECTIONS];	
			/// Value to multiply _sendVol[] to have a 0.0..1.0 range
			float _sendVolMulti[MAX_CONNECTIONS];
			/// Incoming connections activated
			bool _sendValid[MAX_CONNECTIONS];		

			static char* _psName;
		};



		///\todo make that a naked inline function
		#define CPUCOST_INIT(cost)	\
			ULONG cost;				\
			__asm rdtsc				\
			__asm mov cost, eax
		/*
		///\todo make that a naked inline function
		#define CPUCOST_CALC(cost, numSamples)	\
			__asm {								\
			__asm rdtsc							\
			__asm sub eax, cost					\
			__asm mov cost, eax					\
			}									\
			cost = cost* Global::pConfig->_pOutputDriver->_samplesPerSec/ numSamples
		//	cost = (cost*1000)/(numSamples*(Global::_cpuHz/Global::pConfig->_pOutputDriver->_samplesPerSec));
		*/
		///\todo make that a naked inline function
		#define CPUCOST_CALC(cost, numSamples)	\
			__asm rdtsc							\
			__asm sub eax, cost					\
			__asm mov cost, eax


		inline void Machine::SetVolumeCounter(int numSamples)
		{
			_volumeCounter = dsp::GetMaxVol(_pSamplesL, _pSamplesR, numSamples);
			if(_volumeCounter > 32768.0f) _volumeCounter = 32768.0f;
			int temp((f2i(fast_log2(_volumeCounter) * 78.0f * 4 / 14.0f) - (78 * 3)));// not 100% accurate, but looks as it sounds
			// prevent downward jerkiness
			if(temp > 97) temp = 97;
			else if (temp <0) temp=0;
			if(temp > _volumeDisplay) _volumeDisplay = temp;
			--_volumeDisplay;
		};

		/*
		inline void Machine::SetVolumeCounterAccurate(int numSamples)
		{
			_volumeCounter = Dsp::GetMaxVolAccurate(_pSamplesL, _pSamplesR, numSamples);
		};
		*/
	}
}

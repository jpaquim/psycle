///\file
///\brief interface file for psycle::host::Machine
#pragma once
#include "SongStructs.hpp"
#include "Dsp.hpp"
#include "Helpers.hpp"
#include "Constants.hpp"
#include "FileIO.hpp"
namespace psycle
{
	namespace host
	{
		class RiffFile; // forward declaration

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
		private:
			bool crashed_;
		public:
			/// This function should be called when an exception was thrown from the machine.
			/// This will mark the machine as crashed, i.e. crashed() will return true,
			/// and it will be disabled.
			///\param e the exception that occured, converted to a std::exception if needed.
			void crashed(const std::exception & e) throw();
			/// Tells wether this machine has crashed.
			const bool & crashed() const throw() { return crashed_; }
			///\}

		public:
			static Machine * LoadFileChunk(RiffFile* pFile, int index, int version,bool fullopen=true);
			Machine();
			virtual ~Machine() throw();
			virtual void Init();
			virtual void PreWork(int numSamples);
			virtual void Work(int numSamples);
			virtual void Tick() {};
			virtual void Tick(int track, PatternEntry * pData) {};
			virtual void Stop() {};
			virtual void SetPan(int newpan);
			virtual void GetWireVolume(int wireIndex, float &value) { value = _inputConVol[wireIndex] * _wireMultiplier[wireIndex]; }
			virtual void SetWireVolume(int wireIndex,float value) { _inputConVol[wireIndex] = value / _wireMultiplier[wireIndex]; }
			virtual bool GetDestWireVolume(int srcIndex, int WireIndex,float &value);
			virtual bool SetDestWireVolume(int srcIndex, int WireIndex,float value);
			virtual void InitWireVolume(MachineType mType,int wireIndex,float value);
			virtual int FindInputWire(int macIndex);
			virtual int FindOutputWire(int macIndex);
			virtual const char * const GetDllName() const throw() { return "built-in"; }
			virtual char * GetName() = 0;
			virtual int GetNumParams() /* = 0; */ { return _numPars; } // [bohan] was { return _numPars; } but there's no need for dummy body anymore ; msvc6 used to generate buggy code on pure virtual functions but it's not gone with msvc7.1 ;-)
			virtual void GetParamName(int numparam, char * name) { name[0]='\0'; }
			virtual void GetParamValue(int numparam, char * parval) { parval[0]='\0'; };
			virtual int GetParamValue(int numparam) { return 0; };
			virtual bool SetParameter(int numparam, int value) { return false;}; 
			virtual void SetSampleRate(int sr) {};
			virtual bool Load(RiffFile * pFile);
			virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version);

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
			int _numPars; // [bohan] there's no need for dummy body anymore ; msvc6 used to generate buggy code on pure virtual functions but it's not gone with msvc7.1 ;-)
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
			virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version);
			/// Marks that the Dummy was in fact a VST plugin that couldn't be loaded
			bool wasVST;
		protected:
			static char * _psName;
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
			virtual bool LoadSpecificFileChunk(RiffFile * pFile, int version);
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


		/// Base class for exceptions thrown from plugins.
		class exception : public std::runtime_error
		{
		public:
			 exception(const std::string & what) : std::runtime_error(what) {}
		};

		/// Classes derived from exception.
		namespace exceptions
		{
			/// Base class for exceptions caused by errors on library operation.
			class library_error : public exception
			{
			public:
				library_error(const std::string & what) : exception(what) {}
			};

			/// Classes derived from library.
			namespace library_errors
			{
				/// Exception caused by library loading failure.
				class loading_error : public library_error
				{
				public:
					loading_error(const std::string & what) : library_error(what) {}
				};

				/// Exception caused by symbol resolving failure in a library.
				class symbol_resolving_error : public library_error
				{
				public:
					symbol_resolving_error(const std::string & what) : library_error(what) {}
				};
			}

			/// Base class for exceptions caused by an error in a library function.
			class function_error : public exception
			{
			public:
				function_error(const std::string & what) : exception(what) {}
			};
			
			/// Classes derived from function.
			namespace function_errors
			{
				namespace
				{
					template<typename e> const std::string string(const e & e) { std::ostringstream s; s << e; return s.str(); }
					template<> const std::string string<std::exception>(const std::exception & e) { return e.what(); }
					template<> const std::string string<const void *>(const void * const &) { return "Type of exception is unkown, cannot display any further information."; }
				}

				template<typename e> void rethrow(Machine & machine, const std::string & function, const e * const e = 0) throw(function_error)
				{
					std::ostringstream s; s
						<< "Machine crashed: " << machine._editName;
					if(machine.GetDllName()) s
						<< ": " << machine.GetDllName();
					s
						<< std::endl
						<< "Machine had an exception in function '" << function << "'." << std::endl
						<< typeid(*e).name() << std::endl
						<< string(*e);
					const function_error function_error(s.str());
					machine.crashed(function_error);
					throw function_error;
				}

				/// Exception caused by a bad returned value from a library function.
				class bad_returned_value : public function_error
				{
				public:
					bad_returned_value(const std::string & what) : function_error(what) {}
				};
			}
		}

		inline void Machine::SetVolumeCounter(int numSamples)
		{
			_volumeCounter = Dsp::GetMaxVol(_pSamplesL, _pSamplesR, numSamples);
			if(_volumeCounter > 32768.0f) _volumeCounter = 32768.0f;
			int temp((f2i(fast_log2(_volumeCounter) * 78.0f * 4 / 14.0f) - (78 * 3)));// not 100% accurate, but looks as it sounds
			// prevent downward jerkiness
			if(temp > 97) temp = 97;
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

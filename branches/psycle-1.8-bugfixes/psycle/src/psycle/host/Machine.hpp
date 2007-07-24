///\file
///\brief interface file for psycle::host::Machine
#pragma once
#include "SongStructs.hpp"
#include "Dsp.hpp"
#include "Helpers.hpp"
#include "Constants.hpp"
#include "FileIO.hpp"
#include "configuration_options.hpp"
#include <universalis/exception.hpp>
#include <universalis/compiler/location.hpp>
#if !defined PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
	#error PSYCLE__CONFIGURATION__FPU_EXCEPTIONS isn't defined! Check the code where this error is triggered.
#elif PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
	#include <universalis/processor/exceptions/fpu.hpp>
#endif
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
				/// Exception caused by a bad returned value from a library function.
				class bad_returned_value : public function_error
				{
					public:
						bad_returned_value(std::string const & what) : function_error(what) {}
				};

				///\internal
				namespace detail
				{
					class rethrow_functor
					{
						public:
							rethrow_functor(Machine & machine) : machine_(machine) {}
							template<typename E> void operator_                (universalis::compiler::location const & location,              E const * const e = 0) const throw(function_error) { rethrow(location, e, 0); }
							template<          > void operator_<std::exception>(universalis::compiler::location const & location, std::exception const * const e    ) const throw(function_error) { rethrow(location, e, e); }
						private:
							template<typename E> void rethrow                  (universalis::compiler::location const & location,              E const * const e, std::exception const * const standard) const throw(function_error)
							{
								std::ostringstream s;
								s
									<< "Machine had an exception in function '" << location << "'." << std::endl
									<< universalis::compiler::typenameof(*e) << std::endl
									<< universalis::exceptions::string(*e);
								function_error const function_error(s.str(), standard);
								machine_.crashed(function_error);
								throw function_error;
							}
							Machine & machine_;
					};
				}

				/// This macro is to be used in place of a catch statement
				/// to catch an exception of any type thrown by a machine.
				/// It performs the following operations:
				/// - It catches everything.
				/// - It marks the machine as crashed.
				/// - It converts the exception to a std::exception (if needed).
				/// - It throws the converted exception.
				/// The usage is:
				/// - for the proxy between the host and a machine:
				///     try { some_machine.do_something(); } PSYCLE__HOST__CATCH_ALL(some_machine)
				/// - for the host:
				///     try { machine_proxy.do_something(); } catch(std::exception) { /* don't rethrow the exception */ }
				#define PSYCLE__HOST__CATCH_ALL(machine) \
					UNIVERSALIS__EXCEPTIONS__CATCH_ALL_AND_CONVERT_TO_STANDARD_AND_RETHROW__WITH_FUNCTOR(psycle::host::exceptions::function_errors::detail::rethrow_functor(machine))
				//	UNIVERSALIS__EXCEPTIONS__CATCH_ALL_AND_CONVERT_TO_STANDARD_AND_RETHROW__WITH_FUNCTOR(boost::bind(&Machine::on_crash, &machine, _1, _2, _3))
			}
		}

		/// Class storing the parameter description of Internal Machines.
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

		/// Base class for "Machines", the audio producing elements.
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
			///\}
			///\name crash handling ... fpu exception mask
			///\{
				#if !defined PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
					#error PSYCLE__CONFIGURATION__FPU_EXCEPTIONS isn't defined! Check the code where this error is triggered.
				#elif PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
					public:
						universalis::processor::exceptions::fpu::mask::type const inline & fpu_exception_mask() const throw() { return fpu_exception_mask_; }
						universalis::processor::exceptions::fpu::mask::type       inline & fpu_exception_mask()       throw() { return fpu_exception_mask_; }
					private:
						universalis::processor::fpu::exceptions::mask::type                fpu_exception_mask_;
				#endif
			///\}

#if 0 // v1.9
			///\name cpu cost measurement ... for the time spent in the machine's processing function
			///\{
				public:
					void             inline work_cpu_cost(cpu::cycles_type const & value)       throw() { work_cpu_cost_ = value; }
					cpu::cycles_type inline work_cpu_cost(                              ) const throw() { return work_cpu_cost_; }
				private:
					cpu::cycles_type        work_cpu_cost_;
			///\}
			///\name cpu cost measurement ... for the time spent routing audio
			///\{
				public:
					void             inline wire_cpu_cost(cpu::cycles_type const & value)       throw() { wire_cpu_cost_ = value; }
					cpu::cycles_type inline wire_cpu_cost(                              ) const throw() { return wire_cpu_cost_; }
				private:
					cpu::cycles_type        wire_cpu_cost_;
			///\}
#else
			public:///\todo private
				std::uint64_t _cpuCost;
				std::uint64_t _wireCost;
#endif

#if 0 // v1.9
			///\name each machine has a type attribute so that we can make yummy switch statements
			///\{
				public:
					///\see enum MachineClass which defined somewhere outside
					typedef MachineClass class_type;
					Machine::class_type inline subclass() const throw() { return _subclass; }
				public:///\todo private:
					class_type _subclass;
			///\}

			///\name each machine has a mode attribute so that we can make yummy switch statements
			///\{
				public:
					///\see enum MachineMode which is defined somewhere outside
					typedef MachineMode mode_type;
					mode_type inline mode() const throw() { return _mode; }
				public:///\todo private:
					mode_type _mode;
			///\}

			///\name machine's numeric identifier used in the patterns and gui display
			///\{
				public:
					/// legacy
					///\todo should be unsigned but some functions return negative values to signal errors instead of throwing an exception
					//PSYCLE__STRONG_TYPEDEF(int, id_type);
					typedef int id_type;
					id_type id() const throw() { return _macIndex; }
				public:///\todo private:
					/// it's actually used as an array index, but that shouldn't be part of the interface
					id_type _macIndex;
			///\}
#else
			public:///\todo private
				int _macIndex;
				MachineType _type;
				MachineMode _mode;
#endif

			///\name the life cycle of a mahine
			///\{
				public:
					virtual void Init();
					virtual void PreWork(int numSamples,bool clear=true);
					virtual void Work(int numSamples);
					virtual void WorkNoMix(int numSamples);
					virtual void Tick() {}
					virtual void Tick(int track, PatternEntry * pData) {}
					virtual void Stop() {}
			///\}

			///\name (de)serialization
			///\{
				public:
					virtual bool Load(RiffFile * pFile);
					static Machine * LoadFileChunk(RiffFile* pFile, int index, int version,bool fullopen=true);
					virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
					virtual void SaveDllNameAndIndex(RiffFile * pFile,int index);
					virtual void SaveFileChunk(RiffFile * pFile);
					virtual void SaveSpecificChunk(RiffFile * pFile);
			///\}

			///\name connections ... wires
			///\{
				public:
					// Set or replace output wire
					virtual void InsertOutputWireIndex(int wireIndex,int dstmac);
					// Set or replace input wire
					virtual void InsertInputWireIndex(int wireIndex,int srcmac,float wiremultiplier,float initialvol=1.0f);
//					virtual void InitWireVolume(MachineType mType,int wireIndex,float value);
					virtual void ExchangeInputWires(int first,int second);
					virtual void ExchangeOutputWires(int first,int second);
					virtual void DeleteOutputWireIndex(int wireIndex);
					virtual void DeleteInputWireIndex(int wireIndex);
					virtual void DeleteWires();
					virtual int FindInputWire(int macIndex);
					virtual int FindOutputWire(int macIndex);
					virtual int GetFreeInputWire(int slottype=0);
					virtual int GetFreeOutputWire(int slottype=0);
					virtual int GetInputSlotTypes() { return 1; }
					virtual int GetOutputSlotTypes() { return 1; }
					virtual float GetAudioRange()=0;

			///\}

			///\name amplification of the signal in connections/wires
			///\{
				public:
					virtual void GetWireVolume(int wireIndex, float &value) { value = GetWireVolume(wireIndex); }
					virtual float GetWireVolume(int wireIndex) { return _inputConVol[wireIndex] * _wireMultiplier[wireIndex]; }
					virtual void SetWireVolume(int wireIndex,float value) { _inputConVol[wireIndex] = value / _wireMultiplier[wireIndex]; };
					virtual bool GetDestWireVolume(int srcIndex, int WireIndex,float &value);
					virtual bool SetDestWireVolume(int srcIndex, int WireIndex,float value);
			///\}

			///\name general information
			///\{
				public:
					///\todo in v1.9 there was virtual void SetEditName(std::string const & newname) { _editName = newname; }
					const char * GetEditName() { return _editName; }
				public:///\todo private:
					///\todo this was a std::string in v1.9
					char _editName[32];

				public:
					virtual char * GetName() = 0;
					virtual const char * const GetDllName() const throw() { return ""; }
					virtual int GetPluginCategory() { return 0; }
					virtual bool IsShellMaster() { return false; }
					virtual int GetShellIdx() { return 0; }
			///\}

			///\name parameters
			///\{
				public:
					virtual int GetNumCols() { return _nCols; }
					virtual int GetNumParams() { return _numPars; }
					virtual void GetParamName(int numparam, char * name) { name[0]='\0'; }
					virtual void GetParamRange(int numparam, int &minval, int &maxval) {minval=0; maxval=0; }
					virtual void GetParamValue(int numparam, char * parval) { parval[0]='\0'; }
					virtual int GetParamValue(int numparam) { return 0; }
					virtual bool SetParameter(int numparam, int value) { return false; }
				public:///\todo private:
					int _numPars;
					int _nCols;
			///\}

			///\name gui stuff
			///\{
				public:
					virtual int  GetPosX() { return _x; };
					virtual void SetPosX(int x) {_x = x;};
					virtual int  GetPosY() { return _y; };
					virtual void SetPosY(int y) {_y = y;};
				public:///\todo private:
					int _x;
					int _y;
			///\}

			///\name states
			///\{
				public:
					virtual bool Bypass() { return _bypass; }
					virtual void Bypass(bool e) { _bypass = e; }
				public:///\todo private:
					bool _bypass;

				public:
					virtual bool Standby() { return _standby; }
					virtual void Standby(bool e) { _standby = e; }
				public:///\todo private:
					bool _standby;

				public:
					bool Mute() { return _mute; }
					void Mute(bool e) { _mute = e; }
				public:///\todo private:
					bool _mute;
			///\}

			///\name panning
			///\{
				public:
					///\todo int GetPan() { return _panning; }
					///\todo 3 dimensional?
					virtual void SetPan(int newpan);
				public:///\todo private:
					/// numerical value of panning
					int _panning;							
					/// left chan volume
					float _lVol;							
					/// right chan volume
					float _rVol;							
			///\}

		public:
			Machine();
			Machine(MachineType msubclass, MachineMode mode, int id);
			virtual ~Machine() throw();
			virtual void SetSampleRate(int sr) {}
		protected:
			void SetVolumeCounter(int numSamples);
			//void SetVolumeCounterAccurate(int numSamples);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//\todo below are unencapsulated data members

		public://\todo private:

			///\name gui stuff
			///\{
				/// The topleft point of a square where the wire triangle is centered when drawn. (Used to detect when to open the wire dialog)
				///\todo hardcoded limits and wastes with MAX_CONNECTIONS
				CPoint _connectionPoint[MAX_CONNECTIONS];
			///\}

			///\name signal measurements (and also gui stuff)
			///\{
				/// output peak level for DSP
				float _volumeCounter;					
				/// output peak level for display
				int _volumeDisplay;	
				/// output peak level for display
				int _volumeMaxDisplay;
				/// output peak level for display
				int _volumeMaxCounterLife;
				///\todo doc
				int _scopePrevNumSamples;
				///\todo doc
				int	_scopeBufferIndex;
				///\todo doc
				float *_pScopeBufferL;
				///\todo doc
				float *_pScopeBufferR;
			///\}

			///\name misc
			///\{
				/// machine has started its work call, and is waiting for inputs to finish generating sound.
				bool _waitingForSound;
				/// machine has finished working, and samples are ready in the buffers until next work call.
				bool _worked;
				/// left data
				float *_pSamplesL;
				/// right data
				float *_pSamplesR;						
			///\}

			///\name various player-related states
			///\todo hardcoded limits and wastes with MAX_TRACKS
			///\{
				///\todo doc
				PatternEntry TriggerDelay[MAX_TRACKS];
				///\todo doc
				int TriggerDelayCounter[MAX_TRACKS];
				///\todo doc
				int RetriggerRate[MAX_TRACKS];
				///\todo doc
				int ArpeggioCount[MAX_TRACKS];
				///\todo doc
				bool TWSActive;
				///\todo doc
				int TWSInst[MAX_TWS];
				///\todo doc
				int TWSSamples;
				///\todo doc
				float TWSDelta[MAX_TWS];
				///\todo doc
				float TWSCurrent[MAX_TWS];
				///\todo doc
				float TWSDestination[MAX_TWS];
			///\}

			///\name input ports
			///\{
				/// number of Incoming connections
				int _numInputs;							
				/// Incoming connections Machine number
				int _inputMachines[MAX_CONNECTIONS];	
				/// Incoming connections activated
				bool _inputCon[MAX_CONNECTIONS];		
				/// Incoming connections Machine vol
				float _inputConVol[MAX_CONNECTIONS];	
				/// Value to multiply _inputConVol[] with to have a 0.0...1.0 range
				// The reason of the _wireMultiplier variable is because VSTs output wave data
				// in the range -1.0 to +1.0, while natives and internals output at -32768.0 to +32768.0
				// Initially (when the format was made), Psycle did convert this in the "Work" function,
				// but since it already needs to multiply the output by inputConVol, I decided to remove
				// that extra conversion and use directly the volume to do so.
				float _wireMultiplier[MAX_CONNECTIONS];	
			///\}

			///\name output ports
			///\{
				/// number of Outgoing connections
				int _numOutputs;						
				/// Outgoing connections Machine number
				int _outputMachines[MAX_CONNECTIONS];	
				/// Outgoing connections activated
				bool _connection[MAX_CONNECTIONS];      
			///\}
		};

		inline void Machine::SetVolumeCounter(int numSamples)
		{
			_volumeCounter = dsp::GetMaxVol(_pSamplesL, _pSamplesR, numSamples);
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

		/// master machine.
		class Master : public Machine
		{
		public:
			Master();
			Master(int index);
			virtual void Init(void);
			virtual void Work(int numSamples);
			virtual float GetAudioRange(){ return 32768.0f; }
			virtual char* GetName(void) { return _psName; };
			virtual bool Load(RiffFile * pFile);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile);

			int _outDry;
			bool vuupdated;
			bool _clip;
			bool decreaseOnClip;
			int peaktime;
			float currentpeak;
			float _lMax;
			float _rMax;
			static float* _pMasterSamples;
			/// this is for the VstHost
			double sampleCount;
		protected:
			static char* _psName;
		};

	}
}

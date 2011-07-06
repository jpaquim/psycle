///\file
///\brief interface file for psycle::host::Machine
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Global.hpp"
#include <psycle/helpers/dsp.hpp>
#include "FileIO.hpp"
#include "cpu_time_clock.hpp"
#include <universalis/exception.hpp>
#include <universalis/compiler/location.hpp>
#include <universalis/stdlib/chrono.hpp>
#include <universalis/os/loggers.hpp>
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
					/// Crashable type concept requirement: it must have a member function void crashed(std::exception const &) throw();
					template<typename Crashable>
					class rethrow_functor
					{
						public:
							rethrow_functor(Crashable & crashable) : crashable_(crashable) {}
							template<typename E> void operator_                (universalis::compiler::location const & location,              E const * const e = 0) const throw(function_error) { rethrow(location, e, 0); }
							template<          > void operator_<std::exception>(universalis::compiler::location const & location, std::exception const * const e    ) const throw(function_error) { rethrow(location, e, e); }
						private:
							template<typename E> void rethrow                  (universalis::compiler::location const & location,              E const * const e, std::exception const * const standard) const throw(function_error)
							{
								std::ostringstream s;
								s
									<< "An exception occured in"
									<< " module: " << location.module()
									<< ", function: " << location.function()
									<< ", file: " << location.file() << ':' << location.line()
									<< '\n';
								if(e) {
									s
										<< "exception type: " << universalis::compiler::typenameof(*e) << '\n'
										<< universalis::exceptions::string(*e);
								} else {
									s << universalis::compiler::exceptions::ellipsis_desc();
								}
								function_error const f_error(s.str(), standard);
								crashable_.crashed(f_error);
								throw f_error;
							}
							Crashable & crashable_;
					};

					template<typename Crashable>
					rethrow_functor<Crashable> make_rethrow_functor(Crashable & crashable)
					{
						return rethrow_functor<Crashable>(crashable);
					}
				}

				/// This macro is to be used in place of a catch statement
				/// to catch an exception of any type thrown by a machine.
				/// It performs the following operations:
				/// - It catches everything.
				/// - It converts the exception to a std::exception (if needed).
				/// - It marks the machine as crashed by calling the machine's member function void crashed(std::exception const &) throw();
				/// - It throws the converted exception.
				/// The usage is:
				/// - for the proxy between the host and a machine:
				///     try { some_machine.do_something(); } PSYCLE__HOST__CATCH_ALL(some_machine)
				/// - for the host:
				///     try { machine_proxy.do_something(); } catch(std::exception) { /* don't rethrow the exception */ }
				///
				/// Note that the crashable argument can be of any type as long as it has a member function void crashed(std::exception const &) throw();
//				#define PSYCLE__HOST__CATCH_ALL(crashable) \
//					UNIVERSALIS__EXCEPTIONS__CATCH_ALL_AND_CONVERT_TO_STANDARD_AND_RETHROW__WITH_FUNCTOR(psycle::host::exceptions::function_errors::detail::make_rethrow_functor(crashable))

				///\see PSYCLE__HOST__CATCH_ALL
//				#define PSYCLE__HOST__CATCH_ALL__NO_CLASS(crashable) \
//					UNIVERSALIS__EXCEPTIONS__CATCH_ALL_AND_CONVERT_TO_STANDARD_AND_RETHROW__WITH_FUNCTOR__NO_CLASS(psycle::host::exceptions::function_errors::detail::make_rethrow_functor(crashable))


				#define PSYCLE__HOST__CATCH_ALL(crashable) catch(bool) {throw;}
					//UNIVERSALIS__EXCEPTIONS__CATCH_ALL_AND_CONVERT_TO_STANDARD_AND_RETHROW
				#define PSYCLE__HOST__CATCH_ALL__NO_CLASS(crashable) catch(bool){throw;}
					//UNIVERSALIS__EXCEPTIONS__CATCH_ALL_AND_CONVERT_TO_STANDARD_AND_RETHROW__NO_CLASS
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

			///\name cpu time usage measurement
			///\{
				public: void reset_time_measurement() throw() { accumulated_processing_time_ = 0; processing_count_ = 0; }

				public:  cpu_time_clock::duration accumulated_processing_time() const throw() { return accumulated_processing_time_; }
				private: cpu_time_clock::duration accumulated_processing_time_;
				protected: void accumulate_processing_time(std::chrono::nanoseconds d) throw() {
						if(loggers::warning() && d.count() < 0) {
							std::ostringstream s;
							s << "time went backward by: " << std::chrono::nanoseconds(d).count() * 1e-9 << 's';
							loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						} else accumulated_processing_time_ += d;
					}

				public:    std::uint64_t processing_count() const throw() { return processing_count_; }
				protected: std::uint64_t processing_count_;
			///\}

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
				static bool autoStopMachine;
				int _macIndex;
				MachineType _type;
				MachineMode _mode;
#endif

			///\name the life cycle of a mahine
			///\{
				public:
					virtual void Init();
					virtual void PreWork(int numSamples,bool clear, bool measure_cpu_usage);
					virtual int GenerateAudio(int numsamples, bool measure_cpu_usage);
					virtual int GenerateAudioInTicks(int startSample, int numsamples);
					virtual void Tick() {}
					virtual void Tick(int track, PatternEntry * pData) {}
					virtual void Stop() {}
			///\}
			///\name used by the single-threaded, recursive scheduler
			///\{
					/// virtual because the mixer machine has its own implementation
					virtual void recursive_process(unsigned int frames, bool measure_cpu_usage);
					void recursive_process_deps(unsigned int frames, bool mix, bool measure_cpu_usage);
			///\}
			///\name used by the multi-threaded scheduler
			///\{
					typedef std::list<Machine const*> sched_deps;

					/// tells the scheduler which machines to process before this one
					virtual void sched_inputs(sched_deps&) const;
					/// tells the scheduler which machines may be processed after this one
					virtual void sched_outputs(sched_deps&) const;
					/// called by the scheduler to ask for the actual processing of the machine
					virtual bool sched_process(unsigned int frames, bool measure_cpu_usage);
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
					virtual void PostLoad(){};
			///\}

			///\name connections ... wires
			///\{
				public:
					// Set or replace output wire
					virtual void InsertOutputWireIndex(Song* pSong,int wireIndex,int dstmac);
					// Set or replace input wire
					virtual void InsertInputWireIndex(Song* pSong,int wireIndex,int srcmac,float wiremultiplier,float initialvol=1.0f);
					virtual void ExchangeInputWires(int first,int second);
					virtual void ExchangeOutputWires(int first,int second);
					virtual void NotifyNewSendtoMixer(Song* pSong,int callerMac,int senderMac);
					virtual void ClearMixerSendFlag(Song* pSong);
					virtual void DeleteOutputWireIndex(Song* pSong,int wireIndex);
					virtual void DeleteInputWireIndex(Song* pSong,int wireIndex);
					virtual void DeleteWires(Song *pSong);
					virtual int FindInputWire(int macIndex);
					virtual int FindOutputWire(int macIndex);
					virtual int GetFreeInputWire(int slottype=0);
					virtual int GetFreeOutputWire(int slottype=0);
					virtual int GetInputSlotTypes() { return 1; }
					virtual int GetOutputSlotTypes() { return 1; }
					virtual float GetAudioRange() { return 1.0f; }

			///\}

			///\name amplification of the signal in connections/wires
			///\{
				public:
					virtual void GetWireVolume(int wireIndex, float &value) { value = GetWireVolume(wireIndex); }
					virtual float GetWireVolume(int wireIndex) { return _inputConVol[wireIndex] * _wireMultiplier[wireIndex]; }
					virtual void SetWireVolume(int wireIndex,float value) { _inputConVol[wireIndex] = value / _wireMultiplier[wireIndex]; }
					virtual bool GetDestWireVolume(Song* pSong,int srcIndex, int WireIndex,float &value);
					virtual bool SetDestWireVolume(Song* pSong,int srcIndex, int WireIndex,float value);
			///\}

			///\name general information
			///\{
				public:
					///\todo: update this to std::string.
					virtual void SetEditName(std::string const & newname) { std::strncpy(_editName,newname.c_str(),32); }
					const char * GetEditName() const { return _editName; }
				public:///\todo private:
					///\todo this was a std::string in v1.9
					char _editName[32];

				public:
					virtual char * GetName() = 0;
					virtual const char * const GetDllName() const throw() { return ""; }
					virtual int GetPluginCategory() { return 0; }
					virtual bool IsShellMaster() { return false; }
					virtual int GetShellIdx() { return 0; }
					virtual bool NeedsAuxColumn() { return false; }
					virtual const char* AuxColumnName(int idx) {return ""; }
					virtual int NumAuxColumnIndexes() { return 0;}
			///\}

			///\name parameters
			///\{
				public:
					virtual int GetNumCols() { return _nCols; }
					virtual int GetNumParams() { return _numPars; }
					virtual int GetParamType(int numparam) { return 0; }
					virtual void GetParamName(int numparam, char * name) { name[0]='\0'; }
					virtual void GetParamRange(int numparam, int &minval, int &maxval) {minval=0; maxval=0; }
					virtual void GetParamValue(int numparam, char * parval) { parval[0]='\0'; }
					virtual int GetParamValue(int numparam) { return 0; }
					virtual bool SetParameter(int numparam, int value) { return false; }
					virtual void SetCurrentProgram(int idx) {};
					virtual int GetCurrentProgram() {return 0;};
					virtual void GetCurrentProgramName(char* val) {strcpy(val,"Program 0");};
					virtual void GetIndexProgramName(int bnkidx, int prgIdx, char* val){strcpy(val,"Program 0");};
					virtual int GetNumPrograms(){ return 1;}; // total programs of the bank.
					virtual int GetTotalPrograms(){ return 1;}; //total programs independently of the bank
					virtual void SetCurrentBank(int idx) {};
					virtual int GetCurrentBank() {return 0;};
					virtual void GetCurrentBankName(char* val) {strcpy(val,"Internal");};
					virtual void GetIndexBankName(int bnkidx, char* val){strcpy(val,"Internal");};
					virtual int GetNumBanks(){ return 1;};
				public:///\todo private:
					int _numPars;
					int _nCols;
			///\}

			///\name gui stuff
			///\{
				public:
					virtual int  GetPosX() { return _x; }
					virtual void SetPosX(int x) {_x = x;}
					virtual int  GetPosY() { return _y; }
					virtual void SetPosY(int y) {_y = y;}
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
			Machine(Machine* mac);
			Machine(MachineType msubclass, MachineMode mode, int id);
			virtual ~Machine() throw();
			virtual void SetSampleRate(int sr)
			{
#if PSYCLE__CONFIGURATION__RMS_VUS
				rms.count=0;
				rms.AccumLeft=0.;
				rms.AccumRight=0.;
				rms.previousLeft=0.;
				rms.previousRight=0.;
#endif
			}
		protected:
			void UpdateVuAndStanbyFlag(int numSamples);

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
#if PSYCLE__CONFIGURATION__RMS_VUS
				helpers::dsp::RMSData rms;
#endif
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
				/// this machine is used by a send/return mixer. (Some things cannot be done on these machines)
				bool _isMixerSend;
				/// left data
				float *_pSamplesL;
				/// right data
				float *_pSamplesR;						
			///\}
			///\name used by the single-threaded, recursive scheduler
			///\{
				/// guard to avoid feedback loops
				bool recursive_is_processing_;
				bool recursive_processed_;
			///\}

			///\name used by the multi-threaded scheduler
			///\{
				/// The multi-threaded scheduler cannot use _worked because it's not thread-synchronised.
				/// So, we define another boolean that's modified only by the multi-threaded scheduler,
				/// with proper thread synchronisations.
				/// The multi-threaded scheduler doesn't use recursive_processed_ nor recursive_is_processing_.
				bool sched_processed_;
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

		/// master machine.
		class Master : public Machine
		{
		public:
			Master();
			Master(int index);
			virtual void Init(void);
			virtual int GenerateAudio(int numsamples, bool measure_cpu_usage);
			virtual float GetAudioRange(){ return 32768.0f; }
			virtual char* GetName(void) { return _psName; }
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
		protected:
			static char* _psName;
		};
	}
}

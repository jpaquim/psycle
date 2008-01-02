///\interface psy::core::Machine
#pragma once

#include "constants.h"
#include "commands.h"
#include "cstdint.h"
//#include "dsp.h"
//#include "fileio.h"
#include "helpers.h"
#include "patternevent.h"
#include "playertimeinfo.h"

#include <cassert>
#include <deque>
#include <map>
#include <stdexcept>

namespace psy
{
	namespace core
	{
		class RiffFile;

		// FIXME: stole these from analzyer.h just to fix compile error.
		const int MAX_SCOPE_BANDS = 128;
		const int SCOPE_BUF_SIZE  = 4096;
		const int SCOPE_SPEC_SAMPLES = 256;

		//cpu::cycles_type cost(cpu::cycles());
		//cost = cpu::cycles() - cost;

		class Machine; // forward declaration
		class CoreSong; // forward declaration

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
					function_error(std::string const & what, std::exception const * const exception = 0) : core::exception(what), exception_(exception) {}
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
			}

			#if 0
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
				#define PSYCLE__HOST__CATCH_ALL(machine) \
					UNIVERSALIS__EXCEPTIONS__CATCH_ALL_AND_CONVERT_TO_STANDARD_AND_RETHROW__WITH_FUNCTOR(psy::core::exceptions::function_errors::detail::rethrow_functor(machine))
					//UNIVERSALIS__EXCEPTIONS__CATCH_ALL_AND_CONVERT_TO_STANDARD_AND_RETHROW__WITH_FUNCTOR(boost::bind(&Machine::on_crash, &machine, _1, _2, _3))
			#endif
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

		class AudioPort;

		// A wire is what interconnects two AudioPorts. Appart from being the graphically representable element,
		// the wire is also responsible of volume changes and even pin reassignation (convert 5.1 to stereo, etc.. not yet)
		class Wire
		{
			public:
				/// legacy
		typedef std::int32_t id_type;

				Wire()
					:volume(1.0f),pan(0.0f),multiplier(1.0f),rvol(1.0f),lvol(1.0f)
					,index(0),senderport(0),receiverport(0){}
				virtual ~Wire()
				{
					if (senderport) Disconnect(senderport);
					if (receiverport) Disconnect(receiverport);
				}
				virtual void Connect(AudioPort *senderp,AudioPort *receiverp);
				virtual void ChangeSource(AudioPort* newsource);
				virtual void ChangeDestination(AudioPort* newdest);
				virtual void CollectData(int numSamples);
				virtual void SetVolume(float newvol);
				virtual void SetPan(float newpan);
				virtual inline int GetIndex() { return index; }
				virtual inline void SetIndex(int idx) { index = idx; }
				
			protected:
				virtual void Disconnect(AudioPort* port);
				virtual inline float RVol() { return rvol; }
				virtual inline float LVol() { return lvol; }
				float volume;
				float pan;
				float multiplier;
				float rvol;
				float lvol;
				int index;
				AudioPort *senderport;
				AudioPort *receiverport;
		};

		// Class which allows the setup (as in shape) of the machines' connectors.
		// An Audio port is synonym of a multiplexed channel. In other words, it is an element that defines
		// the characteristics of one or more individual inputs that are used in conjunction.
		// From this definition, we could have one Stereo Audio Port (one channel, two inputs or outputs),
		// a 5.1 Port, or several Stereo Ports (in the case of a mixer table), between others..
		// Note that several wires can be connected to the same AudioPort (automatic mixing in the case of an input port?).
		class AudioPort
		{
			public:
				///\todo: Port creation, assign buffers to it (passed via ctor? they might be shared/pooled). 
				///\todo: Also, Multiple buffers or a packed buffer (left/right/left/right...)?
				AudioPort(Machine & parent, int arrangement, std::string const & name) : parent_(parent),  arrangement_(arrangement), name_(name) {}
				virtual ~AudioPort() {}
				virtual void CollectData(int /*numSamples*/) {}
				virtual void Connected(Wire * wire);
				virtual void Disconnected(Wire * wire);
				virtual inline Wire* GetWire(unsigned int index) { assert(index<wires_.size()); return wires_[index]; }
				virtual inline int NumberOfWires() { return static_cast<int>(wires_.size()); }
				virtual inline int Arrangement() throw() { return arrangement_; }
				virtual inline Machine * GetMachine() throw() { return &parent_; }
				///\todo : should change arrangement/name be allowed? (Mutating Port?)
				virtual inline void ChangeArrangement(int arrangement) { this->arrangement_ = arrangement; }
				virtual inline std::string const & Name() const throw() { return name_; }
				virtual inline void ChangeName(std::string const & name) { this->name_ = name; }
			protected:
				Machine & parent_;
				int arrangement_;
				std::string name_;
				typedef std::vector<Wire*> wires_type;
				wires_type wires_;
		};

		class InPort : public AudioPort
		{
			public:
				/// legacy
		typedef std::int32_t id_type;
				InPort(Machine & parent, int arrangement, std::string const & name) : AudioPort(parent, arrangement, name) {}
				virtual ~InPort(){};
				virtual void CollectData(int numSamples);
		};

		class OutPort : public AudioPort
		{
			public:
				/// legacy
		typedef std::int32_t id_type;
				OutPort(Machine & parent, int arrangement, std::string const & name) : AudioPort(parent, arrangement, name) {}
				virtual ~OutPort() {}
				virtual void CollectData(int numSamples);
		};

		enum MachineType
		{
			MACH_UNDEFINED = -1, //< :-(
			MACH_MASTER = 0,
			MACH_SINE = 1, //< for psycle old fileformat version 2
			MACH_DIST = 2, //< for psycle old fileformat version 2
			MACH_SAMPLER = 3,
			MACH_DELAY = 4, //< for psycle old fileformat version 2
			MACH_2PFILTER = 5, //< for psycle old fileformat version 2
			MACH_GAIN = 6, //< for psycle old fileformat version 2
			MACH_FLANGER = 7, //< for psycle old fileformat version 2
			MACH_PLUGIN = 8,
			MACH_VST = 9,
			MACH_VSTFX = 10,
			MACH_SCOPE = 11,
			MACH_XMSAMPLER = 12,
			MACH_DUPLICATOR = 13,
			MACH_MIXER = 14,
			MACH_LFO = 15,
			MACH_LADSPA = 16,
			MACH_DUMMY = 255
		};

		enum MachineMode
		{
			MACHMODE_UNDEFINED = -1, //< :-(
			MACHMODE_GENERATOR = 0,
			MACHMODE_FX = 1,
			MACHMODE_MASTER = 2,
		};

		class WorkEvent {
			public:
				WorkEvent();
				WorkEvent(double beatOffset, int track, const PatternEvent & patternEvent);
				const PatternEvent &  event() const;
				double beatOffset() const;
				int track() const;
				void changeposition(double beatOffset) { offset_ = beatOffset; }

			private:
				
				double offset_;
				int track_;
				PatternEvent event_;

		};

		class MachineCallbacks {
			public:
				virtual PlayerTimeInfo & timeInfo()  = 0;
				virtual bool autoStopMachines() const = 0;
				virtual ~MachineCallbacks() {}
		};

		/// Base class for "Machines", the audio producing elements.
		class Machine
		{

			///\name crash handling
			///\{
				public:
					virtual int GenerateAudioInTicks(int startSample, int numsamples );
					virtual int GenerateAudio(int numsamples );
					virtual void AddEvent(double offset, int track, const PatternEvent & event);
					virtual void reallocateRemainingEvents(double beatOffset);

				protected:
					std::deque<WorkEvent> workEvents;
					std::map<int,int> playCol;
					int playColIndex;

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
				#if 0 ///\todo re-enable this
					public:
						universalis::processor::exceptions::fpu::mask::type const inline & fpu_exception_mask() const throw() { return fpu_exception_mask_; }
						universalis::processor::exceptions::fpu::mask::type       inline & fpu_exception_mask()       throw() { return fpu_exception_mask_; }
					private:
						universalis::processor::exceptions::fpu::mask::type                fpu_exception_mask_;
				#endif
			///\}

			#if 0
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
						cpu::cycles_type        wire_cpu_cost_;*/
				///\}
			#endif

			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			// Draft for a new Machine Specification.
			// A machine is created via pMachine = new Machine;
			// Creation does not give a ready to use Machine. "The machine is over the table, but the power is off!"
			// Use LoadDll(std::string) to load a specific plugin to operate this machine. The function does a loadlibrary, and
			// the basic information becomes accessible (name, parameters...). Note that this call will only return "true" 
			// [ there is another option, which would be a constructor that gets the std::string, and LoadDll() be protected and called from within ]
			// for Plugin or vst::plugin, since any other machine do not use an external dll.
			// Use UnloadDll() to undo the previous action. Else, the destructor will do it for you.
			// Use SwitchOn() to obtain a ready-to-use machine. This will return false for Plugin and vst::plugin if
			// OpenDll() has not been called, or if it has returned false.
			// Use Reset() to reinitialize the machine status and recall all default values for the parameters.
			// Use SwitchOff() to stop using this machine. Else, the destructor will do it for you.
			// Use StandBy(bool) to set or unset the machine to a stopped state. ("suspend" in vst terminology).
			// "Process()" will still be called in order for the machine to update state, but will return with no data
			// Everything else works as usual.
			// This function can be used as an "audio-only" reset. (Panic button)
			// Bypass(bool) un/sets the Bypass flag, and calls to StandBy() accordingly.
			// Process() Call it to start the processing of input buffers and generate the output.
			// AddEvent(timestampedEvent)
			// MasterChanged(changetype)
			// SaveState(ofstream)
			// LoadState(ifstream)
			// several "Get" for Information (name, params...)
			// several "Set" for Information (name, params...)
			// Automation... calling, or being called? ( calling automata.work() or automata calling machine.work())
			// Use the concept of "Ports" to define inputs/outputs.
			//
			// bool IsDllLoaded()
			// bool IsPowered()
			// bool IsBypass()
			// bool IsStandBy()
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////

			///\name each machine has a type attribute so that we can make yummy switch statements
			///\{
				public:
					///\see enum MachineType which defined somewhere outside
			typedef std::int32_t type_type; // Was: MachineType type_type
					type_type inline type() const throw() { return type_; }
				private:
					void type(type_type type) { type_ = type; } friend class CoreSong;
					type_type type_;
			///\}

			///\name each machine has a mode attribute so that we can make yummy switch statements
			///\{
				public:
					///\see enum MachineMode which is defined somewhere outside
					typedef MachineMode mode_type;
					mode_type inline mode() const throw() { return mode_; }
				private:
					void mode(mode_type mode) { mode_ = mode; } friend class Plugin;
					mode_type mode_;
			///\}

			///\name machine's numeric identifier used in the patterns and gui display
			///\{
				public:
					/// legacy
					///\todo should be unsigned but some functions return negative values to signal errors instead of throwing an exception
					typedef std::int32_t id_type;
					id_type id() const throw() { return id_; }
				private:
					/// it's currently actually used as an array index, but that shouldn't be part of the interface
					id_type id_;
					void id(id_type id) { id_ = id; } friend class Psy2Filter;
			///\}

			///\name ctor/dtor
			///\{
				public:
					Machine(MachineCallbacks* callbacks, type_type type, mode_type mode, id_type id, CoreSong * song);
					Machine(Machine *mac,type_type type, mode_type mode);
					virtual ~Machine();
			///\}
			
			protected:
				MachineCallbacks* callbacks;

			///\name song
			///\{
				public:
					/// the song this machine belongs to
					CoreSong const * const song() const { return song_; }
					CoreSong * const song() { return song_; }
				private:
					CoreSong* song_;
			///\}

			///\name the life cycle of a mahine
			///\{
				public:
					virtual void Init();
					virtual void PreWork(int numSamples);
					virtual void Work(int numSamples );
					virtual void WorkNoMix(int numSamples );
					virtual void Tick( ) {};
					virtual void Tick(int /*channel*/, const PatternEvent &) {}
					virtual void Stop() { playCol.clear(); playColIndex =0; }
			///\}

			///\name (de)serialization
			///\{
				public:
					virtual void SaveDllName(RiffFile * pFile) const;
					virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
					static Machine * LoadFileChunk(std::string const & plugin_path, CoreSong* pSong , RiffFile* pFile, MachineCallbacks* callbacks, Machine::id_type index, int version,bool fullopen=true);
					virtual void SaveFileChunk(RiffFile * pFile) const;
					virtual void SaveSpecificChunk(RiffFile * pFile) const;
					/// Loader for psycle fileformat version 2.
					virtual bool LoadPsy2FileFormat(std::string const & plugin_path, RiffFile* pFile);
				protected:
					// already friend class CoreSong;
			///\}

			///\name connections ... ports
			///\{
				public:
					virtual bool ConnectTo(Machine & dst, InPort::id_type dstport = InPort::id_type(0), OutPort::id_type outport = OutPort::id_type(0), float volume = 1.0f);
					virtual bool Disconnect(Machine & dst);
			///\}

			///\name connections ... wires
			///\{
				public:
					virtual void InitWireVolume(type_type, Wire::id_type, float value);
					virtual Wire::id_type FindInputWire(id_type);
					virtual Wire::id_type FindOutputWire(id_type);
					bool acceptsConnections();
					bool emitsConnections();
			///\}

			///\name multichannel
			///\{
				public:
					void DefineStereoInput(int numins);
					void DefineStereoOutput(int numouts);
			///\}

			//////////////////////////////////////////////////////////////////////////
			// Properties

			public:
				virtual void SetSampleRate(int /*hertz*/)
				{
#if defined PSYCLE__CONFIGURATION__RMS_VUS
					rms.count=0;
					rms.AccumLeft=0.;
					rms.AccumRight=0.;
					rms.previousLeft=0.;
					rms.previousRight=0.;
#endif
				};

				///\todo 3 dimensional?
				virtual void SetPan(int newpan);

			///\name ports
			///\{
				public:
					virtual unsigned int GetInPorts() { return numInPorts; };
					virtual unsigned int GetOutPorts() { return numOutPorts; };
					virtual AudioPort& GetInPort(InPort::id_type i) { assert(i<numInPorts); return inports[i]; };
					virtual AudioPort& GetOutPort(OutPort::id_type i) { assert(i<numOutPorts); return inports[i]; };
			///\}

			///\name audio range
			///\{
				public:
					///\todo doc
					virtual float GetAudioRange() { return audio_range_; }
				protected:
					void SetAudioRange(float audio_range) { audio_range_ = audio_range; }
				private:
					float audio_range_;
			///\}

			///\name amplification of the signal in connections/wires
			///\{
				public:
					virtual void GetWireVolume(Wire::id_type wire, float & result) { result = _inputConVol[wire] * _wireMultiplier[wire]; };
					virtual void SetWireVolume(Wire::id_type wire, float value) { _inputConVol[wire] = value / _wireMultiplier[wire]; };
					virtual bool GetDestWireVolume(id_type src, Wire::id_type, float & result);
					virtual bool SetDestWireVolume(id_type src, Wire::id_type, float value);
			///\}

			///\name name
			///\{
				public:
					virtual std::string GetDllName() const { return "built-in"; };
					virtual std::string GetName() const = 0;

				public:
					virtual std::string const & GetEditName() const { return editName_; }
					virtual void SetEditName(std::string const & editName) { editName_ = editName; }
				private:
					std::string  editName_;
			///\}

			///\name parameters
			///\{
				public:
					virtual int GetNumCols() const { return _nCols; }
					virtual int GetNumParams() const { return _numPars; }
					virtual void GetParamName(int /*numparam*/, char * name) const { name[0] = '\0'; }
					virtual void GetParamRange(int /*numparam*/, int & minval, int & maxval) const { minval = 0; maxval = 0; }
					virtual void GetParamValue(int /*numparam*/, char * parval) const { parval[0] = '\0'; }
					virtual int GetParamValue(int /*numparam*/) const { return 0; }
					virtual bool SetParameter(int /*numparam*/, int /*value*/) { return false; }
			///\}

			///\name more misplaced gui stuff
			///\{
				public:
					virtual int  GetPosX() { return _x; };
					virtual void SetPosX(int x) {_x = x;};
					virtual int  GetPosY() { return _y; };
					virtual void SetPosY(int y) {_y = y;};

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
		protected:
			void UpdateVuAndStanbyFlag(int numSamples);


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///\todo below are unencapsulated data members

		public:///\todo private:

			InPort *inports;
			OutPort *outports;
			int numInPorts;
			int numOutPorts;
			bool _waitingForSound;
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
		std::int32_t _panning;
			int _x;
			int _y;
			int _numPars;
			int _nCols;

			///\name input ports
			///\{
				/// number of Incoming connections
		std::int32_t _connectedInputs;
				/// Incoming connections Machine numbers
				///\todo hardcoded limits and wastes
				Machine::id_type _inputMachines[MAX_CONNECTIONS];
				/// Incoming connections activated
				///\todo hardcoded limits and wastes
				bool _inputCon[MAX_CONNECTIONS];
				/// Incoming connections Machine volumes
				///\todo hardcoded limits and wastes
				float _inputConVol[MAX_CONNECTIONS];
				/// Value to multiply _inputConVol[] to have a 0.0...1.0 range
				///\todo hardcoded limits and wastes
				float _wireMultiplier[MAX_CONNECTIONS];
			///\}

			///\name output ports
			///\{
				/// number of Outgoing connections
		std::int32_t _connectedOutputs;
				/// Outgoing connections Machine numbers
				///\todo hardcoded limits and wastes
				Machine::id_type _outputMachines[MAX_CONNECTIONS];
				/// Outgoing connections activated
				///\todo hardcoded limits and wastes
				bool _connection[MAX_CONNECTIONS];
			///\}

			///\name signal measurements, perhaps can be considered misplaced gui stuff
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
				int _scopeBufferIndex;
				///\todo doc
				float *_pScopeBufferL;
				///\todo doc
				float *_pScopeBufferR;
			///\}

			///\ various player-related states
			///\{
				///\todo hardcoded limits and wastes
				PatternEvent TriggerDelay[MAX_TRACKS];
				///\todo hardcoded limits and wastes
				int TriggerDelayCounter[MAX_TRACKS];
				///\todo hardcoded limits and wastes
				int RetriggerRate[MAX_TRACKS];
				///\todo hardcoded limits and wastes
				int ArpeggioCount[MAX_TRACKS];
				///\todo doc
				bool TWSActive;
				///\todo hardcoded limits and wastes
				int TWSInst[MAX_TWS];
				///\todo doc
				int TWSSamples;
				///\todo hardcoded limits and wastes
				float TWSDelta[MAX_TWS];
				///\todo hardcoded limits and wastes
				float TWSCurrent[MAX_TWS];
				///\todo hardcoded limits and wastes
				float TWSDestination[MAX_TWS];
			///\}
		};
	}
}

///\file
///\brief interface file for psycle::host::Machine
#pragma once
#include "SongStructs.hpp"
#include <psycle/helpers/dsp.hpp>
#include <psycle/helpers/helpers.hpp>
#include "constants.hpp"
#include <psycle/engine/global.hpp>
#include <universalis/processor/exceptions/fpu.hpp>
#include <universalis/exception.hpp>
#include <universalis/compiler/location.hpp>
#include <universalis/compiler/typenameof.hpp>
#include <stdexcept>
#include <cstdint>
namespace psycle
{
	namespace host
	{
		class Machine; // forward declaration
		class RiffFile;

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
				#define PSYCLE__HOST__CATCH_ALL(machine) \
					UNIVERSALIS__EXCEPTIONS__CATCH_ALL_AND_CONVERT_TO_STANDARD_AND_RETHROW__WITH_FUNCTOR(psycle::host::exceptions::function_errors::detail::rethrow_functor(machine))
				//	UNIVERSALIS__EXCEPTIONS__CATCH_ALL_AND_CONVERT_TO_STANDARD_AND_RETHROW__WITH_FUNCTOR(boost::bind(&Machine::on_crash, &machine, _1, _2, _3))
			}
		}

		class AudioPort;

		// A wire is what interconnects two AudioPorts. Appart from being the graphically representable element,
		// the wire is also responsible of volume changes and even pin reassignation (convert 5.1 to stereo, etc.. not yet)
		class Wire
		{
			public:
				/// legacy
				PSYCLE__STRONG_TYPEDEF(int, id_type);

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
				virtual inline int SetIndex(int idx) { index = idx; }
				
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
				//\todo: Port creation, assign buffers to it (passed via ctor? they might be shared/pooled). 
				//\todo: Also, Multiple buffers or a packed buffer (left/right/left/right...)?
				AudioPort(Machine & parent, int arrangement, std::string const & name) : parent_(parent), name_(name), arrangement_(arrangement) {}
				virtual ~AudioPort() {}
				virtual void CollectData(int numSamples) {}
				virtual void Connected(Wire * wire);
				virtual void Disconnected(Wire * wire);
				virtual inline Wire* GetWire(unsigned int index) { assert(index<wires_.size()); return wires_[index]; }
				virtual inline bool NumberOfWires() { return wires_.size(); }
				virtual inline int Arrangement() throw() { return arrangement_; }
				virtual inline Machine * GetMachine() throw() { return &parent_; }
				//\todo : should change arrangement/name be allowed? (Mutating Port?)
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
				PSYCLE__STRONG_TYPEDEF(int, id_type);
				InPort(Machine & parent, int arrangement, std::string const & name) : AudioPort(parent, arrangement, name) {}
				virtual ~InPort(){};
				virtual void CollectData(int numSamples);
		};

		class OutPort : public AudioPort
		{
			public:
				/// legacy
				PSYCLE__STRONG_TYPEDEF(int, id_type);
				OutPort(Machine & parent, int arrangement, std::string const & name) : AudioPort(parent, arrangement, name) {}
				virtual ~OutPort() {}
				virtual void CollectData(int numSamples);
		};

		enum MachineClass
		{
			MACH_UNDEFINED	= -1, //< :-(
			MACH_MASTER		= 0,
			MACH_SINE		= 1, //< for psycle old fileformat version 2
			MACH_DIST		= 2, //< for psycle old fileformat version 2
			MACH_SAMPLER	= 3,
			MACH_DELAY		= 4, //< for psycle old fileformat version 2
			MACH_2PFILTER	= 5, //< for psycle old fileformat version 2
			MACH_GAIN		= 6, //< for psycle old fileformat version 2
			MACH_FLANGER	= 7, //< for psycle old fileformat version 2
			MACH_PLUGIN		= 8,
			MACH_VST		= 9,
			MACH_VSTFX		= 10, //< Original host had a clear difference between VST gens and fx. Nowadays, they are almost the same.
			MACH_SCOPE		= 11, //< Deprecated machine (existed in some betas). It's a GUI element now. If encountered, load it as a Dummy.
			MACH_XMSAMPLER	= 12,
			MACH_DUPLICATOR	= 13,
			MACH_MIXER		= 14,
			MACH_LFO		= 15,
			MACH_AUTOMATOR	= 16,
			MACH_DUMMY		= 255
		};

		enum MachineMode
		{
			MACHMODE_UNDEFINED	= -1, //< :-(
			MACHMODE_GENERATOR	= 0,
			MACHMODE_FX			= 1,
			MACHMODE_MASTER		= 2,
		};

		/// Class for the Internal Machines' Parameters.
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

		class InternalMachineInfo;
		class InternalMachinePackage;

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
				public:
					universalis::processor::exceptions::fpu::mask::type const inline & fpu_exception_mask() const throw() { return fpu_exception_mask_; }
					universalis::processor::exceptions::fpu::mask::type       inline & fpu_exception_mask()       throw() { return fpu_exception_mask_; }
				private:
					universalis::processor::exceptions::fpu::mask::type                fpu_exception_mask_;
			///\}

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

			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			// Draft for a new Machine Specification.
			// A machine is created via pMachine = new Machine;
			// Creation does not give a ready to use Machine. "The machine is over the table, but the power is off!"
			// Use LoadDll(std::string) to load a specific plugin to operate this machine. The function does a loadlibrary, and
			//	the basic information becomes accessible (name, parameters...). Note that this call will only return "true" 
			// [ there is another option, which would be a constructor that gets the std::string, and LoadDll() be protected and called from within ]
			//	for Plugin or vst::plugin, since any other machine do not use an external dll.
			// Use UnloadDll() to undo the previous action. Else, the destructor will do it for you.
			// Use SwitchOn() to obtain a ready-to-use machine. This will return false for Plugin and vst::plugin if
			//	OpenDll() has not been called, or if it has returned false.
			// Use Reset() to reinitialize the machine status and recall all default values for the parameters.
			// Use SwitchOff() to stop using this machine. Else, the destructor will do it for you.
			// Use StandBy(bool) to set or unset the machine to a stopped state. ("suspend" in vst terminology).
			//	"Process()" will still be called in order for the machine to update state, but will return with no data
			//	Everything else works as usual.
			//	This function can be used as an "audio-only" reset. (Panic button)
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
					///\see enum MachineClass which defined somewhere outside
					typedef MachineClass class_type;
					Machine::class_type inline subclass() const throw() { return _subclass; }
				PSYCLE__PRIVATE:
					class_type _subclass;
			///\}

			///\name each machine has a mode attribute so that we can make yummy switch statements
			///\{
				public:
					///\see enum MachineMode which is defined somewhere outside
					typedef MachineMode mode_type;
					mode_type inline mode() const throw() { return _mode; }
				PSYCLE__PRIVATE:
					mode_type _mode;
			///\}

			///\name machine's numeric identifier used in the patterns and gui display
			///\{
				public:
					/// legacy
					///\todo should be unsigned but some functions return negative values to signal errors instead of throwing an exception
					PSYCLE__STRONG_TYPEDEF(int, id_type);
					id_type id() const throw() { return _macIndex; }
				PSYCLE__PRIVATE:
					/// it's currently actually used as an array index, but that shouldn't be part of the interface
					id_type _macIndex;
			///\}

			public:
				Machine(class_type msubclass, mode_type mode, id_type id);
				virtual ~Machine() throw();
				static Machine* CreateFromType(Machine::class_type msubclass,Machine::id_type id,std::string dllname);
			//////////////////////////////////////////////////////////////////////////
			// Actions

			///\name the life cycle of a mahine
			///\{
				public:
					virtual void Init();
					virtual void PreWork(int numSamples);
					virtual void Work(int numSamples);
					virtual void WorkNoMix(int numSamples);
					virtual void Tick() {};
					virtual void Tick(int track, PatternEntry * pData) {};
					virtual void Stop() {};
			///\}

			///\name (de)serialization
			///\{
				public:
					virtual void SaveDllName(RiffFile * pFile);
					virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
					static Machine * LoadFileChunk(RiffFile* pFile, Machine::id_type index, int version,bool fullopen=true);
					virtual void SaveFileChunk(RiffFile * pFile);
					virtual void SaveSpecificChunk(RiffFile * pFile);
				protected: friend class Song;
					/// Loader for psycle fileformat version 2.
					virtual bool LoadOldFileFormat(RiffFile * pFile);
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
					virtual void InitWireVolume(class_type, Wire::id_type, float value);
					virtual Wire::id_type FindInputWire(id_type);
					virtual Wire::id_type FindOutputWire(id_type);
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
				virtual void SetSampleRate(int hertz) { /* \todo should this be a pure virtual function? */ };

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

			virtual float GetAudioRange() { return _audiorange; }

			///\name amplification of the signal in connections/wires
			///\{
				public:
					virtual void GetWireVolume(Wire::id_type wire, float & result) { result = _inputConVol[wire] * _wireMultiplier[wire]; };
					virtual void SetWireVolume(Wire::id_type wire, float value) { _inputConVol[wire] = value / _wireMultiplier[wire]; };
					virtual bool GetDestWireVolume(id_type src, Wire::id_type, float & result);
					virtual bool SetDestWireVolume(id_type src, Wire::id_type, float value);
			///\}

			///\name machine information
			///\{
				public:
					static InternalMachinePackage& infopackage();
					static const InternalMachineInfo* GetInfoFromType(Machine::class_type msubclass);
					virtual const std::string GetDllName() { return ""; }; //\todo: Empty string. This is (to be) used in the song saver.
					virtual const std::string GetBrand();
					virtual const std::string GetVendorName();
					virtual const std::uint32_t GetVersion();
					virtual const std::uint32_t GetCategory();
					virtual std::string const & GetEditName() { return _editName; }
					virtual void SetEditName(std::string newname) { _editName = newname; }

				PSYCLE__PRIVATE:
					std::string  _editName;
			///\}

			///\name parameters
			///\{
				public:
					virtual int GetNumCols() { return _nCols; };
					virtual int GetNumParams() { return _numPars; };
					virtual void GetParamName(int numparam, char * name) { name[0]='\0'; };
					virtual void GetParamRange(int numparam, int &minval, int &maxval) {minval=0; maxval=0; };
					virtual void GetParamValue(int numparam, char * parval) { parval[0]='\0'; };
					virtual int GetParamValue(int numparam) { return 0; };
					virtual bool SetParameter(int numparam, int value) { return false;}; 
			///\}

			///\name more misplaced gui stuff
			///\{
				public:
					virtual int  GetPosX() { return _x; };
					virtual void SetPosX(int x) {_x = x;};
					virtual int  GetPosY() { return _y; };
					virtual void SetPosY(int y) {_y = y;};

			///\}
		protected:
			void SetVolumeCounter(int numSamples, int multiplier=1.0f);
			//void SetVolumeCounterAccurate(int numSamples);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//\todo below are unencapsulated data members

		PSYCLE__PRIVATE:

			InPort *inports;
			OutPort *outports;
			int numInPorts;
			int numOutPorts;
			bool _bypass;
			bool _mute;
			bool _waitingForSound;
			bool _stopped;
			bool _worked;
			float _audiorange;
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
			int _numPars;
			int _nCols;

			///\name input ports
			///\{
				/// number of Incoming connections
				int _connectedInputs;
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
				int _connectedOutputs;
				/// Outgoing connections Machine numbers
				///\todo hardcoded limits and wastes
				Machine::id_type _outputMachines[MAX_CONNECTIONS];
				/// Outgoing connections activated
				///\todo hardcoded limits and wastes
				bool _connection[MAX_CONNECTIONS];
			///\}

			///\name misplaced gui stuff
			///\{
				/// The topleft point of a square where the wire triangle is centered when drawn. (Used to detect when to open the wire dialog)
				///\todo hardcoded limits and wastes
				CPoint _connectionPoint[MAX_CONNECTIONS];
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
				int	_scopeBufferIndex;
				///\todo doc
				float *_pScopeBufferL;
				///\todo doc
				float *_pScopeBufferR;
			///\}

			///\ various player-related states
			///\{
				///\todo hardcoded limits and wastes
				PatternEntry TriggerDelay[MAX_TRACKS];
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

		// Helper class for Machine Creation.
		typedef Machine* (*CreatorFromType)(Machine::id_type _id, std::string _dllname);

		class InternalMachineInfo
		{
		public:
			InternalMachineInfo() { ; }
			InternalMachineInfo(Machine::class_type _class,Machine::mode_type _mode,CreatorFromType _creator, bool _host, bool _deprecated,
				char const* _brandname,char const* _shortname,char const* _vendor,
				std::uint32_t _category, std::uint32_t _version, std::uint32_t _parameters)
				:mclass(_class),mode(_mode),CreateFromType(_creator), host(_host), deprecated(_deprecated)
				,brandname(_brandname),shortname(_shortname),vendor(_vendor),category(_category)
				,version(_version),parameters(_parameters) { ; }

			bool operator<(const InternalMachineInfo & info) const { return mclass < info.mclass; }
			void operator=(const InternalMachineInfo & info)
			{
				mclass=info.mclass; mode=info.mode; CreateFromType=info.CreateFromType; host=info.host; deprecated=info.deprecated;
				brandname=info.brandname;shortname=info.shortname;vendor=info.vendor;
				category=info.category;version=info.version;parameters=info.parameters;
			}
		public:
			///< Class of machine (master, sampler, dummy,...). See MachineClass
			Machine::class_type mclass;
			///< Mode of the plugin, ( generator, effect,...) See MachineMode
			Machine::mode_type mode;
			///< Creator function. Needed for the loader.
			CreatorFromType CreateFromType;
			///< Indicates if the machine is unique or a host of machines (.dll's)
			bool host;
			///< Indicates that this Info exists just for compatibility reasons (loading old songs).
			bool deprecated;
			///< Name of the machine
			char const *brandname;
			///< Default Display name.
			char const *shortname;
			///< Authority of the machine
			char const *vendor;
			///< Default category.
			//\todo: define categories.
			std::uint32_t category;
			///< version numbering. Prefered form is " 1.0 -> 1000 "
			std::uint32_t version;
			///< The Number of parameters that this machine exports.
			std::uint32_t parameters;
			//\todo : description field?
		};




		inline void Machine::SetVolumeCounter(int numSamples, int multiplier)
		{
			_volumeCounter = dsp::GetMaxVol(_pSamplesL, _pSamplesR, numSamples)*multiplier;
			if(_volumeCounter > 32768.0f) _volumeCounter = 32768.0f;
			else if (_volumeCounter <= 0) _volumeCounter = 1.0f;
			int temp(f2i(fast_log2(_volumeCounter) * 78.0f * 4 / 14.0f) - (78 * 3));// not 100% accurate, but looks as it sounds
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

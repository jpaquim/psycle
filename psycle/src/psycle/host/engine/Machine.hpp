///\file
///\brief interface file for psycle::host::Machine
#pragma once
#include <psycle/host/engine/SongStructs.hpp>
#include <psycle/host/engine/dsp.hpp>
#include <psycle/host/engine/helpers.hpp>
#include <psycle/host/engine/constants.hpp>
#include <psycle/host/engine/FileIO.hpp>
#include <psycle/host/global.hpp>
#include <universalis/processor/exceptions/fpu.hpp>
#include <universalis/compiler/location.hpp>
#include <stdexcept>
#include <cstdint>
namespace psycle
{
	namespace host
	{
		/// we don't really need a macro for just one little expression...
		#define PSYCLE__CPU_COST__INIT(cost) cpu::cycles_type cost(cpu::cycles());
		/// we don't really need a macro for just one little expression...
		#define PSYCLE__CPU_COST__CALCULATE(cost, _) cost = cpu::cycles() - cost;

		class Machine; // forward declaration
//		class RiffFile; // forward declaration

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
					template<typename E> const std::string string(E const & e) { std::ostringstream s; s << e; return s.str(); }
					template<> const std::string string<std::exception>(std::exception const & e) { return e.what(); }
					template<> const std::string string<void const *>(void const * const &) { return "Type of exception is unkown, cannot display any further information."; }
				}

				template<typename E> void rethrow(Machine & machine, std::string const & function, E const * const e, std::exception const * const standard) throw(function_error)
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

				#define PSYCLE__HOST__CATCH_ALL \
					catch(          std::exception const & e) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(                 wchar_t const e[]) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(                  char   const e[]) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(  signed          char   const & e) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(unsigned          char   const & e) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(  signed     short int   const & e) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(unsigned     short int   const & e) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(  signed           int   const & e) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(unsigned           int   const & e) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(  signed      long int   const & e) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(unsigned      long int   const & e) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(  signed long long int   const & e) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(unsigned long long int   const & e) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(            void const * const   e) { host::exceptions::function_errors::rethrow       (host(), UNIVERSALIS__COMPILER__LOCATION, &e); } \
					catch(               ...                ) { host::exceptions::function_errors::rethrow<void*>(host(), UNIVERSALIS__COMPILER__LOCATION    ); }

				/// Exception caused by a bad returned value from a library function.
				class bad_returned_value : public function_error
				{
					public:
						bad_returned_value(std::string const & what) : function_error(what) {}
				};
			}
		}

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

		class AudioPort;
		// A wire is what interconnects two AudioPorts. Appart from being the graphically representable element,
		//	the wire is also responsible of volume changes and even pin reassignation (convert 5.1 to stereo, etc.. not yet)
		class Wire
		{
		public:
			Wire()
				:volume(1.0f),pan(0.0f),multiplier(1.0f),rvol(1.0f),lvol(1.0f)
				,index(0),senderport(0),receiverport(0){};
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
			virtual inline int GetIndex() { return index; };
			virtual inline int SetIndex(int idx) { index = idx; };
			
		protected:
			virtual void Disconnect(AudioPort* port);
			virtual inline float RVol() { return rvol; };
			virtual inline float LVol() { return lvol; };
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
		// An Audio port is synonim of a channel. In other words, it is an element that defines
		//	the characteristics of one or more individual inputs that are used in conjunction.
		//	From this definition, we could have one Stereo Audio Port (one channel, two inputs or outputs),
		//  a 5.1 Port, or several Stereo Ports (in the case of a mixer table), between others..
		// Note that several wires can be connected to the same AudioPort.
		class AudioPort
		{
		protected:
			AudioPort(){};
		public:
			//\todo: Port creation, assign buffers to it (passed via ctor? they might be shared). 
			//\todo: Also, Multiple buffers or a packed buffer (left/right/left/right...)?
			AudioPort(Machine* powner,int arrangement,std::string name)
			{
				pParent=powner;
				portName=name;
				portArrangement=arrangement;
			}
			virtual ~AudioPort(){};
			virtual void CollectData(int numSamples){};
			virtual void Connected(Wire *wire);
			virtual void Disconnected(Wire *wire);
			virtual inline Wire* GetWire(unsigned int index){ assert(index<wires.size()); return wires[index]; };
			virtual inline bool NumberOfWires() { return wires.size(); };
			virtual inline int Arrangement() { return portArrangement; };
			virtual inline Machine * GetMachine() { return pParent; };
			//\todo : should change arrangement/name be allowed? (Mutating Port?)
			virtual inline void ChangeArrangement(int arrangement) { portArrangement = arrangement; };
			virtual inline std::string  Name() { return portName; };
			virtual inline void ChangeName(std::string name) { portName = name; };
		protected:
			int portArrangement;
			std::string portName;
			std::vector<Wire*> wires;
			Machine* pParent;
		};

		class InPort : public AudioPort
		{
		protected:
			InPort(){};
		public:
			InPort(Machine* powner,int arrangement,std::string name)
			{
				AudioPort(powner,arrangement,name);
			}
			virtual ~InPort(){};
			virtual void CollectData(int numSamples);
		};

		class OutPort : public AudioPort
		{
		protected:
			OutPort(){};
		public:
			OutPort(Machine* powner,int arrangement,std::string name)
			{
				AudioPort(powner,arrangement,name);
			}
			virtual ~OutPort(){};
			virtual void CollectData(int numSamples);
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

				public:
					universalis::processor::exceptions::fpu::mask::type const inline & fpu_exception_mask() const throw() { return fpu_exception_mask_; }
					universalis::processor::exceptions::fpu::mask::type       inline & fpu_exception_mask()       throw() { return fpu_exception_mask_; }
				private:
					universalis::processor::exceptions::fpu::mask::type                fpu_exception_mask_;
			///\}

		public:
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
			

			// bool IsDllLoaded()
			// bool IsPowered()
			// bool IsBypass()
			// bool IsStandBy()
			// GetDllName()
			Machine();
			virtual ~Machine() throw();
			//////////////////////////////////////////////////////////////////////////
			// Actions

			virtual void Init();
			virtual void PreWork(int numSamples);
			virtual void Work(int numSamples);
			virtual void WorkNoMix(int numSamples);
			virtual void Tick() {};
			virtual void Tick(int track, PatternEntry * pData) {};
			virtual void Stop() {};
			/// Loader for old psycle fileformat.
			virtual bool LoadOldFileFormat(RiffFile * pFile);
			virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
			static Machine * LoadFileChunk(RiffFile* pFile, int index, int version,bool fullopen=true);
			virtual void SaveFileChunk(RiffFile * pFile);
			virtual void SaveSpecificChunk(RiffFile * pFile);
			virtual void SaveDllName(RiffFile * pFile);
			virtual bool ConnectTo(Machine* dstMac,int dstport=0,int outport=0,float volume=1.0f);
			virtual bool Disconnect(Machine* dstMac);
			virtual void InitWireVolume(MachineType mType,int wireIndex,float value);
			virtual int FindInputWire(int macIndex);
			virtual int FindOutputWire(int macIndex);
			void DefineStereoInput(int numins);
			void DefineStereoOutput(int numouts);
			//////////////////////////////////////////////////////////////////////////
			// Properties

			virtual void SetSampleRate(int sr) {};
			virtual void SetPan(int newpan);
			virtual int GetInPorts() { return numInPorts; };
			virtual int GetOutPorts() { return numOutPorts; };
			virtual AudioPort& GetInPort(unsigned int i) { assert(i<numInPorts); return inports[i]; };
			virtual AudioPort& GetOutPort(unsigned int i) { assert(i<numOutPorts); return inports[i]; };
			virtual float GetAudioRange() { return _audiorange; }
			virtual void GetWireVolume(int wireIndex, float &value) { value = _inputConVol[wireIndex] * _wireMultiplier[wireIndex]; };
			virtual void SetWireVolume(int wireIndex,float value) { _inputConVol[wireIndex] = value / _wireMultiplier[wireIndex]; };
			virtual bool GetDestWireVolume(int srcIndex, int WireIndex,float &value);
			virtual bool SetDestWireVolume(int srcIndex, int WireIndex,float value);
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
		protected:
			void SetVolumeCounter(int numSamples);
			//void SetVolumeCounterAccurate(int numSamples);

		public:
			InPort *inports;
			OutPort *outports;
			int numInPorts;
			int numOutPorts;
			int _macIndex;
			MachineType _type;
			MachineMode _mode;
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
			int _connectedInputs;
			/// number of Outgoing connections
			int _connectedOutputs;
			/// The topleft point of a square where the wire triangle is centered when drawn. (Used to detect when to open the wire dialog)
			CPoint _connectionPoint[MAX_CONNECTIONS];

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

			int _scopePrevNumSamples;
			int	_scopeBufferIndex;
			float *_pScopeBufferL;
			float *_pScopeBufferR;

			public:
				void             inline work_cpu_cost(cpu::cycles_type const & value)       throw() { work_cpu_cost_ = value; }
				cpu::cycles_type inline work_cpu_cost(                              ) const throw() { return work_cpu_cost_; }
			private:
				cpu::cycles_type        work_cpu_cost_;

			public:
				void             inline wire_cpu_cost(cpu::cycles_type const & value)       throw() { wire_cpu_cost_ = value; }
				cpu::cycles_type inline wire_cpu_cost(                              ) const throw() { return wire_cpu_cost_; }
			private:
				cpu::cycles_type        wire_cpu_cost_;
		};

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



		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// internal machines



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

		/// note duplicator machine.
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
			virtual int GetAudioInputs() { return 24; };
			virtual int GetAudioOutputs() { return 1; };
			virtual std::string GetAudioInputName(int port);
			virtual std::string GetAutioOutputName(int port) { std::string rettxt = "Stereo Output"; return rettxt; };
			virtual bool ConnectTo(Machine* dstMac,int dstport=0,int outport=0,float volume=1.0f);
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
	}
}


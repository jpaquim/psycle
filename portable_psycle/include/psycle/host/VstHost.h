#pragma once
#include "Machine.h"
#include <vst/AEffectx.h>
#include "constants.h"
#include "Helpers.h"
#include "Configuration.h"
#include <operating_system/exception.h>
#include <operating_system/exceptions/code_description.h>
#if !defined _WINAMP_PLUGIN_
	#include "NewMachine.h"
#endif
///\file
///\brief interface file for psycle::host::VSTPlugin
namespace psycle
{
	namespace host
	{
		/// VST Host.
		namespace vst
		{
			/// Classes derived from exception thrown by vst plugins.
			namespace exceptions
			{
				/// Exception caused by an error in a call to the vst dispatch function.
				class dispatch_error : public host::exceptions::function_error
				{
				public:
					inline dispatch_error(const std::string & what) : function_error(what) {}
				};
			}

			/// Maximum number of Audio Input/outputs
			/// \todo : this shouldn't be a static value. Host should ask the plugin and the array get created dynamically.
			const int max_io = 16;

			/// Dialog max ticks for parameters.
			const int quantization = 65535;

			class plugin; // forward declaration

			/// Proxy between the host and a plugin.
			class proxy
			{
			public:
				proxy(vst::plugin & host, AEffect * const plugin = 0);
				~proxy() throw();

			private:
				plugin & host_;
				plugin & host() throw();
				const plugin & host() const throw();

			private:
				AEffect * plugin_;
				AEffect & plugin() throw();
				const AEffect & plugin() const throw();
			public:
				void operator()(AEffect * const plugin) throw(host::exceptions::function_error);
				const bool operator()() const throw();

				long int magic() throw(host::exceptions::function_error);
				long int dispatcher(long int operation = 0, long int index = 0, long int value = 0, void * ptr = 0, float opt = 0) throw(host::exceptions::function_error);
				void process(float * * inputs, float * * outputs, long int sampleframes) throw(host::exceptions::function_error);
				void processReplacing(float * * inputs, float * * outputs, long int sampleframes) throw(host::exceptions::function_error);
				void setParameter(long int index, float parameter) throw(host::exceptions::function_error);
				float getParameter(long int index) throw(host::exceptions::function_error);
				long int numPrograms() throw(host::exceptions::function_error);
				long int numParams() throw(host::exceptions::function_error);
				long int numInputs() throw(host::exceptions::function_error);
				long int numOutputs() throw(host::exceptions::function_error);
				long int flags() throw(host::exceptions::function_error);
				long int uniqueId() throw(host::exceptions::function_error);
				long int version() throw(host::exceptions::function_error);

				/// Create and initialize the VST plugin ( plugin Side ). Call this before using it. (except for string data)
				long int open()
				{
					return dispatcher(effOpen);
				}
				#pragma warning(push)
				#pragma warning(disable:4702) // unreachable code
				/// Destroys the VST plugin instance ( plugin side ).
				long int close()
				{
					try
					{
						/// also clears plugin_ pointer since it is no longer valid after effClose.
						long int retval = dispatcher(effClose);
						plugin_ = 0;
						return retval;
					}
					catch(...)
					{
						plugin_ = 0;
						throw;
					}
				}
				#pragma warning(pop)
				// Tells the VST plugin the desired samplerate.
				long int setSampleRate(float sr)
				{
					assert(sr>0.0f);
					return dispatcher(effSetSampleRate,0,0,0,sr);
				}
				// Tels the VST plugin the MAX block size of data that it will request.
			
				long int setBlockSize(int bs)
				{
					assert(bs>0);
					return dispatcher(effSetBlockSize,0,bs);
				}
				long int setSpeakerArrangement(VstSpeakerArrangement* inputArrangement, VstSpeakerArrangement* outputArrangement)
				{
					assert(inputArrangement && outputArrangement);
					return dispatcher(effSetSpeakerArrangement, 0, (long) inputArrangement, outputArrangement);
				}
				long int getProgram()
				{
					return dispatcher(effGetProgram);
				}
				long int setProgram(int program)
				{
					assert(program>=0);
					assert(program<numPrograms() || numPrograms()==0);
					return dispatcher(effSetProgram,0,program);
				}
				/// Gets the VST implementation's Version that the plugin uses. ( 1.0,2.0,2.1,2.2 or 2.3)
				long int getVstVersion()
				{
					return dispatcher(effGetVstVersion);
				}
				/// Turns on or off the plugin. If it is disabled, it won't produce output, but should behave without errors.
				long int mainsChanged(bool on)
				{
					return dispatcher(effMainsChanged, 0, on ? 1 : 0);
				}
				long int getEffectName(char * buffer)
				{
					assert(buffer);
					buffer[0]=0;
					return dispatcher(effGetEffectName, 0, 0, buffer);
				}
				long int getVendorString(char * buffer)
				{
					assert(buffer);
					buffer[0]=0;
					return dispatcher(effGetVendorString, 0, 0, buffer);
				}
			};

			/// VST plugin.
			class plugin : public Machine
			{
			private:
				typedef AEffect * (* PVSTMAIN) (audioMasterCallback audioMaster);
			public:
				/// Host callback dispatcher.
				static long int AudioMaster(AEffect * effect, long int opcode, long int index, long int value, void * ptr, float opt);
			public:
				plugin();
				virtual ~plugin() throw();
				virtual const char * const GetDllName() const throw() { return _sDllName.c_str(); }
				virtual char * GetName() throw() { return (char*)_sProductName.c_str(); }
				virtual int GetNumParams()
				{
					try
					{
						return proxy().numParams();
					}
					catch(const std::exception &)
					{
						// <bohan>
						// exception blocked here for now,
						// but we really should do something...
						//throw;
						return 0; /// \todo <bohan> ???
					}
				}
				virtual void GetParamName(int numparam, char * name)
				{
					try
					{
						if(numparam < proxy().numParams())
							proxy().dispatcher(effGetParamName, numparam, 0, name);
						else std::strcpy(name,"Out of Range");
					}
					catch(const std::exception &)
					{
						// <bohan>
						// exception blocked here for now,
						// but we really should do something...
						//throw;
						std::strcpy(name, "fucked up"); /// \todo <bohan> ???
					}
				}
				virtual void GetParamValue(int numparam, char * parval);
				#pragma warning(push)
				#pragma warning(disable:4702) // unreachable code
				virtual int GetParamValue(int numparam)
				{
					try
					{
						if(numparam < proxy().numParams())
							return f2i(proxy().getParameter(numparam) * 65535);
						else
							return 0; /// \todo <bohan> ???
					}
					catch(const std::exception &)
					{
						// <bohan>
						// exception blocked here for now,
						// but we really should do something...
						//throw;
						return 0; /// \todo <bohan> ???
					}
				}
				#pragma warning(pop)
				virtual bool Load(RiffFile * pFile);
				virtual bool LoadSpecificFileChunk(RiffFile * pFile, int version);

				#if !defined _WINAMP_PLUGIN_
					virtual void SaveSpecificChunk(RiffFile* pFile) ;
					virtual void SaveDllName(RiffFile* pFile);
				#endif

				void Instance(std::string dllname, const bool overwriteName = true) throw(...);
				void Free() throw(...);
				bool DescribeValue(int parameter, char * psTxt);
				bool LoadDll(std::string psFileName);
				///\todo To be removed when changing the fileformat.
				bool LoadChunk(RiffFile* pFile);
				inline const long int & GetVersion() const throw() { return _version; }
				inline const char * const GetVendorName() const throw() { return _sVendorName.c_str(); }
				inline const bool & IsSynth() const throw() { return _isSynth; }
				inline bool AddMIDI(unsigned char data0, unsigned char data1 = 0, unsigned char data2 = 0);
				inline void SendMidi();
				inline proxy & proxy() throw() { return *proxy_; };
				bool SetParameter(int parameter, float value);
				bool SetParameter(int parameter, int value);
				//void SetCurrentProgram(int prg);
				//int GetCurrentProgram();

				unsigned char _program;
				bool instantiated;
				///\todo Remove when Changing the FileFormat.
				int _instance;
				/// It needs to use Process
				bool requiresProcess;
				/// It needs to use ProcessRepl
				bool requiresRepl;		
				#if !defined _WINAMP_PLUGIN_
					CFrameWnd * editorWnd;
				#endif

			protected:
				/// midi events queue, is sent to processEvents.
				VstMidiEvent midievent[MAX_VST_EVENTS];
				int	queue_size;

				/// reserves space for a new midi event in the queue.
				/// \return midi event to be filled in, or null if queue is full.
				VstMidiEvent* reserveVstMidiEvent();
				VstMidiEvent* reserveVstMidiEventAtFront(); // ugly hack

				float * inputs[max_io];
				float * outputs[max_io];
				bool wantidle;
			private:
				HMODULE h_dll;
				/// Contains dll name
				std::string _sDllName;
				std::string _sProductName;
				std::string _sVendorName;
				long _version;
				bool _isSynth;
				float junk[STREAM_SIZE];
				static VstTimeInfo _timeInfo;
				VstEvents mevents;
				vst::proxy * proxy_;

				#ifndef NDEBUG
					class note_checker
					{
					public:
						note_checker()
						{
							for(int channel=0;channel<16;channel++)
								for(int note=0;note<128;note++)
									note_on_count_[channel][note]=0;
						}
						~note_checker()
						{
							/*
							for(int channel=0;channel<16;channel++)
								for(int note=0;note<128;note++)
									assert(note_on_count_[channel][note]==0);
							*/
						}
						void note_on(int note, int channel)
						{
							assert(note >= 0 && note < 128);
							assert(channel >= 0 && channel < 16);
							assert(note_on_count_[channel][note]>=0);
							++note_on_count_[channel][note];
						}
						void note_off(int note, int channel)
						{
							assert(note >= 0 && note < 128);
							assert(channel >= 0 && channel < 16);
							--note_on_count_[channel][note];
							//assert(note_on_count_[channel][note]>=0 && "there was a note-off without corresponding note-on!");
							// <bohan> it happens!!!
							if(note_on_count_[channel][note] < 0) psycle::host::loggers::exception("there was a note-off without corresponding note-on!");
						}
					private:
						// 16 channels, 128 keys
						short int note_on_count_[16][128];
					};
					note_checker note_checker_;
				#endif
			};

			/// vst note for an instrument.
			class note
			{
			public:
				unsigned char key;
				unsigned char midichan;
			};

			/// vst "instrument" (input) plugin.
			class instrument : public plugin
			{
			public:
				///\name functions which calls code in the plugin itself
				///\{
				virtual void Work(int numSamples);
				///\}
				instrument(int index);
				virtual void Tick(int channel, PatternEntry * pEntry);
				virtual void Stop(void);
				bool AddNoteOn(unsigned char channel, unsigned char key, unsigned char velocity, unsigned char midichannel = 0);
				bool AddNoteOff(unsigned char channel, unsigned char midichannel = 0, bool addatStart = false);
			protected:
				note trackNote[MAX_TRACKS];
			};

			/// vst "fx" (filter) plugin.
			class fx : public plugin
			{
			public:
				///\name functions which calls code in the plugin itself
				///\{
				virtual void Work(int numSamples);
				///\}
				fx(int index);
				virtual ~fx() throw();
				virtual void Tick(int channel, PatternEntry * pEntry);
			protected:
				float * _pOutSamplesL;
				float * _pOutSamplesR;
			};

			namespace exceptions
			{
				namespace dispatch_errors
				{
					/// Dispatcher operation code descriptions.
					const std::string operation_description(const long code) throw();
					std::string eff_opcode_to_string(long code) throw();

					template<typename e> void rethrow(plugin & plugin, const long int operation, const e * const e = 0) throw(dispatch_error)
					{
						std::ostringstream s;
						s << "Machine crashed: " << plugin._editName;
						if(plugin.GetDllName())
							s << ": " << plugin.GetDllName();
						s << std::endl
						  << "VST plugin had an exception on dispatcher operation: " << operation_description(operation) << '.' << std::endl
						  << typeid(*e).name() << std::endl
						  << host::exceptions::function_errors::string(*e);
						dispatch_error dispatch_error(s.str());
						plugin.crashed(dispatch_error);
						throw dispatch_error;
					}
				}
			}

			inline proxy::proxy(vst::plugin & host, AEffect * const plugin) : host_(host), plugin_(0) { (*this)(plugin); }
			inline proxy::~proxy() throw() { (*this)(0); }

			inline plugin & proxy::host() throw() { return host_; }
			inline const plugin & proxy::host() const throw() { return host_; }

			inline AEffect & proxy::plugin() throw() { assert(plugin_); return *plugin_; }
			inline const AEffect & proxy::plugin() const throw() { assert(plugin_); return *plugin_; }
			inline const bool proxy::operator()() const throw() { return plugin_; }
			inline void proxy::operator()(AEffect * const plugin) throw(host::exceptions::function_error)
			{
				if(this->plugin_)
				{
					//user(0);
					close();
				}
				// <magnus> we shouldn't delete plugin_ because the AEffect is allocated
				// by the plugin's DLL by some unknown means. Dispatching effClose will
				// automatically free up the AEffect structure.
				this->plugin_ = plugin;
				if(plugin)
				{
					static const char function[] = "operator()(AEffect * const plugin)";
					try
					{
						// AEffect's resvd2 data member is right after the resvd1 data member in memory,
						// so, we can use those two 32-bit data members together as a single, potentially 64-bit, address,
						// if we ever recompile psycle on a 64-bit operating system.
						// The vst plugins don't need to be recompiled for this to work,
						// but anyway, some other parts of the vst headers are supposed to be used on a 32-bit compiler
						// (i.e. their authors didn't forsee the problem).
						// So, those headers should be made explicitly 32-bit to support 32-bit vst plugins.
						// Since those headers are under a restricted license by steinberg, only steinberg's employees are allowed to fix them.
						*reinterpret_cast<vst::plugin**>(&plugin->resvd1) = &host();
					}
					catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
					catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
					catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
					catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
					catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
				}
			}

			#pragma warning(push)
			#pragma warning(disable:4702) // unreachable code
			inline long int proxy::magic() throw(host::exceptions::function_error)
			{
				assert((*this)());
				static const char function[] = "magic";
				try
				{
					return plugin().magic;
				}
				catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
				return 0; // dummy return to avoid warning
			}
			inline long int proxy::dispatcher(long int operation, long int index, long int value, void * ptr, float opt) throw(host::exceptions::function_error)
			{
				#ifndef NDEBUG
				{
					std::ostringstream s;
					s
						<< "VST plugin: call to plugin dispatcher: plugin address: " << &plugin()
						<< ", opcode: " << exceptions::dispatch_errors::operation_description(operation)
						<< ", index: " << index
						<< ", value = " << value
						<< ", ptr = " << ptr
						<< ", opt = " << opt;
					host::loggers::trace(s.str());
				}
				#endif
				assert((*this)());
				try
				{
					return plugin().dispatcher(&plugin(), operation, index, value, ptr, opt);
				}
				catch(const std::exception & e) { exceptions::dispatch_errors::rethrow(host(), operation, &e); }
				catch(const char e[]) { exceptions::dispatch_errors::rethrow(host(), operation, &e); }
				catch(const long int & e) { exceptions::dispatch_errors::rethrow(host(), operation, &e); }
				catch(const unsigned long int & e) { exceptions::dispatch_errors::rethrow(host(), operation, &e); }
				catch(...) { exceptions::dispatch_errors::rethrow<void*>(host(), operation); }
				return 0; // dummy return to avoid warning
			}
			inline void proxy::process(float * * inputs, float * * outputs, long int sampleframes) throw(host::exceptions::function_error)
			{
				assert((*this)());
				static const char function[] = "process";
				try
				{
					plugin().process(&plugin(), inputs, outputs, sampleframes);
				}
				catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
			}
			inline void proxy::processReplacing(float * * inputs, float * * outputs, long int sampleframes) throw(host::exceptions::function_error)
			{
				assert((*this)());
				static const char function[] = "processReplacing";
				try
				{
					plugin().processReplacing(&plugin(), inputs, outputs, sampleframes);
				}
				catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
			}
			inline void proxy::setParameter(long int index, float parameter) throw(host::exceptions::function_error)
			{
				assert((*this)());
				static const char function[] = "setParameter";
				try
				{
					plugin().setParameter(&plugin(), index, parameter);
				}
				catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
			}
			inline float proxy::getParameter(long int index) throw(host::exceptions::function_error)
			{
				assert((*this)());
				static const char function[] = "getParameter";
				try
				{
					return plugin().getParameter(&plugin(), index);
				}
				catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
				return 0; // dummy return to avoid warning
			}
			inline long int proxy::numPrograms() throw(host::exceptions::function_error)
			{
				assert((*this)());
				static const char function[] = "numPrograms";
				try
				{
					return plugin().numPrograms;
				}
				catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
				return 0; // dummy return to avoid warning
			}
			inline long int proxy::numParams() throw(host::exceptions::function_error)
			{
				assert((*this)());
				static const char function[] = "numParams";
				try
				{
					return plugin().numParams;
				}
				catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
				return 0; // dummy return to avoid warning
			}
			inline long int proxy::numInputs() throw(host::exceptions::function_error)
			{
				assert((*this)());
				static const char function[] = "numInputs";
				try
				{
					return plugin().numInputs;
				}
				catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
				return 0; // dummy return to avoid warning
			}
			inline long int proxy::numOutputs() throw(host::exceptions::function_error)
			{
				assert((*this)());
				static const char function[] = "numOutputs";
				try
				{
					return plugin().numOutputs;
				}
				catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
				return 0; // dummy return to avoid warning
			}
			inline long int proxy::flags() throw(host::exceptions::function_error)
			{
				assert((*this)());
				static const char function[] = "flags";
				try
				{
					return plugin().flags;
				}
				catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
				return 0; // dummy return to avoid warning
			}
			inline long int proxy::uniqueId() throw(host::exceptions::function_error)
			{
				assert((*this)());
				static const char function[] = "uniqueId";
				try
				{
					return plugin().uniqueID;
				}
				catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
				return 0; // dummy return to avoid warning
			}
			inline long int proxy::version() throw(host::exceptions::function_error)
			{
				assert((*this)());
				static const char function[] = "version";
				try
				{
					return plugin().version;
				}
				catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
				return 0; // dummy return to avoid warning
			}
			#pragma warning(pop)
		}
	}
}

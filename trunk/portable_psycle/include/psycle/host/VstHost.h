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

			/// midi's 16 channels limit?
			const int max_io = 16;

			/// Dialog max ticks for parameters.
			const int quantization = 65535;

			class plugin; // forward declaration

			/// Proxy between the host and a plugin.
			class proxy
			{
			private:
				plugin & host_;
				AEffect * plugin_;
			private:
				inline plugin & host() throw();
				inline const plugin & host() const throw();
				inline AEffect & plugin() throw();
				inline const AEffect & plugin() const throw();
			public:
				inline proxy(vst::plugin & host, AEffect * plugin = 0);
				inline ~proxy() throw();
				inline const bool operator()() const throw();
				inline void operator()(AEffect * plugin) throw(host::exceptions::function_error);
				inline long int magic() throw(host::exceptions::function_error);
				inline long int dispatcher(long int operation = 0, long int index = 0, long int value = 0, void * ptr = 0, float opt = 0) throw(exceptions::dispatch_error);
				inline void process(float * * inputs, float * * outputs, long int sampleframes) throw(host::exceptions::function_error);
				inline void processReplacing(float * * inputs, float * * outputs, long int sampleframes) throw(host::exceptions::function_error);
				inline void setParameter(long int index, float parameter) throw(host::exceptions::function_error);
				inline float getParameter(long int index) throw(host::exceptions::function_error);
				inline long int numPrograms() throw(host::exceptions::function_error);
				inline long int numParams() throw(host::exceptions::function_error);
				inline long int numInputs() throw(host::exceptions::function_error);
				inline long int numOutputs() throw(host::exceptions::function_error);
				inline long int flags() throw(host::exceptions::function_error);
				inline long int uniqueId() throw(host::exceptions::function_error);
				inline long int version() throw(host::exceptions::function_error);
				inline void user(void * user) throw(host::exceptions::function_error);
			};

			/// VST plugin.
			class plugin : public Machine
			{
			private:
				typedef AEffect * (* PVSTMAIN) (audioMasterCallback audioMaster);
			public:
				/// Host callback dispatcher.
				static long AudioMaster(AEffect * effect, long int opcode, long int index, long int value, void * ptr, float opt);
			public:
				plugin();
				virtual ~plugin() throw();
				inline virtual const char * const GetDllName() const throw() { return _sDllName; }
				inline virtual char * GetName() throw() { return _sProductName; }
				inline virtual void GetParamName(int numparam, char * name)
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
																std::strcpy(name, "fucked up");
															}
														}
				virtual void GetParamValue(int numparam, char * parval);
				inline virtual int GetParamValue(int numparam)
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
				virtual bool Load(RiffFile * pFile);
				virtual bool LoadSpecificFileChunk(RiffFile * pFile, int version);

				#if !defined _WINAMP_PLUGIN_
					virtual void SaveSpecificChunk(RiffFile* pFile) ;
					virtual void SaveDllName(RiffFile* pFile);
				#endif

				void Instance(const char dllname[], const bool overwriteName = true) throw(...);
				void Free() throw(...);
				bool DescribeValue(int parameter, char * psTxt);
				bool LoadDll(char * psFileName);
				///\todo To be removed when changing the fileformat.
				bool LoadChunk(RiffFile* pFile);
				inline const long int & GetVersion() const throw() { return _version; }
				inline const char * const GetVendorName() const throw() { return _sVendorName; }
				inline const bool & IsSynth() const throw() { return _isSynth; }
				inline bool AddMIDI(unsigned char data0, unsigned char data1 = 0, unsigned char data2 = 0);
				inline void SendMidi();
				inline proxy & proxy() throw() { return proxy_; };
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
				VstMidiEvent midievent[MAX_VST_EVENTS];
				int	queue_size;
				float * inputs[max_io];
				float * outputs[max_io];
				bool wantidle;
			private:
				HMODULE h_dll;
				/// Contains dll name
				char * _sDllName;
				char _sProductName[64];
				char _sVendorName[64];
				long _version;
				bool _isSynth;
				float junk[STREAM_SIZE];
				static VstTimeInfo _timeInfo;
				VstEvents events;
				vst::proxy proxy_;
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
					const std::string operation_description(const long int & code) throw();

					template<typename e> void rethrow(plugin & plugin, const long int operation, const e * const e = 0) throw(dispatch_error)
					{
						std::ostringstream title; title << "VST Plugin: " << plugin._editName << ": " << plugin.GetDllName();
						std::ostringstream s; s
							<< title.str().c_str() << std::endl
							<< "VST plugin had an exception on dispatcher operation: " << operation_description(operation) << '.' << std::endl
							<< typeid(*e).name() << std::endl
							<< host::exceptions::function_errors::string(*e);
						::MessageBox(0, s.str().c_str(), title.str().c_str(), MB_OK | MB_ICONWARNING);
						dispatch_error dispatch_error(s.str());
						plugin.crashed(dispatch_error);
						throw dispatch_error;
					}
				}
			}

			inline plugin & proxy::host() throw() { return host_; }
			inline const plugin & proxy::host() const throw() { return host_; }
			inline AEffect & proxy::plugin() throw() { return *plugin_; }
			inline const AEffect & proxy::plugin() const throw() { return *plugin_; }
			inline proxy::proxy(vst::plugin & host, AEffect * plugin) : host_(host), plugin_(0) { (*this)(plugin); }
			inline proxy::~proxy() throw() { (*this)(0); }
			inline const bool proxy::operator()() const throw() { return plugin_; }
			inline void proxy::operator()(AEffect * plugin) throw(host::exceptions::function_error)
			{
				if((*this)()) user(0);
				delete &this->plugin();
				this->plugin_ = plugin;
				if((*this)()) user(this);
			}
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
			}
			inline long int proxy::dispatcher(long int operation, long int index, long int value, void * ptr, float opt) throw(exceptions::dispatch_error)
			{
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
			}
			inline void proxy::user(void * user) throw(host::exceptions::function_error)
			{
				assert((*this)());
				static const char function[] = "processReplacing";
				try
				{
					plugin().user = user;
				}
				catch(const std::exception & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const char e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(host(), function, &e); }
				catch(...) { host::exceptions::function_errors::rethrow<void*>(host(), function); }
			}
		}
	}
}

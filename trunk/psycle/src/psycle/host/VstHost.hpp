/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\brief interface file for psycle::host::VSTPlugin
#pragma once
#include "Constants.hpp"
#include "Helpers.hpp"
#include <operating_system/exception.hpp>
#include <operating_system/exceptions/code_description.hpp>
#include "Machine.hpp"
#include <vst/AEffectx.h>
namespace psycle
{
	namespace host
	{
		// VST Host.
		namespace vst
		{
			// Maximum number of Audio Input/outputs
			// \todo : this shouldn't be a static value. Host should ask the plugin and the array get created dynamically.
			const int max_io = 16;
			// Dialog max ticks for parameters.
			const int quantization = 65535;
			// forward declaration
			class plugin;

//////////////////////////////////////////////////////////////////////////
//		Class vst::proxy .
//		Proxy between the host and a plugin.
//////////////////////////////////////////////////////////////////////////
			class proxy
			{
			public:
				proxy(vst::plugin & host, AEffect * const plugin = 0) : host_(host), plugin_(0) { (*this)(plugin); }
				~proxy() throw() { (*this)(0); }
			public:
				void operator()(AEffect * const plugin) throw(host::exceptions::function_error);
				const bool operator()() const throw() { return plugin_; }
			private:
				plugin & host_;
				plugin & host() throw() { return host_; }
				const plugin & host() const throw() { return host_; }
			private:
				AEffect * plugin_;
				AEffect & plugin() throw() { assert(plugin_); return *plugin_; }
				const AEffect & plugin() const throw() { assert(plugin_); return *plugin_; }

			public:
				//////////////////////////////////////////////////////////////////////////
				// AEffect Properties
				//////////////////////////////////////////////////////////////////////////
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
				//\todo: maybe exchange "flags()" with functions for each flag.
				long int flags() throw(host::exceptions::function_error);
				long int uniqueId() throw(host::exceptions::function_error);
				long int version() throw(host::exceptions::function_error);
				long int initialDelay() throw(host::exceptions::function_error);

				//////////////////////////////////////////////////////////////////////////
				// AEffect "eff" OpCodes. Version 1.0
				//////////////////////////////////////////////////////////////////////////
				// Create and initialize the VST plugin ( plugin Side ). Call this before using it. (except for string data)
				long int open() { return dispatcher(effOpen); }
				// Destroys the VST plugin instance ( plugin side ). Also clears plugin_ pointer since it is no longer valid after effClose.
				long int close() { int retval = dispatcher(effClose); plugin_=0; return retval;	}
				long int getProgram() { return dispatcher(effGetProgram); }
				long int setProgram(int program)
				{
					assert(program>=0);
					assert(program<numPrograms() || numPrograms()==0);
					return dispatcher(effSetProgram,0,program);
				}
/*				effSetProgramName,	// user changed program name (max 24 char + 0) to as passed in string 
*				effGetProgramName,	// stuff program name (max 24 char + 0) into string 
*				effGetParamLabel,	// stuff parameter <index> label (max 8 char + 0) into string
*									// (examples: sec, dB, type)
*				effGetParamDisplay,	// stuff parameter <index> textual representation into string
*									// (examples: 0.5, -3, PLATE)
*				effGetParamName,	// stuff parameter <index> label (max 8 char + 0) into string
*									// (examples: Time, Gain, RoomType) 
*				effGetVu,			// called if (flags & (effFlagsHasClip | effFlagsHasVu))
*/
				// Tells the VST plugin the desired samplerate.
				long int setSampleRate(float sr)
				{
					assert(sr > 0);
					return dispatcher(effSetSampleRate,0,0,0,sr);
				}
				// Tells the VST plugin the MAX block size of data that it will request. (default value for VST's is 1024)
				long int setBlockSize(int bs)
				{
					assert(bs>0);
					return dispatcher(effSetBlockSize,0,bs);
				}
				/// Turns on or off the plugin. If it is disabled, it won't produce output, but should behave without errors.
				long int mainsChanged(bool on) { return dispatcher(effMainsChanged, 0, on ? 1 : 0); }
/*
*				effEditGetRect,		// stuff rect (top, left, bottom, right) into ptr
*				effEditOpen,		// system dependant Window pointer in ptr
*				effEditClose,		// no arguments
*				effEditKey,			// system keycode in value
*				effEditIdle,		// no arguments. Be gentle!
*				effEditTop,			// window has topped, no arguments
*				effEditSleep,		// window goes to background
*
*				effIdentify,		// returns 'NvEf'
*				effGetChunk,		// host requests pointer to chunk into (void**)ptr, byteSize returned
*				effSetChunk,		// plug-in receives saved chunk, byteSize passed
*/
				//////////////////////////////////////////////////////////////////////////
				// AEffect "eff" OpCodes. Version 2.0
				//////////////////////////////////////////////////////////////////////////

				long int setSpeakerArrangement(VstSpeakerArrangement* inputArrangement, VstSpeakerArrangement* outputArrangement)
				{
					assert(inputArrangement && outputArrangement);
					return dispatcher(effSetSpeakerArrangement, 0, (long) inputArrangement, outputArrangement);
				}
				/// Gets the VST implementation's Version that the plugin uses. ( 1.0,2.0,2.1,2.2 or 2.3)
				long int getVstVersion() {	return dispatcher(effGetVstVersion); }
				long int getEffectName(char * buffer)
				{
					assert(buffer);		buffer[0]=0;
					return dispatcher(effGetEffectName, 0, 0, buffer);
				}
				long int getVendorString(char * buffer)
				{
					assert(buffer);		buffer[0]=0;
					return dispatcher(effGetVendorString, 0, 0, buffer);
				}
			};

///////////////////////////////////////////////////////////////////////////////////
//		Class vst::plugin .
//		Base Class VST plugin. Implements the functions common to VST Fx and VSTi's
///////////////////////////////////////////////////////////////////////////////////
			class plugin : public Machine
			{
			private:
				class note
				{
				public:
					unsigned char key;
					unsigned char midichan;
				};
				typedef AEffect * (* PVSTMAIN) (audioMasterCallback audioMaster);
			public:
				/// Host callback dispatcher.
				static long int AudioMaster(AEffect * effect, long int opcode, long int index, long int value, void * ptr, float opt);
			public:
				plugin();
				virtual ~plugin() throw();
				virtual bool Load(RiffFile * pFile);
				virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
				virtual void SaveSpecificChunk(RiffFile* pFile) ;
				virtual void SaveDllName(RiffFile* pFile);
				bool LoadDll(std::string psFileName);
				// Loader for old psycle fileformat.
				bool LoadChunk(RiffFile* pFile);
				virtual const char * const GetDllName() const throw() { return _sDllName.c_str(); }
				virtual char * GetName() throw() { return (char*)_sProductName.c_str(); }
				virtual void SetSampleRate(int sr)	{	proxy().setSampleRate((float)sr); };

				void Instance(std::string dllname, const bool overwriteName = true) throw(...);
				void Free() throw(...);
				virtual int GetNumParams()
				{
					try
					{
						return proxy().numParams();
					}
					catch(const std::exception &)
					{
						// [bohan]
						// exception blocked here for now,
						// but we really should do something...
						//throw;
						return 0; /// \todo [bohan] ???
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
						// [bohan]
						// exception blocked here for now,
						// but we really should do something...
						//throw;
						std::strcpy(name, "fucked up"); /// \todo [bohan] ???
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
							return 0; /// \todo [bohan] ???
					}
					catch(const std::exception &)
					{
						// [bohan]
						// exception blocked here for now,
						// but we really should do something...
						//throw;
						return 0; /// \todo [bohan] ???
					}
				}
				#pragma warning(pop)

				bool DescribeValue(int parameter, char * psTxt);
				inline const long int & GetVersion() const throw() { return _version; }
				inline const char * const GetVendorName() const throw() { return _sVendorName.c_str(); }
				inline const bool & IsSynth() const throw() { return _isSynth; }
				bool AddMIDI(unsigned char data0, unsigned char data1 = 0, unsigned char data2 = 0);
				bool AddNoteOn(unsigned char channel, unsigned char key, unsigned char velocity, unsigned char midichannel = 0);
				bool AddNoteOff(unsigned char channel, unsigned char midichannel = 0, bool addatStart = false);
				inline void SendMidi();
				inline proxy & proxy() throw() { return *proxy_; };
				bool SetParameter(int parameter, float value);
				bool SetParameter(int parameter, int value);
				//void SetCurrentProgram(int prg);
				//int GetCurrentProgram();

				//\todo: this variable is just used in load/save. 
				unsigned char _program;
				//\todo: Having exception checking, this variable could be removed.
				bool instantiated;
				///\todo Remove when Changing the FileFormat.
				///      It is used in song load only. Probably it comes from an old fileformat ( 0.x )
				int _instance;
				/// It needs to use Process
				bool requiresProcess;
				/// It needs to use ProcessRepl
				bool requiresRepl;		
				CFrameWnd * editorWnd;

			protected:
				/// midi events queue, is sent to processEvents.
				note trackNote[MAX_TRACKS];
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
							// [bohan] it happens!!!
							if(note_on_count_[channel][note] < 0) psycle::host::loggers::warning("there was a note-off without corresponding note-on!");
						}
					private:
						// 16 channels, 128 keys
						short int note_on_count_[16][128];
					};
					note_checker note_checker_;
				#endif
			};

///////////////////////////////////////////////////////////////////////////////////
//		Class vst::instrument .
//		VST Instrument ( input plugin )
///////////////////////////////////////////////////////////////////////////////////
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
			};

///////////////////////////////////////////////////////////////////////////////////
//		Class vst::fx .
//		VST Effect ( effect plugin )
//		\todo :  [JAZ] i think this class isn't really needed anymore.
///////////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////////
//		Class vst::exceptions::dispatch_error
//		Exception handling.
///////////////////////////////////////////////////////////////////////////////////
			namespace exceptions
			{
				/// Exception caused by an error in a call to the vst dispatch function.
				class dispatch_error : public host::exceptions::function_error
				{
				public:
					inline dispatch_error(const std::string & what) : function_error(what) {}
				};

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

///////////////////////////////////////////////////////////////////////////////////
//		Inline Implementations.
///////////////////////////////////////////////////////////////////////////////////
			#if defined $catch$
				#error "macro clash"
			#endif
			#define $catch$(function) \
				catch(        std::exception const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
				catch(                  char const e[]) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
				catch(  signed long long int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
				catch(  signed      long int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
				catch(  signed           int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
				catch(  signed     short int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
				catch(  signed          char const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
				catch(unsigned long long int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
				catch(unsigned      long int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
				catch(unsigned           int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
				catch(unsigned     short int const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
				catch(unsigned          char const & e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
				catch(            void const * const e) { host::exceptions::function_errors::rethrow(host(), function, &e); } \
				catch(               ...              ) { host::exceptions::function_errors::rethrow<void*>(host(), function); }



			inline void proxy::operator()(AEffect * const plugin) throw(host::exceptions::function_error)
			{
				if(this->plugin_)
				{
					close();
				}
				// [magnus] we shouldn't delete plugin_ because the AEffect is allocated
				// by the plugin's DLL by some unknown means. Dispatching effClose will
				// automatically free up the AEffect structure.
				this->plugin_ = plugin;
				if(plugin)
				{
					try
					{
						// AEffect's resvd2 data member is right after the resvd1 data member in memory,
						// so, we can use those two 32-bit data members together as a single, potentially 64-bit, address
						*reinterpret_cast<vst::plugin**>(&plugin->resvd1) = &host();
					}
					$catch$("operator()(AEffect * const plugin)")
				}
			}

			#pragma warning(push)
			#pragma warning(disable:4702) // unreachable code

			inline long int proxy::dispatcher(long int operation, long int index, long int value, void * ptr, float opt) throw(host::exceptions::function_error)
			{
				#ifndef NDEBUG
				{
					std::ostringstream s; s
						<< "VST plugin: call to plugin dispatcher: plugin address: " << &plugin()
						<< ", opcode: " << exceptions::dispatch_errors::operation_description(operation)
						<< ", index: " << index
						<< ", value = " << value
						<< ", ptr = " << ptr
						<< ", opt = " << opt;
					host::loggers::trace(s.str());
				}
				#endif
				assert((*this)()); try { return plugin().dispatcher(&plugin(), operation, index, value, ptr, opt); } $catch$("dispatcher") return 0; /* dummy return to avoid warning */
			}
			inline long int proxy::magic() throw(host::exceptions::function_error)
			{ assert((*this)()); try { return plugin().magic; } $catch$("magic") return 0; /* dummy return to avoid warning */ }
			inline void proxy::process(float * * inputs, float * * outputs, long int sampleframes) throw(host::exceptions::function_error)
			{ assert((*this)()); try { plugin().process(&plugin(), inputs, outputs, sampleframes); } $catch$("process") }
			inline void proxy::processReplacing(float * * inputs, float * * outputs, long int sampleframes) throw(host::exceptions::function_error)
			{ assert((*this)()); try { plugin().processReplacing(&plugin(), inputs, outputs, sampleframes); } $catch$("processReplacing") }
			inline void proxy::setParameter(long int index, float parameter) throw(host::exceptions::function_error)
			{ assert((*this)()); try { plugin().setParameter(&plugin(), index, parameter); } $catch$("setParameter") }
			inline float proxy::getParameter(long int index) throw(host::exceptions::function_error)
			{ assert((*this)()); try { return plugin().getParameter(&plugin(), index); } $catch$("getParameter") return 0; /* dummy return to avoid warning */ }
			inline long int proxy::numPrograms() throw(host::exceptions::function_error)
			{ assert((*this)()); try { return plugin().numPrograms; } $catch$("numPrograms") return 0; /* dummy return to avoid warning */ }
			inline long int proxy::numParams() throw(host::exceptions::function_error)
			{ assert((*this)()); try { return plugin().numParams; } $catch$("numParams") return 0; /* dummy return to avoid warning */ }
			inline long int proxy::numInputs() throw(host::exceptions::function_error)
			{ assert((*this)()); try { return plugin().numInputs; } $catch$("numInputs") return 0; /* dummy return to avoid warning */ }
			inline long int proxy::numOutputs() throw(host::exceptions::function_error)
			{ assert((*this)()); try { return plugin().numOutputs; } $catch$("numOutputs") return 0; /* dummy return to avoid warning */ }
			inline long int proxy::flags() throw(host::exceptions::function_error)
			{ assert((*this)()); try { return plugin().flags; } $catch$("flags") return 0; /* dummy return to avoid warning */ }
			inline long int proxy::uniqueId() throw(host::exceptions::function_error)
			{ assert((*this)()); try { return plugin().uniqueID; } $catch$("uniqueId") return 0; /* dummy return to avoid warning */ }
			inline long int proxy::version() throw(host::exceptions::function_error)
			{ assert((*this)()); try { return plugin().version; } $catch$("version") return 0; /* dummy return to avoid warning */ }
			inline long int proxy::initialDelay() throw(host::exceptions::function_error)
			{ assert((*this)()); try { return plugin().initialDelay; } $catch$("initialDelay") return 0; /* dummy return to avoid warning */ }

			#pragma warning(pop)

			#undef $catch$
		}
	}
}

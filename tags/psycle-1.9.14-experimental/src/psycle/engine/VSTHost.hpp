///\file
///\brief interface file for psycle::host::VSTPlugin
#pragma once
#include <psycle/engine/constants.hpp>
#include <psycle/helpers/helpers.hpp>
#include <universalis/operating_system/exception.hpp>
#include <universalis/operating_system/exceptions/code_description.hpp>
#include <psycle/engine/machine.hpp>
//todo:
#include <psycle/host/uiglobal.hpp>

#define VST_FORCE_DEPRECATED 0
#include <vst/AEffectx.h>
namespace psycle
{
	namespace host
	{
		/// VST Host.
		namespace vst
		{
			/// Maximum number of Audio Input/outputs
			/// \todo : this shouldn't be a static value. Host should ask the plugin and the array get created dynamically.
			const int max_io = 16;
			/// Dialog max ticks for parameters.
			const int quantization = 65535;
			
			class plugin; // forward declaration

			//////////////////////////////////////////////////////////////////////////
			/// Proxy between the host and a plugin.
			//////////////////////////////////////////////////////////////////////////
			class proxy
			{
				public:
					proxy(vst::plugin & host, AEffect * const plugin = 0) : host_(host), plugin_(0) { (*this)(plugin); }
					~proxy() throw() { (*this)(0); }

				///\name reference to the host side
				///\{
					private:
						plugin const inline & host() const throw() { return host_; }
						plugin       inline & host() throw() { return host_; }
					private:
						plugin & host_;
				///\}

				///\name reference to the plugin side
				///\{
					public:
						AEffect const inline & plugin() const throw() { assert(plugin_); return *plugin_; }
						AEffect       inline & plugin()       throw() { assert(plugin_); return *plugin_; }
					private:
						AEffect              * plugin_;
					public:
						inline bool operator()() const throw() { return plugin_; }
						void operator()(AEffect * plugin) throw(host::exceptions::function_error);
				///\}

				///\name protected calls from the host side to the plugin side
				///\{
					public:
						long int inline magic()                                                                      throw(host::exceptions::function_error);
						void inline     process(float * * inputs, float * * outputs, long int sampleframes)          throw(host::exceptions::function_error);
						void inline     processReplacing(float * * inputs, float * * outputs, long int sampleframes) throw(host::exceptions::function_error);
						void inline     setParameter(long int index, float parameter)                                throw(host::exceptions::function_error);
						float inline    getParameter(long int index)                                                 throw(host::exceptions::function_error);
						long int inline numPrograms()                                                                throw(host::exceptions::function_error);
						long int inline numParams()                                                                  throw(host::exceptions::function_error);
						long int inline numInputs()                                                                  throw(host::exceptions::function_error);
						long int inline numOutputs()                                                                 throw(host::exceptions::function_error);
						//\todo: maybe exchange "flags()" with functions for each flag.
						long int inline flags()                                                                      throw(host::exceptions::function_error);
						long int inline uniqueId()                                                                   throw(host::exceptions::function_error);
						long int inline version()                                                                    throw(host::exceptions::function_error);
						long int inline initialDelay()                                                               throw(host::exceptions::function_error);
						long int inline dispatcher(long int operation, long int index = 0, long int value = 0, void * ptr = 0, float opt = 0) throw(host::exceptions::function_error);
				///\}

				//////////////////////////////////////////////////////////////////////////
				///\name AEffect "eff" OpCodes. Version 1.0
				//////////////////////////////////////////////////////////////////////////
				/// effSetProgramName:  user changed program name (max 24 char + 0) to as passed in string 
				/// effGetProgramName:  stuff program name (max 24 char + 0) into string 
				/// effGetParamLabel:   stuff parameter <index> label (max 8 char + 0) into string (examples: sec, dB, type)
				/// effGetParamDisplay: stuff parameter <index> textual representation into string (examples: 0.5, -3, PLATE)
				/// effGetParamName:    stuff parameter <index> label (max 8 char + 0) into string (examples: Time, Gain, RoomType) 
				/// effGetVu:           called if (flags & (effFlagsHasClip | effFlagsHasVu))
				///\{
				///\}

				//////////////////////////////////////////////////////////////////////////
				///\name AEffect "eff" OpCodes. Version 2.0
				//////////////////////////////////////////////////////////////////////////
				/// Turns on or off the plugin. If it is disabled, it won't produce output, but should behave without errors.
				/// effEditGetRect: stuff rect (top, left, bottom, right) into ptr
				/// effEditOpen:    system dependant Window pointer in ptr
				/// effEditClose:   no arguments
				/// effEditKey:     system keycode in value
				/// effEditIdle:    no arguments. Be gentle!
				/// effEditTop:     window has topped, no arguments
				/// effEditSleep:   window goes to background
				/// effIdentify:    returns 'NvEf'
				/// effGetChunk:    host requests pointer to chunk into (void**)ptr, byteSize returned
				/// effSetChunk:    plug-in receives saved chunk, byteSize passed
				///\{
				///\}

				///\name stuff
				///\{
					public:
						/// Create and initialize the VST plugin ( plugin Side ). Call this before using it. (except for string data)
						long int inline open() { return dispatcher(effOpen); }
						/// Destroys the VST plugin instance ( plugin side ). Also clears plugin_ pointer since it is no longer valid after effClose.
						long int inline close() { int retval = dispatcher(effClose); plugin_=0; return retval;	}
						long int inline getProgram() { return dispatcher(effGetProgram); }
						long int inline setProgram(int program) { assert(program >= 0); assert(program < numPrograms() || numPrograms() == 0); return dispatcher(effSetProgram, 0, program); }
						/// Tells the VST plugin the desired samplerate.
						long int inline setSampleRate(float sr) { assert(sr > 0); return dispatcher(effSetSampleRate,0,0,0,sr); }
						/// Tells the VST plugin the MAX block size of data that it will request. (default value for VST's is 1024)
						long int inline setBlockSize(int bs) { assert(bs>0); return dispatcher(effSetBlockSize,0,bs); }
						long int inline mainsChanged(bool on) { return dispatcher(effMainsChanged, 0, on ? 1 : 0); }
						/// Gets the VST implementation's Version that the plugin uses. ( 1.0,2.0,2.1,2.2 or 2.3)
						long int inline getVstVersion() { return dispatcher(effGetVstVersion); }
						long int inline getEffectName  (char * result) { assert(result); *result = 0; return dispatcher(effGetEffectName  , 0, 0, result); }
						long int inline getProductString  (char * result) { assert(result); *result = 0; return dispatcher(effGetProductString  , 0, 0, result); }
						long int inline getVendorString(char * result) { assert(result); *result = 0; return dispatcher(effGetVendorString, 0, 0, result); }
						long int inline setSpeakerArrangement(VstSpeakerArrangement* inputArrangement, VstSpeakerArrangement* outputArrangement)
						{
							assert(inputArrangement && outputArrangement);
							#if defined DIVERSALIS__COMPILER__MICROSOFT
								#pragma warning(push)
								#pragma warning(disable:4311) // 'reinterpret_cast' : pointer truncation from 'VstSpeakerArrangement *' to 'long'
							#endif
							BOOST_STATIC_ASSERT(sizeof(void*) == sizeof(long int));
							return dispatcher
								(
									effSetSpeakerArrangement, 0,
									reinterpret_cast<long int>(inputArrangement), // will break on 64-bit system ; the problem lies in steinberg's code itself ; can't fix
									outputArrangement
								);
							#if defined DIVERSALIS__COMPILER__MICROSOFT
								#pragma warning(pop)
							#endif
						}
				///\}
			};

			///////////////////////////////////////////////////////////////////////////////////
			/// Base Class VST plugin. Implements the functions common to VST Fx and VSTi's
			///////////////////////////////////////////////////////////////////////////////////
			class plugin : public Machine
			{
				///\name the multi-stepped construction
				///\{
					public:
						plugin(Machine::class_type subclass, Machine::mode_type mode, Machine::id_type id);
						bool LoadDll (std::string const & file_name, const bool overwriteName = true);
						void Instance(std::string const & file_name, const bool overwriteName = true);
						///< Helper class for Machine Creation.
						static Machine* CreateFromType(Machine::id_type _id, std::string _dllname);
				///\}

				///\name two-stepped destruction
				///\{
					public:
						void Free();
						virtual ~plugin() throw();
				///\}

				///\name general info
				///\{
					public:  virtual const std::string GetDllName() { return _sDllName; }
					private: std::string                 _sDllName;

					public:  virtual const std::string GetBrand() { return _sProductName; }
					private: std::string _sProductName;

					public: virtual const std::uint32_t GetCategory() { return 0; }
					public:  virtual const std::uint32_t GetVersion() { return static_cast<std::uint32_t>(_version); }
					private: long int                  _version;

					public: virtual const std::string GetVendorName() { return _sVendorName; }
					private: std::string                _sVendorName;

					public:  bool const inline &  IsSynth() const throw() { return _isSynth; }
					private: bool                _isSynth;

					public: virtual std::uint32_t GetCategory() const { return 0; }
				///\}

				///\name calls to the plugin side go thru the proxy
				///\{
						public:
							/// calls to the plugin side go thru the proxy
							vst::proxy inline & proxy() throw() { return *proxy_; };
						private:
							vst::proxy        * proxy_;
				///\}

				///\name calls from the plugin side go thru the AudioMaster
				///\{
					public:
						/// calls from the plugin side go thru the AudioMaster
						static long int AudioMaster(AEffect * effect, long int opcode, long int index, long int value, void * ptr, float opt);
				///\}

				///\name signal processing
				///\{
					public:
						virtual void SetSampleRate(int sr) { proxy().setSampleRate(static_cast<float>(sr)); };
					protected:
						float * inputs[max_io];
						float * outputs[max_io];
					PSYCLE__PRIVATE:
						/// It needs to use Process
						bool requiresProcess;
						/// It needs to use ProcessRepl
						bool requiresRepl;		
				///\}

				///\name midi & note event processing
				///\{
					public:
						bool AddMIDI(unsigned char data0, unsigned char data1 = 0, unsigned char data2 = 0);
						bool AddNoteOn(unsigned char channel, unsigned char key, unsigned char velocity, unsigned char midichannel = 0);
						bool AddNoteOff(unsigned char channel, unsigned char midichannel = 0, bool addatStart = false);
						inline void SendMidi();
					protected:
						/// reserves space for a new midi event in the queue.
						/// \return midi event to be filled in, or null if queue is full.
						VstMidiEvent* reserveVstMidiEvent();
						///\todo ugly hack
						VstMidiEvent* reserveVstMidiEventAtFront();
						VstMidiEvent midievent[MAX_VST_EVENTS];
						int	queue_size;
					private:
						class note
						{
							public:
								unsigned char key;
								unsigned char midichan;
						};
					protected:
						/// midi events queue, is sent to processEvents.
						note trackNote[MAX_TRACKS];
					private:
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
				///\}

				///\name parameters
				///\{
					public:
						//void SetCurrentProgram(int prg);
						//int GetCurrentProgram();
						virtual int  GetNumParams() { try { return proxy().numParams(); } catch(const std::exception &) { /* \todo we really should do something */ /* throw; */ return 0; } }
						virtual void GetParamName(int parameter, char * name)
						{
							try
							{
								if(parameter < proxy().numParams())
									proxy().dispatcher(effGetParamName, parameter, 0, name);
								else std::strcpy(name,"Out of Range");
							}
							catch(const std::exception &)
							{
								std::strcpy(name, "crashed");
							}
						}
						virtual void GetParamRange(int numparam, int & minval, int & maxval) { minval = 0; maxval = quantization; }
						bool DescribeValue(int parameter, char * psTxt);
						virtual void GetParamValue(int parameter, char * parval);
						#if defined DIVERSALIS__COMPILER__MICROSOFT
							#pragma warning(push)
							#pragma warning(disable:4702) // unreachable code
						#endif
						virtual int  GetParamValue(int parameter)
						{
							try
							{
								if(parameter < proxy().numParams())
									return f2i(proxy().getParameter(parameter) * quantization);
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
						#if defined DIVERSALIS__COMPILER__MICROSOFT
							#pragma warning(pop)
						#endif
						bool SetParameter(int parameter, float value);
						bool SetParameter(int parameter, int   value);
				///\}

				///\name (de)serialization
				///\{
					public:
						virtual bool LoadOldFileFormat(RiffFile * pFile);
						/// Loader for old psycle fileformat.
						bool LoadChunkOldFileFormat(RiffFile* pFile);
						virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
						virtual void SaveSpecificChunk(RiffFile* pFile) ;
						virtual void SaveDllName(RiffFile* pFile);
				///\}

				//////////////////////////////////////////////////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////////////////////////////
				// private, bazaar
				
				private:
					typedef AEffect * (* PVSTMAIN) (audioMasterCallback audioMaster);
					static VstTimeInfo _timeInfo;
					VstEvents mevents;
				protected:
					bool wantidle;
				PSYCLE__PRIVATE:
					CFrameWnd * editorWnd;
				private:
					HMODULE h_dll;
				private:
					///\todo what is this?
					float junk[MAX_BUFFER_LENGTH];
				PSYCLE__PRIVATE:
					///\todo: this variable is just used in load/save. 
					unsigned char _program;
					///\todo: Having exception checking, this variable could be removed.
					bool instantiated;
					///\todo Remove when Changing the FileFormat.
					/// It is used in song load only. Probably it comes from an old fileformat ( 0.x )
					int _instance;
			};

			///////////////////////////////////////////////////////////////////////////////////
			/// VST Instrument ( input plugin )
			///////////////////////////////////////////////////////////////////////////////////
			class instrument : public plugin
			{
			public:
				virtual void Work(int numSamples);
				instrument(Machine::id_type index);
				virtual void Tick(int channel, PatternEntry * pEntry);
				virtual void Stop(void);
			};

			///////////////////////////////////////////////////////////////////////////////////
			/// VST Effect ( effect plugin )
			///\todo :  [JAZ] i think this class isn't really needed anymore.
			///////////////////////////////////////////////////////////////////////////////////
			class fx : public plugin
			{
			public:
				virtual void Work(int numSamples);
				fx(Machine::id_type index);
				virtual ~fx() throw();
				virtual void Tick(int channel, PatternEntry * pEntry);
			protected:
				float * _pOutSamplesL;
				float * _pOutSamplesR;
			};

			///////////////////////////////////////////////////////////////////////////////////
			///	Exception handling.
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
						s << "Machine crashed: " << plugin.GetEditName();
						if(!plugin.GetDllName().empty())
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
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// inline implementations. we need to define body of inlined function after the class definition because of dependencies

namespace psycle
{
	namespace host
	{
		namespace vst
		{
			#if defined DIVERSALIS__COMPILER__MICROSOFT
				#pragma warning(push)
				#pragma warning(disable:4702) // unreachable code
			#endif

			long int inline proxy::magic()                                                                      throw(host::exceptions::function_error) { try { return plugin().magic;                                               } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
			void inline     proxy::process(float * * inputs, float * * outputs, long int sampleframes)          throw(host::exceptions::function_error) { try { plugin().process(&plugin(), inputs, outputs, sampleframes);          } PSYCLE__HOST__CATCH_ALL(host()) }
			void inline     proxy::processReplacing(float * * inputs, float * * outputs, long int sampleframes) throw(host::exceptions::function_error) { try { plugin().processReplacing(&plugin(), inputs, outputs, sampleframes); } PSYCLE__HOST__CATCH_ALL(host()) }
			void inline     proxy::setParameter(long int index, float parameter)                                throw(host::exceptions::function_error) { try { plugin().setParameter(&plugin(), index, parameter);                  } PSYCLE__HOST__CATCH_ALL(host()) }
			float inline    proxy::getParameter(long int index)                                                 throw(host::exceptions::function_error) { try { return plugin().getParameter(&plugin(), index);                      } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
			long int inline proxy::numPrograms()                                                                throw(host::exceptions::function_error) { try { return plugin().numPrograms;                                         } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
			long int inline proxy::numParams()                                                                  throw(host::exceptions::function_error) { try { return plugin().numParams;                                           } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
			long int inline proxy::numInputs()                                                                  throw(host::exceptions::function_error) { try { return plugin().numInputs;                                           } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
			long int inline proxy::numOutputs()                                                                 throw(host::exceptions::function_error) { try { return plugin().numOutputs;                                          } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
			//\todo: maybe exchange "flags()" with functions for each flag.
			long int inline proxy::flags()                                                                      throw(host::exceptions::function_error) { try { return plugin().flags;                                               } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
			long int inline proxy::uniqueId()                                                                   throw(host::exceptions::function_error) { try { return plugin().uniqueID;                                            } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
			long int inline proxy::version()                                                                    throw(host::exceptions::function_error) { try { return plugin().version;                                             } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
			long int inline proxy::initialDelay()                                                               throw(host::exceptions::function_error) { try { return plugin().initialDelay;                                        } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */ }
			long int inline proxy::dispatcher(long int operation, long int index, long int value, void * ptr, float opt) throw(host::exceptions::function_error)
			{
				#ifndef NDEBUG
					switch(operation)
					{
						case effEditIdle: break; // floods
						default:
							{
								std::ostringstream s;
								s
									<< "VST: host call to plugin dispatcher: plugin address: " << &plugin()
									<< ", opcode: " << exceptions::dispatch_errors::operation_description(operation)
									<< ", index: " << index
									<< ", value = " << value
									<< ", ptr = " << ptr
									<< ", opt = " << opt;
								host::loggers::trace(s.str());
							}
					}
				#endif
				try { return plugin().dispatcher(&plugin(), operation, index, value, ptr, opt); } PSYCLE__HOST__CATCH_ALL(host()) return 0; /* dummy return to avoid warning */
			}
			#if defined DIVERSALIS__COMPILER__MICROSOFT
				#pragma warning(pop)
			#endif
		}
	}
}

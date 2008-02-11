// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief implementation file for psycle::host::plugin
#include <project.private.hpp>
#include "psycle.hpp"
#include "MainFrm.hpp"
#include "VstEditorDlg.hpp"
#include "VstHost.hpp"
#include "song.hpp"
//#include "Configuration.hpp"
#include "Player.hpp"
#include "InputHandler.hpp"
#include "NewMachine.hpp"
#include <algorithm>
#include <cctype>
namespace psycle
{
	namespace host
	{
		extern CPsycleApp theApp;

		namespace vst
		{
			namespace exceptions
			{
				namespace dispatch_errors
				{
					std::string eff_opcode_to_string(long opcode) throw()
					{
						switch(opcode)
						{
							#if defined $
								#error "macro clash"
							#endif
							#define $(code) case eff##code: return "eff"#code;

							// from AEffect.h
							$(Open) $(Close)
							$(SetProgram) $(GetProgram)
							$(SetProgramName) $(GetProgramName)
							$(GetParamLabel) $(GetParamDisplay) $(GetParamName)
							$(GetVu)
							
							// system
							$(SetSampleRate) $(SetBlockSize) $(MainsChanged)
	
							// editor
							$(EditGetRect) $(EditOpen) $(EditClose) $(EditDraw) $(EditMouse) $(EditKey) $(EditIdle) $(EditTop) $(EditSleep)
	
							// other
							$(Identify) $(GetChunk) $(SetChunk)

							// from aeffectx.h

							// VstEvents
							$(ProcessEvents)

							// parameters and programs
							$(CanBeAutomated) $(String2Parameter) $(GetNumProgramCategories) $(GetProgramNameIndexed) $(CopyProgram)

							// connections, configuration
							$(ConnectInput) $(ConnectOutput) $(GetInputProperties) $(GetOutputProperties) $(GetPlugCategory)

							// realtime
							$(GetCurrentPosition) $(GetDestinationBuffer)

							// offline
							$(OfflineNotify) $(OfflinePrepare) $(OfflineRun)

							// other
							$(ProcessVarIo) $(SetSpeakerArrangement) $(SetBlockSizeAndSampleRate) $(SetBypass)
							$(GetEffectName) $(GetVendorString) $(GetProductString) $(GetVendorVersion) $(VendorSpecific)
							$(CanDo) $(GetTailSize) $(Idle) $(GetErrorText)
							
							// gui
							$(GetIcon) $(SetViewPosition)

							// and...
							$(GetParameterProperties) $(KeysRequired) $(GetVstVersion)
							
							// vst 2.1
							$(EditKeyDown) $(EditKeyUp) $(SetEditKnobMode) $(GetMidiKeyName)
							$(BeginSetProgram) $(EndSetProgram)
							$(GetMidiProgramName) $(GetCurrentMidiProgram) $(GetMidiProgramCategory) $(HasMidiProgramsChanged)

							// vst2.3
							$(GetSpeakerArrangement) $(ShellGetNextPlugin) $(StartProcess) $(StopProcess)
							$(SetTotalSampleToProcess) $(SetPanLaw) $(BeginLoadBank) $(BeginLoadProgram)

							#undef $
							
							default:
							{
								std::ostringstream s;
								s << "unknown opcode " << opcode;
								return s.str();
							}
						}
					}
					const std::string operation_description(long code) throw()
					{
						std::ostringstream s; s << code << ": " << eff_opcode_to_string(code);
						return s.str();
					}
				}
			}

			VstTimeInfo plugin::_timeInfo;

			plugin::plugin()
				: queue_size(0), wantidle(false), _sDllName(""), h_dll(0), _program(0), instantiated(false), _instance(0)
				, requiresProcess(false), requiresRepl(false), _version(0), _isSynth(false), proxy_(0), editorWnd(0)
			{
				proxy_ = new vst::proxy(*this);
				std::memset(junk, 0, STREAM_SIZE * sizeof(float));
				for(int i(0) ; i < vst::max_io ; ++i)
				{
					inputs[i]=junk;
					outputs[i]=junk;
				}
				outputs[0] = _pSamplesL;
				outputs[1] = _pSamplesR;
				_sProductName[0]=0;
				_sVendorName[0]=0;
				for(int i(0) ; i < MAX_TRACKS; ++i)
				{
					trackNote[i].key = 255; // No Note.
					trackNote[i].midichan = 0;
				}
			}

			plugin::~plugin() throw()
			{
				try
				{
					Free();
				}
				catch(...)
				{
					// wow.. cannot do anything in a destructor
					assert(false);
				}
				zapObject(proxy_);
			}

			void plugin::Instance(std::string dllname, bool overwriteName) throw(...)
			{
				Free();
				_sDllName=dllname;
				TRACE("VST plugin: library file name: %s\n", _sDllName.c_str());
				proxy()(0);
				instantiated = false;
				if(!(h_dll = ::LoadLibrary(_sDllName.c_str())))
				{
					std::ostringstream s; s
						<< "could not load library: " << dllname << std::endl
						<< operating_system::exceptions::code_description();
					throw host::exceptions::library_errors::loading_error(s.str());
				}
				PVSTMAIN main(reinterpret_cast<PVSTMAIN>(::GetProcAddress(h_dll, "main")));
				if(!main)
				{	
					std::ostringstream s; s
						<< "library is not a VST plugin:" << std::endl
						<< "could not resolve symbol 'main' in library: " << dllname << std::endl
						<< operating_system::exceptions::code_description();
					throw host::exceptions::library_errors::symbol_resolving_error(s.str());
				}
				// 1: calls the "main" function and receives the pointer to the AEffect structure.
				{
					AEffect * effect(0);
					try 
					{
						effect=main(reinterpret_cast<audioMasterCallback>(&AudioMaster));
					}
					catch(std::exception const & e) { host::exceptions::function_errors::rethrow(*this, "main", &e); }
					catch(...) { host::exceptions::function_errors::rethrow<void*>(*this, "main"); }
					proxy()(effect);
				}
				if(!proxy()() || proxy().magic() != kEffectMagic)
				{
					std::ostringstream s;
					s << "call to function 'main' returned a bad value: ";
					if(proxy()()) s << "returned value signature: " << proxy().magic();
					else s << "returned value is a null pointer";
					throw host::exceptions::function_errors::bad_returned_value(s.str());
				}
				TRACE("VST plugin: instanciated.");
				// 2: Host to Plug, setSampleRate ( 44100.000000 )
				proxy().setSampleRate((float) Global::pConfig->GetSamplesPerSec());
				// 3: Host to Plug, setBlockSize ( 512 ) 
				proxy().setBlockSize(STREAM_SIZE);
				// 4: Host to Plug, open
				{
					//init plugin (probably a call to "Init()" function should be done here)
					proxy().open();
				}

				// 5: Host to Plug, setSpeakerArrangement returned: false 
				VstSpeakerArrangement VSTsa;
				{
					VSTsa.type = kSpeakerArrStereo;
					VSTsa.numChannels = 2;
					VSTsa.speakers[0].type = kSpeakerL;
					VSTsa.speakers[1].type = kSpeakerR;
					proxy().setSpeakerArrangement(&VSTsa,&VSTsa);
				}

				// 6: Host to Plug, setSampleRate ( 44100.000000 ) 
				proxy().setSampleRate((float) Global::pConfig->GetSamplesPerSec());
				// 7: Host to Plug, setBlockSize ( 512 ) 
				proxy().setBlockSize(STREAM_SIZE);
				// 8: Host to Plug, setSpeakerArrangement returned: false 
				proxy().setSpeakerArrangement(&VSTsa,&VSTsa);
				// 9: Host to Plug, setSampleRate ( 44100.000000 ) 
				proxy().setSampleRate((float) Global::pConfig->GetSamplesPerSec());
				// 10: Host to Plug, setBlockSize ( 512 ) 
				proxy().setBlockSize(STREAM_SIZE);
				// 11: Host to Plug, getProgram returned: 0 
				
				long int program = proxy().getProgram();
				// 12: Host to Plug, getProgram returned: 0 
				program = proxy().getProgram();
				// 13: Host to Plug, getVstVersion returned: 2300 
				{
					_version = proxy().getVstVersion();
					if(!_version) _version = 1;
				}
				// 14: Host to Plug, getProgramNameIndexed ( -1 , 0 , ptr to char ) 
				proxy().setProgram(0);
				proxy().mainsChanged(true);
				{
					char temp[32];
					/// [bohan] \todo ProductName is a missleading name
					/// confusion possible with: effGetProductString, // fills <ptr> with a string with product name (max 64 char)
					if(proxy().getEffectName(temp) && temp[0]) _sProductName=temp;
					else
					{
						std::string temp;
						std::string::size_type pos;
						pos = _sDllName.rfind('\\');
						if(pos==std::string::npos)
							temp=_sDllName;
						else
							temp=_sDllName.substr(pos+1);
						_sProductName=temp.substr(0,temp.rfind('.'));
					}
				}
				{
					if(overwriteName) strncpy(_editName, _sProductName.c_str(), 32);
					_editName[31]='\0';
				}
				// Compatibility hacks
				{
					if(_sProductName == "sc-101")
					{
						requiresRepl = true;
					}
				}
				{
					char temp[64];
					if(proxy().getVendorString(temp) && temp[0]) _sVendorName = temp;
					else _sVendorName = "Unknown vendor";
				}
				_isSynth = proxy().flags() & effFlagsIsSynth;
				instantiated = true;
				TRACE("VST plugin: successfully instanciated. inputs: %d, outputs: %d\n", proxy().numInputs(), proxy().numOutputs());
			}

			void plugin::Free() throw(...) // called also in destructor
			{
				TRACE("VST plugin: freeing ...");
				const std::exception * exception(0);
				if(proxy()())
				{
					assert(h_dll);
					TRACE("VST plugin: freeing ... dispatcher");
					try
					{
						proxy().mainsChanged(false);
					}
					catch(const std::exception & e)
					{
						if(!exception)
						{
							std::ostringstream s; s
								<< typeid(e).name() << std::endl
								<< e.what();
							exception = new host::exceptions::function_error(s.str());
						}
					}
					try
					{
						proxy().close(); // frees the AEffect too
					}
					catch(const std::exception & e)
					{
						if(!exception)
						{
							std::ostringstream s; s
								<< typeid(e).name() << std::endl
								<< e.what();
							exception = new host::exceptions::function_error(s.str());
						}
					}
				}
				if(h_dll)
				{
					assert(!proxy()());
					TRACE("VST plugin: freeing ... library");
					try
					{
						::FreeLibrary(h_dll);
					}
					catch(...)
					{
						h_dll = 0;
						throw; // [bohan] magnus, does it cause any problem to rethrow?
					}
					h_dll = 0;
				}
				instantiated = false;
				if(exception) throw *exception;
			}
			
			bool plugin::LoadSpecificChunk(RiffFile * pFile, int version)
			{
				UINT size;
				pFile->Read(&size, sizeof size );
				if(size)
				{
					if(version > CURRENT_FILE_VERSION_MACD)
					{
						pFile->Skip(size);
						std::ostringstream s; s
							<< version << " > " << CURRENT_FILE_VERSION_MACD << std::endl
							<< "Data is from a newer format of psycle, it might be unsafe to load." << std::endl;
						MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
						return false;
					}
					pFile->Read(&_program, sizeof _program);
					// set the program
					try
					{
						proxy().dispatcher(effSetProgram, 0, _program);
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					UINT count;
					pFile->Read(&count, sizeof count);
					for(UINT i(0) ; i < count ; ++i)
					{
						float temp;
						pFile->Read(&temp, sizeof temp);
						SetParameter(i, temp);
					}
					size -= sizeof _program + sizeof count + sizeof(float) * count;
					if(size)
					{
						// [bohan] why don't we just call LoadChunk?
						// [bohan] hmm, shit, LoadCunk actually reads the chunk size too.
						if(proxy().flags() & effFlagsProgramChunks)
						{
							char * data(new char[size]);
							pFile->Read(data, size); // Number of parameters
							try 
							{
								proxy().dispatcher(effSetChunk, 0, size, data);
							}
							catch(...)
							{
								// [bohan] hmm, so, data just gets lost?
								zapArray(data);
								return false;
							}
							zapArray(data);
						}
						else
						{
							// there is a data chunk, but this machine does not want one.
							pFile->Skip(size);
							return false;
						}
					}

				}
				return true;
			};


			void plugin::SaveDllName(RiffFile * pFile) 
			{
				::CString str = GetDllName();
				char str2[1 << 10];
				std::strcpy(str2, str.Mid(str.ReverseFind('\\') + 1));
				pFile->Write(str2, std::strlen(str2) + 1);
			};

			void plugin::SaveSpecificChunk(RiffFile * pFile) 
			{
				UINT count(GetNumParams());
				UINT size(sizeof _program + sizeof count + sizeof(float) * count);
				char * pData(0);
				bool b;
				try
				{
					b = proxy().flags() & effFlagsProgramChunks;
				}
				catch(const std::exception &)
				{
					b = false;
				}
				if(b)
				{
					try 
					{
						size += proxy().dispatcher(effGetChunk, 0, 0, &pData);
					}
					catch(const std::exception &)
					{
						// [bohan]
						// i think it's not necessary to set the size to the same value again.
						// if an exception is thrown, size won't be changed.
						size = sizeof _program  + sizeof count  + sizeof(float) * count;
					}
				}
				pFile->Write(&size, sizeof size);
				_program = proxy().getProgram();
				pFile->Write(&_program, sizeof _program);
				pFile->Write(&count, sizeof count);
				for(UINT i(0); i < count; ++i)
				{
					float temp;
					try
					{
						temp = proxy().getParameter(i);
					}
					catch(const std::exception &)
					{
						temp = 0; // hmm
					}
					pFile->Write(&temp, sizeof temp);
				}
				size -= sizeof _program + sizeof count + sizeof(float) * count;
				if(size > 0)
				{
					pFile->Write(pData, size);
				}
			};

			bool plugin::LoadDll(std::string psFileName)
			{
				std::transform(psFileName.begin(),psFileName.end(),psFileName.begin(),std::tolower);
				std::string sPath2;
				::CString sPath;
				if(CNewMachine::lookupDllName(psFileName, sPath2))
				{
					if(!CNewMachine::TestFilename(sPath2)) return false;
					try
					{
						Instance(sPath2.c_str(), false);
					}
					catch(const std::exception & e)
					{
						std::ostringstream s; s
							<< "Exception while instanciating:" << sPath2 << std::endl
							<< "Replacing with dummy." << std::endl
							<< typeid(e).name() << std::endl
							<< e.what();
						MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
						return false;
					}
					catch(...)
					{
						std::ostringstream s; s
							<< "Exception while instanciating:" << sPath2 << std::endl
							<< "Replacing with dummy." << std::endl
							<< "Unkown type of exception";
						MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
						return false;
					}
				}
				else
				{
					std::ostringstream s; s
						<< "Missing:" << psFileName << std::endl
						<< "Replacing with dummy.";
					MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
					return false;
				}
				return true;
			};

			void plugin::GetParamValue(int numparam, char * parval)
			{
				try
				{
					if(numparam < proxy().numParams())
					{
						if(!DescribeValue(numparam, parval))
						{
							std::sprintf(parval,"%.0f",proxy().getParameter(numparam) * quantization);
						}
					}
					else std::strcpy(parval,"Out of Range");
				}
				catch(const std::exception &)
				{
					// [bohan]
					// exception blocked here for now,
					// but we really should do something...
					//throw;
					std::strcpy(parval, "fucked up");
				}
			}

			bool plugin::DescribeValue(int parameter, char * psTxt)
			{
				if(instantiated)
				{
					bool b;
					try
					{
						b = parameter < proxy().numParams();
					}
					catch(const std::exception &)
					{
						b = false;
					}
					if(b)
					{
						char par_display[64]={0};
						char par_label[64]={0};
						try
						{
							proxy().dispatcher(effGetParamDisplay, parameter, 0, par_display);
						}
						catch(const std::exception &)
						{
							// [bohan]
							// exception blocked here for now,
							// but we really should do something...
							//throw;
							std::strcpy(par_display, "fucked up");
						}
						try
						{
							proxy().dispatcher(effGetParamLabel, parameter, 0, par_label);
						}
						catch(const std::exception &)
						{
							// [bohan]
							// exception blocked here for now,
							// but we really should do something...
							//throw;
							std::strcpy(par_label, "fucked up");
						}
						std::sprintf(psTxt, "%s(%s)", par_display, par_label);
						return true; // maybe we could return false on error instead of writing errors in the description
					}
					else std::sprintf(psTxt, "Invalid NumParams Value");
				}
				else std::sprintf(psTxt, "Not loaded"); // [bohan] wow.. can this fucked up situtation really happens?
				return false;
			}

			bool plugin::SetParameter(int parameter, float value)
			{
				if(!instantiated) return false;
				bool b;
				try
				{
					b = parameter < proxy().numParams();
				}
				catch(const std::exception &)
				{
					b = false;
				}
				if(!b) return false;
				try
				{
					proxy().setParameter(parameter, value);
				}
				catch(const std::exception &)
				{
					return false;
				}
				return true;
			}

			bool plugin::SetParameter(int parameter, int value)
			{
				return SetParameter(parameter, value / 65535.0f);
			}

			/*
			int plugin::GetCurrentProgram()
			{
				if(instantiated)
				{
					try
					{
						return proxy().dispatcher(effGetProgram);
					}
					catch(const std::exception &)
					{
						return 0; // [bohan] well, what to return if it fails? 0 is wrong..
					}
				}
				else
				{
					return 0;
				}
			}

			void plugin::SetCurrentProgram(int prg)
			{
				if(instantiated)
				{
					try
					{
						proxy().dispatcher(effSetProgram, 0, prg);
					}
					catch(const std::exception &)
					{
						// o_O`
					}
				}
			}
			*/

			VstMidiEvent* plugin::reserveVstMidiEvent() {
				assert(queue_size>=0 && queue_size <= MAX_VST_EVENTS);
				if(queue_size >= MAX_VST_EVENTS) {
					loggers::info("vst::plugin warning: event buffer full, midi message could not be sent to plugin");
					return NULL;
				}
				return &midievent[queue_size++];
			}
			
			VstMidiEvent* plugin::reserveVstMidiEventAtFront() {
				assert(queue_size>=0 && queue_size <= MAX_VST_EVENTS);
				if(queue_size >= MAX_VST_EVENTS) {
					loggers::info("vst::plugin warning: event buffer full, midi message could not be sent to plugin");
					return NULL;
				}
				for(int i=queue_size; i > 0 ; --i) midievent[i] = midievent[i - 1];
				queue_size++;
				return &midievent[0];
			}


			bool plugin::AddMIDI(unsigned char data0, unsigned char data1, unsigned char data2)
			{
				if(!instantiated) return false;
				VstMidiEvent * pevent(reserveVstMidiEvent());
				if(!pevent) return false;
				pevent->type = kVstMidiType;
				pevent->byteSize = 24;
				pevent->deltaFrames = 0;
				pevent->flags = 0;
				pevent->detune = 0;
				pevent->noteLength = 0;
				pevent->noteOffset = 0;
				pevent->reserved1 = 0;
				pevent->reserved2 = 0;
				pevent->noteOffVelocity = 0;
				pevent->midiData[0] = data0;
				pevent->midiData[1] = data1;
				pevent->midiData[2] = data2;
				pevent->midiData[3] = 0;
				return true;
			}
			bool plugin::AddNoteOn(unsigned char channel, unsigned char key, unsigned char velocity, unsigned char midichannel)
			{
				if(instantiated)
				{
					if(trackNote[channel].key != 255)
						AddNoteOff(channel, trackNote[channel].key, true);

					if(AddMIDI(0x90 | midichannel /*Midi On*/, key, velocity)) {
						note thisnote;
						thisnote.key = key;
						thisnote.midichan = midichannel;
						trackNote[channel] = thisnote;
						return true;
					}
				}
				return false;
			}

			bool plugin::AddNoteOff(unsigned char channel, unsigned char midichannel, bool addatStart)
			{
				if(!instantiated)
					return false;
				if(trackNote[channel].key == 255)
					return false;
				VstMidiEvent * pevent;
				if( addatStart)
				{
					// PATCH:
					// When a new note enters, it adds a note-off for the previous note playing in
					// the track (this is ok). But if you have like: A-4 C-5 and in the next line
					// C-5 E-5 , you will only hear E-5.
					// Solution: Move the NoteOffs at the beginning.
					pevent = reserveVstMidiEventAtFront();
				}
				else 
				{
					pevent = reserveVstMidiEvent();
				}
				if(!pevent)
					return false;
				pevent->type = kVstMidiType;
				pevent->byteSize = 24;
				pevent->deltaFrames = 0;
				pevent->flags = 0;
				pevent->detune = 0;
				pevent->noteLength = 0;
				pevent->noteOffset = 0;
				pevent->reserved1 = 0;
				pevent->reserved2 = 0;
				pevent->noteOffVelocity = 0;
				pevent->midiData[0] = 0x80 | static_cast<unsigned char>(trackNote[channel].midichan); //midichannel; // Midi Off
				pevent->midiData[1] = trackNote[channel].key;
				pevent->midiData[2] = 0;
				pevent->midiData[3] = 0;

				note thisnote;
				thisnote.key = 255;
				thisnote.midichan = 0;
				trackNote[channel] = thisnote;
				return true;
			}
			void plugin::SendMidi()
			{
				assert(queue_size >= 0 && queue_size <= MAX_VST_EVENTS);

				if(instantiated && queue_size > 0)
				{
					// Prepare MIDI events and free queue dispatching all events
					mevents.numEvents = queue_size;
					mevents.reserved = 0;
					for(int q(0) ; q < queue_size ; ++q) {
#ifndef NDEBUG

						// assert that events are sent in order.
						// although the standard doesn't require this,
						// many synths rely on this.
						if(q>0) {
							assert(midievent[q-1].deltaFrames <= 
								midievent[q].deltaFrames);
						}

						// assert that the note sequence is well-formed,
						// which means, no note-offs happen without a
						// corresponding preceding note-on.
						switch(midievent[q].midiData[0]&0xf0) {
						case 0x90: // note-on
							note_checker_.note_on(midievent[q].midiData[1],
								midievent[q].midiData[0]&0x0f);
							break;
						case 0x80: // note-off
							note_checker_.note_off(midievent[q].midiData[1],
								midievent[q].midiData[0]&0x0f);
							break;
						}
#endif
						mevents.events[q] = (VstEvent*) &midievent[q];
					}

					queue_size = 0;
					//Finally Send the events.
					try
					{
						proxy().dispatcher(effProcessEvents, 0, 0, &mevents);
					}
					catch(const std::exception &)
					{
						assert(false);
						//throw;
					}
				}
			}

			#if !defined NDEBUG
				static std::string audioMaster_opcode_to_string(long opcode)
				{
					#if defined $
						#error "macro clash"
					#else
						#define $(code) case audioMaster##code: return "audioMaster"#code;
						switch(opcode)
						{
							// from AEffect.h
							$(Automate) $(Version) $(CurrentId) $(Idle) $(PinConnected)

							// from AEffectX.h
							$(GetTime) $(SetTime)

							$(OfflineStart) $(OfflineRead) $(OfflineWrite) $(OfflineGetCurrentPass) $(OfflineGetCurrentMetaPass)

							$(GetVendorString) $(GetProductString) $(GetVendorVersion) $(VendorSpecific)

							$(OpenWindow) $(CloseWindow)

							$(SetOutputSampleRate)
							$(GetSampleRate)
							$(GetBlockSize)
							$(GetInputLatency) $(GetOutputLatency)

							$(GetParameterQuantization)

							$(GetOutputSpeakerArrangement)

							$(GetPreviousPlug) $(GetNextPlug)

							$(WantMidi)
							$(ProcessEvents)
							$(TempoAt)
							$(GetNumAutomatableParameters)
							$(IOChanged)
							$(NeedIdle)
							$(SizeWindow)
							$(WillReplaceOrAccumulate)
							$(GetCurrentProcessLevel)
							$(GetAutomationState)
							$(SetIcon)
							$(CanDo)
							$(GetLanguage)
							$(GetDirectory)
							$(UpdateDisplay)

							// vst 2.1

							$(BeginEdit) $(EndEdit)
							$(OpenFileSelector)

							// vst 2.2

							$(CloseFileSelector)
							$(EditFile)
							$(GetChunkFile)

							// vst 2.3
							$(GetInputSpeakerArrangement)
						default:
							{
								std::ostringstream s;
								s << "unknown opcode " << opcode;
								return s.str();
							}
						}
						#undef $
					#endif
				}
			#endif

			long int plugin::AudioMaster(AEffect * effect, long opcode, long index, long value, void *ptr, float opt)
			{
				#if !defined NDEBUG
					if(opcode!=audioMasterGetTime)
					{
						std::ostringstream s;
						s<< "VST plugin: call to host dispatcher: Eff: " << effect
						<< " Opcode = " << audioMaster_opcode_to_string(opcode)
						<< " Index = " << index
						<< " Value = " << value
						<< " Ptr = " << ptr
						<< " Opt = " << opt;
						host::loggers::trace(s.str());
					}
				#endif
				
				// believe it or not, some plugs tried to call psycle with a null AEffect.
				plugin * host(effect ? *reinterpret_cast<plugin**>(&effect->resvd1) : 0);
				
				// Support opcodes
				switch(opcode)
				{
				//  Version 1.0
				case audioMasterAutomate:
					Global::pPlayer->Tweaker = true;
					if(effect && host)
					{
						if(index<0 || index >= effect->numParams) {
							host::loggers::info("error audioMasterAutomate: index<0 || index >= effect->numParams");
						}
						if(Global::pConfig->_RecordTweaks)  
						{
							if(Global::pConfig->_RecordMouseTweaksSmooth)
								((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweakSlide(host->_macIndex, index, f2i(opt * vst::quantization));
							else
								((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweak(host->_macIndex, index, f2i(opt * vst::quantization));
						}
						if(host->editorWnd)
							((CVstEditorDlg *) host->editorWnd)->Refresh(index, opt);
					}
					return 0; // index, value, returns 0
				case audioMasterVersion:			
					return 9; // vst version, currently 7 (0 for older)
				case audioMasterCurrentId:			
					break;
				case audioMasterIdle: // call application idle routine (this will call effEditIdle for all open editors too) 
					if(effect && host)
					{
						try
						{
							host->proxy().dispatcher(effEditIdle);
						}
						catch(const std::exception &)
						{
							// o_O`
						}
					}
					return 0; 
				case audioMasterPinConnected:
					if(value == 0) // input
					{
						if(index < 2) return 0; // 0 means connected, 1 disconnected.
						else return 1;
					}
					else // output
					{
						if(index < 2) return 0;
						else return 1;
					}
				//  Version 2.0
				case audioMasterGetTime:
					std::memset(&_timeInfo, 0, sizeof _timeInfo);
					/*
					kVstTransportChanged 		= 1,		// Indicates that Playing, Cycle or Recording has changed
					kVstTransportPlaying 		= 1 << 1,
					kVstTransportCycleActive	= 1 << 2,
					kVstTransportRecording		= 1 << 3,

					kVstAutomationWriting		= 1 << 6,
					kVstAutomationReading		= 1 << 7,

					// flags which indicate which of the fields in this VstTimeInfo
					//  are valid; samplePos and sampleRate are always valid
					kVstNanosValid  			= 1 << 8,
						double nanoSeconds;			// system time
					kVstPpqPosValid 			= 1 << 9,
						double ppqPos;				// 1 ppq
					kVstTempoValid				= 1 << 10,
						double tempo;				// in bpm
					kVstBarsValid				= 1 << 11,
						double barStartPos;			// last bar start, in 1 ppq
					kVstCyclePosValid			= 1 << 12,	// start and end
						double cycleStartPos;		// 1 ppq
						double cycleEndPos;			// 1 ppq
					kVstTimeSigValid 			= 1 << 13,
						long timeSigNumerator;		// time signature
						long timeSigDenominator;
					kVstSmpteValid				= 1 << 14,
						long smpteOffset;
						long smpteFrameRate;		// 0:24, 1:25, 2:29.97, 3:30, 4:29.97 df, 5:30 df
					kVstClockValid 				= 1 << 15
						long samplesToNextClock;	// midi clock resolution (24 ppq), can be negative
					*/
					if((Global::pPlayer)->_playing) 
					{
						_timeInfo.flags |= kVstTransportPlaying;
						if(((Master *) (Global::_pSong->_pMachine[MASTER_INDEX]))->sampleCount == 0)
							_timeInfo.flags |= kVstTransportChanged;
					}
					if(Global::_pSong->_pMachine[MASTER_INDEX]) // This happens on song loading with new fileformat.
					{
						_timeInfo.samplePos = ((Master *) (Global::_pSong->_pMachine[MASTER_INDEX]))->sampleCount;
					}
					else _timeInfo.samplePos = 0;
					_timeInfo.sampleRate = Global::pConfig->GetSamplesPerSec();
					
					/* WTF?! error C3861: 'timeGetTime': identifier not found, even with argument-dependent lookup
					if(value & kVstNanosValid)
					{
						_timeInfo.flags |= kVstNanosValid;
						_timeInfo.nanoSeconds = timeGetTime();
					}
					*/
					if(value & kVstPpqPosValid)
					{
						_timeInfo.flags |= kVstPpqPosValid;
						// Code in "else" is preferable. code in if = old code
						/*
						if((Global::pPlayer)->_playing) 
						{
							const float currentline = (float)(Global::pPlayer->_lineCounter%(Global::pPlayer->tpb * 4)) / Global::pPlayer->tpb;
							const float linestep = (((float)(Global::_pSong->SamplesPerRow()-Global::pPlayer->_ticksRemaining)) / Global::_pSong->SamplesPerTick) / Global::pPlayer->tpb;
							_timeInfo.ppqPos = currentline+linestep;
						}
						else
						{
						*/
							//const double ppq = ((((Master*)(Global::_pSong->_pMachine[MASTER_INDEX]))->sampleCount/ _timeInfo.sampleRate ) * (Global::pPlayer->bpm / 60.0f));
							//_timeInfo.ppqPos =  ppq - 4*((int)ppq/4);
							_timeInfo.ppqPos = (((Master *) (Global::_pSong->_pMachine[MASTER_INDEX]))->sampleCount * Global::pPlayer->bpm ) / (_timeInfo.sampleRate* 60.0f);
						//}
					}
					if(value & kVstTempoValid)
					{
						_timeInfo.flags |= kVstTempoValid;
						_timeInfo.tempo = Global::pPlayer->bpm;
					}
					if(value & kVstTimeSigValid)
					{
						_timeInfo.flags |= 	kVstTimeSigValid;
						_timeInfo.timeSigNumerator = 4;
						_timeInfo.timeSigDenominator = 4;
					}
					return (long) &_timeInfo;
				case audioMasterProcessEvents:		
					return 0; // Support of vst events to host is not available
				case audioMasterSetTime:
					//IGNORE!
					break;
				case audioMasterTempoAt:
					// This might be incorrect:
					// Declaration: virtual long tempoAt (long pos); // returns tempo (in bpm * 10000) at sample frame location <pos>
					return Global::pPlayer->bpm * 10000;
				case audioMasterGetParameterQuantization:	
					return vst::quantization;
				case audioMasterNeedIdle:
					if(effect && host)
					{
						try
						{
							host->wantidle = true;
						}
						catch(const std::exception &)
						{
							// o_O`
						}
					}
					return 1;
				case audioMasterSizeWindow:
					if(effect && host)
					{
						try
						{
							if(host->editorWnd)
								reinterpret_cast<CVstEditorDlg *>(host->editorWnd)->Resize(index, value);
						}
						catch(const std::exception &)
						{
							// o_O`
						}
					}
					return 0;
				case audioMasterGetSampleRate:
					{
						long sampleRate=Global::pConfig->GetSamplesPerSec();
						if(effect && host)
							host->proxy().setSampleRate(sampleRate);
						return sampleRate;
					}
				case audioMasterGetBlockSize:
					if(effect && host)
					{
						host->proxy().setBlockSize(STREAM_SIZE);
					}
					return STREAM_SIZE;
				case audioMasterGetVendorString:
					// Just fooling version string
					// [bohan] why? do we have to fool some plugins to make them work with psycle's host?
					std::strcpy((char *) ptr,"Steinberg");
					//std::strcpy((char*)ptr,"Psycledelics");
					return 1;
				case audioMasterGetProductString:
					// Just fooling product string
					// [bohan] why? do we have to fool some plugins to make them work with psycle's host?
					std::strcpy((char *) ptr, "Cubase VST");
					//std::strcpy((char*) ptr, "Psycle");
					return 1;
				case audioMasterGetVendorVersion:	
					return 5000; // HOST version 5000
					// [bohan] is that a Cubase VST version?
				case audioMasterUpdateDisplay:
					if(effect && host)
					{
						try
						{
							host->proxy().dispatcher(effEditIdle);
						}
						catch(const std::exception &)
						{
							// o_O`
						}
					}
					return 0;
				case audioMasterCanDo:
					if(!std::strcmp((char *) ptr, "sendVstEvents")) return 1;
					if(!std::strcmp((char *) ptr, "sendVstMidiEvent")) return 1;
					if(!std::strcmp((char *) ptr, "sendVstTimeInfo")) return 1;
					//			"receiveVstEvents",
					//			"receiveVstMidiEvent",
					//			"receiveVstTimeInfo",
						
					//			"reportConnectionChanges",
					//			"acceptIOChanges",
					if(!std::strcmp((char *) ptr, "sizeWindow")) return 1;
					if(!std::strcmp((char *) ptr, "supplyIdle")) return 1;
					return -1;
					break;
				default: 
					TRACE("VST master dispatcher: undefed: %d\n",opcode);
					break;
				}	
				return 0;
			}
	


			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// VST Instrument



			instrument::instrument(int index)
			{
				_macIndex = index;
				_type = MACH_VST;
				_mode = MACHMODE_GENERATOR;
				std::sprintf(_editName, "Vst2 Instr.");
				_program = 0;
			}

			void instrument::Tick(int channel, PatternEntry * pData)
			{
				if(instantiated)
				{
					const int note = pData->_note;
					if(pData->_note == cdefMIDICC) // Mcm (MIDI CC) Command
					{
						AddMIDI(pData->_inst, pData->_cmd, pData->_parameter);
					}
					else if(note < 120) // Note on
					{
						if(pData->_cmd == 0x10) // _OLD_ MIDI Command
						{
							if((pData->_inst & 0xF0) == 0x80 || (pData->_inst & 0xF0) == 0x90)
							{
								AddMIDI(pData->_inst, note, pData->_parameter);
							}
							else AddMIDI(pData->_inst,pData->_parameter);
						}
						else if(pData->_cmd == 0x0C) 
						{
							if(pData->_inst == 0xFF) AddNoteOn(channel, note, pData->_parameter / 2);
							else AddNoteOn(channel,note,pData->_parameter/2,pData->_inst&0x0F);
						}
						else 
						{
							if(pData->_inst == 0xFF) AddNoteOn(channel, note, 127); // should be 100, but previous host used 127
							else AddNoteOn(channel, note, 127, pData->_inst & 0x0F);
						}
					}
					else if(note == 120) // Note Off.
					{
						if(pData->_inst == 0xFF) AddNoteOff(channel);
						else AddNoteOff(channel, pData->_inst & 0x0F);
					}
					else if(note == cdefTweakM || note == cdefTweakE) // Tweak Command
					{
						const float value(((pData->_cmd * 256) + pData->_parameter) / 65535.0f);
						SetParameter(pData->_inst, value);
						Global::pPlayer->Tweaker = true;
					}
					else if(note == cdefTweakS)
					{
						int i;
						if(TWSActive)
						{
							for(i = 0 ; i < MAX_TWS; ++i) if(TWSInst[i] == pData->_inst && TWSDelta[i]) break;
							if(i == MAX_TWS) for(i = 0 ; i < MAX_TWS; ++i) if(!TWSDelta[i]) break;
						}
						else for(i = MAX_TWS - 1 ; i > 0 ; --i) TWSDelta[i] = 0;
						if(i < MAX_TWS)
						{
							TWSDestination[i] = ((pData->_cmd * 256) + pData->_parameter) / 65535.0f;
							TWSInst[i] = pData->_inst;
							try
							{
								TWSCurrent[i] = proxy().getParameter(TWSInst[i]);
							}
							catch(const std::exception &)
							{
								TWSCurrent[i] = 0;
							}
							TWSDelta[i] = ((TWSDestination[i] - TWSCurrent[i]) * TWEAK_SLIDE_SAMPLES) / Global::pPlayer->SamplesPerRow();
							TWSSamples = 0;
							TWSActive = true;
						}
						else
						{
							// we have used all our slots, just send a twk
							const float value(((pData->_cmd * 256) + pData->_parameter) / 65535.0f);
							SetParameter(pData->_inst, value);
						}
						Global::pPlayer->Tweaker = true;
					}
				}
			}

			void instrument::Stop()
			{
				if(instantiated)
				{
					for(int i(0) ; i < MAX_TRACKS ; ++i) AddNoteOff(i);
					// <alk>
					// has been commented out because it crashes Z3ta+
					// and doesnt seem to be needed.
					/*
					for(int i(0) ; i < 16 ; ++i) AddMIDI(0xb0 + i, 0x7b); // Reset all controllers
					SendMidi();
					*/
					try
					{
						proxy().process(inputs, inputs, 64);
					}
					catch(const std::exception &)
					{
						// o_O`
					}
				}
			}

			void instrument::Work(int numSamples)
			{
				CPUCOST_INIT(cost);
				if(!_mute && instantiated)
				{
					if(wantidle) 
					{
						try
						{
							proxy().dispatcher(effIdle);
						}
						catch(const std::exception &)
						{
							// o_O`
						}
					}
					SendMidi();
					float * tempinputs[vst::max_io];
					float * tempoutputs[vst::max_io];
					for(int i(0) ; i < vst::max_io ; ++i)
					{
						tempinputs[i] = inputs[i];
						tempoutputs[i] = outputs[i];
					}
					int ns(numSamples);
					while(ns)
					{
						int nextevent;
						if(TWSActive) nextevent = TWSSamples; else nextevent = ns + 1;
						for(int i(0) ; i < Global::_pSong->SONGTRACKS ; ++i)
						{
							if(TriggerDelay[i]._cmd && TriggerDelayCounter[i] < nextevent) nextevent = TriggerDelayCounter[i];
						}
						if(nextevent > ns)
						{
							if(TWSActive) TWSSamples -= ns;
							for(int i(0) ; i < Global::_pSong->SONGTRACKS ; ++i)
							{
								// come back to this
								if(TriggerDelay[i]._cmd) TriggerDelayCounter[i] -= ns;
							}
							try
							{
								if(requiresProcess || !requiresRepl) {
									assert(!requiresRepl);
									proxy().process(tempinputs, tempoutputs, ns);
								}
								else {
									proxy().processReplacing(tempinputs, tempoutputs, ns);
								}
							}
							catch(const std::exception &)
							{
								// o_O`
							}
							ns = 0;
						}
						else
						{
							if(nextevent)
							{
								ns -= nextevent;
								try
								{
									if(!requiresRepl)
										proxy().process(tempinputs, tempoutputs, nextevent);
									else 
										proxy().processReplacing(tempinputs, tempoutputs, nextevent);
								}
								catch(const std::exception &)
								{
									// o_O`
								}
								for(int i(0) ; i < vst::max_io; ++i)
								{
									tempinputs[i] += nextevent;
									tempoutputs[i] += nextevent;
								}
							}
							if(TWSActive)
							{
								if(TWSSamples == nextevent)
								{
									int activecount(0);
									TWSSamples = TWEAK_SLIDE_SAMPLES;
									for(int i(0) ; i < MAX_TWS; ++i)
									{
										if(TWSDelta[i] != 0)
										{
											TWSCurrent[i] += TWSDelta[i];
											if
												(
													(TWSDelta[i] > 0 && TWSCurrent[i] >= TWSDestination[i]) ||
													(TWSDelta[i] < 0 && TWSCurrent[i] <= TWSDestination[i])
												)
											{
												TWSCurrent[i] = TWSDestination[i];
												TWSDelta[i] = 0;
											}
											else ++activecount;
											SetParameter(TWSInst[i],TWSCurrent[i]);
										}
									}
									if(!activecount) TWSActive = false;
								}
							}
							for(int i(0) ; i < Global::_pSong->SONGTRACKS ; ++i)
							{
								// come back to this
								if(TriggerDelay[i]._cmd == PatternCmd::NOTE_DELAY)
								{
									if(TriggerDelayCounter[i] == nextevent)
									{
										// do event
										Tick(i, &TriggerDelay[i]);
										TriggerDelay[i]._cmd = 0;
									}
									else TriggerDelayCounter[i] -= nextevent;
								}
								else if(TriggerDelay[i]._cmd == PatternCmd::RETRIGGER)
								{
									if(TriggerDelayCounter[i] == nextevent)
									{
										// do event
										Tick(i, &TriggerDelay[i]);
										TriggerDelayCounter[i] = (RetriggerRate[i] * Global::pPlayer->SamplesPerRow()) / 256;
									}
									else TriggerDelayCounter[i] -= nextevent;
								}
								else if(TriggerDelay[i]._cmd == PatternCmd::RETR_CONT)
								{
									if(TriggerDelayCounter[i] == nextevent)
									{
										// do event
										Tick(i, &TriggerDelay[i]);
										TriggerDelayCounter[i] = (RetriggerRate[i] * Global::pPlayer->SamplesPerRow()) / 256;
										int parameter(TriggerDelay[i]._parameter & 0x0f);
										if(parameter < 9) RetriggerRate[i] += 4  *parameter;
										else
										{
											RetriggerRate[i] -= 2 * (16 - parameter);
											if(RetriggerRate[i] < 16) RetriggerRate[i] = 16;
										}
									}
									else TriggerDelayCounter[i] -= nextevent;
								}
								else if (TriggerDelay[i]._cmd == PatternCmd::ARPEGGIO)
								{
									if (TriggerDelayCounter[i] == nextevent)
									{
										PatternEntry entry =TriggerDelay[i];
										switch(ArpeggioCount[i])
										{
										case 0: 
											Tick(i,&TriggerDelay[i]);
											ArpeggioCount[i]++;
											break;
										case 1:
											entry._note+=((TriggerDelay[i]._parameter&0xF0)>>4);
											Tick(i,&entry);
											ArpeggioCount[i]++;
											break;
										case 2:
											entry._note+=(TriggerDelay[i]._parameter&0x0F);
											Tick(i,&entry);
											ArpeggioCount[i]=0;
											break;
										}
										TriggerDelayCounter[i] = Global::pPlayer->SamplesPerRow()*Global::pPlayer->tpb/24;
									}
									else
									{
										TriggerDelayCounter[i] -= nextevent;
									}
								}
							}
						}
					}
					try
					{
						if(proxy().numOutputs() == 1) dsp::Add(outputs[0], outputs[1], numSamples, 1);
					}
					catch(const std::exception &)
					{
						// o_O`
					}
					// volume "counter"
					{
						_volumeCounter = dsp::GetMaxVSTVol(_pSamplesL, _pSamplesR, numSamples) * 32768.0f;
						if(_volumeCounter > 32768.0f) _volumeCounter = 32768.0f;
						int temp((f2i(fast_log2(_volumeCounter) * 78.0f * 4 / 14.0f) - (78 * 3))); // * 2; // not 100% accurate, but looks as it sounds
						// prevent downward jerkiness
						if(temp > 97) temp = 97;
						if(temp > _volumeDisplay) _volumeDisplay = temp;
						--_volumeDisplay;
						if( Global::pConfig->autoStopMachines)
						{
							if(_volumeCounter < 8.0f)
							{
								_volumeCounter = 0.0f;
								_volumeDisplay = 0;
								_stopped = true;
							}
							else _stopped = false;
						}
					}
				}
				CPUCOST_CALC(cost, numSamples);
				_cpuCost += cost;
				_worked = true;
			}



			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// VST Effect



			fx::fx(int index)
			{
				_macIndex = index;
				for(int i(0) ; i < MAX_CONNECTIONS; ++i) _inputConVol[i] = 1.f / 32767; // VST plugins use the range -1.0 .. +1.0
				plugin::plugin();
				_type = MACH_VSTFX;
				_mode = MACHMODE_FX;
				std::sprintf(_editName, "Vst2 Fx");
				_pOutSamplesL = new float[STREAM_SIZE];
				_pOutSamplesR = new float[STREAM_SIZE];
				dsp::Clear(_pOutSamplesL, STREAM_SIZE);
				dsp::Clear(_pOutSamplesR, STREAM_SIZE);
				inputs[0] = _pSamplesL;
				inputs[1] = _pSamplesR;
				outputs[0] = _pOutSamplesL;
				outputs[1] = _pOutSamplesR;
				_program = 0;
			}

			fx::~fx()
			{
				zapArray(_pOutSamplesL);
				zapArray(_pOutSamplesR);
			}

			void fx::Tick(int channel, PatternEntry * pData)
			{
				if(instantiated)
				{
					if( pData->_cmd == 0x10) // _OLD_ MIDI Command
					{
						if((pData->_inst & 0xF0) == 0x80 || (pData->_inst & 0xF0) == 0x90)
							AddMIDI(pData->_inst, pData->_note, pData->_parameter);
						else
							AddMIDI(pData->_inst, pData->_parameter);
					}
					else if(pData->_note == cdefMIDICC) // Mcm (MIDI CC) Command
					{
						AddMIDI(pData->_inst, pData->_cmd, pData->_parameter);
					}
					else if(pData->_note < 120) // Note on
					{
						if(pData->_cmd == 0x10) // _OLD_ MIDI Command
						{
							if((pData->_inst & 0xF0) == 0x80 || (pData->_inst & 0xF0) == 0x90)
							{
								AddMIDI(pData->_inst, pData->_note, pData->_parameter);
							}
							else AddMIDI(pData->_inst,pData->_parameter);
						}
						else if(pData->_cmd == 0x0C) 
						{
							if(pData->_inst == 0xFF) AddNoteOn(channel, pData->_note, pData->_parameter / 2);
							else AddNoteOn(channel,pData->_note,pData->_parameter/2,pData->_inst&0x0F);
						}
						else 
						{
							if(pData->_inst == 0xFF) AddNoteOn(channel, pData->_note, 127); // should be 100, but previous host used 127
							else AddNoteOn(channel, pData->_note, 127, pData->_inst & 0x0F);
						}
					}
					else if(pData->_note == 120) // Note Off.
					{
						if(pData->_inst == 0xFF) AddNoteOff(channel);
						else AddNoteOff(channel, pData->_inst & 0x0F);
					}

					else if(pData->_note == cdefTweakM || pData->_note == cdefTweakE) // Tweak command
					{
						const float value(((pData->_cmd * 256) + pData->_parameter) / 65535.0f);
						SetParameter(pData->_inst, value);
						Global::pPlayer->Tweaker = true;
					}
					else if(pData->_note == cdefTweakS)
					{
						int i;
						if(TWSActive)
						{
							for(i = 0 ; i < MAX_TWS; ++i) if(TWSInst[i] == pData->_inst && TWSDelta[i]) break;
							if(i == MAX_TWS) for(i = 0 ; i < MAX_TWS; ++i) if(!TWSDelta[i]) break;
						}
						else for (i = MAX_TWS-1 ; i > 0 ; --i) TWSDelta[i] = 0;
						if(i < MAX_TWS)
						{
							TWSDestination[i] = ((pData->_cmd * 256) + pData->_parameter) / 65535.0f;
							TWSInst[i] = pData->_inst;
							try
							{
								TWSCurrent[i] = proxy().getParameter(TWSInst[i]);
							}
							catch(const std::exception &)
							{
								TWSCurrent[i] = 0;
							}
							TWSDelta[i] = ((TWSDestination[i] - TWSCurrent[i]) * TWEAK_SLIDE_SAMPLES) / Global::pPlayer->SamplesPerRow();
							TWSSamples = 0;
							TWSActive = true;
						}
						else
						{
							// we have used all our slots, just send a twk
							const float value(((pData->_cmd * 256) + pData->_parameter) / 65535.0f);
							SetParameter(pData->_inst, value);
						}
						Global::pPlayer->Tweaker = true;
					}
				}
			}

			void fx::Work(int numSamples)
			{
				Machine::Work(numSamples);
				CPUCOST_INIT(cost);
				if((!_mute) && (!_stopped) && (!_bypass))
				{
					if(instantiated)
					{
						if(wantidle) 
						{
							try
							{
								proxy().dispatcher(effIdle, 0, 0, NULL, 0.0f);
							}
							catch (...)
							{
								// o_O`
							}
						}
						SendMidi();
						//\todo: why this duplicated undenormalize? it is done in Machine::Work() above.
						dsp::Undenormalize(_pSamplesL, _pSamplesR, numSamples);
						try
						{
							if(proxy().numInputs() == 1)
							{
								///\todo MIX input0 and input1!
							}
						}
						catch(const std::exception &)
						{
							// o_O`
						}
						try
						{
							if(!(proxy().flags() & effFlagsCanReplacing || requiresRepl))
							{
								dsp::Clear(_pOutSamplesL, numSamples);
								dsp::Clear(_pOutSamplesR, numSamples);
							}
						}
						catch(const std::exception &)
						{
							// o_O`
						}
						float * tempinputs[vst::max_io];
						float * tempoutputs[vst::max_io];
						for(int i(0) ; i < vst::max_io; ++i)
						{
							tempinputs[i] = inputs[i];
							tempoutputs[i] = outputs[i];
						}
						int ns(numSamples);
						while(ns)
						{
							int nextevent;
							if(TWSActive) nextevent = TWSSamples; else nextevent = ns + 1;
							for(int i(0) ; i < Global::_pSong->SONGTRACKS ; ++i)
							{
								if(TriggerDelay[i]._cmd) if(TriggerDelayCounter[i] < nextevent) nextevent = TriggerDelayCounter[i];
							}
							if(nextevent > ns)
							{
								if(TWSActive) TWSSamples -= ns;
								for(int i(0) ; i < Global::_pSong->SONGTRACKS; ++i)
								{
									// come back to this
									if(TriggerDelay[i]._cmd) TriggerDelayCounter[i] -= ns;
								}
								try
								{
									if((proxy().flags() & effFlagsCanReplacing) || requiresRepl)
										proxy().processReplacing(tempinputs, tempoutputs, ns);
									else
										proxy().process(tempinputs, tempoutputs, ns);
								}
								catch(const std::exception &)
								{
									// o_O`
								}
								ns = 0;
							}
							else
							{
								if(nextevent)
								{
									ns -= nextevent;
									try
									{
										if((proxy().flags() & effFlagsCanReplacing) || requiresRepl)
											proxy().processReplacing(tempinputs, tempoutputs, nextevent);
										else
											proxy().process(tempinputs, tempoutputs, nextevent);
									}
									catch(const std::exception &)
									{
										// o_O`
									}
									for(int i(0) ; i < vst::max_io ; ++i)
									{
										tempinputs[i]+=nextevent;
										tempoutputs[i]+=nextevent;
									}
								}
								if(TWSActive)
								{
									if(TWSSamples == nextevent)
									{
										int activecount = 0;
										TWSSamples = TWEAK_SLIDE_SAMPLES;
										for(int i(0) ; i < MAX_TWS; ++i)
										{
											if(TWSDelta[i])
											{
												TWSCurrent[i] += TWSDelta[i];
												if(
													(TWSDelta[i] > 0 && TWSCurrent[i] >= TWSDestination[i]) ||
													(TWSDelta[i] < 0 && TWSCurrent[i] <= TWSDestination[i]))
												{
													TWSCurrent[i] = TWSDestination[i];
													TWSDelta[i] = 0;
												}
												else ++activecount;
												SetParameter(TWSInst[i],TWSCurrent[i]);
											}
										}
										if(activecount == 0) TWSActive = false;
									}
								}
								for(int i(0) ; i < Global::_pSong->SONGTRACKS; ++i)
								{
									// come back to this
									if(TriggerDelay[i]._cmd == PatternCmd::NOTE_DELAY)
									{
										if(TriggerDelayCounter[i] == nextevent)
										{
											// do event
											Tick(i, &TriggerDelay[i]);
											TriggerDelay[i]._cmd = 0;
										}
										else TriggerDelayCounter[i] -= nextevent;
									}
									else if(TriggerDelay[i]._cmd == PatternCmd::RETRIGGER)
									{
										if(TriggerDelayCounter[i] == nextevent)
										{
											// do event
											Tick(i, &TriggerDelay[i]);
											TriggerDelayCounter[i] = (RetriggerRate[i] * Global::pPlayer->SamplesPerRow()) / 256;
										}
										else TriggerDelayCounter[i] -= nextevent;
									}
									else if(TriggerDelay[i]._cmd == PatternCmd::RETR_CONT)
									{
										if(TriggerDelayCounter[i] == nextevent)
										{
											// do event
											Tick(i, &TriggerDelay[i]);
											TriggerDelayCounter[i] = (RetriggerRate[i] * Global::pPlayer->SamplesPerRow()) / 256;
											int parameter(TriggerDelay[i]._parameter & 0x0f);
											if(parameter < 9) RetriggerRate[i] += 4 * parameter;
											else
											{
												RetriggerRate[i] -= 2 * (16 - parameter);
												if(RetriggerRate[i] < 16) RetriggerRate[i] = 16;
											}
										}
										else TriggerDelayCounter[i] -= nextevent;
									}
								}
							}
						}
						try
						{
							if(proxy().numOutputs() == 1) std::memcpy(outputs[1], outputs[0], numSamples * sizeof(float));
						}
						catch(const std::exception &)
						{
							// o_O`
						}
						// Just an inversion of the pointers
						// so that i don't need to copy the
						// whole _pOutSamples to _pSamples
						float* const tempSamplesL=inputs[0];
						float* const tempSamplesR=inputs[1];	
						_pSamplesL = _pOutSamplesL;				
						_pSamplesR = _pOutSamplesR;
						inputs[0] = _pOutSamplesL;
						inputs[1] = _pOutSamplesR;
						_pOutSamplesL = tempSamplesL;
						_pOutSamplesR = tempSamplesR;
						outputs[0] = tempSamplesL;
						outputs[1] = tempSamplesR;
					}
					// volume "counter"
					{
						_volumeCounter = dsp::GetMaxVSTVol(_pSamplesL, _pSamplesR,numSamples) * 32768.0f;
						if(_volumeCounter > 32768.0f) _volumeCounter = 32768.0f;
						int temp((f2i(fast_log2(_volumeCounter) * 78.0f * 4 / 14.0f) - (78 * 3))); // * 2; // not 100% accurate, but looks as it sounds
						// prevent downward jerkiness
						if(temp > 97) temp = 97;
						if(temp > _volumeDisplay) _volumeDisplay = temp;
						--_volumeDisplay;
						if(Global::pConfig->autoStopMachines)
						{
							if(_volumeCounter < 8.0f)
							{
								_volumeCounter = 0.0f;
								_volumeDisplay = 0;
								_stopped = true;
							}
							else _stopped = false;
						}
					}
				}
				CPUCOST_CALC(cost, numSamples);
				_cpuCost += cost;
				_worked = true;
			}



			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// old file format vomit, don't look at it.

			// Load for Old Psycle fileformat
			bool plugin::LoadChunk(RiffFile * pFile)
			{
				bool b;
				try
				{
					b = proxy().flags() & effFlagsProgramChunks;
				}
				catch(const std::exception &)
				{
					b = false;
				}
				if(!b) return false;
				// read chunk size
				long chunk_size;
				pFile->Read(&chunk_size, sizeof chunk_size);
				// read chunk data
				char * chunk(new char[chunk_size]);
				pFile->Read(chunk, chunk_size);
				try
				{
					proxy().dispatcher(effSetChunk, 0, chunk_size, chunk);
				}
				catch(const std::exception &)
				{
					// [bohan] hmm, so, data just gets lost?
					zapArray(chunk);
					return false;
				}
				zapArray(chunk);
				return true;
			}

			/// old file format vomit, don't look at it.
			bool plugin::Load(RiffFile * pFile)
			{
				char junkdata[256];
				std::memset(&junkdata, 0, sizeof(junkdata));
				Machine::Init();

			/*  This part is read when loading the song to detect the machine type.
				Might change in the new fileformat (i.e. Moving this to Machine::Load(RiffFile* pFile).

				pFile->Read(&_x, sizeof(_x));
				pFile->Read(&_y, sizeof(_y));
				pFile->Read(&_type, sizeof(_type));
			*/

			/*  Enable this when changing the File Format.

				CString sPath;
				char sPath2[_MAX_PATH];
				char dllName[128];
				pFile->Read(dllName,sizeof(dllName));
				_strlwr(dllName);

				if ( CNewMachine::dllNames.Lookup(dllName,sPath) ) 
				{
					strcpy(sPath2,sPath);
					if ( Instance(sPath2) != VSTINSTANCE_NOERROR )
					{
						CString error;
						sprintf(error,"Error. '%s' is Missing or Corrupted,tempName);
						MessageBox(error,"VST Plugin Loading Error",MB_OK | MB_ICONERROR);
					}
				}
				else
				{
					CString error;
					sprintf(error,"Error. '%s' is Missing or Corrupted,tempName);
					MessageBox(error,"VST Plugin Loading Error",MB_OK | MB_ICONERROR);
				}

				Init();
				pFile->Read(&_editName, sizeof(_editName));

				int num;
				pFile->Read(&num,sizeof(int));

				if ( !instantiated ) 
				{
					for (int p=0;p<num;p++ ) pFile->Read(&junkdata,sizeof(float));
					pFile->Read(&junkdata,sizeof(int));

					if(_pEffect->flags & effFlagsProgramChunks)
					{
						long chunk_size;
						pFile->Read(&chunk_size,sizeof(long));
						
						// Read chunk
						char *chunk=new char[chunk_size];
						pFile->Read(chunk,chunk_size);
						zapArray(chunk);
					}
				}
				else 
				{
					float value;
					for(int p=0;p<num;p++)
					{
						pFile->Write(&value,sizeof(float));
						_pEffect->setParameter(_pEffect,p,value);
					}
					int cprog;
					pFile->Read(&cprog,sizeof(int));
					Dispatch(effSetProgram,0,cprog,NULL,0.0f);

					if(_pEffect->flags & effFlagsProgramChunks)
					{
						long chunk_size;
						pFile->Read(&chunk_size,sizeof(long));
						
						// Read chunk
						char *chunk=new char[chunk_size];	
						pFile->Read(chunk,chunk_size);
						Dispatch(effSetChunk,0,chunk_size, chunk ,0.0f);
						zapArray(chunk);
					}

				}

			*/
				pFile->Read(&_editName, 16);	//Remove when changing the fileformat.
				_editName[15]='\0';
				pFile->Read(&_inputMachines[0], sizeof(_inputMachines));
				pFile->Read(&_outputMachines[0], sizeof(_outputMachines));
				pFile->Read(&_inputConVol[0], sizeof(_inputConVol));
				pFile->Read(&_connection[0], sizeof(_connection));
				pFile->Read(&_inputCon[0], sizeof(_inputCon));
				pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
				pFile->Read(&_numInputs, sizeof(_numInputs));
				pFile->Read(&_numOutputs, sizeof(_numOutputs));

				pFile->Read(&_panning, sizeof(_panning));
				Machine::SetPan(_panning);
				pFile->Read(&junkdata[0], 8*sizeof(int)); // SubTrack[]
				pFile->Read(&junkdata[0], sizeof(int)); // numSubtracks
				pFile->Read(&junkdata[0], sizeof(int)); // interpol

				pFile->Read(&junk[0], sizeof(int)); // outwet
				pFile->Read(&junk[0], sizeof(int)); // outdry

				pFile->Read(&junkdata[0], sizeof(int)); // distPosThreshold
				pFile->Read(&junkdata[0], sizeof(int)); // distPosClamp
				pFile->Read(&junkdata[0], sizeof(int)); // distNegThreshold
				pFile->Read(&junkdata[0], sizeof(int)); // distNegClamp

				pFile->Read(&junkdata[0], sizeof(char)); // sinespeed
				pFile->Read(&junkdata[0], sizeof(char)); // sineglide
				pFile->Read(&junkdata[0], sizeof(char)); // sinevolume
				pFile->Read(&junkdata[0], sizeof(char)); // sinelfospeed
				pFile->Read(&junkdata[0], sizeof(char)); // sinelfoamp

				pFile->Read(&junkdata[0], sizeof(int)); // delayTimeL
				pFile->Read(&junkdata[0], sizeof(int)); // delayTimeR
				pFile->Read(&junkdata[0], sizeof(int)); // delayFeedbackL
				pFile->Read(&junkdata[0], sizeof(int)); // delayFeedbackR

				pFile->Read(&junkdata[0], sizeof(int)); // filterCutoff
				pFile->Read(&junkdata[0], sizeof(int)); // filterResonance
				pFile->Read(&junkdata[0], sizeof(int)); // filterLfospeed
				pFile->Read(&junkdata[0], sizeof(int)); // filterLfoamp
				pFile->Read(&junkdata[0], sizeof(int)); // filterLfophase
				pFile->Read(&junkdata[0], sizeof(int)); // filterMode

				bool old;
				pFile->Read(&old, sizeof old); // old format
				pFile->Read(&_instance, sizeof _instance); // ovst.instance
				if(old)
				{
					char mch;
					pFile->Read(&mch, sizeof mch);
					_program = 0;
				}
				else
				{
					pFile->Read(&_program, sizeof _program);
				}
				return true;
			}
		}
	}
}

#include "stdafx.h"
#if !defined _WINAMP_PLUGIN_
	#include "psycle.h"
	#include "MainFrm.h"
	#include "VstEditorDlg.h"
#endif
#include "VstHost.h"
#include "song.h"
#include "Configuration.h"
#include "Player.h"
#include "InputHandler.h"
///\file
///\brief implementation file for psycle::host::plugin
namespace psycle
{
	namespace host
	{
		#if !defined _WINAMP_PLUGIN_
			extern CPsycleApp theApp;
		#endif

		namespace vst
		{
			VstTimeInfo plugin::_timeInfo;

			plugin::plugin()
				: queue_size(0)
				, wantidle(false)
				, _sDllName(0)
				, _pEffect(0)
				, h_dll(0)
				, instantiated(false)
				, requiresProcess(false)
				, requiresRepl(false)
				#if !defined _WINAMP_PLUGIN_
					, editorWnd(0)
				#endif
			{
				outputs[0] = _pSamplesL;
				outputs[1] = _pSamplesR;
				std::memset(junk, 0, STREAM_SIZE * sizeof(float));
				for(int i(0) ; i < vst::max_io ; ++i)
				{
					inputs[i]=junk;
					outputs[i]=junk;
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
				}
				delete _sDllName;
			}

			void plugin::Instance(const char dllname[], bool overwriteName) throw(...)
			{
				try
				{
					if(_sDllName) delete _sDllName;
					_sDllName = new char[std::strlen(dllname) + 1];
					std::strcpy(_sDllName, dllname);
					TRACE("VST plugin: library file name: %s\n", _sDllName);
					_pEffect = 0;
					instantiated = false;
					if(!(h_dll = ::LoadLibrary(dllname)))
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
							<< "could not resolve symbol 'main' in library: " << dllname << std::endl
							<< operating_system::exceptions::code_description();
						throw host::exceptions::library_errors::symbol_resolving_error(s.str());
					}
					// 1: calls the "main" function and receives the pointer to the AEffect structure.
					{
						try 
						{
							_pEffect = main(reinterpret_cast<audioMasterCallback>(&AudioMaster));
						}
						catch(const std::exception & e) { host::exceptions::function_errors::rethrow(*this, "main", &e); }
						catch(const char * const e) { host::exceptions::function_errors::rethrow(*this, "main", &e); }
						catch(const long int & e) { host::exceptions::function_errors::rethrow(*this, "main", &e); }
						catch(const unsigned long int & e) { host::exceptions::function_errors::rethrow(*this, "main", &e); }
						catch(...) { host::exceptions::function_errors::rethrow<void*>(*this, "main"); }
					}
					if(!_pEffect || _pEffect->magic!=kEffectMagic)
					{
						std::ostringstream s; s << "call to function 'main' returned a bad value";
						if(_pEffect) s << std::endl << "returned value signature: " << _pEffect->magic;
						_pEffect = 0;
						throw host::exceptions::function_errors::bad_returned_value(s.str());
					}
					TRACE("VST plugin: instanciated at (Effect*): %.8X\n", (int) _pEffect);
					// 2: Host to Plug, setSampleRate ( 44100.000000 )
					Dispatch(effSetSampleRate, 0, 0, 0, (float) Global::pConfig->GetSamplesPerSec());
					// 3: Host to Plug, setBlockSize ( 512 ) 
					Dispatch(effSetBlockSize, 0, STREAM_SIZE);
					// 4: Host to Plug, open
					{
						//init plugin (probably a call to "Init()" function should be done here)
						_pEffect->user = this;
						Dispatch(effOpen);
					}
					// 5: Host to Plug, setSpeakerArrangement returned: false 
					VstSpeakerArrangement VSTsa;
					{
						VSTsa.type = kSpeakerArrStereo;
						VSTsa.numChannels = 2;
						VSTsa.speakers[0].type = kSpeakerL;
						VSTsa.speakers[1].type = kSpeakerR;
						Dispatch(effSetSpeakerArrangement, 0, (long) &VSTsa, &VSTsa);
					}
					// 6: Host to Plug, setSampleRate ( 44100.000000 ) 
					Dispatch(effSetSampleRate, 0, 0, 0, (float) Global::pConfig->GetSamplesPerSec());
					// 7: Host to Plug, setBlockSize ( 512 ) 
					Dispatch(effSetBlockSize,  0, STREAM_SIZE);
					// 8: Host to Plug, setSpeakerArrangement returned: false 
					Dispatch(effSetSpeakerArrangement, 0, (long) &VSTsa, &VSTsa);
					// 9: Host to Plug, setSampleRate ( 44100.000000 ) 
					Dispatch(effSetSampleRate, 0, 0, 0, (float) Global::pConfig->GetSamplesPerSec());
					// 10: Host to Plug, setBlockSize ( 512 ) 
					Dispatch(effSetBlockSize, 0, STREAM_SIZE);
					// 11: Host to Plug, getProgram returned: 0 
					long int program(Dispatch(effGetProgram));
					// 12: Host to Plug, getProgram returned: 0 
					program = Dispatch( effGetProgram);
					// 13: Host to Plug, getVstVersion returned: 2300 
					{
						_version = Dispatch(effGetVstVersion);
						if(!_version) _version=1;
					}
					// 14: Host to Plug, getProgramNameIndexed ( -1 , 0 , ptr to char ) 
					Dispatch(effSetProgram);
					Dispatch(effMainsChanged, 0, 1);
					if(!Dispatch(effGetEffectName, 0, 0, &_sProductName))
					{
						CString str1(dllname);
						CString str2 = str1.Mid(str1.ReverseFind('\\')+1);
						str1 = str2.Left(str2.Find('.'));
						std::strcpy(_sProductName,str1);
					}
					if(overwriteName) std::memcpy(_editName, _sProductName, 31); // <bohan> \todo why is that 31 ?
					_editName[31]='\0'; // <bohan> \todo why is that 31 ?
					// Compatibility hacks
					{
						if(std::strcmp(_sProductName, "sc-101") == 0)
						{
							requiresRepl = true;
						}
					}
					if(!Dispatch(effGetVendorString, 0, 0, &_sVendorName)) std::strcpy(_sVendorName, "Unknown vendor");
					_isSynth = (_pEffect->flags & effFlagsIsSynth) ? true : false;
					instantiated = true;
					TRACE("VST plugin: successfully instanciated. inputs: %d, outputs: %d\n", _pEffect->numInputs, _pEffect->numOutputs);
				}
				catch(...)
				{
					throw; // this is now all handled by parent calls
				}
			}

			void plugin::Free() throw(...) // called also in destructor
			{
				if(instantiated)
				{
					TRACE("VST plugin: freeing");
					if(_pEffect)
					{
						TRACE("VST plugin: querying free 0x%.8X\n", (int) _pEffect);
						_pEffect->user = 0;
						Dispatch(effMainsChanged);
						Dispatch(effClose);
						_pEffect = 0;
					}
					if(h_dll)
					{
						TRACE("VST plugin: freeing library 0x%.8X\n", (int) _pEffect);
						FreeLibrary(h_dll);
						h_dll = 0;
					}
					instantiated = false;
				}
			}

			namespace exceptions
			{
				namespace dispatch_errors
				{
					/// Dispatcher operation code descriptions.
					const std::string operation_description(const long int & code) throw()
					{
						std::ostringstream s; s << code << ": ";
						switch(code)
						{
						case effOpen: s << "open"; break;
						case effClose: s << "close"; break;
						case effSetProgram: s << "set program"; break;
						case effGetProgram: s << "get program"; break;
						case effSetProgramName: s << "set program name"; break;
						case effGetProgramName: s << "get program name"; break;
						case effGetParamLabel: s << "get parameter label"; break;
						case effGetParamDisplay: s << "get parameter display"; break;
						case effGetParamName: s << "get parameter name"; break;
						case effGetVu: s << "get vu"; break;
						// system
						case effSetSampleRate: s << "set sample rate"; break;
						case effSetBlockSize: s << "set block size"; break;
						case effMainsChanged: s << "mains changed"; break;
						// editor
						case effEditGetRect: s << "get edit rectangle"; break;
						case effEditOpen: s << "open edit"; break;
						case effEditClose: s << "close edit"; break;
						case effEditDraw: s << "draw edit"; break;
						case effEditMouse: s << "edit mouse"; break;
						case effEditKey: s << "edit key"; break;
						case effEditIdle: s << "edit idle"; break;
						case effEditTop: s << "edit top"; break;
						case effEditSleep: s << "edit sleep"; break;
						// other
						case effIdentify: s << "identify"; break;
						case effGetChunk: s << "get chunk"; break;
						case effSetChunk: s << "set chunk"; break;
						default: s << "unkown op code";
						}
						return s.str();
					}

					template<typename e> void rethrow(const plugin & plugin, const long int operation, const e * const e = 0) throw(dispatch_error)
					{
						std::ostringstream title; title << "VST Plugin: " << plugin._editName << ": " << plugin.GetDllName();
						std::ostringstream s; s
							<< title.str().c_str() << std::endl
							<< "VST plugin had an exception on dispatcher operation: " << operation_description(operation) << '.' << std::endl
							<< typeid(*e).name() << std::endl
							<< host::exceptions::function_errors::string(*e);
						::MessageBox(0, s.str().c_str(), title.str().c_str(), MB_OK | MB_ICONWARNING);
						throw dispatch_error(s.str());
					}
				}
			}

			long int plugin::Dispatch(long int operation, long int index, long int value, void * ptr, float opt) throw(exceptions::dispatch_error)
			{
				try
				{
					return _pEffect->dispatcher(_pEffect, operation, index, value, ptr, opt);
				}
				///<bohan> \todo i don't know why we get a "warning C4702: unreachable code" with msvc7.1 here...
				catch(const std::exception & e) { exceptions::dispatch_errors::rethrow(*this, operation, &e); }
				catch(const char e[]) { exceptions::dispatch_errors::rethrow(*this, operation, &e); }
				catch(const long int & e) { exceptions::dispatch_errors::rethrow(*this, operation, &e); }
				catch(const unsigned long int & e) { exceptions::dispatch_errors::rethrow(*this, operation, &e); }
				catch(...) { exceptions::dispatch_errors::rethrow<void*>(*this, operation); }
			}

			bool plugin::LoadSpecificFileChunk(RiffFile * pFile, int version)
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
						::MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
						return false;
					}
					pFile->Read(&_program, sizeof _program);
					// set the program
					SetCurrentProgram(_program);
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
						// <bohan> why don't we just call LoadChunk?
						// <bohan> hmm, shit, LoadCunk actually reads the chunk size too.
						if(_pEffect->flags & effFlagsProgramChunks)
						{
							char * const data(new char[size]);
							pFile->Read(data, size); // Number of parameters
							try 
							{
								Dispatch(effSetChunk, 0, size, data);
							}
							catch(...)
							{
								// <bohan> hmm, so, data just gets lost?
								delete data;
								return false;
							}
							delete data;
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

			bool plugin::LoadChunk(RiffFile * pFile)
			{
				if(_pEffect->flags & effFlagsProgramChunks)
				{
					// read chunk size
					long chunk_size;
					pFile->Read(&chunk_size, sizeof chunk_size);
					// read chunk data
					char * chunk(new char[chunk_size]);
					pFile->Read(chunk, chunk_size);
					try
					{
						Dispatch(effSetChunk, 0, chunk_size, chunk);
					}
					catch (...)
					{
						// <bohan> hmm, so, data just gets lost?
						delete chunk;
						return false;
					}
					delete chunk;
					return true;
				}
				return false;
			}

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
				if(_pEffect->flags & effFlagsProgramChunks)
				{
					try 
					{
						size += Dispatch(effGetChunk, 0, 0, &pData);
					}
					catch(...)
					{
						// <bohan>
						// i think it's not necessary to set the size to the same value again.
						// if an exception is thrown, size won't be changed.
						size = sizeof _program  + sizeof count  + sizeof(float) * count;
					}
				}
				pFile->Write(&size, sizeof size);
				pFile->Write(&_program, sizeof _program);
				pFile->Write(&count, sizeof count);
				for(UINT i(0); i < count; ++i)
				{
					float temp(GetParameter(i));
					pFile->Write(&temp,sizeof(temp));
				}
				size -= sizeof _program + sizeof count + sizeof(float) * count;
				if(size > 0)
				{
					pFile->Write(pData, size);
				}
			};

			bool plugin::LoadDll(char* psFileName)
			{
				_strlwr(psFileName); ///\todo _strlwr can rot in hell, it's really not the standard way of converting case.
				char sPath2[1 << 10];
				::CString sPath;
				#if defined _WINAMP_PLUGIN_
					sPath = Global::pConfig->GetVstDir();
					if(FindFileinDir(psFileName, sPath))
					{
						std::strcpy(sPath2, sPath);
						try
						{
							Instance(sPath2, false);
						}
						catch(const std::exception & e)
						{
							std::ostringstream s; s
								<< "Exception while instanciating:" << sPath2 << std::endl
								<< "Replacing with dummy." << std::endl
								<< typeid(e).name() << std::endl
								<< e.what();
							::MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
							return false;
						}
						catch(...)
						{
							std::ostringstream s; s
								<< "Exception while instanciating:" << sPath2 << std::endl
								<< "Replacing with dummy." << std::endl
								<< "Unkown type of exception";
							::MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
							return false;
						}
					}
					else
					{
						std::ostringstream s; s
							<< "Missing:" << psFileName << std::endl
							<< "Replacing with dummy."
						::MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
						return false;
					}
				#else
					if(CNewMachine::dllNames.Lookup(psFileName, sPath))
					{
						std::strcpy(sPath2, sPath);
						if(!CNewMachine::TestFilename(sPath2)) return false;
						try
						{
							Instance(sPath2, false);
						}
						catch(const std::exception & e)
						{
							std::ostringstream s; s
								<< "Exception while instanciating:" << sPath2 << std::endl
								<< "Replacing with dummy." << std::endl
								<< typeid(e).name() << std::endl
								<< e.what();
							::MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
							return false;
						}
						catch(...)
						{
							std::ostringstream s; s
								<< "Exception while instanciating:" << sPath2 << std::endl
								<< "Replacing with dummy." << std::endl
								<< "Unkown type of exception";
							::MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
							return false;
						}
					}
					else
					{
						std::ostringstream s; s
							<< "Missing:" << psFileName << std::endl
							<< "Replacing with dummy.";
						::MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
						return false;
					}
				#endif // _WINAMP_PLUGIN_
				return true;
			};

			bool plugin::DescribeValue(int parameter, char * psTxt)
			{
				if(instantiated)
				{
					if(parameter < _pEffect->numParams)
					{
						char par_display[64]={0};
						char par_label[64]={0};
						try
						{
							Dispatch(effGetParamDisplay, parameter, 0, par_display);
						}
						catch (...)
						{
							// o_O`
						}
						try
						{
							Dispatch(effGetParamLabel, parameter, 0, par_label);
						}
						catch (...)
						{
							// o_O`
						}
						std::sprintf(psTxt, "%s(%s)", par_display, par_label);
						return true;
					}
					else std::sprintf(psTxt, "Invalid NumParams Value");
				}
				else std::sprintf(psTxt, "Not loaded"); // <bohan> wow.. can this fucked up situtation really happens?
				return false;
			}

			bool plugin::SetParameter(int parameter, float value)
			{
				if(instantiated)
				{
					if(parameter < _pEffect->numParams)
					{
						_pEffect->setParameter(_pEffect, parameter, value);
						return true;
					}
				}
				return false;
			}

			bool plugin::SetParameter(int parameter, int value)
			{
				return SetParameter(parameter, value / 65535.0f);
			}

			int plugin::GetCurrentProgram()
			{
				if(instantiated)
				{
					try
					{
						return Dispatch(effGetProgram);
					}
					catch (...)
					{
						return 0; // <bohan> well, what to return if it fails? 0 is wrong..
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
						Dispatch(effSetProgram, 0, prg);
					}
					catch (...)
					{
						// o_O`
					}
				}
			}

			bool plugin::AddMIDI(unsigned char data0, unsigned char data1, unsigned char data2)
			{
				if(instantiated)
				{
					VstMidiEvent * pevent(&midievent[queue_size]);
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
					if(queue_size < MAX_VST_EVENTS - 1) ++queue_size;
					else queue_size = MAX_VST_EVENTS - 1;
					return true;
				}
				else return false;
			}


			void plugin::SendMidi()
			{
				if(instantiated && queue_size > 0)
				{
					// Prepare MIDI events and free queue dispatching all events
					if(queue_size > MAX_VST_EVENTS - 1) queue_size = MAX_VST_EVENTS - 1;
					events.numEvents = queue_size;
					events.reserved = 0;
					for(int q(0) ; q < queue_size ; ++q) events.events[q] = (VstEvent*) &midievent[q];
					//Finally Send the events.
					try
					{
						Dispatch(effProcessEvents, 0, 0, &events);
					}
					catch(...)
					{
						queue_size = 0;
						//throw;
					}
				}
				queue_size = 0;
			}

			long plugin::AudioMaster(AEffect * effect, long opcode, long index, long value, void *ptr, float opt)
			{
				TRACE("VST plugin: call to host dispatcher: Eff: 0x%.8X, Opcode = %d, Index = %d, Value = %d, PTR = %.8X, OPT = %.3f\n", (int) effect, opcode, index, value, (int) ptr, opt);
				// believe it or not, some plugs tried to call psycle with a null AEffect.
				// Support opcodes
				switch(opcode)
				{
				case audioMasterAutomate:
					#if !defined _WINAMP_PLUGIN_
							Global::_pSong->Tweaker = true;
							if(effect)
							{
								if( effect->user) /// \todo ugly solution...
								{
									if(Global::pConfig->_RecordTweaks)  
									{
										if(Global::pConfig->_RecordMouseTweaksSmooth)
											((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweakSlide(reinterpret_cast<plugin *>(effect->user)->_macIndex, index, f2i(opt * vst::quantization));
										else
											((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweak(reinterpret_cast<plugin *>(effect->user)->_macIndex, index, f2i(opt * vst::quantization));
									}
									if(reinterpret_cast<plugin *>(effect->user)->editorWnd)
										((CVstEditorDlg *) reinterpret_cast<plugin *>(effect->user)->editorWnd)->Refresh(index, opt);
								}
							}
							
					#endif
					return 0; // index, value, returns 0
				case audioMasterVersion:			
					return 2; // vst version, currently 7 (0 for older)
				case audioMasterCurrentId:			
					return 'AASH'; // returns the unique id of a plug that's currently loading
				case audioMasterIdle:
					if(effect)
					{
						try
						{
							reinterpret_cast<plugin *>(effect->user)->Dispatch(effEditIdle);
						}
						catch (...)
						{
							// o_O`
						}
					}
					return 0; // call application idle routine (this will call effEditIdle for all open editors too) 
				case audioMasterPinConnected:
					if(value == 0) //input
					{
						if(index < 2) return 0; // 0 means connected, 1 disconnected.
						else return 1;
					}
					else //output
					{
						if(index < 2) return 0;
						else return 1;
					}
					// return 0; // inquire if an input or output is beeing connected;
				case audioMasterWantMidi:			
					return 0;
				case audioMasterProcessEvents:		
					return 0; // Support of vst events to host is not available
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
					/*
					// msvc6: "error C2065: 'timeGetTime' : undeclared identifier" @#~!"%&$!"!!!!
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
							const float linestep = (((float)(Global::_pSong->SamplesPerTick-Global::pPlayer->_ticksRemaining)) / Global::_pSong->SamplesPerTick) / Global::pPlayer->tpb;
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
				case audioMasterTempoAt:
					// This might be incorrect:
					// Declaration: virtual long tempoAt (long pos); // returns tempo (in bpm * 10000) at sample frame location <pos>
					return Global::pPlayer->bpm * 10000;
				case audioMasterNeedIdle:
					if(effect)
					{
						if( effect->user) 
						{
							reinterpret_cast<plugin *>(effect->user)->wantidle = true;
						}
					}
					return 1;
				case audioMasterGetSampleRate:		
					return Global::pConfig->GetSamplesPerSec();
				case audioMasterGetVendorString:
					// Just fooling version string
					// <bohan> why? do we have to fool some plugins to make them work with psycle's host?
					std::strcpy((char *) ptr,"Steinberg");
					//std::strcpy((char*)ptr,"Psycledelics");
					return 0;
				case audioMasterGetVendorVersion:	
					return 5000; // HOST version 5000
					// <bohan> is that a Cubase VST version?
				case audioMasterGetProductString:
					// Just fooling product string
					// <bohan> why? do we have to fool some plugins to make them work with psycle's host?
					std::strcpy((char *) ptr, "Cubase VST");
					//std::strcpy((char*) ptr, "Psycle");
					return 0;
				case audioMasterVendorSpecific:		
					return 0;
				case audioMasterGetLanguage:		
					return kVstLangEnglish;
				case audioMasterUpdateDisplay:
					if(effect)
					{
						try
						{
							reinterpret_cast<plugin *>(effect->user)->Dispatch(effEditIdle);
						}
						catch (...)
						{
							// o_O`
						}
					}
					return 0;
				case audioMasterSizeWindow:
					#if !defined _WINAMP_PLUGIN_
							if(effect)
							{
								if( effect->user)
								{
									if(reinterpret_cast<plugin *>(effect->user)->editorWnd)
										reinterpret_cast<CVstEditorDlg *>(reinterpret_cast<plugin *>(effect->user)->editorWnd)->Resize(index, value);
								}
							}
					#endif
					return 0;
				case audioMasterGetParameterQuantization:	
					return vst::quantization;
				case audioMasterGetBlockSize:
					return STREAM_SIZE;
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
				case audioMasterSetTime:						
					TRACE("VST master dispatcher: Set Time\n");
					break;
				case audioMasterGetNumAutomatableParameters:	
					TRACE("VST master dispatcher: GetNumAutPar\n");
					break;
				//case audioMasterGetParameterQuantization:	
					//TRACE("VST master dispatcher: ParamQuant\n");
					//break;
				case audioMasterIOChanged:					
					TRACE("VST master dispatcher: IOchanged\n");
					break;
				//case audioMasterSizeWindow:					
					//TRACE("VST master dispatcher: Size Window\n");
					//break;
				case audioMasterGetInputLatency:				
					TRACE("VST master dispatcher: GetInLatency\n");
					break;
				case audioMasterGetOutputLatency:			
					TRACE("VST master dispatcher: GetOutLatency\n");
					break;
				case audioMasterGetPreviousPlug:				
					TRACE("VST master dispatcher: PrevPlug\n");
					break;
				case audioMasterGetNextPlug:					
					TRACE("VST master dispatcher: NextPlug\n");
					break;
				case audioMasterWillReplaceOrAccumulate:		
					TRACE("VST master dispatcher: WillReplace\n");
					break;
				case audioMasterGetCurrentProcessLevel:		
					TRACE("VST master dispatcher: GetProcessLevel\n");
					break;
				case audioMasterGetAutomationState:			
					TRACE("VST master dispatcher: GetAutState\n");
					break;
				case audioMasterOfflineStart:				
					TRACE("VST master dispatcher: Offlinestart\n");
					break;
				case audioMasterOfflineRead:					
					TRACE("VST master dispatcher: Offlineread\n");
					break;
				case audioMasterOfflineWrite:				
					TRACE("VST master dispatcher: Offlinewrite\n");
					break;
				case audioMasterOfflineGetCurrentPass:		
					TRACE("VST master dispatcher: OfflineGetcurrentpass\n");
					break;
				case audioMasterOfflineGetCurrentMetaPass:	
					TRACE("VST master dispatcher: OfflineGetCurrentMetapass\n");
					break;
				case audioMasterSetOutputSampleRate:			
					TRACE("VST master dispatcher: SetOutputsamplerate\n");
					break;
				case audioMasterGetSpeakerArrangement:		
					TRACE("VST master dispatcher: Getspeaker\n");
					break;
				case audioMasterSetIcon:						
					TRACE("VST master dispatcher: SetIcon\n");
					break;
				case audioMasterOpenWindow:					
					TRACE("VST master dispatcher: OpenWindow\n");
					break;
				case audioMasterCloseWindow:					
					TRACE("VST master dispatcher: CloseWindow\n");
					break;
				case audioMasterGetDirectory:				
					TRACE("VST master dispatcher: GetDirectory\n");
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
				for(int i(0) ; i < MAX_TRACKS; ++i)
				{
					trackNote[i].key = 255; // No Note.
					trackNote[i].midichan = 0;
				}
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
						#if !defined _WINAMP_PLUGIN_
							Global::_pSong->Tweaker = true;
						#endif
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
							TWSCurrent[i] = GetParameter(TWSInst[i]);
							TWSDelta[i] = ((TWSDestination[i] - TWSCurrent[i]) * TWEAK_SLIDE_SAMPLES) / Global::_pSong->SamplesPerTick;
							TWSSamples = 0;
							TWSActive = true;
						}
						else
						{
							// we have used all our slots, just send a twk
							const float value(((pData->_cmd * 256) + pData->_parameter) / 65535.0f);
							SetParameter(pData->_inst, value);
						}
						#if !defined _WINAMP_PLUGIN_
							Global::_pSong->Tweaker = true;
						#endif
					}
				}
			}

			bool instrument::AddNoteOn(unsigned char channel, unsigned char key, unsigned char velocity, unsigned char midichannel)
			{
				if(instantiated)
				{
					if(trackNote[channel].key != 255) AddNoteOff(channel, trackNote[channel].key, true);
					VstMidiEvent * pevent(&midievent[queue_size]);
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
					pevent->midiData[0] = 0x90 | midichannel; // Midi On
					pevent->midiData[1] = key;
					pevent->midiData[2] = velocity;
					pevent->midiData[3] = 0;
					if(queue_size < MAX_VST_EVENTS - 1) ++queue_size; else queue_size = MAX_VST_EVENTS - 1;
					note thisnote;
					thisnote.key = key;
					thisnote.midichan = midichannel;
					trackNote[channel] = thisnote;
					return true;
				}
				return false;
			}

			bool instrument::AddNoteOff(unsigned char channel, unsigned char midichannel, bool addatStart)
			{
				if(instantiated)
				{
					if(trackNote[channel].key != 255)
					{
						VstMidiEvent * pevent;
						if( addatStart)
						{
							// PATCH:
							// When a new note enters, it adds a note-off for the previous note playing in
							// the track (this is ok). But if you have like: A-4 C-5 and in the next line
							// C-5 E-5 , you will only hear E-5.
							// Solution: Move the NoteOffs at the beginning.
							for(int i(MAX_VST_EVENTS - 1) ; i > 0 ; --i) midievent[i] = midievent[i - 1];
							pevent = &midievent[0];
						}
						else pevent = &midievent[queue_size];
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
						if(queue_size < MAX_VST_EVENTS - 1) ++queue_size; else queue_size = MAX_VST_EVENTS - 1;
						note thisnote;
						thisnote.key = 255;
						thisnote.midichan = 0;
						trackNote[channel] = thisnote;
						return true;
					}
					else return false;
				}
				else return false;
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
					_pEffect->process(_pEffect, inputs, inputs, 64);
				}
			}

			void instrument::Work(int numSamples)
			{
				#if !defined _WINAMP_PLUGIN_
					CPUCOST_INIT(cost);
				#endif
				if(!_mute && instantiated)
				{
					if(wantidle) 
					{
						try
						{
							Dispatch(effIdle);
						}
						catch (...)
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
							if(!requiresRepl) 
								_pEffect->process(_pEffect, tempinputs, tempoutputs, ns);
							else
								_pEffect->processReplacing(_pEffect, tempinputs, tempoutputs, ns);
							ns = 0;
						}
						else
						{
							if(nextevent)
							{
								ns -= nextevent;
								if(!requiresRepl) 
									_pEffect->process(_pEffect, tempinputs, tempoutputs, nextevent);
								else 
									_pEffect->processReplacing(_pEffect, tempinputs, tempoutputs, nextevent);
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
								if(TriggerDelay[i]._cmd == 0xfd)
								{
									if(TriggerDelayCounter[i] == nextevent)
									{
										// do event
										Tick(i, &TriggerDelay[i]);
										TriggerDelay[i]._cmd = 0;
									}
									else TriggerDelayCounter[i] -= nextevent;
								}
								else if(TriggerDelay[i]._cmd == 0xfb)
								{
									if(TriggerDelayCounter[i] == nextevent)
									{
										// do event
										Tick(i, &TriggerDelay[i]);
										TriggerDelayCounter[i] = (RetriggerRate[i] * Global::_pSong->SamplesPerTick) / 256;
									}
									else TriggerDelayCounter[i] -= nextevent;
								}
								else if(TriggerDelay[i]._cmd == 0xfa)
								{
									if(TriggerDelayCounter[i] == nextevent)
									{
										// do event
										Tick(i, &TriggerDelay[i]);
										TriggerDelayCounter[i] = (RetriggerRate[i] * Global::_pSong->SamplesPerTick) / 256;
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
							}
						}
					}
					if( _pEffect->numOutputs == 1) Dsp::Add(outputs[0], outputs[1], numSamples, 1);
					#if !defined _WINAMP_PLUGIN_
					// volume "counter"
					{
						_volumeCounter = Dsp::GetMaxVSTVol(_pSamplesL, _pSamplesR, numSamples) * 32768.0f;
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
					#endif
				}
				#if !defined _WINAMP_PLUGIN_
					CPUCOST_CALC(cost, numSamples);
					_cpuCost += cost;
				#endif
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
				Dsp::Clear(_pOutSamplesL, STREAM_SIZE);
				Dsp::Clear(_pOutSamplesR, STREAM_SIZE);
				inputs[0] = _pSamplesL;
				inputs[1] = _pSamplesR;
				outputs[0] = _pOutSamplesL;
				outputs[1] = _pOutSamplesR;
				_program = 0;
			}

			fx::~fx()
			{
				delete _pOutSamplesL;
				delete _pOutSamplesR;
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
					else if(pData->_note == cdefTweakM || pData->_note == cdefTweakE) // Tweak command
					{
						const float value(((pData->_cmd * 256) + pData->_parameter) / 65535.0f);
						SetParameter(pData->_inst, value);
						#if !defined _WINAMP_PLUGIN_
							Global::_pSong->Tweaker = true;
						#endif
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
							TWSCurrent[i] = GetParameter(TWSInst[i]);
							TWSDelta[i] = ((TWSDestination[i] - TWSCurrent[i]) * TWEAK_SLIDE_SAMPLES) / Global::_pSong->SamplesPerTick;
							TWSSamples = 0;
							TWSActive = true;
						}
						else
						{
							// we have used all our slots, just send a twk
							const float value(((pData->_cmd * 256) + pData->_parameter) / 65535.0f);
							SetParameter(pData->_inst, value);
						}
						#if !defined _WINAMP_PLUGIN_
							Global::_pSong->Tweaker = true;
						#endif
					}
				}
			}

			void fx::Work(int numSamples)
			{
				Machine::Work(numSamples);
				#if !defined _WINAMP_PLUGIN_
					CPUCOST_INIT(cost);
				#endif
				if((!_mute) && (!_stopped) && (!_bypass))
				{
					if(instantiated)
					{
						if(wantidle) 
						{
							try
							{
								Dispatch(effIdle, 0, 0, NULL, 0.0f);
							}
							catch (...)
							{
								// o_O`
							}
						}
						SendMidi();
						Dsp::Undenormalize(_pSamplesL, _pSamplesR, numSamples);
						if(_pEffect->numInputs == 1)
						{
							///\todo MIX input0 and input1!
						}
						if(!(_pEffect->flags & effFlagsCanReplacing || requiresRepl))
						{
							Dsp::Clear(_pOutSamplesL, numSamples);
							Dsp::Clear(_pOutSamplesR, numSamples);
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
								if((_pEffect->flags & effFlagsCanReplacing) || requiresRepl)
									_pEffect->processReplacing(_pEffect, tempinputs, tempoutputs, ns);
								else
									_pEffect->process(_pEffect, tempinputs, tempoutputs, ns);
								ns = 0;
							}
							else
							{
								if(nextevent)
								{
									ns -= nextevent;
									if((_pEffect->flags & effFlagsCanReplacing) || requiresRepl)
										_pEffect->processReplacing(_pEffect, tempinputs, tempoutputs, nextevent);
									else
										_pEffect->process(_pEffect, tempinputs, tempoutputs, nextevent);
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
									if(TriggerDelay[i]._cmd == 0xfd)
									{
										if(TriggerDelayCounter[i] == nextevent)
										{
											// do event
											Tick(i, &TriggerDelay[i]);
											TriggerDelay[i]._cmd = 0;
										}
										else TriggerDelayCounter[i] -= nextevent;
									}
									else if(TriggerDelay[i]._cmd == 0xfb)
									{
										if(TriggerDelayCounter[i] == nextevent)
										{
											// do event
											Tick(i, &TriggerDelay[i]);
											TriggerDelayCounter[i] = (RetriggerRate[i] * Global::_pSong->SamplesPerTick) / 256;
										}
										else TriggerDelayCounter[i] -= nextevent;
									}
									else if(TriggerDelay[i]._cmd == 0xfa)
									{
										if(TriggerDelayCounter[i] == nextevent)
										{
											// do event
											Tick(i, &TriggerDelay[i]);
											TriggerDelayCounter[i] = (RetriggerRate[i] * Global::_pSong->SamplesPerTick) / 256;
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
						if( _pEffect->numOutputs == 1) std::memcpy(outputs[1], outputs[0], numSamples * sizeof(float));
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
					#if !defined _WINAMP_PLUGIN_
					// volume "counter"
					{
						_volumeCounter = Dsp::GetMaxVSTVol(_pSamplesL, _pSamplesR,numSamples) * 32768.0f;
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
					#endif
				}
				#if !defined _WINAMP_PLUGIN_
					CPUCOST_CALC(cost, numSamples);
					_cpuCost += cost;
				#endif
				_worked = true;
			}



			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// old file format vomit, don't look at it.



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
						delete chunk;
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
						delete chunk;
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
				#if defined (_WINAMP_PLUGIN_)
					pFile->Skip(96) ; // sizeof(CPoint) = 8.
				#else
					pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
				#endif
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

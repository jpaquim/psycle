///\file
///\brief implementation file for psycle::host::Machine
#include <project.private.hpp>
#include "global.hpp"
//#include "machine.hpp"
//#include "song.hpp"
//#include <psycle/helpers/dsp.hpp>
//#include "configuration.hpp"
#include "player.hpp"
#include <psycle/host/psycle.hpp> // Can this be removed?
//#include <psycle/host/WireDlg.hpp> // Can this be removed?
#include <psycle/host/MainFrm.hpp> // Can this be removed?
#include "VstEditorDlg.hpp"
//#include <psycle/host/InputHandler.hpp> // Can this be removed?
//#include <universalis/processor/exception.hpp>

// The inclusion of the following headers is needed because of a bad design.
// The use of these subclasses in a function of the base class should be 
// moved to the Song loader.
//#include <psycle/engine/Sampler.hpp>
//#include <psycle/engine/XMSampler.hpp>
//#include <psycle/engine/plugin.hpp>
#include "vsthost24.hpp"
namespace seib{
	namespace vst{
		extern const char* hostCanDos [];
		extern const char* plugCanDos [];
	}
}

namespace psycle
{
	namespace host
	{
		extern CPsycleApp theApp;

		namespace vst
		{
			using namespace seib::vst;

			/*****************************************************************************/
			/* GetPreviousPlugIn : returns predecessor to this plugin                    */
			/* This function is identified in the VST docs as "for future expansion",	 */
			/* and in fact there is a bug in the audioeffectx.cpp (in the host call)	 */
			/* where it forgets about the index completely.								 */
			/*****************************************************************************/
			plugin* host::GetPreviousPlugIn(CEffect & pEffect, int pinIndex)
			{
				/* What this function might have to do:
				if (pinIndex == -1)
				return "Any-plugin-which-is-input-connected-to-this-one";
				else if (pinIndex < numInputs ) 
				return Input_plugin[pinIndex];
				return 0;
				*/
				return 0;
			}

			/*****************************************************************************/
			/* GetNextPlugIn : returns successor to this plugin                          */
			/* This function is identified in the VST docs as "for future expansion",	 */
			/* and in fact there is a bug in the audioeffectx.cpp (in the host call)	 */
			/* where it forgets about the index completely.								 */
			/*****************************************************************************/

			plugin* host::GetNextPlugIn(CEffect & pEffect, int pinIndex)
			{
				/* What this function might have to do:
				if (pinIndex == -1)
				return "Any-plugin-which-is-output-connected-to-this-one";
				else if (pinIndex < numOutputs ) 
				return Output_plugin[pinIndex];
				return 0;
				*/
				return 0;
			}

/*			Machine* host::CreateFromType(int _id, std::string _dllname)
			{
				//\todo;
				//return new;
			}
*/
			void host::CalcTimeInfo(long lMask)
			{
				// Move TranportPlaying, cycleactive and recording to a "Start()" function.
				// create(?) a "tick" function called each work cycle in order to reset transportchanged,
				// automationwriting and automationreading.
				//
				/*
				kVstTransportChanged 		= 1,		// Indicates that Playing, Cycle or Recording has changed
				kVstTransportPlaying 		= 1 << 1,
				kVstTransportCycleActive	= 1 << 2,
				kVstTransportRecording		= 1 << 3,

				kVstAutomationWriting		= 1 << 6,
				kVstAutomationReading		= 1 << 7,
				*/

				if((Global::pPlayer)->_playing) 
				{
					vstTimeInfo.flags |= kVstTransportPlaying;
					if(((Master *) (Global::song()._pMachine[MASTER_INDEX]))->sampleCount == 0)
						vstTimeInfo.flags |= kVstTransportChanged;
				}
				if(Global::song()._pMachine[MASTER_INDEX]) // This happens on song loading with new fileformat.
				{
					vstTimeInfo.samplePos = ((Master *) (Global::song()._pMachine[MASTER_INDEX]))->sampleCount;
				}
				else vstTimeInfo.samplePos = 0;

				//\todo : ensure that samplePos is correct before calling it.
				// The base class function gives SampleRate, kVstTimeSigValid, kVstSmpteValid and kVstPpqPosValid
				CVSTHost::CalcTimeInfo(lMask);

				if(lMask & kVstTempoValid)
				{
					vstTimeInfo.flags |= kVstTempoValid;
					vstTimeInfo.tempo = Global::player().bpm;
				}
				//kVstCyclePosValid			= 1 << 12,	// start and end
				//	cyclestart // locator positions in quarter notes.
				//	cycleend   // locator positions in quarter notes.

				if(lMask & kVstNanosValid)
				{
					vstTimeInfo.flags |= kVstNanosValid;
					vstTimeInfo.nanoSeconds = cpu::cycles() / Global::cpu_frequency() * 1e9; //::GetTickCount(); ::timeGetTime(); // error C3861: 'timeGetTime': identifier not found, even with argument-dependent lookup
				}

				//kVstBarsValid				= 1 << 11,
				//	barstartpos,  ( 10.25ppq , 1ppq = 1 beat). Seems like ppqPos, but instead of sample pos, the last bar.
				//kVstClockValid 				= 1 << 15
				//	samplestoNextClock, how many samples from the current position to the next 24ppq.  ( i.e. 1/24 beat )

			}


			bool host::OnCanDo(CEffect &pEffect, const char *ptr)
			{
				bool value =  CVSTHost::OnCanDo(pEffect,ptr);
				if (value) return value;
				else if (
					//||	(!strcmp(ptr, hostCanDos[3] )) // "receiveVstEvents",
					//||	(!strcmp(ptr, hostCanDos[4] )) // "receiveVstMidiEvent",
					//||	(!strcmp(ptr, hostCanDos[5] )) // "receiveVstTimeInfo",

					(!strcmp(ptr, hostCanDos[6] )) // "reportConnectionChanges",
					//||	(!strcmp(ptr, hostCanDos[7] )) // "acceptIOChanges",
					||	(!strcmp(ptr, hostCanDos[8] )) // "sizeWindow",

					//||	(!strcmp(ptr, hostCanDos[9] )) // "asyncProcessing",
					//||	(!strcmp(ptr, hostCanDos[10] )) // "offline",
					||	(!strcmp(ptr, hostCanDos[11] )) // "supplyIdle",
					//||	(!strcmp(ptr, hostCanDos[12] )) // "supportShell",
					||	(!strcmp(ptr, hostCanDos[13] )) // "openFileSelector"
					//||	(!strcmp(ptr, hostCanDos[14] )) // "editFile",
					||	(!strcmp(ptr, hostCanDos[15] )) // "closeFileSelector"
					||	(!strcmp(ptr, hostCanDos[16] )) // "startStopProcess"
					)
					return true;
				return false;                           /* per default, no.                  */
			}
			long host::OnTempoAt(CEffect &pEffect, long pos)
			{
				//  pos in Sample frames, return bpm* 10000
				return 0;
			}
			long host::OnGetOutputLatency(CEffect &pEffect)
			{
				//\todo : return Global::pConfig->_pOutputDriver->LatencyInSamples();
				AudioDriver* pdriver = Global::pConfig->_pOutputDriver;

				return (pdriver->_numBlocks*pdriver->_blockSize)/4;
			}

			void host::OnIdle(CEffect &pEffect)
			{	//\todo:
				return CVSTHost::OnIdle(pEffect);
			}
			bool host::OnNeedIdle(CEffect &pEffect)
			{	//\todo:
				return CVSTHost::OnNeedIdle(pEffect);
			}

			// Get information about the following five functions, and especially on how the host automates the plugins
			// (i.e. the inverse step)
			long host::OnGetNumAutomatableParameters(CEffect &pEffect) { return 0; }
			long host::OnGetAutomationState(CEffect &pEffect) { return 0; }
			bool host::OnBeginEdit(CEffect &pEffect,long index) { return false; }
			bool host::OnEndEdit(CEffect &pEffect,long index) { return false; }
			void host::OnSetParameterAutomated(CEffect &pEffect, long index, float value)
			{
				Global::player().Tweaker = true;
				if(index<0 || index >= pEffect.numParams()) {
					psycle::host::loggers::info("error audioMasterAutomate: index<0 || index >= pEffect.numParams()");
				}
				// Send the event to the IOhandler. It will know what to do. It's not the host's work.
				if(Global::configuration()._RecordTweaks)
				{
					if(Global::configuration()._RecordMouseTweaksSmooth)
						((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweakSlide(((plugin*)&pEffect)->_macIndex, index, f2i(value * quantization));
					else
						((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweak(((plugin*)&pEffect)->_macIndex, index, f2i(value * quantization));
				}
				///\todo: This refresh shouldn't come here, player().Tweaker should do it.
				if(((plugin*)&pEffect)->editorWnd)
					((CVstEditorDlg *) ((plugin*)&pEffect)->editorWnd)->Refresh(index, value);

				return;
			}

			bool host::OnOpenFileSelector (CEffect &pEffect, VstFileSelect *ptr)
			{
				if (!ptr)
					throw (int)1;

				char fileName[_MAX_PATH];
				char *filePath;

				if	((ptr->command == kVstFileLoad) 
				||	(ptr->command == kVstFileSave)
				||	(ptr->command == kVstMultipleFilesLoad))
				{
					OPENFILENAME ofn = {0}; // common dialog box structure
					ofn.lStructSize = sizeof(OPENFILENAME);

					std::ostringstream filefilter;
					for (int i=0;i<ptr->nbFileTypes;i++)
					{
						filefilter << ptr->fileTypes[i].name << "\0" << ptr->fileTypes[i].dosType << "\0";
					}
					filefilter << "All (*.*)"<< "\0" << "*" << "\0";

					if (ptr->command == kVstMultipleFilesLoad)
						filePath = new char [_MAX_PATH * 100];
					else
						filePath = new char[_MAX_PATH];

					filePath[0] = 0;
					// Initialize OPENFILENAME
					ofn.hwndOwner = ((CMainFrame *) theApp.m_pMainWnd)->m_hWnd;

					ofn.lpstrFile = filePath;
					ofn.nMaxFile    = sizeof (filePath) - 1;
					ofn.lpstrFilter =filefilter.str().c_str();
					ofn.nFilterIndex = 1;
					ofn.lpstrTitle = ptr->title;
					ofn.lpstrFileTitle = fileName;
					ofn.nMaxFileTitle = sizeof(fileName) - 1;
					ofn.lpstrInitialDir = ptr->initialPath;
					if (ptr->nbFileTypes >= 1)
						ofn.lpstrDefExt = ptr->fileTypes[0].dosType;
					if (ptr->command == kVstFileSave)
						ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING;
					else
						ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING;

					// Display the Open dialog box. 
					if(::GetOpenFileName(&ofn)==TRUE)
					{
						if (ptr->command == kVstMultipleFilesLoad)
						{
							char string[_MAX_PATH], directory[_MAX_PATH];
							char *previous = ofn.lpstrFile;
							long len;
							bool dirFound = false;
							ptr->returnMultiplePaths = new char*[_MAX_PATH];
							long i = 0;
							while (*previous != 0)
							{
								if (!dirFound) 
								{
									dirFound = true;
									strcpy (directory, previous);
									len = strlen (previous) + 1;  // including 0
									previous += len;

									if (*previous == 0)
									{  // 1 selected file only		
										ptr->returnMultiplePaths[i] = new char [strlen (directory) + 1];
										strcpy (ptr->returnMultiplePaths[i++], directory);
									}
									else
									{
										if (directory[strlen (directory) - 1] != '\\')
											strcat (directory, "\\");
									}
								}
								else 
								{
									sprintf (string, "%s%s", directory, previous);
									len = strlen (previous) + 1;  // including 0
									previous += len;

									ptr->returnMultiplePaths[i] = new char [strlen (string) + 1];
									strcpy (ptr->returnMultiplePaths[i++], string);
								}
							}
							ptr->nbReturnPath = i;
							delete filePath;
						}
						else if ( ptr->returnPath == 0 )
						{
							ptr->reserved = 1;
							ptr->returnPath = filePath;
							ptr->sizeReturnPath = sizeof(filePath);
							ptr->nbReturnPath = 1;
						}
						else 
						{
							strncpy(ptr->returnPath,filePath,ptr->sizeReturnPath);
							ptr->nbReturnPath = 1;
							delete filePath;
						}
						return true;
					}
					else delete filePath;
				}
				else if (ptr->command == kVstDirectorySelect)
				{
					LPMALLOC pMalloc;
					// Gets the Shell's default allocator
					//
					if (::SHGetMalloc(&pMalloc) == NOERROR)
					{
						BROWSEINFO bi;
						LPITEMIDLIST pidl;
						if ( ptr->returnPath == 0)
						{
							ptr->reserved = 1;
							ptr->returnPath = new char[_MAX_PATH];
							ptr->sizeReturnPath = _MAX_PATH;
							ptr->nbReturnPath = 1;
						}
						// Get help on BROWSEINFO struct - it's got all the bit settings.
						//
						bi.hwndOwner = ((CMainFrame *) theApp.m_pMainWnd)->m_hWnd;

						bi.pidlRoot = NULL;
						bi.pszDisplayName = ptr->returnPath;
						bi.lpszTitle = ptr->title;
						bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
						bi.lpfn = NULL;
						bi.lParam = 0;
						// This next call issues the dialog box.
						//
						if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
						{
							if (::SHGetPathFromIDList(pidl, ptr->returnPath))
							{
								return true;
							}
							// Free the PIDL allocated by SHBrowseForFolder.
							//
							pMalloc->Free(pidl);
						}
						if ( ptr->reserved ) { delete ptr->returnPath; ptr->reserved = 0; }
						// Release the shell's allocator.
						//
						pMalloc->Release();
					}
				}
				return false;
			}
			bool host::OnCloseFileSelector (CEffect &pEffect, VstFileSelect *ptr)
			{
				if ( ptr->command == kVstMultipleFilesLoad)
				{
					for (int i=0; i < ptr->nbReturnPath;i++)
					{
						delete[] ptr->returnMultiplePaths[i];
					}
					delete[] ptr->returnMultiplePaths;
					return true;
				}
				else if ( ptr->reserved == 1) 
				{
					delete ptr->returnPath;
					return true;
				}
				return false;
			}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

			plugin::plugin(LoadedAEffect &loadstruct)
				:CEffect(loadstruct)
				,queue_size(0)
				,requiresRepl(0)
				,requiresProcess(0)
				, editorWnd(0)
			{
				if ( IsSynth())
				{
					_mode=MACHMODE_GENERATOR; _type=MACH_VST;
				}
				else 
				{
					_mode=MACHMODE_FX; _type=MACH_VSTFX;
				}

				std::memset(junk, 0, STREAM_SIZE * sizeof(float));
				for(int i(0) ; i < vst::max_io ; ++i)
				{
					inputs[i]=junk;
					outputs[i]=junk;
				}
				_pOutSamplesL = new float[STREAM_SIZE];
				_pOutSamplesR = new float[STREAM_SIZE];
				dsp::Clear(_pOutSamplesL, STREAM_SIZE);
				dsp::Clear(_pOutSamplesR, STREAM_SIZE);
				inputs[0] = _pSamplesL;
				inputs[1] = _pSamplesR;
				outputs[0] = _pOutSamplesL;
				outputs[1] = _pOutSamplesR;
				for(int i(0) ; i < MAX_TRACKS; ++i)
				{
					trackNote[i].key = 255; // No Note.
					trackNote[i].midichan = 0;
				}
				_sDllName=loadstruct.sFileName;
				char temp[64];
				if (GetEffectName(temp) && temp[0])_sProductName=temp;
				else if(GetProductString(temp) && temp[0]) _sProductName=temp;
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
				if(GetVendorString(temp) && temp[0]) _sVendorName = temp;
				else _sVendorName = "Unknown vendor";
				std::strcpy(_editName,_sProductName.c_str());
				MainsChanged(true);
			}

			plugin::~plugin()
			{
			}
			void plugin::GetParamValue(int numparam, char * parval)
			{
				try
				{
					if(numparam < numParams())
					{
						if(!DescribeValue(numparam, parval))
						{
							std::sprintf(parval,"%.0f",GetParameter(numparam) * quantization);
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
				if(parameter < numParams())
				{
					char par_display[64]={0};
					char par_label[64]={0};
					GetParamDisplay(parameter,par_display);
					GetParamLabel(parameter,par_label);
					std::sprintf(psTxt, "%s(%s)", par_display, par_label);
					return true;
				}
				else std::sprintf(psTxt, "Invalid NumParams Value");
				return false;
			}
			bool plugin::LoadSpecificChunk(RiffFile * pFile, int version)
			{
				UINT size;
				unsigned char _program;
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
					SetProgram(_program);

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
						if(ProgramIsChunk())
						{
							char * data(new char[size]);
							pFile->Read(data, size); // Number of parameters
							SetChunk(data,size);
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

			void plugin::SaveSpecificChunk(RiffFile * pFile) 
			{
				UINT count(numParams());
				unsigned char _program=0;
				UINT size(sizeof _program + sizeof count + sizeof(float) * count);
				char * pData(0);
				bool b = ProgramIsChunk();
				if(b)
				{
					size += GetChunk((void**)&pData);
				}
				pFile->Write(&size, sizeof size);
				_program = static_cast<unsigned char>(GetProgram());
				pFile->Write(&_program, sizeof _program);
				pFile->Write(&count, sizeof count);
				for(UINT i(0); i < count; ++i)
				{
					float temp = GetParameter(i);
					pFile->Write(&temp, sizeof temp);
				}
				if(b)
				{
					pFile->Write(pData, size);
				}
			};
			void plugin::SaveDllName(RiffFile * pFile) 
			{
				::CString str = GetDllName();
				char str2[1 << 10];
				std::strcpy(str2, str.Mid(str.ReverseFind('\\') + 1));
				pFile->Write(str2, std::strlen(str2) + 1);
			};
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
				if(trackNote[channel].key != 255)
					AddNoteOff(channel, trackNote[channel].key, true);

				if(AddMIDI(0x90 | midichannel /*Midi On*/, key, velocity)) {
					note thisnote;
					thisnote.key = key;
					thisnote.midichan = midichannel;
					trackNote[channel] = thisnote;
					return true;
				}
				return false;
			}

			bool plugin::AddNoteOff(unsigned char channel, unsigned char midichannel, bool addatStart)
			{
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
				assert(queue_size >= 0);
				assert(queue_size <= MAX_VST_EVENTS);


				if(queue_size > 0)
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

/*						// assert that the note sequence is well-formed,
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
*/
#endif
						mevents.events[q] = (VstEvent*) &midievent[q];
					}

					queue_size = 0;
					//Finally Send the events.
					ProcessEvents(reinterpret_cast<VstEvents*>(&mevents));
				}
			}

			void plugin::Tick(int channel, PatternEntry * pData)
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
							TWSCurrent[i] = GetParameter(TWSInst[i]);
							TWSCurrent[i] = 0;
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

			void plugin::Stop()
			{
					for(int i(0) ; i < MAX_TRACKS ; ++i) AddNoteOff(i);
					// <alk>
					// has been commented out because it crashes Z3ta+
					// and doesnt seem to be needed.
					/*
					for(int i(0) ; i < 16 ; ++i) AddMIDI(0xb0 + i, 0x7b); // Reset all controllers
					*/
			}

			void plugin::Work(int numSamples)
			{
				if(_mode != MACHMODE_GENERATOR) Machine::Work(numSamples);
				CPUCOST_INIT(cost);
				if((!_mute) && (!_stopped) && (!_bypass))
				{
					if(bNeedIdle) 
					{
						try
						{
							Idle();
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
						if(numInputs() == 1)
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
						if(!(CanProcessReplace() || requiresRepl))
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
								if(CanProcessReplace() || requiresRepl)
									ProcessReplacing(tempinputs, tempoutputs, ns);
								else
									Process(tempinputs, tempoutputs, ns);
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
									if(CanProcessReplace() || requiresRepl)
										ProcessReplacing(tempinputs, tempoutputs, nextevent);
									else
										Process(tempinputs, tempoutputs, nextevent);
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
						if(numOutputs() == 1) std::memcpy(outputs[1], outputs[0], numSamples * sizeof(float));
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
				CPUCOST_CALC(cost, numSamples);
				_cpuCost += cost;
				_worked = true;
			}



			// Load for Old Psycle fileformat
			bool plugin::LoadChunk(RiffFile * pFile)
			{
				bool b;
				try
				{
					b = ProgramIsChunk();
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
					SetChunk(chunk,chunk_size);
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
			bool plugin::PreLoad(RiffFile * pFile, unsigned char &_program, int &_instance)
			{
				char junkdata[256];
				std::memset(&junkdata, 0, sizeof(junkdata));
				Machine::Init();

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

				pFile->Read(&junkdata[0], sizeof(int)); // outwet
				pFile->Read(&junkdata[0], sizeof(int)); // outdry

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
			bool plugin::LoadFromMac(vst::plugin *pMac)
			{
				Machine::Init();
				strcpy(_editName,pMac->_editName);
				memcpy(_inputMachines,pMac->_inputMachines,sizeof(_inputMachines));
				memcpy(_outputMachines,pMac->_outputMachines,sizeof(_outputMachines));
				memcpy(_inputConVol,pMac->_inputConVol,sizeof(_inputConVol));
				memcpy(_connection,pMac->_connection,sizeof(_connection));
				memcpy(_inputCon,pMac->_inputCon,sizeof(_inputCon));
				memcpy(_connectionPoint,pMac->_connectionPoint,sizeof(_connectionPoint));
				_numInputs= pMac->_numInputs;
				_numOutputs= pMac->_numOutputs;
				
				Machine::SetPan(pMac->_panning);
				return true;
			}


		}
	}
}

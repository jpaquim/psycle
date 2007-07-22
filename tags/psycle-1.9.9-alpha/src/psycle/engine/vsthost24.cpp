///\file
///\brief implementation file for psycle::host::Machine
#include <packageneric/pre-compiled.private.hpp>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <psycle/engine/global.hpp>
//#include "machine.hpp"
//#include "song.hpp"
//#include <psycle/helpers/dsp.hpp>
//#include "configuration.hpp"

//#include <psycle/host/psycle.hpp> // Can this be removed?
//#include <psycle/host/WireDlg.hpp> // Can this be removed?
//#include <psycle/host/MainFrm.hpp> // Can this be removed?
//#include <psycle/host/InputHandler.hpp> // Can this be removed?
//#include <universalis/processor/exception.hpp>

// The inclusion of the following headers is needed because of a bad design.
// The use of these subclasses in a function of the base class should be 
// moved to the Song loader.
//#include <psycle/engine/Sampler.hpp>
//#include <psycle/engine/XMSampler.hpp>
//#include <psycle/engine/plugin.hpp>
#include <psycle/engine/vsthost24.hpp>
namespace psycle
{
	namespace engine
	{
		namespace vst
		{
			extern const char* hostCanDos [];
			extern const char* plugCanDos [];
			extern CVSTHost* CVSTHost::pHost;

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

			Machine* host::CreateFromType(Machine::id_type _id, std::string _dllname)
			{
				//\todo;
				//return new;
			}

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
					host::loggers::info("error audioMasterAutomate: index<0 || index >= pEffect.numParams()");
				}
				// Send the event to the IOhandler. It will know what to do. It's not the host's work.
				if(Global::configuration()._RecordTweaks)
				{
					if(Global::configuration()._RecordMouseTweaksSmooth)
						((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweakSlide(host->id(), index, f2i(opt * quantization));
					else
						((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweak(host->id(), index, f2i(opt * quantization));
				}
				// This refresh shouldn't come here, player().Tweaker should do it.
				if(host->editorWnd)
					((CVstEditorDlg *) host->editorWnd)->Refresh(index, opt);

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

					std::string filefilter;
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
					ofn.hwndOwner = GetParent()->m_hWnd;

					ofn.lpstrFile = filePath;
					ofn.nMaxFile    = sizeof (filePath) - 1;
					ofn.lpstrFilter =filefilter.c_str();
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
						bi.hwndOwner = GetParent()->m_hWnd;

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


		plugin::plugin(id_t id,LoadedAEffect &loadstruct)
			:CEffect(loadstruct)
		{
			if ( IsSynth())
				Machine::Machine(MachineClass::vstplug,MachineMode::generator,id);
			else 
				Machine::Machine(MachineClass::vstfxplug,MachineMode::effect,id);
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
						std::sprintf(parval,"%.0f",GetParameter(numparam) * CVSTHost::pHost->quantization);
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
		}
	}
}

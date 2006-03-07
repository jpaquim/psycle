
/*****************************************************************************/
/* ImplVSTHost.cpp: CImplVSTHost / plugin implementation                */
/* Work Derived from vsthost. Copyright (c) H. Seib, 2002-2005               */
/* (http://www.hermannseib.com/english/vsthost.htm)"						 */
/*****************************************************************************/
/*                                                                           */
/* $Archive::                                                              $ */
/* $Revision$ */
/* $Date$ */
/* $Author$ */
/* $Modtime::                                                              $ */
/* $Workfile::                                                             $ */
/*                                                                           */
/*****************************************************************************/
#include <packageneric/pre-compiled.private.hpp>
#include "ImplVSTHost.hpp"
#include <vst/vstsysex.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define GetApp()  ((CVsthostApp *)AfxGetApp())

namespace psycle {
	namespace host {
		namespace vst2 {
			/*===========================================================================*/
			/* CImplVSTHost class members                                                */
			/*===========================================================================*/

			static const int nMaxMidi = 2000;       /* max. # MIDI messages buffered for */
			/* EffProcess()                      */
			static const int nMaxSysEx = 100;       /* max. # MIDI SysEx messages        */
			/* buffered for EffProcess()         */

			/*****************************************************************************/
			/* CImplVSTHost : constructor                                                */
			/*****************************************************************************/

			CImplVSTHost::CImplVSTHost()
			{
				// for this sample project, I've assumed a maximum buffer size
				// of 1/3 second at 96KHz - presumably, this is MUCH too much
				// and should be tweaked to more reasonable values, since it
				// requires a machine with lots of main memory this way...
				// user configurability would be nice, of course.
				for (int i = 0; i < 2; i++)
					pOutputs[i] = new float[32000];
				pMidis = new DWORD[nMaxMidi * 2];
				nMidis = 0;
				pSysEx = new CMidiMsg[nMaxSysEx];
				nSysEx = 0;
				InitializeCriticalSection(&cs);
			}

			/*****************************************************************************/
			/* ~CImplVSTHost : destructor                                                */
			/*****************************************************************************/

			CImplVSTHost::~CImplVSTHost()
			{
				for (int i = 0; i < 2; i++)
					delete[] pOutputs[i];
				if (pMidis)
					delete[] pMidis;
				if (pSysEx)
					delete[] pSysEx;
			}

			/*****************************************************************************/
			/* OnAudioMasterCallback : redefine the callback a bit                       */
			/*****************************************************************************/

			long CImplVSTHost::OnAudioMasterCallback
				(
				int nEffect,
				long opcode,
				long index,
				long value,
				void *ptr,
				float opt
				)
			{
#if defined(_DEBUG) || defined(_DEBUGFILE)

				switch (opcode)
				{
				case audioMasterAutomate :
					TRACE1("callback: %d audioMasterAutomate\n", nEffect);
					break;
				case audioMasterVersion :
					TRACE1("callback: %d audioMasterVersion\n", nEffect);
					break;
				case audioMasterCurrentId :
					TRACE1("callback: %d audioMasterCurrentId\n", nEffect);
					break;
				case audioMasterIdle :
					TRACE1("callback: %d audioMasterIdle\n", nEffect);
					break;
				case audioMasterPinConnected :
					TRACE1("callback: %d audioMasterPinConnected\n", nEffect);
					break;
					/* VST 2.0 additions...              */
				case audioMasterWantMidi :
					TRACE1("callback: %d audioMasterWantMidi\n", nEffect);
					break;
				case audioMasterGetTime :
					TRACE1("callback: %d audioMasterGetTime\n", nEffect);
					break;
				case audioMasterProcessEvents :
					TRACE1("callback: %d audioMasterProcessEvents\n", nEffect);
					break;
				case audioMasterSetTime :
					TRACE1("callback: %d audioMasterSetTime\n", nEffect);
					break;
				case audioMasterTempoAt :
					TRACE1("callback: %d audioMasterTempoAt\n", nEffect);
					break;
				case audioMasterGetNumAutomatableParameters :
					TRACE1("callback: %d audioMasterGetNumAutomatableParameters\n", nEffect);
					break;
				case audioMasterGetParameterQuantization :
					TRACE1("callback: %d audioMasterGetParameterQuantization\n", nEffect);
					break;
				case audioMasterIOChanged :
					TRACE1("callback: %d audioMasterIOChanged\n", nEffect);
					break;
				case audioMasterNeedIdle :
					TRACE1("callback: %d audioMasterNeedIdle\n", nEffect);
					break;
				case audioMasterSizeWindow :
					TRACE1("callback: %d audioMasterSizeWindow\n", nEffect);
					break;
				case audioMasterGetSampleRate :
					TRACE1("callback: %d audioMasterGetSampleRate\n", nEffect);
					break;
				case audioMasterGetBlockSize :
					TRACE1("callback: %d audioMasterGetBlockSize\n", nEffect);
					break;
				case audioMasterGetInputLatency :
					TRACE1("callback: %d audioMasterGetInputLatency\n", nEffect);
					break;
				case audioMasterGetOutputLatency :
					TRACE1("callback: %d audioMasterGetOutputLatency\n", nEffect);
					break;
				case audioMasterGetPreviousPlug :
					TRACE1("callback: %d audioMasterGetPreviousPlug\n", nEffect);
					break;
				case audioMasterGetNextPlug :
					TRACE1("callback: %d audioMasterGetNextPlug\n", nEffect);
					break;
				case audioMasterWillReplaceOrAccumulate :
					TRACE1("callback: %d audioMasterWillReplaceOrAccumulate\n", nEffect);
					break;
				case audioMasterGetCurrentProcessLevel :
					TRACE1("callback: %d audioMasterGetCurrentProcessLevel\n", nEffect);
					break;
				case audioMasterGetAutomationState :
					TRACE1("callback: %d audioMasterGetAutomationState\n", nEffect);
					break;
				case audioMasterOfflineStart :
					TRACE1("callback: %d audioMasterOfflineStart\n", nEffect);
					break;
				case audioMasterOfflineRead :
					TRACE1("callback: %d audioMasterOfflineRead\n", nEffect);
					break;
				case audioMasterOfflineWrite :
					TRACE1("callback: %d audioMasterOfflineWrite\n", nEffect);
					break;
				case audioMasterOfflineGetCurrentPass :
					TRACE1("callback: %d audioMasterOfflineGetCurrentPass\n", nEffect);
					break;
				case audioMasterOfflineGetCurrentMetaPass :
					TRACE1("callback: %d audioMasterOfflineGetCurrentMetaPass\n", nEffect);
					break;
				case audioMasterSetOutputSampleRate :
					TRACE1("callback: %d audioMasterSetOutputSampleRate\n", nEffect);
					break;
				case audioMasterGetSpeakerArrangement :
					TRACE1("callback: %d audioMasterGetSpeakerArrangement\n", nEffect);
					break;
				case audioMasterGetVendorString :
					TRACE1("callback: %d audioMasterGetVendorString\n", nEffect);
					break;
				case audioMasterGetProductString :
					TRACE1("callback: %d audioMasterGetProductString\n", nEffect);
					break;
				case audioMasterGetVendorVersion :
					TRACE1("callback: %d audioMasterGetVendorVersion\n", nEffect);
					break;
				case audioMasterVendorSpecific :
					TRACE1("callback: %d audioMasterVendorSpecific\n", nEffect);
					break;
				case audioMasterSetIcon :
					TRACE1("callback: %d audioMasterSetIcon\n", nEffect);
					break;
				case audioMasterCanDo :
					TRACE1("callback: %d audioMasterCanDo\n", nEffect);
					break;
				case audioMasterGetLanguage :
					TRACE1("callback: %d audioMasterGetLanguage\n", nEffect);
					break;
				case audioMasterOpenWindow :
					TRACE1("callback: %d audioMasterOpenWindow\n", nEffect);
					break;
				case audioMasterCloseWindow :
					TRACE1("callback: %d audioMasterCloseWindow\n", nEffect);
					break;
				case audioMasterGetDirectory :
					TRACE1("callback: %d audioMasterGetDirectory\n", nEffect);
					break;
				case audioMasterUpdateDisplay :
					TRACE1("callback: %d audioMasterUpdateDisplay\n", nEffect);
					break;
					//---from here VST 2.1 extension opcodes------------------------------------------------------
#ifdef VST_2_1_EXTENSIONS
				case audioMasterBeginEdit :
					TRACE1("callback: %d audioMasterBeginEdit\n", nEffect);
					break;
				case audioMasterEndEdit :
					TRACE1("callback: %d audioMasterEndEdit\n", nEffect);
					break;
				case audioMasterOpenFileSelector :
					TRACE1("callback: %d audioMasterOpenFileSelector\n", nEffect);
					break;
#endif
					//---from here VST 2.2 extension opcodes------------------------------------------------------
#ifdef VST_2_2_EXTENSIONS
				case audioMasterCloseFileSelector :
					TRACE1("callback: %d audioMasterCloseFileSelector\n", nEffect);
					break;
				case audioMasterEditFile :
					TRACE1("callback: %d audioMasterEditFile\n", nEffect);
					break;
				case audioMasterGetChunkFile :
					TRACE1("callback: %d audioMasterGetChunkFile\n", nEffect);
					break;
#endif
					//---from here VST 2.3 extension opcodes------------------------------------------------------
#ifdef VST_2_1_EXTENSIONS
				case audioMasterGetInputSpeakerArrangement :
					TRACE1("callback: %d audioMasterGetInputSpeakerArrangement\n", nEffect);
					break;
#endif
				}
#endif

				return CVSTHost::OnAudioMasterCallback(nEffect, opcode, index, value, ptr, opt);
			}

			/*****************************************************************************/
			/* OnSetParameterAutomated : make sure parameter changes are reflected       */
			/*****************************************************************************/

			bool CImplVSTHost::OnSetParameterAutomated(int nEffect, long index, float value)
			{
				plugin *pEffect = (plugin *)GetAt(nEffect);
				if (pEffect)
					pEffect->OnSetParameterAutomated(index, value);
				//\todo: This return is always false. might need to check for correctness.
				return CVSTHost::OnSetParameterAutomated(nEffect, index, value);
			}


			/*****************************************************************************/
			/* OnProcessEvents : called when an effect sends events                      */
			/*****************************************************************************/

			bool CImplVSTHost::OnProcessEvents(int nEffect, VstEvents *events)
			{
				int i;
				CMidiMsg msg;

				for (i = 0; i < events->numEvents; i++) /* process all sent MIDI events      */
				{
					switch (events->events[i]->type)
					{
					case kVstMidiType :                 /* normal MIDI message?              */
						msg.Set(((VstMidiEvent *)(events->events[i]))->midiData, 3);
						MidiOut.Output(msg);              /* send it to MIDI Out               */
						break;
					case kVstSysExType :                /* SysEx message ?                   */
						msg.Set(((VstMidiSysexEvent *)(events->events[i]))->sysexDump,
							((VstMidiSysexEvent *)(events->events[i]))->dumpBytes);
						MidiOut.Output(msg);              /* send it to MIDI Out               */
						break;
					}
				}

				return CVSTHost::OnProcessEvents(nEffect, events);
			}

			/*****************************************************************************/
			/* OnSamples : called when sample data come in                               */
			/*****************************************************************************/

			void CImplVSTHost::OnSamples
				(
				float **pBuffer,
				int nLength,
				int nChannels,
				DWORD dwStamp
				)
			{
				int nEffs = 0;                          /* # contributing effects            */
				int i, j, k;                            /* loop counters                     */
				int nSize = GetSize();                  /* get # affected effects            */
				int nm = nMidis;
				int ns = nSysEx;
				plugin *pEff, *pNextEff;
				float *pBuf1, *pBuf2;
				CVsthostApp *pApp = GetApp();
				float *pEmpty = pApp->GetEmptyBuffer();

				// NO optimizations in here, as well as NO checks for overruns etc...

				vstTimeInfo.nanoSeconds = (double)timeGetTime() * 1000000.0L;
				CalcTimeInfo();

				BYTE *pEvData = CreateMIDIEvents(nLength, dwStamp, nm, ns);

				for (i = 0; i < nSize; i++)             /* pass them through all interested  */
				{                                     /* effects                           */
					pEff = (plugin *)GetAt(i);
					if ((!pEff) ||                        /* if no effect at this position     */
						(pEff->GetPrev()))                /* or it's part of a chain           */
						continue;                           /* skip this one                     */

					/* pass through the effect           */
					PassThruEffect(pEff,
						pBuffer, nLength, nChannels,
						pEvData,
						true);

					while ((pNextEff = pEff->GetNext()))  /* while more in chain               */
					{
						PassThruEffect(pNextEff,            /* pass these through next effect    */
							pEff->GetOutputBuffers(), nLength, nChannels,
							pEvData,
							true);
						pEff = pNextEff;                    /* and advance to next in chain      */
					}

					nEffs++;                              /* increment #participating effects  */
				}

				if (pEvData)                            /* delete eventual MIDI event data   */
					delete pEvData;
				/* make sure we don't lose new events*/
				EnterCriticalSection(&cs);
				if (nm < nMidis)
					for (i = nm; i < nMidis; i++)
						pMidis[i - nm] = pMidis[i];
				nMidis -= nm;
				if (ns < nSysEx)
					for (i = ns; i < nSysEx; i++)
						pSysEx[i - ns] = pSysEx[i];
				nSysEx -= ns;
				LeaveCriticalSection(&cs);

				if (!nEffs)                             /* if no effects there               */
				{                                     /* just mute input                   */
					for (j = 0; j < nChannels; j++)
					{
						pBuf1 = pBuffer[j];
						memcpy(pBuf1, pEmpty, nLength * sizeof(float));
					}
					pApp->SendResult(pBuffer, nLength);   /* and blow out some silence         */
				}
				else                                    /* otherwise                         */
				{
					int nOut = 0;
					// this is a hard-wired mixer - should be replaced with a "real" mixer
					float fMult = (float)sqrt(1.0f / nEffs);

					for (i = 0; i < GetSize(); i++)       /* merge all generated outputs       */
					{
						// VERY simple version... all loaded effects' outputs are merged
						// into a parallelized stream of stereo samples; no chaining, fixed mixing;
						// no sample rate conversion or the like;
						// the # effects determines a single effect's contribution to the output.

						pEff = (plugin *)GetAt(i);
						if ((!pEff) ||                      /* if no effect at this position     */
							(pEff->GetPrev()))              /* or it's part of a chain           */
							continue;

						while ((pNextEff = pEff->GetNext()))  /* walk to last in chain           */
							pEff = pNextEff;                  /* which has the final buffers       */

						pEff->EnterCritical();              /* make sure we don't get killed now */

						for (j = 0; j < 2; j++)             /* do for our 2 outputs              */
						{
							pBuf1 = pOutputs[j];
							pBuf2 = pEff->GetOutputBuffer(j);
							if ((!pBuf1) || (!pBuf2))
								break;
							if (!nOut)
								for (k = 0; k < nLength; k++)   /* loop through the samples          */
									*pBuf1++ = *pBuf2++ *fMult;
							else
								for (k = 0; k < nLength; k++)   /* loop through the samples          */
									*pBuf1++ += *pBuf2++ * fMult;
						}

						pEff->LeaveCritical();              /* reallow killing                   */

						nOut++;
					}

					// now that we've got a populated output buffer set, send it to 
					// the Wave Output device.
					pApp->SendResult(pOutputs, nLength);
				}

				vstTimeInfo.samplePos += (float)nLength;
			}

			/*****************************************************************************/
			/* OnCanDo : plugin wants to know whether host can do...                     */
			/*****************************************************************************/

			bool CImplVSTHost::OnCanDo(const char *ptr)
			{
				if ((!strcmp(ptr, "openFileSelector")) ||
					(!strcmp(ptr, "closeFileSelector")) ||
					(!strcmp(ptr, "supplyIdle")) )
					return true;

				return CVSTHost::OnCanDo(ptr);
			}

			/*****************************************************************************/
			/* OnOpenFileSelector : called when an effect needs a file selector          */
			/*****************************************************************************/

			static int CALLBACK BrowseCallbackProc
				(
				HWND hwnd, 
				UINT uMsg, 
				LPARAM lParam, 
				LPARAM lpData
				)
			{
				if (uMsg == BFFM_INITIALIZED)
				{
					if (lpData && *((LPCSTR)lpData))
						::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
				}
				return 0;
			}

			bool CImplVSTHost::OnOpenFileSelector (int nEffect, VstFileSelect *ptr)
			{
#if defined(VST_2_1_EXTENSIONS)
				ptr->reserved = 0;
				if (ptr->command == kVstMultipleFilesLoad)
					return false;  // not yet
				else if ((ptr->command == kVstFileLoad) ||
					(ptr->command == kVstFileSave))
				{
					CString sFilter;

					for (int i = 0; i < ptr->nbFileTypes; i++)
					{
						sFilter += CString(ptr->fileTypes[i].name) +
							" (*." +
							ptr->fileTypes[i].dosType +
							")|*." +
							ptr->fileTypes[i].dosType +
							"|";
					}
					if (!sFilter.IsEmpty())
						sFilter += "|";
					CFileDialog dlg((ptr->command != kVstFileSave),
						NULL,                 /* default extension                 */
						ptr->initialPath,     /* filename                          */
						OFN_HIDEREADONLY |    /* flags                             */
						OFN_OVERWRITEPROMPT
#if 0
						// not yet
						| ((ptr->command == kVstMultipleFilesLoad) ? OFN_ALLOWMULTISELECT : 0)
#endif
						,
						sFilter,              /* filter                            */
						NULL);                /* parent window                     */
					dlg.m_ofn.lpstrTitle = ptr->title;

					if (dlg.DoModal() == IDOK)
					{
						if (!ptr->returnPath)
						{
							ptr->returnPath = new char[dlg.GetPathName().GetLength() + 1];
							ptr->reserved = 1;
						}
						strcpy(ptr->returnPath, dlg.GetPathName());
						ptr->nbReturnPath = 1;
						return true;
					}
				}
				else if (ptr->command == kVstDirectorySelect)
				{
					char szDisplayName[_MAX_PATH];
					BROWSEINFO bi = {0};
					bi.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
					bi.pszDisplayName = szDisplayName;
					bi.lpszTitle = ptr->title;
					bi.ulFlags = BIF_RETURNONLYFSDIRS;
					bi.lpfn = BrowseCallbackProc;
					bi.lParam = (LPARAM)ptr->initialPath;
					LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
					if (pidl)
					{
						if (SHGetPathFromIDList(pidl, szDisplayName))
						{
							if (!ptr->returnPath)
							{
								ptr->returnPath = new char[strlen(szDisplayName) + 1];
								ptr->reserved = 1;
							}
							strcpy(ptr->returnPath, szDisplayName);
							ptr->nbReturnPath = 1;
							return true;
						}
					}
				}

#endif
				return false;
			}

			/*****************************************************************************/
			/* OnCloseFileSelector : called when an effect closes a file selector        */
			/*****************************************************************************/

			bool CImplVSTHost::OnCloseFileSelector (int nEffect, VstFileSelect *ptr)
			{
				if (ptr->reserved == 1)
				{
					delete[] ptr->returnPath;
					ptr->returnPath = 0;
					ptr->reserved = 0;
				}
				return false;
			}

			/*****************************************************************************/
			/* OnGetChunkFile : returns current .fxb / .fxp name                         */
			/*****************************************************************************/

			bool CImplVSTHost::OnGetChunkFile(int nEffect, void * nativePath)
			{
				plugin *pEffect = (plugin *)GetAt(nEffect);
				if (pEffect && pEffect->GetChunkFile().GetLength())
				{
					strcpy((char *)nativePath, pEffect->GetChunkFile());
					return true;
				}
				return false;
			}

			/*===========================================================================*/
			/* plugin class members                                                  */
			/*===========================================================================*/

			/*****************************************************************************/
			/* plugin : constructor                                                  */
			/*****************************************************************************/

			plugin::plugin(CVSTHost *pHost)
				: CEffect(pHost)
			{
				InitializeCriticalSection(&cs);

				//			pEditWnd = pParmWnd = 0;
				inBufs = outBufs = 0;
				nAllocatedInbufs = nAllocatedOutbufs = 0;
				wChnMask = 0xffff;
				//			pPrev = pNext = NULL;
				for(int i(0) ; i < MAX_TRACKS; ++i)
				{
					trackNote[i].key = 255; // No Note.
					trackNote[i].midichan = 0;
				}
			}

			/*****************************************************************************/
			/* ~plugin : destructor                                                  */
			/*****************************************************************************/

			plugin::~plugin()
			{
				int i;

				RemoveFromChain();                      /* remove from chain (if any)        */

				EnterCritical();                        /* inhibit others!                   */

				if (inBufs)
				{
					// these are managed by the effect object, so we 
					// have to delete them here
					for (i = 0; i < nAllocatedInbufs; i++)
						if (inBufs[i])
							delete[] inBufs[i];
					delete[] inBufs;
				}

				if (outBufs)
				{
#if 0
					// these are set by VST Host; it would ruin performance
					// if we copied data into the effect every time
					for (i = 0; i < nAllocatedOutbufs; i++)
						if (outBufs[i])
							delete[] outBufs[i];
#endif
					delete[] outBufs;
				}

				LeaveCritical();                        /* reallow others                    */

				DeleteCriticalSection(&cs);
			}

			/*****************************************************************************/
			/* Load : loads a plugin                                                     */
			/*****************************************************************************/

			bool plugin::Load(const char *name)
			{
				if (!CEffect::Load(name))               /* do basic stuff                    */
					return false;

				int i, j;                               /* loop counter                      */
				if (pEffect->numInputs)                 /* allocate input pointers           */
				{
					inBufs = new float *[pEffect->numInputs];
					if (!inBufs)
						return false;
					/* and the buffers pointed to        */
					for (i = 0; i < pEffect->numinputs; i++)
					{
						// for this sample project, I've assumed a maximum buffer size
						// of 1/4 second at 96KHz - presumably, this is MUCH too much
						// and should be tweaked to more reasonable values, since it
						// requires a machine with lots of main memory this way...
						// user configurability would be nice, of course.
						inBufs[i] = new float[24000];
						if (!outBufs[i])
							return false;
						for (j = 0; j < 24000; j++)
							inBufs[i][j] = 0.f;
					}
					nAllocatedInbufs = pEffect->numInputs;
				}

				if (pEffect->numOutputs)                /* allocate output pointers          */
				{
					outBufs = new float *[pEffect->numOutputs];
					if (!outBufs)
						return false;
					for (i = 0; i < pEffect->numOutputs; i++)
						outBufs[i] = 0;
					nAllocatedOutbufs = pEffect->numOutputs;
				}
				return true;
			}

			/*****************************************************************************/
			/* Unload : removes an effect from memory                                    */
			/*****************************************************************************/

			bool plugin::Unload()
			{
				bool bRC = false;

				__try
				{
					bRC = CEffect::Unload();
				}
				__except (EvalException(GetExceptionCode()))
				{
					// no code here; isn't executed
				}

				return bRC;
			}

			/*****************************************************************************/
			/* OnOpenWindow : called to open yet another window                          */
			/*****************************************************************************/

			void * plugin::OnOpenWindow(VstWindow* window)
			{
				CChildFrame *pFrame = GetFrameWnd();
				if (pFrame)
				{
					CEffectWnd *pWnd;

					pWnd = (CEffectWnd *)GetApp()->CreateChild(pEditWnd ?
						RUNTIME_CLASS(CEffSecWnd) :
					RUNTIME_CLASS(CEffectWnd),
						IDR_EFFECTTYPE,
						pFrame->GetEditMenu());
					if (pWnd)
					{
						HWND hWnd = pWnd->GetSafeHwnd();
						// ignored at the moment: style, position
						ERect rc = {0};
						rc.right = window->width;
						rc.bottom = window->height;
						pWnd->ShowWindow(SW_SHOWNORMAL);
						pWnd->SetEffect(pFrame->GetEffect());
						pWnd->SetMain(pFrame);
						pWnd->SetEffSize(&rc);
						pWnd->SetupTitleText(window->title);
						if (!pEditWnd)
						{
							pFrame->SetEditWnd(pWnd);
							SetEditWnd(pWnd);
							//    EditOpen(hWnd);
						}
						pWnd->SetupTitle();
						window->winHandle = hWnd;
						return pWnd->GetSafeHwnd();
					}

				}
				return 0;                               /* no effect - no window.            */
			}

			/*****************************************************************************/
			/* OnCloseWindow : called to close a window opened in OnOpenWindow           */
			/*****************************************************************************/

			bool plugin::OnCloseWindow(VstWindow* window)
			{
				if ((!window) || (!::IsWindow((HWND)window->winHandle)))
					return false;

				::SendMessage((HWND)window->winHandle, WM_CLOSE, 0, 0);
				window->winHandle = 0;
				return true;
			}

			/*****************************************************************************/
			/* OnSizeEditorWindow : called to resize the editor window                   */
			/*****************************************************************************/

			void plugin::OnSizeEditorWindow(long width, long height)
			{
				if (pEditWnd)
					pEditWnd->SetEffSize((int)width, (int)height);
			}

			/*****************************************************************************/
			/* OnUpdateDisplay() : called when the display is updated                    */
			/*****************************************************************************/

			bool plugin::OnUpdateDisplay()
			{
				pFrameWnd->PostMessage(WM_COMMAND, IDM_DISPLAY_UPDATE);
				return CEffect::OnUpdateDisplay();
			}

			/*****************************************************************************/
			/* EvalException : handle exceptions in called effect                        */
			/*****************************************************************************/

			int plugin::EvalException(int n_except)
			{
				TRACE0("Ouch! Ouch! Effect just died and wants to take us with it! OUCH!\n");

				GetApp()->FullStop();
				return EXCEPTION_CONTINUE_SEARCH;       /* go to next handler                */
			}

			/*****************************************************************************/
			/* EffDispatch : called for any effect method to be sent                     */
			/*****************************************************************************/

			long plugin::Dispatch(long opCode, long index, long value, void *ptr, float opt)
			{
#if defined(_DEBUG) || defined(_DEBUGFILE)
				switch (opCode)
				{
				case effOpen :
					TRACE0("effDispatch: effOpen\n");
					break;
				case effClose :
					TRACE0("effDispatch: effClose\n");
					break;
				case effSetProgram :
					TRACE0("effDispatch: effSetProgram\n");
					break;
				case effGetProgram :
					TRACE0("effDispatch: effGetProgram\n");
					break;
				case effSetProgramName :
					TRACE0("effDispatch: effSetProgramName\n");
					break;
				case effGetProgramName :
					TRACE0("effDispatch: effGetProgramName\n");
					break;
				case effGetParamLabel :
					TRACE0("effDispatch: effGetParamLabel\n");
					break;
				case effGetParamDisplay :
					TRACE0("effDispatch: effGetParamDisplay\n");
					break;
				case effGetParamName :
					TRACE0("effDispatch: effGetParamName\n");
					break;
				case effGetVu :
					TRACE0("effDispatch: effGetVu\n");
					break;
				case effSetSampleRate :
					TRACE0("effDispatch: effSetSampleRate\n");
					break;
				case effSetBlockSize :
					TRACE0("effDispatch: effSetBlockSize\n");
					break;
				case effMainsChanged :
					TRACE0("effDispatch: effMainsChanged\n");
					break;
				case effEditGetRect :
					TRACE0("effDispatch: effEditGetRect\n");
					break;
				case effEditOpen :
					TRACE0("effDispatch: effEditOpen\n");
					break;
				case effEditClose :
					TRACE0("effDispatch: effEditClose\n");
					break;
				case effEditDraw :
					TRACE0("effDispatch: effEditDraw\n");
					break;
				case effEditMouse :
					TRACE0("effDispatch: effEditMouse\n");
					break;
				case effEditKey :
					TRACE0("effDispatch: effEditKey\n");
					break;
				case effEditIdle :
					TRACE0("effDispatch: effEditIdle\n");
					break;
				case effEditTop :
					TRACE0("effDispatch: effEditTop\n");
					break;
				case effEditSleep :
					TRACE0("effDispatch: effEditSleep\n");
					break;
				case effIdentify :
					TRACE0("effDispatch: effIdentify\n");
					break;
				case effGetChunk :
					TRACE0("effDispatch: effGetChunk\n");
					break;
				case effSetChunk :
					TRACE0("effDispatch: effSetChunk\n");
					break;
				case effProcessEvents :
					TRACE0("effDispatch: effProcessEvents\n");
					break;
				case effCanBeAutomated :
					TRACE0("effDispatch: effCanBeAutomated\n");
					break;
				case effString2Parameter :
					TRACE0("effDispatch: effString2Parameter\n");
					break;
				case effGetNumProgramCategories :
					TRACE0("effDispatch: effGetNumProgramCategories\n");
					break;
				case effGetProgramNameIndexed :
					TRACE0("effDispatch: effGetProgramNameIndexed\n");
					break;
				case effCopyProgram :
					TRACE0("effDispatch: effCopyProgram\n");
					break;
				case effConnectInput :
					TRACE0("effDispatch: effConnectInput\n");
					break;
				case effConnectOutput :
					TRACE0("effDispatch: effConnectOutput\n");
					break;
				case effGetInputProperties :
					TRACE0("effDispatch: effGetInputProperties\n");
					break;
				case effGetOutputProperties :
					TRACE0("effDispatch: effGetOutputProperties\n");
					break;
				case effGetPlugCategory :
					TRACE0("effDispatch: effGetPlugCategory\n");
					break;
				case effGetCurrentPosition :
					TRACE0("effDispatch: effGetCurrentPosition\n");
					break;
				case effGetDestinationBuffer :
					TRACE0("effDispatch: effGetDestinationBuffer\n");
					break;
				case effOfflineNotify :
					TRACE0("effDispatch: effOfflineNotify\n");
					break;
				case effOfflinePrepare :
					TRACE0("effDispatch: effOfflinePrepare\n");
					break;
				case effOfflineRun :
					TRACE0("effDispatch: effOfflineRun\n");
					break;
				case effProcessVarIo :
					TRACE0("effDispatch: effProcessVarIo\n");
					break;
				case effSetSpeakerArrangement :
					TRACE0("effDispatch: effSetSpeakerArrangement\n");
					break;
				case effSetBlockSizeAndSampleRate :
					TRACE0("effDispatch: effSetBlockSizeAndSampleRate\n");
					break;
				case effSetBypass :
					TRACE0("effDispatch: effSetBypass\n");
					break;
				case effGetEffectName :
					TRACE0("effDispatch: effGetEffectName\n");
					break;
				case effGetErrorText :
					TRACE0("effDispatch: effGetErrorText\n");
					break;
				case effGetVendorString :
					TRACE0("effDispatch: effGetVendorString\n");
					break;
				case effGetProductString :
					TRACE0("effDispatch: effGetProductString\n");
					break;
				case effGetVendorVersion :
					TRACE0("effDispatch: effGetVendorVersion\n");
					break;
				case effVendorSpecific :
					TRACE0("effDispatch: effVendorSpecific\n");
					break;
				case effCanDo :
					TRACE0("effDispatch: effCanDo\n");
					break;
				case effGetTailSize :
					TRACE0("effDispatch: effGetTailSize\n");
					break;
				case effIdle :
					TRACE0("effDispatch: effIdle\n");
					break;
				case effGetIcon :
					TRACE0("effDispatch: effGetIcon\n");
					break;
				case effSetViewPosition :
					TRACE0("effDispatch: effSetViewPosition\n");
					break;
				case effGetParameterProperties :
					TRACE0("effDispatch: effGetParameterProperties\n");
					break;
				case effKeysRequired :
					TRACE0("effDispatch: effKeysRequired\n");
					break;
				case effGetVstVersion :
					TRACE0("effDispatch: effGetVstVersion\n");
					break;
#if defined(VST_2_1_EXTENSIONS
					//---from here VST 2.1 extension opcodes---------------------------------------------------------
				case effEditKeyDown :
					TRACE0("effDispatch: effEditKeyDown\n");
					break;
				case effEditKeyUp :
					TRACE0("effDispatch: effEditKeyUp\n");
					break;
				case effSetEditKnobMode :
					TRACE0("effDispatch: effSetEditKnobMode\n");
					break;
				case effGetMidiProgramName :
					TRACE0("effDispatch: effGetMidiProgramName\n");
					break;
				case effGetCurrentMidiProgram :
					TRACE0("effDispatch: effGetCurrentMidiProgram\n");
					break;
				case effGetMidiProgramCategory :
					TRACE0("effDispatch: effGetMidiProgramCategory\n");
					break;
				case effHasMidiProgramsChanged :
					TRACE0("effDispatch: effHasMidiProgramsChanged\n");
					break;
				case effGetMidiKeyName :
					TRACE0("effDispatch: effGetMidiKeyName\n");
					break;
				case effBeginSetProgram :
					TRACE0("effDispatch: effBeginSetProgram\n");
					break;
				case effEndSetProgram :
					TRACE0("effDispatch: effEndSetProgram\n");
					break;
#endif
#if defined(VST_2_3_EXTENSIONS)
					//---from here VST 2.3 extension opcodes---------------------------------------------------------
				case effGetSpeakerArrangement :
					TRACE0("effDispatch: effGetSpeakerArrangement\n");
					break;
				case effShellGetNextPlugin :
					TRACE0("effDispatch: effShellGetNextPlugin\n");
					break;
				case effStartProcess :
					TRACE0("effDispatch: effStartProcess\n");
					break;
				case effStopProcess :
					TRACE0("effDispatch: effStopProcess\n");
					break;
				case effSetTotalSampleToProcess :
					TRACE0("effDispatch: effSetTotalSampleToProcess\n");
					break;
				case effSetPanLaw :
					TRACE0("effDispatch: effSetPanLaw\n");
					break;
				case effBeginLoadBank :
					TRACE0("effDispatch: effBeginLoadBank\n");
					break;
				case effBeginLoadProgram :
					TRACE0("effDispatch: effBeginLoadProgram\n");
					break;
#endif
				}
#endif

				long lRC = 0;
				//__try
				{
					lRC = CEffect::Dispatch(opCode, index, value, ptr, opt);
				}
				//__except (EvalException(GetExceptionCode()))
				{
					// no code here; isn't executed
				}

				return lRC;
			}

			/*****************************************************************************/
			/* OnSetParameterAutomated : called when an effect reports a parm change     */
			/*****************************************************************************/

			bool plugin::OnSetParameterAutomated(long index, float value)
			{
				if (pParmWnd)                           /* pass it on.                       */
					return pParmWnd->OnSetParameterAutomated(index, value);
				return false;
			}

			/*****************************************************************************/
			/* EnterCritical : enter critical section                                    */
			/*****************************************************************************/

			void plugin::EnterCritical()
			{
				EnterCriticalSection(&cs);              /* inhibit others!                   */
			}

			/*****************************************************************************/
			/* LeaveCritical : leave critical section                                    */
			/*****************************************************************************/

			void plugin::LeaveCritical()
			{
				LeaveCriticalSection(&cs);              /* reallow others                    */
			}

			/*****************************************************************************/
			/* LoadBank : loads a bank into the effect                                   */
			/*****************************************************************************/

			bool plugin::LoadBank(const char *name)
			{
				CFxBank b((char *)name);
				bool brc = false;

				if ((!b.IsLoaded()) ||
					(pEffect->uniqueID != b.GetFxID()))
					return false;

				if (b.IsChunk())
				{
					if (!(pEffect->flags & effFlagsProgramChunks))
						return false;
					brc = (SetChunk(b.GetChunk(), b.GetChunkSize()) > 0);
				}
				else
				{
					int cProg = GetProgram();
					int i, j;
					int nParms = b.GetNumParams();
					for (i = 0; i < b.GetNumPrograms(); i++)
					{
						SetProgram(i);
						SetProgramName(b.GetProgramName(i));
						for (j = 0; j < nParms; j++)
							SetParameter(j, b.GetProgParm(i, j));

					}
					SetProgram(cProg);
					brc = true;
				}
				if (brc)
					SetChunkFile(name);
				return brc;
			}

			/*****************************************************************************/
			/* OnMidiIn : called when a MIDI message comes in                            */
			/*****************************************************************************/

			void plugin::OnMidiIn(CMidiMsg &msg)
			{
				if (msg[0] > 0xf0)                      /* VSTInstruments don't expect       */
					return;                               /* realtime messages!                */
				else if (msg[0] == 0xf0)                /* but SysEx are processed           */
				{
					EnterCriticalSection(&cs);            /* must not interfere!               */
					if ((pSysEx) &&                       /* if allocated, and                 */
						(nSysEx < nMaxSysEx))             /* if still space for another one    */
						pSysEx[nSysEx++] = msg;             /* copy it in                        */
					LeaveCriticalSection(&cs);
				}
				else
				{
					EnterCriticalSection(&cs);            /* must not interfere!               */
					if (nMidis < 2 * nMaxMidi)            /* if enough place to do it,         */
					{
						pMidis[nMidis++] = msg;             /* buffer the message                */
						pMidis[nMidis++] = msg.GetStamp();  /* and its timestamp                 */
						//  TRACE2("MIDI Message %08lX at %d\n", (DWORD)msg, msg.GetStamp());
					}
					LeaveCriticalSection(&cs);
				}
			}
			/*****************************************************************************/
			/* CreateMIDIEvents : creates a set of VST MIDI events                       */
			/*****************************************************************************/

			BYTE * plugin::CreateMIDIEvents
				(
				int nLength,                        /* # samples                         */
				DWORD dwStamp,                      /* timestamp of 1st sample           */
				int nm,                             /* # normal MIDI messages            */
				int ns                              /* # sysex messages                  */
				)
			{
				if ((!nm) && (!ns))                     /* if neither normal nor sysex there */
					return NULL;                          /* nothing to do in here             */

				BYTE *pEvData;
				int nHdrLen, nTotLen;
				int i;
				DWORD dwDelta;
				VstMidiEvent *pEv;
				VstMidiSysexEvent *pSEv;
				int nSysBase;

				nHdrLen = sizeof(VstEvents) +           /* calculate header length           */
					((nm - 1) * sizeof(VstMidiEvent *)) +
					(ns * sizeof(VstMidiSysexEvent *));
				nTotLen = nHdrLen +                     /* calculate total events length     */
					(nm * sizeof(VstMidiEvent)) +
					(ns * sizeof(VstMidiSysexEvent));
				pEvData = new BYTE[nTotLen];            /* allocate space for events         */
				if (pEvData)                            /* if that worked,                   */
				{                                     /* copy in normal and SysEx          */
					VstEvents *pEvents = (VstEvents *) pEvData;
					memset(pEvents, 0, nTotLen);
					pEvents->numEvents = (nm / 2) + ns;
					for (i = 0; i < nm; i += 2)           /* copy in all normal MIDI messages  */
					{
						pEv = ((VstMidiEvent *)(pEvData + nHdrLen)) + (i / 2);
						pEvents->events[i / 2] = (VstEvent *)pEv;
						pEv->type = kVstMidiType;
						pEv->byteSize = sizeof(VstMidiEvent);
						memcpy(pEv->midiData, pMidis + i, sizeof(DWORD));
						// offset assumes a fixed rate of 44.1kHz and drops 1 percent accuracy
						dwDelta = (pMidis[i + 1] - dwStamp) * 44;
						if (dwDelta > (DWORD)nLength)
							dwDelta = nLength - 1;
						pEv->deltaFrames = dwDelta;
					}
					nm >>= 1;
					nSysBase = nHdrLen + (sizeof(VstMidiEvent) * nm);
					for (i = 0; i < ns; i++)              /* copy in all SysEx messages        */
					{
						pSEv = ((VstMidiSysexEvent *)(pEvData + nSysBase)) + i;
						pEvents->events[nm + i] = (VstEvent *)pSEv;
						pSEv->type = kVstSysExType;
						pSEv->byteSize = sizeof(VstMidiSysexEvent);
						pSEv->deltaFrames = pSysEx[i].GetStamp() - dwStamp;
						pSEv->dumpBytes = pSysEx[i].Length();
						pSEv->sysexDump = pSysEx[i];
						pSEv->flags = pSEv->resvd1 = pSEv->resvd2 = 0;
					}
				}

				return pEvData;
			}

			/*****************************************************************************/
			/* CreateMIDISubset : creates an event subset for a specific effect          */
			/*****************************************************************************/

			BYTE * plugin::CreateMIDISubset(void *pEvData, unsigned short wChnMask)
			{
				if (!pEvData)
					return NULL;

				VstEvents *pEvents = (VstEvents *) pEvData;
				VstMidiEvent *pEv;
				int i, nMatching = 0;
				for (i = pEvents->numEvents - 1; i >= 0; i--)
				{
					pEv = (VstMidiEvent *)pEvents->events[i];
					if ((pEv->type == kVstSysExType) ||   /* SysEx goes to all,                */
						(wChnMask &                       /* for normal MIDI channel has to fit*/
						(1 << (pEv->midiData[0] & 0x0F))))
						nMatching++;
				}
				if (!nMatching)
					return NULL;

				int nHdrLen = sizeof(VstEvents) + 
					((nMatching - 1) * sizeof(VstMidiEvent *));
				BYTE * pEffEvData = new BYTE[nHdrLen];
				VstEvents *pEffEvents = (VstEvents *) pEffEvData;
				pEffEvents->numEvents = nMatching;
				nMatching = 0;
				for (i = 0; i < pEvents->numEvents; i++)
				{
					pEv = (VstMidiEvent *)pEvents->events[i];
					if ((pEv->type == kVstSysExType) ||   /* SysEx goes to all,                */
						(wChnMask &                       /* for normal MIDI channel has to fit*/
						(1 << (pEv->midiData[0] & 0x0F))))
						pEffEvents->events[nMatching++] = pEvents->events[i];
				}

				return pEffEvData;
			}

			/*****************************************************************************/
			/* PassThruEffect : passes the sample data through one of the effects        */
			/*****************************************************************************/

			void plugin::PassThruEffect
				(
				plugin *pEff,                   /* effect to call                    */
				float **pBuffer,                    /* input buffers                     */
				int nLength,                        /* length of these                   */
				int nChannels,                      /* # channels                        */
				BYTE *pEvData,                      /* Midi events during this period    */
				bool bReplacing
				)
			{
				BYTE *pEffEvData = CreateMIDISubset(pEvData, pEff->GetChnMask());
				float *pBuf1;
				int j;                                  /* loop counters                     */

				pEff->EnterCritical();                  /* make sure we don't get killed now */

				if (pEff->bWantMidi)                    /* if effect wants MIDI events       */
				{
					static VstEvents noEvData = {0};      /* necessary for Reaktor ...         */
					try
					{
						pEff->ProcessEvents((pEffEvData) ? (VstEvents *)pEffEvData : &noEvData);
					}
					catch(...)
					{
						TRACE0("Serious error in pEff->ProcessEvents()!\n");
					}
				}

				for (j = 0; j < nChannels; j++)
					pEff->SetInputBuffer(j, pBuffer[j]);

				/* then process the buffer itself    */
				/* clean all output buffers          */
				// NB: this has to be done even in EffProcessReplacing() since some VSTIs
				// (most notably those from Steinberg... hehe) obviously don't implement 
				// processReplacing() as a separate function but rather use process()
				float *pEmpty = GetApp()->GetEmptyBuffer();
				for (j = 0; j < pEff->pEffect->numOutputs; j++)
				{
					pBuf1 = pEff->GetOutputBuffer(j);
					if (pBuf1)
						memcpy(pBuf1, pEmpty, nLength * sizeof(float));
				}
				/* if replacing implemented          */
				if ((bReplacing) &&                     /* and wanted                        */
					(pEff->pEffect->flags & effFlagsCanReplacing))
					pEff->ProcessReplacing(nLength);   /* do processReplacing               */
				else                                    /* otherwise                         */
					pEff->Process(nLength);            /* call process                      */

				pEff->LeaveCritical();                  /* reallow killing                   */

				if (pEffEvData)                         /* if there are event data for this  */
					delete pEffEvData;                    /* delete 'em                        */
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

			void plugin::Stop()
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

			void plugin::Work(int numSamples)
			{
				PSYCLE__CPU_COST__INIT(cost);
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
				PSYCLE__CPU_COST__CALCULATE(cost, numSamples);
				_cpuCost += cost;
				_worked = true;
			}
		}
	}
}

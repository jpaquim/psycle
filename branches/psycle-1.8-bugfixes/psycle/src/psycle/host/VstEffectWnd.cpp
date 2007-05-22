///\file
///\brief implementation file for psycle::host::CVstEditorDlg.
#include <project.private.hpp>
#include "psycle.hpp"
#include "InputHandler.hpp"
#include "Configuration.hpp"
#include "PresetsDlg.hpp"

#include "VstEffectWnd.hpp"
#include "VstGui.hpp"
#include "vsthost24.hpp"


NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)

		IMPLEMENT_DYNCREATE(CVstEffectWnd, CFrameWnd)


		CVstEffectWnd::CVstEffectWnd(vst::plugin* effect):CEffectWnd(effect)
		, pView(0) , _machine(effect)
		{
		}

		BEGIN_MESSAGE_MAP(CVstEffectWnd, CFrameWnd)
			ON_WM_CREATE()
			ON_WM_CLOSE()
//			ON_WM_DESTROY()
			ON_WM_TIMER()
			ON_WM_SETFOCUS()
			ON_WM_KEYDOWN()
			ON_WM_KEYUP()
//			ON_WM_LBUTTONDOWN()
/*
			ON_COMMAND(ID_PARAMETERS_LOADPRESET, OnLoadPreset)
			ON_COMMAND(ID_PARAMETERS_SAVEPRESET, OnSavePreset)
			ON_COMMAND(ID_PARAMETERS_SAVEPRESETAS, OnSavePresetAs)
			ON_COMMAND(ID_PARAMETERS_PARAMETERLISTDIALOG, OnParametersListDlg)
			ON_COMMAND(ID_PARAMETERS_RANDOMPARAMETERS, OnParametersRandomparameters)
			ON_COMMAND(ID_PARAMETERS_SHOWPRESET, OnParametersShowpreset)
*/
		END_MESSAGE_MAP()

		int CVstEffectWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
		{
			if ( CFrameWnd::OnCreate(lpCreateStruct) == -1)
				return -1;
			pView = CreateView(this);
			SetTimer(449, 25, 0);
			return 0;
		}

		CWnd* CVstEffectWnd::CreateView(CWnd* pParentWnd)
		{
			CWnd* gui = new CWnd;
			gui->Create(NULL, NULL, WS_CHILD|WS_VISIBLE,
				CRect(0, 0, 0, 0), pParentWnd, AFX_IDW_PANE_FIRST, NULL);
			return gui;
		}

		void CVstEffectWnd::PostOpenWnd()
		{
			machine().EditOpen(pView->GetSafeHwnd());
			ResizeWindow(0);
		}
		/**********************************************************/

		bool CVstEffectWnd::GetWindowSize(ERect &rcFrame, ERect &rcClient, ERect *pRect)
		{
			if (CEffectWnd::GetWindowSize(rcFrame,rcClient,pRect))
			{

				rcFrame.bottom += ::GetSystemMetrics(SM_CYCAPTION) +
					::GetSystemMetrics(SM_CYMENUSIZE) +
//					4 * ::GetSystemMetrics(SM_CXDLGFRAME);
					4 * ::GetSystemMetrics(SM_CYBORDER) +
					2 * ::GetSystemMetrics(SM_CYFIXEDFRAME);
				rcFrame.right += //4 * ::GetSystemMetrics(SM_CXDLGFRAME);
					4 * ::GetSystemMetrics(SM_CXBORDER) +
					2 * ::GetSystemMetrics(SM_CXFIXEDFRAME);

				return true;
			}
			return false;
		}
		void CVstEffectWnd::ResizeWindow(int w,int h)
		{
			CRect rcW;
			GetWindowRect(&rcW);

			ERect rc;
			rc.left = (short)rcW.left;	rc.top = (short)rcW.top;
			rc.right = rc.left + w;	rc.bottom = rc.top + h;
			ResizeWindow(&rc);
		}
		void CVstEffectWnd::ResizeWindow(ERect *pRect)
		{
			ERect rcEffFrame={0,0,0,0},rcEffClient={0,0,0,0};
			if (!GetWindowSize(rcEffFrame, rcEffClient, pRect))
			{
				rcEffFrame.right = 400; rcEffFrame.bottom = 300;
				rcEffClient.right = 400; rcEffClient.bottom = 300;
			}
			MoveWindow(rcEffFrame.left,rcEffFrame.top,rcEffFrame.right-rcEffFrame.left,rcEffFrame.bottom-rcEffFrame.top);
			if (pView)
				pView->MoveWindow(0,0, rcEffClient.right,rcEffClient.bottom,true);
		}


		/*****************************************************************************/
		/* OnOpenFileSelector : called when effect needs a file selector             */
		/*																			 */
		/*		This function is based on the VSTGUI3.0 source					     */
		/*****************************************************************************/
		bool CVstEffectWnd::OpenFileSelector (VstFileSelect *ptr)
		{
			if (!ptr)
				return false;

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
					filefilter = ptr->fileTypes[i].name; filefilter.push_back('\0');
					filefilter += "*."; filefilter += ptr->fileTypes[i].dosType; filefilter.push_back('\0');
				}
				filefilter += "All (*.*)"; filefilter.push_back('\0');
				filefilter += "*.*"; filefilter.push_back('\0');

				if (ptr->command == kVstMultipleFilesLoad)
					filePath = new char [_MAX_PATH * 100];
				else
					filePath = new char[_MAX_PATH];

				filePath[0] = 0;
				// Initialize OPENFILENAME
				ofn.hwndOwner = GetSafeHwnd();

				ofn.lpstrFile = filePath;
				ofn.nMaxFile    = sizeof (filePath) - 1;
				ofn.lpstrFilter =filefilter.c_str();
				ofn.nFilterIndex = 1;
				ofn.lpstrTitle = ptr->title;
				ofn.lpstrFileTitle = fileName;
				ofn.nMaxFileTitle = sizeof(fileName) - 1;
				if ( ptr->initialPath != 0)
					ofn.lpstrInitialDir = ptr->initialPath;
				else
					ofn.lpstrInitialDir =  (char*)machine().OnGetDirectory();
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
					bi.hwndOwner = GetSafeHwnd();

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
		/*****************************************************************************/
		/* OnCloseFileSelector : called when effect needs a file selector            */
		/*																			 */
		/*		This function is based on the VSTGUI3.0 source					     */
		/*****************************************************************************/
		bool CVstEffectWnd::CloseFileSelector (VstFileSelect *ptr)
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

		/*****************************************************************************/
		/* OnOpenWindow : called to open yet another window                          */
		/*****************************************************************************/

		void * CVstEffectWnd::OpenSecondaryWnd(VstWindow& window)
		{
			CFrameWnd *pWnd = new CFrameWnd;
			if (pWnd)
			{
				HWND hWnd = pWnd->GetSafeHwnd();
				// ignored at the moment: style, position
				ERect rc = {0};
				rc.right = window.width;
				rc.bottom = window.height;
				pWnd->SetWindowText(window.title);
				pWnd->ShowWindow(SW_SHOWNORMAL);
				pWnd->MoveWindow(0,0,window.width,window.height,true);
				window.winHandle = hWnd;
				return hWnd;
				///\todo: Maintain a list of opened windows, in case we need to close them
				// in the OnClose event.
			}
			return 0;
		}

		/*****************************************************************************/
		/* OnCloseWindow : called to close a window opened in OnOpenWindow           */
		/*****************************************************************************/

		bool CVstEffectWnd::CloseSecondaryWnd(VstWindow& window)
		{
			if (!::IsWindow((HWND)window.winHandle))
				return false;

			::SendMessage((HWND)window.winHandle, WM_CLOSE, 0, 0);
			window.winHandle = 0;
			return true;
		}

		/***********************************************************************/

		void CVstEffectWnd::OnClose()
		{
//			machine().EnterCritical();             /* make sure we're not processing    */
			machine().EditClose();              /* tell effect edit window's closed  */
//			machine().LeaveCritical();             /* re-enable processing              */
			CFrameWnd::OnClose();
		}

		void CVstEffectWnd::OnTimer(UINT nIDEvent)
		{
			if ( nIDEvent == 449 )
			{
				machine().EditIdle();
			}
			CFrameWnd::OnTimer(nIDEvent);
		}

		void CVstEffectWnd::OnSetFocus(CWnd* pOldWnd) 
		{
			CFrameWnd::OnSetFocus(pOldWnd);
		}
		void CVstEffectWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{
			const BOOL bRepeat = nFlags&0x4000;
			CmdDef cmd(Global::pInputHandler->KeyToCmd(nChar,nFlags));
			if(!bRepeat && cmd.IsValid())
			{
				switch(cmd.GetType())
				{
				case CT_Note:
					{
						const int outnote = cmd.GetNote();
						if ( machine()._mode == MACHMODE_GENERATOR || Global::pConfig->_notesToEffects)
						{
							Global::pInputHandler->PlayNote(outnote,127,true,&machine());
						}
						else Global::pInputHandler->PlayNote(outnote,127,true);
					}
					break;
				case CT_Immediate:
					Global::pInputHandler->PerformCmd(cmd,bRepeat);
					break;
				}
			}
			CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		}

		void CVstEffectWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{
			CmdDef cmd(Global::pInputHandler->KeyToCmd(nChar,nFlags));
			const int outnote = cmd.GetNote();
			if(outnote != -1) {
				if(machine()._mode == MACHMODE_GENERATOR || Global::pConfig->_notesToEffects)
					Global::pInputHandler->StopNote(outnote, true, &machine());
				else
					Global::pInputHandler->StopNote(outnote, true);
			}
			CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
		}

/*		void CVstEffectWnd::OnLButtonDown(UINT nFlags, CPoint point) 
		{
			this->SetFocus();
			CFrameWnd::OnLButtonDown(nFlags, point);
		}
*/

/*

		void CVstEffectWnd::OnLoadPreset()
		{
			char tmp[1024];
			machine().OnGetChunkFile(tmp);
			CFileDialog dlg(TRUE,
				"fxb",
				tmp,
				OFN_ENABLESIZING | OFN_NOCHANGEDIR,
				"Effect Bank Files (.fxb)|*.fxb|Effect Program Files (.fxp)|*.fxp|All Files|*.*||");

			if (dlg.DoModal() != IDOK)
				return;

			if ( dlg.GetFileExt() == ".fxb" )
			{
				CFxBank b(dlg.GetPathName());
				if ( b.Initialized() ) machine().LoadBank(b);
				else
					MessageBox("Error Loading file", NULL, MB_ICONERROR);
			}
			else if ( dlg.GetFileExt() == ".fxp" )
			{
				CFxProgram p(dlg.GetPathName());
				if ( p.Initialized() ) machine().LoadProgram(p);
				else
					MessageBox("Error Loading file", NULL, MB_ICONERROR);

			}
		}

		void CVstEffectWnd::OnSavePreset() 
		{
			char tmp[1024];
			machine().OnGetChunkFile(tmp);
			std::string sFile = tmp;
			if (sFile.empty())
				OnSavePresetAs();
			else
			{
				std::string ext = sFile.substr(sFile.size()-4,4);
				if ( ext == ".fxb")
					SaveBank(sFile);
				else if ( ext == ".fxp")
					SaveProgram(sFile);
			}
		}

		void CVstEffectWnd::OnSavePresetAs() 
		{
			char tmp[1024];
			machine().OnGetChunkFile(tmp);
			CFileDialog dlg(FALSE,
				"fxb",
				tmp,
				OFN_CREATEPROMPT | OFN_ENABLESIZING |
				OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN |
				OFN_OVERWRITEPROMPT,
				"Effect Bank Files (.fxb)|*.fxb|Effect Program Files (.fxp)|*.fxp|All Files|*.*||");
			if (dlg.DoModal() == IDOK)
			{
				if ( dlg.GetFileExt() == ".fxb")
					SaveBank((char *)(LPCSTR)dlg.GetPathName());
				else if ( dlg.GetFileExt() == ".fxp")
					SaveProgram((char *)(LPCSTR)dlg.GetPathName());
			}
		}






		void CVstEffectWnd::OnParametersListDlg()
		{
			if ( !pParamGui ) pParamGui= new CDefaultVstGui(wndView);
			
			pParamGui->_pMachine = _pMachine;
			pParamGui->MachineIndex = MachineIndex;
			pParamGui->childView=wndView;
//			pParamGui->Create(this);
			pParamGui->DoModal();
//			pParamGui->Init();
			delete pParamGui;
			pParamGui=0;
		}
		void CVstEffectWnd::OnParametersRandomparameters() 
		{
			// Randomize controls
			int numParameters;
			try
			{
				numParameters = _pMachine->GetNumParams();
			}
			catch(const std::exception &)
			{
				numParameters = 0;
			}
			catch(...) // reinterpret_cast sucks
			{
				numParameters = 0;
			}
			for(int c(0); c < numParameters ; ++c)
			{
				try
				{
					_pMachine->SetParameter(c, rand());
				}
				catch(const std::exception &)
				{
					// o_O`
				}
				catch(...) // reinterpret_cast sucks
				{
					// o_O`
				}
			}
			Invalidate(false);
		}

		void CVstEffectWnd::OnParametersShowpreset() 
		{
			CPresetsDlg dlg;
			dlg._pMachine=_pMachine;
			dlg.DoModal();
//			pParamGui->UpdateOne();
//			if (!editorgui) pGui->Invalidate(false);
//			pGui->SetFocus();
		}
*/

	NAMESPACE__END
NAMESPACE__END

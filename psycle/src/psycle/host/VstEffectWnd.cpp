//\file
///\brief implementation file for psycle::host::CVstEditorDlg.
#include <psycle/project.private.hpp>
#include "VstEffectWnd.hpp"
#include "psycle.hpp"
#include "vsthost24.hpp"

#include "VstParamList.hpp"

#include "InputHandler.hpp"
#include "Configuration.hpp"

#include "PresetsDlg.hpp"

///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
#include "MainFrm.hpp"
#include "ChildView.hpp"

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		extern CPsycleApp theApp;

		//////////////////////////////////////////////////////////////////////////
	
		CVstGui::CVstGui(vst::plugin* effect)
		:pEffect(effect)
		{}

		void CVstGui::Open()
		{ 
			pEffect->EditOpen(WindowPtr());
		}
		void CVstGui::WindowIdle() { pEffect->EditIdle(); }
		bool CVstGui::GetViewSize(CRect& rect)
		{
			ERect* pRect(0);
			pEffect->EditGetRect(&pRect);
			if (!pRect)
				return false;

			rect.left = rect.top = 0;
			rect.right = pRect->right - pRect->left;
			rect.bottom = pRect->bottom - pRect->top;
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		IMPLEMENT_DYNAMIC(CVstEffectWnd, CFrameWnd)

		BEGIN_MESSAGE_MAP(CVstEffectWnd, CFrameWnd)
			ON_WM_CREATE()
			ON_WM_CLOSE()
//			ON_WM_DESTROY()
			ON_WM_TIMER()
			ON_WM_SETFOCUS()
			ON_WM_KEYDOWN()
			ON_WM_KEYUP()
			ON_WM_SIZING()
			ON_WM_INITMENUPOPUP()
//			ON_WM_LBUTTONDOWN()
			ON_COMMAND(ID_OPERATIONS_ENABLED, OnOperationsEnabled)
			ON_UPDATE_COMMAND_UI(ID_OPERATIONS_ENABLED, OnUpdateOperationsEnabled)
			ON_COMMAND(ID_PROGRAMS_OPENPRESET, OnProgramsOpenpreset)
			ON_COMMAND(ID_PROGRAMS_SAVEPRESET, OnProgramsSavepreset)
			ON_COMMAND(ID_PROGRAMS_RANDOMIZEPROGRAM, OnProgramsRandomizeprogram)
			ON_COMMAND(ID_VIEWS_PARAMETERLIST, OnViewsParameterlist)
			ON_UPDATE_COMMAND_UI(ID_VIEWS_PARAMETERLIST, OnUpdateViewsParameterlist)
			ON_COMMAND(ID_VIEWS_BANKMANAGER, OnViewsBankmanager)
			ON_UPDATE_COMMAND_UI(ID_VIEWS_BANKMANAGER, OnUpdateViewsBankmanager)
			ON_COMMAND(ID_VIEWS_MIDICHANNELS, OnViewsMidichannels)
			ON_UPDATE_COMMAND_UI(ID_VIEWS_MIDICHANNELS, OnUpdateViewsMidichannels)
			ON_COMMAND(ID_ABOUT_ABOUTVST, OnAboutAboutvst)
//			ON_LBN_SELCHANGE(ID_COMBO_BANK, OnSelchangeBank)
//			ON_CBN_CLOSEUP(ID_COMBO_BANK, OnCloseupProgram)
//			ON_COMMAND_RANGE(ID_SELECTBANK_0, ID_SELECTBANK_0+99, OnSetBank)
			ON_LBN_SELCHANGE(ID_COMBO_PRG, OnSelchangeProgram)
			ON_CBN_CLOSEUP(ID_COMBO_PRG, OnCloseupProgram)
			ON_COMMAND_RANGE(ID_SELECTPROGRAM_0, ID_SELECTPROGRAM_0+999, OnSetProgram)
			ON_COMMAND(ID_PROGRAMLESS, OnProgramLess)
			ON_UPDATE_COMMAND_UI(ID_PROGRAMLESS, OnUpdateProgramLess)
			ON_COMMAND(ID_PROGRAMMORE, OnProgramMore)
			ON_UPDATE_COMMAND_UI(ID_PROGRAMMORE, OnUpdateProgramMore)
			ON_COMMAND(ID_VIEWS_SHOWTOOLBAR, OnViewsShowtoolbar)
			ON_UPDATE_COMMAND_UI(ID_VIEWS_SHOWTOOLBAR, OnUpdateViewsShowtoolbar)
		END_MESSAGE_MAP()

		CVstEffectWnd::CVstEffectWnd(vst::plugin* effect):CEffectWnd(effect)
		, pView(0) , _machine(effect) , pParamGui(0)
		{
		}

		int CVstEffectWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
		{
			if ( CFrameWnd::OnCreate(lpCreateStruct) == -1)
				return -1;

			if (!toolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
				|  CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
				!toolBar.LoadToolBar(IDR_VSTFRAME))
			{
				TRACE0("Failed to create toolbar\n");
				return -1;      // fail to create
			}
/*
			nice, but will make the toolbar too big.
			toolBar.SetButtonText(0,"blabla");
			CRect temp;
			toolBar.GetItemRect(0,&temp);
			toolBar.GetToolBarCtrl().SetButtonSize(CSize(temp.Width(),
				temp.Height()));
*/

			CRect rect;
/*			int nIndex = toolBar.GetToolBarCtrl().CommandToIndex(ID_COMBO_BANK);
			toolBar.SetButtonInfo(nIndex, ID_COMBO_BANK, TBBS_SEPARATOR, 150);
			toolBar.GetToolBarCtrl().GetItemRect(nIndex, &rect);
			rect.top = 1;
			rect.bottom = rect.top + 400; //drop height

			if(!comboBank.Create( WS_CHILD | CBS_DROPDOWNLIST | WS_VISIBLE | CBS_AUTOHSCROLL 
				| WS_VSCROLL, rect, &toolBar, ID_COMBO_BANK))
			{
				TRACE0("Failed to create combobox\n");
				return -1;      // fail to create
			}
*/
			
			int nIndex = toolBar.GetToolBarCtrl().CommandToIndex(ID_COMBO_PRG);
			toolBar.SetButtonInfo(nIndex, ID_COMBO_PRG, TBBS_SEPARATOR, 160);
			toolBar.GetToolBarCtrl().GetItemRect(nIndex, &rect);
			rect.top = 1;
			rect.bottom = rect.top + 400; //drop height
			if(!comboProgram.Create( WS_CHILD |  CBS_DROPDOWNLIST | WS_VISIBLE | CBS_AUTOHSCROLL 
				 | WS_VSCROLL, rect, &toolBar, ID_COMBO_PRG))
			{
				TRACE0("Failed to create combobox\n");
				return -1;      // fail to create
			}
			HGDIOBJ hFont = GetStockObject( DEFAULT_GUI_FONT );
			CFont font;
			font.Attach( hFont );
//			comboBank.SetFont(&font);
			comboProgram.SetFont(&font);

//			comboBank.AddString("Internal Bank");
//			comboBank.AddString("-----------------");
//			comboBank.SetCurSel(0);
			FillProgramCombobox();


			pView = CreateView();
			toolBar.SetWindowText("Vst Toolbar");
			toolBar.EnableDocking(CBRS_ALIGN_ANY);
			EnableDocking(CBRS_ALIGN_TOP);
			DockControlBar(&toolBar);
			if (!Global::pConfig->_toolbarOnVsts) ShowControlBar(&toolBar,FALSE,FALSE);
			machine().SetEditWnd(this);
			*_pActive=true;
			SetTimer(449, 25, 0);
			return 0;
		}
		void CVstEffectWnd::OnClose()
		{
//			machine().EnterCritical();             /* make sure we're not processing    */
			machine().EditClose();              /* tell effect edit window's closed  */
//			machine().LeaveCritical();             /* re-enable processing              */
			pView->DestroyWindow();
			if (pParamGui) pParamGui->DestroyWindow();
			std::list<HWND>::iterator it = secwinlist.begin();
			while ( it != secwinlist.end() )
			{
				::SendMessage(*it, WM_CLOSE, 0, 0);
				++it;
			}
			*_pActive=false;
			CFrameWnd::OnClose();
		}

		void CVstEffectWnd::OnTimer(UINT nIDEvent)
		{
			if ( nIDEvent == 449 )
			{
				pView->WindowIdle();
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
			if(cmd.IsValid())
			{
				switch(cmd.GetType())
				{
				case CT_Note:
					if (!bRepeat)
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
				case CT_Editor:
					Global::pInputHandler->PerformCmd(cmd,bRepeat);
					break;
				}
			}
			this->SetFocus();

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

		/*****************************************************************************/
		/* OnInitMenuPopup : called when a popup menu is initialized                 */
		/*****************************************************************************/

		void CVstEffectWnd::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
		{
			/* if Effect Edit menu popping up    */
			if ((pPopupMenu->GetMenuItemCount() > 0) &&
				(pPopupMenu->GetMenuItemID(0) == ID_PROGRAMS_OPENPRESET))
			{
				FillPopup(pPopupMenu);
			}

			CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
		}
		void CVstEffectWnd::FillPopup(CMenu* pPopupMenu)
		{
			if (machine().numPrograms() > 1)
			{
				CMenu* popPrg=0;
				// Find the popup menu for programs.
				for (int i = pPopupMenu->GetMenuItemCount() - 1; i >= 2; i--)
				{
					if ((popPrg = pPopupMenu->GetSubMenu(i)))
					{
						CMenu* secMenu=0;
						if (popPrg->GetMenuItemID(0) == ID_SELECTPROGRAM_0)
						{
							popPrg->DeleteMenu(0, MF_BYPOSITION);
							break;
						}
						else if ((secMenu=popPrg->GetSubMenu(0)))
						{
							if (secMenu->GetMenuItemID(0) == ID_SELECTPROGRAM_0)
							{
								while (popPrg->GetSubMenu(0))
								{
									popPrg->DeleteMenu(0,MF_BYPOSITION);
								}
								break;
							}
						}
					}
				}
				if (!popPrg)
					return;

				for (int i = 0; i < machine().numPrograms(); i += 16)
				{
					CMenu popup;
					popup.CreatePopupMenu();
					for (int j = i; (j < i + 16) && (j < machine().numPrograms()); j++)
					{
						char szProg[kVstMaxProgNameLen+7] = "";
						char szPgName[kVstMaxProgNameLen+1] = "";
						machine().GetProgramNameIndexed(-1, j, szPgName);
						std::sprintf(szProg,"%d. %s",j,szPgName);
						popup.AppendMenu(MF_STRING, ID_SELECTPROGRAM_0 + j, szProg);
					}
					char szSub[256] = "";;
					std::sprintf(szSub,"Programs %d-%d",i,i+15);
					popPrg->AppendMenu(MF_POPUP | MF_STRING,
						(UINT)popup.Detach(),
						szSub);
				}
				popPrg->CheckMenuItem(ID_SELECTPROGRAM_0 + machine().GetProgram(),
					MF_CHECKED | MF_BYCOMMAND);
			}
		}
		void CVstEffectWnd::FillProgramCombobox()
		{
			comboProgram.ResetContent();

			int nump;
			nump = machine().numPrograms();
			for(int i(0) ; i < nump; ++i)
			{
				char s1[kVstMaxProgNameLen+7];
				char s2[kVstMaxProgNameLen+1];
				machine().GetProgramNameIndexed(-1, i, s2);
				std::sprintf(s1,"%d: %s",i,s2);
				comboProgram.AddString(s1);
			}
			comboProgram.SetCurSel(machine().GetProgram());
		}
		void CVstEffectWnd::OnSelchangeProgram() 
		{
			int const se=comboProgram.GetCurSel();
			machine().SetProgram(se);
			SetFocus();
		}
		void CVstEffectWnd::OnCloseupProgram()
		{
			SetFocus();
		}
		void CVstEffectWnd::RefreshUI()
		{
			///\todo: anything more?
			FillProgramCombobox();
		}
		CBaseGui* CVstEffectWnd::CreateView()
		{
			if ( pEffect->HasEditor())
			{
				CVstGui* gui = new CVstGui(&machine());
				CRect rcClient(0,0,0,0);
				// C_Tuner.dll crashes if asked before opening
				//gui->GetViewSize(rcClient);
				gui->Create(NULL, "vstgui", WS_CHILD | WS_VISIBLE,
					rcClient, this, AFX_IDW_PANE_FIRST, NULL);
				return gui;
			}
			else
			{
				CNativeGui* gui = new CNativeGui(&machine());
				CRect rcClient;
				gui->GetViewSize(rcClient);
				gui->Create(NULL, "nativegui", WS_CHILD | WS_VISIBLE,
					rcClient, this, AFX_IDW_PANE_FIRST, NULL);
				return gui;
			}
		}

		void CVstEffectWnd::PostOpenWnd()
		{
			pView->Open();
			ResizeWindow(0);
		}
		/**********************************************************/

		void CVstEffectWnd::GetWindowSize(CRect &rcFrame, CRect &rcClient, ERect *pRect)
		{
			if ( !pRect )
			{
				if (!pView->GetViewSize(rcClient))
				{
					rcClient.top = 0; rcClient.left = 0;
					rcClient.right = 400; rcClient.bottom = 300;
				}
				rcFrame = rcClient;
			}
			else 
			{
				rcFrame.left = pRect->left;
				rcFrame.top = pRect->top;
				rcFrame.right = pRect->right;
				rcFrame.bottom = pRect->bottom;
				rcClient.top = 0; rcClient.left = 0;
				rcClient.right = pRect->right - pRect->left; rcClient.bottom = pRect->bottom - pRect->top;
			}
			CRect tbRect;
			toolBar.GetWindowRect(&tbRect);
			rcClient.top+=tbRect.bottom - tbRect.top;
			if ( Global::pConfig->_toolbarOnVsts)
			{
				rcFrame.bottom += ::GetSystemMetrics(SM_CYCAPTION) +
					::GetSystemMetrics(SM_CYMENUSIZE) +
					(tbRect.bottom-tbRect.top) +
					6 * ::GetSystemMetrics(SM_CYBORDER) +
					2 * ::GetSystemMetrics(SM_CYFIXEDFRAME);
			}
			else
			{
				rcFrame.bottom += ::GetSystemMetrics(SM_CYCAPTION) +
					::GetSystemMetrics(SM_CYMENUSIZE) +
					6 * ::GetSystemMetrics(SM_CYBORDER) +
					2 * ::GetSystemMetrics(SM_CYFIXEDFRAME);
				rcClient.top=0;
			}
			rcFrame.right += 6 * ::GetSystemMetrics(SM_CXBORDER) +
				2 * ::GetSystemMetrics(SM_CXFIXEDFRAME);
		}
		void CVstEffectWnd::ResizeWindow(int w,int h)
		{
			CRect rcW;
			GetWindowRect(&rcW);

			ERect rc;
			rc.left = (short)rcW.left;	rc.top = (short)rcW.top;
			rc.right = rc.left + w;	rc.bottom = rc.top + h;
			ResizeWindow(&rc);
			pView->WindowIdle();
		}
		void CVstEffectWnd::ResizeWindow(ERect *pRect)
		{
			CRect rcEffFrame,rcEffClient;
			GetWindowSize(rcEffFrame, rcEffClient, pRect);
			SetWindowPos(NULL,0,0,rcEffFrame.right-rcEffFrame.left,rcEffFrame.bottom-rcEffFrame.top,SWP_NOZORDER | SWP_NOMOVE);
			pView->SetWindowPos(NULL,0,rcEffClient.top,rcEffClient.right+1,rcEffClient.bottom+1,SWP_SHOWWINDOW);
			pView->WindowIdle();
		}
		void CVstEffectWnd::OnSizing(UINT fwSide, LPRECT pRect)
		{
			pView->WindowIdle();
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
				secwinlist.push_back(hWnd);
				return hWnd;
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

			secwinlist.remove((HWND)window.winHandle);
			::SendMessage((HWND)window.winHandle, WM_CLOSE, 0, 0);
			window.winHandle = 0;
			return true;
		}

		bool CVstEffectWnd::SetParameterAutomated(long index, float value)
		{
			///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
			if(index>= 0 || index < machine().GetNumParams())
			{
				if(Global::configuration()._RecordTweaks)
				{
					if(Global::configuration()._RecordMouseTweaksSmooth)
						((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweakSlide(machine()._macIndex, index, helpers::math::rounded(value * vst::quantization));
					else
						((CMainFrame *) theApp.m_pMainWnd)->m_wndView.MousePatternTweak(machine()._macIndex, index, helpers::math::rounded(value * vst::quantization));
				}
				if(pParamGui)
					pParamGui->UpdateNew(index, value);
				return true;
			}
			return false;
		}


		/***********************************************************************/

		void CVstEffectWnd::OnOperationsEnabled()
		{
			if (machine().IsSynth())
			{
				machine()._mute = !machine()._mute;
			}
			else
			{
				machine().Bypass(!machine().Bypass());
				if (machine()._mute) machine()._mute = false;
			}
		}

		void CVstEffectWnd::OnUpdateOperationsEnabled(CCmdUI *pCmdUI)
		{
			pCmdUI->SetText("Activated");
			if (machine().IsSynth())
			{
				if (machine()._mute)
				{
					pCmdUI->SetCheck(false);
				}
				else 
				{
					pCmdUI->SetCheck(true);
				}
			}
			else
			{
				if (machine()._mute || machine().Bypass())
				{
					pCmdUI->SetCheck(false);
				}
				else
				{
					pCmdUI->SetCheck(true);
				}
			}
		}

		void CVstEffectWnd::OnProgramsOpenpreset()
		{
			char tmp[2048];
			if (!machine().OnGetChunkFile(tmp))
			{
				std::strncpy(tmp,reinterpret_cast<char*>(machine().OnGetDirectory()),1024);
				std::strcat(tmp,"\\fxb");
			}
			CFileDialog dlg(TRUE,
				"fxb",
				tmp,
				OFN_ENABLESIZING | OFN_NOCHANGEDIR,
				"Effect Bank Files (.fxb)|*.fxb|Effect Program Files (.fxp)|*.fxp|All Files|*.*||");

			if (dlg.DoModal() != IDOK)
				return;

			if ( dlg.GetFileExt() == "fxb" )
			{
				CFxBank b(dlg.GetPathName());
				if ( b.Initialized() ) machine().LoadBank(b);
				else
					MessageBox("Error Loading file", NULL, MB_ICONERROR);
			}
			else if ( dlg.GetFileExt() == "fxp" )
			{
				CFxProgram p(dlg.GetPathName());
				if ( p.Initialized() ) machine().LoadProgram(p);
				else
					MessageBox("Error Loading file", NULL, MB_ICONERROR);

			}
			FillProgramCombobox();
		}

		void CVstEffectWnd::OnProgramsSavepreset()
		{
			char tmp[2048];
			if (!machine().OnGetChunkFile(tmp))
			{
				std::strncpy(tmp,reinterpret_cast<char*>(machine().OnGetDirectory()),1024);
				std::strcat(tmp,"\\filename");
			}
			CFileDialog dlg(FALSE,
				"fxb",
				tmp,
				OFN_CREATEPROMPT | OFN_ENABLESIZING |
				OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN |
				OFN_OVERWRITEPROMPT,
				"Effect Bank Files (.fxb)|*.fxb|Effect Program Files (.fxp)|*.fxp|All Files|*.*||");
			if (dlg.DoModal() == IDOK)
			{
				if ( dlg.GetFileExt() == "fxb")
					SaveBank((char *)(LPCSTR)dlg.GetPathName());
				else if ( dlg.GetFileExt() == "fxp")
					SaveProgram((char *)(LPCSTR)dlg.GetPathName());
			}
		}
		/*
		void CVstEffectWnd::OnSavePreset() 
		{
		char tmp[2048];
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
		*/
		void CVstEffectWnd::OnProgramsRandomizeprogram()
		{
			int numParameters = machine().numParams();
			for(int c(0); c < numParameters ; ++c)
			{
				machine().SetParameter(c, rand());
			}
			UpdateWindow();
		}
		void CVstEffectWnd::OnSetProgram(UINT nID)
		{
			machine().SetProgram(nID - ID_SELECTPROGRAM_0);
			comboProgram.SetCurSel(nID - ID_SELECTPROGRAM_0);
		}
		void CVstEffectWnd::OnProgramLess()
		{
			int numProgram = machine().GetProgram()-1;
			machine().SetProgram(numProgram);
			comboProgram.SetCurSel(numProgram);
			UpdateWindow();
		}
		void CVstEffectWnd::OnUpdateProgramLess(CCmdUI *pCmdUI)
		{
			if ( machine().GetProgram() == 0)
			{
				pCmdUI->Enable(false);
			}
			else pCmdUI->Enable(true);
		}
		void CVstEffectWnd::OnProgramMore()
		{
			int numProgram = machine().GetProgram()+1;
			machine().SetProgram(numProgram);
			comboProgram.SetCurSel(numProgram);
			UpdateWindow();
		}
		void CVstEffectWnd::OnUpdateProgramMore(CCmdUI *pCmdUI)
		{
			if ( machine().GetProgram()+1 == machine().numPrograms())
			{
				pCmdUI->Enable(false);
			}
			else pCmdUI->Enable(true);
		}

		void CVstEffectWnd::OnViewsParameterlist()
		{
			CRect rc;
			GetWindowRect(&rc);
			if (!pParamGui)
			{
				pParamGui= new CVstParamList(&machine());
				pParamGui->Create();
			}
			if (pParamGui->IsWindowVisible())
			{
				pParamGui->ShowWindow(SW_HIDE); 
			}
			else
			{
				pParamGui->SetWindowPos(0,rc.right+1,rc.top,0,0,SWP_NOSIZE | SWP_NOZORDER);
				pParamGui->ShowWindow(SW_SHOWNORMAL); 
			}
		}

		void CVstEffectWnd::OnUpdateViewsParameterlist(CCmdUI *pCmdUI)
		{
			if ( pParamGui )
			{
				if (pParamGui->IsWindowVisible())
					pCmdUI->SetCheck(true);
				else
					pCmdUI->SetCheck(false);
			}
			else
				pCmdUI->SetCheck(false);
		}

		void CVstEffectWnd::OnViewsBankmanager()
		{
			CPresetsDlg dlg;
			dlg._pMachine=_machine;
			dlg.DoModal();
		}

		void CVstEffectWnd::OnUpdateViewsBankmanager(CCmdUI *pCmdUI)
		{
		}

		void CVstEffectWnd::OnViewsMidichannels()
		{
			// TODO: Agregue aquí su código de controlador de comandos
		}

		void CVstEffectWnd::OnUpdateViewsMidichannels(CCmdUI *pCmdUI)
		{
			pCmdUI->Enable(false);
		}

		void CVstEffectWnd::OnViewsShowtoolbar()
		{
			Global::pConfig->_toolbarOnVsts = !Global::pConfig->_toolbarOnVsts;

			if (Global::pConfig->_toolbarOnVsts) ShowControlBar(&toolBar,TRUE,FALSE);
			else ShowControlBar(&toolBar,FALSE,FALSE);
			ResizeWindow(0);
		}

		void CVstEffectWnd::OnUpdateViewsShowtoolbar(CCmdUI *pCmdUI)
		{
			pCmdUI->SetCheck(Global::pConfig->_toolbarOnVsts);
		}

		void CVstEffectWnd::OnAboutAboutvst()
		{
			///\todo: made an informative dialog like in seib's vsthost.
			std::string message;
			(message = "Vst Plugin by " )+ machine().GetVendorName();
			MessageBox(message.c_str(),"About");
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

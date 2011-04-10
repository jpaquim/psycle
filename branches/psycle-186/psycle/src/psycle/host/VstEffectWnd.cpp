//\file
///\brief implementation file for psycle::host::CVstEditorDlg.

#include "VstEffectWnd.hpp"

#include "VstHost24.hpp"

#include "Configuration.hpp"
#include "VstParamList.hpp"
///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
#include "ChildView.hpp"


namespace psycle { namespace host {

		//////////////////////////////////////////////////////////////////////////
	
		CVstGui::CVstGui(CFrameMachine* frame,vst::plugin* effect)
		:CBaseParamView(frame)
		,pEffect(effect)
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

		IMPLEMENT_DYNAMIC(CVstEffectWnd, CFrameMachine)

		BEGIN_MESSAGE_MAP(CVstEffectWnd, CFrameMachine)
			ON_WM_CREATE()
			ON_WM_CLOSE()
			ON_WM_SIZING()
			ON_WM_INITMENUPOPUP()
			ON_COMMAND(ID_PROGRAMS_OPENPRESET, OnProgramsOpenpreset)
			ON_COMMAND(ID_PROGRAMS_SAVEPRESET, OnProgramsSavepreset)
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
			ON_COMMAND(ID_VIEWS_PARAMETERLIST, OnViewsParameterlist)
			ON_UPDATE_COMMAND_UI(ID_VIEWS_PARAMETERLIST, OnUpdateViewsParameterlist)
			ON_COMMAND(ID_VIEWS_MIDICHANNELS, OnViewsMidichannels)
			ON_UPDATE_COMMAND_UI(ID_VIEWS_MIDICHANNELS, OnUpdateViewsMidichannels)
			ON_COMMAND(ID_VIEWS_SHOWTOOLBAR, OnViewsShowtoolbar)
			ON_UPDATE_COMMAND_UI(ID_VIEWS_SHOWTOOLBAR, OnUpdateViewsShowtoolbar)
		END_MESSAGE_MAP()

		CVstEffectWnd::CVstEffectWnd(vst::plugin* effect):CEffectWnd(effect)
		, CFrameMachine(effect), pParamGui(0)
		{
		}
		int CVstEffectWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
		{
			if (CFrameMachine::OnCreate(lpCreateStruct) == -1)
			{
				return -1;
			}

			if (!toolBar.CreateEx(this, TBSTYLE_FLAT, TBSTYLE_TRANSPARENT) ||
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

			toolBar.SetBarStyle(toolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_GRIPPER);
			toolBar.SetWindowText("Vst Toolbar");
			toolBar.EnableDocking(CBRS_ALIGN_TOP);

			EnableDocking(CBRS_ALIGN_ANY);
			DockControlBar(&toolBar);
			LoadBarState(_T("VstParamToolbar"));

			vstmachine().SetEditWnd(this);
			return 0;
		}

		void CVstEffectWnd::OnClose()
		{
//			vstmachine().EnterCritical();             /* make sure we're not processing    */
			vstmachine().EditClose();              /* tell effect edit window's closed  */
//			vstmachine().LeaveCritical();             /* re-enable processing              */
			SaveBarState(_T("VstParamToolbar"));
			if (pParamGui) pParamGui->DestroyWindow();
			std::list<HWND>::iterator it = secwinlist.begin();
			while ( it != secwinlist.end() )
			{
				::SendMessage(*it, WM_CLOSE, 0, 0);
				++it;
			}
			CFrameWnd::OnClose();
		}


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
			if (vstmachine().numPrograms() > 1)
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

				for (int i = 0; i < vstmachine().numPrograms() && i < 980 ; i += 16)
				{
					CMenu popup;
					popup.CreatePopupMenu();
					for (int j = i; (j < i + 16) && (j < vstmachine().numPrograms()); j++)
					{
						char szProg[kVstMaxProgNameLen+7] = "";
						char szPgName[kVstMaxProgNameLen+1] = "";
						vstmachine().GetProgramNameIndexed(-1, j, szPgName);
						std::sprintf(szProg,"%d. %s",j,szPgName);
						popup.AppendMenu(MF_STRING, ID_SELECTPROGRAM_0 + j, szProg);
					}
					char szSub[256] = "";;
					std::sprintf(szSub,"Programs %d-%d",i,i+15);
					popPrg->AppendMenu(MF_POPUP | MF_STRING,
						(UINT)popup.Detach(),
						szSub);
				}
				popPrg->CheckMenuItem(ID_SELECTPROGRAM_0 + vstmachine().GetProgram(),
					MF_CHECKED | MF_BYCOMMAND);
			}
		}
		void CVstEffectWnd::FillProgramCombobox()
		{
			comboProgram.ResetContent();

			int nump;
			nump = vstmachine().numPrograms();
			for(int i(0) ; i < nump; ++i)
			{
				char s1[kVstMaxProgNameLen+7];
				char s2[kVstMaxProgNameLen+1];
				vstmachine().GetProgramNameIndexed(-1, i, s2);
				std::sprintf(s1,"%d: %s",i,s2);
				comboProgram.AddString(s1);
			}
			int i = vstmachine().GetProgram();
			if ( i > nump || i < 0) {  i = 0; }
			comboProgram.SetCurSel(i);
		}
		void CVstEffectWnd::OnSelchangeProgram() 
		{
			int const se=comboProgram.GetCurSel();
			vstmachine().SetProgram(se);
			if (pParamGui && pParamGui->IsWindowVisible()){
				pParamGui->SelectProgram(se);
			}
			SetFocus();
		}
		void CVstEffectWnd::OnCloseupProgram()
		{
			SetFocus();
		}
		void CVstEffectWnd::RefreshUI()
		{
			FillProgramCombobox();
			if (pParamGui && pParamGui->IsWindowVisible()){
				int const se=comboProgram.GetCurSel();
				pParamGui->SelectProgram(se);
			}
		}
		CBaseParamView* CVstEffectWnd::CreateView()
		{
			if ( pEffect->HasEditor())
			{
				CVstGui* gui = new CVstGui(this,&vstmachine());
				gui->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
					CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL);
				return gui;
			}
			else
			{
				return CFrameMachine::CreateView();
			}
		}
		void CVstEffectWnd::GetWindowSize(CRect &rcFrame, CRect &rcClient, CRect *pRect)
		{
			CFrameMachine::GetWindowSize(rcFrame, rcClient, pRect);
			if ( Global::psycleconf().macParam().toolbarOnMachineParams)
			{
				//SM_CYBORDER The height of a window border, in pixels. This is equivalent to the SM_CYEDGE value for windows with the 3-D look.
				CRect tbRect;
				toolBar.GetWindowRect(&tbRect);
				int heiTool = tbRect.bottom - tbRect.top - (2 * ::GetSystemMetrics(SM_CYBORDER) );
				rcClient.top+=heiTool;
				rcFrame.bottom += heiTool;
			}
		}

		void CVstEffectWnd::ResizeWindow(int w,int h)
		{
			CRect rcW;
			GetWindowRect(&rcW);

			CRect rc;
			rc.left = (short)rcW.left;	rc.top = (short)rcW.top;
			rc.right = rc.left + w;	rc.bottom = rc.top + h;
			ResizeWindow(&rc);
			pView->WindowIdle();
		}
		void CVstEffectWnd::ResizeWindow(CRect* pRect)
		{
			CFrameMachine::ResizeWindow(pRect);
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
			fileName[0]='\0';
			char *filePath;

			if	((ptr->command == kVstFileLoad) 
				||	(ptr->command == kVstFileSave)
				||	(ptr->command == kVstMultipleFilesLoad))
			{
				OPENFILENAME ofn; // common dialog box structure
				// Initialize OPENFILENAME
				ZeroMemory(&ofn, sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = GetParent()->m_hWnd;
				ofn.lpstrTitle = ptr->title;
				ofn.lpstrFileTitle = fileName;
				ofn.nMaxFileTitle = sizeof (fileName) - 1;
				if (ptr->command == kVstMultipleFilesLoad) {
					filePath = new char[_MAX_PATH * 100];
					ofn.lpstrFile = filePath;
					ofn.nMaxFile = _MAX_PATH * 100 - 1;
				} else {
					filePath = new char[_MAX_PATH];
					ofn.lpstrFile = filePath;
					ofn.nMaxFile = _MAX_PATH - 1;
				}
				filePath[0] = '\0';

				std::string filefilter;
				for (int i=0;i<ptr->nbFileTypes;i++)
				{
					filefilter += ptr->fileTypes[i].name; filefilter.push_back('\0');
					filefilter += "*."; filefilter += ptr->fileTypes[i].dosType; filefilter.push_back('\0');
				}
				filefilter += "All (*.*)"; filefilter.push_back('\0');
				filefilter += "*.*"; filefilter.push_back('\0');

				ofn.lpstrFilter =filefilter.c_str();
				if (ptr->nbFileTypes >= 1) {
					ofn.lpstrDefExt = ptr->fileTypes[0].dosType;
				}
				ofn.nFilterIndex = 1;
				if ( ptr->initialPath != 0) {
					ofn.lpstrInitialDir = ptr->initialPath;
				} else {
					ofn.lpstrInitialDir =  (char*)vstmachine().OnGetDirectory();
				}
				if (ptr->command == kVstFileSave) {
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING;
				} else if (ptr->command == kVstMultipleFilesLoad) {
					ofn.Flags = OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING;
				} else {
					ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING;
				}
				// Display the Open dialog box.
				int asdf = ::GetOpenFileName(&ofn);
				if(asdf==TRUE)
				{
					if (ptr->command == kVstMultipleFilesLoad)
					{
						char string[_MAX_PATH], directory[_MAX_PATH];
						string[0] = '\0'; directory[0] = '\0';
						char *previous = ofn.lpstrFile;
						unsigned long len;
						bool dirFound = false;
						ptr->returnMultiplePaths = new char*[100];
						long i = 0;
						while (*previous != 0)
						{
							if (!dirFound) 
							{
								dirFound = true;
								strcpy (directory, previous);
								len = (unsigned long)strlen (previous) + 1;  // including 0
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
								len = (unsigned long)strlen (previous) + 1;  // including 0
								previous += len;

								ptr->returnMultiplePaths[i] = new char [strlen (string) + 1];
								strcpy (ptr->returnMultiplePaths[i++], string);
							}
						}
						ptr->nbReturnPath = i;
						delete[] filePath;
					}
					else if ( ptr->returnPath == NULL || ptr->sizeReturnPath == 0)
					{
						ptr->reserved = 1;
						ptr->returnPath = filePath;
						ptr->sizeReturnPath = (VstInt32)strlen(filePath);
						ptr->nbReturnPath = 1;
					}
					else 
					{
						strncpy(ptr->returnPath,filePath,ptr->sizeReturnPath);
						ptr->nbReturnPath = 1;
						delete[] filePath;
					}
					return true;
				}
				else delete[] filePath;
			}
			else if (ptr->command == kVstDirectorySelect)
			{
				std::string result;
				if ( ptr->returnPath == 0)
				{
					ptr->reserved = 1;
					ptr->returnPath = new char[_MAX_PATH];
					ptr->sizeReturnPath = _MAX_PATH -1;
					ptr->nbReturnPath = 1;
				}
				if(ptr->initialPath)
					result = ptr->initialPath;
				else
					result = "";
				
				if(CPsycleApp::BrowseForFolder(GetParent()->m_hWnd, ptr->title, result))
				{
					strncpy(ptr->returnPath, result.c_str(), _MAX_PATH-1);
					ptr->nbReturnPath=1;
				}
			}
			return ptr->returnPath>0;
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
				delete[] ptr->returnPath;
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
			if(index>= 0 || index < vstmachine().GetNumParams())
			{
				if(Global::psycleconf().inputHandler()._RecordTweaks)
				{
					if(Global::psycleconf().inputHandler()._RecordMouseTweaksSmooth)
						wndView->MousePatternTweakSlide(vstmachine()._macIndex, index, helpers::math::lround<int,float>(value * vst::quantization));
					else
						wndView->MousePatternTweak(vstmachine()._macIndex, index, helpers::math::lround<int,float>(value * vst::quantization));
				}
				if(pParamGui)
					pParamGui->UpdateNew(index, value);
				return true;
			}
			return false;
		}


		/***********************************************************************/


		void CVstEffectWnd::OnProgramsOpenpreset()
		{
			char tmp[2048];
			if (!vstmachine().OnGetChunkFile(tmp))
			{
				std::strncpy(tmp,reinterpret_cast<char*>(vstmachine().OnGetDirectory()),1024);
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
				if ( b.Initialized() ) vstmachine().LoadBank(b);
				else
					MessageBox("Error Loading file", NULL, MB_ICONERROR);
			}
			else if ( dlg.GetFileExt() == "fxp" )
			{
				CFxProgram p(dlg.GetPathName());
				if ( p.Initialized() ) vstmachine().LoadProgram(p);
				else
					MessageBox("Error Loading file", NULL, MB_ICONERROR);

			}
			FillProgramCombobox();
		}

		void CVstEffectWnd::OnProgramsSavepreset()
		{
			char tmp[2048];
			if (!vstmachine().OnGetChunkFile(tmp))
			{
				std::strncpy(tmp,reinterpret_cast<char*>(vstmachine().OnGetDirectory()),1024);
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
	
		void CVstEffectWnd::OnSetProgram(UINT nID)
		{
			vstmachine().SetProgram(nID - ID_SELECTPROGRAM_0);
			comboProgram.SetCurSel(nID - ID_SELECTPROGRAM_0);
			if (pParamGui && pParamGui->IsWindowVisible()){
				pParamGui->SelectProgram(nID - ID_SELECTPROGRAM_0);
			}
		}
		void CVstEffectWnd::OnProgramLess()
		{
			int numProgram = vstmachine().GetProgram()-1;
			vstmachine().SetProgram(numProgram);
			comboProgram.SetCurSel(numProgram);
			if (pParamGui && pParamGui->IsWindowVisible()){
				pParamGui->SelectProgram(numProgram);
			}
			UpdateWindow();
		}
		void CVstEffectWnd::OnUpdateProgramLess(CCmdUI *pCmdUI)
		{
			if ( vstmachine().GetProgram() == 0)
			{
				pCmdUI->Enable(false);
			}
			else pCmdUI->Enable(true);
		}
		void CVstEffectWnd::OnProgramMore()
		{
			int numProgram = vstmachine().GetProgram()+1;
			vstmachine().SetProgram(numProgram);
			comboProgram.SetCurSel(numProgram);
			if (pParamGui && pParamGui->IsWindowVisible()){
				pParamGui->SelectProgram(numProgram);
			}
			UpdateWindow();
		}
		void CVstEffectWnd::OnUpdateProgramMore(CCmdUI *pCmdUI)
		{
			if ( vstmachine().GetProgram()+1 == vstmachine().numPrograms())
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
				pParamGui= new CVstParamList(&vstmachine());
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
				pCmdUI->SetCheck(pParamGui->IsWindowVisible());
			}
			else
				pCmdUI->SetCheck(false);
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
			Global::psycleconf().macParam().toolbarOnMachineParams = !Global::psycleconf().macParam().toolbarOnMachineParams;

			if (Global::psycleconf().macParam().toolbarOnMachineParams) ShowControlBar(&toolBar,TRUE,FALSE);
			else ShowControlBar(&toolBar,FALSE,FALSE);
			ResizeWindow(0);
		}

		void CVstEffectWnd::OnUpdateViewsShowtoolbar(CCmdUI *pCmdUI)
		{
			pCmdUI->SetCheck(Global::psycleconf().macParam().toolbarOnMachineParams);
		}
	}   // namespace
}   // namespace


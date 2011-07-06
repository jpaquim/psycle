///\file
///\brief implementation file for psycle::host::CFrameMachine.
#include <psycle/host/detail/project.private.hpp>
#include "FrameMachine.hpp"

#include "InputHandler.hpp"
#include "ChildView.hpp"
#include "Machine.hpp"
#include "NativeView.hpp"
#include "MixerFrameView.hpp"
#include "Plugin.hpp"
#include "vsthost24.hpp"
#include "PresetsDlg.hpp"
#include "ParamList.hpp"

namespace psycle { namespace host {
		int const ID_TIMER_PARAM_REFRESH = 2104;
		extern CPsycleApp theApp;

		//////////////////////////////////////////////////////////////////////////

		void CMyToolBar::OnBarStyleChange(DWORD dwOldStyle, DWORD dwNewStyle)
		{
			// Call base class implementation.
			CToolBar::OnBarStyleChange(dwOldStyle, dwNewStyle);

			// Use exclusive-or to detect changes in style bits.
			DWORD changed = dwOldStyle ^ dwNewStyle;

			if (changed & CBRS_FLOATING) {
				if (dwNewStyle & CBRS_FLOATING) {
					((CFrameMachine*)GetOwner())->ResizeWindow(NULL);
				}
				else {
					((CFrameMachine*)GetOwner())->ResizeWindow(NULL);
				}
			}
#if 0
			if (changed & CBRS_ORIENT_ANY) {
				if (dwNewStyle & CBRS_ORIENT_HORZ) {
					// ToolBar now horizontal
				}
				else if (dwNewStyle & CBRS_ORIENT_VERT) {
					// ToolBar now vertical            
				}
			}
#endif
		}

		IMPLEMENT_DYNAMIC(CFrameMachine, CFrameWnd)

		BEGIN_MESSAGE_MAP(CFrameMachine, CFrameWnd)
			ON_WM_CREATE()
			ON_WM_CLOSE()
			ON_WM_DESTROY()
			ON_WM_TIMER()
			ON_WM_SETFOCUS()
			ON_WM_KEYDOWN()
			ON_WM_KEYUP()
			ON_WM_SIZING()
			ON_COMMAND_RANGE(ID_SELECTBANK_0, ID_SELECTBANK_0+99, OnSetBank)
			ON_COMMAND_RANGE(ID_SELECTPROGRAM_0, ID_SELECTPROGRAM_0+199, OnSetProgram)
			ON_COMMAND(ID_PROGRAMS_RANDOMIZEPROGRAM, OnProgramsRandomizeprogram)
			ON_COMMAND(ID_PROGRAMS_RESETDEFAULT, OnParametersResetparameters)
			ON_COMMAND(ID_OPERATIONS_ENABLED, OnOperationsEnabled)
			ON_UPDATE_COMMAND_UI(ID_OPERATIONS_ENABLED, OnUpdateOperationsEnabled)
			ON_COMMAND(ID_VIEWS_PARAMETERLIST, OnViewsParameterlist)
			ON_UPDATE_COMMAND_UI(ID_VIEWS_PARAMETERLIST, OnUpdateViewsParameterlist)
			ON_COMMAND(ID_VIEWS_BANKMANAGER, OnViewsBankmanager)
			ON_COMMAND(ID_VIEWS_SHOWTOOLBAR, OnViewsShowtoolbar)
			ON_UPDATE_COMMAND_UI(ID_VIEWS_SHOWTOOLBAR, OnUpdateViewsShowtoolbar)
			ON_COMMAND(ID_MACHINE_COMMAND, OnParametersCommand)
			ON_UPDATE_COMMAND_UI(ID_MACHINE_COMMAND, OnUpdateParametersCommand)
			ON_COMMAND(ID_ABOUT_ABOUTMAC, OnMachineAboutthismachine)
			ON_LBN_SELCHANGE(ID_COMBO_PRG, OnSelchangeProgram)
			ON_CBN_CLOSEUP(ID_COMBO_PRG, OnCloseupProgram)
			ON_COMMAND(ID_PROGRAMLESS, OnProgramLess)
			ON_UPDATE_COMMAND_UI(ID_PROGRAMLESS, OnUpdateProgramLess)
			ON_COMMAND(ID_PROGRAMMORE, OnProgramMore)
			ON_UPDATE_COMMAND_UI(ID_PROGRAMMORE, OnUpdateProgramMore)
		END_MESSAGE_MAP()

		CFrameMachine::CFrameMachine(Machine* pMachine, CChildView* wndView_, CFrameMachine** windowVar_)
		: _machine(pMachine), wndView(wndView_), windowVar(windowVar_), pView(NULL) , pParamGui(0)
		{
			//Use OnCreate.
		}

		CFrameMachine::~CFrameMachine()
		{
			//Use OnDestroy
		}

		int CFrameMachine::OnCreate(LPCREATESTRUCT lpCreateStruct) 
		{
			if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
			{
				return -1;
			}
			pView = CreateView();
			if(!pView)
			{
				TRACE0("Failed to create view window\n");
				return -1;
			}
			if ( _machine->_type == MACH_PLUGIN )
			{
				GetMenu()->GetSubMenu(1)->ModifyMenu(5, MF_BYPOSITION | MF_STRING, ID_MACHINE_COMMAND, 
					((Plugin*)_machine)->GetInfo()->Command);
			}
			if (!toolBar.CreateEx(this, TBSTYLE_FLAT|/*TBSTYLE_LIST*|*/TBSTYLE_TRANSPARENT|TBSTYLE_TOOLTIPS|TBSTYLE_WRAPABLE) ||
				!toolBar.LoadToolBar(IDR_FRAMEMACHINE))
			{
				TRACE0("Failed to create toolbar\n");
				return -1;      // fail to create
			}
#if 0
			//nice, but will make the toolbar too big.
			toolBar.SetButtonText(0,"blabla");
			CRect temp;
			toolBar.GetItemRect(0,&temp);
			toolBar.GetToolBarCtrl().SetButtonSize(CSize(temp.Width(),
				temp.Height()));
#endif

			CRect rect;
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
			comboProgram.SetFont(&font);

			FillProgramCombobox();
			toolBar.SetBarStyle(toolBar.GetBarStyle() | CBRS_FLYBY | CBRS_GRIPPER);
			toolBar.SetWindowText("Params Toolbar");
			toolBar.EnableDocking(CBRS_ALIGN_TOP);

			EnableDocking(CBRS_ALIGN_ANY);
			DockControlBar(&toolBar);
			LoadBarState(_T("VstParamToolbar"));

			// Sets Icon
			HICON tIcon;
			tIcon=theApp.LoadIcon(IDR_FRAMEMACHINE);
			SetIcon(tIcon, true);
			SetIcon(tIcon, false);

			SetTimer(ID_TIMER_PARAM_REFRESH,30,0);
			return 0;
		}

		BOOL CFrameMachine::PreCreateWindow(CREATESTRUCT& cs)
		{
			if( !CFrameWnd::PreCreateWindow(cs) )
				return FALSE;
			
			cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
			cs.lpszClass = AfxRegisterWndClass(0);
			return TRUE;
		}

		void CFrameMachine::OnClose() 
		{
			KillTimer(ID_TIMER_PARAM_REFRESH);
			CFrameWnd::OnClose();
		}

		void CFrameMachine::OnDestroy()
		{
			HICON _icon = GetIcon(false);
			DestroyIcon(_icon);
			comboProgram.DestroyWindow();
			if (pView != NULL) { pView->DestroyWindow(); delete pView; }
			if (pParamGui) pParamGui->SendMessage(WM_CLOSE);
			SaveBarState(_T("VstParamToolbar"));
		}
		void CFrameMachine::PostNcDestroy() 
		{
			if(windowVar!= NULL) *windowVar = NULL;
			delete this;
		}

		void CFrameMachine::OnTimer(UINT_PTR nIDEvent) 
		{
			if ( nIDEvent == ID_TIMER_PARAM_REFRESH )
			{
				pView->WindowIdle();
			}
			CFrameWnd::OnTimer(nIDEvent);
		}

		void CFrameMachine::OnSetFocus(CWnd* pOldWnd) 
		{
			CFrameWnd::OnSetFocus(pOldWnd);
			pView->WindowIdle();
		}

		void CFrameMachine::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{
			// ignore repeats: nFlags&0x4000
			const BOOL bRepeat = nFlags&0x4000;
			CmdDef cmd(Global::pInputHandler->KeyToCmd(nChar,nFlags));
			if(cmd.IsValid())
			{
				switch(cmd.GetType())
				{
				case CT_Note:
					if (!bRepeat)
					{
						///\todo: change the option: "notesToEffects" to mean "notesToWindowOwner".
						const int outnote = cmd.GetNote();
						if ( _machine->_mode == MACHMODE_GENERATOR || Global::psycleconf().inputHandler()._notesToEffects)
							Global::pInputHandler->PlayNote(outnote,255,127,true,_machine);
						else
							Global::pInputHandler->PlayNote(outnote);
					}
					break;

				case CT_Immediate:
				case CT_Editor:
					Global::pInputHandler->PerformCmd(cmd,bRepeat);
					break;
				}
			}
			else if(nChar == VK_ESCAPE) {
				PostMessage(WM_CLOSE);
			}
			this->SetFocus();

			CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);	
		}

		void CFrameMachine::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{

			CmdDef cmd(Global::pInputHandler->KeyToCmd(nChar,nFlags));
			const int outnote = cmd.GetNote();
			if(outnote>=0)
			{
				if ( _machine->_mode == MACHMODE_GENERATOR ||Global::psycleconf().inputHandler()._notesToEffects)
				{
					Global::pInputHandler->StopNote(outnote,255,true,_machine);
				}
				else Global::pInputHandler->StopNote(outnote);
			}

			CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
		}

		void CFrameMachine::OnSizing(UINT fwSide, LPRECT pRect)
		{
			pView->WindowIdle();
		}


		//////////////////////////////////////////////////////////////////////////
		// OnInitMenuPopup : called when a popup menu is initialized            //
		//////////////////////////////////////////////////////////////////////////

		void CFrameMachine::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
		{
			// if Effect Edit menu popping up
			if ((pPopupMenu->GetMenuItemCount() > 0) &&
				(pPopupMenu->GetMenuItemID(0) == ID_PROGRAMS_OPENPRESET))
			{
				FillBankPopup(pPopupMenu);
				FillProgramPopup(pPopupMenu);
			}

			CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
		}
		void CFrameMachine::OnSetBank(UINT nID)
		{
			int bank = nID - ID_SELECTBANK_0;
			if (bank < machine().GetNumBanks()) {
				machine().SetCurrentBank(bank);
			}
			else {
				///todo
			}

			FillProgramCombobox();
		}
		void CFrameMachine::OnSetProgram(UINT nID)
		{
			ChangeProgram(nID - ID_SELECTPROGRAM_0);
		}


		void CFrameMachine::OnProgramsRandomizeprogram()
		{
			int numParameters = machine().GetNumParams();
			for(int c(0); c < numParameters ; ++c)
			{
				int minran,maxran;
				_machine->GetParamRange(c,minran,maxran);

				int dif = (maxran-minran);

				float randsem = (float)rand()*0.000030517578125f;

				float roffset = randsem*(float)dif;

				Global::pInputHandler->AddMacViewUndo();
				machine().SetParameter(c,minran+int(roffset));
			}
			Invalidate(false);
		}

		void CFrameMachine::OnParametersResetparameters() 
		{
			if ( _machine->_type == MACH_PLUGIN)
			{
				int numpars = _machine->GetNumParams();
				for (int c=0; c<numpars; c++)
				{
					int dv = ((Plugin*)_machine)->GetInfo()->Parameters[c]->DefValue;
					Global::pInputHandler->AddMacViewUndo();
					_machine->SetParameter(c,dv);
				}
			}
			Invalidate(false);
		}

		void CFrameMachine::OnOperationsEnabled()
		{
			if (machine()._mode == MACHMODE_GENERATOR)
			{
				machine()._mute = !machine()._mute;
			}
			else
			{
				machine().Bypass(!machine().Bypass());
				if (machine()._mute) machine()._mute = false;
			}
		}

		void CFrameMachine::OnUpdateOperationsEnabled(CCmdUI *pCmdUI)
		{
			if (machine()._mode == MACHMODE_GENERATOR)
			{
				pCmdUI->SetCheck(!machine()._mute);
			}
			else
			{
				pCmdUI->SetCheck(!(machine()._mute || machine().Bypass()));
			}
		}

		void CFrameMachine::OnViewsBankmanager()
		{
			CPresetsDlg dlg;
			dlg._pMachine=_machine;
			dlg.DoModal();
		}

		void CFrameMachine::OnViewsParameterlist()
		{
			CRect rc;
			GetWindowRect(&rc);
			if (!pParamGui)
			{
				pParamGui= new CParamList(machine(), this, &pParamGui);
				pParamGui->SetWindowPos(0,rc.right+1,rc.top,0,0,SWP_NOSIZE | SWP_NOZORDER);
				pParamGui->ShowWindow(SW_SHOWNORMAL); 
			}
			else
			{
				pParamGui->SendMessage(WM_CLOSE);
			}
		}

		void CFrameMachine::OnUpdateViewsParameterlist(CCmdUI *pCmdUI)
		{
			if ( pParamGui )
			{
				pCmdUI->SetCheck(true);
			}
			else
				pCmdUI->SetCheck(false);
		}

		void CFrameMachine::OnViewsShowtoolbar()
		{
			Global::psycleconf().macParam().toolbarOnMachineParams = !Global::psycleconf().macParam().toolbarOnMachineParams;

			if (Global::psycleconf().macParam().toolbarOnMachineParams) ShowControlBar(&toolBar,TRUE,FALSE);
			else ShowControlBar(&toolBar,FALSE,FALSE);
			ResizeWindow(0);
		}

		void CFrameMachine::OnUpdateViewsShowtoolbar(CCmdUI *pCmdUI)
		{
			pCmdUI->SetCheck(Global::psycleconf().macParam().toolbarOnMachineParams);
		}

		void CFrameMachine::OnParametersCommand() 
		{
			if ( _machine->_type == MACH_PLUGIN)
			{
				((Plugin*)_machine)->GetCallback()->hWnd = m_hWnd;
				try
				{
					((Plugin*)_machine)->proxy().Command();
				}
				catch(const std::exception &)
				{
					// o_O`
				}
			}
		}

		void CFrameMachine::OnUpdateParametersCommand(CCmdUI *pCmdUI)
		{
			if ( _machine->_type == MACH_PLUGIN)
			{
				pCmdUI->Enable(true);
			}
			else {
				pCmdUI->Enable(false);
			}
		}

		void CFrameMachine::OnMachineAboutthismachine() 
		{
			if ( _machine->_type == MACH_PLUGIN)
			{
				MessageBox(CString("Authors: ") + CString(((Plugin*)_machine)->GetInfo()->Author),
						CString("About ") + CString(machine().GetName()));
			}
			else if ( _machine->_type == MACH_VST || _machine->_type == MACH_VSTFX)
			{
				///\todo: made an informative dialog like in seib's vsthost.
				MessageBox(CString("Vst Plugin by " )+ CString(((vst::plugin*)_machine)->GetVendorName()),
					CString("About") + CString(machine().GetName()));
			}
		}
		void CFrameMachine::OnSelchangeProgram() 
		{
			ChangeProgram(comboProgram.GetCurSel());
			SetFocus();
		}
		void CFrameMachine::OnCloseupProgram()
		{
			SetFocus();
		}

		void CFrameMachine::OnProgramLess()
		{
			ChangeProgram(machine().GetCurrentProgram()-1);
			UpdateWindow();
		}
		void CFrameMachine::OnUpdateProgramLess(CCmdUI *pCmdUI)
		{
			if ( machine().GetCurrentProgram() == 0)
			{
				pCmdUI->Enable(false);
			}
			else pCmdUI->Enable(true);
		}
		void CFrameMachine::OnProgramMore()
		{
			ChangeProgram(machine().GetCurrentProgram()+1);
			UpdateWindow();
		}
		void CFrameMachine::OnUpdateProgramMore(CCmdUI *pCmdUI)
		{
			if ( machine().GetCurrentProgram()+1 == machine().GetNumPrograms() || machine().GetNumPrograms()==0)
			{
				pCmdUI->Enable(false);
			}
			else pCmdUI->Enable(true);
		}

		//////////////////////////////////////////////////////////////////////////

		CBaseParamView* CFrameMachine::CreateView()
		{
			CBaseParamView* gui;
			if(machine()._type == MACH_MIXER) {
				gui = new MixerFrameView(this,&machine());
			}
			else {
				gui = new CNativeView(this,&machine());
			}
			gui->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
				CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL);
			return gui;
		}

		void CFrameMachine::PostOpenWnd()
		{
			pView->Open();
			ResizeWindow(0);
		}

		void CFrameMachine::GetWindowSize(CRect &rcFrame, CRect &rcClient, CRect *pRect)
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
			//(non rezisable) SM_CXFIXEDFRAME is the height of the horizontal border, and SM_CYFIXEDFRAME is the width of the vertical border.
			//This value is the same as SM_CXDLGFRAME/SM_CYDLGFRAME.
			//(resizable) SM_CXSIZEFRAME is the width of the horizontal border, and SM_CYSIZEFRAME is the height of the vertical border.
			//This value is the same as SM_CXFRAME/SM_CYFRAME.
			//SM_CYMENU The height of a single-line menu bar, in pixels.
			//SM_CYCAPTION The height of a caption area, in pixels.
			rcFrame.bottom += ::GetSystemMetrics(SM_CYCAPTION) +
				::GetSystemMetrics(SM_CYMENU) +
				2 * ::GetSystemMetrics(SM_CYFIXEDFRAME);
			rcFrame.right +=
				2 * ::GetSystemMetrics(SM_CXFIXEDFRAME);

			if ( Global::psycleconf().macParam().toolbarOnMachineParams && !(toolBar.GetBarStyle() & CBRS_FLOATING))
			{
				//SM_CYBORDER The height of a window border, in pixels. This is equivalent to the SM_CYEDGE value for windows with the 3-D look.
				CRect tbRect;
				toolBar.GetWindowRect(&tbRect);
				int heiTool = tbRect.bottom - tbRect.top - (2 * ::GetSystemMetrics(SM_CYBORDER) );
				rcClient.top+=heiTool;
				rcFrame.bottom += heiTool;
			}

		}
		void CFrameMachine::ResizeWindow(CRect *pRect)
		{
			CRect rcEffFrame,rcEffClient,rcTemp,tbRect;
			GetWindowSize(rcEffFrame, rcEffClient, pRect);
			SetWindowPos(NULL,0,0,rcEffFrame.right-rcEffFrame.left,rcEffFrame.bottom-rcEffFrame.top,SWP_NOZORDER | SWP_NOMOVE);
			pView->SetWindowPos(NULL,rcEffClient.left, rcEffClient.top, rcEffClient.right,rcEffClient.bottom,SWP_NOZORDER);
			pView->WindowIdle();
		}


		//////////////////////////////////////////////////////////////////////////

		void CFrameMachine::FillBankPopup(CMenu* pPopupMenu)
		{
			if (machine().GetNumBanks() > 1)
			{
				CMenu* popBnk=0;
				popBnk = pPopupMenu->GetSubMenu(3);
				if (!popBnk)
					return;

				DeleteBankMenu(popBnk);
				for (int i = 0; i < machine().GetNumBanks(); i++)
				{
					char s1[38];
					char s2[32];
					_machine->GetIndexBankName(i, s2);
					std::sprintf(s1,"%d: %s",i,s2);
					popBnk->AppendMenu(MF_STRING, ID_SELECTBANK_0 + i, s1);
				}
				popBnk->CheckMenuItem(ID_SELECTBANK_0 + machine().GetCurrentBank(),
					MF_CHECKED | MF_BYCOMMAND);
			}
		}

		bool CFrameMachine::DeleteBankMenu(CMenu* popBnk)
		{
			if (popBnk->GetMenuItemID(0) == ID_SELECTBANK_0)
			{
				while (popBnk->GetMenuItemCount() > 0)
				{
					popBnk->DeleteMenu(0,MF_BYPOSITION);
				}
				return true;
			}
			return false;
		}
		void CFrameMachine::FillProgramPopup(CMenu* pPopupMenu)
		{
			if (machine().GetTotalPrograms() > 1)
			{
				CMenu* popPrg=0;
				popPrg = pPopupMenu->GetSubMenu(4);
				if (!popPrg)
					return;

				DeleteProgramMenu(popPrg);

				for (int i = 0; i < machine().GetNumPrograms() && i < 980 ; i += 16)
				{
					CMenu popup;
					popup.CreatePopupMenu();
					for (int j = i; (j < i + 16) && (j < machine().GetNumPrograms()); j++)
					{
						char s1[38];
						char s2[32];
						_machine->GetIndexProgramName(_machine->GetCurrentBank(), j, s2);
						std::sprintf(s1,"%d: %s",j,s2);
						popup.AppendMenu(MF_STRING, ID_SELECTPROGRAM_0 + j, s1);
					}
					char szSub[256] = "";;
					std::sprintf(szSub,"Programs %d-%d",i,i+15);
					popPrg->AppendMenu(MF_POPUP | MF_STRING,
						(UINT)popup.Detach(),
						szSub);
				}
				popPrg->CheckMenuItem(ID_SELECTPROGRAM_0 + machine().GetCurrentProgram(),
					MF_CHECKED | MF_BYCOMMAND);
			}
		}

		bool CFrameMachine::DeleteProgramMenu(CMenu* popPrg)
		{
			CMenu* secMenu=0;
			if (popPrg->GetMenuItemID(0) == ID_SELECTPROGRAM_0)
			{
				popPrg->DeleteMenu(0, MF_BYPOSITION);
				return true;
			}
			else if ((secMenu=popPrg->GetSubMenu(0)))
			{
				if (secMenu->GetMenuItemID(0) == ID_SELECTPROGRAM_0)
				{
					while (popPrg->GetSubMenu(0))
					{
						popPrg->DeleteMenu(0,MF_BYPOSITION);
					}
				}
				return true;
			}
			return false;
		}
		void CFrameMachine::FillProgramCombobox()
		{
			comboProgram.ResetContent();

			int nump;
			nump = _machine->GetNumPrograms();
			for(int i(0) ; i < nump; ++i)
			{
				char s1[38];
				char s2[32];
				_machine->GetIndexProgramName(-1, i, s2);
				std::sprintf(s1,"%d: %s",i,s2);
				comboProgram.AddString(s1);
			}
			int i = _machine->GetCurrentProgram();
			if ( i > nump || i < 0) {  i = 0; }
			comboProgram.SetCurSel(i);

			if (pParamGui){
				pParamGui->InitializePrograms();
			}
		}

		void CFrameMachine::ChangeProgram(int numProgram)
		{
			_machine->SetCurrentProgram(numProgram);
			comboProgram.SetCurSel(numProgram);
			if (pParamGui){
				pParamGui->SelectProgram(numProgram);
			}
		}
		void CFrameMachine::Automate(int param, int value, bool undo)
		{
			if(undo) {
				Global::pInputHandler->AddMacViewUndo();
			}

			///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
			if(Global::psycleconf().inputHandler()._RecordTweaks)
			{
				if(Global::psycleconf().inputHandler()._RecordMouseTweaksSmooth)
					wndView->MousePatternTweakSlide(machine()._macIndex, param, value);
				else
					wndView->MousePatternTweak(machine()._macIndex, param, value );
			}
			if(pParamGui)
				pParamGui->UpdateNew(param, value);
		}

	}   // namespace
}   // namespace

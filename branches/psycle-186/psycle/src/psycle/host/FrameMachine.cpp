///\file
///\brief implementation file for psycle::host::CFrameMachine.

#include "FrameMachine.hpp"

//#include "MainFrm.hpp"
#include "InputHandler.hpp"
#include "ChildView.hpp"
//#include "Configuration.hpp"
//#include "NewVal.hpp"
#include "Machine.hpp"
#include "NativeGui.hpp"
#include "MixerFrameView.hpp"
#include "Plugin.hpp"
#include "vsthost24.hpp"
#include "PresetsDlg.hpp"

int const ID_TIMER_PARAM_REFRESH = 2104;
namespace psycle { namespace host {

		IMPLEMENT_DYNAMIC(CFrameMachine, CFrameWnd)

		BEGIN_MESSAGE_MAP(CFrameMachine, CFrameWnd)
			ON_WM_CREATE()
			ON_WM_TIMER()
			ON_WM_DESTROY()
			ON_WM_SETFOCUS()
			ON_WM_KEYDOWN()
			ON_WM_KEYUP()
			ON_WM_SIZING()
			ON_COMMAND(ID_OPERATIONS_ENABLED, OnOperationsEnabled)
			ON_UPDATE_COMMAND_UI(ID_OPERATIONS_ENABLED, OnUpdateOperationsEnabled)
			ON_COMMAND(ID_PROGRAMS_RANDOMIZEPROGRAM, OnProgramsRandomizeprogram)
			ON_COMMAND(ID_PARAMETERS_RESETPARAMETERS, OnParametersResetparameters)
			ON_COMMAND(ID_VIEWS_BANKMANAGER, OnViewsBankmanager)
			ON_UPDATE_COMMAND_UI(ID_VIEWS_BANKMANAGER, OnUpdateViewsBankmanager)
			ON_COMMAND(ID_MACHINE_COMMAND, OnParametersCommand)
			ON_COMMAND(ID_ABOUT_ABOUTMAC, OnMachineAboutthismachine)
		END_MESSAGE_MAP()

		CFrameMachine::CFrameMachine(Machine* pMachine)
		: pView(0) , _machine(pMachine)
		{
			//do not use! Use OnCreate Instead.
		}

		CFrameMachine::~CFrameMachine()
		{
			//do not use! Use OnDestroy Instead.
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
				GetMenu()->GetSubMenu(0)->ModifyMenu(0, MF_BYPOSITION | MF_STRING, ID_MACHINE_COMMAND, ((Plugin*)_machine)->GetInfo()->Command);
			}

			*_pActive=true;
			SetTimer(ID_TIMER_PARAM_REFRESH,33,0);
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

		void CFrameMachine::OnDestroy() 
		{
			KillTimer(ID_TIMER_PARAM_REFRESH);
			pView->DestroyWindow();
			if ( _pActive != NULL ) *_pActive=false;
			CFrameWnd::OnDestroy();
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
			pCmdUI->SetText("Activated");
			if (machine()._mode == MACHMODE_GENERATOR)
			{
				pCmdUI->SetCheck(!machine()._mute);
			}
			else
			{
				pCmdUI->SetCheck(!(machine()._mute || machine().Bypass()));
			}
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

				wndView->AddMacViewUndo();
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
					wndView->AddMacViewUndo();
					_machine->SetParameter(c,dv);
				}
			}
			Invalidate(false);
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


		void CFrameMachine::OnViewsBankmanager()
		{
			CPresetsDlg dlg;
			dlg._pMachine=_machine;
			dlg.DoModal();
		}

		void CFrameMachine::OnUpdateViewsBankmanager(CCmdUI *pCmdUI)
		{
		}


		/**********************************************************/

		CBaseParamView* CFrameMachine::CreateView()
		{
			CNativeGui* gui;
			if(machine()._type == MACH_MIXER) {
				gui = new MixerFrameView(this,&machine(), wndView);
			}
			else {
				gui = new CNativeGui(this,&machine(), wndView);
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
		}
		void CFrameMachine::ResizeWindow(CRect *pRect)
		{
			CRect rcEffFrame,rcEffClient,rcTemp,tbRect;
			GetWindowSize(rcEffFrame, rcEffClient, pRect);
			SetWindowPos(NULL,0,0,rcEffFrame.right-rcEffFrame.left,rcEffFrame.bottom-rcEffFrame.top,SWP_NOZORDER | SWP_NOMOVE);
			pView->SetWindowPos(NULL,rcEffClient.left, rcEffClient.top, rcEffClient.right,rcEffClient.bottom,SWP_NOZORDER);
			pView->WindowIdle();
		}

		void CFrameMachine::OnSizing(UINT fwSide, LPRECT pRect)
		{
			pView->WindowIdle();
		}

	}   // namespace
}   // namespace

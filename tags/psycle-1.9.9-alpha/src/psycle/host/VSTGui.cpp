///\file
///\brief implementation file for psycle::host::CVstGui.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/psycle.hpp>
#include <psycle/host/VstGui.hpp>
//#include <psycle/host/inputhandler.hpp>
#include <psycle/host/MainFrm.hpp>
#include <psycle/engine/machine.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		extern CPsycleApp theApp;

		IMPLEMENT_DYNCREATE(CVstGui, CFrameWnd)

		CVstGui::CVstGui()
		{
		}

		CVstGui::~CVstGui()
		{
		}

		BEGIN_MESSAGE_MAP(CVstGui, CFrameWnd)
			//{{AFX_MSG_MAP(CVstGui)
			ON_WM_PAINT()
			ON_WM_DESTROY()
			ON_WM_ENTERIDLE()
			ON_WM_TIMER()
			ON_WM_KEYDOWN()
			ON_WM_KEYUP()
			ON_WM_LBUTTONDOWN()
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		void CVstGui::DoTheInit()
		{
			try
			{
				proxy->dispatcher(effEditTop);
			}
			catch(const std::exception &)
			{
				// o_O`
			}
			SetTimer(0, 25, 0);
		}

		void CVstGui::OnPaint() 
		{
			CPaintDC dc(this); // device context for painting
		}

		void CVstGui::OnDestroy()
		{
			try
			{
				proxy->dispatcher(effEditClose);
			}
			catch(const std::exception &)
			{
				// o_O`
			}
			CFrameWnd::OnDestroy();
		}

		void CVstGui::OnEnterIdle(UINT nWhy, CWnd* pWho) 
		{
			CFrameWnd::OnEnterIdle(nWhy, pWho);
			try
			{
				proxy->dispatcher(effEditIdle);
			}
			catch(const std::exception &)
			{
				// o_O`
			}
		}

		void CVstGui::OnTimer(UINT nIDEvent)
		{
			try
			{
				proxy->dispatcher(effEditIdle);
			}
			catch(const std::exception &)
			{
				// o_O`
			}
			CFrameWnd::OnTimer(nIDEvent);
		}

		void CVstGui::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{
			const BOOL bRepeat = nFlags&0x4000;
			CmdDef cmd(UIGlobal::pInputHandler->KeyToCmd(nChar,nFlags));
			if(!bRepeat && cmd.IsValid())
			{
				switch(cmd.GetType())
				{
				case CT_Note:
					{
						const int outnote = cmd.GetNote();
						if ( _pMachine->_mode == MACHMODE_GENERATOR || Global::configuration()._notesToEffects)
						{
							UIGlobal::pInputHandler->PlayNote(outnote,127,true,_pMachine);
						}
						else UIGlobal::pInputHandler->PlayNote(outnote,127,true);
					}
					break;
				case CT_Immediate:
					UIGlobal::pInputHandler->PerformCmd(cmd,bRepeat);
					break;
				}
			}

		//	((CMainFrame *)theApp.m_pMainWnd)->m_wndView.KeyDown(nChar, nRepCnt, nFlags);
			CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		}

		void CVstGui::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{
			CmdDef cmd(UIGlobal::pInputHandler->KeyToCmd(nChar,nFlags));
			const int outnote = cmd.GetNote();
			if(outnote != -1) {
				if(_pMachine->_mode == MACHMODE_GENERATOR || Global::configuration()._notesToEffects)
					UIGlobal::pInputHandler->StopNote(outnote, true, _pMachine);
				else
					UIGlobal::pInputHandler->StopNote(outnote, true);
			}
			CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
		}

		void CVstGui::OnLButtonDown(UINT nFlags, CPoint point) 
		{
			this->SetFocus();
			CFrameWnd::OnLButtonDown(nFlags, point);
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END

#include <psycle/host/detail/project.private.hpp>
#include "InstrumentEditorUI.hpp"
#include "MainFrm.hpp"
#include "InputHandler.hpp"

/////////////////////////////////////////////////////////////////////////////
// InstrumentEditorUI dialog

namespace psycle { namespace host {


IMPLEMENT_DYNAMIC(InstrumentEditorUI, CPropertySheet)

InstrumentEditorUI::InstrumentEditorUI(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
: CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
, init(false)
, windowVar_(NULL)
{
}

InstrumentEditorUI::InstrumentEditorUI(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
, init(false)
, windowVar_(NULL)
{
}
InstrumentEditorUI::~InstrumentEditorUI()
{
}

BEGIN_MESSAGE_MAP(InstrumentEditorUI, CPropertySheet)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void InstrumentEditorUI::OnClose()
{
	CPropertySheet::OnClose();
	DestroyWindow();
}
void InstrumentEditorUI::PostNcDestroy()
{
	CPropertySheet::PostNcDestroy();
	if(windowVar_!= NULL) *windowVar_ = NULL;
	delete this;
}
BOOL InstrumentEditorUI::PreTranslateChildMessage(MSG* pMsg, HWND focusWin)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		/*	DWORD dwID = GetDlgCtrlID(focusWin);
		if (dwID == IDD_SOMETHING)*/
		TCHAR out[256];
		GetClassName(focusWin,out,256);
		bool editbox=(strncmp(out,"Edit",5) == 0);
		if (pMsg->wParam == VK_ESCAPE) {
			PostMessage (WM_CLOSE);
			return TRUE;
		} else if (pMsg->wParam == VK_UP ||pMsg->wParam == VK_DOWN ||pMsg->wParam == VK_LEFT 
			|| pMsg->wParam == VK_RIGHT ||pMsg->wParam == VK_TAB || pMsg->wParam == VK_NEXT 
			||pMsg->wParam == VK_PRIOR || pMsg->wParam == VK_HOME|| pMsg->wParam == VK_END) {
			return FALSE;
		} else if (!editbox) {
			// get command
			CmdDef cmd = PsycleGlobal::inputHandler().KeyToCmd(pMsg->wParam,pMsg->lParam>>16);
			if(cmd.IsValid() && (cmd.GetType() == CT_Note || cmd.GetType() == CT_Immediate ))
			{
				CMainFrame* win = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
				win->m_wndView.SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
				return TRUE;
			}
		}
	}
	else if (pMsg->message == WM_KEYUP)
	{
		TCHAR out[256];
		GetClassName(focusWin,out,256);
		bool editbox=(strncmp(out,"Edit",5) == 0);
		if (pMsg->wParam == VK_ESCAPE) {
			return TRUE;
		} else if (pMsg->wParam == VK_UP ||pMsg->wParam == VK_DOWN ||pMsg->wParam == VK_LEFT 
			|| pMsg->wParam == VK_RIGHT ||pMsg->wParam == VK_TAB || pMsg->wParam == VK_NEXT 
			||pMsg->wParam == VK_PRIOR || pMsg->wParam == VK_HOME|| pMsg->wParam == VK_END) {
			//default action.
			return FALSE;
		} else if (!editbox) {
			// get command
			CmdDef cmd = PsycleGlobal::inputHandler().KeyToCmd(pMsg->wParam,pMsg->lParam>>16);
			if(cmd.IsValid() && (cmd.GetType() == CT_Note || cmd.GetType() == CT_Immediate ))
			{
				CMainFrame* win = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
				win->m_wndView.SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
				return TRUE;
			}
		}
	}

	return FALSE;
}


void InstrumentEditorUI::Init(InstrumentEditorUI** windowVar) 
{
	windowVar_ = windowVar;
	AddPage(&m_InstrBasic);
	AddPage(&m_InstrSampulse);
	AddPage(&m_SampleBank);
	init = true;
}
void InstrumentEditorUI::UpdateUI(void)
{
	if ( !init ) return;
	if (GetActivePage() == &m_InstrBasic ) m_InstrBasic.WaveUpdate();
	//TODO: Refresh other tabs too.
}

}}

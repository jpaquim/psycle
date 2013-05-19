#include <psycle/host/detail/project.private.hpp>
#include "XMSamplerUI.hpp"
#include "XMSampler.hpp"
#include "MainFrm.hpp"
#include "InputHandler.hpp"

/////////////////////////////////////////////////////////////////////////////
// XMSamplerUI dialog

namespace psycle { namespace host {


IMPLEMENT_DYNAMIC(XMSamplerUI, CPropertySheet)

XMSamplerUI::XMSamplerUI(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
: CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
, init(false)
, windowVar_(NULL)
{
}

XMSamplerUI::XMSamplerUI(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
, init(false)
, windowVar_(NULL)
{
}
XMSamplerUI::~XMSamplerUI()
{
}

BEGIN_MESSAGE_MAP(XMSamplerUI, CPropertySheet)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void XMSamplerUI::OnClose()
{
	CPropertySheet::OnClose();
	DestroyWindow();
}
void XMSamplerUI::PostNcDestroy()
{
	CPropertySheet::PostNcDestroy();
	if(windowVar_!= NULL) *windowVar_ = NULL;
	delete this;
}

BOOL XMSamplerUI::PreTranslateChildMessage(MSG* pMsg, HWND focusWin)
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


void XMSamplerUI::Init(XMSampler* pMachine,XMSamplerUI** windowVar) 
{
	windowVar_ = windowVar;
	_pMachine = pMachine;
	m_General.pMachine(pMachine);
	m_Instrument.pMachine(pMachine);
	m_Sample.pMachine(pMachine);
	m_Mixer.pMachine(pMachine);
	AddPage(&m_Mixer);
	AddPage(&m_Instrument);
	AddPage(&m_Sample);
	AddPage(&m_General);
	init = true;
}
void XMSamplerUI::UpdateUI(void)
{
	if ( !init ) return;
	if (GetActivePage() == &m_Mixer ) m_Mixer.UpdateAllChannels();
}

}}

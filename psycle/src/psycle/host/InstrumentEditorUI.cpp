#include <psycle/host/detail/project.private.hpp>
#include "InstrumentEditorUI.hpp"
#include "MainFrm.hpp"
#include "InputHandler.hpp"
#include <psycle/host/Song.hpp>

/////////////////////////////////////////////////////////////////////////////
// InstrumentEditorUI dialog

namespace psycle { namespace host {


IMPLEMENT_DYNAMIC(InstrumentEditorUI, CPropertySheet)

InstrumentEditorUI::InstrumentEditorUI(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
: CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
, init(false)
, windowVar_(NULL)
{
	m_hAccelTable = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_INSTRUMENTEDIT));
}

InstrumentEditorUI::InstrumentEditorUI(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
, init(false)
, windowVar_(NULL)
{
	m_hAccelTable = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_INSTRUMENTEDIT));
}
InstrumentEditorUI::~InstrumentEditorUI()
{
}

BEGIN_MESSAGE_MAP(InstrumentEditorUI, CPropertySheet)
	ON_COMMAND(ID_ACCEL_SAMPLER, OnShowSampler)
	ON_COMMAND(ID_ACCEL_SAMPULSE, OnShowSampulse)
	ON_COMMAND(ID_ACCEL_WAVE, OnShowWaveBank)
	ON_COMMAND(ID_ACCEL_GEN, OnShowGen)
	ON_COMMAND(ID_ACCEL_VOL, OnShowVol)
	ON_COMMAND(ID_ACCEL_PAN, OnShowPan)
	ON_COMMAND(ID_ACCEL_FILTER, OnShowFilter)
	ON_COMMAND(ID_ACCEL_PITCH, OnShowPitch)
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
	if (m_hAccelTable) {
      if (::TranslateAccelerator(m_hWnd, m_hAccelTable, pMsg)) {
         return(TRUE);
      }
    }
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
	if (GetActivePage() == &m_InstrSampulse ) m_InstrSampulse.SetInstrumentData(Global::song().instSelected);
	if (GetActivePage() == &m_SampleBank ) m_SampleBank.WaveUpdate();
}

void InstrumentEditorUI::ShowSampler() {
	SetActivePage(0);
}
void InstrumentEditorUI::ShowSampulse() {
	SetActivePage(1);
}

void InstrumentEditorUI::OnShowSampler() {
	SetActivePage(0);
}
void InstrumentEditorUI::OnShowSampulse() {
	SetActivePage(1);
}
void InstrumentEditorUI::OnShowWaveBank() {
	SetActivePage(2);
}
void InstrumentEditorUI::OnShowGen() {
	SetActivePage(1);
	m_InstrSampulse.SetActivePage(0);
}
void InstrumentEditorUI::OnShowVol() {
	SetActivePage(1);
	m_InstrSampulse.SetActivePage(1);
}
void InstrumentEditorUI::OnShowPan() {
	SetActivePage(1);
	m_InstrSampulse.SetActivePage(2);
}
void InstrumentEditorUI::OnShowFilter() {
	SetActivePage(1);
	m_InstrSampulse.SetActivePage(3);
}
void InstrumentEditorUI::OnShowPitch() {
/*	SetActivePage(1);
	m_InstrSampulse.SetActivePage(4);
*/
}


}}

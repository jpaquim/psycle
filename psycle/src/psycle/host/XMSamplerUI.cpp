
#include "XMSamplerUI.hpp"
#include "Psycle.hpp"
#include "XMSampler.hpp"
#include "ChildView.hpp"
#include "MachineGui.hpp"

/////////////////////////////////////////////////////////////////////////////
// XMSamplerUI dialog

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)
extern CPsycleApp theApp;

IMPLEMENT_DYNAMIC(XMSamplerUI, CPropertySheet)

XMSamplerUI::XMSamplerUI(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
: CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
, init(false)
	{
	}

XMSamplerUI::XMSamplerUI(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
, init(false)
	{
	}

XMSamplerUI::XMSamplerUI(LPCTSTR pszCaption, MachineGui* gui, CWnd* pParentWnd, UINT iSelectPage)
:CPropertySheet(pszCaption, pParentWnd, iSelectPage),
gui_(gui)
, init(false)
	{
	}

BEGIN_MESSAGE_MAP(XMSamplerUI, CPropertySheet)
	//{{AFX_MSG_MAP(XMSamplerUI)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///void XMSamplerUI::OnClose()
void XMSamplerUI::OnDestroy()
{
#ifdef use_test_canvas
	gui_->BeforeDeleteDlg();
#else
	((CChildView*)m_pParentWnd)->XMSamplerMachineDialog = NULL;
#endif
	CPropertySheet::OnDestroy();
}


void XMSamplerUI::Init(XMSampler* pMachine) 
	{
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

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

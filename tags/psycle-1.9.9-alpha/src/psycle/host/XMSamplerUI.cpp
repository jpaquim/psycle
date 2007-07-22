#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/Psycle.hpp>
#include <psycle/host/XMSamplerUI.hpp>
#include <psycle/engine/XMSampler.hpp>
#include <psycle/host/ChildView.hpp>

/////////////////////////////////////////////////////////////////////////////
// XMSamplerUI dialog

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
extern CPsycleApp theApp;

IMPLEMENT_DYNAMIC(XMSamplerUI, CPropertySheet)

XMSamplerUI::XMSamplerUI(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
: CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
	{
		init=false;
	}

XMSamplerUI::XMSamplerUI(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
	{
		init=false;
	}

BEGIN_MESSAGE_MAP(XMSamplerUI, CPropertySheet)
	//{{AFX_MSG_MAP(XMSamplerUI)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///void XMSamplerUI::OnClose()
void XMSamplerUI::OnDestroy()
{
	((CChildView*)m_pParentWnd)->XMSamplerMachineDialog = NULL;
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

UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END

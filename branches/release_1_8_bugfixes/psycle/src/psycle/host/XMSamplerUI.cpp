#include <project.private.hpp>
#include "Psycle.hpp"
#include "XMSamplerUI.hpp"
#include "XMSampler.hpp"


/////////////////////////////////////////////////////////////////////////////
// XMSamplerUI dialog

NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)
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

XMSamplerUI::~XMSamplerUI()
	{
	}

BEGIN_MESSAGE_MAP(XMSamplerUI, CPropertySheet)
	//{{AFX_MSG_MAP(XMSamplerUI)
	// NOTE - the ClassWizard will add and remove mapping macros here.	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void XMSamplerUI::Init(XMSampler* pMachine) 
	{
	_pMachine = pMachine;
	m_General.pMachine(pMachine);
//	m_Instrument.pMachine(pMachine);
	m_Sample.pMachine(pMachine);
	m_Mixer.pMachine(pMachine);
	AddPage(&m_Mixer);
//	AddPage(&m_Instrument);
	AddPage(&m_Sample);
	AddPage(&m_General);
	init = true;
}
void XMSamplerUI::UpdateUI(void)
{
	if ( !init ) return;
	if (GetActivePage() == &m_Mixer ) m_Mixer.UpdateAllChannels();
}
int XMSamplerUI::DoModal() 
	{
	int retVal = CPropertySheet::DoModal();
	if (retVal == IDOK)
		{

		}
	return retVal;
	}


NAMESPACE__END
NAMESPACE__END
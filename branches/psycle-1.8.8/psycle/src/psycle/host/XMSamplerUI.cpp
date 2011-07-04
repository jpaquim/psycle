#include "XMSamplerUI.hpp"

#include "ChildView.hpp"

#include "XMSampler.hpp"

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


void XMSamplerUI::PostNcDestroy()
{
	if(windowVar_!= NULL) *windowVar_ = NULL;
	delete this;
}
void XMSamplerUI::OnClose()
{
	CPropertySheet::OnClose();
	DestroyWindow();
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

}   // namespace
}   // namespace

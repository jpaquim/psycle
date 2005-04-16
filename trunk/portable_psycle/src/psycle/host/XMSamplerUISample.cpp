#include "project.private.hpp"
#include "Psycle.hpp"
#include "XMSamplerUISample.hpp"
#include "XMSampler.hpp"
#include ".\xmsampleruisample.hpp"

NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)

// XMSamplerUISample

IMPLEMENT_DYNAMIC(XMSamplerUISample, CPropertyPage)
XMSamplerUISample::XMSamplerUISample()
: CPropertyPage(XMSamplerUISample::IDD)
{
}

XMSamplerUISample::~XMSamplerUISample()
{
}

void XMSamplerUISample::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}



BEGIN_MESSAGE_MAP(XMSamplerUISample, CPropertyPage)
	ON_LBN_SELCHANGE(IDC_SAMPLELIST, OnLbnSelchangeSamplelist)
END_MESSAGE_MAP()

// Controladores de mensajes de XMSamplerUISample
BOOL XMSamplerUISample::OnSetActive()
{
	//Fill Samples' list.
	//Select a sample
	//call list's onselect() to update the sample's information.

	return CPropertyPage::OnSetActive();
}

void XMSamplerUISample::OnLbnSelchangeSamplelist()
{
	// TODO: Agregue aquí su código de controlador de notificación de control
}

NAMESPACE__END
NAMESPACE__END
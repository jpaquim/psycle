/** @file
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */

#include <project.private.hpp>
#include "psycle.hpp"
#include "XMSamplerUISample.hpp"

NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)
// XMSamplerUISample

IMPLEMENT_DYNCREATE(XMSamplerUISample, CPropertyPage)

XMSamplerUISample::XMSamplerUISample() : CPropertyPage(XMSamplerUISample::IDD)
	{
	}

XMSamplerUISample::~XMSamplerUISample()
	{
	}

void XMSamplerUISample::DoDataExchange(CDataExchange* pDX)
	{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDirectoryDlg)
	//}}AFX_DATA_MAP
	}

BOOL XMSamplerUISample::OnInitDialog() 
	{
	CPropertyPage::OnInitDialog();
	return TRUE;
	}


BEGIN_MESSAGE_MAP(XMSamplerUISample, CPropertyPage)
END_MESSAGE_MAP()


NAMESPACE__END
NAMESPACE__END

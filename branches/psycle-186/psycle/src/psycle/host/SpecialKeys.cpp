
#include "SpecialKeys.hpp"
#include "Psycle.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)

CSpecialKeys::CSpecialKeys(CWnd* pParent /* = 0 */) : CDialog(CSpecialKeys::IDD, pParent)
,key(0)
,mod(0)
{
}

void CSpecialKeys::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSpecialKeys, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CSpecialKeys::OnInitDialog() 
{
	CDialog::OnInitDialog();
	return TRUE;
}

void CSpecialKeys::OnBnClickedOk()
{
	key=0;mod=0;
	if (((CButton *)GetDlgItem(IDC_SPKEYS_SPACE))->GetCheck()) key = 0x20;
	else if (((CButton *)GetDlgItem(IDC_SPKEYS_TAB))->GetCheck()) key = 0x09;
	else if (((CButton *)GetDlgItem(IDC_SPKEYS_BACKSPACE))->GetCheck()) key = 0x08;
	else if (((CButton *)GetDlgItem(IDC_SPKEYS_DELETE))->GetCheck())
	{	key = 0x2e;	mod=0x04;	}
	else if (((CButton *)GetDlgItem(IDC_SPKEYS_RETURN))->GetCheck()) key = 0x0d;
	else if (((CButton *)GetDlgItem(IDC_SPKEYS_INTRO))->GetCheck())
	{	key = 0x0d; mod=0x04;	}
	else if (((CButton *)GetDlgItem(IDC_SPKEYS_PREVPAG))->GetCheck())
	{ key = 0x21; mod=0x04; }
	else if (((CButton *)GetDlgItem(IDC_SPKEYS_NEXTPAG))->GetCheck())
	{ key = 0x22; mod=0x04; }

	if ( ((CButton *)GetDlgItem(IDC_SPKEYS_SHIFT))->GetCheck() ) mod++;
	if ( ((CButton *)GetDlgItem(IDC_SPKEYS_CONTROL))->GetCheck() ) mod+=2;

	OnOK();
}


PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

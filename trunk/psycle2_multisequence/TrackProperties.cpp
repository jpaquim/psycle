// TrackProperties.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle.h"
#include "TrackProperties.h"
#include ".\trackproperties.h"


// CTrackProperties dialog

IMPLEMENT_DYNAMIC(CTrackProperties, CDialog)
CTrackProperties::CTrackProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CTrackProperties::IDD, pParent)
{
}

CTrackProperties::~CTrackProperties()
{
}

void CTrackProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTrackProperties, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_WM_ACTIVATE()
	ON_EN_CHANGE(IDC_TRACKNAMEEB, OnEnChangeTracknameeb)
END_MESSAGE_MAP()


// CTrackProperties message handlers

void CTrackProperties::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CEdit *cc =(CEdit *)this->GetDlgItem(IDC_TRACKNAMEEB);
	cc->GetWindowText(trackName,32);
	OnOK();
}

void CTrackProperties::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);
	CEdit *cc =(CEdit *)this->GetDlgItem(IDC_TRACKNAMEEB);
	cc->SetWindowText(trackName);
	// TODO: Add your message handler code here
}

void CTrackProperties::OnEnChangeTracknameeb()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here

}

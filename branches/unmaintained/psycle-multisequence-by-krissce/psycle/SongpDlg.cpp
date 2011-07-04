#include "stdafx.h"

// SongpDlg.cpp : implementation file
//

#include "Psycle.h"
#include "Song.h"
#include "SongpDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSongpDlg dialog


CSongpDlg::CSongpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSongpDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSongpDlg)
	//}}AFX_DATA_INIT
}


void CSongpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSongpDlg)
	DDX_Control(pDX, IDC_EDIT4, m_songcomments);
	DDX_Control(pDX, IDC_EDIT3, m_songcredits);
	DDX_Control(pDX, IDC_EDIT1, m_songtitle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSongpDlg, CDialog)
	//{{AFX_MSG_MAP(CSongpDlg)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeTitle)
	ON_EN_CHANGE(IDC_EDIT3, OnChangeAuthor)
	ON_EN_CHANGE(IDC_EDIT4, OnChangeComment)
	ON_BN_CLICKED(IDC_BUTTON1, OnOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSongpDlg message handlers

BOOL CSongpDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_songtitle.SetLimitText(64);
	m_songcredits.SetLimitText(64);
	m_songcomments.SetLimitText(256);

	m_songtitle.SetWindowText(_pSong->Name);
	m_songcredits.SetWindowText(_pSong->Author);
	m_songcomments.SetWindowText(_pSong->Comment);

	m_songtitle.SetFocus();
	m_songtitle.SetSel(0,-1);

	return FALSE;
}

void CSongpDlg::OnChangeTitle() 
{
	m_songtitle.GetWindowText(_pSong->Name,64);
}

void CSongpDlg::OnChangeAuthor() 
{
	m_songcredits.GetWindowText(_pSong->Author,64);
}

void CSongpDlg::OnChangeComment() 
{
	m_songcomments.GetWindowText(_pSong->Comment,256);
}

void CSongpDlg::OnOk() 
{
	CDialog::OnOK();
}
// ControlDesk.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
//#include "psy.h"
#include "ChildView.h"
#include "ControlDesk.h"
#include "InstrumentEditor.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CControlDesk dialog


CControlDesk::CControlDesk(CWnd* pParent /*=NULL*/)
	: CDialog(CControlDesk::IDD, pParent)
{
	//{{AFX_DATA_INIT(CControlDesk)
	//}}AFX_DATA_INIT
}


void CControlDesk::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CControlDesk)
	DDX_Control(pDX, IDC_CURRLINE3, m_loopend);
	DDX_Control(pDX, IDC_CURRLINE2, m_loopbegin);
	DDX_Control(pDX, IDC_PLAYLOOPING, m_ploop);
	DDX_Control(pDX, IDC_PLAYDISPLAY, m_playdisplay);
	DDX_Control(pDX, IDC_CURRLINE, m_timeline);
	DDX_Control(pDX, IDC_BPMLABEL, m_bpmlabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CControlDesk, CDialog)
	//{{AFX_MSG_MAP(CControlDesk)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_PLAYLOOPING, OnPlaylooping)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CControlDesk message handlers

void CControlDesk::OnButton1() 
{
}

BOOL CControlDesk::OnInitDialog() 
{
	CDialog::OnInitDialog();
	DoUpdate();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CControlDesk::OnButton2() 
{
	if(songRef->BeatsPerMin>33)
	{
	songRef->SetBPM(--songRef->BeatsPerMin,44100);

	char buffer[4];
	sprintf(buffer,"%d",songRef->BeatsPerMin);
	m_bpmlabel.SetWindowText(buffer);
	}
}

void CControlDesk::OnButton4() 
{
	if(songRef->BeatsPerMin<256)
	{
	songRef->SetBPM(++songRef->BeatsPerMin,44100);

	char buffer[4];
	sprintf(buffer,"%d",songRef->BeatsPerMin);
	m_bpmlabel.SetWindowText(buffer);
	}
}

void CControlDesk::OnPlaylooping() 
{
	if(m_ploop.GetCheck()==1)
		songRef->loopPlay=true;
	else
		songRef->loopPlay=false;
}

void CControlDesk::UpdateLoops()
{
	char buffer[16];
	sprintf(buffer,"%d",songRef->loopBeg);
	m_loopbegin.SetWindowText(buffer);
	sprintf(buffer,"%d",songRef->loopEnd);
	m_loopend.SetWindowText(buffer);
}


void CControlDesk::DoUpdate()
{
char buffer[32];

if(songRef->loopPlay)
m_ploop.SetCheck(1);
else
m_ploop.SetCheck(0);
	
UpdateLoops();
	
sprintf(buffer,"%d",songRef->BeatsPerMin);
m_bpmlabel.SetWindowText(buffer);
}

#include "stdafx.h"

// MidiLearn.cpp : implementation file
//

#include "Psycle.h"
#include "OutputDlg.h"
#include "MidiInput.h"
#include "Configuration.h"
#include "MidiLearn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMidiLearn dialog


CMidiLearn::CMidiLearn(CWnd* pParent /*=NULL*/)
	: CDialog(CMidiLearn::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMidiLearn)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	Message = -1;
//	Global::pConfig->_pMidiInput->fnMidiCallback_Test = (MidiCallback_Test);
}


void CMidiLearn::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMidiLearn)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_TEXT, m_Text);
	//}}AFX_DATA_MAP
}



BEGIN_MESSAGE_MAP(CMidiLearn, CDialog)
	//{{AFX_MSG_MAP(CMidiLearn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMidiLearn message handlers

void CMidiLearn::OnCancel() 
{
	// TODO: Add extra cleanup here
	Message = -1;
	
	CDialog::OnCancel();
}


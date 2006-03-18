// SequencerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "SequencerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSequencerDlg dialog


CSequencerDlg::CSequencerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSequencerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSequencerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSequencerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSequencerDlg)
	DDX_Control(pDX, IDC_SEQVIEW, m_seqview);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSequencerDlg, CDialog)
	//{{AFX_MSG_MAP(CSequencerDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSequencerDlg message handlers

CSequencerDlg::UpdateSequencer()
{
CClientDC canvas((CWnd *)&m_seqview);

	int yoffset=0;
	char buffer[32];
	COLORREF cl;
	COLORREF clf;

	for(int c=-5;c<6;c++)
	{
		int const relp=c+songRef->editPosition;
		yoffset=80+(c*16);

		if(relp>=0 && relp<songRef->playLength)
		{
			if(c==0)
			{
				cl=0x00005577;
				clf=0x00AABBCC;
			}
			else
			{
				cl=0x00000000;
				clf=0x00667788;
			}

			if(songRef->playPosition==relp)
			{
				cl+=0x00666666;
				clf-=0x00223344;
			}

			canvas.SetBkColor(cl);
			canvas.SetTextColor(clf);
			
			sprintf(buffer,"%.3d ",relp);
			canvas.TextOut(0,yoffset,buffer);
			
			canvas.SetBkColor(cl+0x00112244);
			canvas.SetTextColor(clf+0x00112233);
			
			sprintf(buffer,"%.3d ",songRef->playOrder[relp]);
			canvas.TextOut(28,yoffset,buffer);

		}
		else
		canvas.FillSolidRect(0,yoffset,57,16,0);
	
	}
}

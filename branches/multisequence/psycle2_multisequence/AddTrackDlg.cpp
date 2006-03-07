// AddTrackDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle.h"
#include "AddTrackDlg.h"
#include "machine.h"
#include "Song.h"


// CAddTrackDlg dialog

IMPLEMENT_DYNAMIC(CAddTrackDlg, CDialog)
CAddTrackDlg::CAddTrackDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddTrackDlg::IDD, pParent)
{
}

CAddTrackDlg::~CAddTrackDlg()
{
}

void CAddTrackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MACH_LB, trackList);
}


BEGIN_MESSAGE_MAP(CAddTrackDlg, CDialog)
	ON_WM_ACTIVATE()
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CAddTrackDlg message handlers

void CAddTrackDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);

	// TODO: Add your message handler code here
	char cbuffer[64];
	trackList.ResetContent();
	for(unsigned int i = 0; i < MAX_MACHINES-1; i++)
	{
		if (Global::_pSong->_pMachine[i]) 
		{
			Machine *tmac	= Global::_pSong->_pMachine[i];
			// machine track name
			sprintf(cbuffer, "%s", tmac->_editName);
			trackList.AddString(cbuffer);
		}
	}
	trackList.AddString("...default");
}

void CAddTrackDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	char cbuffer[64];
	int tID = trackList.GetCurSel();
	trackList.GetText(tID, cbuffer);
	if ( strcmp("...default",cbuffer) == 0 )
	{
		int nextTrack = Global::_pSong->pSequencer->machineList.size() + 1;
		if ( nextTrack < MAX_MACHINES ) 
			Global::_pSong->pSequencer->CreateMachineSequenceTrack(nextTrack, "default");
	}
	else 
	{
		for(unsigned int i = 0; i < MAX_MACHINES-1; i++)
		{
			if (Global::_pSong->_pMachine[i]) 
			{
				Machine *tmac	= Global::_pSong->_pMachine[i];
				// machine track name
				if ( strcmp(tmac->_editName, cbuffer) == 0 )
				{
					bool exist = false;
					for (unsigned int x = 0; x < Global::_pSong->pSequencer->machineList.size(); x++) 
					{
						if ( Global::_pSong->pSequencer->machineList.at(x) == i ) 
						{
							int nextTrack = Global::_pSong->pSequencer->machineList.size() + 1;
							Global::_pSong->pSequencer->CreateMachineSequenceTrack(nextTrack,tmac->_editName);
							exist = true;
						}
					}
					if ( exist == false ) 
						Global::_pSong->pSequencer->machineList.push_back(i);
					break;
				}
			}
		}
	}

	OnOK();
}

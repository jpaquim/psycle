// SaveWavDlg.cpp : implementation file
//

#include "stdafx.h"
#include "psycle2.h"
#include "SaveWavDlg.h"
#include "Global.h"
#include "Song.h"
#include "configuration.h"
#include "MidiInput.h"
#include "Player.h"
#include "Machine.h"
#include "helpers.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DWORD WINAPI __stdcall RecordThread(void *b);


/////////////////////////////////////////////////////////////////////////////
// CSaveWavDlg dialog


CSaveWavDlg::CSaveWavDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSaveWavDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSaveWavDlg)
	//}}AFX_DATA_INIT
}


void CSaveWavDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveWavDlg)
	DDX_Control(pDX, IDC_SAVEWAVE, m_savewave);
	DDX_Control(pDX, IDC_SAVEWIRESSEPARATED, m_savewires);
	DDX_Control(pDX, IDC_SAVESONG, m_savesong);
	DDX_Control(pDX, IDC_RANGESTART, m_rangestart);
	DDX_Control(pDX, IDC_RANGEEND, m_rangeend);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_PATNUMBER, m_patnumber);
	DDX_Control(pDX, IDC_FILENAME, m_filename);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveWavDlg, CDialog)
	//{{AFX_MSG_MAP(CSaveWavDlg)
	ON_BN_CLICKED(IDC_FILEBROWSE, OnFilebrowse)
	ON_BN_CLICKED(IDC_SAVESONG, OnSelAllSong)
	ON_BN_CLICKED(IDC_SAVERANGE, OnSelRange)
	ON_BN_CLICKED(IDC_SAVEPATTERN, OnSelPattern)
	ON_BN_CLICKED(IDC_SAVEWAVE, OnSavewave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveWavDlg message handlers

BOOL CSaveWavDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	Song* pSong = Global::_pSong;
	thread_handle=INVALID_HANDLE_VALUE;
	kill_thread=0;
	
	CString name = Global::pConfig->GetSongDir();
	name+="\\";
	name+=pSong->fileName;
	name = name.Left(name.GetLength()-4);
	name+=".wav";
	m_filename.SetWindowText(name);
	
	m_savesong.SetCheck(1);
	m_rangeend.EnableWindow(FALSE);
	m_rangestart.EnableWindow(FALSE);
	m_patnumber.EnableWindow(FALSE);
	
	char num[3];
	sprintf(num,"%02x",pSong->playOrder[0]);
	m_patnumber.SetWindowText(num);
	m_rangestart.SetWindowText(num);
	sprintf(num,"%02x",pSong->playOrder[pSong->playLength-1]);
	m_rangeend.SetWindowText(num);
	
	int j=0;
	for (int i=0;i<pSong->playLength;i++)
	{
		j+=pSong->patternLines[pSong->playOrder[i]];
	}
	m_progress.SetRange(0,j);
	m_progress.SetPos(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSaveWavDlg::OnFilebrowse() 
{
	static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|All Files (*.*)|*.*||";
	CFileDialog dlg(false,"wav",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	if ( dlg.DoModal() == IDOK ) 
	{
		CString str = dlg.GetFileName();
		CString str2 = str.Right(4);
		if ( str2.CompareNoCase(".wav") != 0 ) str.Insert(str.GetLength(),".wav");
		m_filename.SetWindowText(str);
	}
}

void CSaveWavDlg::OnSelAllSong() 
{
	m_rangeend.EnableWindow(FALSE);
	m_rangestart.EnableWindow(FALSE);
	m_patnumber.EnableWindow(FALSE);
}

void CSaveWavDlg::OnSelRange() 
{
	m_rangeend.EnableWindow(FALSE);
	m_rangestart.EnableWindow(FALSE);
	m_patnumber.EnableWindow(TRUE);
}

void CSaveWavDlg::OnSelPattern() 
{
	m_rangeend.EnableWindow(TRUE);
	m_rangestart.EnableWindow(TRUE);
	m_patnumber.EnableWindow(FALSE);
}

void CSaveWavDlg::OnSavewave() 
{
	unsigned long tmp;
	unsigned long cont;

	m_savewave.EnableWindow(FALSE);
	Global::pConfig->_pOutputDriver->Enable(false);
	Global::pConfig->_pMidiInput->Close();
	
	bool autostop = Global::pConfig->autoStopMachines;
	if ( Global::pConfig->autoStopMachines )
	{
		Global::pConfig->autoStopMachines = false;
		for (int c=0; c<MAX_MACHINES; c++)
		{
			if (Global::_pSong->_machineActive[c])
			{
				Global::_pSong->_pMachines[c]->_stopped=false;
			}
		}
	}
	bool playblock = Global::pPlayer->_playBlock;
	bool sel[MAX_SONG_POSITIONS];
	memcpy(sel,Global::_pSong->playOrderSel,MAX_SONG_POSITIONS);
	memset(Global::_pSong->playOrderSel,0,MAX_SONG_POSITIONS);
	
	CString name;
	m_filename.GetWindowText(name);
	Global::pPlayer->StopRecording();
	Global::pPlayer->StartRecording(name.GetBuffer(4));
	int pstart;
	kill_thread = 0;

	switch( m_savesong.GetCheck())
	{
	case 1:
		Global::pPlayer->_playBlock=false;
		Global::pPlayer->Start(0,0);
		break;
	case 2:
		Global::pPlayer->_playBlock=false;
		m_patnumber.GetWindowText(name);
		pstart=_httoi(name.GetBuffer(2));
		Global::pPlayer->Start(pstart,0);
		break;
	case 3:
		Global::pPlayer->_playBlock=false;
		m_rangestart.GetWindowText(name);
		pstart=_httoi(name.GetBuffer(2));
		m_rangeend.GetWindowText(name);
		tmp=_httoi(name.GetBuffer(2));
		for (cont=0;cont<=tmp;cont++)
		{
			Global::_pSong->playOrderSel[cont]=true;
		}
		Global::pPlayer->Start(pstart,0);
		break;
	default:
		kill_thread=1;
		break;
	}
	thread_handle = (HANDLE) CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) RecordThread,(void *) &kill_thread,0,&tmp);
	
	if ( autostop ) Global::pConfig->autoStopMachines=true;
	Global::pPlayer->_playBlock=playblock;
	memcpy(Global::_pSong->playOrderSel,sel,MAX_SONG_POSITIONS);
	Global::pConfig->_pOutputDriver->Enable(true);
	Global::pConfig->_pMidiInput->Open();
}

DWORD WINAPI __stdcall RecordThread(void *b)
{
	Player* pPlayer = Global::pPlayer;
	float* float_buffer;
	int stream_size = 32767;
//	int stream_buffer[65535];
	while(!*(int*)b)
	{
		if (!pPlayer->_playing) // Right now this won't happen because it loops.
		{
			*(int*)b=1; return 0;
		}
		float_buffer = pPlayer->Work(pPlayer,stream_size);
//		AudioDriver::Quantize(float_buffer,stream_buffer,stream_size*2);
	}
	return 0;
}

void CSaveWavDlg::OnCancel() 
{
	kill_thread=1;
	

	// Probably here should be done some other things, like reenabling the sound
	// instead of doing it in the savewave() function.

	CDialog::OnCancel();
}

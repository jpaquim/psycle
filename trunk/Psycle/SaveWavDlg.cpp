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
	m_recmode = 0;
	//}}AFX_DATA_INIT
}


void CSaveWavDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveWavDlg)
	DDX_Control(pDX, IDCANCEL, m_cancel);
	DDX_Control(pDX, IDC_SAVEWAVE, m_savewave);
	DDX_Control(pDX, IDC_SAVEWIRESSEPARATED, m_savewires);
	DDX_Control(pDX, IDC_RANGESTART, m_rangestart);
	DDX_Control(pDX, IDC_RANGEEND, m_rangeend);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_PATNUMBER, m_patnumber);
	DDX_Control(pDX, IDC_FILENAME, m_filename);
	DDX_Radio(pDX, IDC_RECSONG, m_recmode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveWavDlg, CDialog)
	//{{AFX_MSG_MAP(CSaveWavDlg)
	ON_BN_CLICKED(IDC_FILEBROWSE, OnFilebrowse)
	ON_BN_CLICKED(IDC_RECSONG, OnSelAllSong)
	ON_BN_CLICKED(IDC_RECRANGE, OnSelRange)
	ON_BN_CLICKED(IDC_RECPATTERN, OnSelPattern)
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
	lastpostick=0;
	lastlinetick=0;
	saving=false;
	
	CString name = Global::pConfig->GetSongDir();
	name+="\\";
	name+=pSong->fileName;
	name = name.Left(name.GetLength()-4);
	name+=".wav";
	m_filename.SetWindowText(name);
	
	m_rangeend.EnableWindow(FALSE);
	m_rangestart.EnableWindow(FALSE);
	m_patnumber.EnableWindow(FALSE);
	
	char num[3];
	sprintf(num,"%02x",pSong->playOrder[0]);
	m_patnumber.SetWindowText(num);
	sprintf(num,"%02x",0);
	m_rangestart.SetWindowText(num);
	sprintf(num,"%02x",pSong->playLength-1);
	m_rangeend.SetWindowText(num);
	
	m_progress.SetRange(0,1);
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
	m_recmode=0;
}

void CSaveWavDlg::OnSelPattern() 
{
	m_rangeend.EnableWindow(FALSE);
	m_rangestart.EnableWindow(FALSE);
	m_patnumber.EnableWindow(TRUE);
	m_recmode=1;
}

void CSaveWavDlg::OnSelRange() 
{
	m_rangeend.EnableWindow(TRUE);
	m_rangestart.EnableWindow(TRUE);
	m_patnumber.EnableWindow(FALSE);
	m_recmode=2;
}

void CSaveWavDlg::OnSavewave() 
{
	Song *pSong = Global::_pSong;
	Player *pPlayer = Global::pPlayer;
	int tmp;
	int cont;

	m_savewave.EnableWindow(FALSE);
	m_cancel.SetWindowText("Stop");
	Global::pConfig->_pOutputDriver->Enable(false);
	Global::pConfig->_pMidiInput->Close();
	
	autostop = Global::pConfig->autoStopMachines;
	if ( Global::pConfig->autoStopMachines )
	{
		Global::pConfig->autoStopMachines = false;
		for (int c=0; c<MAX_MACHINES; c++)
		{
			if (pSong->_pMachine[c])
			{
				pSong->_pMachine[c]->_stopped=false;
			}
		}
	}
	playblock = pPlayer->_playBlock;
	memcpy(sel,pSong->playOrderSel,MAX_SONG_POSITIONS);
	memset(pSong->playOrderSel,0,MAX_SONG_POSITIONS);
	saving=true;
	
	CString name;
	pPlayer->StopRecording();
//	m_samprate.GetWindowText(name);
//	int val = _httoi(name.GetBuffer(5));
//	pPlayer->SetSampleRate(val);
//  int bitdepth = (m_bitdepth.GetCurSel()+1)*8;
	m_filename.GetWindowText(name);
	pPlayer->StartRecording(name.GetBuffer(4)/*,bitdepth*/);
	int pstart;
	kill_thread = 0;
	tickcont=0;
	lastlinetick=0;
	int i,j;
	
	switch (m_recmode)
	{
	case 0:
		j=0; // Calculate progress bar range.
		for (i=0;i<pSong->playLength;i++)
		{
			j+=pSong->patternLines[pSong->playOrder[i]];
		}
		m_progress.SetRange(0,j);
		
		pPlayer->_playBlock=false;
		lastpostick=0;
		pPlayer->Start(0,0);
		break;
	case 1:
		m_patnumber.GetWindowText(name);
		pstart=_httoi(name.GetBuffer(2));
		m_progress.SetRange(0,pSong->patternLines[pstart]);
		for (cont=0;cont<pSong->playLength;cont++)
		{
			if ( (int)pSong->playOrder[cont] == pstart)
			{
				pstart= cont;
				break;
			}
		}
		lastpostick=pstart;
		pSong->playOrderSel[cont]=true;
		pPlayer->Start(pstart,0);
		pPlayer->_playBlock=true;
		pPlayer->_loopSong=false;
		break;
	case 2:
		m_rangestart.GetWindowText(name);
		pstart=_httoi(name.GetBuffer(2));
		m_rangeend.GetWindowText(name);
		tmp=_httoi(name.GetBuffer(2));
		j=0;
		for (cont=pstart;cont<=tmp;cont++)
		{
			pSong->playOrderSel[cont]=true;
			j+=pSong->patternLines[pSong->playOrder[cont]];
		}
		m_progress.SetRange(0,j);

		lastpostick=pstart;
		pPlayer->Start(pstart,0);
		pPlayer->_playBlock=true;
		pPlayer->_loopSong=false;
		break;
	default:
		SaveEnd();
		return;
	}
	unsigned long tmp2;
	thread_handle = (HANDLE) CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) RecordThread,(void *) this,0,&tmp2);
	
}

DWORD WINAPI __stdcall RecordThread(void *b)
{
	Player* pPlayer = Global::pPlayer;
	int stream_size = 32767; // Player has just a single buffer of 65535 samples to allocate both channels
//	int stream_buffer[65535];
	while(!((CSaveWavDlg*)b)->kill_thread)
	{
		if (!pPlayer->_recording) // the player automatically closes the wav recording when looping.
		{
			pPlayer->Stop();
			((CSaveWavDlg*)b)->SaveEnd();
			ExitThread(0);
			return 0;
		}
		pPlayer->Work(pPlayer,stream_size);
		((CSaveWavDlg*)b)->SaveTick();
	}

	pPlayer->Stop();
	pPlayer->StopRecording();
	((CSaveWavDlg*)b)->SaveEnd();
	ExitThread(0);
	return 0;
}

void CSaveWavDlg::OnCancel() 
{
	if (saving)
	{
		kill_thread=1;
		while (saving) Sleep(20);
	}
	else CDialog::OnCancel();
}

void CSaveWavDlg::SaveEnd()
{
	saving=false;
	if ( autostop ) Global::pConfig->autoStopMachines=true;
	Global::pPlayer->_playBlock=playblock;
	memcpy(Global::_pSong->playOrderSel,sel,MAX_SONG_POSITIONS);
	Global::pConfig->_pOutputDriver->Enable(true);
	Global::pConfig->_pMidiInput->Open();
	m_progress.SetPos(0);
	m_savewave.EnableWindow(TRUE);
	m_cancel.SetWindowText("Close");
}

void CSaveWavDlg::SaveTick()
{
	Song* pSong = Global::_pSong;
	Player* pPlayer = Global::pPlayer;
	for (int i=lastpostick+1;i<pPlayer->_playPosition;i++)
	{
		tickcont+=pSong->patternLines[pSong->playOrder[i]];
	}
	if (lastpostick!= pPlayer->_playPosition ) 
	{
		tickcont+=pSong->patternLines[pSong->playOrder[lastpostick]]-(lastlinetick+1)+pPlayer->_lineCounter;
	}
	else tickcont+=pPlayer->_lineCounter-lastlinetick;

	lastlinetick = pPlayer->_lineCounter;
	lastpostick = pPlayer->_playPosition;

   if (!kill_thread ) m_progress.SetPos(tickcont);
}

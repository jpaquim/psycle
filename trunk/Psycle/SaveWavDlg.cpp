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
#include "MainFrm.h"

extern CPsycleApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DWORD WINAPI __stdcall RecordThread(void *b);

int CSaveWavDlg::channelmode = -1;
int CSaveWavDlg::rate = -1;
int CSaveWavDlg::bits = -1;
BOOL CSaveWavDlg::savewires = false;
BOOL CSaveWavDlg::savetracks = false;


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
	DDX_Control(pDX, IDC_SAVETRACKSSEPARATED, m_savetracks);
	DDX_Control(pDX, IDC_RANGESTART, m_rangestart);
	DDX_Control(pDX, IDC_RANGEEND, m_rangeend);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_PATNUMBER, m_patnumber);
	DDX_Control(pDX, IDC_FILENAME, m_filename);
	DDX_Control(pDX, IDC_COMBO_RATE, m_rate);
	DDX_Control(pDX, IDC_COMBO_BITS, m_bits);
	DDX_Control(pDX, IDC_COMBO_CHANNELS, m_channelmode);
	DDX_Control(pDX, IDC_TEXT, m_text);

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
	ON_CBN_SELCHANGE(IDC_COMBO_BITS, OnSelchangeComboBits)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNELS, OnSelchangeComboChannels)
	ON_CBN_SELCHANGE(IDC_COMBO_RATE, OnSelchangeComboRate)
	ON_BN_CLICKED(IDC_SAVETRACKSSEPARATED, OnSavetracksseparated)
	ON_BN_CLICKED(IDC_SAVEWIRESSEPARATED, OnSavewiresseparated)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveWavDlg message handlers

BOOL CSaveWavDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	threadopen = 0;
	Song* pSong = Global::_pSong;
	thread_handle=INVALID_HANDLE_VALUE;
	kill_thread=1;
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
	sprintf(num,"%02x",pSong->playOrder[((CMainFrame *)theApp.m_pMainWnd)->m_wndView.editPosition]);
	m_patnumber.SetWindowText(num);
	sprintf(num,"%02x",0);
	m_rangestart.SetWindowText(num);
	sprintf(num,"%02x",pSong->playLength-1);
	m_rangeend.SetWindowText(num);
	
	m_progress.SetRange(0,1);
	m_progress.SetPos(0);

	if ((rate < 0) || (rate >5))
	{
		if (Global::pConfig->_pOutputDriver->_samplesPerSec <= 8192)
		{
			rate = 0;
		}
		else if (Global::pConfig->_pOutputDriver->_samplesPerSec <= 11025)
		{
			rate = 1;
		}
		else if (Global::pConfig->_pOutputDriver->_samplesPerSec <= 22050)
		{
			rate = 2;
		}
		else if (Global::pConfig->_pOutputDriver->_samplesPerSec <= 44100)
		{
			rate = 3;
		}
		else if (Global::pConfig->_pOutputDriver->_samplesPerSec <= 48000)
		{
			rate = 4;
		}
		else 
		{
			rate = 5;
		}
	}

	m_rate.AddString("8192 hz");
	m_rate.AddString("11025 hz");
	m_rate.AddString("22050 hz");
	m_rate.AddString("44100 hz");
	m_rate.AddString("48000 hz");
	m_rate.AddString("96000 hz");
	m_rate.SetCurSel(rate);

	if ((bits < 0) || (bits > 3))
	{
		if (Global::pConfig->_pOutputDriver->_bitDepth <= 8)
		{
			bits = 0;
		}
		else if (Global::pConfig->_pOutputDriver->_bitDepth <= 16)
		{
			bits = 1;
		}
		else if (Global::pConfig->_pOutputDriver->_bitDepth <= 24)
		{
			bits = 2;
		}
		else if (Global::pConfig->_pOutputDriver->_bitDepth <= 32)
		{
			bits = 3;
		}
	}

	m_bits.AddString("8 bit");
	m_bits.AddString("16 bit");
	m_bits.AddString("24 bit");
	m_bits.AddString("32 bit");

	m_bits.SetCurSel(bits);

	m_channelmode.AddString("Mono (Mix)");
	m_channelmode.AddString("Mono (Left)");
	m_channelmode.AddString("Mono (Right)");
	m_channelmode.AddString("Stereo");

	if ((channelmode < 0) || (channelmode > 3))
	{
		channelmode = Global::pConfig->_pOutputDriver->_channelmode;
	}
	m_channelmode.SetCurSel(channelmode);

	m_savetracks.SetCheck(savetracks);
	m_savewires.SetCheck(savewires);

	m_text.SetWindowText("");
	
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
	
	CString name;
//	m_samprate.GetWindowText(name);
//	int val = _httoi(name.GetBuffer(5));
//	pPlayer->SetSampleRate(val);
//  int bitdepth = (m_bitdepth.GetCurSel()+1)*8;
	m_filename.GetWindowText(name);
	const int real_rate[]={8192,11025,22050,44100,48000,96000};
	const int real_bits[]={8,16,24,32};

	m_savetracks.EnableWindow(FALSE);
	m_savewires.EnableWindow(FALSE);

	if (m_savetracks.GetCheck())
	{
		memcpy(_Muted,pSong->_trackMuted,sizeof(pSong->_trackMuted));

		strcpy(rootname,name);
		rootname[strlen(rootname)-4] = 0;

		int count = 0;

		for (int i = 0; i < pSong->SONGTRACKS; i++)
		{
			if (!_Muted[i])
			{
				count++;
				current = i;
				for (int j = 0; j < pSong->SONGTRACKS; j++)
				{
					if (j != i)
					{
						pSong->_trackMuted[j] = TRUE;
					}
					else
					{
						pSong->_trackMuted[j] = FALSE;
					}
				}
				// now save the song
				char filename[MAX_PATH];
				sprintf(filename,"%s-track %.2u.wav",rootname,i);
				SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
				return;
			}
		}
		current = 256;
		SaveEnd();
	}
	else if (m_savewires.GetCheck())
	{
		// this is tricky - sort of
		// back up our connections first

		for (int i = 0; i < MAX_CONNECTIONS; i++)
		{
			if (pSong->_pMachine[MASTER_INDEX]->_inputCon[i])
			{
				_Muted[i] = pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[i]]->_mute;
			}
			else
			{
				_Muted[i] = TRUE;
			}
		}

		strcpy(rootname,name);
		rootname[strlen(rootname)-4] = 0;

		for (i = 0; i < MAX_CONNECTIONS; i++)
		{
			if (!_Muted[i])
			{
				current = i;
				for (int j = 0; j < MAX_CONNECTIONS; j++)
				{
					if (pSong->_pMachine[MASTER_INDEX]->_inputCon[j])
					{
						if (j != i)
						{
							pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[j]]->_mute = TRUE;
						}
						else
						{
							pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[j]]->_mute = FALSE;
						}
					}
				}
				// now save the song
				char filename[MAX_PATH];
				sprintf(filename,"%s-wire %.2u %s.wav",rootname,i,pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[i]]->_editName);
				SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
				return;
			}
		}
		current = 256;
		SaveEnd();
	}
	else
	{
		SaveWav(name.GetBuffer(4),real_bits[bits],real_rate[rate],channelmode);
	}
}

void CSaveWavDlg::SaveWav(char* file, int bits, int rate, int channelmode)
{
	saving=true;
	Player *pPlayer = Global::pPlayer;
	Song *pSong = Global::_pSong;
	pPlayer->StopRecording();

	char *q = strrchr(file,'\\')+1; // =
	if (!q)
	{
		m_text.SetWindowText(file);
	}
	else
	{
		m_text.SetWindowText(q);
	}

	pPlayer->StartRecording(file,bits,rate,channelmode);

	int tmp;
	int cont;
	CString name;

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

	/*
	int stream_size = 32767; // Player has just a single buffer of 65535 samples to allocate both channels
//	int stream_buffer[65535];
	while(!kill_thread)
	{
		if (!pPlayer->_recording) // the player automatically closes the wav recording when looping.
		{
			kill_thread = 1;
		}
		else
		{
			pPlayer->Work(pPlayer,stream_size);
			SaveTick();
		}
		Sleep(1);
	}

	pPlayer->Stop();
	pPlayer->StopRecording();
	SaveEnd();
	*/
}

DWORD WINAPI __stdcall RecordThread(void *b)
{
	((CSaveWavDlg*)b)->threadopen++;
	Player* pPlayer = Global::pPlayer;
	int stream_size = 4096; // Player has just a single buffer of 65535 samples to allocate both channels
//	int stream_buffer[65535];
	while(!((CSaveWavDlg*)b)->kill_thread)
	{
		if (!pPlayer->_recording) // the player automatically closes the wav recording when looping.
		{
			pPlayer->Stop();
			((CSaveWavDlg*)b)->SaveEnd();
			((CSaveWavDlg*)b)->threadopen--;
			ExitThread(0);
			return 0;
		}
		pPlayer->Work(pPlayer,stream_size);
		((CSaveWavDlg*)b)->SaveTick();
	}

	pPlayer->Stop();
	pPlayer->StopRecording();
	((CSaveWavDlg*)b)->SaveEnd();
	((CSaveWavDlg*)b)->threadopen--;
	ExitThread(0);
	return 0;
}

void CSaveWavDlg::OnCancel() 
{
	if (saving || (threadopen > 0))
	{
//		while (threadopen > 0) 
		{
			current = 256;
			kill_thread=1;
			Sleep(100);
		}
	}
	else if (threadopen <= 0)
	{
		CDialog::OnCancel();
	}
}

void CSaveWavDlg::SaveEnd()
{
	saving=false;
	kill_thread=1;
	if ( autostop ) 
	{
		Global::pConfig->autoStopMachines=true;
	}
	Global::pPlayer->_playBlock=playblock;
	memcpy(Global::_pSong->playOrderSel,sel,MAX_SONG_POSITIONS);
	Global::pConfig->_pOutputDriver->Enable(true);
	Global::pConfig->_pMidiInput->Open();

	if (m_savetracks.GetCheck())
	{
		Song *pSong = Global::_pSong;

		const int real_rate[]={8192,11025,22050,44100,48000,96000};
		const int real_bits[]={8,16,24,32};

		for (int i = current+1; i < pSong->SONGTRACKS; i++)
		{
			if (!_Muted[i])
			{
				current = i;
				for (int j = 0; j < pSong->SONGTRACKS; j++)
				{
					if (j != i)
					{
						pSong->_trackMuted[j] = TRUE;
					}
					else
					{
						pSong->_trackMuted[j] = FALSE;
					}
				}
				// now save the song
				char filename[MAX_PATH];
				sprintf(filename,"%s-track %.2u.wav",rootname,i);
				SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
				return;
			}
		}
		memcpy(pSong->_trackMuted,_Muted,sizeof(pSong->_trackMuted));
	}

	else if (m_savewires.GetCheck())
	{
		Song *pSong = Global::_pSong;

		const int real_rate[]={8192,11025,22050,44100,48000,96000};
		const int real_bits[]={8,16,24,32};

		for (int i = current+1; i < MAX_CONNECTIONS; i++)
		{
			if (!_Muted[i])
			{
				current = i;
				for (int j = 0; j < MAX_CONNECTIONS; j++)
				{
					if (pSong->_pMachine[MASTER_INDEX]->_inputCon[j])
					{
						if (j != i)
						{
							pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[j]]->_mute = TRUE;
						}
						else
						{
							pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[j]]->_mute = FALSE;
						}
					}
				}
				// now save the song
				char filename[MAX_PATH];
				sprintf(filename,"%s-wire %.2u %s.wav",rootname,i,pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[i]]->_editName);
				SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
				return;
			}
		}

		for (i = 0; i < MAX_CONNECTIONS; i++)
		{
			if (pSong->_pMachine[MASTER_INDEX]->_inputCon[i])
			{
				pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[i]]->_mute = _Muted[i];
			}
		}
	}

	m_text.SetWindowText("");

	m_savetracks.EnableWindow(TRUE);
	m_savewires.EnableWindow(TRUE);

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

   if (!kill_thread ) 
   {
	   m_progress.SetPos(tickcont);
   }
}

void CSaveWavDlg::OnSelchangeComboBits() 
{
	// TODO: Add your control notification handler code here
	bits = m_bits.GetCurSel();
	
}

void CSaveWavDlg::OnSelchangeComboChannels() 
{
	// TODO: Add your control notification handler code here
	channelmode = m_channelmode.GetCurSel();
	
}

void CSaveWavDlg::OnSelchangeComboRate() 
{
	// TODO: Add your control notification handler code here
	rate = m_rate.GetCurSel();
	
}

void CSaveWavDlg::OnSavetracksseparated() 
{
	// TODO: Add your control notification handler code here
	if (savetracks = m_savetracks.GetCheck())
	{
		m_savewires.SetCheck(FALSE);
		savewires = FALSE;
	}
}

void CSaveWavDlg::OnSavewiresseparated() 
{
	// TODO: Add your control notification handler code here
	if (savewires = m_savewires.GetCheck())
	{
		m_savetracks.SetCheck(FALSE);
		savetracks = FALSE;
	}
}

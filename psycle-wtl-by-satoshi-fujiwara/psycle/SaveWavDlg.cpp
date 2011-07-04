/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.5 $
 */

#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "SaveWavDlg.h"
#include "Global.h"
#include "Song.h"
#include "Instrument.h"
#include "configuration.h"
#include "MidiInput.h"
#include "Player.h"
#include "Machine.h"
#include "helpers.h"
#include "MainFrm.h"
#include ".\savewavdlg.h"

//extern CPsycleApp theApp;



DWORD WINAPI __stdcall RecordThread(void *b);

int CSaveWavDlg::channelmode = -1;
int CSaveWavDlg::rate = -1;
int CSaveWavDlg::bits = -1;
BOOL CSaveWavDlg::savewires = false;
BOOL CSaveWavDlg::savetracks = false;
BOOL CSaveWavDlg::savegens = false;


/////////////////////////////////////////////////////////////////////////////
// CSaveWavDlg dialog


CSaveWavDlg::CSaveWavDlg()
{
	m_recmode = 0;
}




void CSaveWavDlg::SaveWav(TCHAR* file, int bits, int rate, int channelmode)
{
	saving=true;
	Player *pPlayer = Global::pPlayer;
	Song *pSong = Global::_pSong;
	pPlayer->StopRecording();
	Global::pConfig->_pOutputDriver->Enable(false);
	Global::pConfig->_pMidiInput->Close();

	TCHAR *q = _tcschr(file,L'\\') + 1; // =
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
		for (i=0;i<pSong->PlayLength();i++)
		{
			j+=pSong->PatternLines(pSong->PlayOrder(i));
		}
		m_progress.SetRange(0,j);
		
		pPlayer->_playBlock=false;
		lastpostick=0;
		pPlayer->Start(0,0);
		break;
	case 1:
		m_patnumber.GetWindowText(name);
		pstart=_httoi(name.GetBuffer(2));
		m_progress.SetRange(0,pSong->PatternLines(pstart));
		for (cont=0;cont<pSong->PlayLength();cont++)
		{
			if ( (int)pSong->PlayOrder(cont) == pstart)
			{
				pstart= cont;
				break;
			}
		}
		lastpostick=pstart;
		pSong->PlayOrderSel(cont,true);
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
			pSong->PlayOrderSel(cont,true);
			j+=pSong->PatternLines(pSong->PlayOrder(cont));
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
	((CSaveWavDlg*)b)->threadopen++;
	Player* pPlayer = Global::pPlayer;
	int stream_size = 576; // Player has just a single buffer of 65535 samples to allocate both channels
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


void CSaveWavDlg::SaveEnd()
{
	saving=false;
	kill_thread=1;
	if ( autostop ) 
	{
		Global::pConfig->autoStopMachines=true;
	}
	Global::pPlayer->_playBlock=playblock;
	for(int i = 0;i < MAX_SONG_POSITIONS;i++){
		Global::_pSong->PlayOrderSel(i,*(sel + 1));
	}
	//memcpy(Global::_pSong->PlayOrderSel,sel,MAX_SONG_POSITIONS);
	Global::pConfig->_pOutputDriver->Enable(true);
	Global::pConfig->_pMidiInput->Open();

	if (m_savetracks.GetCheck())
	{
		Song *pSong = Global::_pSong;

		const int real_rate[]={8192,11025,22050,44100,48000,96000};
		const int real_bits[]={8,16,24,32};

		for (int i = current+1; i < pSong->SongTracks(); i++)
		{
			if (!_Muted[i])
			{
				current = i;
				for (int j = 0; j < pSong->SongTracks(); j++)
				{
					if (j != i)
					{
						pSong->IsTrackMuted(j,true);
					}
					else
					{
						pSong->IsTrackMuted(j,false);
					}
				}
				// now save the song
				TCHAR filename[MAX_PATH];
				_stprintf(filename,_T("%s-track %.2u.wav"),rootname,i);
				SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
				return;
			}
		}
		
		for(int i = 0;i < MAX_TRACKS;i++){
			pSong->IsTrackMuted(i,_Muted[i]);
		}
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
					if (pSong->pMachine(MASTER_INDEX)->_inputCon[j])
					{
						if (j != i)
						{
							pSong->pMachine(pSong->pMachine(MASTER_INDEX)->_inputMachines[j])->_mute = TRUE;
						}
						else
						{
							pSong->pMachine(pSong->pMachine(MASTER_INDEX)->_inputMachines[j])->_mute = FALSE;
						}
					}
				}
				// now save the song
				TCHAR filename[MAX_PATH];
				_stprintf(filename,_T("%s-wire %.2u %s.wav"),rootname,i,pSong->pMachine(pSong->pMachine(MASTER_INDEX)->_inputMachines[i])->_editName);
				SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
				return;
			}
		}

		for (i = 0; i < MAX_CONNECTIONS; i++)
		{
			if (pSong->pMachine(MASTER_INDEX)->_inputCon[i])
			{
				pSong->pMachine(pSong->pMachine(MASTER_INDEX)->_inputMachines[i])->_mute = _Muted[i];
			}
		}
	}

	else if (m_savegens.GetCheck())
	{
		Song *pSong = Global::_pSong;

		const int real_rate[]={8192,11025,22050,44100,48000,96000};
		const int real_bits[]={8,16,24,32};

		for (int i = current+1; i < MAX_BUSES; i++)
		{
			if (!_Muted[i])
			{
				current = i;
				for (int j = 0; j < MAX_BUSES; j++)
				{
					if (pSong->pMachine(j))
					{
						if (j != i)
						{
							pSong->pMachine(j)->_mute = TRUE;
						}
						else
						{
							pSong->pMachine(j)->_mute = FALSE;
						}
					}
				}
				// now save the song
				TCHAR filename[MAX_PATH];
				_stprintf(filename,_T("%s-generator %.2u %s.wav"),rootname,i,pSong->pMachine(i)->_editName);
				SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
				return;
			}
		}

		for (i = 0; i < MAX_BUSES; i++)
		{
			if (pSong->pMachine(i))
			{
				pSong->pMachine(i)->_mute = _Muted[i];
			}
		}
	}

	m_text.SetWindowText(_T(""));

	GetDlgItem(IDC_RECSONG).EnableWindow(TRUE);
	GetDlgItem(IDC_RECPATTERN).EnableWindow(TRUE);
	GetDlgItem(IDC_RECRANGE).EnableWindow(TRUE);

	GetDlgItem(IDC_FILEBROWSE).EnableWindow(TRUE);

	m_filename.EnableWindow(TRUE);
	m_savetracks.EnableWindow(TRUE);
	m_savegens.EnableWindow(TRUE);
	m_savewires.EnableWindow(TRUE);
	m_rate.EnableWindow(TRUE);
	m_bits.EnableWindow(TRUE);
	m_channelmode.EnableWindow(TRUE);

	switch (m_recmode)
	{
	case 0:
		m_rangeend.EnableWindow(FALSE);
		m_rangestart.EnableWindow(FALSE);
		m_patnumber.EnableWindow(FALSE);
		break;
	case 1:
		m_rangeend.EnableWindow(FALSE);
		m_rangestart.EnableWindow(FALSE);
		m_patnumber.EnableWindow(TRUE);
		break;
	case 2:
		m_rangeend.EnableWindow(TRUE);
		m_rangestart.EnableWindow(TRUE);
		m_patnumber.EnableWindow(FALSE);
		break;
	}

	m_progress.SetPos(0);
	m_savewave.EnableWindow(TRUE);
	m_cancel.SetWindowText(SF::CResourceString(IDS_MSG0092));
}

void CSaveWavDlg::SaveTick()
{
	Song* pSong = Global::_pSong;
	Player* pPlayer = Global::pPlayer;
	for (int i=lastpostick+1;i<pPlayer->_playPosition;i++)
	{
		tickcont+=pSong->PatternLines(pSong->PlayOrder(i));
	}
	if (lastpostick!= pPlayer->_playPosition ) 
	{
		tickcont+=pSong->PatternLines(pSong->PlayOrder(lastpostick))-(lastlinetick+1)+pPlayer->_lineCounter;
	}
	else tickcont+=pPlayer->_lineCounter-lastlinetick;

	lastlinetick = pPlayer->_lineCounter;
	lastpostick = pPlayer->_playPosition;

   if (!kill_thread ) 
   {
	   m_progress.SetPos(tickcont);
   }
}


LRESULT CSaveWavDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_cancel.Attach(GetDlgItem(IDCANCEL));
	m_savewave.Attach(GetDlgItem(IDC_SAVEWAVE));
	m_savewires.Attach(GetDlgItem(IDC_SAVEWIRESSEPARATED));
	m_savetracks.Attach(GetDlgItem(IDC_SAVETRACKSSEPARATED));
	m_savegens.Attach(GetDlgItem(IDC_SAVEGENERATORSEPARATED));
	m_rangestart.Attach(GetDlgItem(IDC_RANGESTART));
	m_rangeend.Attach(GetDlgItem(IDC_RANGEEND));
	m_progress.Attach(GetDlgItem(IDC_PROGRESS));
	m_patnumber.Attach(GetDlgItem(IDC_PATNUMBER));
	m_filename.Attach(GetDlgItem(IDC_FILENAME));
	m_rate.Attach(GetDlgItem(IDC_COMBO_RATE));
	m_bits.Attach(GetDlgItem(IDC_COMBO_BITS));
	m_channelmode.Attach(GetDlgItem(IDC_COMBO_CHANNELS));
	m_text.Attach(GetDlgItem(IDC_TEXT));
	m_radio_recmode.Attach(GetDlgItem(IDC_RECSONG));
	m_radio_recmode.SetCheck(m_recmode);

	threadopen = 0;
	Song* pSong = Global::_pSong;
	thread_handle=INVALID_HANDLE_VALUE;
	kill_thread=1;
	lastpostick=0;
	lastlinetick=0;
	saving=false;

	string name = Global::pConfig->GetSongDir();
	name += _T("\\");
	name += pSong->FileName();
	name = name.substr(0,name.length() - 4);
	name += _T(".wav");
	m_filename.SetWindowText(name.c_str());
	
	m_rangeend.EnableWindow(FALSE);
	m_rangestart.EnableWindow(FALSE);
	m_patnumber.EnableWindow(FALSE);
	
	TCHAR num[3];

	_stprintf(num,_T("%02x"),pSong->PlayOrder(CMainFrame::GetInstance().m_view.editPosition));
	m_patnumber.SetWindowText(num);
	_stprintf(num,_T("%02x"),0);
	m_rangestart.SetWindowText(num);
	_stprintf(num,_T("%02x"),pSong->PlayLength()-1);
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

	m_rate.AddString(_T("8192 Hz"));
	m_rate.AddString(_T("11025 Hz"));
	m_rate.AddString(_T("22050 Hz"));
	m_rate.AddString(_T("44100 Hz"));
	m_rate.AddString(_T("48000 Hz"));
	m_rate.AddString(_T("96000 Hz"));
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

	m_bits.AddString(_T("8 bit"));
	m_bits.AddString(_T("16 bit"));
	m_bits.AddString(_T("24 bit"));
	m_bits.AddString(_T("32 bit"));

	m_bits.SetCurSel(bits);

	m_channelmode.AddString(SF::CResourceString(IDS_MSG0055));
	m_channelmode.AddString(SF::CResourceString(IDS_MSG0056));
	m_channelmode.AddString(SF::CResourceString(IDS_MSG0057));
	m_channelmode.AddString(SF::CResourceString(IDS_MSG0058));

	if ((channelmode < 0) || (channelmode > 3))
	{
		channelmode = Global::pConfig->_pOutputDriver->_channelmode;
	}
	m_channelmode.SetCurSel(channelmode);

	m_savetracks.SetCheck(savetracks);
	m_savegens.SetCheck(savegens);
	m_savewires.SetCheck(savewires);

	m_text.SetWindowText(_T(""));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CSaveWavDlg::OnBnClickedFilebrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//static char BASED_CODE szFilter[] = _T("WAV Ì§²Ù (*.wav)|*.wav|‚·‚×‚Ä‚ÌÌ§²Ù (*.*)|*.*||");
	CFileDialog dlg(false,_T("wav"),NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,RES_STR(IDS_FILTER_WAVE));
	if ( dlg.DoModal() == IDOK ) 
	{
		CString str(dlg.m_szFileName);
		CString str2 = str.Right(4);
		if ( str2.CompareNoCase(_T(".wav")) != 0 ) 
		{
			str.Insert(str.GetLength(),_T(".wav"));
		}
		m_filename.SetWindowText(str);
	}
	return 0;
}

LRESULT CSaveWavDlg::OnBnClickedSavewiresseparated(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	if (savewires = m_savewires.GetCheck())
	{
		m_savetracks.SetCheck(FALSE);
		savetracks = FALSE;
		m_savegens.SetCheck(FALSE);
		savegens = FALSE;
	}
	return 0;
}

LRESULT CSaveWavDlg::OnBnClickedSavetracksseparated(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	if (savetracks = m_savetracks.GetCheck())
	{
		m_savewires.SetCheck(FALSE);
		savewires = FALSE;
		m_savegens.SetCheck(FALSE);
		savegens = FALSE;
	}
	return 0;
}

LRESULT CSaveWavDlg::OnBnClickedSavegeneratorseparated(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	if (savewires = m_savegens.GetCheck())
	{
		m_savetracks.SetCheck(FALSE);
		savetracks = FALSE;
		m_savewires.SetCheck(FALSE);
		savewires = FALSE;
	}
	return 0;
}

LRESULT CSaveWavDlg::OnBnClickedRecsong(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_rangeend.EnableWindow(FALSE);
	m_rangestart.EnableWindow(FALSE);
	m_patnumber.EnableWindow(FALSE);
	m_recmode = 0;
	return 0;
}

LRESULT CSaveWavDlg::OnBnClickedRecpattern(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_rangeend.EnableWindow(FALSE);
	m_rangestart.EnableWindow(FALSE);
	m_patnumber.EnableWindow(TRUE);
	m_recmode = 1;
	return 0;
}

LRESULT CSaveWavDlg::OnBnClickedRecrange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_rangeend.EnableWindow(TRUE);
	m_rangestart.EnableWindow(TRUE);
	m_patnumber.EnableWindow(FALSE);
	m_recmode = 3;
	return 0;
}

LRESULT CSaveWavDlg::OnCbnSelchangeComboRate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	rate = m_rate.GetCurSel();
	return 0;
}

LRESULT CSaveWavDlg::OnCbnSelchangeComboBits(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	bits = m_bits.GetCurSel();
	return 0;
}

LRESULT CSaveWavDlg::OnCbnSelchangeComboChannels(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	channelmode = m_channelmode.GetCurSel();
	return 0;
}

LRESULT CSaveWavDlg::OnBnClickedSavewave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//m_recmode = m_radio_recmode.GetState();
	Song *pSong = Global::_pSong;
	Player *pPlayer = Global::pPlayer;

	m_savewave.EnableWindow(FALSE);
	m_cancel.SetWindowText(SF::CResourceString(IDS_MSG0059));
	
	autostop = Global::pConfig->autoStopMachines;
	if ( Global::pConfig->autoStopMachines )
	{
		Global::pConfig->autoStopMachines = false;
		for (int c=0; c<MAX_MACHINES; c++)
		{
			if (pSong->pMachine(c))
			{
				pSong->pMachine(c)->_stopped=false;
			}
		}
	}
	playblock = pPlayer->_playBlock;
	for(int i = 0; i < MAX_SONG_POSITIONS;i++)
	{
		*(sel + i) = pSong->PlayOrderSel(i);
		pSong->PlayOrderSel(i,false);
	}
	//memcpy(sel,pSong->PlayOrderSel(),MAX_SONG_POSITIONS);
	//memset(pSong->PlayOrderSel(),0,MAX_SONG_POSITIONS);
	
	CString name;
	m_filename.GetWindowText(name);

	_tcscpy(rootname,name);
	rootname[_tcslen(rootname)-4] = 0;

	const int real_rate[]={8192,11025,22050,44100,48000,96000};
	const int real_bits[]={8,16,24,32};

	GetDlgItem(IDC_RECSONG).EnableWindow(FALSE);
	GetDlgItem(IDC_RECPATTERN).EnableWindow(FALSE);
	GetDlgItem(IDC_RECRANGE).EnableWindow(FALSE);
	GetDlgItem(IDC_FILEBROWSE).EnableWindow(FALSE);

	m_filename.EnableWindow(FALSE);
	m_savetracks.EnableWindow(FALSE);
	m_savegens.EnableWindow(FALSE);
	m_savewires.EnableWindow(FALSE);
	m_rate.EnableWindow(FALSE);
	m_bits.EnableWindow(FALSE);
	m_channelmode.EnableWindow(FALSE);

	m_rangeend.EnableWindow(FALSE);
	m_rangestart.EnableWindow(FALSE);
	m_patnumber.EnableWindow(FALSE);

	if (m_savetracks.GetCheck())
	{
		for(int i = 0;i < MAX_TRACKS;i++){
			_Muted[i] = pSong->IsTrackMuted(i);
		}
		//memcpy(_Muted,pSong->_trackMuted,sizeof(pSong->_trackMuted));

		int count = 0;

		for (int i = 0; i < pSong->SongTracks(); i++)
		{
			if (!_Muted[i])
			{
				count++;
				current = i;
				for (int j = 0; j < pSong->SongTracks(); j++)
				{
					if (j != i)
					{
						pSong->IsTrackMuted(j,true);
					}
					else
					{
						pSong->IsTrackMuted(j,false);
					}
				}
				// now save the song
				TCHAR filename[MAX_PATH];
				_stprintf(filename,_T("%s-track %.2u.wav"),rootname,i);
				SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
				return TRUE;
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
			if (pSong->pMachine(MASTER_INDEX)->_inputCon[i])
			{
				_Muted[i] = pSong->pMachine(pSong->pMachine(MASTER_INDEX)->_inputMachines[i])->_mute;
			}
			else
			{
				_Muted[i] = TRUE;
			}
		}

		for (i = 0; i < MAX_CONNECTIONS; i++)
		{
			if (!_Muted[i])
			{
				current = i;
				for (int j = 0; j < MAX_CONNECTIONS; j++)
				{
					if (pSong->pMachine(MASTER_INDEX)->_inputCon[j])
					{
						if (j != i)
						{
							pSong->pMachine(pSong->pMachine(MASTER_INDEX)->_inputMachines[j])->_mute = TRUE;
						}
						else
						{
							pSong->pMachine(pSong->pMachine(MASTER_INDEX)->_inputMachines[j])->_mute = FALSE;
						}
					}
				}
				// now save the song
				TCHAR filename[MAX_PATH];
				_stprintf(filename,_T("%s-wire %.2u %s.wav"),rootname,i,pSong->pMachine(pSong->pMachine(MASTER_INDEX)->_inputMachines[i])->_editName);
				SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
				return TRUE;
			}
		}
		current = 256;
		SaveEnd();
	}
	else if (m_savegens.GetCheck())
	{
		// this is tricky - sort of
		// back up our connections first

		for (int i = 0; i < MAX_BUSES; i++)
		{
			if (pSong->pMachine(i))
			{
				_Muted[i] = pSong->pMachine(i)->_mute;
			}
			else
			{
				_Muted[i] = TRUE;
			}
		}

		for (i = 0; i < MAX_BUSES; i++)
		{
			if (!_Muted[i])
			{
				current = i;
				for (int j = 0; j < MAX_BUSES; j++)
				{
					if (pSong->pMachine(j))
					{
						if (j != i)
						{
							pSong->pMachine(j)->_mute = TRUE;
						}
						else
						{
							pSong->pMachine(j)->_mute = FALSE;
						}
					}
				}
				// now save the song
				TCHAR filename[MAX_PATH];
				_stprintf(filename,_T("%s-generator %.2u %s.wav"),rootname,i,pSong->pMachine(i)->_editName);
				SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
				return TRUE;
			}
		}
		current = 256;
		SaveEnd();
	}
	else
	{
		SaveWav(name.GetBuffer(4),real_bits[bits],real_rate[rate],channelmode);
	}
	return 0;
}

LRESULT CSaveWavDlg::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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
		EndDialog(0);
	}
	return 0;
}

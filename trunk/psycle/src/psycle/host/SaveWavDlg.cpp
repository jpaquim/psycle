// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "SaveWavDlg.hpp"

#include "ChildView.hpp"
#include "Configuration.hpp"
#include "MainFrm.hpp"
#include "MidiInput.hpp"
#include "PatternView.hpp"

#include <psycle/core/machine.h>
#include <psycle/core/player.h>
#include <psycle/core/song.h>

#include <psycle/helpers/hexstring_to_integer.hpp>

#include <boost/bind.hpp>

#include <iomanip>
#include <iostream>

namespace psycle { namespace host {

BEGIN_MESSAGE_MAP(CSaveWavDlg, CDialog)
	ON_BN_CLICKED(IDC_FILEBROWSE, OnFilebrowse)
	ON_BN_CLICKED(IDC_RECSONG, OnSelAllSong)
	ON_BN_CLICKED(IDC_RECRANGE, OnSelRange)
	ON_BN_CLICKED(IDC_RECPATTERN, OnSelPattern)
	ON_BN_CLICKED(IDC_RECBLOCK, OnRecblock)
	ON_BN_CLICKED(IDC_SAVEWAVE, OnSavewave)
	ON_CBN_SELCHANGE(IDC_COMBO_BITS, OnSelchangeComboBits)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNELS, OnSelchangeComboChannels)
	ON_CBN_SELCHANGE(IDC_COMBO_RATE, OnSelchangeComboRate)
	ON_CBN_SELCHANGE(IDC_COMBO_PDF, OnSelchangeComboPdf)
	ON_CBN_SELCHANGE(IDC_COMBO_NOISESHAPING, OnSelchangeComboNoiseShaping)
	ON_BN_CLICKED(IDC_SAVETRACKSSEPARATED, OnSavetracksseparated)
	ON_BN_CLICKED(IDC_SAVEWIRESSEPARATED, OnSavewiresseparated)
	ON_BN_CLICKED(IDC_SAVEGENERATORSEPARATED, OnSavegensseparated)
	ON_BN_CLICKED(IDC_CHECK_DITHER,	OnToggleDither)
	ON_BN_CLICKED(IDC_OUTPUTFILE, OnOutputfile)
	ON_BN_CLICKED(IDC_OUTPUTCLIPBOARD, OnOutputclipboard)
	ON_BN_CLICKED(IDC_OUTPUTSAMPLE, OnOutputsample)			
END_MESSAGE_MAP()


const int real_rate[]={8000,11025,16000,22050,32000,44100,48000,88200,96000};
const int real_bits[]={8,16,24,32,32};

CSaveWavDlg::CSaveWavDlg(CChildView* pChildView, CSelection* pBlockSel, CWnd* pParent) 
	: CDialog(CSaveWavDlg::IDD, pParent),
	  m_recmode(0),
	  m_outputtype(0) {
	this->pChildView = pChildView;
	this->pBlockSel = pBlockSel;					
	seq_tmp_play_line_ = new psycle::core::SequenceLine();
}

void CSaveWavDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILEBROWSE, browse_btn_);
	DDX_Control(pDX, IDCANCEL, cancel_btn_);
	DDX_Control(pDX, IDC_SAVEWAVE, savewave_btn_);
	DDX_Control(pDX, IDC_SAVEWIRESSEPARATED, savewires_btn_);
	DDX_Control(pDX, IDC_SAVETRACKSSEPARATED, savetracks_btn_);
	DDX_Control(pDX, IDC_SAVEGENERATORSEPARATED, savegens_btn_);
	DDX_Control(pDX, IDC_RANGESTART, m_rangestart);
	DDX_Control(pDX, IDC_RANGEEND, m_rangeend);
	DDX_Control(pDX, IDC_LINESTART, m_linestart);
	DDX_Control(pDX, IDC_LINEEND, m_lineend);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_PATNUMBER, m_patnumber);
	DDX_Control(pDX, IDC_PATNUMBER2, m_patnumber2);
	DDX_Control(pDX, IDC_FILENAME, m_filename);
	DDX_Control(pDX, IDC_COMBO_RATE, m_rate);
	DDX_Control(pDX, IDC_COMBO_BITS, m_bits);
	DDX_Control(pDX, IDC_COMBO_CHANNELS, m_channelmode);
	DDX_Control(pDX, IDC_TEXT, m_text);
	DDX_Control(pDX, IDC_CHECK_DITHER, dither_btn_);
	DDX_Control(pDX, IDC_COMBO_PDF, m_pdf);
	DDX_Control(pDX, IDC_COMBO_NOISESHAPING, m_noiseshaping);
	DDX_Radio(pDX, IDC_RECSONG, m_recmode);
	DDX_Radio(pDX, IDC_OUTPUTFILE, m_outputtype);
}

BOOL CSaveWavDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	Song& pSong = *pChildView->pattern_view()->song();
	std::string name = Global::pConfig->GetCurrentWaveRecDir().c_str();
	name+='\\';
	name+=pSong.filename();
	name = name.substr(0,std::max(std::string::size_type(0),name.length()-4));
	name+=".wav";
	m_filename.SetWindowText(name.c_str());
	
	m_rangeend.EnableWindow(false);
	m_rangestart.EnableWindow(false);
	m_patnumber.EnableWindow(false);
	
	m_lineend.EnableWindow(false);
	m_linestart.EnableWindow(false);
	m_patnumber2.EnableWindow(false);

	char num[10];
	int pattern_id = pChildView->pattern_view()->main()->m_wndSeq.selected_entry()->pattern().id();
	sprintf(num,"%02x",pattern_id);
	m_patnumber.SetWindowText(num);
	sprintf(num,"%02x",0);
	m_rangestart.SetWindowText(num);
	int len = static_cast<int>(pChildView->pattern_view()->main()->m_wndSeq.pos_map().size());
	sprintf(num,"%02x",len-1);
	m_rangeend.SetWindowText(num);			

	sprintf(num,"%02x",pattern_id);
	m_patnumber2.SetWindowText(num);

	if (pChildView->pattern_view()->blockSelected) {
		sprintf(num,"%02x",pBlockSel->start.line);
		m_linestart.SetWindowText(num);
		sprintf(num,"%02x",pBlockSel->end.line+1);
		m_lineend.SetWindowText(num);
	} else {
		sprintf(num,"%02x",0);
		m_linestart.SetWindowText(num);
		sprintf(num,"%02x",1);
		m_lineend.SetWindowText(num);
	}

	m_progress.SetRange(0,1);
	m_progress.SetPos(0);

	int rate;
	if (Global::pConfig->GetSamplesPerSec() <= 8000) {
		rate = 0;
	} else if (Global::pConfig->GetSamplesPerSec() <= 11025) {
		rate = 1;
	} else if (Global::pConfig->GetSamplesPerSec() <= 16000) {
		rate = 2;
	} else if (Global::pConfig->GetSamplesPerSec() <= 22050) {
		rate = 3;
	} else if (Global::pConfig->GetSamplesPerSec() <= 32000) {
		rate = 4;
	} else if (Global::pConfig->GetSamplesPerSec() <= 44100) {
		rate = 5;
	} else if (Global::pConfig->GetSamplesPerSec() <= 48000) {
		rate = 6;
	} else if (Global::pConfig->GetSamplesPerSec() <= 88200) {
		rate = 7;
	} else  {
		rate = 8;
	}
	m_rate.AddString("8000 hz");
	m_rate.AddString("11025 hz");
	m_rate.AddString("16000 hz");
	m_rate.AddString("22050 hz");
	m_rate.AddString("32000 hz");
	m_rate.AddString("44100 hz");
	m_rate.AddString("48000 hz");
	m_rate.AddString("88200 hz");
	m_rate.AddString("96000 hz");
	m_rate.SetCurSel(rate);

	int bits=2;
	if (Global::pConfig->GetBitDepth() <= 8) {
		bits = 0;
	} else if (Global::pConfig->GetBitDepth() <= 16) {
		bits = 1;
	} else if (Global::pConfig->GetBitDepth() <= 24) {
		bits = 2;
	}  else if (Global::pConfig->GetBitDepth() <= 32) {
		bits = 4;
	}			
	m_bits.AddString("8 bit");
	m_bits.AddString("16 bit");
	m_bits.AddString("24 bit");
	m_bits.AddString("32 bit (int)");
	m_bits.AddString("32 bit (float)");
	m_bits.SetCurSel(bits);

	m_channelmode.AddString("Mono (Mix)");
	m_channelmode.AddString("Mono (Left)");
	m_channelmode.AddString("Mono (Right)");
	m_channelmode.AddString("Stereo");
	m_channelmode.SetCurSel(Global::pConfig->_pOutputDriver->playbackSettings().channelMode());

	dither_btn_.SetCheck(BST_CHECKED);
	m_pdf.AddString("Triangular");
	m_pdf.AddString("Rectangular");
	m_pdf.AddString("Gaussian");			
	m_pdf.SetCurSel(0);
	m_noiseshaping.AddString("None");
	m_noiseshaping.AddString("High-Pass Contour");
	m_noiseshaping.SetCurSel(0);

	if (bits == 3 ) {
		dither_btn_.EnableWindow(false);
		m_pdf.EnableWindow(false);
		m_noiseshaping.EnableWindow(false);
	}
	psycle::audiodrivers::AudioDriverSettings settings = file_out_.playbackSettings();
	settings.setSamplesPerSec(real_rate[rate]);
	settings.setBitDepth(real_bits[bits]);
	settings.setChannelMode(Global::pConfig->_pOutputDriver->playbackSettings().channelMode());
	file_out_.setPlaybackSettings(settings);
	savetracks_btn_.SetCheck(false);
	savegens_btn_.SetCheck(false);
	savewires_btn_.SetCheck(false);
	m_text.SetWindowText("");		

	return true;  // return true unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return false
}

void CSaveWavDlg::ActivateOutputs(bool on) {
	savewave_btn_.EnableWindow(on);
	savewires_btn_.EnableWindow(on);
	savetracks_btn_.EnableWindow(on);
	savegens_btn_.EnableWindow(on);
	m_filename.EnableWindow(on);
	browse_btn_.EnableWindow(on);
}

void CSaveWavDlg::OnOutputfile() {			
	m_outputtype=0;
	ActivateOutputs(true);
}

void CSaveWavDlg::OnOutputclipboard() {
	m_outputtype=1;
	ActivateOutputs(false);
}

void CSaveWavDlg::OnOutputsample() {
	m_outputtype=2;
	ActivateOutputs(false);
}

void CSaveWavDlg::OnFilebrowse()  {
	static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|All Files (*.*)|*.*||";
	CString direct;
	m_filename.GetWindowText(direct);
	CFileDialog dlg(false,"wav",direct,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	if ( dlg.DoModal() == IDOK ) 
	{
		CString str = dlg.GetPathName();
		CString str2 = str.Right(4);
		if ( str2.CompareNoCase(".wav") != 0 ) 
		{
			str.Insert(str.GetLength(),".wav");
		}
		m_filename.SetWindowText(str);
		std::string::size_type index = str.ReverseFind('\\');
		if (index != std::string::npos)
		{
			str.Truncate(index);
			Global::pConfig->SetCurrentWaveRecDir(str.GetString());
		}

	}
}

void CSaveWavDlg::OnSelAllSong()  {
	m_rangeend.EnableWindow(false);
	m_rangestart.EnableWindow(false);
	m_patnumber.EnableWindow(false);
	m_lineend.EnableWindow(false);
	m_linestart.EnableWindow(false);
	m_patnumber2.EnableWindow(false);
	m_recmode = 0;
}

void CSaveWavDlg::OnSelPattern() {
	m_rangeend.EnableWindow(false);
	m_rangestart.EnableWindow(false);
	m_patnumber.EnableWindow(true);
	m_lineend.EnableWindow(false);
	m_linestart.EnableWindow(false);
	m_patnumber2.EnableWindow(false);
	m_recmode = 1;
}

void CSaveWavDlg::OnSelRange() {
	m_rangeend.EnableWindow(true);
	m_rangestart.EnableWindow(true);
	m_patnumber.EnableWindow(false);
	m_lineend.EnableWindow(false);
	m_linestart.EnableWindow(false);
	m_patnumber2.EnableWindow(false);
	m_recmode = 2;
}

void CSaveWavDlg::OnRecblock() {
	m_rangeend.EnableWindow(false);
	m_rangestart.EnableWindow(false);
	m_patnumber.EnableWindow(false);
	m_lineend.EnableWindow(true);
	m_linestart.EnableWindow(true);
	m_patnumber2.EnableWindow(true);
	m_recmode = 3;
}

void CSaveWavDlg::SwitchToTmpPlay() {
	Player* player = &Player::singleton();
	CoreSong& song = player->song();
	psycle::core::Sequence& seq = song.sequence();
	seq_main_play_line_ = *(seq.begin()+1);
	*(seq.begin()+1) = seq_tmp_play_line_;
	seq_tmp_play_line_->setSequence(song.sequence());
}

void CSaveWavDlg::SwitchToNormalPlay() {
	Player* player = &Player::singleton();
	CoreSong& song = player->song();
	psycle::core::Sequence& seq = song.sequence();
	*(seq.begin()+1) = seq_main_play_line_;
}

void CSaveWavDlg::OnSavewave() {
	// set gui to recmode
	GetDlgItem(IDC_RECSONG)->EnableWindow(false);
	GetDlgItem(IDC_RECPATTERN)->EnableWindow(false);
	GetDlgItem(IDC_RECRANGE)->EnableWindow(false);
	GetDlgItem(IDC_FILEBROWSE)->EnableWindow(false);
	savewave_btn_.EnableWindow(false);
	cancel_btn_.SetWindowText("Stop");
	m_filename.EnableWindow(false);
	savetracks_btn_.EnableWindow(false);
	savegens_btn_.EnableWindow(false);
	savewires_btn_.EnableWindow(false);
	m_rate.EnableWindow(false);
	m_bits.EnableWindow(false);
	m_channelmode.EnableWindow(false);
	m_pdf.EnableWindow(false);
	m_noiseshaping.EnableWindow(false);
	dither_btn_.EnableWindow(false);
	m_rangeend.EnableWindow(false);
	m_rangestart.EnableWindow(false);
	m_patnumber.EnableWindow(false);
	Player::singleton().stop();			
	Player::singleton().driver().set_started(false);
	Sleep(1000);	
	// create a thread for writing the audio data
	kill_thread_ = 0;
	if (savewires_btn_.GetCheck()) {
		thread_ = new thread(boost::bind(&CSaveWavDlg::SaveWires, this));
	} else 
	if (savegens_btn_.GetCheck()) {
		thread_ = new thread(boost::bind(&CSaveWavDlg::SaveGenerators, this));
	} else
	if (this->savetracks_btn_.GetCheck()) {
		thread_ = new thread(boost::bind(&CSaveWavDlg::SaveTracks, this));
	} else {
		thread_ = new thread(boost::bind(&CSaveWavDlg::SaveNormal, this));
	}
}

void CSaveWavDlg::SaveFile() {
	Player* player = &Player::singleton();
	CoreSong& song = player->song();
	// prepare recmodes
	if (m_recmode == 0) {
		// record entire song
		Player::singleton().start(0);
		m_progress.SetRange(0, Player::singleton().song().sequence().max_beats());
	} else
	if (m_recmode == 1) {
		CString name;
		int pstart;
		m_patnumber.GetWindowText(name);
		hexstring_to_integer(name.GetBuffer(2), pstart);
		SwitchToTmpPlay();
		seq_tmp_play_line_->clear();
		psycle::core::Pattern * pattern = song.sequence().FindPattern(pstart);
		assert(pattern);
		SequenceEntry & entry = *new SequenceEntry(*seq_tmp_play_line_, *pattern);
		seq_tmp_play_line_->insert(0, entry);	
		Player::singleton().start(0);
		m_progress.SetRange(0, static_cast<short>(pattern->beats()));
	} else
	if (m_recmode == 2) {
		CString name;
		int seq_start, seq_end;
		m_rangestart.GetWindowText(name);
		hexstring_to_integer(name.GetBuffer(2), seq_start);
		m_rangeend.GetWindowText(name);
		hexstring_to_integer(name.GetBuffer(2), seq_end);
		CMainFrame * mainFrame = pChildView->pattern_view()->main();
		SequenceEntry* start_entry = mainFrame->m_wndSeq.GetEntry(seq_start);
		seq_end_entry_ = mainFrame->m_wndSeq.GetEntry(seq_end);
		Player::singleton().start(start_entry->tickPosition());
		m_progress.SetRange(static_cast<short>(start_entry->tickPosition()),
						    static_cast<short>(start_entry->tickEndPosition()));
	}
	file_out_.set_opened(true);
	const int frames = 256;
	while(!kill_thread_ && player->playing()) {
		if(
			m_recmode == 2 &&
			player->playPos() >= seq_end_entry_->tickPosition() + seq_end_entry_->pattern().beats()
		) break;
		file_out_.Write(player->Work(frames), frames);
		m_progress.SetPos(static_cast<short>(player->playPos()));
	}
	file_out_.set_opened(false);
	Player::singleton().stop();
	if(m_recmode == 1) SwitchToNormalPlay();
}

void CSaveWavDlg::SaveWires() {			
	CoreSong& song = Player::singleton().song();
	// back up our connections first
	for (int i = 0; i < MAX_CONNECTIONS; ++i) {
		if (song.machine(MASTER_INDEX)->_inputCon[i]) {
			muted_[i] = song.machine(song.machine(MASTER_INDEX)->_inputMachines[i])->_mute;
		} else {
			muted_[i] = true;
		}
	}
	CString name;
	m_filename.GetWindowText(name); 
	std::string rootname = name;
	rootname = rootname.substr(0, std::max(std::string::size_type(0),rootname.length()-4));
	// save			
	for (int i = 0; (i < MAX_CONNECTIONS) && !kill_thread_; ++i) {
		if (!muted_[i]) {
			for (int j = 0; j < MAX_CONNECTIONS; j++) {
				if (song.machine(MASTER_INDEX)->_inputCon[j]) {							
					song.machine(song.machine(MASTER_INDEX)->_inputMachines[j])->_mute = (j != i);
				}
			}
			// now save the song
			char filename[MAX_PATH];
			sprintf(filename,"%s-wire %.2u %s.wav",rootname.c_str(),i,
				song.machine(song.machine(MASTER_INDEX)->_inputMachines[i])->GetEditName().c_str());
			psycle::audiodrivers::AudioDriverSettings settings = file_out_.playbackSettings();
			settings.setDeviceName(filename);
			file_out_.setPlaybackSettings(settings);
			SaveFile();
		}
	}
	// restore our connections
	for (int i = 0; i < MAX_CONNECTIONS; ++i) {
		if (song.machine(MASTER_INDEX)->_inputCon[i]) {
			song.machine(song.machine(MASTER_INDEX)->_inputMachines[i])->_mute = muted_[i];
		}
	}
	SaveEnd();
}

void CSaveWavDlg::SaveGenerators() {
	CoreSong& song = Player::singleton().song();
	// back up mute state of machines
	for (int i = 0; i < MAX_BUSES; ++i) {
		if (song.machine(i)) {
			muted_[i] = song.machine(i)->_mute;
		} else {
			muted_[i] = true;
		}
	}
	CString name;
	m_filename.GetWindowText(name); 
	std::string rootname = name;
	rootname = rootname.substr(0, std::max(std::string::size_type(0),rootname.length()-4));
	// save			
	for (int i = 0; (i < MAX_BUSES) && !kill_thread_; ++i) {
		if (!muted_[i]) {
			for (int j = 0; j < MAX_BUSES; j++) {
				if (song.machine(j)) {							
					song.machine(j)->_mute = (j != i);
				}
			}
			// now save the song
			char filename[MAX_PATH];
			sprintf(filename,"%s-generator %.2u %s.wav",rootname.c_str(),i,
				song.machine(i)->GetEditName().c_str());
			psycle::audiodrivers::AudioDriverSettings settings = file_out_.playbackSettings();
			settings.setDeviceName(filename);
			file_out_.setPlaybackSettings(settings);
			SaveFile();
		}
	}
	// restore the mute status of the machines
	for (int i = 0; i < MAX_BUSES; ++i) {
		if (song.machine(i)) {
			song.machine(i)->_mute = muted_[i];
		}
	}
	SaveEnd();				
}

void CSaveWavDlg::SaveTracks() {
	// backup track mute status
	CoreSong& song = Player::singleton().song();
	std::vector<bool> backup_tracks = song.sequence().muted_tracks();
	// build rootname
	CString name;
	m_filename.GetWindowText(name); 
	std::string rootname = name;
	rootname = rootname.substr(0, std::max(std::string::size_type(0),rootname.length()-4));
	// save			
	for (int i = 0; i < song.sequence().numTracks(); ++i) {
		if (!backup_tracks[i]) {					
			for (int j = 0; j < song.sequence().numTracks(); ++j) {
				song.sequence().setMutedTrack(j, j != i);
			}
			// now save the song
			std::ostringstream filename;
			filename << rootname;
			filename << "-track "
						<< std::setprecision(2) << (unsigned)i << " .wav";
			psycle::audiodrivers::AudioDriverSettings settings = file_out_.playbackSettings();
			settings.setDeviceName(filename.str());
			file_out_.setPlaybackSettings(settings);
			SaveFile();
		}
	}
	// restore track mute status			
	song.sequence().muted_tracks() = backup_tracks;
	SaveEnd();
}

void CSaveWavDlg::SaveNormal() {
	CString name;
	m_filename.GetWindowText(name);
	std::string file_name = name;
	psycle::audiodrivers::AudioDriverSettings settings = file_out_.playbackSettings();
	settings.setDeviceName(file_name);
	file_out_.setPlaybackSettings(settings);						
	SaveFile();
	SaveEnd();
}



/*
			else if (m_outputtype == 1 || m_outputtype == 2)
	{
		// Clear clipboardmem if needed (should not. it's a safety measure)
		if ( clipboardmem.size() > 0)
		{
			for (unsigned int i=0;i<clipboardmem.size();i++)
			{
				delete[] clipboardmem[i];
			}
			clipboardmem.clear();
		}
		//allocate first vector value to store the size of the clipboard memory.
		char *size = new char[4];
		memset(size,0,4);
		clipboardmem.push_back(size);
		// No name -> record to clipboard.
		SaveWav("",real_bits[bits],real_rate[rate],channelmode,isFloat);
	}
}
*/

void CSaveWavDlg::OnCancel() {
	kill_thread_ = 1;
	Sleep(100);
	delete seq_tmp_play_line_;			
	CDialog::OnCancel();
}

void CSaveWavDlg::SaveEnd()
{
	kill_thread_ = 1;
	delete thread_;
/*
	if (autostop) {
		Global::pConfig->autoStopMachines=true;
	}

	if (m_outputtype == 1) {
		SaveToClipboard();
	}

	else if ( m_outputtype == 2)
	{
		// todo : copy clipboardmem to the current selected instrument.
		#if 0
			int length = *reinterpret_cast<int*>(clipboardmem[0]);
			int copiedsize=0;
			int i=1;
			while (copiedsize+1000000<=length)
			{
				CopyMemory(pClipboardData +copiedsize,clipboardmem[i], 1000000);
				i++;
				copiedsize+=1000000;
			}
			CopyMemory(pClipboardData +copiedsize,clipboardmem[i], length-copiedsize);
		#endif
		for (unsigned int i=0;i<clipboardmem.size();i++)
		{
			delete[] clipboardmem[i];
		}
		clipboardmem.clear();

*/
	m_text.SetWindowText("");

	GetDlgItem(IDC_RECSONG)->EnableWindow(true);
	GetDlgItem(IDC_RECPATTERN)->EnableWindow(true);
	GetDlgItem(IDC_RECRANGE)->EnableWindow(true);
	GetDlgItem(IDC_RECBLOCK)->EnableWindow(true);
	GetDlgItem(IDC_FILEBROWSE)->EnableWindow(true);

	m_filename.EnableWindow(true);
	savetracks_btn_.EnableWindow(true);
	savegens_btn_.EnableWindow(true);
	savewires_btn_.EnableWindow(true);
	m_rate.EnableWindow(true);
	m_bits.EnableWindow(true);
	m_channelmode.EnableWindow(true);
	m_pdf.EnableWindow(dither_btn_.GetCheck());
	m_noiseshaping.EnableWindow(dither_btn_.GetCheck());
	dither_btn_.EnableWindow(true);
	switch (m_recmode) {
	case 0:
		m_rangeend.EnableWindow(false);
		m_rangestart.EnableWindow(false);
		m_patnumber.EnableWindow(false);
		break;
	case 1:
		m_rangeend.EnableWindow(false);
		m_rangestart.EnableWindow(false);
		m_patnumber.EnableWindow(true);
		break;
	case 2:
		m_rangeend.EnableWindow(true);
		m_rangestart.EnableWindow(true);
		m_patnumber.EnableWindow(false);
		break;
	}
	m_progress.SetPos(0);
	savewave_btn_.EnableWindow(true);
	cancel_btn_.SetWindowText("Close");
}

void CSaveWavDlg::SaveToClipboard() {
	OpenClipboard();
	EmptyClipboard();
	///\todo: Investigate why i can't paste to audacity (psycle's fault?)
	clipboardwavheader.head = 'FFIR';
	clipboardwavheader.head2= 'EVAW';
	clipboardwavheader.fmthead = ' tmf';
	clipboardwavheader.fmtsize = sizeof(WAVEFORMATEX) + 2; // !!!!!!!!!!!!!!!!????????? - works...
	clipboardwavheader.fmtcontent.wFormatTag = WAVE_FORMAT_PCM;
	clipboardwavheader.fmtcontent.nChannels = (m_channelmode.GetCurSel() == 3) ? 2 : 1;
	clipboardwavheader.fmtcontent.nSamplesPerSec = real_rate[m_rate.GetCurSel()];
	clipboardwavheader.fmtcontent.wBitsPerSample = real_bits[m_bits.GetCurSel()];
	clipboardwavheader.fmtcontent.nBlockAlign = clipboardwavheader.fmtcontent.wBitsPerSample/8*clipboardwavheader.fmtcontent.nChannels;
	clipboardwavheader.fmtcontent.nAvgBytesPerSec =clipboardwavheader.fmtcontent.nBlockAlign*clipboardwavheader.fmtcontent.nSamplesPerSec;
	clipboardwavheader.fmtcontent.cbSize = 0;
	clipboardwavheader.datahead = 'atad';

	int length = *reinterpret_cast<int*>(clipboardmem[0]);

	clipboardwavheader.datasize = length;
	clipboardwavheader.size = clipboardwavheader.datasize + sizeof(fullheader) - 8;


	HGLOBAL hClipboardData = GlobalAlloc(GMEM_MOVEABLE, clipboardwavheader.datasize + sizeof(fullheader));
	char*	pClipboardData = (char*) GlobalLock(hClipboardData);

	CopyMemory(pClipboardData, &clipboardwavheader, sizeof(fullheader) );

	// In bytes
	int copiedsize=0;
	int i=1;
	pClipboardData += sizeof(fullheader);
	while (copiedsize+1000000<=length)
	{
		CopyMemory(pClipboardData +copiedsize,clipboardmem[i], 1000000);
		i++;
		copiedsize+=1000000;
	}
	CopyMemory(pClipboardData +copiedsize,clipboardmem[i], length-copiedsize);

	for (unsigned int i=0;i<clipboardmem.size();i++)
	{
		delete[] clipboardmem[i];
	}
	clipboardmem.clear();

	GlobalUnlock(hClipboardData);
	SetClipboardData(CF_WAVE, hClipboardData);
	CloseClipboard();

}

void CSaveWavDlg::OnSelchangeComboBits() {
	if (m_bits.GetCurSel() == 3) {
		dither_btn_.EnableWindow(false);
		m_pdf.EnableWindow(false);
		m_noiseshaping.EnableWindow(false);
	} else {
		dither_btn_.EnableWindow(true);
		m_pdf.EnableWindow(true);
		m_noiseshaping.EnableWindow(true);
	}
	psycle::audiodrivers::AudioDriverSettings settings = file_out_.playbackSettings();
	settings.setBitDepth(real_bits[m_bits.GetCurSel()]);
	file_out_.setPlaybackSettings(settings);
}

void CSaveWavDlg::OnSelchangeComboChannels() {
	psycle::audiodrivers::AudioDriverSettings settings = file_out_.playbackSettings();
	settings.setChannelMode(m_channelmode.GetCurSel());
	file_out_.setPlaybackSettings(settings);
}

void CSaveWavDlg::OnSelchangeComboRate() {
	psycle::audiodrivers::AudioDriverSettings settings = file_out_.playbackSettings();
	settings.setSamplesPerSec(real_rate[m_rate.GetCurSel()]);
	file_out_.setPlaybackSettings(settings);
}

void CSaveWavDlg::OnSelchangeComboPdf() {			
	switch (m_pdf.GetCurSel()) {
		case 0:
			file_out_.set_pdf(dsp::Dither::Pdf::triangular);
		break;
		case 1:
			file_out_.set_pdf(dsp::Dither::Pdf::rectangular);
		break;
		case 2:
			file_out_.set_pdf(dsp::Dither::Pdf::gaussian);
		break;
		default:
			file_out_.set_pdf(dsp::Dither::Pdf::triangular);
	}
}

void CSaveWavDlg::OnSelchangeComboNoiseShaping() {			
	switch (m_noiseshaping.GetCurSel()) {
		case 0:
			file_out_.set_noiseshaping(dsp::Dither::NoiseShape::none);
		break;
		case 1:
			file_out_.set_noiseshaping(dsp::Dither::NoiseShape::highpass);
		break;
		default:
			file_out_.set_noiseshaping(dsp::Dither::NoiseShape::none);
	}
}

void CSaveWavDlg::OnToggleDither() {
	m_noiseshaping.EnableWindow(dither_btn_.GetCheck());
	m_pdf.EnableWindow(dither_btn_.GetCheck());
	file_out_.set_dither_enabled(dither_btn_.GetCheck());
}

void CSaveWavDlg::OnSavetracksseparated() {
	savewires_btn_.SetCheck(!savetracks_btn_.GetCheck());
	savegens_btn_.SetCheck(!savetracks_btn_.GetCheck());
}

void CSaveWavDlg::OnSavewiresseparated()  {
	savetracks_btn_.SetCheck(!savewires_btn_.GetCheck());
	savegens_btn_.SetCheck(!savewires_btn_.GetCheck());
}

void CSaveWavDlg::OnSavegensseparated() {
	savetracks_btn_.SetCheck(!savegens_btn_.GetCheck());
	savewires_btn_.SetCheck(!savegens_btn_.GetCheck());
}

}}

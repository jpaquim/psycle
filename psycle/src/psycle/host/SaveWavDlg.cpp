///\file
///\brief implementation file for psycle::host::CSaveWavDlg.

#include "SaveWavDlg.hpp"

#include <psycle/core/song.h>
#include <psycle/core/player.h>
#include <psycle/core/machine.h>

#include <boost/bind.hpp>

#include "Configuration.hpp"
#include "MidiInput.hpp"
#include "MainFrm.hpp"
#include "ChildView.hpp"
#include "PatternView.hpp"
#include "SeqView.hpp"
#include <iostream>
#include <iomanip>
#include <psycle/helpers/math.hpp>
#include <psycle/helpers/hexstring_to_integer.hpp>
#include <psycle/helpers/dither.hpp>

using namespace psycle::helpers;
using namespace psycle::helpers::math;
using namespace psycle::helpers::dsp;
using namespace psycle::core;

namespace psycle {
	namespace host {

		const int real_rate[]={8000,11025,16000,22050,32000,44100,48000,88200,96000};
		const int real_bits[]={8,16,24,32,32};

		extern CPsycleApp theApp;

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
			DDX_Control(pDX, IDC_FILEBROWSE, m_browse);
			DDX_Control(pDX, IDCANCEL, m_cancel);
			DDX_Control(pDX, IDC_SAVEWAVE, m_savewave);
			DDX_Control(pDX, IDC_SAVEWIRESSEPARATED, m_savewires);
			DDX_Control(pDX, IDC_SAVETRACKSSEPARATED, m_savetracks);
			DDX_Control(pDX, IDC_SAVEGENERATORSEPARATED, m_savegens);
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
			DDX_Control(pDX, IDC_CHECK_DITHER, m_dither);
			DDX_Control(pDX, IDC_COMBO_PDF, m_pdf);
			DDX_Control(pDX, IDC_COMBO_NOISESHAPING, m_noiseshaping);
			DDX_Radio(pDX, IDC_RECSONG, m_recmode);
			DDX_Radio(pDX, IDC_OUTPUTFILE, m_outputtype);
		}

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

		BOOL CSaveWavDlg::OnInitDialog() {
			CDialog::OnInitDialog();

			CMainFrame * mainFrame = ((CMainFrame *)theApp.m_pMainWnd);
			Song& pSong = mainFrame->projects()->active_project()->song();
			kill_thread=1;
			std::string name = Global::pConfig->GetCurrentWaveRecDir().c_str();
			name+='\\';
			name+=pSong.fileName;
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
			int pattern_id = pChildView->pattern_view()->main()->m_wndSeq.selected_entry()->pattern()->id();
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

			int bits;
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

			m_dither.SetCheck(BST_CHECKED);
			m_pdf.AddString("Triangular");
			m_pdf.AddString("Rectangular");
			m_pdf.AddString("Gaussian");			
			m_pdf.SetCurSel(0);
			m_noiseshaping.AddString("None");
			m_noiseshaping.AddString("High-Pass Contour");
			m_noiseshaping.SetCurSel(0);

			if (bits == 3 ) {
				m_dither.EnableWindow(false);
				m_pdf.EnableWindow(false);
				m_noiseshaping.EnableWindow(false);
			}

			psycle::audiodrivers::AudioDriverSettings settings = file_out_.playbackSettings();
			settings.setSamplesPerSec(real_rate[rate]);
			settings.setBitDepth(real_bits[bits]);
			settings.setChannelMode(Global::pConfig->_pOutputDriver->playbackSettings().channelMode());
			file_out_.setPlaybackSettings(settings);

			m_savetracks.SetCheck(false);
			m_savegens.SetCheck(false);
			m_savewires.SetCheck(false);

			m_text.SetWindowText("");		

			return true;  // return true unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return false
		}

		void CSaveWavDlg::OnOutputfile()
		{
			m_savewave.EnableWindow(true);
			m_outputtype=0;
			m_savewires.EnableWindow(true);
			m_savetracks.EnableWindow(true);
			m_savegens.EnableWindow(true);
			m_filename.EnableWindow(true);
			m_browse.EnableWindow(true);
		}

		void CSaveWavDlg::OnOutputclipboard()
		{
//			m_savewave.EnableWindow(false);
			m_outputtype=1;
			m_savewires.EnableWindow(false);
			m_savetracks.EnableWindow(false);
			m_savegens.EnableWindow(false);
			m_filename.EnableWindow(false);
			m_browse.EnableWindow(false);
		}
		
		void CSaveWavDlg::OnOutputsample()
		{
			m_savewave.EnableWindow(false);
			m_outputtype=2;
			m_savewires.EnableWindow(false);
			m_savetracks.EnableWindow(false);
			m_savegens.EnableWindow(false);
			m_filename.EnableWindow(false);
			m_browse.EnableWindow(false);
		}

		void CSaveWavDlg::OnFilebrowse() 
		{
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
			psycle::core::PatternSequence& seq = song.patternSequence();
			seq_main_play_line_ = *(seq.begin()+1);
			*(seq.begin()+1) = seq_tmp_play_line_;
			seq_tmp_play_line_->SetSequence(&song.patternSequence());
		}

		void CSaveWavDlg::SwitchToNormalPlay() {
			Player* player = &Player::singleton();
			CoreSong& song = player->song();
			psycle::core::PatternSequence& seq = song.patternSequence();
			*(seq.begin()+1) = seq_main_play_line_;
		}

		void CSaveWavDlg::OnSavewave() {
			// set gui to recmode
			GetDlgItem(IDC_RECSONG)->EnableWindow(false);
			GetDlgItem(IDC_RECPATTERN)->EnableWindow(false);
			GetDlgItem(IDC_RECRANGE)->EnableWindow(false);
			GetDlgItem(IDC_FILEBROWSE)->EnableWindow(false);
			m_savewave.EnableWindow(false);
			m_cancel.SetWindowText("Stop");
			m_filename.EnableWindow(false);
			m_savetracks.EnableWindow(false);
			m_savegens.EnableWindow(false);
			m_savewires.EnableWindow(false);
			m_rate.EnableWindow(false);
			m_bits.EnableWindow(false);
			m_channelmode.EnableWindow(false);
			m_pdf.EnableWindow(false);
			m_noiseshaping.EnableWindow(false);
			m_dither.EnableWindow(false);
			m_rangeend.EnableWindow(false);
			m_rangestart.EnableWindow(false);
			m_patnumber.EnableWindow(false);
			Player::singleton().stop();			
			Player::singleton().driver().set_started(false);
			Sleep(1000);
			// prepare recmodes
			if (m_recmode == 0) {
				// record entire song
				Player::singleton().start(0);
				m_progress.SetRange(0, Player::singleton().song().patternSequence().max_beats());
			} else
			if (m_recmode == 1) {
				CString name;
				int pstart;
				m_patnumber.GetWindowText(name);
				hexstring_to_integer(name.GetBuffer(2), pstart);
				SwitchToTmpPlay();
				seq_tmp_play_line_->clear();
				Player* player = &Player::singleton();
				CoreSong& song = player->song();
				psycle::core::Pattern* pattern = song.patternSequence().FindPattern(pstart);
				SequenceEntry* entry = new SequenceEntry(seq_tmp_play_line_);
				entry->setPattern(pattern);
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
				CMainFrame * mainFrame = ((CMainFrame *)theApp.m_pMainWnd);
				SequenceEntry* start_entry = mainFrame->m_wndSeq.GetEntry(seq_start);
				seq_end_entry_ = mainFrame->m_wndSeq.GetEntry(seq_end);
				Player::singleton().start(start_entry->tickPosition());
				m_progress.SetRange(static_cast<short>(start_entry->tickPosition()),
								    static_cast<short>(start_entry->tickEndPosition()));
			}
			// prepare player for recording			
			// create a WaveFile and a thread for writing the audio data
			CString name;
			m_filename.GetWindowText(name);
			std::string file_name = name;
			psycle::audiodrivers::AudioDriverSettings settings = file_out_.playbackSettings();
			settings.setDeviceName(file_name);
			file_out_.setPlaybackSettings(settings);
			file_out_.set_opened(true);
			kill_thread = 0;
			unsigned long tmp;			
			thread_ = new std::thread(boost::bind(&CSaveWavDlg::thread_function, this));			
		}

		void CSaveWavDlg::thread_function() {
			Player* player = &Player::singleton();
			const int frames = 256;
			while(!kill_thread && player->playing()) {
				 if ((m_recmode == 2) && (player->playPos() >=
					  seq_end_entry_->tickPosition() + 
					  seq_end_entry_->pattern()->beats() 
				     )) break;
 			    file_out_.Write(player->Work(frames), frames);
				m_progress.SetPos(static_cast<short>(player->playPos()));
			}
			file_out_.set_opened(false);
			Player::singleton().stop();
			if (m_recmode == 1) {
				SwitchToNormalPlay();
			}
			SaveEnd();
		}



/*
		void CSaveWavDlg::OnSavewave() 
		{
			const int real_rate[]={8000,11025,16000,22050,32000,44100,48000,88200,96000};
			const int real_bits[]={8,16,24,32,32};
			bool isFloat = (bits == 4);
			CMainFrame * mainFrame = ((CMainFrame *)theApp.m_pMainWnd);
			Song& pSong = mainFrame->projects()->active_project()->song();
			Player *pPlayer = Global::pPlayer;

			GetDlgItem(IDC_RECSONG)->EnableWindow(false);
			GetDlgItem(IDC_RECPATTERN)->EnableWindow(false);
			GetDlgItem(IDC_RECRANGE)->EnableWindow(false);
			GetDlgItem(IDC_FILEBROWSE)->EnableWindow(false);

			m_savewave.EnableWindow(false);
			m_cancel.SetWindowText("Stop");
			m_filename.EnableWindow(false);
			m_savetracks.EnableWindow(false);
			m_savegens.EnableWindow(false);
			m_savewires.EnableWindow(false);
			m_rate.EnableWindow(false);
			m_bits.EnableWindow(false);
			m_channelmode.EnableWindow(false);
			m_pdf.EnableWindow(false);
			m_noiseshaping.EnableWindow(false);
			m_dither.EnableWindow(false);

			m_rangeend.EnableWindow(false);
			m_rangestart.EnableWindow(false);
			m_patnumber.EnableWindow(false);
			
			autostop = Global::pConfig->autoStopMachines;
			if ( Global::pConfig->autoStopMachines )
			{
				Global::pConfig->autoStopMachines = false;
				for (int c=0; c<MAX_MACHINES; c++)
				{
					if (pSong.machine(c))
					{
						pSong.machine(c)->Standby(false);
					}
				}
			}
			playblock = pPlayer->_playBlock;
			loopsong = pPlayer->_loopSong;
			memcpy(sel,pSong.playOrderSel,MAX_SONG_POSITIONS);
			memset(pSong.playOrderSel,0,MAX_SONG_POSITIONS);
			CString name;
			m_filename.GetWindowText(name);

			rootname=name;
			rootname=rootname.substr(0,
				std::max(std::string::size_type(0),rootname.length()-4));


			if (m_outputtype == 0)
			{	//record to file
				if (m_savetracks.GetCheck())
				{
					memcpy(_Muted,pSong._trackMuted,sizeof(pSong._trackMuted));

					int count = 0;

					for (int i = 0; i < pSong.tracks(); i++)
					{
						if (!_Muted[i])
						{
							count++;
							current = i;
							for (int j = 0; j < pSong.tracks(); j++)
							{
								if (j != i)
								{
									pSong._trackMuted[j] = true;
								}
								else
								{
									pSong._trackMuted[j] = false;
								}
							}
							// now save the song
							std::ostringstream filename;
							filename << rootname;
							filename << "-track "
								<< std::setprecision(2) << (unsigned)i;
							SaveWav(filename.str().c_str(),real_bits[bits],real_rate[rate],channelmode,isFloat);
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
						if (pSong.machine(MASTER_INDEX)->_inputCon[i])
						{
							_Muted[i] = pSong.machine(pSong.machine(MASTER_INDEX)->_inputMachines[i])->_mute;
						}
						else
						{
							_Muted[i] = true;
						}
					}

					for (int i = 0; i < MAX_CONNECTIONS; i++)
					{
						if (!_Muted[i])
						{
							current = i;
							for (int j = 0; j < MAX_CONNECTIONS; j++)
							{
								if (pSong.machine(MASTER_INDEX)->_inputCon[j])
								{
									if (j != i)
									{
										pSong.machine(pSong.machine(MASTER_INDEX)->_inputMachines[j])->_mute = true;
									}
									else
									{
										pSong.machine(pSong.machine(MASTER_INDEX)->_inputMachines[j])->_mute = false;
									}
								}
							}
							// now save the song
							char filename[MAX_PATH];
							sprintf(filename,"%s-wire %.2u %s.wav",rootname.c_str(),i,pSong.machine(pSong.machine(MASTER_INDEX)->_inputMachines[i])->GetEditName().c_str());
							SaveWav(filename,real_bits[bits],real_rate[rate],channelmode,isFloat);
							return;
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
						if (pSong.machine(i))
						{
							_Muted[i] = pSong.machine(i)->_mute;
						}
						else
						{
							_Muted[i] = true;
						}
					}

					for (int i = 0; i < MAX_BUSES; i++)
					{
						if (!_Muted[i])
						{
							current = i;
							for (int j = 0; j < MAX_BUSES; j++)
							{
								if (pSong.machine(j))
								{
									if (j != i)
									{
										pSong.machine(j)->_mute = true;
									}
									else
									{
										pSong.machine(j)->_mute = false;
									}
								}
							}
							// now save the song
							char filename[MAX_PATH];
							sprintf(filename,"%s-generator %.2u %s.wav",rootname.c_str(),i,pSong.machine(i)->GetEditName().c_str());
							SaveWav(filename,real_bits[bits],real_rate[rate],channelmode,isFloat);
							return;
						}
					}
					current = 256;
					SaveEnd();
				}
				else
				{
					SaveWav(name.GetBuffer(4),real_bits[bits],real_rate[rate],channelmode,isFloat);
				}
			}
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

		void CSaveWavDlg::OnCancel() 
		{
			kill_thread = 1;
			Sleep(100);
			delete seq_tmp_play_line_;			
			CDialog::OnCancel();
		}

		void CSaveWavDlg::SaveEnd()
		{
			kill_thread=1;
			delete thread_;
/*
			if (autostop) {
				Global::pConfig->autoStopMachines=true;
			}
			#if PSYCLE__CONFIGURATION__USE_PSYCORE
				///todo: restore the previous values
				Global::pConfig->_pOutputDriver->set_started(true);
			#else
				Global::pPlayer->_playBlock=playblock;
				Global::pPlayer->_loopSong=loopsong;
				std::memcpy(Global::song().playOrderSel,sel,MAX_SONG_POSITIONS);
				Global::pConfig->_pOutputDriver->Enable(true);
			#endif
			Global::pConfig->_pMidiInput->Open();

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

			}
			else if (m_savetracks.GetCheck())
			{
				CMainFrame * mainFrame = ((CMainFrame *)theApp.m_pMainWnd);
				Song& pSong = mainFrame->projects()->active_project()->song();

				const int real_rate[]={8000,11025,16000,22050,32000,44100,48000,88200,96000};
				const int real_bits[]={8,16,24,32,32};
				const bool isFloat = (bits == 4);

				for (int i = current+1; i < pSong.tracks(); i++)
				{
					if (!_Muted[i])
					{
						current = i;
						for (int j = 0; j < pSong.tracks(); j++)
						{
							if (j != i)
							{
								pSong._trackMuted[j] = true;
							}
							else
							{
								pSong._trackMuted[j] = false;
							}
						}
						// now save the song
						char filename[MAX_PATH];
						sprintf(filename,"%s-track %.2u.wav",rootname.c_str(),i);
//						SaveWav(filename,real_bits[bits],real_rate[rate],channelmode,isFloat);
						return;
					}
				}
				memcpy(pSong._trackMuted,_Muted,sizeof(pSong._trackMuted));
			}

			else if (m_savewires.GetCheck())
			{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				CMainFrame * mainFrame = ((CMainFrame *)theApp.m_pMainWnd);
				Song& pSong = mainFrame->projects()->active_project()->song();
#else
				Song& pSong = Global::song();
#endif

				const int real_rate[]={8000,11025,16000,22050,32000,44100,48000,88200,96000};
				const int real_bits[]={8,16,24,32,32};
				const bool isFloat = (bits == 4);

				for (int i = current+1; i < MAX_CONNECTIONS; i++)
				{
					if (!_Muted[i])
					{
						current = i;
						for (int j = 0; j < MAX_CONNECTIONS; j++)
						{
							if (pSong.machine(MASTER_INDEX)->_inputCon[j])
							{
								if (j != i)
								{
									pSong.machine(pSong.machine(MASTER_INDEX)->_inputMachines[j])->_mute = true;
								}
								else
								{
									pSong.machine(pSong.machine(MASTER_INDEX)->_inputMachines[j])->_mute = false;
								}
							}
						}
						// now save the song
						char filename[MAX_PATH];
						sprintf(filename,"%s-wire %.2u %s.wav",rootname.c_str(),i,pSong.machine(pSong.machine(MASTER_INDEX)->_inputMachines[i])->GetEditName().c_str());
//						SaveWav(filename,real_bits[bits],real_rate[rate],channelmode,isFloat);
						return;
					}
				}

				for (int i = 0; i < MAX_CONNECTIONS; i++)
				{
					if (pSong.machine(MASTER_INDEX)->_inputCon[i])
					{
						pSong.machine(pSong.machine(MASTER_INDEX)->_inputMachines[i])->_mute = _Muted[i];
					}
				}
			}

			else if (m_savegens.GetCheck())
			{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				CMainFrame * mainFrame = ((CMainFrame *)theApp.m_pMainWnd);
				Song& pSong = mainFrame->projects()->active_project()->song();
#else
				Song& pSong = Global::song();
#endif

				const int real_rate[]={8000,11025,16000,22050,32000,44100,48000,88200,96000};
				const int real_bits[]={8,16,24,32,32};
				const bool isFloat = (bits == 4);

				for (int i = current+1; i < MAX_BUSES; i++)
				{
					if (!_Muted[i])
					{
						current = i;
						for (int j = 0; j < MAX_BUSES; j++)
						{
							if (pSong.machine(j))
							{
								if (j != i)
								{
									pSong.machine(j)->_mute = true;
								}
								else
								{
									pSong.machine(j)->_mute = false;
								}
							}
						}
						// now save the song
						char filename[MAX_PATH];
						sprintf(filename,"%s-generator %.2u %s.wav",rootname.c_str(),i,pSong.machine(i)->GetEditName().c_str());
//						SaveWav(filename,real_bits[bits],real_rate[rate],channelmode,isFloat);
						return;
					}
				}

				for (int i = 0; i < MAX_BUSES; i++)
				{
					if (pSong.machine(i))
					{
						pSong.machine(i)->_mute = _Muted[i];
					}
				}
			}
			*/

			m_text.SetWindowText("");

			GetDlgItem(IDC_RECSONG)->EnableWindow(true);
			GetDlgItem(IDC_RECPATTERN)->EnableWindow(true);
			GetDlgItem(IDC_RECRANGE)->EnableWindow(true);
			GetDlgItem(IDC_RECBLOCK)->EnableWindow(true);
			GetDlgItem(IDC_FILEBROWSE)->EnableWindow(true);

			m_filename.EnableWindow(true);
			m_savetracks.EnableWindow(true);
			m_savegens.EnableWindow(true);
			m_savewires.EnableWindow(true);
			m_rate.EnableWindow(true);
			m_bits.EnableWindow(true);
			m_channelmode.EnableWindow(true);
			m_pdf.EnableWindow(m_dither.GetCheck());
			m_noiseshaping.EnableWindow(m_dither.GetCheck());
			m_dither.EnableWindow(true);

			switch (m_recmode)
			{
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
			m_savewave.EnableWindow(true);
			m_cancel.SetWindowText("Close");
		}

		void CSaveWavDlg::SaveToClipboard()
		{
			OpenClipboard();
			EmptyClipboard();

			const int real_rate[]={8000,11025,16000,22050,32000,44100,48000,88200,96000};
			const int real_bits[]={8,16,24,32};

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
				m_dither.EnableWindow(false);
				m_pdf.EnableWindow(false);
				m_noiseshaping.EnableWindow(false);
			} else {
				m_dither.EnableWindow(true);
				m_pdf.EnableWindow(true);
				m_noiseshaping.EnableWindow(true);
			}
			psycle::audiodrivers::AudioDriverSettings settings = file_out_.playbackSettings();
			settings.setBitDepth(real_bits[m_bits.GetCurSel()]);
			file_out_.setPlaybackSettings(settings);
		}

		void CSaveWavDlg::OnSelchangeComboChannels() {
			psycle::audiodrivers::AudioDriverSettings settings = file_out_.playbackSettings();
			settings.setChannelMode(this->m_channelmode.GetCurSel());
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
			m_noiseshaping.EnableWindow(m_dither.GetCheck());
			m_pdf.EnableWindow(m_dither.GetCheck());
			file_out_.set_dither_enabled(m_dither.GetCheck());
		}

		void CSaveWavDlg::OnSavetracksseparated() {
			if (m_savetracks.GetCheck()) {
				m_savewires.SetCheck(false);
				m_savegens.SetCheck(false);
			}
		}

		void CSaveWavDlg::OnSavewiresseparated()  {
			if (m_savewires.GetCheck()) {
				m_savetracks.SetCheck(false);
				m_savegens.SetCheck(false);
			}
		}

		void CSaveWavDlg::OnSavegensseparated() {
			if (m_savegens.GetCheck()) {
				m_savetracks.SetCheck(false);
				m_savewires.SetCheck(false);
			}
		}

	}   // namespace
}   // namespace

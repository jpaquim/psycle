///\file
///\brief implementation file for psycle::host::CSaveWavDlg.

#include "SaveWavDlg.hpp"
#include "Configuration.hpp"
#include "MidiInput.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/song.h>
#include <psycle/core/player.h>
#include <psycle/core/machine.h>
using namespace psy::core;
#else
#include "Song.hpp"
#include "Player.hpp"
#include "Machine.hpp"
#endif

#include "MainFrm.hpp"
#include "ChildView.hpp"
#include "PatternView.hpp"
#include <iostream>
#include <iomanip>
#include <psycle/helpers/helpers.hpp>
#include <psycle/helpers/dither.hpp>
using namespace psycle::helpers::dsp;

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		extern CPsycleApp theApp;
		DWORD WINAPI __stdcall RecordThread(void *b);
		int CSaveWavDlg::channelmode = -1;
		int CSaveWavDlg::rate = -1;
		int CSaveWavDlg::bits = -1;
		int CSaveWavDlg::noiseshape = 0;
		int CSaveWavDlg::ditherpdf = (int)Dither::Pdf::triangular;
		BOOL CSaveWavDlg::savewires = false;
		BOOL CSaveWavDlg::savetracks = false;
		BOOL CSaveWavDlg::savegens = false;

		CSaveWavDlg::CSaveWavDlg(CChildView* pChildView, CSelection* pBlockSel, CWnd* pParent /* = 0 */) : CDialog(CSaveWavDlg::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CSaveWavDlg)
			m_recmode = 0;		
			m_outputtype = 0;
			//}}AFX_DATA_INIT
			this->pChildView = pChildView;
			this->pBlockSel = pBlockSel;
		}

		void CSaveWavDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CSaveWavDlg)
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
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CSaveWavDlg, CDialog)
			//{{AFX_MSG_MAP(CSaveWavDlg)
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
			//}}AFX_MSG_MAP			
			
		END_MESSAGE_MAP()

		BOOL CSaveWavDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			threadopen = 0;
			CMainFrame * mainFrame = ((CMainFrame *)theApp.m_pMainWnd);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			Song& pSong = mainFrame->projects()->active_project()->song();
#else
			Song& pSong = Global::song();
#endif
			thread_handle=INVALID_HANDLE_VALUE;
			kill_thread=1;
			lastpostick=0;
			lastlinetick=0;
			saving=false;

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

			char num[3];
			sprintf(num,"%02x",pSong.playOrder[mainFrame->m_wndView.pattern_view()->editPosition]);
			m_patnumber.SetWindowText(num);
			sprintf(num,"%02x",0);
			m_rangestart.SetWindowText(num);
			sprintf(num,"%02x",pSong.playLength-1);
			m_rangeend.SetWindowText(num);			

			sprintf(num,"%02x",pSong.playOrder[mainFrame->m_wndView.pattern_view()->editPosition]);
			m_patnumber2.SetWindowText(num);


			if (pChildView->pattern_view()->blockSelected)
			{
				sprintf(num,"%02x",pBlockSel->start.line);
				m_linestart.SetWindowText(num);
				sprintf(num,"%02x",pBlockSel->end.line+1);
				m_lineend.SetWindowText(num);
			}
			else
			{
				sprintf(num,"%02x",0);
				m_linestart.SetWindowText(num);
				sprintf(num,"%02x",1);
				m_lineend.SetWindowText(num);
			}

			m_progress.SetRange(0,1);
			m_progress.SetPos(0);

			if ((rate < 0) || (rate >5))
			{
				if (Global::pConfig->_pOutputDriver->_samplesPerSec <= 8000)
				{
					rate = 0;
				}
				else if (Global::pConfig->_pOutputDriver->_samplesPerSec <= 11025)
				{
					rate = 1;
				}
				else if (Global::pConfig->_pOutputDriver->_samplesPerSec <= 16000)
				{
					rate = 2;
				}
				else if (Global::pConfig->_pOutputDriver->_samplesPerSec <= 22050)
				{
					rate = 3;
				}
				else if (Global::pConfig->_pOutputDriver->_samplesPerSec <= 32000)
				{
					rate = 4;
				}
				else if (Global::pConfig->_pOutputDriver->_samplesPerSec <= 44100)
				{
					rate = 5;
				}
				else if (Global::pConfig->_pOutputDriver->_samplesPerSec <= 48000)
				{
					rate = 6;
				}
				else if (Global::pConfig->_pOutputDriver->_samplesPerSec <= 88200)
				{
					rate = 7;
				}
				else 
				{
					rate = 8;
				}
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
					bits = 4;
				}
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

			if ((channelmode < 0) || (channelmode > 3))
			{
				channelmode = Global::pConfig->_pOutputDriver->_channelmode;
			}
			m_channelmode.SetCurSel(channelmode);

			m_dither.SetCheck(BST_CHECKED);

			m_pdf.AddString("Triangular");
			m_pdf.AddString("Rectangular");
			m_pdf.AddString("Gaussian");
			ditherpdf = (int)Dither::Pdf::triangular;
			m_pdf.SetCurSel(ditherpdf);

			m_noiseshaping.AddString("None");
			m_noiseshaping.AddString("High-Pass Contour");
			noiseshape=0;
			m_noiseshaping.SetCurSel(noiseshape);

			if (bits == 3 )
			{
				m_dither.EnableWindow(false);
				m_pdf.EnableWindow(false);
				m_noiseshaping.EnableWindow(false);
			}

			m_savetracks.SetCheck(savetracks);
			m_savegens.SetCheck(savegens);
			m_savewires.SetCheck(savewires);

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

		void CSaveWavDlg::OnSelAllSong() 
		{
			m_rangeend.EnableWindow(false);
			m_rangestart.EnableWindow(false);
			m_patnumber.EnableWindow(false);
			m_lineend.EnableWindow(false);
			m_linestart.EnableWindow(false);
			m_patnumber2.EnableWindow(false);
			m_recmode=0;
		}

		void CSaveWavDlg::OnSelPattern() 
		{
			m_rangeend.EnableWindow(false);
			m_rangestart.EnableWindow(false);
			m_patnumber.EnableWindow(true);
			m_lineend.EnableWindow(false);
			m_linestart.EnableWindow(false);
			m_patnumber2.EnableWindow(false);
			m_recmode=1;
		}

		void CSaveWavDlg::OnSelRange() 
		{
			m_rangeend.EnableWindow(true);
			m_rangestart.EnableWindow(true);
			m_patnumber.EnableWindow(false);
			m_lineend.EnableWindow(false);
			m_linestart.EnableWindow(false);
			m_patnumber2.EnableWindow(false);
			m_recmode=2;
		}

		void CSaveWavDlg::OnRecblock()
		{
			m_rangeend.EnableWindow(false);
			m_rangestart.EnableWindow(false);
			m_patnumber.EnableWindow(false);
			m_lineend.EnableWindow(true);
			m_linestart.EnableWindow(true);
			m_patnumber2.EnableWindow(true);
			m_recmode=3;
		}

		void CSaveWavDlg::OnSavewave() 
		{
			const int real_rate[]={8000,11025,16000,22050,32000,44100,48000,88200,96000};
			const int real_bits[]={8,16,24,32,32};
			bool isFloat = (bits == 4);
			CMainFrame * mainFrame = ((CMainFrame *)theApp.m_pMainWnd);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			Song& pSong = mainFrame->projects()->active_project()->song();
#else
			Song& pSong = Global::song();
#endif
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
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			//todo: keep previous values
#else
			playblock = pPlayer->_playBlock;
			loopsong = pPlayer->_loopSong;
			memcpy(sel,pSong.playOrderSel,MAX_SONG_POSITIONS);
			memset(pSong.playOrderSel,0,MAX_SONG_POSITIONS);
#endif
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

		void CSaveWavDlg::SaveWav(std::string file, int bits, int rate, int channelmode,bool isFloat)
		{
			Player *pPlayer = Global::pPlayer;
			CMainFrame * mainFrame = ((CMainFrame *)theApp.m_pMainWnd);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			Song& pSong = mainFrame->projects()->active_project()->song();
#else
			Song& pSong = Global::song();
#endif
			saving=true;
			pPlayer->stopRecording();
			Global::pConfig->_pOutputDriver->Enable(false);
			///\todo: for zealan, this call is not closing the midi driver, and when doing the Open again, it crashes.
			Global::pConfig->_pMidiInput->Close();

			std::string::size_type pos = file.rfind('\\');
			if (pos == std::string::npos)
			{
				m_text.SetWindowText(file.c_str());
			}
			else
			{
				m_text.SetWindowText(file.substr(pos+1).c_str());
			}
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			///todo: recording in psycore doesn't support other mediums, except via audio drivers.
			// so for clipboard, we need an audio driver.
			if (!file.empty()) {
				pPlayer->setFileName(file);
				pPlayer->startRecording(m_dither.GetCheck()==BST_CHECKED && bits!=32, Dither::Pdf::type(ditherpdf), Dither::NoiseShape::type(noiseshape));
			}
#else
			pPlayer->StartRecording(file,bits,rate,channelmode,isFloat,
									m_dither.GetCheck()==BST_CHECKED && bits!=32, ditherpdf, noiseshape,&clipboardmem);
#endif
			int tmp;
			int cont;
			CString name;

			int pstart;
			kill_thread = 0;
			tickcont=0;
			lastlinetick=0;
			int i,j;
			
			int blockSLine;
			int blockELine;

			switch (m_recmode)
			{
				using helpers::hexstring_to_integer;
			case 0:
				{
				j=0; // Calculate progress bar range.
				for (i=0;i<pSong.playLength;i++)
				{
					j+=pSong.patternLines[pSong.playOrder[i]];
				}
				m_progress.SetRange(0,j);
				lastpostick=0;
				
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				pPlayer->setLoopSong();
				pPlayer->start();
#else
				pPlayer->_playBlock=false;
				pPlayer->Start(0,0);
#endif
				}
				break;
			case 1:
				{
				m_patnumber.GetWindowText(name);
				hexstring_to_integer(name.GetBuffer(2), pstart);
				m_progress.SetRange(0,pSong.patternLines[pstart]);
				for (cont=0;cont<pSong.playLength;cont++)
				{
					if ( (int)pSong.playOrder[cont] == pstart)
					{
						pstart= cont;
						break;
					}
				}
				lastpostick=pstart;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				int findPattern=0;
				SequenceLine* patternline = *pSong.patternSequence().begin();
				SequenceLine::iterator iter = patternline->begin();
				for(; iter != patternline->end() && findPattern < cont; iter++ , findPattern++);
				if (iter != patternline->end()) {
					pPlayer->setLoopSequenceEntry(iter->second);
					pPlayer->start(iter->second->tickPosition());
				}
#else
				pSong.playOrderSel[cont]=true;
				pPlayer->Start(pstart,0);
				pPlayer->_playBlock=true;
				pPlayer->_loopSong=false;
#endif
				}
				break;
			case 2:
				{
				m_rangestart.GetWindowText(name);
				hexstring_to_integer(name.GetBuffer(2), pstart);
				m_rangeend.GetWindowText(name);
				hexstring_to_integer(name.GetBuffer(2), tmp);
				j=0;
				for (cont=pstart;cont<=tmp;cont++)
				{
					pSong.playOrderSel[cont]=true;
					j+=pSong.patternLines[pSong.playOrder[cont]];
				}
				m_progress.SetRange(0,j);

				lastpostick=pstart;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				int findPattern=0;
				SequenceLine* patternline = *pSong.patternSequence().begin();
				SequenceLine::iterator iter = patternline->begin();
				for(; iter != patternline->end() && findPattern < pstart; iter++ , findPattern++);

				SequenceLine::iterator iterend = iter;
				for(; iterend != patternline->end() && findPattern < tmp; iterend++ , findPattern++);

				if (iter != patternline->end() && iterend != patternline->end()) {
					pPlayer->setLoopRange(iter->second->tickPosition(), iterend->second->tickEndPosition());
					pPlayer->start(iter->second->tickPosition());
				}

#else
				pPlayer->Start(pstart,0);
				pPlayer->_playBlock=true;
				pPlayer->_loopSong=false;
#endif
				}
				break;
			case 3:
				{
				m_patnumber.GetWindowText(name);
				hexstring_to_integer(name.GetBuffer(2), pstart);
				m_linestart.GetWindowText(name);
				hexstring_to_integer(name.GetBuffer(2), blockSLine);
				m_lineend.GetWindowText(name);
				hexstring_to_integer(name.GetBuffer(2), blockELine);

				m_progress.SetRange(blockSLine,blockELine);
				//find the position in the sequence where the pstart pattern is located.
				for (cont=0;cont<pSong.playLength;cont++)
				{
					if ( (int)pSong.playOrder[cont] == pstart)
					{
						pstart= cont;
						break;
					}
				}
				lastpostick=pstart;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				//todo: this records the whole sequenceEntry, not jus the lines selected.
				int findPattern=0;
				SequenceLine* patternline = *pSong.patternSequence().begin();
				SequenceLine::iterator iter = patternline->begin();
				for(; iter != patternline->end() && findPattern < cont; iter++ , findPattern++);
				if (iter != patternline->end()) {
					pPlayer->setLoopSequenceEntry(iter->second);
					pPlayer->start(iter->second->tickPosition());
				}

#else
				pSong.playOrderSel[cont]=true;
				pPlayer->Start(pstart,blockSLine, blockELine);
				pPlayer->_playBlock=true;
				pPlayer->_loopSong=false;
#endif

				}
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
			int stream_size = 8192; // Player has just a single buffer of 65535 samples to allocate both channels
			//int stream_buffer[65535];
			while(!((CSaveWavDlg*)b)->kill_thread)
			{
				if (!pPlayer->recording()) // the player automatically closes the wav recording when looping.
				{
					pPlayer->stop();
					((CSaveWavDlg*)b)->SaveEnd();
					((CSaveWavDlg*)b)->threadopen--;
					ExitThread(0);
					//return 0;
				}
				pPlayer->Work(pPlayer,stream_size);
				((CSaveWavDlg*)b)->SaveTick();
			}

			pPlayer->stop();
			pPlayer->stopRecording();
			((CSaveWavDlg*)b)->SaveEnd();
			((CSaveWavDlg*)b)->threadopen--;
			ExitThread(0);
			//return 0;
		}

		void CSaveWavDlg::OnCancel() 
		{
			if (saving || (threadopen > 0))
			{
				//while(threadopen > 0) 
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
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			///todo: restore the previous values
#else
			Global::pPlayer->_playBlock=playblock;
			Global::pPlayer->_loopSong=loopsong;
			memcpy(Global::song().playOrderSel,sel,MAX_SONG_POSITIONS);
#endif
			Global::pConfig->_pOutputDriver->Enable(true);
			Global::pConfig->_pMidiInput->Open();

			if (m_outputtype == 1)
			{
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
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				Song& pSong = mainFrame->projects()->active_project()->song();
#else
				Song& pSong = Global::song();
#endif

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
						SaveWav(filename,real_bits[bits],real_rate[rate],channelmode,isFloat);
						return;
					}
				}
				memcpy(pSong._trackMuted,_Muted,sizeof(pSong._trackMuted));
			}

			else if (m_savewires.GetCheck())
			{
				CMainFrame * mainFrame = ((CMainFrame *)theApp.m_pMainWnd);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
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
						SaveWav(filename,real_bits[bits],real_rate[rate],channelmode,isFloat);
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
				CMainFrame * mainFrame = ((CMainFrame *)theApp.m_pMainWnd);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
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
						SaveWav(filename,real_bits[bits],real_rate[rate],channelmode,isFloat);
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
			clipboardwavheader.fmtcontent.nChannels = (channelmode == 3) ? 2 : 1;
			clipboardwavheader.fmtcontent.nSamplesPerSec = real_rate[rate];
			clipboardwavheader.fmtcontent.wBitsPerSample = real_bits[bits];
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

		void CSaveWavDlg::SaveTick()
		{
			CMainFrame * mainFrame = ((CMainFrame *)theApp.m_pMainWnd);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			Song& pSong = mainFrame->projects()->active_project()->song();
#else
			Song& pSong = Global::song();
#endif
			Player* pPlayer = Global::pPlayer;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			///todo: bar positioning.
#else
			for (int i=lastpostick+1;i<pPlayer->_sequencePosition;i++)
			{
				tickcont+=pSong.patternLines[pSong.playOrder[i]];
			}
			if (lastpostick!= pPlayer->_sequencePosition ) 
			{
				tickcont+=pSong.patternLines[pSong.playOrder[lastpostick]]-(lastlinetick+1)+pPlayer->_lineCounter;
			}
			else tickcont+=pPlayer->_lineCounter-lastlinetick;

			lastlinetick = pPlayer->_lineCounter;
			lastpostick = pPlayer->_sequencePosition;
#endif
			if (!kill_thread ) 
			{
				m_progress.SetPos(tickcont);
			}
		}

		void CSaveWavDlg::OnSelchangeComboBits() 
		{
			bits = m_bits.GetCurSel();
			if (bits == 3 )
			{
				m_dither.EnableWindow(false);
				m_pdf.EnableWindow(false);
				m_noiseshaping.EnableWindow(false);
			}
			else
			{
				m_dither.EnableWindow(true);
				m_pdf.EnableWindow(true);
				m_noiseshaping.EnableWindow(true);
			}
		}

		void CSaveWavDlg::OnSelchangeComboChannels() 
		{
			channelmode = m_channelmode.GetCurSel();
		}

		void CSaveWavDlg::OnSelchangeComboRate() 
		{
			rate = m_rate.GetCurSel();
		}

		void CSaveWavDlg::OnSelchangeComboPdf()
		{
			ditherpdf = m_pdf.GetCurSel();
		}
		void CSaveWavDlg::OnSelchangeComboNoiseShaping()
		{
			noiseshape = m_noiseshaping.GetCurSel();
		}
		void CSaveWavDlg::OnToggleDither()
		{
			m_noiseshaping.EnableWindow(m_dither.GetCheck());
			m_pdf.EnableWindow(m_dither.GetCheck());
		}
		void CSaveWavDlg::OnSavetracksseparated() 
		{
			if (savetracks = m_savetracks.GetCheck())
			{
				m_savewires.SetCheck(false);
				savewires = false;
				m_savegens.SetCheck(false);
				savegens = false;
			}
		}

		void CSaveWavDlg::OnSavewiresseparated() 
		{
			if (savewires = m_savewires.GetCheck())
			{
				m_savetracks.SetCheck(false);
				savetracks = false;
				m_savegens.SetCheck(false);
				savegens = false;
			}
		}

		void CSaveWavDlg::OnSavegensseparated() 
		{
			if (savewires = m_savegens.GetCheck())
			{
				m_savetracks.SetCheck(false);
				savetracks = false;
				m_savewires.SetCheck(false);
				savewires = false;
			}
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

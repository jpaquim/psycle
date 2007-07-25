///\file
///\brief implementation file for psycle::host::CSaveWavDlg.
#include <psycle/project.private.hpp>
#include "SaveWavDlg.hpp"
#include "psycle.hpp"
#include "Song.hpp"
#include "configuration.hpp"
#include "MidiInput.hpp"
#include "Player.hpp"
#include "Machine.hpp"
#include "helpers.hpp"
#include "MainFrm.hpp"
#include "childview.hpp"
#include <iostream>
#include <iomanip>
#include "mfc_namespace.hpp"
#include ".\savewavdlg.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		extern CPsycleApp theApp;
		DWORD WINAPI __stdcall RecordThread(void *b);
		int CSaveWavDlg::channelmode = -1;
		int CSaveWavDlg::rate = -1;
		int CSaveWavDlg::bits = -1;
		int CSaveWavDlg::noiseshape = 0;
		int CSaveWavDlg::ditherpdf = (int)pdf::triangular;
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
			Song* pSong = Global::_pSong;
			thread_handle=INVALID_HANDLE_VALUE;
			kill_thread=1;
			lastpostick=0;
			lastlinetick=0;
			saving=false;

			std::string name = Global::pConfig->GetCurrentSongDir();
			name+='\\';
			name+=pSong->fileName;
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
			sprintf(num,"%02x",pSong->playOrder[((CMainFrame *)theApp.m_pMainWnd)->m_wndView.editPosition]);
			m_patnumber.SetWindowText(num);
			sprintf(num,"%02x",0);
			m_rangestart.SetWindowText(num);
			sprintf(num,"%02x",pSong->playLength-1);
			m_rangeend.SetWindowText(num);			

			sprintf(num,"%02x",pSong->playOrder[((CMainFrame *)theApp.m_pMainWnd)->m_wndView.editPosition]);
			m_patnumber2.SetWindowText(num);

			if (pChildView->blockSelected)
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

			m_dither.SetCheck(BST_CHECKED);

			m_pdf.AddString("Triangular");
			m_pdf.AddString("Rectangular");
			m_pdf.AddString("Gaussian");
			ditherpdf = (int)pdf::triangular;
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
			m_savewave.EnableWindow(false);
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
			CFileDialog dlg(false,"wav",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
			if ( dlg.DoModal() == IDOK ) 
			{
				CString str = dlg.GetPathName();
				CString str2 = str.Right(4);
				if ( str2.CompareNoCase(".wav") != 0 ) 
				{
					str.Insert(str.GetLength(),".wav");
				}
				m_filename.SetWindowText(str);
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
			Song *pSong = Global::_pSong;
			Player *pPlayer = Global::pPlayer;

			m_savewave.EnableWindow(false);
			m_cancel.SetWindowText("Stop");
			
			autostop = Global::pConfig->autoStopMachines;
			if ( Global::pConfig->autoStopMachines )
			{
				Global::pConfig->autoStopMachines = false;
				for (int c=0; c<MAX_MACHINES; c++)
				{
					if (pSong->_pMachine[c])
					{
						pSong->_pMachine[c]->Standby(false);
					}
				}
			}
			playblock = pPlayer->_playBlock;
			loopsong = pPlayer->_loopSong;
			memcpy(sel,pSong->playOrderSel,MAX_SONG_POSITIONS);
			memset(pSong->playOrderSel,0,MAX_SONG_POSITIONS);
			
			CString name;
			m_filename.GetWindowText(name);

			rootname=name;
			rootname=rootname.substr(0,
				std::max(std::string::size_type(0),rootname.length()-4));

			const int real_rate[]={8000,11025,16000,22050,32000,44100,48000,88200,96000};
			const int real_bits[]={8,16,24,32};

			GetDlgItem(IDC_RECSONG)->EnableWindow(false);
			GetDlgItem(IDC_RECPATTERN)->EnableWindow(false);
			GetDlgItem(IDC_RECRANGE)->EnableWindow(false);
			GetDlgItem(IDC_FILEBROWSE)->EnableWindow(false);

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

			if (m_outputtype == 0)
			{	//record to file
				if (m_savetracks.GetCheck())
				{
					memcpy(_Muted,pSong->_trackMuted,sizeof(pSong->_trackMuted));

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
									pSong->_trackMuted[j] = true;
								}
								else
								{
									pSong->_trackMuted[j] = false;
								}
							}
	/*
	similar conversions;
	\operating_system\exception.h(43)
	'std::ostringstream &operator <<(std::ostringstream &,const operating_system::exception &)'
	\include\string(603):
	'std::basic_ostream<_Elem,_Traits> &std::operator <<<char,std::char_traits<char>,std::allocator<_Ty>>(std::basic_ostream<_Elem,_Traits> &,const std::basic_string<_Elem,_Traits,_Ax> &)
	with [_Elem=char,_Traits=std::char_traits<char>,_Ty=char,_Ax=std::allocator<char>]'
	[found using argument-dependent lookup];
	while trying to match the argument list
	'(std::ostringstream, std::string)'
	*/
							// now save the song
							std::ostringstream filename;
							filename << rootname;
							filename << "-track "
								<< std::setprecision(2) << (unsigned)i;
							SaveWav(filename.str().c_str(),real_bits[bits],real_rate[rate],channelmode);
	/*
	'std::ostringstream &operator <<(std::ostringstream &,const operating_system::exception &)'
	'std::basic_ostream<_Elem,_Traits> &std::operator <<<char,std::char_traits<char>,std::allocator<_Ty>>(std::basic_ostream<_Elem,_Traits> &,const std::basic_string<_Elem,_Traits,_Ax> &)
	with [_Elem=char,_Traits=std::char_traits<char>,_Ty=char,_Ax=std::allocator<char>]'
	[found using argument-dependent lookup]; while trying to match the argument list
	'(std::ostringstream, std::string)'
	*/
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
								if (pSong->_pMachine[MASTER_INDEX]->_inputCon[j])
								{
									if (j != i)
									{
										pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[j]]->_mute = true;
									}
									else
									{
										pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[j]]->_mute = false;
									}
								}
							}
							// now save the song
							char filename[MAX_PATH];
							sprintf(filename,"%s-wire %.2u %s.wav",rootname.c_str(),i,pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[i]]->_editName);
							SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
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
						if (pSong->_pMachine[i])
						{
							_Muted[i] = pSong->_pMachine[i]->_mute;
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
								if (pSong->_pMachine[j])
								{
									if (j != i)
									{
										pSong->_pMachine[j]->_mute = true;
									}
									else
									{
										pSong->_pMachine[j]->_mute = false;
									}
								}
							}
							// now save the song
							char filename[MAX_PATH];
							sprintf(filename,"%s-generator %.2u %s.wav",rootname.c_str(),i,pSong->_pMachine[i]->_editName);
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
			else if (m_outputtype == 1)
			{
				//record to clipboard				
			}
			else //m_outputtype == 2
			{
				//record to next free sample slot
			}

		}

		void CSaveWavDlg::SaveWav(std::string file, int bits, int rate, int channelmode)
		{
			saving=true;
			Player *pPlayer = Global::pPlayer;
			Song *pSong = Global::_pSong;
			pPlayer->StopRecording();
			Global::pConfig->_pOutputDriver->Enable(false);
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

			pPlayer->StartRecording(file,bits,rate,channelmode,
									m_dither.GetCheck()==BST_CHECKED && bits!=32, ditherpdf, noiseshape);

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
				hexstring_to_integer(name.GetBuffer(2), pstart);
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
				hexstring_to_integer(name.GetBuffer(2), pstart);
				m_rangeend.GetWindowText(name);
				hexstring_to_integer(name.GetBuffer(2), tmp);
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
			case 3:
				m_patnumber.GetWindowText(name);
				hexstring_to_integer(name.GetBuffer(2), pstart);
				m_linestart.GetWindowText(name);
				hexstring_to_integer(name.GetBuffer(2), blockSLine);
				m_lineend.GetWindowText(name);
				hexstring_to_integer(name.GetBuffer(2), blockELine);

				m_progress.SetRange(blockSLine,blockELine);
				//<alk> what's this for? (start)
				for (cont=0;cont<pSong->playLength;cont++)
				{
					if ( (int)pSong->playOrder[cont] == pstart)
					{
						pstart= cont;
						break;
					}
				}
				//(end)
				lastpostick=pstart;
				pSong->playOrderSel[cont]=true;
				pPlayer->Start(pstart,blockSLine, blockELine);
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
			int stream_size = 8192; // Player has just a single buffer of 65535 samples to allocate both channels
			//int stream_buffer[65535];
			while(!((CSaveWavDlg*)b)->kill_thread)
			{
				if (!pPlayer->_recording) // the player automatically closes the wav recording when looping.
				{
					pPlayer->Stop();
					((CSaveWavDlg*)b)->SaveEnd();
					((CSaveWavDlg*)b)->threadopen--;
					ExitThread(0);
					//return 0;
				}
				pPlayer->Work(pPlayer,stream_size);
				((CSaveWavDlg*)b)->SaveTick();
			}

			pPlayer->Stop();
			pPlayer->StopRecording();
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
			Global::pPlayer->_playBlock=playblock;
			Global::pPlayer->_loopSong=loopsong;
			memcpy(Global::_pSong->playOrderSel,sel,MAX_SONG_POSITIONS);
			Global::pConfig->_pOutputDriver->Enable(true);
			Global::pConfig->_pMidiInput->Open();

			if (m_savetracks.GetCheck())
			{
				Song *pSong = Global::_pSong;

				const int real_rate[]={8000,11025,16000,22050,32000,44100,48000,88200,96000};
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
								pSong->_trackMuted[j] = true;
							}
							else
							{
								pSong->_trackMuted[j] = false;
							}
						}
						// now save the song
						char filename[MAX_PATH];
						sprintf(filename,"%s-track %.2u.wav",rootname.c_str(),i);
						SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
						return;
					}
				}
				memcpy(pSong->_trackMuted,_Muted,sizeof(pSong->_trackMuted));
			}

			else if (m_savewires.GetCheck())
			{
				Song *pSong = Global::_pSong;

				const int real_rate[]={8000,11025,16000,22050,32000,44100,48000,88200,96000};
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
									pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[j]]->_mute = true;
								}
								else
								{
									pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[j]]->_mute = false;
								}
							}
						}
						// now save the song
						char filename[MAX_PATH];
						sprintf(filename,"%s-wire %.2u %s.wav",rootname.c_str(),i,pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[i]]->_editName);
						SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
						return;
					}
				}

				for (int i = 0; i < MAX_CONNECTIONS; i++)
				{
					if (pSong->_pMachine[MASTER_INDEX]->_inputCon[i])
					{
						pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[i]]->_mute = _Muted[i];
					}
				}
			}

			else if (m_savegens.GetCheck())
			{
				Song *pSong = Global::_pSong;

				const int real_rate[]={8000,11025,16000,22050,32000,44100,48000,88200,96000};
				const int real_bits[]={8,16,24,32};

				for (int i = current+1; i < MAX_BUSES; i++)
				{
					if (!_Muted[i])
					{
						current = i;
						for (int j = 0; j < MAX_BUSES; j++)
						{
							if (pSong->_pMachine[j])
							{
								if (j != i)
								{
									pSong->_pMachine[j]->_mute = true;
								}
								else
								{
									pSong->_pMachine[j]->_mute = false;
								}
							}
						}
						// now save the song
						char filename[MAX_PATH];
						sprintf(filename,"%s-generator %.2u %s.wav",rootname.c_str(),i,pSong->_pMachine[i]->_editName);
						SaveWav(filename,real_bits[bits],real_rate[rate],channelmode);
						return;
					}
				}

				for (int i = 0; i < MAX_BUSES; i++)
				{
					if (pSong->_pMachine[i])
					{
						pSong->_pMachine[i]->_mute = _Muted[i];
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

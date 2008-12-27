///\file
///\brief implementation file for psycle::host::CPsycleApp.
#define _WIN32_DCOM

#include "Psycle.hpp"
#include "Version.hpp"
#include "ConfigDlg.hpp"
#include "MainFrm.hpp"
#include "ChildView.hpp"
#include "MidiInput.hpp"
#include "NewMachine.hpp"
#include "SInstance.h"
#include "Loggers.hpp"
#include <universalis/processor/exception.hpp>
#include <diversalis/compiler.hpp>
#include <sstream>
#include <comdef.h>
#include <wbemidl.h>
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	# pragma comment(lib, "wbemuuid")
#endif

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		BEGIN_MESSAGE_MAP(CPsycleApp, CWinApp)
			ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		END_MESSAGE_MAP()

		CPsycleApp theApp; /// The one and only CPsycleApp object

		CPsycleApp::CPsycleApp()
		:m_uUserMessage(0)
		{
			universalis::processor::exception::install_handler_in_thread();
			// support for unicode characters on mswin98
			{
				#if 0
					if(!::LoadLibrary("unicows"))
					{
						std::runtime_error e("could not load library unicows: " + universalis::operating_system::exceptions::code_description());
						MessageBox(0, e.what(), "exception", MB_OK | MB_ICONERROR);
						throw e;
					}
				#endif // 0
			}
		}

		CPsycleApp::~CPsycleApp()
		{
		}

		BOOL CPsycleApp::InitInstance()
		{
			// Allow only one instance of the program
			m_uUserMessage=RegisterWindowMessage("Psycle.exe_CommandLine");
			CInstanceChecker instanceChecker;

			SetRegistryKey(_T("psycle")); // Change the registry key under which our settings are stored.
			
			LoadStdProfileSettings();  // Load standard INI file options (including MRU)
			
			Global::_cpuHz = cpu::cycles_per_second();

			// To create the main window, this code creates a new frame window
			// object and then sets it as the application's main window object.
			CMainFrame* pFrame = new CMainFrame;
			m_pMainWnd = pFrame;

			loggers::info("build identifier: \n" PSYCLE__BUILD__IDENTIFIER("\n"));

			if(!Global::pConfig->Read()) // problem reading registry info. missing or damaged
			{
				Global::pConfig->_initialized = false;
				CConfigDlg dlg("Psycle Settings");
				dlg.Init(Global::pConfig);
				if (dlg.DoModal() == IDOK)
				{
					pFrame->m_wndView._outputActive = true;
					Global::pConfig->_initialized = true;
				}
			}
			else
			{
				pFrame->m_wndView._outputActive = true;
			}
			if (instanceChecker.PreviousInstanceRunning() && !Global::pConfig->_allowMultipleInstances)
			{
//				AfxMessageBox(_T("Previous version detected, will now restore it"), MB_OK);
				HWND prevWnd = instanceChecker.ActivatePreviousInstance();
				if(*(m_lpCmdLine) != 0)
				{
					PostMessage(prevWnd,m_uUserMessage,reinterpret_cast<WPARAM>(m_lpCmdLine),0);
				}
				_global.pConfig->_pOutputDriver->Enable(false);
				///\todo lock/unlock
				Sleep(256);
				_global.pConfig->_pMidiInput->Close();

				return FALSE;
			}


			// create and load the frame with its resources
			pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, 0, 0);

			// Sets Icon
			instanceChecker.TrackFirstInstanceRunning();
			HICON tIcon;
			tIcon=LoadIcon(IDR_MAINFRAME);
			pFrame->SetIcon(tIcon, true);
			pFrame->SetIcon(tIcon, false);
			
			pFrame->m_wndView.LoadMachineSkin();
#ifdef use_test_canvas	
			pFrame->m_wndView.machine_view()->InitSkin();
			pFrame->m_wndView.machine_view()->Rebuild();
#endif
			pFrame->m_wndView.LoadPatternHeaderSkin();
			pFrame->m_wndView.LoadMachineBackground();
			pFrame->m_wndView.RecalcMetrics();
			pFrame->m_wndView.RecalculateColourGrid();


			// The one and only window has been initialized, so show and update it.

			pFrame->ShowWindow(SW_MAXIMIZE);
			
			// center master machine
			pFrame->m_wndView._pSong->_pMachine[MASTER_INDEX]->_x=(pFrame->m_wndView.CW-pFrame->m_wndView.MachineCoords.sMaster.width)/2;
			pFrame->m_wndView._pSong->_pMachine[MASTER_INDEX]->_y=(pFrame->m_wndView.CH-pFrame->m_wndView.MachineCoords.sMaster.width)/2;
			
			pFrame->UpdateWindow();
			
			CNewMachine::LoadPluginInfo(false);

			LoadRecent(pFrame); // Import recent files from registry.

			if (*m_lpCmdLine)
				ProcessCmdLine(m_lpCmdLine); // Process Command Line
			else
			{
				// Show splash screen
				if (Global::pConfig->_showAboutAtStart)
				{
					OnAppAbout();
				}
				pFrame->CheckForAutosave();
			}
			return TRUE;
		}

		BOOL CPsycleApp::PreTranslateMessage(MSG* pMsg)
		{
			if( pMsg->message == m_uUserMessage )
			{
				ProcessCmdLine(reinterpret_cast<LPSTR>(pMsg->wParam));
			}
			return CWinApp::PreTranslateMessage(pMsg);
		}

		// Returning false on WM_TIMER prevents the statusbar from being updated. So it's disabled for now.
		BOOL CPsycleApp::IsIdleMessage( MSG* pMsg )
		{
			if (!CWinApp::IsIdleMessage( pMsg ) || 
				pMsg->message == WM_TIMER) 
			{
//				return FALSE;
			}
//			else
				return TRUE;
		}

		BOOL CPsycleApp::OnIdle(LONG lCount)
		{
			BOOL bMore = CWinApp::OnIdle(lCount);

			///\todo: 
			return bMore;
		}


		/////////////////////////////////////////////////////////////////////////////
		// CPsycleApp message handlers


		int CPsycleApp::ExitInstance() 
		{
			_global.pConfig->Write();
			_global.pConfig->_pOutputDriver->Enable(false);
			///\todo lock/unlock
			Sleep(256);
			_global.pConfig->_pMidiInput->Close();
			CNewMachine::DestroyPluginInfo();
			return CWinApp::ExitInstance();
		}

		void CPsycleApp::ProcessCmdLine(LPSTR cmdline)
		{
			if (*(cmdline) != 0)
			{
				char tmpName [257];
				std::strncpy(tmpName, m_lpCmdLine+1, 256 );
				tmpName[std::strlen(m_lpCmdLine+1) -1 ] = 0;
				reinterpret_cast<CMainFrame*>(m_pMainWnd)->m_wndView.OnFileLoadsongNamed(tmpName, 1);
			}
		}

		void CPsycleApp::LoadRecent(CMainFrame* pFrame)
		{
			//This one should be into Configuration class. It isn't, coz I'm not much
			//into Psycle internal configuration loading routines.
			//If YOU are, go and put it where it sould be put.
			//
			//I know there's a class "Registry" in psycle, but... I don't like using it.
			//I think it's a little bit nonsense to use class that does not nuch more
			//than API itself. The only one for plus is variable encapsulation.
			//
			//Fideloop.
			//
			::HMENU hRootMenuBar, hFileMenu;
			hRootMenuBar = ::GetMenu(pFrame->m_hWnd);
			hFileMenu = ::GetSubMenu(hRootMenuBar, 0);
			pFrame->m_wndView.hRecentMenu = ::GetSubMenu(hFileMenu, 11);

			std::string key(PSYCLE__PATH__REGISTRY__ROOT);
			key += "\\recent-files";
			HKEY RegKey;
			if(::RegOpenKeyEx(HKEY_CURRENT_USER , key.c_str(), 0, KEY_READ, &RegKey) == ERROR_SUCCESS)
			{
				DWORD nValues = 0;
				::RegQueryInfoKey(RegKey, 0, 0, 0, 0, 0, 0, &nValues, 0, 0, 0, 0);
				if(nValues)
				{
					::MENUITEMINFO hNewItemInfo;
					int iCount = 0;
					char cntBuff[3];
					DWORD cntSize = sizeof cntBuff;
					char nameBuff[1 << 10];
					DWORD nameSize = sizeof nameBuff;
					::DeleteMenu(pFrame->m_wndView.hRecentMenu, 0, MF_BYPOSITION);
					while
						(
						::RegEnumValue
						(
						RegKey,
						iCount,
						cntBuff,
						&cntSize,
						0,
						0,
						reinterpret_cast<unsigned char*>(nameBuff),
						&nameSize
						) == ERROR_SUCCESS
						)
					{
						::UINT ids[] =
						{
							ID_FILE_RECENT_01,
								ID_FILE_RECENT_02,
								ID_FILE_RECENT_03,
								ID_FILE_RECENT_04
						};
						hNewItemInfo.cbSize = sizeof hNewItemInfo;
						hNewItemInfo.fMask = MIIM_ID | MIIM_TYPE;
						hNewItemInfo.fType = MFT_STRING;
						hNewItemInfo.wID = ids[iCount];
						hNewItemInfo.cch = std::strlen(nameBuff);
						hNewItemInfo.dwTypeData = nameBuff;
						::InsertMenuItem(pFrame->m_wndView.hRecentMenu, iCount, TRUE, &hNewItemInfo);
						cntSize = sizeof cntBuff;
						nameSize = sizeof nameBuff;
						++iCount;
					}
					::RegCloseKey(RegKey);
				}
			}
		}

		void CPsycleApp::SaveRecent(CMainFrame* pFrame)
		{
			HMENU hRootMenuBar, hFileMenu;
			hRootMenuBar = ::GetMenu(pFrame->m_hWnd);
			hFileMenu = GetSubMenu(hRootMenuBar, 0);
			pFrame->m_wndView.hRecentMenu = GetSubMenu(hFileMenu, 11);

			{
				HKEY RegKey;
				if(::RegOpenKeyEx(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT, 0, KEY_WRITE, &RegKey) == ERROR_SUCCESS)
				{
					::RegDeleteKey(RegKey, "recent-files");
				}
				::RegCloseKey(RegKey);
			}

			std::string key(PSYCLE__PATH__REGISTRY__ROOT);
			key += "\\recent-files";
			HKEY RegKey;
			DWORD Effect;
			if
				(
				::RegCreateKeyEx
				(
				HKEY_CURRENT_USER,
				key.c_str(),
				0,
				0,
				REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS,
				0,
				&RegKey,
				&Effect
				) == ERROR_SUCCESS
				)
			{
				if(GetMenuItemID(pFrame->m_wndView.hRecentMenu, 0) == ID_FILE_RECENT_NONE)
				{
					::RegCloseKey(RegKey);
					return;
				}

				for(int iCount(0) ; iCount < ::GetMenuItemCount(pFrame->m_wndView.hRecentMenu) ; ++iCount)
				{
					UINT nameSize = ::GetMenuString(pFrame->m_wndView.hRecentMenu, iCount, 0, 0, MF_BYPOSITION) + 1;
					char nameBuff[1 << 10];
					::GetMenuString(pFrame->m_wndView.hRecentMenu, iCount, nameBuff, nameSize, MF_BYPOSITION);
					std::ostringstream s;
					s << iCount;
					::RegSetValueEx(RegKey, s.str().c_str(), 0, REG_SZ, reinterpret_cast<unsigned char const *>(nameBuff), nameSize);
				}
				::RegCloseKey(RegKey);
			}
		}

		void CPsycleApp::OnAppAbout()
		{
			CAboutDlg dlg;
			dlg.DoModal();
		}

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		
		CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
		{
			//{{AFX_DATA_INIT(CAboutDlg)
			//}}AFX_DATA_INIT
		}

		void CAboutDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CAboutDlg)
			DDX_Control(pDX, IDC_ASIO, m_asio);
			DDX_Control(pDX, IDC_EDIT5, m_sourceforge);
			DDX_Control(pDX, IDC_EDIT2, m_psycledelics);
			DDX_Control(pDX, IDC_STEINBERGCOPY, m_steincopyright);
			DDX_Control(pDX, IDC_HEADERDLG, m_headerdlg);
			DDX_Control(pDX, IDC_SHOWATSTARTUP, m_showabout);
			DDX_Control(pDX, IDC_HEADER, m_headercontrib);
			DDX_Control(pDX, IDC_ABOUTBMP, m_aboutbmp);
			DDX_Control(pDX, IDC_EDIT1, m_contrib);
			DDX_Control(pDX, IDC_VERSION_INFO_MULTI_LINE, m_versioninfo);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
			//{{AFX_MSG_MAP(CAboutDlg)
			ON_BN_CLICKED(IDC_BUTTON1, OnContributors)
			ON_BN_CLICKED(IDC_SHOWATSTARTUP, OnShowatstartup)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()



		void CAboutDlg::OnContributors() 
		{
			if ( m_aboutbmp.IsWindowVisible() )
			{
				m_aboutbmp.ShowWindow(SW_HIDE);
				m_contrib.ShowWindow(SW_SHOW);
				m_headercontrib.ShowWindow(SW_SHOW);
				m_headerdlg.ShowWindow(SW_SHOW);
				m_psycledelics.ShowWindow(SW_SHOW);
				m_sourceforge.ShowWindow(SW_SHOW);
				m_asio.ShowWindow(SW_SHOW);
				m_steincopyright.ShowWindow(SW_SHOW);
			}
			else 
			{
				m_aboutbmp.ShowWindow(SW_SHOW);
				m_contrib.ShowWindow(SW_HIDE);
				m_headercontrib.ShowWindow(SW_HIDE);
				m_headerdlg.ShowWindow(SW_HIDE);
				m_psycledelics.ShowWindow(SW_HIDE);
				m_sourceforge.ShowWindow(SW_HIDE);
				m_asio.ShowWindow(SW_HIDE);
				m_steincopyright.ShowWindow(SW_HIDE);
			}
		}

		BOOL CAboutDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			m_contrib.SetWindowText
					(
						"Josep Mª Antolín. [JAZ]\t\tDeveloper since release 1.5" "\r\n"
						"Johan Boulé [bohan]\t\tDeveloper since release 1.7.3" "\r\n"
						"James Redfern [alk]\t\tDeveloper and plugin coder" "\r\n"
						"Magnus Jonsson [zealmange]\t\tDeveloper during 1.7.x and 1.9alpha" "\r\n"
						"Jeremy Evers [pooplog]\t\tDeveloper in releases 1.7.x" "\r\n"
						"Daniel Arena\t\t\tDeveloper in release 1.5 & 1.6" "\r\n"
						"Marcin Kowalski [FideLoop]\t\tDeveloper in release 1.6" "\r\n"
						"Mark McCormack\t\t\tMIDI (in) Support in release 1.6" "\r\n"
						"Mats Höjlund\t\t\tMain Developer until release 1.5" "\r\n" // (Internal Recoding) .. doesn't fit in the small box :/
						"Juan Antonio Arguelles [Arguru] (RIP)\tOriginal Developer and maintainer until 1.0" "\r\n"
						"Satoshi Fujiwara\t\t\tBase code for the Sampulse machine\r\n"
						"Hermann Seib\t\t\tBase code of the new VSTHost in 1.8.5\r\n"
						"Martin Etnestad Johansen [lobywang]\tCoding Help" "\r\n"
						"Patrick Haworth [TranceMyriad]\tAuthor of the Help File" "\r\n"
						"Hamarr Heylen\t\t\tInitial Graphics" "\r\n"
						"David Buist\t\t\tAdditional Graphics" "\r\n"
						"frown\t\t\t\tAdditional Graphics" "\r\n"
						"/\\/\\ark\t\t\t\tAdditional Graphics" "\r\n"
						"Haralabos Michael\t\t\tInstaller and Debugging help" "\r\n\r\n"
						"This release of Psycle also contains VST plugins from:" "\r\n"
						"Digital Fish Phones\t( http://www.digitalfishphones.com/ )" "\r\n"
						"DiscoDSP\t\t( http://www.discodsp.com/ )" "\r\n"
						"SimulAnalog\t( http://www.simulanalog.org/ )" "\r\n"
						"Jeroen Breebaart\t( http://www.jeroenbreebaart.com/ )" "\r\n"
						"George Yohng\t( http://www.yohng.com/ )" "\r\n"
						"Christian Budde\t( http://www.savioursofsoul.de/Christian/ )" "\r\n"
						"DDMF\t\t( http://www.ddmf.eu/ )" "\r\n"
						"Loser\t\t( http://loser.asseca.com/ )" "\r\n"
						"E-phonic\t\t( http://www.e-phonic.com/ )" "\r\n"
						"Argu\t\t( http://www.aodix.com/ )" "\r\n"
						"Oatmeal by Fuzzpilz\t( http://bicycle-for-slugs.org/ )"
					);
			m_showabout.SetCheck(Global::pConfig->_showAboutAtStart);

			m_psycledelics.SetWindowText("http://psycle.pastnotecut.org");
			m_sourceforge.SetWindowText("http://psycle.sourceforge.net");
			m_versioninfo.SetWindowText(PSYCLE__BUILD__IDENTIFIER("\r\n"));

			// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
			return TRUE;
		}

		void CAboutDlg::OnShowatstartup() 
		{
			if ( m_showabout.GetCheck() )  Global::pConfig->_showAboutAtStart = true;
			else Global::pConfig->_showAboutAtStart=false;
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

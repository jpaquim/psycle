///\file
///\brief implementation file for psycle::host::CPsycleApp.
#define _WIN32_DCOM
#include <project.private.hpp>
#include "psycle.hpp"
#include "version.hpp"
#include "ConfigDlg.hpp"
#include "MainFrm.hpp"
#include "midiinput.hpp"
#include "NewMachine.hpp"
#include <operating_system/exception.hpp>
#include <sstream>
#include <comdef.h>
#include <Wbemidl.h>
#include "sinstance.h"

# pragma comment(lib, "wbemuuid.lib")

NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)

		BEGIN_MESSAGE_MAP(CPsycleApp, CWinApp)
			ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		END_MESSAGE_MAP()

		CPsycleApp theApp; /// The one and only CPsycleApp object

		CPsycleApp::CPsycleApp()
		:m_uUserMessage(0)
		{
			operating_system::exceptions::translated::new_thread("mfc gui");
			// support for unicode characters on mswin98
			{
				#if 0
					if(!::LoadLibrary("unicows"))
					{
						std::runtime_error e("could not load library unicows: " + operating_system::exceptions::code_description());
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

			SetRegistryKey(_T("AAS")); // Change the registry key under which our settings are stored.
			
			LoadStdProfileSettings();  // Load standard INI file options (including MRU)
			
			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency);
			Global::_cpuHz = frequency.QuadPart;

			// To create the main window, this code creates a new frame window
			// object and then sets it as the application's main window object.
			CMainFrame* pFrame = new CMainFrame;
			m_pMainWnd = pFrame;

			host::loggers::info("build identifier: " EOL PSYCLE__BUILD__IDENTIFIER(EOL));

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
				Sleep(LOCK_LATENCY);
				_global.pConfig->_pMidiInput->Close();

				return FALSE;
			}


			// create and load the frame with its resources
			pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, 0, 0);
				// For some reason, there'a First-Chance exception when
				// another pFrame member is called after this LoadFrame
				// (for example, pFrame->ShowWindow(SW_MAXIMIZE);)

			// Sets Icon
			HICON tIcon;
			tIcon=LoadIcon(IDR_MAINFRAME);
			pFrame->SetIcon(tIcon, true);
			pFrame->SetIcon(tIcon, false);
			
			pFrame->m_wndView.LoadMachineSkin();
			pFrame->m_wndView.LoadPatternHeaderSkin();
			pFrame->m_wndView.LoadMachineBackground();
			pFrame->m_wndView.RecalcMetrics();
			pFrame->m_wndView.RecalculateColourGrid();

			instanceChecker.TrackFirstInstanceRunning();

			// The one and only window has been initialized, so show and update it.

			pFrame->ShowWindow(SW_MAXIMIZE);
			
			// center master machine
			pFrame->m_wndView._pSong->_pMachine[MASTER_INDEX]->_x=(pFrame->m_wndView.CW-pFrame->m_wndView.MachineCoords.sMaster.width)/2;
			pFrame->m_wndView._pSong->_pMachine[MASTER_INDEX]->_y=(pFrame->m_wndView.CH-pFrame->m_wndView.MachineCoords.sMaster.width)/2;
			
			pFrame->UpdateWindow();
			
			CNewMachine::LoadPluginInfo();

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
			Sleep(LOCK_LATENCY);
			_global.pConfig->_pMidiInput->Close();
			CNewMachine::DestroyPluginInfo();
			return CWinApp::ExitInstance();
		}

		/////////////////////////////////////////////////////////////////////////////
		// CPsycleApp naive method to get the CPU frequency


		void CPsycleApp::GetNaiveCPUFreq()
		{
			LARGE_INTEGER cost,next;
			QueryPerformanceCounter(&cost);
			Sleep(1000);
			QueryPerformanceCounter(&next);
			Global::_cpuHz = next.QuadPart - cost.QuadPart;
/*			
			ULONG cpuHz;
			__asm rdtsc ///< read time stamp to EAX
			__asm mov cpuHz, eax
			Sleep(1000);
			__asm rdtsc
			__asm sub eax, cpuHz ///< Find the difference
			__asm mov cpuHz, eax
			Global::_cpuHz = cpuHz;
*/
		}
		void CPsycleApp::ProcessCmdLine(LPSTR cmdline)
		{
			if (*(cmdline) != 0)
			{
				char * tmpName =new char[257];
				strncpy(tmpName, m_lpCmdLine+1, 256 );
				tmpName[strlen(m_lpCmdLine+1) -1 ] = 0;
				reinterpret_cast<CMainFrame*>(m_pMainWnd)->m_wndView.OnFileLoadsongNamed(tmpName, 1);
				zapArray(tmpName);
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
			pFrame->m_wndView.hRecentMenu = ::GetSubMenu(hFileMenu, 10);

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
			pFrame->m_wndView.hRecentMenu = GetSubMenu(hFileMenu, 10);

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
						"Josep M� Antol�n. [JAZ]\t\tDeveloper since release 1.5" "\r\n"
						"Jeremy Evers [pooplog]\t\tDeveloper since release 1.7" "\r\n"
						"James Redfern [alk]\t\tDeveloper since release 1.7.6" "\r\n"
						"Johan Boul� [bohan]\t\tDeveloper since release 1.7.3" "\r\n"
						"Magnus Jonsson [zealmange]\t\tDeveloper since release 1.7.7" "\r\n"
						"Daniel Arena\t\t\tDeveloper in release 1.5 & 1.6" "\r\n"
						"Marcin Kowalski [FideLoop]\t\tDeveloper in release 1.6" "\r\n"
						"Mark McCormack\t\t\tMIDI (in) Support" "\r\n"
						"Mats H�jlund\t\t\tMain Developer until release 1.5" "\r\n"
						"Juan Antonio Arguelles [Arguru]\tCreator and Main Developer until release 1.0" "\r\n" // (Internal Recoding) .. doesn't fit in the small box :/
						"Satoshi Fujiwara\t\t\tInitial code on the XM sampler support\r\n"
						"Martin Etnestad Johansen [lobywang]\tCoding Help" "\r\n"
						"Patrick Haworth [TranceMyriad]\tAuthor of the Help File" "\r\n"
						"Hamarr Heylen\t\t\tInitial Graphics" "\r\n"
						"David Buist\t\t\tAdditional Graphics" "\r\n"
						"frown\t\t\t\tAdditional Graphics" "\r\n"
						"/\\/\\ark\t\t\t\tAdditional Graphics" "\r\n"
						"Haralabos Michael\t\t\tInstaller and Debugging help"
					);
			m_showabout.SetCheck(Global::pConfig->_showAboutAtStart);

			m_psycledelics.SetWindowText("http://psycle.pastnotecut.org");
			m_sourceforge.SetWindowText("http://psycle.sourceforge.net");
			m_versioninfo.SetWindowText(PSYCLE__BUILD__IDENTIFIER(EOL));

			// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
			return TRUE;
		}

		void CAboutDlg::OnShowatstartup() 
		{
			if ( m_showabout.GetCheck() )  Global::pConfig->_showAboutAtStart = true;
			else Global::pConfig->_showAboutAtStart=false;
		}

	NAMESPACE__END
NAMESPACE__END
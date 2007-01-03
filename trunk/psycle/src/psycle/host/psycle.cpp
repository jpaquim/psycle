// This file is part of Psycle, which is the legal property of its developers, copyright 2000-2007 the psycle development team, whose names are too numerous to list here. Please refer to the AUTHORS file distributed with this source distribution.
 
///\file
///\brief implementation file for psycle::host::CPsycleApp.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include "psycle.hpp"
#include "ConfigDlg.hpp"
#include "MainFrm.hpp"
//#include <psycle/engine/midiinput.hpp>
#include <universalis/processor/exception.hpp>
#include <sstream>

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		class CAboutDlg : public CDialog
		{
		public:
			CAboutDlg();
			enum { IDD = IDD_ABOUTBOX };
			CStatic	m_asio;
			CEdit	m_sourceforge;
			CEdit	m_psycledelics;
			CStatic	m_steincopyright;
			CStatic	m_headerdlg;
			CButton	m_showabout;
			CStatic	m_headercontrib;
			CStatic	m_aboutbmp;
			CEdit	m_contrib;
			CStatic m_versioninfo;
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			afx_msg void OnContributors();
			virtual BOOL OnInitDialog();
			afx_msg void OnShowatstartup();
			DECLARE_MESSAGE_MAP()
		};

		BEGIN_MESSAGE_MAP(CPsycleApp, CWinApp)
			ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		END_MESSAGE_MAP()

		CPsycleApp::CPsycleApp()
		{
			universalis::processor::exception::new_thread("mfc gui");
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

		CPsycleApp theApp; /// The one and only CPsycleApp object

		BOOL CPsycleApp::InitInstance()
		{
			{
				///\todo check this
				SetRegistryKey(PSYCLE__PATH__REGISTRY__ROOT "\\" PSYCLE__PATH__REGISTRY__CONFIGKEY); // Change the registry key under which our settings are stored.
				LoadStdProfileSettings();  // Load standard INI file options (including MRU)
			}
			
			// To create the main window, this code creates a new frame window
			// object and then sets it as the application's main window object.
			CMainFrame* pFrame = new CMainFrame;
			m_pMainWnd = pFrame;

			{
				std::ostringstream s;
				s << "build identifier: " << std::endl << PSYCLE__BUILD__IDENTIFIER;
				host::loggers::info(s.str());
			}

			if(!Global::configuration().Read()) // problem reading registry info. missing or damaged
			{
				Global::configuration()._initialized = false;
				CConfigDlg dlg("Psycle Settings");
				dlg.Init(Global::pConfig);
				if (dlg.DoModal() == IDOK)
				{
					pFrame->m_wndView._outputActive = true;
					Global::configuration()._initialized = true;
				}
			}
			else
			{
				pFrame->m_wndView._outputActive = true;
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

			// The one and only window has been initialized, so show and update it.

			pFrame->ShowWindow
				(
					#if defined NDEBUG // boring when developping
						SW_MAXIMIZE
					#else
						SW_NORMAL
					#endif
				);
			
			// center master machine
			pFrame->m_wndView._pSong->_pMachine[MASTER_INDEX]->SetPosX((pFrame->m_wndView.CW-pFrame->m_wndView.MachineCoords.sMaster.width)/2);
			pFrame->m_wndView._pSong->_pMachine[MASTER_INDEX]->SetPosY((pFrame->m_wndView.CH-pFrame->m_wndView.MachineCoords.sMaster.width)/2);
			
			pFrame->UpdateWindow();
			
			#if defined NDEBUG // boring when developping
				// Show splash screen
				// If has been commented out for non-stable builds..
				//if (Global::configuration()._showAboutAtStart)
					OnAppAbout();
			#endif
			
			ProcessCmdLine(pFrame); // Process Command Line

			LoadRecent(pFrame); // Import recent files from registry.

			return true;
		}

		/////////////////////////////////////////////////////////////////////////////
		// CPsycleApp message handlers


		int CPsycleApp::ExitInstance() 
		{
			_global.pConfig->Write();
			return CWinApp::ExitInstance();
		}

		/////////////////////////////////////////////////////////////////////////////
		// CAboutDlg dialog used for App About

		CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
		{
		}

		void CAboutDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
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
		}

		BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
			ON_BN_CLICKED(IDC_BUTTON1, OnContributors)
			ON_BN_CLICKED(IDC_SHOWATSTARTUP, OnShowatstartup)
		END_MESSAGE_MAP()


		/////////////////////////////////////////////////////////////////////////////
		// CPsycleApp message handlers


		void CPsycleApp::OnAppAbout()
		{
			CAboutDlg dlg;
			dlg.DoModal();
		}


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
						"Jeremy Evers [pooplog]\t\tDeveloper since release 1.7" "\r\n"
						"James Redfern [alk]\t\tDeveloper since release 1.7.6" "\r\n"
						"Johan Boulé [bohan]\t\tDeveloper since release 1.7.3" "\r\n"
						"Magnus Jonsson [zealmange]\t\tDeveloper since release 1.7.7" "\r\n"
						"Daniel Arena\t\t\tDeveloper in release 1.5 & 1.6" "\r\n"
						"Marcin Kowalski [FideLoop]\t\tDeveloper in release 1.6" "\r\n"
						"Mark McCormack\t\t\tMIDI (in) Support" "\r\n"
						"Mats Höjlund\t\t\tMain Developer until release 1.5" "\r\n"
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
			m_showabout.SetCheck(Global::configuration()._showAboutAtStart);

			m_psycledelics.SetWindowText("http://psycle.pastnotecut.org");
			m_sourceforge.SetWindowText("http://psycle.sourceforge.net");
			m_versioninfo.SetWindowText(PSYCLE__BUILD__IDENTIFIER);

			// return true unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return false
			return true;
		}

		void CAboutDlg::OnShowatstartup() 
		{
			if ( m_showabout.GetCheck() )  Global::configuration()._showAboutAtStart = true;
			else Global::configuration()._showAboutAtStart=false;
		}

		void CPsycleApp::ProcessCmdLine(CMainFrame* pFrame)
		{
			if(*(m_lpCmdLine) != 0)
			{
				char * tmpName =new char[257];
				strncpy(tmpName, m_lpCmdLine+1, strlen(m_lpCmdLine+1) -1 );
				tmpName[strlen(m_lpCmdLine+1) -1 ] = 0;
				pFrame->m_wndView.OnFileLoadsongNamed(tmpName, 1);
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
						::InsertMenuItem(pFrame->m_wndView.hRecentMenu, iCount, true, &hNewItemInfo);
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
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END

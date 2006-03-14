#pragma once
#include "iController.h"

namespace view {
	class MainFrame;
}

namespace model {
	namespace detail {
		static const TCHAR SATELLITE_DLL_NAME[] = _T("\\PsycleWTLRes.dll"); 

		typedef struct LANGINFO_DEF {
			int		Count;
			LANGID	LangID;
		} LANGINFO;

		typedef LANGINFO *PLANGINFO;

//		void DeleteObject(iDisposable* p);

		template < template < class > class SmartPtrType = boost::shared_ptr >
		class Controller :
			public iController
		{

		public:
			Controller() : m_pResampler(new Cubic())
			{
				m_bOutputActive = false;

				m_pResampler->SetQuality(RESAMPLE_LINEAR);
				//m_pSong = new ::Song(this);
				//m_pPlayer = new ::Player();
				//m_pConfig = new ::configuration::configuration(this);
				
				TCHAR _dir[MAX_PATH];
				::GetCurrentDirectory(MAX_PATH,_dir);
				m_CurrentDirectory = _dir;
				m_CurrentDirectory += _T("\\");

			#ifndef _CYRIX_PROCESSOR_

				ULONG cpuHz;
				__asm	rdtsc				// Read time stamp to EAX
				__asm	mov	cpuHz, eax

				Sleep(1000);

				__asm	rdtsc
				__asm	sub		eax,cpuHz	// Find the difference
				__asm	mov		cpuHz, eax

				m_CpuHz = cpuHz;
			#else
				m_CpuHz = 1;
			#endif // _CYRIX_PROCESSOR_

				// TODO: ドライバー列挙コードの再構築を行う

				_numOutputDrivers = 4;// decrease it if no asio
			//	_numOutputDrivers = 1;// decrease it if no asio
				_outputDriverIndex = 2;

				audiodriver::iAudioDriver* _paudio_driver  = new audiodriver::DummyAudioDriver(this);

				m_OutputDrivers[_paudio_driver->DriverName].reset(_paudio_driver,&DeleteObject);
				_paudio_driver  = new audiodriver::DirectSound(this);
				m_OutputDrivers[_paudio_driver->DriverName].reset(_paudio_driver,&DeleteObject);

				//	_ppOutputDrivers = new iAudioDriver*[_numOutputDrivers];

				//	_ppOutputDrivers[0] = new ::DummyAudioDriver(this);
				//	_ppOutputDrivers[1] = new WaveOut;
				//	_ppOutputDrivers[2] = new ::DirectSound(this);
				//	_ppOutputDrivers[3] = new ASIOInterface;
				
				//if (((ASIOInterface*)(_ppOutputDrivers[3]))->drivercount <= 0)
				//{
				//	_numOutputDrivers--;
				//	delete _ppOutputDrivers[3];
				//}
				
				// TODO とりあえず、あとできちんと実装する
				m_pOutputDriver = _paudio_driver;

			//	_pMidiInput = new CMidiInput;	// MIDI IMPLEMENTATION
			};

			HRESULT Run(ULONG hInstance,BSTR lpstrCmdLine,LONG nCmdShow)
			{

				HMODULE hResDll;
				//hResDll = ::LoadLibrary( L"1033\\PsycleWTLRes.dll") ; // Japanese

				hResDll = LoadSatelliteDLL(DetectLanguage());
				if(hResDll == NULL)
				{
					hResDll = ::LoadLibrary( L"1033\\PsycleWTLRes.dll");
				}
				
				win32::gui::detail::res_handle::instance().init(hResDll);

				_AtlBaseModule.SetResourceInstance(hResDll);	

				AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls
				

			//	hRes = MessageLoops.Init(NULL, hInstance);
			//	ATLASSERT(SUCCEEDED(hRes));

			//	CComPtr<ISkinX>	spISkinX;
			//	spISkinX.CoCreateInstance(CLSID_MacSkin);
			//	if (spISkinX)
			//		spISkinX->InstallSkin(MessageLoops.m_dwMainThreadID);

			//	int nRet = Run(lpstrCmdLine, nCmdShow);

				OSVERSIONINFO osvi;
				osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

				GetVersionEx(&osvi); 
				if (!((osvi.dwMinorVersion >= 1 && osvi.dwMajorVersion >= 5) || (osvi.dwMajorVersion > 5)))
				{
					::MessageBox(NULL,SF::CResourceString(IDS_ERR_MSG0096),SF::CResourceString(IDS_ERR_MSG0097),0);
					return 0;
				}
				
				// GDI+ STartup

				Gdiplus::GdiplusStartupInput gdiplusStartupInput;
				ULONG_PTR           gdiplusToken;
				Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

				//wnd<view::MainFrame> m_wnd = 
				//	create_wnd<view::MainFrame,iController,string>(
				//			*this,
				//			string(_T("Psycle Featuring Win32 GUI Generics")),
				//			null_wnd,
				//			create_info()
				//			.menu(IDR_MAINFRAME)
				//	);

				//m_wnd->wait();

				m_pOutputDriver->Configuration()->Configure();

				//m_pMainFrame = m_wnd.;

				//_pmain->CreateEx();
				//_pmain->ShowWindow(nCmdShow);

				
				//SetRegistryKey(_T("AAS"));
				
				//LoadStdProfileSettings();  // Load standard INI file options (including MRU)
				
				// To create the main window, this code creates a new frame window
				// object and then sets it as the application's main window object.
				
				// Set Custom Window Class
				//SF::UI::Volume::GetWndClassInfo().Register((WNDPROC *)(&::DefWindowProc));
				
				//if(wndMain.CreateEx() == NULL)
				//{
				//	ATLTRACE(_T("Main window creation failed!\n"));
				//	return 0;
				//}

				//if (!Global::pConfig->Read()) // problem reading registry info. missing or damaged
				//{
				//	Global::pConfig->_initialized = false;
				//	CConfigDlg dlg(_T("Psycle configuration"));
				//	dlg.Init(Global::pConfig);
				//	if (dlg.DoModal() == IDOK)
				//	{
				//		wndMain.m_view._outputActive = true;
				//		Global::pConfig->_initialized = true;
				//	}
				//}
				//else
				//{
				//	wndMain.m_view._outputActive = true;
				//}

				//wndMain.InitSequencer();

				//wndMain.ShowWindow(nCmdShow);
				//// Sets Icon
				//HICON tIcon;
				//tIcon = LoadIcon(_AtlBaseModule.GetModuleInstance(),MAKEINTRESOURCE(IDR_MAINFRAME));
				//wndMain.SetIcon(tIcon,true);
				//wndMain.SetIcon(tIcon,false);
				//
				//wndMain.m_view.LoadMachineSkin();
				//wndMain.m_view.LoadPatternHeaderSkin();
				//wndMain.m_view.LoadMachineBackground();
				//wndMain.m_view.RecalcMetrics();
				//wndMain.m_view.RecalculateColourGrid();
				//// The one and only window has been initialized, so show and update it.
				//wndMain.ShowWindow(SW_MAXIMIZE);
				//
				//// center master machine
				//wndMain.m_view._pSong->pMachine(MASTER_INDEX)->_x
				//	= (wndMain.m_view.CW-wndMain.m_view.MachineCoords.sMaster.width) / 2;
				//wndMain.m_view._pSong->pMachine(MASTER_INDEX)->_y
				//	= (wndMain.m_view.CH-wndMain.m_view.MachineCoords.sMaster.width) / 2;
				//wndMain.UpdateWindow();
				//
				//CNewMachine::LoadPluginInfo();
				//// Show splash screen
				//if (Global::pConfig->_showAboutAtStart)
				//{
				//	CAboutDlg dlg;
				//	dlg.DoModal();
				//}
				
				//Import recent files from registry.
			//	wndMain.LoadRecentConfig();
				//wndMain.LoadRecent();

				//ManagedWindowFactory _t;
				
				//_t.CreateManagedWindow(wndMain.m_view,_T("temp\\XMSamplerUI.dll"),_T("SF.XMSamplerUITest"));


				//int nRet = theLoop.Run();

				//testmain();

				//CMessageLoopContainer::Instance().RemoveMessageLoop();
				
				//_pmain->DestroyWindow();
				//
				//_global->pConfig->Write();
				//_global->pConfig->_pOutputDriver->Enable(false);
				//::Sleep(LOCK_LATENCY);
				//_global->pConfig->_pMidiInput->Close();
				//CNewMachine::DestroyPluginInfo();

				//delete _global;	

				Gdiplus::GdiplusShutdown(gdiplusToken);

			//	spISkinX.Release();

			//	CMessageLoopContainer::Instance().Term();

			// プログラムの最後でコールする
				//SF::UI::Volume<int>::Release();

				return S_OK;
			};


			IConfiguration* const Configuration(){return m_pConfig;};

			iSong* const Song(){return m_pSong.get();};

			iPlayer* const Player(){return m_pPlayer.get();};
			iResampler* const Resampler(){return m_pResampler.get();};
			view::iMainFrame* const MainFrame(){return m_pMainFrame;};

			void UpdatePlayOrder(const bool mode){;};
			void UpdateSequencer(){;};

			void SelectFile(string& inputFileName){;};
			void InitializeBeforeLoading(){;};
			void ProcessAfterLoading(){;};

			void AddBpm(const int addValue){;};
			void AddTpb(const int addValue){;};

			void NewFile(){;};
			void SaveFile(const bool bFileSaveAs){;};

			const bool IsOutputActive() const {return  m_bOutputActive;};
			void IsOutputActive(const bool bActive){m_bOutputActive = bActive;};

			const ULONG CpuHz() const {return m_CpuHz;};

			const int SamplesPerSec() const
			{
				return m_pOutputDriver->SamplesPerSec;
			};

			audiodriver::iAudioDriver* const OutputDriver() const {return m_pOutputDriver;};
			
			IMidiInput* const MidiInput() const {return m_pMidiInput;};	// MIDI IMPLEMENTATION
			const string& CurrentDirectory() const {return m_CurrentDirectory;};


		private:
			//HMODULE	LoadSatelliteDLL(LANGID DesiredLanguage) ;
			///**  The following functions contain code to
			//detect the language in which the initial
			//user interface should be displayed */
			//static BOOL CALLBACK EnumLangProc(HANDLE hModule, LPCTSTR lpszType, LPCTSTR lpszName,
			//					WORD wIDLanguage, LONG_PTR lParam);

			///** Detects the language of ntdll.dll with some specific processing for 
			//the Hongkong SAR version */
			//LANGID GetNTDLLNativeLangID();
			///**  Checks if NT4 system is Hongkong SAR version */
			//BOOL IsHongKongVersion();
			///**  This function detects a correct initial UI language for all
			//platforms (Win9x, ME, NT4, Windows 2000, Windows XP) */
			//LANGID DetectLanguage();

			HMODULE		LoadSatelliteDLL(LANGID DesiredLanguage) {
				TCHAR		BaseDirectory[MAX_PATH];
				TCHAR		SatellitePath[MAX_PATH];
				TCHAR		buffer[100];
				HMODULE		hDLL;

				// Get the base directory for the satellite DLL search
				if( GetCurrentDirectory(MAX_PATH, BaseDirectory)) {
					_tcscat(BaseDirectory, _T("\\"));
				}
				else
					return NULL;

				// First try to load the library with the fully specified language
				_tcscpy(SatellitePath, BaseDirectory);
				_itot(DesiredLanguage,buffer,10);
				_tcscat(SatellitePath, buffer);
				_tcscat(SatellitePath, SATELLITE_DLL_NAME);
				if( hDLL = LoadLibrary(SatellitePath) )
					return hDLL;
				else {   // try the primary language ID
					_tcscpy(SatellitePath, BaseDirectory);
					DesiredLanguage = PRIMARYLANGID(DesiredLanguage);
					_itot(DesiredLanguage,buffer,10);
					_tcscat(SatellitePath, buffer);
					_tcscat(SatellitePath, SATELLITE_DLL_NAME);
					if( hDLL = LoadLibrary(SatellitePath) )
						return hDLL;
					else
						return NULL;
				}
			}

			// The following functions contain code to
			// detect the language in which the initial
			// user interface should be displayed
			static BOOL EnumLangProc(HANDLE hModule, LPCTSTR lpszType, LPCTSTR lpszName,
									WORD wIDLanguage, LONG_PTR lParam)
			{
				PLANGINFO LangInfo;

				LangInfo = (PLANGINFO) lParam;
				LangInfo->Count++;
				LangInfo->LangID  = wIDLanguage;

				return (TRUE);        // continue enumeration
			};

			// Detects the language of ntdll.dll with some specific processing for 
			// the Hongkong SAR version
			LANGID GetNTDLLNativeLangID()
			{

				LANGINFO LangInfo;
				LPCTSTR Type = (LPCTSTR) ((LPVOID)((WORD)16));
				LPCTSTR Name = (LPCTSTR) 1;

				ZeroMemory(&LangInfo,sizeof(LangInfo));
			    
				// Get the HModule for ntdll.
				HMODULE hMod = GetModuleHandle(_T("ntdll.dll"));
				if (hMod==NULL) {
					return(0);
				}

				BOOL result = EnumResourceLanguages(hMod, Type, Name, (ENUMRESLANGPROC)EnumLangProc, (LONG_PTR) &LangInfo);
			    
				if (!result || (LangInfo.Count > 2) || (LangInfo.Count < 1) ) {
					return (0);
				}
			    
				return (LangInfo.LangID);
			};

			// Checks if NT4 system is Hongkong SAR version
			BOOL IsHongKongVersion()
			{
				HMODULE hMod;
				BOOL bRet=FALSE;
				typedef BOOL (WINAPI *IMMRELEASECONTEXT)(HWND,HIMC);
				IMMRELEASECONTEXT pImmReleaseContext;

				hMod = LoadLibrary(_T("imm32.dll"));
				if (hMod) {
					pImmReleaseContext = (IMMRELEASECONTEXT)GetProcAddress(hMod,"ImmReleaseContext");
					if (pImmReleaseContext) {
						bRet = pImmReleaseContext(NULL,NULL);
					}
					FreeLibrary(hMod);
				}
				return (bRet);
			};

			// This function detects a correct initial UI language for all
			// platforms (Win9x, ME, NT4, Windows 2000, Windows XP)
			LANGID DetectLanguage() {

			#define MAX_KEY_BUFFER	80

				OSVERSIONINFO		VersionInfo;
				LANGID				uiLangID = 0;
				HKEY				hKey;
				DWORD				Type, BuffLen = MAX_KEY_BUFFER;
				TCHAR				LangKeyValue[MAX_KEY_BUFFER];


				VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
				if( !GetVersionEx(&VersionInfo) )
					return(0);

				switch( VersionInfo.dwPlatformId ) {
					// On Windows NT, Windows 2000 or higher
					case VER_PLATFORM_WIN32_NT:
						if( VersionInfo.dwMajorVersion >= 5)   // Windows 2000 or higher
							uiLangID = GetUserDefaultUILanguage();
						else {   // for NT4 check the language of ntdll.dll
							uiLangID = GetNTDLLNativeLangID();   
							if (uiLangID == 1033) {		// special processing for Honkong SAR version of NT4
								if (IsHongKongVersion()) {
									uiLangID = 3076;
								}
							}
						}
						break;
					// On Windows 95, Windows 98 or Windows ME
					case VER_PLATFORM_WIN32_WINDOWS:
						// Open the registry key for the UI language
						if( RegOpenKeyEx(HKEY_CURRENT_USER,_T("Default\\Control Panel\\Desktop\\ResourceLocale"), 0,
							KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS ) {
							// Get the type of the default key
							if( RegQueryValueEx(hKey, NULL, NULL, &Type, NULL, NULL) == ERROR_SUCCESS 
								&& Type == REG_SZ ) {
								// Read the key value
								if( RegQueryValueEx(hKey, NULL, NULL, &Type, (LPBYTE)LangKeyValue, &BuffLen) 
									== ERROR_SUCCESS ) {
									uiLangID = _ttoi(LangKeyValue);
								}
							}
							RegCloseKey(hKey);
						}				
						break;
				}

				if (uiLangID == 0) {
					uiLangID = GetUserDefaultLangID();
				}
				// Return the found language ID.
				return (uiLangID);
			};

			ULONG m_CpuHz;
			
			IConfiguration *m_pConfig;

			SmartPtrType<iSong> m_pSong;
			SmartPtrType<iPlayer> m_pPlayer;
			SmartPtrType<iResampler> m_pResampler;
			
			::view::iMainFrame* m_pMainFrame;

			//InputHandler m_InputHandler;
			string m_CurrentDirectory;
			bool m_bOutputActive;
			int _numOutputDrivers;
			int _outputDriverIndex;
			typedef std::map<string,SmartPtrType<audiodriver::iAudioDriver> > OutputDriverMap;
			OutputDriverMap m_OutputDrivers;

		//	iAudioDriver** _ppOutputDrivers;
			audiodriver::iAudioDriver* m_pOutputDriver;
			IMidiInput* m_pMidiInput;	// MIDI IMPLEMENTATION

			int _samplesPerSec;

			//static std::auto_ptr<Controller> m_pController;
			static boost::mutex m_mutex;
		};

	}// namespace detail

typedef detail::Controller<boost::shared_ptr> Controller;

}

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

#include "Dsp.h"
#include "AudioDriver.h"
//#include "WaveOut.h"
#include "DirectSound.h"
//#include "ASIOInterface.h"
#include "testcontroller.h"
#include "TestMainFrm.h"

// CMainObject
using namespace SF;

std::auto_ptr<CMessageLoopContainer> CMessageLoopContainer::m_pMessageLoops;

boost::mutex CMessageLoopContainer::m_mutex;


namespace model {
	namespace detail {
	//template < template < class > class SmartPtrType >
	//	Controller<SmartPtrType>::Controller() : m_pResampler(new Cubic(),&DeleteObject)
	//{
	//	m_bOutputActive = false;

	//	m_pResampler->SetQuality(RESAMPLE_LINEAR);
	//	//m_pSong = new ::Song(this);
	//	//m_pPlayer = new ::Player();
	//	//m_pConfig = new ::configuration::configuration(this);
	//	
	//	TCHAR _dir[MAX_PATH];
	//	::GetCurrentDirectory(MAX_PATH,_dir);
	//	m_CurrentDirectory = _dir;
	//	m_CurrentDirectory += _T("\\");

	//#ifndef _CYRIX_PROCESSOR_

	//	ULONG cpuHz;
	//	__asm	rdtsc				// Read time stamp to EAX
	//	__asm	mov	cpuHz, eax

	//	Sleep(1000);

	//	__asm	rdtsc
	//	__asm	sub		eax,cpuHz	// Find the difference
	//	__asm	mov		cpuHz, eax

	//	m_CpuHz = cpuHz;
	//#else
	//	m_CpuHz = 1;
	//#endif // _CYRIX_PROCESSOR_

	//	// TODO: ドライバー列挙コードの再構築を行う

	//	_numOutputDrivers = 4;// decrease it if no asio
	////	_numOutputDrivers = 1;// decrease it if no asio
	//	_outputDriverIndex = 2;

	//	audiodriver::iAudioDriver* _paudio_driver  = new audiodriver::DummyAudioDriver(this);

	//	m_OutputDrivers[_paudio_driver->DriverName].reset(_paudio_driver,&DeleteObject);
	//	_paudio_driver  = new audiodriver::DirectSound(this);
	//	m_OutputDrivers[_paudio_driver->DriverName].reset(_paudio_driver,&DeleteObject);

	//	//	_ppOutputDrivers = new iAudioDriver*[_numOutputDrivers];

	//	//	_ppOutputDrivers[0] = new ::DummyAudioDriver(this);
	//	//	_ppOutputDrivers[1] = new WaveOut;
	//	//	_ppOutputDrivers[2] = new ::DirectSound(this);
	//	//	_ppOutputDrivers[3] = new ASIOInterface;
	//	
	//	//if (((ASIOInterface*)(_ppOutputDrivers[3]))->drivercount <= 0)
	//	//{
	//	//	_numOutputDrivers--;
	//	//	delete _ppOutputDrivers[3];
	//	//}
	//	
	//	// TODO とりあえず、あとできちんと実装する
	//	m_pOutputDriver = _paudio_driver;

	////	_pMidiInput = new CMidiInput;	// MIDI IMPLEMENTATION
	//}

//Controller<SmartPtrType<Type> >::~Controller()
//{
//	//for(OutputDriverMap::iterator it = m_OutputDrivers.begin();it != m_OutputDrivers.end();it++)
//	//{
//	//	(*it).second->Release() ;
//	//	(*it).second = NULL;
//	//}
//	//delete m_pResampler;
//	//m_pResampler = NULL;
//	//delete dynamic_cast<Cubic*>(m_pResampler);
//	//delete dynamic_cast<view::MainFrame*>(m_pMainFrame);
//}

//template < template < class > class SmartPtrType >
//HRESULT Controller< SmartPtrType >::Run(ULONG hInstance,BSTR lpstrCmdLine,LONG nCmdShow)
//{
//
//	HMODULE hResDll;
//	//hResDll = ::LoadLibrary( L"1033\\PsycleWTLRes.dll") ; // Japanese
//
//	hResDll = LoadSatelliteDLL(DetectLanguage());
//	if(hResDll == NULL)
//	{
//		hResDll = ::LoadLibrary( L"1033\\PsycleWTLRes.dll");
//	}
//	
//	detail::res_handle::instance().init(hResDll);
//
//	_AtlBaseModule.SetResourceInstance(hResDll);	
//
//	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls
//	
//
////	hRes = MessageLoops.Init(NULL, hInstance);
////	ATLASSERT(SUCCEEDED(hRes));
//
////	CComPtr<ISkinX>	spISkinX;
////	spISkinX.CoCreateInstance(CLSID_MacSkin);
////	if (spISkinX)
////		spISkinX->InstallSkin(MessageLoops.m_dwMainThreadID);
//
////	int nRet = Run(lpstrCmdLine, nCmdShow);
//
//    OSVERSIONINFO osvi;
//    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
//
//    GetVersionEx(&osvi); 
//    if (!((osvi.dwMinorVersion >= 1 && osvi.dwMajorVersion >= 5) || (osvi.dwMajorVersion > 5)))
//	{
//		::MessageBox(NULL,SF::CResourceString(IDS_ERR_MSG0096),SF::CResourceString(IDS_ERR_MSG0097),0);
//		return 0;
//	}
//	
//	// GDI+ STartup
//
//	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
//	ULONG_PTR           gdiplusToken;
//	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
//
//	//wnd<view::MainFrame> m_wnd = 
//	//	create_wnd<view::MainFrame,iController,string>(
//	//			*this,
//	//			string(_T("Psycle Featuring Win32 GUI Generics")),
//	//			null_wnd,
//	//			create_info()
//	//			.menu(IDR_MAINFRAME)
//	//	);
//
//	//m_wnd->wait();
//
//	m_pOutputDriver->Configuration()->Configure();
//
//	//m_pMainFrame = m_wnd.;
//
//	//_pmain->CreateEx();
//	//_pmain->ShowWindow(nCmdShow);
//
//	
//	//SetRegistryKey(_T("AAS"));
//	
//	//LoadStdProfileSettings();  // Load standard INI file options (including MRU)
//	
//	// To create the main window, this code creates a new frame window
//	// object and then sets it as the application's main window object.
//	
//	// Set Custom Window Class
//	//SF::UI::Volume::GetWndClassInfo().Register((WNDPROC *)(&::DefWindowProc));
//	
//	//if(wndMain.CreateEx() == NULL)
//	//{
//	//	ATLTRACE(_T("Main window creation failed!\n"));
//	//	return 0;
//	//}
//
//	//if (!Global::pConfig->Read()) // problem reading registry info. missing or damaged
//	//{
//	//	Global::pConfig->_initialized = false;
//	//	CConfigDlg dlg(_T("Psycle configuration"));
//	//	dlg.Init(Global::pConfig);
//	//	if (dlg.DoModal() == IDOK)
//	//	{
//	//		wndMain.m_view._outputActive = true;
//	//		Global::pConfig->_initialized = true;
//	//	}
//	//}
//	//else
//	//{
//	//	wndMain.m_view._outputActive = true;
//	//}
//
//	//wndMain.InitSequencer();
//
//	//wndMain.ShowWindow(nCmdShow);
//	//// Sets Icon
//	//HICON tIcon;
//	//tIcon = LoadIcon(_AtlBaseModule.GetModuleInstance(),MAKEINTRESOURCE(IDR_MAINFRAME));
//	//wndMain.SetIcon(tIcon,true);
//	//wndMain.SetIcon(tIcon,false);
//	//
//	//wndMain.m_view.LoadMachineSkin();
//	//wndMain.m_view.LoadPatternHeaderSkin();
//	//wndMain.m_view.LoadMachineBackground();
//	//wndMain.m_view.RecalcMetrics();
//	//wndMain.m_view.RecalculateColourGrid();
//	//// The one and only window has been initialized, so show and update it.
//	//wndMain.ShowWindow(SW_MAXIMIZE);
//	//
//	//// center master machine
//	//wndMain.m_view._pSong->pMachine(MASTER_INDEX)->_x
//	//	= (wndMain.m_view.CW-wndMain.m_view.MachineCoords.sMaster.width) / 2;
//	//wndMain.m_view._pSong->pMachine(MASTER_INDEX)->_y
//	//	= (wndMain.m_view.CH-wndMain.m_view.MachineCoords.sMaster.width) / 2;
//	//wndMain.UpdateWindow();
//	//
//	//CNewMachine::LoadPluginInfo();
//	//// Show splash screen
//	//if (Global::pConfig->_showAboutAtStart)
//	//{
//	//	CAboutDlg dlg;
//	//	dlg.DoModal();
//	//}
//	
//	//Import recent files from registry.
////	wndMain.LoadRecentConfig();
//	//wndMain.LoadRecent();
//
//	//ManagedWindowFactory _t;
//	
//	//_t.CreateManagedWindow(wndMain.m_view,_T("temp\\XMSamplerUI.dll"),_T("SF.XMSamplerUITest"));
//
//
//	//int nRet = theLoop.Run();
//
//	//testmain();
//
//	//CMessageLoopContainer::Instance().RemoveMessageLoop();
//	
//	//_pmain->DestroyWindow();
//	//
//	//_global->pConfig->Write();
//	//_global->pConfig->_pOutputDriver->Enable(false);
//	//::Sleep(LOCK_LATENCY);
//	//_global->pConfig->_pMidiInput->Close();
//	//CNewMachine::DestroyPluginInfo();
//
//	//delete _global;	
//
//	Gdiplus::GdiplusShutdown(gdiplusToken);
//
////	spISkinX.Release();
//
////	CMessageLoopContainer::Instance().Term();
//
// // プログラムの最後でコールする
//	//SF::UI::Volume<int>::Release();
//
//	return S_OK;
//}


// Loads the satellite DLL specified for the language DesiredLanguage

//void DeleteObject(iDisposable* p)
//{
//	p->Release();
//};

}

}

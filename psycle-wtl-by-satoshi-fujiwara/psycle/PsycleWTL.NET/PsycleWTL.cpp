/** @file 
 *  @brief main source file for PsycleWTL.exe
 *  $Date: 2005/01/15 22:44:35 $
 *  $Revision: 1.2 $
 */

#include <tchar.h>
#include <atldef.h>
#include <atlbase.h>

#include "WindowFactoryImpl.h"

#include "..\_Main.h"
#include "..\_Main_i.c"

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	
	// _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	// _CrtSetBreakAlloc(1879);


/*	_CrtMemState state;
    _CrtMemCheckpoint(&state)*/;
	//_CrtSetBreakAlloc(8936);

//	HRESULT hRes = ::CoInitializeEx(NULL,COINIT_MULTITHREADED);
	HRESULT hRes = ::CoInitialize(NULL);
	
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);
	

	//HMODULE hResDll;
	////hResDll = ::LoadLibrary( L"1033\\PsycleWTLRes.dll") ; // Japanese

	//hResDll = LoadSatelliteDLL(DetectLanguage());
	//if(hResDll == NULL)
	//{
	//	hResDll = ::LoadLibrary( L"1033\\PsycleWTLRes.dll");
	//}

	//_Module.SetResourceInstance(hResDll);	

	//AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls
	//

	//hRes = _Module.Init(NULL, hInstance);
	//ATLASSERT(SUCCEEDED(hRes));

//	CComPtr<ISkinX>	spISkinX;
//	spISkinX.CoCreateInstance(CLSID_MacSkin);
//	if (spISkinX)
//		spISkinX->InstallSkin(_Module.m_dwMainThreadID);

	//int nRet = Run(lpstrCmdLine, nCmdShow);

//	spISkinX.Release();

	//_Module.Term();
	{
		ATL::CComPtr<IMainObject> pMain;
		pMain.CoCreateInstance(CLSID_CMainObject);
		pMain->Run((ULONG) hInstance,ATL::CComBSTR(lpstrCmdLine),nCmdShow);
	}
	::CoUninitialize();
 // プログラムの最後でコールする
	//SF::UI::Volume<int>::Release();
    //_CrtMemDumpAllObjectsSince(&state);
	return 0;
}

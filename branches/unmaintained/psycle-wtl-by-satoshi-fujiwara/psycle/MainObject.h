// MainObject.h : CMainObject の宣言

#pragma once
#include "resource.h"       // メイン シンボル


// IMainObject
[
	object,
	uuid("2CA78C1E-D645-4B67-8979-68BD9DF2D7DA"),
	dual,	helpstring("IMainObject Interface"),
	pointer_default(unique)
]
__interface IMainObject : IDispatch
{
	[id(1), helpstring("メソッドRun")] HRESULT Run([in] ULONG hInstance,[in] BSTR lpstrCmdLine, [in] LONG nCmdShow);
};

//IMain
[
	object,
	uuid("9AFFA8E7-B4C7-46f3-81EE-801B6D12ABFF"),
	dual,	helpstring("IMainController Interface"),
	pointer_default(unique)
]
__interface IMain : IDispatch 
{

};

// _IMainObjectEvents
[
	dispinterface,
	uuid("57335A8D-317A-43B2-BC48-392170220B49"),
	helpstring("_IMainObjectEvents インターフェイス")
]
__interface _IMainObjectEvents
{
};


// CMainObject

[
	coclass,
	threading("apartment"),
	support_error_info("IMainObject"),
	event_source("com"),
	vi_progid("PsycleWTL.MainObject"),
	progid("PsycleWTL.MainObject.1"),
	version(1.0),
	uuid("194F2C2F-7394-44BD-8E83-03923F98189F"),
	helpstring("MainObject Class")
]
class ATL_NO_VTABLE CMainObject : 
	public IObjectWithSiteImpl<CMainObject>,
	public IMainObject,public IMain
{
public:
	CMainObject()
	{
		//m_pMain = this;
	}

	~CMainObject()
	{


	}

	__event __interface _IMainObjectEvents;

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}
// Automation Method 
public:
	STDMETHOD(Run)(ULONG hInstance,BSTR lpstrCmdLine,LONG nCmdShow);
// Innner 	
//	string& CurrentDirectory(){return m_CurrentDirectory;};
private:

};


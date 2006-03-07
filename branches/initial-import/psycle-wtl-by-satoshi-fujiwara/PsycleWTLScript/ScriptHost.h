// ScriptHost.h : CScriptHost の宣言

#pragma once
#include "resource.h"       // メイン シンボル
#include "AtlActiveScriptSite.h"

// IScriptHost
[
	object,
	uuid("146B45AB-51B9-4A22-9B60-3787FEBDF439"),
	dual,	helpstring("IScriptHost Interface"),
	pointer_default(unique)
]
__interface IScriptHost : IDispatch
{
		[id(1), helpstring("Create Script Engine")] HRESULT CreateEngine(BSTR pstrProgID);
		[id(2), helpstring("Create COM object")] HRESULT CreateObject([in]BSTR strProgID, [out,retval]LPDISPATCH* ppObject);
		[id(3), helpstring("Add a Script Item")] HRESULT AddScriptItem([in]BSTR pstrNamedItem, [in]LPUNKNOWN lpUnknown);
		[id(4), helpstring("Add Script code")] HRESULT AddScriptCode([in]BSTR pstrScriptCode);
		[id(5), helpstring("Add Scriptlet (with Event)")] HRESULT AddScriptlet([in]BSTR pstrDefaultName, [in]BSTR pstrCode, [in]BSTR pstrItemName, [in]BSTR pstrEventName);
		[id(6), helpstring("Display MessageBox with provide text")] HRESULT Display(BSTR strText);

};


// _IScriptHostEvents
[
	dispinterface,
	uuid("13E8EF20-EBC4-4833-9093-47B56D3249C3"),
	helpstring("_IScriptHostEvents インターフェイス")
]
__interface _IScriptHostEvents
{
};


// CScriptHost

[
	coclass,
	threading("apartment"),
	support_error_info("IScriptHost"),
	event_source("com"),
	vi_progid("PsycleWTL.ScriptHost"),
	progid("PsycleWTL.ScriptHost.1"),
	version(1.0),
	uuid("484F364C-E070-4FEF-8AD8-3AF6DE9942E8"),
	helpstring("ScriptHost Class")
]
class ATL_NO_VTABLE CScriptHost : 
	public IObjectWithSiteImpl<CScriptHost>,
	public IActiveScriptHostImpl<CScriptHost>,
	public IDispatchImpl<IScriptHost>
{
public:
	CScriptHost()
	{
	}

	__event __interface _IScriptHostEvents;

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IScriptHost
	STDMETHOD(CreateEngine)(BSTR pstrProgID);
	STDMETHOD(CreateObject)(/*[in]*/BSTR strProgID, 
	                        /*[out,retval]*/LPDISPATCH* ppObject);
	STDMETHOD(AddScriptItem)(/*[in]*/BSTR pstrNamedItem, 
	                        /*[in]*/LPUNKNOWN lpUnknown);
	STDMETHOD(AddScriptCode)(/*[in]*/BSTR pstrScriptCode);
	STDMETHOD(AddScriptlet)(/*[in]*/BSTR pstrDefaultName, 
	                        /*[in]*/BSTR pstrCode, 
	                        /*[in]*/BSTR pstrItemName, 
	                        /*[in]*/BSTR pstrEventName);
	STDMETHOD(Display)(BSTR strText);
};



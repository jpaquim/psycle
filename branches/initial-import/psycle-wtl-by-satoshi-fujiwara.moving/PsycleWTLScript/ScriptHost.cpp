// ScriptHost.cpp : CScriptHost ‚ÌŽÀ‘•

#include "stdafx.h"
#include "ScriptHost.h"
#include "AtlActiveScriptSite.h"


// CScriptHost


STDMETHODIMP CScriptHost::CreateEngine(BSTR pstrProgID)
{
	BOOL bRet
	    = IActiveScriptHostImpl<CScriptHost>::CreateEngine( pstrProgID );
	return (bRet? S_OK : E_FAIL);
}

STDMETHODIMP CScriptHost::CreateObject(BSTR strProgID, LPDISPATCH* ppObject)
{
	LPDISPATCH lpDispatch = IActiveScriptHostImpl
	                         <CScriptHost>::CreateObjectHelper( strProgID );
	*ppObject = lpDispatch;
	return ((lpDispatch!=NULL)? S_OK : E_FAIL);
}

STDMETHODIMP CScriptHost::AddScriptItem(BSTR pstrNamedItem, LPUNKNOWN lpUnknown)
{
	BOOL bRet = IActiveScriptHostImpl<CScriptHost>::AddScriptItem( pstrNamedItem, lpUnknown );

	return (bRet? S_OK : E_FAIL);
}

STDMETHODIMP CScriptHost::AddScriptCode(BSTR pstrScriptCode)
{
	BOOL bRet = IActiveScriptHostImpl<CScriptHost>::AddScriptCode( pstrScriptCode );

	return (bRet? S_OK : E_FAIL);
}

STDMETHODIMP CScriptHost::AddScriptlet(BSTR pstrDefaultName, BSTR pstrCode, BSTR pstrItemName, BSTR pstrEventName)
{
	BOOL bRet = IActiveScriptHostImpl<CScriptHost>::AddScriptlet(
		pstrDefaultName, pstrCode, pstrItemName, pstrEventName);
	

	return (bRet? S_OK : E_FAIL);
}

STDMETHODIMP CScriptHost::Display(BSTR strText)
{
	USES_CONVERSION;
	LPCTSTR lpText = OLE2CT(strText);
	MessageBox(NULL, lpText, _T("ScriptHost"), MB_OK);

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//	File:		AtlActiveScriptSite.cpp
//	Version:	1.00
//
//	Author:		Ernest Laurentin
//	E-mail:		elaurentin@sympatico.ca
//
//	This class implements an Active Script Hosting for client application
//
//	This code may be used in compiled form in any way you desire. This
//	file may be redistributed unmodified by any means PROVIDING it is
//	not sold for profit without the authors written consent, and
//	providing that this notice and the authors name and all copyright
//	notices remains intact.
//
//	An email letting me know how you are using it would be nice as well.
//
//	This file is provided "as is" with no expressed or implied warranty.
//	The author accepts no liability for any damage/loss of business that
//	this c++ class may cause.
//
//	Version history
///////////////////////////////////////////////////////////////////////////////
#ifndef _SCRIPTHOST_H_
#define _SCRIPTHOST_H_

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLBASE_H__
	#error atlcom.h requires atlbase.h to be included first
#endif

#include <xstring>
#include <map>
#include <activscp.h>
using namespace std;

class str_lower : public binary_function<wstring, wstring, bool> {
public:
    bool operator()(const wstring& x, const wstring& y) const
	{
		return (x.compare( y.c_str() ) < 0);
	}
};

typedef map<wstring, LPUNKNOWN, str_lower> IUnknownPtrMap;

#define ACTIVS_HOST		L"ScriptHost"
#define SCRIPTITEM_NAMEDITEM		(SCRIPTITEM_ISSOURCE | \
									 SCRIPTITEM_ISVISIBLE | \
									 SCRIPTITEM_GLOBALMEMBERS)

#define LANG_ENGLISH_NEUTRAL	(MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL))
#define LOCALE_SCRIPT_DEFAULT	(MAKELCID(LANG_ENGLISH_NEUTRAL, SORT_DEFAULT))

//////////////////////////////////////////////////////////////////////////////
// IActiveScriptSiteImpl
// "class T" must implement:
//		LPUNKNOWN T::GetObjectPointer(const WCHAR* pstrName)
template <class T>
class ATL_NO_VTABLE IActiveScriptSiteImpl : public IActiveScriptSite
{
protected:
public:
		///////////////////////////////////////////////////////////////////////////
		// O P E R A T I O N S
		///////////////////////////////////////////////////////////////////////////

		STDMETHOD(GetLCID)(LCID* plcid)
		{
			*plcid = LOCALE_SCRIPT_DEFAULT;
			return S_OK;
		}

		STDMETHOD(GetItemInfo)(
			/* [in] */ LPCOLESTR pstrName,
			/* [in] */ DWORD dwReturnMask,
			/* [out] */LPUNKNOWN* ppiunkItem,
			/* [out] */LPTYPEINFO* ppti)
		{
			if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
			{
				if (!ppti)
					return E_INVALIDARG;
				*ppti = NULL;
			}

			if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
			{
				if (!ppiunkItem)
					return E_INVALIDARG;
				*ppiunkItem = NULL;
			}

			// Required LPUNKNOWN T::GetObjectPointer(const WCHAR* pstrName)
			T* pT = static_cast<T*>(this);
			LPUNKNOWN lpUnknown = pT->GetObjectPointer( pstrName );
			ATLASSERT(NULL != lpUnknown);
			if (NULL == lpUnknown)
			{
				ATLTRACE("No IUnknown for item!\n");
				// This shouldn't ever happen, since we're providing the items
				return TYPE_E_ELEMENTNOTFOUND;
			}

			if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
			{
				// Use IProvideClassInfo to get ITypeInfo of coclass!
				IProvideClassInfo *pci = NULL;
				HRESULT hr = lpUnknown->QueryInterface(IID_IProvideClassInfo, (void**)&pci);
				if (SUCCEEDED(hr) && pci)
				{
					hr = pci->GetClassInfo(ppti);
				}

				// Release interface
				if (pci)
					pci->Release();

				if (FAILED(hr))
					return E_FAIL;
			}

			if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
			{
				*ppiunkItem = lpUnknown;
				(*ppiunkItem)->AddRef();    // because returning
			}

			return S_OK;
		}

        STDMETHOD(GetDocVersionString)(BSTR* /*pbstrVersion*/)
		{
			ATLTRACENOTIMPL(_T("IActiveScriptSiteImpl::GetDocVersionString\n"));
		}

        STDMETHOD(OnScriptTerminate)(const VARIANT* /*pvarResult*/, const EXCEPINFO* /*pexcepinfo*/)
		{
			ATLTRACE(_T("IActiveScriptSiteImpl::OnScriptTerminate\n"));
			return S_OK;
		}

        STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState)
		{
			ATLTRACE(_T("IActiveScriptSiteImpl::OnStateChange\n"));
			return S_OK;
		}

        STDMETHOD(OnScriptError)(IActiveScriptError* /*pse*/)
		{
			ATLTRACE(_T("IActiveScriptSiteImpl::OnScriptError\n"));
			return S_OK;
		}

        STDMETHOD(OnEnterScript)()
		{
			ATLTRACE(_T("IActiveScriptSiteImpl::OnEnterScript\n"));
			return S_OK;
		}

        STDMETHOD(OnLeaveScript)()
		{
			ATLTRACE(_T("IActiveScriptSiteImpl::OnLeaveScript\n"));
			return S_OK;
		}
};


//////////////////////////////////////////////////////////////////////////////
// IActiveScriptSiteWindowImpl
// "class T" must implement:
//		LPUNKNOWN T::GetBaseWindow()
template <class T>
class ATL_NO_VTABLE IActiveScriptSiteWindowImpl : public IActiveScriptSiteWindow
{
protected:
public:
		///////////////////////////////////////////////////////////////////////////
		// O P E R A T I O N S
		///////////////////////////////////////////////////////////////////////////

		STDMETHOD(EnableModeless)(BOOL /*fEnable*/)
		{
			return S_OK;
		}

		STDMETHOD(GetWindow)(HWND* phWnd)
		{
			T* pT = static_cast<T*>(this);
			*phWnd = pT->GetBaseWindow();
			return S_OK;
		}
};


//////////////////////////////////////////////////////////////////////////////
// IActiveScriptHostImpl
template <class T>
class ATL_NO_VTABLE IActiveScriptHostImpl :
		public IActiveScriptSiteImpl<IActiveScriptHostImpl>,
		public IActiveScriptSiteWindowImpl<IActiveScriptHostImpl>
{
public:
// Constructors
	IActiveScriptHostImpl()
	{
	}
	virtual ~IActiveScriptHostImpl()
	{
		ReleaseObjects();
	}

	void ReleaseObjects()
	{
		if (m_pAxsScript != NULL)
			m_pAxsParse = NULL;

		if (m_pAxsScript != NULL)
		{
			m_pAxsScript->Close();
			m_pAxsScript = NULL;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// O V E R R I D E
	///////////////////////////////////////////////////////////////////////////

	virtual LPUNKNOWN GetObjectPointer(const WCHAR* pstrName)
	{
		LPUNKNOWN lpUnknown = NULL;
		wstring sNamedItem = pstrName;
		IUnknownPtrMap::iterator iter = m_mapNamedItems.find( sNamedItem );
		if ( iter != m_mapNamedItems.end())
			lpUnknown = (*iter).second;
		return lpUnknown;
	}

	virtual HWND GetBaseWindow()
	{
		return GetDesktopWindow();
	}

	///////////////////////////////////////////////////////////////////////////
	// O P E R A T I O N S
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// CreateEngine
	BOOL CreateEngine(LPCOLESTR pstrProgID)
	{
		// Auto deletion of script engine
		ReleaseObjects();

		CLSID clsid;
		// Search for LanguageID and create instance of script engine
		if (SUCCEEDED(CLSIDFromProgID(pstrProgID, &clsid)))
		{
			// If this happens, the scripting engine is probably not properly registered
			HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_ALL, IID_IActiveScript, (void **)&m_pAxsScript);
			// Script Engine must support IActiveScriptParse for us to use it
			if (SUCCEEDED(hr))
				hr = m_pAxsScript->QueryInterface(IID_IActiveScriptParse, (void **)&m_pAxsParse);

			if (SUCCEEDED(hr))
				hr = m_pAxsScript->SetScriptSite( (IActiveScriptSiteImpl<IActiveScriptHostImpl>*) this);

			// InitNew the object:
			if (SUCCEEDED(hr))
				hr = m_pAxsParse->InitNew();

			// Store 'Host' object
			wstring sNamedItem = ACTIVS_HOST;
			T* pT = static_cast<T*>(this);
			m_mapNamedItems.insert(IUnknownPtrMap::value_type(sNamedItem,pT->GetUnknown()) );

			// Add Top-level Global Named Item
			if (SUCCEEDED(hr))
				hr = m_pAxsScript->AddNamedItem(ACTIVS_HOST, SCRIPTITEM_NAMEDITEM);

			if (SUCCEEDED(hr))
				hr = m_pAxsScript->SetScriptState(SCRIPTSTATE_STARTED);

			if (SUCCEEDED(hr))
				return TRUE;

			m_mapNamedItems.erase( sNamedItem );
			ReleaseObjects();
		}
		return FALSE;
	}

	///////////////////////////////////////////////////////////////////////////
	// AddScriptItem
	BOOL AddScriptItem(LPCOLESTR pstrNamedItem, LPUNKNOWN lpUnknown,
		DWORD dwFlags = SCRIPTITEM_NAMEDITEM)
	{
		// Add Top-level Global Named Item
		if (m_pAxsScript != NULL && lpUnknown != NULL)
		{
			HRESULT hr;
			USES_CONVERSION;

			// Insert object into map
			wstring sNamedItem = pstrNamedItem;
			m_mapNamedItems.insert(IUnknownPtrMap::value_type(sNamedItem,lpUnknown) );

			hr = m_pAxsScript->AddNamedItem(pstrNamedItem, dwFlags);
			// Connected to object sink-interface
			if (SUCCEEDED(hr))
				hr = m_pAxsScript->SetScriptState(SCRIPTSTATE_CONNECTED);

			if (SUCCEEDED(hr))
				return TRUE;

			m_mapNamedItems.erase( sNamedItem );
		}
		return FALSE;
	}

	///////////////////////////////////////////////////////////////////////////
	// AddScriptCode
	BOOL AddScriptCode(LPCOLESTR pstrScriptCode)
	{
		if (m_pAxsScript != NULL)
		{
			HRESULT hr;
			EXCEPINFO ei = { 0 };
			hr = m_pAxsParse->ParseScriptText(pstrScriptCode, 0, 0, 0, 0, 0, 
								SCRIPTTEXT_ISPERSISTENT|SCRIPTTEXT_ISVISIBLE,
								0, &ei);
			if (SUCCEEDED(hr))
				return TRUE;
		}
		return FALSE;
	}

	///////////////////////////////////////////////////////////////////////////
	// AddScriptlet
	BOOL AddScriptlet(LPCOLESTR pstrDefaultName, LPCOLESTR pstrCode,
						LPCOLESTR pstrItemName, LPCOLESTR pstrEventName)
	{
		if (m_pAxsParse != NULL)
		{
			HRESULT hr;
			EXCEPINFO ei = { 0 };
			BSTR bstrName = NULL;
			hr = m_pAxsParse->AddScriptlet(pstrDefaultName, 
									pstrCode,
									pstrItemName, // name object
									pstrItemName, // no subobject - same as object
									pstrEventName, // event name - attach to 'pstrItemName'
									 L"", 
									 0, 
									 0,
									 0, 
									 &bstrName,
									 &ei);
			SysFreeString(bstrName);
			if (SUCCEEDED(hr))
				return TRUE;
		}
		return FALSE;
	}

	// Helper functions
	///////////////////////////////////////////////////////////////////////////
	// CreateObjectHelper
	LPDISPATCH CreateObjectHelper(LPCOLESTR bstrProgID)
	{
		LPDISPATCH lpObject = NULL;
		CLSID clsidObject = CLSID_NULL;
		HRESULT hr = CLSIDFromProgID(bstrProgID, &clsidObject );
		if( SUCCEEDED( hr ) )
		{
			// Only Local-Server is safe (run as separated process)
			hr = CoCreateInstance(clsidObject, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void**)&lpObject );
			if (FAILED(hr))
			{
				ATLTRACE("(Script) Failed to create object Return = 0x%x\n", hr);
			}
		}
		else
		{
			ATLTRACE("(Script) Invalid ProgID\n");
		}

		return lpObject;
	}
	
	///////////////////////////////////////////////////////////////////////////
	// ReadTextFileHelper
	WCHAR*	ReadTextFileHelper(LPCOLESTR strFileName)
	{
		WCHAR *pwszResult = NULL;
		COLE2T szFileName(strFileName);
		
		if(szFileName.m_psz == NULL)
			return NULL;


		HANDLE hfile = CreateFile(szFileName, GENERIC_READ,
							   FILE_SHARE_READ, 0, OPEN_EXISTING,
							   FILE_ATTRIBUTE_NORMAL, 0);
		if (hfile != INVALID_HANDLE_VALUE)
		{
		 DWORD cch = GetFileSize(hfile, 0);
		 char *psz = (char*)CoTaskMemAlloc(cch + sizeof(char));

		 if (psz)
		 {
			 DWORD cb;
			 ReadFile(hfile, psz, cch, &cb, 0);
			 pwszResult = (WCHAR*)CoTaskMemAlloc((cch + 1) * sizeof(WCHAR));
			 if (pwszResult)
				 mbstowcs(pwszResult, psz, cch + 1);
			 pwszResult[cch] = 0;
			 CoTaskMemFree(psz);
		 }
		 CloseHandle(hfile);
		}
		return pwszResult;
	}

	///////////////////////////////////////////////////////////////////////////
	// DestroyDataHelper
	void DestroyDataHelper(LPVOID lpvData)
	{
		CoTaskMemFree(lpvData);
	}

	///////////////////////////////////////////////////////////////////////////
	// InvokeFuncHelper
	HRESULT InvokeFuncHelper(LPCOLESTR lpszName, VARIANT* pvarParams, int nParams, VARIANT* pvarRet = NULL)
	{
		HRESULT hr = E_FAIL;
		DISPID dispid;
		if (m_pAxsScript != NULL)
		{
			LPDISPATCH pDisp = NULL;
			DISPPARAMS dispparams = { pvarParams, NULL, nParams, 0};
			hr = m_pAxsScript->GetScriptDispatch(0, &pDisp);
			if (SUCCEEDED(hr))
				hr = pDisp->GetIDsOfNames(IID_NULL, (LPOLESTR*)&lpszName, 1, LOCALE_SCRIPT_DEFAULT, &dispid);
			if (SUCCEEDED(hr))
				hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_SCRIPT_DEFAULT, DISPATCH_METHOD, &dispparams, pvarRet, NULL, NULL);
		}
		return hr;
	}

protected:
	CComPtr<IActiveScriptParse> m_pAxsParse;	// Active Script parse object
	CComPtr<IActiveScript>		m_pAxsScript;	// Active Script
	IUnknownPtrMap				m_mapNamedItems; // Named items map
};

#endif // _SCRIPTHOST_H_

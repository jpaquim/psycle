

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Wed Jan 05 22:00:15 2005
 */
/* Compiler settings for _PsycleWTLScript.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef ___PsycleWTLScript_h__
#define ___PsycleWTLScript_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IScriptHost_FWD_DEFINED__
#define __IScriptHost_FWD_DEFINED__
typedef interface IScriptHost IScriptHost;
#endif 	/* __IScriptHost_FWD_DEFINED__ */


#ifndef ___IScriptHostEvents_FWD_DEFINED__
#define ___IScriptHostEvents_FWD_DEFINED__
typedef interface _IScriptHostEvents _IScriptHostEvents;
#endif 	/* ___IScriptHostEvents_FWD_DEFINED__ */


#ifndef __CScriptHost_FWD_DEFINED__
#define __CScriptHost_FWD_DEFINED__

#ifdef __cplusplus
typedef class CScriptHost CScriptHost;
#else
typedef struct CScriptHost CScriptHost;
#endif /* __cplusplus */

#endif 	/* __CScriptHost_FWD_DEFINED__ */


/* header files for imported files */
#include "prsht.h"
#include "mshtml.h"
#include "mshtmhst.h"
#include "exdisp.h"
#include "objsafe.h"
#include "activscp.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IScriptHost_INTERFACE_DEFINED__
#define __IScriptHost_INTERFACE_DEFINED__

/* interface IScriptHost */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IScriptHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("146B45AB-51B9-4A22-9B60-3787FEBDF439")
    IScriptHost : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateEngine( 
            /* [in] */ BSTR pstrProgID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateObject( 
            /* [in] */ BSTR strProgID,
            /* [retval][out] */ LPDISPATCH *ppObject) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddScriptItem( 
            /* [in] */ BSTR pstrNamedItem,
            /* [in] */ LPUNKNOWN lpUnknown) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddScriptCode( 
            /* [in] */ BSTR pstrScriptCode) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddScriptlet( 
            /* [in] */ BSTR pstrDefaultName,
            /* [in] */ BSTR pstrCode,
            /* [in] */ BSTR pstrItemName,
            /* [in] */ BSTR pstrEventName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Display( 
            /* [in] */ BSTR strText) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IScriptHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IScriptHost * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IScriptHost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IScriptHost * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IScriptHost * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IScriptHost * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IScriptHost * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IScriptHost * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CreateEngine )( 
            IScriptHost * This,
            /* [in] */ BSTR pstrProgID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CreateObject )( 
            IScriptHost * This,
            /* [in] */ BSTR strProgID,
            /* [retval][out] */ LPDISPATCH *ppObject);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddScriptItem )( 
            IScriptHost * This,
            /* [in] */ BSTR pstrNamedItem,
            /* [in] */ LPUNKNOWN lpUnknown);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddScriptCode )( 
            IScriptHost * This,
            /* [in] */ BSTR pstrScriptCode);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddScriptlet )( 
            IScriptHost * This,
            /* [in] */ BSTR pstrDefaultName,
            /* [in] */ BSTR pstrCode,
            /* [in] */ BSTR pstrItemName,
            /* [in] */ BSTR pstrEventName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Display )( 
            IScriptHost * This,
            /* [in] */ BSTR strText);
        
        END_INTERFACE
    } IScriptHostVtbl;

    interface IScriptHost
    {
        CONST_VTBL struct IScriptHostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptHost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptHost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptHost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptHost_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IScriptHost_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IScriptHost_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IScriptHost_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IScriptHost_CreateEngine(This,pstrProgID)	\
    (This)->lpVtbl -> CreateEngine(This,pstrProgID)

#define IScriptHost_CreateObject(This,strProgID,ppObject)	\
    (This)->lpVtbl -> CreateObject(This,strProgID,ppObject)

#define IScriptHost_AddScriptItem(This,pstrNamedItem,lpUnknown)	\
    (This)->lpVtbl -> AddScriptItem(This,pstrNamedItem,lpUnknown)

#define IScriptHost_AddScriptCode(This,pstrScriptCode)	\
    (This)->lpVtbl -> AddScriptCode(This,pstrScriptCode)

#define IScriptHost_AddScriptlet(This,pstrDefaultName,pstrCode,pstrItemName,pstrEventName)	\
    (This)->lpVtbl -> AddScriptlet(This,pstrDefaultName,pstrCode,pstrItemName,pstrEventName)

#define IScriptHost_Display(This,strText)	\
    (This)->lpVtbl -> Display(This,strText)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IScriptHost_CreateEngine_Proxy( 
    IScriptHost * This,
    /* [in] */ BSTR pstrProgID);


void __RPC_STUB IScriptHost_CreateEngine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IScriptHost_CreateObject_Proxy( 
    IScriptHost * This,
    /* [in] */ BSTR strProgID,
    /* [retval][out] */ LPDISPATCH *ppObject);


void __RPC_STUB IScriptHost_CreateObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IScriptHost_AddScriptItem_Proxy( 
    IScriptHost * This,
    /* [in] */ BSTR pstrNamedItem,
    /* [in] */ LPUNKNOWN lpUnknown);


void __RPC_STUB IScriptHost_AddScriptItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IScriptHost_AddScriptCode_Proxy( 
    IScriptHost * This,
    /* [in] */ BSTR pstrScriptCode);


void __RPC_STUB IScriptHost_AddScriptCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IScriptHost_AddScriptlet_Proxy( 
    IScriptHost * This,
    /* [in] */ BSTR pstrDefaultName,
    /* [in] */ BSTR pstrCode,
    /* [in] */ BSTR pstrItemName,
    /* [in] */ BSTR pstrEventName);


void __RPC_STUB IScriptHost_AddScriptlet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IScriptHost_Display_Proxy( 
    IScriptHost * This,
    /* [in] */ BSTR strText);


void __RPC_STUB IScriptHost_Display_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IScriptHost_INTERFACE_DEFINED__ */



#ifndef __PsycleWTLScript_LIBRARY_DEFINED__
#define __PsycleWTLScript_LIBRARY_DEFINED__

/* library PsycleWTLScript */
/* [helpstring][uuid][version] */ 


EXTERN_C const IID LIBID_PsycleWTLScript;

#ifndef ___IScriptHostEvents_DISPINTERFACE_DEFINED__
#define ___IScriptHostEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IScriptHostEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__IScriptHostEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("13E8EF20-EBC4-4833-9093-47B56D3249C3")
    _IScriptHostEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IScriptHostEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IScriptHostEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IScriptHostEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IScriptHostEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IScriptHostEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IScriptHostEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IScriptHostEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IScriptHostEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _IScriptHostEventsVtbl;

    interface _IScriptHostEvents
    {
        CONST_VTBL struct _IScriptHostEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IScriptHostEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IScriptHostEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IScriptHostEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IScriptHostEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IScriptHostEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IScriptHostEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IScriptHostEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IScriptHostEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CScriptHost;

#ifdef __cplusplus

class DECLSPEC_UUID("484F364C-E070-4FEF-8AD8-3AF6DE9942E8")
CScriptHost;
#endif
#endif /* __PsycleWTLScript_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



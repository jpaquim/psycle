

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Fri Jan 14 06:38:55 2005
 */
/* Compiler settings for _Main.idl:
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

#ifndef ___Main_h__
#define ___Main_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IMainObject_FWD_DEFINED__
#define __IMainObject_FWD_DEFINED__
typedef interface IMainObject IMainObject;
#endif 	/* __IMainObject_FWD_DEFINED__ */


#ifndef __IMain_FWD_DEFINED__
#define __IMain_FWD_DEFINED__
typedef interface IMain IMain;
#endif 	/* __IMain_FWD_DEFINED__ */


#ifndef ___IMainObjectEvents_FWD_DEFINED__
#define ___IMainObjectEvents_FWD_DEFINED__
typedef interface _IMainObjectEvents _IMainObjectEvents;
#endif 	/* ___IMainObjectEvents_FWD_DEFINED__ */


#ifndef __CMainObject_FWD_DEFINED__
#define __CMainObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class CMainObject CMainObject;
#else
typedef struct CMainObject CMainObject;
#endif /* __cplusplus */

#endif 	/* __CMainObject_FWD_DEFINED__ */


/* header files for imported files */
#include "prsht.h"
#include "mshtml.h"
#include "mshtmhst.h"
#include "exdisp.h"
#include "objsafe.h"
#include "shldisp.h"
#include "shobjidl.h"
#include "_PsycleWTLScript.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IMainObject_INTERFACE_DEFINED__
#define __IMainObject_INTERFACE_DEFINED__

/* interface IMainObject */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IMainObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2CA78C1E-D645-4B67-8979-68BD9DF2D7DA")
    IMainObject : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Run( 
            /* [in] */ ULONG hInstance,
            /* [in] */ BSTR lpstrCmdLine,
            /* [in] */ LONG nCmdShow) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMainObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMainObject * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMainObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMainObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMainObject * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMainObject * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMainObject * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMainObject * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Run )( 
            IMainObject * This,
            /* [in] */ ULONG hInstance,
            /* [in] */ BSTR lpstrCmdLine,
            /* [in] */ LONG nCmdShow);
        
        END_INTERFACE
    } IMainObjectVtbl;

    interface IMainObject
    {
        CONST_VTBL struct IMainObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMainObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMainObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMainObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMainObject_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMainObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMainObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMainObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMainObject_Run(This,hInstance,lpstrCmdLine,nCmdShow)	\
    (This)->lpVtbl -> Run(This,hInstance,lpstrCmdLine,nCmdShow)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMainObject_Run_Proxy( 
    IMainObject * This,
    /* [in] */ ULONG hInstance,
    /* [in] */ BSTR lpstrCmdLine,
    /* [in] */ LONG nCmdShow);


void __RPC_STUB IMainObject_Run_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMainObject_INTERFACE_DEFINED__ */


#ifndef __IMain_INTERFACE_DEFINED__
#define __IMain_INTERFACE_DEFINED__

/* interface IMain */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IMain;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9AFFA8E7-B4C7-46f3-81EE-801B6D12ABFF")
    IMain : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IMainVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMain * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMain * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMain * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMain * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMain * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMain * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMain * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IMainVtbl;

    interface IMain
    {
        CONST_VTBL struct IMainVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMain_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMain_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMain_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMain_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMain_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMain_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMain_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IMain_INTERFACE_DEFINED__ */



#ifndef __Main_LIBRARY_DEFINED__
#define __Main_LIBRARY_DEFINED__

/* library Main */
/* [helpstring][uuid][version] */ 


EXTERN_C const IID LIBID_Main;

#ifndef ___IMainObjectEvents_DISPINTERFACE_DEFINED__
#define ___IMainObjectEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IMainObjectEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__IMainObjectEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("57335A8D-317A-43B2-BC48-392170220B49")
    _IMainObjectEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IMainObjectEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IMainObjectEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IMainObjectEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IMainObjectEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IMainObjectEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IMainObjectEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IMainObjectEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IMainObjectEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _IMainObjectEventsVtbl;

    interface _IMainObjectEvents
    {
        CONST_VTBL struct _IMainObjectEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IMainObjectEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IMainObjectEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IMainObjectEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IMainObjectEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IMainObjectEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IMainObjectEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IMainObjectEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IMainObjectEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CMainObject;

#ifdef __cplusplus

class DECLSPEC_UUID("194F2C2F-7394-44BD-8E83-03923F98189F")
CMainObject;
#endif
#endif /* __Main_LIBRARY_DEFINED__ */

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



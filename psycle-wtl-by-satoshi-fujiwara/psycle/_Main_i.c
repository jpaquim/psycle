

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


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

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IMainObject,0x2CA78C1E,0xD645,0x4B67,0x89,0x79,0x68,0xBD,0x9D,0xF2,0xD7,0xDA);


MIDL_DEFINE_GUID(IID, IID_IMain,0x9AFFA8E7,0xB4C7,0x46f3,0x81,0xEE,0x80,0x1B,0x6D,0x12,0xAB,0xFF);


MIDL_DEFINE_GUID(IID, LIBID_Main,0x79CF997C,0x9828,0x4C28,0xAE,0xE8,0x68,0xAA,0xA4,0x44,0x25,0x13);


MIDL_DEFINE_GUID(IID, DIID__IMainObjectEvents,0x57335A8D,0x317A,0x43B2,0xBC,0x48,0x39,0x21,0x70,0x22,0x0B,0x49);


MIDL_DEFINE_GUID(CLSID, CLSID_CMainObject,0x194F2C2F,0x7394,0x44BD,0x8E,0x83,0x03,0x92,0x3F,0x98,0x18,0x9F);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/


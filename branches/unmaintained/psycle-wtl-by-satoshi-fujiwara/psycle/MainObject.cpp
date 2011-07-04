// MainObject.cpp : CMainObject ‚ÌŽÀ‘•

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

#include "_PsycleWTLScript.h"
#include "_PsycleWTLScript_i.c"

#include "MainObject.h"

extern void testmain();

HRESULT CMainObject::Run(ULONG hInstance,BSTR lpstrCmdLine,LONG nCmdShow)
{
	//_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

#ifdef _DEBUG 
	testmain();
#else
#endif
	return S_OK;
};

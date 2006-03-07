/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
// --------------------------------------------------------------------------
// Description : several WTL utility classes for managing the layout of window children
// Author	   : Serge Weinstock
//
//	You are free to use, distribute or modify this code
//	as long as this header is not removed or modified.
// --------------------------------------------------------------------------
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

#include "LayoutMgr.h"


//=============================================================================
// Private messages
//=============================================================================
const RegisteredWindowMessage	WMU_PS_ATTACH_PAGE(_T("WTLU_LM::WMU_PS_ATTACH_PAGE"));
const RegisteredWindowMessage	WMU_PS_INIT(_T("WTLU_LM::WMU_PS_INIT"));
const RegisteredWindowMessage	WMU_PS_ATTACH_PS(_T("WTLU_LM::WMU_PS_ATTACH_PS"));

//=============================================================================
// 
//=============================================================================
int SysInfo::_isThemeSupported = -1;

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__FE0D36E6_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)
#define AFX_STDAFX_H__FE0D36E6_E90A_11D3_8913_9F3AED8AB763__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_MEAN_AND_LEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers



#if defined(_WINAMP_PLUGIN_)

//	#include <afxwin.h>         // MFC core and standard components
	#include <windows.h>

	#if defined(FOO_INPUT_STD_EXPORTS)
		#include "FoobarPlugin\resource.h"
	#else
		#include "WinampPlugin\resource.h"
	#endif	

#else

	#include <afxwin.h>         // MFC core and standard components
	#include <afxext.h>         // MFC extensions
	#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
	#ifndef _AFX_NO_AFXCMN_SUPPORT
	#include <afxcmn.h>			// MFC support for Windows Common Controls
	#endif // _AFX_NO_AFXCMN_SUPPORT
	#include <afxmt.h>

#endif //!defined(_WINAMP_PLUGIN_)

#include "Global.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__FE0D36E6_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)


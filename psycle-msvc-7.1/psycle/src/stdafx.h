#include <project.h>
#ifndef AFX
#define AFX
#pragma once

#if defined _WINAMP_PLUGIN_
#	include <afxwin.h> // MFC core and standard components
#	if defined FOO_INPUT_STD_EXPORTS
#		include "FoobarPlugin\resource.h"
#	else
#		include "WinampPlugin\resource.h"
#	endif
#else
#	include <afxwin.h> // MFC core and standard components
#	include <afxext.h> // MFC extensions
#	include <afxdtctl.h> // MFC support for Internet Explorer 4 Common Controls
#	if !defined _AFX_NO_AFXCMN_SUPPORT
#		include <afxcmn.h> // MFC support for Windows Common Controls
#	endif
#	include <afxmt.h>
#endif

#include <psycle/host/Global.h> // missing #include in psycle's source code

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX)

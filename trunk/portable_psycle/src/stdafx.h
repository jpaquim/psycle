#include <project.h>
#pragma once

#	define WINVER 0x410 ///< this is needed for compatibility with mswin98
#	include <afxwin.h> ///< MFC core and standard components

#if defined _WINAMP_PLUGIN_
#	if defined FOO_INPUT_STD_EXPORTS
#		include "FoobarPlugin\resource.h"
#	else
#		include "WinampPlugin\resource.h"
#	endif
#else
#	include <afxext.h> ///< MFC extensions
#	include <afxdtctl.h> ///< MFC support for Internet Explorer 4 Common Controls
#	if !defined _AFX_NO_AFXCMN_SUPPORT
#		include <afxcmn.h> ///< MFC support for Windows Common Controls
#	endif
#	include <afxmt.h>
#endif

#include <psycle/host/Global.h> // missing #include in psycle's source code

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

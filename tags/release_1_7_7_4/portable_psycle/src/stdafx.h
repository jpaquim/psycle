#pragma once

///\file
///\brief precompiled headers, including mfc.

#include <project.h> // project-wide compiler, operating system, and processor specific tweaks.

/// this is needed for compatibility of mfc with mswin98.
#define WINVER 0x410

/// mswinxp.
//#define WINVER 0x501
/// mswinxp.
//#define _WIN32_WINNT 0x501

#include <afxwin.h> // MFC core and standard components.

#if defined _WINAMP_PLUGIN_
	#if defined FOO_INPUT_STD_EXPORTS
		#include "FoobarPlugin\resource.h"
	#else
		#include "WinampPlugin\resource.h"
	#endif
#else
	#include <afxext.h> // MFC extensions.
	#include <afxdtctl.h> // MFC support for Internet Explorer 4 Common Controls.
	#if !defined _AFX_NO_AFXCMN_SUPPORT
		#include <afxcmn.h> // MFC support for Windows Common Controls.
	#endif
	#include <afxmt.h>
#endif

#include <psycle/host/Global.h> // missing #includes in psycle's source code.

// GDI+.
#include <gdiplus.h>				
#pragma comment(lib, "gdiplus")

#pragma once
/** @file 
 *  @brief implementation file
 *  stdafx.h : include file for standard system include files,
 *  or project specific include files that are used frequently, but
 *     are changed infrequently
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.9 $
 */
#pragma warning(disable:4275)
#define _WTL_NO_CSTRING
#define _CSTRING_NS
//#define _WTL_USE_CSTRING
#define _WTL_NEW_PAGE_NOTIFY_HANDLER
#define TRACE ATLTRACE
#define VERIFY(e) e

//#define _PORTING

// Change these values to use different versions
//#define WINVER		0x0501
//#define _WIN32_WINDOWS 0x0490
//#define WINVER		0x0490
//#define _WIN32_WINNT	0x0400

#define _WIN32_IE	0x0500
#define _RICHEDIT_VER	0x0100
#define RES_STR_MAX 512; // Resource String Size

#define _ATL_APARTMENT_THREADED
//#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 一部の CString コンストラクタは明示的です。

// 一般的で無視しても安全な MFC の警告メッセージの一部の非表示を解除します。
#define _ATL_ALL_WARNINGS

#include <tchar.h>

// ATL & WTL

#include <atldef.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
//#include <atltypes.h>
#include <atlctl.h>
#include <atlhost.h>
//using namespace ATL;

#include <atlsync.h>
#include <atlapp.h>
#include "shellapi.h"
//
//
#include <atlwin.h>
#include <atlcrack.h>
#include <atlstr.h>
#include <atlframe.h>
#include <atldlgs.h>
#include "atlctrls_sf.h"
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atlsplit.h>
#include "atlddxe.h"
#include <atlmisc.h>
#include "resource.h"
#include "resourceobject.h"
#include <atlcoll.h>
#include <atlscrl.h>

// WIN32 GUI Generics

#include <win32gui/window.hpp>
#include <win32gui/controls.hpp>
#include <win32gui/event_handler.hpp>
#include <win32gui/frame.hpp>

// Win32GUI Window base class 
using win32::gui::window_base;
using win32::gui::create_wnd;
// ... null window
using win32::gui::null_wnd;

// for extending windows/dialogs
using win32::gui::wnd_extend;

// Dialog class
using win32::gui::dialog;
using win32::gui::create_dlg;
using win32::gui::create_modal_dlg;

// for defining event handlers
using win32::gui::event_handler;

// Resizing Ability
using win32::gui::resizable_wnd;

// Frames
using win32::gui::view_frame;
using win32::gui::sdi_frame;
using win32::gui::mdi_frame;


// STL
#include <iostream>
#include <exception>
#include <memory>
#include <string>
#include <map>
#include <locale>
#include <fstream>

// Boost
#include <boost/serialization/version.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/scoped_ptr.hpp> 
#include <boost/scoped_array.hpp> 
#include <boost/shared_ptr.hpp> 
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/optional.hpp>
//
//#import "msxml.dll"

//namespace SF {
	typedef boost::basic_format<TCHAR,std::char_traits<TCHAR > > format;
//	typedef std::basic_string<TCHAR,std::char_traits<TCHAR > > string;
//
//}

// よく使うもの

//#include "Global.h"
//#include "controller.h"
#include "Constants.h"
#include "simpledialog.h"
#include "sfstreamop.h"
#include "sfhelper.h"
#include "MessageLoopContainer.h"
#include "mmsystem.h"
#include "MenuXP.h"


#define MSG_WM_KEYDOWN2(func) \
	if (uMsg == WM_KEYDOWN) \
	{ \
		SetMsgHandled(TRUE); \
		func((UINT)wParam, (UINT)lParam & 0xFFFF, (UINT)((lParam & 0xFFFF0000) >> 16)); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MSG_WM_KEYUP2(func) \
	if (uMsg == WM_KEYUP) \
	{ \
		SetMsgHandled(TRUE); \
		func((UINT)wParam, (UINT)lParam & 0xFFFF, (UINT)((lParam & 0xFFFF0000) >> 16)); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}
//#include "uxtheme.h"

// Direct X Headers
#include <D3D9.h>
#include <dxerr9.h>
#include "DXUtil.h"
#include "D3DUtil.h"
#include "d3denumeration.h"
#include "d3dsettings.h"
#include "d3dapp.h"
#include <gdiplus.h>



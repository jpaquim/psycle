#pragma once
/** @file 
 *  @brief 
 *  $Date: 2004/11/09 12:40:35 $
 *  $Revision: 1.1 $
 */
#include "iwindow.h"
namespace SF {


struct ATL_NO_VTABLE IWindowFactory
{
	virtual const IWindow* Create(const SF::WINDOW_TYPE type,const HWND parentHwnd,const TCHAR *pDllName,const TCHAR *pClassName) = 0;
};

}

#pragma once
/** @file 
 *  @brief 
 *  $Date: 2005/01/15 22:44:35 $
 *  $Revision: 1.2 $
 */
#pragma push_macro("new")
#undef new
#include <map>
#include "ManagedWindowImpl.h"
#include "..\iwindowfactory.h"

#using <mscorlib.dll>
#using "System.Windows.Forms.dll"
#using "..\XMSamplerUI\obj\Debug\XMSamplerUI.dll"
#include <vcclr.h>

#pragma push_macro("GetCurrentDirectory")
#undef GetCurrentDirectory
namespace SF {

__nogc class WindowFactoryImpl : public IWindowFactory
{
public:
	typedef const IWindow * (*pCreateWindowFunc)(const HWND,const TCHAR*,const TCHAR*);

	WindowFactoryImpl(void)
	{

	};

	virtual ~WindowFactoryImpl(void)
	{

	};

	const IWindow* Create(const SF::WINDOW_TYPE windowType,const HWND parentHwnd,const TCHAR *pDllName,const TCHAR *pClassName);
	
private:
	std::map<SF::WINDOW_TYPE,pCreateWindowFunc> m_CreateFuncMap;
};

const IWindow* WindowFactoryImpl::Create(const SF::WINDOW_TYPE windowType,const HWND parentHwnd,const TCHAR *pDllName,const TCHAR *pClassName)
{
	return (*m_CreateFuncMap[windowType])(parentHwnd,pDllName,pClassName);
};

}
#pragma pop_macro("GetCurrentDirectory")
#pragma pop_macro("new")


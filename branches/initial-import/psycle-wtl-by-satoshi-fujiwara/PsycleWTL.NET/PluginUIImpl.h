#pragma once
#pragma once
/** @file 
 *  @brief 
 *  $Date: 2004/11/09 12:39:21 $
 *  $Revision: 1.1 $
 */
#pragma push_macro("new")
#undef new

#include "ManagedWindowImpl.h"

#using <mscorlib.dll>
#using "System.Windows.Forms.dll"
#using "XMSamplerUI\obj\Debug\XMSamplerUI.dll"
#include <vcclr.h>

#pragma push_macro("GetCurrentDirectory")
#undef GetCurrentDirectory

#include "ipluginui.h"
#include "IMachine.h"
namespace SF {
	class PluginUIImpl :
		public IPluginUI
	{
	public:
		#pragma managed
		PluginUIImpl(const HWND parentHwnd,const TCHAR *pDllName,const TCHAR *pClassName,IMachine *pMachine)
		{
			m_pMWindow = new ManagedWindowImpl(parentHwnd,pDllName,pClassName);
			m_pMachine = pMachine;
		}

		virtual ~PluginUIImpl(void)
		{
			m_pMWindow->Close();
		}
		
		virtual const IMachine * pMachine(){return m_pMachine;};
		
		void Close(){m_pMWindow->Close();};
		void Show(){m_pMWindow->Show();};

	private:
		ManagedWindowImpl* m_pMWindow;
		IMachine * m_pMachine;
	};
}

#pragma pop_macro("GetCurrentDirectory")
#pragma pop_macro("new")


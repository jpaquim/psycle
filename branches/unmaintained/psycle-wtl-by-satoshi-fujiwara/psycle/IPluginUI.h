#pragma once
/** @file 
 *  @brief 
 *  $Date: 2004/11/09 12:40:35 $
 *  $Revision: 1.1 $
 */

#include "IWindow.h"
#include "IMachine.h"

namespace SF {

class ATL_NO_VTABLE IPluginUI : public IWindow
{
public:
	~IPluginUI(void) = 0;
	virtual const IMachine * pMachine() = 0;
};
}

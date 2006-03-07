#pragma once
/** @file 
 *  @brief 
 *  $Date: 2004/11/09 12:40:35 $
 *  $Revision: 1.1 $
 */
namespace SF {
enum WINDOW_TYPE 
{
	NATIVE_WINDOW,
	ACTIVEX_WINDOW,
	MANAGE_WINDOW
};

#pragma unmanaged
/** IWindow Interface */
class IWindow
{
public:
	virtual void Show() = 0;///< Show Window
	virtual void Close() = 0;///< Close Window
};
}

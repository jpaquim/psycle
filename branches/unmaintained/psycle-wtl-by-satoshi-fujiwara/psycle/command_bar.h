#pragma once

/** @file
 *  @brief  
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.1 $
 */

namespace win32 { 
	namespace gui {
	struct command_bar : wnd_extend<window_base,command_bar>
	{
		command_bar(const int menu_id);
	    static create_info def_create_info();
	};

};

};

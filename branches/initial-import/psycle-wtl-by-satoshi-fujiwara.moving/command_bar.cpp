#include "stdafx.h"
#include "command_bar.h"
namespace win32 { 
	namespace gui {
		command_bar::command_bar(const int menu_id)
		{

		}

		//
		//command_bar::~command_bar(void)
		//{
		//}

		create_info command_bar::def_create_info() {
			return create_info()
				.class_name(_T("CommandBar"));
		}
	}
}

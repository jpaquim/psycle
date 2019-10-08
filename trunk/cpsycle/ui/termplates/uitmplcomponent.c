#include "uiname.h"

void ui_name_init(void* self, ui_name* name, ui_component* parent)
{  
	ui_win32_component_init(&self->component, parent, TEXT(""), 0, 1);	
}



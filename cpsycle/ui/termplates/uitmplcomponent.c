#include "uiname.h"

void ui_name_init(void* self, ui_name* name, psy_ui_Component* parent)
{  
	ui_win_component_init(&self->component, parent, TEXT(""), 0, 1);	
}



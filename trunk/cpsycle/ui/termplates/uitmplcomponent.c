#include "uiname.h"
#include "hashtbl.h"

extern IntHashTable selfmap;

void ui_name_init(void* self, ui_name* name, ui_component* parent)
{  
    memset(&name->component.events, 0, sizeof(ui_events));
	name->component.hwnd = CreateWindow (TEXT(""), NULL,
		WS_CHILD | WS_VISIBLE | LBS_STANDARD,
		0, 0, 90, 90,
		parent->hwnd, NULL,
		(HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE),
		NULL);		
	InsertIntHashTable(&selfmap, (int)name->component.hwnd, &name->component.events);	
	name->component.events.self = self;
}



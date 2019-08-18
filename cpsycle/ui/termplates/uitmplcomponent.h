#if !defined(UIname_H)
#define UIname_H

#include "uicomponent.h"

typedef struct {
   ui_component component;
} ui_name;


void ui_name_init(void* self, ui_name* name, ui_component* parent);

#endif
#if !defined(UIname_H)
#define UIname_H

#include "uicomponent.h"

typedef struct {
   psy_ui_Component component;
} ui_name;

void ui_name_init(void* self, ui_name* name, psy_ui_Component* parent);

#endif
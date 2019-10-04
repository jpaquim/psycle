// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(UIBUTTON_H)
#define UIBUTTON_H

#include "uicomponent.h"

typedef enum {
	UI_ICON_NONE,
	UI_ICON_LESS,
	UI_ICON_MORE,
	UI_ICON_MOREMORE,
	UI_ICON_LESSLESS
} ButtonIcon;

typedef struct {
   ui_component component;
   Signal signal_clicked;    
   int ownerdrawn;
   char* text; // ownerdrawn 
   int hover;
   ButtonIcon icon;
} ui_button;

void ui_button_init(ui_button* button, ui_component* parent);
void ui_button_settext(ui_button* button, const char* text);
void ui_button_seticon(ui_button* button, ButtonIcon);
void ui_button_highlight(ui_button* button);
void ui_button_disablehighlight(ui_button* button);

#endif
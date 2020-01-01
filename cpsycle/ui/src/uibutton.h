// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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
	psy_ui_Component component;
	psy_Signal signal_clicked;
	int ownerdrawn;
	char* text; // ownerdrawn 
	int hover;
	int highlight;
	ButtonIcon icon;
	int charnumber;
	UiAlignment textalignment;
} psy_ui_Button;

void ui_button_init(psy_ui_Button*, psy_ui_Component* parent);
void ui_button_settext(psy_ui_Button*, const char* text);
void ui_button_seticon(psy_ui_Button*, ButtonIcon);
void ui_button_highlight(psy_ui_Button*);
void ui_button_disablehighlight(psy_ui_Button*);
void ui_button_setcharnumber(psy_ui_Button*, int number);
void ui_button_settextalignment(psy_ui_Button*, UiAlignment);

#endif

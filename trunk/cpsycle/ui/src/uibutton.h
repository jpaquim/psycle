// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_BUTTON_H
#define psy_ui_BUTTON_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	psy_ui_ICON_NONE,
	psy_ui_ICON_LESS,
	psy_ui_ICON_MORE,
	psy_ui_ICON_MOREMORE,
	psy_ui_ICON_LESSLESS
} psy_ui_ButtonIcon;

typedef struct {
	psy_ui_Component component;
	psy_Signal signal_clicked;	
	char* text;
	int hover;
	int highlight;
	psy_ui_ButtonIcon icon;
	int charnumber;
	psy_ui_Alignment textalignment;
} psy_ui_Button;

void psy_ui_button_init(psy_ui_Button*, psy_ui_Component* parent);
void psy_ui_button_settext(psy_ui_Button*, const char* text);
void psy_ui_button_seticon(psy_ui_Button*, psy_ui_ButtonIcon);
void psy_ui_button_highlight(psy_ui_Button*);
void psy_ui_button_disablehighlight(psy_ui_Button*);
void psy_ui_button_setcharnumber(psy_ui_Button*, int number);
void psy_ui_button_settextalignment(psy_ui_Button*, psy_ui_Alignment);

INLINE psy_ui_Component* psy_ui_button_base(psy_ui_Button* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_BUTTON_H */

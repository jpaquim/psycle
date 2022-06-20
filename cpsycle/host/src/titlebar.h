/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(TITLEBAR_H)
#define TITLEBAR_H

/* ui */
#include <uibutton.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TitleBar */

typedef struct TitleBar {
	/*/ inherits */
	psy_ui_Component component;
	psy_ui_Component client;
	psy_ui_Label title;
	psy_ui_Button hide;
	char* dragid;
} TitleBar;

void titlebar_init(TitleBar*, psy_ui_Component* parent, const char* title);

void titlebar_hide_on_close(TitleBar*);
void titlebar_enable_drag(TitleBar*, const char* dragid);

INLINE psy_ui_Component* titlebar_base(TitleBar* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* TITLEBAR_H */

/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(CLOCKBAR_H)
#define CLOCKBAR_H

#include "uilabel.h"
#include "uibutton.h"
#include "workspace.h"
/* std */
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 
** ClockBar
* 
** Displays the edit time of a song
*/

typedef struct ClockBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label header;
	psy_ui_Label position;
	time_t start;
	bool display_minutes;
} ClockBar;

void clockbar_init(ClockBar*, psy_ui_Component* parent);

void clockbar_reset(ClockBar*);
void clockbar_start(ClockBar*);
void clockbar_stop(ClockBar*);

INLINE void clockbar_display_minutes(ClockBar* self)
{
	self->display_minutes = TRUE;
}

INLINE psy_ui_Component* clockbar_base(ClockBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* CLOCKBAR_H */

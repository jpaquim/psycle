/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "clockbar.h"

/* platform */
#include "../../detail/portable.h"

#define CLOCKBAR_REFRESHRATE 200

/* prototypes */
static void clockbar_update_label(ClockBar*);
static void clockbar_on_timer(ClockBar*, uintptr_t timer_id);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ClockBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);		
		vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			clockbar_on_timer;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(clockbar_base(self), &vtable);
}

/* implementation */
void clockbar_init(ClockBar* self, psy_ui_Component* parent)
{		
	psy_ui_component_init(&self->component, parent, NULL);	
	vtable_init(self);
	self->display_minutes = FALSE;
	self->start = time(NULL);
	psy_ui_component_set_align_expand(clockbar_base(self), psy_ui_HEXPAND);
	psy_ui_component_set_default_align(clockbar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_label_init(&self->position, clockbar_base(self));
	psy_ui_label_prevent_translation(&self->position);
	psy_ui_label_set_char_number(&self->position, 10.0);
	clockbar_update_label(self);
}

void clockbar_update_label(ClockBar* self)
{
	time_t currtime;
	char text[80];

	currtime = time(NULL) - self->start;
	if (self->display_minutes) {
		psy_snprintf(text, 40, "%02ldm %02lds",
			(int)(currtime / 60.0), (int)(currtime) % 60);
	} else {
		psy_snprintf(text, 40, "%02ld:%02ld",
			(int)(currtime / 3600.0), (int)(currtime / 60.0) % 60);
	}
#ifndef PSYCLE_DEBUG_PREVENT_TIMER_DRAW
	psy_ui_label_set_text(&self->position, text);
#endif
}

void clockbar_reset(ClockBar* self)
{
	self->start = time(NULL);
	clockbar_update_label(self);
}

void clockbar_start(ClockBar* self)
{
	psy_ui_component_start_timer(clockbar_base(self), 0, CLOCKBAR_REFRESHRATE);
}

void clockbar_stop(ClockBar* self)
{
	psy_ui_component_stop_timer(clockbar_base(self), 0);
}

void clockbar_on_timer(ClockBar* self, uintptr_t timer_id)
{
	clockbar_update_label(self);
}

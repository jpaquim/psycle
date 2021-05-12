// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "clockbar.h"

#include <songio.h>

#include <stdio.h>

#include "../../detail/portable.h"

static void clockbar_updatelabel(ClockBar*);
static void clockbar_onsongchanged(ClockBar*, Workspace*,
	int flag, psy_audio_Song*);

void clockbar_init(ClockBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	self->start = time(NULL);
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;	
	psy_ui_label_init(&self->position, &self->component, NULL);	
	psy_ui_label_preventtranslation(&self->position);
	psy_ui_label_setcharnumber(&self->position, 6);	
	clockbar_updatelabel(self);
	psy_signal_connect(&workspace->signal_songchanged, self,
		clockbar_onsongchanged);
}

void clockbar_idle(ClockBar* self)
{	
	clockbar_updatelabel(self);	
}

void clockbar_updatelabel(ClockBar* self)
{

	time_t currtime;
	char text[80];

	currtime = time(NULL) - self->start;		
	psy_snprintf(text, 40, "%02ld:%02ld",
		(int)(currtime / 3600.0), (int)(currtime / 60.0) % 60);
	psy_ui_label_settext(&self->position, text);
}

void clockbar_onsongchanged(ClockBar* self, Workspace* sender, int flag,
	psy_audio_Song* song)
{
	self->start = time(NULL);
	clockbar_updatelabel(self);
}

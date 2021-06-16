/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "playposbar.h"
/* audio */
#include <songio.h>
/* ui */
#include <uiapp.h>
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void playposbar_ontimer(PlayPosBar*, uintptr_t timerid);
static void playposbar_updatelabel(PlayPosBar*);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PlayPosBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			playposbar_ontimer;
		vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(&self->component, &vtable);
}
/* implementation */
void playposbar_init(PlayPosBar* self, psy_ui_Component* parent,
	psy_audio_Player* player)
{
	assert(self);
	assert(player);

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HEXPAND);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->player = player;	
	psy_ui_label_init(&self->position, &self->component, NULL);	
	psy_ui_label_preventtranslation(&self->position);
	psy_ui_label_setcharnumber(&self->position, 20);	
	playposbar_updatelabel(self);	
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void playposbar_ontimer(PlayPosBar* self, uintptr_t timerid)
{	
	playposbar_updatelabel(self);	
}

void playposbar_updatelabel(PlayPosBar* self)
{
	psy_dsp_seconds_t currplaytime;
	char text[80];

	currplaytime = psy_audio_sequencer_currplaytime(
		&self->player->sequencer);
	psy_snprintf(text, 40, "%02dm%02ds %.2fb",
		(int)(currplaytime / 60), ((int)currplaytime % 60),
		(float)psy_audio_player_position(self->player));
	psy_ui_label_settext(&self->position, text);
}

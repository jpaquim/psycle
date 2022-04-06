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
static void playposbar_updatelabel(PlayPosBar*);

/* implementation */
void playposbar_init(PlayPosBar* self, psy_ui_Component* parent,
	psy_audio_Player* player)
{
	assert(self);
	assert(player);

	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_set_defaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->player = player;	
	psy_ui_label_init(&self->position, &self->component);	
	psy_ui_label_prevent_translation(&self->position);
	psy_ui_label_set_charnumber(&self->position, 24.0);
	playposbar_updatelabel(self);		
}

void playposbar_idle(PlayPosBar* self)
{	
	playposbar_updatelabel(self);	
}

void playposbar_updatelabel(PlayPosBar* self)
{
	psy_dsp_seconds_t currplaytime;
	char text[80];

	currplaytime = psy_audio_sequencer_curr_play_time(
		&self->player->sequencer);
	psy_snprintf(text, 40, "%02dm%02ds %.2fb %d/%d",
		(int)(currplaytime / 60), ((int)currplaytime % 60),
		(float)psy_audio_player_position(self->player),
		(int)self->player->sequencer.seqtime.timesig_numerator,
		(int)self->player->sequencer.seqtime.timesig_denominator);
#ifndef PSYCLE_DEBUG_PREVENT_TIMER_DRAW
	psy_ui_label_set_text(&self->position, text);	
#endif
}

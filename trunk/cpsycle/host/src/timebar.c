/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "timebar.h"
/* host */
#include "styles.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void timebar_onlesslessclicked(TimeBar*, psy_ui_Button* sender);
static void timebar_onlessclicked(TimeBar*, psy_ui_Button* sender);
static void timebar_onmoreclicked(TimeBar*, psy_ui_Button* sender);
static void timebar_onmoremoreclicked(TimeBar*, psy_ui_Button* sender);
static void timebar_offsetbpm(TimeBar*, psy_dsp_big_beat_t bpm);

/* implementation */
void timebar_init(TimeBar* self, psy_ui_Component* parent,
	psy_audio_Player* player)
{
	assert(self);
	assert(player);

	psy_ui_component_init(timebar_base(self), parent, NULL);	
	psy_ui_component_set_align_expand(timebar_base(self), psy_ui_HEXPAND);
	psy_ui_component_set_default_align(timebar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->player = player;	
	/* bpm description label */
	psy_ui_label_init(&self->desc, timebar_base(self));
	psy_ui_label_set_text(&self->desc, "timebar.tempo");
#ifdef PSYCLE_TIMEBAR_OLD
	/* bpm - 10 */
	psy_ui_button_init_connect(&self->lessless, timebar_base(self),
		self, timebar_onlesslessclicked);
	psy_ui_button_set_icon(&self->lessless, psy_ui_ICON_LESSLESS);			
#endif
	/* bpm -1 */
	psy_ui_button_init_connect(&self->less, timebar_base(self),
		self, timebar_onlessclicked);
	self->less.click_mode = psy_ui_CLICK_MODE_REPEAT;
	self->less.first_repeat_rate = 300;
	self->less.repeat_rate = 50;
	psy_ui_button_set_icon(&self->less, psy_ui_ICON_LESS);
	/* bpm (realbpm) number label */
	valueui_init(&self->bpmlabel, timebar_base(self), NULL, 0,
		&player->tempo_param.machineparam);
	self->bpmlabel.paramtweak.tweakscale = 999;
	/* bpm +1 */
	psy_ui_button_init_connect(&self->more, timebar_base(self),
		self, timebar_onmoreclicked);		
	psy_ui_button_set_icon(&self->more, psy_ui_ICON_MORE);
	self->more.click_mode = psy_ui_CLICK_MODE_REPEAT;
	self->more.first_repeat_rate = 300;
	self->more.repeat_rate = 50;
#ifdef PSYCLE_TIMEBAR_OLD
	/* bpm + 10 */
	psy_ui_button_init_connect(&self->moremore, timebar_base(self),
		self, timebar_onmoremoreclicked);
	psy_ui_button_set_icon(&self->moremore, psy_ui_ICON_MOREMORE);
#endif		
	// psy_ui_component_start_timer(timebar_base(self), 0, TIMEBAR_REFRESHRATE);
}

void timebar_onlesslessclicked(TimeBar* self, psy_ui_Button* sender)
{
	assert(self);

	timebar_offsetbpm(self, -10);
}

void timebar_onlessclicked(TimeBar* self, psy_ui_Button* sender)
{
	assert(self);
	
	timebar_offsetbpm(self, (psy_ui_button_hasctrl(sender)) ? -10 : -1);
}

void timebar_onmoreclicked(TimeBar* self, psy_ui_Button* sender)
{
	assert(self);

	timebar_offsetbpm(self, (psy_ui_button_hasctrl(sender)) ? 10 : 1);
}

void timebar_onmoremoreclicked(TimeBar* self, psy_ui_Button* sender)
{
	assert(self);

	timebar_offsetbpm(self, 10);
}

void timebar_offsetbpm(TimeBar* self, psy_dsp_big_beat_t delta)
{
	assert(self);

	psy_audio_player_setbpm(self->player,
		psy_audio_player_bpm(self->player) + delta);
}

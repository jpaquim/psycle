/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "timebar.h"
/* host */
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

#define TIMEBAR_REFRESHRATE 50

/*
** TimeBar
** prototypes
*/
static void timebar_onlesslessclicked(TimeBar*, psy_ui_Button* sender);
static void timebar_onlessclicked(TimeBar*, psy_ui_Button* sender);
static void timebar_onmoreclicked(TimeBar*, psy_ui_Button* sender);
static void timebar_onmoremoreclicked(TimeBar*, psy_ui_Button* sender);
static void timebar_on_timer(TimeBar*, uintptr_t timerid);
static void timebar_offsetbpm(TimeBar*, psy_dsp_big_beat_t bpm);
static void timebar_updatebpmlabel(TimeBar*);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(TimeBar* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(timebar_base(self)->vtable);
		vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			timebar_on_timer;
		vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(timebar_base(self), &vtable);
}
/* implementation */
void timebar_init(TimeBar* self, psy_ui_Component* parent,
	psy_audio_Player* player)
{
	assert(self);
	assert(player);

	psy_ui_component_init(timebar_base(self), parent, NULL);
	vtable_init(self);
	psy_ui_component_setalignexpand(timebar_base(self), psy_ui_HEXPAND);
	psy_ui_component_set_defaultalign(timebar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->player = player;
	self->bpm = self->realbpm = (psy_dsp_big_beat_t)0.0;
	/* bpm description label */
	psy_ui_label_init(&self->desc, timebar_base(self));
	psy_ui_label_set_text(&self->desc, "timebar.tempo");
#ifdef PSYCLE_TIMEBAR_OLD
	/* bpm - 10 */
	psy_ui_button_init_connect(&self->lessless, timebar_base(self),
		self, timebar_onlesslessclicked);
	psy_ui_button_seticon(&self->lessless, psy_ui_ICON_LESSLESS);			
#endif
	/* bpm -1 */
	psy_ui_button_init_connect(&self->less, timebar_base(self),
		self, timebar_onlessclicked);
	psy_ui_button_seticon(&self->less, psy_ui_ICON_LESS);
	/* bpm (realbpm) number label */
	psy_ui_label_init(&self->bpmlabel, timebar_base(self));
	psy_ui_component_set_style_type(psy_ui_label_base(&self->bpmlabel),
		STYLE_TIMEBAR_NUMLABEL);
	psy_ui_label_prevent_translation(&self->bpmlabel);
	psy_ui_label_set_charnumber(&self->bpmlabel, 14.5);
	psy_ui_label_set_textalignment(&self->bpmlabel, psy_ui_ALIGNMENT_CENTER);
	timebar_updatebpmlabel(self);
	/* bpm +1 */
	psy_ui_button_init_connect(&self->more, timebar_base(self),
		self, timebar_onmoreclicked);		
	psy_ui_button_seticon(&self->more, psy_ui_ICON_MORE);		
#ifdef PSYCLE_TIMEBAR_OLD
	/* bpm + 10 */
	psy_ui_button_init_connect(&self->moremore, timebar_base(self),
		self, timebar_onmoremoreclicked);
	psy_ui_button_seticon(&self->moremore, psy_ui_ICON_MOREMORE);
#endif		
	psy_ui_component_start_timer(timebar_base(self), 0, TIMEBAR_REFRESHRATE);
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

void timebar_on_timer(TimeBar* self, uintptr_t timerid)
{
	assert(self);	

	if (self->bpm != psy_audio_player_bpm(self->player) ||
		self->realbpm != psy_audio_player_realbpm(self->player)) {
			timebar_updatebpmlabel(self);
	}
}

void timebar_updatebpmlabel(TimeBar* self)
{
	char txt[64];

	assert(self);

	self->bpm = psy_audio_player_bpm(self->player);
	self->realbpm = psy_audio_player_realbpm(self->player);
	psy_snprintf(txt, 64, "%d (%.2f)", (int)self->bpm, self->realbpm);
	psy_ui_label_set_text(&self->bpmlabel, txt);
}

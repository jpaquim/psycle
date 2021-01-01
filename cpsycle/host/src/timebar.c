// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "timebar.h"
// std
#include <stdio.h>
// platform
#include "../../detail/portable.h"

#define TIMEBAR_REFRESHRATE 50

// TimeBar
// prototypes
static void timebar_onlesslessclicked(TimeBar*, psy_ui_Button* sender);
static void timebar_onlessclicked(TimeBar*, psy_ui_Button* sender);
static void timebar_onmoreclicked(TimeBar*, psy_ui_Button* sender);
static void timebar_onmoremoreclicked(TimeBar*, psy_ui_Button* sender);
static void timebar_ontimer(TimeBar*, uintptr_t timerid);
static void timebar_offsetbpm(TimeBar*, psy_dsp_big_beat_t bpm);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(TimeBar* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ontimer = (psy_ui_fp_component_ontimer)timebar_ontimer;		
		vtable_initialized = TRUE;
	}
	return &vtable;
}
// implementation
void timebar_init(TimeBar* self, psy_ui_Component* parent, Workspace* workspace)
{
	assert(self);
	assert(workspace);

	psy_ui_component_init(timebar_base(self), parent);
	psy_ui_component_setvtable(timebar_base(self), vtable_init(self));
	psy_ui_component_setalignexpand(timebar_base(self), psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setdefaultalign(timebar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;	
	self->bpm = (psy_dsp_big_beat_t)0.0;
	psy_ui_label_init(&self->bpmdesc, &self->component);
	psy_ui_label_settext(&self->bpmdesc, "timebar.tempo");
	psy_ui_label_init(&self->bpmlabel, &self->component);
	psy_ui_label_settext(&self->bpmlabel, "125");
	psy_ui_label_preventtranslation(&self->bpmlabel);
	psy_ui_label_setcharnumber(&self->bpmlabel, 8);	
#ifdef PSYCLE_TIMEBAR_OLD
	// bpm -10
	psy_ui_button_init_connect(&self->lessless, &self->component,
		self, timebar_onlesslessclicked);
	psy_ui_button_seticon(&self->lessless, psy_ui_ICON_LESSLESS);			
#endif
	// bpm -1
	psy_ui_button_init_connect(&self->less, timebar_base(self),
		self, timebar_onlessclicked);
	psy_ui_button_seticon(&self->less, psy_ui_ICON_LESS);		
	psy_ui_button_init_connect(&self->more, timebar_base(self),
		self, timebar_onmoreclicked);
	// bpm +1
	psy_ui_button_seticon(&self->more, psy_ui_ICON_MORE);		
#ifdef PSYCLE_TIMEBAR_OLD
	// bpm +10
	psy_ui_button_init_connect(&self->moremore, &self->component,
		self, timebar_onmoremoreclicked);
	psy_ui_button_seticon(&self->moremore, psy_ui_ICON_MOREMORE);
#endif		
	psy_ui_component_starttimer(timebar_base(self), 0, TIMEBAR_REFRESHRATE);
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

	psy_audio_player_setbpm(workspace_player(self->workspace),
		psy_audio_player_bpm(workspace_player(self->workspace)) + delta);
}

void timebar_ontimer(TimeBar* self, uintptr_t timerid)
{
	assert(self);	

	if (self->bpm != psy_audio_player_bpm(workspace_player(self->workspace))) {
		char txt[20];

		self->bpm = psy_audio_player_bpm(workspace_player(self->workspace));
		psy_snprintf(txt, 10, "%.2f", self->bpm);
		psy_ui_label_settext(&self->bpmlabel, txt);
	}
}

/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "swingfillview.h"
/* platform */
#include "../../detail/portable.h"

#define DESC_CHARNUM 22
#define EDIT_CHARNUM 10

/* Prototypes */
static void swingfillview_initactions(SwingFillView*);
static void swingfillview_onhide(SwingFillView*, psy_ui_Component* sender);
static void swingfillview_onactualbpm(SwingFillView*,
	psy_ui_Component* sender);
static void swingfillview_oncenterbpm(SwingFillView*,
	psy_ui_Component* sender);
static void swingfillview_setoffset(SwingFillView*, bool offset);
static void swingfillview_onapply(SwingFillView*, psy_ui_Component* sender);

/* implementation */
void swingfillview_init(SwingFillView* self, psy_ui_Component* parent,
	psy_ui_Component* view, PatternViewState* pvstate)
{
	assert(self);
	assert(pvstate);

	psy_ui_component_init(swingfillview_base(self), parent, view);	
	self->pvstate = pvstate;
	self->trackmodeswingfill = FALSE;
	psy_ui_component_init(&self->client, swingfillview_base(self), NULL);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setmargin(&self->client,
		psy_ui_defaults_cmargin(psy_ui_defaults()));
	psy_ui_component_setdefaultalign(&self->client, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	intedit_init(&self->tempo, &self->client, NULL, "swingfill.tempo", 125, 0, 0);
	intedit_seteditcharnumber(&self->tempo, EDIT_CHARNUM);
	intedit_setdesccharnumber(&self->tempo, DESC_CHARNUM);
	psy_ui_component_init(&self->offsetrow, &self->client, NULL);
	psy_ui_component_setdefaultalign(&self->offsetrow, psy_ui_ALIGN_RIGHT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_label_init_text(&self->offsetdesc, &self->offsetrow, NULL,
		"swingfill.bpm");
	psy_ui_button_init_text_connect(&self->center_bpm, &self->offsetrow, NULL,
		"swingfill.center", self, swingfillview_oncenterbpm);
	psy_ui_button_init_text_connect(&self->actual_bpm, &self->offsetrow, NULL,
		"swingfill.actual", self, swingfillview_onactualbpm);		
	swingfillview_setoffset(self, TRUE);
	intedit_init(&self->width, &self->client, NULL,
		"swingfill.cycle", 2, 0, 0);
	intedit_seteditcharnumber(&self->width, EDIT_CHARNUM);
	intedit_setdesccharnumber(&self->width, DESC_CHARNUM);	
	realedit_init(&self->variance, &self->client, NULL,
		"swingfill.variance", 13.f, 0, 100.0f);
	realedit_seteditcharnumber(&self->variance, EDIT_CHARNUM);
	realedit_setdesccharnumber(&self->variance, DESC_CHARNUM);
	realedit_init(&self->phase, &self->client, NULL,
		"swingfill.phase", -90.f, 0, 0);
	realedit_seteditcharnumber(&self->phase, EDIT_CHARNUM);
	realedit_setdesccharnumber(&self->phase, DESC_CHARNUM);		
	swingfillview_initactions(self);
	psy_ui_component_setalign(swingfillview_base(self), psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(swingfillview_base(self));
}

void swingfillview_initactions(SwingFillView* self)
{
	psy_ui_Margin margin;

	assert(self);

	psy_ui_component_init(&self->actions, &self->client, NULL);
	psy_ui_margin_init_em(&margin, 1.0, 0.0, 0.0, 0.0);		
	psy_ui_component_setmargin(&self->actions, margin);
	psy_ui_component_setdefaultalign(&self->actions,
		psy_ui_ALIGN_RIGHT, psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_button_init_text_connect(&self->cancel, &self->actions, NULL,
		"swingfill.cancel", self, swingfillview_onhide);
	psy_ui_button_init_text_connect(&self->apply, &self->actions, NULL,
		"swingfill.apply", self, swingfillview_onapply);
}

void swingfillview_reset(SwingFillView* self, int bpm)
{	
	assert(self);

	swingfillview_setvalues(self, bpm, 2, 13.f, -90.f, TRUE);
}

void swingfillview_setvalues(SwingFillView* self, int tempo, int width,
	float variance, float phase, bool offset)
{
	assert(self);

	intedit_setvalue(&self->tempo, tempo);
	intedit_setvalue(&self->width, width);
	realedit_setvalue(&self->variance, variance);
	realedit_setvalue(&self->phase, phase);
	swingfillview_setoffset(self, offset);
}

psy_audio_SwingFill swingfillview_values(SwingFillView* self)
{
	psy_audio_SwingFill rv;
	
	assert(self);

	rv.tempo = intedit_value(&self->tempo);
	rv.width = intedit_value(&self->width);
	rv.variance = realedit_value(&self->variance);
	rv.phase = realedit_value(&self->phase);
	rv.offset = self->offset;
	return rv;
}

void swingfillview_onhide(SwingFillView* self, psy_ui_Component* sender)
{
	assert(self);

	psy_ui_component_hide_align(&self->component);
}

void swingfillview_onactualbpm(SwingFillView* self, psy_ui_Component* sender)
{
	assert(self);

	swingfillview_setoffset(self, TRUE);
}

void swingfillview_oncenterbpm(SwingFillView* self, psy_ui_Component* sender)
{

	assert(self);

	swingfillview_setoffset(self, FALSE);	
}

void swingfillview_setoffset(SwingFillView* self, bool offset)
{
	assert(self);

	self->offset = offset;
	if (offset) {
		psy_ui_button_highlight(&self->actual_bpm);
		psy_ui_button_disablehighlight(&self->center_bpm);		
	} else {
		psy_ui_button_highlight(&self->center_bpm);
		psy_ui_button_disablehighlight(&self->actual_bpm);
	}	
}

void swingfillview_onapply(SwingFillView* self, psy_ui_Component* sender)
{
	assert(self);

	if (self->trackmodeswingfill || psy_audio_blockselection_valid(
			&self->pvstate->selection)) {
		psy_audio_pattern_swingfill(patternviewstate_pattern(self->pvstate),
			&self->pvstate->selection,
			self->trackmodeswingfill,
			patternviewstate_bpl(self->pvstate),
			swingfillview_values(self));
	}
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "swingfillview.h"
// platform
#include "../../detail/portable.h"

#define DESC_CHARNUM 22
#define EDIT_CHARNUM 10

static void swingfillview_initactions(SwingFillView*);
static void swingfillview_onhide(SwingFillView*, psy_ui_Component* sender);
static void swingfillview_onactualbpm(SwingFillView*, psy_ui_Component* sender);
static void swingfillview_oncenterbpm(SwingFillView*, psy_ui_Component* sender);
static void swingfillview_setoffset(SwingFillView*, bool offset);

void swingfillview_init(SwingFillView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(swingfillview_base(self), parent, NULL);
	self->workspace = workspace;		
	psy_ui_component_init(&self->client, swingfillview_base(self), NULL);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setmargin(&self->client,
		psy_ui_defaults_cmargin(psy_ui_defaults()));
	psy_ui_component_setdefaultalign(&self->client, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	intedit_init(&self->tempo, &self->client, "Tempo(BPM)", 125, 0, 0);
	intedit_seteditcharnumber(&self->tempo, EDIT_CHARNUM);
	intedit_setdesccharnumber(&self->tempo, DESC_CHARNUM);
	psy_ui_component_init(&self->offsetrow, &self->client, NULL);
	psy_ui_component_setdefaultalign(&self->offsetrow, psy_ui_ALIGN_RIGHT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_label_init_text(&self->offsetdesc, &self->offsetrow, NULL, "BPM");
	psy_ui_button_init_text_connect(&self->center_bpm, &self->offsetrow, NULL,
		"Center", self, swingfillview_oncenterbpm);
	psy_ui_button_init_text_connect(&self->actual_bpm, &self->offsetrow, NULL,
		"Actual", self, swingfillview_onactualbpm);		
	swingfillview_setoffset(self, TRUE);
	intedit_init(&self->width, &self->client,
		"Cycle Length(lines)", 2, 0, 0);
	intedit_seteditcharnumber(&self->width, EDIT_CHARNUM);
	intedit_setdesccharnumber(&self->width, DESC_CHARNUM);	
	realedit_init(&self->variance, &self->client,
		"Variance(%)", 13.f, 0, 100.0f);
	realedit_seteditcharnumber(&self->variance, EDIT_CHARNUM);
	realedit_setdesccharnumber(&self->variance, DESC_CHARNUM);
	realedit_init(&self->phase, &self->client,
		"Phase (degrees)", -90.f, 0, 0);
	realedit_seteditcharnumber(&self->phase, EDIT_CHARNUM);
	realedit_setdesccharnumber(&self->phase, DESC_CHARNUM);		
	swingfillview_initactions(self);	
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
		"Cancel", self, swingfillview_onhide);
	psy_ui_button_init_text(&self->apply, &self->actions, NULL, "Apply");
}


void swingfillview_reset(SwingFillView* self, int bpm)
{	
	swingfillview_setvalues(self, bpm, 2, 13.f, -90.f, TRUE);
}

void swingfillview_setvalues(SwingFillView* self, int tempo, int width,
	float variance, float phase, bool offset)
{
	intedit_setvalue(&self->tempo, tempo);
	intedit_setvalue(&self->width, width);
	realedit_setvalue(&self->variance, variance);
	realedit_setvalue(&self->phase, phase);
	swingfillview_setoffset(self, offset);
}

void swingfillview_values(SwingFillView* self, int* rv_tempo, int* rv_width,
	float* rv_variance, float* rv_phase, bool* rv_offset)
{
	*rv_tempo = intedit_value(&self->tempo);
	*rv_width = intedit_value(&self->width);
	*rv_variance = realedit_value(&self->variance);
	*rv_phase = realedit_value(&self->phase);
	*rv_offset = self->offset;
}

void swingfillview_onhide(SwingFillView* self, psy_ui_Component* sender)
{
	assert(self);

	psy_ui_component_hide_align(&self->component);
}

void swingfillview_onactualbpm(SwingFillView* self, psy_ui_Component* sender)
{
	swingfillview_setoffset(self, TRUE);
}

void swingfillview_oncenterbpm(SwingFillView* self, psy_ui_Component* sender)
{
	swingfillview_setoffset(self, FALSE);	
}

void swingfillview_setoffset(SwingFillView* self, bool offset)
{
	self->offset = offset;
	if (offset) {
		psy_ui_button_highlight(&self->actual_bpm);
		psy_ui_button_disablehighlight(&self->center_bpm);		
	} else {
		psy_ui_button_highlight(&self->center_bpm);
		psy_ui_button_disablehighlight(&self->actual_bpm);
	}	
}

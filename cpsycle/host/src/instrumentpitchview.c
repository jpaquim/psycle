/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "instrumentpitchview.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void instrumentpitchview_on_destroyed(InstrumentPitchView*);
static void instrumentpitchview_ontweaked(InstrumentPitchView*,
	psy_ui_Component*, int pointindex);
static void instrumentpitchview_onenvelopeviewtweaked(InstrumentPitchView*,
	psy_ui_Component* sender, int pointindex);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(InstrumentPitchView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			instrumentpitchview_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void instrumentpitchview_init(InstrumentPitchView* self,
	psy_ui_Component* parent, psy_audio_Instruments* instruments,
	Workspace* workspace)
{	
	psy_ui_Margin margin;

	self->instruments = instruments;
	self->instrument = 0;
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_signal_init(&self->signal_status);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));	
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_make_eh(1.0);		
	envelopeview_init(&self->envelopeview, &self->component);
	envelopeview_settext(&self->envelopeview, "instrumentview.pitch-envelope");
	psy_ui_component_set_margin(&self->envelopeview.component, margin);	
	adsrsliders_init(&self->adsrsliders, &self->component);
	psy_signal_connect(&self->adsrsliders.signal_tweaked, self,
		instrumentpitchview_ontweaked);
	psy_signal_connect(&self->envelopeview.envelopebox.signal_tweaked, self,
		instrumentpitchview_ontweaked);
	psy_signal_connect(&self->envelopeview.signal_tweaked, self,
		instrumentpitchview_onenvelopeviewtweaked);
}

void instrumentpitchview_on_destroyed(InstrumentPitchView* self)
{
	psy_signal_dispose(&self->signal_status);
}

void instrumentpitchview_setinstrument(InstrumentPitchView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	if (self->instrument) {
		adsrsliders_set_envelope(&self->adsrsliders,			
			&self->instrument->pitchenvelope);
		envelopeview_setenvelope(&self->envelopeview,
			&self->instrument->pitchenvelope);
	} else {
		adsrsliders_set_envelope(&self->adsrsliders, NULL);
		envelopeview_setenvelope(&self->envelopeview, NULL);
	}	
}

void instrumentpitchview_ontweaked(InstrumentPitchView* self,
	psy_ui_Component* sender, int pointindex)
{
	if (self->instrument) {
		char statustext[256];
		psy_dsp_EnvelopePoint pt;

		envelopeview_update(&self->envelopeview);
		pt = psy_dsp_envelope_at(&self->instrument->pitchenvelope, pointindex);
		psy_snprintf(statustext, 256, "Point %d (%f, %f)", pointindex,
			(float)pt.time, (float)pt.value);
		psy_signal_emit(&self->signal_status, self, 1, statustext);
	}
}

void instrumentpitchview_onenvelopeviewtweaked(InstrumentPitchView* self,
	psy_ui_Component* sender, int pointindex)
{
	adsrsliders_update(&self->adsrsliders);
}

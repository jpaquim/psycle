// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentpitchview.h"
// platform
#include "../../detail/portable.h"

// prototypes
void instrumentpitchview_ondestroy(InstrumentPitchView*,
	psy_ui_Component* sender);
static void instrumentpitchview_ontweaked(InstrumentPitchView*,
	psy_ui_Component*, int pointindex);
// implementation
void instrumentpitchview_init(InstrumentPitchView* self,
	psy_ui_Component* parent, psy_audio_Instruments* instruments,
	Workspace* workspace)
{	
	psy_ui_Margin margin;

	self->instruments = instruments;
	self->instrument = 0;
	psy_ui_component_init(&self->component, parent);
	psy_signal_connect(&self->component.signal_destroy, self,
		instrumentpitchview_ondestroy);
	psy_signal_init(&self->signal_status);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));	
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_makeeh(1.0);		
	envelopeview_init(&self->envelopeview, &self->component, workspace);
	envelopeview_settext(&self->envelopeview, "instrumentview.pitch-envelope");
	psy_ui_component_setmargin(&self->envelopeview.component, &margin);	
	adsrsliders_init(&self->adsrsliders, &self->component);
	psy_signal_connect(&self->adsrsliders.signal_tweaked, self,
		instrumentpitchview_ontweaked);
	psy_signal_connect(&self->envelopeview.envelopebox.signal_tweaked, self,
		instrumentpitchview_ontweaked);
}

void instrumentpitchview_ondestroy(InstrumentPitchView* self,
	psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_status);
}

void instrumentpitchview_setinstrument(InstrumentPitchView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	if (self->instrument) {
		adsrsliders_setenvelope(&self->adsrsliders,			
			&self->instrument->pitchenvelope);
		envelopeview_setenvelope(&self->envelopeview,
			&self->instrument->pitchenvelope);
	} else {
		adsrsliders_setenvelope(&self->adsrsliders, NULL);
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

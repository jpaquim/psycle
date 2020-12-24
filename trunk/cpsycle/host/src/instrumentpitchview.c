// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentpitchview.h"

// prototypes
static void instrumentpitchview_onadsrtweaked(InstrumentPitchView*,
	AdsrSliders*);
// implementation
void instrumentpitchview_init(InstrumentPitchView* self,
	psy_ui_Component* parent, psy_audio_Instruments* instruments,
	Workspace* workspace)
{	
	psy_ui_Margin margin;

	self->instruments = instruments;
	self->instrument = 0;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));	
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_makeeh(1.0);		
	envelopeview_init(&self->envelopeview, &self->component, workspace);
	envelopeview_settext(&self->envelopeview, "instrumentview.pitch-envelope");
	psy_ui_component_setmargin(&self->envelopeview.component, &margin);	
	adsrsliders_init(&self->adsrsliders, &self->component);
	psy_signal_connect(&self->adsrsliders.signal_tweaked, self,
		instrumentpitchview_onadsrtweaked);
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

void instrumentpitchview_onadsrtweaked(InstrumentPitchView* self,
	AdsrSliders* sender)
{
	envelopeview_update(&self->envelopeview);
}

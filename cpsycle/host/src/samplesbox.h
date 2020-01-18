// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net


#if !defined(SAMPLESBOX_H)
#define SAMPLESBOX_H

#include <uilistbox.h>
#include <uilabel.h>
#include "samples.h"
#include "instruments.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Listbox samplelist;
	psy_ui_Label header;
	psy_ui_Label group;
	psy_ui_Listbox subsamplelist;	
	psy_audio_Samples* samples;
	psy_audio_Instruments* instruments;
	psy_Signal signal_changed;
	int changeinstrumentslot;
} SamplesBox;

void samplesbox_init(SamplesBox*, psy_ui_Component* parent, psy_audio_Samples*, psy_audio_Instruments*);
void samplesbox_setsamples(SamplesBox*, psy_audio_Samples*, psy_audio_Instruments*);
SampleIndex samplesbox_selected(SamplesBox*);

#endif

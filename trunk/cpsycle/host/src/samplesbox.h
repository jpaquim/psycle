// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(SAMPLESBOX_H)
#define SAMPLESBOX_H

#include <uilistbox.h>
#include <uilabel.h>
#include "samples.h"
#include "instruments.h"

typedef struct {
	ui_component component;
	ui_listbox samplelist;
	ui_label header;
	ui_label group;
	ui_listbox subsamplelist;	
	Samples* samples;
	Instruments* instruments;
	psy_Signal signal_changed;
	int changeinstrumentslot;
} SamplesBox;

void samplesbox_init(SamplesBox*, ui_component* parent, Samples*, Instruments*);
void samplesbox_setsamples(SamplesBox*, Samples*, Instruments*);
SampleIndex samplesbox_selected(SamplesBox*);

#endif

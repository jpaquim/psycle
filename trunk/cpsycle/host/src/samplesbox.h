// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLESBOX_H)
#define SAMPLESBOX_H

#include "workspace.h"

#include <uilabel.h>
#include <uilistbox.h>

#include <samples.h>

#ifdef __cplusplus
extern "C" {
#endif

// Displays the samples of the current song. It is divided into two views.
// The upper displays the first sample of group, the lower all the samples
// of the selected group.

typedef struct SamplesBox {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_changed;
	// internal
	psy_ui_ListBox samplelist;
	psy_ui_Label header;
	psy_ui_Label group;
	psy_ui_ListBox subsamplelist;
	psy_Property sample_load;
	bool load_on_select;
	uintptr_t restore_view;
	// references
	psy_audio_Samples* samples;
	Workspace* workspace;
} SamplesBox;

void samplesbox_init(SamplesBox*, psy_ui_Component* parent, psy_audio_Samples*,
	Workspace*);
void samplesbox_setsamples(SamplesBox*, psy_audio_Samples*);
psy_audio_SampleIndex samplesbox_selected(SamplesBox*);
void samplesbox_select(SamplesBox*, psy_audio_SampleIndex);

#ifdef __cplusplus
}
#endif

#endif /* SAMPLESBOX_H */

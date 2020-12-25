// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTRUMENTAMPVIEW_H)
#define INSTRUMENTAMPVIEW_H

// host
#include "adsrsliders.h"
#include "envelopeview.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct InstrumentVolumeView {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_status;
	// ui elements
	psy_ui_Slider randomvolume;
	psy_ui_Slider volumefadespeed;
	EnvelopeView envelopeview;
	AdsrSliders adsrsliders;
	// references	
	psy_audio_Player* player;
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
} InstrumentVolumeView;

void instrumentvolumeview_init(InstrumentVolumeView*,
	psy_ui_Component* parent, psy_audio_Instruments*, Workspace*);
void instrumentvolumeview_setinstrument(InstrumentVolumeView*,
	psy_audio_Instrument*);

#ifdef __cplusplus
}
#endif

#endif /* INSTRUMENTAMPVIEW_H */

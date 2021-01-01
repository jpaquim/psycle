// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTRUMENTPITCHVIEW_H)
#define INSTRUMENTPITCHVIEW_H

// host
#include "adsrsliders.h"
#include "envelopeview.h"
#include "workspace.h"
// ui
#include <uislider.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct InstrumentPitchView {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_status;
	// ui elements
	EnvelopeView envelopeview;
	AdsrSliders adsrsliders;
	// references
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
} InstrumentPitchView;

void instrumentpitchview_init(InstrumentPitchView*,
	psy_ui_Component* parent, psy_audio_Instruments*, Workspace*);
void instrumentpitchview_setinstrument(InstrumentPitchView*,
	psy_audio_Instrument*);

#ifdef __cplusplus
}
#endif

#endif /* INSTRUMENTPITCHVIEW_H */

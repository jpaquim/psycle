// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTRUMENTPANVIEW_H)
#define INSTRUMENTPANVIEW_H

// host
#include "adsrsliders.h"
#include "envelopeview.h"
#include "workspace.h"
// ui
#include <uicheckbox.h>
#include <uislider.h>

#ifdef __cplusplus
extern "C" {
#endif

struct InstrumentView;

typedef struct InstrumentPanView {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_status;
	// ui elements
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
	psy_ui_Component top;
	psy_ui_Component instpan;
	psy_ui_CheckBox instpanenabled;
	psy_ui_Slider instpanning;
	psy_ui_Slider randompanning;
	psy_ui_Slider notemodcenternote;
	psy_ui_Slider notemodamount;
	EnvelopeView envelopeview;
	AdsrSliders adsrsliders;
	// references
	Workspace* workspace;
} InstrumentPanView;

void instrumentpanview_init(InstrumentPanView*, psy_ui_Component* parent,
	psy_audio_Instruments*, Workspace*);
void instrumentpanview_setinstrument(InstrumentPanView*,
	psy_audio_Instrument*);

#ifdef __cplusplus
}
#endif

#endif /* INSTRUMENTPANVIEW_H */

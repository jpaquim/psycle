// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTRUMENTFILTERVIEW_H)
#define INSTRUMENTFILTERVIEW_H

// host
#include "adsrsliders.h"
#include "envelopeview.h"
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uicombobox.h>
#include <uilabel.h>
#include <uislider.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct InstrumentFilterView {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_status;
	// ui elements
	psy_ui_Component top;
	psy_ui_Component filter;
	psy_ui_Label filtertypeheader;
	psy_ui_ComboBox filtertype;
	psy_ui_Slider randomcutoff;
	psy_ui_Slider randomresonance;
	EnvelopeView envelopeview;
	psy_ui_Component bottom;
	AdsrSliders adsrsliders;
	psy_ui_Slider cutoff;
	psy_ui_Slider res;
	psy_ui_Slider modamount;
	// references
	psy_audio_Player* player;
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
} InstrumentFilterView;

void instrumentfilterview_init(InstrumentFilterView*, psy_ui_Component* parent,
	psy_audio_Instruments*, Workspace*);
void instrumentfilterview_setinstrument(InstrumentFilterView*,
	psy_audio_Instrument*);

#ifdef __cplusplus
}
#endif

#endif /* INSTRUMENTFILTERVIEW_H */

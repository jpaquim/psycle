// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTRUMENTGENVIEW_H)
#define INSTRUMENTGENVIEW_H

// host
#include "envelopeview.h"
#include "instnotemapview.h"
#include "workspace.h"
// ui elements
#include <uibutton.h>
#include <uicheckbox.h>
#include <uicombobox.h>
#include <uiedit.h>
#include <uilabel.h>
#include <uislider.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct InstrumentGeneralView {
	// inherits
	psy_ui_Component component;
	// ui elements
	// nna
	psy_ui_Component nna;	
	psy_ui_Label nnaheader;
	psy_ui_Button nnacut;
	psy_ui_Button nnarelease;
	psy_ui_Button nnafadeout;
	psy_ui_Button nnanone;
	// dct
	psy_ui_Component dct;
	psy_ui_Label dctheader;
	psy_ui_Button dctnone;
	psy_ui_Button dctnote;
	psy_ui_Button dctsample;
	psy_ui_Button dctinstrument;	
	// dca
	psy_ui_Component dca;
	psy_ui_Label dcaheader;
	psy_ui_Button dcacut;
	psy_ui_Button dcarelease;
	psy_ui_Button dcafadeout;
	psy_ui_Button dcanone;
	psy_ui_Component fitrow;
	psy_ui_CheckBox fitrowcheck;
	psy_ui_Edit fitrowedit;
	psy_ui_Label fitrowlabel;
	psy_ui_Component left;
	psy_ui_Slider globalvolume;	
	InstrumentNoteMapView notemapview;
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
} InstrumentGeneralView;

void instrumentgeneralview_init(InstrumentGeneralView*,
	psy_ui_Component* parent, psy_audio_Instruments*,
	Workspace*);
void instrumentgeneralview_setinstrument(InstrumentGeneralView*,
	psy_audio_Instrument*);

#ifdef __cplusplus
}
#endif

#endif /* INSTRUMENTGENVIEW_H */

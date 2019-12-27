// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTNOTEMAPVIEW_H)
#define INSTNOTEMAPVIEW_H

#include "envelopeview.h"
#include "instrumentsbox.h"
#include "samplesbox.h"
#include "tabbar.h"
#include "workspace.h"

#include <uibutton.h>
#include <uigroupbox.h>
#include <uilabel.h>
#include <uiedit.h>
#include <uinotebook.h>
#include <uislider.h>

typedef struct {
	int keysize;
	int lineheight;
} InstrumentNoteMapMetrics;

typedef struct {
	psy_ui_Component component;	
	int dy;
	InstrumentNoteMapMetrics metrics;
} InstrumentKeyboardView;

void instrumentkeyboardview_init(InstrumentKeyboardView*,
	psy_ui_Component* parent);

typedef struct {
	psy_ui_Component component;
	psy_ui_Button add;
	psy_ui_Button remove;
} InstrumentNoteMapButtons;

enum {
	INSTVIEW_DRAG_NONE,
	INSTVIEW_DRAG_LEFT,
	INSTVIEW_DRAG_RIGHT
};

typedef struct {
	psy_ui_Component component;	
	int dy;
	psy_audio_Instrument* instrument;
	InstrumentNoteMapMetrics metrics;
} InstrumentParameterView;

typedef struct {
	psy_ui_Component component;	
	int dy;
	psy_audio_Instrument* instrument;
	uintptr_t selected;
	int dragmode;
	InstrumentParameterView* parameterview;
	InstrumentNoteMapMetrics metrics;
} InstrumentEntryView;

typedef struct {	
	psy_ui_Component component;
	psy_ui_Label label;
	InstrumentNoteMapButtons buttons;
	InstrumentKeyboardView keyboard;
	InstrumentEntryView entryview;
	InstrumentParameterView parameterview;
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
	InstrumentNoteMapMetrics metrics;
} InstrumentNoteMapView;

void instrumentnotemapview_init(InstrumentNoteMapView*,
	psy_ui_Component* parent);
void instrumentnotemapview_setinstrument(InstrumentNoteMapView*, psy_audio_Instrument*);
void instrumentnotemapview_update(InstrumentNoteMapView*);

#endif

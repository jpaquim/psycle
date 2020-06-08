// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTNOTEMAPVIEW_H)
#define INSTNOTEMAPVIEW_H

#include "envelopeview.h"
#include "instrumentsbox.h"
#include "samplesbox.h"
#include "tabbar.h"
#include "workspace.h"

#include <uibutton.h>
#include <uiedit.h>
#include <uilabel.h>
#include <uinotebook.h>
#include <uislider.h>

// aim: InstrumentNoteMapView plays the rule of the Sampulse Note mapping
//      editor. Sampulse instruments can use more than one sample at a time,
//      and to do so, they use the note mapping, which means that each of the
//      possible notes, will trigger a specific note and sample number. PS1 had
//      no possiblity to define different samples for different note keys.
//      Sampulse introduced a ft2 mapping, where each note can have its own
//      sample, making it easier to import xm files. The new combined Sampler
//      (Sampulse PS1) uses the zone concept of soundfonts, that use a list of
//      zones, each with a keyrange define. This allows additionaly sample
//      layering for one note.

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

void instrumentnotemapbuttons_init(InstrumentNoteMapButtons*,
	psy_ui_Component* parent);

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

void instrumentparameterview_setinstrument(InstrumentParameterView*,
	psy_audio_Instrument*);
void instrumentparameterview_init(InstrumentParameterView*,
	psy_ui_Component* parent);


typedef struct {
	psy_ui_Component component;	
	int dy;
	psy_audio_Instrument* instrument;
	uintptr_t selected;
	int dragmode;
	int currkey;
	InstrumentParameterView* parameterview;
	InstrumentNoteMapMetrics metrics;
} InstrumentEntryView;

void instrumententryview_init(InstrumentEntryView*, psy_ui_Component* parent,
	InstrumentParameterView*);
void instrumententryview_setinstrument(InstrumentEntryView*,
	psy_audio_Instrument*);
void instrumententryview_adjustscroll(InstrumentEntryView*);

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
	SamplesBox samplesbox;
} InstrumentNoteMapView;

void instrumentnotemapview_init(InstrumentNoteMapView*,
	psy_ui_Component* parent);
void instrumentnotemapview_setinstrument(InstrumentNoteMapView*, psy_audio_Instrument*);
void instrumentnotemapview_update(InstrumentNoteMapView*);

#endif

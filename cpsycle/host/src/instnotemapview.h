// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTNOTEMAPVIEW_H)
#define INSTNOTEMAPVIEW_H

#include "envelopeview.h"
#include "instrumentsbox.h"
#include "intedit.h"
#include "samplesbox.h"
#include "tabbar.h"
#include "workspace.h"

#include <uibutton.h>
#include <uiedit.h>
#include <uilabel.h>
#include <uinotebook.h>
#include <uislider.h>
#include <uisplitbar.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

// InstrumentNoteMapView
//
// plays the rule of the Sampulse Note mapping editor. Sampulse instruments
// can use more than one sample at a time, and to do so, they use the note
// mapping, which means that each of the possible notes, will trigger a
// specific note and sample number. PS1 had no possiblity to define different
// samples for different note keys. Sampulse introduced a ft2 mapping, where
// each note can have its own sample, making it easier to import xm files.
// The new combined Sampler (Sampulse PS1) uses the zone concept of
// soundfonts, that use a list of zones, each with a keyrange define. This
// allows additionaly sample layering for one note.

typedef struct InstrumentNoteMapMetrics {
	double keysize;
	int lineheight;
} InstrumentNoteMapMetrics;

typedef struct InstrumentKeyboardView {
	// inherits
	psy_ui_Component component;
	// internal
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

// InstrumentEntryView
typedef struct InstrumentEntryView {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_selected;
	// internal	
	uintptr_t selected;
	int dragmode;
	uint8_t currkey;	
	InstrumentNoteMapMetrics metrics;
	// references
	psy_audio_Instrument* instrument;	
} InstrumentEntryView;

void instrumententryview_init(InstrumentEntryView*, psy_ui_Component* parent);
void instrumententryview_setinstrument(InstrumentEntryView*,
	psy_audio_Instrument*);
psy_audio_InstrumentEntry* instrumententryview_selected(InstrumentEntryView*);
void instrumententryview_setsample(InstrumentEntryView*, psy_audio_SampleIndex);
void instrumententryview_select(InstrumentEntryView*, uintptr_t row);

INLINE psy_ui_Component* instrumententryview_base(InstrumentEntryView* self)
{
	return &self->component;
}

// Table

// TableColumn
typedef struct InstrumentEntryTableColumn {
	psy_ui_Value width;
	char_dyn_t* label;
} InstrumentEntryTableColumn;

void instrumententrytablecolumn_init(InstrumentEntryTableColumn*, const char* label);
void instrumententrytablecolumn_dispose(InstrumentEntryTableColumn*);

InstrumentEntryTableColumn* instrumententrytablecolumn_alloc(void);
InstrumentEntryTableColumn* instrumententrytablecolumn_allocinit(const char* label);

// TableState
typedef struct InstrumentEntryTableState {
	psy_Table columns;
} InstrumentEntryTableState;

void instrumententrytablestate_init(InstrumentEntryTableState*);
void instrumententrytablestate_dispose(InstrumentEntryTableState*);

// TableHeader
typedef struct InstrumentEntryTableViewHeader {
	// inherits
	psy_ui_Component component;
	// references
	InstrumentEntryTableState* state;
} InstrumentEntryTableViewHeader;

void instrumententrytableviewheader_init(InstrumentEntryTableViewHeader* self,
	psy_ui_Component* parent, InstrumentEntryTableState* state,
	Workspace* workspace);

// TableView
typedef struct InstrumentEntryTableView {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_selected;
	// internal	
	IntEdit edit;
	uintptr_t selected;
	int dragmode;
	uint8_t currkey;
	psy_ui_Value lineheight;
	InstrumentNoteMapMetrics metrics;
	// references	
	psy_audio_Instrument* instrument;
	InstrumentEntryTableState* state;
} InstrumentEntryTableView;

void instrumententrytableview_init(InstrumentEntryTableView*,
	psy_ui_Component* parent, InstrumentEntryTableState*);
void instrumententrytableview_setinstrument(InstrumentEntryTableView*,
	psy_audio_Instrument*);
void instrumententrytableview_select(InstrumentEntryTableView*, uintptr_t row);

psy_audio_InstrumentEntry* instrumententrytableview_selected(InstrumentEntryTableView*);

INLINE psy_ui_Component* instrumententrytableview_base(
	InstrumentEntryTableView* self)
{
	return &self->component;
}

typedef struct InstrumentNoteMapView {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Label label;
	InstrumentNoteMapButtons buttons;
	psy_ui_Component entries;
	InstrumentKeyboardView keyboard;
	InstrumentEntryView entryview;
	psy_ui_SplitBar splitter;
	psy_ui_Component table;
	InstrumentEntryTableState tablestate;
	InstrumentEntryTableViewHeader tableheader;
	InstrumentEntryTableView tableview;	
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
	InstrumentNoteMapMetrics metrics;
	SamplesBox samplesbox;
	psy_ui_Scroller scroller;
	psy_ui_Scroller scroller_table;
} InstrumentNoteMapView;

void instrumentnotemapview_init(InstrumentNoteMapView*,
	psy_ui_Component* parent, Workspace*);
void instrumentnotemapview_setinstrument(InstrumentNoteMapView*,
	psy_audio_Instrument*);
void instrumentnotemapview_update(InstrumentNoteMapView*);

INLINE psy_ui_Component* instrumentnotemapview_base(
	InstrumentNoteMapView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* INSTNOTEMAPVIEW_H */

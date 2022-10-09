// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTNOTEMAPVIEW_H)
#define INSTNOTEMAPVIEW_H

#include "envelopeview.h"
#include "instrumentsbox.h"
#include "pianokeyboard.h"
#include "intedit.h"
#include "samplesbox.h"
#include <uitabbar.h>
#include "workspace.h"

#include <uibutton.h>
#include <uitextarea.h>
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
	INSTVIEW_DRAG_RIGHT,
	INSTVIEW_DRAG_LEFTRIGHT
};

struct psy_audio_InstrumentEntry;


/* InstrumentEntryState */

typedef struct InstrumentEntryState {
	/* signals */
	psy_Signal signal_select;
	psy_Signal signal_entrychanged;	
	/* references */
	struct psy_audio_InstrumentEntry* selectedentry;
	Workspace* workspace;
} InstrumentEntryState;

void instrumententrystate_init(InstrumentEntryState*, Workspace*);
void instrumententrystate_dispose(InstrumentEntryState*);

void instrumententrystate_selectentry(
	InstrumentEntryState*, psy_audio_InstrumentEntry*);
void instrumententrystate_updateentry(InstrumentEntryState*,
	psy_audio_InstrumentEntry*);


/* InstrumentRangeRow */

typedef struct InstrumentRangeRow {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	int dragmode;
	uint8_t currkey;
	double keysize;
	/* references */
	psy_audio_InstrumentEntry* entry;
	InstrumentEntryState* state;	
} InstrumentRangeRow;

void instrumentrangerow_init(InstrumentRangeRow*,
	psy_ui_Component* parent, psy_audio_InstrumentEntry*, InstrumentEntryState*);

InstrumentRangeRow* instrumentrangerow_alloc(void);
InstrumentRangeRow* instrumentrangerow_allocinit(
	psy_ui_Component* parent, psy_audio_InstrumentEntry*,
	InstrumentEntryState*);

/* InstrumentEntryView */

typedef struct InstrumentEntryView {
	/* inherits */
	psy_ui_Component component;		
	/* internal	*/
	int dragmode;
	uint8_t currkey;	
	/* references */
	psy_audio_Instrument* instrument;	
	InstrumentEntryState* state;
} InstrumentEntryView;

void instrumententryview_init(InstrumentEntryView*, psy_ui_Component* parent,
	InstrumentEntryState*);
void instrumententryview_setinstrument(InstrumentEntryView*,
	psy_audio_Instrument*);
void instrumententryview_build(InstrumentEntryView*);
psy_audio_InstrumentEntry* instrumententryview_selected(InstrumentEntryView*);
void instrumententryview_setsample(InstrumentEntryView*, psy_audio_SampleIndex);
void instrumententryview_select(InstrumentEntryView*, uintptr_t row);

INLINE psy_ui_Component* instrumententryview_base(InstrumentEntryView* self)
{
	return &self->component;
}

/* TableHeader */

typedef struct InstrumentEntryTableViewHeader {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label index;
	psy_ui_Label key_lo;
	psy_ui_Label key_hi;
	psy_ui_Label key_fixed;
	/* references */
	InstrumentEntryState* state;
} InstrumentEntryTableViewHeader;

void instrumententrytableviewheader_init(InstrumentEntryTableViewHeader*,
	psy_ui_Component* parent, InstrumentEntryState*);

/* InstrumentEntryEdit */

typedef struct InstrumentEntryEdit {
	/* inherits */
	psy_ui_TextArea input;
	/* internal */	
	uintptr_t column;
	/* references */
	psy_audio_InstrumentEntry* entry;
	InstrumentEntryState* state;
} InstrumentEntryEdit;

void instrumententryedit_init(InstrumentEntryEdit*, psy_ui_Component* parent,
	psy_audio_InstrumentEntry*, uintptr_t column, InstrumentEntryState* state);

void instrumententryedit_read(InstrumentEntryEdit*);

/* InstrumentEntryRow */

typedef struct InstrumentEntryTableRow {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	psy_ui_Label index;
	InstrumentEntryEdit key_lo;
	InstrumentEntryEdit key_hi;
	InstrumentEntryEdit key_fixed;
	/* references */
	psy_audio_InstrumentEntry* entry;
	InstrumentEntryState* state;
} InstrumentEntryTableRow;

void instrumententrytablerow_init(InstrumentEntryTableRow*,
	psy_ui_Component* parent, psy_audio_InstrumentEntry*, InstrumentEntryState*);

InstrumentEntryTableRow* instrumententrytablerow_alloc(void);
InstrumentEntryTableRow* instrumententrytablerow_allocinit(
	psy_ui_Component* parent, psy_audio_InstrumentEntry*,
	InstrumentEntryState*);


/* TableView */

typedef struct InstrumentEntryTableView {
	/* inherits */
	psy_ui_Component component;		
	/* internal */	
	/* references */
	psy_audio_Instrument* instrument;
	InstrumentEntryState* state;
} InstrumentEntryTableView;

void instrumententrytableview_init(InstrumentEntryTableView*,
	psy_ui_Component* parent, InstrumentEntryState*);
void instrumententrytableview_setinstrument(InstrumentEntryTableView*,
	psy_audio_Instrument*);
void instrumententrytableview_build(InstrumentEntryTableView*);

INLINE psy_ui_Component* instrumententrytableview_base(
	InstrumentEntryTableView* self)
{
	return &self->component;
}


/* InstrumentNoteMapView */

typedef struct InstrumentNoteMapView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label label;
	InstrumentNoteMapButtons buttons;
	psy_ui_Component entries;
	KeyboardState keyboard_state;
	PianoKeyboard keyboard;		
	InstrumentEntryView entryview;
	psy_ui_Splitter splitter;
	psy_ui_Component table;
	InstrumentEntryState state;
	InstrumentEntryTableViewHeader tableheader;
	InstrumentEntryTableView tableview;	
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;	
	SamplesBox samplesbox;
	psy_ui_Scroller scroller;
	psy_ui_Scroller scroller_table;
} InstrumentNoteMapView;

void instrumentnotemapview_init(InstrumentNoteMapView*,
	psy_ui_Component* parent, Workspace*);
void instrumentnotemapview_set_instrument(InstrumentNoteMapView*,
	psy_audio_Instrument*);

INLINE psy_ui_Component* instrumentnotemapview_base(InstrumentNoteMapView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* INSTNOTEMAPVIEW_H */

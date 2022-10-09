/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "instnotemapview.h"
/* host */
#include "styles.h"
/* dsp */
#include <notestab.h>
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"

#define COLWIDTH 12.0


static int numwhitekey(int key)
{
	int octave = key / 12;
	int offset = key % 12;
	int c = 0;
	int i;

	for (i = 1; i <= offset; ++i) {
		if (!psy_dsp_isblack(i)) ++c;
	}
	return octave * 7 + c;
}

static uint8_t whitekeytokey(uint8_t whitekey)
{
	uint8_t octave = whitekey / 7;
	uint8_t offset = whitekey % 7;
	/*
	** 0 1 2 3 4 5 6 7 8 9 10 11
	** c   d   e f   g   a    h
	*/
	static int numkey[] = { 0, 2, 4, 5, 7, 9, 11 };
	return octave * 12 + numkey[offset];
}

static uint8_t screentokey(double x, double keysize)
{
	uint8_t rv;
	uint8_t numwhitekey;	

	numwhitekey = (uint8_t)(psy_max(0.0, x) / keysize);
	rv = whitekeytokey(numwhitekey);
	/*
	** 0 1 2 3 4 5 6 7 8 9 10 11
	** c   d   e f   g   a    h 
	*/
	if ((rv % 12) != 4 && (rv % 12) != 11) {
		double delta;

		delta = x - (numwhitekey * keysize);
		if (delta > 0.5 * keysize) {
			++rv;
		}
	}
	if (rv >= psy_audio_NOTECOMMANDS_RELEASE) {
		rv = psy_audio_NOTECOMMANDS_RELEASE - 1;
	}
	return rv;	
}


/* InstrumentRangeRow */

/* prototypes */
static void instrumentrangerow_on_destroyed(InstrumentRangeRow*);
static void instrumentrangerow_on_draw(InstrumentRangeRow*, psy_ui_Graphics*);
static void instrumentrangerow_on_mouse_down(InstrumentRangeRow*,
	psy_ui_MouseEvent*);
static void instrumentrangerow_on_mouse_move(InstrumentRangeRow*,
	psy_ui_MouseEvent*);
static void instrumentrangerow_on_mouse_up(InstrumentRangeRow*,
	psy_ui_MouseEvent*);
static void instrumentrangerow_on_align(InstrumentRangeRow*);
static void instrumentrangerow_outputstatus(InstrumentRangeRow*, uint8_t key);
	
/* vtable */
static psy_ui_ComponentVtable instrumentrangerow_vtable;
static bool instrumentrangerow_vtable_initialized = FALSE;

static void instrumentrangerow_vtable_init(InstrumentRangeRow* self)
{
	if (!instrumentrangerow_vtable_initialized) {
		instrumentrangerow_vtable = *(self->component.vtable);
		instrumentrangerow_vtable.on_destroyed =
			(psy_ui_fp_component)
			instrumentrangerow_on_destroyed;
		instrumentrangerow_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			instrumentrangerow_on_draw;
		instrumentrangerow_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			instrumentrangerow_on_mouse_down;
		instrumentrangerow_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			instrumentrangerow_on_mouse_move;
		instrumentrangerow_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			instrumentrangerow_on_mouse_up;
		instrumentrangerow_vtable.onalign =
			(psy_ui_fp_component)
			instrumentrangerow_on_align;
		instrumentrangerow_vtable_initialized = TRUE;
	}
	self->component.vtable = &instrumentrangerow_vtable;
}

/* implementation */
void instrumentrangerow_init(InstrumentRangeRow* self,
	psy_ui_Component* parent, psy_audio_InstrumentEntry* entry,
	InstrumentEntryState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	instrumentrangerow_vtable_init(self);		
	self->entry = entry;
	self->state = state;
	self->dragmode = 0;
	self->keysize = 8.0;
	psy_ui_component_set_preferred_height(&self->component,
		psy_ui_value_make_eh(1.0));
}

InstrumentRangeRow* instrumentrangerow_alloc(void)
{
	return (InstrumentRangeRow*)malloc(sizeof(InstrumentRangeRow));
}

InstrumentRangeRow* instrumentrangerow_allocinit(
	psy_ui_Component* parent, psy_audio_InstrumentEntry* entry,
	InstrumentEntryState* state)
{
	InstrumentRangeRow* rv;

	rv = instrumentrangerow_alloc();
	if (rv) {
		instrumentrangerow_init(rv, parent, entry, state);
		psy_ui_component_deallocate_after_destroyed(&rv->component);		
	}
	return rv;
}

void instrumentrangerow_on_destroyed(InstrumentRangeRow* self)
{		
	// psy_signal_disconnect(&self->state->signal_select, self,
	//	instrumentrangerow_onentryselected);
	// psy_signal_disconnect(&self->state->signal_entrychanged, self,
	//	instrumentrangerow_onentryupdate);
}

void instrumentrangerow_on_draw(InstrumentRangeRow* self, psy_ui_Graphics* g)
{
	psy_audio_InstrumentEntry* entry;
	double keylo_startx;
	double keylo_endx;
	double keyhi_startx;
	double keyhi_endx;
	psy_ui_RealRectangle r;
	uint8_t keymin = 0;
	uint8_t keymax = psy_audio_NOTECOMMANDS_RELEASE;
	uint8_t key;
	uint8_t numwhitekeys;
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;
	double keysize;
			
	if (!self->entry) {
		return;
	}
	entry = self->entry;
	numwhitekeys = 0;
	for (key = keymin; key < keymax;  ++key) {
		if (!psy_dsp_isblack(key)) {
			++numwhitekeys;
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size_px(&self->component);
	keysize = self->keysize;
	keylo_startx = (int)(
		(float)numwhitekey((uint8_t)entry->keyrange.low) /
		numwhitekeys * size.width) +
		(int)(psy_dsp_isblack((uint8_t)entry->keyrange.low)
			? keysize / 2 : 0);
	keylo_endx = (int)(
		(float)numwhitekey((uint8_t)entry->keyrange.low + 1) /
		numwhitekeys * size.width) +
		(int)(psy_dsp_isblack((uint8_t)entry->keyrange.low + 1)
			? keysize / 2 : 0);								
	keyhi_startx = (int)(
		(float)numwhitekey((uint8_t)entry->keyrange.high) /
		numwhitekeys * size.width) +
		(int)(psy_dsp_isblack((uint8_t)entry->keyrange.high)
			? keysize / 2 : 0);
	keyhi_endx = (int)(
		(float)numwhitekey((uint8_t)entry->keyrange.high + 1) /
		numwhitekeys * size.width) +
		(int)(psy_dsp_isblack((uint8_t)entry->keyrange.high + 1)
			? keysize / 2 : 0);
	r = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(keylo_startx, 2),
			psy_ui_realsize_make(keyhi_endx - keylo_startx, size.height - 2));
	psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00333333));
}

void instrumentrangerow_on_mouse_down(InstrumentRangeRow* self,
	psy_ui_MouseEvent* ev)
{
	psy_audio_InstrumentEntry* entry;
	double keysize;
	
	if (!self->entry) {
		return;
	}
	entry = self->entry;
	self->dragmode = 1;
	keysize = self->keysize;
	self->currkey = screentokey(psy_ui_mouseevent_pt(ev).x,
		keysize);
	instrumentrangerow_outputstatus(self, (uint8_t)self->currkey);
	if (abs((int)(entry->keyrange.low  -
			screentokey(psy_ui_mouseevent_pt(ev).x,
			keysize))) <
		abs((int)(entry->keyrange.high  -
			screentokey(psy_ui_mouseevent_pt(ev).x,
			keysize)))) {
		self->dragmode = INSTVIEW_DRAG_LEFT;
		psy_ui_component_setcursor(&self->component,
			psy_ui_CURSORSTYLE_COL_RESIZE);
	} else {
		if (entry->keyrange.low == entry->keyrange.high) {
			self->dragmode = INSTVIEW_DRAG_LEFTRIGHT;
		} else {
			self->dragmode = INSTVIEW_DRAG_RIGHT;
		}
		psy_ui_component_setcursor(&self->component,
			psy_ui_CURSORSTYLE_COL_RESIZE);
	}
	psy_ui_component_capture(&self->component);
}

void instrumentrangerow_on_mouse_move(InstrumentRangeRow* self,
	psy_ui_MouseEvent* ev)
{
	bool showresizecursor;
	psy_audio_InstrumentEntry* entry;
	double keysize;
	
	if (!self->entry) {
		return;
	}
	entry = self->entry;
	showresizecursor = FALSE;
	keysize = self->keysize;
	self->currkey = screentokey(psy_ui_mouseevent_pt(ev).x, keysize);	
	if (self->dragmode != INSTVIEW_DRAG_NONE) {		
		uintptr_t screenkey;
				
		screenkey = screentokey(psy_ui_mouseevent_pt(ev).x, keysize);		
		if (self->dragmode == INSTVIEW_DRAG_LEFTRIGHT) {
			if (screenkey < entry->keyrange.low) {
				self->dragmode = INSTVIEW_DRAG_LEFT;
			} else if (screenkey > entry->keyrange.high) {
				self->dragmode = INSTVIEW_DRAG_RIGHT;
			} else {
				psy_ui_component_setcursor(&self->component,
					psy_ui_CURSORSTYLE_COL_RESIZE);
				return;
			}
		}
		if (self->dragmode == INSTVIEW_DRAG_LEFT) {
			entry->keyrange.low = screenkey;
			entry->keyrange.low = psy_max(entry->keyrange.min,
				entry->keyrange.low);
			if (entry->keyrange.low > entry->keyrange.high) {
				entry->keyrange.low = entry->keyrange.high;
			}
			instrumentrangerow_outputstatus(self,
			 	(uint8_t)entry->keyrange.low);
		} else {
			entry->keyrange.high = screenkey;
			entry->keyrange.high = psy_min(entry->keyrange.max,
				entry->keyrange.high);
			if (entry->keyrange.high < entry->keyrange.low) {
				entry->keyrange.high = entry->keyrange.low;
			}
			instrumentrangerow_outputstatus(self,
				(uint8_t)entry->keyrange.high);
		}
		instrumententrystate_updateentry(self->state, entry);		
		showresizecursor = TRUE;
	}
	if (showresizecursor != FALSE) {
		psy_ui_component_setcursor(&self->component,
			psy_ui_CURSORSTYLE_COL_RESIZE);
	}
}
	
void instrumentrangerow_on_mouse_up(InstrumentRangeRow* self,
	psy_ui_MouseEvent* ev)
{
	if (!self->entry) {
		return;
	}
	self->dragmode = 0;		
	psy_ui_component_release_capture(&self->component);	
}

void instrumentrangerow_on_align(InstrumentRangeRow* self)
{
	int keymin = 0;
	int keymax = psy_audio_NOTECOMMANDS_RELEASE;
	int key;
	int numwhitekeys;
	const psy_ui_TextMetric* tm;
	psy_ui_IntSize size;

	numwhitekeys = 0;
	for (key = keymin; key < keymax; ++key) {
		if (!psy_dsp_isblack(key)) {
			++numwhitekeys;
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_scroll_size(&self->component), tm, NULL);
	self->keysize = size.width / (double)numwhitekeys;	
}

void instrumentrangerow_outputstatus(InstrumentRangeRow* self, uint8_t key)
{	
	char text[64];
	const char* notestr;
	const char* keydesc;

	keydesc = psy_ui_translate("instrumentview.key");
	if (!keydesc) {
		keydesc = "Key";
	}
	notestr = psy_dsp_notetostr(key,
		patternviewconfig_notetabmode(&self->state->workspace->config.visual.patview));
	if (notestr) {				
		psy_snprintf(text, 64, "%s %s", keydesc, notestr);		
	} else {
		psy_snprintf(text, 64, "%s %d", keydesc, (int)key);
	}
	workspace_output_status(self->state->workspace, text);
}


/* InstrumentEntryView */

/* prototypes */
static void instrumententryview_on_destroyed(InstrumentEntryView*);
static void instrumententryview_onentryselected(
	InstrumentEntryView*, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry*);
static void instrumententryview_onentryupdate(
	InstrumentEntryView*, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry*);
static void instrumententryview_outputstatus(InstrumentEntryView*,
	uint8_t key);
	
/* vtable */
static psy_ui_ComponentVtable instrumententryview_vtable;
static bool instrumententryview_vtable_initialized = FALSE;

static void instrumententryview_vtable_init(InstrumentEntryView* self)
{
	if (!instrumententryview_vtable_initialized) {
		instrumententryview_vtable = *(self->component.vtable);
		instrumententryview_vtable.on_destroyed =
			(psy_ui_fp_component)
			instrumententryview_on_destroyed;		
		instrumententryview_vtable_initialized = TRUE;
	}
	self->component.vtable = &instrumententryview_vtable;
}

/* implementation */
void instrumententryview_init(InstrumentEntryView* self,
	psy_ui_Component* parent, InstrumentEntryState* state)
{	
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	instrumententryview_vtable_init(self);	
	psy_ui_component_set_default_align(&self->component,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());	
	psy_ui_component_set_wheel_scroll(&self->component, 1);	
	psy_ui_component_set_scroll_step(&self->component,
		psy_ui_size_make_em(0.0, 1.0));
	self->instrument = NULL;		
	self->dragmode = 0;	
	self->state = state;		
	psy_signal_connect(&self->state->signal_select, self,
		instrumententryview_onentryselected);
	psy_signal_connect(&self->state->signal_entrychanged, self,
		instrumententryview_onentryupdate);
	psy_ui_component_set_overflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
}

void instrumententryview_on_destroyed(InstrumentEntryView* self)
{	
	assert(self);
	
	psy_signal_disconnect(&self->state->signal_select, self,
		instrumententryview_onentryselected);
	psy_signal_disconnect(&self->state->signal_select, self,
		instrumententryview_onentryupdate);
}

void instrumententryview_set_instrument(InstrumentEntryView* self,
	psy_audio_Instrument* instrument)
{
	assert(self);
	
	self->instrument = instrument;
	instrumententryview_build(self);
	psy_ui_component_set_scroll_top(&self->component, psy_ui_value_zero());	
	psy_ui_component_updateoverflow(&self->component);	
	psy_ui_component_invalidate(&self->component);	
}

void instrumententryview_build(InstrumentEntryView* self)
{		
	psy_ui_component_clear(&self->component);	
	if (self->instrument) {
		psy_List* p;		

		for (p = self->instrument->entries; p != NULL; p = p->next) {
			psy_audio_InstrumentEntry* entry;
			InstrumentRangeRow* row;

			entry = (psy_audio_InstrumentEntry*)psy_list_entry(p);
			row = (InstrumentRangeRow*)instrumentrangerow_allocinit(
				&self->component, entry, self->state);			
		}
	}
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->component);
}

psy_audio_InstrumentEntry* instrumententryview_selected(
	InstrumentEntryView* self)
{
	return self->state->selectedentry;	
}

void instrumententryview_select(InstrumentEntryView* self, uintptr_t row)
{
	psy_audio_InstrumentEntry* entry;

	entry = psy_audio_instrument_entryat(self->instrument, row);
	instrumententrystate_selectentry(self->state, entry);	
	psy_ui_component_invalidate(&self->component);
}

void instrumententryview_setsample(InstrumentEntryView* self,
	psy_audio_SampleIndex index)
{
	if (psy_audio_sampleindex_valid(&index)) {
		psy_audio_InstrumentEntry* entry;

		entry = instrumententryview_selected(self);
		if (entry) {
			entry->sampleindex = index;
		}
	}
}

void instrumententryview_onentryselected(InstrumentEntryView* self,
	InstrumentEntryState* sender, psy_audio_InstrumentEntry* entry)
{
	psy_ui_component_invalidate(&self->component);
}

void instrumententryview_onentryupdate(
	InstrumentEntryView* self, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry* entry)
{
	psy_ui_component_invalidate(&self->component);
}

/* InstrumentEntryState */
/* implementation */
void instrumententrystate_init(InstrumentEntryState* self, Workspace* workspace)
{	

	self->workspace = workspace;
	psy_signal_init(&self->signal_select);
	psy_signal_init(&self->signal_entrychanged);		
}

void instrumententrystate_dispose(InstrumentEntryState* self)
{
	psy_signal_dispose(&self->signal_select);
	psy_signal_dispose(&self->signal_entrychanged);	
}

void instrumententrystate_selectentry(
	InstrumentEntryState* self, psy_audio_InstrumentEntry* entry)
{
	self->selectedentry = entry;
	psy_signal_emit(&self->signal_select, self, 1, entry);
}

void instrumententrystate_updateentry(
	InstrumentEntryState* self, psy_audio_InstrumentEntry* entry)
{	
	psy_signal_emit(&self->signal_entrychanged, self, 1, entry);
}

/* InstrumentEntryTableViewHeader */

/* implementation */
void instrumententrytableviewheader_init(InstrumentEntryTableViewHeader* self,
	psy_ui_Component* parent, InstrumentEntryState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->state = state;
	psy_ui_component_set_default_align(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_margin_zero());
		/* index */
	psy_ui_label_init_text(&self->index, &self->component,
		"instrumentview.smplidx");
	psy_ui_label_set_char_number(&self->index, COLWIDTH);
	/* lo */
	psy_ui_label_init_text(&self->key_lo, &self->component,
		"instrumentview.keylo");
	psy_ui_label_set_char_number(&self->key_lo, COLWIDTH);
	/* hi */
	psy_ui_label_init_text(&self->key_hi, &self->component,
		"instrumentview.keyhi");
	psy_ui_label_set_char_number(&self->key_hi, COLWIDTH);
	/* fixed */
	psy_ui_label_init_text(&self->key_fixed, &self->component,
		"instrumentview.keyfixed");
	psy_ui_label_set_char_number(&self->key_fixed, COLWIDTH);	
}

/* InstrumentEntryEdit */

/* prototypes */
static void instrumententryedit_on_edit_key_down(InstrumentEntryEdit*,
	psy_ui_Component* sender, psy_ui_KeyboardEvent*);
static void instrumententryedit_on_accept(InstrumentEntryEdit*,
	psy_ui_Component* sender);
static intptr_t instrumententryedit_value(InstrumentEntryEdit*);

/* implementation */
void instrumententryedit_init(InstrumentEntryEdit* self,
	psy_ui_Component* parent, psy_audio_InstrumentEntry* entry,
	uintptr_t column, InstrumentEntryState* state)
{
	psy_ui_textarea_init_single_line(&self->input, parent);
	self->entry = entry;
	self->column = column;
	self->state = state;
	psy_ui_textarea_set_char_number(&self->input, COLWIDTH);	
	psy_ui_textarea_enable_input_field(&self->input);
	instrumententryedit_read(self);
	psy_signal_connect(&self->input.component.signal_keydown, self,
		instrumententryedit_on_edit_key_down);	
	psy_signal_connect(&self->input.signal_accept, self,
		instrumententryedit_on_accept);
}

void instrumententryedit_read(InstrumentEntryEdit* self)
{
	char text[64];	
	
	if (!self->entry) {
		return;
	}
	switch (self->column) {	
	case 1:
		psy_snprintf(text, 64, "%.3d", self->entry->keyrange.low);
		break;
	case 2:
		psy_snprintf(text, 64, "%.3d", self->entry->keyrange.high);
		break;
	case 3:		
		if (self->entry->fixedkey == psy_audio_NOTECOMMANDS_EMPTY) {
			psy_snprintf(text, 64, "%s", "-");
		} else {
			psy_snprintf(text, 64, "%d", (int)self->entry->fixedkey);
		}
		break;
	default:
		text[0] = '\0';
		break;
	}
	psy_ui_textarea_set_text(&self->input, text);
}

void instrumententryedit_on_edit_key_down(InstrumentEntryEdit* self,
	psy_ui_Component* sender, psy_ui_KeyboardEvent* ev)
{
	if (isalpha(psy_ui_keyboardevent_keycode(ev))) {		
		psy_ui_keyboardevent_prevent_default(ev);
		psy_ui_keyboardevent_stop_propagation(ev);
		return;
	}	
}

void instrumententryedit_on_accept(InstrumentEntryEdit* self,
	psy_ui_Component* sender)
{
	switch (self->column) {	
	case 1:
		self->entry->keyrange.low = (uint8_t)instrumententryedit_value(self);		
		break;
	case 2:
		self->entry->keyrange.high = (uint8_t)instrumententryedit_value(self);
		break;
	case 3:		
		if (strcmp(psy_ui_textarea_text(&self->input), "-") == 0) {
			self->entry->fixedkey = psy_audio_NOTECOMMANDS_EMPTY;
		} else {
			self->entry->fixedkey = (uint8_t)instrumententryedit_value(self);
		}		
		break;
	default:		
		break;
	}
	instrumententrystate_updateentry(self->state, self->entry);
}

intptr_t instrumententryedit_value(InstrumentEntryEdit* self)
{
	return (intptr_t)atoi(psy_ui_textarea_text(&self->input));
}

/* InstrumentEntryTableRow */

/* prototypes */
static void instrumententrytablerow_on_destroyed(InstrumentEntryTableRow*);
static void instrumententrytablerow_on_mouse_down(InstrumentEntryTableRow*,
	psy_ui_MouseEvent*);
static void instrumententrytablerow_onentryselected(
	InstrumentEntryTableRow*, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry* entry);
static void instrumententrytablerow_onentryupdate(
	InstrumentEntryTableRow*, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry* entry);

/* vtable */
static psy_ui_ComponentVtable instrumententrytablerow_vtable;
static bool instrumententrytablerow_vtable_initialized = FALSE;

static void instrumententryrow_vtableinit_init(InstrumentEntryTableRow* self)
{
	if (!instrumententrytablerow_vtable_initialized) {
		instrumententrytablerow_vtable = *(self->component.vtable);
		instrumententrytablerow_vtable.on_destroyed =
			(psy_ui_fp_component)
			instrumententrytablerow_on_destroyed;		
		instrumententrytablerow_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			instrumententrytablerow_on_mouse_down;		
		instrumententrytablerow_vtable_initialized = TRUE;
	}
	self->component.vtable = &instrumententrytablerow_vtable;
}

/* implementation */
void instrumententrytablerow_init(InstrumentEntryTableRow* self,
	psy_ui_Component* parent, psy_audio_InstrumentEntry* entry,
	InstrumentEntryState* state)
{
	char text[64];
	
	psy_ui_component_init(&self->component, parent, NULL);
	instrumententryrow_vtableinit_init(self);		
	self->entry = entry;
	self->state = state;	
	psy_ui_component_set_default_align(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_margin_zero());
	/* index */
	psy_ui_label_init(&self->index, &self->component);
	psy_ui_label_set_char_number(&self->index, COLWIDTH);
	psy_ui_label_prevent_translation(&self->index);
	psy_snprintf(text, 64, "%0X:%0X", self->entry->sampleindex.slot,
		self->entry->sampleindex.subslot);
	psy_ui_label_set_text(&self->index, text);	
	/* lo */	
	instrumententryedit_init(&self->key_lo, &self->component, entry, 1, state);		
	instrumententryedit_init(&self->key_hi, &self->component, entry, 2, state);	
	instrumententryedit_init(&self->key_fixed, &self->component, entry, 3,
		state);		
	// psy_signal_connect(&self->state->signal_select, self,
	// 	instrumententryrow_onentryselected);
	psy_signal_connect(&self->state->signal_entrychanged, self,
		instrumententrytablerow_onentryupdate);
}

InstrumentEntryTableRow* instrumententrytablerow_alloc(void)
{
	return (InstrumentEntryTableRow*)malloc(sizeof(InstrumentEntryTableRow));
}

InstrumentEntryTableRow* instrumententrytablerow_allocinit(
	psy_ui_Component* parent, psy_audio_InstrumentEntry* entry,
	InstrumentEntryState* state)
{
	InstrumentEntryTableRow* rv;

	rv = instrumententrytablerow_alloc();
	if (rv) {
		instrumententrytablerow_init(rv, parent, entry, state);
		psy_ui_component_deallocate_after_destroyed(&rv->component);		
	}
	return rv;
}

void instrumententrytablerow_on_destroyed(InstrumentEntryTableRow* self)
{		
	psy_signal_disconnect(&self->state->signal_select, self,
		instrumententrytablerow_onentryselected);
	psy_signal_disconnect(&self->state->signal_entrychanged, self,
		instrumententrytablerow_onentryupdate);
}

void instrumententrytablerow_on_mouse_down(InstrumentEntryTableRow* self,
	psy_ui_MouseEvent* ev)
{		
	instrumententrystate_selectentry(self->state, self->entry);	
}

void instrumententrytablerow_onentryselected(
	InstrumentEntryTableRow* self, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry* entry)
{
		
}

void instrumententrytablerow_onentryupdate(InstrumentEntryTableRow* self,
	InstrumentEntryState* sender, psy_audio_InstrumentEntry* entry)
{
	if (entry == self->entry) {
		instrumententryedit_read(&self->key_lo);
		instrumententryedit_read(&self->key_hi);
		instrumententryedit_read(&self->key_fixed);	
	}
}

/* InstrumentEntryTableView */

/* implementation */
void instrumententrytableview_init(InstrumentEntryTableView* self,
	psy_ui_Component* parent, InstrumentEntryState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);	
	self->state = state;	
	self->instrument = NULL;	
	psy_ui_component_set_default_align(&self->component,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());
//	psy_ui_component_set_style_types(&self->component,
//		STYLE_TABLEROW, psy_INDEX_INVALID, psy_INDEX_INVALID,
//		psy_INDEX_INVALID);
	instrumententrytableview_build(self);	
	psy_ui_component_set_wheel_scroll(&self->component, 4);
	psy_ui_component_set_scroll_step(&self->component,
		psy_ui_size_make_em(0.0, 1.0));
	psy_ui_component_set_overflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
}

void instrumententrytableview_setinstrument(InstrumentEntryTableView* self,
	psy_audio_Instrument* instrument)
{
	if (self->instrument != instrument) {
		self->instrument = instrument;
		instrumententrytableview_build(self);		
		psy_ui_component_updateoverflow(&self->component);		
	}
}

void instrumententrytableview_build(InstrumentEntryTableView* self)
{		
	psy_ui_component_clear(&self->component);	
	if (self->instrument) {
		psy_List* p;		

		for (p = self->instrument->entries; p != NULL; p = p->next) {
			psy_audio_InstrumentEntry* entry;
			InstrumentEntryTableRow* row;

			entry = (psy_audio_InstrumentEntry*)psy_list_entry(p);
			row = (InstrumentEntryTableRow*)instrumententrytablerow_allocinit(
				&self->component, entry, self->state);			
		}
	}
	psy_ui_component_align(&self->component);
}

/* InstrumentNoteMapButtons */
/* implementation */
void instrumentnotemapbuttons_init(InstrumentNoteMapButtons* self,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init(&self->add, &self->component);
	psy_ui_button_set_text(&self->add, "instrumentview.add");
	psy_ui_button_init(&self->remove, &self->component);
	psy_ui_button_set_text(&self->remove, "instrumentview.remove");	
}

/* InstrumentNoteMapView */

/* prototypes */
static void instrumentnotemapview_on_destroyed(InstrumentNoteMapView*);	
static void instrumentnotemapview_initentries(InstrumentNoteMapView*,
	Workspace*);
static void instrumentnotemapview_inittable(InstrumentNoteMapView*,
	Workspace*);
static void instrumentnotemapview_on_add_entry(InstrumentNoteMapView*,
	psy_ui_Component* sender);
static void instrumentnotemapview_on_remove_entry(InstrumentNoteMapView*,
	psy_ui_Component* sender);
static void instrumentnotemapview_onentryselected(InstrumentNoteMapView*,
	InstrumentEntryState* sender, psy_audio_InstrumentEntry*);
static void instrumentnotemapview_onsampleselected(InstrumentNoteMapView*,
	SamplesBox*);
static void instrumentnotemapview_onentryupdate(
	InstrumentNoteMapView*, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry*);

/* vtable */
static psy_ui_ComponentVtable instrumentnotemapview_vtable;
static bool instrumentnotemapview_vtable_initialized = FALSE;

static void instrumentnotemapview_vtable_init(InstrumentNoteMapView* self)
{
	if (!instrumentnotemapview_vtable_initialized) {
		instrumentnotemapview_vtable = *(self->component.vtable);
		instrumentnotemapview_vtable.on_destroyed =
			(psy_ui_fp_component)
			instrumentnotemapview_on_destroyed;		
		instrumentnotemapview_vtable_initialized = TRUE;
	}
	self->component.vtable = &instrumentnotemapview_vtable;
}

/* implementation */
void instrumentnotemapview_init(InstrumentNoteMapView* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent, NULL);
	instrumentnotemapview_vtable_init(self);	
	instrumententrystate_init(&self->state, workspace);	
	psy_ui_label_init_text(&self->label, &self->component,
		"instrumentview.notemap");
	psy_ui_component_set_align(&self->label.component, psy_ui_ALIGN_TOP);
	samplesbox_init(&self->samplesbox, &self->component, NULL, workspace);
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_set_margin(&self->samplesbox.component, margin);
	psy_ui_component_set_align(&self->samplesbox.component, psy_ui_ALIGN_LEFT);	
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 1.5, 0.0);
	psy_ui_component_set_margin(&self->label.component, margin);
	instrumentnotemapbuttons_init(&self->buttons, &self->component);
	psy_ui_component_set_align(&self->buttons.component, psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(&self->buttons.component, margin);
	instrumentnotemapview_initentries(self, workspace);	
	instrumentnotemapview_inittable(self, workspace);
	psy_ui_splitter_init(&self->splitter, &self->component);
	psy_ui_component_set_align(psy_ui_splitter_base(&self->splitter),
		psy_ui_ALIGN_BOTTOM);	
	psy_signal_connect(&self->buttons.add.signal_clicked, self,
		instrumentnotemapview_on_add_entry);
	psy_signal_connect(&self->buttons.remove.signal_clicked, self,
		instrumentnotemapview_on_remove_entry);
	psy_signal_connect(&self->state.signal_select, self,
		instrumentnotemapview_onentryselected);	
	psy_signal_connect(&self->samplesbox.signal_changed, self,
		instrumentnotemapview_onsampleselected);
	psy_signal_connect(&self->state.signal_entrychanged, self,
		instrumentnotemapview_onentryupdate);
}

void instrumentnotemapview_on_destroyed(InstrumentNoteMapView* self)
{
	instrumententrystate_dispose(&self->state);
}

void instrumentnotemapview_initentries(InstrumentNoteMapView* self,
	Workspace* workspace)
{
	psy_ui_component_init(&self->entries, &self->component, NULL);
	psy_ui_component_set_align(&self->entries, psy_ui_ALIGN_CLIENT);
	pianokeyboard_init(&self->keyboard, &self->entries, &self->keyboard_state,
		NULL, NULL);
	psy_ui_component_set_preferred_height(&self->keyboard.component,
		psy_ui_value_make_eh(2.0));	
	psy_ui_component_set_align(&self->keyboard.component, psy_ui_ALIGN_TOP);	
	psy_ui_component_set_margin(&self->keyboard.component,
		psy_ui_margin_make_em(0.0, 0.0, 1.0, 0.0));
	instrumententryview_init(&self->entryview, &self->entries, &self->state);
	psy_ui_scroller_init(&self->scroller, &self->entries, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->entryview.component);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_align(&self->entryview.component, psy_ui_ALIGN_HCLIENT);	
	psy_ui_component_set_margin(&self->entryview.component,
		psy_ui_margin_make_em(0.0, 2.0, 0.0, 0.0));
	keyboardstate_init(&self->keyboard_state, psy_ui_HORIZONTAL, TRUE);
	self->keyboard_state.align_keys = TRUE;	
}

void instrumentnotemapview_inittable(InstrumentNoteMapView* self,
	Workspace* workspace)
{
	psy_ui_component_init_align(&self->table, &self->component, NULL,
		psy_ui_ALIGN_BOTTOM);	
	psy_ui_component_set_preferred_size(&self->table,
		psy_ui_size_make_em(0.0, 6.0));	
	instrumententrytableviewheader_init(&self->tableheader, &self->table,
		&self->state);
	psy_ui_component_set_align(&self->tableheader.component, psy_ui_ALIGN_TOP);	
	psy_ui_scroller_init(&self->scroller_table, &self->table, NULL, NULL);	
	instrumententrytableview_init(&self->tableview, &self->scroller_table.pane,
		&self->state);	
	psy_ui_scroller_set_client(&self->scroller_table,
		&self->tableview.component);
	psy_ui_component_set_align(&self->scroller_table.component,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_align(&self->tableview.component, psy_ui_ALIGN_FIXED);	
}

void instrumentnotemapview_set_instrument(InstrumentNoteMapView* self,
	psy_audio_Instrument* instrument)
{
	psy_audio_InstrumentEntry* entry;

	self->instrument = instrument;
	self->state.selectedentry = NULL;
	instrumententryview_set_instrument(&self->entryview, instrument);
	instrumententrytableview_setinstrument(&self->tableview, instrument);
	entry = instrumententryview_selected(&self->entryview);
	if (entry) {
		samplesbox_select(&self->samplesbox, entry->sampleindex);
	} else {
		samplesbox_select(&self->samplesbox, psy_audio_sampleindex_make(0, 0));
	}
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_align(&self->scroller_table.pane);
}

void instrumentnotemapview_on_add_entry(InstrumentNoteMapView* self,
	psy_ui_Component* sender)
{	
	if (self->instrument) {
		psy_audio_InstrumentEntry entry;

		psy_audio_instrumententry_init(&entry);
		entry.sampleindex = samplesbox_selected(&self->samplesbox);
		psy_audio_instrument_addentry(self->instrument, &entry);
		instrumententrytableview_build(&self->tableview);
		instrumententryview_build(&self->entryview);
		psy_ui_component_align(&self->scroller.pane);
		psy_ui_component_invalidate(&self->scroller.component);
		psy_ui_component_align(&self->scroller_table.pane);
		psy_ui_component_invalidate(&self->scroller_table.component);
	}
}

void instrumentnotemapview_on_remove_entry(InstrumentNoteMapView* self,
	psy_ui_Component* sender)
{
	if (self->instrument) {		
		uintptr_t row;

		row = psy_list_entry_index(self->instrument->entries, self->state.selectedentry);
		psy_audio_instrument_removeentry(self->instrument, row);
		self->state.selectedentry = NULL;
		instrumententrytableview_build(&self->tableview);
		instrumententryview_build(&self->entryview);
		psy_ui_component_align(&self->scroller.pane);
		psy_ui_component_invalidate(&self->scroller.component);
		psy_ui_component_align(&self->scroller_table.pane);
		psy_ui_component_invalidate(&self->scroller_table.component);
	}
}

void instrumentnotemapview_onentryselected(InstrumentNoteMapView* self,
	InstrumentEntryState* sender, psy_audio_InstrumentEntry* entry)
{			
	if (self->instrument && entry) {
		if (self->state.selectedentry) {
			self->keyboard_state.entry = *self->state.selectedentry;
		} else {
			self->keyboard_state.entry.keyrange.low = 255;
			self->keyboard_state.entry.keyrange.high = 255;
		}		
		samplesbox_select(&self->samplesbox, entry->sampleindex);				
	} else {
		self->keyboard_state.entry.keyrange.low = 255;
		self->keyboard_state.entry.keyrange.high = 255;
		samplesbox_select(&self->samplesbox, psy_audio_sampleindex_make(0, 0));
	}
	psy_ui_component_invalidate(&self->keyboard.component);
}

void instrumentnotemapview_onsampleselected(InstrumentNoteMapView* self,
	SamplesBox* samplebox)
{	
	instrumententryview_setsample(&self->entryview,
		samplesbox_selected(&self->samplesbox));	
	psy_ui_component_invalidate(&self->tableview.component);
}

void instrumentnotemapview_onentryupdate(
	InstrumentNoteMapView* self, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry* entry)
{
	if (self->state.selectedentry) {
		self->keyboard_state.entry = *self->state.selectedentry;
	} else {
		self->keyboard_state.entry.keyrange.low = 255;
		self->keyboard_state.entry.keyrange.high = 255;
	}
	psy_ui_component_invalidate(&self->keyboard.component);

}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instnotemapview.h"

// std
#include <math.h>
// platform
#include "../../detail/portable.h"

static int isblack(int key)
{
	int offset = key % 12;
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
	return (offset == 1 || offset == 3 || offset == 6 || offset == 8
		|| offset == 10);
}

static int numwhitekey(int key)
{
	int octave = key / 12;
	int offset = key % 12;
	int c = 0;
	int i;

	for (i = 1; i <= offset; ++i) {
		if (!isblack(i)) ++c;
	}
	return octave * 7 + c;
}

static uint8_t whitekeytokey(uint8_t whitekey)
{
	uint8_t octave = whitekey / 7;
	uint8_t offset = whitekey % 7;
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
	static int numkey[] = { 0, 2, 4, 5, 7, 9, 11 };
	return octave * 12 + numkey[offset];
}

static uint8_t screentokey(double x, double keysize)
{
	uint8_t rv;
	uint8_t numwhitekey;	

	numwhitekey = (uint8_t)(x / keysize);
	rv = whitekeytokey(numwhitekey);
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
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

// keyboardview
static void instrumentkeyboardview_ondraw(InstrumentKeyboardView*,
	psy_ui_Graphics*);
static void instrumentkeyboardview_onsize(InstrumentKeyboardView*, psy_ui_Size*);
static void instrumentkeyboardview_updatemetrics(InstrumentKeyboardView*);


static psy_ui_ComponentVtable instrumentkeyboardview_vtable;
static int instrumentkeyboardview_vtable_initialized = 0;

static void instrumentkeyboardview_vtable_init(InstrumentKeyboardView* self)
{
	if (!instrumentkeyboardview_vtable_initialized) {
		instrumentkeyboardview_vtable = *(self->component.vtable);
		instrumentkeyboardview_vtable.ondraw = (psy_ui_fp_component_ondraw)
			instrumentkeyboardview_ondraw;
		instrumentkeyboardview_vtable.onsize = (psy_ui_fp_component_onsize)
			instrumentkeyboardview_onsize;
		instrumentkeyboardview_vtable_initialized = 1;
	}
}

void instrumentkeyboardview_init(InstrumentKeyboardView* self,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent);		
	instrumentkeyboardview_vtable_init(self);	
	self->component.vtable = &instrumentkeyboardview_vtable;
	self->metrics.keysize = 6;
	self->metrics.lineheight = 15;
	psy_ui_component_preventalign(&self->component);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_makepx(0),
		psy_ui_value_makeeh(3)));	
}

void instrumentkeyboardview_ondraw(InstrumentKeyboardView* self, psy_ui_Graphics* g)
{		
	int keymin = 0;
	int keymax = psy_audio_NOTECOMMANDS_RELEASE;
	int key;
	double cp = 0;
	float top = 0.50;
	float bottom = 1 - top;	
	psy_ui_RealSize size;
	
	size = psy_ui_component_sizepx(&self->component);		
	psy_ui_setcolour(g, psy_ui_colour_make(0x00333333));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00333333));
	for (key = keymin; key < keymax; ++key) {
		if (!isblack(key)) {
			psy_ui_RealRectangle r;

			psy_ui_setrectangle(&r,
				cp, 0,
				(self->metrics.keysize + 1), size.height);
			psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00CACACA));
			psy_ui_drawline(g, psy_ui_realpoint_make(cp, 0),
				psy_ui_realpoint_make(cp, size.height));
			cp += self->metrics.keysize;			
		}
	}
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00CACACA));
	for (cp = 0, key = keymin; key < keymax; ++key) {							
		if (!isblack(key)) {			
			cp += self->metrics.keysize;
		} else {
			psy_ui_RealRectangle r;
			int x;
			int width;

			x = (int)cp - (int)(self->metrics.keysize * 0.8 / 2);
			width = (int)(self->metrics.keysize * 0.8);
			psy_ui_setrectangle(&r, x, 0, width, (int)(size.height * top));
			psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00444444));
		}
	}
}

void instrumentkeyboardview_onsize(InstrumentKeyboardView* self, psy_ui_Size* size)
{
	instrumentkeyboardview_updatemetrics(self);
}

void instrumentkeyboardview_updatemetrics(InstrumentKeyboardView* self)
{
	int keymin = 0;
	int keymax = psy_audio_NOTECOMMANDS_RELEASE;
	int key;
	int numwhitekeys;
	const psy_ui_TextMetric* tm;
	psy_ui_IntSize size;

	numwhitekeys = 0;
	for (key = keymin; key < keymax; ++key) {
		if (!isblack(key)) {
			++numwhitekeys;
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), tm);
	self->metrics.keysize = size.width / (double)numwhitekeys;
	self->component.scrollstepy = psy_ui_value_makepx(self->metrics.lineheight * 3);
}

// entry view
static void instrumententryview_ondestroy(InstrumentEntryView*);
static void instrumententryview_onpreferredsize(InstrumentEntryView*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void instrumententryview_ondraw(InstrumentEntryView*, psy_ui_Graphics*);
static void instrumententryview_onsize(InstrumentEntryView*, psy_ui_Size*);
static void instrumententryview_onmousedown(InstrumentEntryView*,
	psy_ui_MouseEvent*);
static void instrumententryview_onmousemove(InstrumentEntryView*,
	psy_ui_MouseEvent*);
static void instrumententryview_onmouseup(InstrumentEntryView*,
	psy_ui_MouseEvent*);
static double instrumententryview_keysize(InstrumentEntryView*);
static void instrumententryview_updatemetrics(InstrumentEntryView*);
// vtable
static psy_ui_ComponentVtable instrumententryview_vtable;
static int instrumententryview_vtable_initialized = 0;

static void instrumententryview_vtable_init(InstrumentEntryView* self)
{
	if (!instrumententryview_vtable_initialized) {
		instrumententryview_vtable = *(self->component.vtable);
		instrumententryview_vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			instrumententryview_ondestroy;
		instrumententryview_vtable.ondraw = (psy_ui_fp_component_ondraw)
			instrumententryview_ondraw;
		instrumententryview_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			instrumententryview_onmousedown;
		instrumententryview_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)
			instrumententryview_onmousemove;
		instrumententryview_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)
			instrumententryview_onmouseup;
		instrumententryview_vtable.onsize = (psy_ui_fp_component_onsize)
			instrumententryview_onsize;
		instrumententryview_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			instrumententryview_onpreferredsize;
		instrumententryview_vtable_initialized = 1;
	}
}
// implementation
void instrumententryview_init(InstrumentEntryView* self,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent);
	instrumententryview_vtable_init(self);
	self->component.vtable = &instrumententryview_vtable;
	psy_ui_component_doublebuffer(&self->component);	
	self->instrument = 0;
	self->metrics.keysize = 8;
	self->metrics.lineheight = 15;
	self->dragmode = 0;
	self->selected = psy_INDEX_INVALID;
	self->component.scrollstepy = psy_ui_value_makepx(45);
	instrumententryview_updatemetrics(self);	
	psy_signal_init(&self->signal_selected);
}

void instrumententryview_ondestroy(InstrumentEntryView* self)
{
	psy_signal_dispose(&self->signal_selected);
}

void instrumententryview_setinstrument(InstrumentEntryView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	if (self->instrument && self->instrument->entries) {
		self->selected = 0;
	} else {
		self->selected = psy_INDEX_INVALID;
	}
	psy_ui_component_setscrolltop(&self->component, psy_ui_value_zero());	
	psy_ui_component_updateoverflow(&self->component);	
	psy_ui_component_invalidate(&self->component);	
}

void instrumententryview_ondraw(InstrumentEntryView* self, psy_ui_Graphics* g)
{	
	if (self->instrument) {
		int cpy;
		psy_List* p;		
		uintptr_t c = 0;		
		const psy_ui_TextMetric* tm;
		psy_ui_RealSize size;
		uint8_t keymin = 0;
		uint8_t keymax = psy_audio_NOTECOMMANDS_RELEASE;
		uint8_t key;
		uint8_t numwhitekeys;

		numwhitekeys = 0;
		for (key = keymin; key < keymax;  ++key) {
			if (!isblack(key)) {
				++numwhitekeys;
			}
		}
		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_component_sizepx(&self->component);
		cpy = 0;
		if (self->selected != psy_INDEX_INVALID && self->instrument && self->instrument->entries) {
			psy_audio_InstrumentEntry* entry;
			double keylo_startx;
			double keylo_endx;
			double keyhi_startx;
			double keyhi_endx;
			psy_ui_RealRectangle r;
			double scrollleft;
			double scrolltop;

			scrollleft = psy_ui_component_scrollleftpx(&self->component);
			scrolltop = psy_ui_component_scrolltoppx(&self->component);
			entry = (psy_audio_InstrumentEntry*)psy_audio_instrument_entryat(
				self->instrument, self->selected);
			if (entry) {
				keylo_startx = (int)(
					(float)numwhitekey((uint8_t)entry->keyrange.low) /
					numwhitekeys * size.width) +
					(int)(isblack((uint8_t)entry->keyrange.low)
						? self->metrics.keysize / 2 : 0);
				keylo_endx = (int)(
					(float)numwhitekey((uint8_t)entry->keyrange.low + 1) /
					numwhitekeys * size.width) +
					(int)(isblack((uint8_t)entry->keyrange.low + 1)
						? self->metrics.keysize / 2 : 0);								
				keyhi_startx = (int)(
					(float)numwhitekey((uint8_t)entry->keyrange.high) /
					numwhitekeys * size.width) +
					(int)(isblack((uint8_t)entry->keyrange.high)
						? self->metrics.keysize / 2 : 0);
				keyhi_endx = (int)(
					(float)numwhitekey((uint8_t)entry->keyrange.high + 1) /
					numwhitekeys * size.width) +
					(int)(isblack((uint8_t)entry->keyrange.high + 1)
						? self->metrics.keysize / 2 : 0);
				psy_ui_setrectangle(&r,
					keylo_startx, scrolltop,
					keyhi_endx - keylo_startx, scrolltop + size.height);
				psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00333333));
				psy_ui_setrectangle(&r,
					keylo_startx, scrolltop,
					keylo_endx - keylo_startx, scrolltop + size.height);
				psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00444444));
				psy_ui_setrectangle(&r,
					keyhi_startx, scrolltop,
					keyhi_endx - keyhi_startx, scrolltop + size.height);
				psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00444444));
			}
		}
		for (p = self->instrument->entries; p != NULL; psy_list_next(&p), ++c) {
			psy_audio_InstrumentEntry* entry;
			int startx;
			int endx;
			psy_ui_RealRectangle r;

			entry = (psy_audio_InstrumentEntry*) p->entry;
			assert(entry);
			startx = (int)(
				(float)numwhitekey((uint8_t)entry->keyrange.low) /
				numwhitekeys * size.width) +
				(int)(isblack((uint8_t)entry->keyrange.low)
					? self->metrics.keysize / 2 : 0);
			endx = (int)(
				(float)numwhitekey((uint8_t)entry->keyrange.high + 1) /
				numwhitekeys * size.width) +
				(int)(isblack((uint8_t)entry->keyrange.high + 1)
					? self->metrics.keysize / 2 : 0) - 1;
			if (c == self->selected) {
				psy_ui_setcolour(g, psy_ui_colour_make(0x00EAEAEA));
			} else {
				psy_ui_setcolour(g, psy_ui_colour_make(0x00CACACA));
			}
			psy_ui_realrectangle_init_all(&r,
				psy_ui_realpoint_make(startx, cpy),
				psy_ui_realsize_make(endx - startx, 5));
			psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00FFFFFF));			
			cpy += self->metrics.lineheight;
		}		
		if (!self->instrument->entries) {
			static const char* nomapping = "No Instrument Mapping";

			psy_ui_textout(g, 
				(size.width - tm->tmAveCharWidth * strlen(nomapping)) / 2,
				(size.height - tm->tmHeight) / 2,
				nomapping, strlen(nomapping));
		}
	} else {
		const psy_ui_TextMetric* tm;
		psy_ui_RealSize size;
		static const char* noinst = "No Instrument";

		tm = psy_ui_component_textmetric(&self->component);
		size =  psy_ui_component_sizepx(&self->component);
		psy_ui_textout(g,
			(size.width - tm->tmAveCharWidth * strlen(noinst)) / 2,
			(size.height - tm->tmHeight) / 2,
			noinst, strlen(noinst));		
	}
}

void instrumententryview_onsize(InstrumentEntryView* self, psy_ui_Size* size)
{
	instrumententryview_updatemetrics(self);
}

void instrumententryview_onpreferredsize(InstrumentEntryView* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (self->instrument && self->instrument->entries) {		
		rv->height = psy_ui_value_makepx(
			(self->metrics.lineheight) * (double)psy_list_size(self->instrument->entries));
	} else {
		*rv = psy_ui_size_zero();
	}
}

void instrumententryview_updatemetrics(InstrumentEntryView* self)
{
	int keymin = 0;
	int keymax = psy_audio_NOTECOMMANDS_RELEASE;
	int key;
	int numwhitekeys;
	const psy_ui_TextMetric* tm;
	psy_ui_IntSize size;

	numwhitekeys = 0;
	for (key = keymin; key < keymax; ++key) {
		if (!isblack(key)) {
			++numwhitekeys;
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), tm);
	self->metrics.keysize = size.width / (double)numwhitekeys;
	self->component.scrollstepy = psy_ui_value_makepx(self->metrics.lineheight * 3);
}

void instrumententryview_onmousedown(InstrumentEntryView* self,
	psy_ui_MouseEvent* ev)
{
	if (self->instrument) {		
		psy_audio_InstrumentEntry* entry;
							
		if (self->instrument) {
			uintptr_t numentry;	

			numentry = (uintptr_t)(ev->pt.y / (self->metrics.lineheight));
			if (numentry < psy_list_size(
					psy_audio_instrument_entries(self->instrument))) {
				self->selected = numentry;
			} else {
				self->selected = psy_INDEX_INVALID;
			}
		} else {
			self->selected = psy_INDEX_INVALID;
		}
		self->dragmode = 1;
		self->currkey = screentokey(ev->pt.x, self->metrics.keysize);		
		entry = psy_audio_instrument_entryat(self->instrument, self->selected);
		if (entry) {
			psy_signal_emit(&self->signal_selected, self, 0);
			if (abs((int)(entry->keyrange.low  - screentokey(ev->pt.x,
					self->metrics.keysize))) <
				abs((int)(entry->keyrange.high  - screentokey(ev->pt.x,
					self->metrics.keysize)))) {
				self->dragmode = INSTVIEW_DRAG_LEFT;
				psy_ui_component_setcursor(&self->component,
					psy_ui_CURSORSTYLE_COL_RESIZE);
			} else {
				self->dragmode = INSTVIEW_DRAG_RIGHT;
				psy_ui_component_setcursor(&self->component,
					psy_ui_CURSORSTYLE_COL_RESIZE);
			}
			psy_ui_component_capture(&self->component);
		}
		psy_ui_component_invalidate(&self->component);
	}
}

double instrumententryview_keysize(InstrumentEntryView* self)
{	
	int key;
	int keymin = 0;
	int keymax = psy_audio_NOTECOMMANDS_RELEASE - 1;
	int numwhitekeys;
	const psy_ui_TextMetric* tm;
	psy_ui_Size size;

	numwhitekeys = 0;
	for (key = keymin; key < keymax; ++key) {
		if (!isblack(key)) {
			++numwhitekeys;
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size(&self->component);
	return psy_ui_value_px(&size.width, tm) / numwhitekeys;
}

void instrumententryview_onmousemove(InstrumentEntryView* self,
	psy_ui_MouseEvent* ev)
{	
	bool showresizecursor;

	showresizecursor = FALSE;
	self->currkey = screentokey(ev->pt.x, self->metrics.keysize);
	if (self->dragmode != INSTVIEW_DRAG_NONE && self->instrument) {
		psy_audio_InstrumentEntry* entry;		
				
		entry = psy_audio_instrument_entryat(self->instrument, self->selected);
		if (entry) {			
			if (self->dragmode == INSTVIEW_DRAG_LEFT) {
				entry->keyrange.low = screentokey(ev->pt.x,
					self->metrics.keysize);
				if (entry->keyrange.low > entry->keyrange.high) {
					entry->keyrange.low = entry->keyrange.high;
				}
			} else {
				entry->keyrange.high = screentokey(ev->pt.x,
					self->metrics.keysize);
				if (entry->keyrange.high < entry->keyrange.low) {
					entry->keyrange.high = entry->keyrange.low;
				}
			}
			psy_ui_component_invalidate(&self->component);			
		}
		showresizecursor = TRUE;
	}
	if (showresizecursor != FALSE) {
		psy_ui_component_setcursor(&self->component,
			psy_ui_CURSORSTYLE_COL_RESIZE);
	}
}

void instrumententryview_onmouseup(InstrumentEntryView* self,
	psy_ui_MouseEvent* ev)
{
	if (self->instrument) {
		self->dragmode = 0;		
		psy_ui_component_releasecapture(&self->component);
		psy_signal_emit(&self->signal_selected, self, 0);
	}
}

psy_audio_InstrumentEntry* instrumententryview_selected(InstrumentEntryView* self)
{
	if (self->instrument) {
		return psy_audio_instrument_entryat(self->instrument, self->selected);
	}
	return NULL;
}

void instrumententryview_select(InstrumentEntryView* self, uintptr_t row)
{
	self->selected = row;
	psy_ui_component_invalidate(&self->component);
}

void instrumententryview_setsample(InstrumentEntryView* self, psy_audio_SampleIndex index)
{
	if (psy_audio_sampleindex_valid(&index)) {
		psy_audio_InstrumentEntry* entry;

		entry = instrumententryview_selected(self);
		if (entry) {
			entry->sampleindex = index;
		}
	}
}

// Table

// TableColumn
void instrumententrytablecolumn_init(InstrumentEntryTableColumn* self, const char* label)
{	
	self->width = psy_ui_value_makeew(10.0);
	self->label = psy_strdup(label);
}

void instrumententrytablecolumn_dispose(InstrumentEntryTableColumn* self)
{
	free(self->label);
}

InstrumentEntryTableColumn* instrumententrytablecolumn_alloc(void)
{
	return (InstrumentEntryTableColumn*)malloc(sizeof(InstrumentEntryTableColumn));
}

InstrumentEntryTableColumn* instrumententrytablecolumn_allocinit(const char* label)
{
	InstrumentEntryTableColumn* rv;
	
	rv = instrumententrytablecolumn_alloc();
	if (rv) {
		instrumententrytablecolumn_init(rv, label);
	}
	return rv;
}

void instrumententrytablestate_init(InstrumentEntryTableState* self)
{
	InstrumentEntryTableColumn* column;				

	psy_table_init(&self->columns);
	// sample index
	column = instrumententrytablecolumn_allocinit("Smpl Idx");
	psy_table_insert(&self->columns, 0, column);
	// key low
	column = instrumententrytablecolumn_allocinit("Key Lo");
	psy_table_insert(&self->columns, 1, column);
	// key hi
	column = instrumententrytablecolumn_allocinit("Key Hi");
	psy_table_insert(&self->columns, 2, column);
	// fixedkey
	column = instrumententrytablecolumn_allocinit("Key Fixed");
	psy_table_insert(&self->columns, 3, column);
}

void instrumententrytablestate_dispose(InstrumentEntryTableState* self)
{
	psy_table_disposeall(&self->columns, (psy_fp_disposefunc)
		instrumententrytablecolumn_dispose);
}

// InstrumentEntryTableViewHeader
// prototypes
static void instrumententrytableviewheader_ondraw(InstrumentEntryTableViewHeader*,
	psy_ui_Graphics*);
static void instrumententrytableviewheader_onpreferredsize(InstrumentEntryTableViewHeader*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable instrumententrytableviewheader_vtable;
static bool instrumententrytableviewheader_vtable_initialized = FALSE;

static void instrumententrytableviewheader_vtable_init(InstrumentEntryTableViewHeader* self)
{
	if (!instrumententrytableviewheader_vtable_initialized) {
		instrumententrytableviewheader_vtable = *(self->component.vtable);
		instrumententrytableviewheader_vtable.ondraw = (psy_ui_fp_component_ondraw)
			instrumententrytableviewheader_ondraw;
		instrumententrytableviewheader_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			instrumententrytableviewheader_onpreferredsize;
		instrumententrytableviewheader_vtable_initialized = TRUE;
	}
}
// implementation
void instrumententrytableviewheader_init(InstrumentEntryTableViewHeader* self,
	psy_ui_Component* parent, InstrumentEntryTableState* state,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	instrumententrytableviewheader_vtable_init(self);
	self->component.vtable = &instrumententrytableviewheader_vtable;
	psy_ui_component_doublebuffer(&self->component);
	self->state = state;
}

void instrumententrytableviewheader_ondraw(InstrumentEntryTableViewHeader* self,
	psy_ui_Graphics* g)
{		
	psy_TableIterator it;
	double cpx;
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);
	for (it = psy_table_begin(&self->state->columns), cpx = 0.0;
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		char text[40];
		InstrumentEntryTableColumn* column;

		column = (InstrumentEntryTableColumn*)psy_tableiterator_value(&it);
		if (column->label) {
			psy_snprintf(text, 40, "%s", column->label);
			psy_ui_textout(g, cpx, 0, text, strlen(text));
		}
		cpx += psy_ui_value_px(&column->width, tm);
	}
}

void instrumententrytableviewheader_onpreferredsize(InstrumentEntryTableViewHeader* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	*rv = psy_ui_size_makeem(80.0, 1.0);
}

// InstrumentEntryTableView
// prototypes
static void instrumententrytableview_ondestroy(InstrumentEntryTableView*);
static void instrumententrytableview_onpreferredsize(InstrumentEntryTableView*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void instrumententrytableview_ondraw(InstrumentEntryTableView*,
	psy_ui_Graphics*);
static void instrumententrytableview_onmousedown(InstrumentEntryTableView*,
	psy_ui_MouseEvent*);
static void instrumententrytableview_onmousemove(InstrumentEntryTableView*,
	psy_ui_MouseEvent*);
static void instrumententrytableview_onmouseup(InstrumentEntryTableView*,
	psy_ui_MouseEvent*);

// vtable
static psy_ui_ComponentVtable instrumententrytableview_vtable;
static bool instrumententrytableview_vtable_initialized = FALSE;

static void instrumententrytableview_vtable_init(InstrumentEntryTableView* self)
{
	if (!instrumententrytableview_vtable_initialized) {
		instrumententrytableview_vtable = *(self->component.vtable);
		instrumententrytableview_vtable.ondraw = (psy_ui_fp_component_ondraw)
			instrumententrytableview_ondraw;
		instrumententrytableview_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			instrumententrytableview_onmousedown;
		instrumententrytableview_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)
			instrumententrytableview_onmousemove;
		instrumententrytableview_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)
			instrumententrytableview_onmouseup;		
		instrumententrytableview_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			instrumententrytableview_onpreferredsize;
		instrumententrytableview_vtable_initialized = TRUE;
	}
}

void instrumententrytableview_init(InstrumentEntryTableView* self,
	psy_ui_Component* parent, InstrumentEntryTableState* state)
{
	psy_ui_component_init(&self->component, parent);
	instrumententrytableview_vtable_init(self);
	self->component.vtable = &instrumententrytableview_vtable;
	psy_ui_component_doublebuffer(&self->component);
	self->state = state;	
	self->instrument = NULL;
	self->selected = psy_INDEX_INVALID;
	psy_signal_init(&self->signal_selected);
	psy_ui_component_setbackgroundcolour(&self->component,
		psy_ui_colour_make(0x00232323));
}

void instrumententrytableview_ondestroy(InstrumentEntryTableView* self)
{
	psy_signal_dispose(&self->signal_selected);
}

void instrumententrytableview_setinstrument(InstrumentEntryTableView* self,
	psy_audio_Instrument* instrument)
{
	if (self->instrument != instrument) {
		self->instrument = instrument;
		psy_ui_component_updateoverflow(&self->component);
		self->selected = 0;
	}
}

psy_audio_InstrumentEntry* instrumententrytableview_selected(InstrumentEntryTableView* self)
{
	if (self->instrument) {
		return psy_audio_instrument_entryat(self->instrument, self->selected);
	}
	return NULL;
}

void instrumententrytableview_select(InstrumentEntryTableView* self, uintptr_t row)
{
	self->selected = row;
	psy_ui_component_invalidate(&self->component);
}

void instrumententrytableview_onpreferredsize(InstrumentEntryTableView* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (self->instrument) {		
		*rv = psy_ui_size_makeem(80.0,
			psy_max(6.0, (double)psy_list_size(self->instrument->entries)));
	} else {
		*rv = psy_ui_size_makeem(80.0, 6.0);
	}
}

void instrumententrytableview_ondraw(InstrumentEntryTableView* self,
	psy_ui_Graphics* g)
{
	int cpy = 0;
	psy_List* p;	
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;
	uintptr_t i;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_sizepx(&self->component);
	psy_ui_setcolour(g, psy_ui_colour_make(0x00CACACA));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00CACACA));
	for (i = 0, p = self->instrument->entries; p != NULL; p = p->next, ++i) {
		psy_audio_InstrumentEntry* entry;		
		psy_TableIterator it;						
		double cpx;

		entry = (psy_audio_InstrumentEntry*)psy_list_entry(p);
		if (self->selected == i) {
			psy_ui_RealRectangle r;

			psy_ui_realrectangle_init_all(&r,
				psy_ui_realpoint_make(0, cpy),
				psy_ui_realsize_make(size.width, 20));
			psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00333333));
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00FFFFFF));
		} else {
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00F2F2F2));
		}
		for (it = psy_table_begin(&self->state->columns), cpx = 0.0;
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			char text[40];
			InstrumentEntryTableColumn* column;

			column = (InstrumentEntryTableColumn*)psy_tableiterator_value(&it);			
			switch (psy_tableiterator_key(&it)) {
			case 0:
				psy_snprintf(text, 40, "%0X:%0X", entry->sampleindex.slot,
					entry->sampleindex.subslot);
				break;
			case 1:
				psy_snprintf(text, 40, "%.3d", entry->keyrange.low);
				break;
			case 2:
				psy_snprintf(text, 40, "%.3d", entry->keyrange.high);
				break;
			case 3:				
				if (entry->fixedkey == psy_audio_NOTECOMMANDS_EMPTY) {
					psy_snprintf(text, 40, "%s", "-");
				} else {
					psy_snprintf(text, 40, "%d", (int)entry->fixedkey);
				}
				break;			
			default:
				break;
			}						
			psy_ui_textout(g, cpx, cpy, text, strlen(text));
			cpx += psy_ui_value_px(&column->width, tm);
		}
		entry = (psy_audio_InstrumentEntry*)p->entry;		
		cpy += 20;
	}
}

void instrumententrytableview_onmousedown(InstrumentEntryTableView* self,
	psy_ui_MouseEvent* ev)
{
	if (self->instrument) {
		psy_audio_InstrumentEntry* entry;

		if (self->instrument) {
			uintptr_t numentry;

			numentry = (uintptr_t)(ev->pt.y / (20.0));
			if (numentry < psy_list_size(
				psy_audio_instrument_entries(self->instrument))) {
				self->selected = numentry;
			} else {
				self->selected = psy_INDEX_INVALID;
			}
		} else {
			self->selected = psy_INDEX_INVALID;
		}
		self->dragmode = 1;
		self->currkey = screentokey(ev->pt.x, self->metrics.keysize);
		entry = psy_audio_instrument_entryat(self->instrument, self->selected);
		if (entry) {
			psy_signal_emit(&self->signal_selected, self, 0);
		}
		psy_ui_component_invalidate(&self->component);
	}
}

void instrumententrytableview_onmousemove(InstrumentEntryTableView* self,
	psy_ui_MouseEvent* ev)
{

}

void instrumententrytableview_onmouseup(InstrumentEntryTableView* self,
	psy_ui_MouseEvent* ev)
{

}

psy_audio_InstrumentEntry* instrumententrytavleview_selected(InstrumentEntryTableView* self)
{
	if (self->instrument) {
		return psy_audio_instrument_entryat(self->instrument, self->selected);
	}
	return NULL;
}


// InstrumentNoteMapButtons
// implementation
void instrumentnotemapbuttons_init(InstrumentNoteMapButtons* self,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init(&self->add, &self->component);
	psy_ui_button_settext(&self->add, "Add");
	psy_ui_button_init(&self->remove, &self->component);
	psy_ui_button_settext(&self->remove, "Remove");	
}

// InstrumentNoteMapView
// prototypes
static void instrumentnotemapview_initentries(InstrumentNoteMapView*,
	Workspace*);
static void instrumentnotemapview_inittable(InstrumentNoteMapView*,
	Workspace*);
static void instrumentnotemapview_setmetrics(InstrumentNoteMapView*,
	InstrumentNoteMapMetrics);
static void instrumentnotemapview_onaddentry(InstrumentNoteMapView*, psy_ui_Component* sender);
static void instrumentnotemapview_onremoveentry(InstrumentNoteMapView*, psy_ui_Component* sender);
static void instrumentnotemapview_onentryselected(InstrumentNoteMapView*,
	InstrumentEntryView*);
static void instrumentnotemapview_ontableentryselected(InstrumentNoteMapView*,
	InstrumentEntryTableView*);
static void instrumentnotemapview_onsampleselected(InstrumentNoteMapView*,
	SamplesBox*);
// implementation
void instrumentnotemapview_init(InstrumentNoteMapView* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent);	
	self->metrics.keysize = 8;
	self->metrics.lineheight = 15;
	psy_ui_label_init_text(&self->label, &self->component, "Notemap");
	psy_ui_component_setalign(&self->label.component, psy_ui_ALIGN_TOP);
	samplesbox_init(&self->samplesbox, &self->component, NULL, workspace);
	psy_ui_margin_init_all_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_setmargin(&self->samplesbox.component, &margin);
	psy_ui_component_setalign(&self->samplesbox.component, psy_ui_ALIGN_LEFT);	
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 1.5, 0.0);
	psy_ui_component_setmargin(&self->label.component, &margin);
	instrumentnotemapbuttons_init(&self->buttons, &self->component);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->buttons.component, &margin);
	instrumentnotemapview_initentries(self, workspace);	
	instrumentnotemapview_inittable(self, workspace);
	psy_ui_splitbar_init(&self->splitter, &self->component);
	psy_ui_component_setalign(psy_ui_splitbar_base(&self->splitter),
		psy_ui_ALIGN_BOTTOM);
	instrumentnotemapview_setmetrics(self, self->metrics);
	psy_signal_connect(&self->buttons.add.signal_clicked, self,
		instrumentnotemapview_onaddentry);
	psy_signal_connect(&self->buttons.remove.signal_clicked, self,
		instrumentnotemapview_onremoveentry);
	psy_signal_connect(&self->entryview.signal_selected, self,
		instrumentnotemapview_onentryselected);
	psy_signal_connect(&self->tableview.signal_selected, self,
		instrumentnotemapview_ontableentryselected);
	psy_signal_connect(&self->samplesbox.signal_changed, self,
		instrumentnotemapview_onsampleselected);
}

void instrumentnotemapview_initentries(InstrumentNoteMapView* self, Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->entries, &self->component);
	psy_ui_component_setalign(&self->entries, psy_ui_ALIGN_CLIENT);
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 0.0, 2.0);
	instrumententryview_init(&self->entryview, &self->entries);
	psy_ui_scroller_init(&self->scroller, &self->entryview.component,
		&self->entries);
	psy_ui_component_setoverflow(&self->entryview.component, psy_ui_OVERFLOW_VSCROLL);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	// todo: scroller here slows X11 down
	// psy_ui_component_setalign(&self->entryview.component, psy_ui_ALIGN_CLIENT);
	psy_ui_margin_init_all_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_setmargin(&self->entryview.component, &margin);	
	instrumentkeyboardview_init(&self->keyboard, &self->entries);
	psy_ui_component_setalign(&self->keyboard.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setmargin(&self->keyboard.component, &margin);
	psy_ui_margin_init_all_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_setmargin(&self->keyboard.component, &margin);
}

void instrumentnotemapview_inittable(InstrumentNoteMapView* self, Workspace* workspace)
{
	// table
	instrumententrytablestate_init(&self->tablestate);
	psy_ui_component_init(&self->table, &self->component);
	psy_ui_component_setalign(&self->table, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setpreferredsize(&self->table,
		psy_ui_size_makeem(0.0, 6.0));
	self->table.preventpreferredsizeatalign = 1;	
	instrumententrytableviewheader_init(&self->tableheader,
		&self->table, &self->tablestate, workspace);
	psy_ui_component_setalign(&self->tableheader.component, psy_ui_ALIGN_TOP);
	instrumententrytableview_init(&self->tableview,
		&self->table, &self->tablestate);
	psy_ui_scroller_init(&self->scroller_table, &self->tableview.component,
		&self->table);
	psy_ui_component_setalign(&self->scroller_table.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setoverflow(&self->tableview.component, psy_ui_OVERFLOW_VSCROLL);
}

void instrumentnotemapview_setinstrument(InstrumentNoteMapView* self,
	psy_audio_Instrument* instrument)
{
	psy_audio_InstrumentEntry* entry;

	self->instrument = instrument;
	instrumententryview_setinstrument(&self->entryview, instrument);
	instrumententrytableview_setinstrument(&self->tableview, instrument);
	entry = instrumententryview_selected(&self->entryview);
	if (entry) {
		samplesbox_select(&self->samplesbox, entry->sampleindex);
	} else {
		samplesbox_select(&self->samplesbox, psy_audio_sampleindex_make(0, 0));
	}
}

void instrumentnotemapview_setmetrics(InstrumentNoteMapView* self,
	InstrumentNoteMapMetrics metrics)
{
	self->metrics = metrics;
	self->entryview.metrics = metrics;	
	self->keyboard.metrics = metrics;
}

void instrumentnotemapview_update(InstrumentNoteMapView* self)
{
	instrumentkeyboardview_updatemetrics(&self->keyboard);
	instrumententryview_updatemetrics(&self->entryview);
	psy_ui_component_updateoverflow(instrumententryview_base(
		&self->entryview));
	psy_ui_component_updateoverflow(&self->tableview.component);
	psy_ui_component_invalidate(instrumentnotemapview_base(self));
}

void instrumentnotemapview_onaddentry(InstrumentNoteMapView* self,
	psy_ui_Component* sender)
{	
	if (self->instrument) {
		psy_audio_InstrumentEntry entry;

		psy_audio_instrumententry_init(&entry);
		entry.sampleindex = samplesbox_selected(&self->samplesbox);
		psy_audio_instrument_addentry(self->instrument, &entry);
		instrumentnotemapview_update(self);
	}
}

void instrumentnotemapview_onremoveentry(InstrumentNoteMapView* self,
	psy_ui_Component* sender)
{
	if (self->instrument) {
		psy_audio_instrument_removeentry(self->instrument,
			self->entryview.selected);
		self->entryview.selected = psy_INDEX_INVALID;
		instrumentnotemapview_update(self);
	}
}

void instrumentnotemapview_onentryselected(InstrumentNoteMapView* self,
	InstrumentEntryView* entryview)
{
	psy_audio_InstrumentEntry* entry;
	
	entry = instrumententryview_selected(&self->entryview);
	if (self->instrument && entry) {
		uintptr_t index;
		samplesbox_select(&self->samplesbox, entry->sampleindex);

		index = psy_list_entry_index(self->instrument->entries,
			entry);
		instrumententrytableview_select(&self->tableview, index);
	} else {
		instrumententrytableview_select(&self->tableview, psy_INDEX_INVALID);
	}
}

void instrumentnotemapview_ontableentryselected(InstrumentNoteMapView* self,
	InstrumentEntryTableView* view)
{
	psy_audio_InstrumentEntry* entry;

	entry = instrumententrytableview_selected(&self->tableview);
	if (self->instrument && entry) {
		uintptr_t index;
		samplesbox_select(&self->samplesbox, entry->sampleindex);

		index = psy_list_entry_index(self->instrument->entries,
			entry);
		instrumententryview_select(&self->entryview, index);
	} else {
		instrumententryview_select(&self->entryview, psy_INDEX_INVALID);
	}
}

void instrumentnotemapview_onsampleselected(InstrumentNoteMapView* self,
	SamplesBox* samplebox)
{	
	instrumententryview_setsample(&self->entryview,
		samplesbox_selected(&self->samplesbox));
	psy_ui_component_invalidate(&self->tableview.component);
}

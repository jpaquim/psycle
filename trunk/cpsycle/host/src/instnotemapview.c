// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instnotemapview.h"

#include <math.h>
#include <string.h>

#include "../../detail/portable.h"
#include "../../detail/os.h"

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

static int whitekeytokey(int whitekey)
{
	int octave = whitekey / 7;
	int offset = whitekey % 7;
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
	static int numkey[] = { 0, 2, 4, 5, 7, 9, 11 };
	return octave * 12 + numkey[offset];
}

static int screentokey(int x, double keysize)
{
	int rv;
	int numwhitekey;	

	numwhitekey = (int)(x / keysize);
	rv = whitekeytokey(numwhitekey);
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
	if ((rv % 12) != 4 && (rv % 12) != 11) {
		int delta;

		delta = x - (int)(numwhitekey * keysize);
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
	self->metrics.keysize = 6;
	self->metrics.lineheight = 15;
	psy_ui_component_init(&self->component, parent);
	instrumentkeyboardview_vtable_init(self);
	self->component.vtable = &instrumentkeyboardview_vtable;
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
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;

	tm = psy_ui_component_textmetric(&self->component);	
	size = psy_ui_intsize_init_size(psy_ui_component_size(&self->component),
		&tm);	
	psy_ui_setcolour(g, psy_ui_colour_make(0x00333333));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00333333));
	for (key = keymin; key < keymax; ++key) {
		if (!isblack(key)) {
			psy_ui_Rectangle r;

			psy_ui_setrectangle(&r,
				(int)(cp), 0,
				(int)(self->metrics.keysize + 1), size.height);
			psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00CACACA));
			psy_ui_drawline(g, (int)cp, 0, (int)cp, size.height);
			cp += self->metrics.keysize;			
		}
	}
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00CACACA));
	for (cp = 0, key = keymin; key < keymax; ++key) {							
		if (!isblack(key)) {			
			cp += self->metrics.keysize;
		} else {
			psy_ui_Rectangle r;
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
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;

	numwhitekeys = 0;
	for (key = keymin; key < keymax; ++key) {
		if (!isblack(key)) {
			++numwhitekeys;
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), &tm);
	self->metrics.keysize = size.width / (double)numwhitekeys;
}

// entry view
static void instrumententryview_ondraw(InstrumentEntryView*, psy_ui_Graphics*);
static void instrumententryview_onsize(InstrumentEntryView*, psy_ui_Size*);
static void instrumententryview_onscroll(InstrumentEntryView*, psy_ui_Component* sender);
static void instrumententryview_onmousedown(InstrumentEntryView*,
	psy_ui_MouseEvent*);
static void instrumententryview_onmousemove(InstrumentEntryView*,
	psy_ui_MouseEvent*);
static void instrumententryview_onmouseup(InstrumentEntryView*,
	psy_ui_MouseEvent*);
static float instrumententryview_keysize(InstrumentEntryView*);
static void instrumententryview_updatemetrics(InstrumentEntryView*);

static psy_ui_ComponentVtable instrumententryview_vtable;
static int instrumententryview_vtable_initialized = 0;

static void instrumententryview_vtable_init(InstrumentEntryView* self)
{
	if (!instrumententryview_vtable_initialized) {
		instrumententryview_vtable = *(self->component.vtable);
		instrumententryview_vtable.ondraw = (psy_ui_fp_component_ondraw)
			instrumententryview_ondraw;
		instrumententryview_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			instrumententryview_onmousedown;
		instrumententryview_vtable.onmousemove = (psy_ui_fp_component_onmousemove)
			instrumententryview_onmousemove;
		instrumententryview_vtable.onmouseup = (psy_ui_fp_component_onmouseup)
			instrumententryview_onmouseup;
		instrumententryview_vtable.onsize = (psy_ui_fp_component_onsize)
			instrumententryview_onsize;
		instrumententryview_vtable_initialized = 1;
	}
}

void instrumententryview_init(InstrumentEntryView* self,
	psy_ui_Component* parent, InstrumentParameterView* parameterview)
{
	
	psy_ui_component_init(&self->component, parent);
	instrumententryview_vtable_init(self);
	self->component.vtable = &instrumententryview_vtable;
	self->parameterview = parameterview;
	self->instrument = 0;
	self->metrics.keysize = 8;
	self->metrics.lineheight = 15;
	self->dragmode = 0;
	self->selected = UINTPTR_MAX;
	self->component.scrollstepy = 45;
	psy_ui_component_doublebuffer(&self->component);
	instrumententryview_updatemetrics(self);
	psy_signal_connect(&self->component.signal_scroll, self,
		instrumententryview_onscroll);	
}

void instrumententryview_setinstrument(InstrumentEntryView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	self->selected = UINTPTR_MAX;
	psy_ui_component_setscrolltop(&self->component, 0);
	instrumententryview_adjustscroll(self);
	psy_ui_component_invalidate(&self->component);
}

void instrumententryview_ondraw(InstrumentEntryView* self, psy_ui_Graphics* g)
{	
	if (self->instrument) {
		int cpy;
		psy_List* p;		
		uintptr_t c = 0;		
		psy_ui_TextMetric tm;
		psy_ui_IntSize size;
		int keymin = 0;
		int keymax = psy_audio_NOTECOMMANDS_RELEASE;
		int key;
		int numwhitekeys;

		numwhitekeys = 0;
		for (key = keymin; key < keymax;  ++key) {
			if (!isblack(key)) {
				++numwhitekeys;
			}
		}
		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_intsize_init_size(
			psy_ui_component_size(&self->component), &tm);
		cpy = 0;
		if (self->selected != UINTPTR_MAX && self->instrument) {
			psy_audio_InstrumentEntry* entry;
			int startx;
			int endx;
			psy_ui_Rectangle r;

			entry = (psy_audio_InstrumentEntry*)psy_audio_instrument_entryat(
				self->instrument, self->selected);
			if (entry) {
				startx = (int)(
					(float)numwhitekey(entry->keyrange.low) /
					numwhitekeys * size.width) +
					(int)(isblack(entry->keyrange.low)
						? self->metrics.keysize / 2 : 0);
				endx = (int)(
					(float)numwhitekey(entry->keyrange.low + 1) /
					numwhitekeys * size.width) +
					(int)(isblack(entry->keyrange.low + 1)
						? self->metrics.keysize / 2 : 0);
				psy_ui_setrectangle(&r, startx, 0, endx - startx, size.height);
				psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00272727));
				startx = (int)(
					(float)numwhitekey(entry->keyrange.high) /
					numwhitekeys * size.width) +
					(int)(isblack(entry->keyrange.high)
						? self->metrics.keysize / 2 : 0);
				endx = (int)(
					(float)numwhitekey(entry->keyrange.high + 1) /
					numwhitekeys * size.width) +
					(int)(isblack(entry->keyrange.high + 1)
						? self->metrics.keysize / 2 : 0);
				psy_ui_setrectangle(&r, startx, 0, endx - startx, size.height);
				psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00272727));
			}
		}
		for (p = self->instrument->entries; p != NULL; psy_list_next(&p), ++c) {
			psy_audio_InstrumentEntry* entry;
			int startx;
			int endx;			

			entry = (psy_audio_InstrumentEntry*) p->entry;
			assert(entry);
			startx = (int)(
				(float)numwhitekey(entry->keyrange.low) /
				numwhitekeys * size.width) +
				(int)(isblack(entry->keyrange.low)
					? self->metrics.keysize / 2 : 0);
			endx = (int)(
				(float)numwhitekey(entry->keyrange.high + 1) /
				numwhitekeys * size.width) +
				(int)(isblack(entry->keyrange.high + 1)
					? self->metrics.keysize / 2 : 0) - 1;
			if (c == self->selected) {
				psy_ui_setcolour(g, psy_ui_colour_make(0x00EAEAEA));
			} else {
				psy_ui_setcolour(g, psy_ui_colour_make(0x00CACACA));
			}
			psy_ui_drawline(g, startx, cpy + 5, endx, cpy + 5);
			psy_ui_drawline(g, startx, cpy, startx, cpy + 10);
			psy_ui_drawline(g, endx, cpy, endx, cpy + 10);			
			cpy += self->metrics.lineheight * 3;
		}		
	}
}

void instrumententryview_onsize(InstrumentEntryView* self, psy_ui_Size* size)
{
	instrumententryview_updatemetrics(self);
	instrumententryview_adjustscroll(self);
}

void instrumententryview_updatemetrics(InstrumentEntryView* self)
{
	int keymin = 0;
	int keymax = psy_audio_NOTECOMMANDS_RELEASE;
	int key;
	int numwhitekeys;
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;

	numwhitekeys = 0;
	for (key = keymin; key < keymax; ++key) {
		if (!isblack(key)) {
			++numwhitekeys;
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), &tm);
	self->metrics.keysize = size.width / (double)numwhitekeys;
}

void instrumententryview_onscroll(InstrumentEntryView* self,
	psy_ui_Component* sender)
{
	psy_ui_component_setscrolltop(&self->parameterview->component,
		psy_ui_component_scrolltop(&self->component));
}

void instrumententryview_adjustscroll(InstrumentEntryView* self)
{
	psy_ui_Size size;
	int vscrollmax;
	int numentries;
	int visientries;
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size(&self->component);
	numentries = self->instrument
		? psy_list_size(psy_audio_instrument_entries(self->instrument))
		: 0;	
	visientries = psy_ui_value_px(&size.height, &tm) / (self->metrics.lineheight * 3);
	vscrollmax = numentries < visientries ? 0 : numentries - visientries;
	psy_ui_component_setverticalscrollrange(&self->component, 0, vscrollmax);
}

void instrumententryview_onmousedown(InstrumentEntryView* self,
	psy_ui_MouseEvent* ev)
{
	if (self->instrument) {		
		psy_audio_InstrumentEntry* entry;
							
		if (self->instrument) {
			uintptr_t numentry;	

			numentry = ev->y / (self->metrics.lineheight * 3);
			if (numentry < psy_list_size(
					psy_audio_instrument_entries(self->instrument))) {
				self->selected = numentry;
			} else {
				self->selected = UINTPTR_MAX;
			}
		} else {
			self->selected = UINTPTR_MAX;
		}
		self->dragmode = 1;
		self->currkey = screentokey(ev->x, self->metrics.keysize);
		entry = psy_audio_instrument_entryat(self->instrument, self->selected);
		if (entry) {
			if (abs(entry->keyrange.low  - screentokey(ev->x,
					self->metrics.keysize)) <
				abs(entry->keyrange.high  - screentokey(ev->x,
					self->metrics.keysize))) {
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

float instrumententryview_keysize(InstrumentEntryView* self)
{	
	int key;
	int keymin = 0;
	int keymax = psy_audio_NOTECOMMANDS_RELEASE - 1;
	int numwhitekeys;
	psy_ui_TextMetric tm;
	psy_ui_Size size;

	numwhitekeys = 0;
	for (key = keymin; key < keymax; ++key) {
		if (!isblack(key)) {
			++numwhitekeys;
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size(&self->component);
	return psy_ui_value_px(&size.width, &tm) / (float)numwhitekeys;
}

void instrumententryview_onmousemove(InstrumentEntryView* self,
	psy_ui_MouseEvent* ev)
{	
	bool showresizecursor;

	showresizecursor = FALSE;
	self->currkey = screentokey(ev->x, self->metrics.keysize);
	if (self->dragmode != INSTVIEW_DRAG_NONE && self->instrument) {
		psy_audio_InstrumentEntry* entry;		
				
		entry = psy_audio_instrument_entryat(self->instrument, self->selected);
		if (entry) {			
			if (self->dragmode == INSTVIEW_DRAG_LEFT) {
				entry->keyrange.low = screentokey(ev->x,
					self->metrics.keysize);
				if (entry->keyrange.low > entry->keyrange.high) {
					entry->keyrange.low = entry->keyrange.high;
				}
			} else {
				entry->keyrange.high = screentokey(ev->x,
					self->metrics.keysize);
				if (entry->keyrange.high < entry->keyrange.low) {
					entry->keyrange.high = entry->keyrange.low;
				}
			}
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_invalidate(&self->parameterview->component);
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
	}
}

// ParameterView
static void instrumentparameterview_ondraw(InstrumentParameterView*,
	psy_ui_Graphics*);

static psy_ui_ComponentVtable instrumentparameterview_vtable;
static int instrumentparameterview_vtable_initialized = 0;

static void instrumentparameterview_vtable_init(InstrumentParameterView* self)
{
	if (!instrumentparameterview_vtable_initialized) {
		instrumentparameterview_vtable = *(self->component.vtable);
		instrumentparameterview_vtable.ondraw = (psy_ui_fp_component_ondraw)
			instrumentparameterview_ondraw;		
		instrumentparameterview_vtable_initialized = 1;
	}
}

void instrumentparameterview_init(InstrumentParameterView* self,
	psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	instrumentparameterview_vtable_init(self);
	self->component.vtable = &instrumentparameterview_vtable;
	self->instrument = NULL;
	psy_ui_component_doublebuffer(&self->component);		
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(20),
			psy_ui_value_makeeh(20)));
}

void instrumentparameterview_setinstrument(InstrumentParameterView* self,
		psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	psy_ui_component_setscrolltop(&self->component, 0);
	psy_ui_component_invalidate(&self->component);
}

void instrumentparameterview_ondraw(InstrumentParameterView* self,
	psy_ui_Graphics* g)
{	
	if (self->instrument) {
		int cpy = 0;
		psy_List* p;

		psy_ui_setcolour(g, psy_ui_colour_make(0x00CACACA));
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00CACACA));
		for (p = self->instrument->entries; p != NULL; p = p->next) {
			psy_audio_InstrumentEntry* entry;
			char text[40];

			entry = (psy_audio_InstrumentEntry*) p->entry;
			psy_snprintf(text, 40, "Sample %0X:%0X", entry->sampleindex.slot,
				entry->sampleindex.subslot);
			psy_ui_textout(g, 0, cpy, text, strlen(text));
			psy_snprintf(text, 40, "Notes %.3d - %.3d", entry->keyrange.low,
				entry->keyrange.high);
			psy_ui_textout(g, 0, cpy + self->metrics.lineheight, text,
				strlen(text));
			psy_snprintf(text, 40, "Volume %.2X - %.2X", entry->velocityrange.low,
				entry->velocityrange.high);
			psy_ui_textout(g, 0, cpy + self->metrics.lineheight * 2, text,
				strlen(text));
			cpy += (self->metrics.lineheight * 3);
		}
	}
}

void instrumentnotemapbuttons_init(InstrumentNoteMapButtons* self,
	psy_ui_Component* parent)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(1.5), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->component, parent);	
	psy_ui_button_init(&self->add, &self->component);
	psy_ui_button_settext(&self->add, "Add");
	psy_ui_button_init(&self->remove, &self->component);
	psy_ui_button_settext(&self->remove, "Remove");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
			&margin));
}

// InstrumentNoteMapView
static void instrumentnotemapview_setmetrics(InstrumentNoteMapView*,
	InstrumentNoteMapMetrics);

void instrumentnotemapview_init(InstrumentNoteMapView* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent);	
	self->metrics.keysize = 8;
	self->metrics.lineheight = 15;
	psy_ui_label_init(&self->label, &self->component);
	psy_ui_label_settext(&self->label, "Notemap");
	psy_ui_component_setalign(&self->label.component, psy_ui_ALIGN_TOP);
	samplesbox_init(&self->samplesbox, &self->component, NULL, workspace);
	psy_ui_component_setalign(&self->samplesbox.component, psy_ui_ALIGN_LEFT);	
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(1.5),
		psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->label.component, &margin);
	instrumentnotemapbuttons_init(&self->buttons, &self->component);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->buttons.component, &margin);
	instrumentparameterview_init(&self->parameterview, &self->component);
	psy_ui_component_setalign(&self->parameterview.component, psy_ui_ALIGN_LEFT);
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(0), psy_ui_value_makeew(0),
		psy_ui_value_makepx(0), psy_ui_value_makeew(2));
	psy_ui_component_setmargin(&self->parameterview.component, &margin);
	instrumententryview_init(&self->entryview,
		&self->component, &self->parameterview);
#ifdef DIVERSALIS__OS__MICROSOFT
	psy_ui_scroller_init(&self->scroller, &self->entryview.component,
		&self->component);
	psy_ui_component_setoverflow(&self->entryview.component, psy_ui_OVERFLOW_VSCROLL);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
#else
	// todo: scroller here slows X11 down
	psy_ui_component_setalign(&self->entryview.component, psy_ui_ALIGN_CLIENT);
#endif
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(0), psy_ui_value_makeew(2),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->entryview.component, &margin);	
	instrumentkeyboardview_init(&self->keyboard, &self->component);
	psy_ui_component_setalign(&self->keyboard.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setmargin(&self->keyboard.component, &margin);
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(0), psy_ui_value_makeew(2),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->keyboard.component, &margin);
	instrumentnotemapview_setmetrics(self, self->metrics);
}

void instrumentnotemapview_setinstrument(InstrumentNoteMapView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	instrumententryview_setinstrument(&self->entryview, instrument);
	instrumentparameterview_setinstrument(&self->parameterview, instrument);
}

void instrumentnotemapview_setmetrics(InstrumentNoteMapView* self,
	InstrumentNoteMapMetrics metrics)
{
	self->metrics = metrics;
	self->entryview.metrics = metrics;
	self->parameterview.metrics = metrics;
	self->keyboard.metrics = metrics;
}

void instrumentnotemapview_update(InstrumentNoteMapView* self)
{
	instrumententryview_adjustscroll(&self->entryview);
	psy_ui_component_invalidate(&self->component);
}

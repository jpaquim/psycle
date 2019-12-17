// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instnotemapview.h"
#include <portable.h>

static void instrumententryview_init(InstrumentEntryView*, ui_component* parent,
	InstrumentParameterView*);
static void instrumententryview_setinstrument(InstrumentEntryView*, Instrument*);
static void instrumentnotemapview_setmetrics(InstrumentNoteMapView*,
	InstrumentNoteMapMetrics);
static void instrumentparameterview_setinstrument(InstrumentParameterView*,
	Instrument*);
static void instrumententryview_ondraw(InstrumentEntryView*, ui_component* sender,
	ui_graphics* g);
static void instrumententryview_onmousedown(InstrumentEntryView*,
	ui_component* sender, MouseEvent*);
static void instrumententryview_onmousemove(InstrumentEntryView*,
	ui_component* sender, MouseEvent*);
static void instrumententryview_onmouseup(InstrumentEntryView*,
	ui_component* sender, MouseEvent*);
static int isblack(int key);
static int whitekeytokey(int whitekey);
static int screentokey(int x, int keysize);
// instrument parameter view
static void instrumentparameterview_init(InstrumentParameterView*,
	ui_component* parent);
static void instrumentparameterview_ondraw(InstrumentParameterView*,
	ui_component* sender, ui_graphics*);
static void instrumentnotemapbuttons_init(InstrumentNoteMapButtons*,
	ui_component* parent);
// keyboardview
static void instrumentkeyboardview_ondraw(InstrumentKeyboardView*,
	ui_component* sender, ui_graphics*);

void instrumentnotemapview_init(InstrumentNoteMapView* self,
	ui_component* parent)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makepx(0),
	ui_value_makeeh(1.5), ui_value_makepx(0));
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	self->metrics.keysize = 6;
	self->metrics.lineheight = 15;
	ui_label_init(&self->label, &self->component);
	ui_label_settext(&self->label, "Notemap");
	ui_component_setalign(&self->label.component, UI_ALIGN_TOP);
	ui_component_setmargin(&self->label.component, &margin);
	instrumentnotemapbuttons_init(&self->buttons, &self->component);
	ui_component_setalign(&self->buttons.component, UI_ALIGN_TOP);
	ui_component_setmargin(&self->buttons.component, &margin);
	instrumentparameterview_init(&self->parameterview, &self->component);
	ui_component_setalign(&self->parameterview.component, UI_ALIGN_LEFT);	
	instrumententryview_init(&self->entryview,
		&self->component, &self->parameterview);	
	ui_component_setalign(&self->entryview.component, UI_ALIGN_TOP);
	instrumentkeyboardview_init(&self->keyboard, &self->component);
	ui_component_setalign(&self->keyboard.component, UI_ALIGN_BOTTOM);
	instrumentnotemapview_setmetrics(self, self->metrics);
}

void instrumentnotemapview_setinstrument(InstrumentNoteMapView* self,
	Instrument* instrument)
{
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

void instrumentkeyboardview_init(InstrumentKeyboardView* self,
	ui_component* parent)
{
	self->dy = 0;
	self->metrics.keysize = 6;
	self->metrics.lineheight = 15;
	ui_component_init(&self->component, parent);
	ui_component_resize(&self->component, 0, 40);
	psy_signal_connect(&self->component.signal_draw, self,
		instrumentkeyboardview_ondraw);
}

int isblack(int key)
{
	int offset = key % 12;

	return (offset == 1 || offset == 3 || offset == 6 || offset == 8 
		|| offset == 10);
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
}

void instrumentkeyboardview_ondraw(InstrumentKeyboardView* self, ui_component* sender, ui_graphics* g)
{		
	int keymin = 0;
	int keymax = NOTECOMMANDS_RELEASE;
	int key;	
	int keyboardsize;
	int keysize = self->metrics.keysize;
	int cp = 0;
	float top = 0.50;
	float bottom = 1 - top;
	ui_textmetric tm;
	ui_size size;

	tm = ui_component_textmetric(&self->component);
	keyboardsize = (keymax - keymin) * keysize;
	size = ui_component_size(&self->component);
	ui_setcolor(g, 0x00333333);

	ui_setbackgroundmode(g, TRANSPARENT);
	ui_settextcolor(g, 0x00333333);
	for (key = keymin; key < keymax; ++key) {					
		ui_drawline(g, cp, 0, cp, size.height);
		if (!isblack(key)) {
			ui_rectangle r;
			ui_setrectangle(&r, cp, 0, keysize, size.height);
			ui_drawsolidrectangle(g, r, 0x00CACACA);
			ui_drawline(g, cp, 0, cp, size.height);			
			cp += keysize;			
			ui_drawline(g, cp, 0, cp, size.height);			
		}
	}
	ui_settextcolor(g, 0x00CACACA);
	for (cp = 0, key = keymin; key < keymax; ++key) {							
		if (!isblack(key)) {			
			cp += keysize;			
		} else {
			ui_rectangle r;
			int x;
			int width;

			x = cp - (int)(keysize * 0.8 / 2);
			width = (int)(keysize * 0.8);
			ui_setrectangle(&r, x, 0, width, (int)(size.height * top));
			ui_drawsolidrectangle(g, r, 0x00444444);			
		}
	}
}

// entry view
void instrumententryview_init(InstrumentEntryView* self,
	ui_component* parent, InstrumentParameterView* parameterview)
{
	self->parameterview = parameterview;
	self->instrument = 0;
	self->dy = 0;
	self->metrics.keysize = 8;
	self->metrics.lineheight = 15;
	self->dragmode = 0;
	self->selected = NOINSTRUMENT_INDEX;
	ui_component_init(&self->component, parent);	
	ui_component_resize(&self->component, 0, 40);
	psy_signal_connect(&self->component.signal_draw, self,
		instrumententryview_ondraw);
	psy_signal_connect(&self->component.signal_mousedown, self,
		instrumententryview_onmousedown);
	psy_signal_connect(&self->component.signal_mousemove, self,
		instrumententryview_onmousemove);
	psy_signal_connect(&self->component.signal_mouseup, self,
		instrumententryview_onmouseup);
	ui_component_resize(&self->component, 100, 200);
}

void instrumententryview_setinstrument(InstrumentEntryView* self, Instrument* instrument)
{
	self->instrument = instrument;
	ui_component_invalidate(&self->component);
}

int numwhitekey(int key)
{
	int octave = key / 12;
	int offset = key % 12;
	int c = 0;
	int i;

	for (i = 0; i < offset; ++i) {
		if (!isblack(i)) ++c;
	}
	return octave * 7 + c;
}

int whitekeytokey(int whitekey)
{
	int octave = whitekey / 7;
	int offset = whitekey % 7;
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
	static int numkey[] = { 0, 2, 4, 5, 7, 9, 11 };	
	return octave * 12 + numkey[offset];	
}

void instrumententryview_ondraw(InstrumentEntryView* self, ui_component* sender, ui_graphics* g)
{	
	if (self->instrument) {
		int cpy = 0;
		List* p;
		ui_size size;
		int keysize = self->metrics.keysize;
		uintptr_t c = 0;
		int width = keysize * numwhitekey(NOTECOMMANDS_RELEASE);		

		size = ui_component_size(&self->component);		
		for (p = self->instrument->entries; p != 0; p = p->next, ++c) {
			InstrumentEntry* entry;
			int startx;
			int endx;

			entry = (InstrumentEntry*) p->entry;
			startx = (int)(
				(float) numwhitekey(entry->keyrange.low) / 
					numwhitekey(NOTECOMMANDS_RELEASE) * width);
			endx = (int)(
				(float)numwhitekey(entry->keyrange.high) / 
					numwhitekey(NOTECOMMANDS_RELEASE) * width);
			if (c == self->selected) {
				ui_setcolor(g, 0x00EAEAEA);
			} else {
				ui_setcolor(g, 0x00CACACA);
			}
			ui_drawline(g, startx, cpy + 5, endx, cpy + 5);
			ui_drawline(g, startx, cpy, startx, cpy + 10);
			ui_drawline(g, endx, cpy, endx, cpy + 10);
			cpy += self->metrics.lineheight * 3;
		}
	}
}

void instrumententryview_onmousedown(InstrumentEntryView* self,
	ui_component* sender, MouseEvent* ev)
{
	InstrumentEntry* entry;
	if (self->instrument) {
		uintptr_t numentry;	

		numentry = ev->y / (self->metrics.lineheight * 3);
		if (numentry < list_size(instrument_entries(self->instrument))) {
			self->selected = numentry;
		} else {
			self->selected = NOINSTRUMENT_INDEX;
		}
	} else {
		self->selected = NOINSTRUMENT_INDEX;
	}
	self->dragmode = 1;
		
	entry = instrument_entryat(self->instrument, self->selected);
	if (entry) {
		if (abs(entry->keyrange.low  - screentokey(ev->x,
				self->metrics.keysize)) < 
			abs(entry->keyrange.high  - screentokey(ev->x,
				self->metrics.keysize))) {
			self->dragmode = INSTVIEW_DRAG_LEFT;
		} else {
			self->dragmode = INSTVIEW_DRAG_RIGHT;
		}
		ui_component_capture(&self->component);
	}
	ui_component_invalidate(&self->component);
}

void instrumententryview_onmousemove(InstrumentEntryView* self,
	ui_component* sender, MouseEvent* ev)
{
	if (self->dragmode != INSTVIEW_DRAG_NONE && self->instrument) {
		InstrumentEntry* entry;
		
		entry = instrument_entryat(self->instrument, self->selected);
		if (entry) {
			if (self->dragmode == INSTVIEW_DRAG_LEFT) {
				entry->keyrange.low = screentokey(ev->x,
					self->metrics.keysize);
			} else {
				entry->keyrange.high = screentokey(ev->x,
					self->metrics.keysize);
			}
			ui_component_invalidate(&self->component);
			ui_component_invalidate(&self->parameterview->component);
		}
	}
}

void instrumententryview_onmouseup(InstrumentEntryView* self,
	ui_component* sender, MouseEvent* ev)
{
	self->dragmode = 0;
	ui_component_releasecapture(&self->component);
}

int screentokey(int x, int keysize)
{		
	return whitekeytokey(x / keysize);
}

void instrumentparameterview_init(InstrumentParameterView* self,
	ui_component* parent)
{
	self->instrument = 0;
	self->dy = 0;
	ui_component_init(&self->component, parent);	
	ui_component_resize(&self->component, 0, 40);
	psy_signal_connect(&self->component.signal_draw, self,
		instrumentparameterview_ondraw);
	ui_component_resize(&self->component, 100, 200);
}

void instrumentparameterview_setinstrument(InstrumentParameterView* self,
		Instrument* instrument)
{
	self->instrument = instrument;
	ui_component_invalidate(&self->component);
}

void instrumentparameterview_ondraw(InstrumentParameterView* self,
	ui_component* sender, ui_graphics* g)
{	
	if (self->instrument) {
		int cpy = 0;
		List* p;
		ui_size size;
		int keysize = self->metrics.keysize;
		int width = keysize * numwhitekey(NOTECOMMANDS_RELEASE);		

		size = ui_component_size(&self->component);
		ui_setcolor(g, 0x00CACACA);
		ui_setbackgroundmode(g, TRANSPARENT);
		ui_settextcolor(g, 0x00CACACA);
		for (p = self->instrument->entries; p != 0; p = p->next) {
			InstrumentEntry* entry;
			char text[40];

			entry = (InstrumentEntry*) p->entry;
			psy_snprintf(text, 40, "Sample %0X:%0X", entry->sampleindex.slot,
				entry->sampleindex.subslot);
			ui_textout(g, 0, cpy, text, strlen(text));
			psy_snprintf(text, 40, "Notes %.3d - %.3d", entry->keyrange.low,
				entry->keyrange.high);
			ui_textout(g, 0, cpy + self->metrics.lineheight, text,
				strlen(text));
			psy_snprintf(text, 40, "Volume %.2X - %.2X", entry->velocityrange.low,
				entry->velocityrange.high);
			ui_textout(g, 0, cpy + self->metrics.lineheight * 2, text,
				strlen(text));
			cpy += (self->metrics.lineheight * 3);
		}
	}
}

void instrumentnotemapbuttons_init(InstrumentNoteMapButtons* self,
	ui_component* parent)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(1.5),
	ui_value_makepx(0), ui_value_makepx(0));
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_button_init(&self->add, &self->component);
	ui_button_settext(&self->add, "Add");
	ui_button_init(&self->remove, &self->component);
	ui_button_settext(&self->remove, "Remove");
	list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT,
			&margin));
}

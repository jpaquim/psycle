/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
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

/* keyboardview */
/* prototypes */
static void instrumentkeyboardview_ondraw(InstrumentKeyboardView*,
	psy_ui_Graphics*);
static void instrumentkeyboardview_onsize(InstrumentKeyboardView*);
static void instrumentkeyboardview_updatemetrics(InstrumentKeyboardView*);
static void instrumentkeyboardview_drawwhitekeys(InstrumentKeyboardView*,
	psy_ui_Graphics*);
static void instrumentkeyboardview_drawblackkeys(InstrumentKeyboardView*,
	psy_ui_Graphics*);
/* vtable */
static psy_ui_ComponentVtable instrumentkeyboardview_vtable;
static bool instrumentkeyboardview_vtable_initialized = FALSE;

static void instrumentkeyboardview_vtable_init(InstrumentKeyboardView* self)
{
	if (!instrumentkeyboardview_vtable_initialized) {
		instrumentkeyboardview_vtable = *(self->component.vtable);
		instrumentkeyboardview_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			instrumentkeyboardview_ondraw;
		instrumentkeyboardview_vtable.onsize =
			(psy_ui_fp_component_event)
			instrumentkeyboardview_onsize;
		instrumentkeyboardview_vtable_initialized = TRUE;
	}
	self->component.vtable = &instrumentkeyboardview_vtable;
}

/* implementation */
void instrumentkeyboardview_init(InstrumentKeyboardView* self,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	instrumentkeyboardview_vtable_init(self);
	psy_ui_component_doublebuffer(&self->component);
	self->metrics.keysize = 6;
	self->metrics.lineheight = 15;
	psy_audio_instrumententry_init(&self->entry);
	self->entry.keyrange.low = 255;
	self->entry.keyrange.high = 255;
	psy_ui_component_preventalign(&self->component);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(0.0, 2.0));	
}

void instrumentkeyboardview_ondraw(InstrumentKeyboardView* self,
	psy_ui_Graphics* g)
{				
	instrumentkeyboardview_drawwhitekeys(self, g);	
	instrumentkeyboardview_drawblackkeys(self, g);	
}

void instrumentkeyboardview_drawwhitekeys(InstrumentKeyboardView* self,
	psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	int keymin = 0;
	int keymax = psy_audio_NOTECOMMANDS_RELEASE;
	int key;
	double cp = 0;
	double top = 0.60;
	double bottom = 1 - top;		
	
	size = psy_ui_component_size_px(&self->component);
	psy_ui_setcolour(g, psy_ui_colour_make(0x00333333));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00333333));	
	for (key = keymin; key < keymax; ++key) {
		if (!psy_dsp_isblack(key)) {
			psy_ui_RealRectangle r;
			psy_ui_Colour colour;

			psy_ui_setrectangle(&r,
				cp, 0,
				(self->metrics.keysize + 1), size.height);
			if (psy_audio_parameterrange_intersect(&self->entry.keyrange, key)) {
				colour = psy_ui_colour_make(0x00FF2288);
			} else {
				colour = psy_ui_colour_make(0x00CACACA);
			}
			psy_ui_drawsolidrectangle(g, r, colour);
			psy_ui_drawline(g, psy_ui_realpoint_make(cp, 0),
				psy_ui_realpoint_make(cp, size.height));
			cp += self->metrics.keysize;			
		}
	}
}

void instrumentkeyboardview_drawblackkeys(InstrumentKeyboardView* self,
	psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	int keymin = 0;
	int keymax = psy_audio_NOTECOMMANDS_RELEASE;
	int key;
	double cp = 0;
	double top = 0.60;
	double bottom = 1 - top;		
	
	size = psy_ui_component_size_px(&self->component);
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00CACACA));
	for (cp = 0, key = keymin; key < keymax; ++key) {							
		if (!psy_dsp_isblack(key)) {
			cp += self->metrics.keysize;
		} else {
			psy_ui_RealRectangle r;
			int x;
			int width;
			psy_ui_Colour colour;

			x = (int)cp - (int)(self->metrics.keysize * 0.68 / 2);
			width = (int)(self->metrics.keysize * 0.68);
			psy_ui_setrectangle(&r, x, 0, width, (int)(size.height * top));
			if (psy_audio_parameterrange_intersect(&self->entry.keyrange,
					key)) {
				colour = psy_ui_colour_make(0x00FF2288);
				colour = psy_ui_colour_weighted(&colour, 800);
			} else {
				colour = psy_ui_colour_make(0x00444444);
			}
			psy_ui_drawsolidrectangle(g, r, colour);
		}
	}
}

void instrumentkeyboardview_onsize(InstrumentKeyboardView* self)
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
		if (!psy_dsp_isblack(key)) {
			++numwhitekeys;
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_scrollsize(&self->component), tm, NULL);
	self->metrics.keysize = size.width / (double)numwhitekeys;
	psy_ui_component_setscrollstep_height(&self->component,
		psy_ui_value_make_px(self->metrics.lineheight * 3));
}

/* InstrumentEntryView */
/* prototypes */
static void instrumententryview_ondestroy(InstrumentEntryView*);
static void instrumententryview_onpreferredsize(InstrumentEntryView*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void instrumententryview_ondraw(InstrumentEntryView*, psy_ui_Graphics*);
static void instrumententryview_onsize(InstrumentEntryView*);
static void instrumententryview_onmousedown(InstrumentEntryView*,
	psy_ui_MouseEvent*);
static void instrumententryview_onmousemove(InstrumentEntryView*,
	psy_ui_MouseEvent*);
static void instrumententryview_onmouseup(InstrumentEntryView*,
	psy_ui_MouseEvent*);
static double instrumententryview_keysize(InstrumentEntryView*);
static void instrumententryview_updatemetrics(InstrumentEntryView*);
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
		instrumententryview_vtable.ondestroy =
			(psy_ui_fp_component_event)
			instrumententryview_ondestroy;
		instrumententryview_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			instrumententryview_ondraw;
		instrumententryview_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			instrumententryview_onmousedown;
		instrumententryview_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			instrumententryview_onmousemove;
		instrumententryview_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			instrumententryview_onmouseup;
		instrumententryview_vtable.onsize =
			(psy_ui_fp_component_event)
			instrumententryview_onsize;
		instrumententryview_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			instrumententryview_onpreferredsize;
		instrumententryview_vtable_initialized = TRUE;
	}
}
/* implementation */
void instrumententryview_init(InstrumentEntryView* self,
	psy_ui_Component* parent, InstrumentEntryState* state)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	instrumententryview_vtable_init(self);
	self->component.vtable = &instrumententryview_vtable;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 1);	
	psy_ui_component_setscrollstep(&self->component,
		psy_ui_size_make_em(0.0, 1.0));
	self->instrument = 0;
	self->metrics.keysize = 8;
	self->metrics.lineheight = 15;
	self->dragmode = 0;	
	self->state = state;
	psy_ui_component_setscrollstep_height(&self->component,
		psy_ui_value_make_px(45));
	instrumententryview_updatemetrics(self);	
	psy_signal_connect(&self->state->signal_select, self,
		instrumententryview_onentryselected);
	psy_signal_connect(&self->state->signal_entrychanged, self,
		instrumententryview_onentryupdate);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
}

void instrumententryview_ondestroy(InstrumentEntryView* self)
{	
	psy_signal_disconnect(&self->state->signal_select, self,
		instrumententryview_onentryselected);
	psy_signal_disconnect(&self->state->signal_select, self,
		instrumententryview_onentryupdate);
}

void instrumententryview_setinstrument(InstrumentEntryView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
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
			if (!psy_dsp_isblack(key)) {
				++numwhitekeys;
			}
		}
		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_component_size_px(&self->component);
		cpy = 0;
		if (self->state->selectedentry && self->instrument &&
				self->instrument->entries) {
			psy_audio_InstrumentEntry* entry;
			double keylo_startx;
			double keylo_endx;
			double keyhi_startx;
			double keyhi_endx;
			psy_ui_RealRectangle r;
			double scrollleft;
			double scrolltop;

			scrollleft = psy_ui_component_scrollleftpx(&self->component);
			scrolltop = psy_ui_component_scrolltop_px(&self->component);
			entry = self->state->selectedentry;
			if (entry) {
				keylo_startx = (int)(
					(float)numwhitekey((uint8_t)entry->keyrange.low) /
					numwhitekeys * size.width) +
					(int)(psy_dsp_isblack((uint8_t)entry->keyrange.low)
						? self->metrics.keysize / 2 : 0);
				keylo_endx = (int)(
					(float)numwhitekey((uint8_t)entry->keyrange.low + 1) /
					numwhitekeys * size.width) +
					(int)(psy_dsp_isblack((uint8_t)entry->keyrange.low + 1)
						? self->metrics.keysize / 2 : 0);								
				keyhi_startx = (int)(
					(float)numwhitekey((uint8_t)entry->keyrange.high) /
					numwhitekeys * size.width) +
					(int)(psy_dsp_isblack((uint8_t)entry->keyrange.high)
						? self->metrics.keysize / 2 : 0);
				keyhi_endx = (int)(
					(float)numwhitekey((uint8_t)entry->keyrange.high + 1) /
					numwhitekeys * size.width) +
					(int)(psy_dsp_isblack((uint8_t)entry->keyrange.high + 1)
						? self->metrics.keysize / 2 : 0);
				psy_ui_setrectangle(&r,
					keylo_startx, scrolltop,
					keyhi_endx - keylo_startx, scrolltop + size.height);
				psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00333333));				
			}
		}
		for (p = self->instrument->entries; p != NULL; psy_list_next(&p), ++c) {
			psy_audio_InstrumentEntry* entry;
			int startx;
			int endx;
			psy_ui_RealRectangle r;
			psy_ui_Colour colour;

			entry = (psy_audio_InstrumentEntry*) p->entry;
			assert(entry);
			startx = (int)(
				(float)numwhitekey((uint8_t)entry->keyrange.low) /
				numwhitekeys * size.width) +
				(int)(psy_dsp_isblack((uint8_t)entry->keyrange.low)
					? self->metrics.keysize / 2 : 0);
			endx = (int)(
				(float)numwhitekey((uint8_t)entry->keyrange.high + 1) /
				numwhitekeys * size.width) +
				(int)(psy_dsp_isblack((uint8_t)entry->keyrange.high + 1)
					? self->metrics.keysize / 2 : 0) - 1;
			if (entry == self->state->selectedentry) {
				colour = psy_ui_colour_make(0x00FF2288);
			} else {
				colour = psy_ui_colour_make(0x00545454);
			}
			psy_ui_realrectangle_init_all(&r,
				psy_ui_realpoint_make(startx, cpy),
				psy_ui_realsize_make(endx - startx, 5));
			psy_ui_drawsolidrectangle(g, r, colour);			
			cpy += self->metrics.lineheight;
		}		
		if (!self->instrument->entries) {
			static const char* nomapping = "No Instrument Mapping";

			psy_ui_textout(g, 
				(size.width - tm->tmAveCharWidth * psy_strlen(nomapping)) / 2,
				(size.height - tm->tmHeight) / 2,
				nomapping, psy_strlen(nomapping));
		}
	} else {
		const psy_ui_TextMetric* tm;
		psy_ui_RealSize size;
		static const char* noinst = "No Instrument";

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_component_size_px(&self->component);
		psy_ui_textout(g,
			(size.width - tm->tmAveCharWidth * psy_strlen(noinst)) / 2,
			(size.height - tm->tmHeight) / 2,
			noinst, psy_strlen(noinst));
	}
}

void instrumententryview_onsize(InstrumentEntryView* self)
{
	instrumententryview_updatemetrics(self);
}

void instrumententryview_onpreferredsize(InstrumentEntryView* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (self->instrument && self->instrument->entries) {		
		if (limit) {
			rv->width = limit->width;
		}
		rv->height = psy_ui_value_make_px(
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
		if (!psy_dsp_isblack(key)) {
			++numwhitekeys;
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_scrollsize(&self->component), tm, NULL);
	self->metrics.keysize = size.width / (double)numwhitekeys;
	psy_ui_component_setscrollstep_height(&self->component,
		psy_ui_value_make_px(self->metrics.lineheight * 3));
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
				entry = psy_audio_instrument_entryat(self->instrument,
					numentry);
				instrumententrystate_selectentry(self->state, entry);				
			} else {
				instrumententrystate_selectentry(self->state, NULL);
			}
		} else {
			instrumententrystate_selectentry(self->state, NULL);
		}
		self->dragmode = 1;
		self->currkey = screentokey(ev->pt.x, self->metrics.keysize);
		instrumententryview_outputstatus(self, (uint8_t)self->currkey);
		entry = self->state->selectedentry;
		if (entry) {			
			if (abs((int)(entry->keyrange.low  - screentokey(ev->pt.x,
					self->metrics.keysize))) <
				abs((int)(entry->keyrange.high  - screentokey(ev->pt.x,
					self->metrics.keysize)))) {
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
		if (!psy_dsp_isblack(key)) {
			++numwhitekeys;
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_scrollsize(&self->component);
	return psy_ui_value_px(&size.width, tm, NULL) / numwhitekeys;
}

void instrumententryview_onmousemove(InstrumentEntryView* self,
	psy_ui_MouseEvent* ev)
{	
	bool showresizecursor;

	showresizecursor = FALSE;
	self->currkey = screentokey(ev->pt.x, self->metrics.keysize);
	if (self->dragmode != INSTVIEW_DRAG_NONE && self->instrument) {
		psy_audio_InstrumentEntry* entry;
		uintptr_t screenkey;
				
		screenkey = screentokey(ev->pt.x, self->metrics.keysize);
		entry = self->state->selectedentry;
		if (entry) {			
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
				instrumententryview_outputstatus(self,
					(uint8_t)entry->keyrange.low);
			} else {
				entry->keyrange.high = screenkey;
				entry->keyrange.high = psy_min(entry->keyrange.max,
					entry->keyrange.high);
				if (entry->keyrange.high < entry->keyrange.low) {
					entry->keyrange.high = entry->keyrange.low;
				}
				instrumententryview_outputstatus(self,
					(uint8_t)entry->keyrange.high);
			}
			instrumententrystate_updateentry(self->state, entry);			
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

void instrumententryview_outputstatus(InstrumentEntryView* self, uint8_t key)
{	
	char text[64];
	const char* notestr;
	const char* keydesc;

	keydesc = psy_ui_translate("instrumentview.key");
	if (!keydesc) {
		keydesc = "Key";
	}
	notestr = psy_dsp_notetostr(key,
		workspace_notetabmode(self->state->workspace));
	if (notestr) {				
		psy_snprintf(text, 64, "%s %s", keydesc, notestr);		
	} else {
		psy_snprintf(text, 64, "%s %d", keydesc, (int)key);
	}
	workspace_outputstatus(self->state->workspace, text);
}

/* Table */
/* TableColumn */
void instrumententrytablecolumn_init(InstrumentEntryTableColumn* self,
	const char* label)
{	
	self->width = psy_ui_value_make_ew(COLWIDTH);
	self->label = psy_strdup(label);
	self->translation = NULL;
}

void instrumententrytablecolumn_dispose(InstrumentEntryTableColumn* self)
{
	free(self->label);
	self->label = NULL;
	free(self->translation);
	self->translation = NULL;
}

InstrumentEntryTableColumn* instrumententrytablecolumn_alloc(void)
{
	return (InstrumentEntryTableColumn*)
		malloc(sizeof(InstrumentEntryTableColumn));
}

InstrumentEntryTableColumn* instrumententrytablecolumn_allocinit(
	const char* label)
{
	InstrumentEntryTableColumn* rv;
	
	rv = instrumententrytablecolumn_alloc();
	if (rv) {
		instrumententrytablecolumn_init(rv, label);
	}
	return rv;
}

void instrumententrytablecolumn_settranslation(InstrumentEntryTableColumn* self,
	const char* text)
{
	psy_strreset(&self->translation, text);
}

/* InstrumentEntryState */
/* implementation */
void instrumententrystate_init(InstrumentEntryState* self, Workspace* workspace)
{
	InstrumentEntryTableColumn* column;				

	self->workspace = workspace;
	psy_signal_init(&self->signal_select);
	psy_signal_init(&self->signal_entrychanged);
	psy_table_init(&self->columns);
	/* sample index */
	column = instrumententrytablecolumn_allocinit("instrumentview.smplidx");
	psy_table_insert(&self->columns, 0, column);
	/* key low */
	column = instrumententrytablecolumn_allocinit("instrumentview.keylo");
	psy_table_insert(&self->columns, 1, column);
	/* key hi */
	column = instrumententrytablecolumn_allocinit("instrumentview.keyhi");
	psy_table_insert(&self->columns, 2, column);
	/* fixedkey */
	column = instrumententrytablecolumn_allocinit("instrumentview.keyfixed");
	psy_table_insert(&self->columns, 3, column);
	self->curredit = NULL;
	self->editcolumn = psy_INDEX_INVALID;
}

void instrumententrystate_dispose(InstrumentEntryState* self)
{
	psy_signal_dispose(&self->signal_select);
	psy_signal_dispose(&self->signal_entrychanged);
	psy_table_disposeall(&self->columns, (psy_fp_disposefunc)
		instrumententrytablecolumn_dispose);
}

InstrumentEntryTableColumn* instrumententrystate_at(
	InstrumentEntryState* self, uintptr_t column)
{
	return (InstrumentEntryTableColumn*)psy_table_at(&self->columns, column);

}

void instrumententrystate_translate(InstrumentEntryState* self,
	psy_Translator* translator)
{
	psy_TableIterator it;

	assert(self);

	for (it = psy_table_begin(&self->columns);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		InstrumentEntryTableColumn* column;

		column = (InstrumentEntryTableColumn*)psy_tableiterator_value(&it);
		instrumententrytablecolumn_settranslation(column,
			psy_translator_translate(translator, column->label));
	}
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

double instrumententrystate_columnpx(
	InstrumentEntryState* self, uintptr_t column,
	const psy_ui_TextMetric* tm)
{
	if (psy_table_size(&self->columns) > 0) {
		uintptr_t i;
		uintptr_t numcolumns;
		double rv;

		numcolumns = psy_table_maxkey(&self->columns);
		rv = 0.0;
		for (i = 0; i < psy_min(numcolumns, column); ++i) {
			InstrumentEntryTableColumn* column;

			column = instrumententrystate_at(self, i);
			if (column) {
				rv += psy_ui_value_px(&column->width, tm, NULL);
			}
		}
		return rv;
	}
	return 0.0;
}

uintptr_t instrumententrystate_pxtocolumn(
	InstrumentEntryState* self, double x,
	const psy_ui_TextMetric* tm)
{
	if (psy_table_size(&self->columns) > 0) {
		uintptr_t rv;
		uintptr_t i;
		uintptr_t numcolumns;
		double cpx;

		numcolumns = psy_table_maxkey(&self->columns);
		rv = psy_INDEX_INVALID;
		cpx = 0.0;
		for (i = 0; i <= numcolumns; ++i) {
			InstrumentEntryTableColumn* column;
			
			column = instrumententrystate_at(self, i);
			if (column) {
				cpx += psy_ui_value_px(&column->width, tm, NULL);
			}
			if (cpx >= x) {
				rv = i;
				break;
			}
		}
		return rv;
	}
	return 0;
}

/* InstrumentEntryTableViewHeader */
/* prototypes */
static void instrumententrytableviewheader_ondraw(InstrumentEntryTableViewHeader*,
	psy_ui_Graphics*);
static void instrumententrytableviewheader_onpreferredsize(InstrumentEntryTableViewHeader*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
/* vtable */
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
/* implementation */
void instrumententrytableviewheader_init(InstrumentEntryTableViewHeader* self,
	psy_ui_Component* parent, InstrumentEntryState* state,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
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
		InstrumentEntryTableColumn* column;

		column = (InstrumentEntryTableColumn*)psy_tableiterator_value(&it);
		if (column->label) {
			char* label;

			if (column->translation) {
				label = column->translation;
			} else {
				label = column->label;
			}			
			psy_ui_textout(g, cpx, 0, label, psy_strlen(label));
		}
		cpx += psy_ui_value_px(&column->width, tm, NULL);
	}
}

void instrumententrytableviewheader_onpreferredsize(InstrumentEntryTableViewHeader* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	*rv = psy_ui_size_make_em(80.0, 1.0);
}

/* InstrumentEntryRow */
/* prototypes */
static void instrumententryrow_ondestroy(InstrumentEntryRow*);
static void instrumententryrow_ondraw(InstrumentEntryRow*, psy_ui_Graphics*);
static void instrumententryrow_onpreferredsize(InstrumentEntryRow*,
		const psy_ui_Size* limit, psy_ui_Size* rv);
static void instrumententryrow_onmousedown(InstrumentEntryRow*,
	psy_ui_MouseEvent*);
static void instrumententryrow_onmouseleave(InstrumentEntryRow*,
	psy_ui_MouseEvent*);
static void instrumententryrow_onmouseenter(InstrumentEntryRow*,
	psy_ui_MouseEvent*);
static psy_ui_Rectangle instrumententryrow_editposition(InstrumentEntryRow*,
	uintptr_t column);
static void instrumententryrow_oneditchanged(InstrumentEntryRow*, IntEdit* sender);
static void instrumententryrow_onentryselected(
	InstrumentEntryRow*, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry* entry);
static void instrumententryrow_onentryupdate(
	InstrumentEntryView*, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry* entry);
static void instrumententryrow_updatestyles(InstrumentEntryRow*);
/* vtable */
static psy_ui_ComponentVtable instrumententryrow_vtable;
static bool instrumententryrow_vtable_initialized = FALSE;

static void instrumententryrow_vtableinit_init(InstrumentEntryRow* self)
{
	if (!instrumententryrow_vtable_initialized) {
		instrumententryrow_vtable = *(self->component.vtable);
		instrumententryrow_vtable.ondestroy =
			(psy_ui_fp_component_event)
			instrumententryrow_ondestroy;
		instrumententryrow_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			instrumententryrow_ondraw;
		instrumententryrow_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			instrumententryrow_onpreferredsize;
		instrumententryrow_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			instrumententryrow_onmousedown;
		instrumententryrow_vtable.onmouseenter =
			(psy_ui_fp_component_event)
			instrumententryrow_onmouseenter;
		instrumententryrow_vtable.onmouseleave =
			(psy_ui_fp_component_event)
			instrumententryrow_onmouseleave;
		instrumententryrow_vtable_initialized = TRUE;
	}
	self->component.vtable = &instrumententryrow_vtable;
}
/* implementation */
void instrumententryrow_init(InstrumentEntryRow* self, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_InstrumentEntry* entry,
	InstrumentEntryState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	instrumententryrow_vtableinit_init(self);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setstyletypes(&self->component,
		STYLE_TABLEROW, STYLE_TABLEROW_HOVER, STYLE_TABLEROW_SELECT,
		psy_INDEX_INVALID);
	self->view = view;
	self->entry = entry;
	self->state = state;
	self->hover = FALSE;
	self->selected = FALSE;
	psy_signal_connect(&self->state->signal_select, self,
		instrumententryrow_onentryselected);
	psy_signal_connect(&self->state->signal_entrychanged, self,
		instrumententryrow_onentryupdate);
}

InstrumentEntryRow* instrumententryrow_alloc(void)
{
	return (InstrumentEntryRow*)malloc(sizeof(InstrumentEntryRow));
}

InstrumentEntryRow* instrumententryrow_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_InstrumentEntry* entry, InstrumentEntryState* state)
{
	InstrumentEntryRow* rv;

	rv = instrumententryrow_alloc();
	if (rv) {
		instrumententryrow_init(rv, parent, view, entry, state);
		psy_ui_component_deallocateafterdestroyed(&rv->component);		
	}
	return rv;
}

void instrumententryrow_ondestroy(InstrumentEntryRow* self)
{	
	self->state->curredit = NULL;
	psy_signal_disconnect(&self->state->signal_select, self,
		instrumententryrow_onentryselected);
	psy_signal_disconnect(&self->state->signal_entrychanged, self,
		instrumententryrow_onentryupdate);
}

void instrumententryrow_ondraw(InstrumentEntryRow* self, psy_ui_Graphics* g)
{		
	double cpx;
	double cpy;
	psy_TableIterator it;	

	psy_ui_setcolour(g, psy_ui_colour_make(0x00CACACA));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00CACACA));
	cpx = 0.0;
	cpy = 0.0;	
	for (it = psy_table_begin(&self->state->columns);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		uintptr_t column;
		InstrumentEntryTableColumn* columndef;
		char text[64];				
						
		column = psy_tableiterator_key(&it);
		columndef = (InstrumentEntryTableColumn*)psy_tableiterator_value(&it);
		switch (column) {
		case 0:
			psy_snprintf(text, 64, "%0X:%0X", self->entry->sampleindex.slot,
				self->entry->sampleindex.subslot);
			break;
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
		psy_ui_textout(g, cpx, cpy, text, psy_strlen(text));
		cpx += psy_ui_value_px(&columndef->width,
			psy_ui_component_textmetric(&self->component), NULL);
	}
}

void instrumententryrow_onmousedown(InstrumentEntryRow* self,
	psy_ui_MouseEvent* ev)
{	
	if (self->selected) {
		uintptr_t column;

		column = instrumententrystate_pxtocolumn(self->state, ev->pt.x,
			psy_ui_component_textmetric(&self->component));
		if (column) {
			if (!self->state->curredit ||
				psy_ui_component_parent(&self->state->curredit->component) != &self->component) {
				if (self->state->curredit) {
					psy_ui_component_destroy(&self->state->curredit->component);
				}
				self->state->curredit = intedit_allocinit(&self->component, "", 0, 0, 0);
				psy_signal_connect(&self->state->curredit->signal_changed, self,
					instrumententryrow_oneditchanged);
			}
			self->state->editcolumn = column;
			switch (column) {
			case 0:
				intedit_setvalue(self->state->curredit, (int)self->entry->sampleindex.subslot);
				break;
			case 1:
				intedit_setvalue(self->state->curredit, (uint8_t)self->entry->keyrange.low);
				break;
			case 2:
				intedit_setvalue(self->state->curredit, (uint8_t)self->entry->keyrange.high);
				break;
			case 3:
				intedit_setvalue(self->state->curredit, (uint8_t)self->entry->fixedkey);
				break;
			}
			psy_ui_component_setposition(intedit_base(self->state->curredit),
				instrumententryrow_editposition(self, column));
		}
	} else {
		instrumententrystate_selectentry(self->state, self->entry);		
	}
}

void instrumententryrow_oneditchanged(InstrumentEntryRow* self, IntEdit* sender)
{
	switch (self->state->editcolumn) {
	case 0:
		self->entry->sampleindex.subslot = intedit_value(self->state->curredit);
		break;
	case 1:
		self->entry->keyrange.low = intedit_value(self->state->curredit);		
		break;
	case 2:
		self->entry->keyrange.high = intedit_value(self->state->curredit);
		break;
	case 3:
		self->entry->fixedkey = intedit_value(self->state->curredit);
		break;
	default:
		break;
	}
	instrumententrystate_updateentry(self->state, self->entry);
}

psy_ui_Rectangle instrumententryrow_editposition(InstrumentEntryRow* self,
	uintptr_t column)
{		
	InstrumentEntryTableColumn* columndef;
	psy_ui_Size size;
	psy_ui_RealRectangle position;
	
	columndef = instrumententrystate_at(self->state, column);
	if (!columndef) {
		return psy_ui_rectangle_make(psy_ui_point_zero(), psy_ui_size_zero());
	}
	size = psy_ui_component_scrollsize(&self->component);
	position = psy_ui_component_position(&self->component);
	return psy_ui_rectangle_make(
		psy_ui_point_make_px(			
			instrumententrystate_columnpx(self->state, column,
				psy_ui_component_textmetric(&self->component)), 0.0),
		psy_ui_size_make(columndef->width, size.height));
}

void instrumententryrow_onpreferredsize(InstrumentEntryRow* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_size_setem(rv, 120.0, 1.0);
}

void instrumententryrow_onmouseenter(InstrumentEntryRow* self,
	psy_ui_MouseEvent* ev)
{
	self->hover = TRUE;
	instrumententryrow_updatestyles(self);
}

void instrumententryrow_onmouseleave(InstrumentEntryRow* self,
	psy_ui_MouseEvent* ev)
{
	self->hover = FALSE;
	instrumententryrow_updatestyles(self);
}

void instrumententryrow_onentryselected(
	InstrumentEntryRow* self, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry* entry)
{
	bool selected;
	
	selected = self->state->selectedentry == self->entry;
	if (selected != self->selected) {
		if (self->state->curredit) {
			psy_ui_component_destroy(intedit_base(self->state->curredit));
			self->state->curredit = NULL;
		}
		self->selected = selected;
		instrumententryrow_updatestyles(self);
	}
}

void instrumententryrow_onentryupdate(
	InstrumentEntryView* self, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry* entry)
{
	psy_ui_component_invalidate(&self->component);
}

void instrumententryrow_updatestyles(InstrumentEntryRow* self)
{		
	/*
	** if (self->selected) {
	** 	self->component.style.currstyle = &self->component.style.select;
	**  } else if (self->hover) {
	** 	self->component.style.currstyle = &self->component.style.hover;
	** } else {
	** 	self->component.style.currstyle = &self->component.style.overridestyle;
	** }
	*/
	psy_ui_component_invalidate(&self->component);
}

/* InstrumentEntryTableView */
/* prototypes */
static void instrumententrytableview_ondestroy(InstrumentEntryTableView*);
static void instrumententrytableview_onpreferredsize(InstrumentEntryTableView*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void instrumententrytableview_ondraw(InstrumentEntryTableView*,
	psy_ui_Graphics*);
/* vtable */
static psy_ui_ComponentVtable instrumententrytableview_vtable;
static bool instrumententrytableview_vtable_initialized = FALSE;

static void instrumententrytableview_vtable_init(InstrumentEntryTableView* self)
{
	if (!instrumententrytableview_vtable_initialized) {
		instrumententrytableview_vtable = *(self->component.vtable);
		instrumententryview_vtable.ondestroy =
			(psy_ui_fp_component_event)
			instrumententrytableview_ondestroy;
		instrumententrytableview_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			instrumententrytableview_onpreferredsize;		
		instrumententrytableview_vtable_initialized = TRUE;
	}
	self->component.vtable = &instrumententrytableview_vtable;
}

void instrumententrytableview_init(InstrumentEntryTableView* self,
	psy_ui_Component* parent, InstrumentEntryState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	instrumententrytableview_vtable_init(self);	
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());
	self->state = state;	
	self->instrument = NULL;		
	psy_ui_component_setstyletypes(&self->component,
		STYLE_TABLEROW, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);
	instrumententrytableview_build(self);	
	psy_ui_component_setwheelscroll(&self->component, 4);
	psy_ui_component_setscrollstep(&self->component,
		psy_ui_size_make_em(0.0, 1.0));
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
}

void instrumententrytableview_ondestroy(InstrumentEntryTableView* self)
{	
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
	self->state->curredit = NULL;
	if (self->instrument) {
		psy_List* p;
		uintptr_t i;

		for (i = 0, p = self->instrument->entries; p != NULL; p = p->next, ++i) {
			psy_audio_InstrumentEntry* entry;
			InstrumentEntryRow* row;

			entry = (psy_audio_InstrumentEntry*)psy_list_entry(p);
			row = (InstrumentEntryRow*)instrumententryrow_allocinit(
				&self->component, NULL, /* &self->component */
				entry, self->state);			
		}
	}
	psy_ui_component_align(&self->component);
}

void instrumententrytableview_onpreferredsize(InstrumentEntryTableView* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (self->instrument) {		
		*rv = psy_ui_size_make_em(80.0,
			psy_max(6.0, (double)psy_list_size(self->instrument->entries)));
	} else {
		*rv = psy_ui_size_make_em(80.0, 6.0);
	}
}

/* InstrumentNoteMapButtons */
/* implementation */
void instrumentnotemapbuttons_init(InstrumentNoteMapButtons* self,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init(&self->add, &self->component, NULL);
	psy_ui_button_settext(&self->add, "instrumentview.add");
	psy_ui_button_init(&self->remove, &self->component, NULL);
	psy_ui_button_settext(&self->remove, "instrumentview.remove");	
}

/* InstrumentNoteMapView */
/* prototypes */
static void instrumentnotemapview_ondestroy(InstrumentNoteMapView*);	
static void instrumentnotemapview_initentries(InstrumentNoteMapView*,
	Workspace*);
static void instrumentnotemapview_inittable(InstrumentNoteMapView*,
	Workspace*);
static void instrumentnotemapview_setmetrics(InstrumentNoteMapView*,
	InstrumentNoteMapMetrics);
static void instrumentnotemapview_onaddentry(InstrumentNoteMapView*, psy_ui_Component* sender);
static void instrumentnotemapview_onremoveentry(InstrumentNoteMapView*, psy_ui_Component* sender);
static void instrumentnotemapview_onentryselected(InstrumentNoteMapView*,
	InstrumentEntryState* sender, psy_audio_InstrumentEntry*);
static void instrumentnotemapview_onsampleselected(InstrumentNoteMapView*,
	SamplesBox*);
static void instrumentnotemapview_onentryupdate(
	InstrumentNoteMapView*, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry*);
static void instrumentnotemapview_onlanguagechanged(InstrumentNoteMapView*);
/* vtable */
static psy_ui_ComponentVtable instrumentnotemapview_vtable;
static bool instrumentnotemapview_vtable_initialized = FALSE;

static void instrumentnotemapview_vtable_init(InstrumentNoteMapView* self)
{
	if (!instrumentnotemapview_vtable_initialized) {
		instrumentnotemapview_vtable = *(self->component.vtable);
		instrumentnotemapview_vtable.ondestroy =
			(psy_ui_fp_component_event)
			instrumentnotemapview_ondestroy;
		instrumentnotemapview_vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			instrumentnotemapview_onlanguagechanged;
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
	self->metrics.keysize = 8;
	self->metrics.lineheight = 15;
	instrumententrystate_init(&self->state, workspace);
	instrumententrystate_translate(&self->state, psy_ui_translator());
	psy_ui_label_init_text(&self->label, &self->component, NULL,
		"instrumentview.notemap");
	psy_ui_component_setalign(&self->label.component, psy_ui_ALIGN_TOP);
	samplesbox_init(&self->samplesbox, &self->component, NULL, workspace);
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_setmargin(&self->samplesbox.component, margin);
	psy_ui_component_setalign(&self->samplesbox.component, psy_ui_ALIGN_LEFT);	
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 1.5, 0.0);
	psy_ui_component_setmargin(&self->label.component, margin);
	instrumentnotemapbuttons_init(&self->buttons, &self->component);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->buttons.component, margin);
	instrumentnotemapview_initentries(self, workspace);	
	instrumentnotemapview_inittable(self, workspace);
	psy_ui_splitter_init(&self->splitter, &self->component, NULL);
	psy_ui_component_setalign(psy_ui_splitter_base(&self->splitter),
		psy_ui_ALIGN_BOTTOM);
	instrumentnotemapview_setmetrics(self, self->metrics);
	psy_signal_connect(&self->buttons.add.signal_clicked, self,
		instrumentnotemapview_onaddentry);
	psy_signal_connect(&self->buttons.remove.signal_clicked, self,
		instrumentnotemapview_onremoveentry);
	psy_signal_connect(&self->state.signal_select, self,
		instrumentnotemapview_onentryselected);	
	psy_signal_connect(&self->samplesbox.signal_changed, self,
		instrumentnotemapview_onsampleselected);
	psy_signal_connect(&self->state.signal_entrychanged, self,
		instrumentnotemapview_onentryupdate);
}

void instrumentnotemapview_ondestroy(InstrumentNoteMapView* self)
{
	instrumententrystate_dispose(&self->state);
}

void instrumentnotemapview_onlanguagechanged(InstrumentNoteMapView* self)
{
	instrumententrystate_translate(&self->state, psy_ui_translator());
	psy_ui_component_invalidate(&self->tableheader.component);
}

void instrumentnotemapview_initentries(InstrumentNoteMapView* self, Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->entries, &self->component, NULL);
	psy_ui_component_setalign(&self->entries, psy_ui_ALIGN_CLIENT);
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 0.0, 2.0);
	instrumententryview_init(&self->entryview, &self->entries, &self->state);
	psy_ui_scroller_init(&self->scroller, &self->entryview.component,
		&self->entries, NULL);	
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->entryview.component, psy_ui_ALIGN_FIXED);
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_setmargin(&self->entryview.component, margin);	
	instrumentkeyboardview_init(&self->keyboard, &self->entries);
	psy_ui_component_setalign(&self->keyboard.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setmargin(&self->keyboard.component, margin);
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_setmargin(&self->keyboard.component, margin);
}

void instrumentnotemapview_inittable(InstrumentNoteMapView* self, Workspace* workspace)
{
	psy_ui_component_init(&self->table, &self->component, NULL);
	psy_ui_component_setalign(&self->table, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setpreferredsize(&self->table,
		psy_ui_size_make_em(0.0, 6.0));	
	instrumententrytableviewheader_init(&self->tableheader,
		&self->table, &self->state, workspace);
	psy_ui_component_setalign(&self->tableheader.component, psy_ui_ALIGN_TOP);
	instrumententrytableview_init(&self->tableview,
		&self->table, &self->state);
	psy_ui_scroller_init(&self->scroller_table, &self->tableview.component,
		&self->table, NULL);
	psy_ui_component_setalign(&self->scroller_table.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->tableview.component, psy_ui_ALIGN_FIXED);
	psy_ui_component_setbackgroundmode(&self->scroller_table.pane, psy_ui_SETBACKGROUND);
}

void instrumentnotemapview_setinstrument(InstrumentNoteMapView* self,
	psy_audio_Instrument* instrument)
{
	psy_audio_InstrumentEntry* entry;

	self->instrument = instrument;
	self->state.selectedentry = NULL;
	instrumententryview_setinstrument(&self->entryview, instrument);
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
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_align(&self->scroller_table.pane);
}

void instrumentnotemapview_onaddentry(InstrumentNoteMapView* self,
	psy_ui_Component* sender)
{	
	if (self->instrument) {
		psy_audio_InstrumentEntry entry;

		psy_audio_instrumententry_init(&entry);
		entry.sampleindex = samplesbox_selected(&self->samplesbox);
		psy_audio_instrument_addentry(self->instrument, &entry);
		instrumententrytableview_build(&self->tableview);
		instrumentnotemapview_update(self);
	}
}

void instrumentnotemapview_onremoveentry(InstrumentNoteMapView* self,
	psy_ui_Component* sender)
{
	if (self->instrument) {		
		uintptr_t row;

		row = psy_list_entry_index(self->instrument->entries, self->state.selectedentry);
		psy_audio_instrument_removeentry(self->instrument, row);
		self->state.selectedentry = NULL;
		instrumententrytableview_build(&self->tableview);
		instrumentnotemapview_update(self);
	}
}

void instrumentnotemapview_onentryselected(InstrumentNoteMapView* self,
	InstrumentEntryState* sender, psy_audio_InstrumentEntry* entry)
{			
	if (self->instrument && entry) {
		if (self->state.selectedentry) {
			self->keyboard.entry = *self->state.selectedentry;
		} else {
			self->keyboard.entry.keyrange.low = 255;
			self->keyboard.entry.keyrange.high = 255;
		}		
		samplesbox_select(&self->samplesbox, entry->sampleindex);				
	} else {
		self->keyboard.entry.keyrange.low = 255;
		self->keyboard.entry.keyrange.high = 255;
		samplesbox_select(&self->samplesbox,
			psy_audio_sampleindex_make(0, 0));
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
		self->keyboard.entry = *self->state.selectedentry;
	} else {
		self->keyboard.entry.keyrange.low = 255;
		self->keyboard.entry.keyrange.high = 255;
	}
	psy_ui_component_invalidate(&self->keyboard.component);

}

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
			(psy_ui_fp_component)
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
	self->metrics.keysize = 6;
	self->metrics.line_height = 15;
	psy_audio_instrumententry_init(&self->entry);
	self->entry.keyrange.low = 255;
	self->entry.keyrange.high = 255;
	psy_ui_component_set_aligner(&self->component, NULL);
	psy_ui_component_set_preferred_size(&self->component,
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
	psy_ui_set_text_colour(g, psy_ui_colour_make(0x00333333));	
	for (key = keymin; key < keymax; ++key) {
		if (!psy_dsp_isblack(key)) {
			psy_ui_RealRectangle r;
			psy_ui_Colour colour;

			r = psy_ui_realrectangle_make(
					psy_ui_realpoint_make(cp, 0),
					psy_ui_realsize_make(
						(self->metrics.keysize + 1),
						size.height));
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
	psy_ui_set_text_colour(g, psy_ui_colour_make(0x00CACACA));
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
			r = psy_ui_realrectangle_make(
					psy_ui_realpoint_make(x, 0),
					psy_ui_realsize_make(width, (int)(size.height * top)));
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
		psy_ui_component_scroll_size(&self->component), tm, NULL);
	self->metrics.keysize = size.width / (double)numwhitekeys;
	psy_ui_component_set_scroll_step_height(&self->component,
		psy_ui_value_make_px(self->metrics.line_height * 3));
}



/* InstrumentEntryView */
/* prototypes */
static void instrumententryview_on_destroyed(InstrumentEntryView*);
static void instrumententryview_onpreferredsize(InstrumentEntryView*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void instrumententryview_ondraw(InstrumentEntryView*, psy_ui_Graphics*);
static void instrumententryview_onsize(InstrumentEntryView*);
static void instrumententryview_on_mouse_down(InstrumentEntryView*,
	psy_ui_MouseEvent*);
static void instrumententryview_onmousemove(InstrumentEntryView*,
	psy_ui_MouseEvent*);
static void instrumententryview_on_mouse_up(InstrumentEntryView*,
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
		instrumententryview_vtable.on_destroyed =
			(psy_ui_fp_component)
			instrumententryview_on_destroyed;
		instrumententryview_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			instrumententryview_ondraw;
		instrumententryview_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			instrumententryview_on_mouse_down;
		instrumententryview_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			instrumententryview_onmousemove;
		instrumententryview_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			instrumententryview_on_mouse_up;
		instrumententryview_vtable.onsize =
			(psy_ui_fp_component)
			instrumententryview_onsize;
		instrumententryview_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
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
	psy_ui_component_set_wheel_scroll(&self->component, 1);	
	psy_ui_component_set_scroll_step(&self->component,
		psy_ui_size_make_em(0.0, 1.0));
	self->instrument = 0;
	self->metrics.keysize = 8;
	self->metrics.line_height = 15;
	self->dragmode = 0;	
	self->state = state;
	psy_ui_component_set_scroll_step_height(&self->component,
		psy_ui_value_make_px(45));
	instrumententryview_updatemetrics(self);	
	psy_signal_connect(&self->state->signal_select, self,
		instrumententryview_onentryselected);
	psy_signal_connect(&self->state->signal_entrychanged, self,
		instrumententryview_onentryupdate);
	psy_ui_component_set_overflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
}

void instrumententryview_on_destroyed(InstrumentEntryView* self)
{	
	psy_signal_disconnect(&self->state->signal_select, self,
		instrumententryview_onentryselected);
	psy_signal_disconnect(&self->state->signal_select, self,
		instrumententryview_onentryupdate);
}

void instrumententryview_set_instrument(InstrumentEntryView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	psy_ui_component_set_scroll_top(&self->component, psy_ui_value_zero());	
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

			scrollleft = psy_ui_component_scroll_left_px(&self->component);
			scrolltop = psy_ui_component_scroll_top_px(&self->component);
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
				r = psy_ui_realrectangle_make(
						psy_ui_realpoint_make(keylo_startx, scrolltop),
						psy_ui_realsize_make(keyhi_endx - keylo_startx, scrolltop + size.height));
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
			cpy += self->metrics.line_height;
		}		
		if (!self->instrument->entries) {
			static const char* nomapping = "No Instrument Mapping";

			psy_ui_textout(g, psy_ui_realpoint_make(
				(size.width - tm->tmAveCharWidth * psy_strlen(nomapping)) / 2,
				(size.height - tm->tmHeight) / 2),
				nomapping, psy_strlen(nomapping));
		}
	} else {
		const psy_ui_TextMetric* tm;
		psy_ui_RealSize size;
		static const char* noinst = "No Instrument";

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_component_size_px(&self->component);
		psy_ui_textout(g,
			psy_ui_realpoint_make(
			(size.width - tm->tmAveCharWidth * psy_strlen(noinst)) / 2,
			(size.height - tm->tmHeight) / 2),
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
			(self->metrics.line_height) * (double)psy_list_size(self->instrument->entries));
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
		psy_ui_component_scroll_size(&self->component), tm, NULL);
	self->metrics.keysize = size.width / (double)numwhitekeys;
	psy_ui_component_set_scroll_step_height(&self->component,
		psy_ui_value_make_px(self->metrics.line_height * 3));
}

void instrumententryview_on_mouse_down(InstrumentEntryView* self,
	psy_ui_MouseEvent* ev)
{
	if (self->instrument) {		
		psy_audio_InstrumentEntry* entry;
							
		if (self->instrument) {
			uintptr_t numentry;	

			numentry = (uintptr_t)(psy_ui_mouseevent_pt(ev).y /
				(self->metrics.line_height));
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
		self->currkey = screentokey(psy_ui_mouseevent_pt(ev).x,
			self->metrics.keysize);
		instrumententryview_outputstatus(self, (uint8_t)self->currkey);
		entry = self->state->selectedentry;
		if (entry) {			
			if (abs((int)(entry->keyrange.low  -
					screentokey(psy_ui_mouseevent_pt(ev).x,
					self->metrics.keysize))) <
				abs((int)(entry->keyrange.high  -
					screentokey(psy_ui_mouseevent_pt(ev).x,
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
	size = psy_ui_component_scroll_size(&self->component);
	return psy_ui_value_px(&size.width, tm, NULL) / numwhitekeys;
}

void instrumententryview_onmousemove(InstrumentEntryView* self,
	psy_ui_MouseEvent* ev)
{	
	bool showresizecursor;

	showresizecursor = FALSE;
	self->currkey = screentokey(psy_ui_mouseevent_pt(ev).x,
		self->metrics.keysize);
	if (self->dragmode != INSTVIEW_DRAG_NONE && self->instrument) {
		psy_audio_InstrumentEntry* entry;
		uintptr_t screenkey;
				
		screenkey = screentokey(psy_ui_mouseevent_pt(ev).x,
			self->metrics.keysize);
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

void instrumententryview_on_mouse_up(InstrumentEntryView* self,
	psy_ui_MouseEvent* ev)
{
	if (self->instrument) {
		self->dragmode = 0;		
		psy_ui_component_release_capture(&self->component);	
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
		patternviewconfig_notetabmode(&self->state->workspace->config.visual.patview));
	if (notestr) {				
		psy_snprintf(text, 64, "%s %s", keydesc, notestr);		
	} else {
		psy_snprintf(text, 64, "%s %d", keydesc, (int)key);
	}
	workspace_output_status(self->state->workspace, text);
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

/* InstrumentEntryRow */

/* prototypes */
static void instrumententryrow_on_destroyed(InstrumentEntryRow*);
static void instrumententryrow_on_mouse_down(InstrumentEntryRow*,
	psy_ui_MouseEvent*);
static void instrumententryrow_onentryselected(
	InstrumentEntryRow*, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry* entry);
static void instrumententryrow_onentryupdate(
	InstrumentEntryRow*, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry* entry);

/* vtable */
static psy_ui_ComponentVtable instrumententryrow_vtable;
static bool instrumententryrow_vtable_initialized = FALSE;

static void instrumententryrow_vtableinit_init(InstrumentEntryRow* self)
{
	if (!instrumententryrow_vtable_initialized) {
		instrumententryrow_vtable = *(self->component.vtable);
		instrumententryrow_vtable.on_destroyed =
			(psy_ui_fp_component)
			instrumententryrow_on_destroyed;		
		instrumententryrow_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			instrumententryrow_on_mouse_down;		
		instrumententryrow_vtable_initialized = TRUE;
	}
	self->component.vtable = &instrumententryrow_vtable;
}

/* implementation */
void instrumententryrow_init(InstrumentEntryRow* self, psy_ui_Component* parent,
	psy_audio_InstrumentEntry* entry, InstrumentEntryState* state)
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
		instrumententryrow_onentryupdate);
}

InstrumentEntryRow* instrumententryrow_alloc(void)
{
	return (InstrumentEntryRow*)malloc(sizeof(InstrumentEntryRow));
}

InstrumentEntryRow* instrumententryrow_allocinit(
	psy_ui_Component* parent, psy_audio_InstrumentEntry* entry,
	InstrumentEntryState* state)
{
	InstrumentEntryRow* rv;

	rv = instrumententryrow_alloc();
	if (rv) {
		instrumententryrow_init(rv, parent, entry, state);
		psy_ui_component_deallocate_after_destroyed(&rv->component);		
	}
	return rv;
}

void instrumententryrow_on_destroyed(InstrumentEntryRow* self)
{		
	psy_signal_disconnect(&self->state->signal_select, self,
		instrumententryrow_onentryselected);
	psy_signal_disconnect(&self->state->signal_entrychanged, self,
		instrumententryrow_onentryupdate);
}

void instrumententryrow_on_mouse_down(InstrumentEntryRow* self,
	psy_ui_MouseEvent* ev)
{		
	instrumententrystate_selectentry(self->state, self->entry);	
}

void instrumententryrow_onentryselected(
	InstrumentEntryRow* self, InstrumentEntryState* sender,
	psy_audio_InstrumentEntry* entry)
{
		
}

void instrumententryrow_onentryupdate(InstrumentEntryRow* self,
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
			InstrumentEntryRow* row;

			entry = (psy_audio_InstrumentEntry*)psy_list_entry(p);
			row = (InstrumentEntryRow*)instrumententryrow_allocinit(
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
static void instrumentnotemapview_setmetrics(InstrumentNoteMapView*,
	InstrumentNoteMapMetrics);
static void instrumentnotemapview_onaddentry(InstrumentNoteMapView*,
	psy_ui_Component* sender);
static void instrumentnotemapview_onremoveentry(InstrumentNoteMapView*,
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
	self->metrics.keysize = 8;
	self->metrics.line_height = 15;
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

void instrumentnotemapview_on_destroyed(InstrumentNoteMapView* self)
{
	instrumententrystate_dispose(&self->state);
}

void instrumentnotemapview_initentries(InstrumentNoteMapView* self,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->entries, &self->component, NULL);
	psy_ui_component_set_align(&self->entries, psy_ui_ALIGN_CLIENT);
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 0.0, 2.0);
	instrumententryview_init(&self->entryview, &self->entries, &self->state);
	psy_ui_scroller_init(&self->scroller, &self->entries, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->entryview.component);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_align(&self->entryview.component, psy_ui_ALIGN_FIXED);
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_set_margin(&self->entryview.component, margin);	
	instrumentkeyboardview_init(&self->keyboard, &self->entries);
	psy_ui_component_set_align(&self->keyboard.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_margin(&self->keyboard.component, margin);
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_set_margin(&self->keyboard.component, margin);
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
		self->keyboard.entry = *self->state.selectedentry;
	} else {
		self->keyboard.entry.keyrange.low = 255;
		self->keyboard.entry.keyrange.high = 255;
	}
	psy_ui_component_invalidate(&self->keyboard.component);

}

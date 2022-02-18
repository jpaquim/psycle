/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditruler.h"
/* host */
#include "resources/resource.h"
#include "sequencetrackbox.h"
#include "styles.h"
/* audio */
#include <exclusivelock.h>
#include <sequencecmds.h>
/* platform */
#include "../../detail/portable.h"

#define DEFAULT_PXPERBEAT 10.0

/* SeqEditRuler */
/* prototypes */
static void seqeditruler_ondestroy(SeqEditRuler*);
static void seqeditruler_ondraw(SeqEditRuler*, psy_ui_Graphics*);
static void seqeditruler_drawruler(SeqEditRuler*, psy_ui_Graphics*,
	double baseline);
static void seqeditruler_draweditposition(SeqEditRuler*, psy_ui_Graphics*,
	double baseline);
static void seqeditruler_onsequenceselect(SeqEditRuler*,
	psy_audio_SequenceSelection* sender, psy_audio_OrderIndex*);
static void seqeditruler_onpreferredsize(SeqEditRuler*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static psy_dsp_big_beat_t seqeditruler_step(const SeqEditRuler*);
static void seqeditruler_oncursorchanged(SeqEditRuler*, SeqEditState*);
static void seqeditruler_onmousemove(SeqEditRuler*, psy_ui_MouseEvent*);
static void seqeditruler_onmouseenter(SeqEditRuler*);
static void seqeditruler_onmouseleave(SeqEditRuler*);
/* vtable */
static psy_ui_ComponentVtable seqeditruler_vtable;
static bool seqeditruler_vtable_initialized = FALSE;

static void seqeditruler_vtable_init(SeqEditRuler* self)
{
	if (!seqeditruler_vtable_initialized) {
		seqeditruler_vtable = *(self->component.vtable);
		seqeditruler_vtable.ondestroy =
			(psy_ui_fp_component_event)
			seqeditruler_ondestroy;
		seqeditruler_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqeditruler_ondraw;
		seqeditruler_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqeditruler_onpreferredsize;
		seqeditruler_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditruler_onmousemove;
		seqeditruler_vtable.onmouseenter =
			(psy_ui_fp_component_event)
			seqeditruler_onmouseenter;
		seqeditruler_vtable.onmouseleave =
			(psy_ui_fp_component_event)
			seqeditruler_onmouseleave;
		seqeditruler_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqeditruler_vtable;
}
/* implementation */
void seqeditruler_init(SeqEditRuler* self, psy_ui_Component* parent,
	SeqEditState* state)
{
	assert(self);
	assert(state);	

	psy_ui_component_init(&self->component, parent, NULL);
	seqeditruler_vtable_init(self);		
	self->state = state;	
	psy_signal_connect(&state->workspace->song->sequence.sequenceselection.signal_select,
		self, seqeditruler_onsequenceselect);	
	psy_signal_connect(&self->state->signal_cursorchanged, self,
		seqeditruler_oncursorchanged);		
}

void seqeditruler_ondestroy(SeqEditRuler* self)
{
	psy_signal_disconnect(
		&self->state->workspace->song->sequence.sequenceselection.signal_select,
		self, seqeditruler_onsequenceselect);
}

void seqeditruler_ondraw(SeqEditRuler* self, psy_ui_Graphics* g)
{
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;
	double baseline;

	size = psy_ui_component_size_px(&self->component);
	tm = psy_ui_component_textmetric(&self->component);	
	baseline = size.height - tm->tmHeight - 2;	
	seqeditruler_drawruler(self, g, baseline);	
	seqeditruler_draweditposition(self, g, baseline);
}

void seqeditruler_drawruler(SeqEditRuler* self, psy_ui_Graphics* g,
	double baseline)
{
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;	
	double textline;
	double linewidth;
	psy_dsp_big_beat_t duration;
	psy_dsp_big_beat_t clipstart;
	psy_dsp_big_beat_t clipend;
	psy_dsp_big_beat_t currbeat;
	psy_dsp_big_beat_t step;
	psy_ui_Colour overlaycolour;
	psy_ui_Colour linecolour;
	psy_ui_Colour linebeatcolour;
	psy_ui_Colour linebeat4colour;

	size = psy_ui_component_size_px(&self->component);
	tm = psy_ui_component_textmetric(&self->component);	
	textline = baseline - tm->tmHeight - 2;
	duration = (size.width + psy_ui_component_scrollleft_px(&self->component)) /
		(psy_dsp_big_beat_t)self->state->pxperbeat;
	/* psy_audio_sequence_duration(&workspace_song(
	 * self->workspace)->sequence); */
	linewidth = duration * self->state->pxperbeat;
	linecolour = psy_ui_style(STYLE_SEQEDT_RULER_BOTTOM)->colour;
	if (psy_ui_app_hasdarktheme(psy_ui_app())) {
		overlaycolour = psy_ui_colour_white();
	} else {
		overlaycolour = psy_ui_colour_black();
	}
	linebeatcolour = psy_ui_colour_overlayed(&linecolour, &overlaycolour, 0.10);
	linebeat4colour = psy_ui_colour_overlayed(&linecolour, &overlaycolour, 0.20);
	psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
		psy_ui_realpoint_zero(), psy_ui_realsize_make(size.width, baseline)),
		psy_ui_style(STYLE_SEQEDT_RULER_TOP)->background.colour);	
	psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, baseline),
		psy_ui_realsize_make(size.width, size.height - baseline)),
		psy_ui_style(STYLE_SEQEDT_RULER_BOTTOM)->background.colour);
	psy_ui_setcolour(g, linecolour);
	psy_ui_drawline(g, psy_ui_realpoint_make(0, baseline),
		psy_ui_realpoint_make(linewidth, baseline));
	clipstart = 0;
	clipend = duration;
	step = seqeditruler_step(self);
	for (currbeat = clipstart; currbeat <= clipend; currbeat += step) {
		double cpx;
		char txt[64];

		cpx = seqeditstate_beattopx(self->state, currbeat);
		if (((intptr_t)(currbeat / step) % 8) == 0) {
			psy_ui_settextcolour(g, linebeat4colour);
			psy_ui_setcolour(g, linebeat4colour);
		} else if (((intptr_t)(currbeat /step) % 4) == 0) {
			psy_ui_settextcolour(g, linebeatcolour);
			psy_ui_setcolour(g, linebeatcolour);
		} else {
			psy_ui_settextcolour(g, linecolour);
			psy_ui_setcolour(g, linecolour);
		}
		psy_ui_drawline(g, psy_ui_realpoint_make(cpx, baseline),
			psy_ui_realpoint_make(cpx, baseline + tm->tmHeight / 2));
		if (self->state->drawcursor && self->state->cursoractive) {
			psy_dsp_big_beat_t cursor;

			cursor = floor(self->state->cursorposition / step) * step;
			if (currbeat >= cursor && currbeat <= cursor + step) {
				double start;
				double end;

				if (cursor == currbeat &&
						self->state->cursorposition - cursor < step / 2) {
					psy_ui_settextcolour(g, linebeat4colour);
				} else if (cursor + step == currbeat &&
					self->state->cursorposition - cursor > step / 2) {
					psy_ui_settextcolour(g, linebeat4colour);
				}
				psy_ui_setcolour(g, linebeat4colour);
				start = cursor * self->state->pxperbeat;
				end = (cursor + step) * self->state->pxperbeat;
				psy_ui_drawline(g,
					psy_ui_realpoint_make(start, baseline),
					psy_ui_realpoint_make(end, baseline));
			}
		}
		psy_snprintf(txt, 64, "%d", (int)(currbeat));
		psy_ui_textout(g, cpx, textline, txt, psy_strlen(txt));
	}
	if (self->state->drawcursor && self->state->cursoractive) {
		psy_ui_RealPoint cp;
		
		cp = psy_ui_realpoint_make(seqeditstate_beattopx(self->state,
			self->state->cursorposition), baseline);
		psy_ui_setcolour(g, psy_ui_style(STYLE_SEQEDT_RULER_CURSOR)->colour);
		psy_ui_drawline(g, cp, psy_ui_realpoint_make(cp.x, baseline +
			size.height - 1));
	}
}

void seqeditruler_draweditposition(SeqEditRuler* self, psy_ui_Graphics* g,
	double baseline)
{
	psy_audio_SequenceEntry* seqentry;

	seqentry = psy_audio_sequence_entry(seqeditstate_sequence(self->state),
		psy_audio_sequenceselection_first(
			&self->state->workspace->song->sequence.sequenceselection));
	if (seqentry) {
		psy_ui_RealSize size;		
		psy_ui_IconDraw icondraw;

		size = psy_ui_component_size_px(&self->component);		
		psy_ui_icondraw_init(&icondraw, psy_ui_ICON_MORE);
		psy_ui_icondraw_draw(&icondraw, g,
			psy_ui_realpoint_make(
				seqeditstate_beattopx(self->state,
					psy_audio_sequenceentry_offset(seqentry)),
				baseline + 2),
			psy_ui_style(STYLE_SEQEDT_RULER_CURSOR)->colour);
	}
}

psy_dsp_big_beat_t seqeditruler_step(const SeqEditRuler* self)
{
	psy_dsp_big_beat_t rv;
	
	rv = 80.0 / self->state->pxperbeat;
	if (rv > 16.0) {
		return floor((rv / 16) * 16.0);
	} else if (rv > 4.0) {
		return floor((rv / 4.0) * 4.0);
	} else if (rv > 2.0) {
		return floor((rv / 2.0) * 2.0);
	}
	return 1.0;
}

void seqeditruler_onsequenceselect(SeqEditRuler* self,
	psy_audio_SequenceSelection* sender, psy_audio_OrderIndex* index)
{
	psy_ui_component_invalidate(&self->component);
}

void seqeditruler_oncursorchanged(SeqEditRuler* self, SeqEditState* state)
{
	psy_ui_component_invalidate(&self->component);
}

void seqeditruler_onpreferredsize(SeqEditRuler* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{	
	if (seqeditstate_sequence(self->state)) {				
		rv->width = psy_ui_value_make_px(
			seqeditstate_beattopx(self->state, psy_audio_sequence_duration(
				seqeditstate_sequence(self->state)) + 400.0));
	} else {
		rv->width = psy_ui_value_make_px(0.0);
	}	
	rv->height = psy_ui_value_make_eh(2.0);	
}

void seqeditruler_onmousemove(SeqEditRuler* self, psy_ui_MouseEvent* ev)
{
	seqeditstate_setcursor(self->state, seqeditstate_quantize(self->state,
		seqeditstate_pxtobeat(self->state, psy_ui_mouseevent_pt(ev).x)));
}

void seqeditruler_onmouseenter(SeqEditRuler* self)
{
	self->state->cursoractive = TRUE;
	psy_ui_component_invalidate(&self->component);
}

void seqeditruler_onmouseleave(SeqEditRuler* self)
{
	self->state->cursoractive = FALSE;
	psy_ui_component_invalidate(&self->component);
}

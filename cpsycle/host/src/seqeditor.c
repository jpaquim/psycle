/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditor.h"
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
static void seqeditruler_drawtimesigs(SeqEditRuler*, psy_ui_Graphics*,
	double baseline);
static void seqeditruler_drawtimesig(SeqEditRuler* self, psy_ui_Graphics*,
	psy_dsp_big_beat_t offset, int nominator, int denominator);
static void seqeditruler_drawruler(SeqEditRuler*, psy_ui_Graphics*,
	double baseline);
static void seqeditruler_draweditposition(SeqEditRuler*, psy_ui_Graphics*,
	double baseline);
static void seqeditruler_onsequenceselectionchanged(SeqEditRuler*,
	psy_audio_SequenceSelection* sender);
static void seqeditruler_onpreferredsize(SeqEditRuler*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static psy_dsp_big_beat_t seqeditruler_step(const SeqEditRuler*);
static void seqeditruler_oncursorchanged(SeqEditRuler*, SeqEditState*);
static void seqeditruler_onmousedown(SeqEditRuler*, psy_ui_MouseEvent*);
static void seqeditruler_onmousedoubleclick(SeqEditRuler*, psy_ui_MouseEvent*);
static void seqeditruler_oneditaccept(SeqEditRuler*, psy_ui_Edit* sender);
static void seqeditruler_oneditreject(SeqEditRuler*, psy_ui_Edit* sender);
static void seqeditruler_edit(SeqEditRuler* self, double x, const char* text);
/* vtable */
static psy_ui_ComponentVtable seqeditruler_vtable;
static bool seqeditruler_vtable_initialized = FALSE;

static void seqeditruler_vtable_init(SeqEditRuler* self)
{
	if (!seqeditruler_vtable_initialized) {
		seqeditruler_vtable = *(self->component.vtable);
		seqeditruler_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqeditruler_ondestroy;
		seqeditruler_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqeditruler_ondraw;
		seqeditruler_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqeditruler_onpreferredsize;
		seqeditruler_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditruler_onmousedown;
		seqeditruler_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqeditruler_onmousedoubleclick;
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
	psy_ui_component_doublebuffer(&self->component);	
	self->state = state;	
	self->showtimesig = TRUE;
	self->preventedit = TRUE;
	self->editnominator = TRUE;
	self->currtimesig = NULL;
	psy_signal_connect(&state->workspace->sequenceselection.signal_changed,
		self, seqeditruler_onsequenceselectionchanged);	
	psy_signal_connect(&self->state->signal_cursorchanged, self,
		seqeditruler_oncursorchanged);
	self->preventedit = TRUE;
	if (self->state->edit) {
		psy_signal_connect(&self->state->edit->signal_accept,
			self, seqeditruler_oneditaccept);
		psy_signal_connect(&self->state->edit->signal_reject,
			self, seqeditruler_oneditreject);
	}
}

void seqeditruler_ondestroy(SeqEditRuler* self)
{
	psy_signal_disconnect(
		&self->state->workspace->sequenceselection.signal_changed,
		self, seqeditruler_onsequenceselectionchanged);
}

void seqeditruler_ondraw(SeqEditRuler* self, psy_ui_Graphics* g)
{
	const psy_ui_TextMetric* tm;
	double baseline;

	tm = psy_ui_component_textmetric(&self->component);
	if (self->showtimesig) {
		baseline = 3 * tm->tmHeight + 2;
	} else {
		baseline = tm->tmHeight + 2;
	}
	seqeditruler_drawruler(self, g, baseline);
	if (self->showtimesig) {
		seqeditruler_drawtimesigs(self, g, baseline);
	}
	seqeditruler_draweditposition(self, g, baseline);
}

void seqeditruler_drawtimesigs(SeqEditRuler* self, psy_ui_Graphics* g,
	double baseline)
{	
	psy_audio_Sequence* sequence;

	sequence = seqeditstate_sequence(self->state);
	if (sequence && sequence->patterns) {
		psy_List* p;

		for (p = sequence->globaltrack.entries; p != NULL; p = p->next) {
			psy_audio_SequenceEntry* seqentry;			

			seqentry = (psy_audio_SequenceEntry*)p->entry;
			if (seqentry && seqentry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				psy_audio_SequencePatternEntry* seqpatternentry;
				psy_audio_Pattern* pattern;				

				seqpatternentry = (psy_audio_SequencePatternEntry*)seqentry;
				pattern = psy_audio_sequencepatternentry_pattern(seqpatternentry,
					sequence->patterns);
				if (pattern) {
					psy_audio_PatternNode* node;

					for (node = pattern->events; node != NULL; node = node->next) {
						psy_audio_PatternEntry* patternentry;
						psy_audio_PatternEvent* e;						

						patternentry = (psy_audio_PatternEntry*)node->entry;
						e = psy_audio_patternentry_front(patternentry);
						if (e->note == psy_audio_NOTECOMMANDS_TIMESIG) {
							seqeditruler_drawtimesig(self, g, patternentry->offset, e->cmd,
								e->parameter);
						}
					}
				}
			}
		}		
	}
}

void seqeditruler_drawtimesig(SeqEditRuler* self, psy_ui_Graphics* g,
	psy_dsp_big_beat_t offset, int nominator, int denominator)
{
	char text[64];
	const psy_ui_TextMetric* tm;
	double cpx;

	tm = psy_ui_component_textmetric(&self->component);
	cpx = seqeditstate_beattopx(self->state, offset);
	psy_snprintf(text, 64, "%d", (int)(nominator));
	psy_ui_textout(g, cpx, 0, text, psy_strlen(text));
	psy_snprintf(text, 64, "%d", (int)(denominator));
	psy_ui_textout(g, cpx, tm->tmHeight, text, psy_strlen(text));
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
	duration = (size.width + psy_ui_component_scrollleftpx(&self->component)) /
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
		psy_ui_style(STYLE_SEQEDT_RULER_TOP)->backgroundcolour);	
	psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, baseline),
		psy_ui_realsize_make(size.width, size.height - baseline)),
		psy_ui_style(STYLE_SEQEDT_RULER_BOTTOM)->backgroundcolour);
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
		self->state->workspace->sequenceselection.editposition);
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

void seqeditruler_onsequenceselectionchanged(SeqEditRuler* self,
	psy_audio_SequenceSelection* sender)
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
		rv->width = psy_ui_value_make_px(0);
	}
	if (self->showtimesig) {
		rv->height = psy_ui_value_make_eh(4.0);
	} else {
		rv->height = psy_ui_value_make_eh(2.0);
	}
}

void seqeditruler_hidetimesig(SeqEditRuler* self)
{
	self->showtimesig = FALSE;
	psy_ui_component_invalidate(&self->component);
}
	
void seqeditruler_showtimesig(SeqEditRuler* self)
{
	self->showtimesig = TRUE;
	psy_ui_component_invalidate(&self->component);
}

void seqeditruler_onmousedown(SeqEditRuler* self, psy_ui_MouseEvent* ev)
{
	if (self->showtimesig) {
		psy_audio_Sequence* sequence;

		sequence = seqeditstate_sequence(self->state);
		if (sequence && sequence->patterns) {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_patterns_at(sequence->patterns,
				psy_audio_GLOBALPATTERN);
			if (pattern) {
				psy_audio_PatternNode* prev;
				psy_audio_PatternNode* node;
				double offset;

				offset = seqeditstate_quantize(self->state,
					seqeditstate_pxtobeat(self->state, ev->pt.x));
				node = psy_audio_pattern_findnode(pattern, 0, offset - 1.0,
					2.0, &prev);
				if (ev->button == 1) {
					if (node) {
						char text[64];
						psy_audio_PatternEntry* entry;
						psy_audio_PatternEvent* e;
						const psy_ui_TextMetric* tm;

						tm = psy_ui_component_textmetric(&self->component);
						entry = (psy_audio_PatternEntry*)node->entry;
						e = psy_audio_patternentry_front(entry);
						self->currtimesig = e;
						if (ev->pt.y < tm->tmHeight) {
							psy_snprintf(text, 64, "%d", (int)e->cmd);
							self->editnominator = TRUE;
						} else {
							psy_snprintf(text, 64, "%d", (int)e->parameter);
							self->editnominator = FALSE;
						}
						seqeditruler_edit(self, seqeditstate_beattopx(
							self->state, entry->offset), text);
					}
				} else if (ev->button == 2) {			
					if (node) {
						psy_audio_pattern_remove(pattern, node);
					}
					self->currtimesig = NULL;
					psy_ui_component_invalidate(&self->component);
				}
			}
		}
	}
}

void seqeditruler_edit(SeqEditRuler* self, double x, const char* text)
{
	if (self->state->edit) {		
		const psy_ui_TextMetric* tm;
		double y;
		
		tm = psy_ui_component_textmetric(&self->component);
		psy_ui_component_setparent(&self->state->edit->component,
			&self->component);
		if (self->editnominator) {
			y = 0;
		} else {
			y = tm->tmHeight;
		}
		psy_ui_component_setposition(&self->state->edit->component,
			psy_ui_rectangle_make(
				psy_ui_point_make_px(x, y),
				psy_ui_size_make_px(tm->tmAveCharWidth * 4, tm->tmHeight)));		
		psy_ui_edit_settext(self->state->edit, text);		
		psy_ui_edit_enableinputfield(self->state->edit);
		self->preventedit = FALSE;
		psy_ui_component_show(&self->state->edit->component);
		psy_ui_component_setfocus(&self->state->edit->component);		
	}
}

void seqeditruler_oneditaccept(SeqEditRuler* self, psy_ui_Edit* sender)
{
	if (!self->preventedit) {
		self->preventedit = TRUE;
		if (self->editnominator && self->currtimesig) {
			int nominator;

			nominator = atoi(psy_ui_edit_text(self->state->edit));
			self->currtimesig->cmd = (uint8_t)nominator;
		} else if (self->currtimesig) {
			int denominator;

			denominator = atoi(psy_ui_edit_text(self->state->edit));
			self->currtimesig->parameter = (uint8_t)denominator;			
		}
		self->currtimesig = NULL;
		psy_ui_component_hide(psy_ui_edit_base(sender));
		psy_ui_component_setparent(&self->state->edit->component,
			self->state->view);
		psy_ui_component_invalidate(&self->component);
	}
}

void seqeditruler_oneditreject(SeqEditRuler* self, psy_ui_Edit* sender)
{
	if (!self->preventedit) {
		self->preventedit = TRUE;
		self->currtimesig = NULL;
		psy_ui_component_hide(psy_ui_edit_base(sender));
		psy_ui_component_setparent(&self->state->edit->component,
			self->state->view);		
	}
}


void seqeditruler_onmousedoubleclick(SeqEditRuler* self, psy_ui_MouseEvent* ev)
{
	if (self->showtimesig) {
		psy_audio_Sequence* sequence;

		sequence = seqeditstate_sequence(self->state);
		if (sequence && sequence->patterns) {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_patterns_at(sequence->patterns,
				psy_audio_GLOBALPATTERN);
			if (pattern) {
				psy_audio_PatternEvent e;
				psy_audio_PatternNode* prev;
				psy_audio_PatternNode* node;

				node = psy_audio_pattern_findnode(pattern, 0 /* track 0 */,
					seqeditstate_quantize(self->state,
						seqeditstate_pxtobeat(self->state, ev->pt.x)) - 1.0,
					2.0, &prev);
				psy_audio_patternevent_init_all(&e,
					psy_audio_NOTECOMMANDS_TIMESIG, 0, 0, 0, 4, 4);
				psy_audio_pattern_insert(pattern, prev, 0,
					seqeditstate_quantize(self->state,
						seqeditstate_pxtobeat(self->state, ev->pt.x)), &e);
				psy_ui_component_invalidate(&self->component);
			}
		}
	}
}

/* SeqEditorLine */
static void seqeditorline_onpreferredsize(SeqEditorLine*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
/* vtable */
static psy_ui_ComponentVtable seqeditorline_vtable;
static bool seqeditorline_vtable_initialized = FALSE;

static void seqeditorline_vtable_init(SeqEditorLine* self)
{
	if (!seqeditorline_vtable_initialized) {
		seqeditorline_vtable = *(self->component.vtable);
		seqeditorline_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqeditorline_onpreferredsize;
	}
	self->component.vtable = &seqeditorline_vtable;
}

void seqeditorline_init(SeqEditorLine* self, psy_ui_Component* parent,
	psy_ui_Component* view, SeqEditState* state)
{
	psy_ui_component_init(seqeditorline_base(self), parent, view);	
	seqeditorline_vtable_init(self);
	self->state = state;
	psy_ui_component_setposition(seqeditorline_base(self),
		psy_ui_rectangle_make(psy_ui_point_zero(), psy_ui_size_zero()));
}

SeqEditorLine* seqeditorline_alloc(void)
{
	return (SeqEditorLine*)malloc(sizeof(SeqEditorLine));
}

SeqEditorLine* seqeditorline_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditState* state)
{
	SeqEditorLine* rv;

	rv = seqeditorline_alloc();
	if (rv) {
		seqeditorline_init(rv, parent, view, state);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

void seqeditorline_onpreferredsize(SeqEditorLine* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = psy_ui_value_make_px(1.0);
	rv->height = psy_ui_value_make_eh(10.0);
}

void seqeditorline_updateposition(SeqEditorLine* self,
	psy_dsp_big_beat_t position)
{
	double position_px;
	psy_ui_RealRectangle newposition;
	psy_ui_RealRectangle updateposition;
	psy_ui_RealSize parentsize;

	position_px = seqeditstate_beattopx(self->state, position);
	updateposition = psy_ui_component_position(&self->component);
	parentsize = psy_ui_component_scrollsize_px(
		psy_ui_component_parent(&self->component));
	if (parentsize.height != psy_ui_realrectangle_height(&updateposition) ||
			position_px != updateposition.left) {		
		newposition = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(position_px, 0.0),
			psy_ui_realsize_make(1.0, parentsize.height));		
		psy_ui_component_setposition(&self->component,
			psy_ui_rectangle_make_px(&newposition));
		psy_ui_realrectangle_union(&updateposition, &newposition);
		psy_ui_component_invalidaterect(
			psy_ui_component_parent(&self->component),
			updateposition);
	}
}

/* SeqEditorPlayLine */
/* prototypes */
static void seqeditorplayline_onmousedown(SeqEditorPlayline*,
	psy_ui_MouseEvent*);
static void seqeditorplayline_onmousemove(SeqEditorPlayline*,
	psy_ui_MouseEvent*);
static void seqeditorplayline_onmouseup(SeqEditorPlayline*, psy_ui_MouseEvent*);
/* vtable */
static psy_ui_ComponentVtable seqeditorplayline_vtable;
static bool seqeditorplayline_vtable_initialized = FALSE;

static void seqeditorplayline_vtable_init(SeqEditorPlayline* self)
{
	if (!seqeditorplayline_vtable_initialized) {
		seqeditorplayline_vtable =
			*(seqeditorline_base(&self->seqeditorline)->vtable);		
		seqeditorplayline_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditorplayline_onmousedown;
		seqeditorplayline_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditorplayline_onmousemove;
		seqeditorplayline_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			seqeditorplayline_onmouseup;
	}
	seqeditorline_base(&self->seqeditorline)->vtable =
		&seqeditorplayline_vtable;
}
/* implementation */
void seqeditorplayline_init(SeqEditorPlayline* self, psy_ui_Component* parent,
	psy_ui_Component* view, SeqEditState* state)
{
	seqeditorline_init(&self->seqeditorline, parent, view, state);	
	seqeditorplayline_vtable_init(self);
	psy_ui_component_setbackgroundcolour(
		seqeditorline_base(&self->seqeditorline),
		psy_ui_colour_make(0x00FF00000));	
	self->drag = FALSE;
	self->dragbase = 0.0;
}

SeqEditorPlayline* seqeditorplayline_alloc(void)
{
	return (SeqEditorPlayline*)malloc(sizeof(SeqEditorPlayline));
}

SeqEditorPlayline* seqeditorplayline_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditState* state)
{
	SeqEditorPlayline* rv;

	rv = seqeditorplayline_alloc();
	if (rv) {
		seqeditorplayline_init(rv, parent, view, state);
		psy_ui_component_deallocateafterdestroyed(
			seqeditorline_base(&rv->seqeditorline));		
	}
	return rv;
}

void seqeditorplayline_onmousedown(SeqEditorPlayline* self,
	psy_ui_MouseEvent* ev)
{	
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(
		seqeditorline_base(&self->seqeditorline));
	self->drag = TRUE;	
	self->dragbase = position.left;
	psy_ui_component_capture(seqeditorline_base(&self->seqeditorline));
	psy_ui_component_setcursor(seqeditorline_base(&self->seqeditorline),
		psy_ui_CURSORSTYLE_COL_RESIZE);
	psy_ui_mouseevent_stop_propagation(ev);
}

void seqeditorplayline_onmousemove(SeqEditorPlayline* self,
	psy_ui_MouseEvent* ev)
{
	psy_ui_component_setcursor(seqeditorline_base(&self->seqeditorline),
		psy_ui_CURSORSTYLE_COL_RESIZE);
	if (self->drag) {		
		psy_audio_exclusivelock_enter();
		psy_audio_player_setposition(
			&self->seqeditorline.state->workspace->player,
			psy_max(0.0, seqeditstate_pxtobeat(self->seqeditorline.state,
				self->dragbase + ev->pt.x)));
		psy_audio_exclusivelock_leave();
		seqeditorplayline_update(self);
		self->dragbase = psy_ui_component_position(
			seqeditorline_base(&self->seqeditorline)).left;		
	}
	psy_ui_mouseevent_stop_propagation(ev);	
}

void seqeditorplayline_onmouseup(SeqEditorPlayline* self, psy_ui_MouseEvent* ev)
{
	self->drag = FALSE;
	self->dragbase = 0.0;
	psy_ui_component_releasecapture(seqeditorline_base(&self->seqeditorline));
	psy_ui_mouseevent_stop_propagation(ev);
}

void seqeditorplayline_update(SeqEditorPlayline* self)
{		
	seqeditorline_updateposition(&self->seqeditorline,		
		psy_audio_player_position(workspace_player(
			self->seqeditorline.state->workspace)));	
}

/* SeqEditTrack */
/* prototypes */
static void seqedittrack_ondestroy(SeqEditTrack*);
static void seqedittrack_onpreferredsize(SeqEditTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqedittrack_onalign(SeqEditTrack*);
static void seqedittrack_onmousedown(SeqEditTrack*, psy_ui_MouseEvent*);
static void seqedittrack_onmousemove(SeqEditTrack*, psy_ui_MouseEvent*);
static void seqedittrack_onmouseup(SeqEditTrack*, psy_ui_MouseEvent*);
static void seqedittrack_onmousedoubleclick(SeqEditTrack*,
	psy_ui_MouseEvent*);
static void seqedittrack_build(SeqEditTrack* self);
static void seqedittrack_onsequenceinsert(SeqEditTrack*,
	psy_audio_Sequence* sender, psy_audio_OrderIndex* index);
static void seqedittrack_onsequenceremove(SeqEditTrack*,
	psy_audio_Sequence* sender, psy_audio_OrderIndex* index);
static void seqedittrack_onsequencetrackreposition(SeqEditTrack*,
	psy_audio_Sequence* sender, uintptr_t trackidx);
/* vtable */
static psy_ui_ComponentVtable seqedittrack_vtable;
static bool seqedittrack_vtable_initialized = FALSE;

static void seqedittrack_vtable_init(SeqEditTrack* self)
{
	if (!seqedittrack_vtable_initialized) {
		seqedittrack_vtable = *(self->component.vtable);
		seqedittrack_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqedittrack_ondestroy;
		seqedittrack_vtable.onalign =
			(psy_ui_fp_component_onalign)
			seqedittrack_onalign;
		seqedittrack_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqedittrack_onpreferredsize;
		seqedittrack_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqedittrack_onmousedown;
		seqedittrack_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqedittrack_onmousemove;
		seqedittrack_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			seqedittrack_onmouseup;
		seqedittrack_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqedittrack_onmousedoubleclick;
		seqedittrack_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqedittrack_vtable;
}
/* implementation */
void seqedittrack_init(SeqEditTrack* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, view);
	seqedittrack_vtable_init(self);	
	psy_ui_component_setalignexpand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);
	psy_ui_component_setminimumsize(&self->component,
		psy_ui_size_make_em(0.0, 2.0));	
	self->state = state;
	self->workspace = workspace;	
	self->currtrack = NULL;
	self->trackindex = 0;		
	self->view = view;	
	self->entries = NULL;
	if (seqeditstate_sequence(self->state)) {
		psy_signal_connect(
			&seqeditstate_sequence(self->state)->signal_insert,
			self, seqedittrack_onsequenceinsert);
		psy_signal_connect(
			&seqeditstate_sequence(self->state)->signal_remove,
			self, seqedittrack_onsequenceremove);
		psy_signal_connect(
			&seqeditstate_sequence(self->state)->signal_trackreposition,
			self, seqedittrack_onsequencetrackreposition);		
	}
}

void seqedittrack_ondestroy(SeqEditTrack* self)
{
	if (seqeditstate_sequence(self->state)) {
		psy_signal_disconnect(
			&seqeditstate_sequence(self->state)->signal_insert,
			self, seqedittrack_onsequenceinsert);
		psy_signal_disconnect(
			&seqeditstate_sequence(self->state)->signal_remove,
			self, seqedittrack_onsequenceremove);
		psy_signal_disconnect(
			&seqeditstate_sequence(self->state)->signal_trackreposition,
			self, seqedittrack_onsequencetrackreposition);		
	}
	psy_list_free(self->entries);
	self->entries = NULL;
}

SeqEditTrack* seqedittrack_alloc(void)
{
	return (SeqEditTrack*)malloc(sizeof(SeqEditTrack));
}

SeqEditTrack* seqedittrack_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditState* state, Workspace* workspace)
{
	SeqEditTrack* rv;

	rv = seqedittrack_alloc();
	if (rv) {
		seqedittrack_init(rv, parent, view, state, workspace);
		psy_ui_component_deallocateafterdestroyed(seqedittrack_base(rv));
	}
	return rv;
}

void seqedittrack_setsequencetrack(SeqEditTrack* self,
	psy_audio_SequenceTrackNode* tracknode, psy_audio_SequenceTrack* track,
	uintptr_t trackindex)
{
	self->currtrack = track;	
	self->trackindex = trackindex;	
	if (track->height != 0.0) {
		psy_ui_component_setpreferredheight(&self->component,
			psy_ui_value_make_eh(track->height));
	}
	seqedittrack_build(self);		
}

void seqedittrack_build(SeqEditTrack* self)
{
	psy_List* p;
	uintptr_t c;	

	psy_ui_component_clear(&self->component);
	psy_list_free(self->entries);
	self->entries = NULL;
	if (!workspace_song(self->workspace)) {
		return;
	}
	if (!self->currtrack) {
		return;
	}
	for (p = self->currtrack->entries, c = 0; p != NULL;
			psy_list_next(&p), ++c) {
		psy_audio_SequenceEntry* seqentry;
		psy_audio_Pattern* pattern;

		seqentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
		if (seqentry) {
			switch (seqentry->type) {
			case psy_audio_SEQUENCEENTRY_PATTERN: {
				psy_audio_SequencePatternEntry* seqpatternentry;

				seqpatternentry = (psy_audio_SequencePatternEntry*)seqentry;
				pattern = psy_audio_patterns_at(
					&workspace_song(self->workspace)->patterns,
					psy_audio_sequencepatternentry_patternslot(
						seqpatternentry));
				if (pattern) {
					SeqEditPatternEntry* seqeditpatternentry;

					seqeditpatternentry = seqeditpatternentry_allocinit(
						&self->component, self->view, seqpatternentry,
						psy_audio_orderindex_make(self->trackindex, c),
						self->state);
					if (seqeditpatternentry) {
						psy_ui_component_setalign(
							seqeditpatternentry_base(seqeditpatternentry),
								psy_ui_ALIGN_LEFT);
						psy_list_append(&self->entries, seqeditpatternentry);
					}
				}
				break; }
			case psy_audio_SEQUENCEENTRY_SAMPLE: {
				psy_audio_SequenceSampleEntry* seqsampleentry;
				SeqEditSampleEntry* seqeditsampleentry;

				seqsampleentry = (psy_audio_SequenceSampleEntry*)seqentry;
				seqeditsampleentry = seqeditsampleentry_allocinit(
					&self->component, self->view, seqsampleentry,
					psy_audio_orderindex_make(self->trackindex, c),
					self->state);
				if (seqsampleentry) {
					psy_ui_component_setalign(
						seqeditsampleentry_base(seqeditsampleentry),
						psy_ui_ALIGN_LEFT);
					psy_list_append(&self->entries, seqeditsampleentry);							
				}
				seqeditsampleentry->wavebox.preventselection = TRUE;
				break; }
			case psy_audio_SEQUENCEENTRY_MARKER: {
				psy_audio_SequenceMarkerEntry* seqmarkerentry;				
				SeqEditMarkerEntry* seqeditmarkerentry;

				seqmarkerentry = (psy_audio_SequenceMarkerEntry*)seqentry;
				seqeditmarkerentry = seqeditmarkerentry_allocinit(
					&self->component, self->view, seqmarkerentry,
					psy_audio_orderindex_make(self->trackindex, c),
					self->state);
				if (seqeditmarkerentry) {
					psy_ui_component_setalign(seqeditmarkerentry_base(seqeditmarkerentry),
						psy_ui_ALIGN_LEFT);
					psy_list_append(&self->entries, seqeditmarkerentry);					
				}
				break; }
			default:
				break;
			}
		}
	}
	psy_ui_component_align(&self->component);
}

void seqedittrack_onalign(SeqEditTrack* self)
{		
	psy_List* p;
	psy_List* q;
	psy_ui_Size size;

	if (!workspace_song(self->workspace)) {
		return;
	}
	if (!self->currtrack) {
		return;
	}
	size = psy_ui_component_scrollsize(&self->component);	
	p = self->entries;
	for (p = self->entries, q = self->currtrack->entries;
			p != NULL && q != NULL; psy_list_next(&p), q = q->next) {
		psy_ui_Component* component;
		psy_audio_SequenceEntry* seqentry;		

		component = (psy_ui_Component*)psy_list_entry(p);
		seqentry = (psy_audio_SequenceEntry*)psy_list_entry(q);
		if (seqentry) {			
			psy_ui_component_setposition(component,
				psy_ui_rectangle_make(
					psy_ui_point_make_px(
						seqeditstate_beattopx(self->state,
							psy_audio_sequenceentry_offset(seqentry)),
						0.0),
					psy_ui_size_make(
						psy_ui_value_make_px(
							seqeditstate_beattopx(self->state,
								psy_audio_sequenceentry_length(seqentry))),
						size.height)));
		}
	}	
}

void seqedittrack_onpreferredsize(SeqEditTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_dsp_big_beat_t trackduration;

	trackduration = 0.0;
	if (self->currtrack) {
		trackduration = psy_audio_sequencetrack_duration(self->currtrack,
			&workspace_song(self->workspace)->patterns);
	}
	trackduration += 400.0;
	rv->width = psy_ui_value_make_px(self->state->pxperbeat *
		trackduration);
	rv->height = self->state->lineheight;
}

void seqedittrack_onmousedown(SeqEditTrack* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		if (self->state->dragstatus == SEQEDIT_DRAG_NONE) {
			psy_audio_OrderIndex seqpos;

			seqpos = workspace_sequenceeditposition(self->state->workspace);
			if (self->trackindex != seqpos.track) {
				seqpos.track = self->trackindex;
				seqpos.order = psy_INDEX_INVALID;
				workspace_setseqeditposition(self->state->workspace, seqpos);
			}
		} else {
			psy_ui_component_capture(&self->component);
		}
	}
}

void seqedittrack_onmousemove(SeqEditTrack* self, psy_ui_MouseEvent* ev)
{	
	if (self->state->dragstatus == SEQEDIT_DRAG_NONE ||
		self->state->dragstatus == SEQEDIT_DRAG_REMOVE ||
		self->state->seqentry == NULL) {
		return;
	}
	if (self->state->dragstatus == SEQEDIT_DRAG_START) {
		psy_dsp_big_beat_t dragposition;
		psy_dsp_big_beat_t bpl;

		dragposition = seqeditstate_pxtobeat(self->state, ev->pt.x) -
			psy_audio_sequenceentry_offset(self->state->seqentry);
		bpl = (1.0 / (psy_dsp_big_beat_t)psy_audio_player_lpb(
			workspace_player(self->workspace)));
		/* start drag only if a difference of one line exists */
		if (fabs(dragposition - self->state->dragposition) >= bpl) {			
			self->state->dragstatus =
				(self->state->dragtype == SEQEDIT_DRAGTYPE_REORDER)
				? SEQEDIT_DRAG_REORDER
				: SEQEDIT_DRAG_MOVE;			
		}		
	} else if (self->state->dragstatus == SEQEDIT_DRAG_LENGTH) {
		psy_dsp_big_beat_t dragposition;
			
		dragposition = seqeditstate_quantize(self->state,
			seqeditstate_pxtobeat(self->state, ev->pt.x));
		psy_audio_sequenceentry_setlength(self->state->seqentry,
			psy_max(1.0, dragposition - psy_audio_sequenceentry_offset(
				self->state->seqentry)));
		psy_audio_sequence_reposition_track(
			&workspace_song(self->workspace)->sequence, self->currtrack);
			
	} else if ((self->state->dragtype & SEQEDIT_DRAGTYPE_MOVE) ==
			SEQEDIT_DRAGTYPE_MOVE) {
		psy_dsp_big_beat_t dragposition;

		dragposition = seqeditstate_pxtobeat(self->state, ev->pt.x);
		if (dragposition - (self->state->seqentry->offset -
			self->state->seqentry->repositionoffset) >= 0) {
			dragposition = seqeditstate_quantize(self->state,
				dragposition);
		} else {
			dragposition = self->state->seqentry->offset -
				self->state->seqentry->repositionoffset;
		}
		self->state->seqentry->repositionoffset = dragposition -
			(self->state->seqentry->offset -
				self->state->seqentry->repositionoffset);
		self->state->seqentry->offset = dragposition;
		psy_audio_sequence_reposition_track(
			&workspace_song(self->workspace)->sequence, self->currtrack);
	}	
}

void seqedittrack_onmouseup(SeqEditTrack* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);	
	if (self->state->dragstatus == SEQEDIT_DRAG_REORDER) {
		psy_audio_exclusivelock_enter();
		psy_audio_sequence_reorder(seqeditstate_sequence(self->state),
			self->state->dragseqpos, self->state->cursorposition);
		psy_audio_exclusivelock_leave();
		psy_ui_mouseevent_stop_propagation(ev);
	}
	self->state->seqentry = NULL;
	self->state->dragstatus = SEQEDIT_DRAG_NONE;
}

void seqedittrack_onmousedoubleclick(SeqEditTrack* self, psy_ui_MouseEvent* ev)
{
	if (psy_audio_orderindex_invalid(&self->state->dragseqpos)) {
		sequencecmds_newentry(self->state->cmds, self->state->inserttype);
		psy_ui_mouseevent_stop_propagation(ev);
	}	
}

void seqedittrack_onsequenceinsert(SeqEditTrack* self,
	psy_audio_Sequence* sender, psy_audio_OrderIndex* index)
{
	if (self->trackindex == index->track) {
		seqedittrack_build(self);
	}
}

void seqedittrack_onsequenceremove(SeqEditTrack* self,
	psy_audio_Sequence* sender, psy_audio_OrderIndex* index)
{
	if (self->trackindex == index->track) {
		seqedittrack_build(self);
	}
}

void seqedittrack_onsequencetrackreposition(SeqEditTrack* self,
	psy_audio_Sequence* sender, uintptr_t trackidx)
{
	seqedittrack_onalign(self);
	psy_ui_component_invalidate(&self->component);
}

/* SeqEditTrackDesc */
/* prototypes */
static void seqedittrackdesc_ondestroy(SeqEditTrackDesc*);
static void seqedittrackdesc_onsequenceselectionselect(SeqEditTrackDesc*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqedittrackdesc_onsequenceselectiondeselect(SeqEditTrackDesc*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqedittrackdesc_onsequenceselectionupdate(SeqEditTrackDesc*,
	psy_audio_SequenceSelection*);
static void seqedittrackdesc_onnewtrack(SeqEditTrackDesc*,
	psy_ui_Button* sender);
static void seqedittrackdesc_ondeltrack(SeqEditTrackDesc*,
	TrackBox* sender);
static void seqedittrackdesc_build(SeqEditTrackDesc*);
static void seqedittrackdesc_ondragstart(SeqEditTrackDesc*,
	psy_ui_Component* sender, psy_ui_DragEvent*);
static void seqedittrackdesc_ondragover(SeqEditTrackDesc*,
	psy_ui_DragEvent*);
static void seqedittrackdesc_ondragdrop(SeqEditTrackDesc*,
	psy_ui_DragEvent*);
static void seqedittrackdesc_onresize(SeqEditTrackDesc*,
	psy_ui_Component* sender, double* offset);
/* vtable */
static psy_ui_ComponentVtable seqedittrackdesc_vtable;
static bool seqedittrackdesc_vtable_initialized = FALSE;

static void seqedittrackdesc_vtable_init(SeqEditTrackDesc* self)
{
	if (!seqedittrackdesc_vtable_initialized) {
		seqedittrackdesc_vtable = *(self->component.vtable);		
		seqedittrackdesc_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqedittrackdesc_ondestroy;		
		seqedittrackdesc_vtable.ondragover =
			(psy_ui_fp_component_ondragover)
			seqedittrackdesc_ondragover;
		seqedittrackdesc_vtable.ondrop =
			(psy_ui_fp_component_ondrop)
			seqedittrackdesc_ondragdrop;
		seqedittrackdesc_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqedittrackdesc_vtable;
}
/*  implementation */
void seqedittrackdesc_init(SeqEditTrackDesc* self, psy_ui_Component* parent,
	SeqEditState* state, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);	
	seqedittrackdesc_vtable_init(self);
	psy_signal_init(&self->signal_resize);
	self->workspace = workspace;
	self->state = state;
	psy_ui_component_doublebuffer(&self->component);		
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());	
	seqedittrackdesc_build(self);	
	psy_signal_connect(&workspace->sequenceselection.signal_select, self,
		seqedittrackdesc_onsequenceselectionselect);
	psy_signal_connect(&workspace->sequenceselection.signal_deselect, self,
		seqedittrackdesc_onsequenceselectiondeselect);
	psy_signal_connect(&workspace->sequenceselection.signal_update, self,
		seqedittrackdesc_onsequenceselectionupdate);	
}

void seqedittrackdesc_ondestroy(SeqEditTrackDesc* self)
{
	psy_signal_dispose(&self->signal_resize);
}

void seqedittrackdesc_onsequenceselectionselect(SeqEditTrackDesc* self,
	psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	psy_ui_Component* track;

	track = psy_ui_component_at(&self->component, index->track);
	if (track) {
		psy_ui_Component* trackbox;
		
		trackbox = psy_ui_component_at(track, 0);
		if (trackbox) {
			psy_ui_component_addstylestate(trackbox, psy_ui_STYLESTATE_SELECT);
		}
	}
}

void seqedittrackdesc_onsequenceselectiondeselect(SeqEditTrackDesc* self,
	psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	psy_ui_Component* track;

	track = psy_ui_component_at(&self->component, index->track);
	if (track) {
		psy_ui_Component* trackbox;

		trackbox = psy_ui_component_at(track, 0);
		if (trackbox) {
			psy_ui_component_removestylestate(trackbox,
				psy_ui_STYLESTATE_SELECT);
		}
	}
}

void seqedittrackdesc_onsequenceselectionupdate(SeqEditTrackDesc* self,
	psy_audio_SequenceSelection* selection)
{
	psy_ui_component_invalidate(&self->component);
}

void seqedittrackdesc_onnewtrack(SeqEditTrackDesc* self,
	psy_ui_Button* sender)
{
	self->state->cmd = SEQEDTCMD_NEWTRACK;
}

void seqedittrackdesc_ondeltrack(SeqEditTrackDesc* self,
	TrackBox* sender)
{
	self->state->cmd = SEQEDTCMD_DELTRACK;
	self->state->dragseqpos.track = trackbox_trackindex(sender);
}

void seqedittrackdesc_build(SeqEditTrackDesc* self)
{
	psy_audio_Sequence* sequence;

	psy_ui_component_clear(&self->component);	
	sequence = seqeditstate_sequence(self->state);
	if (sequence) {
		psy_audio_SequenceTrackNode* t;
		uintptr_t c;
		psy_ui_Button* newtrack;
		psy_audio_OrderIndex firstsel;

		firstsel = psy_audio_sequenceselection_first(
			&self->state->cmds->workspace->sequenceselection);
		for (t = sequence->tracks, c = 0; t != NULL; t = t->next, ++c) {
			SequenceTrackBox* trackbox;
			psy_audio_SequenceTrack* track;
			
			track = (psy_audio_SequenceTrack*)t->entry;			
			trackbox = sequencetrackbox_allocinit(&self->component,
				&self->component, seqeditstate_sequence(self->state), c,
				self->state->edit);
			if (trackbox) {				
				psy_ui_component_show(&trackbox->trackbox.resize);
				psy_ui_component_setminimumsize(
					sequencetrackbox_base(trackbox),
					psy_ui_size_make_em(0.0, 2.0));
				if (track->height != 0.0) {
					psy_ui_component_setpreferredheight(&trackbox->component,
						psy_ui_value_make_eh(track->height));
				}
				sequencetrackbox_showtrackname(trackbox);
			}
			if (trackbox) {				
				/* psy_ui_component_setstyletype_hover(
					&trackbox->trackbox.component,
					STYLE_SEQEDT_DESCITEM_HOVER); */
				psy_ui_component_setstyletype_select(
					&trackbox->trackbox.component,
					STYLE_SEQEDT_DESCITEM_SELECTED);
				psy_ui_component_setstyletype(
					&trackbox->trackbox.track.component,
					STYLE_SEQEDT_TRACK_NUMBER);				
				psy_signal_connect(&trackbox->trackbox.signal_close, self,
					seqedittrackdesc_ondeltrack);				
				trackbox->trackbox.track.component.draggable = TRUE;
				psy_signal_connect(
					&trackbox->trackbox.track.component.signal_dragstart,
					self, seqedittrackdesc_ondragstart);
				if (c == firstsel.track) {					
					psy_ui_component_addstylestate(
						&trackbox->trackbox.component,
						psy_ui_STYLESTATE_SELECT);
				}
				psy_signal_connect(&trackbox->signal_resize, self,
					seqedittrackdesc_onresize);
			}
		}
		newtrack = psy_ui_button_allocinit(&self->component, &self->component);
		if (newtrack) {
			psy_ui_button_settext(newtrack, "seqview.new-trk");
			psy_ui_component_setminimumsize(psy_ui_button_base(newtrack),
				psy_ui_size_make_em(0.0, 2.0));
			newtrack->stoppropagation = FALSE;
			psy_signal_connect(&newtrack->signal_clicked, self,
				seqedittrackdesc_onnewtrack);
		}		
	}
	psy_ui_component_align(&self->component);
}

void seqedittrackdesc_onresize(SeqEditTrackDesc* self,
	psy_ui_Component* sender, double* offset)
{		
	psy_ui_RealSize size;
	double height;
	const psy_ui_TextMetric* tm;

	size = psy_ui_component_size_px(sender);
	tm = psy_ui_component_textmetric(&self->component);
	height = (size.height + *offset) / tm->tmHeight;	
	sequencecmds_update(self->state->cmds);
	psy_audio_sequence_settrackheight(self->state->cmds->sequence,
		(uintptr_t)sender->id, height);
	psy_ui_component_setpreferredheight(sender,
		psy_ui_value_make_eh(height));	
	psy_signal_emit(&self->signal_resize, self, 2, (void*)sender->id,
		&height);
}

void seqedittrackdesc_ondragstart(SeqEditTrackDesc* self,
	psy_ui_Component* sender, psy_ui_DragEvent* ev)
{
	ev->dataTransfer = psy_property_allocinit_key(NULL);	
	psy_property_append_int(ev->dataTransfer,
		"seqedittrack", sender->id, 0, 0);
	psy_ui_dragevent_prevent_default(ev);
}

void seqedittrackdesc_ondragover(SeqEditTrackDesc* self, psy_ui_DragEvent* ev)
{
	psy_ui_dragevent_prevent_default(ev);
}

void seqedittrackdesc_ondragdrop(SeqEditTrackDesc* self, psy_ui_DragEvent* ev)
{
	psy_ui_dragevent_prevent_default(ev);
	if (ev->dataTransfer && self->state->workspace->song) {
		intptr_t trackid;

		trackid = psy_property_at_int(ev->dataTransfer, "seqedittrack", -1);
		if (trackid != -1) {
			uintptr_t index;

			psy_ui_component_intersect(&self->component, ev->mouse.pt, &index);			
			if (index != trackid) {
				psy_audio_sequence_swaptracks(
					&self->state->workspace->song->sequence, trackid, index);
			}
		}
	}
}

/* SeqEditorTracks */
/* prototypes */
static void seqeditortracks_onmousedown(SeqEditorTracks*, psy_ui_MouseEvent*);
static void seqeditortracks_onmousemove(SeqEditorTracks*, psy_ui_MouseEvent*);
static void seqeditortracks_onmouseup(SeqEditorTracks*, psy_ui_MouseEvent*);
static void seqeditortracks_onmouseenter(SeqEditorTracks*);
static void seqeditortracks_onmouseleave(SeqEditorTracks*);
static void seqeditortracks_build(SeqEditorTracks*);
static void seqeditortracks_onsequenceselectionselect(SeqEditorTracks*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortracks_onsequenceselectiondeselect(SeqEditorTracks*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortracks_onsequenceselectionupdate(SeqEditorTracks*,
	psy_audio_SequenceSelection*);
static void seqeditortracks_onalign(SeqEditorTracks*);
static void seqeditortracks_ontimer(SeqEditorTracks*, uintptr_t timerid);
static void seqeditortracks_oncursorchanged(SeqEditorTracks*, SeqEditState*);
static void seqeditortracks_updatecursorlineposition(SeqEditorTracks*);
static void seqeditortracks_updateseqeditlineposition(SeqEditorTracks*);
static void seqeditortracks_outputstatusposition(SeqEditorTracks*, double x);
/* vtable */
static psy_ui_ComponentVtable seqeditortracks_vtable;
static bool seqeditortracks_vtable_initialized = FALSE;

static void seqeditortracks_vtable_init(SeqEditorTracks* self)
{
	if (!seqeditortracks_vtable_initialized) {
		seqeditortracks_vtable = *(self->component.vtable);		
		seqeditortracks_vtable.onalign =
			(psy_ui_fp_component_onalign)
			seqeditortracks_onalign;
		seqeditortracks_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			seqeditortracks_ontimer;
		seqeditortracks_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditortracks_onmousedown;
		seqeditortracks_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			seqeditortracks_onmouseup;
		seqeditortracks_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditortracks_onmousemove;
		seqeditortracks_vtable.onmouseenter =
			(psy_ui_fp_component_onmouseenter)
			seqeditortracks_onmouseenter;
		seqeditortracks_vtable.onmouseleave =
			(psy_ui_fp_component_onmouseleave)
			seqeditortracks_onmouseleave;		
		seqeditortracks_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqeditortracks_vtable;
}
/* implementation */
void seqeditortracks_init(SeqEditorTracks* self, psy_ui_Component* parent,
	SeqEditState* state, Workspace* workspace)
{
	self->workspace = workspace;
	self->state = state;
	self->playline = NULL;
	self->cursorline = NULL;
	self->seqeditposline = NULL;	
	psy_ui_component_init(&self->component, parent, NULL);
	seqeditortracks_vtable_init(self);	
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setscrollstep(&self->component,
		psy_ui_size_make_em(8.0, 0.0));	
	psy_ui_component_setwheelscroll(&self->component, 1);	
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());
	psy_signal_connect(&workspace->sequenceselection.signal_select, self,
		seqeditortracks_onsequenceselectionselect);
	psy_signal_connect(&workspace->sequenceselection.signal_deselect, self,
		seqeditortracks_onsequenceselectiondeselect);
	psy_signal_connect(&workspace->sequenceselection.signal_update, self,
		seqeditortracks_onsequenceselectionupdate);	
	seqeditortracks_build(self);
	psy_signal_connect(&self->state->signal_cursorchanged, self,
		seqeditortracks_oncursorchanged);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void seqeditortracks_build(SeqEditorTracks* self)
{
	psy_audio_Sequence* sequence;
	psy_ui_Colour cursorcolour;

	self->playline = NULL;
	psy_ui_component_clear(&self->component);
	sequence = seqeditstate_sequence(self->state);
	cursorcolour = psy_ui_style(STYLE_SEQEDT_RULER_CURSOR)->colour;
	if (sequence) {
		psy_audio_SequenceTrackNode* t;
		uintptr_t c;
		psy_ui_Component* spacer;		

		for (t = sequence->tracks, c = 0; t != NULL;
				psy_list_next(&t), ++c) {
			psy_audio_SequenceTrack* seqtrack;
			SeqEditTrack* seqedittrack;
						
			seqtrack = (psy_audio_SequenceTrack*)t->entry;
			seqedittrack = seqedittrack_allocinit(&self->component,
				&self->component, self->state, self->workspace);			
			if (seqedittrack) {
				seqedittrack_setsequencetrack(seqedittrack,
					t, (psy_audio_SequenceTrack*)t->entry, c);				
			}
		}
		spacer = psy_ui_component_allocinit(&self->component, &self->component);
		psy_ui_component_setminimumsize(spacer,
			psy_ui_size_make_em(10.0, 2.0));		
	}
	self->playline = seqeditorplayline_allocinit(&self->component,
		&self->component, self->state);
	if (self->playline) {
		psy_ui_component_setalign(&self->playline->seqeditorline.component,
			psy_ui_ALIGN_NONE);
		psy_ui_component_setmargin(&self->playline->seqeditorline.component,
			psy_ui_margin_zero());
		psy_ui_component_setbackgroundcolour(
			&self->playline->seqeditorline.component, cursorcolour);
		seqeditorplayline_update(self->playline);
	}
	self->cursorline = seqeditorline_allocinit(&self->component,
		&self->component, self->state);
	if (self->cursorline) {
		psy_ui_component_setalign(&self->cursorline->component,
			psy_ui_ALIGN_NONE);
		psy_ui_component_setmargin(&self->cursorline->component,
			psy_ui_margin_zero());
		psy_ui_component_hide(&self->cursorline->component);
		psy_ui_component_setbackgroundcolour(&self->cursorline->component,
			cursorcolour);
	}
	self->seqeditposline = seqeditorline_allocinit(&self->component,
		&self->component, self->state);
	if (self->seqeditposline) {
		psy_ui_component_setalign(&self->seqeditposline->component,
			psy_ui_ALIGN_NONE);
		psy_ui_component_setmargin(&self->seqeditposline->component,
			psy_ui_margin_zero());
		psy_ui_component_setbackgroundcolour(&self->seqeditposline->component,
			cursorcolour);
		seqeditortracks_updateseqeditlineposition(self);
	}
}

void seqeditortracks_onsequenceselectionupdate(SeqEditorTracks* self,
	psy_audio_SequenceSelection* selection)
{		
	seqeditortracks_updateseqeditlineposition(self);	
}

void seqeditortracks_onsequenceselectionselect(SeqEditorTracks* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	seqeditortracks_updateseqeditlineposition(self);	
}

void seqeditortracks_onsequenceselectiondeselect(SeqEditorTracks* self,
	psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	if (self->seqeditposline) {
		psy_audio_SequenceEntry* seqentry;
				
		seqentry = psy_audio_sequence_entry(
			seqeditstate_sequence(self->state),
			selection->editposition);
		if (seqentry) {
			seqeditorline_updateposition(self->seqeditposline,
				seqentry->offset);
		}
	}
	psy_ui_component_invalidate(&self->component);
}

void seqeditortracks_onalign(SeqEditorTracks* self)
{	
	if (self->playline) {
		seqeditorplayline_update(self->playline);
	}
	seqeditortracks_updatecursorlineposition(self);
	seqeditortracks_updateseqeditlineposition(self);
}

void seqeditortracks_oncursorchanged(SeqEditorTracks* self,
	SeqEditState* sender)
{
	seqeditortracks_updatecursorlineposition(self);
}

void seqeditortracks_updatecursorlineposition(SeqEditorTracks* self)
{
	if (self->cursorline) {
		if (self->state->cursoractive && !psy_ui_component_visible(
				&self->cursorline->component)) {
			psy_ui_component_show(&self->cursorline->component);
		} else if (!self->state->cursoractive &&
				psy_ui_component_visible(&self->cursorline->component)) {
			psy_ui_component_hide(&self->cursorline->component);
			psy_ui_component_invalidate(&self->component);
			return;
		}
		seqeditorline_updateposition(self->cursorline,
			self->state->cursorposition);		
	}
}

void seqeditortracks_updateseqeditlineposition(SeqEditorTracks* self)
{
	if (self->seqeditposline) {
		psy_audio_SequenceEntry* seqentry;

		seqentry = psy_audio_sequence_entry(
			seqeditstate_sequence(self->state),
			self->workspace->sequenceselection.editposition);
		if (seqentry) {
			seqeditorline_updateposition(self->seqeditposline,
				seqentry->offset);
		}
	}
}

void seqeditortracks_ontimer(SeqEditorTracks* self, uintptr_t timerid)
{		
	if (psy_audio_player_playing(workspace_player(
			self->workspace))) {
		seqeditorplayline_update(self->playline);
	}
}

void seqeditortracks_onmousedown(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	if (self->state->dragstatus == SEQEDIT_DRAG_REMOVE) {
		sequencecmds_delentry(self->state->cmds);
		self->state->dragstatus = SEQEDIT_DRAG_NONE;
		psy_ui_mouseevent_stop_propagation(ev);
	} else if ((self->state->dragtype & SEQEDIT_DRAGTYPE_REORDER)
			== SEQEDIT_DRAGTYPE_REORDER) {
		if (self->cursorline) {
			psy_ui_component_hide(&self->cursorline->component);
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void seqeditortracks_onmousemove(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{	
	seqeditortracks_outputstatusposition(self, ev->pt.x);
	if (self->state->dragstatus && (self->state->dragtype & SEQEDIT_DRAGTYPE_MOVE)
			== SEQEDIT_DRAGTYPE_MOVE) {
		seqeditortracks_updateseqeditlineposition(self);
	}
}

void seqeditortracks_onmouseup(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);	
	self->state->cmd = SEQEDTCMD_NONE;
	self->state->seqentry = NULL;
	self->state->dragseqpos = psy_audio_orderindex_zero();
	self->state->dragstatus = SEQEDIT_DRAG_NONE;
	psy_ui_mouseevent_stop_propagation(ev);
}

void seqeditortracks_onmouseenter(SeqEditorTracks* self)
{
	self->state->cursoractive = TRUE;	
	psy_signal_emit(&self->state->signal_cursorchanged, self->state, 0);
}

void seqeditortracks_onmouseleave(SeqEditorTracks* self)
{
	self->state->cursoractive = FALSE;
	if (self->cursorline) {
		psy_ui_component_hide(&self->cursorline->component);
		psy_ui_component_invalidate(&self->component);
	}
	psy_signal_emit(&self->state->signal_cursorchanged, self->state, 0);
}

void seqeditortracks_outputstatusposition(SeqEditorTracks* self, double x)
{	
	char text[256];

	seqeditstate_setcursor(self->state, seqeditstate_quantize(self->state,
		seqeditstate_pxtobeat(self->state, x)));
	psy_snprintf(text, 256, "Sequence Position %.3fb",
		(float)self->state->cursorposition);
	workspace_outputstatus(self->workspace, text);
}

/* SeqEditorHeaderBar */
/* implementation */
void seqeditorheaderbar_init(SeqEditorHeaderBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(40.0, 4.0));	
	psy_ui_component_init(&self->top, &self->component, NULL);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	zoombox_init(&self->hzoom, &self->top);	
	psy_ui_component_setalign(&self->hzoom.component, psy_ui_ALIGN_LEFT);	
	psy_ui_button_init_text(&self->timesig, &self->top, NULL, "Hide TimeSig");
	psy_ui_button_seticon(&self->timesig, psy_ui_ICON_LESS);
	psy_ui_component_setalign(&self->timesig.component, psy_ui_ALIGN_RIGHT);
}

/* SeqEditToolBar */
static void seqedittoolbar_oninserttypeselchange(SeqEditToolBar*,
	psy_ui_Component* sender, intptr_t index);
static void seqeditortoolbar_ondragmodemove(SeqEditToolBar*,
	psy_ui_Component* sender);
static void seqeditortoolbar_ondragmodereorder(SeqEditToolBar*,
	psy_ui_Component* sender);
static void seqeditortoolbar_onsequenceselectionselect(SeqEditToolBar*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortoolbar_onsequenceselectiondeselect(SeqEditToolBar*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortoolbar_onassignsample(SeqEditToolBar*,
	psy_ui_Component* sender);
static void seqeditortoolbar_onusesamplerindex(SeqEditToolBar*,
	psy_ui_Component* sender);
static void seqeditortoolbar_onsamplerindexchange(SeqEditToolBar*,
	IntEdit* sender);
static void seqeditortoolbar_onconfigure(SeqEditToolBar*,
	psy_ui_Button* sender);
static psy_audio_SequenceSampleEntry* seqeditortoolbar_sampleentry(
	SeqEditToolBar* self);

/* implenentation */
void seqedittoolbar_init(SeqEditToolBar* self, psy_ui_Component* parent,
	SeqEditState* state)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletype(&self->component, STYLE_SEQEDT_TOOLBAR);		
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 0.5, 0.0, 1.0));		
	self->state = state;
	psy_ui_button_init_text(&self->move, &self->component, NULL,
		"seqedit.move");
	psy_ui_button_init_text(&self->reorder, &self->component, NULL,
		"seqedit.reorder");
	seqeditortoolbar_setdragtype(self, self->state->dragtype);
	psy_ui_label_init_text(&self->desctype, seqedittoolbar_base(self), NULL,
		"Insert");	
	psy_ui_combobox_init(&self->inserttype, seqedittoolbar_base(self), NULL);
	psy_ui_combobox_setcharnumber(&self->inserttype, 12.0);
	psy_ui_combobox_addtext(&self->inserttype, "Pattern");
	psy_ui_combobox_addtext(&self->inserttype, "Marker");
	psy_ui_combobox_addtext(&self->inserttype, "Sample");
	psy_ui_combobox_setcursel(&self->inserttype, 0);
	/* expand */
	psy_ui_button_init(&self->expand, &self->component, NULL);
	psy_ui_button_loadresource(&self->expand, IDB_EXPAND_DARK,
		psy_ui_colour_white());
	psy_ui_component_setalign(psy_ui_button_base(&self->expand),
		psy_ui_ALIGN_RIGHT);	
	/* configure */
	psy_ui_button_init_connect(&self->configure, &self->component, NULL,
		self, seqeditortoolbar_onconfigure);
	psy_ui_button_loadresource(&self->configure, IDB_SETTINGS_DARK,
		psy_ui_colour_white());
	psy_ui_component_setalign(psy_ui_button_base(&self->configure),
		psy_ui_ALIGN_RIGHT);
	/* assign sample */
	psy_ui_button_init_text_connect(&self->assignsample, &self->component,
		NULL, "Assign Sample", self, seqeditortoolbar_onassignsample);
	psy_ui_component_hide(psy_ui_button_base(&self->assignsample));
	psy_ui_checkbox_init_text(&self->usesamplerindex, &self->component, "Define Sampler");
	psy_signal_connect(&self->usesamplerindex.signal_clicked, self,
		seqeditortoolbar_onusesamplerindex);
	intedit_init_connect(&self->samplerindex, &self->component, "Index",
		0x3E, 0, 0x3F, self, seqeditortoolbar_onsamplerindexchange);	
	psy_ui_component_hide(psy_ui_checkbox_base(&self->usesamplerindex));
	psy_ui_component_hide(intedit_base(&self->samplerindex));	
	psy_signal_connect(&self->inserttype.signal_selchanged, self,
		seqedittoolbar_oninserttypeselchange);
	psy_signal_connect(&self->move.signal_clicked, self,
		seqeditortoolbar_ondragmodemove);
	psy_signal_connect(&self->reorder.signal_clicked, self,
		seqeditortoolbar_ondragmodereorder);
	psy_signal_connect(&state->workspace->sequenceselection.signal_select,
		self, seqeditortoolbar_onsequenceselectionselect);
	psy_signal_connect(&state->workspace->sequenceselection.signal_deselect,
		self, seqeditortoolbar_onsequenceselectiondeselect);
}

void seqeditortoolbar_ondragmodemove(SeqEditToolBar* self,
	psy_ui_Component* sender)
{	
	seqeditortoolbar_setdragtype(self, SEQEDIT_DRAGTYPE_MOVE);
}

void seqeditortoolbar_ondragmodereorder(SeqEditToolBar* self,
	psy_ui_Component* sender)
{	
	seqeditortoolbar_setdragtype(self, SEQEDIT_DRAGTYPE_REORDER);
}

void seqeditortoolbar_setdragtype(SeqEditToolBar* self, SeqEditorDragType type)
{
	self->state->dragtype = type;
	switch (type) {
	case SEQEDIT_DRAGTYPE_MOVE:
		psy_ui_button_highlight(&self->move);
		psy_ui_button_disablehighlight(&self->reorder);
		break;
	case SEQEDIT_DRAGTYPE_REORDER:
		psy_ui_button_highlight(&self->reorder);
		psy_ui_button_disablehighlight(&self->move);
		break;
	default:
		break;
	}
}

void seqedittoolbar_oninserttypeselchange(SeqEditToolBar* self,
	psy_ui_Component* sender, intptr_t index)
{
	switch (index) {
	case 0:
		self->state->inserttype = psy_audio_SEQUENCEENTRY_PATTERN;
		break;
	case 1:
		self->state->inserttype = psy_audio_SEQUENCEENTRY_MARKER;
		break;
	case 2:
		self->state->inserttype = psy_audio_SEQUENCEENTRY_SAMPLE;
		break;
	default:
		self->state->inserttype = psy_audio_SEQUENCEENTRY_PATTERN;
		break;
	}
}

void seqeditortoolbar_onsequenceselectionselect(SeqEditToolBar* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	psy_audio_SequenceEntry* entry;
	psy_audio_SequenceSampleEntry* sampleentry;

	entry = psy_audio_sequence_entry(seqeditstate_sequence(self->state),
		*index);
	if (entry && entry->type == psy_audio_SEQUENCEENTRY_SAMPLE) {
		if (!psy_ui_component_visible(psy_ui_button_base(&self->assignsample))) {
			psy_ui_component_show(psy_ui_checkbox_base(&self->usesamplerindex));
			psy_ui_component_show(intedit_base(&self->samplerindex));			
			psy_ui_component_show_align(psy_ui_button_base(&self->assignsample));			
		}
	} else {
		if (psy_ui_component_visible(psy_ui_button_base(&self->assignsample))) {
			psy_ui_component_hide(psy_ui_checkbox_base(&self->usesamplerindex));
			psy_ui_component_hide(intedit_base(&self->samplerindex));			
			psy_ui_component_hide_align(psy_ui_button_base(&self->assignsample));
		}
	}	
	sampleentry = seqeditortoolbar_sampleentry(self);
	if (sampleentry) {
		if (sampleentry->samplerindex != psy_INDEX_INVALID) {
			psy_ui_checkbox_check(&self->usesamplerindex);
			intedit_setvalue(&self->samplerindex,
				(int)sampleentry->samplerindex);			
			psy_ui_component_show_align(intedit_base(&self->samplerindex));
		} else {			
			psy_ui_component_hide_align(intedit_base(&self->samplerindex));
		}
	}
}

void seqeditortoolbar_onsequenceselectiondeselect(SeqEditToolBar* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{

}

void seqeditortoolbar_onassignsample(SeqEditToolBar* self,
	psy_ui_Component* sender)
{
	psy_audio_SequenceSampleEntry* sampleentry;

	sampleentry = seqeditortoolbar_sampleentry(self);
	if (sampleentry) {
		psy_audio_OrderIndex editposition;
		psy_audio_SampleIndex sampleindex;
		psy_audio_SequenceTrack* track;
		psy_audio_Sequence* sequence;

		sequence = seqeditstate_sequence(self->state);
		sampleindex = self->state->cmds->workspace->song->samples.selected;
		editposition = seqeditstate_editposition(self->state);
		psy_audio_sequencesampleentry_setsampleslot(sampleentry, sampleindex);
		track = psy_audio_sequence_track_at(sequence, editposition.track);
		if (track) {
			psy_audio_sequence_reposition_track(sequence, track);
		}
	}	
}

void seqeditortoolbar_onusesamplerindex(SeqEditToolBar* self,
	psy_ui_Component* sender)
{
	psy_audio_SequenceSampleEntry* sampleentry;

	sampleentry = seqeditortoolbar_sampleentry(self);
	if (sampleentry) {
		if (!psy_ui_checkbox_checked(&self->usesamplerindex)) {			
			psy_audio_sequencesampleentry_setsamplerindex(sampleentry,
				psy_INDEX_INVALID);
		} else {
			psy_audio_sequencesampleentry_setsamplerindex(sampleentry,
				intedit_value(&self->samplerindex));			
		}
		if (sampleentry->samplerindex != psy_INDEX_INVALID) {						
			psy_ui_component_show_align(intedit_base(&self->samplerindex));			
		} else {			
			psy_ui_component_hide_align(intedit_base(&self->samplerindex));
		}
	}
}

void seqeditortoolbar_onsamplerindexchange(SeqEditToolBar* self,
	IntEdit* sender)
{
	psy_audio_SequenceSampleEntry* sampleentry;

	sampleentry = seqeditortoolbar_sampleentry(self);
	if (sampleentry && (psy_ui_checkbox_checked(&self->usesamplerindex))) {
		psy_audio_sequencesampleentry_setsamplerindex(sampleentry,
			intedit_value(&self->samplerindex));
	}
}

void seqeditortoolbar_onconfigure(SeqEditToolBar* self, psy_ui_Button* sender)
{
	workspace_selectview(self->state->workspace, VIEW_ID_SETTINGSVIEW, 11, 0);
}

psy_audio_SequenceSampleEntry* seqeditortoolbar_sampleentry(SeqEditToolBar* self)
{
	psy_audio_Sequence* sequence;

	sequence = seqeditstate_sequence(self->state);
	if (sequence) {		
		psy_audio_SequenceEntry* seqentry;
				
		seqentry = psy_audio_sequence_entry(sequence,
			seqeditstate_editposition(self->state));
		if (seqentry && seqentry->type == psy_audio_SEQUENCEENTRY_SAMPLE) {
			return (psy_audio_SequenceSampleEntry*)seqentry;			 
		}
	}
	return NULL;
}

/* SeqEditor */
/* prototypes */
static void seqeditor_ondestroy(SeqEditor*);
static void seqeditor_onsongchanged(SeqEditor*, Workspace*, int flag,
	psy_audio_Song*);
static void seqeditor_updatesong(SeqEditor*);
static void seqeditor_build(SeqEditor*);
static void seqeditor_ontracksscroll(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_onconfigure(SeqEditor*, GeneralConfig* sender,
	psy_Property*);
static void seqeditor_onhzoom(SeqEditor*, ZoomBox* sender);
static void seqeditor_onvzoom(SeqEditor*, ZoomBox* sender);
static void seqeditor_updatescrollstep(SeqEditor*);
static void seqeditor_onclear(SeqEditor*, psy_audio_Sequence*);
static void seqeditor_ontrackinsert(SeqEditor*, psy_audio_Sequence*,
	uintptr_t trackidx);
static void seqeditor_ontrackremove(SeqEditor*, psy_audio_Sequence*,
	uintptr_t trackidx);
static void seqeditor_ontrackswap(SeqEditor*,
	psy_audio_Sequence* sender, uintptr_t first, uintptr_t second);
static void seqeditor_onmousemove(SeqEditor*, psy_ui_MouseEvent*);
static void seqeditor_onmouseup(SeqEditor*, psy_ui_MouseEvent*);
static void seqeditor_ontoggleexpand(SeqEditor*, psy_ui_Button* sender);
static void seqeditor_ontrackresize(SeqEditor*, psy_ui_Component* sender,
	uintptr_t trackid, double* height);
static void seqeditor_ontoggletimesig(SeqEditor*, psy_ui_Button* sender);
/* vtable */
static psy_ui_ComponentVtable seqeditor_vtable;
static bool seqeditor_vtable_initialized = FALSE;

static void seqeditor_vtable_init(SeqEditor* self)
{
	if (!seqeditor_vtable_initialized) {
		seqeditor_vtable = *(self->component.vtable);
		seqeditor_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqeditor_ondestroy;
		seqeditor_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditor_onmousemove;
		seqeditor_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			seqeditor_onmouseup;

		seqeditor_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqeditor_vtable;
}
/* implementation */
void seqeditor_init(SeqEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	seqeditor_vtable_init(self);	
	psy_ui_component_setstyletype(&self->component, STYLE_SEQEDT);	
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);
	psy_ui_edit_init(&self->edit, &self->component);
	psy_ui_component_hide(psy_ui_edit_base(&self->edit));
	sequencecmds_init(&self->cmds, workspace);
	seqeditstate_init(&self->state, &self->cmds, &self->edit, &self->component);	
	/* toolbar */
	seqedittoolbar_init(&self->toolbar, &self->component, &self->state);
	psy_ui_component_setalign(&self->toolbar.component, psy_ui_ALIGN_TOP);	
	/* spacer */
	psy_ui_component_init(&self->spacer, &self->component, NULL);
	psy_ui_component_setstyletype(&self->spacer, STYLE_SEQEDT_SPACER);
	psy_ui_component_setalign(&self->spacer, psy_ui_ALIGN_TOP);
	psy_ui_component_setpreferredsize(&self->spacer,
		psy_ui_size_make_em(0.0, 0.25));
	/* left */
	psy_ui_component_init(&self->left, &self->component, NULL);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);	
	psy_ui_component_setstyletype(&self->left, STYLE_SEQEDT_LEFT);
	/* SeqEditorHeaderBar */
	seqeditorheaderbar_init(&self->headerbar, &self->left);
	psy_ui_component_setalign(&self->headerbar.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->headerbar.hzoom.signal_changed, self,
		seqeditor_onhzoom);
	/* track description */
	psy_ui_component_init(&self->trackdescpane, &self->left, NULL);
	psy_ui_component_setalign(&self->trackdescpane, psy_ui_ALIGN_CLIENT);
	seqedittrackdesc_init(&self->trackdesc, &self->trackdescpane,
		&self->state, workspace);
	psy_signal_connect(&self->trackdesc.signal_resize, self,
		seqeditor_ontrackresize);
	psy_ui_component_setalign(&self->trackdesc.component,
		psy_ui_ALIGN_HCLIENT);	
	zoombox_init(&self->vzoom, &self->left);
	psy_ui_component_setalign(&self->vzoom.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->vzoom.signal_changed, self, seqeditor_onvzoom);	
	/* ruler */
	psy_ui_component_init_align(&self->rulerpane, &self->component, NULL,
		psy_ui_ALIGN_TOP);
	seqeditruler_init(&self->ruler, &self->rulerpane, &self->state);
	psy_ui_component_setalign(&self->ruler.component, psy_ui_ALIGN_FIXED);	
	/* connect expand */
	psy_signal_connect(&self->toolbar.expand.signal_clicked, self,
		seqeditor_ontoggleexpand);
	self->expanded = FALSE;
	/* tracks */
	seqeditortracks_init(&self->tracks, &self->component, &self->state,
		workspace);
	psy_ui_scroller_init(&self->scroller, &self->tracks.component,
		&self->component, NULL);	
	psy_ui_component_setalign(&self->tracks.component, psy_ui_ALIGN_FIXED);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
	psy_ui_component_setpreferredsize(&self->component, psy_ui_size_make(
		psy_ui_value_make_ew(20.0), psy_ui_value_make_ph(0.30)));
	seqeditor_updatesong(self);
	psy_signal_connect(&self->state.workspace->signal_songchanged, self,
		seqeditor_onsongchanged);
	psy_signal_connect(&self->tracks.component.signal_scroll, self,
		seqeditor_ontracksscroll);	
	psy_signal_connect(
		&psycleconfig_general(workspace_conf(workspace))->signal_changed,
		self, seqeditor_onconfigure);	
	psy_signal_connect(&self->headerbar.timesig.signal_clicked, self,
		seqeditor_ontoggletimesig);
	seqeditor_updatescrollstep(self);
}

void seqeditor_ondestroy(SeqEditor* self)
{
	seqeditstate_dispose(&self->state);
}

void seqeditor_onsongchanged(SeqEditor* self, Workspace* workspace, int flag,
	psy_audio_Song* song)
{	
	seqeditor_updatesong(self);
}

void seqeditor_updatesong(SeqEditor* self)
{
	psy_audio_Song* song;

	song = self->state.workspace->song;
	if (song) {
		psy_ui_component_move(&self->trackdesc.component, psy_ui_point_zero());
		psy_ui_component_move(&self->tracks.component, psy_ui_point_zero());
		seqeditor_build(self);
		psy_ui_component_invalidate(&self->trackdesc.component);
		psy_ui_component_invalidate(&self->ruler.component);
		psy_ui_component_align(&self->tracks.component);
		psy_ui_component_invalidate(&self->tracks.component);
		psy_signal_connect(&song->sequence.signal_clear, self,
			seqeditor_onclear);
		psy_signal_connect(&song->sequence.signal_trackinsert, self,
			seqeditor_ontrackinsert);
		psy_signal_connect(&song->sequence.signal_trackswap, self,
			seqeditor_ontrackswap);
		psy_signal_connect(&song->sequence.signal_trackremove, self,
			seqeditor_ontrackremove);
	}
}

void seqeditor_updatescrollstep(SeqEditor* self)
{		
	psy_ui_component_setscrollstep_height(&self->tracks.component,
		seqeditstate_lineheight(&self->state));
	psy_ui_component_setscrollstep_height(&self->trackdesc.component,
		seqeditstate_lineheight(&self->state));
}

void seqeditor_onclear(SeqEditor* self, psy_audio_Sequence* sender)
{
	psy_ui_component_move(&self->trackdesc.component, psy_ui_point_zero());
	psy_ui_component_move(&self->tracks.component, psy_ui_point_zero());
	seqeditor_build(self);
	psy_ui_component_invalidate(&self->trackdesc.component);
	psy_ui_component_align(&self->tracks.component);
	psy_ui_component_invalidate(&self->tracks.component);
}

void seqeditor_ontrackinsert(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t trackidx)
{
	seqeditor_build(self);	
}

void seqeditor_ontrackswap(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t first, uintptr_t second)
{
	seqeditor_build(self);
	psy_ui_component_invalidate(&self->trackdesc.component);
	psy_ui_component_align(&self->tracks.component);
	psy_ui_component_invalidate(&self->tracks.component);
}

void seqeditor_ontrackremove(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t trackidx)
{
	seqeditor_build(self);	
}

void seqeditor_build(SeqEditor* self)
{	
	seqeditortracks_build(&self->tracks);
	seqedittrackdesc_build(&self->trackdesc);	
	psy_ui_component_align(&self->scroller.pane);	
	psy_ui_component_align(&self->trackdescpane);		
}

void seqeditor_ontracksscroll(SeqEditor* self, psy_ui_Component* sender)
{	
	psy_ui_component_setscrolltop(&self->trackdesc.component,
		psy_ui_component_scrolltop(&self->tracks.component));
	psy_ui_component_setscrollleft(&self->ruler.component,
		psy_ui_component_scrollleft(&self->tracks.component));
}

void seqeditor_onconfigure(SeqEditor* self, GeneralConfig* sender,
	psy_Property* property)
{	
	self->state.showpatternnames = generalconfig_showingpatternnames(sender);
	psy_ui_component_invalidate(&self->tracks.component);
}

void seqeditor_onhzoom(SeqEditor* self, ZoomBox* sender)
{
	self->state.pxperbeat = sender->zoomrate * DEFAULT_PXPERBEAT;
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_align(&self->rulerpane);
}

void seqeditor_onvzoom(SeqEditor* self, ZoomBox* sender)
{
	self->state.lineheight = psy_ui_mul_value_real(
		self->state.defaultlineheight, zoombox_rate(sender));
	seqeditor_updatescrollstep(self);
	psy_ui_component_align(&self->scroller.pane);	
	psy_ui_component_align(&self->trackdescpane);
}

void seqeditor_onmousemove(SeqEditor* self, psy_ui_MouseEvent* ev)
{
	if (self->state.updatecursorposition) {
		psy_ui_component_invalidate(&self->ruler.component);
		self->state.updatecursorposition = FALSE;		
	}
}

void seqeditor_onmouseup(SeqEditor* self, psy_ui_MouseEvent* ev)
{
	if (self->state.cmd == SEQEDTCMD_NEWTRACK) {
		sequencecmds_appendtrack(self->state.cmds);
	} else if (self->state.cmd == SEQEDTCMD_DELTRACK) {
		sequencecmds_deltrack(self->state.cmds, self->state.dragseqpos.track);
	}	
	self->state.cmd = SEQEDTCMD_NONE;
}

void seqeditor_ontoggleexpand(SeqEditor* self, psy_ui_Button* sender)
{	
	self->expanded = !self->expanded;	
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_zero(), psy_ui_value_make_ph(
			(self->expanded) ? 0.75 : 0.3)));
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}

void seqeditor_ontrackresize(SeqEditor* self, psy_ui_Component* sender,
	uintptr_t trackid, double* height)
{
	psy_ui_Component* track;
	
	assert(height);

	track = psy_ui_component_at(&self->tracks.component, trackid);
	if (track) {		
		psy_ui_component_setpreferredheight(track,
			psy_ui_value_make_eh(*height));
		psy_ui_component_align(&self->scroller.pane);		
		psy_ui_component_align(&self->trackdescpane);
	}
}

void seqeditor_ontoggletimesig(SeqEditor* self, psy_ui_Button* sender)
{	
	if (self->ruler.showtimesig) {
		seqeditruler_hidetimesig(&self->ruler);		
	} else {
		seqeditruler_showtimesig(&self->ruler);
	}
	if (self->ruler.showtimesig) {
		psy_ui_button_settext(sender, "Hide TimeSig");
		psy_ui_button_seticon(sender, psy_ui_ICON_LESS);
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_button_base(sender)));
		psy_ui_component_setpreferredsize(&self->headerbar.component,
			psy_ui_size_make_em(40.0, 4.0));
		psy_ui_component_align(&self->left);
		psy_ui_component_align(&self->component);		
	} else {
		psy_ui_button_settext(sender, "Show TimeSig");
		psy_ui_button_seticon(sender, psy_ui_ICON_MORE);
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_button_base(sender)));
		psy_ui_component_setpreferredsize(&self->headerbar.component,
			psy_ui_size_make_em(40.0, 2.0));
		psy_ui_component_align(&self->left);
		psy_ui_component_align(&self->component);
	}
}

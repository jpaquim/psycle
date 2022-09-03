/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "paramroll.h"
/* host */
#include "patterncmds.h"
/* platform */
#include "../../detail/portable.h"

/* ParamRuler */

/* prototypes */
static void paramruler_on_draw(ParamRuler*, psy_ui_Graphics*);

/* vtable */
static psy_ui_ComponentVtable paramruler_vtable;
static bool paramruler_vtable_initialized = FALSE;

static void paramruler_table_init(ParamRuler* self)
{
	assert(self);

	if (!paramruler_vtable_initialized) {
		paramruler_vtable = *(self->component.vtable);
		paramruler_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			paramruler_on_draw;		
		paramruler_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &paramruler_vtable);	
}

/* implementation */
void paramruler_init(ParamRuler* self, psy_ui_Component* parent)
{
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	paramruler_table_init(self);
}

void paramruler_on_draw(ParamRuler* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	uintptr_t i;
	uintptr_t step;
	uintptr_t maxval;	
	
	size = psy_ui_component_size_px(&self->component);	
	maxval = 256;
	step = maxval / 8;
	for (i = 0; i < maxval; i += step) {
		double cpy;
		double w;
		
		cpy = i * (size.height / (double)maxval);
		if (i == 0x80) {
			w = size.width;
		} else {
			w = size.width / 2;
		}
		psy_ui_drawline(g, psy_ui_realpoint_make(0, cpy),
			psy_ui_realpoint_make(w, cpy));		
	}
}

/* ParamDraw */

/* prototypes */
static void paramdraw_on_draw(ParamDraw*, psy_ui_Graphics*);
static void paramdraw_draw_mid_line(ParamDraw*, psy_ui_Graphics*);
static void paramdraw_draw_entries(ParamDraw*, psy_ui_Graphics*);
static void paramdraw_on_preferred_size(ParamDraw*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void paramdraw_on_track_display(ParamDraw*, psy_Property* sender);
static void paramdraw_on_mouse_down(ParamDraw*, psy_ui_MouseEvent*);
static void paramdraw_on_mouse_up(ParamDraw*, psy_ui_MouseEvent*);
static void paramdraw_on_mouse_move(ParamDraw*, psy_ui_MouseEvent*);
static uint8_t paramdraw_tweak_value(ParamDraw*, psy_ui_MouseEvent*);
static void paramdraw_output(ParamDraw*, uint8_t value);
static void paramdraw_on_mouse_wheel(ParamDraw*, psy_ui_MouseEvent*);
static psy_audio_SequenceCursor paramdraw_make_cursor(ParamDraw*, double x);

/* vtable */
static psy_ui_ComponentVtable paramdraw_vtable;
static bool paramdraw_vtable_initialized = FALSE;

static void paramdraw_vtable_init(ParamDraw* self)
{
	assert(self);

	if (!paramdraw_vtable_initialized) {
		paramdraw_vtable = *(self->component.vtable);
		paramdraw_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			paramdraw_on_draw;
		paramdraw_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			paramdraw_on_preferred_size;
		paramdraw_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			paramdraw_on_mouse_down;
		paramdraw_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			paramdraw_on_mouse_up;
		paramdraw_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			paramdraw_on_mouse_move;
		paramdraw_vtable.on_mouse_wheel =
			(psy_ui_fp_component_on_mouse_event)
			paramdraw_on_mouse_wheel;
		paramdraw_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &paramdraw_vtable);	
}

/* implementation */
void paramdraw_init(ParamDraw* self, psy_ui_Component* parent,
	PianoGridState* state, Workspace* workspace)
{
	assert(self);
	assert(state);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);
	paramdraw_vtable_init(self);	
	self->state = state;
	self->workspace = workspace;
	self->tweak_pattern = NULL;
	self->tweak_node = NULL;
	psy_property_connect(&self->state->track_display,
		self, paramdraw_on_track_display);	
}

void paramdraw_on_draw(ParamDraw* self, psy_ui_Graphics* g)
{
	assert(self);
	
	paramdraw_draw_mid_line(self, g);
	paramdraw_draw_entries(self, g);
}

void paramdraw_draw_mid_line(ParamDraw* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	uintptr_t i;
	uintptr_t step;
	uintptr_t maxval;
	const psy_ui_TextMetric* tm;
	double cpy;
	psy_ui_Colour restore;
	psy_ui_Colour colour;
	
	restore = psy_ui_component_colour(&self->component);
	colour = psy_ui_colour_make(0x333333);	
	psy_ui_setcolour(g, colour);
	size = psy_ui_component_size_px(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	maxval = 256;
	step = maxval / 8;
	i = 0x80;	
	cpy = i * (size.height / (double)maxval);		
	psy_ui_drawline(g, psy_ui_realpoint_make(0, cpy),
		psy_ui_realpoint_make(size.width, cpy));
	psy_ui_setcolour(g, restore);
}	

void paramdraw_draw_entries(ParamDraw* self, psy_ui_Graphics* g)
{
	psy_audio_Sequence* sequence;
	psy_audio_PatternNode* currnode;	
	psy_audio_SequenceTrackIterator ite;
	psy_dsp_big_beat_t seqoffset;	
	bool track_display;
	psy_ui_RealSize size;
	psy_ui_RealSize pt_size;	
	uintptr_t maxval;
	uint8_t param;
	double px;
	
	assert(self);
	
	if (!patternviewstate_sequence(self->state->pv)) {
		return;
	}
	sequence = patternviewstate_sequence(self->state->pv);
				
	size = psy_ui_component_scroll_size_px(&self->component);
	maxval = 256;
	param = 0x0C;
	px = size.height / (double)maxval;
		
	psy_audio_sequencetrackiterator_init(&ite);
	psy_audio_sequence_begin(self->state->pv->sequence,			
		self->state->pv->cursor.order_index.track,
		(patternviewstate_single_mode(self->state->pv)
		? psy_audio_sequencecursor_seqoffset(&self->state->pv->cursor,
			patternviewstate_sequence(self->state->pv))
		: 0.0), &ite);
	currnode = ite.patternnode;
	seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
	pt_size = psy_ui_realsize_make(5.0, 5.0);
	track_display = (pianogridstate_track_display(self->state) ==
		PIANOROLL_TRACK_DISPLAY_CURRENT);
	while (currnode) {
		psy_audio_PatternEntry* curr;
		psy_audio_PatternEvent* currevent;
		double cpy;

		curr = (psy_audio_PatternEntry*)psy_list_entry(currnode);			
		currevent = psy_audio_patternentry_front(curr);
		if ((currevent->cmd == param) && ((!track_display) || (track_display &&
				self->state->pv->cursor.track == curr->track))) {
			psy_dsp_big_beat_t offset;
					
			cpy = (maxval - currevent->parameter) * px;
			offset = psy_audio_patternentry_offset(curr);
			if (!patternviewstate_single_mode(self->state->pv)) {
				offset += seqoffset;
			}
			psy_ui_drawrectangle(g, psy_ui_realrectangle_make(
				psy_ui_realpoint_make(pianogridstate_raw_beat_to_px(
					self->state, offset), cpy),
				pt_size));			
		}
		if (patternviewstate_single_mode(self->state->pv)) {			
			psy_audio_patternnode_next(&currnode);
		} else {
			psy_audio_sequencetrackiterator_inc(&ite);
			currnode = ite.patternnode;
			seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
		}		
	}
	psy_audio_sequencetrackiterator_dispose(&ite);
}

void paramdraw_on_preferred_size(ParamDraw* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	assert(self);

	rv->width = psy_ui_value_make_px(pianogridstate_beat_to_px(self->state,
		patternviewstate_length(self->state->pv)));
	if (limit) {
		rv->height = limit->height;
	}
}

void paramdraw_on_track_display(ParamDraw* self, psy_Property* sender)
{
	psy_ui_component_invalidate(&self->component);
}

void paramdraw_on_mouse_down(ParamDraw* self, psy_ui_MouseEvent* ev)
{	
	psy_audio_PatternNode* prev;
		
	assert(self);
	
	if (patternviewstate_sequence(self->state->pv)) {		
		psy_audio_sequence_set_cursor(patternviewstate_sequence(
			self->state->pv), paramdraw_make_cursor(self,
				psy_ui_mouseevent_offset(ev).x));
	}
	self->tweak_pattern = patternviewstate_pattern(self->state->pv);
	if (!self->tweak_pattern) {
		return;
	}	
	self->tweak_node = psy_audio_pattern_findnode_cursor(self->tweak_pattern,
		self->state->pv->cursor, &prev);
	if (self->tweak_node) {
		psy_audio_PatternEntry* entry;
		psy_audio_PatternEvent* pat_ev;

		entry = (psy_audio_PatternEntry*)self->tweak_node->entry;
		pat_ev = psy_audio_patternentry_front(entry);		
		if (psy_ui_mouseevent_button(ev) == 2) {
			pat_ev->cmd = 0x00;
			pat_ev->parameter = 0x00;
			self->tweak_pattern = NULL;
			self->tweak_node = NULL;			
		} else {
			pat_ev->cmd = 0x0C;
			pat_ev->parameter = paramdraw_tweak_value(self, ev);			
			paramdraw_output(self, pat_ev->parameter);
		}
		psy_ui_component_invalidate(&self->component);
	} else if (psy_ui_mouseevent_button(ev) == 1) {			
		psy_audio_PatternEvent pat_ev;
			
		psy_audio_patternevent_init(&pat_ev);
		pat_ev.cmd = 0x0C;
		pat_ev.parameter = paramdraw_tweak_value(self, ev);
		self->tweak_node = psy_audio_pattern_insert(self->tweak_pattern,
			prev,
			psy_audio_sequencecursor_track(&self->state->pv->cursor),
			psy_audio_sequencecursor_offset(&self->state->pv->cursor),
			&pat_ev);
		paramdraw_output(self, pat_ev.parameter);
		psy_ui_component_invalidate(&self->component);
	} else {
		self->tweak_pattern = NULL;
		self->tweak_node = NULL;	
	}	
	if (self->tweak_node) {
		psy_ui_component_capture(&self->component);		
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

psy_audio_SequenceCursor paramdraw_make_cursor(ParamDraw* self, double x)
{
	psy_audio_SequenceCursor rv;	
		
	assert(self);
	
	rv = self->state->pv->cursor;
	psy_audio_sequencecursor_set_offset(&rv, pianogridstate_px_to_beat(
		self->state, x));
	return rv;
}

void paramdraw_on_mouse_move(ParamDraw* self, psy_ui_MouseEvent* ev)
{
	assert(self);
	
	if (self->tweak_node) {
		uint8_t value;
		psy_audio_PatternEntry* entry;
		psy_audio_PatternEvent* pat_ev;
		
		entry = (psy_audio_PatternEntry*)self->tweak_node->entry;
		pat_ev = psy_audio_patternentry_front(entry);
		value = paramdraw_tweak_value(self, ev);
		if (pat_ev->parameter != value) {			
			pat_ev->parameter = value;
			psy_audio_sequence_tweak(self->state->pv->sequence);			
			psy_ui_component_invalidate(&self->component);
			paramdraw_output(self, value);
		}		
	} else if (psy_ui_mouseevent_button(ev) == 0) {
		psy_audio_SequenceCursor cursor;
		psy_audio_Pattern* pattern;
		psy_audio_PatternNode* prev;
		psy_audio_PatternNode* node;
		
		cursor = paramdraw_make_cursor(self, psy_ui_mouseevent_offset(ev).x);
		pattern = patternviewstate_pattern(self->state->pv);
		if (!pattern) {
			return;
		}	
		node = psy_audio_pattern_findnode_cursor(pattern, cursor,
			&prev);
		if (node) {
			psy_audio_PatternEntry* entry;
			psy_audio_PatternEvent* pat_ev;

			entry = (psy_audio_PatternEntry*)node->entry;
			pat_ev = psy_audio_patternentry_front(entry);
			if (pat_ev->cmd == 0x0C) {
				paramdraw_output(self, pat_ev->parameter);
			} else {
				paramdraw_output(self, paramdraw_tweak_value(self, ev));				
			}
		} else {
			paramdraw_output(self, paramdraw_tweak_value(self, ev));
		}
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void paramdraw_on_mouse_up(ParamDraw* self, psy_ui_MouseEvent* ev)
{
	assert(self);
	
	psy_ui_component_release_capture(&self->component);
	if (self->tweak_pattern && self->tweak_node) {
		psy_audio_PatternEntry* entry;
		psy_audio_PatternEvent* pat_ev;
		
		entry = (psy_audio_PatternEntry*)self->tweak_node->entry;
		pat_ev = psy_audio_patternentry_front(entry);				
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_allocinit(self->tweak_pattern,
				self->state->pv->cursor, *pat_ev, 
				self->state->pv->sequence)->command);
		psy_ui_component_invalidate(&self->component);		
	}
	self->tweak_pattern = NULL;
	self->tweak_node = NULL;
	psy_ui_mouseevent_stop_propagation(ev);
}

void paramdraw_on_mouse_wheel(ParamDraw* self, psy_ui_MouseEvent* ev)
{
	if (!patternviewstate_sequence(self->state->pv)) {		
		return;
	}	
	if (psy_ui_mouseevent_delta(ev) != 0) {
		psy_audio_SequenceCursor cursor;
		intptr_t value;
		psy_audio_PatternNode* prev;
		
		cursor = paramdraw_make_cursor(self, psy_ui_mouseevent_offset(ev).x);
		if (!psy_audio_sequencecursor_equal(&cursor, &self->state->pv->cursor)) {						
			psy_audio_sequence_set_cursor(patternviewstate_sequence(
				self->state->pv), cursor);
		}
		self->tweak_pattern = patternviewstate_pattern(self->state->pv);
		if (!self->tweak_pattern) {
			return;
		}	
		self->tweak_node = psy_audio_pattern_findnode_cursor(self->tweak_pattern,
			self->state->pv->cursor, &prev);
		if (self->tweak_node) {			
			psy_audio_PatternEntry* entry;
			psy_audio_PatternEvent* pat_ev;

			entry = (psy_audio_PatternEntry*)self->tweak_node->entry;
			pat_ev = psy_audio_patternentry_front(entry);					
			if (pat_ev->cmd == 0x0C) {
				value = pat_ev->parameter;
			} else {
				return;
			}			
		} else {
			return;
		}	
		if (psy_ui_mouseevent_delta(ev) > 0) {
			value += 1;
		} else {
			value -= 1;
		}
		value = psy_max(0, psy_min(0xFF, value));
		if (self->tweak_node) {			
			psy_audio_PatternEntry* entry;
			psy_audio_PatternEvent* pat_ev;

			entry = (psy_audio_PatternEntry*)self->tweak_node->entry;
			pat_ev = psy_audio_patternentry_front(entry);					
			if (pat_ev->cmd == 0x0C) {							
				pat_ev->parameter = (uint8_t)value;
				psy_audio_sequence_tweak(self->state->pv->sequence);			
				psy_ui_component_invalidate(&self->component);
				paramdraw_output(self, pat_ev->parameter);
			}			
		}		
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void paramdraw_output(ParamDraw* self, uint8_t value)
{
	char str[64];
			
	psy_snprintf(str, 64, "Tweak Cmd 0C Parameter %X", (int)value);
	workspace_output_status(self->workspace, str);
}

uint8_t paramdraw_tweak_value(ParamDraw* self, psy_ui_MouseEvent* ev)
{
	psy_ui_RealSize size;				
	uintptr_t maxval;
	uint8_t cmd;
	double px;
	intptr_t val;		
		
	size = psy_ui_component_scroll_size_px(&self->component);
	maxval = 256;
	cmd = 0x0C;
	px = size.height / (double)maxval;
	val = (intptr_t)(psy_ui_mouseevent_offset(ev).y / px);
	if (val < 0) {
		val = 255;
	} else if (val < 256) {
		val = maxval - val;
	} else {
		val = 0;
	}
	return (uint8_t)val;
}

/* ParamRoll */

/* implementation */
void paramroll_init(ParamRoll* self, psy_ui_Component* parent,
	PianoGridState* state, Workspace* workspace)
{
	assert(self);
	assert(state);
	
	psy_ui_component_init(&self->component, parent, NULL);
	/* left */
	psy_ui_component_init(&self->left, &self->component, NULL);
	psy_ui_component_set_align(&self->left, psy_ui_ALIGN_LEFT);
	psy_ui_component_set_preferred_size(&self->left,
		psy_ui_size_make_em(10.0, 8.0));
	paramruler_init(&self->ruler, &self->left);
	psy_ui_component_set_align(&self->ruler.component, psy_ui_ALIGN_CLIENT);
	/* pane */
	psy_ui_component_init(&self->pane, &self->component, NULL);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_CLIENT);	
	paramdraw_init(&self->draw, &self->pane, state, workspace);
	psy_ui_component_set_align(&self->draw.component, psy_ui_ALIGN_FIXED);	
}

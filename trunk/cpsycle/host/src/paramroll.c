/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "paramroll.h"

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
		
		cpy = i * (size.height / (double)maxval);
		psy_ui_drawline(g, psy_ui_realpoint_make(0, cpy),
			psy_ui_realpoint_make(size.width, cpy));
	}
}

/* ParamDraw */

/* prototypes */
static void paramdraw_on_draw(ParamDraw*, psy_ui_Graphics*);
static void paramdraw_draw_entries(ParamDraw*, psy_ui_Graphics*);
static void paramdraw_on_preferred_size(ParamDraw*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void paramdraw_on_track_display(ParamDraw*, psy_Property* sender);

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
		paramdraw_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &paramdraw_vtable);	
}

/* implementation */
void paramdraw_init(ParamDraw* self, psy_ui_Component* parent,
	PianoGridState* state)
{
	assert(self);
	assert(state);
	
	psy_ui_component_init(&self->component, parent, NULL);
	paramdraw_vtable_init(self);
	self->state = state;
	psy_property_connect(&self->state->track_display,
		self, paramdraw_on_track_display);	
}

void paramdraw_on_draw(ParamDraw* self, psy_ui_Graphics* g)
{
	assert(self);
	
	paramdraw_draw_entries(self, g);
}

void paramdraw_draw_entries(ParamDraw* self, psy_ui_Graphics* g)
{
	psy_audio_Sequence* sequence;
	psy_audio_PatternNode* currnode;
	psy_TableIterator it;
	psy_audio_SequenceTrackIterator ite;
	psy_dsp_big_beat_t seqoffset;	
	bool track_display;
	psy_ui_RealSize size;
	psy_ui_RealSize pt_size;
	uintptr_t i;	
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
				psy_ui_realpoint_make(pianogridstate_beattopx(
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

	rv->width = psy_ui_value_make_px(pianogridstate_beattopx(self->state,
		patternviewstate_length(self->state->pv)));
	if (limit) {
		rv->height = limit->height;
	}
}

void paramdraw_on_track_display(ParamDraw* self, psy_Property* sender)
{
	psy_ui_component_invalidate(&self->component);
}

/* ParamRoll */

/* implementation */
void paramroll_init(ParamRoll* self, psy_ui_Component* parent,
	PianoGridState* state)
{
	assert(self);
	assert(state);
	
	psy_ui_component_init(&self->component, parent, NULL);
	/* left */
	psy_ui_component_init(&self->left, &self->component, NULL);
	psy_ui_component_set_align(&self->left, psy_ui_ALIGN_LEFT);
	psy_ui_component_set_preferred_width(&self->left,
		psy_ui_value_make_ew(10.0));
	paramruler_init(&self->ruler, &self->left);
	psy_ui_component_set_align(&self->ruler.component, psy_ui_ALIGN_CLIENT);
	/* pane */
	psy_ui_component_init(&self->pane, &self->component, NULL);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_preferred_height(&self->left,
		psy_ui_value_make_eh(10.0));
	paramdraw_init(&self->draw, &self->pane, state);
	psy_ui_component_set_align(&self->draw.component, psy_ui_ALIGN_FIXED);
	/* hscroll */
	psy_ui_scrollbar_init(&self->hscroll, &self->component);
	psy_ui_component_set_align(&self->hscroll.component, psy_ui_ALIGN_BOTTOM);		
}

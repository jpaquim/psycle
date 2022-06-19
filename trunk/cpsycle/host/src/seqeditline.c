/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditline.h"
/* host */
#include "styles.h"
/* audio */
#include <exclusivelock.h>
/* platform */
#include "../../detail/portable.h"

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
			(psy_ui_fp_component_on_preferred_size)
			seqeditorline_onpreferredsize;
	}
	self->component.vtable = &seqeditorline_vtable;
}

void seqeditorline_init(SeqEditorLine* self, psy_ui_Component* parent,
	SeqEditState* state)
{
	psy_ui_component_init(seqeditorline_base(self), parent, NULL);	
	seqeditorline_vtable_init(self);
	self->state = state;
	psy_ui_component_setposition(seqeditorline_base(self),
		psy_ui_rectangle_make(psy_ui_point_zero(), psy_ui_size_zero()));
}

SeqEditorLine* seqeditorline_alloc(void)
{
	return (SeqEditorLine*)malloc(sizeof(SeqEditorLine));
}

SeqEditorLine* seqeditorline_allocinit(psy_ui_Component* parent,
	SeqEditState* state)
{
	SeqEditorLine* rv;

	rv = seqeditorline_alloc();
	if (rv) {
		seqeditorline_init(rv, parent, state);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
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
	parentsize = psy_ui_component_scroll_size_px(
		psy_ui_component_parent(&self->component));
	if (parentsize.height != psy_ui_realrectangle_height(&updateposition) ||
			position_px != updateposition.left) {		
		newposition = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(position_px, 0.0),
			psy_ui_realsize_make(1.0, parentsize.height));		
		psy_ui_component_setposition(&self->component,
			psy_ui_rectangle_make_px(&newposition));
		psy_ui_realrectangle_union(&updateposition, &newposition);
		psy_ui_component_invalidate_rect(
			psy_ui_component_parent(&self->component),
			updateposition);
	}
}

/* SeqEditorPlayLine */
/* prototypes */
static void seqeditorplayline_on_mouse_down(SeqEditorPlayline*,
	psy_ui_MouseEvent*);
static void seqeditorplayline_onmousemove(SeqEditorPlayline*,
	psy_ui_MouseEvent*);
static void seqeditorplayline_on_mouse_up(SeqEditorPlayline*, psy_ui_MouseEvent*);
/* vtable */
static psy_ui_ComponentVtable seqeditorplayline_vtable;
static bool seqeditorplayline_vtable_initialized = FALSE;

static void seqeditorplayline_vtable_init(SeqEditorPlayline* self)
{
	if (!seqeditorplayline_vtable_initialized) {
		seqeditorplayline_vtable =
			*(seqeditorline_base(&self->seqeditorline)->vtable);		
		seqeditorplayline_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			seqeditorplayline_on_mouse_down;
		seqeditorplayline_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			seqeditorplayline_onmousemove;
		seqeditorplayline_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			seqeditorplayline_on_mouse_up;
	}
	seqeditorline_base(&self->seqeditorline)->vtable =
		&seqeditorplayline_vtable;
}

/* implementation */
void seqeditorplayline_init(SeqEditorPlayline* self, psy_ui_Component* parent,
	SeqEditState* state)
{
	seqeditorline_init(&self->seqeditorline, parent, state);	
	seqeditorplayline_vtable_init(self);
	psy_ui_component_set_background_colour(
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
	psy_ui_Component* parent, SeqEditState* state)
{
	SeqEditorPlayline* rv;

	rv = seqeditorplayline_alloc();
	if (rv) {
		seqeditorplayline_init(rv, parent, state);
		psy_ui_component_deallocate_after_destroyed(
			seqeditorline_base(&rv->seqeditorline));		
	}
	return rv;
}

void seqeditorplayline_on_mouse_down(SeqEditorPlayline* self,
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
				self->dragbase + psy_ui_mouseevent_pt(ev).x)));
		psy_audio_exclusivelock_leave();
		seqeditorplayline_update(self);
		self->dragbase = psy_ui_component_position(
			seqeditorline_base(&self->seqeditorline)).left;		
	}
	psy_ui_mouseevent_stop_propagation(ev);	
}

void seqeditorplayline_on_mouse_up(SeqEditorPlayline* self, psy_ui_MouseEvent* ev)
{
	self->drag = FALSE;
	self->dragbase = 0.0;
	psy_ui_component_release_capture(seqeditorline_base(&self->seqeditorline));
	psy_ui_mouseevent_stop_propagation(ev);
}

void seqeditorplayline_update(SeqEditorPlayline* self)
{		
	seqeditorline_updateposition(&self->seqeditorline,		
		psy_audio_player_position(workspace_player(
			self->seqeditorline.state->workspace)));	
}

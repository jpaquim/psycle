// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "generatorui.h"
// host
#include "skingraphics.h"
#include "paramview.h"
#include "wireview.h"
#include "masterui.h"
// audio
#include <exclusivelock.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// GeneratorUi
static void generatorui_dispose(GeneratorUi*);
static void generatorui_initsize(GeneratorUi*);
static int generatorui_hittestpan(GeneratorUi*, psy_ui_RealPoint, double* dx);
static bool generatorui_hittestcoord(GeneratorUi*, psy_ui_RealPoint,
	SkinCoord*);
static void generatorui_ondraw(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_draweditname(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_drawbackground(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_drawpanning(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_drawmute(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_drawsoloed(GeneratorUi*, psy_ui_Graphics*);
static psy_dsp_amp_t generatorui_panvalue(GeneratorUi*, double dx, uintptr_t slot);
static void generatorui_onmousedown(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_onmouseup(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_onmousemove(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_onmousedoubleclick(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_drawvu(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_move(GeneratorUi*, psy_ui_Point topleft);
static void generatorui_updatevolumedisplay(GeneratorUi*);
static void generatorui_invalidate(GeneratorUi*);
static void generatorui_onshowparameters(GeneratorUi*, Workspace* sender,
	uintptr_t slot);
static void generatorui_showparameters(GeneratorUi*, psy_ui_Component* parent);
static void generatorui_onpreferredsize(GeneratorUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable generatorui_vtable;
static psy_ui_ComponentVtable generatorui_super_vtable;
static bool generatorui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* generatorui_vtable_init(GeneratorUi* self)
{
	assert(self);

	if (!generatorui_vtable_initialized) {
		generatorui_vtable = *(self->component.vtable);
		generatorui_super_vtable = generatorui_vtable;
		generatorui_vtable.dispose = (psy_ui_fp_component_dispose)
			generatorui_dispose;
		generatorui_vtable.ondraw = (psy_ui_fp_component_ondraw)
			generatorui_ondraw;
		generatorui_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			generatorui_onmousedown;
		generatorui_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)
			generatorui_onmouseup;
		generatorui_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)
			generatorui_onmousemove;
		generatorui_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			generatorui_onmousedoubleclick;
		generatorui_vtable.move = (psy_ui_fp_component_move)generatorui_move;
		generatorui_vtable.invalidate = (psy_ui_fp_component_invalidate)
			generatorui_invalidate;
		generatorui_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			generatorui_onpreferredsize;
		generatorui_vtable_initialized = TRUE;
	}
	return &generatorui_vtable;
}
// implementation
void generatorui_init(GeneratorUi* self, psy_ui_Component* parent,
	uintptr_t slot, MachineViewSkin* skin,
	psy_ui_Component* view, ParamViews* paramviews, Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(workspace->song);
	assert(skin);
	assert(view);

	psy_ui_component_init(&self->component, parent, view);	
	generatorui_vtable_init(self);
	self->component.vtable = &generatorui_vtable;
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	machineuicommon_init(&self->intern, slot, skin, view, paramviews,
		workspace);
	self->intern.coords = &skin->generator;
	self->intern.font = skin->generator_fontcolour;
	self->intern.bgcolour = psy_ui_colour_make(0x002f3E25);
	generatorui_initsize(self);	
}

void generatorui_dispose(GeneratorUi* self)
{
	assert(self);
	
	free(self->intern.restorename);	
	generatorui_super_vtable.dispose(&self->component);
}

void generatorui_initsize(GeneratorUi* self)
{
	psy_ui_RealPoint topleft;
	psy_ui_RealSize size;

	size = psy_ui_realrectangle_size(&self->intern.coords->background.dest);
	if (self->intern.machine && self->intern.machinepos) {
		psy_audio_machine_position(self->intern.machine, &topleft.x, &topleft.y);
	} else {
		topleft = psy_ui_realpoint_zero();
	}
	psy_ui_component_setposition(&self->component,
		psy_ui_rectangle_make(
			psy_ui_point_make_px(topleft.x, topleft.y),
			psy_ui_size_make_px(size.width, size.height)));
	vudisplay_init(&self->intern.vu, self->intern.skin, self->intern.coords);	
}

void generatorui_move(GeneratorUi* self, psy_ui_Point topleft)
{
	assert(self);

	generatorui_super_vtable.move(&self->component, topleft);
	machineuicommon_move(&self->intern, topleft);	
}

void generatorui_ondraw(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	generatorui_drawbackground(self, g);
	if (!machineui_vuupdate()) {
		generatorui_draweditname(self, g);		
		generatorui_drawpanning(self, g);		
	}
	generatorui_drawmute(self, g);
	generatorui_drawsoloed(self, g);
	if (self->intern.skin->drawvumeters) {
		generatorui_drawvu(self, g);
	}
}

void generatorui_drawbackground(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (!psy_ui_bitmap_empty(&self->intern.skin->skinbmp)) {
		skin_blitcoord(g, &self->intern.skin->skinbmp, psy_ui_realpoint_zero(),
			&self->intern.coords->background);
	} else {		
		psy_ui_drawsolidrectangle(g, self->intern.coords->background.dest,
			self->intern.bgcolour);		
	}
}

void generatorui_draweditname(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (self->intern.mode != psy_audio_MACHMODE_MASTER) {
		char editname[130];

		editname[0] = '\0';
		if (psy_audio_machine_editname(self->intern.machine)) {
			if (self->intern.skin->drawmachineindexes) {
				psy_snprintf(editname, 130, "%.2X:%s", (int)self->intern.slot,
					psy_audio_machine_editname(self->intern.machine));
			} else {
				psy_snprintf(editname, 130, "%s",
					psy_audio_machine_editname(self->intern.machine));
			}
		}
		if (psy_strlen(editname) > 0) {
			psy_ui_settextcolour(g, self->intern.font);
			psy_ui_textoutrectangle(g,
				psy_ui_realrectangle_topleft(&self->intern.coords->name.dest),
				psy_ui_ETO_CLIPPED, self->intern.coords->name.dest, editname,
				strlen(editname));
		}
	}
}

void generatorui_drawpanning(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	skin_blitcoord(g, &self->intern.skin->skinbmp,
		psy_ui_realpoint_make(
			skincoord_position(&self->intern.coords->pan,
				psy_audio_machine_panning(self->intern.machine)),
			0),
		&self->intern.coords->pan);
}

void generatorui_drawmute(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (psy_audio_machine_muted(self->intern.machine)) {
		skin_blitcoord(g, &self->intern.skin->skinbmp,
			psy_ui_realpoint_zero(),
			&self->intern.coords->mute);
	}
}

void generatorui_drawsoloed(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (psy_audio_machines_soloed(self->intern.machines) ==
			self->intern.slot) {
		skin_blitcoord(g, &self->intern.skin->skinbmp,
			psy_ui_realpoint_zero(),
			&self->intern.coords->solo);
	}
}

void generatorui_drawvu(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);
	
	generatorui_updatevolumedisplay(self);
	vudisplay_draw(&self->intern.vu, g);	
}

void generatorui_updatevolumedisplay(GeneratorUi* self)
{
	if (self->intern.machine) {
		vudisplay_update(&self->intern.vu,
			psy_audio_machine_buffermemory(self->intern.machine));
	}
}

void generatorui_showparameters(GeneratorUi* self, psy_ui_Component* parent)
{
	if (self->intern.paramviews) {
		paramviews_show(self->intern.paramviews, self->intern.slot);
	}
}

void generatorui_onmousedown(GeneratorUi* self, psy_ui_MouseEvent* ev)
{	
	if (self->intern.slot != psy_audio_machines_selected(self->intern.machines)) {
		psy_audio_machines_select(self->intern.machines, self->intern.slot);
	}
	if (generatorui_hittestcoord(self, ev->pt,
			&self->intern.skin->generator.solo)) {
		psy_audio_machines_solo(self->intern.machines, self->intern.slot);
		psy_ui_mouseevent_stop_propagation(ev);
	} else if (generatorui_hittestcoord(self, ev->pt,
			&self->intern.skin->generator.mute)) {
		if (psy_audio_machine_muted(self->intern.machine)) {
			psy_audio_machine_unmute(self->intern.machine);
		} else {
			psy_audio_machine_mute(self->intern.machine);
		}
		psy_ui_mouseevent_stop_propagation(ev);
	} else if (generatorui_hittestpan(self, ev->pt, &self->intern.mx)) {
		self->intern.dragmode = MACHINEVIEW_DRAG_PAN;
		psy_ui_mouseevent_stop_propagation(ev);
	}
	if (!ev->event.bubbles) {
		psy_ui_component_invalidate(&self->component);
	}
}

bool generatorui_hittestcoord(GeneratorUi* self, psy_ui_RealPoint pt,
	SkinCoord* coord)
{
	assert(self);
	
	return psy_ui_realrectangle_intersect(&coord->dest, pt);	
}

void generatorui_onmousemove(GeneratorUi* self, psy_ui_MouseEvent* ev)
{	
	if (self->intern.dragmode == MACHINEVIEW_DRAG_PAN) {
		psy_audio_machine_setpanning(self->intern.machine,
			generatorui_panvalue(self, ev->pt.x, self->intern.slot));
		psy_ui_component_invalidate(&self->component);
	}
}

int generatorui_hittestpan(GeneratorUi* self, psy_ui_RealPoint pt, double* dx)
{
	psy_ui_RealRectangle r;
	double offset;	
	
	offset = psy_audio_machine_panning(self->intern.machine) *
		self->intern.coords->pan.range;
	r = skincoord_destposition(&self->intern.coords->pan);
	psy_ui_realrectangle_move(&r, offset, 0);
	*dx = pt.x - r.left;
	return psy_ui_realrectangle_intersect(&r, pt);
}

psy_dsp_amp_t generatorui_panvalue(GeneratorUi* self, double dx, uintptr_t slot)
{
	psy_dsp_amp_t rv;
	MachineCoords* coords;
	
	rv = 0.f;
	coords = self->intern.coords;
	if (coords && coords->pan.range != 0) {
		rv = (psy_dsp_amp_t)(
			(dx - coords->pan.dest.left - (double)self->intern.mx) /
			(double)coords->pan.range);
	}
	return rv;
}

void generatorui_onmouseup(GeneratorUi* self, psy_ui_MouseEvent* ev)
{
	self->intern.dragmode = MACHINEVIEW_DRAG_NONE;
}

void generatorui_onmousedoubleclick(GeneratorUi* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		psy_ui_RealPoint dragpt;

		if (generatorui_hittestcoord(self, ev->pt,
				&self->intern.skin->generator.solo) ||
			generatorui_hittestcoord(self, ev->pt,
				&self->intern.skin->effect.bypass) ||
			generatorui_hittestcoord(self, ev->pt,
				&self->intern.skin->generator.mute) ||
			generatorui_hittestcoord(self, ev->pt,
				&self->intern.skin->effect.mute) ||
			generatorui_hittestpan(self, ev->pt, &dragpt.x)) {
		} else {
			generatorui_showparameters(self, self->intern.view);
		}
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void generatorui_invalidate(GeneratorUi* self)
{
	if (machineui_vuupdate()) {		
		psy_ui_component_invalidaterect(&self->component,
			self->intern.coords->vu0.dest);
		psy_ui_component_invalidaterect(&self->component,
			self->intern.coords->mute.dest);
		psy_ui_component_invalidaterect(&self->component,
			self->intern.coords->solo.dest);
	} else {
		generatorui_super_vtable.invalidate(&self->component);
	}
}

void generatorui_onshowparameters(GeneratorUi* self, Workspace* sender,
	uintptr_t slot)
{
	if (slot == self->intern.slot) {
		generatorui_showparameters(self, self->intern.view);
	}
}

void generatorui_onpreferredsize(GeneratorUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{			
	psy_ui_size_setreal(rv, psy_ui_realrectangle_size(
		&self->intern.coords->background.dest));
}

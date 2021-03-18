// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "arrowui.h"
// host
#include "skingraphics.h"
#include "paramview.h"
#include "wireview.h"
// audio
#include <exclusivelock.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// ArrowUi
static void arrowui_dispose(ArrowUi*);
static void arrowui_initsize(ArrowUi*);
static bool arrowui_hittestcoord(ArrowUi*, psy_ui_RealPoint, SkinCoord*);
static void arrowui_ondraw(ArrowUi*, psy_ui_Graphics*);
static void arrowui_drawbackground(ArrowUi*, psy_ui_Graphics*);
static void arrowui_move(ArrowUi*, psy_ui_Point topleft);
static void arrowui_invalidate(ArrowUi*);
static void arrowui_onpreferredsize(ArrowUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void arrowui_onmousedown(ArrowUi*, psy_ui_MouseEvent*);
static bool arrowui_selected(const ArrowUi*);
// vtable
static psy_ui_ComponentVtable arrowui_vtable;
static psy_ui_ComponentVtable arrowui_super_vtable;
static bool arrowui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* arrowui_vtable_init(ArrowUi* self)
{
	assert(self);

	if (!arrowui_vtable_initialized) {
		arrowui_vtable = *(self->component.vtable);
		arrowui_super_vtable = arrowui_vtable;
		arrowui_vtable.dispose = (psy_ui_fp_component_dispose)arrowui_dispose;
		arrowui_vtable.ondraw = (psy_ui_fp_component_ondraw)arrowui_ondraw;
		arrowui_vtable.move = (psy_ui_fp_component_move)arrowui_move;
		arrowui_vtable.invalidate = (psy_ui_fp_component_invalidate)
			arrowui_invalidate;
		arrowui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			arrowui_onpreferredsize;
		arrowui_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			arrowui_onmousedown;
		arrowui_vtable_initialized = TRUE;
	}
	return &arrowui_vtable;
}
// implementation
void arrowui_init(ArrowUi* self, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_Wire wire, MachineViewSkin* skin,
	Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(workspace->song);
	assert(skin);
	assert(view);

	psy_ui_component_init(&self->component, parent, view);	
	arrowui_vtable_init(self);
	self->component.vtable = &arrowui_vtable;
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	machineuicommon_init(&self->intern, 0, skin, view, NULL, workspace);
	self->intern.coords = &skin->effect;
	self->intern.font = skin->effect_fontcolour;
	self->intern.bgcolour = psy_ui_colour_make(0x003E2f25);
	self->wire = wire;
	arrowui_initsize(self);
}

ArrowUi* arrowui_alloc(void)
{
	return (ArrowUi*)malloc(sizeof(ArrowUi));
}

ArrowUi* arrowui_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_Wire wire, MachineViewSkin* skin,
	Workspace* workspace)
{
	ArrowUi* rv;

	rv = arrowui_alloc();
	if (rv) {
		arrowui_init(rv, parent, view, wire, skin, workspace);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void arrowui_dispose(ArrowUi* self)
{
	assert(self);

	free(self->intern.restorename);
	arrowui_super_vtable.dispose(&self->component);
}

void arrowui_initsize(ArrowUi* self)
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
			psy_ui_point_makepx(topleft.x, topleft.y),
			psy_ui_size_makepx(size.width, size.height)));
	vudisplay_init(&self->intern.vu, self->intern.skin, self->intern.coords);	
}

void arrowui_move(ArrowUi* self, psy_ui_Point topleft)
{
	assert(self);

	arrowui_super_vtable.move(&self->component, topleft);
	machineuicommon_move(&self->intern, topleft);	
}

void arrowui_ondraw(ArrowUi* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	assert(self);
	
	if (arrowui_selected(self)) {
		psy_ui_setcolour(g, self->intern.skin->selwirecolour);
	} else {
		psy_ui_setcolour(g, self->intern.skin->wireaacolour);
	}
	size = psy_ui_component_sizepx(&self->component);	
	psy_ui_drawline(g, 
		psy_ui_realpoint_make(0, size.height / 2),
		psy_ui_realpoint_make(size.width / 2, size.height / 2));
	psy_ui_drawline(g,
		psy_ui_realpoint_make(size.width / 2, size.height / 2),
		psy_ui_realpoint_make(size.width / 2, size.height));
}

bool arrowui_selected(const ArrowUi* self)
{
	if (workspace_song(self->intern.workspace) &&
			psy_audio_wire_valid(&self->wire)) {
		psy_audio_Machines* machines;
		psy_audio_Wire selectedwire;

		machines = &self->intern.workspace->song->machines;
		selectedwire = psy_audio_machines_selectedwire(machines);
		return psy_audio_wire_equal(&self->wire, &selectedwire);
	}
	return FALSE;
}

void arrowui_drawbackground(ArrowUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (!psy_ui_bitmap_empty(&self->intern.skin->skinbmp)) {
		skin_blitcoord(g, &self->intern.skin->skinbmp, psy_ui_realpoint_zero(),
			&self->intern.coords->background);
	} else {
		psy_ui_RealRectangle r;

		r = psy_ui_component_position(&self->component);
		psy_ui_drawsolidrectangle(g, r, self->intern.bgcolour);
	}
}

void arrowui_invalidate(ArrowUi* self)
{
	if (!machineui_vuupdate()) {		
		arrowui_super_vtable.invalidate(&self->component);
	}
}

void arrowui_onpreferredsize(ArrowUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	psy_ui_size_setreal(rv, 		
		psy_ui_realrectangle_size(&
		self->intern.coords->background.dest));
}

void arrowui_onmousedown(ArrowUi* self, psy_ui_MouseEvent* ev)
{
	if (workspace_song(self->intern.workspace)) {
		psy_audio_Machines* machines;

		machines = &self->intern.workspace->song->machines;
		psy_audio_machines_selectwire(machines, self->wire);
	}
}

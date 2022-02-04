// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "masterui.h"
// host
#include "skingraphics.h"
// platform
#include "../../detail/portable.h"

// MasterUi
// prototypes
static void masterui_dispose(MasterUi*);
static void masterui_initsize(MasterUi*);
static void masterui_ondraw(MasterUi*, psy_ui_Graphics*);
static void masterui_drawbackground(MasterUi*, psy_ui_Graphics*);
static void masterui_onmousedoubleclick(MasterUi*, psy_ui_MouseEvent*);
static void masterui_drawhighlight(MasterUi*, psy_ui_Graphics*);
static void masterui_move(MasterUi*, psy_ui_Point topleft);
static void masterui_showparameters(MasterUi*, psy_ui_Component* parent);
static void masterui_onpreferredsize(MasterUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable masterui_vtable;
static psy_ui_ComponentVtable masterui_super_vtable;
static bool masterui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* masterui_vtable_init(MasterUi* self)
{
	assert(self);

	if (!masterui_vtable_initialized) {
		masterui_vtable = *(self->component.vtable);
		masterui_super_vtable = masterui_vtable;
		masterui_vtable.dispose = (psy_ui_fp_component_dispose)masterui_dispose;
		masterui_vtable.ondraw = (psy_ui_fp_component_ondraw)masterui_ondraw;		
		masterui_vtable.onmousedoubleclick = (psy_ui_fp_component_onmouseevent)
			masterui_onmousedoubleclick;
		masterui_vtable.move = (psy_ui_fp_component_move)masterui_move;		
		masterui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			masterui_onpreferredsize;
		masterui_vtable_initialized = TRUE;
	}
	return &masterui_vtable;
}

/* implementation */
void masterui_init(MasterUi* self, psy_ui_Component* parent, MachineViewSkin* skin,
	ParamViews* paramviews, Workspace* workspace)
{
	assert(self);
	assert(parent);
	assert(workspace);
	assert(workspace->song);
	assert(skin);	

	psy_ui_component_init(&self->component, parent, NULL);	
	masterui_vtable_init(self);
	self->component.vtable = &masterui_vtable;
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	machineuicommon_init(&self->intern, &self->component, psy_audio_MASTER_INDEX,
		skin, paramviews, workspace);
	self->intern.coords = &skin->master;
	self->intern.font = skin->effect_fontcolour;
	self->intern.bgcolour = psy_ui_colour_make(0x00333333);
	masterui_initsize(self);	
}

void masterui_dispose(MasterUi* self)
{
	assert(self);
	
	free(self->intern.restorename);	
	masterui_super_vtable.dispose(&self->component);
}

void masterui_initsize(MasterUi* self)
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
}

void masterui_move(MasterUi* self, psy_ui_Point topleft)
{
	assert(self);

	masterui_super_vtable.move(&self->component, topleft);
	machineuicommon_move(&self->intern, topleft);	
}

psy_ui_RealRectangle masterui_coordposition(MasterUi* self, SkinCoord* coord)
{
	psy_ui_RealRectangle r;

	assert(self);

	r = psy_ui_component_position(&self->component);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			r.left + coord->dest.left,
			r.top + coord->dest.top),
		psy_ui_realrectangle_size(&coord->dest));
}

void masterui_ondraw(MasterUi* self, psy_ui_Graphics* g)
{
	assert(self);

	masterui_drawbackground(self, g);	
}

void masterui_drawbackground(MasterUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (!psy_ui_bitmap_empty(&self->intern.skin->skinbmp)) {
		skin_blitcoord(g, &self->intern.skin->skinbmp, psy_ui_realpoint_zero(),
			&self->intern.coords->background);
	} else {		
		psy_ui_drawsolidrectangle(g, self->intern.coords->background.dest,
			self->intern.bgcolour);		
		psy_ui_textoutrectangle(g,
			psy_ui_realrectangle_topleft(&self->intern.coords->name.dest),
			psy_ui_ETO_CLIPPED, self->intern.coords->name.dest, "Master",
			psy_strlen("Master"));
	}
}

void masterui_showparameters(MasterUi* self, psy_ui_Component* parent)
{	
	if (self->intern.paramviews) {
		paramviews_show(self->intern.paramviews, self->intern.slot);
	}
}

void masterui_onmousedoubleclick(MasterUi* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 1) {
		if (self->component.view) {
			masterui_showparameters(self, self->component.view);
		}
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void masterui_onshowparameters(MasterUi* self, Workspace* sender,
	uintptr_t slot)
{
	if (slot == self->intern.slot) {
		if (self->component.view) {
			masterui_showparameters(self, self->component.view);
		}
	}
}

void masterui_onpreferredsize(MasterUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_RealSize sizepx;
	sizepx = psy_ui_realrectangle_size(&self->intern.coords->background.dest);
	*rv = psy_ui_size_make_px(sizepx.width, sizepx.height);
}

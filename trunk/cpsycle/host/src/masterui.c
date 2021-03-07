// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "masterui.h"
// host
#include "skingraphics.h"

// MasterUi
// prototypes
static void masterui_dispose(MasterUi*);
static void masterui_initsize(MasterUi*);
static void masterui_ondraw(MasterUi*, psy_ui_Graphics*);
static void masterui_drawbackground(MasterUi*, psy_ui_Graphics*);
static void masterui_onmousedoubleclick(MasterUi*, psy_ui_MouseEvent*);
static void masterui_drawhighlight(MasterUi*, psy_ui_Graphics*);
static void masterui_onframedestroyed(MasterUi*, psy_ui_Component* sender);
static void masterui_move(MasterUi*, psy_ui_Point topleft);
static void masterui_invalidate(MasterUi*);
static void masterui_onshowparameters(MasterUi*, Workspace* sender,
	uintptr_t slot);
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
		masterui_vtable.invalidate = (psy_ui_fp_component_invalidate)
			masterui_invalidate;
		masterui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			masterui_onpreferredsize;
		masterui_vtable_initialized = TRUE;
	}
	return &masterui_vtable;
}
// implementation
void masterui_init(MasterUi* self, psy_ui_Component* parent, MachineViewSkin* skin,
	psy_ui_Component* view, Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(workspace->song);
	assert(skin);
	assert(view);

	psy_ui_component_init(&self->component, parent, view);
	masterui_vtable_init(self);
	self->component.vtable = &masterui_vtable;	
	machineuicommon_init(&self->intern, psy_audio_MASTER_INDEX, skin, view,
		NULL, workspace);
	self->intern.coords = &skin->master;
	self->intern.font = skin->effect_fontcolour;
	self->intern.bgcolour = psy_ui_colour_make(0x00333333);
	masterui_initsize(self);
	psy_signal_connect(&workspace->signal_showparameters, self,
		masterui_onshowparameters);
}

void masterui_dispose(MasterUi* self)
{
	assert(self);

	if (self->intern.paramview) {
		psy_ui_component_destroy(&self->intern.paramview->component);
		free(self->intern.paramview);
	}
	if (self->intern.machineframe) {
		psy_ui_component_destroy(&self->intern.machineframe->component);
		free(self->intern.machineframe);
	}
	free(self->intern.restorename);
	psy_signal_disconnect(&self->intern.workspace->signal_showparameters, self,
		masterui_onshowparameters);
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
			psy_ui_point_makepx(topleft.x, topleft.y),
			psy_ui_size_makepx(size.width, size.height)));	
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
		psy_ui_RealRectangle r;

		r = psy_ui_component_position(&self->component);
		psy_ui_drawsolidrectangle(g, r, self->intern.bgcolour);		
		psy_ui_textoutrectangle(g,
			psy_ui_realrectangle_topleft(&self->intern.coords->name.dest),
			psy_ui_ETO_CLIPPED, self->intern.coords->name.dest, "Master",
			strlen("Master"));		
	}
}

void masterui_showparameters(MasterUi* self, psy_ui_Component* parent)
{
	if (self->intern.machine) {
		if (!self->intern.machineframe) {
			self->intern.machineframe = machineframe_alloc();
			machineframe_init(self->intern.machineframe, parent, self->intern.workspace);
			psy_signal_connect(&self->intern.machineframe->component.signal_destroy,
				self, masterui_onframedestroyed);
			if (psy_audio_machine_haseditor(self->intern.machine)) {
				MachineEditorView* editorview;

				editorview = machineeditorview_allocinit(
					psy_ui_notebook_base(&self->intern.machineframe->notebook),
					self->intern.machine, self->intern.workspace);
				if (editorview) {
					machineframe_setview(self->intern.machineframe,
						&editorview->component, self->intern.machine);
				}
			} else {
				ParamView* paramview;

				paramview = paramview_allocinit(
					&self->intern.machineframe->notebook.component,
					self->intern.machine, self->intern.workspace);
				if (paramview) {
					machineframe_setparamview(self->intern.machineframe, paramview,
						self->intern.machine);
				}
			}
		}
		if (self->intern.machineframe) {
			psy_ui_component_show(&self->intern.machineframe->component);
		}
	}
}

void masterui_onframedestroyed(MasterUi* self, psy_ui_Component* sender)
{
	self->intern.machineframe = NULL;
}

void masterui_onmousedoubleclick(MasterUi* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {				
		masterui_showparameters(self, self->intern.view);		
		psy_ui_mouseevent_stoppropagation(ev);
	}
}

void masterui_invalidate(MasterUi* self)
{
	if (!machineui_vuupdate()) {
		masterui_super_vtable.invalidate(&self->component);
	}
}

void masterui_onshowparameters(MasterUi* self, Workspace* sender,
	uintptr_t slot)
{
	if (slot == self->intern.slot) {
		masterui_showparameters(self, self->intern.view);
	}
}

void masterui_onpreferredsize(MasterUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_RealSize sizepx;
	sizepx = psy_ui_realrectangle_size(&self->intern.coords->background.dest);
	*rv = psy_ui_size_makepx(sizepx.width, sizepx.height);
}

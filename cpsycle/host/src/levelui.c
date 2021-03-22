// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "levelui.h"
// host
#include "machineparamconfig.h"
#include "skingraphics.h"
// audio
#include <machine.h>
#include <plugin_interface.h>
// platform
#include "../../detail/portable.h"

// LevelUi
// prototypes
static void levelui_ondraw(LevelUi*, psy_ui_Graphics*);
static void levelui_onpreferredsize(LevelUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void levelui_vumeterdraw(LevelUi*, psy_ui_Graphics*,
	psy_ui_RealPoint topleft, double value);
static void levelui_updateparam(LevelUi*);

// vtable
static psy_ui_ComponentVtable levelui_vtable;
static bool levelui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* levelui_vtable_init(LevelUi* self)
{
	assert(self);

	if (!levelui_vtable_initialized) {
		levelui_vtable = *(self->component.vtable);
		levelui_vtable.ondraw = (psy_ui_fp_component_ondraw)levelui_ondraw;		
		levelui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			levelui_onpreferredsize;		
		levelui_vtable_initialized = TRUE;
	}
	return &levelui_vtable;
}
// implementation
void levelui_init(LevelUi* self, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param,
	ParamSkin* skin)
{
	assert(self);	
	assert(skin);	

	psy_ui_component_init(&self->component, parent, view);
	levelui_vtable_init(self);
	self->component.vtable = &levelui_vtable;
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	self->view = view;	
	self->skin = skin;
	self->machine = machine;
	self->paramidx = paramidx;
	self->param = param;	
}

LevelUi* levelui_alloc(void)
{
	return (LevelUi*)malloc(sizeof(LevelUi));
}

LevelUi* levelui_allocinit(psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param, ParamSkin* paramskin)
{
	LevelUi* rv;

	rv = levelui_alloc();
	if (rv) {
		levelui_init(rv, parent, view, machine, paramidx, param, paramskin);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void levelui_ondraw(LevelUi* self, psy_ui_Graphics* g)
{		
	levelui_updateparam(self);
	if (self->param) {
		levelui_vumeterdraw(self, g, psy_ui_realpoint_zero(),
			(double)psy_audio_machineparam_normvalue(self->param));
	} else {
		levelui_vumeterdraw(self, g, psy_ui_realpoint_zero(), 0.0);
	}
}

void levelui_vumeterdraw(LevelUi* self, psy_ui_Graphics* g,
	psy_ui_RealPoint topleft, double value)
{
	double ypos;

	if (value < 0.0) value = 0.0;
	if (value > 1.0) value = 1.0;
	ypos = (1.0 - value) * psy_ui_realrectangle_height(&self->skin->vuoff.dest);
	psy_ui_drawbitmap(g, &self->skin->mixerbitmap,
		psy_ui_realrectangle_make(
			topleft,
			psy_ui_realsize_make(
				psy_ui_realrectangle_width(&self->skin->vuoff.dest),
				ypos)),
		psy_ui_realrectangle_topleft(&self->skin->vuoff.src));
	psy_ui_drawbitmap(g, &self->skin->mixerbitmap,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(topleft.x, topleft.y + ypos),
			psy_ui_realsize_make(
				psy_ui_realrectangle_width(&self->skin->vuoff.dest),
				psy_ui_realrectangle_height(&self->skin->vuoff.dest) - ypos)),
		psy_ui_realpoint_make(
			self->skin->vuon.src.left,
			self->skin->vuon.src.top + ypos));
}

void levelui_onpreferredsize(LevelUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_size_setreal(rv,
		psy_ui_realrectangle_size(&self->skin->vuoff.dest));	
}

void levelui_updateparam(LevelUi* self)
{
	if (self->machine && self->paramidx != psy_INDEX_INVALID) {
		self->param = psy_audio_machine_parameter(self->machine,
			self->paramidx);
	}
}
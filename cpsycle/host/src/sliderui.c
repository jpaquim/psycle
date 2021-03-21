// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sliderui.h"
// host
#include "skingraphics.h"
#include "machineui.h"
// audio
#include <exclusivelock.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// SliderUi
// prototypes
static void sliderui_dispose(SliderUi*);
static void sliderui_ondraw(SliderUi*, psy_ui_Graphics*);
static void sliderui_invalidate(SliderUi*);
static void sliderui_onpreferredsize(SliderUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void sliderui_onmousedown(SliderUi*, psy_ui_MouseEvent*);
static void sliderui_onmouseup(SliderUi*, psy_ui_MouseEvent*);
static void sliderui_onmousemove(SliderUi*, psy_ui_MouseEvent*);

// vtable
static psy_ui_ComponentVtable sliderui_vtable;
static psy_ui_ComponentVtable sliderui_super_vtable;
static bool sliderui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* sliderui_vtable_init(SliderUi* self)
{
	assert(self);

	if (!sliderui_vtable_initialized) {
		sliderui_vtable = *(self->component.vtable);
		sliderui_super_vtable = sliderui_vtable;
		sliderui_vtable.dispose = (psy_ui_fp_component_dispose)sliderui_dispose;
		sliderui_vtable.ondraw = (psy_ui_fp_component_ondraw)sliderui_ondraw;		
		sliderui_vtable.invalidate = (psy_ui_fp_component_invalidate)
			sliderui_invalidate;
		sliderui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			sliderui_onpreferredsize;
		sliderui_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)sliderui_onmousedown;
		sliderui_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)sliderui_onmouseup;
		sliderui_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)sliderui_onmousemove;
		sliderui_vtable_initialized = TRUE;
	}
	return &sliderui_vtable;
}
// implementation
void sliderui_init(SliderUi* self, psy_ui_Component* parent,
	psy_ui_Component* view,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param,
	ParamSkin* skin)
{
	assert(self);	
	assert(skin);

	psy_ui_component_init(&self->component, parent, view);
	sliderui_vtable_init(self);
	self->component.vtable = &sliderui_vtable;
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	self->view = view;	
	self->skin = skin;
	self->machine = machine;
	self->paramidx = paramidx;
	self->param = param;
	paramtweak_init(&self->paramtweak);
}

SliderUi* sliderui_alloc(void)
{
	return (SliderUi*)malloc(sizeof(SliderUi));
}

SliderUi* sliderui_allocinit(psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param, ParamSkin* paramskin)
{
	SliderUi* rv;

	rv = sliderui_alloc();
	if (rv) {
		sliderui_init(rv, parent, view, machine, paramidx, param, paramskin);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void sliderui_dispose(SliderUi* self)
{
	assert(self);
	
	sliderui_super_vtable.dispose(&self->component);
}

void sliderui_ondraw(SliderUi* self, psy_ui_Graphics* g)
{
	double xoffset;
	double yoffset;
	double value;
	psy_ui_RealRectangle r;		
	psy_ui_RealSize size;

	size = psy_ui_component_sizepx(&self->component);
	// todo: make the slider scalable	
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolour);
	skin_blitcoord(g, &self->skin->mixerbitmap,
		psy_ui_realpoint_zero(), &self->skin->slider);
	xoffset = (psy_ui_realrectangle_width(&self->skin->slider.dest) -
		psy_ui_realrectangle_width(&self->skin->knob.dest)) / 2;
	if (self->param) {
		if (self->machine) {
			value = psy_audio_machine_parameter_normvalue(self->machine,
				self->param);
		} else {
			value = psy_audio_machineparam_normvalue(self->param);
		}
	} else {
		value = 0.f;
	}
	yoffset = ((1.0 - value) *
		(psy_ui_realrectangle_height(&self->skin->slider.dest) -
			psy_ui_realrectangle_height(&self->skin->sliderknob.dest)));
	skin_blitcoord(g, &self->skin->mixerbitmap,
		psy_ui_realpoint_make(xoffset, yoffset),
		&self->skin->sliderknob);			
}


void sliderui_invalidate(SliderUi* self)
{
	if (!machineui_vuupdate()) {
		sliderui_super_vtable.invalidate(&self->component);
	}
}

void sliderui_onpreferredsize(SliderUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	psy_ui_size_setreal(rv, psy_ui_realrectangle_size(
		&self->skin->slider.dest));
}

void sliderui_onmousedown(SliderUi* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1 && self->param != NULL) {
		paramtweak_begin(&self->paramtweak, NULL, psy_INDEX_INVALID);
		self->paramtweak.param = self->param;
		paramtweak_onmousedown(&self->paramtweak, ev);
		psy_ui_component_capture(&self->component);
	}
}

void sliderui_onmousemove(SliderUi* self, psy_ui_MouseEvent* ev)
{
	if (self->paramtweak.param != NULL) {
		paramtweak_onmousemove(&self->paramtweak, ev);
		psy_ui_component_invalidate(&self->component);
	}
}

void sliderui_onmouseup(SliderUi* self, psy_ui_MouseEvent* ev)
{
	paramtweak_end(&self->paramtweak);
	psy_ui_component_releasecapture(&self->component);
	psy_ui_component_invalidate(&self->component);
}

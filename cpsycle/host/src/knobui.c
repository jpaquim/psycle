// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "knobui.h"
// host
#include "skingraphics.h"
#include "paramview.h"
#include "machineui.h"
#include "wireview.h"
// audio
#include <exclusivelock.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// KnobUi
// prototypes
static void knobui_ondraw(KnobUi*, psy_ui_Graphics*);
static void knobui_invalidate(KnobUi*);
static void knobui_onpreferredsize(KnobUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void knobui_onmousedown(KnobUi*, psy_ui_MouseEvent*);
static void knobui_onmouseup(KnobUi*, psy_ui_MouseEvent*);
static void knobui_onmousemove(KnobUi*, psy_ui_MouseEvent*);

// vtable
static psy_ui_ComponentVtable knobui_vtable;
static psy_ui_ComponentVtable knobui_super_vtable;
static bool knobui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* knobui_vtable_init(KnobUi* self)
{
	assert(self);

	if (!knobui_vtable_initialized) {
		knobui_vtable = *(self->component.vtable);
		knobui_super_vtable = knobui_vtable;
		knobui_vtable.ondraw = (psy_ui_fp_component_ondraw)knobui_ondraw;		
		knobui_vtable.invalidate = (psy_ui_fp_component_invalidate)
			knobui_invalidate;
		knobui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			knobui_onpreferredsize;
		knobui_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			knobui_onmousedown;
		knobui_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)
			knobui_onmouseup;
		knobui_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)
			knobui_onmousemove;
		knobui_vtable_initialized = TRUE;
	}
	return &knobui_vtable;
}
// implementation
void knobui_init(KnobUi* self, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam* param,
	ParamSkin* paramskin)
{
	assert(self);	
	assert(paramskin);	

	psy_ui_component_init(&self->component, parent, view);
	psy_ui_component_setvtable(&self->component,
		knobui_vtable_init(self));	
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);	
	self->skin = paramskin;
	self->param = param;
	paramtweak_init(&self->paramtweak);
}

KnobUi* knobui_alloc(void)
{
	return (KnobUi*)malloc(sizeof(KnobUi));
}

KnobUi* knobui_allocinit(psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_MachineParam* param, ParamSkin* paramskin)
{
	KnobUi* rv;

	rv = knobui_alloc();
	if (rv) {
		knobui_init(rv, parent, view, param, paramskin);
		rv->component.deallocate = TRUE;
	}
	return rv;
}


void knobui_ondraw(KnobUi* self, psy_ui_Graphics* g)
{	
	KnobDraw knobdraw;

	psy_ui_RealSize size;

	size = psy_ui_component_sizepx(&self->component);
	knobdraw_init(&knobdraw, self->skin, NULL, self->param,
		size, psy_ui_component_textmetric(&self->component),
		FALSE); // self->tweak == paramnum);
	knobdraw_draw(&knobdraw, g);
}

void knobui_invalidate(KnobUi* self)
{
	if (!machineui_vuupdate()) {
		knobui_super_vtable.invalidate(&self->component);
	}
}

void knobui_onpreferredsize(KnobUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	psy_ui_size_setem(rv, 10.0, 2.0);
}

void knobui_onmousedown(KnobUi* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1 && self->param != NULL) {		
		paramtweak_begin(&self->paramtweak, NULL, psy_INDEX_INVALID);
		self->paramtweak.param = self->param;
		paramtweak_onmousedown(&self->paramtweak, ev);		
		psy_ui_component_capture(&self->component);		
	}
}

void knobui_onmousemove(KnobUi* self, psy_ui_MouseEvent* ev)
{
	if (self->paramtweak.param != NULL) {
		paramtweak_onmousemove(&self->paramtweak, ev);
		psy_ui_component_invalidate(&self->component);
	}
}

void knobui_onmouseup(KnobUi* self, psy_ui_MouseEvent* ev)
{
	paramtweak_end(&self->paramtweak);
	psy_ui_component_releasecapture(&self->component);
	psy_ui_component_invalidate(&self->component);
}

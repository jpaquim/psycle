// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "switchui.h"
// host
#include "skingraphics.h"
// platform
#include "../../detail/portable.h"

// SwitchUi
// prototypes
static void switchui_ondraw(SwitchUi*, psy_ui_Graphics*);
static void switchui_onpreferredsize(SwitchUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void switchui_onmousedown(SwitchUi*, psy_ui_MouseEvent*);
static void switchui_onmouseup(SwitchUi*, psy_ui_MouseEvent*);
static void switchui_onmousemove(SwitchUi*, psy_ui_MouseEvent*);

// vtable
static psy_ui_ComponentVtable switchui_vtable;
static bool switchui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* switchui_vtable_init(SwitchUi* self)
{
	assert(self);

	if (!switchui_vtable_initialized) {
		switchui_vtable = *(self->component.vtable);
		switchui_vtable.ondraw = (psy_ui_fp_component_ondraw)switchui_ondraw;		
		switchui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			switchui_onpreferredsize;
		switchui_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			switchui_onmousedown;
		switchui_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)
			switchui_onmouseup;
		switchui_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)
			switchui_onmousemove;
		switchui_vtable_initialized = TRUE;
	}
	return &switchui_vtable;
}
// implementation
void switchui_init(SwitchUi* self, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam* param,
	ParamSkin* paramskin)
{
	assert(self);	
	assert(paramskin);	
	assert(view);

	psy_ui_component_init(&self->component, parent, view);
	switchui_vtable_init(self);
	self->component.vtable = &switchui_vtable;
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	self->view = view;	
	self->skin = paramskin;
	self->param = param;
	paramtweak_init(&self->paramtweak);
}

SwitchUi* switchui_alloc(void)
{
	return (SwitchUi*)malloc(sizeof(SwitchUi));
}

SwitchUi* switchui_allocinit(psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_MachineParam* param, ParamSkin* paramskin)
{
	SwitchUi* rv;

	rv = switchui_alloc();
	if (rv) {
		switchui_init(rv, parent, view, param, paramskin);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void switchui_ondraw(SwitchUi* self, psy_ui_Graphics* g)
{	
	psy_ui_RealSize size;				
	psy_ui_RealRectangle r;
	char label[128];
	
	size = psy_ui_component_sizepx(&self->component);
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolour);
	
	if (!self->param || psy_audio_machineparam_normvalue(self->param) == 0.f) {
		skin_blitcoord(g, &self->skin->mixerbitmap,
			psy_ui_realpoint_zero(), &self->skin->switchoff);
	} 	else {
		skin_blitcoord(g, &self->skin->mixerbitmap,
			psy_ui_realpoint_zero(), &self->skin->switchon);
	}	
	psy_snprintf(label, 128, "%s", "");
	if (self->param && !psy_audio_machineparam_name(self->param, label)) {
		if (!psy_audio_machineparam_label(self->param, label)) {
			psy_snprintf(label, 128, "%s", "");
		}
	}
	psy_ui_setbackgroundcolour(g, self->skin->topcolour);
	psy_ui_settextcolour(g, self->skin->fonttopcolour);
	psy_ui_setrectangle(&r,
		psy_ui_realrectangle_width(&self->skin->switchon.dest), 0,
		size.width - psy_ui_realrectangle_width(&self->skin->switchon.dest),
		size.height / 2);
	psy_ui_textoutrectangle(g,
		psy_ui_realpoint_make(
			psy_ui_realrectangle_width(&self->skin->switchon.dest), 0.0),
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, label, strlen(label));	
}

void switchui_onpreferredsize(SwitchUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	psy_ui_size_setem(rv, 10.0, 2.0);
}

void switchui_onmousedown(SwitchUi* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1 && self->param != NULL) {		
		paramtweak_begin(&self->paramtweak, NULL, psy_INDEX_INVALID);
		self->paramtweak.param = self->param;
		paramtweak_onmousedown(&self->paramtweak, ev);		
		psy_ui_component_capture(&self->component);		
	}
}

void switchui_onmousemove(SwitchUi* self, psy_ui_MouseEvent* ev)
{
	if (self->paramtweak.param != NULL) {
		paramtweak_onmousemove(&self->paramtweak, ev);
		psy_ui_component_invalidate(&self->component);
	}
}

void switchui_onmouseup(SwitchUi* self, psy_ui_MouseEvent* ev)
{
	paramtweak_end(&self->paramtweak);
	psy_ui_component_releasecapture(&self->component);
	psy_ui_component_invalidate(&self->component);
}

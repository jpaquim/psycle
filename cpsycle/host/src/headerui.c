// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "headerui.h"
// host
#include "skingraphics.h"
#include "machineparamconfig.h"
// audio
#include <machine.h>
#include <plugin_interface.h>
// platform
#include "../../detail/portable.h"

// HeaderUi
// prototypes
static void headerui_ondraw(HeaderUi*, psy_ui_Graphics*);
static void headerui_onpreferredsize(HeaderUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void headerui_updateparam(HeaderUi*);

// vtable
static psy_ui_ComponentVtable headerui_vtable;
static bool headerui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* headerui_vtable_init(HeaderUi* self)
{
	assert(self);

	if (!headerui_vtable_initialized) {
		headerui_vtable = *(self->component.vtable);	
		headerui_vtable.ondraw = (psy_ui_fp_component_ondraw)headerui_ondraw;		
		headerui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			headerui_onpreferredsize;		
		headerui_vtable_initialized = TRUE;
	}
	return &headerui_vtable;
}
// implementation
void headerui_init(HeaderUi* self, psy_ui_Component* parent,
	psy_ui_Component* view,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param, ParamSkin* skin)
{
	assert(self);	
	assert(skin);	

	psy_ui_component_init(&self->component, parent, view);
	headerui_vtable_init(self);
	self->component.vtable = &headerui_vtable;
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	self->view = view;	
	self->skin = skin;
	self->machine = machine;
	self->paramidx = paramidx,
	self->param = param;	
}

HeaderUi* headerui_alloc(void)
{
	return (HeaderUi*)malloc(sizeof(HeaderUi));
}

HeaderUi* headerui_allocinit(psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param, ParamSkin* paramskin)
{
	HeaderUi* rv;

	rv = headerui_alloc();
	if (rv) {
		headerui_init(rv, parent, view, machine, paramidx, param, paramskin);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void headerui_ondraw(HeaderUi* self, psy_ui_Graphics* g)
{		
	double half;
	double quarter;
	psy_ui_RealRectangle r;
	char str[128];
	psy_ui_RealSize size;

	headerui_updateparam(self);
	size = psy_ui_component_innersize_px(&self->component);
	half = size.height / 2;
	quarter = half / 2;
	psy_ui_realrectangle_init_all(&r, psy_ui_realpoint_zero(), size);
	psy_ui_drawsolidrectangle(g, r, self->skin->topcolour);
	psy_ui_setrectangle(&r, 0, half + quarter, size.width, quarter);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolour);
	if (!psy_audio_machineparam_name(self->param, str)) {
		if (!psy_audio_machineparam_label(self->param, str)) {
			psy_snprintf(str, 128, "%s", "");
		}
	}
	psy_ui_setrectangle(&r, 0, quarter, size.width, half);
	psy_ui_setbackgroundcolour(g, self->skin->titlecolour);
	psy_ui_settextcolour(g, self->skin->fonttitlecolour);
	// todo font_bold
	psy_ui_textoutrectangle(g, psy_ui_realpoint_make(0, quarter),
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r, str,
		psy_strlen(str));
}

void headerui_onpreferredsize(HeaderUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	headerui_updateparam(self);
	if (self->param) {
		if (psy_audio_machineparam_type(self->param) & MPF_SMALL) {
			psy_ui_size_setem(rv, self->skin->paramwidth_small, 2.0);
			return;
		}
	}
	psy_ui_size_setem(rv, self->skin->paramwidth, 2.0);
}

void headerui_updateparam(HeaderUi* self)
{
	if (self->machine && self->paramidx != psy_INDEX_INVALID) {
		self->param = psy_audio_machine_parameter(self->machine,
			self->paramidx);
	}
}

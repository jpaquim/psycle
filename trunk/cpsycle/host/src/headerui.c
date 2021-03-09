// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "headerui.h"
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

// HeaderUi
// prototypes
static void headerui_ondraw(HeaderUi*, psy_ui_Graphics*);
static void headerui_invalidate(HeaderUi*);
static void headerui_onpreferredsize(HeaderUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);

// vtable
static psy_ui_ComponentVtable headerui_vtable;
static psy_ui_ComponentVtable headerui_super_vtable;
static bool headerui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* headerui_vtable_init(HeaderUi* self)
{
	assert(self);

	if (!headerui_vtable_initialized) {
		headerui_vtable = *(self->component.vtable);
		headerui_super_vtable = headerui_vtable;
		headerui_vtable.ondraw = (psy_ui_fp_component_ondraw)headerui_ondraw;
		headerui_vtable.invalidate = (psy_ui_fp_component_invalidate)
			headerui_invalidate;
		headerui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			headerui_onpreferredsize;		
		headerui_vtable_initialized = TRUE;
	}
	return &headerui_vtable;
}
// implementation
void headerui_init(HeaderUi* self, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam* param,
	ParamSkin* skin)
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
	self->param = param;	
}

void headerui_ondraw(HeaderUi* self, psy_ui_Graphics* g)
{		
	double half;
	double quarter;
	psy_ui_RealRectangle r;
	char str[128];
	psy_ui_RealSize size;

	size = psy_ui_component_sizepx(&self->component);
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
		strlen(str));
}

void headerui_invalidate(HeaderUi* self)
{
	if (!machineui_vuupdate()) {
		headerui_super_vtable.invalidate(&self->component);
	}
}

void headerui_onpreferredsize(HeaderUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_size_setem(rv, 10.0, 2.0);
}

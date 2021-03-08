// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "labelui.h"
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

// LabelUi
// prototypes
static void labelui_ondraw(LabelUi*, psy_ui_Graphics*);
static void labelui_invalidate(LabelUi*);
static void labelui_onpreferredsize(LabelUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);

// vtable
static psy_ui_ComponentVtable labelui_vtable;
static psy_ui_ComponentVtable labelui_super_vtable;
static bool labelui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* labelui_vtable_init(LabelUi* self)
{
	assert(self);

	if (!labelui_vtable_initialized) {
		labelui_vtable = *(self->component.vtable);
		labelui_super_vtable = labelui_vtable;
		labelui_vtable.ondraw = (psy_ui_fp_component_ondraw)labelui_ondraw;
		labelui_vtable.invalidate = (psy_ui_fp_component_invalidate)
			labelui_invalidate;
		labelui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			labelui_onpreferredsize;		
		labelui_vtable_initialized = TRUE;
	}
	return &labelui_vtable;
}
// implementation
void labelui_init(LabelUi* self, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam* param,
	ParamSkin* skin)
{
	assert(self);	
	assert(skin);	

	psy_ui_component_init(&self->component, parent, view);
	labelui_vtable_init(self);
	self->component.vtable = &labelui_vtable;
	self->view = view;	
	self->skin = skin;
	self->param = param;	
}

void labelui_ondraw(LabelUi* self, psy_ui_Graphics* g)
{		
	double half;
	psy_ui_RealRectangle r;
	char str[128];
	psy_ui_RealSize size;

	str[0] = '\0';
	size = psy_ui_component_sizepx(&self->component);
	half = size.height / 2;
	psy_ui_setrectangle(&r, 0.0, 0.0, size.width, half);
	psy_ui_setbackgroundcolour(g, self->skin->titlecolour);
	psy_ui_settextcolour(g, self->skin->fonttitlecolour);
	if (!psy_audio_machineparam_name(self->param, str)) {
		if (!psy_audio_machineparam_label(self->param, str)) {
			psy_snprintf(str, 128, "%s", "");
		}
	}
	psy_ui_textoutrectangle(g,
		psy_ui_realpoint_zero(), psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, strlen(str));
	if (psy_audio_machineparam_describe(self->param, str) == FALSE) {
		psy_snprintf(str, 128, "%s", "");
	}
	psy_ui_setbackgroundcolour(g, self->skin->bottomcolour);
	psy_ui_settextcolour(g, self->skin->fontbottomcolour);
	psy_ui_setrectangle(&r, 0, half, size.width, half);
	psy_ui_textoutrectangle(g, psy_ui_realpoint_make(0, half),
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, strlen(str));
}

void labelui_invalidate(LabelUi* self)
{
	if (!machineui_vuupdate()) {
		labelui_super_vtable.invalidate(&self->component);
	}
}

void labelui_onpreferredsize(LabelUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_size_setpx(rv,
		100.0,
		psy_ui_realrectangle_height(&self->skin->knob.dest));
}

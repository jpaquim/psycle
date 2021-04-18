// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "labelui.h"
// host
#include "machineparamconfig.h"
// audio
#include <machine.h>
#include <plugin_interface.h>
// platform
#include "../../detail/portable.h"

// LabelUi
// prototypes
static void labelui_ondraw(LabelUi*, psy_ui_Graphics*);
static void labelui_invalidate(LabelUi*);
static void labelui_onpreferredsize(LabelUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void labelui_updateparam(LabelUi*);
// vtable
static psy_ui_ComponentVtable labelui_vtable;
static bool labelui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* labelui_vtable_init(LabelUi* self)
{
	assert(self);

	if (!labelui_vtable_initialized) {
		labelui_vtable = *(self->component.vtable);
		labelui_vtable.ondraw = (psy_ui_fp_component_ondraw)labelui_ondraw;
		labelui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			labelui_onpreferredsize;		
		labelui_vtable_initialized = TRUE;
	}
	return &labelui_vtable;
}
// implementation
void labelui_init(LabelUi* self, psy_ui_Component* parent,
	psy_ui_Component* view,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param,
	ParamSkin* skin)
{
	assert(self);	
	assert(skin);	

	psy_ui_component_init(&self->component, parent, view);
	psy_ui_component_setvtable(&self->component, labelui_vtable_init(self));	
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);	
	self->skin = skin;
	self->param = param;
	self->machine = machine;
	self->paramidx = paramidx;
}

LabelUi* labelui_alloc(void)
{
	return (LabelUi*)malloc(sizeof(LabelUi));
}

LabelUi* labelui_allocinit(psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_Machine* machine, uintptr_t paramidx, psy_audio_MachineParam* param,
	ParamSkin* paramskin)
{
	LabelUi* rv;

	rv = labelui_alloc();
	if (rv) {
		labelui_init(rv, parent, view, machine, paramidx, param, paramskin);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void labelui_ondraw(LabelUi* self, psy_ui_Graphics* g)
{		
	double half;
	psy_ui_RealRectangle r;
	char str[128];
	psy_ui_RealSize size;

	str[0] = '\0';
	labelui_updateparam(self);
	size = psy_ui_component_offsetsizepx(&self->component);
	half = size.height / 2;
	psy_ui_setrectangle(&r, 0.0, 0.0, size.width, half);
	psy_ui_setbackgroundcolour(g, self->skin->titlecolour);
	psy_ui_settextcolour(g, self->skin->fonttitlecolour);
	if (self->param) {
		if (!psy_audio_machineparam_name(self->param, str)) {
			if (!psy_audio_machineparam_label(self->param, str)) {
				psy_snprintf(str, 128, "%s", "");
			}
		}
	}
	psy_ui_textoutrectangle(g,
		psy_ui_realpoint_zero(), psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, strlen(str));
	if (self->param && psy_audio_machineparam_describe(self->param, str) == FALSE) {
		psy_snprintf(str, 128, "%s", "");
	}
	psy_ui_setbackgroundcolour(g, self->skin->bottomcolour);
	psy_ui_settextcolour(g, self->skin->fontbottomcolour);
	psy_ui_setrectangle(&r, 0, half, size.width, half);
	psy_ui_textoutrectangle(g, psy_ui_realpoint_make(0, half),
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, strlen(str));
}

void labelui_onpreferredsize(LabelUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	labelui_updateparam(self);
	if (self->param) {
		if (psy_audio_machineparam_type(self->param) & MPF_SMALL) {
			psy_ui_size_setem(rv, self->skin->paramwidth_small, 2.0);
			return;
		}
	}
	psy_ui_size_setem(rv, self->skin->paramwidth, 2.0);
}

void labelui_updateparam(LabelUi* self)
{
	if (self->machine && self->paramidx != psy_INDEX_INVALID) {
		self->param = psy_audio_machine_parameter(self->machine,
			self->paramidx);
	}
}

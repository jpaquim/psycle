// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "knobui.h"
// host
#include "skingraphics.h"
#include "machineparamconfig.h"
// audio
#include <machine.h>
#include <plugin_interface.h>
// platform
#include "../../detail/portable.h"

// KnobUi
// prototypes
static void knobui_ondraw(KnobUi*, psy_ui_Graphics*);
static void knobui_onpreferredsize(KnobUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void knobui_updateparam(KnobUi*);
static void knobui_onmousedown(KnobUi*, psy_ui_MouseEvent*);
static void knobui_onmouseup(KnobUi*, psy_ui_MouseEvent*);
static void knobui_onmousemove(KnobUi*, psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable knobui_vtable;
static bool knobui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* knobui_vtable_init(KnobUi* self)
{
	assert(self);

	if (!knobui_vtable_initialized) {
		knobui_vtable = *(self->component.vtable);		
		knobui_vtable.ondraw = (psy_ui_fp_component_ondraw)knobui_ondraw;		
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
	psy_ui_Component* view,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param,
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
	self->paramidx = paramidx;
	self->machine = machine;
	paramtweak_init(&self->paramtweak);	
}

KnobUi* knobui_alloc(void)
{
	return (KnobUi*)malloc(sizeof(KnobUi));
}

KnobUi* knobui_allocinit(psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param, ParamSkin* paramskin)
{
	KnobUi* rv;

	rv = knobui_alloc();
	if (rv) {
		knobui_init(rv, parent, view, machine, paramidx, param, paramskin);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void knobui_ondraw(KnobUi* self, psy_ui_Graphics* g)
{
	char label[128];
	char str[128];
	psy_ui_RealRectangle r_top;
	psy_ui_RealRectangle r_bottom;
	psy_ui_RealSize size;

	size = psy_ui_component_innersize_px(&self->component);
	knobui_updateparam(self);
	if (self->param) {
		psy_ui_setrectangle(&r_top,
			psy_ui_realrectangle_width(&self->skin->knob.dest), 0,
			size.width - psy_ui_realrectangle_width(&self->skin->knob.dest),
			size.height / 2);
	} else {
		psy_ui_setrectangle(&r_top, 0, 0, size.width, size.height / 2);
	}
	psy_ui_drawsolidrectangle(g, r_top, self->skin->topcolour);
	r_bottom = r_top;
	psy_ui_realrectangle_settopleft(&r_bottom,
		psy_ui_realpoint_make(r_top.left,
			r_top.top + psy_ui_realrectangle_height(&r_top)));
	psy_ui_drawsolidrectangle(g,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(0.0, size.height / 2),
			psy_ui_realsize_make(size.width, size.height / 2)),
		self->skin->bottomcolour);
	if (self->param) {
		if (self->machine) {
			if (!psy_audio_machine_parameter_name(self->machine, self->param, label)) {
				if (!psy_audio_machine_parameter_label(self->machine, self->param, label)) {
					psy_snprintf(label, 128, "%s", "");
				}
			}
			if (!psy_audio_machine_parameter_describe(self->machine, self->param, str)) {
				psy_snprintf(str, 128, "%d",
					(int)psy_audio_machineparam_scaledvalue(self->param));
			}
		} else {
			if (!psy_audio_machineparam_name(self->param, label)) {
				if (!psy_audio_machineparam_label(self->param, label)) {
					psy_snprintf(label, 128, "%s", "");
				}
			}
			if (!psy_audio_machineparam_describe(self->param, str)) {
				psy_snprintf(str, 128, "%d",
					(int)psy_audio_machineparam_scaledvalue(self->param));
			}
		}
		if ((paramtweak_active(&self->paramtweak))) {
			psy_ui_setbackgroundcolour(g, self->skin->htopcolour);
			psy_ui_settextcolour(g, self->skin->fonthtopcolour);
		} else {
			psy_ui_setbackgroundcolour(g, self->skin->topcolour);
			psy_ui_settextcolour(g, self->skin->fonttopcolour);
		}
		psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r_top),
			psy_ui_ETO_OPAQUE, r_top, label, psy_strlen(label));
		psy_ui_setbackgroundcolour(g, (paramtweak_active(&self->paramtweak))
			? self->skin->hbottomcolour : self->skin->bottomcolour);
		psy_ui_settextcolour(g, (paramtweak_active(&self->paramtweak))
			? self->skin->fonthbottomcolour : self->skin->fontbottomcolour);
		psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r_bottom),
			psy_ui_ETO_OPAQUE, r_bottom, str, psy_strlen(str));
		if (!psy_ui_bitmap_empty(&self->skin->knobbitmap)) {
			intptr_t knob_frame;

			if (self->param) {
				if (self->machine) {
					knob_frame = (intptr_t)(
						(psy_audio_machine_parameter_normvalue(self->machine,
							self->param) * 63.f));
				} else {
					knob_frame = (intptr_t)(
						(psy_audio_machineparam_normvalue(self->param) * 63.f));
				}
			} else {
				knob_frame = 0;
			}
			if (size.height < psy_ui_realrectangle_height(&self->skin->knob.dest)) {
				double ratio;
				double w;

				ratio = size.height / (double)psy_ui_realrectangle_height(&self->skin->knob.dest);
				w = ratio * psy_ui_realrectangle_width(&self->skin->knob.dest);
				psy_ui_drawstretchedbitmap(g, &self->skin->knobbitmap,
					psy_ui_realrectangle_make(psy_ui_realpoint_zero(),
						psy_ui_realsize_make(w, size.height)),
					psy_ui_realpoint_make(knob_frame *
						psy_ui_realrectangle_height(&self->skin->knob.dest), 0.0),
					psy_ui_realrectangle_size(&self->skin->knob.dest));
			} else {
				psy_ui_drawbitmap(g, &self->skin->knobbitmap,
					self->skin->knob.dest, psy_ui_realpoint_make(knob_frame *
						psy_ui_realrectangle_width(&self->skin->knob.dest), 0));
			}
		}
	}
	psy_ui_setcolour(g, psy_ui_colour_make(0x00232323));
	psy_ui_drawline(g,
		psy_ui_realpoint_make(0, size.height - 1),
		psy_ui_realpoint_make(size.width, size.height - 1));
	psy_ui_drawline(g,
		psy_ui_realpoint_make(size.width - 1, 0),
		psy_ui_realpoint_make(size.width - 1, size.height - 1));
}

void knobui_onpreferredsize(KnobUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	knobui_updateparam(self);
	if (self->param) {
		if (psy_audio_machineparam_type(self->param) & MPF_SMALL) {
			psy_ui_size_setem(rv, self->skin->paramwidth_small, 2.0);
			return;
		}
	}
	psy_ui_size_setem(rv, self->skin->paramwidth, 2.0);
}

void knobui_onmousedown(KnobUi* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		paramtweak_begin(&self->paramtweak, self->machine, self->paramidx,
			self->param);		
		paramtweak_onmousedown(&self->paramtweak, ev);		
		psy_ui_component_capture(&self->component);
	}
}

void knobui_onmousemove(KnobUi* self, psy_ui_MouseEvent* ev)
{		
	if (paramtweak_active(&self->paramtweak)) {
		paramtweak_onmousemove(&self->paramtweak, ev);	
		psy_ui_component_invalidate(&self->component);
	}
}

void knobui_onmouseup(KnobUi* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);
	if (paramtweak_active(&self->paramtweak)) {
		paramtweak_end(&self->paramtweak);		
		psy_ui_component_invalidate(&self->component);
	}
}

void knobui_updateparam(KnobUi* self)
{
	if (self->machine && self->paramidx != psy_INDEX_INVALID) {
		self->param = psy_audio_machine_parameter(self->machine,
			self->paramidx);
	}
}

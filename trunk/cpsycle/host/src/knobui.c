/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "knobui.h"
/* host */
#include "styles.h"
#include "machineparamconfig.h"
/* audio */
#include <machine.h>
#include <plugin_interface.h>
/* ui */
#include <uiapp.h>
/* platform */
#include "../../detail/portable.h"

/* KnobUi */

/* prototypes */
static void knobui_ondraw(KnobUi*, psy_ui_Graphics*);
static void knobui_onpreferredsize(KnobUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void knobui_updateparam(KnobUi*);
static void knobui_on_mouse_down(KnobUi*, psy_ui_MouseEvent*);
static void knobui_on_mouse_up(KnobUi*, psy_ui_MouseEvent*);
static void knobui_onmousemove(KnobUi*, psy_ui_MouseEvent*);

/* vtable */
static psy_ui_ComponentVtable knobui_vtable;
static bool knobui_vtable_initialized = FALSE;

static void knobui_vtable_init(KnobUi* self)
{
	assert(self);

	if (!knobui_vtable_initialized) {
		knobui_vtable = *(self->component.vtable);		
		knobui_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			knobui_ondraw;		
		knobui_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			knobui_onpreferredsize;
		knobui_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			knobui_on_mouse_down;
		knobui_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			knobui_on_mouse_up;
		knobui_vtable.onmousemove =
			(psy_ui_fp_component_on_mouse_event)
			knobui_onmousemove;
		knobui_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &knobui_vtable);
}

/* implementation */
void knobui_init(KnobUi* self, psy_ui_Component* parent,	
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	assert(self);	

	psy_ui_component_init(&self->component, parent, NULL);
	knobui_vtable_init(self);	
	psy_ui_component_set_style_type(&self->component, STYLE_MV_CHECK);
	self->param = param;
	self->paramidx = paramidx;
	self->machine = machine;
	paramtweak_init(&self->paramtweak);	
}

KnobUi* knobui_alloc(void)
{
	return (KnobUi*)malloc(sizeof(KnobUi));
}

KnobUi* knobui_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	KnobUi* rv;

	rv = knobui_alloc();
	if (rv) {
		knobui_init(rv, parent, machine, paramidx, param);
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
	psy_ui_Style* top_style;
	psy_ui_Style* bottom_style;
	psy_ui_Style* htop_style;
	psy_ui_Style* hbottom_style;
	psy_ui_Style* knob_style;

	top_style = psy_ui_style(STYLE_MACPARAM_TOP);
	bottom_style = psy_ui_style(STYLE_MACPARAM_BOTTOM);
	htop_style = psy_ui_style(STYLE_MACPARAM_TOP_ACTIVE);
	hbottom_style = psy_ui_style(STYLE_MACPARAM_BOTTOM_ACTIVE);
	knob_style = psy_ui_style(STYLE_MACPARAM_KNOB);
	if (!top_style || !bottom_style) {
		return;
	}
	size = psy_ui_component_size_px(&self->component);
	knobui_updateparam(self);	
	r_top = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0.0, 0.0),
		psy_ui_realsize_make(knob_style->background.animation.framesize.width,
			size.height));
	psy_ui_drawsolidrectangle(g, r_top, bottom_style->background.colour);
	r_top = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(knob_style->background.animation.framesize.width, 0.0),
		psy_ui_realsize_make(size.width -
			knob_style->background.animation.framesize.width, size.height / 2));
	psy_ui_drawsolidrectangle(g, r_top, top_style->background.colour);
	r_bottom = r_top;
	psy_ui_realrectangle_settopleft(&r_bottom,
		psy_ui_realpoint_make(
			r_top.left,
			r_top.top + psy_ui_realrectangle_height(&r_top)));
	psy_ui_drawsolidrectangle(g, r_bottom, bottom_style->background.colour);
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
			psy_ui_setbackgroundcolour(g, htop_style->background.colour);
			psy_ui_settextcolour(g, htop_style->colour);
		} else {
			psy_ui_setbackgroundcolour(g, top_style->background.colour);
			psy_ui_settextcolour(g, top_style->colour);
		}
		psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r_top),
			psy_ui_ETO_OPAQUE, r_top, label, psy_strlen(label));
		psy_ui_setbackgroundcolour(g,
			(paramtweak_active(&self->paramtweak))
			? hbottom_style->background.colour
			: bottom_style->background.colour);
		psy_ui_settextcolour(g,
			(paramtweak_active(&self->paramtweak))
			? hbottom_style->colour
			: bottom_style->colour);
		psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r_bottom),
			psy_ui_ETO_OPAQUE, r_bottom, str, psy_strlen(str));
		if (!psy_ui_bitmap_empty(&knob_style->background.bitmap)) {
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
			if (size.height < knob_style->background.animation.framesize.height) {
				double ratio;
				double w;

				ratio = size.height / (double)knob_style->background.animation.framesize.height;
				w = ratio * knob_style->background.animation.framesize.width;
				psy_ui_drawstretchedbitmap(g, &knob_style->background.bitmap,
					psy_ui_realrectangle_make(psy_ui_realpoint_zero(),
						psy_ui_realsize_make(w, size.height)),
					psy_ui_realpoint_make(knob_frame *
						knob_style->background.animation.framesize.height, 0.0),
					knob_style->background.animation.framesize);
			} else {
				psy_ui_drawbitmap(g, &knob_style->background.bitmap,
					psy_ui_realrectangle_make(
						psy_ui_realpoint_zero(),
						knob_style->background.animation.framesize),
						psy_ui_realpoint_make(knob_frame *
						knob_style->background.animation.framesize.width, 0));
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
			psy_ui_size_setem(rv, PARAMWIDTH_SMALL, 2.0);
			return;
		}
	}
	psy_ui_size_setem(rv, PARAMWIDTH, 2.0);
}

void knobui_on_mouse_down(KnobUi* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 1) {
		paramtweak_begin(&self->paramtweak, self->machine, self->paramidx,
			self->param);		
		paramtweak_on_mouse_down(&self->paramtweak, ev);		
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

void knobui_on_mouse_up(KnobUi* self, psy_ui_MouseEvent* ev)
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

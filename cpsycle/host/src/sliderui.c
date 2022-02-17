/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sliderui.h"
/* host */
#include "machineparamconfig.h"
#include "styles.h"
/* ui */
#include <uiapp.h>
/* audio */
#include <machine.h>
#include <plugin_interface.h>
/* platform */
#include "../../detail/portable.h"

/* SliderUi */

/* prototypes */
static void sliderui_ondraw(SliderUi*, psy_ui_Graphics*);
static void sliderui_onpreferredsize(SliderUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void sliderui_onmousedown(SliderUi*, psy_ui_MouseEvent*);
static void sliderui_onmouseup(SliderUi*, psy_ui_MouseEvent*);
static void sliderui_onmousemove(SliderUi*, psy_ui_MouseEvent*);
static void sliderui_updateparam(SliderUi*);

/* vtable */
static psy_ui_ComponentVtable sliderui_vtable;
static bool sliderui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* sliderui_vtable_init(SliderUi* self)
{
	assert(self);

	if (!sliderui_vtable_initialized) {
		sliderui_vtable = *(self->component.vtable);				
		sliderui_vtable.ondraw = (psy_ui_fp_component_ondraw)sliderui_ondraw;				
		sliderui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			sliderui_onpreferredsize;
		sliderui_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)sliderui_onmousedown;
		sliderui_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)sliderui_onmouseup;
		sliderui_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)sliderui_onmousemove;
		sliderui_vtable_initialized = TRUE;
	}
	return &sliderui_vtable;
}

/* implementation */
void sliderui_init(SliderUi* self, psy_ui_Component* parent,	
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	assert(self);	

	psy_ui_component_init(&self->component, parent, NULL);
	sliderui_vtable_init(self);
	self->component.vtable = &sliderui_vtable;
	psy_ui_component_setstyletype(&self->component, STYLE_MACPARAM_SLIDER);
	self->machine = machine;
	self->paramidx = paramidx;
	self->param = param;
	paramtweak_init(&self->paramtweak);
}

SliderUi* sliderui_alloc(void)
{
	return (SliderUi*)malloc(sizeof(SliderUi));
}

SliderUi* sliderui_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	SliderUi* rv;

	rv = sliderui_alloc();
	if (rv) {
		sliderui_init(rv, parent, machine, paramidx, param);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void sliderui_ondraw(SliderUi* self, psy_ui_Graphics* g)
{	
	double yoffset;
	double value;		
	psy_ui_Style* slider_style;
	psy_ui_Style* sliderknob_style;
	psy_ui_Background* bg;
	const psy_ui_TextMetric* tm;
	
	slider_style = psy_ui_style(STYLE_MACPARAM_SLIDER);
	sliderknob_style = psy_ui_style(STYLE_MACPARAM_SLIDERKNOB);	
	sliderui_updateparam(self);
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
		(slider_style->background.size.height -
			sliderknob_style->background.size.height));		
	tm = psy_ui_component_textmetric(&self->component);
	bg = &sliderknob_style->background;
	psy_ui_drawbitmap(g, &bg->bitmap,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_value_px(&sliderknob_style->padding.left, tm, 0),
				yoffset + psy_ui_value_px(
					&sliderknob_style->padding.top, tm, 0)),
			bg->size),
		psy_ui_realpoint_make(-bg->position.x, -bg->position.y));	
}

void sliderui_onpreferredsize(SliderUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	psy_ui_Style* style;

	style = psy_ui_style(STYLE_MACPARAM_SLIDER);
	psy_ui_size_setreal(rv, style->background.size);
}

void sliderui_onmousedown(SliderUi* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 1) {
		paramtweak_begin(&self->paramtweak, self->machine, self->paramidx,
			self->param);		
		paramtweak_onmousedown(&self->paramtweak, ev);
		psy_ui_component_capture(&self->component);		
	}
}

void sliderui_onmousemove(SliderUi* self, psy_ui_MouseEvent* ev)
{
	if ((paramtweak_active(&self->paramtweak))) {		
		paramtweak_onmousemove(&self->paramtweak, ev);
		psy_ui_component_invalidate(&self->component);
	}
}

void sliderui_onmouseup(SliderUi* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);
	if ((paramtweak_active(&self->paramtweak))) {
		paramtweak_end(&self->paramtweak);		
		psy_ui_component_invalidate(&self->component);
	}
}

void sliderui_updateparam(SliderUi* self)
{
	if (self->machine && self->paramidx != psy_INDEX_INVALID) {
		self->param = psy_audio_machine_parameter(self->machine,
			self->paramidx);
	}
}

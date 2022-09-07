/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "valueui.h"
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

/* ValueUi */

/* prototypes */
static void valueui_onpreferredsize(ValueUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void valueui_update_param(ValueUi*);
static void valueui_on_mouse_down(ValueUi*, psy_ui_MouseEvent*);
static void valueui_on_mouse_up(ValueUi*, psy_ui_MouseEvent*);
static void valueui_onmousemove(ValueUi*, psy_ui_MouseEvent*);
static void valueui_on_timer(ValueUi*, uintptr_t timerid);

/* vtable */
static psy_ui_ComponentVtable valueui_vtable;
static bool valueui_vtable_initialized = FALSE;

static void valueui_vtable_init(ValueUi* self)
{
	assert(self);

	if (!valueui_vtable_initialized) {
		valueui_vtable = *(self->label.component.vtable);				
		valueui_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			valueui_onpreferredsize;
		valueui_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			valueui_on_mouse_down;
		valueui_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			valueui_on_mouse_up;
		valueui_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			valueui_onmousemove;
		valueui_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			valueui_on_timer;		
		valueui_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->label.component, &valueui_vtable);
}

/* implementation */
void valueui_init(ValueUi* self, psy_ui_Component* parent,	
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	assert(self);	

	psy_ui_label_init(&self->label, parent);	
	psy_ui_label_prevent_translation(&self->label);
	psy_ui_label_prevent_wrap(&self->label);	
	psy_ui_label_set_text_alignment(&self->label, psy_ui_ALIGNMENT_CENTER);
	valueui_vtable_init(self);	
	self->param = param;
	self->paramidx = paramidx;
	self->machine = machine;
	self->last = -1.f;
	paramtweak_init(&self->paramtweak);
	psy_ui_component_start_timer(&self->label.component, 0, 50);	
}

ValueUi* valueui_alloc(void)
{
	return (ValueUi*)malloc(sizeof(ValueUi));
}

ValueUi* valueui_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	ValueUi* rv;

	rv = valueui_alloc();
	if (rv) {
		valueui_init(rv, parent, machine, paramidx, param);
		rv->label.component.deallocate = TRUE;
	}
	return rv;
}

void valueui_onpreferredsize(ValueUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	psy_ui_size_setem(rv, 15.0, 1.0);
}

void valueui_on_mouse_down(ValueUi* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 1) {
		paramtweak_begin(&self->paramtweak, self->machine, self->paramidx,
			self->param);		
		paramtweak_on_mouse_down(&self->paramtweak, ev);		
		psy_ui_component_capture(&self->label.component);
	}
}

void valueui_onmousemove(ValueUi* self, psy_ui_MouseEvent* ev)
{		
	if (paramtweak_active(&self->paramtweak)) {
		paramtweak_onmousemove(&self->paramtweak, ev);
		psy_ui_component_setcursor(&self->label.component,
			psy_ui_CURSORSTYLE_ROW_RESIZE);
		psy_ui_component_invalidate(&self->label.component);
	}
}

void valueui_on_mouse_up(ValueUi* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_release_capture(&self->label.component);
	if (paramtweak_active(&self->paramtweak)) {
		paramtweak_end(&self->paramtweak);		
		psy_ui_component_invalidate(&self->label.component);
	}
}

void valueui_update_param(ValueUi* self)
{
	char str[128];
	float value;
		
	if (self->machine && self->paramidx != psy_INDEX_INVALID) {
		self->param = psy_audio_machine_parameter(self->machine,
			self->paramidx);							
	}
	if (!self->param) {
		return;
	}		
	if (self->machine) {			
		value = psy_audio_machine_parameter_normvalue(self->machine,
			self->param);
	} else {
		value = psy_audio_machineparam_normvalue(self->param);
	}
	if (value == self->last) {
		return;
	}
	self->last = value;	
	if (self->machine) {			
		if (!psy_audio_machine_parameter_describe(self->machine, self->param,
				str)) {
			psy_snprintf(str, 128, "%d",
				(int)psy_audio_machineparam_scaledvalue(self->param));
		}
	} else {
		if (!psy_audio_machineparam_describe(self->param, str)) {
			psy_snprintf(str, 128, "%d",
				(int)psy_audio_machineparam_scaledvalue(self->param));
		}
	}
#ifndef PSYCLE_DEBUG_PREVENT_TIMER_DRAW		
	psy_ui_label_set_text(&self->label, str);
#endif
}

void valueui_on_timer(ValueUi* self, uintptr_t timerid)
{
	valueui_update_param(self);
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "effectui.h"
/* host */
#include "styles.h"
#include "paramview.h"
/* ui*/
#include <uiapp.h>

/* prototypes */
static void effectui_on_mouse_down(EffectUi*, psy_ui_MouseEvent*);
static void effectui_onmousedoubleclick(EffectUi*, psy_ui_MouseEvent*);
static void effectui_move(EffectUi*, psy_ui_Point topleft);
static void effectui_updatevolumedisplay(EffectUi*);
static void effectui_on_timer(EffectUi*, uintptr_t timerid);
	
/* vtable */
static psy_ui_ComponentVtable effectui_vtable;
static psy_ui_ComponentVtable effectui_super_vtable;
static bool effectui_vtable_initialized = FALSE;

static void effectui_vtable_init(EffectUi* self)
{
	assert(self);

	if (!effectui_vtable_initialized) {
		effectui_vtable = *(self->component.vtable);
		effectui_super_vtable = effectui_vtable;
		effectui_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			effectui_on_mouse_down;
		effectui_vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			effectui_onmousedoubleclick;
		effectui_vtable.move =
			(psy_ui_fp_component_move)
			effectui_move;		
		effectui_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			effectui_on_timer;
		effectui_vtable_initialized = TRUE;	
	}
	psy_ui_component_set_vtable(&self->component, &effectui_vtable);	
}

/* implementation */
void effectui_init(EffectUi* self, psy_ui_Component* parent,
	uintptr_t slot, ParamViews* paramviews, Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(workspace->song);	

	psy_ui_component_init(&self->component, parent, NULL);	
	effectui_vtable_init(self);	
	psy_ui_component_set_style_type(&self->component, STYLE_MV_EFFECT);
	self->paramviews = paramviews;
	self->machines = &workspace->song->machines;
	assert(self->machines);
	self->machine = psy_audio_machines_at(self->machines, slot);
	assert(self->machine);
	self->preventmachinepos = FALSE;
	psy_ui_component_init(&self->mute, &self->component, NULL);
	psy_ui_component_set_style_type(&self->mute, STYLE_MV_EFFECT_MUTE);
	psy_ui_component_set_style_type_select(&self->mute, STYLE_MV_EFFECT_MUTE_SELECT);
	psy_ui_component_init(&self->bypass, &self->component, NULL);
	psy_ui_component_set_style_type(&self->bypass, STYLE_MV_EFFECT_BYPASS);
	psy_ui_component_set_style_type_select(&self->bypass, STYLE_MV_EFFECT_BYPASS_SELECT);
	editnameui_init(&self->editname, &self->component, self->machine,
		STYLE_MV_EFFECT_NAME);
	panui_init(&self->pan, &self->component, self->machine,
		STYLE_MV_EFFECT_PAN, STYLE_MV_EFFECT_PAN_SLIDER);
	vuui_init(&self->vu, &self->component, self->machine,
		STYLE_MV_EFFECT_VU, STYLE_MV_EFFECT_VU0, STYLE_MV_EFFECT_VUPEAK);
	psy_ui_component_start_timer(&self->component, 0, 50);
}

void effectui_move(EffectUi* self, psy_ui_Point topleft)
{
	assert(self);

	effectui_super_vtable.move(&self->component, topleft);	
	if (!self->preventmachinepos) {		
		psy_audio_machine_setposition(self->machine,
			psy_ui_value_px(&topleft.x, NULL, NULL),
			psy_ui_value_px(&topleft.y, NULL, NULL));
	}
}

void effectui_on_mouse_down(EffectUi* self, psy_ui_MouseEvent* ev)
{	
	if (psy_ui_mouseevent_button(ev) == 1) {
		if (psy_audio_machine_slot(self->machine) !=
				psy_audio_machines_selected(self->machines)) {
			if (psy_ui_mouseevent_ctrlkey(ev)) {
				psy_audio_machineselection_select(&self->machines->selection,
					psy_audio_machineindex_make(psy_audio_machine_slot(
						self->machine)));
			} else {
				psy_audio_machines_select(self->machines,
					psy_audio_machine_slot(self->machine));
			}
		}
		if (psy_ui_mouseevent_target(ev) == &self->bypass) {
			if (psy_audio_machine_bypassed(self->machine)) {
				psy_audio_machine_unbypass(self->machine);
			} else {
				psy_audio_machine_bypass(self->machine);
			}
			psy_ui_mouseevent_stop_propagation(ev);
		} else if (psy_ui_mouseevent_target(ev) == &self->mute) {
			if (psy_audio_machine_muted(self->machine)) {
				psy_audio_machine_unmute(self->machine);
			} else {
				psy_audio_machine_mute(self->machine);
			}
			psy_ui_mouseevent_stop_propagation(ev);
		}		
	}
}

void effectui_onmousedoubleclick(EffectUi* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 1 &&
			ev->event.target_ != &self->bypass &&
			ev->event.target_ != &self->mute &&
			self->paramviews) {
		paramviews_show(self->paramviews, psy_audio_machine_slot(self->machine));
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void effectui_on_timer(EffectUi* self, uintptr_t timerid)
{
	if (psy_audio_machine_muted(self->machine)) {
		psy_ui_component_add_style_state(&self->mute,
			psy_ui_STYLESTATE_SELECT);
	} else {
		psy_ui_component_remove_style_state(&self->mute,
			psy_ui_STYLESTATE_SELECT);
	}
	if (psy_audio_machine_bypassed(self->machine)) {
		psy_ui_component_add_style_state(&self->bypass,
			psy_ui_STYLESTATE_SELECT);
	} else {
		psy_ui_component_remove_style_state(&self->bypass,
			psy_ui_STYLESTATE_SELECT);
	}
	if (psy_ui_component_draw_visible(&self->component)) {
		if (vuui_update(&self->vu)) {
			psy_ui_component_invalidate(vuui_base(&self->vu));
		}
	}
}

/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "generatorui.h"
/* host */
#include "paramview.h"
#include "styles.h"
/* ui */
#include <uiapp.h>

/* GeneratorUi */
static void generatorui_onmousedown(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_onmousedoubleclick(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_move(GeneratorUi*, psy_ui_Point topleft);
static void generatorui_onpreferredsize(GeneratorUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void generatorui_ontimer(GeneratorUi*, uintptr_t timerid);
/* vtable */
static psy_ui_ComponentVtable generatorui_vtable;
static psy_ui_ComponentVtable generatorui_super_vtable;
static bool generatorui_vtable_initialized = FALSE;

static void generatorui_vtable_init(GeneratorUi* self)
{
	assert(self);

	if (!generatorui_vtable_initialized) {
		generatorui_vtable = *(self->component.vtable);
		generatorui_super_vtable = generatorui_vtable;		
		generatorui_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			generatorui_onmousedown;
		generatorui_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			generatorui_onmousedoubleclick;
		generatorui_vtable.move =
			(psy_ui_fp_component_move)
			generatorui_move;		
		generatorui_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			generatorui_ontimer;
		generatorui_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			generatorui_onpreferredsize;
		generatorui_vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(&self->component, &generatorui_vtable);
}

/* implementation */
void generatorui_init(GeneratorUi* self, psy_ui_Component* parent,
	uintptr_t slot, ParamViews* paramviews, Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(workspace->song);

	psy_ui_component_init(&self->component, parent, NULL);
	generatorui_vtable_init(self);	
	self->paramviews = paramviews;
	self->machines = &workspace->song->machines;
	assert(self->machines);
	self->machine = psy_audio_machines_at(self->machines, slot);
	assert(self->machine);
	self->preventmachinepos = FALSE;
	psy_ui_component_setstyletype(&self->component, STYLE_MV_GENERATOR);
	psy_ui_component_init(&self->mute, &self->component, NULL);
	psy_ui_component_setstyletype(&self->mute, STYLE_MV_GENERATOR_MUTE);
	psy_ui_component_setstyletype_select(&self->mute,
		STYLE_MV_GENERATOR_MUTE_SELECT);
	psy_ui_component_init(&self->solo, &self->component, NULL);	
	psy_ui_component_setstyletype(&self->solo, STYLE_MV_GENERATOR_SOLO);
	psy_ui_component_setstyletype_select(&self->solo,
		STYLE_MV_GENERATOR_SOLO_SELECT);
	editnameui_init(&self->editname, &self->component, self->machine,
		STYLE_MV_GENERATOR_NAME);
	panui_init(&self->pan, &self->component, self->machine,
		STYLE_MV_GENERATOR_PAN, STYLE_MV_GENERATOR_PAN_SLIDER);
	vuui_init(&self->vu, &self->component, self->machine,
		STYLE_MV_GENERATOR_VU, STYLE_MV_GENERATOR_VU0, STYLE_MV_GENERATOR_VUPEAK);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void generatorui_move(GeneratorUi* self, psy_ui_Point topleft)
{
	assert(self);

	generatorui_super_vtable.move(&self->component, topleft);
	if (!self->preventmachinepos) {
		psy_audio_machine_setposition(self->machine,
			psy_ui_value_px(&topleft.x, psy_ui_component_textmetric(&self->component), NULL),
			psy_ui_value_px(&topleft.y, psy_ui_component_textmetric(&self->component), NULL));
	}
}

void generatorui_onmousedown(GeneratorUi* self, psy_ui_MouseEvent* ev)
{	
	if (psy_ui_mouseevent_button(ev) == 1) {		
		if (psy_audio_machine_slot(self->machine) != psy_audio_machines_selected(self->machines)) {			
			if (psy_ui_mouseevent_ctrlkey(ev)) {
				psy_audio_machineselection_select(&self->machines->selection,
					psy_audio_machineindex_make(psy_audio_machine_slot(self->machine)));
			} else {
				psy_audio_machines_select(self->machines,
					psy_audio_machine_slot(self->machine));
			}
		}
		if (ev->event.target_ == &self->mute) {
			if (psy_audio_machine_muted(self->machine)) {
				psy_audio_machine_unmute(self->machine);
			} else {
				psy_audio_machine_mute(self->machine);
			}
			psy_ui_mouseevent_stop_propagation(ev);
		} else if (ev->event.target_ == &self->solo) {
			psy_audio_machines_solo(self->machines,
				psy_audio_machine_slot(self->machine));
			psy_ui_mouseevent_stop_propagation(ev);
		}		
	}
}

void generatorui_onmousedoubleclick(GeneratorUi* self, psy_ui_MouseEvent* ev)
{	
	if (psy_ui_mouseevent_button(ev) == 1 &&
			ev->event.target_ != &self->solo &&
			ev->event.target_ != &self->mute &&
			self->paramviews) {
		paramviews_show(self->paramviews,
			psy_audio_machine_slot(self->machine));
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void generatorui_onpreferredsize(GeneratorUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{			
	psy_ui_Style* style;

	style = psy_ui_style(STYLE_MV_GENERATOR);
	*rv = (style)
		? psy_ui_size_makereal(style->background.size)
		: psy_ui_size_make_px(138.0, 52.0);
}

void generatorui_ontimer(GeneratorUi* self, uintptr_t timerid)
{
	if (psy_audio_machine_muted(self->machine)) {
		psy_ui_component_addstylestate(&self->mute, psy_ui_STYLESTATE_SELECT);
	} else {
		psy_ui_component_removestylestate(&self->mute,
			psy_ui_STYLESTATE_SELECT);
	}
	if (psy_audio_machines_soloed(self->machines) ==
			psy_audio_machine_slot(self->machine)) {
		psy_ui_component_addstylestate(&self->solo, psy_ui_STYLESTATE_SELECT);
	} else {
		psy_ui_component_removestylestate(&self->solo,
			psy_ui_STYLESTATE_SELECT);
	}
}

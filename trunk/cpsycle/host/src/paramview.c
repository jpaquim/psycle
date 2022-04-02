/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "paramview.h"
/* host */
#include "headerui.h"
#include "labelui.h"
#include "knobui.h"
#include "slidergroupui.h"
#include "styles.h"
#include "switchui.h"
#include "machineui.h"
/* audio */
#include <plugin_interface.h>
/* std */
#include <assert.h>
/* platform */
#include "../../detail/portable.h"

/* paramview */
/* prototypes */
static void paramview_on_destroyed(ParamView*);
static void paramview_updateskin(ParamView*);
static void paramview_on_timer(ParamView*, uintptr_t timerid);
static uintptr_t paramview_numrows(const ParamView*);
static void paramview_build(ParamView*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ParamView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			paramview_on_destroyed;
		vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			paramview_on_timer;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}

/* implementation */
void paramview_init(ParamView* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, MachineParamConfig* config,
	psy_ui_Component** frameview)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_doublebuffer(&self->component);	
	psy_ui_component_setalignexpand(&self->component, psy_ui_HEXPAND);
	self->config = config;	
	self->machine = machine;	
	self->paramstrobe = 0;
	self->sizechanged = 1;
	self->frameview = frameview;	
	paramview_updateskin(self);
	paramview_build(self);		
}

void paramview_on_destroyed(ParamView* self)
{
	if (self->frameview && *self->frameview) {
		*self->frameview = NULL;
	}	
}

ParamView* paramview_alloc(void)
{
	return (ParamView*) malloc(sizeof(ParamView));
}

ParamView* paramview_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine, MachineParamConfig* config,
	psy_ui_Component** frameview)
{
	ParamView* rv;

	rv = paramview_alloc();
	if (rv) {
		paramview_init(rv, parent, machine, config, frameview);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void paramview_updateskin(ParamView* self)
{
	self->fontinfo = machineparamconfig_fontinfo(self->config);
	psy_ui_component_setfontinfo(&self->component, self->fontinfo);	
}

void paramview_on_timer(ParamView* self, uintptr_t timerid)
{
	if (self->machine && psy_audio_machine_paramstrobe(self->machine)
			!= self->paramstrobe) {
		paramview_build(self);
		self->paramstrobe = psy_audio_machine_paramstrobe(self->machine);
	}
	psy_ui_component_invalidate(&self->component);
}

void paramview_build(ParamView* self)
{
	psy_ui_component_clear(&self->component);
	if (self->machine && psy_audio_machine_numparameters(self->machine) > 0
			&& psy_audio_machine_numparametercols(self->machine) > 0) {
		uintptr_t row = 0;
		uintptr_t col = 0;
		uintptr_t numrows = 0;
		uintptr_t paramnum;
		psy_ui_Component* currcolumn;
		SliderGroupUi* currslider;
		
		numrows = paramview_numrows(self);
		currcolumn = NULL;
		currslider = NULL;
		for (paramnum = 0; paramnum < psy_audio_machine_numparameters(self->machine);
				++paramnum) {			
			psy_audio_MachineParam* machineparam;

			if (row == 0) {
				currcolumn = psy_ui_component_allocinit(&self->component, &self->component);
				psy_ui_component_set_align(currcolumn, psy_ui_ALIGN_LEFT);
			}
			machineparam = psy_audio_machine_parameter(self->machine, paramnum);
			if (machineparam) {
				psy_ui_Component* component;
				bool slider;
				bool checkinslidergroup;

				component = NULL;
				slider = FALSE;
				checkinslidergroup = FALSE;
				switch (psy_audio_machine_parameter_type(self->machine, machineparam) & 0x1FF) {
				case MPF_HEADER: {
					HeaderUi* header;

					header = headerui_allocinit(currcolumn,						
						self->machine, paramnum, machineparam);
					if (header) {
						component = &header->component;
					}
					break; }
				case MPF_INFOLABEL: {
					LabelUi* label;

					label = labelui_allocinit(currcolumn, self->machine,
						paramnum, machineparam);
					if (label) {
						component = &label->component;
					}
					break; }
				case MPF_STATE: {
					KnobUi* knob;

					knob = knobui_allocinit(currcolumn,	self->machine,
						paramnum, machineparam);
					if (knob) {
						component = &knob->component;
					}
					break; }
				case MPF_SLIDER: {
					currslider = slidergroupui_allocinit(currcolumn,
						self->machine, paramnum, machineparam,
						psy_INDEX_INVALID, NULL);
					if (currslider) {
						component = &currslider->component;
					}
					break; }
				case MPF_LEVEL: {
					if (currslider) { // && machineparam->isslidergroup)
						currslider->level.param = machineparam;
					}
					break; }
				case MPF_CHECK: {
					SwitchUi* switchui;

					if (currslider && machineparam->isslidergroup) {
						switchui = switchui_allocinit(&currslider->controls,
							self->machine, paramnum, machineparam,
							STYLE_MACPARAM_CHECKOFF,
							STYLE_MACPARAM_CHECKON);
						checkinslidergroup = TRUE;
					} else {
						switchui = switchui_allocinit(currcolumn,
							self->machine, paramnum, machineparam,
							STYLE_MACPARAM_CHECKON,
							STYLE_MACPARAM_CHECKOFF);
					}
					if (switchui) {
						component = &switchui->component;
					}
					break; }				
				case MPF_SWITCH: {
					SwitchUi* switchui;

					if (currslider && machineparam->isslidergroup) {
						switchui = switchui_allocinit(&currslider->controls,
							self->machine, paramnum, machineparam,
							STYLE_MACPARAM_SWITCHOFF,
							STYLE_MACPARAM_SWITCHON);						
						checkinslidergroup = TRUE;
					} else {
						switchui = switchui_allocinit(currcolumn,
							self->machine, paramnum, machineparam,
							STYLE_MACPARAM_SWITCHOFF,
							STYLE_MACPARAM_SWITCHON);
					}
					if (switchui) {
						switchui->maxheight = 2.0;
						component = &switchui->component;
					}
					break; }
				case MPF_IGNORE:					
					break;
				default: {
					component = psy_ui_component_allocinit(currcolumn, NULL);
					psy_ui_component_set_preferred_size(component,
						psy_ui_size_make_em(PARAMWIDTH_SMALL, 2.0));
					psy_ui_component_preventalign(component);
					break; }
				}
				if (component) {
					if ((psy_audio_machine_parameter_type(self->machine, machineparam) &
							MPF_BOTTOM) == MPF_BOTTOM) {						
						psy_ui_component_set_align(component, psy_ui_ALIGN_BOTTOM);						
					} else {
						psy_ui_component_set_align(component, psy_ui_ALIGN_TOP);
					}
					if (slider) {
						psy_ui_component_setminimumsize(component,
							psy_ui_size_make_em(20.0, 15.0));
					}
				}
			}
			++row;
			if (row >= numrows) {
				row = 0;
				++col;
			}
		}
	}
	psy_ui_component_align(&self->component);
	self->sizechanged = 1;
	psy_signal_emit(&self->component.signal_preferredsizechanged, self, 0);
}

uintptr_t paramview_numrows(const ParamView* self)
{
	if (self->machine &&
			psy_audio_machine_numparametercols(self->machine) > 0) {
		return (psy_audio_machine_numparameters(self->machine) - 1) /
			psy_audio_machine_numparametercols(self->machine) + 1;
	}
	return 0;
}

void paramview_setzoom(ParamView* self, double zoomrate)
{
	psy_ui_Font font;	
	psy_ui_FontInfo fontinfo;

	fontinfo = self->fontinfo;
	fontinfo.lfHeight = (int32_t)(self->fontinfo.lfHeight * zoomrate);
	psy_ui_font_init(&font, &fontinfo);
	psy_ui_component_setfont(&self->component, &font);
	psy_ui_font_dispose(&font);
	self->sizechanged = 1;
}

/*void onmousewheel(ParamView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_audio_MachineParam* param;

	self->tweak = hittest(self, ev->pt);
	param = tweakparam(self);
	if (param != NULL) {
		self->tweakval = psy_audio_machine_parameter_normvalue(self->machine, param);
		if (ev->delta > 0) {
			float val;

			val = self->tweakval + 1 / 200.f;
			if (val > 1.f) {
				val = 1.f;
			}
			else
				if (val < 0.f) {
					val = 0.f;
				}
			psy_audio_machine_parameter_tweak(self->machine, param, val);
			psy_ui_component_invalidate(&self->component);
		} else
		if (ev->delta < 0) {
			float val;

			val = self->tweakval - 1 / 200.f;
			if (val > 1.f) {
				val = 1.f;
			} else
			if (val < 0.f) {
				val = 0.f;
			}
			psy_audio_machine_parameter_tweak(self->machine, param, val);
			psy_ui_component_invalidate(&self->component);
		}
	}
	ev->preventdefault = 1;
	self->tweak = psy_INDEX_INVALID;
}*/

/*
void onmousedoubleclick(ParamView* self, psy_ui_MouseEvent* ev)
{
	if (self->machine) {
		psy_audio_MachineParam* tweakpar;
		uintptr_t paramindex;

		paramindex = hittest(self, ev->pt);
		if (paramindex != psy_INDEX_INVALID) {
			tweakpar = psy_audio_machine_parameter(self->machine, paramindex);
			if (tweakpar) {
				// psy_audio_machine_parameter_reset(self->machine, tweakpar);
			}
		}
	}
}
*/

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

// host
#include "headerui.h"
#include "labelui.h"
#include "knobui.h"
#include "slidergroupui.h"
#include "switchui.h"
#include "checkui.h"
#include "paramview.h"
#include "resources/resource.h"
#include "skingraphics.h"
#include "machineui.h"
// std
#include <assert.h>
// platform
#include "../../detail/portable.h"

// ParamView
// Prototypes
static void ontimer(ParamView*, uintptr_t timerid);
static uintptr_t paramview_numrows(const ParamView*);
static void paramview_build(ParamView*);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ParamView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ontimer = (psy_ui_fp_component_ontimer)
			ontimer;
		vtable_initialized = TRUE;
	}
}
// implementation
void paramview_init(ParamView* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, Workspace* workspace)
{
	psy_Property* pv;

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);	
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	self->workspace = workspace;
	self->machine = machine;
	self->numparams = 0;
	self->paramstrobe = 0;
	pv = psy_property_findsection(&self->workspace->config.config, "visual.paramview");
	if (pv) {
		psy_ui_Font font;

		psy_ui_fontinfo_init_string(&self->fontinfo,
			psy_property_at_str(pv, "font", "tahoma;-16"));
		psy_ui_font_init(&font, &self->fontinfo);
		psy_ui_component_setfont(&self->component, &font);
		psy_ui_font_dispose(&font);
	} else {
		psy_ui_fontinfo_init_string(&self->fontinfo, "tahoma;-16");
	}	
	self->skin = machineparamconfig_skin(
		psycleconfig_macparam(workspace_conf(workspace)));
	psy_ui_component_setbackgroundcolour(&self->component,
		self->skin->bottomcolour);
	self->sizechanged = 1;	
	if (self->machine) {
		paramview_build(self);
	}
	psy_ui_component_starttimer(&self->component, 0, 50);	
}

ParamView* paramview_alloc(void)
{
	return (ParamView*) malloc(sizeof(ParamView));
}

ParamView* paramview_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine, Workspace* workspace)
{
	ParamView* rv;

	rv = paramview_alloc();
	if (rv) {
		paramview_init(rv, parent, machine, workspace);
	}
	return rv;
}

void ontimer(ParamView* self, uintptr_t timerid)
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
				psy_ui_component_setalign(currcolumn, psy_ui_ALIGN_LEFT);
			}
			machineparam = psy_audio_machine_parameter(self->machine, paramnum);
			if (machineparam) {
				psy_ui_Component* component;
				bool slider;
				bool checkinslidergroup;

				component = NULL;
				slider = FALSE;
				checkinslidergroup = FALSE;
				switch (psy_audio_machine_parameter_type(self->machine, machineparam) &
					~MPF_SMALL) {
				case MPF_NULL: {
					component = psy_ui_component_allocinit(currcolumn, &self->component);
					break; }
				case MPF_HEADER: {
					HeaderUi* header;

					header = headerui_allocinit(currcolumn,
						&self->component, machineparam, self->skin);
					if (header) {
						component = &header->component;
					}
					break;
				}
				case MPF_INFOLABEL: {
					LabelUi* label;

					label = labelui_allocinit(currcolumn,
						&self->component,
						self->machine, paramnum,
						machineparam, self->skin);
					if (label) {
						component = &label->component;
					}
					break; }
				case MPF_STATE: {
					KnobUi* knob;

					knob = knobui_allocinit(currcolumn,
						&self->component,
						self->machine, paramnum, machineparam,
						self->skin);
					if (knob) {
						component = &knob->component;
					}
					break; }
				case MPF_SLIDER: {
					currslider = slidergroupui_allocinit(currcolumn,
						&self->component, self->machine, paramnum, machineparam,
						psy_INDEX_INVALID, NULL, self->skin);
					if (currslider) {
						component = &currslider->component;
					}
					break; }
				case MPF_SLIDERLEVEL: {
					if (currslider) { // && machineparam->isslidergroup)
						currslider->level.param = machineparam;
					}
					break; }
				case MPF_SLIDERCHECK: {
					CheckUi* check;

					if (currslider && machineparam->isslidergroup) {
						check = checkui_allocinit(&currslider->controls,
							&self->component, machineparam, self->skin);
						checkinslidergroup = TRUE;
					} else {
						check = checkui_allocinit(currcolumn,
							&self->component, machineparam, self->skin);
					}
					if (check) {
						component = &check->component;
					}
					break; }				
				case MPF_SWITCH: {
					SwitchUi* switchui;

					if (currslider && machineparam->isslidergroup) {
						switchui = switchui_allocinit(&currslider->controls,
							&self->component, machineparam, self->skin);
						checkinslidergroup = TRUE;
					} else {
						switchui = switchui_allocinit(currcolumn,
							&self->component, machineparam, self->skin);
					}
					if (switchui) {
						component = &switchui->component;
					}
					break; }
				default:
					break;
				}				
				if (component) {					
					psy_ui_component_setalign(component, psy_ui_ALIGN_TOP);					
					if (slider) {
						psy_ui_component_setminimumsize(component,
							psy_ui_size_makeem(20.0, 15.0));
					} else {
						if (checkinslidergroup) {
							psy_ui_component_setminimumsize(component,
								psy_ui_size_makeem(20.0, 1.0));
						} else {
							psy_ui_component_setminimumsize(component,
								psy_ui_size_makeem(20.0, 2.0));
						}
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

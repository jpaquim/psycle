// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

// host
#include "paramtweak.h"
// audio
#include <plugin_interface.h>
// std
#include <assert.h>
// platform
#include "../../detail/portable.h"

// ParamTweak
static psy_audio_MachineParam* paramtweak_tweakparam(ParamTweak* self);

void paramtweak_init(ParamTweak* self)
{
	assert(self);

	self->machine = NULL;
	self->paramindex = psy_INDEX_INVALID;
	self->param = NULL;
}

void paramtweak_begin(ParamTweak* self, psy_audio_Machine* machine,
	uintptr_t paramindex)
{
	assert(self);

	self->machine = machine;
	self->paramindex = paramindex;
	self->param = NULL;
}

void paramtweak_end(ParamTweak* self)
{
	assert(self);

	self->machine = NULL;
	self->paramindex = psy_INDEX_INVALID;
	self->param = NULL;
}

void paramtweak_onmousedown(ParamTweak* self, psy_ui_MouseEvent* ev)
{	
	psy_audio_MachineParam* param;

	assert(self);

	param = paramtweak_tweakparam(self);
	if (ev->button == 1 && param) {
		uintptr_t paramtype;

		self->tweakbase = (float)ev->pt.y;
		if (self->machine) {
			self->tweakval = psy_audio_machine_parameter_normvalue(self->machine, param);
			paramtype = psy_audio_machine_parameter_type(self->machine, param) & ~MPF_SMALL;
		} else {
			self->tweakval = psy_audio_machineparam_normvalue(param);
			paramtype = psy_audio_machineparam_type(param) & ~MPF_SMALL;
		}		
		
		if (paramtype == MPF_SLIDERCHECK || paramtype == MPF_SWITCH) {
			if (self->tweakval == 0.f) {
				if (self->machine) {
					psy_audio_machine_parameter_tweak(self->machine, param, 1.f);
				} else {
					psy_audio_machineparam_tweak(param, 1.f);
				}
			} else {
				if (self->machine) {
					psy_audio_machine_parameter_tweak(self->machine, param, 0.f);
				} else {
					psy_audio_machineparam_tweak(param, 0.f);
				}
			}
		}		
	}
}

void paramtweak_onmousemove(ParamTweak* self, psy_ui_MouseEvent* ev)
{	
	psy_audio_MachineParam* param;	

	assert(self);

	param = paramtweak_tweakparam(self);	
	if (param) {
		uintptr_t paramtype;
		float val;		

		if (self->machine) {
			paramtype = psy_audio_machine_parameter_type(self->machine, param) & ~MPF_SMALL;
		} else {
			paramtype = psy_audio_machineparam_type(param) & ~MPF_SMALL;
		}
		if ((paramtype != MPF_SLIDERCHECK) && (paramtype != MPF_SWITCH)) {
			val = self->tweakval + (self->tweakbase - (float)ev->pt.y) / 200.f;
			if (val > 1.f) {
				val = 1.f;
			} else if (val < 0.f) {
				val = 0.f;
			}
			if (self->machine) {
				psy_audio_machine_parameter_tweak(self->machine, param, val);
			} else {
				psy_audio_machineparam_tweak(param, val);
			}
		}
	}
}

psy_audio_MachineParam* paramtweak_tweakparam(ParamTweak* self)
{
	assert(self);

	if (self->param) {
		return self->param;
	}
	if (self->machine && self->paramindex != psy_INDEX_INVALID) {
		return psy_audio_machine_parameter(self->machine, self->paramindex);
	}
	return NULL;
}

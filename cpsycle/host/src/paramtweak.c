// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

// host
#include "paramtweak.h"
// audio
#include <machine.h>
#include <plugin_interface.h>
// ui
#include <uievents.h>
// std
#include <assert.h>
// platform
#include "../../detail/portable.h"

// prototypes
static void paramtweak_updateparam(ParamTweak*);
// implementation
void paramtweak_init(ParamTweak* self)
{
	assert(self);

	self->machine = NULL;
	self->paramidx = psy_INDEX_INVALID;
	self->param = NULL;
	self->machine = NULL;
	self->paramidx = psy_INDEX_INVALID;
	self->active = FALSE;
}

void paramtweak_begin(ParamTweak* self, psy_audio_Machine* machine,
	uintptr_t paramindex, psy_audio_MachineParam* param)
{
	assert(self);

	self->machine = machine;
	self->paramidx = paramindex;
	self->param = param;
	self->active = TRUE;
	paramtweak_updateparam(self);
}

void paramtweak_end(ParamTweak* self)
{
	assert(self);

	self->machine = NULL;
	self->paramidx = psy_INDEX_INVALID;
	self->param = NULL;
	self->active = FALSE;
}

void paramtweak_onmousedown(ParamTweak* self, psy_ui_MouseEvent* ev)
{		
	assert(self);

	if (ev->button == 1) {
		uintptr_t paramtype;
		paramtweak_updateparam(self);
		
		if (self->param) {
			self->tweakbase = (float)ev->pt.y;
			if (self->machine) {
				self->tweakval = psy_audio_machine_parameter_normvalue(
					self->machine, self->param);
				paramtype = psy_audio_machine_parameter_type(self->machine,
					self->param) & 0x1FF;
			} else {
				self->tweakval = psy_audio_machineparam_normvalue(self->param);
				paramtype = psy_audio_machineparam_type(self->param)
					& ~MPF_SMALL;
			}
			if (paramtype == MPF_CHECK || paramtype == MPF_SWITCH) {
				if (self->tweakval == 0.f) {
					if (self->machine) {
						psy_audio_machine_parameter_tweak(self->machine,
							self->param, 1.f);
					} else {
						psy_audio_machineparam_tweak(self->param, 1.f);
					}
				} else {
					if (self->machine) {
						psy_audio_machine_parameter_tweak(self->machine,
							self->param, 0.f);
					} else {
						psy_audio_machineparam_tweak(self->param, 0.f);
					}
				}
			}
		}
	}
}

void paramtweak_onmousemove(ParamTweak* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (!self->active) {
		return;
	}
	paramtweak_updateparam(self);
	if (self->param) {
		uintptr_t paramtype;
		float val;		

		if (self->machine) {
			paramtype = psy_audio_machine_parameter_type(self->machine,
				self->param) & ~MPF_SMALL;
		} else {
			paramtype = psy_audio_machineparam_type(self->param) & 0x1FF;
		}
		if ((paramtype != MPF_CHECK) && (paramtype != MPF_SWITCH)) {
			// todo add fine/ultrafine tweak
			val = self->tweakval + (self->tweakbase - (float)ev->pt.y) / 200.f;
			if (val > 1.f) {
				val = 1.f;
			} else if (val < 0.f) {
				val = 0.f;
			}
			if (self->machine) {
				psy_audio_machine_parameter_tweak(self->machine, self->param,
					val);
			} else {
				psy_audio_machineparam_tweak(self->param, val);
			}
		}
		psy_ui_mouseevent_stop_propagation(ev);
	}	
}

void paramtweak_updateparam(ParamTweak* self)
{
	if (self->machine && self->paramidx != psy_INDEX_INVALID) {
		self->param = psy_audio_machine_parameter(self->machine,
			self->paramidx);
	}
}

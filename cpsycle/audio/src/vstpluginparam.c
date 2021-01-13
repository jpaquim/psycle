// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vstpluginparam.h"
// local
#include "aeffectx.h"
#include "plugin_interface.h"
// std
#include <stdlib.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// Parameter
static int vstpluginparam_name(psy_audio_VstPluginParam*, char* text);
static int vstpluginparam_label(psy_audio_VstPluginParam*, char* text);
static void vstpluginparam_tweak(psy_audio_VstPluginParam*, float val);
static int vstpluginparam_describe(psy_audio_VstPluginParam*, char* text);
static float vstpluginparam_normvalue(psy_audio_VstPluginParam*);
static void vstpluginparam_range(psy_audio_VstPluginParam*,
	intptr_t* minval, intptr_t* maxval);

static MachineParamVtable vstpluginparam_vtable;
static int vstpluginparam_vtable_initialized = 0;

static void vstpluginparam_vtable_init(psy_audio_VstPluginParam* self)
{
	if (!vstpluginparam_vtable_initialized) {
		vstpluginparam_vtable = *(self->custommachineparam.machineparam.vtable);
		vstpluginparam_vtable.name = (fp_machineparam_name)vstpluginparam_name;
		vstpluginparam_vtable.label = (fp_machineparam_label)
			vstpluginparam_label;
		vstpluginparam_vtable.tweak = (fp_machineparam_tweak)
			vstpluginparam_tweak;
		vstpluginparam_vtable.normvalue = (fp_machineparam_normvalue)
			vstpluginparam_normvalue;
		vstpluginparam_vtable.range = (fp_machineparam_range)
			vstpluginparam_range;
		vstpluginparam_vtable.describe = (fp_machineparam_describe)
			vstpluginparam_describe;
	}
}

void psy_audio_vstpluginparam_init(
	psy_audio_VstPluginParam* self, struct AEffect* effect,
	uintptr_t index)
{
	psy_audio_custommachineparam_init(&self->custommachineparam,
		"",
		"",
		MPF_STATE,
		0,
		0xFFFF);
	vstpluginparam_vtable_init(self);
	self->custommachineparam.machineparam.vtable = &vstpluginparam_vtable;
	self->effect = effect;
	self->index = index;
}

void psy_audio_vstpluginparam_dispose(psy_audio_VstPluginParam* self)
{
	psy_audio_custommachineparam_dispose(&self->custommachineparam);
}

psy_audio_VstPluginParam* psy_audio_vstpluginparam_alloc(void)
{
	return (psy_audio_VstPluginParam*)malloc(sizeof(psy_audio_VstPluginParam));
}

psy_audio_VstPluginParam* psy_audio_vstpluginparam_allocinit(struct AEffect* effect,
	uintptr_t index)
{
	psy_audio_VstPluginParam* rv;

	rv = psy_audio_vstpluginparam_alloc();
	if (rv) {
		psy_audio_vstpluginparam_init(rv, effect, index);
	}
	return rv;
}

int vstpluginparam_label(psy_audio_VstPluginParam* self, char* text)
{
	text[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamLabel, (VstInt32)self->index, 0, text, 0);
	return *text != '\0';
}

int vstpluginparam_name(psy_audio_VstPluginParam* self, char* text)
{
	text[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamName, (VstInt32)self->index, 0, text, 0);
	return *text != '\0';
}

void vstpluginparam_tweak(psy_audio_VstPluginParam* self, float value)
{	
	self->effect->setParameter(self->effect, (VstInt32)self->index, value);
}

int vstpluginparam_describe(psy_audio_VstPluginParam* self, char* text)
{
	text[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamDisplay, (VstInt32)self->index, 0,
		text, 0);
	return *text != '\0';
}

float vstpluginparam_normvalue(psy_audio_VstPluginParam* self)
{
	return self->effect->getParameter(self->effect, (VstInt32)self->index);
}

void vstpluginparam_range(psy_audio_VstPluginParam* self, intptr_t* minval,
	intptr_t* maxval)
{
	*minval = 0;
	*maxval = 0xFFFF;
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineparam.h"
#include "plugin_interface.h"

#include <convert.h>
#include <valuemapper.h>

#include <math.h>
#include <stdlib.h>

#include "../../detail/portable.h"

// MachineParameter

static void machineparam_tweak(psy_audio_MachineParam* self, float value)
{
	psy_signal_emit_float(&self->signal_tweak, self, value);
}

static void machineparam_reset(psy_audio_MachineParam* self)
{
}

static float machineparam_normvalue(psy_audio_MachineParam* self)
{
	float rv = 0.f;

	psy_signal_emit(&self->signal_normvalue, self, 1, (void*)(&rv));
	return rv;
}

static void machineparam_range(psy_audio_MachineParam* self, intptr_t* minval,
	intptr_t* maxval)
{
	*minval = 0;
	*maxval = 0;
}

static int machineparam_type(psy_audio_MachineParam* self)
{
	return MPF_STATE;
}

static int machineparam_label(psy_audio_MachineParam* self, char* text)
{
	text[0] = '\0';
	return 0;
}

static int machineparam_name(psy_audio_MachineParam* self, char* text)
{
	text[0] = '\0';
	return 0;
}

static int machineparam_describe(psy_audio_MachineParam* self, char* text)
{
	text[0] = '\0';
	return 0;
}

static MachineParamVtable machineparam_vtable;
static int machineparam_vtable_initialized = 0;

static void machineparam_vtable_init(psy_audio_MachineParam* self)
{
	if (!machineparam_vtable_initialized) {
		machineparam_vtable.tweak = machineparam_tweak;
		machineparam_vtable.normvalue = machineparam_normvalue;
		machineparam_vtable.range = machineparam_range;
		machineparam_vtable.name = machineparam_name;
		machineparam_vtable.label = machineparam_label;
		machineparam_vtable.type = machineparam_type;
		machineparam_vtable.describe = machineparam_describe;
		machineparam_vtable_initialized = 1;
	}
}

void psy_audio_machineparam_init(psy_audio_MachineParam* self)
{
	machineparam_vtable_init(self);
	self->vtable = &machineparam_vtable;
	psy_signal_init(&self->signal_normvalue);
	psy_signal_init(&self->signal_tweak);
	psy_signal_init(&self->signal_describe);
	psy_signal_init(&self->signal_name);
	psy_signal_init(&self->signal_label);
	self->isslidergroup = FALSE;
}

void psy_audio_machineparam_dispose(psy_audio_MachineParam* self)
{
	psy_signal_dispose(&self->signal_normvalue);
	psy_signal_dispose(&self->signal_tweak);
	psy_signal_dispose(&self->signal_describe);
	psy_signal_dispose(&self->signal_name);
	psy_signal_dispose(&self->signal_label);
}

// CustomParameter
static void customparam_tweak(psy_audio_CustomMachineParam*, float val);
static float customparam_normvalue(psy_audio_CustomMachineParam*);
static void customparam_range(psy_audio_CustomMachineParam*,
	intptr_t* minval, intptr_t* maxval);
static int customparam_type(psy_audio_CustomMachineParam*);
static int customparam_label(psy_audio_CustomMachineParam*, char* text);
static int customparam_name(psy_audio_CustomMachineParam*, char* text);
static int customparam_describe(psy_audio_CustomMachineParam*, char* text);

static MachineParamVtable customparam_vtable;
static int customparam_vtable_initialized = 0;

static void customparam_vtable_init(psy_audio_CustomMachineParam* self)
{
	if (!customparam_vtable_initialized) {
		customparam_vtable = *(self->machineparam.vtable);
		customparam_vtable.tweak = (fp_machineparam_tweak) customparam_tweak;
		customparam_vtable.normvalue = (fp_machineparam_normvalue) customparam_normvalue;
		customparam_vtable.range = (fp_machineparam_range) customparam_range;
		customparam_vtable.name = (fp_machineparam_name) customparam_name;
		customparam_vtable.label = (fp_machineparam_label) customparam_label;
		customparam_vtable.type = (fp_machineparam_type) customparam_type;
		customparam_vtable.describe = (fp_machineparam_describe) customparam_describe;
		customparam_vtable_initialized = 1;
	}
}

void psy_audio_custommachineparam_init(psy_audio_CustomMachineParam* self, 
	const char* name, const char* label, int type,
	int minval, int maxval)
{
	psy_audio_machineparam_init(&self->machineparam);
	customparam_vtable_init(self);
	self->machineparam.vtable = &customparam_vtable;	
	self->name = (name) ? strdup(name) : 0;
	self->label = (label) ? strdup(label) : 0;
	self->minval = minval;
	self->maxval = maxval;
	self->type = type;
	self->row = 0;
}

void psy_audio_custommachineparam_dispose(psy_audio_CustomMachineParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
	free(self->name);
	free(self->label);
}

psy_audio_CustomMachineParam* psy_audio_custommachineparam_alloc(void)
{
	return (psy_audio_CustomMachineParam*)malloc(sizeof(psy_audio_CustomMachineParam));
}

psy_audio_CustomMachineParam* psy_audio_custommachineparam_allocinit(
	const char* name, const char* label, int type,
	int minval, int maxval)
{
	psy_audio_CustomMachineParam* rv;

	rv = psy_audio_custommachineparam_alloc();
	if (rv) {
		psy_audio_custommachineparam_init(rv, name, label, type, minval, maxval);
	}
	return rv;
}

void customparam_tweak(psy_audio_CustomMachineParam* self, float value)
{
	psy_signal_emit_float(&self->machineparam.signal_tweak, self, value);
}

int customparam_describe(psy_audio_CustomMachineParam* self, char* text)
{
	int rv = 0;

	if (self->machineparam.signal_describe.slots != NULL) {
		psy_signal_emit(&self->machineparam.signal_describe, self, 2,
			(void*)(&rv), (void*)(text));
	}
	return rv;
}

float customparam_normvalue(psy_audio_CustomMachineParam* self)
{
	float rv = 0.f;

	psy_signal_emit(&self->machineparam.signal_normvalue, self, 1, (void*)(&rv));
	return rv;
}

void customparam_range(psy_audio_CustomMachineParam* self,
	intptr_t* minval, intptr_t* maxval)
{
	*minval = self->minval;
	*maxval = self->maxval;
}
int customparam_type(psy_audio_CustomMachineParam* self)
{ 
	return self->type;
}

int customparam_label(psy_audio_CustomMachineParam* self, char* text)
{
	if (self->label) {
		psy_snprintf(text, 128, "%s", self->label);
		psy_signal_emit(&self->machineparam.signal_label, self, 1, text);
		return 1;
	}
	return 0;
}

int customparam_name(psy_audio_CustomMachineParam* self, char* text)
{
	if (self->name) {
		psy_snprintf(text, 128, "%s", self->name);
		psy_signal_emit(&self->machineparam.signal_name, self, 1, text);
		return 1;
	}
	return 0;
}

// InfoMachineParam
static int infomachineparam_type(psy_audio_InfoMachineParam*);
static int infomachineparam_label(psy_audio_InfoMachineParam*, char* text);
static int infomachineparam_name(psy_audio_InfoMachineParam*, char* text);
static int infomachineparam_describe(psy_audio_InfoMachineParam*, char* text);

static MachineParamVtable infomachineparam_vtable;
static int infomachineparam_vtable_initialized = 0;

static void infomachineparam_vtable_init(psy_audio_InfoMachineParam* self)
{
	if (!infomachineparam_vtable_initialized) {
		infomachineparam_vtable = *(self->machineparam.vtable);
		infomachineparam_vtable.name = (fp_machineparam_name)infomachineparam_name;
		infomachineparam_vtable.label = (fp_machineparam_label)infomachineparam_label;
		infomachineparam_vtable.type = (fp_machineparam_type)infomachineparam_type;
		infomachineparam_vtable.describe = (fp_machineparam_describe)infomachineparam_describe;
		infomachineparam_vtable_initialized = 1;
	}
}

void psy_audio_infomachineparam_init(psy_audio_InfoMachineParam* self,
	const char* name, const char* label, int style)
{
	psy_audio_machineparam_init(&self->machineparam);
	infomachineparam_vtable_init(self);
	self->machineparam.vtable = &infomachineparam_vtable;
	self->name = (name) ? strdup(name) : 0;
	self->label = (label) ? strdup(label) : 0;
	self->style = style;
}

void psy_audio_infomachineparam_dispose(psy_audio_InfoMachineParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
	free(self->name);
	free(self->label);
}

psy_audio_InfoMachineParam* psy_audio_infomachineparam_alloc(void)
{
	return (psy_audio_InfoMachineParam*)malloc(sizeof(psy_audio_InfoMachineParam));
}

psy_audio_InfoMachineParam* psy_audio_infomachineparam_allocinit(
	const char* name, const char* label, int style)
{
	psy_audio_InfoMachineParam* rv;

	rv = psy_audio_infomachineparam_alloc();
	if (rv) {
		psy_audio_infomachineparam_init(rv, name, label, style);
	}
	return rv;
}

int infomachineparam_describe(psy_audio_InfoMachineParam* self, char* text)
{	
	if (self->label) {
		psy_snprintf(text, 128, "%s", self->label);
		// psy_signal_emit(&self->machineparam.signal_describe, self, 1, text);
		return 1;
	}
	return 0;
}

int infomachineparam_label(psy_audio_InfoMachineParam* self, char* text)
{
	if (self->label) {
		psy_snprintf(text, 128, "%s", self->label);
		psy_signal_emit(&self->machineparam.signal_label, self, 1, text);
		return 1;
	}
	return 0;
}

int infomachineparam_name(psy_audio_InfoMachineParam* self, char* text)
{
	if (self->name) {
		psy_snprintf(text, 128, "%s", self->name);
		psy_signal_emit(&self->machineparam.signal_name, self, 1, text);
		return 1;
	}
	return 0;
}

int infomachineparam_type(psy_audio_InfoMachineParam* self)
{
	return MPF_INFOLABEL | self->style;
}

// IntMachineParam
static void intmachineparam_tweak(psy_audio_IntMachineParam*, float val);
static float intmachineparam_normvalue(psy_audio_IntMachineParam*);
static void intmachineparam_range(psy_audio_IntMachineParam*,
	intptr_t* minval, intptr_t* maxval);
static int intmachineparam_type(psy_audio_IntMachineParam*);
static int intmachineparam_label(psy_audio_IntMachineParam*, char* text);
static int intmachineparam_name(psy_audio_IntMachineParam*, char* text);
static int intmachineparam_describe(psy_audio_IntMachineParam*, char* text);

static MachineParamVtable intmachineparam_vtable;
static int intmachineparam_vtable_initialized = 0;

static void intmachineparam_vtable_init(psy_audio_IntMachineParam* self)
{
	if (!intmachineparam_vtable_initialized) {
		intmachineparam_vtable = *(self->machineparam.vtable);
		intmachineparam_vtable.tweak = (fp_machineparam_tweak)intmachineparam_tweak;
		intmachineparam_vtable.normvalue = (fp_machineparam_normvalue)intmachineparam_normvalue;
		intmachineparam_vtable.range = (fp_machineparam_range)intmachineparam_range;
		intmachineparam_vtable.name = (fp_machineparam_name)intmachineparam_name;
		intmachineparam_vtable.label = (fp_machineparam_label)intmachineparam_label;
		intmachineparam_vtable.type = (fp_machineparam_type)intmachineparam_type;
		intmachineparam_vtable.describe = (fp_machineparam_describe)intmachineparam_describe;
		intmachineparam_vtable_initialized = 1;
	}
}

void psy_audio_intmachineparam_init(psy_audio_IntMachineParam* self,
	const char* name, const char* label, int type, int32_t* data, intptr_t minval,
	intptr_t maxval)
{
	psy_audio_machineparam_init(&self->machineparam);
	intmachineparam_vtable_init(self);
	self->machineparam.vtable = &intmachineparam_vtable;
	self->name = (name) ? strdup(name) : 0;
	self->label = (label) ? strdup(label) : 0;
	self->mask = 0;
	self->minval = minval;
	self->maxval = maxval;
	self->type = type;
	self->data = data;
}

void psy_audio_intmachineparam_dispose(psy_audio_IntMachineParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
	free(self->name);
	free(self->label);
	free(self->mask);
}

psy_audio_IntMachineParam* psy_audio_intmachineparam_alloc(void)
{
	return (psy_audio_IntMachineParam*)malloc(sizeof(psy_audio_IntMachineParam));
}

psy_audio_IntMachineParam* psy_audio_intmachineparam_allocinit(
	const char* name, const char* label, int type, int32_t* data,
	intptr_t minval, intptr_t maxval)
{
	psy_audio_IntMachineParam* rv;

	rv = psy_audio_intmachineparam_alloc();
	if (rv) {
		psy_audio_intmachineparam_init(rv, name, label, type, data, minval, maxval);
	}
	return rv;
}

void intmachineparam_range(psy_audio_IntMachineParam* self,
	intptr_t* minval, intptr_t* maxval)
{
	*minval = self->minval;
	*maxval = self->maxval;
}

void intmachineparam_tweak(psy_audio_IntMachineParam* self, float value)
{
	if (self->data) {
		intptr_t scaled;

		scaled = (intptr_t)(value * (self->maxval - self->minval) + 0.5f) +
			self->minval;
		*self->data = (int32_t)scaled;
		psy_signal_emit_float(&self->machineparam.signal_tweak, self, value);
	} else {
		psy_signal_emit_float(&self->machineparam.signal_tweak, self, value);
	}
}

float intmachineparam_normvalue(psy_audio_IntMachineParam* self)
{
	float rv = 0.f;

	if (self->data) {
		rv = ((self->maxval - self->minval) != 0)
			? (*self->data - self->minval) /
			(float)(self->maxval - self->minval)
			: 0.f;
	} else {
		psy_signal_emit(&self->machineparam.signal_normvalue, self, 1, (void*)(&rv));
	}
	return rv;
}

int intmachineparam_describe(psy_audio_IntMachineParam* self, char* text)
{
	int rv = 1;

	psy_snprintf(text, 128, (self->mask) ? self->mask : "%d", 
		(int) psy_audio_machineparam_scaledvalue(
			psy_audio_intmachineparam_base(self)));
	return rv;
}

int intmachineparam_label(psy_audio_IntMachineParam* self, char* text)
{
	if (self->label) {
		psy_snprintf(text, 128, "%s", self->label);
		psy_signal_emit(&self->machineparam.signal_label, self, 1, text);
		return 1;
	}
	return 0;
}

int intmachineparam_name(psy_audio_IntMachineParam* self, char* text)
{
	if (self->name) {
		psy_snprintf(text, 128, "%s", self->name);
		psy_signal_emit(&self->machineparam.signal_name, self, 1, text);
		return 1;
	}
	return 0;
}

int intmachineparam_type(psy_audio_IntMachineParam* self)
{
	return self->type;
}

void psy_audio_intmachineparam_setmask(psy_audio_IntMachineParam* self, const char* mask)
{
	free(self->mask);
	self->mask = (mask) ? strdup(mask) : 0; 
}


// UIntPtrMachineParam
static void uintptrmachineparam_tweak(psy_audio_UIntPtrMachineParam*, float val);
static float uintptrmachineparam_normvalue(psy_audio_UIntPtrMachineParam*);
static void uintptrmachineparam_range(psy_audio_UIntPtrMachineParam*,
	uintptr_t* minval, uintptr_t* maxval);
static int uintptrmachineparam_type(psy_audio_UIntPtrMachineParam*);
static int uintptrmachineparam_label(psy_audio_UIntPtrMachineParam*, char* text);
static int uintptrmachineparam_name(psy_audio_UIntPtrMachineParam*, char* text);
static int uintptrmachineparam_describe(psy_audio_UIntPtrMachineParam*, char* text);

static MachineParamVtable uintptrmachineparam_vtable;
static bool uintptrmachineparam_vtable_initialized = FALSE;

static void uintptrmachineparam_vtable_init(psy_audio_UIntPtrMachineParam* self)
{
	if (!uintptrmachineparam_vtable_initialized) {
		uintptrmachineparam_vtable = *(self->machineparam.vtable);
		uintptrmachineparam_vtable.tweak = (fp_machineparam_tweak)uintptrmachineparam_tweak;
		uintptrmachineparam_vtable.normvalue = (fp_machineparam_normvalue)uintptrmachineparam_normvalue;
		uintptrmachineparam_vtable.range = (fp_machineparam_range)uintptrmachineparam_range;
		uintptrmachineparam_vtable.name = (fp_machineparam_name)uintptrmachineparam_name;
		uintptrmachineparam_vtable.label = (fp_machineparam_label)uintptrmachineparam_label;
		uintptrmachineparam_vtable.type = (fp_machineparam_type)uintptrmachineparam_type;
		uintptrmachineparam_vtable.describe = (fp_machineparam_describe)uintptrmachineparam_describe;
		uintptrmachineparam_vtable_initialized = TRUE;
	}
}

void psy_audio_uintptrmachineparam_init(psy_audio_UIntPtrMachineParam* self,
	const char* name, const char* label, int type, uintptr_t* data,
	uintptr_t minval, uintptr_t maxval)
{
	psy_audio_machineparam_init(&self->machineparam);
	uintptrmachineparam_vtable_init(self);
	self->machineparam.vtable = &uintptrmachineparam_vtable;
	self->name = (name) ? strdup(name) : 0;
	self->label = (label) ? strdup(label) : 0;
	self->mask = 0;
	self->minval = minval;
	self->maxval = maxval;
	self->type = type;
	self->data = data;
}

void psy_audio_uintptrmachineparam_dispose(psy_audio_UIntPtrMachineParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
	free(self->name);
	free(self->label);
	free(self->mask);
}

psy_audio_UIntPtrMachineParam* psy_audio_uintptrmachineparam_alloc(void)
{
	return (psy_audio_UIntPtrMachineParam*)malloc(sizeof(psy_audio_UIntPtrMachineParam));
}

psy_audio_UIntPtrMachineParam* psy_audio_uintptrmachineparam_allocinit(
	const char* name, const char* label, int type, uintptr_t* data,
	uintptr_t minval, uintptr_t maxval)
{
	psy_audio_UIntPtrMachineParam* rv;

	rv = psy_audio_uintptrmachineparam_alloc();
	if (rv) {
		psy_audio_uintptrmachineparam_init(rv, name, label, type, data, minval, maxval);
	}
	return rv;
}

void uintptrmachineparam_range(psy_audio_UIntPtrMachineParam* self,
	uintptr_t* minval, uintptr_t* maxval)
{
	*minval = self->minval;
	*maxval = self->maxval;
}

void uintptrmachineparam_tweak(psy_audio_UIntPtrMachineParam* self, float value)
{
	if (self->data) {
		uintptr_t scaled;

		scaled = (uintptr_t)(value * (self->maxval - self->minval) + 0.5f) +
			self->minval;
		*self->data = scaled;
	}
}

float uintptrmachineparam_normvalue(psy_audio_UIntPtrMachineParam* self)
{
	float rv = 0.f;

	if (self->data) {
		rv = ((self->maxval - self->minval) != 0)
			? (*self->data - self->minval) /
			(float)(self->maxval - self->minval)
			: 0.f;
	} else {
		psy_signal_emit(&self->machineparam.signal_normvalue, self, 1, (void*)(&rv));
	}
	return rv;
}

int uintptrmachineparam_describe(psy_audio_UIntPtrMachineParam* self, char* text)
{
	int rv = 1;

	psy_snprintf(text, 128, (self->mask) ? self->mask : "%u",
		(unsigned int)psy_audio_machineparam_scaledvalue(
			psy_audio_uintptrmachineparam_base(self)));
	return rv;
}

int uintptrmachineparam_label(psy_audio_UIntPtrMachineParam* self, char* text)
{
	if (self->label) {
		psy_snprintf(text, 128, "%s", self->label);
		psy_signal_emit(&self->machineparam.signal_label, self, 1, text);
		return 1;
	}
	return 0;
}

int uintptrmachineparam_name(psy_audio_UIntPtrMachineParam* self, char* text)
{
	if (self->name) {
		psy_snprintf(text, 128, "%s", self->name);
		psy_signal_emit(&self->machineparam.signal_name, self, 1, text);
		return 1;
	}
	return 0;
}

int uintptrmachineparam_type(psy_audio_UIntPtrMachineParam* self)
{
	return self->type;
}

void psy_audio_uintptrmachineparam_setmask(psy_audio_UIntPtrMachineParam* self, const char* mask)
{
	free(self->mask);
	self->mask = (mask) ? strdup(mask) : 0;
}


// FloatMachineParam
static void floatmachineparam_tweak(psy_audio_FloatMachineParam*, float val);
static float floatmachineparam_normvalue(psy_audio_FloatMachineParam*);
static void floatmachineparam_range(psy_audio_FloatMachineParam*,
	intptr_t* minval, intptr_t* maxval);
static int floatmachineparam_type(psy_audio_FloatMachineParam*);
static int floatmachineparam_label(psy_audio_FloatMachineParam*, char* text);
static int floatmachineparam_name(psy_audio_FloatMachineParam*, char* text);
static int floatmachineparam_describe(psy_audio_FloatMachineParam*, char* text);

static MachineParamVtable floatmachineparam_vtable;
static int floatmachineparam_vtable_initialized = 0;

static void floatmachineparam_vtable_init(psy_audio_FloatMachineParam* self)
{
	if (!floatmachineparam_vtable_initialized) {
		floatmachineparam_vtable = *(self->machineparam.vtable);
		floatmachineparam_vtable.tweak = (fp_machineparam_tweak)floatmachineparam_tweak;
		floatmachineparam_vtable.normvalue = (fp_machineparam_normvalue)floatmachineparam_normvalue;
		floatmachineparam_vtable.range = (fp_machineparam_range)floatmachineparam_range;
		floatmachineparam_vtable.name = (fp_machineparam_name)floatmachineparam_name;
		floatmachineparam_vtable.label = (fp_machineparam_label)floatmachineparam_label;
		floatmachineparam_vtable.type = (fp_machineparam_type)floatmachineparam_type;
		floatmachineparam_vtable.describe = (fp_machineparam_describe)floatmachineparam_describe;
		floatmachineparam_vtable_initialized = 1;
	}
}

void psy_audio_floatmachineparam_init(psy_audio_FloatMachineParam* self,
	const char* name, const char* label, int type, float* data, intptr_t minval,
	intptr_t maxval)
{
	psy_audio_machineparam_init(&self->machineparam);
	floatmachineparam_vtable_init(self);
	self->machineparam.vtable = &floatmachineparam_vtable;
	self->name = (name) ? strdup(name) : 0;
	self->label = (label) ? strdup(label) : 0;
	self->mask = 0;
	self->minval = minval;
	self->maxval = maxval;
	self->type = type;
	self->data = data;
}

void psy_audio_floatmachineparam_dispose(psy_audio_FloatMachineParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
	free(self->name);
	free(self->label);
	free(self->mask);
}

psy_audio_FloatMachineParam* psy_audio_floatmachineparam_alloc(void)
{
	return (psy_audio_FloatMachineParam*)malloc(sizeof(psy_audio_FloatMachineParam));
}

psy_audio_FloatMachineParam* psy_audio_floatmachineparam_allocinit(
	const char* name, const char* label, int type, float* data,
	intptr_t minval, intptr_t maxval)
{
	psy_audio_FloatMachineParam* rv;

	rv = psy_audio_floatmachineparam_alloc();
	if (rv) {
		psy_audio_floatmachineparam_init(rv, name, label, type, data, minval, maxval);
	}
	return rv;
}

void floatmachineparam_range(psy_audio_FloatMachineParam* self,
	intptr_t* minval, intptr_t* maxval)
{
	*minval = self->minval;
	*maxval = self->maxval;
}

void floatmachineparam_tweak(psy_audio_FloatMachineParam* self, float value)
{
	if (self->data) {		
		*self->data = value;
	}
}

float floatmachineparam_normvalue(psy_audio_FloatMachineParam* self)
{
	float rv = 0.f;

	if (self->data) {
		rv = *self->data;
	}
	return rv;
}

int floatmachineparam_describe(psy_audio_FloatMachineParam* self, char* text)
{	
	int rv = 0;

	if (self->data) {
		if (self->machineparam.signal_describe.slots != NULL) {
			psy_signal_emit(&self->machineparam.signal_describe, self, 2, (void*)(&rv), (void*)(text));
		} else {
			psy_snprintf(text, 128, (self->mask) ? self->mask : "%f", *self->data);
			rv = 1;
		}
	}
	return rv;
}

int floatmachineparam_label(psy_audio_FloatMachineParam* self, char* text)
{
	if (self->label) {
		psy_snprintf(text, 128, "%s", self->label);
		psy_signal_emit(&self->machineparam.signal_label, self, 1, text);
		return 1;
	}
	return 0;
}

int floatmachineparam_name(psy_audio_FloatMachineParam* self, char* text)
{
	if (self->name) {
		psy_snprintf(text, 128, "%s", self->name);
		psy_signal_emit(&self->machineparam.signal_name, self, 1, text);
		return 1;
	}
	return 0;
}

int floatmachineparam_type(psy_audio_FloatMachineParam* self)
{
	return self->type;
}

void psy_audio_floatmachineparam_setmask(psy_audio_FloatMachineParam* self, const char* mask)
{
	free(self->mask);
	self->mask = (mask) ? strdup(mask) : 0;
}

// Choice Machine Parameter
static void choicemachineparam_tweak(psy_audio_ChoiceMachineParam*, float val);
static float choicemachineparam_normvalue(psy_audio_ChoiceMachineParam*);
static void choicemachineparam_range(psy_audio_ChoiceMachineParam*,
	intptr_t* minval, intptr_t* maxval);
static int choicemachineparam_type(psy_audio_ChoiceMachineParam*);
static int choicemachineparam_label(psy_audio_ChoiceMachineParam*, char* text);
static int choicemachineparam_name(psy_audio_ChoiceMachineParam*, char* text);
static int choicemachineparam_describe(psy_audio_ChoiceMachineParam*, char* text);

static MachineParamVtable choicemachineparam_vtable;
static int choicemachineparam_vtable_initialized = 0;

static void choicemachineparam_vtable_init(psy_audio_ChoiceMachineParam* self)
{
	if (!choicemachineparam_vtable_initialized) {
		choicemachineparam_vtable = *(self->machineparam.vtable);
		choicemachineparam_vtable.tweak = (fp_machineparam_tweak)choicemachineparam_tweak;
		choicemachineparam_vtable.normvalue = (fp_machineparam_normvalue)choicemachineparam_normvalue;
		choicemachineparam_vtable.range = (fp_machineparam_range)choicemachineparam_range;
		choicemachineparam_vtable.name = (fp_machineparam_name)choicemachineparam_name;
		choicemachineparam_vtable.label = (fp_machineparam_label)choicemachineparam_label;
		choicemachineparam_vtable.type = (fp_machineparam_type)choicemachineparam_type;
		choicemachineparam_vtable.describe = (fp_machineparam_describe)choicemachineparam_describe;
		choicemachineparam_vtable_initialized = 1;
	}
}

void psy_audio_choicemachineparam_init(psy_audio_ChoiceMachineParam* self,
	const char* name, const char* label, int type, int32_t* data, intptr_t minval,
	intptr_t maxval)
{
	psy_audio_machineparam_init(&self->machineparam);
	choicemachineparam_vtable_init(self);
	self->machineparam.vtable = &choicemachineparam_vtable;
	self->name = (name) ? strdup(name) : 0;
	self->label = (label) ? strdup(label) : 0;
	self->minval = minval;
	self->maxval = maxval;
	self->type = type;
	self->data = data;
	psy_table_init(&self->descriptions);
}

void psy_audio_choicemachineparam_dispose(psy_audio_ChoiceMachineParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
	free(self->name);
	free(self->label);
	psy_table_disposeall(&self->descriptions, (psy_fp_disposefunc)NULL);
}

void choicemachineparam_range(psy_audio_ChoiceMachineParam* self,
	intptr_t* minval, intptr_t* maxval)
{
	*minval = self->minval;
	*maxval = self->maxval;
}

void choicemachineparam_tweak(psy_audio_ChoiceMachineParam* self, float value)
{
	if (self->data) {
		intptr_t scaled;

		scaled = (intptr_t)(value * (self->maxval - self->minval) + 0.5f) +
			self->minval;
		*self->data = (int32_t)scaled;
		psy_signal_emit_float(&self->machineparam.signal_tweak, self, value);
	} else {
		psy_signal_emit_float(&self->machineparam.signal_tweak, self, value);
	}
}

float choicemachineparam_normvalue(psy_audio_ChoiceMachineParam* self)
{
	float rv = 0.f;

	if (self->data) {
		rv = ((self->maxval - self->minval) != 0)
			? (*self->data - self->minval) /
			(float)(self->maxval - self->minval)
			: 0.f;
	} else {
		psy_signal_emit(&self->machineparam.signal_normvalue, self, 1, (void*)(&rv));
	}
	return rv;
}

int choicemachineparam_describe(psy_audio_ChoiceMachineParam* self, char* text)
{
	char* desc;
	uintptr_t selection;
	
	selection = (uintptr_t)(choicemachineparam_normvalue(self) * (self->maxval - self->minval));
	desc = (char*)psy_table_at(&self->descriptions, selection);
	if (desc) {
		psy_snprintf(text, 128, "%s", desc);
	} else {
		psy_snprintf(text, 128, "%d",
			(int)psy_audio_machineparam_scaledvalue(
				psy_audio_choicemachineparam_base(self)));
	}	
	return TRUE;
}

int choicemachineparam_label(psy_audio_ChoiceMachineParam* self, char* text)
{
	if (self->label) {
		psy_snprintf(text, 128, "%s", self->label);
		psy_signal_emit(&self->machineparam.signal_label, self, 1, text);
		return 1;
	}
	return 0;
}

int choicemachineparam_name(psy_audio_ChoiceMachineParam* self, char* text)
{
	if (self->name) {
		psy_snprintf(text, 128, "%s", self->name);
		psy_signal_emit(&self->machineparam.signal_name, self, 1, text);
		return 1;
	}
	return 0;
}

int choicemachineparam_type(psy_audio_ChoiceMachineParam* self)
{
	return self->type;
}

void psy_audio_choicemachineparam_setdescription(psy_audio_ChoiceMachineParam*
	self, uintptr_t index, const char* desc)
{
	char* olddesc;

	olddesc = (char*)psy_table_at(&self->descriptions, index);
	if (olddesc) {		
		psy_table_remove(&self->descriptions, index);
		free(olddesc);
	}
	if (desc) {
		psy_table_insert(&self->descriptions, index, strdup(desc));
	}
}

// VolumeMachineParam
static void volumemachineparam_tweak(psy_audio_VolumeMachineParam*, float val);
static float volumemachineparam_normvalue(psy_audio_VolumeMachineParam*);
static void volumemachineparam_range(psy_audio_VolumeMachineParam*,
	intptr_t* minval, intptr_t* maxval);
static int volumemachineparam_type(psy_audio_VolumeMachineParam*);
static int volumemachineparam_label(psy_audio_VolumeMachineParam*, char* text);
static int volumemachineparam_name(psy_audio_VolumeMachineParam*, char* text);
static int volumemachineparam_describe(psy_audio_VolumeMachineParam*, char* text);

static MachineParamVtable volumemachineparam_vtable;
static int volumemachineparam_vtable_initialized = 0;

static void volumemachineparam_vtable_init(psy_audio_VolumeMachineParam* self)
{
	if (!volumemachineparam_vtable_initialized) {
		volumemachineparam_vtable = *(self->machineparam.vtable);
		volumemachineparam_vtable.tweak = (fp_machineparam_tweak)volumemachineparam_tweak;
		volumemachineparam_vtable.normvalue = (fp_machineparam_normvalue)volumemachineparam_normvalue;
		volumemachineparam_vtable.range = (fp_machineparam_range)volumemachineparam_range;
		volumemachineparam_vtable.name = (fp_machineparam_name)volumemachineparam_name;
		volumemachineparam_vtable.label = (fp_machineparam_label)volumemachineparam_label;
		volumemachineparam_vtable.type = (fp_machineparam_type)volumemachineparam_type;
		volumemachineparam_vtable.describe = (fp_machineparam_describe)volumemachineparam_describe;
		volumemachineparam_vtable_initialized = 1;
	}
}

void psy_audio_volumemachineparam_init(psy_audio_VolumeMachineParam* self,
	const char* name, const char* label, int type, float* data)
{
	psy_audio_machineparam_init(&self->machineparam);
	volumemachineparam_vtable_init(self);
	self->machineparam.vtable = &volumemachineparam_vtable;
	self->name = (name) ? strdup(name) : 0;
	self->label = (label) ? strdup(label) : 0;
	self->mask = 0;
	self->minval = 0;
	self->maxval = 0x1000;
	self->type = type;
	self->data = data;
	self->mode = psy_audio_VOLUME_MIXER;
}

void psy_audio_volumemachineparam_dispose(psy_audio_VolumeMachineParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
	free(self->name);
	free(self->label);
	free(self->mask);
}

psy_audio_VolumeMachineParam* psy_audio_volumemachineparam_alloc(void)
{
	return (psy_audio_VolumeMachineParam*)malloc(sizeof(psy_audio_VolumeMachineParam));
}

psy_audio_VolumeMachineParam* psy_audio_volumemachineparam_allocinit(
	const char* name, const char* label, int type, float* data)
{
	psy_audio_VolumeMachineParam* rv;

	rv = psy_audio_volumemachineparam_alloc();
	if (rv) {
		psy_audio_volumemachineparam_init(rv, name, label, type, data);
	}
	return rv;
}

void volumemachineparam_range(psy_audio_VolumeMachineParam* self,
	intptr_t* minval, intptr_t* maxval)
{
	*minval = self->minval;
	*maxval = self->maxval;
}

void volumemachineparam_tweak(psy_audio_VolumeMachineParam* self, float value)
{
	if (self->data) {				
		if (self->mode == psy_audio_VOLUME_MIXER) {
			float dbs;
			int scaled;

			scaled = (int)(value * self->maxval);
			dbs = (scaled / 42.67f) - 96.0f;
			*self->data = psy_dsp_convert_db_to_amp(dbs);
		} else {
			// psy_audio_VOLUME_LINEAR
			*self->data = (int)(value * self->maxval) / (float)self->maxval;
		}
	}
}

float volumemachineparam_normvalue(psy_audio_VolumeMachineParam* self)
{
	float rv = 0.f;

	if (self->data) {
		if (self->mode == psy_audio_VOLUME_MIXER) {
			float dbs;
			int value;
			int scaled;

			value = (int)(*self->data * 0x1000);

			dbs = psy_dsp_convert_amp_to_db(*self->data);
			scaled = (int)((dbs + 96.0f) * 42.67); // *(0x1000 / 96.0f)		
			rv = scaled / (float)0x1000;
		} else {
			// psy_audio_VOLUME_LINEAR
			rv = *self->data;
		}
	}
	return rv;
}

int volumemachineparam_describe(psy_audio_VolumeMachineParam* self, char* text)
{
	if (self->data) {
		if (*self->data < 0.00002f) {
			psy_snprintf(text, 20, "%s", "-inf");
		} else {
			if (self->mode == psy_audio_VOLUME_MIXER) {
				float dbs;

				dbs = psy_dsp_convert_amp_to_db(*self->data);
				psy_snprintf(text, 20, "%.01fdB", (float)dbs);
			} else {
				psy_snprintf(text, 20, "%d", (int)(*self->data * self->maxval));
			}
		}
	}
	return self->data != NULL;
}

int volumemachineparam_label(psy_audio_VolumeMachineParam* self, char* text)
{
	if (self->label) {
		psy_snprintf(text, 128, "%s", self->label);
		psy_signal_emit(&self->machineparam.signal_label, self, 1, text);
		return 1;
	}
	return 0;
}

int volumemachineparam_name(psy_audio_VolumeMachineParam* self, char* text)
{
	if (self->name) {
		psy_snprintf(text, 128, "%s", self->name);
		psy_signal_emit(&self->machineparam.signal_name, self, 1, text);
		return 1;
	}
	return 0;
}

int volumemachineparam_type(psy_audio_VolumeMachineParam* self)
{
	return self->type;
}

void psy_audio_volumemachineparam_setmode(psy_audio_VolumeMachineParam* self,
	psy_audio_VolumeMode mode)
{
	self->mode = mode;
}

void psy_audio_volumemachineparam_setrange(psy_audio_VolumeMachineParam* self, intptr_t minval,
	intptr_t maxval)
{
	self->minval = minval;
	self->maxval = maxval;
}

void psy_audio_volumemachineparam_setmask(psy_audio_VolumeMachineParam* self, const char* mask)
{
	free(self->mask);
	self->mask = (mask) ? strdup(mask) : 0;
}

// GainMachineParam
static void gainmachineparam_tweak(psy_audio_GainMachineParam*, float val);
static float gainmachineparam_normvalue(psy_audio_GainMachineParam*);
static void gainmachineparam_range(psy_audio_GainMachineParam*,
	intptr_t* minval, intptr_t* maxval);
static int gainmachineparam_type(psy_audio_GainMachineParam*);
static int gainmachineparam_label(psy_audio_GainMachineParam*, char* text);
static int gainmachineparam_name(psy_audio_GainMachineParam*, char* text);
static int gainmachineparam_describe(psy_audio_GainMachineParam*, char* text);

static MachineParamVtable gainmachineparam_vtable;
static int gainmachineparam_vtable_initialized = 0;

static void gainmachineparam_vtable_init(psy_audio_GainMachineParam* self)
{
	if (!gainmachineparam_vtable_initialized) {
		gainmachineparam_vtable = *(self->machineparam.vtable);
		gainmachineparam_vtable.tweak = (fp_machineparam_tweak)gainmachineparam_tweak;
		gainmachineparam_vtable.normvalue = (fp_machineparam_normvalue)gainmachineparam_normvalue;
		gainmachineparam_vtable.range = (fp_machineparam_range)gainmachineparam_range;
		gainmachineparam_vtable.name = (fp_machineparam_name)gainmachineparam_name;
		gainmachineparam_vtable.label = (fp_machineparam_label)gainmachineparam_label;
		gainmachineparam_vtable.type = (fp_machineparam_type)gainmachineparam_type;
		gainmachineparam_vtable.describe = (fp_machineparam_describe)gainmachineparam_describe;
		gainmachineparam_vtable_initialized = 1;
	}
}

void psy_audio_gainmachineparam_init(psy_audio_GainMachineParam* self,
	const char* name, const char* label, int type, float* data, intptr_t minval,
	intptr_t maxval)
{
	psy_audio_machineparam_init(&self->machineparam);
	gainmachineparam_vtable_init(self);
	self->machineparam.vtable = &gainmachineparam_vtable;
	self->name = (name) ? strdup(name) : 0;
	self->label = (label) ? strdup(label) : 0;
	self->mask = 0;
	self->minval = minval;
	self->maxval = maxval;
	self->type = type;
	self->data = data;
}

void psy_audio_gainmachineparam_dispose(psy_audio_GainMachineParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
	free(self->name);
	free(self->label);
	free(self->mask);
}

psy_audio_GainMachineParam* psy_audio_gainmachineparam_alloc(void)
{
	return (psy_audio_GainMachineParam*)malloc(sizeof(psy_audio_GainMachineParam));
}

psy_audio_GainMachineParam* psy_audio_gainmachineparam_allocinit(
	const char* name, const char* label, int type, float* data,
	intptr_t minval, intptr_t maxval)
{
	psy_audio_GainMachineParam* rv;

	rv = psy_audio_gainmachineparam_alloc();
	if (rv) {
		psy_audio_gainmachineparam_init(rv, name, label, type, data, minval, maxval);
	}
	return rv;
}

void gainmachineparam_range(psy_audio_GainMachineParam* self,
	intptr_t* minval, intptr_t* maxval)
{
	*minval = self->minval;
	*maxval = self->maxval;
}

void gainmachineparam_tweak(psy_audio_GainMachineParam* self, float value)
{
	if (self->data) {
		*self->data = value * 4;
	}
}

float gainmachineparam_normvalue(psy_audio_GainMachineParam* self)
{
	float rv = 0.f;

	if (self->data) {
		rv = (float)*self->data / 4;
	}
	return rv;
}

int gainmachineparam_describe(psy_audio_GainMachineParam* self, char* text)
{
	if (self->data) {
		float val = *self->data;
		float dbs = (((val > 0.0f) ? psy_dsp_convert_amp_to_db(*self->data) : -100.0f));
		psy_snprintf(text, 128, "%.01fdB", dbs);	
	}
	return self->data != NULL;
}

int gainmachineparam_label(psy_audio_GainMachineParam* self, char* text)
{
	if (self->label) {
		psy_snprintf(text, 128, "%s", self->label);
		psy_signal_emit(&self->machineparam.signal_label, self, 1, text);
		return 1;
	}
	return 0;
}

int gainmachineparam_name(psy_audio_GainMachineParam* self, char* text)
{
	if (self->name) {
		psy_snprintf(text, 128, "%s", self->name);
		psy_signal_emit(&self->machineparam.signal_name, self, 1, text);
		return 1;
	}
	return 0;
}

int gainmachineparam_type(psy_audio_GainMachineParam* self)
{
	return self->type;
}

void psy_audio_gainmachineparam_setmask(psy_audio_GainMachineParam* self, const char* mask)
{
	free(self->mask);
	self->mask = (mask) ? strdup(mask) : 0;
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MACHINEPARAM_H
#define psy_audio_MACHINEPARAM_H

#include <signal.h>
#include <hashtbl.h>
#include <dsptypes.h>

#include "../../detail/psydef.h"


#ifdef __cplusplus
extern "C" {
#endif

// psy_audio_MachineParam
// 
// Aim: Base class for various parameter classes (e.g. int, choice, gain).
//      Enables reuse of repeating parameter classes. Instead tweaking with
//      a parameter index, the machine is asked with a index for a parameter
//      object. On the parameter object operations can be done, either direct
//      or using the machine interface having proxy exception handling

struct psy_audio_MachineParam;

typedef	void (*fp_machineparam_tweak)(struct psy_audio_MachineParam*, float val);
typedef float (*fp_machineparam_normvalue)(struct psy_audio_MachineParam*);
typedef void (*fp_machineparam_range)(struct psy_audio_MachineParam*, intptr_t* minval, intptr_t* maxval);
typedef	int (*fp_machineparam_type)(struct psy_audio_MachineParam*);
typedef	int (*fp_machineparam_label)(struct psy_audio_MachineParam*, char* text);
typedef	int (*fp_machineparam_name)(struct psy_audio_MachineParam*, char* text);
typedef	int (*fp_machineparam_describe)(struct psy_audio_MachineParam*, char* text);

typedef struct MachineParamVtable {
	fp_machineparam_normvalue normvalue;
	fp_machineparam_range range;
	fp_machineparam_type type;
	fp_machineparam_label label;
	fp_machineparam_name name;
	fp_machineparam_describe describe;
	// events
	fp_machineparam_tweak tweak;
} MachineParamVtable;

typedef struct psy_audio_MachineParam {
	MachineParamVtable* vtable;
	psy_Signal signal_normvalue;
	psy_Signal signal_tweak;
	psy_Signal signal_describe;
	psy_Signal signal_name;
	psy_Signal signal_label;
	int crashed;
} psy_audio_MachineParam;

void psy_audio_machineparam_init(psy_audio_MachineParam*);
void psy_audio_machineparam_dispose(psy_audio_MachineParam*);

INLINE void psy_audio_machineparam_tweak(psy_audio_MachineParam* self, float value)
{
	self->vtable->tweak(self, value);
}

// [0.0f..1.0f] 
INLINE float psy_audio_machineparam_normvalue(psy_audio_MachineParam* self)
{
	return self->vtable->normvalue(self);
}

// scale in integer
INLINE void psy_audio_machineparam_range(psy_audio_MachineParam* self,
	intptr_t* minval, intptr_t* maxval)
{
	self->vtable->range(self, minval, maxval);
}

INLINE int psy_audio_machineparam_name(psy_audio_MachineParam* self, char* text)
{
	return self->vtable->name(self, text);
}

INLINE int psy_audio_machineparam_label(psy_audio_MachineParam* self, char* text)
{
	return self->vtable->label(self, text);
}

INLINE int psy_audio_machineparam_type(psy_audio_MachineParam* self)
{
	return self->vtable->type(self);
}

INLINE int psy_audio_machineparam_describe(psy_audio_MachineParam* self, char* text)
{
	return self->vtable->describe(self, text);
}

// converts normvalue(0.f .. 1.f) -> scaled integer value 
INLINE intptr_t psy_audio_machineparam_scaledvalue(psy_audio_MachineParam* self)
{
	intptr_t rv;
	intptr_t minval;
	intptr_t maxval;
	intptr_t range;

	self->vtable->range(self, & minval, & maxval);
	range = maxval - minval;
	rv = (int)(self->vtable->normvalue(self) * range) + minval;
	return rv;
}

struct psy_audio_CustomMachineParam;

typedef struct psy_audio_CustomMachineParam {
	psy_audio_MachineParam machineparam;
	intptr_t minval;
	intptr_t maxval;
	char* name;
	char* label;
	int type;	
	uintptr_t index;
	intptr_t row;
} psy_audio_CustomMachineParam;

void psy_audio_custommachineparam_init(psy_audio_CustomMachineParam*,
	const char* name, const char* label, int type, int minval, int maxval);
void psy_audio_custommachineparam_dispose(psy_audio_CustomMachineParam*);

psy_audio_CustomMachineParam* psy_audio_custommachineparam_alloc(void);
psy_audio_CustomMachineParam* psy_audio_custommachineparam_allocinit(
	const char* name, const char* label, int type,
	int minval, int maxval);
INLINE psy_audio_MachineParam* psy_audio_custommachineparam_base(psy_audio_CustomMachineParam* self)
{
	return &(self->machineparam);
}

// psy_audio_InfoMachineParam
typedef struct psy_audio_InfoMachineParam {
	psy_audio_MachineParam machineparam;
	char* name;
	char* label;
	int style;
} psy_audio_InfoMachineParam;

void psy_audio_infomachineparam_init(psy_audio_InfoMachineParam*,
	const char* name, const char* label, int style);
void psy_audio_infomachineparam_dispose(psy_audio_InfoMachineParam*);

INLINE psy_audio_MachineParam* psy_audio_infomachineparam_base(psy_audio_InfoMachineParam* self)
{
	return &(self->machineparam);
}

psy_audio_InfoMachineParam* psy_audio_infomachineparam_alloc(void);
psy_audio_InfoMachineParam* psy_audio_infomachineparam_allocinit(
	const char* name, const char* label, int style);

// psy_audio_IntMachineParam
typedef struct psy_audio_IntMachineParam {
	psy_audio_MachineParam machineparam;
	intptr_t minval;
	intptr_t maxval;
	char* name;
	char* label;
	char* mask;
	int type;
	int32_t* data;
} psy_audio_IntMachineParam;

void psy_audio_intmachineparam_init(psy_audio_IntMachineParam*,
	const char* name, const char* label, int type, int32_t* data, intptr_t minval,
	intptr_t maxval);
void psy_audio_intmachineparam_dispose(psy_audio_IntMachineParam*);
void psy_audio_intmachineparam_setmask(psy_audio_IntMachineParam*, const char* mask);

INLINE psy_audio_MachineParam* psy_audio_intmachineparam_base(psy_audio_IntMachineParam* self)
{
	return &(self->machineparam);
}

psy_audio_IntMachineParam* psy_audio_intmachineparam_alloc(void);
psy_audio_IntMachineParam* psy_audio_intmachineparam_allocinit(
	const char* name, const char* label, int type, intptr_t* data, intptr_t minval,
	int32_t maxval);

// psy_audio_FloatMachineParam [0 .. 1]
typedef struct psy_audio_FloatMachineParam {
	psy_audio_MachineParam machineparam;
	intptr_t minval;
	intptr_t maxval;
	char* name;
	char* label;
	char* mask;
	int type;
	float* data;
} psy_audio_FloatMachineParam;

void psy_audio_floatmachineparam_init(psy_audio_FloatMachineParam*,
	const char* name, const char* label, int type, float* data, intptr_t minval,
	intptr_t maxval);
void psy_audio_floatmachineparam_dispose(psy_audio_FloatMachineParam*);
void psy_audio_floatmachineparam_setmask(psy_audio_FloatMachineParam*, const char* mask);

INLINE psy_audio_MachineParam* psy_audio_floatmachineparam_base(psy_audio_FloatMachineParam* self)
{
	return &(self->machineparam);
}

psy_audio_FloatMachineParam* psy_audio_floatmachineparam_alloc(void);
psy_audio_FloatMachineParam* psy_audio_floatmachineparam_allocinit(
	const char* name, const char* label, int type, float* data, intptr_t minval,
	intptr_t maxval);

// psy_audio_ChoiceMachineParam

typedef struct psy_audio_ChoiceMachineParam {
	psy_audio_MachineParam machineparam;
	intptr_t minval;
	intptr_t maxval;
	char* name;
	char* label;
	int type;
	int32_t* data;
	psy_Table descriptions;
} psy_audio_ChoiceMachineParam;

void psy_audio_choicemachineparam_init(psy_audio_ChoiceMachineParam*,
	const char* name, const char* label, int type, int32_t* data, intptr_t minval,
	intptr_t maxval);
void psy_audio_choicemachineparam_dispose(psy_audio_ChoiceMachineParam*);
void psy_audio_choicemachineparam_setdescription(psy_audio_ChoiceMachineParam*,
	uintptr_t index, const char* desc);

INLINE psy_audio_MachineParam* psy_audio_choicemachineparam_base(psy_audio_ChoiceMachineParam* self)
{
	return &(self->machineparam);
}

typedef enum {
	psy_audio_VOLUME_LINEAR,
	psy_audio_VOLUME_MIXER
} psy_audio_VolumeMode;

// psy_audio_VolumeMachineParam [0 .. 1]
typedef struct psy_audio_VolumeMachineParam {
	psy_audio_MachineParam machineparam;
	intptr_t minval;
	intptr_t maxval;
	char* name;
	char* label;
	char* mask;
	int type;
	float* data;
	int scale;
	psy_audio_VolumeMode mode;
} psy_audio_VolumeMachineParam;

void psy_audio_volumemachineparam_init(psy_audio_VolumeMachineParam*,
	const char* name, const char* label, int type, float* data);
void psy_audio_volumemachineparam_dispose(psy_audio_VolumeMachineParam*);
void psy_audio_volumemachineparam_setmode(psy_audio_VolumeMachineParam*, psy_audio_VolumeMode);
void psy_audio_volumemachineparam_setrange(psy_audio_VolumeMachineParam*, intptr_t minval,
intptr_t maxval);
void psy_audio_volumemachineparam_setmask(psy_audio_VolumeMachineParam*, const char* mask);

psy_audio_VolumeMachineParam* psy_audio_volumemachineparam_alloc(void);
psy_audio_VolumeMachineParam* psy_audio_volumemachineparam_allocinit(
	const char* name, const char* label, int type, float* data);

INLINE psy_audio_MachineParam* psy_audio_volumemachineparam_base(psy_audio_VolumeMachineParam* self)
{
	return &(self->machineparam);
}

// psy_audio_GainMachineParam [0 .. 4 // -12db .. 12db]
typedef struct psy_audio_GainMachineParam {
	psy_audio_MachineParam machineparam;
	intptr_t minval;
	intptr_t maxval;
	char* name;
	char* label;
	char* mask;
	int type;
	float* data;
} psy_audio_GainMachineParam;

void psy_audio_gainmachineparam_init(psy_audio_GainMachineParam*,
	const char* name, const char* label, int type, float* data, int32_t minval,
	int32_t maxval);
void psy_audio_gainmachineparam_dispose(psy_audio_GainMachineParam*);
void psy_audio_gainmachineparam_setmask(psy_audio_GainMachineParam*, const char* mask);

psy_audio_GainMachineParam* psy_audio_gainmachineparam_alloc(void);
psy_audio_GainMachineParam* psy_audio_gainmachineparam_allocinit(
	const char* name, const char* label, int type, float* data, intptr_t minval,
	intptr_t maxval);

INLINE psy_audio_MachineParam* psy_audio_gainmachineparam_base(psy_audio_GainMachineParam* self)
{
	return &(self->machineparam);
}



#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINEPARAM_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_LADSPAPLUGIN_H
#define psy_audio_LADSPAPLUGIN_H

#include "custommachine.h"
#include "library.h"
#include "ladspa.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {	
	LADSPA_PortDescriptor descriptor_;
	LADSPA_PortRangeHint hint_;
	char* portName_;
	LADSPA_Data value_;
	LADSPA_Data minVal_;
	LADSPA_Data maxVal_;
	float rangeMultiplier_;
	bool integer_;
	bool logarithmic_;
} LadspaParam;

void ladspaparam_init(LadspaParam*, LADSPA_PortDescriptor, LADSPA_PortRangeHint, const char* newname);
void ladspaparam_dispose(LadspaParam*);
LadspaParam* ladspaparam_alloc(void);
LadspaParam* ladspaparam_allocinit(LADSPA_PortDescriptor, LADSPA_PortRangeHint, const char* newname);
INLINE LADSPA_PortDescriptor ladspaparam_descriptor(LadspaParam* self) { return self->descriptor_; }
INLINE LADSPA_PortRangeHintDescriptor ladspaparam_hint(LadspaParam* self) { return self->hint_.HintDescriptor; }
INLINE const char* ladspaparam_name(LadspaParam* self) { return self->portName_; }
INLINE LADSPA_Data ladspaparam_rawvalue(LadspaParam* self) { return self->value_; }
INLINE LADSPA_Data* ladspaparam_valueaddress(LadspaParam* self) { return &self->value_; }
int ladspaparam_value(LadspaParam*);
void ladspaparam_setValue(LadspaParam*, int data);
INLINE void ladspaparam_setrawvalue(LadspaParam* self, LADSPA_Data data) { self->value_ = data; }
void ladspaparam_setdefault(LadspaParam*);
INLINE LADSPA_Data ladspaparam_minval(LadspaParam* self)
{ 
	return self->integer_ ? (self->minVal_ * self->rangeMultiplier_) : 0;
}
INLINE LADSPA_Data ladspaparam_maxval(LadspaParam* self)
{
	return self->integer_ ? (self->maxVal_ - self->minVal_) : 65535;
}

typedef struct {
	psy_audio_CustomMachine custommachine;	
	psy_Library library;
	psy_audio_MachineInfo* plugininfo;
	int key_;				
	const LADSPA_Descriptor* psDescriptor;
	/*const*/ LADSPA_Handle pluginHandle;
	psy_Table values_;
	//psycle index, ladspa index
	psy_Table inportmap; // int x int
	psy_Table outportmap; // int x int
	int numInputs;
	int numOutputs;
	float* pOutSamplesL;
	float* pOutSamplesR;
} psy_audio_LadspaPlugin;

void psy_audio_ladspaplugin_init(psy_audio_LadspaPlugin*, psy_audio_MachineCallback*, const char* path, uintptr_t shellidx);
int psy_audio_plugin_ladspa_test(const char* path, psy_audio_MachineInfo*, uintptr_t shellidx);

INLINE psy_audio_Machine* psy_audio_ladspaplugin_base(psy_audio_LadspaPlugin* self)
{
	return &(self->custommachine.machine);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_LADSPAPLUGIN_H */

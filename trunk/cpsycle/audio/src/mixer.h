// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MIXER_H
#define psy_audio_MIXER_H

#include "custommachine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_DryWetMixMachineParam {
	psy_audio_MachineParam machineparam;
	int32_t* mute;
	int32_t* dryonly;
	int32_t* wetonly;
} psy_audio_DryWetMixMachineParam;

void psy_audio_drywetmixmachineparam_init(psy_audio_DryWetMixMachineParam*,
	int32_t* mute, int32_t* dryonly, int32_t* wetonly);
void psy_audio_drywetmixmachineparam_dispose(psy_audio_DryWetMixMachineParam*);

INLINE psy_audio_MachineParam* psy_audio_drywetmixmachineparam_base(
	psy_audio_DryWetMixMachineParam* self)
{
	return &(self->machineparam);
}

typedef struct {
	psy_Table sendvols;
	psy_dsp_amp_t volume;
	psy_dsp_amp_t panning;
	psy_dsp_amp_t drymix;
	psy_dsp_amp_t gain;
	int mute;
	int dryonly;
	int wetonly;
	size_t inputslot;
	psy_audio_Buffer* buffer;
	psy_dsp_amp_t volumedisplay;
	psy_audio_InfoMachineParam info_param;
	psy_audio_FloatMachineParam dw_param;
	psy_audio_GainMachineParam gain_param;
	psy_audio_FloatMachineParam pan_param;
	psy_audio_VolumeMachineParam slider_param;
	psy_audio_CustomMachineParam level_param;
	psy_audio_IntMachineParam solo_param;
	psy_audio_IntMachineParam mute_param;
	psy_audio_IntMachineParam dryonly_param;
	psy_audio_IntMachineParam wetonly_param;
	psy_audio_DryWetMixMachineParam drywetmix_param;
} psy_audio_MasterChannel;

struct psy_audio_Mixer;

typedef struct {
	psy_Table sendvols;
	psy_dsp_amp_t volume;
	psy_dsp_amp_t panning;
	psy_dsp_amp_t drymix;
	psy_dsp_amp_t gain;
	int mute;
	int dryonly;
	int wetonly;
	size_t inputslot;
	psy_audio_Buffer* buffer;
	psy_dsp_amp_t volumedisplay;
	psy_audio_InfoMachineParam info_param;
	psy_audio_FloatMachineParam mix_param;
	psy_audio_CustomMachineParam sendvol_param;
	psy_audio_CustomMachineParam gain_param;
	psy_audio_FloatMachineParam pan_param;
	psy_audio_VolumeMachineParam slider_param;
	psy_audio_CustomMachineParam level_param;
	psy_audio_IntMachineParam solo_param;
	psy_audio_IntMachineParam mute_param;
	psy_audio_IntMachineParam dryonly_param;
	psy_audio_IntMachineParam wetonly_param;
	psy_audio_DryWetMixMachineParam drywetmix_param;
	struct psy_audio_Mixer* mixer;
} psy_audio_MixerChannel;

typedef struct {
	psy_Table sendsto;
	unsigned char mastersend;
	psy_dsp_amp_t volume;
	psy_dsp_amp_t panning;
	int mute;
	size_t fxslot;
	psy_audio_Buffer* buffer;
	psy_audio_Machine* fx;
	psy_audio_CustomMachineParam send_param;
	psy_audio_CustomMachineParam info_param;
	psy_audio_FloatMachineParam pan_param;
	psy_audio_VolumeMachineParam slider_param;
	psy_audio_CustomMachineParam level_param;
	psy_audio_IntMachineParam solo_param;
	psy_audio_IntMachineParam mute_param;
} psy_audio_ReturnChannel;

typedef struct psy_audio_Mixer {
	psy_audio_CustomMachine custommachine;
	psy_Table inputs;
	psy_Table sends;
	psy_Table returns;
	int solocolumn;	
	psy_audio_MasterChannel master;
	psy_dsp_amp_t mastervolumedisplay;
	uintptr_t slot;
	psy_audio_CustomMachineParam blank_param;
	psy_audio_CustomMachineParam ignore_param;
	psy_audio_CustomMachineParam route_param;
	psy_audio_CustomMachineParam routemaster_param;
} psy_audio_Mixer;

void mixer_init(psy_audio_Mixer*, MachineCallback);
const psy_audio_MachineInfo* mixer_info(void);

INLINE psy_audio_Machine* psy_audio_mixer_base(psy_audio_Mixer* self)
{
	return &(self->custommachine.machine);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MIXER_H */

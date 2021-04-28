// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MIXER_H
#define psy_audio_MIXER_H

#include "custommachine.h"

#ifdef __cplusplus
extern "C" {
#endif

// psy_audio_Mixer
// 
// Aim: Send Return Mixer and chain sheduling. Psycle has two different mix
// systems, the Buzz wire and the newer send return mixer. To handle both,
// Machine implements the method mix for the wires. Mixer overwrites mix to 
// implement the chain mixing. Mainly the player is doing the machine
// sheduling, but the mixer can exclude them by marking them in
// machines as "sent" and doing its own work calls.
//
// Structure:
//      psy_audio_CustomMachine
//            ^
//            |
//      psy_audio_Mixer

struct psy_audio_Mixer;
struct psy_audio_ReturnChannel;
struct psy_audio_InputChannel;

typedef struct psy_audio_InputLabelParam {
	psy_audio_MachineParam machineparam;
	struct psy_audio_InputChannel* channel;
} psy_audio_InputLabelParam;

void psy_audio_inputlabelparam_init(psy_audio_InputLabelParam*,
	struct psy_audio_InputChannel*);
void psy_audio_inputlabelparam_dispose(psy_audio_InputLabelParam*);

INLINE psy_audio_MachineParam* psy_audio_inputlabelparam_base(
	psy_audio_InputLabelParam* self)
{
	return &(self->machineparam);
}

typedef struct psy_audio_SendReturnLabelParam {
	psy_audio_MachineParam machineparam;
	struct psy_audio_ReturnChannel* channel;
	bool displaysend;
} psy_audio_SendReturnLabelParam;

void psy_audio_sendreturnlabelparam_init(psy_audio_SendReturnLabelParam*,
	struct psy_audio_ReturnChannel*, bool displaysend);
void psy_audio_sendreturnlabelparam_dispose(psy_audio_SendReturnLabelParam*);

INLINE psy_audio_MachineParam* psy_audio_sendreturnlabelparam_base(
	psy_audio_SendReturnLabelParam* self)
{
	return &(self->machineparam);
}

struct psy_audio_ReturnChannel;

typedef struct psy_audio_RouteMachineParam {
	psy_audio_MachineParam machineparam;
	uintptr_t send;	
	struct psy_audio_ReturnChannel* channel;
	struct psy_audio_Mixer* mixer;
} psy_audio_RouteMachineParam;

void psy_audio_routemachineparam_init(psy_audio_RouteMachineParam*,
	struct psy_audio_ReturnChannel*, struct psy_audio_Mixer* mixer, uintptr_t send);
void psy_audio_routemachineparam_dispose(psy_audio_RouteMachineParam*);

INLINE psy_audio_MachineParam* psy_audio_routemachineparam_base(
	psy_audio_RouteMachineParam* self)
{
	return &(self->machineparam);
}

typedef struct psy_audio_MixerSend {
	uintptr_t slot;
	/// Incoming connections Machine vol
	psy_dsp_amp_t inputconvol;
} psy_audio_MixerSend;

void psy_audio_mixersend_init(psy_audio_MixerSend*, uintptr_t slot);
void psy_audio_mixersend_dispose(psy_audio_MixerSend*, uintptr_t slot);

psy_audio_MixerSend* psy_audio_mixersend_alloc(void);
psy_audio_MixerSend* psy_audio_mixersend_allocinit(uintptr_t slot);

struct psy_audio_Mixer;

// psy_audio_MasterChannel
typedef struct psy_audio_MasterChannel {
	struct psy_audio_Mixer* mixer;
	psy_Table sendvols;
	psy_dsp_amp_t volume;
	psy_dsp_amp_t panning;
	psy_dsp_amp_t drymix;
	psy_dsp_amp_t gain;
	int mute;
	int dryonly;
	int wetonly;
	psy_audio_Buffer* buffer;
	psy_dsp_amp_t volumedisplay;
	psy_audio_InfoMachineParam info_param;
	psy_audio_FloatMachineParam dw_param;
	psy_audio_GainMachineParam gain_param;
	psy_audio_FloatMachineParam pan_param;
	psy_audio_VolumeMachineParam slider_param;
	psy_audio_IntMachineParam level_param;	
} psy_audio_MasterChannel;

void masterchannel_init(psy_audio_MasterChannel*, struct psy_audio_Mixer*,
	const char* name, const char* label);
void masterchannel_dispose(psy_audio_MasterChannel*);

psy_audio_MasterChannel* masterchannel_allocinit(struct psy_audio_Mixer*,
	const char* name, const char* label);

// psy_audio_InputChannel
typedef struct psy_audio_InputChannel {
	uintptr_t id;
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
	psy_audio_InputLabelParam info_param;
	psy_audio_FloatMachineParam mix_param;
	psy_audio_CustomMachineParam sendvol_param;
	psy_audio_CustomMachineParam gain_param;
	psy_audio_FloatMachineParam pan_param;
	psy_audio_VolumeMachineParam slider_param;
	psy_audio_IntMachineParam level_param;	
	struct psy_audio_Mixer* mixer;	
} psy_audio_InputChannel;

void inputchannel_init(psy_audio_InputChannel*, uintptr_t id,
	struct psy_audio_Mixer*, uintptr_t inputslot);
void inputchannel_dispose(psy_audio_InputChannel*);

psy_audio_InputChannel* inputchannel_allocinit(struct psy_audio_Mixer*,
	uintptr_t id, uintptr_t inputslot);

psy_dsp_amp_t inputchannel_wirevolume(psy_audio_InputChannel*);
bool inputchannel_issoloed(const psy_audio_InputChannel*);
bool psy_audio_inputchannel_active(const psy_audio_InputChannel*);

typedef struct psy_audio_ReturnChannel {
	uintptr_t id;
	struct psy_audio_Mixer* mixer;
	psy_Table sendsto;
	unsigned char mastersend;
	psy_dsp_amp_t volume;
	psy_dsp_amp_t panning;
	int mute;
	size_t fxslot;
	psy_audio_Buffer* buffer;
	psy_audio_Machine* fx;
	psy_List* path;
	psy_audio_CustomMachineParam send_param;
	psy_audio_SendReturnLabelParam sendlabel_param;
	psy_audio_SendReturnLabelParam returnlabel_param;	
	psy_audio_RouteMachineParam route_param;
	psy_audio_CustomMachineParam info_param;
	psy_audio_FloatMachineParam pan_param;
	psy_audio_VolumeMachineParam slider_param;
	psy_audio_IntMachineParam level_param;	
} psy_audio_ReturnChannel;

bool psy_audio_returnchannel_active(const psy_audio_ReturnChannel*);

// psy_audio_Mixer
typedef struct psy_audio_Mixer {
	psy_audio_CustomMachine custommachine;
	psy_Table inputs;
	uintptr_t maxinput;
	psy_Table sends;
	psy_Table returns;
	uintptr_t maxreturn;
	uintptr_t inputsolo;
	uintptr_t returnsolo;
	uintptr_t strobe;
	psy_audio_MasterChannel master;
	psy_audio_CustomMachineParam blank_param;
	psy_audio_InputLabelParam blank_inputinfo_param;
	psy_audio_CustomMachineParam ignore_param;	
	psy_audio_CustomMachineParam routemaster_param;
	psy_audio_CustomMachineParam label_mix_param;
	psy_audio_CustomMachineParam label_gain_param;
	psy_audio_CustomMachineParam label_pan_param;
	psy_audio_CustomMachineParam label_ch_input_param;
	// ui
	psy_audio_IntMachineParam solo_param;
	psy_audio_IntMachineParam mute_param;
	psy_audio_IntMachineParam dry_param;
	psy_audio_IntMachineParam wet_param;
	// pattern tweak
	psy_audio_IntMachineParam solo_tweak_param;
	psy_audio_IntMachineParam mute_tweak_param;
	psy_audio_IntMachineParam dry_tweak_param;	
	psy_audio_IntMachineParam returngrid_tweak_param;
	psy_Table legacyreturn_;
	psy_Table legacysend_;
} psy_audio_Mixer;

void psy_audio_mixer_init(psy_audio_Mixer*, psy_audio_MachineCallback*);
const psy_audio_MachineInfo* psy_audio_mixer_info(void);

INLINE psy_audio_InputChannel* psy_audio_mixer_channel(psy_audio_Mixer* self,
	uintptr_t i)
{
	return (psy_audio_InputChannel*) psy_table_at(&self->inputs, i);
}

INLINE psy_audio_ReturnChannel* psy_audio_mixer_return(psy_audio_Mixer* self,
uintptr_t i)
{
	return (psy_audio_ReturnChannel*) psy_table_at(&self->returns, i);
}

INLINE psy_audio_MixerSend* psy_audio_mixer_Send(psy_audio_Mixer* self,
	uintptr_t i)
{
	return (psy_audio_MixerSend*)psy_table_at(&self->sends, i);
}

psy_audio_InputChannel* psy_audio_mixer_insertchannel(psy_audio_Mixer*,
	uintptr_t idx, psy_audio_InputChannel* input);
psy_audio_ReturnChannel* psy_audio_mixer_insertreturn(psy_audio_Mixer*,
	uintptr_t idx, psy_audio_ReturnChannel* retchan);
psy_audio_MixerSend* psy_audio_mixer_insertsend(psy_audio_Mixer*,
	uintptr_t idx, psy_audio_MixerSend* send);
void psy_audio_mixer_discardchannel(psy_audio_Mixer*, uintptr_t idx);
void psy_audio_mixer_discardreturn(psy_audio_Mixer*, uintptr_t idx);
void psy_audio_mixer_discardsend(psy_audio_Mixer*, uintptr_t idx);

INLINE psy_audio_Machine* psy_audio_mixer_base(psy_audio_Mixer* self)
{
	return &(self->custommachine.machine);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MIXER_H */

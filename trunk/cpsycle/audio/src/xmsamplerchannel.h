// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_XMSAMPLERCHANNEL_H
#define psy_audio_XMSAMPLERCHANNEL_H

#include "custommachine.h"
#include "instrument.h"
#include "sample.h"
#include "multiresampler.h"
#include <adsr.h>
#include <filter.h>
#include <hashtbl.h>
#include <valuemapper.h>

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_SongFile;
struct psy_audio_BufferContext;
struct psy_audio_PatternEvent;

typedef struct psy_audio_XMSamplerChannel {
	// // Channel Index. UINTPTR_MAX used for master
	uintptr_t index;
	// (0..1.0f) value used for Playback (channel volume)
	psy_dsp_amp_t volume;
	//  0..200 .  &0x100 == Surround. // value used for Storage and reset
	float channeldefvolume;
	// memory of note volume of last voice played.
	int lastvoicevolume; 
	// memory of note volume of last voice played.
	float lastvoicerandvol;
	// value used for playback.
	bool mute;
	// (0..1.0f) value used for Playback (pan factor)	
	float panfactor;
	int defaultpanfactor;  //  0..200 .  &0x100 == Surround. // value used for Storage and reset
	float lastvoicepanfactor;
	// (0..1.0f) <-> (0..200)
	bool surround;// value used for playback (is channel set to surround?)
	
	FilterType defaultfiltertype;	
	psy_audio_InfoMachineParam param_channel;
	psy_audio_IntMachineParam filter_cutoff;
	psy_audio_IntMachineParam filter_res;
	psy_audio_IntMachineParam pan;
	psy_audio_VolumeMachineParam slider_param;
	psy_audio_IntMachineParam level_param;

	// effects
	psy_List* effects;
	// volume slide
	int volumeslidemem;
	// volume channel slide
	float chanvolslidespeed;
	int chanvolslidemem;
	// pitchslide
	int	pitchslidespeed;
	int pitchslidemem;
	// pan
	float panslidespeed;
	int panslidemem;
	// vibratro
	psy_audio_WaveForms vibratotype;
	int vibratodepthmem;
	int vibratospeedmem;
	// tremor
	int tremorontime;
	int tremorofftime;
	// tremolo
	psy_audio_WaveForms tremolotype;
	int tremolospeed;
	int tremolodepth;
	float tremolodelta;
	int tremolopos;
	int tremolodepthmem;
	int tremolospeedmem;
	// panbrello
	psy_audio_WaveForms panbrellotype;
	int panbrellospeed;
	int panbrellodepth;
	float panbrellodelta;
	int panbrellopos;
	int panbrellodepthmem;
	int panbrellospeedmem;
	// arpeggio
	double arpeggioperiod[2];
	int arpeggiomem;
	// grissando
	bool grissando;
	// retrig
	int retrigoperation;
	int retrigvol;
	int retrigmem;
	int ticks;
	// offset	
	int offsetMem;
	//TODO: some way to let the user know the value set to this.
	int midi_set;
	int cutoff;
	int ressonance;
	int defaultcutoff;
	int defaultressonance;
	FilterType m_DefaultFilterType;
} psy_audio_XMSamplerChannel;

void psy_audio_xmsamplerchannel_init(psy_audio_XMSamplerChannel*, uintptr_t index);
void psy_audio_xmsamplerchannel_dispose(psy_audio_XMSamplerChannel*);
void psy_audio_xmsamplerchannel_restore(psy_audio_XMSamplerChannel*);
bool  psy_audio_xmsamplerchannel_load(psy_audio_XMSamplerChannel*,
	struct psy_audio_SongFile*);
void psy_audio_xmsamplerchannel_save(psy_audio_XMSamplerChannel*,
	struct psy_audio_SongFile*);
void psy_audio_xmsamplerchannel_work(psy_audio_XMSamplerChannel*, struct psy_audio_BufferContext*);
// effects
void psy_audio_xmsamplerchannel_newline(psy_audio_XMSamplerChannel*);
void psy_audio_xmsamplerchannel_effectinit(psy_audio_XMSamplerChannel*);
void psy_audio_xmsamplerchannel_seteffect(psy_audio_XMSamplerChannel*,
	const struct psy_audio_PatternEvent*);
void psy_audio_xmsamplerchannel_performfx(psy_audio_XMSamplerChannel*);


INLINE void psy_audio_xmsamplerchannel_setpanfactor(psy_audio_XMSamplerChannel*
	self, psy_dsp_amp_t value)
{
	self->panfactor = value;
}

INLINE psy_dsp_amp_t psy_audio_xmsamplerchannel_panfactor(
	psy_audio_XMSamplerChannel* self)
{
	return self->panfactor;
}

INLINE int psy_audio_xmsamplerchannel_defaultpanfactor(
	psy_audio_XMSamplerChannel* self)
{
	return self->defaultpanfactor;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_XMSAMPLERCHANNEL_H */

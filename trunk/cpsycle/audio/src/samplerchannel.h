// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SAMPLERCHANNEL_H
#define psy_audio_SAMPLERCHANNEL_H

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

typedef struct psy_audio_SamplerChannel {
	// // Channel Index. UINTPTR_MAX used for master
	uintptr_t index;
	// (0..1.0f) value used for Playback (channel volume)
	psy_dsp_amp_t volume;	
	// (0..1.0f) value used for Playback (pan factor)	
	float panfactor;
	int defaultpanfactor;  //  0..200 .  &0x100 == Surround. // value used for Storage and reset
	float lastvoicepanfactor;
	// (0..1.0f) <-> (0..200)
	float channeldefvolume;
	int m_DefaultCutoff;
	int m_DefaultRessonance;
	//  0..200 .  &0x100 == Surround. // value used for Storage and reset
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
	// retrig
	int retrigoperation;
	int retrigvol;
	int retrigmem;
	int ticks;
	// offset	
	int offsetMem;
} psy_audio_SamplerChannel;

void psy_audio_samplerchannel_init(psy_audio_SamplerChannel*, uintptr_t index);
void psy_audio_samplerchannel_dispose(psy_audio_SamplerChannel*);
void psy_audio_samplerchannel_restore(psy_audio_SamplerChannel*);
bool  psy_audio_samplerchannel_load(psy_audio_SamplerChannel*,
	struct psy_audio_SongFile*);
void psy_audio_samplerchannel_save(psy_audio_SamplerChannel*,
	struct psy_audio_SongFile*);
void psy_audio_samplerchannel_work(psy_audio_SamplerChannel*, struct psy_audio_BufferContext*);
// effects
void psy_audio_samplerchannel_newline(psy_audio_SamplerChannel*);
void psy_audio_samplerchannel_effectinit(psy_audio_SamplerChannel*);
void psy_audio_samplerchannel_seteffect(psy_audio_SamplerChannel*,
	const struct psy_audio_PatternEvent*);
void psy_audio_samplerchannel_performfx(psy_audio_SamplerChannel*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SAMPLERCHANNEL_H */

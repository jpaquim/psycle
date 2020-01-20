// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SAMPLER_H
#define psy_audio_SAMPLER_H

#include "custommachine.h"
#include "instrument.h"
#include "sample.h"
#include "multiresampler.h"
#include <adsr.h>
#include <multifilter.h>
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLER_DEFAULT_POLYPHONY	8

#define SAMPLER_CMD_NONE			0x00
#define SAMPLER_CMD_PORTAUP			0x01
#define SAMPLER_CMD_PORTADOWN		0x02
#define SAMPLER_CMD_PORTA2NOTE		0x03
#define SAMPLER_CMD_PANNING			0x08
#define SAMPLER_CMD_OFFSET			0x09
#define SAMPLER_CMD_VOLUME			0x0c
#define SAMPLER_CMD_RETRIG			0x15
#define SAMPLER_CMD_EXTENDED		0x0e
#define SAMPLER_CMD_EXT_NOTEOFF		0xc0
#define SAMPLER_CMD_EXT_NOTEDELAY	0xd0

typedef enum
{
	INTERPOL_NONE = 0,
	INTERPOL_LINEAR = 1,
	INTERPOL_SPLINE = 2
}
InterpolationType;

typedef struct {
	struct psy_audio_Sampler* sampler;	
	psy_audio_Instrument* instrument;
	psy_audio_Samples* samples;
	psy_dsp_ADSR env;
	psy_dsp_ADSR filterenv;
	psy_dsp_MultiFilter filter_l;
	psy_dsp_MultiFilter filter_r;
	psy_dsp_MultiResampler resampler;
	psy_List* positions;	
	uintptr_t channel;
	psy_dsp_amp_t vol;
	psy_dsp_amp_t pan;
	int usedefaultvolume;	
	double portaspeed;
	uintptr_t portanumframes;
	uintptr_t portacurrframe;
	int effcmd;
	int effval;
	int dopan;
	int dooffset;
	uint8_t offset;
} Voice;

typedef struct ZxxMacro {
	int mode;
	int value;
} ZxxMacro;

typedef struct psy_audio_Sampler {
	psy_audio_CustomMachine custommachine;		
	psy_List* voices;
	uintptr_t numvoices;
	ResamplerType resamplingmethod;
	int defaultspeed;	
	psy_Table lastinst;
	int maxvolume; // psycle 0CFF, xm 0C80
	int xmsamplerload;
} psy_audio_Sampler;

void psy_audio_sampler_init(psy_audio_Sampler*, MachineCallback);
psy_audio_Sampler* psy_audio_sampler_alloc(void);
psy_audio_Sampler* psy_audio_sampler_allocinit(MachineCallback);
const psy_audio_MachineInfo* psy_audio_sampler_info(void);

INLINE psy_audio_Machine* psy_audio_sampler_base(psy_audio_Sampler* self)
{
	return &(self->custommachine.machine);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SAMPLER_H */

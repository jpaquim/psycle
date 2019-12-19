// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLER_H)
#define SAMPLER_H

#include "custommachine.h"
#include "instrument.h"
#include "sample.h"
#include <adsr.h>
#include <multifilter.h>
#include <hashtbl.h>

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
	struct Sampler* sampler;
	Instrument* instrument;
	Samples* samples;
	ADSR env;
	ADSR filterenv;
	MultiFilter filter_l;
	MultiFilter filter_r;
	List* positions;	
	uintptr_t channel;
	psy_dsp_amp_t vol;
	psy_dsp_amp_t pan;
	int usedefaultvolume;
} Voice;

typedef struct Sampler {
	CustomMachine custommachine;		
	List* voices;
	uintptr_t numvoices;
	int resamplingmethod;
	int defaultspeed;	
	Table lastinst;
	int maxvolume; // psycle 0CFF, xm 0C80
} Sampler;

void sampler_init(Sampler*, MachineCallback);

const MachineInfo* sampler_info(void);

#endif

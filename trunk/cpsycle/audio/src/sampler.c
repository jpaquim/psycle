#include "sampler.h"
#include "pattern.h"
#include "plugin_interface.h"
#include "samples.h"
#include <operations.h>
#include <math.h>

static void generateaudio(Sampler* self, Buffer* input, Buffer* output,
	int numsamples, int tracks);
static void seqtick(Sampler* self, int channel, const PatternEvent* event);
static const CMachineInfo* info(Sampler* self);
static void parametertweak(Sampler* self, int par, int val);
static int describevalue(Sampler*, char* txt, int const param, int const value);
static int value(Sampler*, int const param);
static void setvalue(Sampler*, int const param, int const value);
static void dispose(Sampler* self);
static int mode(Sampler* self) { return MACHMODE_GENERATOR; }
static int unused_voice(Sampler* self);
static void release_voices(Sampler* self, int channel);

static void voice_init(Voice* self, Sample* sample, int channel);
static void voice_dispose(Voice* self);
static void voice_seqtick(Voice* self, const PatternEvent* event);
static void voice_work(Voice* self, Buffer* buffer, int numsamples);
static void voice_release(Voice* self);
static void voice_fastrelease(Voice* self);

static int songtracks = 16;

static CMachineParameter const paraPolyphony = 
{ 
	"Polyphony",
	"Polyphony Voices",								// description
	1,												// MinValue	
	16,												// MaxValue
	MPF_STATE,										// Flags
	0
};

static CMachineParameter const paraInterpolation = {
	"Resampling",
	"Resampling method",
	0,
	3,
	MPF_STATE,
	0
};

static CMachineParameter const paraSpeed = 
{ 
	"Default Speed",
	"Default Speed",								// description
	0,												// MinValue	
	1,												// MaxValue
	MPF_STATE,										// Flags
	0
};

static CMachineParameter const *pParameters[] = {
	&paraPolyphony,
	&paraInterpolation,	
	&paraSpeed
};

static CMachineInfo const MacInfo = {
	MI_VERSION,
	0x0250,
	GENERATOR | 32 | 64,
	sizeof pParameters / sizeof *pParameters,
	pParameters,
	"Sampler"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Sampler",
	"Psycledelics",
	"help",
	3
};

void sampler_init(Sampler* self)
{
	int voice;
	
	machine_init(&self->machine);
	self->machine.generateaudio = generateaudio;
	self->machine.seqtick = seqtick;
	self->machine.info = info;
	self->machine.parametertweak = parametertweak;
	self->machine.describevalue = describevalue;
	self->machine.setvalue = setvalue;
	self->machine.value = value;
	self->machine.dispose = dispose;
	self->machine.mode = mode;
	buffer_init(&self->machine.inputs, 2);
	buffer_init(&self->machine.outputs, 2);	
	self->numvoices = SAMPLER_MAX_POLYPHONY;
	for (voice = 0; voice < self->numvoices; ++voice) {
		voice_init(&self->voices[voice], 0, 0);		
	}
	self->resamplingmethod = 2;
	self->defaultspeed = 1;
}

void dispose(Sampler* self)
{
	int voice;

	for (voice=0; voice < self->numvoices; ++voice) {
		voice_dispose(&self->voices[voice]);
	}
	buffer_dispose(&self->machine.inputs);
	buffer_dispose(&self->machine.outputs);
	machine_dispose(&self->machine);
}

void generateaudio(Sampler* self, Buffer* input, Buffer* output, int numsamples, int tracks)
{	
	int voice;

	for (voice = 0; voice < self->numvoices; ++voice) {
		voice_work(&self->voices[voice], output, numsamples);
	}
}

void seqtick(Sampler* self, int channel, const PatternEvent* event)
{
	Samples* samples = self->machine.machinecallback.samples(0);
	Sample* sample = SearchIntHashTable(&samples->container, event->inst);
	release_voices(self, channel);

	if (sample) {
		int voice;

		voice = unused_voice(self);
		if (voice != -1) {
			voice_init(&self->voices[voice], sample, channel);
			voice_seqtick(&self->voices[voice], event);			
		}
	}
}

int unused_voice(Sampler* self)
{
	int voice ;

	for (voice = 0; voice < self->numvoices; ++voice) {
		if (self->voices[voice].env.stage == ENV_OFF) {
			return voice;
		}
	}
	return -1;
}

void release_voices(Sampler* self, int channel)
{
	int voice;

	for (voice = 0; voice < self->numvoices; ++voice) {
		if (self->voices[voice].channel == channel) {
			voice_release(&self->voices[voice]);
		}		
	}
}

const CMachineInfo* info(Sampler* self)
{	
	return &MacInfo;
}

void parametertweak(Sampler* self, int param, int value)
{
	switch (param) {
		case 0: self->numvoices = value; break;
		case 1: self->resamplingmethod = value; break;
		case 2: self->defaultspeed = value; break;
		default:
		break;
	}
}

int describevalue(Sampler* self, char* txt, int const param, int const value)
{ 
	if (param == 1) {
		switch(value)
		{
			case 0:sprintf(txt,"Hold/Chip [Lowest quality]");return 1;break;
			case 1:sprintf(txt,"Linear [Low quality]");return 1;break;
			case 2:sprintf(txt,"Spline [Medium quality]");return 1;break;
			case 3:sprintf(txt,"32Tap Sinc [High Quality]");return 1;break;			
		}		
	} else
	if (param == 2) {
		switch(value)
		{
			case 0:sprintf(txt,"played by C3");return 1;break;
			case 1:sprintf(txt,"played by C4");return 1;break;		
		}
	}
	return 0;
}

int value(Sampler* self, int const param)
{	
	switch (param) {
		case 0: return self->numvoices; break;
		case 1: return self->resamplingmethod; break;
		case 2: return self->defaultspeed; break;
		default:
		break;
	}
	return 0;
}

void setvalue(Sampler* self, int const param, int const value)
{	
}

void voice_init(Voice* self, Sample* sample, int channel)
{
	self->position = sample_begin(self->sample);
	self->sample = sample;
	self->channel = channel;
	adsr_init(&self->env);
	adsr_init(&self->filterenv);
}

void voice_dispose(Voice* self)
{
}

void voice_seqtick(Voice* self, const PatternEvent* event)
{
	int baseC;	
	
	baseC = 60;	
	self->position = sample_begin(self->sample);
	double_setvalue(&self->position.speed,
		pow(2.0f,
			(event->note + self->sample->tune - baseC + 
				((float)self->sample->finetune * 0.01f)) / 12.0f) *
			((float)self->sample->samplerate / 44100));
	adsr_start(&self->env);
	adsr_start(&self->filterenv);
}

void voice_work(Voice* self, Buffer* output, int numsamples)
{	
	if (self->sample && self->env.stage != ENV_OFF) {
		float* left = buffer_at(output, 0);
		float* right = buffer_at(output, 1);

		float* src = self->sample->channels.samples[0];
		int i;		
		for (i = 0; i < numsamples; ++i) {
			left[i] += src[sampleiterator_frameposition(&self->position)] * self->env.value;
			adsr_tick(&self->env);
			adsr_tick(&self->filterenv);
			if (!sampleiterator_inc(&self->position)) {			
				voice_init(self, self->sample, self->channel);
				break;				
			}		
		}									
	}
}

void voice_release(Voice* self)
{
	adsr_release(&self->env);	
	adsr_release(&self->filterenv);
}

void voice_fastrelease(Voice* self)
{
	adsr_release(&self->env);	
	adsr_release(&self->filterenv);
}

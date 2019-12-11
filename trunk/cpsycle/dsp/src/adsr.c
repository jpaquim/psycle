// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "adsr.h"

static const psy_dsp_seconds_t defaultfastrelease = 0.003f;
static void computestep(ADSR* self, psy_dsp_amp_t dvalue,
	psy_dsp_seconds_t numseconds);

void envelopepoint_init(EnvelopePoint* self, 
	psy_dsp_seconds_t time, psy_dsp_amp_t value,
	psy_dsp_seconds_t mintime, psy_dsp_seconds_t maxtime,
	psy_dsp_amp_t minvalue, psy_dsp_amp_t maxvalue)
{
	self->time = time;
	self->value = value;
	self->mintime = mintime;
	self->maxtime = maxtime;
	self->minvalue = minvalue;
	self->maxvalue = maxvalue;
}

void adsr_settings_init(ADSRSettings* self, psy_dsp_seconds_t attack,
	psy_dsp_seconds_t decay, psy_dsp_percent_t sustain,
	psy_dsp_seconds_t release)
{	
	self->attack = attack;
	self->decay = decay;
	self->sustain = sustain;
	self->release = release;
	self->fastrelease = defaultfastrelease;
}

void adsr_settings_initdefault(ADSRSettings* self)
{
	self->attack = 0.005f;
	self->decay = 0.005f;
	self->sustain = 1.0f;
	self->release = 0.005f;
	self->fastrelease = defaultfastrelease;
}

psy_dsp_seconds_t adsr_settings_attack(ADSRSettings* self)
{
	return self->attack;
}

void adsr_settings_setattack(ADSRSettings* self, psy_dsp_seconds_t value)
{
	self->attack = value;
}

psy_dsp_seconds_t adsr_settings_decay(ADSRSettings* self)
{
	return self->decay;
}

void adsr_settings_setdecay(ADSRSettings* self, psy_dsp_seconds_t value)
{
	self->decay = value;
}

psy_dsp_percent_t adsr_settings_sustain(ADSRSettings* self)
{
	return self->sustain;
}

void adsr_settings_setsustain(ADSRSettings* self, psy_dsp_percent_t value)
{
	self->sustain = value;
}

psy_dsp_seconds_t adsr_settings_release(ADSRSettings* self)
{
	return self->release;
}

void adsr_settings_setrelease(ADSRSettings* self, psy_dsp_seconds_t value)
{
	self->release = value;
}


psy_dsp_seconds_t adsr_settings_fastrelease(ADSRSettings* self)
{
	return self->fastrelease;
}

void adsr_settings_setfastrelease(ADSRSettings* self, psy_dsp_seconds_t value)
{
	self->fastrelease = value;
}

void adsr_initdefault(ADSR* self, unsigned int samplerate)
{	
	self->stage = ENV_OFF;
	self->value = 0.f;
	self->step = 0.f;
	self->samplerate = samplerate;
	adsr_settings_initdefault(&self->settings);
}

void adsr_init(ADSR* self, const ADSRSettings* settings, unsigned int samplerate)
{
	self->stage = ENV_OFF;
	self->value = 0.f;
	self->step = 0.f;
	self->samplerate = samplerate;
	if (settings) {
		self->settings = *settings;
	} else {
		adsr_settings_initdefault(&self->settings);
	}
}

void adsr_reset(ADSR* self)
{
	self->stage = ENV_OFF;
	self->value = 0.f;
	self->step = 0.f;	
}

void adsr_setsamplerate(ADSR* self, unsigned int samplerate)
{
	self->samplerate = samplerate;	
}

void adsr_tick(ADSR* self)
{	
	switch (self->stage)
	{
		case ENV_ATTACK:
			self->value += self->step;
			if (self->value > 1.0f) {
				self->value = 1.0f;
				self->stage = ENV_DECAY;
				computestep(self, 1.0f - adsr_settings_sustain(&self->settings),
					adsr_settings_decay(&self->settings));
			}
		break;
		case ENV_DECAY:
			self->value -= self->step;
			if (self->value < adsr_settings_sustain(&self->settings)) {
				self->value = adsr_settings_sustain(&self->settings);
				self->stage = ENV_SUSTAIN;
			}
		break;
		case ENV_SUSTAIN:
			self->stage = ENV_SUSTAIN;
		break;
		case ENV_RELEASE:
		case ENV_FASTRELEASE:
			self->value -= self->step;			
			if (self->value <= 0) {
				self->value = 0;
				self->stage = ENV_OFF;
			}
		break;
		default:
		break;
	}	
}

void adsr_start(ADSR* self)
{	
	self->stage = ENV_ATTACK;
	self->value = 0.f;	
	computestep(self, 1.0f, adsr_settings_attack(&self->settings));	
}

void adsr_release(ADSR* self)
{
	if (self->stage != ENV_OFF) {
		self->stage = ENV_RELEASE;
		computestep(self, self->value, adsr_settings_release(&self->settings));
	}
}

void adsr_fastrelease(ADSR* self)
{
	if (self->stage != ENV_OFF) {
		self->stage = ENV_RELEASE;
		computestep(self, self->value, 
			adsr_settings_fastrelease(&self->settings));
	}
}

void computestep(ADSR* self, psy_dsp_amp_t dvalue,
	psy_dsp_seconds_t numseconds)
{
	self->step = dvalue / (numseconds * self->samplerate);
}

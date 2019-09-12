// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "adsr.h"

void adsr_envelopesettings_init(ADSREnvelopeSettings* self, float attack,
	float decay, float sustain, float release)
{
	self->attack = attack;
	self->decay = decay;
	self->sustain = sustain;
	self->release = release;
}

void adsr_envelopesettings_initdefault(ADSREnvelopeSettings* self)
{
	adsr_envelopesettings_init(self, 0.1f, 1.0f, 0.5f, 1.0f);	
}

void adsr_init(ADSR* self)
{	
	self->stage = ENV_OFF;
	self->value = 0.f;
	self->step = 0.f;
	self->samplerate = 44100;
	adsr_envelopesettings_initdefault(&self->settings);		
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
				self->step = ((1.0f - self->settings.sustain)/self->settings.decay)*(44100.0f/self->samplerate);
			}
		break;
		case ENV_DECAY:
			self->value -= self->step;
			if (self->value < self->settings.sustain) {
				self->value = self->settings.sustain;
				self->stage = ENV_RELEASE; //SUSTAIN;
			}
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
	self->value = 0.f;
	self->step = 1.0f / self->settings.attack * 1.0f/44100;	
	self->stage = ENV_ATTACK;
}

void adsr_release(ADSR* self)
{
	if (self->stage != ENV_OFF) {
		self->stage = ENV_RELEASE;
	}
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sampleiterator.h"
#include "sample.h"

#include <operations.h>

#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
#include "../../detail/trace.h"
#endif

#define REFILLBUFFERSIZE 64 * 3

static INLINE psy_audio_SampleLoopType psy_audio_sampleiterator_looptype(psy_audio_SampleIterator* self)
{
	return self->sample->loop.type;
}

static INLINE uint32_t psy_audio_sampleiterator_loopstart(psy_audio_SampleIterator* self)
{
	return self->sample->loop.start;
}

static INLINE uint32_t psy_audio_sampleiterator_loopend(psy_audio_SampleIterator* self)
{
	return self->sample->loop.end;
}

static INLINE psy_audio_SampleLoopType psy_audio_sampleiterator_sustainlooptype(psy_audio_SampleIterator* self)
{
	return self->sample->sustainloop.type;
}

static INLINE uint32_t psy_audio_sampleiterator_sustainloopstart(psy_audio_SampleIterator* self)
{
	return self->sample->sustainloop.start;
}

static INLINE uint32_t psy_audio_sampleiterator_sustainloopend(psy_audio_SampleIterator* self)
{
	return self->sample->sustainloop.end;
}

static INLINE uint32_t psy_audio_sampleiterator_length(psy_audio_SampleIterator* self)
{
	return self->sample->numframes;
}

static void psy_audio_sampleiterator_psy_audio_sampleiterator_initloop(
	psy_audio_SampleIterator*, psy_audio_Sample*);
static void psy_audio_sampleiterator_initbuffer(psy_audio_SampleIterator*);
static void psy_audio_sampleiterator_disposebuffer(psy_audio_SampleIterator*);
static void psy_audio_sampleiterator_refillbuffers(psy_audio_SampleIterator*,
	bool released/*=false*/);
static void psy_audio_sampleiterator_refillbuffer(psy_audio_SampleIterator*,
	psy_dsp_amp_t buffer[192], psy_dsp_amp_t* data, bool released);

void psy_audio_sampleiterator_init(psy_audio_SampleIterator* self,
	psy_audio_Sample* sample, ResamplerType quality)
{
	self->sample = sample;
	self->looped = FALSE;
	self->playing = FALSE;
	self->forward = 1;
	self->pos.QuadPart = 0;
	self->speed = (int64_t) 4294967296.0f;
	self->speedinternal = self->speed;	
	psy_audio_sampleiterator_psy_audio_sampleiterator_initloop(self, sample);
	//psy_audio_buffer_init(&self->buffer, 2);
	psy_audio_sampleiterator_initbuffer(self);
	psy_dsp_multiresampler_init(&self->resampler, quality);	
}

void psy_audio_sampleiterator_initbuffer(psy_audio_SampleIterator* self)
{
	// uintptr_t channel;
	
	//for (channel = 0; channel < self->buffer.numchannels; ++channel) {
	//	self->buffer.samples[channel] = dsp.memory_alloc(
	//		size, sizeof(psy_dsp_amp_t));
	//}
	//psy_audio_buffer_clearsamples(&self->buffer, size);
	self->lBuffer = dsp.memory_alloc(REFILLBUFFERSIZE, sizeof(psy_dsp_amp_t));
	self->rBuffer = dsp.memory_alloc(REFILLBUFFERSIZE, sizeof(psy_dsp_amp_t));
	if (self->sample) {
		psy_audio_sampleiterator_refillbuffers(self, FALSE);
	}
}

void psy_audio_sampleiterator_dispose(psy_audio_SampleIterator* self)
{
	//uintptr_t channel;
	//uintptr_t size;

	//for (channel = 0; channel < self->buffer.numchannels; ++channel) {
//		dsp.memory_dealloc(self->buffer.samples[channel]);
	//}
	//psy_audio_buffer_dispose(&self->buffer);
	dsp.memory_dealloc(self->lBuffer);
	dsp.memory_dealloc(self->rBuffer);
	psy_dsp_resampler_dispose(psy_dsp_multiresampler_base(&self->resampler));	
}

void psy_audio_sampleiterator_psy_audio_sampleiterator_initloop(
	psy_audio_SampleIterator* self, psy_audio_Sample* sample)
{
	if (sample) {
		if (psy_audio_sampleiterator_sustainlooptype(self)
			!= psy_audio_SAMPLE_LOOP_DO_NOT) {
			self->currentlooptype = psy_audio_sampleiterator_sustainlooptype(self);
			self->currentloopstart =
				psy_audio_sampleiterator_sustainloopstart(self);
			self->currentloopend = psy_audio_sampleiterator_sustainloopend(self);

		} else
			if (psy_audio_sampleiterator_looptype(self)
				!= psy_audio_SAMPLE_LOOP_DO_NOT) {
				self->currentlooptype = psy_audio_sampleiterator_looptype(self);
				self->currentloopstart = psy_audio_sampleiterator_loopstart(self);
				self->currentloopend = psy_audio_sampleiterator_loopend(self);
			} else {
				// No loop is considered a loop that stops at the end.
				// This way, it is not needed to check if a loop is enabled when
				// checking if end loop is reached.
				self->currentlooptype = psy_audio_SAMPLE_LOOP_DO_NOT;
				self->currentloopstart = 0;
				self->currentloopend = psy_audio_sampleiterator_length(self);
			}
		if (self->currentloopstart >= self->currentloopend) {
			self->currentloopstart = self->currentloopend - 1;
		}		
	} else {
		self->currentlooptype = psy_audio_SAMPLE_LOOP_DO_NOT;
		self->currentloopstart = 0;
		self->currentloopend = 0;
	}
	self->currentloopdirection = psy_audio_LOOPDIRECTION_FORWARD;
	self->speedinternal = self->speed;
}

psy_audio_SampleIterator* psy_audio_sampleiterator_alloc(void)
{
	return malloc(sizeof(psy_audio_SampleIterator));
}

psy_audio_SampleIterator* psy_audio_sampleiterator_allocinit(psy_audio_Sample*
	sample)
{
	psy_audio_SampleIterator* rv;

	rv = psy_audio_sampleiterator_alloc();
	if (rv) {		
		psy_audio_sampleiterator_init(rv, sample, RESAMPLERTYPE_LINEAR);
	}
	return rv;
}

intptr_t psy_audio_sampleiterator_inc(psy_audio_SampleIterator* self)
{			
	intptr_t old;
	intptr_t diff;
	
	old = self->pos.HighPart;
	self->pos.QuadPart += self->speedinternal;
	diff = (intptr_t)(self->pos.HighPart) - old;	
	// Note: pos might be poiting at an erroneous place here. (like in looped
	// samples). Let Postwork take care of this.	
	return diff;
}

psy_dsp_amp_t psy_audio_sampleiterator_work(psy_audio_SampleIterator* self, uintptr_t channel)
{
	psy_dsp_amp_t* src;

	src = psy_audio_buffer_at(&self->sample->channels, channel);
	return psy_dsp_resampler_work_float_unchecked(
		psy_dsp_multiresampler_base(&self->resampler),
		(channel == 0) ? self->m_pL : self->m_pR,
		self->pos.LowPart);
}

void psy_audio_sampleiterator_postwork(psy_audio_SampleIterator* self)
{
	int32_t newIntPos = (int32_t)(self->pos.HighPart);
	if (psy_audio_sampleiterator_currentloopdirection(self) ==
				psy_audio_LOOPDIRECTION_FORWARD &&
			newIntPos >= self->currentloopend) {
		self->looped = TRUE;
		switch (self->currentlooptype) {
		case psy_audio_SAMPLE_LOOP_NORMAL:
			do {
				self->pos.HighPart = self->currentloopstart + (newIntPos -
					self->currentloopend);
				newIntPos = (int32_t)(self->pos.HighPart);
				// For very small loops, the while is necessary
			} while ((int32_t)(self->pos.HighPart) > self->currentloopend);
			break;
		case psy_audio_SAMPLE_LOOP_BIDI:
			do {
				self->pos.HighPart = self->currentloopend - (newIntPos - self->currentloopend);
				newIntPos = (int32_t)(self->pos.HighPart);
				// For very small loops, the while is necessary
			} while ((int32_t)(self->pos.HighPart) > self->currentloopend);
			self->pos.LowPart = 4294967295 - self->pos.LowPart;
			self->currentloopdirection = psy_audio_LOOPDIRECTION_BACKWARD;
			self->speedinternal = -1 * self->speed;
			break;
		case psy_audio_SAMPLE_LOOP_DO_NOT: // fallthrough
		default:
			psy_audio_sampleiterator_stop(self);
			break;
		}
	} else
	if (psy_audio_sampleiterator_currentloopdirection(self) ==
				psy_audio_LOOPDIRECTION_BACKWARD &&
			newIntPos <= self->currentloopstart) {
		switch (self->currentlooptype)
		{
		case psy_audio_SAMPLE_LOOP_NORMAL: // fallthrough
		case psy_audio_SAMPLE_LOOP_BIDI:
			do {
				int32_t newIntPos = (int32_t)(self->pos.HighPart);
				self->pos.HighPart = self->currentloopstart +
					(self->currentloopstart - newIntPos);
				//For very small loops, the while is necessary
			} while ((int32_t)(self->pos.HighPart) < self->currentloopstart);
			self->pos.LowPart = 4294967295 - self->pos.LowPart;
			self->currentloopdirection = psy_audio_LOOPDIRECTION_FORWARD;
			self->speedinternal = self->speed;
			break;
		case psy_audio_SAMPLE_LOOP_DO_NOT://fallthrough
		default:
			// Playing(false);
			break;
		}
	}
#ifndef NDEBUG
	if ((int32_t)(self->pos.HighPart) >= self->currentloopend + 17) {
//		TRACE("430: highpart > loopend+17 bug triggered!\n");
	}
#endif
}

void psy_audio_sampleiterator_dooffset(psy_audio_SampleIterator* self,
	uint8_t offset)
{
	uint64_t w_offset;

	w_offset = offset * self->sample->numframes;
	w_offset = w_offset << 24;
	self->pos.QuadPart += w_offset;
	if (self->pos.HighPart > self->sample->numframes) {
		double_setvalue(&self->pos, 0.0);
	}
}

void psy_audio_sampleiterator_refillbuffers(psy_audio_SampleIterator* self,
	bool released/*=false*/)
{
	psy_audio_sampleiterator_refillbuffer(self, self->lBuffer, self->sample->channels.samples[0], released);
	if (self->sample->channels.numchannels > 1) {
		psy_audio_sampleiterator_refillbuffer(self, self->rBuffer, self->sample->channels.samples[1], released);
	}
}

void psy_audio_sampleiterator_refillbuffer(psy_audio_SampleIterator* self,
	psy_dsp_amp_t buffer[192], psy_dsp_amp_t* data, bool released)
{
	//These values are for the max size of sinc resampler (which suits the rest).
	const uint32_t presamples = 15;
	const uint32_t postsamples = 16;
	const uint32_t totalsamples = presamples + postsamples + 1; //pre+post+current
	const uint32_t secbegin = 64;	// start of second window = totalsamples*2
	const uint32_t thirdbegin = 128; // start of third window = secbegin+(totalsamples*2)

	psy_audio_SampleLoopType looptype;
	uint32_t loopstart;
	uint32_t loopend;

	if (!released && psy_audio_sampleiterator_sustainlooptype(self) != psy_audio_SAMPLE_LOOP_DO_NOT) {
		looptype = psy_audio_sampleiterator_sustainlooptype(self);
		loopstart = psy_audio_sampleiterator_sustainloopstart(self);
		loopend = psy_audio_sampleiterator_sustainloopend(self);
	} else {
		looptype = psy_audio_sampleiterator_looptype(self);
		loopstart = psy_audio_sampleiterator_loopstart(self);
		loopend = psy_audio_sampleiterator_loopend(self);
	}
	//Begin
	memset(buffer, 0, REFILLBUFFERSIZE * sizeof(psy_dsp_amp_t));
	memcpy(buffer + presamples, data, min(psy_audio_sampleiterator_length(self), totalsamples) * sizeof(psy_dsp_amp_t));
	if (looptype == psy_audio_SAMPLE_LOOP_DO_NOT) {
		//End
		if (psy_audio_sampleiterator_length(self) < totalsamples) {
			memset(buffer + secbegin, 0, totalsamples + postsamples);
			memcpy(buffer + secbegin + totalsamples - psy_audio_sampleiterator_length(self),
				data, psy_audio_sampleiterator_length(self) * sizeof(psy_dsp_amp_t));
		} else {
			memcpy(buffer + secbegin, data + psy_audio_sampleiterator_length(self) - totalsamples, totalsamples * sizeof(psy_dsp_amp_t));
			memset(buffer + secbegin + totalsamples, 0, postsamples);
		}
	} else if (looptype == psy_audio_SAMPLE_LOOP_NORMAL) {
		//Forward only loop.
		if (loopend - loopstart < totalsamples) {
			int startpos = loopstart;
			int endpos = loopend;
			int i;
			for (i = 0; i < totalsamples; i++) {
				buffer[secbegin + totalsamples - i] = data[endpos];
				buffer[secbegin + totalsamples + i] = data[startpos];
				endpos--;
				startpos++;
				if (endpos < loopstart) {
					endpos = loopend;
					startpos = loopstart;
				}
			}
		} else {
			memcpy(buffer + secbegin, data + loopend - totalsamples, totalsamples * sizeof(psy_dsp_amp_t));
			memcpy(buffer + secbegin + totalsamples, data + loopstart, totalsamples * sizeof(psy_dsp_amp_t));
		}
	} else if (looptype == psy_audio_SAMPLE_LOOP_BIDI) {
		if (loopend - loopstart < totalsamples) {
			//Ping pong loop (end and start).
			int pos = loopend;
			bool forward = FALSE;
			int i;

			for (i = 0; i < totalsamples; i++) {
				buffer[secbegin + totalsamples - i] = data[pos];
				buffer[secbegin + totalsamples + i] = data[pos];
				if (forward) {
					pos++;
					if (pos > loopend) {
						pos = loopend - 1;
						forward = FALSE;
					}
				} else {
					pos--;
					if (pos < loopstart) {
						pos = loopstart + 1;
						forward = TRUE;
					}
				}
			}
		} else {
			int i;

			//Ping pong loop (end and start).
			memcpy(buffer + secbegin, data + loopend - totalsamples, totalsamples * sizeof(psy_dsp_amp_t));
			memcpy(buffer + thirdbegin + totalsamples, data + loopstart, totalsamples * sizeof(psy_dsp_amp_t));
			for (i = 0; i < totalsamples; i++) {
				buffer[secbegin + totalsamples + i] = data[loopend - i - 1];
				buffer[thirdbegin + i] = data[loopstart + totalsamples - i];
			}
		}
	}
}

int psy_audio_sampleiterator_prework(psy_audio_SampleIterator* self, int numSamples, bool released)
{
	// WorkFunction* pWork
	// *pWork = (IsStereo()) ? WorkStereoStatic : WorkMonoStatic;

	//These values are for the max size of the sinc resampler (which suits the rest).
	const int presamples = 15;
	const int postsamples = 16;
	const int totalsamples = presamples + postsamples + 1; //pre+post+current
	const int secbegin = 64;	// start of second window = totalsamples*2
	const int thirdbegin = 128; // start of third window = secbegin+(totalsamples*2)

	int32_t max;
	Double amount;
	int32_t pos;

	amount.QuadPart = self->pos.QuadPart + self->speedinternal * numSamples;
	pos = self->pos.HighPart;
	//TRACE("RealPos %d\n",pos);
	if (psy_audio_sampleiterator_currentloopdirection(self) == psy_audio_LOOPDIRECTION_FORWARD) {
		if (pos < presamples && !self->looped) {
			self->m_pL = &self->lBuffer[presamples + pos];
			self->m_pR = &self->rBuffer[presamples + pos];
			max = presamples - pos;
			//TRACE("Begin buffer at pos %d for samples %d\n" ,pos+presamples , max);
		} else if (pos + postsamples >= self->currentloopend && pos < self->currentloopend + presamples) {
			self->m_pL = &self->lBuffer[secbegin + (totalsamples + pos - self->currentloopend)];
			self->m_pR = &self->rBuffer[secbegin + (totalsamples + pos - self->currentloopend)];
			if ((released || psy_audio_sampleiterator_sustainlooptype(self) == psy_audio_SAMPLE_LOOP_DO_NOT)
				&& psy_audio_sampleiterator_looptype(self) == psy_audio_SAMPLE_LOOP_DO_NOT) {
				max = self->currentloopend - pos;
				//TRACE("End buffer at pos %d for samples %d\n", secbegin+(pos+totalsamples-m_CurrentLoopEnd) , max);
			} else {
				max = presamples + self->currentloopend - pos;
				//TRACE("forward-loop buffer at pos %d for samples %d\n" , secbegin+(pos+totalsamples-m_CurrentLoopEnd) , max);
			}
		} else if (self->looped && pos + postsamples >= self->currentloopstart && pos < self->currentloopstart + presamples) {
			if ((!released && psy_audio_sampleiterator_sustainlooptype(self) == psy_audio_SAMPLE_LOOP_NORMAL)
				|| (psy_audio_sampleiterator_sustainlooptype(self) == psy_audio_SAMPLE_LOOP_DO_NOT && self->sample->loop.type == psy_audio_SAMPLE_LOOP_NORMAL)) {
				self->m_pL = &self->lBuffer[secbegin + (totalsamples + pos - self->currentloopstart)];
				self->m_pR = &self->rBuffer[secbegin + (totalsamples + pos - self->currentloopstart)];
			} else {
				self->m_pL = &self->lBuffer[thirdbegin + (totalsamples + pos - self->currentloopstart)];
				self->m_pR = &self->rBuffer[thirdbegin + (totalsamples + pos - self->currentloopstart)];
			}
			max = presamples + self->currentloopstart - pos;
			//TRACE("forward-loop buffer at pos %d for samples %d\n" , secbegin+(pos+totalsamples-m_CurrentLoopEnd) , max);
		} else {
			self->m_pL = (psy_dsp_amp_t*)(self->sample->channels.samples[0] + pos);
			self->m_pR = (psy_dsp_amp_t*)(self->sample->channels.samples[1] + pos);
			if ((int32_t)(amount.HighPart) + postsamples < self->currentloopend) {
				return numSamples;
			}
			max = self->currentloopend - (int32_t)(self->pos.HighPart) - postsamples;
			//TRACE("sample buffer at pos %d for samples %d\n" , pos , max);
		}
		if (max < 0) {
			//Disallow negative values. (Generally, it indicates a bug in calculations)
			max = 1;
			// TRACE("336: max<0 bug triggered!\n");
		}
		amount.HighPart = (uint32_t)(max);
		amount.LowPart = 0;
		amount.QuadPart -= self->pos.LowPart;
	} else if (psy_audio_sampleiterator_currentloopdirection(self) == psy_audio_LOOPDIRECTION_BACKWARD) {
		if (pos - presamples <= self->currentloopstart) {
			self->m_pL = &self->lBuffer[thirdbegin + (totalsamples + pos - self->currentloopstart)];
			self->m_pR = &self->rBuffer[thirdbegin + (totalsamples + pos - self->currentloopstart)];
			max = pos - self->currentloopstart;
			//TRACE("backward-loop buffer at pos %d for samples %d\n" ,thirdbegin+(pos+totalsamples-m_CurrentLoopStart) , max);
		} else {
			self->m_pL = (psy_dsp_amp_t*)(self->sample->channels.samples[0] + pos);
			self->m_pR = (psy_dsp_amp_t*)(self->sample->channels.samples[1] + pos);
			if ((int32_t)(amount.HighPart) - presamples >= self->currentloopstart) {
				return numSamples;
			}
			max = (int32_t)(self->pos.HighPart) - self->currentloopstart - postsamples;
			//TRACE("sample buffer (backwards) at pos %d for samples %d\n",  pos , max);
		}
		if (max < 0) {
			//Disallow negative values. (Generally, it indicates a bug in calculations)
			max = 1;
			// TRACE("361: max<0 bug triggered!\n");
		}
		amount.HighPart = (uint32_t)(max);
		amount.LowPart = self->pos.LowPart;
	}
#ifndef NDEBUG
	// if (*m_pL != *(m_pWave->pWaveDataL() + pos) && pos < Length()) {
		// TRACE("368 ERROR. Samples differ! %d - %d (%d,%d)\n", *m_pL, *(m_pWave->pWaveDataL() + pos), (m_pL - lBuffer), pos);
	// }
#endif
	if (psy_audio_sampleiterator_speed(self) != 0) {
		amount.QuadPart /= psy_audio_sampleiterator_speed(self);
	} else {
		amount.QuadPart = 0;
	}
	return amount.LowPart + 1;
}

void psy_audio_sampleiterator_setquality(psy_audio_SampleIterator* self,
	ResamplerType quality)
{
	psy_dsp_multiresampler_settype(&self->resampler, quality);
}

ResamplerType psy_audio_sampleiterator_quality(psy_audio_SampleIterator* self)
{
	return psy_dsp_multiresampler_type(&self->resampler);	
}

void psy_audio_sampleiterator_setsample(psy_audio_SampleIterator* self,
	psy_audio_Sample* sample)
{
	self->sample = sample;
	psy_audio_sampleiterator_psy_audio_sampleiterator_initloop(self, sample);
	self->pos.QuadPart = 0;
	if (self->sample) {
		psy_audio_sampleiterator_refillbuffers(self, FALSE);
	}
}
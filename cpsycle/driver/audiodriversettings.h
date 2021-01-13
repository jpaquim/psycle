// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../detail/os.h"
#include "../detail/psydef.h"

#include "../dsp/src/dsptypes.h"

#include <assert.h>

#if !defined(PSY_AUDIODRIVERSETTINGS_H)
#define PSY_AUDIODRIVERSETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

// holds the info about sample rate, bit depth, etc

#define MAX_SAMPLES_WORKFN		65536

typedef enum {
	psy_AUDIODRIVERCHANNELMODE_NO_MODE = -1,
	psy_AUDIODRIVERCHANNELMODE_MONO_MIX = 0,
	psy_AUDIODRIVERCHANNELMODE_MONO_LEFT,
	psy_AUDIODRIVERCHANNELMODE_MONO_RIGHT,
	psy_AUDIODRIVERCHANNELMODE_STEREO
} psy_AudioDriverChannelMode;

typedef struct {
	psy_dsp_big_hz_t samplespersec_;
	psy_AudioDriverChannelMode channelmode_;
	bool dither_;
	uintptr_t validbitdepth_;
	uintptr_t bitdepth_;
	uintptr_t framebytes_;
	uintptr_t blockframes_;
	uintptr_t blockcount_;
} psy_AudioDriverSettings;

INLINE psy_AudioDriverChannelMode psy_audiodriversettings_channelmode(
	psy_AudioDriverSettings* self)
{
	return self->channelmode_;
}

INLINE uintptr_t psy_audiodriversettings_numchannels(
	psy_AudioDriverSettings* self)
{
	return (self->channelmode_ == psy_AUDIODRIVERCHANNELMODE_STEREO) ? 2 : 1;
}

INLINE void psy_audiodriversettings_setchannelmode(
	psy_AudioDriverSettings* self, psy_AudioDriverChannelMode mode)
{
	self->channelmode_ = mode;
}

INLINE psy_dsp_big_hz_t psy_audiodriversettings_samplespersec(
	psy_AudioDriverSettings* self)
{
	return self->samplespersec_;
}

INLINE void psy_audiodriversettings_setsamplespersec(
	psy_AudioDriverSettings* self, psy_dsp_big_hz_t value)
{
	if (value <= 22050) {
		value = 22050;
	} else if (value <= 44100) {
		value = 44100;
	} else if (value <= 48000) {
		value = 48000;
	} else {
		value = 96000;
	}
	self->samplespersec_ = value;
}

// validbitdepth is the actual number of bits used.
// Valid values (bitdepth/validbitdepth)
// 8/8, 16/16 , 32/24 (int) and 32/32 (float)
INLINE uintptr_t psy_audiodriversettings_validbitdepth(
	psy_AudioDriverSettings* self)
{
	return self->validbitdepth_;
}

// bitdepth is the container size. (e.g. valid 24 bits, size 32 bits)
INLINE uintptr_t psy_audiodriversettings_bitdepth(
	psy_AudioDriverSettings* self)
{
	return self->bitdepth_;
}

// Valid values (bitdepth/validbitdepth)
// 8/8, 16/16 , 32/24 (int) and 32/32 (float)
INLINE void psy_audiodriversettings_setvalidbitdepth(
	psy_AudioDriverSettings* self, uintptr_t validbitdepth)
{
	validbitdepth = (validbitdepth / 8) * 8;
	self->validbitdepth_ = validbitdepth;
	self->bitdepth_ = (validbitdepth == 24) ? 32 : validbitdepth;
	self->framebytes_ = 
		(self->channelmode_ == psy_AUDIODRIVERCHANNELMODE_STEREO)
		? (self->bitdepth_ >> 2)
		: (self->bitdepth_ >> 3);
}

INLINE uintptr_t psy_audiodriversettings_blockcount(
	psy_AudioDriverSettings* self)
{
	return self->blockcount_;
}

INLINE void psy_audiodriversettings_setblockcount(
	psy_AudioDriverSettings* self, uintptr_t blockcount)
{
	self->blockcount_ = blockcount;
}

///\name getter/setter for the audio block size (in samples comprising all channels)	
INLINE uintptr_t psy_audiodriversettings_blockframes(
	psy_AudioDriverSettings* self)
{
	return self->blockframes_;
}

INLINE void psy_audiodriversettings_setblockframes(
	psy_AudioDriverSettings* self, uintptr_t value)
{
	self->blockframes_ = value;
	assert(self->blockframes_ < MAX_SAMPLES_WORKFN);
}

///\name getter for the audio block size (in bytes)
INLINE uintptr_t psy_audiodriversettings_blockbytes(
	psy_AudioDriverSettings* self)
{
	return self->blockframes_ * self->framebytes_;
}

/// getter for the whole buffer size (in bytes).
INLINE uintptr_t psy_audiodriversettings_totalbufferbytes(
	psy_AudioDriverSettings* self)
{
	return self->blockframes_ * self->framebytes_ * self->blockcount_;
}

INLINE uintptr_t psy_audiodriversettings_framebytes(
	psy_AudioDriverSettings* self)
{
	return self->framebytes_;
}

INLINE void psy_audiodriversettings_init(psy_AudioDriverSettings* self)
{
	self->samplespersec_ = 44100;
	self->channelmode_ = psy_AUDIODRIVERCHANNELMODE_STEREO;
	self->blockcount_ = 6;
	self->bitdepth_ = 16;
	self->blockframes_ = 2048;
	psy_audiodriversettings_setvalidbitdepth(self, 16);
}

#ifdef __cplusplus
}
#endif

#endif /* PSY_AUDIODRIVERSETTINGS_H */

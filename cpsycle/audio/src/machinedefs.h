/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_MACHINEDEFS_H
#define psy_audio_MACHINEDEFS_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
	psy_audio_SUPPORTS_INSTRUMENTS = 128
};

#define psy_audio_MACH_AUTO_POSITION -1.0

typedef enum {
	psy_audio_MACHMODE_GENERATOR = 0,
	psy_audio_MACHMODE_FX = 1,
	psy_audio_MACHMODE_MASTER = 2,
	psy_audio_MACHMODE_UNDEFINED = 3,
} psy_audio_MachineMode;

typedef enum  {
	psy_audio_UNDEFINED = -1,
	psy_audio_MASTER = 0,
	psy_audio_SINE = 1, /*< now a plugin */
	psy_audio_DIST = 2, /*< now a plugin */
	psy_audio_SAMPLER = 3,
	psy_audio_DELAY = 4, /*< now a plugin */
	psy_audio_2PFILTER = 5, /*< now a plugin */
	psy_audio_GAIN = 6, /*/< now a plugin */
	psy_audio_FLANGER = 7, /*< now a plugin */
	psy_audio_PLUGIN = 8,
	psy_audio_VST = 9,
	psy_audio_VSTFX = 10,
	psy_audio_SCOPE = 11, /*< Test machine. removed */
	psy_audio_XMSAMPLER = 12,
	psy_audio_DUPLICATOR = 13,
	psy_audio_MIXER = 14,
	psy_audio_RECORDER = 15,
	psy_audio_DUPLICATOR2 = 16,
	psy_audio_LUA = 17,
	psy_audio_LADSPA = 18,
	psy_audio_VIRTUALGENERATOR = 19,
	psy_audio_DUMMY = 255
} psy_audio_MachineType;

#define MACHINE_USES_INSTRUMENTS 64

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINEDEFS_H */

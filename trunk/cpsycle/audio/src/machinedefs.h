// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEDEFS_H)
#define MACHINEDEFS_H

typedef enum {
	MACHMODE_GENERATOR = 0,
	MACHMODE_FX = 1,
	MACHMODE_MASTER = 2,
} psy_audio_MachineMode;

typedef enum  {
	MACH_UNDEFINED = -1,
	MACH_MASTER = 0,
	MACH_SINE = 1, ///< now a plugin
	MACH_DIST = 2, ///< now a plugin
	MACH_SAMPLER = 3,
	MACH_DELAY = 4, ///< now a plugin
	MACH_2PFILTER = 5, ///< now a plugin
	MACH_GAIN = 6, ///< now a plugin
	MACH_FLANGER = 7, ///< now a plugin
	MACH_PLUGIN = 8,
	MACH_VST = 9,
	MACH_VSTFX = 10,
	MACH_SCOPE = 11, ///< Test machine. removed
	MACH_XMSAMPLER = 12,
	MACH_DUPLICATOR = 13,
	MACH_MIXER = 14,
	MACH_RECORDER = 15,
	MACH_DUPLICATOR2 = 16,
	MACH_LUA = 17,
	MACH_LADSPA = 18,
	MACH_DUMMY = 255
} MachineType;

#define MACHINE_USES_INSTRUMENTS 64

#endif

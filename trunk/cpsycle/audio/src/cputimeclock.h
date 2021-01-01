// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_CPUTIMECLOCK_H
#define psy_audio_CPUTIMECLOCK_H

#include "../../detail/os.h"
#include "../../detail/psydef.h"

#include <time.h>

typedef struct psy_audio_CpuTimeClock {
	clock_t start, end;
	double last_perc;
	double perc;
} psy_audio_CpuTimeClock;

INLINE void psy_audio_cputimeclock_init(psy_audio_CpuTimeClock* self)
{
	self->last_perc = 0.0;
	self->perc = 0.0;
}

INLINE void psy_audio_cputimeclock_measure(psy_audio_CpuTimeClock* self)
{
	self->start = clock();
}

INLINE void psy_audio_cputimeclock_stop(psy_audio_CpuTimeClock* self)
{
	self->end = clock();
}

INLINE void psy_audio_cputimeclock_update(psy_audio_CpuTimeClock* self, uintptr_t amount, uintptr_t samplerate)
{
	if (amount > 0) {
		double cpu_time_used;
		double max_time;
		double currperc;

		cpu_time_used = ((double)(self->end - self->start)) / CLOCKS_PER_SEC;
		max_time = amount / (double)samplerate;
		currperc = cpu_time_used / max_time;
		self->perc = (self->last_perc + currperc) / 2;
		self->last_perc = self->perc;
	}
}

#endif /* psy_audio_CPUTIMECLOCK_H */

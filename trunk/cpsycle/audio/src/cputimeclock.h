// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_CPUTIMECLOCK_H
#define psy_audio_CPUTIMECLOCK_H

#include "../../detail/os.h"
#include "../../detail/psydef.h"

#include <time.h>

typedef struct psy_audio_CpuTimeClock {
	clock_t start, end;
	double cpu_time_used;
	unsigned int count;
} psy_audio_CpuTimeClock;

INLINE void psy_audio_cputimeclock_init(psy_audio_CpuTimeClock* self)
{
	self->count = 0;
	self->start = 0;
	self->end = 0;
	self->cpu_time_used = 0.0;
}

INLINE void psy_audio_cputimeclock_begin(psy_audio_CpuTimeClock* self)
{
	self->start = clock();
}

INLINE void psy_audio_cputimeclock_end(psy_audio_CpuTimeClock* self)
{
	self->end = clock();
	self->cpu_time_used = ((double)self->end - (double)self->start) /
		CLOCKS_PER_SEC;
}

INLINE void psy_audio_cputimeclock_reset(psy_audio_CpuTimeClock* self)
{
	self->cpu_time_used = 0.0;
}

INLINE double psy_audio_cputimeclock_cputime(psy_audio_CpuTimeClock* self)
{
	return self->cpu_time_used;
}

INLINE unsigned int psy_audio_cputimeclock_cputime_count(
	psy_audio_CpuTimeClock* self)
{
	return self->count;
}

#endif /* psy_audio_CPUTIMECLOCK_H */

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_LOCK_H
#define psy_audio_LOCK_H

#include "../../detail/os.h"

#if defined DIVERSALIS__OS__POSIX
#include <pthread.h>
#elif defined DIVERSALIS__OS__MICROSOFT
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_Lock {
#if defined DIVERSALIS__OS__POSIX
	pthread_mutex_t lock;
#elif defined DIVERSALIS__OS__MICROSOFT
	CRITICAL_SECTION lock;
#endif
	int disabled;
} psy_audio_Lock;

void psy_audio_lock_init(psy_audio_Lock*);
void psy_audio_lock_dispose(psy_audio_Lock*);
psy_audio_Lock* psy_audio_lock_alloc(void);
psy_audio_Lock* psy_audio_lock_allocinit(void);

void psy_audio_lock_enable(psy_audio_Lock*);
void psy_audio_lock_disable(psy_audio_Lock*);
void psy_audio_lock_enter(psy_audio_Lock*);
void psy_audio_lock_leave(psy_audio_Lock*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_EXCLUSIVELOCK_H */

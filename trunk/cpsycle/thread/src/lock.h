/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_LOCK_H
#define psy_LOCK_H

#include "../../detail/os.h"

#if defined DIVERSALIS__OS__POSIX
#include <pthread.h>
#elif defined DIVERSALIS__OS__MICROSOFT
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_Lock {
#if defined DIVERSALIS__OS__POSIX
	pthread_mutex_t lock;
#elif defined DIVERSALIS__OS__MICROSOFT
	CRITICAL_SECTION lock;
#endif
	int disabled;
} psy_Lock;

void psy_lock_init(psy_Lock*);
void psy_lock_dispose(psy_Lock*);
psy_Lock* psy_lock_alloc(void);
psy_Lock* psy_lock_allocinit(void);

void psy_lock_enable(psy_Lock*);
void psy_lock_disable(psy_Lock*);
void psy_lock_enter(psy_Lock*);
void psy_lock_leave(psy_Lock*);

#ifdef __cplusplus
}
#endif

#endif /* psy_LOCK_H */

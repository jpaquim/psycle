/* This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_TIMERS_H
#define psy_TIMERS_H

/* container */
#include <list.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*psy_fp_timerwork)(void*, uintptr_t);

typedef struct psy_TimerTask {
	uintptr_t id;	
	void* context;
	psy_fp_timerwork timerwork;
	psy_Signal* timersignal;
	uintptr_t customid;
	uintptr_t interval;
	uintptr_t counter;
	bool expired;	
} psy_TimerTask;

void psy_timertask_init(psy_TimerTask*, uintptr_t id,
	void* context, psy_fp_timerwork, psy_Signal* timersignal,
	uintptr_t customid, uintptr_t interval);

psy_TimerTask* psy_timertask_alloc(void);
psy_TimerTask* psy_timertask_allocinit(uintptr_t id,
	void* context, psy_fp_timerwork, psy_Signal* timersignal,
	uintptr_t customid, uintptr_t interval);

typedef struct psy_Timers {	
	uintptr_t counter;
	psy_List* tasks;	
} psy_Timers;

void psy_timers_init(psy_Timers*);
void psy_timers_dispose(psy_Timers*);

void psy_timers_tick(psy_Timers*);
void psy_timers_addtimer(psy_Timers*, uintptr_t id, void* context,
	psy_fp_timerwork timerwork, psy_Signal* timersignal,
	uintptr_t customid, uintptr_t interval);
void psy_timers_removetimer(psy_Timers*, uintptr_t id, uintptr_t customid);

uintptr_t psy_gettime_ms(void);

#ifdef __cplusplus
}
#endif

#endif /* psy_TIMERS_H */

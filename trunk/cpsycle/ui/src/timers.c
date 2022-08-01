/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "timers.h"
/* std */
#include <stdlib.h>

/* time calculation */
#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif

void psy_timertask_init(psy_TimerTask* self, uintptr_t id,
	void* context, psy_fp_timerwork timerwork, psy_Signal* timersignal,
	uintptr_t customid, uintptr_t interval)
{
	self->id = id;
	self->context = context;
	self->timerwork = timerwork;
	self->timersignal = timersignal;
	self->customid = customid;
	self->interval = interval;
	self->counter = 0;
	self->expired = FALSE;
}

psy_TimerTask* psy_timertask_alloc(void)
{
	return (psy_TimerTask*)malloc(sizeof(psy_TimerTask));
}

psy_TimerTask* psy_timertask_allocinit(uintptr_t id,
	void* context, psy_fp_timerwork timerwork, psy_Signal* timersignal,
	uintptr_t customid, uintptr_t interval)
{
	psy_TimerTask* rv;
	
	rv = psy_timertask_alloc();
	if (rv) {
		psy_timertask_init(rv, id, context, timerwork, timersignal,
			customid, interval);
	}	
	return rv;
}

static void psy_timertask_tick(psy_TimerTask*);

void psy_timertask_tick(psy_TimerTask* self)
{		
	self->counter += 10;
	if (self->counter >= self->interval) {
		self->counter = 0;
		if (self->timersignal) {
			psy_signal_emit(self->timersignal, self->context, 1,
				self->customid);
		}
		if (self->timerwork) {
			self->timerwork(self->context, self->customid);
		}		
	}
}

void psy_timers_init(psy_Timers* self)
{
	self->tasks = NULL;
}

void psy_timers_dispose(psy_Timers* self)
{
	psy_list_deallocate(&self->tasks, (psy_fp_disposefunc)NULL);
}

void psy_timers_tick(psy_Timers* self)
{
	psy_List* p;
	psy_List* q;
	
	for (p = q = self->tasks; q != NULL; p = q) {
		psy_TimerTask* task;
		
		q = p->next;
		task = (psy_TimerTask*)p->entry;
		if (!task->expired) {
			psy_timertask_tick(task);
		} else {
			psy_list_remove(&self->tasks, p);
			free(task);
		}
	}	
}

void psy_timers_addtimer(psy_Timers* self, uintptr_t id, void* context,
	psy_fp_timerwork timerwork, psy_Signal* timersignal,
	uintptr_t customid, uintptr_t interval)
{
	psy_TimerTask* task;
	psy_List* p;	
	
	for (p = self->tasks; p != NULL; p = p->next) {
		psy_TimerTask* task;
				
		task = (psy_TimerTask*)p->entry;
		if (task->id == id && (customid == customid)) {
			task->interval = interval;
			return;			
		}		
	}
	task = psy_timertask_allocinit(id, context, timerwork, timersignal,
		customid, interval);
	if (task) {
		psy_list_append(&self->tasks, (void*)task);		
	}
}

void psy_timers_removetimer(psy_Timers* self, uintptr_t id,
	uintptr_t customid)
{
	psy_List* p;
	psy_List* q;
	
	for (p = q = self->tasks; q != NULL; p = q) {
		psy_TimerTask* task;
		
		q = p->next;
		task = (psy_TimerTask*)p->entry;
		if ((task->id == id) && 
			(customid == psy_INDEX_INVALID || task->customid == customid)) {
				task->expired = TRUE;			
		}		
	}
}

/* http://stackoverflow.com/questions/1861294/how-to-calculate-execution-time-of-a-code-snippet-in-c */
uintptr_t psy_gettime_ms(void)
{
#ifdef WIN32
	/* Windows */
	FILETIME ft;
	LARGE_INTEGER li;

	/* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
	 * to a LARGE_INTEGER structure. */
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	uintptr_t ret = li.QuadPart;
	ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
	ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

	return ret;
#else
	/* Linux */
	struct timeval tv;

	gettimeofday(&tv, NULL);

	uintptr_t ret = tv.tv_usec;
	/* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
	ret /= 1000;

	/* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
	ret += (tv.tv_sec * 1000);

	return ret;
#endif
}

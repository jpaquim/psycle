/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "timers.h"
/* std */
#include <stdlib.h>

void psy_timertask_init(psy_TimerTask* self, uintptr_t id,
	void* context, psy_fp_timerwork timerwork, uintptr_t customid,
	uintptr_t interval)
{
	self->id = id;
	self->context = context;
	self->timerwork = timerwork;
	self->customid = customid;
	self->interval = interval;
	self->counter = 0;
}

psy_TimerTask* psy_timertask_alloc(void)
{
	return (psy_TimerTask*)malloc(sizeof(psy_TimerTask));
}

psy_TimerTask* psy_timertask_allocinit(uintptr_t id,
	void* context, psy_fp_timerwork timerwork, uintptr_t customid,
	uintptr_t interval)
{
	psy_TimerTask* rv;
	
	rv = psy_timertask_alloc();
	if (rv) {
		psy_timertask_init(rv, id, context, timerwork, customid, interval);
	}	
	return rv;
}

static void psy_timertask_tick(psy_TimerTask*);

void psy_timertask_tick(psy_TimerTask* self)
{		
	self->counter += 10;
	if (self->counter >= self->interval) {		
		self->timerwork(self->context, self->customid);
		self->counter = 0;
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
	
	for (p = q = self->tasks; p != NULL; p = q) {
		psy_TimerTask* task;
		
		q = p->next;
		task = (psy_TimerTask*)p->entry;
		psy_timertask_tick(task);
	}
}

void psy_timers_addtimer(psy_Timers* self, uintptr_t id, void* context,
	psy_fp_timerwork timerwork, uintptr_t customid, uintptr_t interval)
{
	psy_TimerTask* task;
	psy_List* p;	
	
	for (p = self->tasks; p != NULL; p = p->next) {
		psy_TimerTask* task;
				
		task = (psy_TimerTask*)p->entry;
		if (task->id == id && (customid == customid)) {
			return;			
		}		
	}
	task = psy_timertask_allocinit(id, context, timerwork,
		customid, interval);
	if (task) {
		psy_list_append(&self->tasks, (void*)task);		
	}
}

void psy_timers_removetimer(psy_Timers* self, uintptr_t id, uintptr_t customid)
{
	psy_List* p;
	psy_List* q;
	
	for (p = q = self->tasks; p != NULL; p = q) {
		psy_TimerTask* task;
		
		q = p->next;
		task = (psy_TimerTask*)p->entry;
		if (task->id == id && 
			(customid == psy_INDEX_INVALID || task->customid == customid)) {
			free(task);
			psy_list_remove(&self->tasks, p);
		}		
	}
}

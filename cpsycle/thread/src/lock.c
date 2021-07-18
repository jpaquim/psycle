/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "../../detail/os.h"
/* local */
#include "lock.h"

#if defined DIVERSALIS__OS__MICROSOFT

void psy_lock_init(psy_Lock* self)
{
	InitializeCriticalSection(&self->lock);	
	self->disabled = 0;	
}

void psy_lock_dispose(psy_Lock* self)
{	
	DeleteCriticalSection(&self->lock);
	self->disabled = 0;	
}

void psy_lock_enable(psy_Lock* self)
{
	self->disabled = 0;
}

void psy_lock_disable(psy_Lock* self)
{
	self->disabled = 1;
}

void psy_lock_enter(psy_Lock* self)
{
	if (!self->disabled) {
		EnterCriticalSection(&self->lock);
	}
}

void psy_lock_leave(psy_Lock* self)
{
	if (!self->disabled) {
		LeaveCriticalSection(&self->lock);
	}
}

#elif defined DIVERSALIS__OS__POSIX

#include <pthread.h>
#include <stdlib.h>

void psy_lock_init(psy_Lock* self)
{	
	self->disabled = 0;
	pthread_mutexattr_t recursiveattr;
	pthread_mutexattr_init(&recursiveattr);
	pthread_mutexattr_settype(&recursiveattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&self->lock, &recursiveattr);
	pthread_mutexattr_destroy(&recursiveattr);		
}

void psy_lock_dispose(psy_Lock* self)
{	
	pthread_mutex_destroy(&self->lock);
	self->disabled = 0;	
}

void psy_lock_enable(psy_Lock* self)
{
	self->disabled = 0;
}

void psy_lock_disable(psy_Lock* self)
{
	self->disabled = 1;
}

void psy_lock_enter(psy_Lock* self)
{
	if (!self->disabled) {
		pthread_mutex_lock(&self->lock);
	}
}

void psy_lock_leave(psy_Lock* self)
{
	if (!self->disabled) {
		 pthread_mutex_unlock(&self->lock);
	}
}

#elif
	#error "Exclusive Lock Platform Not Supported"
#endif

psy_Lock* psy_lock_alloc(void)
{
	return (psy_Lock*) malloc(sizeof(psy_Lock));
}

psy_Lock* psy_lock_allocinit(void)
{
	psy_Lock* rv;

	rv = psy_lock_alloc();
	if (rv) {
		psy_lock_init(rv);
	}
	return rv;
}

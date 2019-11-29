// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "exclusivelock.h"


#if defined DIVERSALIS__OS__MICROSOFT

#include <windows.h>

static int disabled = 0;

static CRITICAL_SECTION worklock;

void lock_init(void)
{
	InitializeCriticalSection(&worklock);	
	disabled = 0;	
}

void lock_dispose(void)
{	
	DeleteCriticalSection(&worklock);
	disabled = 0;
	
}

void lock_enable(void)
{
	disabled = 0;
}

void lock_disable(void)
{
	disabled = 1;
}

void lock_enter(void)
{
	if (!disabled) {
		EnterCriticalSection(&worklock);
	}
}

void lock_leave(void)
{
	if (!disabled) {
		LeaveCriticalSection(&worklock);
	}
}

#elif defined DIVERSALIS__OS__POSIX

#include <pthread.h>

static pthread_mutex_t worklock;

static int disabled = 0;

void lock_init(void)
{	
	disabled = 0;
	pthread_mutexattr_t recursiveattr;
	pthread_mutexattr_init(&recursiveattr);
	pthread_mutexattr_settype(&recursiveattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&worklock, &recursiveattr);
	pthread_mutexattr_destroy(&recursiveattr);		
}

void lock_dispose(void)
{	
	pthread_mutex_destroy(&worklock);
	disabled = 0;	
}

void lock_enable(void)
{
	disabled = 0;
}

void lock_disable(void)
{
	disabled = 1;
}

void lock_enter(void)
{
	if (!disabled) {
		pthread_mutex_lock(&worklock);
	}
}

void lock_leave(void)
{
	if (!disabled) {
		 pthread_mutex_unlock(&worklock);
	}
}

#elif
	#error "

#endif


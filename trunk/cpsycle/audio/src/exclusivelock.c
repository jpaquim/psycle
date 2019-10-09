// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "exclusivelock.h"

#define __cplusplus
#include <diversalis/os.hpp>
#undef __cplusplus

#if defined DIVERSALIS__OS__MICROSOFT

#include <windows.h>

static HANDLE worklock = 0;
static HANDLE workdone = 0;
static HANDLE mutex = 0;
static int disabled = 0;
static int suspended = 0;

void lock_init(void)
{
	worklock = CreateEvent(NULL, TRUE, TRUE, NULL);
	workdone = CreateEvent (NULL, FALSE, FALSE, NULL);
	mutex = CreateEvent(NULL, FALSE, FALSE, NULL);
	suspended = 0;
}

void lock_dispose(void)
{
	CloseHandle(worklock);
	CloseHandle(workdone);
	CloseHandle(mutex);
	suspended = 0;
}

void lock_enable(void)
{
	disabled = 0;
}

void lock_disable(void)
{
	disabled = 1;
}

void suspendwork(void)
{		
	if (!disabled && !suspended) {
		ResetEvent(workdone);
		WaitForSingleObject(mutex, INFINITE);	
		ResetEvent(worklock);
		WaitForSingleObject(workdone, 500);		
	}
	++suspended;
}

void signalwaithost(void)
{		
	if (!disabled) {
		ResetEvent(mutex);
		SetEvent(workdone);	
		WaitForSingleObject(worklock, INFINITE);		
		SetEvent(mutex);		
	}
}

void resumework(void)
{
	--suspended;
	if (!disabled && !suspended) {
		SetEvent(worklock);
	}	
}

#elif defined DIVERSALIS__OS__UNIX


void lock_init(void)
{	
}

void lock_dispose(void)
{
}

void suspendwork(void)
{
}

void signalwaithost(void)
{
}

#endif

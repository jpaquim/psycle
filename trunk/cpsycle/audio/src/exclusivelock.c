// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "exclusivelock.h"

#define __cplusplus
#include <diversalis/os.hpp>
#undef __cplusplus

#if defined DIVERSALIS__OS__MICROSOFT

#include <windows.h>

static HANDLE hGuiEvent = 0;
static HANDLE hWorkDoneEvent = 0;

void lock_init(void)
{
	hGuiEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	hWorkDoneEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
}

void lock_dispose(void)
{
	CloseHandle(hGuiEvent);
	CloseHandle(hWorkDoneEvent);
}

void suspendwork(void)
{	
	ResetEvent(hGuiEvent);
	WaitForSingleObject(hWorkDoneEvent, 500);
}

void resumework(void)
{
	SetEvent(hGuiEvent);
}

void signalwaithost(void)
{
	SetEvent(hWorkDoneEvent);
	WaitForSingleObject(hGuiEvent, INFINITE);	
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

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "exclusivelock.h"
#include "lock.h"

static psy_audio_Lock lock;

void psy_audio_exclusivelock_init(void)
{
	psy_audio_lock_init(&lock);	
}

void psy_audio_exclusivelock_dispose(void)
{	
	psy_audio_lock_dispose(&lock);
}

void psy_audio_exclusivelock_enable(void)
{
	psy_audio_lock_enable(&lock);
}

void psy_audio_exclusivelock_disable(void)
{
	psy_audio_lock_disable(&lock);
}

void psy_audio_exclusivelock_enter(void)
{
	psy_audio_lock_enter(&lock);
}

void psy_audio_exclusivelock_leave(void)
{
	psy_audio_lock_leave(&lock);
}

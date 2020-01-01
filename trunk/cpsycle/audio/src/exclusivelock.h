// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(EXCLUSIVELOCK_H)
#define EXCLUSIVELOCK_H

void lock_init(void);
void lock_dispose(void);
void lock_enable(void);
void lock_disable(void);
void lock_enter(void);
void lock_leave(void);

#endif

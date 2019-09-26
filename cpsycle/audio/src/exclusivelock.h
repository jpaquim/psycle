// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(EXCLUSIVELOCK_H)
#define EXCLUSIVELOCK_H

void lock_init(void);
void lock_dispose(void);
void suspendwork(void);
void resumework(void);
void signalwaithost(void);
void signalhost(void);

#endif
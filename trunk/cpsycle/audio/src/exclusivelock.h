// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_AUDIO_EXCLUSIVELOCK_H)
#define PSY_AUDIO_EXCLUSIVELOCK_H

void psy_audio_lock_init(void);
void psy_audio_lock_dispose(void);
void psy_audio_lock_enable(void);
void psy_audio_lock_disable(void);
void psy_audio_lock_enter(void);
void psy_audio_lock_leave(void);

#endif

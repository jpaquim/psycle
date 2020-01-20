// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_EXCLUSIVELOCK_H
#define psy_audio_EXCLUSIVELOCK_H

#ifdef __cplusplus
extern "C" {
#endif

void psy_audio_lock_init(void);
void psy_audio_lock_dispose(void);
void psy_audio_lock_enable(void);
void psy_audio_lock_disable(void);
void psy_audio_lock_enter(void);
void psy_audio_lock_leave(void);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_EXCLUSIVELOCK_H */

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_KBDDRIVER_H
#define psy_audio_KBDDRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#define PSY_EVENTDRIVER_KBD_GUID 0x0001

struct psy_EventDriver* psy_audio_kbddriver_create(void);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_KBDDRIVER_H */

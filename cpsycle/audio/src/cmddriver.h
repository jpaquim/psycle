/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_CMDDRIVER_H
#define psy_audio_CMDDRIVER_H

#include "../../driver/eventdriver.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSY_EVENTDRIVER_CMD_GUID 0x0005

psy_EventDriver* psy_audio_cmddriver_create(void);

void psy_audio_cmddriver_setcmd(psy_EventDriver*, const char* section, psy_EventDriverCmd);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_CMDDRIVER_H */

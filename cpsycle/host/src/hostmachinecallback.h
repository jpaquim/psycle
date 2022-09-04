/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(HOSTMACHINECALLBACK_H)
#define HOSTMACHINECALLBACK_H

/* host */
#include "dirconfig.h"
/* audio */
#include <machinefactory.h>
#include <signal.h>
/* file */
#include <logger.h>

/*
** HostMachineCallback
**
** connects the machines with the psycle host ui
**
** psy_audio_MachineCallback
**         ^
**         |
**     HostMachineCallback
**             <>---- PsycleConfig
**             <>---- psy_audio_MachineFactory
**             <>---- psy_Logger
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HostMachineCallback {
	/* implements */
	psy_audio_MachineCallback machinecallback;
	/* references */	
	DirConfig* dir_config;
	psy_Logger* logger;
	psy_Signal* signal_machineeditresize;
	psy_Signal* signal_buschanged;
} HostMachineCallback;

void hostmachinecallback_init(HostMachineCallback*, DirConfig*,
	psy_Signal* signal_machineeditresize, psy_Signal* signal_buschanged);
	

INLINE psy_audio_MachineCallback* hostmachinecallback_base(
	HostMachineCallback* self)
{
	return &self->machinecallback;
}

#ifdef __cplusplus
}
#endif

#endif /* HOSTMACHINECALLBACK_H */

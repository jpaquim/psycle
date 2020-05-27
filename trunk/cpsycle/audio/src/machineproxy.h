// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MACHINEPROXY_H
#define psy_audio_MACHINEPROXY_H

#include "machine.h"

#ifdef __cplusplus
extern "C" {
#endif

// psy_audio_MachineProxy
// Proxy
// aim: protecting the host from crashing
//      MachineProxy gets a Machine as client injected and delegates all of
//   	its methods to the client. The MachineProxy acts exactly like the
//      client by delegating all calls, but inside a try catch block, and
//      tries to protect the host from crashing if the client fails.
//      The MachineProxy dispose method deallocates all client resources.
// platform notes: Has only effect on windows using the SEH api.
//                 Todo: unix equivalent for C?
// Structure:
//      psy_audio_Machine
//             ^
//             |                                   
//	psy_audio_MachineProxy <@>----- psy_audio_Machine
//                                1

typedef struct psy_audio_MachineProxy {
	psy_audio_Machine machine;
	/// Injected machine at init
	psy_audio_Machine* client;
	/// Tells wether the machine client has crashed.
	int crashed;
	/// Super call fp to machine client's dispose method
	fp_machine_dispose machinedispose;
} psy_audio_MachineProxy;

// inits the proxy and injects the client
void psy_audio_machineproxy_init(psy_audio_MachineProxy* self,
	psy_audio_Machine* client);

psy_audio_MachineProxy* psy_audio_machineproxy_alloc(void);
psy_audio_MachineProxy* psy_audio_machineproxy_allocinit(psy_audio_Machine*
	client);

INLINE psy_audio_Machine* psy_audio_machineproxy_base(
	psy_audio_MachineProxy* self)
{
	return (psy_audio_Machine*)&self->machine;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINEPROXY_H */

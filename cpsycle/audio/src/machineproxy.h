// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEPROXY_H)
#define MACHINEPROXY_H

#include "machine.h"

typedef struct {
	Machine machine;
	Machine* client;
	/// Tells wether the machine client has crashed.
	int crashed;
	fp_machine_dispose machinedispose;
} MachineProxy;

void machineproxy_init(MachineProxy* self, Machine* client);

#endif

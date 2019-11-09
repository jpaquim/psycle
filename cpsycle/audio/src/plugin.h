// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(PLUGIN_H)
#define PLUGIN_H

#include "machine.h"
#include "library.h"

typedef struct {
	Machine machine;	
	CMachineInterface* mi;
	Library library;
	float pan;
	MachineInfo* plugininfo;
	char* editname;
} Plugin;

void plugin_init(Plugin* self, MachineCallback, const char* path);
int plugin_psycle_test(const char* path, MachineInfo*);

#endif

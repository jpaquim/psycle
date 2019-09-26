// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEFACTORY_H)
#define MACHINEFACTORY_H

#include "machine.h"
#include <properties.h>

typedef struct {
	MachineCallback machinecallback;
	Properties* configuration;
	Properties* directories;
} MachineFactory;

void machinefactory_init(MachineFactory*, MachineCallback, Properties* configuration);
Machine* machinefactory_make(MachineFactory*, MachineType, const char* path);

#endif
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(PLUGINCATCHER_H)
#define PLUGINCATCHER_H

#include <properties.h>

typedef struct {
	Properties* plugins;
} PluginCatcher;

void plugincatcher_init(PluginCatcher*);
void plugincatcher_dispose(PluginCatcher* self);
void plugincatcher_scan(PluginCatcher*, const char* path, int type);

#endif
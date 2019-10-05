// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLUGINCATCHER_H)
#define PLUGINCATCHER_H

#include <properties.h>
#include <signal.h>

typedef struct {
	Properties* plugins;
	char* inipath;
	Properties* dirconfig;
	Signal signal_changed;
} PluginCatcher;

void plugincatcher_init(PluginCatcher*, Properties* dirconfig);
void plugincatcher_dispose(PluginCatcher*);
void plugincatcher_clear(PluginCatcher*);
void plugincatcher_scan(PluginCatcher*);
int plugincatcher_load(PluginCatcher*);
void plugincatcher_save(PluginCatcher*);

#endif
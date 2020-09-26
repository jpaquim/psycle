// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PSY2CONVERTER_H
#define psy_audio_PSY2CONVERTER_H

#include <hashtbl.h>

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_SongFile;
struct psy_audio_Song;
struct psy_audio_Machine;

typedef struct PluginNames {
	psy_Table container;
} PluginNames;

void pluginnames_init(PluginNames*);
void pluginnames_dispose(PluginNames*);
bool pluginnames_exists(PluginNames*, int type, const char* name);
const char* pluginnames_convname(PluginNames*, int type, const char* name);

typedef struct InternalMachinesConvert {
	PluginNames pluginnames;
	psy_Table machine_converted_from;
} InternalMachinesConvert;

void internalmachinesconvert_init(InternalMachinesConvert*);
void internalmachinesconvert_dispose(InternalMachinesConvert*);

INLINE bool internalmachinesconvert_pluginname_exists(
	InternalMachinesConvert* self, int type, const char* name)
{
	return pluginnames_exists(&self->pluginnames, type, name);
}

struct psy_audio_Machine* internalmachinesconvert_redirect(
	InternalMachinesConvert*,
	struct psy_audio_SongFile*, int* index, int type, const char* name);
void internalmachineconverter_retweak_song(InternalMachinesConvert*,
	struct psy_audio_Song*,
	double samplerate);
#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PSY2CONVERTER_H */

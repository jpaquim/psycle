// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_VSTPLUGIN_H
#define psy_audio_VSTPLUGIN_H

#include "custommachine.h"
#include "library.h"

typedef struct {
	unsigned char key;
	unsigned char midichan;
} VstNote;

typedef struct {
	psy_audio_CustomMachine custommachine;		
	psy_Library library;
	struct AEffect* effect;
	psy_audio_MachineInfo* info;
	void* editorhandle;
	struct VstEvents* events;
	int eventcap;	
	psy_audio_MachineInfo* plugininfo;
	psy_Table tracknote;
} psy_audio_VstPlugin;

void psy_audio_vstplugin_init(psy_audio_VstPlugin*, MachineCallback, const char* path);
INLINE psy_audio_Machine* psy_audio_vstplugin_base(psy_audio_VstPlugin* self)
{
	return &(self->custommachine.machine);
}

int psy_audio_plugin_vst_test(const char* path, psy_audio_MachineInfo* rv);

#endif /* psy_audio_VSTPLUGIN_H */

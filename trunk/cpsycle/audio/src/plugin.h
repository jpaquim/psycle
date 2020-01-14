// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_AUDIO_PLUGIN_H)
#define PSY_AUDIO_PLUGIN_H

#include "custommachine.h"
#include "library.h"

typedef struct {
	psy_audio_CustomMachine custommachine;	
	CMachineInterface* mi;
	psy_Library library;	
	psy_audio_MachineInfo* plugininfo;
	unsigned int preventsequencerlinetick;
} psy_audio_Plugin;

void psy_audio_plugin_init(psy_audio_Plugin*, MachineCallback, const char* path);
psy_audio_Machine* psy_audio_plugin_base(psy_audio_Plugin*);
int psy_audio_plugin_psycle_test(const char* path, psy_audio_MachineInfo*);

#endif

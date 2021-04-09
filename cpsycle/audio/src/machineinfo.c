// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinedefs.h"
#include "machineinfo.h"
#include "plugin_interface.h"
// platform
#include "../../detail/portable.h"

void machineinfo_init(psy_audio_MachineInfo* self)
{
	self->Author = strdup("");
	self->Command = strdup("");
	self->Flags = 0;
	self->mode = psy_audio_MACHMODE_FX;
	self->Name = strdup("");	
	self->ShortName = strdup("");
	self->APIVersion = 0;
	self->PlugVersion = 0;
	self->type = 0;	
	self->modulepath = strdup("");
	self->shellidx = 0;
	self->helptext = strdup("");
	self->desc = strdup("");
}

void machineinfo_init_copy(psy_audio_MachineInfo* self, psy_audio_MachineInfo* src)
{
	self->Author = psy_strdup(src->Author);
	self->Command = psy_strdup(src->Command);
	self->Flags = src->Flags;
	self->Name = psy_strdup(src->Name);
	self->ShortName = psy_strdup(src->ShortName);
	self->APIVersion = src->APIVersion;
	self->PlugVersion = src->PlugVersion;
	self->type = src->type;
	self->modulepath = psy_strdup(src->modulepath);
	self->shellidx = src->shellidx;
	self->helptext = psy_strdup(src->helptext);
	self->desc = psy_strdup(src->desc);
}

void machineinfo_copy(psy_audio_MachineInfo* self, const psy_audio_MachineInfo* src)
{
	machineinfo_dispose(self);
	self->Author = psy_strdup(src->Author);
	self->Command = psy_strdup(src->Command);
	self->Flags = src->Flags;
	self->Name = psy_strdup(src->Name);
	self->ShortName = psy_strdup(src->ShortName);
	self->APIVersion = src->APIVersion;
	self->PlugVersion = src->PlugVersion;
	self->type = src->type;
	self->modulepath = psy_strdup(src->modulepath);
	self->shellidx = src->shellidx;
	self->helptext = psy_strdup(src->helptext);
	self->desc = psy_strdup(src->desc);
}

void machineinfo_set(psy_audio_MachineInfo* self,
		const char* author,
		const char* command,
		int flags,
		int mode,
		const char* name,
		const char* shortname,
		int16_t apiversion,
		int16_t plugversion,
		int type,		
		const char* modulepath,
		uintptr_t shellidx,
		const char* helptext,
		const char* desc)
{		
	psy_strreset(&self->Author, author);
	psy_strreset(&self->Command, command);
	self->Flags = flags;
	self->mode = mode;
	psy_strreset(&self->Name, name);
	psy_strreset(&self->ShortName, shortname);
	self->APIVersion = apiversion;
	self->PlugVersion = plugversion;
	self->type = type;
	psy_strreset(&self->modulepath, modulepath);
	self->shellidx = shellidx;
	psy_strreset(&self->helptext, helptext);
	psy_strreset(&self->desc, desc);
}

void machineinfo_setnativeinfo(psy_audio_MachineInfo* self,
	CMachineInfo* info,
	int type,
	const char* modulepath,
	int shellidx)
{
	machineinfo_dispose(self);
	psy_strreset(&self->Author, info->Author);
	psy_strreset(&self->Command, info->Command);
	self->Flags = info->Flags;
	self->mode = ((info->Flags & 3) == 3)
		? psy_audio_MACHMODE_GENERATOR
		: psy_audio_MACHMODE_FX;
	psy_strreset(&self->Name, info->Name);
	psy_strreset(&self->ShortName, info->ShortName);
	self->APIVersion = info->APIVersion;
	self->PlugVersion = info->PlugVersion;
	self->type = type;
	psy_strreset(&self->modulepath, modulepath);
	self->shellidx = shellidx;
	psy_strreset(&self->helptext, info->Command);
	if (self->mode == psy_audio_MACHMODE_GENERATOR) {
		psy_strreset(&self->desc, "Psycle instrument");
	} else {
		psy_strreset(&self->desc, "Psycle effect");
	}	
}

void machineinfo_dispose(psy_audio_MachineInfo* self)
{
	free(self->Author);
	self->Author = NULL;
	free(self->Name);
	self->Name = NULL;
	free(self->ShortName);
	self->ShortName = NULL;
	free(self->Command);
	self->Command = NULL;
	free(self->modulepath);	
	self->modulepath = NULL;
	free(self->helptext);
	self->helptext = NULL;
	free(self->desc);
	self->desc = NULL;
}

psy_audio_MachineInfo* machineinfo_alloc(void)
{
	return (psy_audio_MachineInfo*) malloc(sizeof(psy_audio_MachineInfo));
}

psy_audio_MachineInfo* machineinfo_allocinit(void)
{
	psy_audio_MachineInfo* rv;

	rv = machineinfo_alloc();
	if (rv) {
		machineinfo_init(rv);
	}
	return rv;
}

psy_audio_MachineInfo* machineinfo_clone(const psy_audio_MachineInfo* self)
{
	psy_audio_MachineInfo* rv;

	rv = (psy_audio_MachineInfo*) malloc(sizeof(psy_audio_MachineInfo));
	if (rv) {
		rv->Author = psy_strdup(self->Author);
		rv->Command = psy_strdup(self->Command);
		rv->Flags = self->Flags;
		rv->Name = psy_strdup(self->Name);
		rv->ShortName = psy_strdup(self->ShortName);
		rv->APIVersion = self->APIVersion;
		rv->PlugVersion = self->PlugVersion;
		rv->type = self->type;
		rv->modulepath = psy_strdup(self->modulepath);
		rv->shellidx = self->shellidx;
		rv->helptext = psy_strdup(self->helptext);
		rv->desc = psy_strdup(self->desc);
	}
	return rv;
}

void machineinfo_clear(psy_audio_MachineInfo* self)
{
	machineinfo_dispose(self);
	machineinfo_init(self);
}

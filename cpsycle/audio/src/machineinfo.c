// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinedefs.h"
#include "machineinfo.h"
#include <string.h>
#include <stdlib.h> 
#include "plugin_interface.h"

void machineinfo_init(psy_audio_MachineInfo* self)
{
	self->Author = strdup("");
	self->Command = strdup("");
	self->Flags = 0;
	self->mode = MACHMODE_FX;
	self->Name = strdup("");	
	self->ShortName = strdup("");
	self->APIVersion = 0;
	self->PlugVersion = 0;
	self->type = 0;	
	self->modulepath = strdup("");
	self->shellidx = 0;
}

void machineinfo_init_copy(psy_audio_MachineInfo* self, psy_audio_MachineInfo* other)
{
	self->Author = strdup(other->Author ? other->Author : "");
	self->Command = strdup(other->Command ? other->Command : "");
	self->Flags = other->Flags;
	self->Name = strdup(other->Name ? other->Name : "");
	self->ShortName = strdup(other->ShortName ? other->ShortName : "");
	self->APIVersion = other->APIVersion;
	self->PlugVersion = other->PlugVersion;
	self->type = other->type;
	self->modulepath = strdup(other->modulepath ? other->modulepath : "");
	self->shellidx = other->shellidx;
}

void machineinfo_copy(psy_audio_MachineInfo* self, psy_audio_MachineInfo* other)
{
	machineinfo_dispose(self);
	self->Author = strdup(other->Author ? other->Author : "");
	self->Command = strdup(other->Command ? other->Command : "");
	self->Flags = other->Flags;
	self->Name = strdup(other->Name ? other->Name : "");
	self->ShortName = strdup(other->ShortName ? other->ShortName : "");
	self->APIVersion = other->APIVersion;
	self->PlugVersion = other->PlugVersion;
	self->type = other->type;
	self->modulepath = strdup(other->modulepath ? other->modulepath : "");
	self->shellidx = other->shellidx;
}

void machineinfo_set(psy_audio_MachineInfo* self,
		const char* author,
		const char* command,
		int flags,
		int mode,
		const char* name,
		const char* shortname,
		short apiversion,
		short plugversion,
		int type,		
		const char* modulepath,
		int shellidx)
{	
	self->Author = strdup(author);
	self->Command = strdup(command);
	self->Flags = flags;
	self->mode = mode;
	self->Name = strdup(name);	
	self->ShortName = strdup(shortname);
	self->APIVersion = apiversion;
	self->PlugVersion = plugversion;
	self->type = type;
	self->modulepath = strdup(modulepath);
	self->shellidx = shellidx;	
}

void machineinfo_setnativeinfo(psy_audio_MachineInfo* self,
		CMachineInfo* info,
		int type,		
		const char* modulepath,
		int shellidx)
{
	machineinfo_dispose(self);
	self->Author = strdup(info->Author);
	self->Command = strdup(info->Command);
	self->Flags = info->Flags;
	self->mode = (info->Flags & 3) == 3 ? MACHMODE_GENERATOR : MACHMODE_FX;
	self->Name = strdup(info->Name);		
	self->ShortName = strdup(info->ShortName);
	self->APIVersion = info->APIVersion;
	self->PlugVersion = info->PlugVersion;
	self->type = type;	
	self->modulepath = strdup(modulepath);
	self->shellidx = shellidx;
}

void machineinfo_dispose(psy_audio_MachineInfo* self)
{
	free(self->Author);
	self->Author = 0;
	free(self->Name);
	self->Name = 0;
	free(self->Command);
	self->Command = 0;
	free(self->modulepath);	
	self->modulepath = 0;
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
		rv->Author = strdup(self->Author ? self->Author : "");
		rv->Command = strdup(self->Command ? self->Command : "");
		rv->Flags = self->Flags;
		rv->Name = strdup(self->Name ? self->Name : "");
		rv->ShortName = strdup(self->ShortName ? self->ShortName : "");
		rv->APIVersion = self->APIVersion;
		rv->PlugVersion = self->PlugVersion;
		rv->type = self->type;
		rv->modulepath = strdup(self->modulepath ? self->modulepath : "");
		rv->shellidx = self->shellidx;
	}
	return rv;
}

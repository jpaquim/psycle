/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "machineinfo.h"
/* local */
#include "machinedefs.h"
#include "plugin_interface.h"
/* platform */
#include "../../detail/portable.h"


/* implementation */
void machineinfo_init(psy_audio_MachineInfo* self)
{
	assert(self);
	
	self->author = psy_strdup("");
	self->command = psy_strdup("");
	self->flags = 0;
	self->mode = psy_audio_MACHMODE_FX;
	self->name = psy_strdup("");
	self->shortname = psy_strdup("");
	self->apiversion = 0;
	self->plugversion = 0;
	self->type = 0;	
	self->modulepath = psy_strdup("");
	self->shellidx = 0;
	self->helptext = psy_strdup("");
	self->desc = psy_strdup("");
	self->category = psy_strdup("");
}

void machineinfo_init_copy(psy_audio_MachineInfo* self,
	psy_audio_MachineInfo* src)
{
	assert(self);
	
	self->author = psy_strdup(src->author);
	self->command = psy_strdup(src->command);
	self->flags = src->flags;
	self->name = psy_strdup(src->name);
	self->shortname = psy_strdup(src->shortname);
	self->apiversion = src->apiversion;
	self->plugversion = src->plugversion;
	self->type = src->type;
	self->modulepath = psy_strdup(src->modulepath);
	self->shellidx = src->shellidx;
	self->helptext = psy_strdup(src->helptext);
	self->desc = psy_strdup(src->desc);
	self->category = psy_strdup(src->category);
}

void machineinfo_copy(psy_audio_MachineInfo* self,
	const psy_audio_MachineInfo* src)
{
	assert(self);
	
	machineinfo_dispose(self);
	self->author = psy_strdup(src->author);
	self->command = psy_strdup(src->command);
	self->flags = src->flags;
	self->name = psy_strdup(src->name);
	self->shortname = psy_strdup(src->shortname);
	self->apiversion = src->apiversion;
	self->plugversion = src->plugversion;
	self->type = src->type;
	self->modulepath = psy_strdup(src->modulepath);
	self->shellidx = src->shellidx;
	self->helptext = psy_strdup(src->helptext);
	self->desc = psy_strdup(src->desc);
	self->category = psy_strdup(src->category);	
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
		const char* desc,
		const char* category)
{	
	assert(self);
		
	psy_strreset(&self->author, author);
	psy_strreset(&self->command, command);
	self->flags = flags;
	self->mode = mode;
	psy_strreset(&self->name, name);
	psy_strreset(&self->shortname, shortname);
	self->apiversion = apiversion;
	self->plugversion = plugversion;
	self->type = type;
	psy_strreset(&self->modulepath, modulepath);
	self->shellidx = shellidx;
	psy_strreset(&self->helptext, helptext);
	psy_strreset(&self->desc, desc);
	psy_strreset(&self->category, category);
}

void machineinfo_setnativeinfo(psy_audio_MachineInfo* self,
	CMachineInfo* info,
	int type,
	const char* modulepath,
	int shellidx)
{
	assert(self);
	
	machineinfo_dispose(self);
	psy_strreset(&self->author, info->Author);
	psy_strreset(&self->command, info->Command);
	self->flags = info->Flags;
	self->mode = ((info->Flags & 3) == 3)
		? psy_audio_MACHMODE_GENERATOR
		: psy_audio_MACHMODE_FX;
	psy_strreset(&self->name, info->Name);
	psy_strreset(&self->shortname, info->ShortName);
	self->apiversion = info->APIVersion;
	self->plugversion = info->PlugVersion;
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
	assert(self);
	
	free(self->author);
	self->author = NULL;
	free(self->name);
	self->name = NULL;
	free(self->shortname);
	self->shortname = NULL;
	free(self->command);
	self->command = NULL;
	free(self->modulepath);	
	self->modulepath = NULL;
	free(self->helptext);
	self->helptext = NULL;
	free(self->desc);
	self->desc = NULL;
	free(self->category);
	self->category = NULL;
}

psy_audio_MachineInfo* machineinfo_alloc(void)
{
	return (psy_audio_MachineInfo*)malloc(sizeof(psy_audio_MachineInfo));
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
	
	assert(self);

	rv = (psy_audio_MachineInfo*)malloc(sizeof(psy_audio_MachineInfo));
	if (rv) {
		rv->author = psy_strdup(self->author);
		rv->command = psy_strdup(self->command);
		rv->flags = self->flags;
		rv->name = psy_strdup(self->name);
		rv->shortname = psy_strdup(self->shortname);
		rv->apiversion = self->apiversion;
		rv->plugversion = self->plugversion;
		rv->type = self->type;
		rv->modulepath = psy_strdup(self->modulepath);
		rv->shellidx = self->shellidx;
		rv->helptext = psy_strdup(self->helptext);
		rv->desc = psy_strdup(self->desc);
		rv->category = psy_strdup(self->category);
	}
	return rv;
}

void machineinfo_clear(psy_audio_MachineInfo* self)
{
	assert(self);
	
	machineinfo_dispose(self);
	machineinfo_init(self);
}

bool machineinfo_internal(const psy_audio_MachineInfo* self)
{
	assert(self);
	
	return !(self->type == psy_audio_PLUGIN ||
		self->type == psy_audio_LUA ||
		self->type == psy_audio_VST ||
		self->type == psy_audio_VSTFX ||
		self->type == psy_audio_LADSPA);
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEINFO_H)
#define MACHINEINFO_H

#include "plugin_interface.h"

typedef struct {
	/// API version. Use MI_VERSION
	short APIVersion;
	/// plug version. Your machine version. Shown in Hexadecimal.
	short PlugVersion;
	/// Machine flags
	int Flags;
	/// Defines the type of machine
	int mode;
	char  * Name;
	/// "Name of the machine in machine Display"
	char  * ShortName;
	/// "Name of author"
	char  * Author;
	/// "Text to show as custom command (see Command method)"
	char  * Command;	
	/// host type
	int type;		
	/// module path
	char* modulepath;
	/// shellidx
	int shellidx;	
} MachineInfo;

void machineinfo_init(MachineInfo*);
void machineinfo_dispose(MachineInfo*);
MachineInfo* machineinfo_alloc(void);
MachineInfo* machineinfo_allocinit(void);
MachineInfo* machineinfo_clone(const MachineInfo*);

void machineinfo_set(MachineInfo*,
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
		int shellidx);
void machineinfo_setnativeinfo(MachineInfo*,
		CMachineInfo*,
		int type,		
		const char* modulepath,
		int shellidx);

#endif

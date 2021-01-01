// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MACHINEINFO_H
#define psy_audio_MACHINEINFO_H

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	/// API version. Use MI_VERSION
	short APIVersion;
	/// plug version. Your machine version. Shown in Hexadecimal.
	short PlugVersion;
	/// psy_audio_Machine flags
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
	char* helptext;
} psy_audio_MachineInfo;

void machineinfo_init(psy_audio_MachineInfo*);
void machineinfo_dispose(psy_audio_MachineInfo*);
psy_audio_MachineInfo* machineinfo_alloc(void);
psy_audio_MachineInfo* machineinfo_allocinit(void);
psy_audio_MachineInfo* machineinfo_clone(const psy_audio_MachineInfo*);

void machineinfo_set(psy_audio_MachineInfo*,
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
		int shellidx,
		const char* helptext);
struct CMachineInfo;		
void machineinfo_setnativeinfo(psy_audio_MachineInfo*,
		struct CMachineInfo*,
		int type,		
		const char* modulepath,
		int shellidx);
void machineinfo_init_copy(psy_audio_MachineInfo*, psy_audio_MachineInfo* other);
void machineinfo_copy(psy_audio_MachineInfo*, const psy_audio_MachineInfo* other);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINEINFO_H */

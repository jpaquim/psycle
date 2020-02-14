// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MACHINEINFO_H
#define psy_audio_MACHINEINFO_H

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
		short apiversion,
		short plugversion,
		int type,		
		const char* modulepath,
		int shellidx);
struct CMachineInfo;		
void machineinfo_setnativeinfo(psy_audio_MachineInfo*,
		struct CMachineInfo*,
		int type,		
		const char* modulepath,
		int shellidx);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINEINFO_H */

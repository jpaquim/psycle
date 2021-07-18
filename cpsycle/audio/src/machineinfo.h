/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_MACHINEINFO_H
#define psy_audio_MACHINEINFO_H

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_MachineInfo {
	/* API version.Use MI_VERSION */
	short apiversion;
	/* plug version. Your machine version. Shown in Hexadecimal. */
	short plugversion;
	/* psy_audio_Machine flags */
	int flags;
	/* Defines the type of machine */
	int mode;
	char* name;
	/* "Name of the machine in machine Display" */
	char* shortname;
	/* "Name of author" */
	char* author;
	/* "Text to show as custom command (see Command method)" */
	char* command;	
	/* host type */
	int type;		
	/* module path */
	char* modulepath;
	/* shellidx */
	uintptr_t shellidx;
	char* helptext;
	char* desc;
	char* category;	
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
		uintptr_t shellidx,
		const char* helptext,
		const char* desc,
		const char* category);
struct CMachineInfo;		
void machineinfo_setnativeinfo(psy_audio_MachineInfo*,
		struct CMachineInfo*,
		int type,		
		const char* modulepath,
		int shellidx);
void machineinfo_init_copy(psy_audio_MachineInfo*, psy_audio_MachineInfo* other);
void machineinfo_copy(psy_audio_MachineInfo*, const psy_audio_MachineInfo* other);
void machineinfo_clear(psy_audio_MachineInfo*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINEINFO_H */

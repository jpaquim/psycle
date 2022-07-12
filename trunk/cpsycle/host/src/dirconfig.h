/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(DIRCONFIG_H)
#define DIRCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

struct psy_Property;

typedef struct DirConfig {	
	/* internal */
	struct psy_Property* directories;	
} DirConfig;

void dirconfig_init(DirConfig*, struct psy_Property* parent);
void dirconfig_dispose(DirConfig*);

const char* dirconfig_app(const DirConfig*);
const char* dirconfig_songs(const DirConfig*);
const char* dirconfig_samples(const DirConfig*);
const char* dirconfig_plugins32(const DirConfig*);
const char* dirconfig_plugins64(const DirConfig*);
const char* dirconfig_plugins_curr_platform(const DirConfig*);
const char* dirconfig_lua_scripts(const DirConfig*);
const char* dirconfig_vsts32(const DirConfig*);
const char* dirconfig_vsts64(const DirConfig*);
const char* dirconfig_ladspas(const DirConfig*);
const char* dirconfig_skins(const DirConfig*);
const char* dirconfig_doc(const DirConfig*);
const char* dirconfig_config_dir(const DirConfig*);
const char* dirconfig_user_presets(const DirConfig*);

#ifdef __cplusplus
}
#endif

#endif /* DIRCONFIG_H */

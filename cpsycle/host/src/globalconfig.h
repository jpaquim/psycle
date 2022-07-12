/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(GLOBALCONFIG_H)
#define GLOBALCONFIG_H

/* host*/
#include "languageconfig.h"
#include "viewindex.h"
/* container */
#include <properties.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GlobalConfig */

enum {
	PROPERTY_ID_REGENERATEPLUGINCACHE = 10000,
	PROPERTY_ID_ENABLEAUDIO,	
	PROPERTY_ID_IMPORTCONFIG
};

struct psy_audio_Player;
struct PluginScanThread;

typedef struct GlobalConfig {
	/* signals */
	psy_Property* global;
	psy_Property* import_config;
	/* internal */
	LanguageConfig language;
	/* references */
	psy_Property* parent;
	struct psy_audio_Player* player;
	struct PluginScanThread* pluginscanthread;
} GlobalConfig;

void globalconfig_init(GlobalConfig*, psy_Property* parent,
	struct psy_audio_Player*, struct PluginScanThread*);
void globalconfig_dispose(GlobalConfig*);

bool globalconfig_audio_enabled(const GlobalConfig*);
void globalconfig_enable_audio(GlobalConfig*, bool on);

bool globalconfig_hasproperty(const GlobalConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* GLOBALCONFIG_H */

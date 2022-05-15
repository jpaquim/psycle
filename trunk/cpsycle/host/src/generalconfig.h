/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(GENERALCONFIG_H)
#define GENERALCONFIG_H

/* host*/
#include "viewindex.h"
/* container */
#include <properties.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GeneralConfig */

enum {
	PROPERTY_ID_SHOWSTEPSEQUENCER = 20000,
	PROPERTY_ID_SHOWPLAYLIST,
	PROPERTY_ID_SHOWSEQUENCEEDIT,
	PROPERTY_ID_SHOWMINIVIEW
};

typedef struct GeneralConfig {
	/* signals */
	psy_Signal signal_changed;
	psy_Property* general;
	/* references */
	psy_Property* parent;	
} GeneralConfig;

void generalconfig_init(GeneralConfig*, psy_Property* parent);
void generalconfig_dispose(GeneralConfig*);

bool generalconfig_show_song_info_on_load(const GeneralConfig*);
bool generalconfig_showaboutatstart(const GeneralConfig*);
ViewIndex generalconfig_start_view(const GeneralConfig*);
bool generalconfig_showmaximizedatstart(const GeneralConfig*);
bool generalconfig_save_recent_songs(const GeneralConfig*);
bool generalconfig_playsongafterload(const GeneralConfig*);
bool generalconfig_showingpatternnames(const GeneralConfig*);
bool generalconfig_showsequenceedit(const GeneralConfig*);
void generalconfig_setsequenceeditshowstate(GeneralConfig*, bool state);
bool generalconfig_showstepsequencer(const GeneralConfig*);
void generalconfig_setstepsequencershowstate(GeneralConfig*, bool state);
bool generalconfig_showplaylist(const GeneralConfig*);
void generalconfig_setplaylistshowstate(GeneralConfig*, bool state);
void generalconfig_showpatternnames(GeneralConfig*);
void generalconfig_showpatternids(GeneralConfig*);
bool generalconfig_showminiview(const GeneralConfig*);

uintptr_t generalconfig_onchanged(GeneralConfig*, psy_Property*);
bool generalconfig_hasproperty(const GeneralConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* GENERALCONFIG_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PLUGINCATEGORYLIST_H
#define psy_audio_PLUGINCATEGORYLIST_H

#include "../../detail/psydef.h"

// container
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_PluginCategoryList {
	psy_Table container;
} psy_audio_PluginCategoryList;

void psy_audio_plugincategorylist_init(psy_audio_PluginCategoryList*);
void psy_audio_plugincategorylist_dispose(psy_audio_PluginCategoryList*);

const char* psy_audio_plugincategorylist_category(psy_audio_PluginCategoryList*,
	const char* plugincatchername);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PLUGINCATEGORYLIST_H */

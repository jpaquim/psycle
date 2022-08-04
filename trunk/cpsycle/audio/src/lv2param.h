/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_LV2PAPARAM_H
#define psy_audio_LV2PAPARAM_H

#include "../../detail/psyconf.h"

#ifdef PSYCLE_USE_LV2

/* local */
#include "custommachine.h"
/* lilv */
#include "lilv/lilv.h"
/* lv2 */
#include "lv2/core/lv2.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_LV2Param {
	psy_audio_CustomMachineParam custommachineparam;
	const LilvPort* port;
	const LilvPlugin* plugin;
	const char *port_name;
	uintptr_t index;
	uintptr_t port_index;
	float value_;
	float minval_;
	float maxval_;
} psy_audio_LV2Param;

void psy_audio_lv2param_init(psy_audio_LV2Param*,
	const LilvPort * port,
	const LilvPlugin* plugin,
	const char *newname,
	uintptr_t index,
	uintptr_t port_index);
void psy_audio_lv2param_dispose(psy_audio_LV2Param*);

psy_audio_LV2Param* psy_audio_lv2param_alloc(void);
psy_audio_LV2Param* psy_audio_lv2param_allocinit(
	const LilvPort* port,
	const LilvPlugin* plugin,
	const char *newname,	
	uintptr_t index,
	uintptr_t port_index);


#ifdef __cplusplus
}
#endif

#endif /* PSYCLE_USE_LV2 */

#endif /* psy_audio_LV2PARAM_H */


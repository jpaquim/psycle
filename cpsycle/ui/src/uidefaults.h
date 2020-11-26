// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_DEFAULTS_H
#define psy_ui_DEFAULTS_H

#include "../../detail/psyconf.h"

#include "../../detail/stdint.h"
#include "uidef.h"
#include "uistyle.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t errorcolour;
	psy_ui_Style style_common;
} psy_ui_Defaults;

void psy_ui_defaults_init(psy_ui_Defaults*);
void psy_ui_defaults_dispose(psy_ui_Defaults*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_DEFAULTS_H */

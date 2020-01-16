// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_DEFAULTS_H
#define psy_ui_DEFAULTS_H

#include "../../detail/stdint.h"

#include "uidef.h"

typedef struct {
	uint32_t defaultbackgroundcolor;
	uint32_t defaultcolor;
	uint32_t default_color_border;
	psy_ui_Font defaultfont;	
} psy_ui_Defaults;

void ui_defaults_init(psy_ui_Defaults*);
void ui_defaults_dispose(psy_ui_Defaults*);

uint32_t ui_defaults_color(psy_ui_Defaults*);
uint32_t ui_defaults_backgroundcolor(psy_ui_Defaults*);
uint32_t ui_defaults_bordercolor(psy_ui_Defaults*);

#endif /* psy_ui_DEFAULTS_H */

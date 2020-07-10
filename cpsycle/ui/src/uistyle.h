// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_STYLE_H
#define psy_ui_STYLE_H

#include "uifont.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_Style {
	psy_ui_Font font;
	uint32_t color;
	uint32_t backgroundcolor;
	bool use_font;
	bool use_color;
	bool use_backgroundcolor;
} psy_ui_Style;

void psy_ui_style_init(psy_ui_Style*);
void psy_ui_style_dispose(psy_ui_Style*);
int psy_ui_style_active(psy_ui_Style*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_STYLE_H */

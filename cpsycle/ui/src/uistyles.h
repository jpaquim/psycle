/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_STYLES_H
#define psy_ui_STYLES_H

/* local */
#include "uistyle.h"

#ifdef __cplusplus
extern "C" {
#endif

/* psy_ui_Styles */
typedef struct psy_ui_Styles {
	psy_Table styles;
	psy_Property config;
	psy_ui_ThemeMode theme;
} psy_ui_Styles;

void psy_ui_styles_init(psy_ui_Styles*);
void psy_ui_styles_dispose(psy_ui_Styles*);

void psy_ui_styles_set_style(psy_ui_Styles*, uintptr_t styletype, psy_ui_Style*);
psy_ui_Style* psy_ui_styles_at(psy_ui_Styles* self, uintptr_t styletype);
const psy_ui_Style* psy_ui_styles_at_const(const psy_ui_Styles*,
	uintptr_t styletype);
void psy_ui_styles_configure(psy_ui_Styles*, psy_Property*);
const psy_Property* psy_ui_styles_configuration(const psy_ui_Styles*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_STYLES_H */

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_MATERIAL_H
#define psy_ui_MATERIAL_H

/* local */
#include "uistyle.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_MaterialTheme {
	psy_ui_Style* style;
	psy_ui_Colour surface;
	psy_ui_Colour surface_overlay_5p;
	psy_ui_Colour surface_overlay_7p;
	psy_ui_Colour surface_overlay_8p;
	psy_ui_Colour surface_overlay_9p;
	psy_ui_Colour surface_overlay_11p;
	psy_ui_Colour surface_overlay_12p;
	psy_ui_Colour surface_overlay_16p;
	psy_ui_Colour surface_overlay_24p;
	psy_ui_Colour primary;
	psy_ui_Colour secondary;
	psy_ui_Colour overlay;
	psy_ui_Colour overlay_1p;
	psy_ui_Colour overlay_4p;
	psy_ui_Colour overlay_12p;
	psy_ui_Colour onsurface;
	psy_ui_Colour onprimary;
	psy_ui_Colour onprimary_weak;
	psy_ui_Colour onprimary_medium;
	psy_ui_Colour onprimary_strong;
	psy_ui_Colour onsecondary;
	/* colour weight */
	int strong;
	int accent;
	int light;
	int medium;
	int weak;
	int pale;
	int minima;
	psy_ui_ThemeMode mode;
} psy_ui_MaterialTheme;

void psy_ui_materialtheme_init(psy_ui_MaterialTheme*, psy_ui_ThemeMode mode);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_MATERIAL_H */

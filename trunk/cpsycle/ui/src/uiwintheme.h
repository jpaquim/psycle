/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_WINTHEME_H
#define psy_ui_WINTHEME_H

/* local */
#include "uiborder.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_WinTheme {
	psy_ui_Colour cl_black;
	psy_ui_Colour cl_green;
	psy_ui_Colour cl_lime;
	psy_ui_Colour cl_teal;
	psy_ui_Colour cl_gray;
	psy_ui_Colour cl_medgray;
	psy_ui_Colour cl_silver;
	psy_ui_Colour cl_navy;
	psy_ui_Colour cl_blue;
	psy_ui_Colour cl_skyblue;
	psy_ui_Colour cl_aqua;
	psy_ui_Colour cl_white;
	psy_ui_Border raised;
	psy_ui_Border lowered;
} psy_ui_WinTheme;

void psy_ui_wintheme_init(psy_ui_WinTheme*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_MATERIAL_H */

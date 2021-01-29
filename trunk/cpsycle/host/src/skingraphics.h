// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(SKINGRAPHICS_H)
#define SKINGRAPHICS_H

#include "skincoord.h"
#include <uigraphics.h>

#ifdef __cplusplus
extern "C" {
#endif

void skin_blitcoord(psy_ui_Graphics*, psy_ui_Bitmap*, psy_ui_RealPoint dest, SkinCoord*);
void skin_blitcoordstretched(psy_ui_Graphics*, psy_ui_Bitmap*, psy_ui_RealPoint dest,
	double ratiowidth, double ratioheight,
	SkinCoord*);
void skin_stretchratio(SkinCoord*, double width, double height,
	double* ratiowidth, double* ratioheight);

#ifdef __cplusplus
}
#endif

#endif

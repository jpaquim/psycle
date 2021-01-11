// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(SKINCOORD_H)
#define SKINCOORD_H

#include "uidef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	double srcx;
	double srcy;
	double srcwidth;
	double srcheight;
	double destx;
	double desty;
	double destwidth;
	double destheight;
	double range;
} SkinCoord;

void skincoord_setall(SkinCoord*,
	double srcx,
	double srcy,
	double srcwidth,
	double srcheight,
	double destx,
	double desty,
	double destwidth,
	double destheight,
	double range);

void skincoord_setsource(SkinCoord* coord, intptr_t vals[4]);
void skincoord_setdest(SkinCoord* coord, intptr_t vals[4]);

INLINE psy_ui_Rectangle skincoord_destposition(SkinCoord* self)
{
	return psy_ui_rectangle_make(
		self->destx, self->desty,
		self->destwidth, self->destheight);
}

INLINE double skincoord_position(SkinCoord* coord, double value)
{
	return value * coord->range;
}

#ifdef __cplusplus
}
#endif

#endif

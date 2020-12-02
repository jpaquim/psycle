// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SKINCOORD_H)
#define SKINCOORD_H

#include "uidef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int srcx;
	int srcy;
	int srcwidth;
	int srcheight;
	int destx;
	int desty;
	int destwidth;
	int destheight;	
	int range;
} SkinCoord;

void skincoord_setall(SkinCoord*,
	int srcx,
	int srcy,
	int srcwidth,
	int srcheight,
	int destx,
	int desty,
	int destwidth,
	int destheight,
	int range);

void skincoord_setsource(SkinCoord* coord, int vals[4]);
void skincoord_setdest(SkinCoord* coord, int vals[4]);

INLINE psy_ui_Rectangle skincoord_destposition(SkinCoord* self)
{
	return psy_ui_rectangle_make(self->destx, self->desty, self->destwidth,
		self->destheight);
}

INLINE int skincoord_position(SkinCoord* coord, float value)
{
	return (int)(value * coord->range);
}

#ifdef __cplusplus
}
#endif

#endif

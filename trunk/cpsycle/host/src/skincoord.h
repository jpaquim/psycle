// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SKINCOORD_H)
#define SKINCOORD_H

#include "uidef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	intptr_t srcx;
	intptr_t srcy;
	intptr_t srcwidth;
	intptr_t srcheight;
	intptr_t destx;
	intptr_t desty;
	intptr_t destwidth;
	intptr_t destheight;
	intptr_t range;
} SkinCoord;

void skincoord_setall(SkinCoord*,
	intptr_t srcx,
	intptr_t srcy,
	intptr_t srcwidth,
	intptr_t srcheight,
	intptr_t destx,
	intptr_t desty,
	intptr_t destwidth,
	intptr_t destheight,
	intptr_t range);

void skincoord_setsource(SkinCoord* coord, intptr_t vals[4]);
void skincoord_setdest(SkinCoord* coord, intptr_t vals[4]);

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

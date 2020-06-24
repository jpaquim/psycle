// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "skincoord.h"

void skincoord_setall(SkinCoord* self,
	int srcx,
	int srcy,
	int srcwidth,
	int srcheight,
	int destx,
	int desty,
	int destwidth,
	int destheight,
	int range)
{
	self->srcx = srcx;
	self->srcy = srcy;
	self->srcwidth = srcwidth;
	self->srcheight = srcheight;
	self->destx = destx;
	self->desty = desty;
	self->destwidth = destwidth;
	self->destheight = destheight;
	self->range = range;
}

void skincoord_setsource(SkinCoord* coord, int vals[4])
{
	coord->srcx = vals[0];
	coord->srcy = vals[1];
	coord->srcwidth = vals[2];
	coord->srcheight = vals[3];
	coord->destwidth = vals[2];
	coord->destheight = vals[3];
}

void skincoord_setdest(SkinCoord* coord, int vals[4])
{
	coord->destx = vals[0];
	coord->desty = vals[1];
	coord->range = vals[2];
}
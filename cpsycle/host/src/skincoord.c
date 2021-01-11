// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "skincoord.h"

void skincoord_setall(SkinCoord* self,
	double srcx,
	double srcy,
	double srcwidth,
	double srcheight,
	double destx,
	double desty,
	double destwidth,
	double destheight,
	double range)
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

void skincoord_setsource(SkinCoord* coord, intptr_t vals[4])
{
	coord->srcx = (double)vals[0];
	coord->srcy = (double)vals[1];
	coord->srcwidth = (double)vals[2];
	coord->srcheight = (double)vals[3];
	coord->destwidth = (double)vals[2];
	coord->destheight = (double)vals[3];
}

void skincoord_setdest(SkinCoord* coord, intptr_t vals[4])
{
	coord->destx = (double)vals[0];
	coord->desty = (double)vals[1];
	coord->range = (double)vals[2];
}
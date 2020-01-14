// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SKINCOORD_H)
#define SKINCOORD_H

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

#endif

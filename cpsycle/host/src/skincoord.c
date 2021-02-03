// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "skincoord.h"

void skincoord_init(SkinCoord* self)
{
	self->src = psy_ui_realrectangle_zero();
	self->dest = psy_ui_realrectangle_zero();
	self->range = 0.0;
}

void skincoord_init_all(SkinCoord* self,
	double srcx, double srcy, double srcwidth, double srcheight,
	double destx, double desty, double destwidth, double destheight,
	double range)
{	
	self->src = psy_ui_realrectangle_make(psy_ui_realpoint_make(srcx, srcy),
		psy_ui_realsize_make(srcwidth, srcheight));
	self->dest = psy_ui_realrectangle_make(psy_ui_realpoint_make(destx, desty),
		psy_ui_realsize_make(destwidth, destheight));
	self->range = range;
}

void skincoord_setsource(SkinCoord* coord, intptr_t vals[4])
{
	coord->src.left = (double)vals[0];
	coord->src.top = (double)vals[1];	
	psy_ui_realrectangle_resize(&coord->src, (double)vals[2], (double)vals[3]);	
	psy_ui_realrectangle_resize(&coord->dest, (double)vals[2], (double)vals[3]);
}

void skincoord_setdest(SkinCoord* coord, intptr_t vals[4], uintptr_t num)
{
	coord->dest.left = (double)vals[0];
	coord->dest.top = (double)vals[1];
	if (num == 2) {
		coord->range = 0;		
	} else if (num == 3) {
		coord->range = (double)vals[2];		
	}
	psy_ui_realrectangle_setwidth(&coord->dest,
		psy_ui_realrectangle_width(&coord->src));
	psy_ui_realrectangle_setheight(&coord->dest,
		psy_ui_realrectangle_height(&coord->src));
}

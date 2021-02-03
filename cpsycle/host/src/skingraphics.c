// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "skingraphics.h"

void skin_blitcoord(psy_ui_Graphics* g, psy_ui_Bitmap* bitmap,
	psy_ui_RealPoint dest, SkinCoord* coord)
{
	psy_ui_drawbitmap(g, bitmap,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(dest.x + coord->dest.left, dest.y + coord->dest.top),
			psy_ui_realrectangle_size(&coord->dest)),
		psy_ui_realrectangle_topleft(&coord->src));
}

void skin_transblitcoord(psy_ui_Graphics* g, psy_ui_Bitmap* bitmap,
	psy_ui_RealPoint dest, SkinCoord* coord)
{
	psy_ui_drawbitmap(g, bitmap,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(dest.x + coord->dest.left, dest.y + coord->dest.top),
			psy_ui_realrectangle_size(&coord->dest)),
		psy_ui_realrectangle_topleft(&coord->src));
}

void skin_blitcoordstretched(psy_ui_Graphics* g, psy_ui_Bitmap* bitmap,
	psy_ui_RealPoint dest, double ratiowidth, double ratioheight,
	SkinCoord* coord)
{
	psy_ui_drawstretchedbitmap(g, bitmap,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				dest.x + coord->dest.left,
				dest.y + coord->dest.top),
			psy_ui_realsize_make(
				psy_ui_realrectangle_width(&coord->dest) * ratiowidth,
				psy_ui_realrectangle_height(&coord->dest) * ratioheight)),
			psy_ui_realpoint_make(coord->src.left, coord->src.top),
			psy_ui_realrectangle_size(&coord->dest));
}

void skin_stretchratio(SkinCoord* coord, double width, double height,
	double* ratiowidth, double* ratioheight)
{
	*ratiowidth = width / psy_ui_realrectangle_width(&coord->dest);
	*ratioheight = height / psy_ui_realrectangle_height(&coord->dest);
}


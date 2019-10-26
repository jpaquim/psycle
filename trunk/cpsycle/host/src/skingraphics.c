// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "skingraphics.h"

void skin_blitpart(ui_graphics* g, ui_bitmap* bitmap, int x, int y,
	SkinCoord* coord)
{
	ui_drawbitmap(g, bitmap, x + coord->destx, y + coord->desty,
		coord->destwidth, coord->destheight, coord->srcx, coord->srcy);
}


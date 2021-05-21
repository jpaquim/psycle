/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uix11colours.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11

/* X11 */
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
/*std */
#include <assert.h>
#include <stdio.h>


void psy_ui_x11colours_init(psy_ui_X11Colours* self, Display* dpy)
{	
	assert(dpy);
	
	self->dpy = dpy;
	psy_table_init(&self->colourmap);	
}

void psy_ui_x11colours_dispose(psy_ui_X11Colours* self)
{	
	self->dpy = NULL;
	psy_table_dispose(&self->colourmap);
}

int psy_ui_x11colours_index(psy_ui_X11Colours* self, psy_ui_Colour colour)
{
	int rv;
	XColor xcolor;
	uint8_t r;
	uint8_t g;
	uint8_t b;

	if (psy_table_exists(&self->colourmap,
			(uintptr_t)psy_ui_colour_colorref(&colour))) {
		rv = (int)(intptr_t)psy_table_at(&self->colourmap,
			(uintptr_t)psy_ui_colour_colorref(&colour));
	} else {
		psy_ui_colour_rgb(&colour, &r, &g, &b);
		xcolor.red = r * 256;
		xcolor.green = g * 256;
		xcolor.blue = b * 256;
		if (XAllocColor(self->dpy, DefaultColormap(self->dpy,
				DefaultScreen(self->dpy)), &xcolor)) {
			rv = xcolor.pixel;
		} else {
			rv = BlackPixel(self->dpy, DefaultScreen(self->dpy));
			printf("Warning: can't allocate requested colour\n");
		}
		psy_table_insert(&self->colourmap,
			(uintptr_t)psy_ui_colour_colorref(&colour),
			(void*)(uintptr_t)rv);
	}
	return rv;
}

#endif /* PSYCLE_TK_X11 */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT
#ifndef psy_ui_x11_GRAPHICSIMP_H
#define psy_ui_x11_GRAPHICSIMP_H

#include "uigraphics.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GC gc;
	Display* display;
	Window window;
} PlatformXtGC;

typedef struct psy_ui_x11_GraphicsImp {
	psy_ui_GraphicsImp imp;
	GC gc;
	Display* display;
	Window window;
	XftDraw* xfd;
	XftColor black;
	XftFont* xftfont;
	XftColor textcolor;
	psy_ui_Color textbackgroundcolor;
	unsigned int backgroundmode;
	int dx;
	int dy;
	/*HDC hdc;  
	HFONT hfont;
	HFONT hFontPrev;	
	HPEN pen;  
	HPEN penprev;
	HBRUSH brush;
	HBRUSH hBrushPrev;*/
} psy_ui_x11_GraphicsImp;

void psy_ui_x11_graphicsimp_init(psy_ui_x11_GraphicsImp* self, 
  PlatformXtGC*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_x11_GRAPHICSIMP_H */
#endif /* PSYCLE_TK_XT */

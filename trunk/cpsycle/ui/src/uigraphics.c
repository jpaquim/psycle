// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "uigraphics.h"
#include "uiapp.h"
#include "uiimpfactory.h"
#include <stdlib.h>

extern psy_ui_App app;

// VTable Prototypes
static void dispose(psy_ui_Graphics*);
static void textout(psy_ui_Graphics*, intptr_t x, intptr_t y,  const char* text, size_t len);
static void textoutrectangle(psy_ui_Graphics*, intptr_t x, intptr_t y, uintptr_t options,
	psy_ui_Rectangle r, const char* text, uintptr_t len);
static void drawrectangle(psy_ui_Graphics*, const psy_ui_Rectangle);
static void drawroundrectangle(psy_ui_Graphics*, const psy_ui_Rectangle,
	psy_ui_Size cornersize);
static psy_ui_Size textsize(psy_ui_Graphics*, const char* text);
static void drawsolidrectangle(psy_ui_Graphics*, const psy_ui_Rectangle,
	psy_ui_Colour colour);
static void drawsolidroundrectangle(psy_ui_Graphics*, const psy_ui_Rectangle,
	psy_ui_Size cornersize, psy_ui_Colour colour);
static void drawsolidpolygon(psy_ui_Graphics*, psy_ui_IntPoint* points,
	uintptr_t numpoints, uint32_t inner, uint32_t outter);
static void drawline(psy_ui_Graphics*, intptr_t x1, intptr_t y1, intptr_t x2, intptr_t y2);
static void drawfullbitmap(psy_ui_Graphics*, psy_ui_Bitmap* bmp, intptr_t x, intptr_t y);
static void drawbitmap(psy_ui_Graphics*, psy_ui_Bitmap* bmp, intptr_t x, intptr_t y, intptr_t width,
	intptr_t height, intptr_t xsrc, intptr_t ysrc);
static void drawstretchedbitmap(psy_ui_Graphics*, psy_ui_Bitmap* bmp, intptr_t x, intptr_t y, intptr_t width,
	intptr_t height, intptr_t xsrc, intptr_t ysrc, intptr_t wsrc, intptr_t hsrc);
static void setbackgroundcolour(psy_ui_Graphics*, psy_ui_Colour colour);
static void setbackgroundmode(psy_ui_Graphics*, uintptr_t mode);
static void settextcolour(psy_ui_Graphics*, psy_ui_Colour colour);
static void settextalign(psy_ui_Graphics*, uintptr_t align);
static void setcolour(psy_ui_Graphics*, psy_ui_Colour colour);
static void setfont(psy_ui_Graphics*, psy_ui_Font* font);
static void moveto(psy_ui_Graphics*, psy_ui_IntPoint point);
static void curveto(psy_ui_Graphics*, psy_ui_IntPoint control_p1,
	psy_ui_IntPoint control_p2, psy_ui_IntPoint p);
static void drawarc(psy_ui_Graphics*,
	intptr_t x1, intptr_t y1, intptr_t x2, intptr_t y2, intptr_t x3, intptr_t y3, intptr_t x4, intptr_t y4);
static void setlinewidth(psy_ui_Graphics*, uintptr_t width);
static uintptr_t linewidth(psy_ui_Graphics*);

// VTable init
static psy_ui_GraphicsVTable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.dispose = dispose;
		vtable.textout = textout;
		vtable.textoutrectangle = textoutrectangle;
		vtable.drawrectangle = drawrectangle;
		vtable.drawroundrectangle = drawroundrectangle;
		vtable.textsize = textsize;
		vtable.drawsolidrectangle = drawsolidrectangle;
		vtable.drawsolidroundrectangle = drawsolidroundrectangle;
		vtable.drawsolidpolygon = drawsolidpolygon;
		vtable.drawline = drawline;		
		vtable.drawfullbitmap = drawfullbitmap;
		vtable.drawbitmap = drawbitmap;
		vtable.drawstretchedbitmap = drawstretchedbitmap;
		vtable.setbackgroundcolour = setbackgroundcolour;
		vtable.setbackgroundmode = setbackgroundmode;
		vtable.settextcolour = settextcolour;
		vtable.settextalign = settextalign;
		vtable.setcolour = setcolour;
		vtable.setfont = setfont;
		vtable.moveto = moveto;
		vtable.curveto = curveto;
		vtable.drawarc = drawarc;
		vtable.setlinewidth = setlinewidth;
		vtable.linewidth = linewidth;
		vtable_initialized = TRUE;
	}
}

void psy_ui_graphics_init(psy_ui_Graphics* self, void* hdc)
{
	vtable_init();
	self->vtable = &vtable;
	self->imp = psy_ui_impfactory_allocinit_graphicsimp(psy_ui_app_impfactory(&app), hdc);
}

// Delegation Methods to GraphicsImp
static void dispose(psy_ui_Graphics* self)
{
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = 0;
}

static void textout(psy_ui_Graphics* self, intptr_t x, intptr_t y,  const char* text, uintptr_t len)
{
	self->imp->vtable->dev_textout(self->imp, x, y, text, len);
}
static void textoutrectangle(psy_ui_Graphics* self, intptr_t x, intptr_t y, uintptr_t options,
	psy_ui_Rectangle r, const char* text, uintptr_t len)
{
	self->imp->vtable->dev_textoutrectangle(self->imp, x, y, options, r, text, len);
}
static void drawrectangle(psy_ui_Graphics* self, const psy_ui_Rectangle r)
{
	self->imp->vtable->dev_drawrectangle(self->imp, r);
}
static void drawroundrectangle(psy_ui_Graphics* self, const psy_ui_Rectangle r,
	psy_ui_Size cornersize)
{
		self->imp->vtable->dev_drawroundrectangle(self->imp, r, cornersize);
}

static psy_ui_Size textsize(psy_ui_Graphics* self, const char* text)
{
	return self->imp->vtable->dev_textsize(self->imp, text);
}

static void drawsolidrectangle(psy_ui_Graphics* self, const psy_ui_Rectangle r,
	psy_ui_Colour colour)
{
	self->imp->vtable->dev_drawsolidrectangle(self->imp, r, colour);
}

static void drawsolidroundrectangle(psy_ui_Graphics* self, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, psy_ui_Colour colour)
{
		self->imp->vtable->dev_drawsolidroundrectangle(self->imp, r, cornersize, colour);
}
static void drawsolidpolygon(psy_ui_Graphics* self, psy_ui_IntPoint* points,
	uintptr_t numpoints, uint32_t inner, uint32_t outter)
{
	self->imp->vtable->dev_drawsolidpolygon(self->imp, points, numpoints, inner, outter);
}

static void drawline(psy_ui_Graphics* self, intptr_t x1, intptr_t y1, intptr_t x2, intptr_t y2)
{
	self->imp->vtable->dev_drawline(self->imp, x1, y1, x2, y2);
}

static void drawfullbitmap(psy_ui_Graphics* self, psy_ui_Bitmap* bmp, intptr_t x, intptr_t y)
{
	self->imp->vtable->dev_drawfullbitmap(self->imp, bmp, x, y);
}

static void drawbitmap(psy_ui_Graphics* self, psy_ui_Bitmap* bmp, intptr_t x, intptr_t y, intptr_t width,
	intptr_t height, intptr_t xsrc, intptr_t ysrc)
{
	self->imp->vtable->dev_drawbitmap(self->imp, bmp, x, y, width, height, xsrc, ysrc);
}

static void drawstretchedbitmap(psy_ui_Graphics* self, psy_ui_Bitmap* bmp, intptr_t x, intptr_t y, intptr_t width,
	intptr_t height, intptr_t xsrc, intptr_t ysrc, intptr_t wsrc, intptr_t hsrc)
{	
	self->imp->vtable->dev_drawstretchedbitmap(self->imp, bmp, x, y, width,
		height, xsrc, ysrc, wsrc, hsrc);
}

static void setbackgroundcolour(psy_ui_Graphics* self, psy_ui_Colour colour)
{
	self->imp->vtable->dev_setbackgroundcolour(self->imp, colour);
}

static void setbackgroundmode(psy_ui_Graphics* self, uintptr_t mode)
{
	self->imp->vtable->dev_setbackgroundmode(self->imp, mode);
}

static void settextcolour(psy_ui_Graphics* self, psy_ui_Colour colour)
{
	self->imp->vtable->dev_settextcolour(self->imp, colour);
}

static void settextalign(psy_ui_Graphics* self, uintptr_t align)
{
	self->imp->vtable->dev_settextalign(self->imp, align);
}

static void setcolour(psy_ui_Graphics* self, psy_ui_Colour colour)
{
	self->imp->vtable->dev_setcolour(self->imp, colour);
}

static void setfont(psy_ui_Graphics* self, psy_ui_Font* font)
{
	self->imp->vtable->dev_setfont(self->imp, font);
}

static void moveto(psy_ui_Graphics* self, psy_ui_IntPoint point)
{
	self->imp->vtable->dev_moveto(self->imp, point);
}

static void curveto(psy_ui_Graphics* self, psy_ui_IntPoint control_p1,
	psy_ui_IntPoint control_p2, psy_ui_IntPoint p)
{
	self->imp->vtable->dev_curveto(self->imp, control_p1, control_p2, p);
}

static void drawarc(psy_ui_Graphics* self,
	intptr_t x1, intptr_t y1, intptr_t x2, intptr_t y2, intptr_t x3, intptr_t y3, intptr_t x4, intptr_t y4)
{
	self->imp->vtable->dev_drawarc(self->imp, x1, y1, x2, y2, x3, y3, x4, y4);
}

static void setlinewidth(psy_ui_Graphics* self, uintptr_t width)
{
	self->imp->vtable->dev_setlinewidth(self->imp, width);
}

static uintptr_t linewidth(psy_ui_Graphics* self)
{
	return self->imp->vtable->dev_linewidth(self->imp);
}

// psy_ui_GraphicsImp
static void dev_dispose(psy_ui_GraphicsImp* self) { }
static void dev_textout(psy_ui_GraphicsImp* self, intptr_t x, intptr_t y, const char* text, uintptr_t len) { }
static void dev_textoutrectangle(psy_ui_GraphicsImp* self, intptr_t x, intptr_t y, uintptr_t options,
	psy_ui_Rectangle r, const char* text, uintptr_t len){ }
static void dev_drawrectangle(psy_ui_GraphicsImp* self, const psy_ui_Rectangle r) { }
static void dev_drawroundrectangle(psy_ui_GraphicsImp* self, const psy_ui_Rectangle r,
	psy_ui_Size cornersize) { }
static psy_ui_Size dev_textsize(psy_ui_GraphicsImp* self, const char* text)
{
	psy_ui_Size rv = {0, 0};

	return rv;
}
static void dev_drawsolidrectangle(psy_ui_GraphicsImp* self, const psy_ui_Rectangle r,
	psy_ui_Colour colour) { }
static void dev_drawsolidroundrectangle(psy_ui_GraphicsImp* self, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, psy_ui_Colour colour) { }
static void dev_drawsolidpolygon(psy_ui_GraphicsImp* self, psy_ui_IntPoint* pt,
	uintptr_t numpoints, uint32_t inner, uint32_t outter) { }
static void dev_drawline(psy_ui_GraphicsImp* self, intptr_t x1, intptr_t y1, intptr_t x2, intptr_t y2) { }
static void dev_drawfullbitmap(psy_ui_GraphicsImp* self, psy_ui_Bitmap* bmp, intptr_t x, intptr_t y) { }
static void dev_drawbitmap(psy_ui_GraphicsImp* self, psy_ui_Bitmap* bmp, intptr_t x, intptr_t y, intptr_t width,
	intptr_t height, intptr_t xsrc, intptr_t ysrc) { }
static void dev_drawstretchedbitmap(psy_ui_GraphicsImp* self, psy_ui_Bitmap* bmp, intptr_t x, intptr_t y, intptr_t width,
	intptr_t height, intptr_t xsrc, intptr_t ysrc, intptr_t wsrc, intptr_t hsrc) { }
static void dev_setbackgroundcolour(psy_ui_GraphicsImp* self, psy_ui_Colour colour) { }
static void dev_setbackgroundmode(psy_ui_GraphicsImp* self, uintptr_t mode) { }
static void dev_settextcolour(psy_ui_GraphicsImp* self, psy_ui_Colour colour) { }
static void dev_settextalign(psy_ui_GraphicsImp* self, uintptr_t align) { }
static void dev_setcolour(psy_ui_GraphicsImp* self, psy_ui_Colour colour) { }
static void dev_setfont(psy_ui_GraphicsImp* self, psy_ui_Font* font) { }
static void dev_moveto(psy_ui_GraphicsImp* self, psy_ui_IntPoint pt) { }
static void dev_curveto(psy_ui_GraphicsImp* self, psy_ui_IntPoint control_p1,
	psy_ui_IntPoint control_p2, psy_ui_IntPoint p) { }
static void dev_drawarc(psy_ui_GraphicsImp* self,
	intptr_t x1, intptr_t y1, intptr_t x2, intptr_t y2, intptr_t x3, intptr_t y3, intptr_t x4, intptr_t y4) { }
static void dev_setlinewidth(psy_ui_GraphicsImp* self, uintptr_t width) { }
static unsigned int dev_linewidth(psy_ui_GraphicsImp* self) { return 1; }

static psy_ui_GraphicsImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = dev_dispose;
		imp_vtable.dev_textout = dev_textout;
		imp_vtable.dev_textoutrectangle = dev_textoutrectangle;
		imp_vtable.dev_drawrectangle = dev_drawrectangle;
		imp_vtable.dev_drawroundrectangle = dev_drawroundrectangle;
		imp_vtable.dev_textsize = dev_textsize;
		imp_vtable.dev_drawsolidrectangle = dev_drawsolidrectangle;
		imp_vtable.dev_drawsolidroundrectangle = dev_drawsolidroundrectangle;
		imp_vtable.dev_drawsolidpolygon = dev_drawsolidpolygon;
		imp_vtable.dev_drawline = dev_drawline;		
		imp_vtable.dev_drawfullbitmap = dev_drawfullbitmap;
		imp_vtable.dev_drawbitmap = dev_drawbitmap;
		imp_vtable.dev_drawstretchedbitmap = dev_drawstretchedbitmap;
		imp_vtable.dev_setbackgroundcolour = dev_setbackgroundcolour;
		imp_vtable.dev_setbackgroundmode = dev_setbackgroundmode;
		imp_vtable.dev_settextcolour = dev_settextcolour;
		imp_vtable.dev_settextalign = dev_settextalign;
		imp_vtable.dev_setcolour = dev_setcolour;
		imp_vtable.dev_setfont = dev_setfont;
		imp_vtable.dev_moveto = dev_moveto;
		imp_vtable.dev_curveto = dev_curveto;
		imp_vtable.dev_drawarc = dev_drawarc;
		imp_vtable.dev_setlinewidth = dev_setlinewidth;
		imp_vtable.dev_linewidth = dev_linewidth;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_graphics_imp_init(psy_ui_GraphicsImp* self)
{
	imp_vtable_init();	
	self->vtable = &imp_vtable;
}

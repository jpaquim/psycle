// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "uiapp.h"
#include "uigraphics.h"
#include "uiimpfactory.h"
#include <stdlib.h>

extern psy_ui_App app;

// VTable Prototypes
static void dispose(psy_ui_Graphics*);
static void textout(psy_ui_Graphics*, int x, int y,  const char* text, size_t len);
static void textoutrectangle(psy_ui_Graphics*, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len);
static void drawrectangle(psy_ui_Graphics*, const psy_ui_Rectangle);
static void drawroundrectangle(psy_ui_Graphics*, const psy_ui_Rectangle,
	psy_ui_Size cornersize);
static psy_ui_Size textsize(psy_ui_Graphics*, const char* text);
static void drawsolidrectangle(psy_ui_Graphics*, const psy_ui_Rectangle,
	unsigned int color);
static void drawsolidroundrectangle(psy_ui_Graphics*, const psy_ui_Rectangle,
	psy_ui_Size cornersize, unsigned int color);
static void drawsolidpolygon(psy_ui_Graphics*, psy_ui_Point* points,
	unsigned int numpoints,  unsigned int inner, unsigned int outter);
static void drawline(psy_ui_Graphics*, int x1, int y1, int x2, int y2);
static void drawfullbitmap(psy_ui_Graphics*, psy_ui_Bitmap* bmp, int x, int y);
static void drawbitmap(psy_ui_Graphics*, psy_ui_Bitmap* bmp, int x, int y, int width,
	int height, int xsrc, int ysrc);
static void setbackgroundcolor(psy_ui_Graphics*, unsigned int color);
static void setbackgroundmode(psy_ui_Graphics*, unsigned int mode);
static void settextcolor(psy_ui_Graphics*, unsigned int color);
static void setcolor(psy_ui_Graphics*, unsigned int color);
static void setfont(psy_ui_Graphics*, psy_ui_Font* font);
static void moveto(psy_ui_Graphics*, psy_ui_Point point);
static void devcurveto(psy_ui_Graphics*, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p);

// VTable init
static psy_ui_GraphicsVTable vtable;
static int vtable_initialized = 0;

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
		vtable.setbackgroundcolor = setbackgroundcolor;
		vtable.setbackgroundmode = setbackgroundmode;
		vtable.settextcolor = settextcolor;
		vtable.setcolor = setcolor;
		vtable.setfont = setfont;
		vtable.moveto = moveto;
		vtable.devcurveto = devcurveto;
		vtable_initialized = 1;
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

static void textout(psy_ui_Graphics* self, int x, int y,  const char* text, size_t len)
{
	self->imp->vtable->dev_textout(self->imp, x, y, text, len);
}
static void textoutrectangle(psy_ui_Graphics* self, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len)
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
	unsigned int color)
{
	self->imp->vtable->dev_drawsolidrectangle(self->imp, r, color);
}

static void drawsolidroundrectangle(psy_ui_Graphics* self, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color)
{
		self->imp->vtable->dev_drawsolidroundrectangle(self->imp, r, cornersize, color);
}
static void drawsolidpolygon(psy_ui_Graphics* self, psy_ui_Point* points,
	unsigned int numpoints,  unsigned int inner, unsigned int outter)
{
	self->imp->vtable->dev_drawsolidpolygon(self->imp, points, numpoints, inner, outter);
}

static void drawline(psy_ui_Graphics* self, int x1, int y1, int x2, int y2)
{
	self->imp->vtable->dev_drawline(self->imp, x1, y1, x2, y2);
}

static void drawfullbitmap(psy_ui_Graphics* self, psy_ui_Bitmap* bmp, int x, int y)
{
	self->imp->vtable->dev_drawfullbitmap(self->imp, bmp, x, y);
}

static void drawbitmap(psy_ui_Graphics* self, psy_ui_Bitmap* bmp, int x, int y, int width,
	int height, int xsrc, int ysrc)
{
	self->imp->vtable->dev_drawbitmap(self->imp, bmp, x, y, width, height, xsrc, ysrc);
}

static void setbackgroundcolor(psy_ui_Graphics* self, unsigned int color)
{
	self->imp->vtable->dev_setbackgroundcolor(self->imp, color);
}

static void setbackgroundmode(psy_ui_Graphics* self, unsigned int mode)
{
	self->imp->vtable->dev_setbackgroundmode(self->imp, mode);
}

static void settextcolor(psy_ui_Graphics* self, unsigned int color)
{
	self->imp->vtable->dev_settextcolor(self->imp, color);
}

static void setcolor(psy_ui_Graphics* self, unsigned int color)
{
	self->imp->vtable->dev_setcolor(self->imp, color);
}

static void setfont(psy_ui_Graphics* self, psy_ui_Font* font)
{
	self->imp->vtable->dev_setfont(self->imp, font);
}

static void moveto(psy_ui_Graphics* self, psy_ui_Point point)
{
	self->imp->vtable->dev_moveto(self->imp, point);
}

static void devcurveto(psy_ui_Graphics* self, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p)
{
	self->imp->vtable->dev_devcurveto(self->imp, control_p1, control_p2, p);
}

// psy_ui_GraphicsImp

static void psy_ui_imp_dispose(psy_ui_GraphicsImp* self) { }
static void psy_ui_imp_textout(psy_ui_GraphicsImp* self, int x, int y,  const char* text, size_t len) { }
static void psy_ui_imp_textoutrectangle(psy_ui_GraphicsImp* self, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len){ }
static void psy_ui_imp_drawrectangle(psy_ui_GraphicsImp* self, const psy_ui_Rectangle r) { }
static void psy_ui_imp_drawroundrectangle(psy_ui_GraphicsImp* self, const psy_ui_Rectangle r,
	psy_ui_Size cornersize) { }
static psy_ui_Size psy_ui_imp_textsize(psy_ui_GraphicsImp* self, const char* text)
{
	psy_ui_Size rv = {0, 0};

	return rv;
}
static void psy_ui_imp_drawsolidrectangle(psy_ui_GraphicsImp* self, const psy_ui_Rectangle r,
	unsigned int color) { }
static void psy_ui_imp_drawsolidroundrectangle(psy_ui_GraphicsImp* self, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color) { }
static void psy_ui_imp_drawsolidpolygon(psy_ui_GraphicsImp* self, psy_ui_Point* pt,
	unsigned int numpoints, unsigned int inner, unsigned int outter) { }
static void psy_ui_imp_drawline(psy_ui_GraphicsImp* self, int x1, int y1, int x2, int y2) { }
static void psy_ui_imp_drawfullbitmap(psy_ui_GraphicsImp* self, psy_ui_Bitmap* bmp, int x, int y) { }
static void psy_ui_imp_drawbitmap(psy_ui_GraphicsImp* self, psy_ui_Bitmap* bmp, int x, int y, int width,
	int height, int xsrc, int ysrc) { }
static void psy_ui_imp_setbackgroundcolor(psy_ui_GraphicsImp* self, unsigned int color) { }
static void psy_ui_imp_setbackgroundmode(psy_ui_GraphicsImp* self, unsigned int mode) { }
static void psy_ui_imp_settextcolor(psy_ui_GraphicsImp* self, unsigned int color) { }
static void psy_ui_imp_setcolor(psy_ui_GraphicsImp* self, unsigned int color) { }
static void psy_ui_imp_setfont(psy_ui_GraphicsImp* self, psy_ui_Font* font) { }
static void psy_ui_imp_moveto(psy_ui_GraphicsImp* self, psy_ui_Point pt) { }
static void psy_ui_imp_devcurveto(psy_ui_GraphicsImp* self, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p) { }

static psy_ui_GraphicsImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = psy_ui_imp_dispose;
		imp_vtable.dev_textout = psy_ui_imp_textout;
		imp_vtable.dev_textoutrectangle = psy_ui_imp_textoutrectangle;
		imp_vtable.dev_drawrectangle = psy_ui_imp_drawrectangle;
		imp_vtable.dev_drawroundrectangle = psy_ui_imp_drawroundrectangle;
		imp_vtable.dev_textsize = psy_ui_imp_textsize;
		imp_vtable.dev_drawsolidrectangle = psy_ui_imp_drawsolidrectangle;
		imp_vtable.dev_drawsolidroundrectangle = psy_ui_imp_drawsolidroundrectangle;
		imp_vtable.dev_drawsolidpolygon = psy_ui_imp_drawsolidpolygon;
		imp_vtable.dev_drawline = psy_ui_imp_drawline;		
		imp_vtable.dev_drawfullbitmap = psy_ui_imp_drawfullbitmap;
		imp_vtable.dev_drawbitmap = psy_ui_imp_drawbitmap;
		imp_vtable.dev_setbackgroundcolor = psy_ui_imp_setbackgroundcolor;
		imp_vtable.dev_setbackgroundmode = psy_ui_imp_setbackgroundmode;
		imp_vtable.dev_settextcolor = psy_ui_imp_settextcolor;
		imp_vtable.dev_setcolor = psy_ui_imp_setcolor;
		imp_vtable.dev_setfont = psy_ui_imp_setfont;
		imp_vtable.dev_moveto = psy_ui_imp_moveto;
		imp_vtable.dev_devcurveto = psy_ui_imp_devcurveto;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_graphics_imp_init(psy_ui_GraphicsImp* self)
{
	imp_vtable_init();	
	self->vtable = &imp_vtable;
}

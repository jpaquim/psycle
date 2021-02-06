// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "uigraphics.h"
// local
#include "uiapp.h"
#include "uiimpfactory.h"
// std
#include <stdlib.h>

// VTable Prototypes
static void dispose(psy_ui_Graphics*);
static void textout(psy_ui_Graphics*, double x, double y,  const char* text, uintptr_t len);
static void textoutrectangle(psy_ui_Graphics*, double x, double y, uintptr_t options,
	psy_ui_RealRectangle r, const char* text, uintptr_t len);
static void drawrectangle(psy_ui_Graphics*, const psy_ui_RealRectangle);
static void drawroundrectangle(psy_ui_Graphics*, const psy_ui_RealRectangle,
	psy_ui_Size cornersize);
static psy_ui_Size textsize(psy_ui_Graphics*, const char* text);
static void drawsolidrectangle(psy_ui_Graphics*, const psy_ui_RealRectangle,
	psy_ui_Colour colour);
static void drawsolidroundrectangle(psy_ui_Graphics*, const psy_ui_RealRectangle,
	psy_ui_Size cornersize, psy_ui_Colour colour);
static void drawsolidpolygon(psy_ui_Graphics*, psy_ui_RealPoint* points,
	uintptr_t numpoints, uint32_t inner, uint32_t outter);
static void drawline(psy_ui_Graphics*, double x1, double y1, double x2, double y2);
static void drawfullbitmap(psy_ui_Graphics*, psy_ui_Bitmap* bmp, double x, double y);
static void drawbitmap(psy_ui_Graphics*, psy_ui_Bitmap* bmp, double x, double y, double width,
	double height, double xsrc, double ysrc);
static void drawstretchedbitmap(psy_ui_Graphics*, psy_ui_Bitmap* bmp, double x, double y, double width,
	double height, double xsrc, double ysrc, double wsrc, double hsrc);
static void setbackgroundcolour(psy_ui_Graphics*, psy_ui_Colour);
static void setbackgroundmode(psy_ui_Graphics*, uintptr_t mode);
static void settextcolour(psy_ui_Graphics*, psy_ui_Colour);
static void settextalign(psy_ui_Graphics*, uintptr_t align);
static void setcolour(psy_ui_Graphics*, psy_ui_Colour);
static void setfont(psy_ui_Graphics*, const psy_ui_Font*);
static void moveto(psy_ui_Graphics*, psy_ui_RealPoint point);
static void curveto(psy_ui_Graphics*, psy_ui_RealPoint control_p1,
	psy_ui_RealPoint control_p2, psy_ui_RealPoint p);
static void drawarc(psy_ui_Graphics*,
	double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);
static void setlinewidth(psy_ui_Graphics*, uintptr_t width);
static uintptr_t linewidth(psy_ui_Graphics*);
static void setorigin(psy_ui_Graphics* self, double x, double y);

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
		vtable.setorigin = setorigin;
		vtable_initialized = TRUE;
	}
}

void psy_ui_graphics_init(psy_ui_Graphics* self, void* hdc)
{
	vtable_init();
	self->vtable = &vtable;
	self->imp = psy_ui_impfactory_allocinit_graphicsimp(psy_ui_app_impfactory(psy_ui_app()), hdc);
}

void psy_ui_graphics_init_bitmap(psy_ui_Graphics* self, psy_ui_Bitmap* bitmap)
{
	vtable_init();
	self->vtable = &vtable;
	self->imp = psy_ui_impfactory_allocinit_graphicsimp_bitmap(psy_ui_app_impfactory(psy_ui_app()), bitmap);
}

// Delegation Methods to GraphicsImp
void dispose(psy_ui_Graphics* self)
{
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = 0;
}

void textout(psy_ui_Graphics* self, double x, double y,  const char* text, uintptr_t len)
{
	self->imp->vtable->dev_textout(self->imp, x, y, text, len);
}

void textoutrectangle(psy_ui_Graphics* self, double x, double y, uintptr_t options,
	psy_ui_RealRectangle r, const char* text, uintptr_t len)
{
	self->imp->vtable->dev_textoutrectangle(self->imp, x, y, options, r, text, len);
}

void drawrectangle(psy_ui_Graphics* self, const psy_ui_RealRectangle r)
{
	self->imp->vtable->dev_drawrectangle(self->imp, r);
}

void drawroundrectangle(psy_ui_Graphics* self, const psy_ui_RealRectangle r,
	psy_ui_Size cornersize)
{
		self->imp->vtable->dev_drawroundrectangle(self->imp, r, cornersize);
}

psy_ui_Size textsize(psy_ui_Graphics* self, const char* text)
{
	return self->imp->vtable->dev_textsize(self->imp, text);
}

void drawsolidrectangle(psy_ui_Graphics* self, const psy_ui_RealRectangle r,
	psy_ui_Colour colour)
{
	self->imp->vtable->dev_drawsolidrectangle(self->imp, r, colour);
}

void drawsolidroundrectangle(psy_ui_Graphics* self, const psy_ui_RealRectangle r,
	psy_ui_Size cornersize, psy_ui_Colour colour)
{
		self->imp->vtable->dev_drawsolidroundrectangle(self->imp, r, cornersize, colour);
}

void drawsolidpolygon(psy_ui_Graphics* self, psy_ui_RealPoint* points,
	uintptr_t numpoints, uint32_t inner, uint32_t outter)
{
	self->imp->vtable->dev_drawsolidpolygon(self->imp, points, numpoints, inner, outter);
}

void drawline(psy_ui_Graphics* self, double x1, double y1, double x2, double y2)
{
	self->imp->vtable->dev_drawline(self->imp, x1, y1, x2, y2);
}

void drawfullbitmap(psy_ui_Graphics* self, psy_ui_Bitmap* bmp, double x, double y)
{
	self->imp->vtable->dev_drawfullbitmap(self->imp, bmp, x, y);
}

void drawbitmap(psy_ui_Graphics* self, psy_ui_Bitmap* bmp, double x, double y, double width,
	double height, double xsrc, double ysrc)
{
	self->imp->vtable->dev_drawbitmap(self->imp, bmp, x, y, width, height, xsrc, ysrc);
}

void drawstretchedbitmap(psy_ui_Graphics* self, psy_ui_Bitmap* bmp, double x, double y, double width,
	double height, double xsrc, double ysrc, double wsrc, double hsrc)
{	
	self->imp->vtable->dev_drawstretchedbitmap(self->imp, bmp, x, y, width,
		height, xsrc, ysrc, wsrc, hsrc);
}

void setbackgroundcolour(psy_ui_Graphics* self, psy_ui_Colour colour)
{
	self->imp->vtable->dev_setbackgroundcolour(self->imp, colour);
}

void setbackgroundmode(psy_ui_Graphics* self, uintptr_t mode)
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

void setfont(psy_ui_Graphics* self, const psy_ui_Font* font)
{
	self->imp->vtable->dev_setfont(self->imp, font);
}

static void moveto(psy_ui_Graphics* self, psy_ui_RealPoint point)
{
	self->imp->vtable->dev_moveto(self->imp, point);
}

static void curveto(psy_ui_Graphics* self, psy_ui_RealPoint control_p1,
	psy_ui_RealPoint control_p2, psy_ui_RealPoint p)
{
	self->imp->vtable->dev_curveto(self->imp, control_p1, control_p2, p);
}

void drawarc(psy_ui_Graphics* self,
	double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
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

static void setorigin(psy_ui_Graphics* self, double x, double y)
{
	self->imp->vtable->dev_setorigin(self->imp, x, y);
}

// psy_ui_GraphicsImp
static void dev_dispose(psy_ui_GraphicsImp* self) { }
static void dev_textout(psy_ui_GraphicsImp* self, double x, double y, const char* text, uintptr_t len) { }
static void dev_textoutrectangle(psy_ui_GraphicsImp* self, double x, double y, uintptr_t options,
	psy_ui_RealRectangle r, const char* text, uintptr_t len){ }
static void dev_drawrectangle(psy_ui_GraphicsImp* self, const psy_ui_RealRectangle r) { }
static void dev_drawroundrectangle(psy_ui_GraphicsImp* self, const psy_ui_RealRectangle r,
	psy_ui_Size cornersize) { }
static psy_ui_Size dev_textsize(psy_ui_GraphicsImp* self, const char* text)
{
	psy_ui_Size rv = {0, 0};

	return rv;
}
static void dev_drawsolidrectangle(psy_ui_GraphicsImp* self, const psy_ui_RealRectangle r,
	psy_ui_Colour colour) { }
static void dev_drawsolidroundrectangle(psy_ui_GraphicsImp* self, const psy_ui_RealRectangle r,
	psy_ui_Size cornersize, psy_ui_Colour colour) { }
static void dev_drawsolidpolygon(psy_ui_GraphicsImp* self, psy_ui_RealPoint* pt,
	uintptr_t numpoints, uint32_t inner, uint32_t outter) { }
static void dev_drawline(psy_ui_GraphicsImp* self, double x1, double y1, double x2, double y2) { }
static void dev_drawfullbitmap(psy_ui_GraphicsImp* self, psy_ui_Bitmap* bmp, double x, double y) { }
static void dev_drawbitmap(psy_ui_GraphicsImp* self, psy_ui_Bitmap* bmp, double x, double y, double width,
	double height, double xsrc, double ysrc) { }
static void dev_drawstretchedbitmap(psy_ui_GraphicsImp* self, psy_ui_Bitmap* bmp, double x, double y, double width,
	double height, double xsrc, double ysrc, double wsrc, double hsrc) { }
static void dev_setbackgroundcolour(psy_ui_GraphicsImp* self, psy_ui_Colour colour) { }
static void dev_setbackgroundmode(psy_ui_GraphicsImp* self, uintptr_t mode) { }
static void dev_settextcolour(psy_ui_GraphicsImp* self, psy_ui_Colour colour) { }
static void dev_settextalign(psy_ui_GraphicsImp* self, uintptr_t align) { }
static void dev_setcolour(psy_ui_GraphicsImp* self, psy_ui_Colour colour) { }
static void dev_setfont(psy_ui_GraphicsImp* self, const psy_ui_Font* font) { }
static void dev_moveto(psy_ui_GraphicsImp* self, psy_ui_RealPoint pt) { }
static void dev_curveto(psy_ui_GraphicsImp* self, psy_ui_RealPoint control_p1,
	psy_ui_RealPoint control_p2, psy_ui_RealPoint p) { }
static void dev_drawarc(psy_ui_GraphicsImp* self,
	double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4) { }
static void dev_setlinewidth(psy_ui_GraphicsImp* self, uintptr_t width) { }
static unsigned int dev_linewidth(psy_ui_GraphicsImp* self) { return 1; }
static void dev_setorigin(psy_ui_GraphicsImp* self, double x, double y) { }

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
		imp_vtable.dev_setorigin = dev_setorigin;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_graphics_imp_init(psy_ui_GraphicsImp* self)
{
	imp_vtable_init();	
	self->vtable = &imp_vtable;
}

void psy_ui_drawborder(psy_ui_Graphics* self, psy_ui_RealRectangle r,
	psy_ui_Border b)
{	
	if (psy_ui_border_isrect(&b)) {
		psy_ui_setcolour(self, b.colour_top);
		if (psy_ui_border_isround(&b)) {
			psy_ui_drawroundrectangle(self, r,
				psy_ui_size_make(b.border_bottom_left_radius,
					b.border_bottom_left_radius));
		} else {			
			psy_ui_setcolour(self, b.colour_top);			
			psy_ui_drawrectangle(self, r);
		}
	} else {
		if (b.colour_top.mode.set) {
			psy_ui_setcolour(self, b.colour_top);
			//border_top_left_radius
			psy_ui_drawline(self,
				psy_ui_realpoint_make(
					r.left + psy_ui_value_px(&b.border_top_left_radius, 0),
					r.top),
				psy_ui_realpoint_make(r.right - 1, r.top));
		}
		if (b.colour_right.mode.set) {
			psy_ui_setcolour(self, b.colour_right);
			psy_ui_drawline(self, psy_ui_realpoint_make(r.right - 1, r.top),
				psy_ui_realpoint_make(r.right - 1, r.bottom - 1));
		}
		if (b.colour_bottom.mode.set) {
			psy_ui_setcolour(self, b.colour_bottom);
			psy_ui_drawline(self, psy_ui_realpoint_make(r.left, r.bottom - 1),
				psy_ui_realpoint_make(r.right - 1, r.bottom - 1));
		}
		if (b.colour_left.mode.set) {
			psy_ui_setcolour(self, b.colour_left);
			psy_ui_drawline(self, psy_ui_realrectangle_topleft(&r),
				psy_ui_realpoint_make(r.left, r.bottom - 1));
		}
	}
}

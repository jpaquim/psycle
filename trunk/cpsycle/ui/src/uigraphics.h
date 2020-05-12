// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_GRAPHICS_H
#define psy_ui_GRAPHICS_H

#include "../../detail/psydef.h"
#include "uibitmap.h"

// Graphics Device Interface
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_Graphics
// Implementor			psy_ui_GraphicsImp
// Concrete Implementor	psy_ui_win_GraphicsImp
//
// psy_ui_Graphics <>-------- psy_ui_GraphicsImp
//     imp->dev_draw                 ^
//     ...                           |
//                          psy_ui_win_GraphicsImp

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	psy_ui_TRANSPARENT,
	psy_ui_OPAQUE
} psy_ui_TextBackgroundMode;

typedef enum {
	psy_ui_TA_LEFT = 0,
	psy_ui_TA_RIGHT = 2,
	psy_ui_TA_CENTER = 6,
	psy_ui_TA_TOP = 0,
	psy_ui_TA_BOTTOM = 8,
	psy_ui_TA_BASELINE =24
} psy_ui_TextAlign;

struct psy_ui_Graphics;

typedef void (*psy_ui_fp_graphics_dispose)(struct psy_ui_Graphics*);
typedef void (*psy_ui_fp_graphics_textout)(struct psy_ui_Graphics*, int x, int y,  const char*, size_t len);
typedef void (*psy_ui_fp_graphics_textoutrectangle)(struct psy_ui_Graphics*, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len);
typedef void (*psy_ui_fp_graphics_drawrectangle)(struct psy_ui_Graphics*, const psy_ui_Rectangle);
typedef void (*psy_ui_fp_graphics_drawroundrectangle)(struct psy_ui_Graphics*, const psy_ui_Rectangle,
	psy_ui_Size cornersize);
typedef psy_ui_Size (*psy_ui_fp_graphics_textsize)(struct psy_ui_Graphics*, const char*);
typedef void (*psy_ui_fp_graphics_drawsolidrectangle)(struct psy_ui_Graphics*, const psy_ui_Rectangle r,
	unsigned int color);
typedef void (*psy_ui_fp_graphics_drawsolidroundrectangle)(struct psy_ui_Graphics*, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color);
typedef void (*psy_ui_fp_graphics_drawsolidpolygon)(struct psy_ui_Graphics*, psy_ui_Point*,
	unsigned int numpoints,  unsigned int inner, unsigned int outter);
typedef void (*psy_ui_fp_graphics_drawline)(struct psy_ui_Graphics*, int x1, int y1, int x2, int y2);
typedef void (*psy_ui_fp_graphics_drawfullbitmap)(struct psy_ui_Graphics*, psy_ui_Bitmap*, int x, int y);
typedef void (*psy_ui_fp_graphics_drawbitmap)(struct psy_ui_Graphics*, psy_ui_Bitmap*, int x, int y, int width,
	int height, int xsrc, int ysrc);
typedef void (*psy_ui_fp_graphics_setbackgroundcolor)(struct psy_ui_Graphics*, unsigned int color);
typedef void (*psy_ui_fp_graphics_setbackgroundmode)(struct psy_ui_Graphics*, unsigned int mode);
typedef void (*psy_ui_fp_graphics_settextcolor)(struct psy_ui_Graphics*, unsigned int color);
typedef void (*psy_ui_fp_graphics_settextalign)(struct psy_ui_Graphics*, unsigned int color);
typedef void (*psy_ui_fp_graphics_setcolor)(struct psy_ui_Graphics*, unsigned int color);
typedef void (*psy_ui_fp_graphics_setfont)(struct psy_ui_Graphics*, psy_ui_Font* font);
typedef void (*psy_ui_fp_graphics_moveto)(struct psy_ui_Graphics*, psy_ui_Point pt);
typedef void (*psy_ui_fp_graphics_curveto)(struct psy_ui_Graphics*, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p);
typedef void (*psy_ui_fp_graphics_drawarc)(struct psy_ui_Graphics*,
	int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
typedef void (*psy_ui_fp_graphics_setlinewidth)(struct psy_ui_Graphics*, unsigned int width);
typedef unsigned int (*psy_ui_fp_graphics_linewidth)(struct psy_ui_Graphics*);

typedef struct psy_ui_GraphicsVTable {
	psy_ui_fp_graphics_dispose dispose;
	psy_ui_fp_graphics_textout textout;
	psy_ui_fp_graphics_textoutrectangle textoutrectangle;
	psy_ui_fp_graphics_drawrectangle drawrectangle;
	psy_ui_fp_graphics_drawroundrectangle drawroundrectangle;
	psy_ui_fp_graphics_textsize textsize;
	psy_ui_fp_graphics_drawsolidrectangle drawsolidrectangle;
	psy_ui_fp_graphics_drawsolidroundrectangle drawsolidroundrectangle;
	psy_ui_fp_graphics_drawsolidpolygon drawsolidpolygon;
	psy_ui_fp_graphics_drawline drawline;	
	psy_ui_fp_graphics_drawfullbitmap drawfullbitmap;
	psy_ui_fp_graphics_drawbitmap drawbitmap;
	psy_ui_fp_graphics_setbackgroundcolor setbackgroundcolor;
	psy_ui_fp_graphics_setbackgroundmode setbackgroundmode;
	psy_ui_fp_graphics_settextcolor settextcolor;
	psy_ui_fp_graphics_settextalign settextalign;
	psy_ui_fp_graphics_setcolor setcolor;	
	psy_ui_fp_graphics_setfont setfont;
	psy_ui_fp_graphics_moveto moveto;
	psy_ui_fp_graphics_curveto curveto;
	psy_ui_fp_graphics_drawarc drawarc;
	psy_ui_fp_graphics_setlinewidth setlinewidth;
	psy_ui_fp_graphics_linewidth linewidth;
} psy_ui_GraphicsVTable;

struct psy_ui_GraphicsImp;

typedef struct psy_ui_Graphics {
	psy_ui_GraphicsVTable* vtable;
	struct psy_ui_GraphicsImp* imp;
	psy_ui_Rectangle clip;
} psy_ui_Graphics;

void psy_ui_graphics_init(psy_ui_Graphics*, void* hdc);

// vtable calls
INLINE void psy_ui_graphics_dispose(psy_ui_Graphics* self)
{
	self->vtable->dispose(self);	
}

INLINE void psy_ui_textout(psy_ui_Graphics* self, int x, int y, const char* text, size_t len)
{		
	self->vtable->textout(self, x, y, text, len);
}

INLINE void psy_ui_textoutrectangle(psy_ui_Graphics* self, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len)
{
	self->vtable->textoutrectangle(self, x, y, options, r, text, len);
}

INLINE psy_ui_Size psy_ui_textsize(psy_ui_Graphics* self, const char* text)
{
	return self->vtable->textsize(self, text);	
}

INLINE void psy_ui_drawrectangle(psy_ui_Graphics* self, const psy_ui_Rectangle r)
{
	self->vtable->drawrectangle(self, r);	
}

INLINE void psy_ui_drawroundrectangle(psy_ui_Graphics* self, const psy_ui_Rectangle r, psy_ui_Size cornersize)
{
	self->vtable->drawroundrectangle(self, r, cornersize);	
}

INLINE void psy_ui_drawsolidrectangle(psy_ui_Graphics* self, const psy_ui_Rectangle r, unsigned int color)
{
	self->vtable->drawsolidrectangle(self, r, color);     
}

INLINE void psy_ui_drawsolidroundrectangle(psy_ui_Graphics* self, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color)
{
	self->vtable->drawsolidroundrectangle(self, r, cornersize, color);
}

INLINE void psy_ui_drawsolidpolygon(psy_ui_Graphics* self, psy_ui_Point* pts,
	unsigned int numpoints, unsigned int inner, unsigned int outter)
{
	self->vtable->drawsolidpolygon(self, pts, numpoints, inner, outter);	
}

INLINE void psy_ui_drawfullbitmap(psy_ui_Graphics* self, psy_ui_Bitmap* bitmap, int x, int y)
{	
	self->vtable->drawfullbitmap(self, bitmap, x, y);
}

INLINE void psy_ui_drawbitmap(psy_ui_Graphics* self, psy_ui_Bitmap* bitmap, int x, int y, int width,
	int height, int xsrc, int ysrc)
{	
	self->vtable->drawbitmap(self, bitmap, x, y, width, height, xsrc, ysrc);
}

INLINE void psy_ui_setcolor(psy_ui_Graphics* self, unsigned int color)
{	
	self->vtable->setcolor(self, color);
}

INLINE void psy_ui_setbackgroundmode(psy_ui_Graphics* self, unsigned int mode)
{	
	self->vtable->setbackgroundmode(self, mode);
}

INLINE void psy_ui_setbackgroundcolor(psy_ui_Graphics* self, unsigned int color)
{	
	self->vtable->setbackgroundcolor(self, color);
}

INLINE void psy_ui_settextcolor(psy_ui_Graphics* self, unsigned int color)
{
	self->vtable->settextcolor(self, color);
}

INLINE void psy_ui_settextalign(psy_ui_Graphics* self, unsigned int align)
{
	self->vtable->settextalign(self, align);
}

INLINE void psy_ui_setfont(psy_ui_Graphics* self, psy_ui_Font* font)
{		
	self->vtable->setfont(self, font);
}

INLINE void psy_ui_drawline(psy_ui_Graphics* self, int x1, int y1, int x2, int y2)
{	
	self->vtable->drawline(self, x1, y1, x2, y2);
}

INLINE void psy_ui_moveto(psy_ui_Graphics* self, psy_ui_Point pt)
{		
	self->vtable->moveto(self, pt);
}

INLINE void psy_ui_curveto(psy_ui_Graphics* self, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p)
{
	self->vtable->curveto(self, control_p1, control_p2, p);
}

INLINE  psy_ui_drawarc(psy_ui_Graphics* self,
	int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
	self->vtable->drawarc(self, x1, y1, x2, y2, x3, y3, x4, y4);
}

INLINE void psy_ui_setlinewidth(psy_ui_Graphics* self, unsigned int width)
{
	self->vtable->setlinewidth(self, width);
}

INLINE unsigned int psy_ui_linewidth(psy_ui_Graphics* self)
{
	return self->vtable->linewidth(self);
}

// psy_ui_GraphicsImp

typedef void (*psy_ui_fp_graphicsimp_dev_dispose)(struct psy_ui_GraphicsImp*);
typedef void (*psy_ui_fp_graphicsimp_dev_textout)(struct psy_ui_GraphicsImp*, int x, int y,  const char*, size_t len);
typedef void (*psy_ui_fp_graphicsimp_dev_textoutrectangle)(struct psy_ui_GraphicsImp*, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len);
typedef void (*psy_ui_fp_graphicsimp_dev_drawrectangle)(struct psy_ui_GraphicsImp*, const psy_ui_Rectangle);
typedef void (*psy_ui_fp_graphicsimp_dev_drawroundrectangle)(struct psy_ui_GraphicsImp*, const psy_ui_Rectangle,
	psy_ui_Size cornersize);
typedef psy_ui_Size (*psy_ui_fp_graphicsimp_dev_textsize)(struct psy_ui_GraphicsImp*, const char*);
typedef void (*psy_ui_fp_graphicsimp_dev_drawsolidrectangle)(struct psy_ui_GraphicsImp*, const psy_ui_Rectangle r,
	unsigned int color);
typedef void (*psy_ui_fp_graphicsimp_dev_drawsolidroundrectangle)(struct psy_ui_GraphicsImp*, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color);
typedef void (*psy_ui_fp_graphicsimp_dev_drawsolidpolygon)(struct psy_ui_GraphicsImp*, psy_ui_Point*,
	unsigned int numpoints,  unsigned int inner, unsigned int outter);
typedef void (*psy_ui_fp_graphicsimp_dev_drawline)(struct psy_ui_GraphicsImp*, int x1, int y1, int x2, int y2);
typedef void (*psy_ui_fp_graphicsimp_dev_drawfullbitmap)(struct psy_ui_GraphicsImp*, psy_ui_Bitmap*, int x, int y);
typedef void (*psy_ui_fp_graphicsimp_dev_drawbitmap)(struct psy_ui_GraphicsImp*, psy_ui_Bitmap*, int x, int y, int width,
	int height, int xsrc, int ysrc);
typedef void (*psy_ui_fp_graphicsimp_dev_setbackgroundcolor)(struct psy_ui_GraphicsImp*, unsigned int color);
typedef void (*psy_ui_fp_graphicsimp_dev_setbackgroundmode)(struct psy_ui_GraphicsImp*, unsigned int mode);
typedef void (*psy_ui_fp_graphicsimp_dev_settextcolor)(struct psy_ui_GraphicsImp*, unsigned int color);
typedef void (*psy_ui_fp_graphicsimp_dev_settextalign)(struct psy_ui_GraphicsImp*, unsigned int align);
typedef void (*psy_ui_fp_graphicsimp_dev_setcolor)(struct psy_ui_GraphicsImp*, unsigned int color);
typedef void (*psy_ui_fp_graphicsimp_dev_setfont)(struct psy_ui_GraphicsImp*, psy_ui_Font* font);
typedef void (*psy_ui_fp_graphicsimp_dev_moveto)(struct psy_ui_GraphicsImp*, psy_ui_Point pt);
typedef void (*psy_ui_fp_graphicsimp_dev_curveto)(struct psy_ui_GraphicsImp*, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p);
typedef void (*psy_ui_fp_graphicsimp_dev_drawarc)(struct psy_ui_GraphicsImp*,
	int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
typedef void (*psy_ui_fp_graphicsimp_dev_setlinewidth)(struct psy_ui_GraphicsImp*, unsigned int width);
typedef unsigned int (*psy_ui_fp_graphicsimp_dev_linewidth)(struct psy_ui_GraphicsImp*);

typedef struct psy_ui_GraphicsImpVTable {
	psy_ui_fp_graphicsimp_dev_dispose dev_dispose;
	psy_ui_fp_graphicsimp_dev_textout dev_textout;
	psy_ui_fp_graphicsimp_dev_textoutrectangle dev_textoutrectangle;
	psy_ui_fp_graphicsimp_dev_drawrectangle dev_drawrectangle;
	psy_ui_fp_graphicsimp_dev_drawroundrectangle dev_drawroundrectangle;
	psy_ui_fp_graphicsimp_dev_textsize dev_textsize;
	psy_ui_fp_graphicsimp_dev_drawsolidrectangle dev_drawsolidrectangle;
	psy_ui_fp_graphicsimp_dev_drawsolidroundrectangle dev_drawsolidroundrectangle;
	psy_ui_fp_graphicsimp_dev_drawsolidpolygon dev_drawsolidpolygon;
	psy_ui_fp_graphicsimp_dev_drawline dev_drawline;	
	psy_ui_fp_graphicsimp_dev_drawfullbitmap dev_drawfullbitmap;
	psy_ui_fp_graphicsimp_dev_drawbitmap dev_drawbitmap;
	psy_ui_fp_graphicsimp_dev_setbackgroundcolor dev_setbackgroundcolor;
	psy_ui_fp_graphicsimp_dev_setbackgroundmode dev_setbackgroundmode;
	psy_ui_fp_graphicsimp_dev_settextcolor dev_settextcolor;
	psy_ui_fp_graphicsimp_dev_settextalign dev_settextalign;
	psy_ui_fp_graphicsimp_dev_setcolor dev_setcolor;
	psy_ui_fp_graphicsimp_dev_setfont dev_setfont;
	psy_ui_fp_graphicsimp_dev_moveto dev_moveto;
	psy_ui_fp_graphicsimp_dev_curveto dev_curveto;
	psy_ui_fp_graphicsimp_dev_drawarc dev_drawarc;
	psy_ui_fp_graphicsimp_dev_setlinewidth dev_setlinewidth;
	psy_ui_fp_graphicsimp_dev_linewidth dev_linewidth;
} psy_ui_GraphicsImpVTable;

typedef struct psy_ui_GraphicsImp {
	psy_ui_GraphicsImpVTable* vtable;
} psy_ui_GraphicsImp;

void psy_ui_graphics_imp_init(psy_ui_GraphicsImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_GRAPHICS_H */

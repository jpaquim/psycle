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

struct psy_ui_Graphics;

typedef void (*psy_ui_g_fp_dispose)(struct psy_ui_Graphics*);
typedef void (*psy_ui_g_fp_textout)(struct psy_ui_Graphics*, int x, int y,  const char*, size_t len);
typedef void (*psy_ui_g_fp_textoutrectangle)(struct psy_ui_Graphics*, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len);
typedef void (*psy_ui_g_fp_drawrectangle)(struct psy_ui_Graphics*, const psy_ui_Rectangle);
typedef void (*psy_ui_g_fp_drawroundrectangle)(struct psy_ui_Graphics*, const psy_ui_Rectangle,
	psy_ui_Size cornersize);
typedef psy_ui_Size (*psy_ui_g_fp_textsize)(struct psy_ui_Graphics*, const char*);
typedef void (*psy_ui_g_fp_drawsolidrectangle)(struct psy_ui_Graphics*, const psy_ui_Rectangle r,
	unsigned int color);
typedef void (*psy_ui_g_fp_drawsolidroundrectangle)(struct psy_ui_Graphics*, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color);
typedef void (*psy_ui_g_fp_drawsolidpolygon)(struct psy_ui_Graphics*, psy_ui_Point*,
	unsigned int numpoints,  unsigned int inner, unsigned int outter);
typedef void (*psy_ui_g_fp_drawline)(struct psy_ui_Graphics*, int x1, int y1, int x2, int y2);
typedef void (*psy_ui_g_fp_drawfullbitmap)(struct psy_ui_Graphics*, psy_ui_Bitmap*, int x, int y);
typedef void (*psy_ui_g_fp_drawbitmap)(struct psy_ui_Graphics*, psy_ui_Bitmap*, int x, int y, int width,
	int height, int xsrc, int ysrc);
typedef void (*psy_ui_g_fp_setbackgroundcolor)(struct psy_ui_Graphics*, unsigned int color);
typedef void (*psy_ui_g_fp_setbackgroundmode)(struct psy_ui_Graphics*, unsigned int mode);
typedef void (*psy_ui_g_fp_settextcolor)(struct psy_ui_Graphics*, unsigned int color);
typedef void (*psy_ui_g_fp_setcolor)(struct psy_ui_Graphics*, unsigned int color);
typedef void (*psy_ui_g_fp_setfont)(struct psy_ui_Graphics*, psy_ui_Font* font);
typedef void (*psy_ui_g_fp_moveto)(struct psy_ui_Graphics*, psy_ui_Point pt);
typedef void (*psy_ui_g_fp_devcurveto)(struct psy_ui_Graphics*, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p);

typedef struct psy_ui_GraphicsVTable {
	psy_ui_g_fp_dispose dispose;
	psy_ui_g_fp_textout textout;
	psy_ui_g_fp_textoutrectangle textoutrectangle;
	psy_ui_g_fp_drawrectangle drawrectangle;
	psy_ui_g_fp_drawroundrectangle drawroundrectangle;
	psy_ui_g_fp_textsize textsize;
	psy_ui_g_fp_drawsolidrectangle drawsolidrectangle;
	psy_ui_g_fp_drawsolidroundrectangle drawsolidroundrectangle;
	psy_ui_g_fp_drawsolidpolygon drawsolidpolygon;
	psy_ui_g_fp_drawline drawline;	
	psy_ui_g_fp_drawfullbitmap drawfullbitmap;
	psy_ui_g_fp_drawbitmap drawbitmap;
	psy_ui_g_fp_setbackgroundcolor setbackgroundcolor;
	psy_ui_g_fp_setbackgroundmode setbackgroundmode;
	psy_ui_g_fp_settextcolor settextcolor;
	psy_ui_g_fp_setcolor setcolor;
	psy_ui_g_fp_setfont setfont;
	psy_ui_g_fp_moveto moveto;
	psy_ui_g_fp_devcurveto devcurveto;
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

INLINE void psy_ui_devcurveto(psy_ui_Graphics* self, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p)
{
	self->vtable->devcurveto(self, control_p1, control_p2, p);
}

// psy_ui_GraphicsImp

typedef void (*psy_ui_g_imp_fp_dispose)(struct psy_ui_GraphicsImp*);
typedef void (*psy_ui_g_imp_fp_textout)(struct psy_ui_GraphicsImp*, int x, int y,  const char*, size_t len);
typedef void (*psy_ui_g_imp_fp_textoutrectangle)(struct psy_ui_GraphicsImp*, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len);
typedef void (*psy_ui_g_imp_fp_drawrectangle)(struct psy_ui_GraphicsImp*, const psy_ui_Rectangle);
typedef void (*psy_ui_g_imp_fp_drawroundrectangle)(struct psy_ui_GraphicsImp*, const psy_ui_Rectangle,
	psy_ui_Size cornersize);
typedef psy_ui_Size (*psy_ui_g_imp_fp_textsize)(struct psy_ui_GraphicsImp*, const char*);
typedef void (*psy_ui_g_imp_fp_drawsolidrectangle)(struct psy_ui_GraphicsImp*, const psy_ui_Rectangle r,
	unsigned int color);
typedef void (*psy_ui_g_imp_fp_drawsolidroundrectangle)(struct psy_ui_GraphicsImp*, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color);
typedef void (*psy_ui_g_imp_fp_drawsolidpolygon)(struct psy_ui_GraphicsImp*, psy_ui_Point*,
	unsigned int numpoints,  unsigned int inner, unsigned int outter);
typedef void (*psy_ui_g_imp_fp_drawline)(struct psy_ui_GraphicsImp*, int x1, int y1, int x2, int y2);
typedef void (*psy_ui_g_imp_fp_drawfullbitmap)(struct psy_ui_GraphicsImp*, psy_ui_Bitmap*, int x, int y);
typedef void (*psy_ui_g_imp_fp_drawbitmap)(struct psy_ui_GraphicsImp*, psy_ui_Bitmap*, int x, int y, int width,
	int height, int xsrc, int ysrc);
typedef void (*psy_ui_g_imp_fp_setbackgroundcolor)(struct psy_ui_GraphicsImp*, unsigned int color);
typedef void (*psy_ui_g_imp_fp_setbackgroundmode)(struct psy_ui_GraphicsImp*, unsigned int mode);
typedef void (*psy_ui_g_imp_fp_settextcolor)(struct psy_ui_GraphicsImp*, unsigned int color);
typedef void (*psy_ui_g_imp_fp_setcolor)(struct psy_ui_GraphicsImp*, unsigned int color);
typedef void (*psy_ui_g_imp_fp_setfont)(struct psy_ui_GraphicsImp*, psy_ui_Font* font);
typedef void (*psy_ui_g_imp_fp_moveto)(struct psy_ui_GraphicsImp*, psy_ui_Point pt);
typedef void (*psy_ui_g_imp_fp_devcurveto)(struct psy_ui_GraphicsImp*, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p);

typedef struct psy_ui_GraphicsImpVTable {
	psy_ui_g_imp_fp_dispose dev_dispose;
	psy_ui_g_imp_fp_textout dev_textout;
	psy_ui_g_imp_fp_textoutrectangle dev_textoutrectangle;
	psy_ui_g_imp_fp_drawrectangle dev_drawrectangle;
	psy_ui_g_imp_fp_drawroundrectangle dev_drawroundrectangle;
	psy_ui_g_imp_fp_textsize dev_textsize;
	psy_ui_g_imp_fp_drawsolidrectangle dev_drawsolidrectangle;
	psy_ui_g_imp_fp_drawsolidroundrectangle dev_drawsolidroundrectangle;
	psy_ui_g_imp_fp_drawsolidpolygon dev_drawsolidpolygon;
	psy_ui_g_imp_fp_drawline dev_drawline;	
	psy_ui_g_imp_fp_drawfullbitmap dev_drawfullbitmap;
	psy_ui_g_imp_fp_drawbitmap dev_drawbitmap;
	psy_ui_g_imp_fp_setbackgroundcolor dev_setbackgroundcolor;
	psy_ui_g_imp_fp_setbackgroundmode dev_setbackgroundmode;
	psy_ui_g_imp_fp_settextcolor dev_settextcolor;
	psy_ui_g_imp_fp_setcolor dev_setcolor;
	psy_ui_g_imp_fp_setfont dev_setfont;
	psy_ui_g_imp_fp_moveto dev_moveto;
	psy_ui_g_imp_fp_devcurveto dev_devcurveto;
} psy_ui_GraphicsImpVTable;

typedef struct psy_ui_GraphicsImp {
	psy_ui_GraphicsImpVTable* vtable;
} psy_ui_GraphicsImp;

void psy_ui_graphics_imp_init(psy_ui_GraphicsImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_GRAPHICS_H */

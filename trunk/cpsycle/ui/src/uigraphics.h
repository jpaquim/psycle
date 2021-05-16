/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_GRAPHICS_H
#define psy_ui_GRAPHICS_H

/* platform */
#include "../../detail/psydef.h"
/* local */
#include "uibitmap.h"
#include "uifont.h"

/*
** Graphics Device Interface
**
** Bridge
** Aim: avoid coupling to one platform (win32, xt/motif, etc)
** Abstraction/Refined  psy_ui_Graphics
** Implementor			psy_ui_GraphicsImp
** Concrete Implementor	psy_ui_win_GraphicsImp
** 
** psy_ui_Graphics <>-------- psy_ui_GraphicsImp
**     imp->dev_draw                  ^
**     ...                            |
**                          psy_ui_win_GraphicsImp
*/

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
typedef void (*psy_ui_fp_graphics_textout)(struct psy_ui_Graphics*, double x, double y,  const char*, uintptr_t len);
typedef void (*psy_ui_fp_graphics_textoutrectangle)(struct psy_ui_Graphics*, double x, double y, uintptr_t options,
	psy_ui_RealRectangle r, const char* text, uintptr_t len);
typedef void (*psy_ui_fp_graphics_drawrectangle)(struct psy_ui_Graphics*, const psy_ui_RealRectangle);
typedef void (*psy_ui_fp_graphics_drawroundrectangle)(struct psy_ui_Graphics*, const psy_ui_RealRectangle,
	psy_ui_RealSize cornersize);
typedef psy_ui_Size (*psy_ui_fp_graphics_textsize)(struct psy_ui_Graphics*, const char*, uintptr_t count);
typedef void (*psy_ui_fp_graphics_drawsolidrectangle)(struct psy_ui_Graphics*, const psy_ui_RealRectangle,
	psy_ui_Colour colour);
typedef void (*psy_ui_fp_graphics_drawsolidroundrectangle)(struct psy_ui_Graphics*, const psy_ui_RealRectangle,
	psy_ui_RealSize cornersize, psy_ui_Colour colour);
typedef void (*psy_ui_fp_graphics_drawsolidpolygon)(struct psy_ui_Graphics*, psy_ui_RealPoint*,
	uintptr_t numpoints, uint32_t inner, uint32_t outter);
typedef void (*psy_ui_fp_graphics_drawline)(struct psy_ui_Graphics*, double x1, double y1, double x2, double y2);
typedef void (*psy_ui_fp_graphics_drawfullbitmap)(struct psy_ui_Graphics*, psy_ui_Bitmap*, double x, double y);
typedef void (*psy_ui_fp_graphics_drawbitmap)(struct psy_ui_Graphics*, psy_ui_Bitmap*, double x, double y, double width,
	double height, double xsrc, double ysrc);
typedef void (*psy_ui_fp_graphics_drawstretchedbitmap)(struct psy_ui_Graphics*, psy_ui_Bitmap*, double x, double y, double width,
	double height, double xsrc, double ysrc, double wsrc, double hsrc);
typedef void (*psy_ui_fp_graphics_setbackgroundcolour)(struct psy_ui_Graphics*, psy_ui_Colour);
typedef void (*psy_ui_fp_graphics_setbackgroundmode)(struct psy_ui_Graphics*, uintptr_t mode);
typedef void (*psy_ui_fp_graphics_settextcolour)(struct psy_ui_Graphics*, psy_ui_Colour);
typedef void (*psy_ui_fp_graphics_settextalign)(struct psy_ui_Graphics*, uintptr_t align);
typedef void (*psy_ui_fp_graphics_setcolour)(struct psy_ui_Graphics*, psy_ui_Colour);
typedef void (*psy_ui_fp_graphics_setfont)(struct psy_ui_Graphics*, const psy_ui_Font*);
typedef void (*psy_ui_fp_graphics_moveto)(struct psy_ui_Graphics*, psy_ui_RealPoint pt);
typedef void (*psy_ui_fp_graphics_curveto)(struct psy_ui_Graphics*, psy_ui_RealPoint control_p1,
	psy_ui_RealPoint control_p2, psy_ui_RealPoint p);
typedef void (*psy_ui_fp_graphics_drawarc)(struct psy_ui_Graphics*,
	double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);
typedef void (*psy_ui_fp_graphics_setlinewidth)(struct psy_ui_Graphics*, uintptr_t width);
typedef uintptr_t (*psy_ui_fp_graphics_linewidth)(struct psy_ui_Graphics*);
typedef void (*psy_ui_fp_graphics_dev_setorigin)(struct psy_ui_Graphics*, double x, double y);
typedef psy_ui_RealPoint (*psy_ui_fp_graphics_dev_origin)(const struct psy_ui_Graphics*);

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
	psy_ui_fp_graphics_drawstretchedbitmap drawstretchedbitmap;
	psy_ui_fp_graphics_setbackgroundcolour setbackgroundcolour;
	psy_ui_fp_graphics_setbackgroundmode setbackgroundmode;
	psy_ui_fp_graphics_settextcolour settextcolour;
	psy_ui_fp_graphics_settextalign settextalign;
	psy_ui_fp_graphics_setcolour setcolour;	
	psy_ui_fp_graphics_setfont setfont;
	psy_ui_fp_graphics_moveto moveto;
	psy_ui_fp_graphics_curveto curveto;
	psy_ui_fp_graphics_drawarc drawarc;
	psy_ui_fp_graphics_setlinewidth setlinewidth;
	psy_ui_fp_graphics_linewidth linewidth;
	psy_ui_fp_graphics_dev_setorigin setorigin;
	psy_ui_fp_graphics_dev_origin origin;
} psy_ui_GraphicsVTable;

struct psy_ui_GraphicsImp;

typedef struct psy_ui_Graphics {
	psy_ui_GraphicsVTable* vtable;
	struct psy_ui_GraphicsImp* imp;
	psy_ui_RealRectangle clip;
} psy_ui_Graphics;

void psy_ui_graphics_init(psy_ui_Graphics*, void* hdc);
void psy_ui_graphics_init_bitmap(psy_ui_Graphics*, psy_ui_Bitmap*);

/* vtable calls */
INLINE void psy_ui_graphics_dispose(psy_ui_Graphics* self)
{
	self->vtable->dispose(self);	
}

INLINE void psy_ui_setcolour(psy_ui_Graphics* self, psy_ui_Colour colour)
{
	self->vtable->setcolour(self, colour);
}

INLINE void psy_ui_textout(psy_ui_Graphics* self, double x, double y, const char* text, uintptr_t len)
{		
	self->vtable->textout(self, x, y, text, len);
}

INLINE void psy_ui_textoutrectangle(psy_ui_Graphics* self, psy_ui_RealPoint dest, uintptr_t options,
	psy_ui_RealRectangle r, const char* text, uintptr_t len)
{
	self->vtable->textoutrectangle(self, dest.x, dest.y, options, r, text, len);
}

INLINE psy_ui_Size psy_ui_textsize(psy_ui_Graphics* self, const char* text, uintptr_t count)
{
	return self->vtable->textsize(self, text, count);	
}

INLINE void psy_ui_drawrectangle(psy_ui_Graphics* self, const psy_ui_RealRectangle r)
{
	self->vtable->drawrectangle(self, r);	
}

INLINE void psy_ui_drawroundrectangle(psy_ui_Graphics* self, const psy_ui_RealRectangle r, psy_ui_RealSize cornersize)
{
	self->vtable->drawroundrectangle(self, r, cornersize);	
}

INLINE void psy_ui_drawsolidrectangle(psy_ui_Graphics* self, const psy_ui_RealRectangle r, psy_ui_Colour colour)
{
	self->vtable->drawsolidrectangle(self, r, colour);     
}

INLINE void psy_ui_drawsolidroundrectangle(psy_ui_Graphics* self, const psy_ui_RealRectangle r,
	psy_ui_RealSize cornersize, psy_ui_Colour colour)
{
	self->vtable->drawsolidroundrectangle(self, r, cornersize, colour);
}

INLINE void psy_ui_drawsolidpolygon(psy_ui_Graphics* self, psy_ui_RealPoint* pts,
	uintptr_t numpoints, uint32_t inner, uint32_t outter)
{
	self->vtable->drawsolidpolygon(self, pts, numpoints, inner, outter);	
}
/*
** blits a bitmap with its full size to the device context
**  dest: topleft point of the destination rectangle
*/
INLINE void psy_ui_drawfullbitmap(psy_ui_Graphics* self, psy_ui_Bitmap* bitmap,
	psy_ui_RealPoint dest)
{	
	self->vtable->drawfullbitmap(self, bitmap, dest.x, dest.y);
}
/*
** blits a bitmap to the device context
**  dest: destination rectangle of the blitted bitmap and size of the source
**        bitmap part to be copied
**  src: topleft position of the source rectangle
*/
INLINE void psy_ui_drawbitmap(psy_ui_Graphics* self,
	psy_ui_Bitmap* bitmap,
	psy_ui_RealRectangle dest,
	psy_ui_RealPoint src)
{	
	self->vtable->drawbitmap(self, bitmap, dest.left, dest.top,
		psy_ui_realrectangle_width(&dest), psy_ui_realrectangle_height(&dest),
		src.x, src.y);
}
/*
** blits a bitmap to the device context
**  dest: destination rectangle of the blitted bitmap
**        srcsize will be sized to the destination size
**  src: topleft position of the source rectangle
**  srcsize: part of the source bitmap to be copied
*/
INLINE void psy_ui_drawstretchedbitmap(psy_ui_Graphics* self,
	psy_ui_Bitmap* bitmap,
	psy_ui_RealRectangle dest,
	psy_ui_RealPoint src, psy_ui_RealSize srcsize)
{
	self->vtable->drawstretchedbitmap(self, bitmap, dest.left, dest.top,
		psy_ui_realrectangle_width(&dest), psy_ui_realrectangle_height(&dest),
		src.x, src.y, srcsize.width, srcsize.height);
}

INLINE void psy_ui_setbackgroundmode(psy_ui_Graphics* self, uintptr_t mode)
{	
	self->vtable->setbackgroundmode(self, mode);
}

INLINE void psy_ui_setbackgroundcolour(psy_ui_Graphics* self, psy_ui_Colour colour)
{	
	self->vtable->setbackgroundcolour(self, colour);
}

INLINE void psy_ui_settextcolour(psy_ui_Graphics* self, psy_ui_Colour colour)
{
	self->vtable->settextcolour(self, colour);
}

INLINE void psy_ui_settextalign(psy_ui_Graphics* self, uintptr_t align)
{
	self->vtable->settextalign(self, align);
}

INLINE void psy_ui_setfont(psy_ui_Graphics* self, const psy_ui_Font* font)
{		
	self->vtable->setfont(self, font);
}

INLINE void psy_ui_drawline(psy_ui_Graphics* self, psy_ui_RealPoint pt1,
	psy_ui_RealPoint pt2)
{	
	self->vtable->drawline(self, pt1.x, pt1.y, pt2.x, pt2.y);
}

INLINE void psy_ui_moveto(psy_ui_Graphics* self, psy_ui_RealPoint pt)
{		
	self->vtable->moveto(self, pt);
}

INLINE void psy_ui_curveto(psy_ui_Graphics* self, psy_ui_RealPoint control_p1,
	psy_ui_RealPoint control_p2, psy_ui_RealPoint p)
{
	self->vtable->curveto(self, control_p1, control_p2, p);
}

INLINE void psy_ui_drawarc(psy_ui_Graphics* self,
	double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
	self->vtable->drawarc(self, x1, y1, x2, y2, x3, y3, x4, y4);
}

INLINE void psy_ui_setlinewidth(psy_ui_Graphics* self, uintptr_t width)
{
	self->vtable->setlinewidth(self, width);
}

INLINE uintptr_t psy_ui_linewidth(psy_ui_Graphics* self)
{
	return self->vtable->linewidth(self);
}

INLINE void psy_ui_setorigin(psy_ui_Graphics* self, psy_ui_RealPoint origin)
{
	self->vtable->setorigin(self, origin.x, origin.y);
}

INLINE psy_ui_RealPoint psy_ui_origin(const psy_ui_Graphics* self)
{
	return self->vtable->origin(self);
}

void psy_ui_drawborder(psy_ui_Graphics* self, psy_ui_RealRectangle,
	const psy_ui_Border*, const psy_ui_TextMetric*);

/* psy_ui_GraphicsImp */

typedef void (*psy_ui_fp_graphicsimp_dev_dispose)(struct psy_ui_GraphicsImp*);
typedef void (*psy_ui_fp_graphicsimp_dev_textout)(struct psy_ui_GraphicsImp*, double x, double y,  const char*, uintptr_t len);
typedef void (*psy_ui_fp_graphicsimp_dev_textoutrectangle)(struct psy_ui_GraphicsImp*, double x, double y,uintptr_t options,
	psy_ui_RealRectangle r, const char* text, uintptr_t len);
typedef void (*psy_ui_fp_graphicsimp_dev_drawrectangle)(struct psy_ui_GraphicsImp*, const psy_ui_RealRectangle);
typedef void (*psy_ui_fp_graphicsimp_dev_drawroundrectangle)(struct psy_ui_GraphicsImp*, const psy_ui_RealRectangle,
	psy_ui_RealSize cornersize);
typedef psy_ui_Size (*psy_ui_fp_graphicsimp_dev_textsize)(struct psy_ui_GraphicsImp*, const char*, uintptr_t count);
typedef void (*psy_ui_fp_graphicsimp_dev_drawsolidrectangle)(struct psy_ui_GraphicsImp*, const psy_ui_RealRectangle r,
	psy_ui_Colour colour);
typedef void (*psy_ui_fp_graphicsimp_dev_drawsolidroundrectangle)(struct psy_ui_GraphicsImp*, const psy_ui_RealRectangle r,
	psy_ui_RealSize cornersize, psy_ui_Colour colour);
typedef void (*psy_ui_fp_graphicsimp_dev_drawsolidpolygon)(struct psy_ui_GraphicsImp*, psy_ui_RealPoint*,
	uintptr_t numpoints,  uint32_t inner, uint32_t outter);
typedef void (*psy_ui_fp_graphicsimp_dev_drawline)(struct psy_ui_GraphicsImp*, double x1, double y1, double x2, double y2);
typedef void (*psy_ui_fp_graphicsimp_dev_drawfullbitmap)(struct psy_ui_GraphicsImp*, psy_ui_Bitmap*, double x, double y);
typedef void (*psy_ui_fp_graphicsimp_dev_drawbitmap)(struct psy_ui_GraphicsImp*, psy_ui_Bitmap*, double x, double y, double width,
	double height, double xsrc, double ysrc);
typedef void (*psy_ui_fp_graphicsimp_dev_drawstretchedbitmap)(struct psy_ui_GraphicsImp*, psy_ui_Bitmap*, double x, double y, double width,
	double height, double xsrc, double ysrc, double wsrc, double hsrc);
typedef void (*psy_ui_fp_graphicsimp_dev_setbackgroundcolour)(struct psy_ui_GraphicsImp*, psy_ui_Colour);
typedef void (*psy_ui_fp_graphicsimp_dev_setbackgroundmode)(struct psy_ui_GraphicsImp*, uintptr_t mode);
typedef void (*psy_ui_fp_graphicsimp_dev_settextcolour)(struct psy_ui_GraphicsImp*, psy_ui_Colour);
typedef void (*psy_ui_fp_graphicsimp_dev_settextalign)(struct psy_ui_GraphicsImp*, uintptr_t align);
typedef void (*psy_ui_fp_graphicsimp_dev_setcolour)(struct psy_ui_GraphicsImp*, psy_ui_Colour);
typedef void (*psy_ui_fp_graphicsimp_dev_setfont)(struct psy_ui_GraphicsImp*, const psy_ui_Font*);
typedef void (*psy_ui_fp_graphicsimp_dev_moveto)(struct psy_ui_GraphicsImp*, psy_ui_RealPoint pt);
typedef void (*psy_ui_fp_graphicsimp_dev_curveto)(struct psy_ui_GraphicsImp*, psy_ui_RealPoint control_p1,
	psy_ui_RealPoint control_p2, psy_ui_RealPoint p);
typedef void (*psy_ui_fp_graphicsimp_dev_drawarc)(struct psy_ui_GraphicsImp*,
	double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);
typedef void (*psy_ui_fp_graphicsimp_dev_setlinewidth)(struct psy_ui_GraphicsImp*, uintptr_t width);
typedef unsigned int (*psy_ui_fp_graphicsimp_dev_linewidth)(struct psy_ui_GraphicsImp*);
typedef void (*psy_ui_fp_graphicsimp_dev_setorigin)(struct psy_ui_GraphicsImp*, double x, double y);
typedef psy_ui_RealPoint (*psy_ui_fp_graphicsimp_dev_origin)(const struct psy_ui_GraphicsImp*);

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
	psy_ui_fp_graphicsimp_dev_drawstretchedbitmap dev_drawstretchedbitmap;
	psy_ui_fp_graphicsimp_dev_setbackgroundcolour dev_setbackgroundcolour;
	psy_ui_fp_graphicsimp_dev_setbackgroundmode dev_setbackgroundmode;
	psy_ui_fp_graphicsimp_dev_settextcolour dev_settextcolour;
	psy_ui_fp_graphicsimp_dev_settextalign dev_settextalign;
	psy_ui_fp_graphicsimp_dev_setcolour dev_setcolour;
	psy_ui_fp_graphicsimp_dev_setfont dev_setfont;
	psy_ui_fp_graphicsimp_dev_moveto dev_moveto;
	psy_ui_fp_graphicsimp_dev_curveto dev_curveto;
	psy_ui_fp_graphicsimp_dev_drawarc dev_drawarc;
	psy_ui_fp_graphicsimp_dev_setlinewidth dev_setlinewidth;
	psy_ui_fp_graphicsimp_dev_linewidth dev_linewidth;
	psy_ui_fp_graphicsimp_dev_setorigin dev_setorigin;
	psy_ui_fp_graphicsimp_dev_origin dev_origin;
} psy_ui_GraphicsImpVTable;

typedef struct psy_ui_GraphicsImp {
	psy_ui_GraphicsImpVTable* vtable;
} psy_ui_GraphicsImp;

void psy_ui_graphics_imp_init(psy_ui_GraphicsImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_GRAPHICS_H */

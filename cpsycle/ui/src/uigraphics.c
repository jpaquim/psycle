// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uigraphics.h"
#include "uiapp.h"

#include <assert.h>

extern psy_ui_App app;

void psy_ui_graphics_init(psy_ui_Graphics* g, HDC hdc)
{
	g->hdc = hdc;		
	g->pen = CreatePen(PS_SOLID, 1, app.defaults.defaultcolor);	
	g->brush = 0;
	g->hBrushPrev = 0;
	g->penprev = SelectObject(g->hdc, g->pen);
	g->hFontPrev = SelectObject(g->hdc, app.defaults.defaultfont.hfont);
}	

void psy_ui_graphics_dispose(psy_ui_Graphics* g)
{
	SelectObject(g->hdc, g->penprev);
	SelectObject(g->hdc, g->hFontPrev);
	if (g->pen) {		
		DeleteObject(g->pen);
	}	
}

void psy_ui_textout(psy_ui_Graphics* self, int x, int y, const char* str, size_t len)
{	
	TextOut(self->hdc, x, y, str, (int)len);
}

void psy_ui_textoutrectangle(psy_ui_Graphics* g, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len)
{
	RECT rect;
		                
    SetRect (&rect, r.left, r.top, r.right, r.bottom) ;     	
	ExtTextOut(g->hdc, x, y, options, &rect, text, (int)len, NULL);
}

psy_ui_Size psy_ui_textsize(psy_ui_Graphics* g, const char* text)
{
	psy_ui_Size	rv;

	if (text) {
		SIZE size;
				
		GetTextExtentPoint(g->hdc, text, (int)strlen(text), &size) ;	
		rv.width = size.cx; 
		rv.height = size.cy;
	} else {
		rv.width = 0; 
		rv.height = 0;
	}
	return rv;
}

void psy_ui_drawrectangle(psy_ui_Graphics* self, const psy_ui_Rectangle r)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;

	hBrush = GetStockObject (NULL_BRUSH);
	hOldBrush = SelectObject (self->hdc, hBrush);
	Rectangle(self->hdc, r.left, r.top, r.right, r.bottom);
	SelectObject (self->hdc, hOldBrush);
}

void psy_ui_drawroundrectangle(psy_ui_Graphics* self, const psy_ui_Rectangle r, psy_ui_Size cornersize)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;

	hBrush = GetStockObject (NULL_BRUSH);
	hOldBrush = SelectObject (self->hdc, hBrush);
	RoundRect(self->hdc, r.left, r.top, r.right, r.bottom, cornersize.width,
		cornersize.height);
	SelectObject (self->hdc, hOldBrush);
}

void psy_ui_drawsolidrectangle(psy_ui_Graphics* g, const psy_ui_Rectangle r, unsigned int color)
{
     HBRUSH hBrush;     
     RECT   rect;	 
	                
     SetRect (&rect, r.left, r.top, r.right, r.bottom) ;     
     hBrush = CreateSolidBrush(color);
     FillRect (g->hdc, &rect, hBrush);     
     DeleteObject (hBrush) ;
}

void psy_ui_drawsolidroundrectangle(psy_ui_Graphics* self, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;
	HPEN hPen;
	HPEN hOldPen;

	hBrush = CreateSolidBrush(color);
	hOldBrush = SelectObject (self->hdc, hBrush);
	hPen = CreatePen(PS_SOLID, 1, color);
	hOldPen = SelectObject(self->hdc, hPen);
	RoundRect(self->hdc, r.left, r.top, r.right, r.bottom, cornersize.width,
		cornersize.height);
	SelectObject (self->hdc, hOldBrush);
	SelectObject (self->hdc, hOldPen);
	DeleteObject (hBrush) ;
	DeleteObject (hPen) ;
}

void psy_ui_drawsolidpolygon(psy_ui_Graphics* g, psy_ui_Point* pts,
	unsigned int numpoints, unsigned int inner, unsigned int outter)
{
	HBRUSH hBrush;     
	HBRUSH hBrushPrev;
	HPEN hPen;     
	HPEN hPenPrev;
	POINT* wpts;
	unsigned int i;

	wpts = (POINT*)malloc(sizeof(POINT) * numpoints);
	for (i = 0; i < numpoints; ++i) {
		wpts[i].x = pts[i].x;
		wpts[i].y = pts[i].y;
	}    
    hBrush = CreateSolidBrush(inner);
	hBrushPrev = SelectObject(g->hdc, hBrush);
	hPen = CreatePen(PS_SOLID, 1, outter);
	hPenPrev = SelectObject(g->hdc, hPen);	
	Polygon(g->hdc, wpts, numpoints);
	SelectObject(g->hdc, hBrushPrev);
	SelectObject(g->hdc, hPenPrev);
    DeleteObject (hBrush);
	DeleteObject (hPen);
	free(wpts);
}

void psy_ui_drawfullbitmap(psy_ui_Graphics* g, psy_ui_Bitmap* bitmap, int x, int y)
{
	HDC hdcMem;
	psy_ui_Size size;

	hdcMem = CreateCompatibleDC (g->hdc) ;
	SelectObject (hdcMem, bitmap->hBitmap) ;
	size = psy_ui_bitmap_size(bitmap);
	BitBlt(g->hdc, x, y, size.width, size.height, hdcMem, 0, 0, SRCCOPY);
	DeleteDC (hdcMem);  
}

void psy_ui_drawbitmap(psy_ui_Graphics* g, psy_ui_Bitmap* bitmap, int x, int y, int width,
	int height, int xsrc, int ysrc)
{
	HDC hdcMem;

	hdcMem = CreateCompatibleDC (g->hdc) ;
	SelectObject (hdcMem, bitmap->hBitmap) ;	
	BitBlt(g->hdc, x, y, width, height, hdcMem, xsrc, ysrc, SRCCOPY);
	DeleteDC (hdcMem);  
}

void psy_ui_setcolor(psy_ui_Graphics* g, unsigned int color)
{
	HPEN pen;	
	pen = CreatePen(PS_SOLID, 1, color);
	SelectObject(g->hdc, pen);
	if (g->pen) {
		DeleteObject(g->pen);
	}
	g->pen = pen;
}

void psy_ui_setbackgroundmode(psy_ui_Graphics* g, unsigned int mode)
{
	SetBkMode(g->hdc, mode);
}

void psy_ui_setbackgroundcolor(psy_ui_Graphics* g, unsigned int color)
{
	SetBkColor(g->hdc, color);
}

void psy_ui_settextcolor(psy_ui_Graphics* g, unsigned int color)
{
	SetTextColor(g->hdc, color);
}

void psy_ui_setfont(psy_ui_Graphics* g, psy_ui_Font* font)
{	
	if (font && font->hfont) {		
		SelectObject(g->hdc, font->hfont);
	}
}

void psy_ui_drawline(psy_ui_Graphics* g, int x1, int y1, int x2, int y2)
{
	MoveToEx(g->hdc, x1, y1, NULL) ;
	LineTo (g->hdc, x2, y2);
}

void psy_ui_moveto(psy_ui_Graphics* g, psy_ui_Point pt)
{	
	MoveToEx(g->hdc, pt.x, pt.y, NULL) ;	
}

void psy_ui_devcurveto(psy_ui_Graphics* g, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p)
{
   POINT pts[3];
   
   pts[0].x = control_p1.x;
   pts[0].y = control_p1.y;
   pts[1].x = control_p2.x;
   pts[1].y = control_p2.y;
   pts[2].x = p.x;
   pts[2].y = p.y;
   PolyBezierTo(g->hdc, pts, 3);
}

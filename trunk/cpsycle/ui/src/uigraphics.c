// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uigraphics.h"

extern ui_font defaultfont;

void ui_graphics_init(ui_graphics* g, HDC hdc)
{
	g->hdc = hdc;		
	g->pen = CreatePen(PS_SOLID, 1, 0x00666666);	
	g->brush = 0;
	g->hBrushPrev = 0;
	g->penprev = SelectObject(g->hdc, g->pen);
	g->hFontPrev = SelectObject(g->hdc, defaultfont.hfont);
}	

void ui_graphics_dispose(ui_graphics* g)
{
	SelectObject(g->hdc, g->penprev);
	SelectObject(g->hdc, g->hFontPrev);
	if (g->pen) {		
		DeleteObject(g->pen);
	}	
}

void ui_textout(ui_graphics* self, int x, int y, const char* str, size_t len)
{	
	TextOut(self->hdc, x, y, str, (int)len);
}

void ui_textoutrectangle(ui_graphics* g, int x, int y, unsigned int options,
	ui_rectangle r, const char* text, size_t len)
{
	RECT rect;
		                
    SetRect (&rect, r.left, r.top, r.right, r.bottom) ;     	
	ExtTextOut(g->hdc, x, y, options, &rect, text, (int)len, NULL);
}

ui_size ui_textsize(ui_graphics* g, const char* text)
{
	ui_size	rv;
	SIZE size;
	
	GetTextExtentPoint(g->hdc, text, (int)strlen(text), &size) ;	
	rv.width = size.cx; 
	rv.height = size.cy;
	return rv;
}

void ui_drawrectangle(ui_graphics* self, const ui_rectangle r)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;

	hBrush = GetStockObject (NULL_BRUSH);
	hOldBrush = SelectObject (self->hdc, hBrush);
	Rectangle(self->hdc, r.left, r.top, r.right, r.bottom);
	SelectObject (self->hdc, hOldBrush);
}

void ui_drawroundrectangle(ui_graphics* self, const ui_rectangle r, ui_size cornersize)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;

	hBrush = GetStockObject (NULL_BRUSH);
	hOldBrush = SelectObject (self->hdc, hBrush);
	RoundRect(self->hdc, r.left, r.top, r.right, r.bottom, cornersize.width,
		cornersize.height);
	SelectObject (self->hdc, hOldBrush);
}

void ui_drawsolidrectangle(ui_graphics* g, const ui_rectangle r, unsigned int color)
{
     HBRUSH hBrush;     
     RECT   rect;	 
	                
     SetRect (&rect, r.left, r.top, r.right, r.bottom) ;     
     hBrush = CreateSolidBrush(color);
     FillRect (g->hdc, &rect, hBrush);     
     DeleteObject (hBrush) ;
}

void ui_drawsolidroundrectangle(ui_graphics* self, const ui_rectangle r,
	ui_size cornersize, unsigned int color)
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

void ui_drawsolidpolygon(ui_graphics* g, ui_point* pts, unsigned int numpoints, 
	unsigned int inner, unsigned int outter)
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

void ui_drawfullbitmap(ui_graphics* g, ui_bitmap* bitmap, int x, int y)
{
	HDC hdcMem;
	ui_size size;

	hdcMem = CreateCompatibleDC (g->hdc) ;
	SelectObject (hdcMem, bitmap->hBitmap) ;
	size = ui_bitmap_size(bitmap);
	BitBlt(g->hdc, x, y, size.width, size.height, hdcMem, 0, 0, SRCCOPY);
	DeleteDC (hdcMem);  
}

void ui_drawbitmap(ui_graphics* g, ui_bitmap* bitmap, int x, int y, int width,
	int height, int xsrc, int ysrc)
{
	HDC hdcMem;

	hdcMem = CreateCompatibleDC (g->hdc) ;
	SelectObject (hdcMem, bitmap->hBitmap) ;	
	BitBlt(g->hdc, x, y, width, height, hdcMem, xsrc, ysrc, SRCCOPY);
	DeleteDC (hdcMem);  
}

void ui_setcolor(ui_graphics* g, unsigned int color)
{
	HPEN pen;	
	pen = CreatePen(PS_SOLID, 1, color);
	SelectObject(g->hdc, pen);
	if (g->pen) {
		DeleteObject(g->pen);
	}
	g->pen = pen;
}

void ui_setbackgroundmode(ui_graphics* g, unsigned int mode)
{
	SetBkMode(g->hdc, mode);
}

void ui_setbackgroundcolor(ui_graphics* g, unsigned int color)
{
	SetBkColor(g->hdc, color);
}

void ui_settextcolor(ui_graphics* g, unsigned int color)
{
	SetTextColor(g->hdc, color);
}

void ui_setfont(ui_graphics* g, ui_font* font)
{	
	if (font && font->hfont) {		
		SelectObject(g->hdc, font->hfont);
	}
}

ui_font ui_createfont(const char* name, int size)
{
	ui_font font;
	font.hfont = CreateFont(size, 0, 0, 0, 0, 0, 0, 0, 
		1, 0, 0, 0, VARIABLE_PITCH | FF_DONTCARE, name);          
	font.stock = 0;
	return font;
}

void ui_drawline(ui_graphics* g, int x1, int y1, int x2, int y2)
{
	MoveToEx(g->hdc, x1, y1, NULL) ;
	LineTo (g->hdc, x2, y2);
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uidef.h"
#include "uiapp.h"
#include <windows.h>
#include <math.h>

#define EZ_ATTR_BOLD          1
#define EZ_ATTR_ITALIC        2
#define EZ_ATTR_UNDERLINE     4
#define EZ_ATTR_STRIKEOUT     8

extern psy_ui_App app;

static LOGFONT ui_fontinfo_make(HDC hdc, TCHAR * szFaceName, int iDeciPtHeight,
	int iDeciPtWidth, int iAttributes, BOOL fLogRes);

psy_ui_Point psy_ui_point_make(int x, int y)
{
	psy_ui_Point rv;

	rv.x = x;
	rv.y = y;
	return rv;
}

void psy_ui_setrectangle(psy_ui_Rectangle* self, int left, int top, int width,
	int height)
{
   self->left = left;
   self->top = top;
   self->right = left + width;
   self->bottom = top + height;   
}

int psy_ui_rectangle_intersect_rectangle(const psy_ui_Rectangle* self,
	const psy_ui_Rectangle* other)
{
	return !(other->left > self->right ||
		other->right < self->left ||
		other->top > self->bottom ||
		other->bottom < self->top);
}

int psy_ui_rectangle_intersect(psy_ui_Rectangle* self, int x, int y)
{
	return (x >= self->left && x < self->right && 
			y >= self->top && y < self->bottom);
}

void psy_ui_rectangle_union(psy_ui_Rectangle* self,
	const psy_ui_Rectangle* other)
{
	self->left = self->left < other->left ? self->left : other->left;
	self->right = self->right > other->right ? self->right : other->right;
	self->top = self->top < other->top ? self->top : other->top;
	self->bottom = self->bottom > other->bottom ? self->bottom : other->bottom;
}

void psy_ui_margin_init(psy_ui_Margin* self, psy_ui_Value top,
	psy_ui_Value right, psy_ui_Value bottom, psy_ui_Value left)
{   
   self->top = top;
   self->right = right;
   self->bottom = bottom;
   self->left = left;
}

intptr_t psy_ui_margin_width_px(psy_ui_Margin* self,
	const psy_ui_TextMetric* tm)
{
	return psy_ui_value_px(&self->left, tm) +
		psy_ui_value_px(&self->right, tm);
}

intptr_t psy_ui_margin_height_px(psy_ui_Margin* self,
	const psy_ui_TextMetric* tm)
{
	return psy_ui_value_px(&self->top, tm) +
		psy_ui_value_px(&self->bottom, tm);
}

psy_ui_Value psy_ui_value_makepx(intptr_t px)
{
	psy_ui_Value rv;

	rv.quantity.integer = px;
	rv.unit = psy_ui_UNIT_PX;
	return rv;
}

psy_ui_Value psy_ui_value_makeew(double em)
{
	psy_ui_Value rv;

	rv.quantity.real = em;
	rv.unit = psy_ui_UNIT_EW;
	return rv;
}

psy_ui_Value psy_ui_value_makeeh(double em)
{
	psy_ui_Value rv;

	rv.quantity.real = em;
	rv.unit = psy_ui_UNIT_EH;
	return rv;
}

intptr_t psy_ui_value_px(psy_ui_Value* self, const psy_ui_TextMetric* tm)
{
	intptr_t rv = self->quantity.integer;

	switch (self->unit) {
		case psy_ui_UNIT_PX:
			rv = self->quantity.integer;
		break;
		case psy_ui_UNIT_EW:
			if (tm) {
				rv = (intptr_t)(self->quantity.real * tm->tmAveCharWidth);
			}
		break;
		case psy_ui_UNIT_EH:
			if (tm) {
				rv = (intptr_t)(self->quantity.real * tm->tmHeight);
			}
		break;
		default:			
		break;
	}
	return rv;
}

void psy_ui_error(const char* err, const char* shorterr)
{
	MessageBox(NULL, err, shorterr, MB_OK | MB_ICONERROR);
}

void psy_ui_fontinfo_init(psy_ui_FontInfo* self, const char* family,
	int height)
{
	HDC hdc = GetDC (NULL) ;
	self->lf = ui_fontinfo_make(hdc, (TCHAR*)family, height, 0, 0, 0);
	ReleaseDC (NULL, hdc) ;
}

LOGFONT ui_fontinfo_make(HDC hdc, TCHAR * szFaceName, int iDeciPtHeight,
	int iDeciPtWidth, int iAttributes, BOOL fLogRes)
{
     FLOAT      cxDpi, cyDpi ;
     LOGFONT    lf ;
     POINT      pt ;
//     TEXTMETRIC tm ;
     
     SaveDC (hdc) ;
     
     SetGraphicsMode (hdc, GM_ADVANCED) ;
     ModifyWorldTransform (hdc, NULL, MWT_IDENTITY) ;
     SetViewportOrgEx (hdc, 0, 0, NULL) ;
     SetWindowOrgEx   (hdc, 0, 0, NULL) ;
     
     if (fLogRes)
     {
          cxDpi = (FLOAT) GetDeviceCaps (hdc, LOGPIXELSX) ;
          cyDpi = (FLOAT) GetDeviceCaps (hdc, LOGPIXELSY) ;
     }
     else
     {
          cxDpi = (FLOAT) (25.4 * GetDeviceCaps (hdc, HORZRES) /
                                        GetDeviceCaps (hdc, HORZSIZE)) ;
          
          cyDpi = (FLOAT) (25.4 * GetDeviceCaps (hdc, VERTRES) /
                                        GetDeviceCaps (hdc, VERTSIZE)) ;
     }
     
     pt.x = (int) (iDeciPtWidth  * cxDpi / 72) ;
     pt.y = (int) (iDeciPtHeight * cyDpi / 72) ;
     
     DPtoLP (hdc, &pt, 1) ;
     lf.lfHeight         = - (int) (fabs (pt.y) / 10.0 + 0.5) ;
     lf.lfWidth          = 0 ;
     lf.lfEscapement     = 0 ;
     lf.lfOrientation    = 0 ;
     lf.lfWeight         = iAttributes & EZ_ATTR_BOLD      ? 700 : 0 ;
     lf.lfItalic         = iAttributes & EZ_ATTR_ITALIC    ?   1 : 0 ;
     lf.lfUnderline      = iAttributes & EZ_ATTR_UNDERLINE ?   1 : 0 ;
     lf.lfStrikeOut      = iAttributes & EZ_ATTR_STRIKEOUT ?   1 : 0 ;
     lf.lfCharSet        = DEFAULT_CHARSET ;
     lf.lfOutPrecision   = 0 ;
     lf.lfClipPrecision  = 0 ;
     lf.lfQuality        = 0 ;
     lf.lfPitchAndFamily = 0 ;
     
     lstrcpy (lf.lfFaceName, szFaceName) ;     
     RestoreDC (hdc, -1) ;
     return lf ;
}


void psy_ui_font_init(psy_ui_Font* self, const psy_ui_FontInfo* fontinfo)
{		
	if (fontinfo) {
		HDC hdc = GetDC (NULL) ;
		SaveDC (hdc) ;          
		self->hfont = CreateFontIndirect(&fontinfo->lf);
		RestoreDC (hdc, -1);
		ReleaseDC(NULL, hdc);
	} else {
		self->hfont = 0;
		self->stock = 0;
	}
}

void psy_ui_font_copy(psy_ui_Font* self, const psy_ui_Font* other)
{
	if (self->hfont && self->hfont != app.defaults.defaultfont.hfont) {
		psy_ui_font_dispose(self);
	}
	if (other->hfont && other->hfont != app.defaults.defaultfont.hfont) {
		LOGFONT lf;
		GetObject(other->hfont, sizeof(LOGFONT), &lf);
		self->hfont = CreateFontIndirect(&lf);
	}
	if (other->hfont == app.defaults.defaultfont.hfont) {
		self->hfont = app.defaults.defaultfont.hfont;
	}
}

void psy_ui_font_dispose(psy_ui_Font* self)
{
	if (self->hfont && self->hfont != app.defaults.defaultfont.hfont) {
		DeleteObject(self->hfont);
	}
	self->hfont = 0;
	self->stock = 0;
}

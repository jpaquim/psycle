// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uidef.h"
#include <windows.h>
#include <math.h>

#define EZ_ATTR_BOLD          1
#define EZ_ATTR_ITALIC        2
#define EZ_ATTR_UNDERLINE     4
#define EZ_ATTR_STRIKEOUT     8

extern ui_font defaultfont;

static LOGFONT ui_fontinfo_make(HDC hdc, TCHAR * szFaceName, int iDeciPtHeight,
	int iDeciPtWidth, int iAttributes, BOOL fLogRes);

void ui_setrectangle(ui_rectangle* self, int left, int top, int width, int height)
{
   self->left = left;
   self->top = top;
   self->right = left + width;
   self->bottom = top + height;   
}

int ui_rectangle_intersect_rectangle(const ui_rectangle* self, const ui_rectangle* other)
{
	return !(other->left > self->right ||
		other->right < self->left ||
		other->top > self->bottom ||
		other->bottom < self->top);
}

int ui_rectangle_intersect(ui_rectangle* self, int x, int y)
{
	return (x >= self->left && x < self->right && 
			y >= self->top && y < self->bottom);
}

void ui_margin_init(ui_margin* self, ui_value top, ui_value right,
	ui_value bottom, ui_value left)
{   
   self->top = top;
   self->right = right;
   self->bottom = bottom;
   self->left = left;
}

intptr_t ui_margin_width_px(ui_margin* self, const ui_textmetric* tm)
{
	return ui_value_px(&self->left, tm) + ui_value_px(&self->right, tm);
}

intptr_t ui_margin_height_px(ui_margin* self, const ui_textmetric* tm)
{
	return ui_value_px(&self->top, tm) + ui_value_px(&self->bottom, tm);
}

ui_value ui_value_makepx(intptr_t px)
{
	ui_value rv;

	rv.quantity.integer = px;
	rv.unit = UI_UNIT_PX;
	return rv;
}

ui_value ui_value_makeew(double em)
{
	ui_value rv;

	rv.quantity.real = em;
	rv.unit = UI_UNIT_EW;
	return rv;
}

ui_value ui_value_makeeh(double em)
{
	ui_value rv;

	rv.quantity.real = em;
	rv.unit = UI_UNIT_EH;
	return rv;
}

intptr_t ui_value_px(ui_value* self, const ui_textmetric* tm)
{
	intptr_t rv = self->quantity.integer;

	switch (self->unit) {
		case UI_UNIT_PX:
			rv = self->quantity.integer;
		break;
		case UI_UNIT_EW:
			if (tm) {
				rv = (intptr_t)(self->quantity.real * tm->tmAveCharWidth);
			}
		break;
		case UI_UNIT_EH:
			if (tm) {
				rv = (intptr_t)(self->quantity.real * tm->tmHeight);
			}
		break;
		default:			
		break;
	}
	return rv;
}

void ui_error(const char* err, const char* shorterr)
{
	MessageBox(NULL, err, shorterr, MB_OK | MB_ICONERROR);
}

void ui_fontinfo_init(ui_fontinfo* self, const char* family, int height)
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


void ui_font_init(ui_font* self, const ui_fontinfo* fontinfo)
{		
	if (fontinfo) {
		HDC hdc = GetDC (NULL) ;
		SaveDC (hdc) ;          
		self->hfont = CreateFontIndirect (&fontinfo->lf) ;         
		RestoreDC (hdc, -1);
		ReleaseDC(NULL, hdc);
	} else {
		self->hfont = 0;
		self->stock = 0;
	}
}

void ui_font_copy(ui_font* self, const ui_font* other)
{
	if (self->hfont && self->hfont != defaultfont.hfont) {
		ui_font_dispose(self);		
	}
	if (other->hfont && other->hfont != defaultfont.hfont) {
		LOGFONT lf;
		GetObject(other->hfont, sizeof(LOGFONT), &lf);
		self->hfont = CreateFontIndirect(&lf);
	}
	if (other->hfont == defaultfont.hfont) {
		self->hfont = defaultfont.hfont;
	}
}

void ui_font_dispose(ui_font* self)
{
	if (self->hfont && self->hfont != defaultfont.hfont) {
		DeleteObject(self->hfont);
	}
	self->hfont = 0;
	self->stock = 0;
}

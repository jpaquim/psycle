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

void ui_margin_init(ui_margin* self, int top, int right, int bottom, int left)
{   
   self->top = top;
   self->right = right;
   self->bottom = bottom;   
   self->left = left;
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
    HDC hdc = GetDC (NULL) ;
    SaveDC (hdc) ;          
	self->hfont = CreateFontIndirect (&fontinfo->lf) ;         
	RestoreDC (hdc, -1);
	ReleaseDC(NULL, hdc);     
}

void ui_font_dispose(ui_font* self)
{
	if (!self->stock && self->hfont) {		
		DeleteObject(self->hfont);
	}
	self->hfont = 0;
	self->stock = 0;
}

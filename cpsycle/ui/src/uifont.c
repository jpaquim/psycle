// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "uifont.h"
#include "uiwinfontimp.h"
#include <stdlib.h>
#include <math.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#define EZ_ATTR_BOLD          1
#define EZ_ATTR_ITALIC        2
#define EZ_ATTR_UNDERLINE     4
#define EZ_ATTR_STRIKEOUT     8

// VTable Prototypes
static void dispose(psy_ui_Font*);
static void copy(psy_ui_Font*, psy_ui_Font* other);

// VTable init
static psy_ui_FontVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.dispose = dispose;		
		vtable.copy = copy;
		vtable_initialized = 1;
	}
}

void psy_ui_font_init(psy_ui_Font* self, const psy_ui_FontInfo* fontinfo)
{
	vtable_init();
	self->vtable = &vtable;
	self->imp = (psy_ui_FontImp*) malloc(sizeof(psy_ui_win_FontImp));
	psy_ui_win_fontimp_init((psy_ui_win_FontImp*)self->imp, fontinfo);	
}

// Delegation Methods to FontImp
void dispose(psy_ui_Font* self)
{	
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = 0;
}

void copy(psy_ui_Font* self, psy_ui_Font* other)
{
	self->imp->vtable->dev_copy(self->imp, other->imp);	
}

// psy_ui_FontImp
static void psy_ui_imp_dispose(psy_ui_FontImp* self) { }
static void psy_ui_imp_copy(psy_ui_FontImp* self, psy_ui_FontImp* other) { }

static psy_ui_FontImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = psy_ui_imp_dispose;		
		imp_vtable.dev_copy = psy_ui_imp_copy;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_font_imp_init(psy_ui_FontImp* self)
{
	imp_vtable_init();	
	self->vtable = &imp_vtable;
}


//
// fontinfo

static psy_ui_FontInfo psy_ui_fontinfo(LOGFONT lf);
static LOGFONT ui_fontinfo_make(HDC hdc, TCHAR* szFaceName, int iDeciPtHeight,
    int iDeciPtWidth, int iAttributes, BOOL fLogRes);

void psy_ui_fontinfo_init(psy_ui_FontInfo* self, const char* family,
    int height)
{
    LOGFONT lf;

    HDC hdc = GetDC(NULL);
    lf = ui_fontinfo_make(hdc, (TCHAR*)family, height, 0, 0, 0);
    *self = psy_ui_fontinfo(lf);
    ReleaseDC(NULL, hdc);
}

psy_ui_FontInfo psy_ui_fontinfo(LOGFONT lf)
{
    psy_ui_FontInfo rv;

    rv.lfHeight = lf.lfHeight;
    rv.lfWidth = lf.lfWidth;
    rv.lfEscapement = lf.lfEscapement;
    rv.lfOrientation = lf.lfOrientation;
    rv.lfWeight = lf.lfWeight;
    rv.lfItalic = lf.lfItalic;
    rv.lfUnderline = lf.lfUnderline;
    rv.lfStrikeOut = lf.lfStrikeOut;
    rv.lfCharSet = lf.lfCharSet;
    rv.lfOutPrecision = lf.lfOutPrecision;
    rv.lfClipPrecision = lf.lfClipPrecision;
    rv.lfQuality = lf.lfQuality;
    rv.lfPitchAndFamily = lf.lfPitchAndFamily;
    memcpy(rv.lfFaceName, lf.lfFaceName, 32); // TODO UNICODE
    return rv;
}

LOGFONT ui_fontinfo_make(HDC hdc, TCHAR* szFaceName, int iDeciPtHeight,
    int iDeciPtWidth, int iAttributes, BOOL fLogRes)
{
    FLOAT      cxDpi, cyDpi;
    LOGFONT    lf;
    POINT      pt;
    //     TEXTMETRIC tm ;

    SaveDC(hdc);

    SetGraphicsMode(hdc, GM_ADVANCED);
    ModifyWorldTransform(hdc, NULL, MWT_IDENTITY);
    SetViewportOrgEx(hdc, 0, 0, NULL);
    SetWindowOrgEx(hdc, 0, 0, NULL);

    if (fLogRes)
    {
        cxDpi = (FLOAT)GetDeviceCaps(hdc, LOGPIXELSX);
        cyDpi = (FLOAT)GetDeviceCaps(hdc, LOGPIXELSY);
    }
    else
    {
        cxDpi = (FLOAT)(25.4 * GetDeviceCaps(hdc, HORZRES) /
            GetDeviceCaps(hdc, HORZSIZE));

        cyDpi = (FLOAT)(25.4 * GetDeviceCaps(hdc, VERTRES) /
            GetDeviceCaps(hdc, VERTSIZE));
    }

    pt.x = (int)(iDeciPtWidth * cxDpi / 72);
    pt.y = (int)(iDeciPtHeight * cyDpi / 72);

    DPtoLP(hdc, &pt, 1);
    lf.lfHeight = -(int)(fabs(pt.y) / 10.0 + 0.5);
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = iAttributes & EZ_ATTR_BOLD ? 700 : 0;
    lf.lfItalic = iAttributes & EZ_ATTR_ITALIC ? 1 : 0;
    lf.lfUnderline = iAttributes & EZ_ATTR_UNDERLINE ? 1 : 0;
    lf.lfStrikeOut = iAttributes & EZ_ATTR_STRIKEOUT ? 1 : 0;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = 0;
    lf.lfClipPrecision = 0;
    lf.lfQuality = 0;
    lf.lfPitchAndFamily = 0;

    lstrcpy(lf.lfFaceName, szFaceName);
    RestoreDC(hdc, -1);
    return lf;
}

/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiwinfontimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "../../uiapp.h"
#include <stdlib.h>
#include <string.h>

/* prototypes */
static void psy_ui_win_font_imp_dispose(psy_ui_win_FontImp*);
static void psy_ui_win_font_imp_copy(psy_ui_win_FontImp*, psy_ui_win_FontImp* other);
static psy_ui_FontInfo dev_fontinfo(psy_ui_win_FontImp*);
static const psy_ui_TextMetric* dev_textmetric(const psy_ui_win_FontImp*);
static bool dev_equal(const psy_ui_win_FontImp*, const psy_ui_win_FontImp* other);

static LOGFONT logfont(psy_ui_FontInfo lf);
static psy_ui_FontInfo psy_ui_fontinfo(LOGFONT);

/* vtable */
static psy_ui_FontImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_win_FontImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose =
			(psy_ui_font_imp_fp_dispose)
			psy_ui_win_font_imp_dispose;
		imp_vtable.dev_copy =
			(psy_ui_font_imp_fp_copy)
			psy_ui_win_font_imp_copy;
		imp_vtable.dev_fontinfo =
			(psy_ui_font_imp_fp_dev_fontinfo)
			dev_fontinfo;
		imp_vtable.dev_textmetric =
			(psy_ui_font_imp_fp_dev_textmetric)
			dev_textmetric;
		imp_vtable.dev_equal =
			(psy_ui_font_imp_fp_dev_equal)
			dev_equal;
		imp_vtable_initialized = TRUE;
	}
	self->imp.vtable = &imp_vtable;
}

void psy_ui_win_fontimp_init(psy_ui_win_FontImp* self, const psy_ui_FontInfo* fontinfo)
{
	psy_ui_font_imp_init(&self->imp);
	imp_vtable_init(self);
	self->tmcachevalid = FALSE;
	if (fontinfo) {		
		LOGFONT lf;

		lf = logfont(*fontinfo);
		self->hfont = CreateFontIndirect(&lf);		
	} else {
		self->hfont = 0;		
	}	
}

void psy_ui_win_font_imp_dispose(psy_ui_win_FontImp* self)
{	
	if (self->hfont) {
		DeleteObject(self->hfont);	
		self->hfont = 0;
	}
}

void psy_ui_win_font_imp_copy(psy_ui_win_FontImp* self, psy_ui_win_FontImp* other)
{				
	LOGFONT lf;

	psy_ui_win_font_imp_dispose(self);
	GetObject(other->hfont, sizeof(LOGFONT), &lf);
	self->hfont = CreateFontIndirect(&lf);
	self->tmcachevalid = other->tmcachevalid;
	if (other->tmcachevalid) {
		self->tmcache = other->tmcache;
	}	
}

psy_ui_FontInfo dev_fontinfo(psy_ui_win_FontImp* self)
{
	psy_ui_FontInfo rv;
	int ret;	
	LOGFONT lf;

	ret = GetObject(self->hfont, sizeof(lf), &lf);
	if (ret == 0) {
		psy_ui_fontinfo_init(&rv, "", 0);
	} else {		
		rv = psy_ui_fontinfo(lf);		
	}
	return rv;
}

const psy_ui_TextMetric* dev_textmetric(const psy_ui_win_FontImp* self)
{
	psy_ui_TextMetric rv;
		
	if (!self->tmcachevalid) {
		TEXTMETRIC tm;
		HDC hdc;
		HFONT hPrevFont = 0;
		HFONT hfont = 0;
		const psy_ui_Font* font = NULL;

		hdc = GetDC(NULL);
		SaveDC(hdc);
		hfont = self->hfont;
		if (hfont) {
			hPrevFont = SelectObject(hdc, hfont);
		}
		GetTextMetrics(hdc, &tm);
		if (font) {
			if (hPrevFont) {
				SelectObject(hdc, hPrevFont);
			}
		}
		RestoreDC(hdc, -1);
		ReleaseDC(NULL, hdc);
		rv.tmHeight = tm.tmHeight;
		rv.tmAscent = tm.tmAscent;
		rv.tmDescent = tm.tmDescent;
		rv.tmInternalLeading = tm.tmInternalLeading;
		rv.tmExternalLeading = tm.tmExternalLeading;
		rv.tmAveCharWidth = tm.tmAveCharWidth;
		rv.tmMaxCharWidth = tm.tmMaxCharWidth;
		rv.tmWeight = tm.tmWeight;
		rv.tmOverhang = tm.tmOverhang;
		rv.tmDigitizedAspectX = tm.tmDigitizedAspectX;
		rv.tmDigitizedAspectY = tm.tmDigitizedAspectY;
		rv.tmFirstChar = tm.tmFirstChar;
		rv.tmLastChar = tm.tmLastChar;
		rv.tmDefaultChar = tm.tmDefaultChar;
		rv.tmBreakChar = tm.tmBreakChar;
		rv.tmItalic = tm.tmItalic;
		rv.tmUnderlined = tm.tmUnderlined;
		rv.tmStruckOut = tm.tmStruckOut;
		rv.tmPitchAndFamily = tm.tmPitchAndFamily;
		rv.tmCharSet = tm.tmCharSet;
		((psy_ui_win_FontImp*)self)->tmcache = rv;
		((psy_ui_win_FontImp*)self)->tmcachevalid = TRUE;		
	}
	return &self->tmcache;
}

LOGFONT logfont(psy_ui_FontInfo lf)
{
	LOGFONT rv;

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

bool dev_equal(const psy_ui_win_FontImp* self, const psy_ui_win_FontImp* other)
{
	return self->hfont == other->hfont;
}

#endif

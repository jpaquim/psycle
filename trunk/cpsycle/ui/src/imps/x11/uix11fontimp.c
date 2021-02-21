// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uix11fontimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT

#include "uiapp.h"
#include <stdlib.h>
#include <string.h>
#include "uix11app.h"

extern psy_ui_App app;

// VTable Prototypes
static void psy_ui_x11_font_imp_dispose(psy_ui_x11_FontImp*);
static void psy_ui_x11_font_imp_copy(psy_ui_x11_FontImp*, psy_ui_x11_FontImp* other);
static psy_ui_FontInfo dev_fontinfo(psy_ui_x11_FontImp*);

// static LOGFONT logfont(psy_ui_FontInfo lf);
// static psy_ui_FontInfo psy_ui_fontinfo(LOGFONT);

// VTable init
static psy_ui_FontImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(psy_ui_x11_FontImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose = (psy_ui_font_imp_fp_dispose)
			psy_ui_x11_font_imp_dispose;		
		imp_vtable.dev_copy = (psy_ui_font_imp_fp_copy)psy_ui_x11_font_imp_copy;
		imp_vtable.dev_fontinfo = (psy_ui_font_imp_fp_dev_fontinfo)dev_fontinfo;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_x11_fontimp_init(psy_ui_x11_FontImp* self, const psy_ui_FontInfo*
	fontinfo)
{
	printf("Create x11 fontimp \n");
	psy_ui_font_imp_init(&self->imp);
	imp_vtable_init(self);	
	self->imp.vtable = &imp_vtable;
	if (fontinfo) {
		psy_ui_X11App* x11app;		

		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		self->hfont = XftFontOpenXlfd(
			x11app->dpy,
			DefaultScreen(x11app->dpy),
			fontinfo->lfFaceName);
		if (!self->hfont) {
			self->hfont = XftFontOpenName(x11app->dpy,
				DefaultScreen(x11app->dpy),
				fontinfo->lfFaceName);
		}
	} else {
		self->hfont = 0;		
	}
}

// win32 implementation method for psy_ui_Font
void psy_ui_x11_font_imp_dispose(psy_ui_x11_FontImp* self)
{	
	if (self->hfont) {
		psy_ui_X11App* x11app;		

		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		XftFontClose(x11app->dpy, self->hfont);	
		self->hfont = 0;
	}
}

void psy_ui_x11_font_imp_copy(psy_ui_x11_FontImp* self, psy_ui_x11_FontImp* other)
{				
//	LOGFONT lf;

//	psy_ui_x11_font_imp_dispose(self);
//	GetObject(other->hfont, sizeof(LOGFONT), &lf);
//	self->hfont = CreateFontIndirect(&lf);	
}

psy_ui_FontInfo dev_fontinfo(psy_ui_x11_FontImp* self)
{
	psy_ui_FontInfo rv;
	psy_ui_fontinfo_init(&rv, "arial", 12);
	return rv;
}

/*
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
*/

#endif

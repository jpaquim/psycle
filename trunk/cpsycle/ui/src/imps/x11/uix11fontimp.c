/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uix11fontimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11

#include "uiapp.h"
#include <stdlib.h>
#include <string.h>
#include "uix11app.h"
#include "uix11graphicsimp.h"

/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void psy_ui_x11_font_imp_dispose(psy_ui_x11_FontImp*);
static void psy_ui_x11_font_imp_copy(psy_ui_x11_FontImp*, psy_ui_x11_FontImp* other);
static psy_ui_FontInfo dev_fontinfo(psy_ui_x11_FontImp*);
static const psy_ui_TextMetric* dev_textmetric(const psy_ui_x11_FontImp*);
static bool dev_equal(const psy_ui_x11_FontImp*, const psy_ui_x11_FontImp* other);
static psy_ui_Size dev_textsize(const psy_ui_x11_FontImp*,
	const char* text, uintptr_t count);

/* vtable */
static psy_ui_FontImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_x11_FontImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose = (psy_ui_font_imp_fp_dispose)
			psy_ui_x11_font_imp_dispose;		
		imp_vtable.dev_copy =
			(psy_ui_font_imp_fp_copy)
			psy_ui_x11_font_imp_copy;
		imp_vtable.dev_fontinfo =
			(psy_ui_font_imp_fp_dev_fontinfo)
			dev_fontinfo;
		imp_vtable.dev_textmetric =
			(psy_ui_font_imp_fp_dev_textmetric)
			dev_textmetric;
		imp_vtable.dev_equal =
			(psy_ui_font_imp_fp_dev_equal)
			dev_equal;
		imp_vtable.dev_textsize =
			(psy_ui_font_imp_fp_dev_textsize)
			dev_textsize;
		imp_vtable_initialized = TRUE;
	}
	self->imp.vtable = &imp_vtable;
}

/* implementation */
void psy_ui_x11_fontimp_init(psy_ui_x11_FontImp* self, const psy_ui_FontInfo*
	fontinfo)
{	
	psy_ui_font_imp_init(&self->imp);
	imp_vtable_init(self);
	self->tmcachevalid = FALSE;	
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
	self->tmcachevalid = other->tmcachevalid;
	if (other->tmcachevalid) {
		self->tmcache = other->tmcache;
	}
}

psy_ui_FontInfo dev_fontinfo(psy_ui_x11_FontImp* self)
{
	psy_ui_FontInfo rv;
	
	psy_ui_fontinfo_init(&rv, "arial", 12);
	return rv;
}

const psy_ui_TextMetric* dev_textmetric(const psy_ui_x11_FontImp* self)
{
	psy_ui_TextMetric rv;	
	
	if (!self->tmcachevalid) {
		psy_ui_X11App* x11app;
		GC gc;
		PlatformXtGC xgc;
		psy_ui_Graphics g;
		psy_ui_x11_GraphicsImp* gx11;

		rv.tmAveCharWidth = 10;
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		gc = XCreateGC(x11app->dpy, DefaultRootWindow(x11app->dpy), 0, 0);
		xgc.display = x11app->dpy;	
		xgc.window = DefaultRootWindow(x11app->dpy);
		xgc.visual = x11app->visual;
		xgc.gc = gc;
		psy_ui_graphics_init(&g, &xgc);
		gx11 = (psy_ui_x11_GraphicsImp*)g.imp;
		rv.tmHeight = gx11->xftfont->height;
		rv.tmAscent = gx11->xftfont->ascent;
		rv.tmDescent = gx11->xftfont->descent;
		rv.tmMaxCharWidth = gx11->xftfont->max_advance_width;
		rv.tmAveCharWidth = gx11->xftfont->max_advance_width / 4;
		rv.tmInternalLeading = 0;
		rv.tmExternalLeading = 0;
		psy_ui_graphics_dispose(&g);
		((psy_ui_x11_FontImp*)self)->tmcache = rv;
		((psy_ui_x11_FontImp*)self)->tmcachevalid = TRUE;
	}
	return &self->tmcache;
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

bool dev_equal(const psy_ui_x11_FontImp* self, const psy_ui_x11_FontImp* other)
{
	return self->hfont == other->hfont;
}

psy_ui_Size dev_textsize(const psy_ui_x11_FontImp* self,
	const char* text, uintptr_t count)
{		
	psy_ui_Size rv;
	psy_ui_X11App* x11app;
	GC gc;
	PlatformXtGC xgc;
	psy_ui_Graphics g;

	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		
	gc = XCreateGC(x11app->dpy, 
		XDefaultRootWindow(x11app->dpy), 0, NULL);	
	xgc.display =x11app->dpy;	
	xgc.window = XDefaultRootWindow(x11app->dpy);	
	xgc.visual = x11app->visual;
	xgc.gc = gc;
	psy_ui_graphics_init(&g, &xgc);
	rv = psy_ui_textsize(&g, text, psy_strlen(text));
	psy_ui_graphics_dispose(&g);
	return rv;
	
	return rv;
}

#endif

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
static void psy_ui_x11_font_imp_copy(psy_ui_x11_FontImp*,
	psy_ui_x11_FontImp* other);
static psy_ui_FontInfo dev_fontinfo(psy_ui_x11_FontImp*);
static const psy_ui_TextMetric* dev_textmetric(const
	psy_ui_x11_FontImp*);
static bool dev_equal(const psy_ui_x11_FontImp*,
	const psy_ui_x11_FontImp* other);
static psy_ui_Size dev_textsize(const psy_ui_x11_FontImp*,
	const char* text, uintptr_t count);

/* vtable */
static psy_ui_FontImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_x11_FontImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose =
			(psy_ui_font_imp_fp_dispose)
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
		self->hfont = XftFontOpen(
			x11app->dpy, 
			DefaultScreen(x11app->dpy),
			FC_FAMILY, FcTypeString, fontinfo->lfFaceName,
			FC_SIZE, FcTypeDouble, (double)fontinfo->lfHeight,
			FC_WEIGHT, FcTypeInteger, FC_WEIGHT_MEDIUM,
			FC_ANTIALIAS, FcTypeBool, TRUE,
			XFT_CORE, FcTypeBool, False,
			NULL);			
		if (!self->hfont) {
			self->hfont = XftFontOpenName(x11app->dpy,
				DefaultScreen(x11app->dpy),
				fontinfo->lfFaceName);
		}
		self->lfHeight = fontinfo->lfHeight;
	} else {
		self->hfont = 0;
		self->lfHeight = 12;
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

void psy_ui_x11_font_imp_copy(psy_ui_x11_FontImp* self,
	psy_ui_x11_FontImp* other)
{		
	psy_ui_X11App* x11app;		

	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	if (other->hfont) {
		self->hfont = XftFontCopy(x11app->dpy, other->hfont);	
		self->tmcachevalid = other->tmcachevalid;
		self->lfHeight = other->lfHeight;
		if (other->tmcachevalid) {
			self->tmcache = other->tmcache;
		}		
	} else {
		self->tmcachevalid = FALSE;
	}
}

psy_ui_FontInfo dev_fontinfo(psy_ui_x11_FontImp* self)
{		
	psy_ui_FontInfo rv;
	
	if (self->hfont) {
		FcChar8 *name;
		
		name = NULL;	
		if (FcPatternGetString(self->hfont->pattern, FC_FAMILY, 0,
				&name) == FcResultMatch) {
			psy_ui_fontinfo_init(&rv, name, self->lfHeight);
		} else {
			psy_ui_fontinfo_init(&rv, "FreeSans", self->lfHeight);
		}
		return rv;
	}
	psy_ui_fontinfo_init(&rv, "FreeSans", self->lfHeight);
	return rv;
}

const psy_ui_TextMetric* dev_textmetric(const psy_ui_x11_FontImp* self)
{
	psy_ui_TextMetric rv;	
	
	psy_ui_textmetric_init(&rv);	
	if (!self->hfont) {		
		((psy_ui_x11_FontImp*)self)->tmcache = rv;
	} else if (!self->tmcachevalid) {		
		rv.tmHeight = self->hfont->height;
		rv.tmAscent = self->hfont->ascent;
		rv.tmDescent = self->hfont->descent;		
		rv.tmMaxCharWidth = self->hfont->max_advance_width;
		/* todo monospace */
		rv.tmAveCharWidth = self->hfont->max_advance_width / 4.5;
		rv.tmInternalLeading = 0;
		rv.tmExternalLeading = 0;		
		((psy_ui_x11_FontImp*)self)->tmcache = rv;
		((psy_ui_x11_FontImp*)self)->tmcachevalid = TRUE;
	}
	return &self->tmcache;
}

bool dev_equal(const psy_ui_x11_FontImp* self, const
	psy_ui_x11_FontImp* other)
{
	return (self->hfont == other->hfont);
}

psy_ui_Size dev_textsize(const psy_ui_x11_FontImp* self,
	const char* text, uintptr_t count)
{		
	psy_ui_Size rv;
	XGlyphInfo extents;	
	psy_ui_X11App* x11app;
	
	if (!self->hfont) {
		printf("no xftfont\n");
		return psy_ui_size_zero();
	}				
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;		
	XftTextExtentsUtf8(x11app->dpy, self->hfont,
		(const FcChar8*)text,
		count,
		&extents);							
	rv.width = psy_ui_value_make_px(extents.width); 
	rv.height = psy_ui_value_make_px(extents.height);
	return rv;
}

#endif

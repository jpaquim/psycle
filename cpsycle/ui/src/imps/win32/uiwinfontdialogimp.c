/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiwinfontdialogimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "../../uiapp.h"
#include <stdlib.h>
#include "../../uiapp.h"
#include <shlobj.h>
#include "uiwincomponentimp.h"
#include <commdlg.h>
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void dev_dispose(psy_ui_win_FontDialogImp*);
static int dev_execute(psy_ui_win_FontDialogImp*);
static const char* dev_path(psy_ui_win_FontDialogImp*);
static void dev_setfontinfo(psy_ui_win_FontDialogImp*,
	psy_ui_FontInfo);
psy_ui_FontInfo dev_fontinfo(psy_ui_win_FontDialogImp*);
static psy_ui_FontInfo psy_ui_fontinfo(LOGFONT);
static LOGFONT logfont(psy_ui_FontInfo);
/* vtable */
static psy_ui_FontDialogImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(psy_ui_win_FontDialogImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose = (psy_ui_fp_fontdialogimp_dev_dispose) dev_dispose;
		imp_vtable.dev_execute = (psy_ui_fp_fontdialogimp_dev_execute) dev_execute;		
		imp_vtable.dev_fontinfo = (psy_ui_fp_fontdialogimp_dev_fontinfo) dev_fontinfo;
		imp_vtable.dev_setfontinfo = (psy_ui_fp_fontdialogimp_dev_setfontinfo)dev_setfontinfo;
		imp_vtable_initialized = 1;
	}
}
/* implementation */
void psy_ui_win_fontdialogimp_init(psy_ui_win_FontDialogImp* self)
{
	psy_ui_fontdialogimp_init(&self->imp);
	imp_vtable_init(self);	
	self->imp.vtable = &imp_vtable;
	psy_ui_fontinfo_init(&self->fontinfo, "arial", 8);
}

void dev_dispose(psy_ui_win_FontDialogImp* self)
{
}

int dev_execute(psy_ui_win_FontDialogImp* self)
{
	int rv;
	LOGFONT lf;
	CHOOSEFONT cf;

	lf = logfont(self->fontinfo);	
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = 0;
	cf.hDC = NULL;
	cf.lpLogFont = &lf;
	cf.iPointSize = 0;
	cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_EFFECTS;
	cf.rgbColors = 0;
	cf.lCustData = 0;
	cf.lpfnHook = NULL;
	cf.lpTemplateName = NULL;
	cf.hInstance = NULL;
	cf.lpszStyle = NULL;
	cf.nFontType = 0;               /* Returned from ChooseFont */
	cf.nSizeMin = 0;
	cf.nSizeMax = 0;
	rv = ChooseFont(&cf);
	if (rv) {
		if (lf.lfHeight < 0) {
			HDC display = GetDC(NULL);

			lf.lfHeight = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(display, LOGPIXELSY));
		}
		self->fontinfo = psy_ui_fontinfo(lf);
	}
	return rv;
}

psy_ui_FontInfo dev_fontinfo(psy_ui_win_FontDialogImp* self)
{
	return self->fontinfo;	
}

void dev_setfontinfo(psy_ui_win_FontDialogImp* self,
	psy_ui_FontInfo fontinfo)
{
	self->fontinfo = fontinfo;
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
	memcpy(rv.lfFaceName, lf.lfFaceName, 32); /* TODO UNICODE */
	return rv;
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
	memcpy(rv.lfFaceName, lf.lfFaceName, 32); /* TODO UNICODE */
	return rv;
}

#endif /* PSYCLE_TK_WIN32 */

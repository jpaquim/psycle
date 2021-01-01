// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwincolordialogimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "uiapp.h"
#include <stdlib.h>
#include "uiapp.h"
#include <shlobj.h>
#include "uiwincomponentimp.h"
#include <commdlg.h>
#include <stdlib.h>
#include "../../detail/portable.h"

extern psy_ui_App app;

// VTable Prototypes
static void dev_dispose(psy_ui_win_ColourDialogImp*);
static int dev_execute(psy_ui_win_ColourDialogImp*);
static const char* dev_path(psy_ui_win_ColourDialogImp*);
static psy_ui_Colour dev_colour(psy_ui_win_ColourDialogImp*);
static void dev_setcolour(psy_ui_win_ColourDialogImp*, psy_ui_Colour);

// VTable init
static psy_ui_ColourDialogImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static psy_ui_ColourDialogImpVTable* imp_vtable_init(psy_ui_win_ColourDialogImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose = (psy_ui_fp_colourdialogimp_dev_dispose)dev_dispose;
		imp_vtable.dev_execute = (psy_ui_fp_colourdialogimp_dev_execute)dev_execute;		
		imp_vtable.dev_colour = (psy_ui_fp_colourdialogimp_dev_colour)dev_colour;
		imp_vtable.dev_setcolour = (psy_ui_fp_colourdialogimp_dev_setcolour)dev_setcolour;
		imp_vtable_initialized = TRUE;
	}
	return &imp_vtable;
}

void psy_ui_win_colourdialogimp_init(psy_ui_win_ColourDialogImp* self)
{
	psy_ui_colourdialogimp_init(&self->imp);
	self->imp.vtable = imp_vtable_init(self);	
	self->colour = psy_ui_colour_make_rgb(0x80, 0x80, 0x80);
}

// win32 implementation method for psy_ui_ColourDialog
void dev_dispose(psy_ui_win_ColourDialogImp* self)
{
}

int dev_execute(psy_ui_win_ColourDialogImp* self)
{
	int rv;
	static CHOOSECOLOR cc;
	static COLORREF    crCustColours[16];
	uint8_t r;
	uint8_t g;
	uint8_t b;

	assert(self);
	
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = NULL;
	cc.hInstance = NULL;
	psy_ui_colour_rgb(&self->colour, &r, &g, &b);
	cc.rgbResult = RGB(r, g, b);
	cc.lpCustColors = crCustColours;
	cc.Flags = CC_RGBINIT | CC_FULLOPEN;
	cc.lCustData = 0;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;

	rv = ChooseColor(&cc);
	if (rv) {
		self->colour = psy_ui_colour_make(cc.rgbResult);
	}
	return rv;
}

psy_ui_Colour dev_colour(psy_ui_win_ColourDialogImp* self)
{
	return self->colour;	
}

void dev_setcolour(psy_ui_win_ColourDialogImp* self, psy_ui_Colour colour)
{
	self->colour = colour;
}

#endif

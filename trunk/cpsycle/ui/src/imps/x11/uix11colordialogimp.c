// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uix11colordialogimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11

#include "uiapp.h"
#include <stdlib.h>
#include "uiapp.h"
#include "uix11componentimp.h"
#include <stdlib.h>
#include "../../detail/portable.h"

/* prototypes */
static void dev_dispose(psy_ui_x11_ColorDialogImp*);
static int dev_execute(psy_ui_x11_ColorDialogImp*);
static const char* dev_path(psy_ui_x11_ColorDialogImp*);
psy_ui_Colour dev_color(psy_ui_x11_ColorDialogImp*);

/* vtable */
static psy_ui_ColourDialogImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(psy_ui_x11_ColorDialogImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose = (psy_ui_fp_colourdialogimp_dev_dispose)
			dev_dispose;
		imp_vtable.dev_execute = (psy_ui_fp_colourdialogimp_dev_execute)
			dev_execute;
		imp_vtable.dev_colour = (psy_ui_fp_colourdialogimp_dev_colour)dev_color;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_x11_colordialogimp_init(psy_ui_x11_ColorDialogImp* self)
{
	psy_ui_colourdialogimp_init(&self->imp);
	imp_vtable_init(self);	
	self->imp.vtable = &imp_vtable;
	self->color = psy_ui_colour_make(0x00000000);
}

void dev_dispose(psy_ui_x11_ColorDialogImp* self)
{
}

int dev_execute(psy_ui_x11_ColorDialogImp* self)
{
	int rv;
	//static CHOOSECOLOR cc;
	//static COLORREF    crCustColors[16];

	//cc.lStructSize = sizeof(CHOOSECOLOR);
	//cc.hwndOwner = NULL;
	//cc.hInstance = NULL;
	//cc.rgbResult = RGB(0x80, 0x80, 0x80);
	//cc.lpCustColors = crCustColors;
	//cc.Flags = CC_RGBINIT | CC_FULLOPEN;
	//cc.lCustData = 0;
	//cc.lpfnHook = NULL;
	//cc.lpTemplateName = NULL;

	//rv = ChooseColor(&cc);
	//if (rv) {
		//self->color = cc.rgbResult;
	//}
	return rv;
}

psy_ui_Colour dev_color(psy_ui_x11_ColorDialogImp* self)
{
	return self->color;	
}

#endif

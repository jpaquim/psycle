// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uixtcolordialogimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT

#include "uiapp.h"
#include <stdlib.h>
#include "uiapp.h"
#include "uixtcomponentimp.h"
#include <stdlib.h>
#include "../../detail/portable.h"

extern psy_ui_App app;

// VTable Prototypes
static void dev_dispose(psy_ui_xt_ColorDialogImp*);
static int dev_execute(psy_ui_xt_ColorDialogImp*);
static const char* dev_path(psy_ui_xt_ColorDialogImp*);
psy_ui_Color dev_color(psy_ui_xt_ColorDialogImp*);

// VTable init
static psy_ui_ColorDialogImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(psy_ui_xt_ColorDialogImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose = (psy_ui_fp_colordialogimp_dev_dispose) dev_dispose;
		imp_vtable.dev_execute = (psy_ui_fp_colordialogimp_dev_execute) dev_execute;		
		imp_vtable.dev_color = (psy_ui_fp_colordialogimp_dev_color) dev_color;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_xt_colordialogimp_init(psy_ui_xt_ColorDialogImp* self)
{
	psy_ui_colordialogimp_init(&self->imp);
	imp_vtable_init(self);	
	self->imp.vtable = &imp_vtable;
	self->color = 0x00000000;
}

// win32 implementation method for psy_ui_ColorDialog
void dev_dispose(psy_ui_xt_ColorDialogImp* self)
{
}

int dev_execute(psy_ui_xt_ColorDialogImp* self)
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

psy_ui_Color dev_color(psy_ui_xt_ColorDialogImp* self)
{
	return self->color;	
}

#endif

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_COLORDIALOG_H
#define psy_ui_COLORDIALOG_H

#include "uicomponent.h"

// ColorDialog
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_ColorDialog
// Implementor			psy_ui_ColorDialogImp
// Concrete Implementor	psy_ui_win_ColorDialogImp
//
// psy_ui_ColorDialog <>-------- psy_ui_ColorDialogImp
//     imp->dev_execute                     ^
//     ...                                  |
//                             psy_ui_win_ColorDialogImp

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_ColorDialog {
	struct psy_ui_ColorDialogImp* imp;	
} psy_ui_ColorDialog;

void psy_ui_colordialog_init(psy_ui_ColorDialog*, psy_ui_Component* parent);
void psy_ui_colordialog_dispose(psy_ui_ColorDialog*);
int psy_ui_colordialog_execute(psy_ui_ColorDialog*);
psy_ui_Color psy_ui_colordialog_color(psy_ui_ColorDialog*);

// psy_ui_ColorImp

typedef void (*psy_ui_fp_colordialogimp_dev_dispose)(struct psy_ui_ColorDialogImp*);
typedef int (*psy_ui_fp_colordialogimp_dev_execute)(struct psy_ui_ColorDialogImp*);
typedef psy_ui_Color (*psy_ui_fp_colordialogimp_dev_color)(struct psy_ui_ColorDialogImp*);

typedef struct psy_ui_ColorDialogImpVTable {
	psy_ui_fp_colordialogimp_dev_dispose dev_dispose;
	psy_ui_fp_colordialogimp_dev_execute dev_execute;	
	psy_ui_fp_colordialogimp_dev_color dev_color;
} psy_ui_ColorDialogImpVTable;

typedef struct psy_ui_ColorDialogImp {
	psy_ui_ColorDialogImpVTable* vtable;
} psy_ui_ColorDialogImp;

void psy_ui_colordialogimp_init(psy_ui_ColorDialogImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COLORDIALOG_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_FONTDIALOG_H
#define psy_ui_FONTDIALOG_H

#include "uicomponent.h"

// FontDialog Device Interface
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_FontDialog
// Implementor			psy_ui_FontDialogImp
// Concrete Implementor	psy_ui_win_FontDialogImp
//
// psy_ui_FontDialog <>-------- psy_ui_FontDialogImp
//     imp->dev_execute                     ^
//     ...                                  |
//                             psy_ui_win_FontDialogImp

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_FontDialogImp;

typedef struct psy_ui_FontDialog {
	struct psy_ui_FontDialogImp* imp;	
} psy_ui_FontDialog;

void psy_ui_fontdialog_init(psy_ui_FontDialog*, psy_ui_Component* parent);
void psy_ui_fontdialog_dispose(psy_ui_FontDialog*);
int psy_ui_fontdialog_execute(psy_ui_FontDialog*);
psy_ui_FontInfo psy_ui_fontdialog_fontinfo(psy_ui_FontDialog*);
void psy_ui_fontdialog_setfontinfo(psy_ui_FontDialog*, psy_ui_FontInfo);

// psy_ui_FontImp

typedef void (*psy_ui_fp_fontdialogimp_dev_dispose)(struct psy_ui_FontDialogImp*);
typedef int (*psy_ui_fp_fontdialogimp_dev_execute)(struct psy_ui_FontDialogImp*);
typedef psy_ui_FontInfo (*psy_ui_fp_fontdialogimp_dev_fontinfo)(struct psy_ui_FontDialogImp*);
typedef void (*psy_ui_fp_fontdialogimp_dev_setfontinfo)(struct psy_ui_FontDialogImp*,
	psy_ui_FontInfo);

typedef struct psy_ui_FontDialogImpVTable {
	psy_ui_fp_fontdialogimp_dev_dispose dev_dispose;
	psy_ui_fp_fontdialogimp_dev_execute dev_execute;	
	psy_ui_fp_fontdialogimp_dev_fontinfo dev_fontinfo;
	psy_ui_fp_fontdialogimp_dev_setfontinfo dev_setfontinfo;
} psy_ui_FontDialogImpVTable;

typedef struct psy_ui_FontDialogImp {
	psy_ui_FontDialogImpVTable* vtable;
} psy_ui_FontDialogImp;

void psy_ui_fontdialogimp_init(psy_ui_FontDialogImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_FONTDIALOG_H */

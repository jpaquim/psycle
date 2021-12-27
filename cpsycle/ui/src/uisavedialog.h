/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_SAVEDIALOG_H
#define psy_ui_SAVEDIALOG_H

/* local */
#include "uicomponent.h"
/* file */
#include <dir.h>

/*
** SaveDialog Device Interface
** Bridge
** Aim: avoid coupling to one platform (win32, xt/motif, etc)
** Abstraction/Refined  psy_ui_SaveDialog
** Implementor			psy_ui_SaveDialogImp
** Concrete Implementor	psy_ui_win_SaveDialogImp
**
** psy_ui_SaveDialog <>-------- psy_ui_SaveDialogImp
**     imp->dev_execute                     ^
**     ...                                  |
**                             psy_ui_win_SaveDialogImp
*/

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_SaveDialogImp;

typedef struct psy_ui_SaveDialog {
	struct psy_ui_SaveDialogImp* imp;
} psy_ui_SaveDialog;

void psy_ui_savedialog_init(psy_ui_SaveDialog*, psy_ui_Component* parent);
void psy_ui_savedialog_init_all(psy_ui_SaveDialog*,
	psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);
void psy_ui_savedialog_dispose(psy_ui_SaveDialog*);

int psy_ui_savedialog_execute(psy_ui_SaveDialog*);
const psy_Path* psy_ui_savedialog_path(const psy_ui_SaveDialog*);

// psy_ui_SaveImp

typedef void (*psy_ui_fp_savedialogimp_dev_dispose)(struct psy_ui_SaveDialogImp*);
typedef int (*psy_ui_fp_savedialogimp_dev_execute)(struct psy_ui_SaveDialogImp*);
typedef const psy_Path* (*psy_ui_fp_savedialogimp_dev_path)(const struct psy_ui_SaveDialogImp*);

typedef struct psy_ui_SaveDialogImpVTable {
	psy_ui_fp_savedialogimp_dev_dispose dev_dispose;
	psy_ui_fp_savedialogimp_dev_execute dev_execute;
	psy_ui_fp_savedialogimp_dev_path dev_path;
} psy_ui_SaveDialogImpVTable;

typedef struct psy_ui_SaveDialogImp {
	psy_ui_SaveDialogImpVTable* vtable;
} psy_ui_SaveDialogImp;

void psy_ui_savedialogimp_init(psy_ui_SaveDialogImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_SAVEDIALOG_H */

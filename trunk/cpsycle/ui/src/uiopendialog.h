// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_OPENDIALOG_H
#define psy_ui_OPENDIALOG_H

// local
#include "uicomponent.h"
// file
#include <dir.h>

// OpenDialog Device Interface
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_OpenDialog
// Implementor			psy_ui_OpenDialogImp
// Concrete Implementor	psy_ui_win_OpenDialogImp
//
// psy_ui_OpenDialog <>-------- psy_ui_OpenDialogImp
//     imp->dev_execute                     ^
//     ...                                  |
//                             psy_ui_win_OpenDialogImp

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_OpenDialogImp;

typedef struct psy_ui_OpenDialog {
	struct psy_ui_OpenDialogImp* imp;	
} psy_ui_OpenDialog;

void psy_ui_opendialog_init(psy_ui_OpenDialog*, psy_ui_Component* parent);
void psy_ui_opendialog_init_all(psy_ui_OpenDialog*,
	psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);
void psy_ui_opendialog_dispose(psy_ui_OpenDialog*);

psy_ui_OpenDialog* psy_ui_opendialog_alloc(void);
psy_ui_OpenDialog* psy_ui_opendialog_allocinit(psy_ui_Component* parent);

int psy_ui_opendialog_execute(psy_ui_OpenDialog*);
const psy_Path* psy_ui_opendialog_path(const psy_ui_OpenDialog*);

// psy_ui_OpenImp

typedef void (*psy_ui_fp_opendialogimp_dev_dispose)(struct psy_ui_OpenDialogImp*);
typedef int (*psy_ui_fp_opendialogimp_dev_execute)(struct psy_ui_OpenDialogImp*);
typedef const psy_Path* (*psy_ui_fp_opendialogimp_dev_path)(const struct psy_ui_OpenDialogImp*);

typedef struct psy_ui_OpenDialogImpVTable {
	psy_ui_fp_opendialogimp_dev_dispose dev_dispose;
	psy_ui_fp_opendialogimp_dev_execute dev_execute;
	psy_ui_fp_opendialogimp_dev_path dev_path;
} psy_ui_OpenDialogImpVTable;

typedef struct psy_ui_OpenDialogImp {
	psy_ui_OpenDialogImpVTable* vtable;
} psy_ui_OpenDialogImp;

void psy_ui_opendialogimp_init(psy_ui_OpenDialogImp*);


#ifdef __cplusplus
}
#endif

#endif

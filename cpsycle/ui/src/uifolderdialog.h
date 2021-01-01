// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_FOLDERDIALOG_H
#define psy_ui_FOLDERDIALOG_H

#include "uicomponent.h"

// FolderDialog Device Interface
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_FolderDialog
// Implementor			psy_ui_FolderDialogImp
// Concrete Implementor	psy_ui_win_FolderDialogImp
//
// psy_ui_FolderDialog <>-------- psy_ui_FolderDialogImp
//     imp->dev_execute                     ^
//     ...                                  |
//                             psy_ui_win_FolderDialogImp

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_FolderDialogImp;

typedef struct psy_ui_FolderDialog {
	struct psy_ui_FolderDialogImp* imp;
} psy_ui_FolderDialog;

void psy_ui_folderdialog_init(psy_ui_FolderDialog*, psy_ui_Component* parent);
void psy_ui_folderdialog_init_all(psy_ui_FolderDialog*,
	psy_ui_Component* parent,
	const char* title,
	const char* initialdir);
void psy_ui_folderdialog_dispose(psy_ui_FolderDialog*);

int psy_ui_folderdialog_execute(psy_ui_FolderDialog*);
const char* psy_ui_folderdialog_path(psy_ui_FolderDialog*);

// psy_ui_FolderImp

typedef void (*psy_ui_fp_folderdialogimp_dev_dispose)(struct psy_ui_FolderDialogImp*);
typedef int (*psy_ui_fp_folderdialogimp_dev_execute)(struct psy_ui_FolderDialogImp*);
typedef const char* (*psy_ui_fp_folderdialogimp_dev_path)(struct psy_ui_FolderDialogImp*);

typedef struct psy_ui_FolderDialogImpVTable {
	psy_ui_fp_folderdialogimp_dev_dispose dev_dispose;
	psy_ui_fp_folderdialogimp_dev_execute dev_execute;
	psy_ui_fp_folderdialogimp_dev_path dev_path;
} psy_ui_FolderDialogImpVTable;

typedef struct psy_ui_FolderDialogImp {
	psy_ui_FolderDialogImpVTable* vtable;
} psy_ui_FolderDialogImp;

void psy_ui_folderdialogimp_init(psy_ui_FolderDialogImp*);

#ifdef __cplusplus
}
#endif

#endif

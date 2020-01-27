// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_SAVEDIALOG_H
#define psy_ui_SAVEDIALOG_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

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
	const char* psy_ui_savedialog_filename(psy_ui_SaveDialog*);

	// psy_ui_SaveImp

	typedef void (*psy_ui_fp_savedialogimp_dev_dispose)(struct psy_ui_SaveDialogImp*);
	typedef int (*psy_ui_fp_savedialogimp_dev_execute)(struct psy_ui_SaveDialogImp*);
	typedef const char* (*psy_ui_fp_savedialogimp_dev_filename)(struct psy_ui_SaveDialogImp*);

	typedef struct psy_ui_SaveDialogImpVTable {
		psy_ui_fp_savedialogimp_dev_dispose dev_dispose;
		psy_ui_fp_savedialogimp_dev_execute dev_execute;
		psy_ui_fp_savedialogimp_dev_filename dev_filename;
	} psy_ui_SaveDialogImpVTable;

	typedef struct psy_ui_SaveDialogImp {
		psy_ui_SaveDialogImpVTable* vtable;
	} psy_ui_SaveDialogImp;

	void psy_ui_savedialogimp_init(psy_ui_SaveDialogImp*);


#ifdef __cplusplus
}
#endif

#endif

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_COLORDIALOG_H
#define psy_ui_COLORDIALOG_H

#include "uicomponent.h"

// ColourDialog
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_ColourDialog
// Implementor			psy_ui_ColourDialogImp
// Concrete Implementor	psy_ui_win_ColourDialogImp
//
// psy_ui_ColourDialog <>-------- psy_ui_ColourDialogImp
//     imp->dev_execute                     ^
//     ...                                  |
//                             psy_ui_win_ColourDialogImp

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_ColourDialogImp;

typedef struct psy_ui_ColourDialog {
	struct psy_ui_ColourDialogImp* imp;	
} psy_ui_ColourDialog;

void psy_ui_colourdialog_init(psy_ui_ColourDialog*, psy_ui_Component* parent);
void psy_ui_colourdialog_dispose(psy_ui_ColourDialog*);
int psy_ui_colourdialog_execute(psy_ui_ColourDialog*);
psy_ui_Colour psy_ui_colourdialog_colour(psy_ui_ColourDialog*);

// psy_ui_ColourImp

typedef void (*psy_ui_fp_colourdialogimp_dev_dispose)(struct psy_ui_ColourDialogImp*);
typedef int (*psy_ui_fp_colourdialogimp_dev_execute)(struct psy_ui_ColourDialogImp*);
typedef psy_ui_Colour (*psy_ui_fp_colourdialogimp_dev_colour)(struct psy_ui_ColourDialogImp*);

typedef struct psy_ui_ColourDialogImpVTable {
	psy_ui_fp_colourdialogimp_dev_dispose dev_dispose;
	psy_ui_fp_colourdialogimp_dev_execute dev_execute;	
	psy_ui_fp_colourdialogimp_dev_colour dev_colour;
} psy_ui_ColourDialogImpVTable;

typedef struct psy_ui_ColourDialogImp {
	psy_ui_ColourDialogImpVTable* vtable;
} psy_ui_ColourDialogImp;

void psy_ui_colourdialogimp_init(psy_ui_ColourDialogImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COLORDIALOG_H */

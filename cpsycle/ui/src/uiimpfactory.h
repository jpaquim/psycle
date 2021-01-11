// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_IMPFACTORY_H
#define psy_ui_IMPFACTORY_H

#include "../../detail/psydef.h"
#include "uidef.h"

// AbstractFactory
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// AbstractFactory     psy_ui_ImpFactory
// ConcreteFactory     psy_ui_win_ImpFactory, psy_ui_curses_ImpFactory
// AbstractProducts    ui implementors
// ConcreteProducts    win implementors, curses implementors
// psy_ui_app_init     creates concrete impfactory defined with PSY_USES_TK in
//                     psyconf.h
//
//                 psy_ui_ImpFactory --------------------<> psy_ui_App
//                        ^   allocinit_componentimp          <>
//                        |   ...                             |
//          ------------------------  psy_ui_Component/Graphics/...
//          |                      |     init: imp = app->impfactory->
//          |                      |             allocinit_componentimp
//		    |                      |
// psy_ui_win_ImpFactory        psy_ui_curses_ImpFactory
//   allocinit_componentimp       allocinit_componentimp
//   ...                          ...

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_ImpFactory;
struct psy_ui_Component;
struct psy_ui_FontInfo;
struct psy_ui_Bitmap;

typedef struct psy_ui_BitmapImp* (*psy_ui_fp_impfactory_allocinit_bitmapimp)(struct psy_ui_ImpFactory*, psy_ui_RealSize);
typedef struct psy_ui_GraphicsImp* (*psy_ui_fp_impfactory_allocinit_graphicsimp)(struct psy_ui_ImpFactory*, uintptr_t* platformdc);
typedef struct psy_ui_GraphicsImp* (*psy_ui_fp_impfactory_allocinit_graphicsimp_bitmap)(struct psy_ui_ImpFactory*, struct psy_ui_Bitmap*);
typedef struct psy_ui_FontImp* (*psy_ui_fp_impfactory_allocinit_fontimp)(struct psy_ui_ImpFactory*, const struct psy_ui_FontInfo*);
typedef struct psy_ui_ComponentImp* (*psy_ui_fp_impfactory_allocinit_componentimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_ComponentImp* (*psy_ui_fp_impfactory_allocinit_frameimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_EditImp* (*psy_ui_fp_impfactory_allocinit_editimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_EditImp* (*psy_ui_fp_impfactory_allocinit_editimp_multiline)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_ListBoxImp* (*psy_ui_fp_impfactory_allocinit_listboximp)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_ListBoxImp* (*psy_ui_fp_impfactory_allocinit_listboximp_multiselect)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_ComboBoxImp* (*psy_ui_fp_impfactory_allocinit_comboboximp)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_CheckBoxImp* (*psy_ui_fp_impfactory_allocinit_checkboximp)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_CheckBoxImp* (*psy_ui_fp_impfactory_allocinit_checkboximp_multiline)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_ColourDialogImp* (*psy_ui_fp_impfactory_allocinit_colourdialogimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component* parent);
typedef struct psy_ui_OpenDialogImp* (*psy_ui_fp_impfactory_allocinit_opendialogimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component* parent);
typedef struct psy_ui_OpenDialogImp* (*psy_ui_fp_impfactory_allocinit_all_opendialogimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);
typedef struct psy_ui_SaveDialogImp* (*psy_ui_fp_impfactory_allocinit_savedialogimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component* parent);
typedef struct psy_ui_SaveDialogImp* (*psy_ui_fp_impfactory_allocinit_all_savedialogimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);
typedef struct psy_ui_FolderDialogImp* (*psy_ui_fp_impfactory_allocinit_folderdialogimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component* parent);
typedef struct psy_ui_FolderDialogImp* (*psy_ui_fp_impfactory_allocinit_all_folderdialogimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,
	const char* initialdir);
typedef struct psy_ui_FontDialogImp* (*psy_ui_fp_impfactory_allocinit_fontdialogimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component* parent);

typedef struct psy_ui_ImpFactoryVTable {
	psy_ui_fp_impfactory_allocinit_bitmapimp allocinit_bitmapimp;
	psy_ui_fp_impfactory_allocinit_graphicsimp allocinit_graphicsimp;
	psy_ui_fp_impfactory_allocinit_graphicsimp_bitmap allocinit_graphicsimp_bitmap;
	psy_ui_fp_impfactory_allocinit_fontimp allocinit_fontimp;
	psy_ui_fp_impfactory_allocinit_componentimp allocinit_componentimp;
	psy_ui_fp_impfactory_allocinit_frameimp allocinit_frameimp;
	psy_ui_fp_impfactory_allocinit_editimp allocinit_editimp;
	psy_ui_fp_impfactory_allocinit_editimp_multiline allocinit_editimp_multiline;
	psy_ui_fp_impfactory_allocinit_listboximp allocinit_listboximp;
	psy_ui_fp_impfactory_allocinit_listboximp_multiselect allocinit_listboximp_multiselect;
	psy_ui_fp_impfactory_allocinit_comboboximp allocinit_comboboximp;
	psy_ui_fp_impfactory_allocinit_checkboximp allocinit_checkboximp;
	psy_ui_fp_impfactory_allocinit_checkboximp_multiline allocinit_checkboximp_multiline;
	psy_ui_fp_impfactory_allocinit_colourdialogimp allocinit_colourdialogimp;
	psy_ui_fp_impfactory_allocinit_opendialogimp allocinit_opendialogimp;
	psy_ui_fp_impfactory_allocinit_all_opendialogimp allocinit_all_opendialogimp;
	psy_ui_fp_impfactory_allocinit_savedialogimp allocinit_savedialogimp;
	psy_ui_fp_impfactory_allocinit_all_savedialogimp allocinit_all_savedialogimp;
	psy_ui_fp_impfactory_allocinit_folderdialogimp allocinit_folderdialogimp;
	psy_ui_fp_impfactory_allocinit_all_folderdialogimp allocinit_all_folderdialogimp;
	psy_ui_fp_impfactory_allocinit_fontdialogimp allocinit_fontdialogimp;
} psy_ui_ImpFactoryVTable;

typedef struct psy_ui_ImpFactory {
	psy_ui_ImpFactoryVTable* vtable;
} psy_ui_ImpFactory;

void psy_ui_impfactory_init(psy_ui_ImpFactory*);

struct psy_ui_FontInfo;

struct psy_ui_BitmapImp* psy_ui_impfactory_allocinit_bitmapimp(psy_ui_ImpFactory*, psy_ui_RealSize);
struct psy_ui_GraphicsImp* psy_ui_impfactory_allocinit_graphicsimp(psy_ui_ImpFactory*, uintptr_t* platformdc);
struct psy_ui_GraphicsImp* psy_ui_impfactory_allocinit_graphicsimp_bitmap(psy_ui_ImpFactory*, struct psy_ui_Bitmap*);
struct psy_ui_FontImp* psy_ui_impfactory_allocinit_fontimp(psy_ui_ImpFactory*, const struct psy_ui_FontInfo*);
struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_componentimp(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_frameimp(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_EditImp* psy_ui_impfactory_allocinit_editimp(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_EditImp* psy_ui_impfactory_allocinit_editimp_multiline(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_ListBoxImp* psy_ui_impfactory_allocinit_listboximp(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_ListBoxImp* psy_ui_impfactory_allocinit_listboximp_multiselect(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_ComboBoxImp* psy_ui_impfactory_allocinit_comboboximp(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_CheckBoxImp* psy_ui_impfactory_allocinit_checkboximp(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_CheckBoxImp* psy_ui_impfactory_allocinit_checkboximp_multiline(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_ColourDialogImp* psy_ui_impfactory_allocinit_colourdialogimp(psy_ui_ImpFactory*, struct psy_ui_Component* parent);
struct psy_ui_OpenDialogImp* psy_ui_impfactory_allocinit_opendialogimp(psy_ui_ImpFactory*, struct psy_ui_Component* parent);
struct psy_ui_OpenDialogImp* psy_ui_impfactory_allocinit_all_opendialogimp(psy_ui_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);
struct psy_ui_SaveDialogImp* psy_ui_impfactory_allocinit_savedialogimp(psy_ui_ImpFactory*, struct psy_ui_Component* parent);
struct psy_ui_SaveDialogImp* psy_ui_impfactory_allocinit_all_savedialogimp(psy_ui_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);
struct psy_ui_FolderDialogImp* psy_ui_impfactory_allocinit_folderdialogimp(psy_ui_ImpFactory*, struct psy_ui_Component* parent);
struct psy_ui_FolderDialogImp* psy_ui_impfactory_allocinit_all_folderdialogimp(psy_ui_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,	
	const char* initialdir);
struct psy_ui_FontDialogImp* psy_ui_impfactory_allocinit_fontdialogimp(psy_ui_ImpFactory*, struct psy_ui_Component* parent);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_IMPFACTORY_H */

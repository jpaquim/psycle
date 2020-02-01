// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_IMPFACTORY_H
#define psy_ui_IMPFACTORY_H

#include "../../detail/psydef.h"

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

typedef struct psy_ui_BitmapImp* (*psy_ui_fp_impfactory_allocinit_bitmapimp)(struct psy_ui_ImpFactory*);
typedef struct psy_ui_GraphicsImp* (*psy_ui_fp_impfactory_allocinit_graphicsimp)(struct psy_ui_ImpFactory*, uintptr_t* platformdc);
typedef struct psy_ui_ComponentImp* (*psy_ui_fp_impfactory_allocinit_componentimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_ComponentImp* (*psy_ui_fp_impfactory_allocinit_frameimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_EditImp* (*psy_ui_fp_impfactory_allocinit_editimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_EditImp* (*psy_ui_fp_impfactory_allocinit_editimp_multiline)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_LabelImp* (*psy_ui_fp_impfactory_allocinit_labelimp)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_ListBoxImp* (*psy_ui_fp_impfactory_allocinit_listboximp)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_ListBoxImp* (*psy_ui_fp_impfactory_allocinit_listboximp_multiselect)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
typedef struct psy_ui_CheckBoxImp* (*psy_ui_fp_impfactory_allocinit_checkboximp)(struct psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);

typedef struct psy_ui_ImpFactoryVTable {
	psy_ui_fp_impfactory_allocinit_bitmapimp allocinit_bitmapimp;
	psy_ui_fp_impfactory_allocinit_graphicsimp allocinit_graphicsimp;
	psy_ui_fp_impfactory_allocinit_componentimp allocinit_componentimp;
	psy_ui_fp_impfactory_allocinit_frameimp allocinit_frameimp;
	psy_ui_fp_impfactory_allocinit_editimp allocinit_editimp;
	psy_ui_fp_impfactory_allocinit_editimp_multiline allocinit_editimp_multiline;
	psy_ui_fp_impfactory_allocinit_labelimp allocinit_labelimp;
	psy_ui_fp_impfactory_allocinit_listboximp allocinit_listboximp;
	psy_ui_fp_impfactory_allocinit_listboximp_multiselect allocinit_listboximp_multiselect;
	psy_ui_fp_impfactory_allocinit_checkboximp allocinit_checkboximp;
} psy_ui_ImpFactoryVTable;

typedef struct psy_ui_ImpFactory {
	psy_ui_ImpFactoryVTable* vtable;
} psy_ui_ImpFactory;

void psy_ui_impfactory_init(psy_ui_ImpFactory*);

struct psy_ui_BitmapImp* psy_ui_impfactory_allocinit_bitmapimp(psy_ui_ImpFactory*);
struct psy_ui_GraphicsImp* psy_ui_impfactory_allocinit_graphicsimp(psy_ui_ImpFactory*, uintptr_t* platformdc);
struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_componentimp(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_frameimp(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_EditImp* psy_ui_impfactory_allocinit_editimp(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_EditImp* psy_ui_impfactory_allocinit_editimp_multiline(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_LabelImp* psy_ui_impfactory_allocinit_labelimp(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_ListBoxImp* psy_ui_impfactory_allocinit_listboximp(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_ListBoxImp* psy_ui_impfactory_allocinit_listboximp_multiselect(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);
struct psy_ui_CheckBoxImp* psy_ui_impfactory_allocinit_checkboximp(psy_ui_ImpFactory*, struct psy_ui_Component*, struct psy_ui_Component* parent);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_IMPFACTORY_H */

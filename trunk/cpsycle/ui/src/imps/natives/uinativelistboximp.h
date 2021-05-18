/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_native_LISTBOXIMP_H
#define psy_ui_native_LISTBOXIMP_H

#include "../../uilistbox.h"
#include "../../uiscroller.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_Component;

typedef struct psy_ui_ListBoxClient {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_selchanged;
	/* internal */
	psy_Table items;
	intptr_t selindex;
	double charnumber;
} psy_ui_ListBoxClient;

void psy_ui_listboxclient_init(psy_ui_ListBoxClient*, psy_ui_Component* parent);
void psy_ui_listboxclient_clear(psy_ui_ListBoxClient*);
intptr_t psy_ui_listboxclient_addtext(psy_ui_ListBoxClient*, const char* text);
void psy_ui_listboxclient_setcursel(psy_ui_ListBoxClient*, intptr_t index);
intptr_t psy_ui_listboxclient_cursel(psy_ui_ListBoxClient*);
void psy_ui_listboxclient_setcharnumber(psy_ui_ListBoxClient*, double num);


typedef struct psy_ui_native_ListBoxImp {
	psy_ui_ComponentImp* imp;
	struct psy_ui_Component* component;	
	psy_ui_ListBoxClient client;
	psy_ui_Scroller scroller;
} psy_ui_native_ListBoxImp;

void psy_ui_native_listboximp_init(psy_ui_native_ListBoxImp*,
	struct psy_ui_Component*,
	struct psy_ui_Component* parent);

void psy_ui_native_listboximp_multiselect_init(psy_ui_native_ListBoxImp*,
	struct psy_ui_Component* component,
	struct psy_ui_Component* parent);

psy_ui_native_ListBoxImp* psy_ui_native_listboximp_alloc(void);
psy_ui_native_ListBoxImp* psy_ui_native_listboximp_allocinit(
	struct psy_ui_Component* component,
	struct psy_ui_Component* parent);
psy_ui_native_ListBoxImp* psy_ui_native_listboximp_multiselect_allocinit(
	struct psy_ui_Component* component,
	psy_ui_Component* parent);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_native_LISTBOXIMP_H */

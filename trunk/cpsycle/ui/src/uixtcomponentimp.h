// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT
#ifndef psy_ui_xt_COMPONENTIMP_H
#define psy_ui_xt_COMPONENTIMP_H

#include "uicomponent.h"

#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <Xm/Xm.h>
#include <Xm/Display.h>

#ifdef __cplusplus
extern "C" {
#endif

// typedef LRESULT(CALLBACK* winproc)(HWND hwnd, UINT message, WPARAM wParam,
//	LPARAM lParam);

typedef struct psy_ui_xt_ComponentImp {
	psy_ui_ComponentImp imp;
	struct psy_ui_Component* component;
	Widget hwnd;
    Widget bulletin;
	uintptr_t winid;
//	HBRUSH background;
//	winproc wndproc;
} psy_ui_xt_ComponentImp;

void psy_ui_xt_componentimp_init(psy_ui_xt_ComponentImp* self,
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand);

psy_ui_xt_ComponentImp* psy_ui_xt_componentimp_alloc(void);
psy_ui_xt_ComponentImp* psy_ui_xt_componentimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_xt_ComponentImp_H */
#endif /* PSYCLE_TK_XT */

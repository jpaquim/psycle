// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11
#ifndef psy_ui_x11_COMPONENTIMP_H
#define psy_ui_x11_COMPONENTIMP_H

#include "uicomponent.h"

#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/extensions/Xdbe.h>

#ifdef __cplusplus
extern "C" {
#endif

// typedef LRESULT(CALLBACK* winproc)(HWND hwnd, UINT message, WPARAM wParam,
//	LPARAM lParam);

struct psy_ui_x11_ComponentImp;

typedef struct psy_ui_x11_ComponentImp {
	psy_ui_ComponentImp imp;
	struct psy_ui_Component* component;
	Window hwnd;
	uintptr_t winid;
	psy_ui_Colour backgroundcolor;
	// winproc wndproc;
	// int preventwmchar;	
	psy_ui_Size sizecache;
	bool sizecachevalid;
	int dbg;
	psy_ui_TextMetric tm;
	bool tmcachevalid;
	bool visible;
	psy_List* viewcomponents;
	int prev_w; // used to detect change with ConfigureNotify that can be
	int prev_h; // triggered for other reasons than resize		
	bool mapped;
	psy_ui_Graphics g;	
	struct psy_ui_x11_ComponentImp* parent;
	XdbeBackBuffer d_backBuf;
	psy_ui_RealRectangle exposearea;
	bool exposeareavalid;
	psy_List* expose_rectangles;
} psy_ui_x11_ComponentImp;

void psy_ui_x11_componentimp_init(psy_ui_x11_ComponentImp* self,
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand);

psy_ui_x11_ComponentImp* psy_ui_x11_componentimp_alloc(void);
psy_ui_x11_ComponentImp* psy_ui_x11_componentimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_x11_ComponentImp_H */
#endif /* PSYCLE_TK_X11 */

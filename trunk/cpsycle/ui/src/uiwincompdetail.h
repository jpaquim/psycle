// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_WINCOMPONENTDETAIL_H
#define psy_ui_WINCOMPONENTDETAIL_H

#include "../../detail/stdint.h"
#include "../../detail/os.h"
#include "../../detail/psydef.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

// target win98 or nt 4 or later systems
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x400

#ifdef __cplusplus
extern "C" {
#endif

typedef LRESULT(CALLBACK *winproc)(HWND hwnd, UINT message, WPARAM wParam,
	LPARAM lParam);

typedef struct {
	HWND hwnd;
	uintptr_t winid;
	HBRUSH background;
	winproc wndproc;	
} psy_ui_win_ComponentDetails;

psy_ui_win_ComponentDetails* psy_ui_win_component_details(struct psy_ui_Component* self);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_WINCOMPONENTDETAIL_H */

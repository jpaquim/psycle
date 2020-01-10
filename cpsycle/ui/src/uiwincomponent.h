// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(UI_WIN32_COMPONENT_H)
#define UI_WIN32_COMPONENT_H

#include "../../detail/stdint.h"
#include <windows.h>
#include <assert.h>

typedef LRESULT(CALLBACK *winproc)(HWND hwnd, UINT message, WPARAM wParam,
	LPARAM lParam);

typedef struct psy_ui_WinComponent {
	HWND hwnd;
	uintptr_t winid;
	HBRUSH background;
	winproc wndproc;
} psy_ui_WinComponent;

static _inline psy_ui_WinComponent* ui_win_component(psy_ui_Component* self)
{
	assert(self->platform);
	return (psy_ui_WinComponent*) self->platform;
}

static _inline HWND ui_win_component_hwnd(psy_ui_Component* self)
{	
	return ui_win_component(self)->hwnd;
}

static _inline LRESULT ui_win_component_sendmessage(psy_ui_Component* self,
	UINT msg, WPARAM wparam, LPARAM lparam)
{	
	return SendMessage(ui_win_component_hwnd(self), msg, wparam, lparam);
}



#endif;
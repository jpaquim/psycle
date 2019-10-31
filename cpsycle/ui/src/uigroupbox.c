// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uigroupbox.h"

void ui_groupbox_init(ui_groupbox* groupbox, ui_component* parent)
{
	ui_win32_component_init(&groupbox->component, parent, TEXT("BUTTON"),
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
		0);
}

void ui_groupbox_settext(ui_groupbox* groupbox, const char* text)
{
	SetWindowText((HWND)groupbox->component.hwnd, text);
}
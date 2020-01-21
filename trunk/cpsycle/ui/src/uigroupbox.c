// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uigroupbox.h"
#include "uiwincompdetail.h"

void psy_ui_groupbox_init(psy_ui_Groupbox* self, psy_ui_Component* parent)
{
	psy_ui_win32_component_init(&self->component, parent, TEXT("BUTTON"),
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
		0);
}

void psy_ui_groupbox_settext(psy_ui_Groupbox* self, const char* text)
{
	SetWindowText((HWND)psy_ui_win_component_details(&self->component)->hwnd, text);
}
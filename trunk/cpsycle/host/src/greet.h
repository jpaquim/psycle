// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(GREET_H)
#define GREET_H

#include <uilabel.h>
#include <uibutton.h>
#include <uilistbox.h>
#include <uigroupbox.h>

typedef struct {
	ui_component component;
	ui_groupbox groupbox;
	ui_label header;
	ui_listbox greetz;
	ui_button original;
	int current;
} Greet;


void greet_init(Greet* greet, ui_component* parent);

#endif

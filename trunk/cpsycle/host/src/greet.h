// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(GREET_H)
#define GREET_H

#include <uilabel.h>
#include <uibutton.h>
#include <uilistbox.h>

#ifdef __cplusplus
extern "C" {
#endif

// aim: In this dialog, Psycledelics, the Community, wants to thank the people
//      for their contributions in the developement of Psycle.
//      Psycle's development began in May 2000. It was designed by Arguru and
//      the dialog can display his original greetings, aswell.

typedef struct {
	psy_ui_Component component;	
	psy_ui_Label header;
	psy_ui_Label thanks;
	psy_ui_Component view;
	psy_ui_ListBox greetz;
	psy_ui_Button original;
	int current;
} Greet;

void greet_init(Greet* greet, psy_ui_Component* parent);

#ifdef __cplusplus
}
#endif

#endif

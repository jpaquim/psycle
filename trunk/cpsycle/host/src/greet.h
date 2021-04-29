// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(GREET_H)
#define GREET_H

// ui
#include <uibutton.h>
#include <uilabel.h>
#include <uilistbox.h>
#include <uiimage.h>

#ifdef __cplusplus
extern "C" {
#endif

// Greet
//
// In this dialog, Psycledelics, the Community, wants to thank the people
// for their contributions in the developement of Psycle.
// Psycle's development began in May 2000. It was designed by Arguru and
// the dialog can display his original greetings, aswell.

typedef struct Greet {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Component header;
	psy_ui_Label headerlabel;
	psy_ui_Label thanks;
	psy_ui_Component view;
	psy_ui_ListBox greetz;
	psy_ui_Button original;
	psy_ui_Image favicon;
	psy_ui_Bitmap bgmain;
	// internal data
	int current;
} Greet;

void greet_init(Greet* greet, psy_ui_Component* parent);

INLINE psy_ui_Component* greet_base(Greet* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* GREET_H */

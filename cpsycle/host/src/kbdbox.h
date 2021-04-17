// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(KBDBOX_H)
#define KBDBOX_H

// host
#include "workspace.h"
// ui
#include "uicomponent.h"
// container
#include <hashtbl.h>
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// KbdBox
//
// Shows a keyboard with annotations

// defines one key of the keybox
typedef struct KbdBoxKey {
	// inherits;
	psy_ui_Component component;
	// intern
	char* label;
	char* desc0; // row 0 keycode
	char* desc1; // row 1 keycode with shift
	char* desc2; // row 2 keycode with ctrl
	char* desc3; // row 2 keycode with shift + ctrl
	bool marked;	
} KbdBoxKey;

void kbdboxkey_init_all(KbdBoxKey*, psy_ui_Component* parent, psy_ui_Component* view,
	int x, int y, int width, int height, const char* label);
KbdBoxKey* kbdboxkey_allocinit_all(psy_ui_Component* parent, psy_ui_Component* view, 
	int x, int y, int width, int height, const char* label);

typedef struct KbdBox{
	// inherits
	psy_ui_Component component;
	// internal data
	psy_Table keys;
	int cpx;
	int cpy;
	int ident;
	int descident;
	int keyheight;
	int keywidth;
	psy_ui_Size corner;
	psy_Property* keyset;
	// references	
	Workspace* workspace;
} KbdBox;

void kbdbox_init(KbdBox*, psy_ui_Component* parent, Workspace*);
void kbdbox_markkey(KbdBox*, uintptr_t keycode);
void kbdbox_unmarkkey(KbdBox*, uintptr_t keycode);
void kbdbox_cleardescriptions(KbdBox*);
void kbdbox_setdescription(KbdBox*, uintptr_t keycode, int shift, int ctrl,
	const char* desc);

INLINE psy_ui_Component* kbdbox_base(KbdBox* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* KBDBOX_H */

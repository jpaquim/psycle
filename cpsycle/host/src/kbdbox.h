// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(KBDBOX_H)
#define KBDBOX_H

#include "workspace.h"

#include "uicomponent.h"

#include <properties.h>
#include <hashtbl.h>

// aim: shows a keyboard with annotations

typedef struct KbdBoxKey {
	char* label;
	char* desc0;
	char* desc1;
	char* desc2;
	psy_ui_Color color;
	psy_ui_Rectangle position;
} KbdBoxKey;

void kbdboxkey_init_all(KbdBoxKey*, int x, int y, int width, int height, const char* label);
void kbdboxkey_dispose(KbdBoxKey*);
KbdBoxKey* kbdboxkey_allocinit_all(int x, int y, int width, int height, const char* label);

typedef struct KbdBox{
	psy_ui_Component component;
	psy_Table keys;
	int cpx;
	int cpy;
	int ident;
	int descident;
	int keyheight;
	int keywidth;
	psy_ui_Size corner;
	psy_Property* keyset;
	Workspace* workspace;
} KbdBox;

void kbdbox_init(KbdBox*, psy_ui_Component* parent, Workspace*);
void kbdbox_setcolor(KbdBox*, uintptr_t keycode, psy_ui_Color color);
void kbdbox_cleardescriptions(KbdBox*);
void kbdbox_setdescription(KbdBox*, uintptr_t keycode, int shift, int ctrl, const char* desc);

INLINE psy_ui_Component* kbdbox_base(KbdBox* self)
{
	return &self->component;
}

#endif /* KBDBOX_H */

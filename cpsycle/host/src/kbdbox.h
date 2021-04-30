// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(KBDBOX_H)
#define KBDBOX_H

// host
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uilabel.h>
// container
#include <hashtbl.h>
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// KbdBox
//
// Shows a keyboard with annotations

typedef struct KbdBoxState {
	uint32_t pressedkey;
	bool shift;
	bool ctrl;
	bool alt;
} KbdBoxState;

void kbdboxstate_init(KbdBoxState*);
void kbdboxstate_clearmodifier(KbdBoxState*);

// defines one key of the keybox
typedef struct KbdBoxKey {
	// inherits;
	psy_ui_Component component;
	// intern
	psy_ui_Label label;
	psy_ui_Label desc0; // row 0 keycode
	psy_ui_Label desc1; // row 1 keycode with shift
	psy_ui_Label desc2; // row 2 keycode with ctrl
	psy_ui_Label desc3; // row 3 keycode with shift + ctrl	
	// int size;
	uint32_t keycode;
	// references
	KbdBoxState* state;
	Workspace* workspace;	
} KbdBoxKey;

void kbdboxkey_init_all(KbdBoxKey*, psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t size, uint32_t keycode, const char* label,
	Workspace*, KbdBoxState* state);

KbdBoxKey* kbdboxkey_allocinit_all(psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t size, uint32_t keycode, const char* label,
	Workspace*, KbdBoxState* state);

void kbdboxkey_cleardescriptions(KbdBoxKey*);
void kbdboxkey_setdescription(KbdBoxKey*, uint32_t keycode, const char* text);

INLINE psy_ui_Component* kbdboxkey_base(KbdBoxKey* self)
{
	return &self->component;
}

// KbdBox
typedef struct KbdBox {
	// inherits
	psy_ui_Component component;
	// intern
	psy_Table keys;		
	psy_ui_Component* currrow;
	KbdBoxState state;
	// references	
	Workspace* workspace;
} KbdBox;

void kbdbox_init(KbdBox*, psy_ui_Component* parent, Workspace*);
void kbdbox_cleardescriptions(KbdBox*);
void kbdbox_setdescription(KbdBox*, uint32_t keycode, const char* text);
void kbdbox_presskey(KbdBox*, uint32_t keycode);
void kbdbox_releasekey(KbdBox*, uint32_t keycode);

INLINE psy_ui_Component* kbdbox_base(KbdBox* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* KBDBOX_H */

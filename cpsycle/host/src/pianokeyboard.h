/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PIANOKEYBOARD_H)
#define PIANOKEYBOARD_H

/* host */
#include "pianogridstate.h"
#include "pianokeyboardstate.h"
#include "workspace.h"
/* ui */
#include <uicomponent.h>
/* audio */
#include <pattern.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	KEYBOARDTYPE_KEYS,
	KEYBOARDTYPE_NOTES,
	KEYBOARDTYPE_DRUMS,
	KEYBOARDTYPE_NUM
} KeyboardType;

/* Keyboard */
typedef struct PianoKeyboard {
	/* inherits */
	psy_ui_Component component;
	/* references */
	KeyboardState* keyboardstate;
	KeyboardState defaultkeyboardstate;
} PianoKeyboard;

void pianokeyboard_init(PianoKeyboard*, psy_ui_Component* parent,
	KeyboardState*);
void pianokeyboard_setsharedkeyboardstate(PianoKeyboard*, KeyboardState*);
void pianokeyboard_setkeyboardtype(PianoKeyboard*, KeyboardType);

INLINE psy_ui_Component* pianokeyboard_base(PianoKeyboard* self)
{
	return &self->component;
}


#ifdef __cplusplus
}
#endif

#endif /* PIANOKEYBOARD_H */

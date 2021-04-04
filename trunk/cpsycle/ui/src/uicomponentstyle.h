// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_COMPONENTSTYLE_H
#define psy_ui_COMPONENTSTYLE_H

#include "uidef.h"
#include "uistyle.h"

// psy_ui_ComponentStyle

#ifdef __cplusplus
extern "C" {
#endif

typedef enum psy_ui_StyleState {
	psy_ui_STYLESTATE_NONE = 0,
	psy_ui_STYLESTATE_HOVER = 1,
	psy_ui_STYLESTATE_SELECT = 2,
	psy_ui_STYLESTATE_DISABLED = 4
} psy_ui_StyleState;

typedef struct psy_ui_ComponentStyle {
	psy_ui_Style* currstyle;
	psy_ui_Style style;
	psy_ui_Style hover;
	psy_ui_Style select;
	psy_ui_Style disabled;
	uintptr_t style_id;
	uintptr_t hover_id;
	uintptr_t select_id;
	uintptr_t disabled_id;
	psy_ui_StyleState state;
} psy_ui_ComponentStyle;

void psy_ui_componentstyle_init(psy_ui_ComponentStyle*);
void psy_ui_componentstyle_dispose(psy_ui_ComponentStyle*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMPONENTSTYLE_H */

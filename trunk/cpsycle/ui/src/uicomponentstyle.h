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
	psy_ui_STYLESTATE_FOCUS = 4,
	psy_ui_STYLESTATE_DISABLED = 8,
	psy_ui_STYLESTATE_ACTIVE = 16,
	psy_ui_STYLESTATE_NUM = 32
} psy_ui_StyleState;

typedef struct psy_ui_ComponentStyle {
	psy_Table styles;
	psy_Table styleids;
	psy_ui_Style* currstyle;
	psy_ui_Style style;
	psy_ui_StyleState state;
	psy_ui_Margin margin;
	psy_ui_Margin spacing;
} psy_ui_ComponentStyle;

void psy_ui_componentstyle_init(psy_ui_ComponentStyle*);
void psy_ui_componentstyle_dispose(psy_ui_ComponentStyle*);

bool psy_ui_componentstyle_hasstyle(const psy_ui_ComponentStyle*,
	psy_ui_StyleState);
psy_ui_Style* psy_ui_componentstyle_style(psy_ui_ComponentStyle*,
	psy_ui_StyleState);
void psy_ui_componentstyle_setstyle(psy_ui_ComponentStyle*,
	psy_ui_StyleState, uintptr_t style_id);
void psy_ui_componentstyle_readstyles(psy_ui_ComponentStyle*);
void psy_ui_componentstyle_readstyle(psy_ui_ComponentStyle*,
	psy_ui_StyleState, uintptr_t style_id);
bool psy_ui_componentstyle_hasstate(const psy_ui_ComponentStyle*,
	psy_ui_StyleState);
bool psy_ui_componentstyle_addstate(psy_ui_ComponentStyle*, psy_ui_StyleState);
bool psy_ui_componentstyle_removestate(psy_ui_ComponentStyle*, psy_ui_StyleState);
bool psy_ui_componentstyle_updatestate(psy_ui_ComponentStyle*);
bool psy_ui_componentstyle_setcurrstate(psy_ui_ComponentStyle*,
	psy_ui_StyleState);

INLINE void psy_ui_componentstyle_setmargin(psy_ui_ComponentStyle* self, psy_ui_Margin margin)
{
	self->margin = margin;
}

INLINE psy_ui_Margin psy_ui_componentstyle_margin(const psy_ui_ComponentStyle* self)
{
	if (self->currstyle->marginset) {
		return self->currstyle->margin;
	} 
	return self->margin;
}

INLINE void psy_ui_componentstyle_setspacing(psy_ui_ComponentStyle* self, psy_ui_Margin spacing)
{
	self->spacing = spacing;
}

INLINE psy_ui_Margin psy_ui_componentstyle_spacing(const psy_ui_ComponentStyle * self)
{
	if (self->currstyle->spacingset) {
		return self->currstyle->spacing;
	}
	return self->spacing;
}

INLINE psy_ui_StyleState psy_ui_componentstyle_state(const psy_ui_ComponentStyle* self)
{
	return self->state;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMPONENTSTYLE_H */

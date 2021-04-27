// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_SPLITBAR_H
#define psy_ui_SPLITBAR_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

// psy_ui_SplitBar
//
// The Splitter divides a component into resizable panes and is itself
// implemented as a component. The aligntype defines if the splitter is
// vertical or horizontal. More then one splitter in the parent component are
// possible.
//
// psy_ui_Component <>----<> psy_ui_ComponentImp
//        ^
//        |                         
//        |
// psy_ui_SplitBar

typedef struct psy_ui_SplitBar {
	// inherits
	psy_ui_Component component;
	// internal
	double dragoffset;	
	bool resize;
	psy_ui_Size restoresize;
	bool hasrestore;
	double thumbsize;
	bool isvertical;
} psy_ui_SplitBar;

void psy_ui_splitbar_init(psy_ui_SplitBar*, psy_ui_Component* parent);

INLINE bool psy_ui_splitbar_isvertical(const psy_ui_SplitBar* self)
{
	return self->isvertical;
}

INLINE psy_ui_Component* psy_ui_splitbar_base(psy_ui_SplitBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_SPLITBAR_H */

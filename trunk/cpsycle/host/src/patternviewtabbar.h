/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNVIEWTABBAR_H)
#define PATTERNVIEWTABBAR_H

/* host */
#include "workspace.h"
/* ui */
#include <uibutton.h>
#include <uitabbar.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** PatternView
**
**  Displays the tracker and/or pianoroll
**
** PatternViewTabBar
*/

typedef struct PatternViewTabBar {
	/* inherits */
	psy_ui_Component component;
	/* signals */	
	psy_Signal signal_toggle_properties;
	/* iinternal */
	psy_ui_TabBar tabbar;
	psy_ui_Button contextbutton;
	/* references */
	Workspace* workspace;
} PatternViewTabBar;

void patternviewtabbar_init(PatternViewTabBar*, psy_ui_Component* parent,
	Workspace*);

INLINE psy_ui_Component* patternviewtabbar_base(PatternViewTabBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PATTERNVIEWTABBAR_H */

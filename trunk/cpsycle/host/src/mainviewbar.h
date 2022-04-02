/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MAINVIEWBAR_H)
#define MAINVIEWBAR_H

/* host */
#include "minmaximize.h"
#include "navigation.h"
#include "workspace.h"
/* ui */
#include <uibutton.h>
#include <uinotebook.h>
#include <uitabbar.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** MainViewBar
*/
typedef struct MainViewBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Button extract;
	psy_ui_Button maximizebtn;
	psy_ui_Component tabbars;
	Navigation navigation;
	psy_ui_TabBar tabbar;	
	MinMaximize minmaximize;
	psy_ui_Notebook viewtabbars;
} MainViewBar;

void mainviewbar_init(MainViewBar*, psy_ui_Component* parent,
	psy_ui_Component* pane, Workspace*);

void mainviewbar_add_minmaximze(MainViewBar*, psy_ui_Component*);
void mainviewbar_toggle_minmaximze(MainViewBar*);

INLINE psy_ui_Component* mainviewbar_base(MainViewBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MAINVIEWBAR_H */

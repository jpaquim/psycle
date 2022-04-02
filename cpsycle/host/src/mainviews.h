/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MAINVIEWS_H)
#define MAINVIEWS_H

/* host */
#include "mainviewbar.h"
#include "workspace.h"
/* ui */
#include <uiframe.h>
#include <uiterminal.h>
#include <uinotebook.h>
#include <uisplitbar.h>
/* container */
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** MainViews
*/

typedef struct MainViews {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	MainViewBar mainviewbar;
	psy_ui_Notebook notebook;
	/* references */
	Workspace* workspace;
} MainViews;

void mainviews_init(MainViews*, psy_ui_Component* parent, psy_ui_Component* pane,
	Workspace* workspace);

INLINE psy_ui_Component* mainviews_base(MainViews* self, psy_ui_Component* parent)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MAINVIEWS_H */

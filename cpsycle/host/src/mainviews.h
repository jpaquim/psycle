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

typedef struct ViewFrame {
	psy_ui_Component component;
	psy_ui_Component pane;
	psy_ui_Component* view;
	psy_ui_Notebook* dock;
	Workspace* workspace;
} ViewFrame;

void viewframe_init(ViewFrame*, psy_ui_Component* parent,
	psy_ui_Notebook* dock, Workspace* workspace);

ViewFrame* viewframe_alloc(void);
ViewFrame* viewframe_allocinit(psy_ui_Component* parent,
	psy_ui_Notebook* dock, Workspace*);

void viewframe_float(ViewFrame*);
void viewframe_dock(ViewFrame*);

INLINE psy_ui_Component* viewframe_base(ViewFrame* self)
{
	return &self->component;
}

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

INLINE psy_ui_Component* mainviews_base(MainViews* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MAINVIEWS_H */

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MAINVIEWS_H)
#define MAINVIEWS_H

/* host */
#include "exportview.h"
#include "helpview.h"
#include "instrumentview.h"
#include "machineview.h"
#include "mainviewbar.h"
#include "patternview.h"
#include "renderview.h"
#include "samplesview.h"
#include "songproperties.h"
#include "startscript.h"
#include "styleview.h"
#include "viewframe.h"
#include "workspace.h"
/* ui */
#include <uiterminal.h>
#include <uinotebook.h>
#include <uisplitbar.h>
/* container */
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif


/* MainViews */

typedef struct MainViews {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	MainViewBar mainviewbar;
	psy_ui_Notebook notebook;
	EmptyViewPage empty_page;
	MachineView machineview;
	PatternView patternview;		
	SamplesView samplesview;
	InstrumentView instrumentsview;
	SongPropertiesView songpropertiesview;
	ExportView exportview;
	RenderView renderview;	
	PropertiesView settingsview;
	StyleView styleview;
	HelpView helpview;
	ConfirmBox confirm;
	FileView fileview;
	Links links;
	/* references */
	Workspace* workspace;
} MainViews;

void mainviews_init(MainViews*, psy_ui_Component* parent,
	psy_ui_Component* pane, Workspace*);

void mainviews_align(MainViews*);
void mainviews_add_link(MainViews*, Link*);

INLINE psy_ui_Component* mainviews_base(MainViews* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MAINVIEWS_H */

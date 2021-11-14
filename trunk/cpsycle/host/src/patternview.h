/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNVIEW_H)
#define PATTERNVIEW_H

/* host */
#include "interpolatecurveview.h"
#include "patternheader.h"
#include "patternproperties.h"
#include "patternviewmenu.h"
#include "patternviewbar.h"
#include "patternviewtabbar.h"
#include "pianoroll.h"
#include "stepbox.h"
#include "trackerlinenumbers.h"
#include "patterndefaultline.h"
#include "transformpatternview.h"
#include "swingfillview.h"
#include "workspace.h"
/* ui */
#include <uibutton.h>
#include <uicheckbox.h>
#include <uilabel.h>
#include <uinotebook.h>
#include <uitabbar.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** PatternView
**
**  Displays the tracker and/or pianoroll
**
** PatternView
**
** Editor/Viewer for a single pattern or the whole sequence.
** Composite of TrackerView and Pianoroll.
**
** TrackerView:      displays patternevents in a sheet
** Pianoroll:        displays patternevents as a roll
*/

typedef struct PatternView {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	PatternViewTabBar tabbar;	
	TrackerLineNumberBar left;
	PatternDefaultLine defaultline;	
	psy_ui_Component headerpane;
	TrackerHeader header;
	psy_ui_Notebook notebook;
	psy_ui_Notebook editnotebook;	
	TrackerView trackerview;
	Pianoroll pianoroll;	
	PatternProperties properties;
	PatternBlockMenu blockmenu;
	TransformPatternView transformpattern;
	InterpolateCurveView interpolatecurveview;
	SwingFillView swingfillview;	
	TrackConfig trackconfig;
	PatternViewState pvstate;
	TrackerState state;	
	PatternCmds cmds;	
	bool aligndisplay;
	int updatealign;
	/* references */
	Workspace* workspace;
} PatternView;

void patternview_init(PatternView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent,	Workspace*);
void patternview_selectdisplay(PatternView*, PatternDisplayMode);

INLINE psy_ui_Component* patternview_base(PatternView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PATTERNVIEW_H */

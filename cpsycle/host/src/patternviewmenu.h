/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNVIEWMENU_H)
#define PATTERNVIEWMENU_H

/* host */
#include "patternviewstate.h"
#include "workspace.h"
/* ui */
#include <uibutton.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** PatternBlockMenu
** Context Menu for PatternView (Trackerview and Pianoroll)
*/

struct SwingFillView;
struct TransformPatternView;
struct InterpolateCurveView;

typedef struct PatternBlockMenu {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Button cut;
	psy_ui_Button copy;
	psy_ui_Button paste;
	psy_ui_Button mixpaste;
	psy_ui_Button del;
	psy_ui_Button transform;
	psy_ui_Button blockswingfill;
	psy_ui_Button trackswingfill;
	psy_ui_Button interpolatelinear;
	psy_ui_Button interpolatecurve;
	psy_ui_Button changegenerator;
	psy_ui_Button changeinstrument;
	psy_ui_Button blocktransposeup;
	psy_ui_Button blocktransposedown;
	psy_ui_Button blocktransposeup12;
	psy_ui_Button blocktransposedown12;
	psy_ui_Button importbtn;
	psy_ui_Button exportbtn;	
	/* references */
	struct SwingFillView* swingfill;
	struct TransformPatternView* transformpattern;
	struct InterpolateCurveView* interpolatecurveview;	
	PatternViewState* pvstate;	
} PatternBlockMenu;

void patternblockmenu_init(PatternBlockMenu*, psy_ui_Component* parent,
	psy_ui_Component* view,
	struct SwingFillView*, struct TransformPatternView*,
	struct InterpolateCurveView*, PatternViewState*);

INLINE psy_ui_Component* patternblockmenu_base(PatternBlockMenu* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PATTERNVIEWMENU_H */

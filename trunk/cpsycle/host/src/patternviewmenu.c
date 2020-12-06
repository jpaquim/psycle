// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternviewmenu.h"
// host
#include "patterncmds.h"
#include "patternview.h"

// PatternBlockMenu
static void patternblockmenu_oncut(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_oncopy(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onpaste(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onmixpaste(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_ondelete(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onreplace(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_oninterpolatelinear(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_oninterpolatecurve(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onchangegenerator(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onchangeinstrument(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onblocktransposeup(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onblocktransposedown(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onblocktransposeup12(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onblocktransposedown12(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onimport(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onexport(PatternBlockMenu*, psy_ui_Component* sender);
// implementation
void patternblockmenu_init(PatternBlockMenu* self, psy_ui_Component* parent,
	PatternView* view, Workspace* workspace)
{
	assert(self);
	assert(view);
	assert(workspace);

	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;
	self->view = view;
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_button_init_text_connect(&self->cut, &self->component, "edit.cut",
		self, patternblockmenu_oncut);
	psy_ui_button_init_text_connect(&self->copy, &self->component, "edit.copy",
		self, patternblockmenu_oncopy);
	psy_ui_button_init_text_connect(&self->paste, &self->component, "edit.paste",
		self, patternblockmenu_onpaste);
	psy_ui_button_init_text_connect(&self->mixpaste, &self->component, "edit.mixpaste",
		self, patternblockmenu_onmixpaste);	
	psy_ui_button_init_text_connect(&self->del, &self->component, "edit.delete",
		self, patternblockmenu_ondelete);
	psy_ui_button_init_text_connect(&self->transform, &self->component,
		"Search and replace", self, patternblockmenu_onreplace);
	psy_ui_button_init_text_connect(&self->interpolatelinear, &self->component,
		"Interpolate (Linear)", self, patternblockmenu_oninterpolatelinear);
	psy_ui_button_init_text_connect(&self->interpolatecurve, &self->component,
		"Interpolate (Curve)", self, patternblockmenu_oninterpolatecurve);
	psy_ui_button_init_text_connect(&self->changegenerator, &self->component,
		"Change Generator", self, patternblockmenu_onchangegenerator);
	psy_ui_button_init_text_connect(&self->changeinstrument, &self->component,
		"Change Instrument", self, patternblockmenu_onchangeinstrument);

	psy_ui_button_init_text_connect(&self->blocktransposeup, &self->component,
		"Transpose +1", self, patternblockmenu_onblocktransposeup);
	psy_ui_button_init_text_connect(&self->blocktransposedown, &self->component,
		"Transpose -1", self, patternblockmenu_onblocktransposedown);
	psy_ui_button_init_text_connect(&self->blocktransposeup12, &self->component,
		"Transpose +12", self, patternblockmenu_onblocktransposeup12);
	psy_ui_button_init_text_connect(&self->blocktransposedown12, &self->component,
		"Transpose -12", self, patternblockmenu_onblocktransposedown12);

	psy_ui_button_init_text_connect(&self->import, &self->component, "Import (psb)",
		self, patternblockmenu_onimport);
	psy_ui_button_init_text_connect(&self->export, &self->component, "Export (psb)",
		self, patternblockmenu_onexport);
	self->target = PATTERNVIEWTARGET_TRACKER;
}

void patternblockmenu_oncut(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		trackergrid_blockcut(&self->view->trackerview.grid);
	} else {

	}
}

void patternblockmenu_oncopy(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		trackergrid_blockcopy(&self->view->trackerview.grid);
	} else {

	}
}

void patternblockmenu_onpaste(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		trackergrid_blockpaste(&self->view->trackerview.grid);
	} else {

	}
}

void patternblockmenu_onmixpaste(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		trackergrid_blockmixpaste(&self->view->trackerview.grid);
	} else {

	}
}

void patternblockmenu_ondelete(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		trackergrid_blockdelete(&self->view->trackerview.grid);
	} else {

	}
}

void patternblockmenu_onreplace(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		patternview_toggletransformpattern(self->view, &self->view->component);
	} else {

	}
}

void patternblockmenu_oninterpolatelinear(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		patternview_oninterpolatelinear(self->view);
	} else {

	}
}

void patternblockmenu_oninterpolatecurve(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		patternview_toggleinterpolatecurve(self->view, &self->view->component);
	} else {

	}
}

void patternblockmenu_onchangegenerator(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		trackergrid_changegenerator(&self->view->trackerview.grid);
	} else {

	}
}

void patternblockmenu_onchangeinstrument(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		trackergrid_changeinstrument(&self->view->trackerview.grid);
	} else {

	}
}

void patternblockmenu_onblocktransposeup(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		trackergrid_blocktransposeup(&self->view->trackerview.grid);
	} else {

	}
}

void patternblockmenu_onblocktransposedown(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		trackergrid_blocktransposedown(&self->view->trackerview.grid);
	} else {

	}
}

void patternblockmenu_onblocktransposeup12(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		trackergrid_blocktransposeup12(&self->view->trackerview.grid);
	} else {

	}
}

void patternblockmenu_onblocktransposedown12(PatternBlockMenu* self, psy_ui_Component* parent)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		trackergrid_blocktransposedown12(&self->view->trackerview.grid);
	} else {

	}
}

void patternblockmenu_onimport(PatternBlockMenu* self, psy_ui_Component* parent)
{
	patternview_onpatternimport(self->view);
}

void patternblockmenu_onexport(PatternBlockMenu* self, psy_ui_Component* parent)
{
	patternview_onpatternexport(self->view);
}

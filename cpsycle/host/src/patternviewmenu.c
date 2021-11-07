// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternviewmenu.h"
// host
#include "patterncmds.h"
#include "patternview.h"

// prototypes
static const char* patternblockmenu_section(PatternBlockMenu*);
static void patternblockmenu_execute(PatternBlockMenu* self, int id);
static void patternblockmenu_oncut(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_oncopy(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onpaste(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onmixpaste(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_ondelete(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onreplace(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onblockswingfill(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_ontrackswingfill(PatternBlockMenu*, psy_ui_Component* sender);
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
	psy_ui_Margin spacing;

	assert(self);
	assert(view);
	assert(workspace);

	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	self->view = view;
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());	
	psy_ui_button_init_text_connect(&self->cut, &self->component, NULL, "edit.cut",
		self, patternblockmenu_oncut);			
	psy_ui_button_init_text_connect(&self->copy, &self->component, NULL, "edit.copy",
		self, patternblockmenu_oncopy);
	psy_ui_button_init_text_connect(&self->paste, &self->component, NULL, "edit.paste",
		self, patternblockmenu_onpaste);
	psy_ui_button_init_text_connect(&self->mixpaste, &self->component, NULL, "edit.mixpaste",
		self, patternblockmenu_onmixpaste);	
	psy_ui_button_init_text_connect(&self->del, &self->component, NULL, "edit.delete",
		self, patternblockmenu_ondelete);
	psy_ui_button_init_text_connect(&self->transform, &self->component, NULL,
		"Search and replace", self, patternblockmenu_onreplace);
	psy_ui_button_init_text_connect(&self->blockswingfill, &self->component, NULL,
		"Block Swing Fill", self, patternblockmenu_onblockswingfill);
	psy_ui_button_init_text_connect(&self->trackswingfill, &self->component, NULL,
		"Track Swing Fill", self, patternblockmenu_ontrackswingfill);
	psy_ui_button_init_text_connect(&self->interpolatelinear, &self->component, NULL,
		"Interpolate (Linear)", self, patternblockmenu_oninterpolatelinear);
	psy_ui_button_init_text_connect(&self->interpolatecurve, &self->component, NULL,
		"Interpolate (Curve)", self, patternblockmenu_oninterpolatecurve);
	psy_ui_button_init_text_connect(&self->changegenerator, &self->component, NULL,
		"Change Generator", self, patternblockmenu_onchangegenerator);
	psy_ui_button_init_text_connect(&self->changeinstrument, &self->component, NULL,
		"Change Instrument", self, patternblockmenu_onchangeinstrument);
	psy_ui_button_init_text_connect(&self->blocktransposeup, &self->component, NULL,
		"Transpose +1", self, patternblockmenu_onblocktransposeup);
	psy_ui_button_init_text_connect(&self->blocktransposedown, &self->component, NULL,
		"Transpose -1", self, patternblockmenu_onblocktransposedown);
	psy_ui_button_init_text_connect(&self->blocktransposeup12, &self->component, NULL,
		"Transpose +12", self, patternblockmenu_onblocktransposeup12);
	psy_ui_button_init_text_connect(&self->blocktransposedown12, &self->component, NULL,
		"Transpose -12", self, patternblockmenu_onblocktransposedown12);
	psy_ui_button_init_text_connect(&self->importbtn, &self->component, NULL, "Import (psb)",
		self, patternblockmenu_onimport);
	psy_ui_button_init_text_connect(&self->exportbtn, &self->component, NULL, "Export (psb)",
		self, patternblockmenu_onexport);	
	spacing = psy_ui_margin_make_em(0.25, 0.25, 0.25, 0.25);
	psy_ui_component_setspacing_children(&self->component, spacing);
	self->target = PATTERNVIEWTARGET_TRACKER;
}

const char* patternblockmenu_section(PatternBlockMenu* self)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		return "tracker";
	}
	return "pianoroll";
}

void patternblockmenu_execute(PatternBlockMenu* self, int id)
{
	psy_audio_player_sendcmd(workspace_player(self->workspace),
		patternblockmenu_section(self),
		psy_eventdrivercmd_makeid(id));
}

void patternblockmenu_oncut(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternblockmenu_execute(self, CMD_BLOCKCUT);
}

void patternblockmenu_oncopy(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternblockmenu_execute(self, CMD_BLOCKCOPY);
}

void patternblockmenu_onpaste(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternblockmenu_execute(self, CMD_BLOCKPASTE);
}

void patternblockmenu_onmixpaste(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternblockmenu_execute(self, CMD_BLOCKMIX);
}

void patternblockmenu_ondelete(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	patternblockmenu_execute(self, CMD_BLOCKDELETE);
}

void patternblockmenu_onreplace(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		patternview_toggletransformpattern(self->view,
			&self->view->component);
	} else {

	}
}

void patternblockmenu_onblockswingfill(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		self->view->trackmodeswingfill = FALSE;
		patternview_toggleswingfill(self->view,
			&self->view->component);
	} else {

	}
}

void patternblockmenu_ontrackswingfill(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		self->view->trackmodeswingfill = TRUE;
		patternview_toggleswingfill(self->view,
			&self->view->component);
	} else {

	}
}

void patternblockmenu_oninterpolatelinear(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		patternview_oninterpolatelinear(self->view);
	} else {

	}
}

void patternblockmenu_oninterpolatecurve(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		patternview_toggleinterpolatecurve(self->view,
			&self->view->component);
	} else {

	}
}

void patternblockmenu_onchangegenerator(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		trackergrid_changegenerator(&self->view->trackerview.grid);
	} else {

	}
}

void patternblockmenu_onchangeinstrument(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	if (self->target == PATTERNVIEWTARGET_TRACKER) {
		trackergrid_changeinstrument(&self->view->trackerview.grid);
	} else {

	}
}

void patternblockmenu_onblocktransposeup(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternblockmenu_execute(self, CMD_TRANSPOSEBLOCKINC);
}

void patternblockmenu_onblocktransposedown(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternblockmenu_execute(self, CMD_TRANSPOSEBLOCKDEC);
}

void patternblockmenu_onblocktransposeup12(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternblockmenu_execute(self, CMD_TRANSPOSEBLOCKINC12);
}

void patternblockmenu_onblocktransposedown12(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternblockmenu_execute(self, CMD_TRANSPOSEBLOCKDEC12);
}

void patternblockmenu_onimport(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	patternview_onpatternimport(self->view);
}

void patternblockmenu_onexport(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	patternview_onpatternexport(self->view);
}

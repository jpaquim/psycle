/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternviewmenu.h"
/* host */
#include "interpolatecurveview.h"
#include "patterncmds.h"
#include "patternview.h"
#include "swingfillview.h"
#include "transformpatternview.h"

/* prototypes */
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
static void patternblockmenu_onblocktransposedec(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onblocktransposeinc(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onblocktransposedec12(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onblocktransposeinc12(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onimport(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_onexport(PatternBlockMenu*, psy_ui_Component* sender);
static void patternblockmenu_toggleswingfill(PatternBlockMenu*);
static void patternblockmenu_toggleinterpolatecurve(PatternBlockMenu*);
static void patternblockmenu_toggletransformpattern(PatternBlockMenu*);
/* implementation */
void patternblockmenu_init(PatternBlockMenu* self, psy_ui_Component* parent,
	psy_ui_Component* view,
	SwingFillView* swingfill, TransformPatternView* transformpattern,
	InterpolateCurveView* interpolatecurveview, PatternViewState* pvstate)
{
	assert(self);	
	assert(swingfill);
	assert(transformpattern);
	assert(interpolatecurveview);
	assert(pvstate);

	psy_ui_component_init(&self->component, parent, view);	
	self->swingfill = swingfill;
	self->transformpattern = transformpattern;
	self->interpolatecurveview = interpolatecurveview;
	self->pvstate = pvstate;
	psy_ui_component_set_defaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());	
	psy_ui_button_init_text_connect(&self->cut, &self->component,
		"edit.cut", self, patternblockmenu_oncut);			
	psy_ui_button_init_text_connect(&self->copy, &self->component,
		"edit.copy", self, patternblockmenu_oncopy);
	psy_ui_button_init_text_connect(&self->paste, &self->component,
		"edit.paste", self, patternblockmenu_onpaste);
	psy_ui_button_init_text_connect(&self->mixpaste, &self->component,
		"edit.mixpaste", self, patternblockmenu_onmixpaste);	
	psy_ui_button_init_text_connect(&self->del, &self->component,
		"edit.delete", self, patternblockmenu_ondelete);
	psy_ui_button_init_text_connect(&self->transform, &self->component,
		"edit.searchreplace", self, patternblockmenu_onreplace);
	psy_ui_button_init_text_connect(&self->blockswingfill, &self->component,
		"edit.blockswingfill", self, patternblockmenu_onblockswingfill);
	psy_ui_button_init_text_connect(&self->trackswingfill, &self->component,
		"edit.trackswingfill", self, patternblockmenu_ontrackswingfill);
	psy_ui_button_init_text_connect(&self->interpolatelinear, &self->component,
		"edit.blockinterpolate", self, patternblockmenu_oninterpolatelinear);
	psy_ui_button_init_text_connect(&self->interpolatecurve, &self->component,
		"edit.blockinterpolatecurve", self, patternblockmenu_oninterpolatecurve);
	psy_ui_button_init_text_connect(&self->changegenerator, &self->component,
		"edit.blocksetmachine", self, patternblockmenu_onchangegenerator);
	psy_ui_button_init_text_connect(&self->changeinstrument, &self->component,
		"edit.blocksetinstr", self, patternblockmenu_onchangeinstrument);
	psy_ui_button_init_text_connect(&self->blocktransposeup, &self->component,
		"edit.transposeblockdec", self, patternblockmenu_onblocktransposedec);
	psy_ui_button_init_text_connect(&self->blocktransposedown, &self->component,
		"edit.transposeblockinc", self, patternblockmenu_onblocktransposeinc);
	psy_ui_button_init_text_connect(&self->blocktransposeup12, &self->component,
		"edit.transposeblockdec12", self, patternblockmenu_onblocktransposedec12);
	psy_ui_button_init_text_connect(&self->blocktransposedown12, &self->component,
		"edit.transposeblockinc12", self, patternblockmenu_onblocktransposeinc12);
	psy_ui_button_init_text_connect(&self->importbtn, &self->component,
		"edit.importpsb", self, patternblockmenu_onimport);
	psy_ui_button_init_text_connect(&self->exportbtn, &self->component,
		"edit.exportpsb", self, patternblockmenu_onexport);		
	psy_ui_component_setpadding_children(&self->component,
		psy_ui_margin_make_em(0.25, 0.25, 0.25, 0.25));
	psy_ui_component_set_align(patternblockmenu_base(self),
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(patternblockmenu_base(self));
}

void patternblockmenu_oncut(PatternBlockMenu* self,
	psy_ui_Component* sender)
{		
	patternviewstate_block_cut(self->pvstate);	
}

void patternblockmenu_oncopy(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternviewstate_block_copy(self->pvstate);	
}

void patternblockmenu_onpaste(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternviewstate_block_paste(self->pvstate);	
}

void patternblockmenu_onmixpaste(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternviewstate_block_mixpaste(self->pvstate);	
}

void patternblockmenu_ondelete(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	patternviewstate_block_delete(self->pvstate);	
}

void patternblockmenu_oninterpolatelinear(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternviewstate_interpolatelinear(self->pvstate);	
}

void patternblockmenu_onchangegenerator(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	patterncmds_change_machine(self->pvstate->cmds,
		self->pvstate->selection);
}

void patternblockmenu_onchangeinstrument(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	patterncmds_change_instrument(self->pvstate->cmds,
		self->pvstate->selection);	
}

void patternblockmenu_onblocktransposedec(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternviewstate_block_transpose(self->pvstate, -1);	
}

void patternblockmenu_onblocktransposeinc(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternviewstate_block_transpose(self->pvstate, 1);
}

void patternblockmenu_onblocktransposedec12(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternviewstate_block_transpose(self->pvstate, -12);
}

void patternblockmenu_onblocktransposeinc12(PatternBlockMenu* self,
	psy_ui_Component* sender)
{	
	patternviewstate_block_transpose(self->pvstate, 12);
}

void patternblockmenu_onimport(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	patterncmds_import_pattern(self->pvstate->cmds,
		patternviewstate_bpl(self->pvstate));
}

void patternblockmenu_onexport(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	patterncmds_export_pattern(self->pvstate->cmds,		
		patternviewstate_bpl(self->pvstate),
		patternviewstate_num_song_tracks(self->pvstate));
}

void patternblockmenu_onreplace(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	patternblockmenu_toggletransformpattern(self);
}

void patternblockmenu_onblockswingfill(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	self->swingfill->trackmodeswingfill = FALSE;
	patternblockmenu_toggleswingfill(self);
}

void patternblockmenu_ontrackswingfill(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	self->swingfill->trackmodeswingfill = TRUE;
	patternblockmenu_toggleswingfill(self);
}

void patternblockmenu_oninterpolatecurve(PatternBlockMenu* self,
	psy_ui_Component* sender)
{
	patternblockmenu_toggleinterpolatecurve(self);
}

void patternblockmenu_toggleswingfill(PatternBlockMenu* self)
{
	assert(self);

	/*if (!psy_ui_component_visible(swingfillview_base(self->swingfill)) &&
			(self->pvstate->song)) {
		swingfillview_reset(self->swingfill,
			(int)self->pvstate->song->properties.bpm);
	}*/
	psy_ui_component_toggle_visibility(swingfillview_base(self->swingfill));	
}

void patternblockmenu_toggleinterpolatecurve(PatternBlockMenu* self)
{
	assert(self);
	
	interpolatecurveview_set_selection(self->interpolatecurveview,
		&self->pvstate->selection);
	psy_ui_component_toggle_visibility(interpolatecurveview_base(
		self->interpolatecurveview));
}

void patternblockmenu_toggletransformpattern(PatternBlockMenu* self)
{
	assert(self);

	psy_ui_component_toggle_visibility(transformpatternview_base(
		self->transformpattern));
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sequenceview.h"
/* host */
#include "styles.h"


/* prototypes */
static void seqview_on_song_changed(SeqView*, Workspace* sender);
static void seqview_on_sequence_select(SeqView*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqview_on_track_reposition(SeqView*,
	psy_audio_Sequence* sender, uintptr_t trackidx);
static void seqview_on_sequence_changed(SeqView*,
	psy_audio_Sequence* sender);
static void seqview_show_pattern_names(SeqView*);
static void seqview_hide_pattern_names(SeqView*);
static void seqview_on_pattern_names(SeqView*, psy_Property*);
static void seqview_on_scroll(SeqView*, psy_ui_Component* sender);
static void seqview_rebuild(SeqView*);
static void seqview_on_edit_seqlist(SeqView*, psy_ui_Button* sender);

/* implementation */
void seqview_init(SeqView* self, psy_ui_Component* parent, Workspace* workspace)
{	
	assert(self);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component, STYLE_SEQVIEW);
	sequencecmds_init(&self->cmds, workspace);
	/* shared state */
	seqviewstate_init(&self->state, &self->cmds);
	/* sequence listview */
	seqviewlist_init(&self->listview, &self->component, &self->state);
	psy_ui_scroller_init(&self->scroller, &self->component, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->listview.component);
	psy_ui_component_set_style_type(&self->scroller.component,
		STYLE_SEQLISTVIEW);
	psy_ui_component_set_padding(psy_ui_scroller_base(&self->scroller),
		psy_ui_margin_make_em(0.5, 0.5, 0.0, 1.0));	
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_align(&self->listview.component,
		psy_ui_ALIGN_FIXED);
	/* button bar */
	sequencebuttons_init(&self->buttons, &self->component, &self->cmds);
	psy_ui_component_set_align(&self->buttons.component, psy_ui_ALIGN_TOP);
	/* spacer */
	psy_ui_component_init_align(&self->spacer, &self->component, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_preferred_size(&self->spacer,
		psy_ui_size_make_em(0.0, 0.3));	
	/* header */
	seqviewtrackheaders_init(&self->trackheader, seqview_base(self),
		&self->state);
	psy_ui_component_set_align(&self->trackheader.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->listview.component.signal_scrolled, self,
		seqview_on_scroll);	
	/* duration*/
	seqviewduration_init(&self->duration, seqview_base(self), workspace);
	psy_ui_component_set_align(&self->duration.component, psy_ui_ALIGN_BOTTOM);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		seqview_on_song_changed);		
	psy_signal_connect(&workspace->song->sequence.sequenceselection.signal_select,
		self, seqview_on_sequence_select);
	if (self->cmds.sequence && self->cmds.sequence->patterns) {
		psy_signal_connect(&self->cmds.sequence->patterns->signal_namechanged,
			&self->listview,
			seqviewlist_on_pattern_name_changed);
		psy_signal_connect(&self->cmds.sequence->signal_changed,
			self, seqview_on_sequence_changed);
		psy_signal_connect(&self->cmds.sequence->signal_trackreposition,
			self, seqview_on_track_reposition);
	}
	seqview_on_pattern_names(self, generalconfig_property(
		psycleconfig_general(workspace_conf(workspace)),
		"showpatternnames"));
	generalconfig_connect(
		psycleconfig_general(workspace_conf(workspace)),
		"showpatternnames", self, seqview_on_pattern_names);	
}

void seqview_on_scroll(SeqView* self, psy_ui_Component* sender)
{	
	assert(self);
	
	psy_ui_component_set_scroll_left(&self->trackheader.client,
		psy_ui_component_scroll_left(seqviewlist_base(&self->listview)));
}

void seqview_clear(SeqView* self)
{
	assert(self);
		
	sequencecmds_clear(&self->cmds);
}

void seqview_idle(SeqView* self)
{
	assert(self);
	
	seqviewduration_idle(&self->duration);
}

void seqview_on_song_changed(SeqView* self, Workspace* sender)
{
	assert(self);
	
	sequencecmds_update(&self->cmds);	
	if (workspace_song(sender)) {		
		if (self->cmds.sequence && self->cmds.patterns) {
			psy_signal_connect(
				&self->cmds.sequence->patterns->signal_namechanged,
				&self->listview, seqviewlist_on_pattern_name_changed);
			psy_signal_connect(&self->cmds.sequence->signal_changed,
				self, seqview_on_sequence_changed);
			psy_signal_connect(&self->cmds.sequence->signal_trackreposition,
				self, seqview_on_track_reposition);
		}
	}
	seqview_rebuild(self);
}

void seqview_rebuild(SeqView* self)
{
	seqviewduration_stopdurationcalc(&self->duration);
	seqviewtrackheaders_build(&self->trackheader);
	seqviewlist_build(&self->listview);			
	seqviewduration_update(&self->duration, TRUE);	
	psy_ui_component_align_full(&self->scroller.component);	
	psy_ui_component_invalidate(&self->scroller.component);	
}

void seqview_on_edit_seqlist(SeqView* self, psy_ui_Button* sender)
{
	psy_ui_component_set_focus(&self->listview.component);
}

void seqview_on_sequence_select(SeqView* self,
	psy_audio_SequenceSelection* sender, psy_audio_OrderIndex* index)
{		
	uintptr_t c = 0;
	double visilines;
	double listviewtop;
	psy_ui_RealSize clientsize;
	
	assert(self);
	
	c = psy_audio_sequenceselection_first(sender).order;
	if (c == psy_INDEX_INVALID) {
		c = 0;
	}
	clientsize = psy_ui_component_clientsize_px(&self->listview.component);
	visilines = (clientsize.height) /
		psy_ui_value_px(&self->state.line_height,
			psy_ui_component_textmetric(&self->component), NULL);
	listviewtop = psy_ui_component_scroll_top_px(&self->listview.component) /
		psy_ui_value_px(&self->state.line_height,
			psy_ui_component_textmetric(&self->component), NULL);
	if ((double)c < listviewtop) {
		psy_ui_component_set_scroll_top(&self->listview.component,
			psy_ui_value_make_px(c *
				psy_ui_value_px(&self->state.line_height,
					psy_ui_component_textmetric(&self->component), NULL)));				
	} else if ((double)c > listviewtop + visilines - 1) {
		psy_ui_component_set_scroll_top(&self->listview.component,
			psy_ui_value_make_px((c - visilines + 1) * 
				psy_ui_value_px(&self->state.line_height,
					psy_ui_component_textmetric(&self->component), NULL)));
	}	
	psy_ui_component_invalidate(&self->listview.component);
	psy_ui_component_invalidate(&self->trackheader.component);
}

void seqview_on_sequence_changed(SeqView* self,
	psy_audio_Sequence* sender)
{			
	assert(self);
	
	seqviewduration_stopdurationcalc(&self->duration);
	seqviewtrackheaders_build(&self->trackheader);
	seqviewlist_build(&self->listview);
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->scroller.component);
	seqviewduration_update(&self->duration, TRUE);
}

void seqview_on_track_reposition(SeqView* self, psy_audio_Sequence* sender,
	uintptr_t trackindex)
{
	seqviewduration_update(&self->duration, FALSE);	
	psy_ui_component_invalidate(&self->listview.component);
}

void seqview_on_pattern_names(SeqView* self, psy_Property* sender)
{
	assert(self);
	
	if (psy_property_item_bool(sender)) {
		seqview_show_pattern_names(self);
	} else {
		seqview_hide_pattern_names(self);
	}
}

void seqview_show_pattern_names(SeqView* self)
{
	assert(self);
		
	self->state.showpatternnames = TRUE;
	self->state.trackwidth = psy_ui_value_make_ew(26.0);
	seqviewtrackheaders_build(&self->trackheader);
	seqviewlist_build(&self->listview);
	psy_ui_component_align(&self->trackheader.component);
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_invalidate(&self->scroller.pane);
}

void seqview_hide_pattern_names(SeqView* self)
{
	assert(self);
		
	self->state.showpatternnames = FALSE;
	self->state.trackwidth = psy_ui_value_make_ew(18.0);
	seqviewtrackheaders_build(&self->trackheader);
	seqviewlist_build(&self->listview);
	psy_ui_component_align(&self->trackheader.component);
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_invalidate(&self->scroller.pane);
}

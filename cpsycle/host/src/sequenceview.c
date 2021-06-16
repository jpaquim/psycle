/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sequenceview.h"
/* host */
#include "styles.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

/* SeqView */
/* prototypes */
static void seqview_onsongchanged(SeqView*, Workspace*, int flag,
	psy_audio_Song*);
static void seqview_onselectionchanged(SeqView*,
	psy_audio_SequenceSelection*);
static void seqview_ontrackreposition(SeqView*,
	psy_audio_Sequence* sender, uintptr_t trackidx);
static void seqview_onsequencechanged(SeqView*,
	psy_audio_Sequence* sender);
static void seqview_onconfigure(SeqView*, GeneralConfig*, psy_Property*);
static void seqview_onscroll(SeqView*, psy_ui_Component* sender);
static void seqview_rebuild(SeqView*);
static void seqview_oneditseqlist(SeqView*, psy_ui_Button* sender);
/* implementation */
void seqview_init(SeqView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	sequencecmds_init(&self->cmds, workspace);
	/* shared state */
	seqviewstate_init(&self->state, &self->cmds);
	/* sequence listview */
	seqviewlist_init(&self->listview, &self->component, &self->state);
	psy_ui_scroller_init(&self->scroller, &self->listview.component,
		&self->component, NULL);	
	psy_ui_component_setspacing(psy_ui_scroller_base(&self->scroller),
		psy_ui_margin_make_em(0.5, 0.5, 0.0, 1.0));
	psy_ui_scroller_setbackgroundmode(&self->scroller,
		psy_ui_SETBACKGROUND, psy_ui_SETBACKGROUND);
	psy_ui_component_setstyletype_select(&self->scroller.component,
		STYLE_SEQLISTVIEW_FOCUS);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->listview.component,
		psy_ui_ALIGN_FIXED);
	/* button bar */
	sequencebuttons_init(&self->buttons, &self->component, &self->cmds);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);
	/* spacer */
	psy_ui_component_init_align(&self->spacer, &self->component, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_setpreferredsize(&self->spacer,
		psy_ui_size_make_em(0.0, 0.3));	
	/* header */
	seqviewtrackheaders_init(&self->trackheader, &self->component,
		&self->state);
	psy_ui_component_setalign(&self->trackheader.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->listview.component.signal_scroll, self,
		seqview_onscroll);	
	/* duration*/
	seqviewduration_init(&self->duration, &self->component, workspace);
	psy_ui_component_setalign(&self->duration.component, psy_ui_ALIGN_BOTTOM);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		seqview_onsongchanged);		
	psy_signal_connect(&workspace->sequenceselection.signal_changed, self,
		seqview_onselectionchanged);
	if (self->cmds.sequence && self->cmds.sequence->patterns) {
		psy_signal_connect(&self->cmds.sequence->patterns->signal_namechanged,
			&self->listview,
			seqviewlist_onpatternnamechanged);
		psy_signal_connect(&self->cmds.sequence->signal_changed,
			self, seqview_onsequencechanged);
		psy_signal_connect(&self->cmds.sequence->signal_trackreposition,
			self, seqview_ontrackreposition);
	}
	psy_signal_connect(
		&psycleconfig_general(workspace_conf(workspace))->signal_changed,
		self, seqview_onconfigure);	
}

void seqview_onscroll(SeqView* self, psy_ui_Component* sender)
{	
	psy_ui_component_setscrollleft(&self->trackheader.client,
		psy_ui_component_scrollleft(seqviewlist_base(&self->listview)));
}

void seqview_clear(SeqView* self)
{
	sequencecmds_clear(&self->cmds);
}

void seqview_onsongchanged(SeqView* self, Workspace* sender,
	int flag, psy_audio_Song* song)
{
	sequencecmds_update(&self->cmds);	
	if (workspace_song(sender)) {		
		if (self->cmds.sequence && self->cmds.patterns) {
			psy_signal_connect(
				&self->cmds.sequence->patterns->signal_namechanged,
				&self->listview, seqviewlist_onpatternnamechanged);
			psy_signal_connect(&self->cmds.sequence->signal_changed,
				self, seqview_onsequencechanged);
			psy_signal_connect(&self->cmds.sequence->signal_trackreposition,
				self, seqview_ontrackreposition);
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

void seqview_oneditseqlist(SeqView* self, psy_ui_Button* sender)
{
	psy_ui_component_setfocus(&self->listview.component);
}

void seqview_onselectionchanged(SeqView* self,
	psy_audio_SequenceSelection* sender)
{		
	uintptr_t c = 0;
	double visilines;
	double listviewtop;
	psy_ui_RealSize clientsize;
	
	c = sender->editposition.order;
	if (c == psy_INDEX_INVALID) {
		c = 0;
	}
	clientsize = psy_ui_component_clientsize_px(&self->listview.component);
	visilines = (clientsize.height) /
		psy_ui_value_px(&self->state.lineheight,
			psy_ui_component_textmetric(&self->component), NULL);
	listviewtop = psy_ui_component_scrolltop_px(&self->listview.component) /
		psy_ui_value_px(&self->state.lineheight,
			psy_ui_component_textmetric(&self->component), NULL);
	if ((double)c < listviewtop) {
		psy_ui_component_setscrolltop(&self->listview.component,
			psy_ui_value_make_px(c *
				psy_ui_value_px(&self->state.lineheight,
					psy_ui_component_textmetric(&self->component), NULL)));				
	} else if ((double)c > listviewtop + visilines - 1) {
		psy_ui_component_setscrolltop(&self->listview.component,
			psy_ui_value_make_px((c - visilines + 1) * 
				psy_ui_value_px(&self->state.lineheight,
					psy_ui_component_textmetric(&self->component), NULL)));
	}	
	psy_ui_component_invalidate(&self->listview.component);
	psy_ui_component_invalidate(&self->trackheader.component);
}

void seqview_onsequencechanged(SeqView* self,
	psy_audio_Sequence* sender)
{			
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_component_scrollsize(&self->component));
	seqviewduration_stopdurationcalc(&self->duration);
	seqviewtrackheaders_build(&self->trackheader);
	seqviewlist_build(&self->listview);
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->listview.component);
	psy_ui_component_align(&self->trackheader.component);
	seqviewduration_update(&self->duration, TRUE);
}

void seqview_ontrackreposition(SeqView* self, psy_audio_Sequence* sender,
	uintptr_t trackindex)
{
	seqviewduration_update(&self->duration, FALSE);	
	psy_ui_component_invalidate(&self->listview.component);
}

void seqview_onconfigure(SeqView* self, GeneralConfig* config,
	psy_Property* property)
{
	if (generalconfig_showingpatternnames(config)) {
		self->state.showpatternnames = TRUE;
		self->state.trackwidth = psy_ui_value_make_ew(26.0);		
	} else {
		self->state.showpatternnames = FALSE;
		self->state.trackwidth = psy_ui_value_make_ew(18.0);		
	}
	seqviewtrackheaders_build(&self->trackheader);
	psy_ui_component_align(&self->trackheader.component);
	psy_ui_component_align(&self->listview.component);
	psy_ui_component_invalidate(&self->listview.component);	
}

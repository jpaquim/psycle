// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequencebuttons.h"
// host
#include "styles.h"

// SequenceButtons
// prototypes
static void sequencebuttons_onmore(SequenceButtons*,
	psy_ui_Button* sender);
static void sequencebuttons_onnewentry(SequenceButtons*,
	psy_ui_Button* sender);
static void sequencebuttons_oninsertentry(SequenceButtons*,
	psy_ui_Button* sender);
static void sequencebuttons_oncloneentry(SequenceButtons*,
	psy_ui_Button* sender);
static void sequencebuttons_ondelentry(SequenceButtons*,
	psy_ui_Button* sender);
static void sequencebuttons_onincpattern(SequenceButtons*,
	psy_ui_Button* sender);
static void sequencebuttons_ondecpattern(SequenceButtons*,
	psy_ui_Button* sender);
static void sequencebuttons_oncopy(SequenceButtons*,
	psy_ui_Button* sender);
static void sequencebuttons_onpaste(SequenceButtons*,
	psy_ui_Button* sender);
static void sequencebuttons_onsingleselection(SequenceButtons*,
	psy_ui_Button* sender);
static void sequencebuttons_onmultiselection(SequenceButtons*,
	psy_ui_Button* sender);
static void sequencebuttons_onclear(SequenceButtons*,
	psy_ui_Button* sender);
static void sequencebuttons_onrename(SequenceButtons*,
	psy_ui_Button* sender);
static void sequencebuttons_oneditaccept(SequenceButtons*, psy_ui_Edit* sender);
static void sequencebuttons_oneditreject(SequenceButtons*, psy_ui_Edit* sender);
// implementation
void sequencebuttons_init(SequenceButtons* self, psy_ui_Component* parent,
	SequenceCmds* cmds)
{	
	psy_ui_Component* view;
	psy_ui_Margin spacing;

	psy_ui_Margin rowmargin;
	psy_ui_Button* buttons[] = {
		&self->incpattern, &self->insertentry, &self->decpattern,
		&self->newentry, &self->delentry, &self->cloneentry,
		&self->more,
		&self->clear, &self->rename, &self->copy,
		&self->paste, &self->singlesel, &self->multisel
	};
	uintptr_t i;	
	self->cmds = cmds;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setstyletype(&self->component, STYLE_SEQVIEW_BUTTONS);
	view = &self->component;	
	psy_ui_component_init(&self->standard, &self->component, view);
	psy_ui_component_setalign(&self->standard, psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->row0, &self->standard, view);
	psy_ui_component_setalign(&self->row0, psy_ui_ALIGN_TOP);
	psy_ui_component_setdefaultalign(&self->row0, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_margin_init_em(&rowmargin, 0.0, 0.0, 0.5, 0.0);
	psy_ui_component_setmargin(&self->row0, rowmargin);
	psy_ui_button_init(&self->incpattern, &self->row0, view);
	psy_ui_button_preventtranslation(&self->incpattern);
	psy_ui_button_settext(&self->incpattern, "+");
	psy_ui_button_init_text(&self->insertentry, &self->row0, view,
		"seqview.ins");	
	psy_ui_button_init(&self->decpattern, &self->row0, view);
	psy_ui_button_preventtranslation(&self->decpattern);
	psy_ui_button_settext(&self->decpattern, "-");
	psy_ui_component_init(&self->row1, &self->standard, view);
	psy_ui_component_setalign(&self->row1, psy_ui_ALIGN_TOP);
	psy_ui_component_setdefaultalign(&self->row1, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init_text(&self->newentry, &self->row1, view,
		"seqview.new");
	psy_ui_button_init_text(&self->cloneentry, &self->row1, view,
		"seqview.clone");
	psy_ui_button_init_text(&self->delentry, &self->row1, view,
		"seqview.del");
	psy_ui_component_init(&self->expand, &self->component, view);
	psy_ui_component_setalign(&self->expand, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->more, &self->expand, view);
	psy_ui_button_preventtranslation(&self->more);
	psy_ui_button_settext(&self->more, ". . .");
	psy_ui_component_setalign(psy_ui_button_base(&self->more),
		psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->block, &self->component, view);
	psy_ui_component_setalign(&self->block, psy_ui_ALIGN_TOP);
	psy_ui_component_setspacing(&self->block,
		psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));
	psy_ui_component_init(&self->row2, &self->block, view);	
	psy_ui_margin_init_em(&rowmargin, 0.5, 0.0, 0.5, 0.0);
	psy_ui_component_setmargin(&self->row2, rowmargin);
	psy_ui_component_setalign(&self->row2, psy_ui_ALIGN_TOP);
	psy_ui_component_setdefaultalign(&self->row2, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_button_init_text(&self->clear, &self->row2, view,
		"seqview.clear");	
	psy_ui_button_init_text(&self->rename, &self->row2, view,
		"seqview.rename");
	// psy_ui_button_init(&self->cut, &self->component);
	// psy_ui_button_settext(&self->cut, "");
	psy_ui_button_init_text(&self->copy, &self->row2, view,
		"seqview.copy");
	// rename edit
	psy_ui_edit_init(&self->edit, view);// &self->block);
	psy_ui_margin_init_em(&rowmargin, 0.5, 0.0, 1.0, 0.0);
	psy_ui_component_setmargin(psy_ui_edit_base(&self->edit), rowmargin);
	psy_ui_component_setalign(psy_ui_edit_base(&self->edit),
		psy_ui_ALIGN_TOP);
	psy_ui_component_hide(psy_ui_edit_base(&self->edit));
	// row3
	psy_ui_component_init(&self->row3, &self->block, view);
	psy_ui_component_setalign(&self->row3, psy_ui_ALIGN_TOP);
	psy_ui_component_setdefaultalign(&self->row3, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init_text(&self->paste, &self->row3, view,
		"seqview.paste");
	psy_ui_button_init_text(&self->singlesel, &self->row3, view,
		"seqview.singlesel");
	psy_ui_button_init_text(&self->multisel, &self->row3, view,
		"seqview.multisel");
	psy_ui_button_highlight(&self->singlesel);
	psy_ui_button_disablehighlight(&self->multisel);
	psy_ui_component_hide(&self->block);

	for (i = 0; i < sizeof(buttons) / sizeof(psy_ui_Button*); ++i) {
		double colwidth;		

		colwidth = 12.0;
		psy_ui_button_setcharnumber(buttons[i], colwidth);
		if (buttons[i] != &self->more) {
			psy_ui_component_setstyletypes(psy_ui_button_base(buttons[i]),
				STYLE_SEQVIEW_BUTTON, STYLE_SEQVIEW_BUTTON_HOVER,
				STYLE_SEQVIEW_BUTTON_SELECT, psy_INDEX_INVALID);
				psy_ui_margin_init_em(&spacing, 0.5, 0.5, 0.5, 0.5);
				psy_ui_button_setlinespacing(buttons[i], 1.4);
		} else {
			psy_ui_margin_init_em(&spacing, 0.25, 0.25, 0.25, 0.25);
			psy_ui_component_setspacing(psy_ui_button_base(buttons[i]),
				spacing);
			psy_ui_component_setstyletypes(psy_ui_button_base(buttons[i]),
				psy_ui_STYLE_BUTTON, psy_INDEX_INVALID, psy_INDEX_INVALID,
				psy_INDEX_INVALID);
		}
	}
	psy_signal_connect(&self->newentry.signal_clicked, self,
		sequencebuttons_onnewentry);
	psy_signal_connect(&self->insertentry.signal_clicked, self,
		sequencebuttons_oninsertentry);
	psy_signal_connect(&self->cloneentry.signal_clicked, self,
		sequencebuttons_oncloneentry);
	psy_signal_connect(&self->delentry.signal_clicked, self,
		sequencebuttons_ondelentry);
	psy_signal_connect(&self->incpattern.signal_clicked, self,
		sequencebuttons_onincpattern);
	psy_signal_connect(&self->decpattern.signal_clicked, self,
		sequencebuttons_ondecpattern);
	psy_signal_connect(&self->copy.signal_clicked, self,
		sequencebuttons_oncopy);
	psy_signal_connect(&self->paste.signal_clicked, self,
		sequencebuttons_onpaste);
	psy_signal_connect(&self->singlesel.signal_clicked, self,
		sequencebuttons_onsingleselection);
	psy_signal_connect(&self->multisel.signal_clicked, self,
		sequencebuttons_onmultiselection);
	psy_signal_connect(&self->clear.signal_clicked, self,
		sequencebuttons_onclear);
	psy_signal_connect(&self->more.signal_clicked, self,
		sequencebuttons_onmore);
	psy_signal_connect(&self->edit.signal_accept, self,
		sequencebuttons_oneditaccept);
	psy_signal_connect(&self->edit.signal_reject, self,
		sequencebuttons_oneditreject);
	psy_signal_connect(&self->rename.signal_clicked, self,
		sequencebuttons_onrename);
}

void sequencebuttons_onmore(SequenceButtons* self,
	psy_ui_Button* sender)
{
	if (psy_ui_component_visible(&self->block)) {
		psy_ui_component_hide(&self->block);
		psy_ui_button_seticon(&self->more, psy_ui_ICON_NONE);
		psy_ui_button_settext(&self->more, ". . .");		
	} else {
		psy_ui_button_settext(&self->more, "-");
		psy_ui_component_show(&self->block);		
	}
	psy_ui_component_align_full(
		psy_ui_component_preferredsize_parent(&self->component));
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
}

void sequencebuttons_onnewentry(SequenceButtons* self, psy_ui_Button* sender)
{
	sequencecmds_newentry(self->cmds, psy_audio_SEQUENCEENTRY_PATTERN);
}

void sequencebuttons_oninsertentry(SequenceButtons* self,
	psy_ui_Button* sender)
{
	sequencecmds_insertentry(self->cmds, psy_audio_SEQUENCEENTRY_PATTERN);
}

void sequencebuttons_oncloneentry(SequenceButtons* self, psy_ui_Button* sender)
{
	sequencecmds_cloneentry(self->cmds);
}

void sequencebuttons_ondelentry(SequenceButtons* self, psy_ui_Button* sender)
{
	sequencecmds_delentry(self->cmds);
}

void sequencebuttons_onincpattern(SequenceButtons* self, psy_ui_Button* sender)
{
	sequencecmds_incpattern(self->cmds);
}

void sequencebuttons_ondecpattern(SequenceButtons* self, psy_ui_Button* sender)
{
	sequencecmds_decpattern(self->cmds);
}

void sequencebuttons_oncopy(SequenceButtons* self, psy_ui_Button* sender)
{	
	sequencecmds_copy(self->cmds);
}

void sequencebuttons_onpaste(SequenceButtons* self, psy_ui_Button* sender)
{
	sequencecmds_paste(self->cmds);
}

void sequencebuttons_onsingleselection(SequenceButtons* self,
	psy_ui_Button* sender)
{
	psy_ui_button_highlight(&self->singlesel);
	psy_ui_button_disablehighlight(&self->multisel);	
	sequencecmds_singleselection(self->cmds);
}

void sequencebuttons_onmultiselection(SequenceButtons* self,
	psy_ui_Button* sender)
{
	psy_ui_button_highlight(&self->multisel);
	psy_ui_button_disablehighlight(&self->singlesel);
	sequencecmds_multiselection(self->cmds);
}

void sequencebuttons_onclear(SequenceButtons* self, psy_ui_Button* sender)
{
	if (workspace_song(self->cmds->workspace)) {
		workspace_selectview(self->cmds->workspace, VIEW_ID_CHECKUNSAVED, 0,
			CONFIRM_SEQUENCECLEAR);
	}
}

void sequencebuttons_onrename(SequenceButtons* self, psy_ui_Button* sender)
{
	if (!psy_ui_component_visible(psy_ui_edit_base(&self->edit))) {
		psy_audio_Pattern* pattern;

		psy_ui_edit_enableinputfield(&self->edit);
		
		pattern = psy_audio_sequence_pattern(self->cmds->sequence,
			self->cmds->workspace->sequenceselection.editposition);
		if (pattern) {			
			psy_ui_edit_settext(&self->edit, psy_audio_pattern_name(pattern));
			psy_ui_edit_setsel(&self->edit, 0, -1);
			psy_ui_component_show(psy_ui_edit_base(&self->edit));
			psy_ui_component_align(psy_ui_component_parent(&self->component));
			psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
			psy_ui_component_setfocus(psy_ui_edit_base(&self->edit));
		} else {
			workspace_outputstatus(self->cmds->workspace,
				"No SequenceEntry selected");
		}
	}
}

void sequencebuttons_oneditaccept(SequenceButtons* self, psy_ui_Edit* sender)
{
	if (self->cmds->sequence) {
		psy_audio_SequenceEntry* entry;

		entry = (self->cmds->sequence)
			? psy_audio_sequence_entry(self->cmds->sequence,
				self->cmds->workspace->sequenceselection.editposition)
			: NULL;
		if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
			psy_audio_Pattern* pattern;
			psy_audio_SequencePatternEntry* seqpatternentry;

			seqpatternentry = (psy_audio_SequencePatternEntry*)entry;
			pattern = psy_audio_patterns_at(
				self->cmds->patterns,
				seqpatternentry->patternslot);
			if (pattern) {
				psy_audio_pattern_setname(pattern, psy_ui_edit_text(
					&self->edit));
			}
		}
	}
	psy_ui_component_hide(&self->edit.component);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
}

void sequencebuttons_oneditreject(SequenceButtons* self, psy_ui_Edit* sender)
{
	psy_ui_component_hide(&self->edit.component);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
}

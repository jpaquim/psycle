/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sequencerbar.h"
/* host */
#include "styles.h"


/* prototypes */
static void sequencerbar_connect_buttons(SequencerBar*, GeneralConfig*);
static void sequencerbar_on_piano_kbd(SequencerBar*, psy_Property* sender);
static void sequencerbar_on_seq_editor(SequencerBar*, psy_Property* sender);
static void sequencerbar_on_step_sequencer(SequencerBar*, psy_Property* sender);

/* implementation */
void sequencerbar_init(SequencerBar* self, psy_ui_Component* parent,
	KeyboardMiscConfig* misc, GeneralConfig* general)
{
	assert(self);
	assert(misc);
	assert(general);
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_padding(&self->component, 
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));	
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_margin_make_em(0.0, 0.0, 0.25, 0.0));		
	psy_ui_checkbox_init_exchange(&self->follow_song, &self->component,				
		keyboardmiscconfig_property(misc, "followsong"));
	psy_ui_checkbox_init_exchange(&self->shownames, &self->component,			
		generalconfig_property(general, "showpatternnames"));
	psy_ui_checkbox_init_exchange(&self->recordnoteoff, &self->component,		
		keyboardmiscconfig_property(misc, "recordnoteoff"));
	psy_ui_checkbox_init_exchange(&self->recordtweak, &self->component,		
		keyboardmiscconfig_property(misc, "record-tweak"));
	psy_ui_checkbox_init_exchange(&self->multichannelaudition, &self->component,			
		keyboardmiscconfig_property(misc, "multikey"));
	psy_ui_checkbox_init_exchange(&self->allownotestoeffect, &self->component,			
		keyboardmiscconfig_property(misc, "notestoeffects"));		
	/* view buttons */
	psy_ui_component_init(&self->view_buttons, &self->component, NULL);
	psy_ui_component_set_align(&self->view_buttons, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_margin(&self->view_buttons,
		psy_ui_margin_make_em(1.0, 0.0, 0.0, 0.0));
	psy_ui_component_set_default_align(&self->view_buttons, psy_ui_ALIGN_TOP,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.0));
	/* seqedit */
	psy_ui_button_init(&self->toggleseqedit, &self->view_buttons);
	psy_ui_button_set_text(&self->toggleseqedit, "seqview.showseqeditor");
	psy_ui_button_data_exchange(&self->toggleseqedit, generalconfig_property(
		general, "bench.showsequenceedit"));	
	psy_ui_button_set_text_alignment(&self->toggleseqedit, (psy_ui_Alignment)
		(psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL));
	psy_ui_button_set_icon(&self->toggleseqedit, psy_ui_ICON_MORE);				
	/* stepseq */
	psy_ui_button_init(&self->togglestepseq, &self->view_buttons);
	psy_ui_button_set_text(&self->togglestepseq, "seqview.showstepsequencer");
	psy_ui_button_data_exchange(&self->togglestepseq, generalconfig_property(
		general, "bench.showstepsequencer"));	
	psy_ui_button_set_icon(&self->togglestepseq, psy_ui_ICON_MORE);
	psy_ui_button_set_text_alignment(&self->togglestepseq,
		(psy_ui_Alignment)(psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL));		
	/* kbd */
	psy_ui_button_init(&self->togglekbd, &self->view_buttons);
	psy_ui_button_set_text(&self->togglekbd, "seqview.showpianokbd");
	psy_ui_button_data_exchange(&self->togglekbd, generalconfig_property(
		general, "bench.showpianokbd"));	
	psy_ui_button_set_icon(&self->togglekbd, psy_ui_ICON_MORE);
	psy_ui_button_set_text_alignment(&self->togglekbd,
		(psy_ui_Alignment)(psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL));
	/* connect */
	sequencerbar_connect_buttons(self, general);
}

void sequencerbar_connect_buttons(SequencerBar* self, GeneralConfig* config)
{	
	generalconfig_connect(config, "bench.showstepsequencer", self,
		sequencerbar_on_step_sequencer);
	generalconfig_connect(config, "bench.showsequenceedit", self,
		sequencerbar_on_seq_editor);	
	generalconfig_connect(config, "bench.showpianokbd", self,
		sequencerbar_on_piano_kbd);			
}

void sequencerbar_on_piano_kbd(SequencerBar* self, psy_Property* sender)
{	
	if (psy_property_item_bool(sender)) {
		psy_ui_button_highlight(&self->togglekbd);
		psy_ui_button_set_icon(&self->togglekbd, psy_ui_ICON_LESS);
	} else {		
		psy_ui_button_disable_highlight(&self->togglekbd);
		psy_ui_button_set_icon(&self->togglekbd, psy_ui_ICON_MORE);
	}
}

void sequencerbar_on_seq_editor(SequencerBar* self, psy_Property* sender)
{
	if (psy_property_item_bool(sender)) {		
		psy_ui_button_set_icon(&self->toggleseqedit, psy_ui_ICON_LESS);
	} else {				
		psy_ui_button_set_icon(&self->toggleseqedit, psy_ui_ICON_MORE);
	}
}

void sequencerbar_on_step_sequencer(SequencerBar* self, psy_Property* sender)
{
	if (psy_property_item_bool(sender)) {		
		psy_ui_button_set_icon(&self->togglestepseq, psy_ui_ICON_LESS);
	} else {		
		psy_ui_button_set_icon(&self->togglestepseq, psy_ui_ICON_MORE);
	}
}

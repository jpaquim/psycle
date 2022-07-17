/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sequencerbar.h"
/* host */
#include "styles.h"


/* implementation */
void sequencerbar_init(SequencerBar* self, psy_ui_Component* parent,
	KeyboardMiscConfig* misc, GeneralConfig* general)
{
	assert(self);
	assert(misc);
	assert(general);
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component, STYLE_SEQBAR);	
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
	psy_ui_button_init(&self->togglestepseq, &self->component);
	psy_ui_button_data_exchange(&self->togglestepseq, generalconfig_property(
		general, "showstepsequencer"));
	psy_ui_button_set_text(&self->togglestepseq, "seqview.showstepsequencer");
	psy_ui_button_set_icon(&self->togglestepseq, psy_ui_ICON_MORE);
	psy_ui_button_set_text_alignment(&self->togglestepseq,
		(psy_ui_Alignment)(psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL));
	psy_ui_component_set_align(&self->togglestepseq.component,
		psy_ui_ALIGN_BOTTOM);	
	psy_ui_button_init(&self->toggleseqedit, &self->component);
	psy_ui_button_data_exchange(&self->toggleseqedit, generalconfig_property(
		general, "showsequenceedit"));
	psy_ui_button_set_text(&self->toggleseqedit, "seqview.showseqeditor");
	psy_ui_component_set_align(&self->toggleseqedit.component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_button_set_text_alignment(&self->toggleseqedit, (psy_ui_Alignment)
		(psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL));
	psy_ui_button_set_icon(&self->toggleseqedit, psy_ui_ICON_MORE);	
	psy_ui_component_set_margin(&self->allownotestoeffect.component,
		psy_ui_margin_make_em(0.0, 0.0, 1.0, 0.0));
}

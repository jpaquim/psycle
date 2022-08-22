/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "plugineditorbar.h"
/* local */
#include "plugineditor.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void plugineditorbar_on_zoom(PluginEditorBar*, ZoomBox* sender);
static void plugineditorbar_update_line(PluginEditorBar*);

/* implementation */
void plugineditorbar_init(PluginEditorBar* self, psy_ui_Component* parent)
{		
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	self->editor = NULL;
	psy_ui_component_set_default_align(plugineditorbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	/* zoom */
	zoombox_init(&self->zoombox, plugineditorbar_base(self));
	psy_ui_component_set_preferred_size(&self->zoombox.component,
		psy_ui_size_make_em(16.0, 1.0));
	psy_signal_connect(&self->zoombox.signal_changed, self,
		plugineditorbar_on_zoom);	
	/* line */
	psy_ui_label_init(&self->line_desc, &self->component);	
	psy_ui_label_prevent_translation(&self->line_desc);
	psy_ui_label_set_text(&self->line_desc, "Ln");
	psy_ui_label_init(&self->line, &self->component);
	psy_ui_label_set_char_number(&self->line, 10.0);
	psy_ui_label_prevent_translation(&self->line);
	/* column */
	psy_ui_label_init(&self->column_desc, &self->component);
	psy_ui_label_prevent_translation(&self->column_desc);
	psy_ui_label_set_text(&self->column_desc, "Col");
	psy_ui_label_init(&self->column, &self->component);
	psy_ui_label_set_char_number(&self->column, 6.0);
	psy_ui_label_prevent_translation(&self->column);
}

void plugineditorbar_set_editor(PluginEditorBar* self, PluginEditor* editor)
{
	assert(self);
	
	self->editor = editor;
}

void plugineditorbar_on_zoom(PluginEditorBar* self, ZoomBox* sender)
{
	assert(self);
	
	if (self->editor) {
		self->editor->zoom = zoombox_rate(&self->zoombox);
		plugineditor_update_font(self->editor);
	}
}

void plugineditorbar_idle(PluginEditorBar* self)
{	
	assert(self);
	
	plugineditorbar_update_line(self);
}

void plugineditorbar_update_line(PluginEditorBar* self)
{
	assert(self);
	
	if (self->editor && psy_ui_component_draw_visible(&self->component)) {
		char text[64];

		psy_snprintf(text, 64, "%d / %d",
			(int)psy_ui_textarea_cursor_line(&self->editor->editor.textarea),
			(int)psy_ui_textarea_num_lines(&self->editor->editor.textarea));
		psy_ui_label_set_text(&self->line, text);
		psy_snprintf(text, 64, "%d", (int)psy_ui_textarea_cursor_column(
			&self->editor->editor.textarea));
		psy_ui_label_set_text(&self->column, text);
	}
}

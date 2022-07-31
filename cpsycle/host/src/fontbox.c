/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "fontbox.h"
/* host */
#include "styles.h"
/* ui */
#include <uiapp.h>
#include <uifontdialog.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void fontbox_on_destroyed(FontBox*);
static void fontbox_on_property_changed(FontBox*,
	psy_Property* sender);
static void fontbox_before_property_destroyed(FontBox*, psy_Property* sender);
static void fontbox_on_dialog(FontBox*, psy_ui_Component* sender);
static void fontbox_build(FontBox*);
static void fontbox_on_family_selected(FontBox*, psy_ui_Component* sender,
	intptr_t index);
static intptr_t fontbox_index(FontBox*, const char* family);
static void fontbox_update_style(FontBox*, const psy_ui_FontInfo*);
static void fontbox_on_style(FontBox*, psy_ui_Button* sender);
static void fontbox_on_size(FontBox*, IntEdit* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(FontBox* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			fontbox_on_destroyed;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(fontbox_base(self), &vtable);
}

/* implementation */
void fontbox_init(FontBox* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->property = NULL;
	psy_ui_component_set_default_align(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_margin_make_em(
			0.0, 1.0, 0.0, 0.0));
	psy_ui_combobox_init(&self->family, &self->component);
	psy_ui_combobox_set_char_number(&self->family, 16.0);
	intedit_init(&self->size, &self->component, "Size", 
		12, 0, 128);
	psy_ui_button_init_text_connect(&self->bold, &self->component, "B",
		self, fontbox_on_style);
	psy_ui_button_init_text_connect(&self->unterline, &self->component, "U",
		self, fontbox_on_style);
	psy_ui_button_init_text_connect(&self->italic, &self->component, "I",
		self, fontbox_on_style);
	psy_ui_button_init_connect(&self->dialog, &self->component,
		self, fontbox_on_dialog);
	psy_ui_button_prevent_translation(&self->dialog);
	psy_ui_button_set_text(&self->dialog, "...");
	fontbox_build(self);
	psy_signal_connect(&self->family.signal_selchanged, self,
		fontbox_on_family_selected);
	psy_signal_connect(&self->size.signal_changed, self,
		fontbox_on_size);
}

void fontbox_on_destroyed(FontBox* self)
{	
	assert(self);

	if (self->property) {
		psy_property_disconnect(self->property, self);
	}
}

FontBox* fontbox_alloc(void)
{
	return (FontBox*)malloc(sizeof(FontBox));
}

FontBox* fontbox_allocinit(psy_ui_Component* parent)
{
	FontBox* rv;

	rv = fontbox_alloc();
	if (rv) {
		fontbox_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void fontbox_data_exchange(FontBox* self, psy_Property* property)
{
	assert(self);
	assert(property);

	self->property = property;
	if (property) {
		fontbox_on_property_changed(self, property);
		psy_property_connect(property, self,
			fontbox_on_property_changed);
		psy_signal_connect(&self->property->before_destroyed, self,
			fontbox_before_property_destroyed);
	}
}

intptr_t fontbox_index(FontBox* self, const char* family)
{
	intptr_t index;
	uintptr_t i;

	index = -1;
	for (i = 0; i < (uintptr_t)psy_ui_combobox_count(&self->family); ++i) {
		char text[512];

		psy_ui_combobox_text_at(&self->family, text, i);
		if (strcmp(text, family) == 0) {
			index = i;
			break;
		}
	}
	return index;
}

void fontbox_on_property_changed(FontBox* self, psy_Property* sender)
{
	psy_ui_FontInfo fontinfo;
	intptr_t index;
	
	psy_ui_fontinfo_init_string(&fontinfo,
		psy_property_item_str(self->property));
	intedit_set_value(&self->size, (int)fontinfo.lfHeight);
	index = fontbox_index(self, fontinfo.lfFaceName);
	if (index != -1) {
		psy_ui_combobox_select(&self->family, index);
	}
	fontbox_update_style(self, &fontinfo);
}

void fontbox_update_style(FontBox* self, const psy_ui_FontInfo* font_info)
{
	assert(self);
	assert(font_info);

	if (font_info->lfItalic) {
		psy_ui_button_highlight(&self->italic);
	} else {
		psy_ui_button_disable_highlight(&self->italic);
	}
	if (font_info->lfUnderline) {
		psy_ui_button_highlight(&self->unterline);
	} else {
		psy_ui_button_disable_highlight(&self->unterline);
	}
}

void fontbox_before_property_destroyed(FontBox* self, psy_Property* sender)
{
	assert(self);

	self->property = NULL;
}

void fontbox_on_dialog(FontBox* self,psy_ui_Component* sender)
{
	psy_ui_FontDialog dialog;
	psy_ui_FontInfo fontinfo;

	psy_ui_fontdialog_init(&dialog, &self->component);
	psy_ui_fontinfo_init_string(&fontinfo,
		psy_property_item_str(self->property));
	psy_ui_fontdialog_setfontinfo(&dialog, fontinfo);
	if (psy_ui_fontdialog_execute(&dialog)) {
		psy_ui_FontInfo fontinfo;
		char fontstr[256];

		fontinfo = psy_ui_fontdialog_fontinfo(&dialog);
		psy_ui_fontinfo_string(&fontinfo, fontstr, 256);
		psy_property_set_item_font(self->property, fontstr);
	}
	psy_ui_fontdialog_dispose(&dialog);
}

void fontbox_build(FontBox* self)
{
	const psy_List* fonts;
	
	fonts = psy_ui_app_fonts(psy_ui_app());
	if (fonts) {
		const psy_List* p;

		for (p = fonts; p != NULL; p = p->next) {
			const psy_ui_FontInfo* font_info;

			font_info = (const psy_ui_FontInfo*)p->entry;
			psy_ui_combobox_add_text(&self->family, font_info->lfFaceName);			
		}
	}	
}

void fontbox_on_family_selected(FontBox* self, psy_ui_Component* sender,
	intptr_t index)
{
	assert(self);

	if (self->property) {
		char text[512];
		psy_ui_FontInfo fontinfo;

		psy_ui_combobox_text_at(&self->family, text, index);		
		psy_ui_fontinfo_init_string(&fontinfo,
			psy_property_item_str(self->property));
		psy_snprintf(fontinfo.lfFaceName, 32, "%s", text);
		psy_ui_fontinfo_string(&fontinfo, text, 512);				
		psy_property_set_item_str(self->property, text);
	}
}

void fontbox_on_style(FontBox* self, psy_ui_Button* sender)
{
	psy_ui_FontInfo fontinfo;
	char text[512];

	if (psy_ui_button_highlighted(sender)) {
		psy_ui_button_disable_highlight(sender);
	} else {
		psy_ui_button_highlight(sender);
	}
	psy_ui_fontinfo_init_string(&fontinfo,
		psy_property_item_str(self->property));
	fontinfo.lfItalic = psy_ui_button_highlighted(&self->italic);
	fontinfo.lfUnderline = psy_ui_button_highlighted(&self->unterline);
	psy_ui_fontinfo_string(&fontinfo, text, 512);
	psy_property_set_item_str(self->property, text);
}

void fontbox_on_size(FontBox* self, IntEdit* sender)
{
	psy_ui_FontInfo fontinfo;
	char text[512];
	
	psy_ui_fontinfo_init_string(&fontinfo,
		psy_property_item_str(self->property));
	fontinfo.lfHeight = intedit_value(sender);	
	psy_ui_fontinfo_string(&fontinfo, text, 512);
	psy_property_set_item_str(self->property, text);
}

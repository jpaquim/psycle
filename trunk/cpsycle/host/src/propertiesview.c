/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* host */
#include "propertiesview.h"
#include "trackergridstate.h" /* TRACKER CMDS */
#include "styles.h"
/* ui */
#include <uicolordialog.h>
#include <uifolderdialog.h>
#include <uifontdialog.h>
#include <uiswitch.h>
/* platform */
#include "../../detail/portable.h"

/* PropertiesRenderState */
void propertiesrenderstate_init(PropertiesRenderState* self, uintptr_t numcols)
{	
	self->property_line_changed = NULL;	
	self->dialogbutton = FALSE;
	self->numcols = numcols;	
	self->prevent_mouse_propagation = TRUE;	
	psy_ui_size_init_em(&self->size_col0, 80.0, 1.3);
	psy_ui_size_init_em(&self->size_col2, 40.0, 1.3);	
}

/* PropertiesRenderLine */

/* prototypes */
static void propertiesrenderline_on_destroy(PropertiesRenderLine*);
static void propertiesrenderline_on_mouse_down(PropertiesRenderLine*,
	psy_ui_MouseEvent*);
static void propertiesrenderline_on_combo_select(PropertiesRenderLine*,
	psy_ui_Component* sender, intptr_t selidx);
static void propertiesrenderline_on_input_definer_accept(PropertiesRenderLine*,
	InputDefiner* sender);
static void propertiesrenderline_on_action_button(PropertiesRenderLine*,
	psy_ui_Switch* sender);
static void propertiesrenderline_on_font_dialog_button(PropertiesRenderLine*,
	psy_ui_Component* sender);
static void propertiesrenderline_on_colour_dialog_button(PropertiesRenderLine*,
	psy_ui_Component* sender);
static void propertiesrenderline_on_directory_dialog_button(PropertiesRenderLine*,
	psy_ui_Component* sender);
static void propertiesrenderline_on_switch_button(PropertiesRenderLine*,
	psy_ui_Switch* sender);
static void propertiesrenderline_on_edit_accept(PropertiesRenderLine*,
	psy_ui_TextArea* sender);
static void propertiesrenderline_on_edit_keydown(PropertiesRenderLine*,
	psy_ui_Component* sender, psy_ui_KeyboardEvent* ev);
static void propertiesrenderline_on_edit_reject(PropertiesRenderLine*,
	psy_ui_TextArea* sender);

/* vtable */
static psy_ui_ComponentVtable propertiesrenderline_vtable;
static bool propertiesrenderline_vtable_initialized = FALSE;

static void propertiesrenderline_vtable_init(PropertiesRenderLine* self)
{
	if (!propertiesrenderline_vtable_initialized) {
		propertiesrenderline_vtable = *(self->component.vtable);
		propertiesrenderline_vtable.on_destroy =
			(psy_ui_fp_component_event)
			propertiesrenderline_on_destroy;
		propertiesrenderline_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			propertiesrenderline_on_mouse_down;
		propertiesrenderline_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &propertiesrenderline_vtable);
}

/* implementation */
void propertiesrenderline_init(PropertiesRenderLine* self,
	psy_ui_Component* parent,
	PropertiesRenderState* state, psy_Property* property,
	uintptr_t level)
{
	psy_ui_Label* key;

	psy_ui_component_init(&self->component, parent, NULL);	
	propertiesrenderline_vtable_init(self);	
	self->choice_line = NULL;
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_selected);
	psy_ui_component_set_align(&self->component, psy_ui_ALIGN_TOP);	
	psy_ui_component_set_align_expand(&self->component,
		psy_ui_HEXPAND);
	if (state->numcols == 1) {
		psy_ui_component_set_preferred_size(&self->component, state->size_col0);
	} else {
		psy_ui_component_set_preferred_size(&self->component,
			psy_ui_size_make_em(190.0, 1.5));
	}
	self->property = property;	
	assert(self->property);
	self->check = NULL;
	self->colour = NULL;
	self->label = NULL;
	self->combo = NULL;
	self->input_definer = NULL;	
	self->edit = NULL;
	self->state = state;	
	/* column 0 */
	key = psy_ui_label_allocinit(&self->component);	
	psy_ui_component_set_padding(psy_ui_label_base(key),
		psy_ui_margin_make_em(0.0, 0.0, 0.0, psy_min(level, 5.0) * 4.0));
	psy_ui_component_set_preferred_size(psy_ui_label_base(key),
		self->state->size_col0);
	if (state->numcols == 1) {
		psy_ui_component_set_align(&key->component, psy_ui_ALIGN_CLIENT);
	} else {
		psy_ui_component_set_align(&key->component, psy_ui_ALIGN_LEFT);
	}	
	if (!property->item.translate) {
		psy_ui_label_prevent_translation(key);
	}
	if (!psy_property_is_action(self->property)) {
		psy_ui_label_set_text(key, psy_property_text(self->property));
	}
	/* column 1 */
	if (state->numcols > 1) {
		if (psy_property_hint(property) == PSY_PROPERTY_HINT_COMBO) {
			self->combo = psy_ui_combobox_allocinit(&self->component);
			psy_signal_connect(&self->combo->signal_selchanged, self,
				propertiesrenderline_on_combo_select);
			psy_ui_combobox_setcharnumber(self->combo, 50.0);
			psy_ui_component_set_align(&self->combo->component, psy_ui_ALIGN_LEFT);			
		} else if (psy_property_isbool(self->property) || psy_property_is_choice_item(
				self->property)) {
			self->check = psy_ui_switch_allocinit(&self->component, NULL);
			psy_signal_connect(&self->check->signal_clicked, self,
				propertiesrenderline_on_switch_button);
		} else if (psy_property_is_action(self->property)) {
			psy_ui_Button* button;

			button = psy_ui_button_allocinit(&self->component);
			psy_ui_component_set_align(psy_ui_button_base(button), psy_ui_ALIGN_LEFT);
			psy_ui_button_set_text(button, psy_property_text(self->property));
			psy_signal_connect(&button->signal_clicked, self,
				propertiesrenderline_on_action_button);
		} else if (psy_property_is_font(self->property)) {
			self->label = psy_ui_label_allocinit(&self->component);
			psy_ui_component_set_align(psy_ui_label_base(self->label),
				psy_ui_ALIGN_CLIENT);		
		} else if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {
			psy_ui_Component* col1;

			col1 = psy_ui_component_allocinit(&self->component, NULL);
			psy_ui_component_set_align(col1, psy_ui_ALIGN_CLIENT);
			psy_ui_component_set_align_expand(col1, psy_ui_HEXPAND);
			self->label = psy_ui_label_allocinit(col1);
			psy_ui_component_set_align(psy_ui_label_base(self->label),
				psy_ui_ALIGN_LEFT);
			psy_ui_label_set_charnumber(self->label, 20.0);
			self->colour = psy_ui_component_allocinit(col1, NULL);
			psy_ui_component_set_align(self->colour, psy_ui_ALIGN_LEFT);
			psy_ui_component_set_preferred_size(self->colour,
				psy_ui_size_make_em(4.0, 1.0));
			psy_ui_component_set_align(col1, psy_ui_ALIGN_CLIENT);			
		} else if (psy_property_hint(property) == PSY_PROPERTY_HINT_SHORTCUT) {
			self->input_definer = inputdefiner_allocinit(&self->component);
			psy_ui_component_set_align(inputdefiner_base(self->input_definer),
				psy_ui_ALIGN_CLIENT);
			psy_signal_connect(&self->input_definer->signal_accept, self,
				propertiesrenderline_on_input_definer_accept);
		} else if (
				psy_property_is_int(self->property) ||
				psy_property_is_string(self->property) ||
				psy_property_is_font(self->property)) {
			if (psy_property_readonly(self->property)) {
				self->label = psy_ui_label_allocinit(&self->component);
				psy_ui_component_set_align(psy_ui_label_base(self->label),
					psy_ui_ALIGN_CLIENT);
			} else {
				self->edit = psy_ui_textarea_allocinit_single_line(&self->component);
				psy_ui_component_set_align(psy_ui_textarea_base(self->edit),
					psy_ui_ALIGN_CLIENT);
				psy_ui_textarea_enable_input_field(self->edit);
				psy_signal_connect(&self->edit->signal_accept, self,
					propertiesrenderline_on_edit_accept);
				psy_signal_connect(&self->edit->signal_reject, self,
					propertiesrenderline_on_edit_reject);
				psy_signal_connect(&self->edit->component.signal_keydown, self,
					propertiesrenderline_on_edit_keydown);
			}						
		} else {			
			psy_ui_component_set_align(psy_ui_component_allocinit(
				&self->component, NULL), psy_ui_ALIGN_CLIENT);
		}
		if (self->check) {
			psy_ui_component_set_align(&self->check->component, psy_ui_ALIGN_CLIENT);
		}
		if (self->label) {
			psy_ui_label_prevent_translation(self->label);
		}						
	}
	/* column 2 */
	if (state->numcols > 2) {				
		psy_ui_Component* col2;
		psy_ui_Button* dialogbutton;

		col2 = NULL;		
		if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITDIR ||
			psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {			
			dialogbutton = psy_ui_button_allocinit(&self->component);
			psy_ui_button_prevent_translation(dialogbutton);
			psy_ui_button_set_text(dialogbutton, "...");
			if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {
				psy_signal_connect(&dialogbutton->signal_clicked, self,
					propertiesrenderline_on_colour_dialog_button);
			} else {
				psy_signal_connect(&dialogbutton->signal_clicked, self,
					propertiesrenderline_on_directory_dialog_button);
			}
			col2 = &dialogbutton->component;			
		} else if (psy_property_is_font(self->property)) {
			dialogbutton = psy_ui_button_allocinit(&self->component);
			col2 = &dialogbutton->component;
			psy_ui_button_set_text(dialogbutton, "settingsview.choose-font");
			psy_signal_connect(&dialogbutton->signal_clicked, self,
				propertiesrenderline_on_font_dialog_button);			
		} else if (psy_property_hint(property) == PSY_PROPERTY_HINT_SHORTCUT) {
			dialogbutton = psy_ui_button_allocinit(&self->component);
			col2 = &dialogbutton->component;
			psy_ui_button_set_text(dialogbutton, "settingsview.none");
		} else if (psy_property_int_hasrange(property) &&
				!psy_property_readonly(property)) {
			char text[256];
			psy_ui_Label* label;

			/*
			** todo: psy_ui_translate translates only at build, use
			** onlanguageupdate to handle settingsview changes
			*/
			psy_snprintf(text, 256, "%s %d %s %d",
				psy_ui_translate("settingsview.from"),
				property->item.min,
				psy_ui_translate("settingsview.to"),
				property->item.max);
			label = psy_ui_label_allocinit(&self->component);
			col2 = &label->component;
			psy_ui_label_prevent_translation(label);
			psy_ui_label_set_text(label, text);
		}
		if (col2) {
			psy_ui_component_set_align(col2, psy_ui_ALIGN_RIGHT);
			psy_ui_component_set_preferred_size(col2, self->state->size_col2);				
		}
	}
	propertiesrenderline_update(self);
}

void propertiesrenderline_on_destroy(PropertiesRenderLine* self)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_selected);
}

PropertiesRenderLine* propertiesrenderline_alloc(void)
{
	return (PropertiesRenderLine*)malloc(sizeof(PropertiesRenderLine));
}

PropertiesRenderLine* propertiesrenderline_allocinit(
	psy_ui_Component* parent,
	PropertiesRenderState* state, psy_Property* property, uintptr_t level)
{
	PropertiesRenderLine* rv;

	rv = propertiesrenderline_alloc();
	if (rv) {
		propertiesrenderline_init(rv, parent, state, property, level);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void propertiesrenderline_on_mouse_down(PropertiesRenderLine* self,
	psy_ui_MouseEvent* ev)
{	
	psy_signal_emit(&self->signal_selected, self, 0);
	psy_ui_mouseevent_stop_propagation(ev);
}

void propertiesrenderline_on_combo_select(PropertiesRenderLine* self,
	psy_ui_Component* sender, intptr_t selidx)
{
	psy_property_setitem_int(self->property, selidx);
	self->state->property_line_changed = self;
}

bool propertiesrenderline_update_bool(PropertiesRenderLine* self)
{
	bool checked;
	
	if (!self->check&& self->property) {
		return FALSE;
	}
	if (psy_property_isbool(self->property)) {
		checked = psy_property_item_bool(self->property);
	} else if (psy_property_is_choice_item(self->property)) {
		checked = (psy_property_at_choice(psy_property_parent(self->property))
			== self->property);
	} else {
		return FALSE;
	}	
	if (checked) {
		psy_ui_switch_check(self->check);
	} else {
		psy_ui_switch_uncheck(self->check);	
	}	
	return TRUE;
}

bool propertiesrenderline_update_string(PropertiesRenderLine* self)
{
	if (self->label && psy_property_is_string(self->property)) {		
		psy_ui_label_set_text(self->label,
			psy_property_item_str(self->property));
		return TRUE;
	}
	if (self->edit && psy_property_is_string(self->property)) {
		psy_ui_textarea_set_text(self->edit,
			psy_property_item_str(self->property));
		return TRUE;
	}
	return FALSE;
}

bool propertiesrenderline_update_int(PropertiesRenderLine* self)
{
	char text[64];

	if (!psy_property_is_int(self->property)) {
		return FALSE;
	}
	if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_SHORTCUT) {
		if (self->input_definer) {
			inputdefiner_set_input(self->input_definer,
				(uint32_t)psy_property_item_int(self->property));
		}
		return TRUE;		
	}	
	if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {
		if (self->label) {
			char str[64];

			psy_snprintf(str, 64, "0x%d", psy_property_item_int(self->property));
			psy_ui_label_set_text(self->label, str);
		}
		if (self->colour) {
			psy_ui_component_set_background_colour(self->colour,
				psy_ui_colour_make((uint32_t)psy_property_item_int(
					self->property)));
		}
		return TRUE;
	}	
	psy_snprintf(text, 40,
		(psy_property_ishex(self->property)) ? "%X" : "%d",
		(int)psy_property_item_int(self->property));
	if (self->label) {		
		psy_ui_label_set_text(self->label, text);
		return TRUE;
	}
	if (self->edit) {
		psy_ui_textarea_set_text(self->edit, text);
		return TRUE;
	}
	return FALSE;
}

bool propertiesrenderline_update_font(PropertiesRenderLine* self)
{
	if (psy_property_is_font(self->property)) {
		if (self->label) {
			char str[128];
			psy_ui_FontInfo fontinfo;
			psy_ui_Font font;
			int pt;
			const psy_ui_TextMetric* tm;

			psy_ui_fontinfo_init_string(&fontinfo,
				psy_property_item_str(self->property));
			psy_ui_font_init(&font, &fontinfo);
			tm = psy_ui_font_textmetric(&font);
			if (fontinfo.lfHeight < 0) {
				pt = ((tm->tmHeight - tm->tmInternalLeading) * 72) /
					psy_ui_logpixelsy();
			} else {
				pt = tm->tmHeight;
			}
			psy_ui_font_dispose(&font);
			tm = NULL;
			psy_snprintf(str, 128, "%s %d pt", fontinfo.lfFaceName, (int)pt);
			if (self->label) {
				psy_ui_label_set_text(self->label, str);
			}
		}
		return TRUE;
	}
	return FALSE;
}

void propertiesrenderline_update(PropertiesRenderLine* self)
{
	if (self->state->numcols < 2) {
		return;
	}
	if (propertiesrenderline_update_bool(self)) {		
		return;
	}
	if (propertiesrenderline_update_int(self)) {
		return;
	}
	if (propertiesrenderline_update_font(self)) {		
		return;
	}
	if (propertiesrenderline_update_string(self)) {		
		return;
	}	
}

void propertiesrenderline_on_input_definer_accept(PropertiesRenderLine* self,
	InputDefiner* sender)
{
	if (self->property && self->input_definer) {
		psy_property_setitem_int(self->property,
			inputdefiner_input(self->input_definer));
		psy_ui_component_set_focus(psy_ui_component_parent(&self->component));
		psy_signal_emit(&self->signal_changed, self, 0);
	}
}

void propertiesrenderline_on_font_dialog_button(PropertiesRenderLine* self,
	psy_ui_Component* sender)
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
		psy_property_setitem_font(self->property, fontstr);
		psy_signal_emit(&self->signal_changed, self, 0);
	}
	psy_ui_fontdialog_dispose(&dialog);
}

void propertiesrenderline_on_colour_dialog_button(PropertiesRenderLine* self,
	psy_ui_Component* sender)
{
	psy_ui_ColourDialog dialog;

	psy_ui_colourdialog_init(&dialog, &self->component);
	psy_ui_colourdialog_setcolour(&dialog,
		psy_ui_colour_make((uint32_t)psy_property_item_int(self->property)));
	if (psy_ui_colourdialog_execute(&dialog)) {
		psy_ui_Colour colour;

		colour = psy_ui_colourdialog_colour(&dialog);
		psy_property_setitem_int(self->property, psy_ui_colour_colorref(&colour));
		psy_signal_emit(&self->signal_changed, self, 0);
	}
	psy_ui_colourdialog_dispose(&dialog);
}

void propertiesrenderline_on_directory_dialog_button(PropertiesRenderLine* self,
	psy_ui_Component* sender)
{
	psy_ui_FolderDialog dialog;

	psy_ui_folderdialog_init_all(&dialog, 0, psy_ui_translate(
		psy_property_text(self->property)), "");
	if (psy_ui_folderdialog_execute(&dialog)) {
		psy_property_setitem_str(self->property,
			psy_ui_folderdialog_path(&dialog));
		psy_ui_folderdialog_dispose(&dialog);
		psy_signal_emit(&self->signal_changed, self, 0);		
	}	
}

void propertiesrenderline_on_action_button(PropertiesRenderLine* self,
	psy_ui_Switch* sender)
{
	self->state->property_line_changed = self;
}

void propertiesrenderline_on_switch_button(PropertiesRenderLine* self,
	psy_ui_Switch* sender)
{	
	if (psy_property_is_choice_item(self->property)) {
		psy_property_setitem_int(psy_property_parent(self->property),
			psy_property_choiceitem_index(self->property));
		self->state->property_line_changed = self->choice_line;
	} else if (psy_property_isbool(self->property)) {
		psy_property_setitem_bool(self->property,
			!psy_property_item_bool(self->property));
		propertiesrenderline_update_bool(self);
		self->state->property_line_changed = self;
	}	
}

void propertiesrenderline_on_edit_accept(PropertiesRenderLine* self,
	psy_ui_TextArea* sender)
{
	psy_ui_component_set_focus(&self->component);	
	if (psy_property_is_int(self->property)) {
		psy_property_setitem_int(self->property,
			(psy_property_ishex(self->property))
			? strtol(psy_ui_textarea_text(self->edit), NULL, 16)
			: atoi(psy_ui_textarea_text(self->edit)));
	} else if (psy_property_is_string(self->property)) {
		psy_property_setitem_str(self->property,
			psy_ui_textarea_text(self->edit));
	}
	self->state->property_line_changed = self;	
}

void propertiesrenderline_on_edit_keydown(PropertiesRenderLine* self,
	psy_ui_Component* sender, psy_ui_KeyboardEvent* ev)
{
	if (psy_property_ishex(self->property)) {
		if ((psy_ui_keyboardevent_keycode(ev) >= psy_ui_KEY_DIGIT0 && psy_ui_keyboardevent_keycode(ev) <= psy_ui_KEY_DIGIT9) ||
			(psy_ui_keyboardevent_keycode(ev) >= psy_ui_KEY_A && psy_ui_keyboardevent_keycode(ev) <= psy_ui_KEY_F) ||
			(psy_ui_keyboardevent_keycode(ev) < psy_ui_KEY_HELP)) {
			return;
		}
		psy_ui_keyboardevent_prevent_default(ev);
	}
	psy_ui_keyboardevent_stop_propagation(ev);
}

void propertiesrenderline_on_edit_reject(PropertiesRenderLine* self,
	psy_ui_TextArea* sender)
{	
	psy_ui_component_set_focus(&self->component);
}


/* PropertiesRenderer */

/* prototypes */
static void propertiesrenderer_on_destroy(PropertiesRenderer*);
static void propertiesrenderer_on_timer(PropertiesRenderer*, uintptr_t timerid);
static int propertiesrenderer_on_properties_build(PropertiesRenderer*,
	psy_Property*, uintptr_t level);
static void propertiesrenderer_on_line_changed(PropertiesRenderer*,
	PropertiesRenderLine* sender);
static void propertiesrenderer_on_line_selected(PropertiesRenderer*,
	PropertiesRenderLine* sender);
static void propertiesrenderer_build_main_section(PropertiesRenderer*,
	psy_Property* section);
static void propertiesrenderer_on_mouse_down(PropertiesRenderer*,
	psy_ui_MouseEvent*);
static void propertiesview_goto_section(PropertiesView*, uintptr_t index);

/* vtable */
static psy_ui_ComponentVtable propertiesrenderer_vtable;
static bool propertiesrenderer_vtable_initialized = FALSE;

static void propertiesrenderer_vtable_init(PropertiesRenderer* self)
{
	if (!propertiesrenderer_vtable_initialized) {
		propertiesrenderer_vtable = *(self->component.vtable);
		propertiesrenderer_vtable.on_destroy =
			(psy_ui_fp_component_event)
			propertiesrenderer_on_destroy;
		propertiesrenderer_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			propertiesrenderer_on_mouse_down;
		propertiesrenderer_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			propertiesrenderer_on_timer;			
		propertiesrenderer_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &propertiesrenderer_vtable);
}

/* implementation */
void propertiesrenderer_init(PropertiesRenderer* self,
	psy_ui_Component* parent, psy_Property* properties, uintptr_t numcols)
{
	self->properties = properties;	
	self->curr = NULL;
	self->choicelevel = psy_INDEX_INVALID;	
	self->choiceline = NULL;
	self->mainsectionstyle = STYLE_PROPERTYVIEW_MAINSECTION;
	self->mainsectionheaderstyle = STYLE_PROPERTYVIEW_MAINSECTIONHEADER;
	self->keystyle = psy_INDEX_INVALID;
	self->keystyle_hover = psy_INDEX_INVALID;
	self->linestyle_select = psy_INDEX_INVALID;
	self->rebuild_level = 0;
	psy_ui_component_init(&self->component, parent, NULL);
	propertiesrenderer_vtable_init(self);	
	psy_ui_component_set_wheel_scroll(&self->component, 4);	
	psy_ui_component_init(&self->client, &self->component, NULL);	
	psy_ui_component_set_align(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_defaultalign(&self->client,
		psy_ui_ALIGN_TOP, psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));	
	propertiesrenderstate_init(&self->state, numcols);	
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_selected);	
	psy_ui_component_set_overflow(&self->component, psy_ui_OVERFLOW_VSCROLL);	
	psy_ui_component_setscrollstep(&self->component,
		psy_ui_size_make_em(0.0, 4.0));
	psy_table_init(&self->sections);
	propertiesrenderer_build(self);	
	psy_ui_component_start_timer(&self->component, 0, 100);
}

void propertiesrenderer_on_destroy(PropertiesRenderer* self)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_selected);
	psy_table_dispose(&self->sections);
}

void propertiesrenderer_setstyle(PropertiesRenderer* self,
	uintptr_t mainsection,
	uintptr_t mainsectionheader,
	uintptr_t keystyle,
	uintptr_t keystyle_hover,
	uintptr_t linestyle_select)
{
	self->mainsectionstyle = mainsection;
	self->mainsectionheaderstyle = mainsectionheader;
	self->keystyle = keystyle;
	self->keystyle_hover = keystyle_hover;
	self->linestyle_select = linestyle_select;
	propertiesrenderer_build(self);
}

void propertiesrenderer_build(PropertiesRenderer* self)
{
	self->curr = NULL;	
	self->choiceline = NULL;	
	self->choicelevel = psy_INDEX_INVALID;
	self->rebuild_level = 0;	
	psy_table_clear(&self->sections);
	psy_ui_component_clear(&self->client);
	psy_property_enumerate(self->properties, self,
		(psy_PropertyCallback)
		propertiesrenderer_on_properties_build);
	if (self->choicelevel != psy_INDEX_INVALID && self->choiceline) {
		if (self->choiceline->combo) {
			psy_ui_combobox_setcursel(self->choiceline->combo,
				psy_property_item_int(self->choiceline->property));
		}
		self->choicelevel = psy_INDEX_INVALID;
		self->choiceline = NULL;
	}
}

void propertiesrenderer_rebuild(PropertiesRenderer* self,
	psy_Property* mainsection)
{
	psy_ui_Component* section;

	if (!mainsection) {
		return;
	}	
	section = (psy_ui_Component*)psy_table_at(&self->sections,
		psy_property_index(mainsection));
	if (section) {
		psy_ui_Component* clients;

		clients = psy_ui_component_at(section, 1);
		if (clients) {
			psy_ui_component_clear(clients);
			self->rebuild_level = 1;
			self->curr = clients;			
			self->choiceline = NULL;
			self->choicelevel = psy_INDEX_INVALID;
			psy_property_enumerate(mainsection, self, (psy_PropertyCallback)
				propertiesrenderer_on_properties_build);
			psy_ui_component_align(clients);
			psy_ui_component_align_full(&self->component);
			psy_ui_component_invalidate(section);
			self->rebuild_level = 0;
		}		
	}
}

void propertiesrenderer_update_line(PropertiesRenderer* self,
	PropertiesRenderLine* line)
{
	if (!line) {
		return;
	}
	if (psy_property_is_choice_item(line->property)) {
		psy_ui_Component* lines;
		psy_List* q;
		psy_List* p;

		lines = psy_ui_component_parent(&line->component);
		if (lines) {
			q = psy_ui_component_children(lines, psy_ui_NONE_RECURSIVE);
			for (p = q; p != NULL; p = p->next) {
				propertiesrenderline_update((PropertiesRenderLine*)p->entry);
			}
			psy_list_free(q);
		}
		return;
	}
	propertiesrenderline_update(line);
}

int propertiesrenderer_on_properties_build(PropertiesRenderer* self,
	psy_Property* property, uintptr_t level)
{
	if (psy_property_hint(property) == PSY_PROPERTY_HINT_HIDE) {
		return 2;
	}	
	if (self->rebuild_level == 0 && level == 0) {
		propertiesrenderer_build_main_section(self, property);
	} else {
		if (!self->curr) {
			self->curr = &self->client;
		}
		if (self->choicelevel != psy_INDEX_INVALID) {
			if (level != self->choicelevel + 1) {			
				if (self->choiceline && self->choiceline->combo) {
					psy_ui_combobox_setcursel(self->choiceline->combo,
						psy_property_item_int(self->choiceline->property));
				}
				self->choicelevel = psy_INDEX_INVALID;				
				self->choiceline = NULL;
			} else {
				if (self->choiceline) {
					if (self->choiceline->combo) {
						if (psy_property_translation_prevented(property)) {
							psy_ui_combobox_add_text(self->choiceline->combo,
								psy_property_text(property));
						} else {
							psy_ui_combobox_add_text(self->choiceline->combo,
								psy_ui_translate(psy_property_text(property)));
						}
						return 1;
					}					
				}
			}
		}
		if (psy_property_hint(property) != PSY_PROPERTY_HINT_HIDE) {
			PropertiesRenderLine* line;
						
			line = propertiesrenderline_allocinit(self->curr,
				&self->state, property, level + self->rebuild_level);
			line->choice_line = self->choiceline;
			psy_signal_connect(&line->signal_changed, self,
				propertiesrenderer_on_line_changed);
			psy_signal_connect(&line->signal_selected, self,
				propertiesrenderer_on_line_selected);
			psy_ui_component_set_style_type(&line->component,
				self->keystyle);
			psy_ui_component_set_style_type_hover(&line->component,
				self->keystyle_hover);
			psy_ui_component_set_style_type_select(&line->component,
				self->linestyle_select);
			if (psy_property_type(property) == PSY_PROPERTY_TYPE_CHOICE) {				
				self->choicelevel = level;
				self->choiceline = line;
			}
		}
	}
	return 1;
}

void propertiesrenderer_build_main_section(PropertiesRenderer* self,
	psy_Property* section)
{
	if (psy_property_issection(section)) {
		psy_ui_Component* currsection;
		psy_ui_Component* lines;
		psy_ui_Label* label;

		currsection = psy_ui_component_allocinit(&self->client, NULL);
		psy_ui_component_set_defaultalign(currsection, psy_ui_ALIGN_TOP,
			psy_ui_margin_zero());		
		psy_ui_component_set_style_type(currsection, self->mainsectionstyle);
		label = psy_ui_label_allocinit(currsection);		
		psy_ui_component_set_margin(psy_ui_label_base(label),
			psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));
		psy_ui_component_set_padding(psy_ui_label_base(label),
			psy_ui_margin_make_em(0.5, 0.0, 0.5, 1.0));
		psy_ui_component_set_style_type(psy_ui_label_base(label),
			self->mainsectionheaderstyle);		
		lines = psy_ui_component_allocinit(currsection, NULL);
		psy_ui_component_set_defaultalign(lines, psy_ui_ALIGN_TOP,
			psy_ui_margin_zero());
		psy_ui_label_set_text(label, psy_property_text(section));
		psy_table_insert(&self->sections, psy_property_index(section),
			currsection);
		self->curr = lines;		
	}
}

void propertiesrenderer_on_timer(PropertiesRenderer* self, uintptr_t timerid)
{
	PropertiesRenderLine* line;

	line = self->state.property_line_changed;	
	if (line) {
		self->state.property_line_changed = NULL;
		psy_signal_emit(&self->signal_changed, self, 1, line);		
	}
}

void propertiesrenderer_on_mouse_down(PropertiesRenderer* self,
	psy_ui_MouseEvent* ev)
{
	if (self->state.prevent_mouse_propagation) {
		psy_ui_mouseevent_stop_propagation(ev);
	}	
}

void propertiesrenderer_on_line_changed(PropertiesRenderer* self,
	PropertiesRenderLine* sender)
{
	psy_signal_emit(&self->signal_changed, self, 1, sender);
}

void propertiesrenderer_on_line_selected(PropertiesRenderer* self,
	PropertiesRenderLine* sender)
{
	psy_signal_emit(&self->signal_selected, self, 1, sender);
}

/* PropertiesView */

/* prototypes */
static void propertiesview_on_destroy(PropertiesView*);
static void propertiesview_select_section(PropertiesView*,
	psy_ui_Component* sender, uintptr_t section, uintptr_t options);
static void propertiesview_update_tabbar_sections(PropertiesView*);
static void propertiesview_on_tabbar_change(PropertiesView*,
	psy_ui_Component* sender, uintptr_t tabindex);
static void propertiesview_on_property_changed(PropertiesView*,
	PropertiesRenderer* sender, PropertiesRenderLine*);
static void propertiesview_on_selected(PropertiesView*,
	PropertiesRenderer* sender, PropertiesRenderLine*);
static bool propertiesview_on_input(PropertiesView*, InputHandler* sender);
static double propertiesview_check_range(PropertiesView*, double position);
static void propertiesview_on_mouse_down(PropertiesView*, psy_ui_MouseEvent*);
static void propertiesview_on_mouse_up(PropertiesView*, psy_ui_MouseEvent*);
static void propertiesview_on_scroll_pane_align(PropertiesView*,
	psy_ui_Component* sender);

/* vtable */
static psy_ui_ComponentVtable propertiesview_vtable;
static bool propertiesview_vtable_initialized = FALSE;

static void propertiesview_vtable_init(PropertiesView* self)
{
	if (!propertiesview_vtable_initialized) {
		propertiesview_vtable = *(self->component.vtable);
		propertiesview_vtable.on_destroy =
			(psy_ui_fp_component_event)
			propertiesview_on_destroy;
		propertiesview_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			propertiesview_on_mouse_up;
		propertiesview_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			propertiesview_on_mouse_up;
	}
	self->component.vtable = &propertiesview_vtable;
}

/* implementation */
void propertiesview_init(PropertiesView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_Property* properties,
	uintptr_t numcols, InputHandler* input_handler)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_id(&self->component, VIEW_ID_SONGPROPERTIES);
	propertiesview_vtable_init(self);
	self->maximizemainsections = TRUE;
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_selected);
	psy_ui_component_init(&self->viewtabbar, tabbarparent, NULL);
	propertiesrenderer_init(&self->renderer, &self->component, properties,
		numcols);
	psy_ui_scroller_init(&self->scroller, &self->component, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->renderer.component);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_margin(&self->scroller.pane,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));
	psy_ui_component_set_margin(&self->scroller.component,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));
	psy_ui_component_set_align(&self->renderer.component, psy_ui_ALIGN_HCLIENT);
	psy_signal_connect(&self->component.signal_selectsection, self,
		propertiesview_select_section);
	psy_ui_tabbar_init(&self->tabbar, &self->component);
	psy_ui_tabbar_settabalign(&self->tabbar, psy_ui_ALIGN_TOP);
	psy_ui_component_set_align(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_RIGHT);
	propertiesview_update_tabbar_sections(self);
	psy_signal_connect(&self->renderer.signal_changed, self,
		propertiesview_on_property_changed);
	psy_signal_connect(&self->renderer.signal_selected, self,
		propertiesview_on_selected);
	if (input_handler) {
		inputhandler_connect(input_handler, INPUTHANDLER_FOCUS,
			psy_EVENTDRIVER_CMD, "tracker", psy_INDEX_INVALID,
			self, (fp_inputhandler_input)propertiesview_on_input);
	}
	psy_signal_connect(&self->tabbar.signal_change, self,
		propertiesview_on_tabbar_change);
	psy_signal_connect(&self->scroller.pane.signal_beforealign, self,
		propertiesview_on_scroll_pane_align);
}

void propertiesview_on_destroy(PropertiesView* self)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_selected);
}

void propertiesview_select_section(PropertiesView* self,
	psy_ui_Component* sender, uintptr_t section, uintptr_t options)
{			
	psy_ui_tabbar_select(&self->tabbar, section);
}

void propertiesview_update_tabbar_sections(PropertiesView* self)
{	
	psy_ui_tabbar_clear(&self->tabbar);
	if (propertiesrenderer_properties(&self->renderer)) {
		const psy_List* p;
		
		for (p = psy_property_begin_const(propertiesrenderer_properties(
				&self->renderer)); p != NULL; p = p->next) {
			psy_Property* property;

			property = (psy_Property*)psy_list_entry_const(p);
			if (psy_property_issection(property)) {
				psy_ui_tabbar_append(&self->tabbar,
					psy_property_text(property),
				psy_INDEX_INVALID,
				psy_INDEX_INVALID, psy_INDEX_INVALID,
					psy_ui_colour_white());
			}
		}
	}	
	psy_ui_tabbar_select(&self->tabbar, 0);
}

void propertiesview_on_tabbar_change(PropertiesView* self,
	psy_ui_Component* sender, uintptr_t tabindex)
{		
	propertiesview_goto_section(self, tabindex);	
}

void propertiesview_goto_section(PropertiesView* self, uintptr_t index)
{
	psy_ui_Component* section;

	section = (psy_ui_Component*)psy_table_at(&self->renderer.sections, index);
	if (section) {
		psy_ui_RealRectangle position;
		
		position = psy_ui_component_position(section);
		psy_ui_component_set_scroll_top(&self->renderer.component,
			psy_ui_value_make_px(position.top));
	}
}

void propertiesview_on_property_changed(PropertiesView* self,
	PropertiesRenderer* sender, PropertiesRenderLine* line)
{	
	psy_Property* mainsection;
	uintptr_t rebuild_level;	
		
	mainsection = psy_property_parent_level(line->property, 1);	
	rebuild_level = psy_INDEX_INVALID;
	psy_signal_emit(&self->signal_changed, self, 2, line->property, &rebuild_level);
	if (rebuild_level == psy_INDEX_INVALID) {
		propertiesrenderer_update_line(&self->renderer, line);
	} else if (rebuild_level == 0) {
		propertiesview_reload(self);			
	} else if (mainsection) {
		propertiesrenderer_rebuild(&self->renderer, mainsection);		
		psy_ui_component_align_full(&self->component);
	}
}

void propertiesview_reload(PropertiesView* self)
{
	propertiesrenderer_build(&self->renderer);
	psy_ui_component_align_full(&self->component);
	psy_ui_component_invalidate(&self->component);
}

void propertiesview_select(PropertiesView* self, psy_Property* property)
{
	/*psy_ui_Component* line;
	psy_ui_Component* section;
	uintptr_t idx;

	line = NULL;	
	if (property) {		
		section = (psy_ui_Component*)psy_table_at(&self->renderer.sections, 0);
		if (section) {
			psy_ui_Component* clients;

			clients = psy_ui_component_at(section, 1);
			if (clients) {
				idx = psy_property_index(property);
				if (idx != psy_INDEX_INVALID) {
					line = psy_ui_component_at(clients, idx);
				}
			}
		}
	}
	if (self->renderer.state.selectedline) {
		psy_ui_component_removestylestate(
			&self->renderer.state.selectedline->component,
			psy_ui_STYLESTATE_SELECT);
	}
	self->renderer.state.selected = property;	
	self->renderer.state.selectedline = (PropertiesRenderLine*)line;
	if (property) {
		psy_signal_emit(&self->signal_selected, self, 1, property);
	}
	if (self->renderer.state.selectedline) {
		psy_ui_component_addstylestate(
			&self->renderer.state.selectedline->component,
			psy_ui_STYLESTATE_SELECT);
	}*/
}

void propertiesview_mark(PropertiesView* self, psy_Property* property)
{
	psy_signal_preventall(&self->signal_selected);
	propertiesview_select(self, property);
	psy_signal_enableall(&self->signal_selected);
}

psy_Property* propertiesview_selected(PropertiesView* self)
{
	return NULL; 
}

void propertiesview_enablemousepropagation(PropertiesView* self)
{
	self->renderer.state.prevent_mouse_propagation = FALSE;
}

void propertiesview_prevent_maximize_mainsections(PropertiesView* self)
{
	self->maximizemainsections = FALSE;
}

void propertiesview_on_selected(PropertiesView* self,
	PropertiesRenderer* sender, PropertiesRenderLine* selected)
{
	psy_signal_emit(&self->signal_selected, self, 1, selected->property);
}

bool propertiesview_on_input(PropertiesView* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;
	double step;
	double top;
	double newtop;	

	cmd = inputhandler_cmd(sender);
	if (cmd.id == -1) {
		return 0;
	}					
	step = psy_ui_component_scroll_step_height_px(
		&self->renderer.component);
	top = psy_ui_component_scroll_top_px(&self->renderer.component);
	newtop = -1.0;	
	switch (cmd.id) {
	case CMD_NAVTOP:
		newtop = 0.0;		
		break;
	case CMD_NAVBOTTOM:
		newtop = propertiesview_check_range(self, (double)INT32_MAX);
		break;
	case CMD_NAVUP:
		newtop = psy_max(0, top - step);
		break;
	case CMD_NAVDOWN:					
		newtop = propertiesview_check_range(self, top + step);
		break;
	case CMD_NAVPAGEUP:					
		newtop = psy_max(0, top - step * 16.0);
		break;
	case CMD_NAVPAGEDOWN:
		newtop = propertiesview_check_range(self, top + step * 16.0);
		break;
	default:
		break;
	}
	if (newtop != -1.0) {
		psy_ui_component_set_scroll_top(&self->renderer.component,
			psy_ui_value_make_px(psy_max(0.0, newtop)));
	}	
	return 1;
}

double propertiesview_check_range(PropertiesView* self, double position)
{
	intptr_t steps;
	double scrollstepypx;	
	psy_ui_IntPoint scrollrange;
	
	scrollrange = psy_ui_component_verticalscrollrange(
		&self->renderer.component);
	scrollstepypx = psy_ui_component_scroll_step_height_px(
		&self->renderer.component);
	steps = (intptr_t)(position / scrollstepypx);
	steps = psy_min(scrollrange.y, steps);
	return (double)(steps * scrollstepypx);
}

void propertiesview_on_mouse_down(PropertiesView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_set_focus(&self->component);
	if (self->renderer.state.prevent_mouse_propagation) {		
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void propertiesview_on_mouse_up(PropertiesView* self, psy_ui_MouseEvent* ev)
{
	if (self->renderer.state.prevent_mouse_propagation) {
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void propertiesview_on_scroll_pane_align(PropertiesView* self,
	psy_ui_Component* sender)
{
	assert(self);

	if (self->maximizemainsections) {
		psy_TableIterator it;
		psy_ui_Size clientsize;		

		clientsize = psy_ui_component_clientsize(&self->renderer.component);
		for (it = psy_table_begin(&self->renderer.sections);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)psy_tableiterator_value(&it);
			psy_ui_component_set_minimum_size(component,
				psy_ui_size_make(
					psy_ui_value_make_ew(0.0),
					clientsize.height));
		}
	}
}

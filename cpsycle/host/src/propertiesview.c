/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* host */
#include "propertiesview.h"
#include "trackergridstate.h" /* TRACKER CMDS */
#include "rangeedit.h"
#include "styles.h"
#include "zoombox.h"
/* ui */
#include <uicheckbox.h>
#include <uicolordialog.h>
#include <uifolderdialog.h>
#include <uifontdialog.h>
#include <uiswitch.h>
#include <uitextarea.h>
/* platform */
#include "../../detail/portable.h"


/* PropertiesRenderState */
void propertiesrenderstate_init(PropertiesRenderState* self, uintptr_t numcols, bool lazy)
{	
	assert(self);

	self->property_line_selected = NULL;	
	self->numcols = numcols;	
	self->mainsectionstyle = STYLE_PROPERTYVIEW_MAINSECTION;
	self->mainsectionheaderstyle = STYLE_PROPERTYVIEW_MAINSECTIONHEADER;
	self->keystyle = psy_INDEX_INVALID;
	self->keystyle_hover = psy_INDEX_INVALID;
	self->linestyle_select = psy_INDEX_INVALID;
	self->view = NULL;
	self->do_build = !lazy;
	psy_table_init(&self->sections);
	psy_ui_size_init_all(&self->size_col0, 		
		psy_ui_value_make_pw(0.4), 
		psy_ui_value_make_eh(1.3));
	psy_ui_size_init_all(&self->size_col2, 		
		psy_ui_value_make_pw(0.2),
		psy_ui_value_make_eh(1.3));
}

void propertiesrenderstate_dispose(PropertiesRenderState* self)
{
	assert(self);

	psy_table_dispose(&self->sections);	
}

/* PropertiesRenderLine */

/* prototypes */
static void propertiesrenderline_on_destroyed(PropertiesRenderLine*);
static void propertiesrenderline_build(PropertiesRenderLine*);
static void propertiesrenderline_on_mouse_down(PropertiesRenderLine*,
	psy_ui_MouseEvent*);
static void propertiesrenderline_on_input_definer_accept(PropertiesRenderLine*,
	InputDefiner* sender);
static void propertiesrenderline_on_font_dialog_button(PropertiesRenderLine*,
	psy_ui_Component* sender);
static void propertiesrenderline_on_colour_dialog_button(PropertiesRenderLine*,
	psy_ui_Component* sender);
static void propertiesrenderline_on_directory_dialog_button(PropertiesRenderLine*,
	psy_ui_Component* sender);
static void propertiesrenderline_on_edit_keydown(PropertiesRenderLine*,
	psy_ui_Component* sender, psy_ui_KeyboardEvent*);
static void propertiesrenderline_on_edit_reject(PropertiesRenderLine*,
	psy_ui_TextArea* sender);
static void propertiesrenderline_on_color_changed(PropertiesRenderLine*,
	psy_Property* sender);
static void propertiesrenderline_on_shortcut_changed(PropertiesRenderLine*,
	psy_Property* sender);
static void propertiesrenderline_on_rebuild(PropertiesRenderLine*,
	psy_Property* sender);
static void propertiesrenderline_before_property_destroyed(PropertiesRenderLine*,
	psy_Property* sender);

/* vtable */
static psy_ui_ComponentVtable propertiesrenderline_vtable;
static bool propertiesrenderline_vtable_initialized = FALSE;

static void propertiesrenderline_vtable_init(PropertiesRenderLine* self)
{
	if (!propertiesrenderline_vtable_initialized) {
		propertiesrenderline_vtable = *(self->component.vtable);
		propertiesrenderline_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			propertiesrenderline_on_destroyed;
		propertiesrenderline_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			propertiesrenderline_on_mouse_down;
		propertiesrenderline_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component,
		&propertiesrenderline_vtable);
}

/* implementation */
void propertiesrenderline_init(PropertiesRenderLine* self,
	psy_ui_Component* parent, PropertiesRenderState* state,
	psy_Property* property, uintptr_t level)
{	
	assert(property);

	psy_ui_component_init(&self->component, parent, NULL);
	propertiesrenderline_vtable_init(self);
	self->property = property;	
	self->input_definer = NULL;
	self->state = state;
	self->level = level;
	if (strcmp(psy_property_key(property), "range") == 0) {
		self = self;
	}
	psy_ui_component_set_align(&self->component, psy_ui_ALIGN_TOP);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());
	if (psy_property_hidden(self->property)) {
		psy_ui_component_hide(&self->component);
	}
	psy_ui_component_set_style_type(&self->component, self->state->keystyle);
	psy_ui_component_set_style_type_hover(&self->component,
		self->state->keystyle_hover);
	psy_ui_component_set_style_type_select(&self->component,
		self->state->linestyle_select);
	psy_signal_connect(&self->property->rebuild, self,
		propertiesrenderline_on_rebuild);
	psy_signal_connect(&self->property->before_destroyed, self,
		propertiesrenderline_before_property_destroyed);
	propertiesrenderline_build(self);
}

void propertiesrenderline_on_rebuild(PropertiesRenderLine* self, psy_Property* sender)
{
	
	psy_ui_component_clear(&self->component);
	propertiesrenderline_build(self);	
	if (self->state->view) {
		psy_ui_component_align(self->state->view);
		psy_ui_component_invalidate(self->state->view);
	}
}

void propertiesrenderline_build(PropertiesRenderLine* self)
{	
	psy_ui_Component* col0;
	psy_ui_Component* lines;

	self->colour = NULL;
	self->label = NULL;
	lines = &self->component;	

	if (self->level == 0) {

	} else if (psy_property_is_section(self->property) && psy_property_hint(self->property) != PSY_PROPERTY_HINT_RANGE) {
		if (self->level == 1) {
		psy_ui_Label* label;
		
				
		psy_ui_component_set_style_type(&self->component, self->state->mainsectionstyle);
		label = psy_ui_label_allocinit(&self->component);
		psy_ui_component_set_margin(psy_ui_label_base(label),
			psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));
		psy_ui_component_set_padding(psy_ui_label_base(label),
			psy_ui_margin_make_em(0.5, 0.0, 0.5, 1.0));		
			psy_ui_component_set_style_type(psy_ui_label_base(label),
				self->state->mainsectionheaderstyle);
			psy_ui_label_set_text(label, psy_property_text(self->property));
			lines = psy_ui_component_allocinit(&self->component, NULL);
			psy_ui_component_set_default_align(lines, psy_ui_ALIGN_TOP,
				psy_ui_margin_zero());
			psy_table_insert(&self->state->sections, psy_property_index(self->property),
				&self->component);
		}
	} else {		
		psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
		if (self->state->numcols == 1) {
			psy_ui_component_set_preferred_size(&self->component, self->state->size_col0);
		} else if (psy_property_hint(self->property) != PSY_PROPERTY_HINT_LIST) {
			psy_ui_component_set_preferred_size(&self->component,
				psy_ui_size_make_em(120.0, 1.5));
		}		
		/* column 0 */
		if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_CHECK) {
			psy_ui_CheckBox* check;

			check = psy_ui_checkbox_allocinit_exchange(&self->component,
				self->property);
			col0 = psy_ui_checkbox_base(check);
			psy_ui_checkbox_set_text(check, psy_property_text(self->property));
		} else {
			psy_ui_Label* key;

			key = psy_ui_label_allocinit(&self->component);
			col0 = psy_ui_label_base(key);
			if (!self->property->item.translate) {
				psy_ui_label_prevent_translation(key);
			}
			if (!psy_property_is_action(self->property)) {
				psy_ui_label_set_text(key, psy_property_text(self->property));
			}
		}		
		psy_ui_component_set_padding(col0,
			psy_ui_margin_make(psy_ui_value_zero(), psy_ui_value_zero(),
				psy_ui_value_zero(), psy_ui_value_make_ew(psy_min(self->level, 5.0) * 4.0)));
		psy_ui_component_set_preferred_size(col0, self->state->size_col0);
		
		if (self->state->numcols == 1) {
			psy_ui_component_set_align(col0, psy_ui_ALIGN_CLIENT);
			return;
		}
		psy_ui_component_set_align(col0, psy_ui_ALIGN_LEFT);
		/* column 1 */
		if (self->state->numcols > 1) {
			if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_RANGE) {				
				RangeEdit* range_edit;				
				
				range_edit = rangeedit_allocinit(&self->component);				
				psy_ui_component_set_align(&range_edit->component, psy_ui_ALIGN_LEFT);				
				rangeedit_data_exchange(range_edit, psy_property_at_index(self->property, 0),
					psy_property_at_index(self->property, 1));
			} else if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_COMBO) {
				psy_ui_ComboBox* combo;
				psy_List* p;

				combo = psy_ui_combobox_allocinit(&self->component);			
				psy_ui_combobox_set_char_number(combo, 50.0);
				psy_ui_component_set_align(&combo->component, psy_ui_ALIGN_LEFT);				
				p = psy_property_begin(self->property);
				for (; p != NULL; p = p->next) {
					psy_Property* property;

					property = (psy_Property*)p->entry;
					psy_ui_combobox_add_text(combo,
						(psy_property_translation_prevented(property))
						? psy_property_text(property)
						: psy_ui_translate(psy_property_text(property)));
				}
				psy_ui_combobox_data_exchange(combo, self->property);
			} else if (psy_property_is_bool(self->property) || psy_property_is_choice_item(
					self->property)) {
				if (psy_property_hint(self->property) != PSY_PROPERTY_HINT_CHECK) {
					psy_ui_Switch* check;

					check = psy_ui_switch_allocinit_exchange(&self->component,
						self->property);
					psy_ui_component_set_align(&check->component, psy_ui_ALIGN_CLIENT);
				}
			} else if (psy_property_is_action(self->property)) {
				psy_ui_Button* button;

				button = psy_ui_button_allocinit(&self->component);
				psy_ui_button_data_exchange(button, self->property);
				psy_ui_component_set_align(psy_ui_button_base(button), psy_ui_ALIGN_LEFT);			
			} else if (psy_property_is_font(self->property)) {
				self->label = psy_ui_label_allocinit(&self->component);
				psy_ui_label_prevent_translation(self->label);
				psy_ui_label_data_exchange(self->label, self->property);
				psy_ui_component_set_align(psy_ui_label_base(self->label),
					psy_ui_ALIGN_CLIENT);			
			} else if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {
				psy_ui_Component* col1;

				col1 = psy_ui_component_allocinit(&self->component, NULL);
				psy_ui_component_set_align(col1, psy_ui_ALIGN_CLIENT);
				psy_ui_component_set_align_expand(col1, psy_ui_HEXPAND);
				self->label = psy_ui_label_allocinit(col1);
				psy_ui_label_prevent_translation(self->label);
				psy_ui_component_set_align(psy_ui_label_base(self->label),
					psy_ui_ALIGN_LEFT);
				psy_ui_label_set_char_number(self->label, 20.0);
				self->colour = psy_ui_component_allocinit(col1, NULL);
				psy_ui_component_set_align(self->colour, psy_ui_ALIGN_LEFT);
				psy_ui_component_set_preferred_size(self->colour,
					psy_ui_size_make_em(4.0, 1.0));
				psy_ui_component_set_align(col1, psy_ui_ALIGN_CLIENT);
				psy_ui_label_data_exchange(self->label, self->property);
				psy_property_connect(self->property, self,
					propertiesrenderline_on_color_changed);
				propertiesrenderline_on_color_changed(self, self->property);
			} else if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_SHORTCUT) {
				self->input_definer = inputdefiner_allocinit(&self->component);
				psy_ui_component_set_align(inputdefiner_base(self->input_definer),
					psy_ui_ALIGN_CLIENT);
				psy_signal_connect(&self->input_definer->signal_accept, self,
					propertiesrenderline_on_input_definer_accept);	
				psy_property_connect(self->property, self,
					propertiesrenderline_on_shortcut_changed);
				propertiesrenderline_on_shortcut_changed(self, self->property);
			} else if (
					psy_property_is_int(self->property) ||
					psy_property_is_double(self->property) ||
					psy_property_is_string(self->property)) {
				if (psy_property_readonly(self->property)) {
					self->label = psy_ui_label_allocinit(&self->component);
					psy_ui_label_prevent_translation(self->label);				
					psy_ui_label_data_exchange(self->label, self->property);				
					psy_ui_component_set_align(psy_ui_label_base(self->label),
						psy_ui_ALIGN_CLIENT);
				} else {
					if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_ZOOM) {
						ZoomBox* zoom;
					
						zoom = zoombox_allocinit_exchange(&self->component,
							self->property);
						psy_ui_component_set_align(zoombox_base(zoom),
							psy_ui_ALIGN_LEFT);					
					} else {
						psy_ui_TextArea* edit;
					
						edit = psy_ui_textarea_allocinit_single_line(&self->component);
						psy_ui_component_set_align(psy_ui_textarea_base(edit),
							psy_ui_ALIGN_CLIENT);
						psy_ui_textarea_enable_input_field(edit);										
						psy_signal_connect(&edit->signal_reject, self,
							propertiesrenderline_on_edit_reject);
						psy_signal_connect(&edit->component.signal_keydown, self,
							propertiesrenderline_on_edit_keydown);
						psy_ui_textarea_data_exchange(edit, self->property);					
					}
				}						
			}		
		}
		/* column 2 */
		if (self->state->numcols > 2) {
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
				}
				else {
					psy_signal_connect(&dialogbutton->signal_clicked, self,
						propertiesrenderline_on_directory_dialog_button);
				}
				col2 = &dialogbutton->component;
			}
			else if (psy_property_is_font(self->property)) {
				dialogbutton = psy_ui_button_allocinit(&self->component);
				col2 = &dialogbutton->component;
				psy_ui_button_set_text(dialogbutton, "settings.choose-font");
				psy_signal_connect(&dialogbutton->signal_clicked, self,
					propertiesrenderline_on_font_dialog_button);
			}
			else if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_SHORTCUT) {
				dialogbutton = psy_ui_button_allocinit(&self->component);
				col2 = &dialogbutton->component;
				psy_ui_button_set_text(dialogbutton, "settings.none");
			}
			else if (psy_property_int_hasrange(self->property) &&
				!psy_property_readonly(self->property)) {
				char text[256];
				psy_ui_Label* label;

				/*
				** todo: psy_ui_translate translates only at build, use
				** onlanguageupdate to handle settingsview changes
				*/
				psy_snprintf(text, 256, "%s %d %s %d",
					psy_ui_translate("settings.from"),
					self->property->item.min,
					psy_ui_translate("settings.to"),
					self->property->item.max);
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
	}	
	if ((psy_property_hint(self->property) != PSY_PROPERTY_HINT_COMBO) &&
			(psy_property_hint(self->property) != PSY_PROPERTY_HINT_RANGE)) {
		psy_List* p;
		psy_ui_Component* list;

		list = psy_ui_component_allocinit(lines, NULL);
		psy_ui_component_set_align(list, psy_ui_ALIGN_TOP);
		for (p = psy_property_begin(self->property); p != NULL; p = p->next) {
			psy_Property* curr;

			curr = (psy_Property*)p->entry;			
			propertiesrenderline_allocinit(list, self->state, curr,
				self->level + 1);			
		}
	}
}

void propertiesrenderline_on_destroyed(PropertiesRenderLine* self)
{
	assert(self);

	if (self->property) {
		if (self->property->item.id == 200) {
			self = self;
		}
		psy_property_disconnect(self->property, self);
	}
}

void propertiesrenderline_before_property_destroyed(PropertiesRenderLine* self,
	psy_Property* sender)
{
	assert(self);

	self->property = NULL;
}

PropertiesRenderLine* propertiesrenderline_alloc(void)
{
	return (PropertiesRenderLine*)malloc(sizeof(PropertiesRenderLine));
}

PropertiesRenderLine* propertiesrenderline_allocinit(psy_ui_Component* parent,
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
	self->state->property_line_selected = self;
	psy_ui_mouseevent_stop_propagation(ev);		
}

void propertiesrenderline_on_color_changed(PropertiesRenderLine* self,
	psy_Property* sender)
{
	if (self->colour) {
		psy_ui_component_set_background_colour(self->colour,
			psy_ui_colour_make((uint32_t)psy_property_item_int(
				self->property)));
	}
}

void propertiesrenderline_on_shortcut_changed(PropertiesRenderLine* self,
	psy_Property* sender)
{
	if (self->input_definer) {
		inputdefiner_set_input(self->input_definer,
			(uint32_t)psy_property_item_int(self->property));
	}
}

void propertiesrenderline_on_input_definer_accept(PropertiesRenderLine* self,
	InputDefiner* sender)
{
	if (self->property && self->input_definer) {
		psy_property_set_item_int(self->property,
			inputdefiner_input(self->input_definer));
		psy_ui_component_set_focus(psy_ui_component_parent(&self->component));		
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
		psy_property_set_item_font(self->property, fontstr);		
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
		psy_property_set_item_int(self->property,
			psy_ui_colour_colorref(&colour));		
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
		psy_property_set_item_str(self->property,
			psy_ui_folderdialog_path(&dialog));
		psy_ui_folderdialog_dispose(&dialog);		
	}	
}

void propertiesrenderline_on_edit_keydown(PropertiesRenderLine* self,
	psy_ui_Component* sender, psy_ui_KeyboardEvent* ev)
{
	if (psy_property_is_hex(self->property)) {
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
static void propertiesrenderer_on_destroyed(PropertiesRenderer*);
static void propertiesrenderer_on_timer(PropertiesRenderer*, uintptr_t timerid);
static void propertiesview_goto_section(PropertiesView*, uintptr_t index);

/* vtable */
static psy_ui_ComponentVtable propertiesrenderer_vtable;
static bool propertiesrenderer_vtable_initialized = FALSE;

static void propertiesrenderer_vtable_init(PropertiesRenderer* self)
{
	if (!propertiesrenderer_vtable_initialized) {
		propertiesrenderer_vtable = *(self->component.vtable);
		propertiesrenderer_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			propertiesrenderer_on_destroyed;
		propertiesrenderer_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			propertiesrenderer_on_timer;			
		propertiesrenderer_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &propertiesrenderer_vtable);
}

/* implementation */
void propertiesrenderer_init(PropertiesRenderer* self, psy_ui_Component* parent,
	psy_Property* properties, uintptr_t numcols, bool lazy)
{
	self->properties = properties;		
	psy_ui_component_init(&self->component, parent, NULL);
	propertiesrenderer_vtable_init(self);	
	psy_ui_component_set_wheel_scroll(&self->component, 4);	
	psy_ui_component_init(&self->client, &self->component, NULL);	
	psy_ui_component_set_align(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_default_align(&self->client,
		psy_ui_ALIGN_TOP, psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));	
	/* state */
	propertiesrenderstate_init(&self->state, numcols, lazy);
	self->state.view = parent;
	psy_signal_init(&self->signal_selected);	
	psy_ui_component_set_overflow(&self->component, psy_ui_OVERFLOW_VSCROLL);	
	psy_ui_component_setscrollstep(&self->component,
		psy_ui_size_make_em(0.0, 4.0));	
	propertiesrenderer_build(self);	
	psy_ui_component_start_timer(&self->component, 0, 100);
}

void propertiesrenderer_on_destroyed(PropertiesRenderer* self)
{
	psy_signal_dispose(&self->signal_selected);	
	propertiesrenderstate_dispose(&self->state);
}

void propertiesrenderer_set_style(PropertiesRenderer* self,
	uintptr_t mainsection,
	uintptr_t mainsectionheader,
	uintptr_t keystyle,
	uintptr_t keystyle_hover,
	uintptr_t linestyle_select)
{
	self->state.mainsectionstyle = mainsection;
	self->state.mainsectionheaderstyle = mainsectionheader;
	self->state.keystyle = keystyle;
	self->state.keystyle_hover = keystyle_hover;
	self->state.linestyle_select = linestyle_select;
	propertiesrenderer_build(self);
}

void propertiesrenderer_build(PropertiesRenderer* self)
{		
	assert(self);
	
	if (self->state.do_build) {
		psy_table_clear(&self->state.sections);
		psy_ui_component_clear(&self->client);
		if (self->properties) {
			propertiesrenderline_allocinit(&self->client, &self->state,
				self->properties, 0);
		}
	}
}

void propertiesrenderer_maximize_sections(PropertiesRenderer* self)
{
	assert(self);
	
	psy_TableIterator it;
	psy_ui_Size clientsize;

	clientsize = psy_ui_component_client_size(&self->component);
	for (it = psy_table_begin(&self->state.sections);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)psy_tableiterator_value(&it);
		psy_ui_component_set_minimum_size(component, psy_ui_size_make(
			psy_ui_value_zero(), clientsize.height));
	}	
}

void propertiesrenderer_on_timer(PropertiesRenderer* self, uintptr_t timerid)
{
	PropertiesRenderLine* line;

	line = self->state.property_line_selected;
	if (line) {
		self->state.property_line_selected = NULL;
		psy_signal_emit(&self->signal_selected, self, 1, line);
	}	
}

/* PropertiesView */

/* prototypes */
static void propertiesview_on_destroyed(PropertiesView*);
static void propertiesview_select_section(PropertiesView*,
	psy_ui_Component* sender, uintptr_t section, uintptr_t options);
static void propertiesview_update_tabbar_sections(PropertiesView*);
static void propertiesview_on_tabbar_change(PropertiesView*,
	psy_ui_Component* sender, uintptr_t tabindex);
static void propertiesview_on_selected(PropertiesView*,
	PropertiesRenderer* sender, PropertiesRenderLine*);
static bool propertiesview_on_input(PropertiesView*, InputHandler* sender);
static void propertiesview_on_mouse_down(PropertiesView*, psy_ui_MouseEvent*);
static void propertiesview_on_mouse_up(PropertiesView*, psy_ui_MouseEvent*);
static void propertiesview_on_scroll_pane_align(PropertiesView*,
	psy_ui_Component* sender);
static void propertiesview_on_show(PropertiesView*);

/* vtable */
static psy_ui_ComponentVtable propertiesview_vtable;
static bool propertiesview_vtable_initialized = FALSE;

static void propertiesview_vtable_init(PropertiesView* self)
{
	if (!propertiesview_vtable_initialized) {
		propertiesview_vtable = *(self->component.vtable);
		propertiesview_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			propertiesview_on_destroyed;
		propertiesview_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			propertiesview_on_mouse_down;
		propertiesview_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			propertiesview_on_mouse_up;
		propertiesview_vtable.show =
			(psy_ui_fp_component_event)
			propertiesview_on_show;
	}
	psy_ui_component_set_vtable(propertiesview_base(self),
		&propertiesview_vtable);
}

/* implementation */
void propertiesview_init(PropertiesView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_Property* properties,
	uintptr_t numcols, bool lazy, InputHandler* input_handler)
{
	psy_ui_component_init(&self->component, parent, NULL);
	propertiesview_vtable_init(self);
	psy_ui_component_set_id(&self->component, VIEW_ID_SONGPROPERTIES);
	psy_ui_component_set_tab_index(&self->component, 0);	
	self->maximize_main_sections = TRUE;	
	psy_signal_init(&self->signal_selected);	
	psy_ui_component_init(&self->viewtabbar, tabbarparent, NULL);	
	propertiesrenderer_init(&self->renderer, &self->component, properties,
		numcols, lazy);
	psy_ui_scroller_init(&self->scroller, &self->component, NULL, NULL);
	self->scroller.prevent_mouse_down_propagation = FALSE;
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
	psy_ui_tabbar_set_tab_align(&self->tabbar, psy_ui_ALIGN_TOP);
	psy_ui_component_set_align(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_RIGHT);
	propertiesview_update_tabbar_sections(self);	
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

void propertiesview_on_destroyed(PropertiesView* self)
{	
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
			if (psy_property_is_section(property)) {								
				psy_ui_tabbar_append(&self->tabbar,
					psy_property_text(property),
				psy_INDEX_INVALID,
					property->item.icon_light_id, property->item.icon_dark_id,
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

	section = (psy_ui_Component*)psy_table_at(&self->renderer.state.sections,
		index);
	if (section) {
		psy_ui_RealRectangle position;
		
		position = psy_ui_component_position(section);
		psy_ui_component_set_scroll_top(&self->renderer.component,
			psy_ui_value_make_px(position.top));
		psy_ui_component_invalidate(&self->component);
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
	/* todo */
}

void propertiesview_mark(PropertiesView* self, psy_Property* property)
{
	psy_signal_prevent_all(&self->signal_selected);
	propertiesview_select(self, property);
	psy_signal_enable_all(&self->signal_selected);
}

psy_Property* propertiesview_selected(PropertiesView* self)
{
	return NULL; 
}

void propertiesview_prevent_maximize_main_sections(PropertiesView* self)
{
	self->maximize_main_sections = FALSE;
}

void propertiesview_on_selected(PropertiesView* self,
	PropertiesRenderer* sender, PropertiesRenderLine* selected)
{
	psy_signal_emit(&self->signal_selected, self, 1,
		selected->property);
}

bool propertiesview_on_input(PropertiesView* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;
	double step;
	double top;	

	cmd = inputhandler_cmd(sender);
	if (cmd.id == -1) {
		return INPUTHANDLER_CONTINUE;
	}					
	step = psy_ui_component_scroll_step_height_px(
		propertiesrenderer_base(&self->renderer));
	top = psy_ui_component_scroll_top_px(propertiesrenderer_base(
		&self->renderer));
	switch (cmd.id) {
	case CMD_NAVTOP:
		top = 0.0;		
		break;
	case CMD_NAVBOTTOM:
		top = (double)INT32_MAX;
		break;
	case CMD_NAVUP:
		top = psy_max(0, top - step);
		break;
	case CMD_NAVDOWN:					
		top = top + step;
		break;
	case CMD_NAVPAGEUP:					
		top = psy_max(0, top - step * 16.0);		
		break;
	case CMD_NAVPAGEDOWN:
		top = top + step * 16.0;
		break;
	default:
		top = -1.0;
		break;
	}
	if (top != -1.0) {
		psy_ui_component_set_scroll_top(propertiesrenderer_base(
			&self->renderer), psy_ui_value_make_px(psy_max(0.0, top)));
	}	
	return INPUTHANDLER_STOP;
}

void propertiesview_on_mouse_down(PropertiesView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_set_focus(&self->component);	
	psy_ui_mouseevent_stop_propagation(ev);	
}

void propertiesview_on_mouse_up(PropertiesView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_mouseevent_stop_propagation(ev);	
}

void propertiesview_on_scroll_pane_align(PropertiesView* self,
	psy_ui_Component* sender)
{
	assert(self);

	if (self->maximize_main_sections) {
		propertiesrenderer_maximize_sections(&self->renderer);
	}
}

void propertiesview_on_show(PropertiesView* self)
{
	if (!self->renderer.state.do_build) {
		self->renderer.state.do_build = TRUE;
		propertiesrenderer_build(&self->renderer);
	}
}

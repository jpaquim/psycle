/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "propertiesview.h"
/* host */
#include "colourbox.h"
#include "fileedit.h"
#include "fontbox.h"
#include "rangeedit.h"
#include "styles.h"
#include "trackergridstate.h" /* TRACKER CMDS */
#include "zoombox.h"
/* ui */
#include <uicheckbox.h>
#include <uiswitch.h>
#include <uitextarea.h>
#include <uiopendialog.h>
/* platform */
#include "../../detail/os.h"


/* PropertiesFileBar */

/* prototypes */
static void propertiesfilebar_on_ok(PropertiesFileBar* self,
	psy_ui_Component* sender);
static void propertiesfilebar_on_cancel(PropertiesFileBar* self,
	psy_ui_Component* sender);
	
/* implementation */
void propertiesfilebar_init(PropertiesFileBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->notebook = NULL;
	self->filebox = NULL;
	psy_ui_component_set_style_type(&self->component,
		STYLE_NEWMACHINE_RESCANBAR);
	psy_ui_component_set_default_align(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_button_init_text_connect(&self->cancel, &self->component,
		"Cancel", self, propertiesfilebar_on_cancel);		
	psy_ui_component_set_align(psy_ui_button_base(&self->cancel),
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_preferred_width(psy_ui_button_base(
		&self->cancel), psy_ui_value_make_ew(15.0));
	psy_ui_button_init_text_connect(&self->add, &self->component,
		"OK", self, propertiesfilebar_on_ok);
	psy_ui_component_set_preferred_width(psy_ui_button_base(&self->add),
		psy_ui_value_make_ew(15.0));		
	psy_ui_component_set_align(psy_ui_button_base(&self->add),
		psy_ui_ALIGN_RIGHT);	
}

void propertiesfilebar_on_cancel(PropertiesFileBar* self, psy_ui_Component* sender)
{
	if (self->notebook) {
		psy_ui_notebook_select(self->notebook, 1);
	}
}

void propertiesfilebar_on_ok(PropertiesFileBar* self,
	psy_ui_Component* sender)
{
	if (self->filebox && self->filebox->property) {
		psy_property_set_item_str(self->filebox->property,
			filebox_directory(self->filebox));
	}
	if (self->notebook) {
		psy_ui_notebook_select(self->notebook, 1);
	}
}

/* implementation */
void propertiesdirbar_init(PropertiesDirBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->notebook = NULL;
	self->filebox = NULL;	
	psy_ui_component_set_style_type(&self->component, STYLE_FILEVIEW_DIRBAR);
	psy_ui_label_init(&self->label, &self->component);
	psy_ui_component_set_align(&self->label.component, psy_ui_ALIGN_TOP);
	psy_ui_component_set_padding(&self->label.component,
		psy_ui_margin_make_em(0.25, 0.0, 0.25, 2.0));
	
}

void propertiesdirbar_set_text(PropertiesDirBar* self, const char* text)
{
	psy_ui_label_set_text(&self->label, text);
}


/* PropertiesRenderState */
void propertiesrenderstate_init(PropertiesRenderState* self, uintptr_t numcols,
	bool lazy)
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
	self->renderer = NULL;
	self->do_build = !lazy;
	self->filebox = NULL;
	self->notebook = NULL;
	psy_ui_size_init_all(&self->size_col0, psy_ui_value_make_pw(0.4),
		psy_ui_value_make_eh(1.5));	
}

void propertiesrenderstate_dispose(PropertiesRenderState* self)
{
	assert(self);	
}

/* PropertiesRenderLine */

/* prototypes */
static void propertiesrenderline_on_destroyed(PropertiesRenderLine*);
static void propertiesrenderline_before_property_destroyed(
	PropertiesRenderLine*, psy_Property* sender);
static void propertiesrenderline_build(PropertiesRenderLine*);
static psy_ui_Component* propertiesrenderline_build_section(
	PropertiesRenderLine*, uintptr_t section_style_type,
	uintptr_t header_style_type);
static void propertiesrenderline_build_children(PropertiesRenderLine*,
	psy_ui_Component* parent);
static void propertiesrenderline_on_mouse_down(PropertiesRenderLine*,
	psy_ui_MouseEvent*);
static void propertiesrenderline_on_edit_keydown(PropertiesRenderLine*,
	psy_ui_Component* sender, psy_ui_KeyboardEvent*);
static void propertiesrenderline_on_edit_reject(PropertiesRenderLine*,
	psy_ui_TextArea* sender);
static void propertiesrenderline_on_rebuild(PropertiesRenderLine*,
	psy_Property* sender);
static void propertiesrenderline_on_scrollto(PropertiesRenderLine*,
	psy_Property* sender);
static void propertiesrenderline_on_file_request(PropertiesRenderLine*,
	psy_Property* sender, const char* wildcard);
static void propertiesrenderline_on_file_accept(PropertiesRenderLine*,
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
	self->state = state;
	self->level = level;	
	psy_ui_component_set_align(&self->component, psy_ui_ALIGN_TOP);	
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
	psy_signal_connect(&self->property->scrollto, self,
		propertiesrenderline_on_scrollto);
	psy_signal_connect(&self->property->before_destroyed, self,
		propertiesrenderline_before_property_destroyed);
	propertiesrenderline_build(self);
}

void propertiesrenderline_on_rebuild(PropertiesRenderLine* self,
	psy_Property* sender)
{
	
	psy_ui_component_clear(&self->component);
	propertiesrenderline_build(self);	
	if (self->state->view) {
		psy_ui_component_align(self->state->view);
		psy_ui_component_invalidate(self->state->view);
	}
}

void propertiesrenderline_on_scrollto(PropertiesRenderLine* self,
	psy_Property* sender)
{
	if (self->state->renderer) {
		psy_ui_RealRectangle position;
		psy_ui_RealRectangle paneposition;

		position = psy_ui_component_screenposition(&self->component);
		paneposition = psy_ui_component_screenposition(self->state->renderer);
		psy_ui_component_set_scroll_top(self->state->renderer,
			psy_ui_value_make_px(position.top - paneposition.top));
		psy_ui_component_invalidate(&self->component);
	}
}

void propertiesrenderline_build(PropertiesRenderLine* self)
{	
	psy_ui_Component* col0;
	psy_ui_Component* lines;
	
	lines = &self->component;
	if (self->level != 0) {
		if (psy_property_is_section(self->property) &&
				psy_property_hint(self->property) != PSY_PROPERTY_HINT_RANGE) {
			if (self->level == 1) {				
				lines = propertiesrenderline_build_section(self,
					self->state->mainsectionstyle,
					self->state->mainsectionheaderstyle);
			} else if (self->level == 2) {
				lines = propertiesrenderline_build_section(self,
					STYLE_PROPERTYVIEW_SUBSECTION,
					STYLE_PROPERTYVIEW_SUBSECTIONHEADER);
			}
		} else {
			psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
			if (self->state->numcols == 1) {
				psy_ui_component_set_preferred_size(&self->component,
					self->state->size_col0);
			}
			else if (psy_property_hint(self->property) != PSY_PROPERTY_HINT_LIST) {
				psy_ui_component_set_preferred_size(&self->component,
					psy_ui_size_make_em(120.0, 1.5));
			}
			/* column 0 */
			if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_CHECK) {
				psy_ui_CheckBox* check;

				check = psy_ui_checkbox_allocinit_exchange(&self->component,
					self->property);
				col0 = psy_ui_checkbox_base(check);
				if (!self->property->item.translate) {
					psy_ui_label_prevent_translation(&check->text);
				}
				psy_ui_checkbox_set_text(check, psy_property_text(self->property));
			}
			else {
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

					range_edit = rangeedit_allocinit_range(&self->component,
						psy_property_at_index(self->property, 0),
						psy_property_at_index(self->property, 1));
					psy_ui_component_set_align(&range_edit->component, psy_ui_ALIGN_LEFT);
				}
				else if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_COMBO) {
					psy_ui_ComboBox* combo;					

					combo = psy_ui_combobox_allocinit(&self->component);
					psy_ui_combobox_set_char_number(combo, 50.0);
					psy_ui_component_set_align(&combo->component, psy_ui_ALIGN_LEFT);					
					psy_ui_combobox_data_exchange(combo, self->property);
				}
				else if (psy_property_is_bool(self->property) || psy_property_is_choice_item(
					self->property)) {
					if (psy_property_hint(self->property) != PSY_PROPERTY_HINT_CHECK) {
						psy_ui_Switch* check;

						check = psy_ui_switch_allocinit_exchange(&self->component,
							self->property);
						psy_ui_component_set_align(&check->component, psy_ui_ALIGN_LEFT);
					}
				}
				else if (psy_property_is_action(self->property)) {
					psy_ui_Button* button;

					button = psy_ui_button_allocinit(&self->component);
					psy_ui_button_data_exchange(button, self->property);
					psy_ui_component_set_align(psy_ui_button_base(button), psy_ui_ALIGN_LEFT);
					psy_signal_connect(&self->property->signal_file_request,
						self, propertiesrenderline_on_file_request);
					psy_signal_connect(&self->property->signal_file_accept,
						self, propertiesrenderline_on_file_accept);
				}
				else if (psy_property_is_font(self->property)) {
					FontBox* fontbox;

					fontbox = fontbox_allocinit(&self->component);
					fontbox_data_exchange(fontbox, self->property);
					psy_ui_component_set_align(fontbox_base(fontbox), psy_ui_ALIGN_CLIENT);
				}
				else if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {
					ColourBox* colourbox;

					colourbox = colourbox_allocinit(&self->component);
					colourbox_data_exchange(colourbox, self->property);
					psy_ui_component_set_align(colourbox_base(colourbox), psy_ui_ALIGN_CLIENT);
				}
				else if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_SHORTCUT) {
					InputDefiner* input_definer;

					input_definer = inputdefiner_allocinit(&self->component);
					inputdefiner_data_exchange(input_definer, self->property);
					psy_ui_component_set_align(inputdefiner_base(input_definer),
						psy_ui_ALIGN_CLIENT);
				}
				else if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITDIR) {
					FileEdit* fileedit;

					fileedit = fileedit_allocinit(&self->component);
					fileedit_set_file_box(fileedit, self->state->filebox);
					fileedit_data_exchange(fileedit, self->property);
					psy_ui_component_set_align(fileedit_base(fileedit),
						psy_ui_ALIGN_CLIENT);
				}
				else if (psy_property_is_int(self->property) ||
					psy_property_is_double(self->property) ||
					psy_property_is_string(self->property)) {
					if (psy_property_readonly(self->property)) {
						psy_ui_Label* label;

						label = psy_ui_label_allocinit(&self->component);
						psy_ui_label_prevent_translation(label);
						psy_ui_label_data_exchange(label, self->property,
							psy_ui_PROPERTY_MODE_VALUE);
						psy_ui_component_set_align(psy_ui_label_base(label),
							psy_ui_ALIGN_CLIENT);
					}
					else {
						if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_ZOOM) {
							ZoomBox* zoom;

							zoom = zoombox_allocinit_exchange(&self->component,
								self->property);
							psy_ui_component_set_align(zoombox_base(zoom),
								psy_ui_ALIGN_LEFT);
						}
						else {
							psy_ui_TextArea* edit;

							edit = psy_ui_textarea_allocinit_single_line(&self->component);
							if (psy_property_readonly(self->property)) {
								psy_ui_textarea_prevent(edit);
							}
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
		}
		/* column 2 */
		if (self->state->numcols > 2) {
			if (!psy_property_is_choice_item(self->property) &&
				psy_property_int_has_range(self->property) &&
				!psy_property_readonly(self->property)) {				
				psy_ui_Label* label;
				
				label = psy_ui_label_allocinit(&self->component);				
				psy_ui_label_data_exchange(label, self->property,
					psy_ui_PROPERTY_MODE_RANGE);
				psy_ui_component_set_align(psy_ui_label_base(label),
					psy_ui_ALIGN_RIGHT);	
			}
		}				
	}
	if ((psy_property_hint(self->property) != PSY_PROPERTY_HINT_COMBO) &&
			(psy_property_hint(self->property) != PSY_PROPERTY_HINT_RANGE)) {
		propertiesrenderline_build_children(self, lines);		
	}
}

psy_ui_Component* propertiesrenderline_build_section(PropertiesRenderLine* self,
	uintptr_t section_style, uintptr_t section_header_style)
{
	psy_ui_Label* label;
	psy_ui_Component* rv;

	assert(self);

	psy_ui_component_set_style_type(&self->component, section_style);
	label = psy_ui_label_allocinit(&self->component);
	psy_ui_component_set_align(psy_ui_label_base(label), psy_ui_ALIGN_TOP);
	psy_ui_component_set_style_type(psy_ui_label_base(label),
		section_header_style);
	psy_ui_label_set_text(label, psy_property_text(self->property));
	rv = psy_ui_component_allocinit(&self->component, NULL);
	psy_ui_component_set_align(rv, psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(rv, psy_ui_margin_make_em(0.0, 1.0, 0.0, 2.0));
	psy_ui_component_set_default_align(rv, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());
	return rv;
}

void propertiesrenderline_build_children(PropertiesRenderLine* self,
	psy_ui_Component* parent)
{
	psy_List* p;
	psy_ui_Component* list;

	assert(self);

	list = psy_ui_component_allocinit(parent, NULL);
	psy_ui_component_set_align(list, psy_ui_ALIGN_TOP);
	for (p = psy_property_begin(self->property); p != NULL; p = p->next) {
		psy_Property* curr;

		curr = (psy_Property*)p->entry;
		propertiesrenderline_allocinit(list, self->state, curr,
			self->level + 1);
	}
}

void propertiesrenderline_on_destroyed(PropertiesRenderLine* self)
{
	assert(self);

	if (self->property) {		
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

void propertiesrenderline_on_file_request(PropertiesRenderLine* self,
	psy_Property* sender, const char* wildcard)
{
	assert(self);
	
	if (self->state->filebox && self->state->notebook) {
		self->state->filebox->dirsonly = FALSE;
		self->state->filebox->property = sender;
		filebox_set_wildcard(self->state->filebox, wildcard);
		psy_ui_notebook_select(self->state->notebook, 0);
	} else {
		psy_ui_OpenDialog opendialog;

		psy_ui_opendialog_init_all(&opendialog, 0,
			"Load", wildcard, "", "");
		if (psy_ui_opendialog_execute(&opendialog)) {
			psy_signal_emit(&sender->signal_file_accept, sender, 1,
				psy_path_full(psy_ui_opendialog_path(&opendialog)));			
		}
		psy_ui_opendialog_dispose(&opendialog);
	}
}

void propertiesrenderline_on_file_accept(PropertiesRenderLine* self,
	psy_Property* sender)
{
	assert(self);
	
	if (self->state->notebook) {
		psy_ui_notebook_select(self->state->notebook, 1);
	}
}

/* PropertiesRenderer */

/* prototypes */
static void propertiesrenderer_on_destroyed(PropertiesRenderer*);
static void propertiesrenderer_on_timer(PropertiesRenderer*, uintptr_t timerid);

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
	psy_Property* properties, uintptr_t numcols, bool lazy,
	FileBox* filebox)
{
	self->properties = properties;		
	psy_ui_component_init(&self->component, parent, NULL);
	propertiesrenderer_vtable_init(self);	
	psy_ui_component_set_wheel_scroll(&self->component, 4);	
	psy_ui_component_init(&self->client, &self->component, NULL);	
	psy_ui_component_set_align(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_default_align(&self->client, psy_ui_ALIGN_TOP,
		psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));	
	/* state */
	propertiesrenderstate_init(&self->state, numcols, lazy);
	self->state.view = parent;
	self->state.renderer = propertiesrenderer_base(self);
#if PSYCLE_USE_TK == PSYCLE_TK_X11	
	self->state.filebox = filebox;
#endif	
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
			
	psy_ui_Component* root;	

	root = psy_ui_component_at(&self->client, 0);
	if (root) {
		root = psy_ui_component_at(root, 0);
		if (root) {
			psy_ui_Size clientsize;
			psy_List* p;
			psy_List* q;

			clientsize = psy_ui_component_client_size(&self->component);
			q = psy_ui_component_children(root, psy_ui_NONE_RECURSIVE);
			for (p = q; p != NULL; p = p->next) {
				psy_ui_Component* component;

				component = (psy_ui_Component*)p->entry;
				psy_ui_component_set_minimum_size(component, psy_ui_size_make(
					psy_ui_value_zero(), clientsize.height));

			}
			psy_list_free(q);
			q = NULL;
		}
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
static void propertiesview_scroll_to(PropertiesView*, const char* key);
static void propertiesview_on_dir_changed(PropertiesView*, FileBox* sender);
static void propertiesview_on_file_selected(PropertiesView*, FileBox* sender);

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
	psy_ui_component_set_id(&self->component, VIEW_ID_SETTINGSVIEW);
	psy_ui_component_set_tab_index(&self->component, 0);	
	self->maximize_main_sections = TRUE;
	psy_signal_init(&self->signal_selected);		
	if (tabbarparent) {
		psy_ui_component_init(&self->viewtabbar, tabbarparent, NULL);
		psy_ui_component_set_id(&self->viewtabbar, VIEW_ID_SETTINGSVIEW);	
	}
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_set_align(&self->notebook.component, psy_ui_ALIGN_CLIENT);
	/* filebox */
	psy_ui_component_init(&self->fileview, &self->notebook.component, NULL);	
	filebox_init(&self->filebox, &self->fileview);
	psy_signal_connect(&self->filebox.signal_dir_changed, self,
		propertiesview_on_dir_changed);
	psy_signal_connect(&self->filebox.signal_selected, self,
		propertiesview_on_file_selected);
	psy_ui_component_set_align(&self->filebox.component, psy_ui_ALIGN_CLIENT);
	propertiesfilebar_init(&self->filebar, &self->fileview);
	propertiesdirbar_init(&self->dirbar, &self->fileview);
	psy_ui_component_set_align(&self->dirbar.component, psy_ui_ALIGN_TOP);
	self->filebar.filebox = &self->filebox;
	self->filebar.notebook = &self->notebook;
	psy_ui_component_set_align(&self->filebar.component, psy_ui_ALIGN_BOTTOM);
	/* mainview */
	psy_ui_component_init(&self->mainview, &self->notebook.component, NULL);	
	propertiesrenderer_init(&self->renderer, &self->mainview, properties,
		numcols, lazy, &self->filebox);
	self->renderer.state.notebook = &self->notebook;
	psy_ui_scroller_init(&self->scroller, &self->mainview, NULL, NULL);
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
	psy_ui_tabbar_init(&self->tabbar, &self->mainview);
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
	psy_ui_notebook_select(&self->notebook, 1);
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
				psy_ui_Tab* tab;
				
				tab = psy_ui_tabbar_append(&self->tabbar,
					psy_property_text(property), psy_INDEX_INVALID,
					property->item.icon_light_id, 
					property->item.icon_dark_id,
					psy_ui_colour_white());
				psy_ui_tab_set_key(tab, psy_property_key(property));
			}
		}
	}	
	psy_ui_tabbar_select(&self->tabbar, 0);
}

void propertiesview_on_tabbar_change(PropertiesView* self,
	psy_ui_Component* sender, uintptr_t tabindex)
{		
	psy_ui_Tab* tab;

	tab = psy_ui_tabbar_tab(&self->tabbar, tabindex);
	if (tab) {
		propertiesview_scroll_to(self, tab->key);
	}
}

void propertiesview_scroll_to(PropertiesView* self, const char* key)
{
	psy_Property* property;

	property = psy_property_find(self->renderer.properties, key,
		PSY_PROPERTY_TYPE_NONE);
	if (property) {
		psy_property_scrollto(property);
	}	
}

void propertiesview_reload(PropertiesView* self)
{
	propertiesrenderer_build(&self->renderer);
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->component);
}

void propertiesview_select(PropertiesView* self, psy_Property* property)
{	
	/* todo */
}

void propertiesview_mark(PropertiesView* self, psy_Property* property)
{
	/* todo */
}

psy_Property* propertiesview_selected(PropertiesView* self)
{
	/* todo */
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
		top += step;
		break;
	case CMD_NAVPAGEUP:					
		top = psy_max(0, top - step * 16.0);		
		break;
	case CMD_NAVPAGEDOWN:
		top += (step * 16.0);
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
		psy_ui_component_align(&self->scroller.pane);
		psy_ui_component_invalidate(&self->scroller.pane);
	}
}

void propertiesview_on_dir_changed(PropertiesView* self, FileBox* sender)
{
	propertiesdirbar_set_text(&self->dirbar, filebox_directory(sender));
}

void propertiesview_on_file_selected(PropertiesView* self, FileBox* sender)
{
	char path[4096];
	
	filebox_full_name(sender, path, 4096);
	psy_signal_emit(&sender->property->signal_file_accept, sender->property, 1,
		path);
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

// host
#include "propertiesview.h"
#include "trackergridstate.h" // TRACKER CMDS
#include "styles.h" // TRACKER CMDS
// ui
#include <uicolordialog.h>
#include <uifolderdialog.h>
#include <uifontdialog.h>
#include <uiswitch.h>
// platform
#include "../../detail/portable.h"

// PropertiesRenderState
void propertiesrenderstate_init(PropertiesRenderState* self)
{
	self->property = NULL;
	self->selected = NULL;
	self->line = NULL;
}

// PropertiesRenderLine
// prototypes
static void propertiesrenderline_onpreferredsize(PropertiesRenderLine*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void propertiesrenderline_onmousedown(PropertiesRenderLine*,
	psy_ui_MouseEvent*);
static void propertiesrenderline_updatecheck(PropertiesRenderLine*);

// vtable
static psy_ui_ComponentVtable propertiesrenderline_vtable;
static bool propertiesrenderline_vtable_initialized = FALSE;

static void propertiesrenderline_vtable_init(PropertiesRenderLine* self)
{
	if (!propertiesrenderline_vtable_initialized) {
		propertiesrenderline_vtable = *(self->component.vtable);
		propertiesrenderline_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			propertiesrenderline_onpreferredsize;
		propertiesrenderline_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			propertiesrenderline_onmousedown;
		propertiesrenderline_vtable_initialized = TRUE;
	}
	self->component.vtable = &propertiesrenderline_vtable;
}
// implementation
void propertiesrenderline_init(PropertiesRenderLine* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	PropertiesRenderState* state, psy_Property* property,
	uintptr_t level, uintptr_t numcols)
{	
	psy_ui_component_init(&self->component, parent, view);
	propertiesrenderline_vtable_init(self);
	psy_ui_component_setalign(&self->component, psy_ui_ALIGN_TOP);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_zero());
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);	
	self->property = property;
	self->level = level;
	self->check = NULL;
	self->state = state;
	self->numcols = numcols;
	assert(self->property);
	// column 0
	psy_ui_component_init(&self->col0, &self->component, view);
	psy_ui_component_setdefaultalign(&self->col0, psy_ui_ALIGN_CLIENT,
		psy_ui_margin_zero());
	psy_ui_component_setpreferredsize(&self->col0,
		psy_ui_size_make_em(80.0, 1.0));
	psy_ui_label_init(&self->key, &self->col0, view);
	if (!property->item.translate) {
		psy_ui_label_preventtranslation(&self->key);
	}
	psy_ui_label_settext(&self->key, psy_property_text(self->property));	
	//psy_ui_label_setcharnumber(&self->key, 60.0 - self->level * 4.0);
	psy_ui_component_setmargin(psy_ui_label_base(&self->key),
		psy_ui_margin_makeem(0.0, 0.0, 0.0, self->level * 4.0));	
	if (numcols > 1) {
		// col1
		psy_ui_component_init(&self->col1, &self->component, view);
		psy_ui_component_setalign(&self->col1, psy_ui_ALIGN_CLIENT);
		psy_ui_component_setdefaultalign(&self->col1, psy_ui_ALIGN_LEFT,
			psy_ui_margin_zero());		
		if (psy_property_type(self->property) == PSY_PROPERTY_TYPE_BOOL) {
			self->check = psy_ui_switch_allocinit(&self->col1, view);
			propertiesrenderline_updatecheck(self);
		} else if (psy_property_ischoiceitem(self->property)) {
			self->check = psy_ui_switch_allocinit(&self->col1, view);
			propertiesrenderline_updatecheck(self);
		} else if (psy_property_type(self->property) == PSY_PROPERTY_TYPE_ACTION) {
			psy_ui_Button* button;

			button = psy_ui_button_allocinit(&self->col1, view);
			psy_ui_button_settext(button, psy_property_text(self->property));
		} else if (psy_property_hint(property) == PSY_PROPERTY_HINT_SHORTCUT) {
			char str[256];
			psy_ui_Label* label;

			inputdefiner_inputtotext(
				(uint32_t)psy_property_item_int(property),
				str);
			label = psy_ui_label_allocinit(&self->col1, view);
			psy_ui_label_preventtranslation(label);
			psy_ui_label_settext(label, str);
			psy_ui_label_setcharnumber(label, 40.0);
		} else if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {
			char str[128];
			psy_ui_Label* label;
			psy_ui_Component* colour;

			label = psy_ui_label_allocinit(&self->col1, view);
			psy_ui_label_preventtranslation(label);
			psy_snprintf(str, 128, "0x%d", psy_property_item_int(property));
			psy_ui_label_settext(label, str);
			psy_ui_label_setcharnumber(label, 40.0);
			colour = psy_ui_component_allocinit(&self->col1, view);
			psy_ui_component_setminimumsize(colour,
				psy_ui_size_make_em(4.0, 1.0));
			psy_ui_component_setbackgroundcolour(colour,
				psy_ui_colour_make(psy_property_item_int(property)));
		} else if (psy_property_type(self->property) == PSY_PROPERTY_TYPE_INTEGER) {
			psy_ui_Label* value;
			char text[64];

			value = psy_ui_label_allocinit(&self->col1, view);
			psy_ui_label_preventtranslation(value);
			if (psy_property_hint(property) == PSY_PROPERTY_HINT_EDITHEX) {
				psy_snprintf(text, 64, "%X", (int)psy_property_item_int(property));
			} else {
				psy_snprintf(text, 64, "%d", (int)psy_property_item_int(self->property));
			}
			psy_ui_label_settext(value, text);
		} else if (psy_property_type(self->property) == PSY_PROPERTY_TYPE_STRING) {
			psy_ui_Label* label;

			label = psy_ui_label_allocinit(&self->col1, view);
			// if (!property->item.translate) {
			psy_ui_label_preventtranslation(label);
			//}
			psy_ui_label_settext(label, psy_property_item_str(self->property));
		} else if (psy_property_type(self->property) == PSY_PROPERTY_TYPE_FONT) {
			psy_ui_Label* label;
			char str[128];
			psy_ui_FontInfo fontinfo;
			psy_ui_Font font;
			int pt;
			const psy_ui_TextMetric* tm;

			label = psy_ui_label_allocinit(&self->col1, view);
			psy_ui_label_preventtranslation(label);
			psy_ui_fontinfo_init_string(&fontinfo, psy_property_item_str(property));
			psy_ui_font_init(&font, &fontinfo);
			psy_ui_component_setfont(self->state->dummy, &font);
			tm = psy_ui_component_textmetric(self->state->dummy);
			psy_ui_font_dispose(&font);
			if (fontinfo.lfHeight < 0) {
				pt = ((tm->tmHeight - tm->tmInternalLeading) * 72) / psy_ui_logpixelsy();
			} else {
				pt = tm->tmHeight;
			}
			psy_snprintf(str, 128, "%s %d pt", fontinfo.lfFaceName, (int)pt);
			psy_ui_label_settext(label, str);
		}
	}
	if (numcols > 2) {
		// col2
		psy_ui_component_init(&self->col2, &self->component, view);		
		psy_ui_component_setalign(&self->col2, psy_ui_ALIGN_RIGHT);
		psy_ui_component_setpreferredsize(&self->col2,
			psy_ui_size_make_em(40.0, 1.0));
		psy_ui_component_setdefaultalign(&self->col2, psy_ui_ALIGN_CLIENT,
			psy_ui_margin_zero());
		if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITDIR ||
			psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {
			psy_ui_Button* button;

			button = psy_ui_button_allocinit(&self->col2, view);
			psy_ui_button_preventtranslation(button);
			psy_ui_button_settext(button, "...");
		} else if (psy_property_type(self->property) == PSY_PROPERTY_TYPE_FONT) {
			psy_ui_Button* button;

			button = psy_ui_button_allocinit(&self->col2, view);
			psy_ui_button_settext(button, "settingsview.choose-font");
		} else if (psy_property_hint(property) == PSY_PROPERTY_HINT_SHORTCUT) {
			psy_ui_Button* button;

			button = psy_ui_button_allocinit(&self->col2, view);
			psy_ui_button_preventtranslation(button);
			psy_ui_button_settext(button, "None");
		} else if (psy_property_int_hasrange(property) && !psy_property_readonly(property)) {
			char text[256];
			psy_ui_Label* label;

			psy_snprintf(text, 256, "from %d to %d", property->item.min, property->item.max);
			label = psy_ui_label_allocinit(&self->col2, view);
			psy_ui_label_preventtranslation(label);
			psy_ui_label_settext(label, text);
		}
	}	
}

PropertiesRenderLine* propertiesrenderline_alloc(void)
{
	return (PropertiesRenderLine*)malloc(sizeof(PropertiesRenderLine));
}

PropertiesRenderLine* propertiesrenderline_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	PropertiesRenderState* state, psy_Property* property,
	uintptr_t level, uintptr_t numcols)
{
	PropertiesRenderLine* rv;

	rv = propertiesrenderline_alloc();
	if (rv) {
		propertiesrenderline_init(rv, parent, view, state, property, level,
			numcols);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

void propertiesrenderline_onmousedown(PropertiesRenderLine* self,
	psy_ui_MouseEvent* ev)
{
	self->state->property = NULL;
	self->state->selected = self->property;
	self->state->line = self;
	if (psy_property_readonly(self->property)) {
		return;
	}
	self->state->property = self->property;
	if (psy_property_ischoiceitem(self->property)) {		
		psy_property_setitem_int(psy_property_parent(self->property), 
			psy_property_choiceitem_index(self->property));	
	} else if (psy_property_type(self->property) == PSY_PROPERTY_TYPE_BOOL) {
		psy_property_setitem_bool(self->property,
			!psy_property_item_bool(self->property));			
		propertiesrenderline_updatecheck(self);		
	}
}

void propertiesrenderline_updatecheck(PropertiesRenderLine* self)
{
	if (self->check && psy_property_type(self->property) ==
			PSY_PROPERTY_TYPE_BOOL) {
		if (psy_property_item_bool(self->property)) {
			psy_ui_switch_check(self->check);
		} else {
			psy_ui_switch_uncheck(self->check);
		}		
	} else if (self->check && psy_property_ischoiceitem(self->property)) {
		if (psy_property_at_choice(psy_property_parent(self->property)) ==
				self->property) {
			psy_ui_switch_check(self->check);
		} else {		
			psy_ui_switch_uncheck(self->check);
		}		
	}
}

void propertiesrenderline_onpreferredsize(PropertiesRenderLine* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (self->numcols == 1) {
		psy_ui_size_setem(rv, 40.0, 1.5);
	} else {
		psy_ui_size_setem(rv, 190.0, 1.5);
	}
}

// PropertiesRenderer
// prototypes
static void propertiesrenderer_build(PropertiesRenderer*);
static void propertiesrenderer_ondraw(PropertiesRenderer*, psy_ui_Graphics*);
int propertiesrenderer_onpropertiesbuild(PropertiesRenderer*,
	psy_Property*, uintptr_t level);
void propertiesrenderer_buildsection(PropertiesRenderer*,
	psy_Property* section);
static void propertiesrenderer_onmousedown(PropertiesRenderer*,
	psy_ui_MouseEvent*);
static void propertiesrenderer_oneditchange(PropertiesRenderer*,
	psy_ui_Edit* sender);
static void propertiesrenderer_oneditkeydown(PropertiesRenderer*,
	psy_ui_Component* sender, psy_ui_KeyEvent*);
static void propertiesrenderer_ondestroy(PropertiesRenderer*,
	psy_ui_Component* sender);
static void propertiesrenderer_oninputdefineraccept(PropertiesRenderer*,
	InputDefiner* sender);
static void propertiesview_gotosection(PropertiesView*, const char* section);
static void propertiesrenderer_showedit(PropertiesRenderer*,
	psy_ui_Component* line, psy_ui_Component* edit);
// vtable
static psy_ui_ComponentVtable propertiesrenderer_vtable;
static bool propertiesrenderer_vtable_initialized = FALSE;

static void propertiesrenderer_vtable_init(PropertiesRenderer* self)
{
	if (!propertiesrenderer_vtable_initialized) {
		propertiesrenderer_vtable = *(self->component.vtable);
		propertiesrenderer_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			propertiesrenderer_onmousedown;
		propertiesrenderer_vtable_initialized = TRUE;
	}
	self->component.vtable = &propertiesrenderer_vtable;
}
// implementation
void propertiesrenderer_init(PropertiesRenderer* self,
	psy_ui_Component* parent, psy_Property* properties, uintptr_t numcols,
	Workspace* workspace)
{
	self->workspace = workspace;
	self->properties = properties;
	self->numcols = numcols;
	self->currsection = NULL;
	psy_ui_component_init(&self->component, parent, NULL);
	propertiesrenderer_vtable_init(self);
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);	
	psy_ui_component_setwheelscroll(&self->component, 4);
	psy_signal_connect(&self->component.signal_destroy, self,
		propertiesrenderer_ondestroy);
	psy_ui_component_init(&self->client, &self->component, NULL);
	psy_ui_component_doublebuffer(&self->client);	
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setdefaultalign(&self->client,
		psy_ui_ALIGN_TOP, psy_ui_margin_makeem(0.0, 0.0, 0.5, 0.0));
	psy_ui_component_init(&self->dummy, &self->component, NULL);
	psy_ui_component_hide(&self->dummy);
	propertiesrenderstate_init(&self->state);
	self->state.dummy = &self->dummy;
	self->selected = 0;
	self->keyselected = 0;	
	self->floated = 0;
	self->showkeyselection = FALSE;	
	self->valuecolour = psy_ui_colour_make(0x00CACACA);
	self->sectioncolour = psy_ui_colour_make(0x00CACACA);
	self->separatorcolour = psy_ui_colour_make(0x00333333);
	self->valueselcolour = psy_ui_colour_make(0x00FFFFFF);
	self->valueselbackgroundcolour = psy_ui_colour_make(0x009B7800);
	psy_ui_edit_init(&self->edit, &self->component);
	psy_signal_connect(&self->edit.component.signal_keydown, self,
		propertiesrenderer_oneditkeydown);
	psy_ui_component_hide(&self->edit.component);
	inputdefiner_init(&self->inputdefiner, &self->component);
	psy_ui_component_hide(&self->inputdefiner.component);
	psy_signal_connect(&self->inputdefiner.signal_accept, self,
		propertiesrenderer_oninputdefineraccept);	
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_selected);	
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_VSCROLL);	
	psy_ui_component_setscrollstep(&self->component,
		psy_ui_size_make_em(0.0, 4.0));
	psy_table_init(&self->sections);
	propertiesrenderer_build(self);	
}

void propertiesrenderer_ondestroy(PropertiesRenderer* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_selected);
	psy_table_dispose(&self->sections);
}

void propertiesrenderer_build(PropertiesRenderer* self)
{
	self->currsection = NULL;
	psy_table_clear(&self->sections);
	psy_ui_component_clear(&self->client);
	psy_property_enumerate(self->properties, self,
		(psy_PropertyCallback)
		propertiesrenderer_onpropertiesbuild);
}

int propertiesrenderer_onpropertiesbuild(PropertiesRenderer* self,
	psy_Property* property, uintptr_t level)
{
	if (level == 0) {
		propertiesrenderer_buildsection(self, property);
	} else {
		if (!self->currsection) {
			self->currsection = &self->client;
		}
		if (psy_property_hint(property) != PSY_PROPERTY_HINT_HIDE) {
			propertiesrenderline_allocinit(self->currsection, &self->client,
				&self->state, property, level, self->numcols);
		}
	}
	return 1;
}

void propertiesrenderer_buildsection(PropertiesRenderer* self,
	psy_Property* section)
{
	if (psy_property_type(section) == PSY_PROPERTY_TYPE_SECTION) {
		psy_ui_Label* label;

		self->currsection = psy_ui_component_allocinit(&self->client,
			&self->client);		
		psy_ui_component_setdefaultalign(self->currsection, psy_ui_ALIGN_TOP,
			psy_ui_margin_zero());
		psy_ui_component_setstyletype(self->currsection,
			STYLE_PROPERTYVIEW_MAINSECTION);	
		label = psy_ui_label_allocinit(self->currsection, &self->client);		
		psy_ui_component_setmargin(psy_ui_label_base(label),
			psy_ui_margin_makeem(0.0, 0.0, 0.5, 0.0));
		psy_ui_component_setspacing(psy_ui_label_base(label),
			psy_ui_margin_makeem(0.5, 0.0, 0.5, 1.0));
		psy_ui_component_setstyletype(psy_ui_label_base(label),
			STYLE_PROPERTYVIEW_MAINSECTIONHEADER);		
		psy_ui_label_settext(label, psy_property_text(section));
		psy_table_insert_strhash(&self->sections, psy_property_text(section),
			self->currsection);
	}
}

void propertiesrenderer_onmousedown(PropertiesRenderer* self,
	psy_ui_MouseEvent* ev)
{
	if (self->state.selected) {
		self->selected = self->state.selected;
		psy_signal_emit(&self->signal_selected, self, 1,
			self->selected);
	}
	if (self->state.property) {
		self->selected = self->state.property;		
		if (psy_property_ischoiceitem(self->selected)) {
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selected);
		} else if (psy_property_hint(self->selected) == PSY_PROPERTY_HINT_EDITCOLOR) {
			psy_ui_ColourDialog colourdialog;

			psy_ui_colourdialog_init(&colourdialog, &self->component);
			psy_ui_colourdialog_setcolour(&colourdialog,
				psy_ui_colour_make((uint32_t)psy_property_item_int(self->selected)));
			if (psy_ui_colourdialog_execute(&colourdialog)) {
				psy_ui_Colour colour;

				colour = psy_ui_colourdialog_colour(&colourdialog);
				psy_property_set_int(self->selected->parent,
					self->selected->item.key,
					colour.value);
				psy_signal_emit(&self->signal_changed, self, 1,
					self->selected);
			}
			psy_ui_colourdialog_dispose(&colourdialog);
		} else if (psy_property_hint(self->state.property) == PSY_PROPERTY_HINT_SHORTCUT) {
			if (self->state.line) {
				inputdefiner_setinput(&self->inputdefiner, (uint32_t)
					psy_property_item_int(self->state.property));
				propertiesrenderer_showedit(self, &self->state.line->component,
					&self->inputdefiner.component);
			}		
		} else if (psy_property_type(self->selected) == PSY_PROPERTY_TYPE_INTEGER) {
			if (self->state.line) {
				char text[40];
				if (psy_property_hint(self->selected) == PSY_PROPERTY_HINT_EDITHEX) {
					psy_snprintf(text, 40, "%X",
						psy_property_item_int(self->selected));
				} else {
					psy_snprintf(text, 40, "%d",
						psy_property_item_int(self->selected));
				}
				psy_ui_edit_settext(&self->edit, text);
				propertiesrenderer_showedit(self, &self->state.line->component,
					&self->edit.component);
			}
		} else if (psy_property_hint(self->state.property) == PSY_PROPERTY_HINT_EDITDIR) {
			psy_ui_FolderDialog dialog;

			psy_ui_folderdialog_init_all(&dialog, 0, psy_ui_translate(
				psy_property_text(self->selected)), "");
			if (psy_ui_folderdialog_execute(&dialog)) {
				psy_property_set_str(self->selected->parent,
					self->selected->item.key, psy_ui_folderdialog_path(&dialog));
				psy_signal_emit(&self->signal_changed, self, 1,
					self->selected);
			}		
		} else if (psy_property_type(self->selected) == PSY_PROPERTY_TYPE_STRING) {
			if (!psy_property_readonly(self->selected)) {
				psy_ui_edit_settext(&self->edit, psy_property_item_str(
					self->selected));
				propertiesrenderer_showedit(self, &self->state.line->component,
					&self->edit.component);
			}
		} else {
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selected);
		}		
		psy_ui_component_invalidate(&self->component);
	}
	self->state.property = NULL;
	self->state.line = NULL;
	self->state.selected = NULL;
}

void propertiesrenderer_showedit(PropertiesRenderer* self,
	psy_ui_Component* line, psy_ui_Component* edit)
{	
	psy_ui_RealRectangle position;
	psy_ui_Value colwidth;

	position = psy_ui_component_position(line);
	colwidth = psy_ui_value_makeew(60.0);
	psy_ui_component_setposition(edit,
		psy_ui_rectangle_make(
			psy_ui_point_make(colwidth, psy_ui_value_make_px(position.top)),
			psy_ui_size_make_em(80.0, 1.5)));				
	psy_ui_component_show(edit);
	psy_ui_component_invalidate(&self->component);
	psy_ui_component_setfocus(edit);
}

void propertiesrenderer_oninputdefineraccept(PropertiesRenderer* self,
	InputDefiner* sender)
{
	if (self->selected && psy_property_type(self->selected) ==
			PSY_PROPERTY_TYPE_INTEGER) {
		psy_property_setitem_int(self->selected,
			inputdefiner_input(&self->inputdefiner));
		psy_ui_component_hide(&self->inputdefiner.component);
		psy_ui_component_invalidate(&self->component);
	}
	psy_signal_emit(&self->signal_changed, self, 1, self->selected);
}

void propertiesrenderer_oneditchange(PropertiesRenderer* self, psy_ui_Edit* sender)
{
	if (self->selected) {
		switch (psy_property_type(self->selected)) {
			case PSY_PROPERTY_TYPE_INTEGER:
				if (psy_property_hint(self->selected) == PSY_PROPERTY_HINT_EDITHEX) {
					psy_property_setitem_int(self->selected,
						strtol(psy_ui_edit_text(&self->edit), NULL, 16));
				} else {
					psy_property_setitem_int(self->selected,
						atoi(psy_ui_edit_text(&self->edit)));
				}
			break;
			case PSY_PROPERTY_TYPE_STRING:
				psy_property_setitem_str(self->selected,
					psy_ui_edit_text(&self->edit));
				break;		
			default:
				break;
		}		
		psy_signal_emit(&self->signal_changed, self, 1, self->selected);
		self->selected = NULL;
	}
}

void propertiesrenderer_oneditkeydown(PropertiesRenderer* self,
	psy_ui_Component* sender, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_RETURN) {
		psy_ui_component_hide(&self->edit.component);
		psy_ui_component_setfocus(&self->component);
		propertiesrenderer_oneditchange(self, &self->edit);
	} else if (ev->keycode == psy_ui_KEY_ESCAPE) {
		psy_ui_component_hide(&self->edit.component);
		psy_ui_component_setfocus(&self->component);		
	} else if (psy_property_hint(self->selected) == PSY_PROPERTY_HINT_EDITHEX) {
		if ((ev->keycode >= psy_ui_KEY_DIGIT0 && ev->keycode <= psy_ui_KEY_DIGIT9) ||
			(ev->keycode >= psy_ui_KEY_A && ev->keycode <= psy_ui_KEY_F) ||
			(ev->keycode < psy_ui_KEY_HELP)) {
			return;
		} else {
			psy_ui_keyevent_preventdefault(ev);
		}
	}
	psy_ui_keyevent_stoppropagation(ev);
}

// PropertiesView
// prototypes
static void propertiesview_ondestroy(PropertiesView*, psy_ui_Component* sender);
static void propertiesview_initsectionfloated(PropertiesView*);
static void propertiesview_selectsection(PropertiesView*, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options);
static void propertiesview_updatetabbarsections(PropertiesView*);
static void propertiesview_ontabbarchange(PropertiesView*, psy_ui_Component* sender,
	uintptr_t tabindex);
static void propertiesview_onpropertiesrendererchanged(PropertiesView*,
	PropertiesRenderer* sender, psy_Property*);
static void propertiesview_onpropertiesrendererselected(PropertiesView*,
	PropertiesRenderer* sender, psy_Property*);
static void propertiesview_oneventdriverinput(PropertiesView*, psy_EventDriver* sender);
static double propertiesview_checkrange(PropertiesView*, double position);
static void propertiesview_onfocus(PropertiesView*, psy_ui_Component* sender);
static void propertiesview_onmousedown(PropertiesView*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void propertiesview_onmouseup(PropertiesView*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void propertiesview_onscroll(PropertiesView*, psy_ui_Component* sender);
// implementation
void propertiesview_init(PropertiesView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_Property* properties,
	uintptr_t numcols, Workspace* workspace)
{
	psy_ui_Margin tabmargin;

	self->workspace = workspace;	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);
	psy_ui_notebook_init(&self->notebook, propertiesview_base(self));
	psy_ui_component_setalign(&self->notebook.component, psy_ui_ALIGN_CLIENT);
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_selected);
	psy_signal_connect(&self->component.signal_destroy, self,
		propertiesview_ondestroy);
	psy_signal_connect(&self->component.signal_focus,
		self, propertiesview_onfocus);	
	psy_ui_component_init(&self->viewtabbar, tabbarparent, NULL);
	psy_ui_component_init(&self->client,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_signal_connect(&self->client.signal_mousedown,
		self, propertiesview_onmousedown);
	psy_signal_connect(&self->client.signal_mouseup,
		self, propertiesview_onmouseup);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	propertiesrenderer_init(&self->renderer, &self->client, properties,
		numcols, workspace);
	psy_ui_scroller_init(&self->scroller, &self->renderer.component,
		&self->client, NULL);	
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->renderer.component, psy_ui_ALIGN_HCLIENT);
	psy_signal_connect(&self->component.signal_selectsection, self,
		propertiesview_selectsection);	
	psy_ui_tabbar_init(&self->tabbar, &self->client);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar), psy_ui_ALIGN_RIGHT);
	psy_ui_tabbar_settabalignment(&self->tabbar, psy_ui_ALIGN_RIGHT);	
	psy_ui_margin_init_all_em(&tabmargin, 0.0, 1.0, 0.5, 2.0);		
	psy_ui_tabbar_setdefaulttabmargin(&self->tabbar, tabmargin);
	propertiesview_updatetabbarsections(self);
	psy_signal_connect(&self->renderer.signal_changed, self,
		propertiesview_onpropertiesrendererchanged);
	psy_signal_connect(&self->renderer.signal_selected, self,
		propertiesview_onpropertiesrendererselected);	
	psy_signal_connect(&workspace_player(self->workspace)->eventdrivers.signal_input,
		self, propertiesview_oneventdriverinput);
	propertiesview_initsectionfloated(self);
	psy_signal_connect(&self->tabbar.signal_change, self,
		propertiesview_ontabbarchange);
	psy_ui_notebook_select(&self->notebook, 0);
	psy_signal_connect(&self->renderer.component.signal_scroll, self,
		propertiesview_onscroll);
	self->preventscrollupdate = FALSE;
}

void propertiesview_ondestroy(PropertiesView* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_selected);
}

void propertiesview_initsectionfloated(PropertiesView* self)
{
	psy_ui_component_init(&self->sectionfloated,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_ui_component_hide(&self->sectionfloated);
	psy_ui_label_init(&self->floatdesc, &self->sectionfloated, NULL);
	psy_ui_label_preventtranslation(&self->floatdesc);
	psy_ui_label_settext(&self->floatdesc, "This view is floated.");
	psy_ui_component_setalign(&self->floatdesc.component,
		psy_ui_ALIGN_CENTER);
}

void propertiesview_selectsection(PropertiesView* self,
	psy_ui_Component* sender, uintptr_t section, uintptr_t options)
{	
	if (options > 0) {
		const psy_List* p;
		uintptr_t c;
		psy_Property* property;
		
		property = NULL;
		for (c = 0, p = psy_property_begin_const(propertiesrenderer_properties(
			&self->renderer)); p != NULL; psy_list_next_const(&p)) {			

			property = (psy_Property*)psy_list_entry_const(p);
			if (c == section) {
				break;
			}			
			if (psy_property_hastype(property, PSY_PROPERTY_TYPE_SECTION)) {
				++c;
			}
		}
		if (p && !psy_property_empty(property)) {
			uintptr_t c;

			p = psy_property_begin_const(property);
			for (c = 0; p != NULL; psy_list_next_const(&p)) {
				property = (psy_Property*)psy_list_entry_const(p);
				if (c == options) {
					break;
				}				
				if (psy_property_hastype(property, PSY_PROPERTY_TYPE_SECTION)) {
					++c;
				}
			}
			if (p) {
				propertiesview_gotosection(self,
					psy_property_text(psy_list_entry_const(p)));
			}
		}
	} else {
		self->preventscrollupdate = TRUE;
		psy_ui_tabbar_select(&self->tabbar, section);
		self->preventscrollupdate = FALSE;
	}
}

void propertiesview_updatetabbarsections(PropertiesView* self)
{	
	psy_ui_tabbar_clear(&self->tabbar);
	if (propertiesrenderer_properties(&self->renderer)) {
		const psy_List* p;
		
		for (p = psy_property_begin_const(propertiesrenderer_properties(
				&self->renderer)); p != NULL; psy_list_next_const(&p)) {
			psy_Property* property;

			property = (psy_Property*)psy_list_entry_const(p);
			if (psy_property_hastype(property, PSY_PROPERTY_TYPE_SECTION)) {
				psy_ui_tabbar_append(&self->tabbar, psy_property_text(property));
			}
		}
	}	
	psy_ui_tabbar_select(&self->tabbar, 0);		
}

void propertiesview_ontabbarchange(PropertiesView* self, psy_ui_Component* sender,
	uintptr_t tabindex)
{	
	psy_ui_Tab* tab;
	
	if (self->renderer.properties) {	
		psy_Property* property = NULL;
		
		tab = psy_ui_tabbar_tab(&self->tabbar, tabindex);
		if (tab) {
			psy_List* p;

			p = psy_property_begin(self->renderer.properties);
			while (p) {
				property = (psy_Property*)p->entry;
				if (psy_property_type(property) == PSY_PROPERTY_TYPE_SECTION) {
					if (strcmp(psy_ui_translate(psy_property_text(property)), tab->translation) == 0) {
						break;
					}
				}
				property = NULL;
				psy_list_next(&p);
			}			
			propertiesview_gotosection(self,
				psy_property_text(psy_list_entry_const(p)));
		}		
	}
}

void propertiesview_gotosection(PropertiesView* self, const char* sectionkey)
{
	psy_ui_Component* section;

	section = psy_table_at_strhash(&self->renderer.sections, sectionkey);
	if (section) {
		psy_ui_RealRectangle position;

		position = psy_ui_component_position(section);
		psy_ui_component_setscrolltop(&self->renderer.component,
			psy_ui_value_make_px(position.top));
	}
}

void propertiesview_onpropertiesrendererchanged(PropertiesView* self,
	PropertiesRenderer* sender, psy_Property* selected)
{	
	psy_ui_component_invalidate(&self->renderer.client);
	psy_ui_component_update(&self->renderer.client);
	psy_signal_emit(&self->signal_changed, self, 1, selected);
	propertiesrenderer_build(&self->renderer);
	psy_ui_component_align(&self->renderer.client);
	psy_ui_component_updateoverflow(propertiesrenderer_base(&self->renderer));
}

void propertiesview_onpropertiesrendererselected(PropertiesView* self,
	PropertiesRenderer* sender, psy_Property* selected)
{
	psy_signal_emit(&self->signal_selected, self, 1, selected);
}

void propertiesview_oneventdriverinput(PropertiesView* self, psy_EventDriver* sender)
{
	if (psy_ui_component_hasfocus(&self->renderer.component)) {
		psy_EventDriverCmd cmd;


		cmd = psy_eventdriver_getcmd(sender, "tracker");
		if (cmd.id != -1) {
			double scrollstepypx;
			psy_ui_Value scrollstepy;
			double scrollstepxpx;
			psy_ui_Value scrollstepx;
			const psy_ui_TextMetric* tm;

			tm = psy_ui_component_textmetric(&self->renderer.component);			
			scrollstepy = psy_ui_component_scrollstep_height(&self->renderer.component);
			scrollstepypx = psy_ui_value_px(&scrollstepy, tm);
			scrollstepx = psy_ui_component_scrollstep_width(&self->renderer.component);
			scrollstepxpx = psy_ui_value_px(&scrollstepx, tm);
			switch (cmd.id) {
				case CMD_NAVTOP:
					psy_ui_component_setscrolltop(&self->renderer.component,
						psy_ui_value_zero());
					break;
				case CMD_NAVBOTTOM:
					psy_ui_component_setscrolltop(&self->renderer.component, 
						psy_ui_value_make_px(propertiesview_checkrange(self,
							INT32_MAX)));
					break;
				case CMD_NAVUP: {					
					psy_ui_component_setscrolltop(&self->renderer.component,
						psy_ui_value_make_px(
							psy_max(0,
								psy_ui_component_scrolltoppx(&self->renderer.component) -
								scrollstepypx)));
					break; }
				case CMD_NAVDOWN: {
					double position;					
					
					position = psy_ui_component_scrolltoppx(&self->renderer.component) +						
						scrollstepypx;
					psy_ui_component_setscrolltop(&self->renderer.component,
						psy_ui_value_make_px(
							propertiesview_checkrange(self, position)));
					break; }
				case CMD_NAVPAGEUP:
					psy_ui_component_setscrolltop(&self->renderer.component,
						psy_ui_value_make_px(
							psy_max(0,
								psy_ui_component_scrolltoppx(&self->renderer.component) -
								scrollstepypx * 16)));
					break;
				case CMD_NAVPAGEDOWN: {					
					double position;
									
					position = psy_ui_component_scrolltoppx(&self->renderer.component) +
						scrollstepypx * 16;
					psy_ui_component_setscrolltop(&self->renderer.component,
						psy_ui_value_make_px(propertiesview_checkrange(self, position)));
					break; }
				default:
					break;
			}
		}	
	}
}

double propertiesview_checkrange(PropertiesView* self, double position)
{
	intptr_t steps;
	double scrollstepypx;	
	psy_ui_IntPoint scrollrange;
	
	scrollrange = psy_ui_component_verticalscrollrange(&self->renderer.component);	
	scrollstepypx = psy_ui_component_scrollstep_height_px(&self->renderer.component);
	steps = (intptr_t)(position / scrollstepypx);
	steps = psy_min(scrollrange.y, steps);
	return (double)(steps * scrollstepypx);
}

void propertiesview_onfocus(PropertiesView* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(&self->renderer.component);
}

void propertiesview_onmousedown(PropertiesView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	//psy_ui_component_setfocus(&self->renderer.component);
	if (ev->button == 2) {
		if (psy_ui_component_visible(&self->sectionfloated)) {
			workspace_docksection(self->workspace, VIEW_ID_SETTINGSVIEW, 0);			
		} else {
			workspace_floatsection(self->workspace, VIEW_ID_SETTINGSVIEW, 0);
		}		
		psy_ui_mouseevent_stoppropagation(ev);
	}
}

void propertiesview_onmouseup(PropertiesView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_ui_mouseevent_stoppropagation(ev);
}

void propertiesview_float(PropertiesView* self, uintptr_t section, psy_ui_Component* dest)
{
//	if (section == HELPVIEWSECTION_HELP) {		
		psy_ui_component_hide(&self->client);		
		psy_ui_component_insert(dest, &self->client, NULL);
		psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);		
		self->renderer.floated = TRUE;
		//psy_ui_component_preventalign(&self->renderer.component);		
		psy_ui_component_show_align(&self->client);		
		psy_ui_notebook_select(&self->notebook, 0);				
//	}
}

void propertiesview_dock(PropertiesView* self, uintptr_t section, psy_ui_Component* dest)
{
//	if (section == HELPVIEWSECTION_HELP) {
		self->renderer.floated = FALSE;
		psy_ui_component_hide(&self->sectionfloated);	
		psy_ui_component_insert(&self->notebook.component,
			&self->client, NULL);
		psy_ui_component_enablealign(&self->client);
		psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
		psy_ui_component_show_align(&self->client);
		psy_ui_component_align(&self->client);
		psy_ui_notebook_select(&self->notebook, 0);		
//	}
}

void propertiesview_onscroll(PropertiesView* self, psy_ui_Component* sender)
{
	/*if (!self->preventscrollupdate) {
		PropertiesRenderLineState* state;
		int pos;

		pos = 0;
		state = propertiesrenderer_findfirstmainlinestate(&self->renderer,
			psy_ui_component_scrolltoppx(sender), &pos);
		if (state) {
			if (pos > 0 && state->cpy > psy_ui_component_scrolltoppx(sender)) {
				--pos;
			}
			if (psy_ui_tabbar_selected(&self->tabbar) != pos) {
				psy_signal_prevent(&self->tabbar.signal_change, self,
					propertiesview_ontabbarchange);
				psy_ui_tabbar_select(&self->tabbar, pos);
				psy_signal_enable(&self->tabbar.signal_change, self,
					propertiesview_ontabbarchange);
			}
		}
	}*/
}

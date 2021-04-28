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
	self->dialogbutton = FALSE;
	self->edit = NULL;
}

// PropertiesRenderLine
// prototypes
static void propertiesrenderline_onpreferredsize(PropertiesRenderLine*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void propertiesrenderline_onmousedown(PropertiesRenderLine*,
	psy_ui_MouseEvent*);

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
	self->colour = NULL;
	self->label = NULL;	
	self->dialogbutton = NULL;
	self->state = state;
	self->numcols = numcols;
	assert(self->property);
	// column 0
	psy_ui_component_init(&self->col0, &self->component, view);	
	psy_ui_component_setpreferredsize(&self->col0,
		psy_ui_size_make_em(80.0, 1.0));
	psy_ui_label_init(&self->key, &self->col0, view);
	psy_ui_component_setalign(&self->key.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setmargin(psy_ui_label_base(&self->key),
		psy_ui_margin_make_em(0.0, 0.0, 0.0, self->level * 4.0));
	if (!property->item.translate) {
		psy_ui_label_preventtranslation(&self->key);
	}
	psy_ui_label_settext(&self->key, psy_property_text(self->property));	
	if (numcols > 1) {
		// col1
		psy_ui_component_init(&self->col1, &self->component, view);
		psy_ui_component_setalign(&self->col1, psy_ui_ALIGN_CLIENT);
		if (psy_property_type(self->property) == PSY_PROPERTY_TYPE_BOOL) {
			self->check = psy_ui_switch_allocinit(&self->col1, view);
			psy_ui_component_setalign(&self->check->component, psy_ui_ALIGN_LEFT);
			propertiesrenderline_updatecheck(self);
		} else if (psy_property_ischoiceitem(self->property)) {
			self->check = psy_ui_switch_allocinit(&self->col1, view);
			psy_ui_component_setalign(&self->check->component, psy_ui_ALIGN_LEFT);
			propertiesrenderline_updatecheck(self);
		} else if (psy_property_type(self->property) == PSY_PROPERTY_TYPE_ACTION) {
			psy_ui_Button* button;

			button = psy_ui_button_allocinit(&self->col1, view);
			psy_ui_button_settext(button, psy_property_text(self->property));
		} else if (psy_property_hint(property) == PSY_PROPERTY_HINT_SHORTCUT) {
			char str[256];

			inputdefiner_inputtotext(
				(uint32_t)psy_property_item_int(property),
				str);
			self->label = psy_ui_label_allocinit(&self->col1, view);
			psy_ui_component_setalign(&self->label->component, psy_ui_ALIGN_LEFT);
			psy_ui_label_preventtranslation(self->label);
			psy_ui_label_setcharnumber(self->label, 40.0);					
		} else if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {
			self->label = psy_ui_label_allocinit(&self->col1, view);
			psy_ui_component_setalign(&self->label->component, psy_ui_ALIGN_LEFT);
			psy_ui_label_preventtranslation(self->label);			
			psy_ui_label_setcharnumber(self->label, 40.0);
			self->colour = psy_ui_component_allocinit(&self->col1, view);
			psy_ui_component_setalign(self->colour, psy_ui_ALIGN_LEFT);
			psy_ui_component_setminimumsize(self->colour,
				psy_ui_size_make_em(4.0, 1.0));			
		} else if (psy_property_type(self->property) == PSY_PROPERTY_TYPE_INTEGER) {
			self->label = psy_ui_label_allocinit(&self->col1, view);
			psy_ui_component_setalign(&self->label->component, psy_ui_ALIGN_LEFT);
			psy_ui_label_preventtranslation(self->label);			
		} else if (psy_property_type(self->property) == PSY_PROPERTY_TYPE_STRING) {
			self->label = psy_ui_label_allocinit(&self->col1, view);
			psy_ui_component_setalign(&self->label->component, psy_ui_ALIGN_LEFT);			
			psy_ui_label_preventtranslation(self->label);			
		} else if (psy_property_type(self->property) == PSY_PROPERTY_TYPE_FONT) {
			self->label = psy_ui_label_allocinit(&self->col1, view);
			psy_ui_component_setalign(&self->label->component, psy_ui_ALIGN_LEFT);
			psy_ui_label_preventtranslation(self->label);			
		}
	}
	if (numcols > 2) {
		// col2
		psy_ui_component_init(&self->col2, &self->component, view);		
		psy_ui_component_setalign(&self->col2, psy_ui_ALIGN_RIGHT);
		psy_ui_component_setpreferredsize(&self->col2,
			psy_ui_size_make_em(40.0, 1.0));		
		if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITDIR ||
			psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {			
			self->dialogbutton = psy_ui_button_allocinit(&self->col2, view);
			psy_ui_component_setalign(&self->dialogbutton->component, psy_ui_ALIGN_CLIENT);
			psy_ui_button_preventtranslation(self->dialogbutton);
			psy_ui_button_settext(self->dialogbutton, "...");
		} else if (psy_property_type(self->property) == PSY_PROPERTY_TYPE_FONT) {
			self->dialogbutton = psy_ui_button_allocinit(&self->col2, view);
			psy_ui_component_setalign(&self->dialogbutton->component, psy_ui_ALIGN_CLIENT);
			psy_ui_button_settext(self->dialogbutton, "settingsview.choose-font");
		} else if (psy_property_hint(property) == PSY_PROPERTY_HINT_SHORTCUT) {
			self->dialogbutton = psy_ui_button_allocinit(&self->col2, view);
			psy_ui_component_setalign(&self->dialogbutton->component,
				psy_ui_ALIGN_CLIENT);			
			psy_ui_button_settext(self->dialogbutton, "settingsview.none");
		} else if (psy_property_int_hasrange(property) && !psy_property_readonly(property)) {
			char text[256];
			psy_ui_Label* label;

			// todo: psy_ui_translate translates only at build
			// use onlanguageupdate when the language is later changed in the
			// settingsview
			psy_snprintf(text, 256, "%s %d %s %d",
				psy_ui_translate("settingsview.from"),
				property->item.min,
				psy_ui_translate("settingsview.to"),
				property->item.max);
			label = psy_ui_label_allocinit(&self->col2, view);			
			psy_ui_component_setalign(&label->component, psy_ui_ALIGN_CLIENT);
			psy_ui_label_preventtranslation(label);
			psy_ui_label_settext(label, text);
		}
	}
	propertiesrenderline_update(self);
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
	self->state->line = self;
	if (psy_ui_component_visible(psy_ui_edit_base(self->state->edit))) {
		return;
	}
	if (ev->button == 1) {
		self->state->dialogbutton = (self->dialogbutton &&
			ev->target == &self->dialogbutton->component);			
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
		return;
	}
	psy_ui_mouseevent_stoppropagation(ev);
}

bool propertiesrenderline_updatecheck(PropertiesRenderLine* self)
{
	if (self->check && psy_property_type(self->property) ==
			PSY_PROPERTY_TYPE_BOOL) {
		if (psy_property_item_bool(self->property)) {
			psy_ui_switch_check(self->check);
		} else {
			psy_ui_switch_uncheck(self->check);
		}
		return TRUE;
	} else if (self->check && psy_property_ischoiceitem(self->property)) {
		if (psy_property_at_choice(psy_property_parent(self->property)) ==
				self->property) {
			psy_ui_switch_check(self->check);
		} else {		
			psy_ui_switch_uncheck(self->check);
		}
		return TRUE;
	}
	return FALSE;
}

bool propertiesrenderline_updateintegerlabel(PropertiesRenderLine* self)
{
	char text[64];

	if (self->label && psy_property_type(self->property) == PSY_PROPERTY_TYPE_INTEGER) {
		if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITHEX) {
			psy_snprintf(text, 64, "%X", (int)psy_property_item_int(self->property));
		} else {
			psy_snprintf(text, 64, "%d", (int)psy_property_item_int(self->property));
		}
		psy_ui_label_settext(self->label, text);
		return TRUE;
	}
	return FALSE;
}

bool propertiesrenderline_updatestringlabel(PropertiesRenderLine* self)
{
	if (self->label && psy_property_type(self->property) == PSY_PROPERTY_TYPE_STRING) {
		psy_ui_label_settext(self->label,
			psy_property_item_str(self->property));
		return TRUE;
	}
	return FALSE;
}

bool propertiesrenderline_updatefontlabel(PropertiesRenderLine* self)
{
	if (self->label && psy_property_type(self->property) == PSY_PROPERTY_TYPE_FONT) {
		char str[128];
		psy_ui_FontInfo fontinfo;
		psy_ui_Font font;
		int pt;
		const psy_ui_TextMetric* tm;
				
		psy_ui_fontinfo_init_string(&fontinfo, psy_property_item_str(self->property));
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
		psy_ui_label_settext(self->label, str);
		return TRUE;
	}
	return FALSE;
}

bool propertiesrenderline_updateshortcut(PropertiesRenderLine* self)
{
	if (self->label && psy_property_hint(self->property) == PSY_PROPERTY_HINT_SHORTCUT) {
		char str[256];

		inputdefiner_inputtotext(
			(uint32_t)psy_property_item_int(self->property),
			str);
		psy_ui_label_settext(self->label, str);
		return TRUE;
	}
	return FALSE;
}

bool propertiesrenderline_updatecolour(PropertiesRenderLine* self)
{
	if (self->colour && self->label &&
		psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {
		char str[128];
		
		psy_snprintf(str, 128, "0x%d", psy_property_item_int(self->property));
		psy_ui_label_settext(self->label, str);
		psy_ui_label_setcharnumber(self->label, 40.0);
		psy_ui_component_setbackgroundcolour(self->colour,
			psy_ui_colour_make(psy_property_item_int(self->property)));
		return TRUE;
	}
	return FALSE;
}

void propertiesrenderline_onpreferredsize(PropertiesRenderLine* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (self->numcols == 1) {
		psy_ui_size_setem(rv, 40.0, 1.3);
	} else {
		psy_ui_size_setem(rv, 190.0, 1.3);
	}
}

void propertiesrenderline_update(PropertiesRenderLine* self)
{
	if (self->numcols < 2) {
		return;
	}
	if (propertiesrenderline_updatecheck(self)) {
		psy_ui_component_invalidate(&self->component);
		return;
	}
	if (propertiesrenderline_updatecolour(self)) {
		psy_ui_component_invalidate(&self->component);
		return;
	}
	if (propertiesrenderline_updateshortcut(self)) {
		psy_ui_component_invalidate(&self->component);
		return;
	}
	if (propertiesrenderline_updatefontlabel(self)) {
		psy_ui_component_invalidate(&self->component);
		return;
	}
	if (propertiesrenderline_updatestringlabel(self)) {
		psy_ui_component_invalidate(&self->component);
		return;
	}
	if (propertiesrenderline_updateintegerlabel(self)) {
		psy_ui_component_invalidate(&self->component);
		return;
	}		
}

// PropertiesRenderer
// prototypes
static void propertiesrenderer_ondestroy(PropertiesRenderer*);
static void propertiesrenderer_build(PropertiesRenderer*);
static int propertiesrenderer_onpropertiesbuild(PropertiesRenderer*,
	psy_Property*, uintptr_t level);
void propertiesrenderer_buildsection(PropertiesRenderer*,
	psy_Property* section);
static void propertiesrenderer_onmousedown(PropertiesRenderer*,
	psy_ui_MouseEvent*);
static void propertiesrenderer_oninputdefineraccept(PropertiesRenderer*,
	InputDefiner* sender);
static void propertiesrenderer_oneditkeydown(PropertiesRenderer*,
	psy_ui_Component* sender, psy_ui_KeyEvent* ev);
static void propertiesrenderer_oneditaccept(PropertiesRenderer*,
	psy_ui_Edit* sender);
static void propertiesrenderer_oneditreject(PropertiesRenderer*,
	psy_ui_Edit* sender);
static void propertiesview_gotosection(PropertiesView*, const char* section);
static void propertiesrenderer_showedit(PropertiesRenderer*,
	PropertiesRenderLine*, psy_ui_Component* edit);
// vtable
static psy_ui_ComponentVtable propertiesrenderer_vtable;
static bool propertiesrenderer_vtable_initialized = FALSE;

static void propertiesrenderer_vtable_init(PropertiesRenderer* self)
{
	if (!propertiesrenderer_vtable_initialized) {
		propertiesrenderer_vtable = *(self->component.vtable);
		propertiesrenderer_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			propertiesrenderer_ondestroy;
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
	self->mainsectionstyle = STYLE_PROPERTYVIEW_MAINSECTION;
	self->mainsectionheaderstyle = STYLE_PROPERTYVIEW_MAINSECTIONHEADER;
	self->keystyle = psy_INDEX_INVALID;
	self->keystyle_hover = psy_INDEX_INVALID;
	psy_ui_component_init(&self->component, parent, NULL);
	propertiesrenderer_vtable_init(self);
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);
	psy_ui_component_setwheelscroll(&self->component, 4);	
	psy_ui_component_init(&self->client, &self->component, NULL);
	psy_ui_component_doublebuffer(&self->client);	
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setdefaultalign(&self->client,
		psy_ui_ALIGN_TOP, psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));
	psy_ui_component_init(&self->dummy, &self->component, NULL);
	psy_ui_component_hide(&self->dummy);
	propertiesrenderstate_init(&self->state);
	self->state.dummy = &self->dummy;
	self->state.edit = &self->edit;
	self->keyselected = 0;	
	self->showkeyselection = FALSE;	
	self->valuecolour = psy_ui_colour_make(0x00CACACA);
	self->sectioncolour = psy_ui_colour_make(0x00CACACA);
	self->separatorcolour = psy_ui_colour_make(0x00333333);
	self->valueselcolour = psy_ui_colour_make(0x00FFFFFF);
	self->valueselbackgroundcolour = psy_ui_colour_make(0x009B7800);
	psy_ui_edit_init(&self->edit, &self->component);
	psy_ui_edit_enableinputfield(&self->edit);
	psy_signal_connect(&self->edit.component.signal_keydown, self,
		propertiesrenderer_oneditkeydown);
	psy_signal_connect(&self->edit.signal_accept, self,
		propertiesrenderer_oneditaccept);
	psy_signal_connect(&self->edit.signal_reject, self,
		propertiesrenderer_oneditreject);
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

void propertiesrenderer_ondestroy(PropertiesRenderer* self)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_selected);
	psy_table_dispose(&self->sections);
}

void propertiesrenderer_setstyle(PropertiesRenderer* self,
	uintptr_t mainsection,
	uintptr_t mainsectionheader,
	uintptr_t linestyle,
	uintptr_t linestyle_hover)
{
	self->mainsectionstyle = mainsection;
	self->mainsectionheaderstyle = mainsectionheader;
	self->keystyle = linestyle;
	self->keystyle_hover = linestyle_hover;
	propertiesrenderer_build(self);
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
	if (psy_property_hint(property) == PSY_PROPERTY_HINT_HIDE) {
		return 2;
	}
	if (level == 0) {
		propertiesrenderer_buildsection(self, property);
	} else {
		if (!self->currsection) {
			self->currsection = &self->client;
		}
		if (psy_property_hint(property) != PSY_PROPERTY_HINT_HIDE) {
			PropertiesRenderLine* line;

			line = propertiesrenderline_allocinit(self->currsection, &self->client,
				&self->state, property, level, self->numcols);
			psy_ui_component_setstyletype(&line->key.component,
				self->keystyle);
			psy_ui_component_setstyletype_hover(&line->key.component,
				self->keystyle_hover);
		}
	}
	return 1;
}

void propertiesrenderer_buildsection(PropertiesRenderer* self,
	psy_Property* section)
{
	if (psy_property_type(section) == PSY_PROPERTY_TYPE_SECTION) {
		psy_ui_Component* currsection;
		psy_ui_Component* lines;
		psy_ui_Label* label;

		currsection = psy_ui_component_allocinit(&self->client,
			&self->client);		
		psy_ui_component_setdefaultalign(currsection, psy_ui_ALIGN_TOP,
			psy_ui_margin_zero());
		psy_ui_component_setstyletype(currsection,
			self->mainsectionstyle);
		label = psy_ui_label_allocinit(currsection, &self->client);		
		psy_ui_component_setmargin(psy_ui_label_base(label),
			psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));
		psy_ui_component_setspacing(psy_ui_label_base(label),
			psy_ui_margin_make_em(0.5, 0.0, 0.5, 1.0));
		psy_ui_component_setstyletype(psy_ui_label_base(label),
			self->mainsectionheaderstyle);
		lines = psy_ui_component_allocinit(currsection,
			&self->client);
		psy_ui_component_setdefaultalign(lines, psy_ui_ALIGN_TOP,
			psy_ui_margin_zero());
		psy_ui_label_settext(label, psy_property_text(section));
		psy_table_insert_strhash(&self->sections, psy_property_text(section),
			currsection);
		self->currsection = lines;
	}
}

void propertiesrenderer_onmousedown(PropertiesRenderer* self,
	psy_ui_MouseEvent* ev)
{
	if (psy_ui_component_visible(&self->edit.component)) {
		return;
	}
	if (self->state.selected) {		
		psy_signal_emit(&self->signal_selected, self, 1,
			self->state.selected);
	}
	if (self->state.property) {
		self->state.selected = self->state.property;
		if (psy_property_ischoiceitem(self->state.selected)) {
			psy_signal_emit(&self->signal_changed, self, 1,
				self->state.selected);
		} else if (psy_property_type(self->state.selected) == PSY_PROPERTY_TYPE_INTEGER) {
			if (psy_property_hint(self->state.selected) == PSY_PROPERTY_HINT_SHORTCUT) {				
				if (self->state.line) {
					if (self->state.dialogbutton) {
						psy_property_setitem_int(self->state.property, 0);
						psy_signal_emit(&self->signal_changed, self, 1,
							self->state.selected);
					} else {
						inputdefiner_setinput(&self->inputdefiner, (uint32_t)
							psy_property_item_int(self->state.selected));
						propertiesrenderer_showedit(self, self->state.line,
							&self->inputdefiner.component);
					}
				}
			} else if (psy_property_hint(self->state.selected) == PSY_PROPERTY_HINT_EDITCOLOR) {
				psy_ui_ColourDialog colourdialog;

				psy_ui_colourdialog_init(&colourdialog, &self->component);
				psy_ui_colourdialog_setcolour(&colourdialog,
					psy_ui_colour_make((uint32_t)psy_property_item_int(self->state.selected)));
				if (psy_ui_colourdialog_execute(&colourdialog)) {
					psy_ui_Colour colour;

					colour = psy_ui_colourdialog_colour(&colourdialog);
					psy_property_set_int(self->state.selected->parent,
						self->state.selected->item.key,
						colour.value);
					psy_signal_emit(&self->signal_changed, self, 1,
						self->state.selected);
				}
				psy_ui_colourdialog_dispose(&colourdialog);
			} else {
				char text[40];

				if (psy_property_hint(self->state.selected) == PSY_PROPERTY_HINT_EDITHEX) {
					psy_snprintf(text, 40, "%X",
						psy_property_item_int(self->state.selected));
				} else {
					psy_snprintf(text, 40, "%d",
						psy_property_item_int(self->state.selected));
				}
				psy_ui_edit_settext(&self->edit, text);
				propertiesrenderer_showedit(self, self->state.line,
					&self->edit.component);
			}
		} else if (psy_property_hint(self->state.property) == PSY_PROPERTY_HINT_EDITDIR) {
			if (self->state.dialogbutton) {
				psy_ui_FolderDialog dialog;

				psy_ui_folderdialog_init_all(&dialog, 0, psy_ui_translate(
					psy_property_text(self->state.selected)), "");
				if (psy_ui_folderdialog_execute(&dialog)) {
					psy_property_set_str(self->state.selected->parent,
						self->state.selected->item.key, psy_ui_folderdialog_path(&dialog));
					psy_signal_emit(&self->signal_changed, self, 1,
						self->state.selected);
				}
			} else {				
				psy_ui_edit_settext(&self->edit,
					psy_property_item_str(self->state.selected));
				propertiesrenderer_showedit(self, self->state.line,
					&self->edit.component);
			}
		} else if (psy_property_type(self->state.selected) == PSY_PROPERTY_TYPE_FONT) {
			if (self->state.dialogbutton) {
				psy_ui_FontDialog fontdialog;
				psy_ui_FontInfo fontinfo;

				psy_ui_fontdialog_init(&fontdialog, &self->component);
				psy_ui_fontinfo_init_string(&fontinfo,
					psy_property_item_str(self->state.selected));
				psy_ui_fontdialog_setfontinfo(&fontdialog, fontinfo);
				if (psy_ui_fontdialog_execute(&fontdialog)) {
					psy_ui_FontInfo fontinfo;

					fontinfo = psy_ui_fontdialog_fontinfo(&fontdialog);
					psy_property_set_font(self->state.selected->parent,
						self->state.selected->item.key,
						psy_ui_fontinfo_string(&fontinfo));
				}
				psy_ui_fontdialog_dispose(&fontdialog);
				psy_signal_emit(&self->signal_changed, self, 1,
					self->state.selected);
			}
		} else if (psy_property_type(self->state.selected) == PSY_PROPERTY_TYPE_STRING) {
			if (!psy_property_readonly(self->state.selected)) {
				psy_ui_edit_settext(&self->edit, psy_property_item_str(
					self->state.selected));
				propertiesrenderer_showedit(self, self->state.line,
					&self->edit.component);
			}
		} else {
			psy_signal_emit(&self->signal_changed, self, 1,
				self->state.selected);
		}		
	}
	self->state.property = NULL;
}

void propertiesrenderer_showedit(PropertiesRenderer* self,
	PropertiesRenderLine* line, psy_ui_Component* edit)
{	
	psy_ui_RealRectangle colscreenposition;
	psy_ui_RealRectangle screenposition;
	const psy_ui_TextMetric* tm;

	colscreenposition = psy_ui_component_screenposition(&line->col1);
	screenposition = psy_ui_component_screenposition(&self->client);
	tm = psy_ui_component_textmetric(&line->col1);
	psy_ui_component_setposition(edit,
		psy_ui_rectangle_make(
			psy_ui_point_make(
				psy_ui_value_make_px(colscreenposition.left - screenposition.left),
				psy_ui_value_make_px(colscreenposition.top - screenposition.top +
					(colscreenposition.bottom - colscreenposition.top - tm->tmHeight) / 2.0)),
			psy_ui_size_make(
				psy_ui_value_make_px(colscreenposition.right - colscreenposition.left),
				psy_ui_value_make_eh(1.0))));
	psy_ui_component_show(edit);
	psy_ui_component_invalidate(&self->client);
	psy_ui_component_setfocus(edit);
}

void propertiesrenderer_oneditkeydown(PropertiesRenderer* self,
	psy_ui_Component* sender, psy_ui_KeyEvent* ev)
{
	if (!self->state.selected) {
		return;
	}
	if (psy_property_hint(self->state.selected) == PSY_PROPERTY_HINT_EDITHEX) {
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

void propertiesrenderer_oneditaccept(PropertiesRenderer* self,
	psy_ui_Edit* sender)
{
	psy_ui_component_hide(&self->edit.component);
	psy_ui_component_setfocus(&self->component);
	if (self->state.selected) {
		switch (psy_property_type(self->state.selected)) {
		case PSY_PROPERTY_TYPE_INTEGER:
			if (psy_property_hint(self->state.selected) == PSY_PROPERTY_HINT_EDITHEX) {
				psy_property_setitem_int(self->state.selected,
					strtol(psy_ui_edit_text(&self->edit), NULL, 16));
			} else {
				psy_property_setitem_int(self->state.selected,
					atoi(psy_ui_edit_text(&self->edit)));
			}
			break;
		case PSY_PROPERTY_TYPE_STRING:
			psy_property_setitem_str(self->state.selected,
				psy_ui_edit_text(&self->edit));
			break;
		default:
			break;
		}
		psy_signal_emit(&self->signal_changed, self, 1, self->state.selected);		
	}
}

void propertiesrenderer_oneditreject(PropertiesRenderer* self,
	psy_ui_Edit* sender)
{
	psy_ui_component_hide(&self->edit.component);
	psy_ui_component_setfocus(&self->component);
}

void propertiesrenderer_oninputdefineraccept(PropertiesRenderer* self,
	InputDefiner* sender)
{
	if (self->state.selected && psy_property_type(self->state.selected) ==
			PSY_PROPERTY_TYPE_INTEGER) {
		psy_property_setitem_int(self->state.selected,
			inputdefiner_input(&self->inputdefiner));
		psy_ui_component_hide(&self->inputdefiner.component);
		psy_ui_component_invalidate(&self->client);
		psy_signal_emit(&self->signal_changed, self, 1, self->state.selected);
	}	
}

// PropertiesView
// prototypes
static void propertiesview_ondestroy(PropertiesView*, psy_ui_Component* sender);
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
	// psy_ui_Margin tabmargin;

	self->workspace = workspace;	
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_selected);
	psy_signal_connect(&self->component.signal_destroy, self,
		propertiesview_ondestroy);
	psy_signal_connect(&self->component.signal_focus,
		self, propertiesview_onfocus);	
	psy_ui_component_init(&self->viewtabbar, tabbarparent, NULL);	
	psy_signal_connect(&self->component.signal_mousedown,
		self, propertiesview_onmousedown);
	psy_signal_connect(&self->component.signal_mouseup,
		self, propertiesview_onmouseup);	
	propertiesrenderer_init(&self->renderer, &self->component, properties,
		numcols, workspace);
	psy_ui_scroller_init(&self->scroller, &self->renderer.component,
		&self->component, NULL);	
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->renderer.component, psy_ui_ALIGN_HCLIENT);
	psy_signal_connect(&self->component.signal_selectsection, self,
		propertiesview_selectsection);	
	psy_ui_tabbar_init(&self->tabbar, &self->component);
	psy_ui_tabbar_settabalign(&self->tabbar, psy_ui_ALIGN_TOP);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar), psy_ui_ALIGN_RIGHT);	
	// psy_ui_margin_init_em(&tabmargin, 0.0, 1.0, 0.5, 2.0);		
	// psy_ui_tabbar_setdefaulttabmargin(&self->tabbar, tabmargin);
	propertiesview_updatetabbarsections(self);
	psy_signal_connect(&self->renderer.signal_changed, self,
		propertiesview_onpropertiesrendererchanged);
	psy_signal_connect(&self->renderer.signal_selected, self,
		propertiesview_onpropertiesrendererselected);	
	psy_signal_connect(&workspace_player(self->workspace)->eventdrivers.signal_input,
		self, propertiesview_oneventdriverinput);
	psy_signal_connect(&self->tabbar.signal_change, self,
		propertiesview_ontabbarchange);	
	psy_signal_connect(&self->renderer.component.signal_scroll, self,
		propertiesview_onscroll);
	self->preventscrollupdate = FALSE;
}

void propertiesview_ondestroy(PropertiesView* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_selected);
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

void propertiesview_reload(PropertiesView* self)
{
	propertiesrenderer_build(&self->renderer);
	psy_ui_component_align_full(&self->component);
	// psy_ui_component_align(&self->scroller);
	// psy_ui_component_align(&self->component);
	psy_ui_component_updateoverflow(propertiesrenderer_base(&self->renderer));
	//psy_ui_component_invalidate(&self->renderer);
}

void propertiesview_onpropertiesrendererchanged(PropertiesView* self,
	PropertiesRenderer* sender, psy_Property* selected)
{	
	uintptr_t rebuild;
	
	rebuild = FALSE;
	psy_signal_emit(&self->signal_changed, self, 2, selected, &rebuild);
	if (rebuild != FALSE) {
		propertiesview_reload(self);
	} else if (self->renderer.state.line) {
		if (psy_property_ischoiceitem(selected)) {
			psy_ui_Component* lines;
			psy_List* q;
			psy_List* p;

			lines = psy_ui_component_parent(&self->renderer.state.line->component);
			if (lines) {
				q = psy_ui_component_children(lines, psy_ui_NONRECURSIVE);
				for (p = q; p != NULL; p = p->next) {
					PropertiesRenderLine* line;

					line = (PropertiesRenderLine*)p->entry;
					propertiesrenderline_update(line);
				}
				psy_list_free(q);
			}
		} else {
			propertiesrenderline_update(self->renderer.state.line);			
		}		
	}
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
	if (psy_ui_component_visible(&self->renderer.edit.component)) {
		return;
	}
	psy_ui_mouseevent_stoppropagation(ev);
	//psy_ui_component_setfocus(&self->renderer.component);	
}

void propertiesview_onmouseup(PropertiesView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_ui_mouseevent_stoppropagation(ev);
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

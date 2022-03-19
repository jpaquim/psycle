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
void propertiesrenderstate_init(PropertiesRenderState* self, uintptr_t numcols,
	psy_ui_TextInput* edit)
{	
	self->property = self->selected = NULL;
	self->selectedline = NULL;
	self->edit = edit;
	self->dialogbutton = FALSE;
	self->numcols = numcols;	
	self->preventmousepropagation = TRUE;
	self->comboselect = FALSE;
	psy_ui_size_init_em(&self->size_col0, 80.0, 1.3);
	psy_ui_size_init_em(&self->size_col2, 40.0, 1.3);	
}

/* PropertiesRenderLine */
/* prototypes */
static void propertiesrenderline_onmousedown(PropertiesRenderLine*,
	psy_ui_MouseEvent*);
static void propertiesrenderline_oncomboselect(PropertiesRenderLine*,
	psy_ui_Component* sender, intptr_t selidx);
/* vtable */
static psy_ui_ComponentVtable propertiesrenderline_vtable;
static bool propertiesrenderline_vtable_initialized = FALSE;

static void propertiesrenderline_vtable_init(PropertiesRenderLine* self)
{
	if (!propertiesrenderline_vtable_initialized) {
		propertiesrenderline_vtable = *(self->component.vtable);
		propertiesrenderline_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			propertiesrenderline_onmousedown;
		propertiesrenderline_vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(&self->component, &propertiesrenderline_vtable);
}

/* implementation */
void propertiesrenderline_init(PropertiesRenderLine* self,
	psy_ui_Component* parent,
	PropertiesRenderState* state, psy_Property* property,
	uintptr_t level)
{
	psy_ui_component_init(&self->component, parent, NULL);	
	propertiesrenderline_vtable_init(self);
	psy_ui_component_setalign(&self->component, psy_ui_ALIGN_TOP);	
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HEXPAND);
	if (state->numcols == 1) {
		psy_ui_component_setpreferredsize(&self->component, state->size_col0);
	} else {
		psy_ui_component_setpreferredsize(&self->component,
			psy_ui_size_make_em(190.0, 1.5));
	}
	self->property = property;	
	self->check = NULL;
	self->colour = NULL;
	self->label = NULL;
	self->combo = NULL;
	self->dialogbutton = NULL;
	self->state = state;	
	assert(self->property);
	/* column 0 */
	psy_ui_label_init(&self->key, &self->component);	
	psy_ui_component_set_padding(psy_ui_label_base(&self->key),
		psy_ui_margin_make_em(0.0, 0.0, 0.0, psy_min(level, 5.0) * 4.0));
	psy_ui_component_setpreferredsize(psy_ui_label_base(&self->key),
		self->state->size_col0);
	if (state->numcols == 1) {
		psy_ui_component_setalign(&self->key.component, psy_ui_ALIGN_CLIENT);
	} else {
		psy_ui_component_setalign(&self->key.component, psy_ui_ALIGN_LEFT);		
	}	
	if (!property->item.translate) {
		psy_ui_label_preventtranslation(&self->key);
	}
	if (!psy_property_isaction(self->property)) {
		psy_ui_label_settext(&self->key, psy_property_text(self->property));
	}
	/* column 1 */
	if (state->numcols > 1) {
		if (psy_property_hint(property) == PSY_PROPERTY_HINT_COMBO) {
			self->combo = psy_ui_combobox_allocinit(&self->component);
			psy_signal_connect(&self->combo->signal_selchanged, self,
				propertiesrenderline_oncomboselect);
			psy_ui_combobox_setcharnumber(self->combo, 50.0);
			psy_ui_component_setalign(&self->combo->component, psy_ui_ALIGN_LEFT);			
		} else if (psy_property_isbool(self->property) || psy_property_ischoiceitem(
				self->property)) {
			self->check = psy_ui_switch_allocinit(&self->component, NULL);
		} else if (psy_property_isaction(self->property)) {
			psy_ui_Button* button;

			button = psy_ui_button_allocinit(&self->component);
			psy_ui_component_setalign(psy_ui_button_base(button), psy_ui_ALIGN_LEFT);
			psy_ui_button_settext(button, psy_property_text(self->property));			
		} else if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {
			psy_ui_Component* col1;

			col1 = psy_ui_component_allocinit(&self->component, NULL);
			psy_ui_component_setalign(col1, psy_ui_ALIGN_CLIENT);
			psy_ui_component_setalignexpand(col1, psy_ui_HEXPAND);
			self->label = psy_ui_label_allocinit(col1);
			psy_ui_component_setalign(psy_ui_label_base(self->label),
				psy_ui_ALIGN_LEFT);			
			psy_ui_label_setcharnumber(self->label, 20.0);
			self->colour = psy_ui_component_allocinit(col1, NULL);
			psy_ui_component_setalign(self->colour, psy_ui_ALIGN_LEFT);
			psy_ui_component_setpreferredsize(self->colour,
				psy_ui_size_make_em(4.0, 1.0));
			psy_ui_component_setalign(col1, psy_ui_ALIGN_CLIENT);
		} else if (psy_property_hint(property) == PSY_PROPERTY_HINT_SHORTCUT ||
				psy_property_isint(self->property) ||
				psy_property_isstr(self->property) ||
				psy_property_isfont(self->property)) {
			self->label = psy_ui_label_allocinit(&self->component);
			psy_ui_component_setalign(psy_ui_label_base(self->label),
				psy_ui_ALIGN_CLIENT);
		} else {			
			psy_ui_component_setalign(psy_ui_component_allocinit(
				&self->component, NULL), psy_ui_ALIGN_CLIENT);
		}
		if (self->check) {
			psy_ui_component_setalign(&self->check->component, psy_ui_ALIGN_CLIENT);
		}
		if (self->label) {
			psy_ui_label_preventtranslation(self->label);
		}						
	}
	/* column 2 */
	if (state->numcols > 2) {				
		psy_ui_Component* col2;

		col2 = NULL;		
		if (psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITDIR ||
			psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {			
			self->dialogbutton = psy_ui_button_allocinit(&self->component);
			psy_ui_button_preventtranslation(self->dialogbutton);
			psy_ui_button_settext(self->dialogbutton, "...");
			col2 = &self->dialogbutton->component;
		} else if (psy_property_isfont(self->property)) {
			self->dialogbutton = psy_ui_button_allocinit(&self->component);
			col2 = &self->dialogbutton->component;
			psy_ui_button_settext(self->dialogbutton, "settingsview.choose-font");
		} else if (psy_property_hint(property) == PSY_PROPERTY_HINT_SHORTCUT) {
			self->dialogbutton = psy_ui_button_allocinit(&self->component);
			col2 = &self->dialogbutton->component;
			psy_ui_button_settext(self->dialogbutton, "settingsview.none");
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
			psy_ui_label_preventtranslation(label);
			psy_ui_label_settext(label, text);
		}
		if (col2) {
			psy_ui_component_setalign(col2, psy_ui_ALIGN_RIGHT);
			psy_ui_component_setpreferredsize(col2, self->state->size_col2);				
		}
	}
	propertiesrenderline_update(self);
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
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

void propertiesrenderline_onmousedown(PropertiesRenderLine* self,
	psy_ui_MouseEvent* ev)
{
	self->state->property = NULL;	
	if (psy_ui_component_visible(psy_ui_textinput_base(self->state->edit))) {
		return;
	}	
	if (self->combo) {
		psy_ui_mouseevent_stop_propagation(ev);
		return;
	}
	if (psy_ui_mouseevent_button(ev) == 1) {
		self->state->dialogbutton = (self->dialogbutton &&
			psy_ui_event_target(&ev->event) == &self->dialogbutton->component);
		self->state->property = NULL;
		self->state->selected = self->property;
		if (self->state->selectedline) {
			psy_ui_component_removestylestate(
				&self->state->selectedline->component,
				psy_ui_STYLESTATE_SELECT);
		}
		self->state->selectedline = self;		
		psy_ui_component_addstylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
		if (psy_property_readonly(self->property)) {
			/* bubble to propertiesrenderer_onmousedown */
			return;
		}
		self->state->property = self->property;
		if (psy_property_ischoiceitem(self->property)) {
			psy_property_setitem_int(psy_property_parent(self->property),
				psy_property_choiceitem_index(self->property));
		} else if (psy_property_isbool(self->property)) {
			psy_property_setitem_bool(self->property,
				!psy_property_item_bool(self->property));			
			propertiesrenderline_updatecheck(self);
		}
		/* bubble to propertiesrenderer_onmousedown */
		return;
	}
	if (self->state->preventmousepropagation) {
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void propertiesrenderline_oncomboselect(PropertiesRenderLine* self,
	psy_ui_Component* sender, intptr_t selidx)
{
	psy_property_setitem_int(self->property, selidx);
	self->state->property = NULL;
	self->state->selectedline = self;
	self->state->selected = psy_property_at_index(self->property, selidx);
	self->state->property = self->state->selected;
	self->state->comboselect = TRUE;
}

bool propertiesrenderline_updatecheck(PropertiesRenderLine* self)
{
	bool checked;
	
	if (!self->check&& self->property) {
		return FALSE;
	}
	if (psy_property_isbool(self->property)) {
		checked = psy_property_item_bool(self->property);
	} else if (psy_property_ischoiceitem(self->property)) {
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

bool propertiesrenderline_updateintegerlabel(PropertiesRenderLine* self)
{
	if (self->label && psy_property_isint(self->property)) {
		char text[64];
		const char* mask;
		
		if (psy_property_ishex(self->property)) {
			mask = "%X";			
		} else {
			mask = "%d";			
		}		
		psy_snprintf(text, 64, mask, (int)psy_property_item_int(
			self->property));
		psy_ui_label_settext(self->label, text);
		return TRUE;
	}
	return FALSE;
}

bool propertiesrenderline_updatestringlabel(PropertiesRenderLine* self)
{
	if (self->label && psy_property_isstr(self->property)) {
		psy_ui_label_settext(self->label,
			psy_property_item_str(self->property));
		return TRUE;
	}
	return FALSE;
}

bool propertiesrenderline_updatefontlabel(PropertiesRenderLine* self)
{
	if (self->label && psy_property_isfont(self->property)) {
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
		psy_ui_label_settext(self->label, str);
		return TRUE;
	}
	return FALSE;
}

bool propertiesrenderline_updateshortcut(PropertiesRenderLine* self)
{
	if (self->label && psy_property_hint(self->property) ==
			PSY_PROPERTY_HINT_SHORTCUT) {
		char str[256];

		inputdefiner_inputtotext((uint32_t)
			psy_property_item_int(self->property), str);
		psy_ui_label_settext(self->label, str);
		return TRUE;
	}
	return FALSE;
}

bool propertiesrenderline_updatecolour(PropertiesRenderLine* self)
{
	if (self->colour && self->label &&
		psy_property_hint(self->property) == PSY_PROPERTY_HINT_EDITCOLOR) {
		char str[64];
		
		psy_snprintf(str, 64, "0x%d", psy_property_item_int(self->property));
		psy_ui_label_settext(self->label, str);		
		psy_ui_component_setbackgroundcolour(self->colour,
			psy_ui_colour_make((uint32_t)psy_property_item_int(
				self->property)));
		return TRUE;
	}
	return FALSE;
}

void propertiesrenderline_update(PropertiesRenderLine* self)
{
	if (self->state->numcols < 2) {
		return;
	}
	if (propertiesrenderline_updatecheck(self)) {		
		return;
	}
	if (propertiesrenderline_updatecolour(self)) {		
		return;
	}
	if (propertiesrenderline_updateshortcut(self)) {		
		return;
	}
	if (propertiesrenderline_updatefontlabel(self)) {		
		return;
	}
	if (propertiesrenderline_updatestringlabel(self)) {		
		return;
	}
	if (propertiesrenderline_updateintegerlabel(self)) {		
		return;
	}		
}

/* PropertiesRenderer */
/* prototypes */
static void propertiesrenderer_ondestroy(PropertiesRenderer*);
static void propertiesrenderer_ontimer(PropertiesRenderer*, uintptr_t timerid);
static int propertiesrenderer_onpropertiesbuild(PropertiesRenderer*,
	psy_Property*, uintptr_t level);
static void propertiesrenderer_buildmainsection(PropertiesRenderer*,
	psy_Property* section);
static void propertiesrenderer_onmousedown(PropertiesRenderer*,
	psy_ui_MouseEvent*);
static void propertiesrenderer_oninputdefineraccept(PropertiesRenderer*,
	InputDefiner* sender);
static void propertiesrenderer_oneditkeydown(PropertiesRenderer*,
	psy_ui_Component* sender, psy_ui_KeyboardEvent* ev);
static void propertiesrenderer_oneditaccept(PropertiesRenderer*,
	psy_ui_TextInput* sender);
static void propertiesrenderer_oneditreject(PropertiesRenderer*,
	psy_ui_TextInput* sender);
static void propertiesview_gotosection(PropertiesView*, uintptr_t index);
static void propertiesrenderer_checkdialog(PropertiesRenderer*,
	psy_Property* selected);
static void propertiesrenderer_checkedit(PropertiesRenderer*,
	psy_Property* selected);
static void propertiesrenderer_showedit(PropertiesRenderer*,
	PropertiesRenderLine*, psy_ui_Component* edit);
/* vtable */
static psy_ui_ComponentVtable propertiesrenderer_vtable;
static bool propertiesrenderer_vtable_initialized = FALSE;

static void propertiesrenderer_vtable_init(PropertiesRenderer* self)
{
	if (!propertiesrenderer_vtable_initialized) {
		propertiesrenderer_vtable = *(self->component.vtable);
		propertiesrenderer_vtable.ondestroy =
			(psy_ui_fp_component_event)
			propertiesrenderer_ondestroy;
		propertiesrenderer_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			propertiesrenderer_onmousedown;
		propertiesrenderer_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			propertiesrenderer_ontimer;			
		propertiesrenderer_vtable_initialized = TRUE;
	}
	self->component.vtable = &propertiesrenderer_vtable;
}
/* implementation */
void propertiesrenderer_init(PropertiesRenderer* self,
	psy_ui_Component* parent, psy_Property* properties, uintptr_t numcols)
{
	self->properties = properties;	
	self->curr = NULL;
	self->combolevel = psy_INDEX_INVALID;
	self->comboline = NULL;
	self->mainsectionstyle = STYLE_PROPERTYVIEW_MAINSECTION;
	self->mainsectionheaderstyle = STYLE_PROPERTYVIEW_MAINSECTIONHEADER;
	self->keystyle = psy_INDEX_INVALID;
	self->keystyle_hover = psy_INDEX_INVALID;
	self->linestyle_select = psy_INDEX_INVALID;
	self->rebuild_level = 0;
	psy_ui_component_init(&self->component, parent, NULL);
	propertiesrenderer_vtable_init(self);	
	psy_ui_component_setwheelscroll(&self->component, 4);	
	psy_ui_component_init(&self->client, &self->component, NULL);	
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setdefaultalign(&self->client,
		psy_ui_ALIGN_TOP, psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));	
	propertiesrenderstate_init(&self->state, numcols, &self->edit);	
	psy_ui_textinput_init(&self->edit, &self->component);
	psy_ui_textinput_enableinputfield(&self->edit);
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
	psy_ui_component_starttimer(&self->component, 0, 100);
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
	self->comboline = NULL;
	self->combolevel = psy_INDEX_INVALID;
	self->rebuild_level = 0;
	self->state.selected = NULL;
	self->state.selectedline = NULL;
	psy_table_clear(&self->sections);
	psy_ui_component_clear(&self->client);
	psy_property_enumerate(self->properties, self,
		(psy_PropertyCallback)
		propertiesrenderer_onpropertiesbuild);
	if (self->combolevel != psy_INDEX_INVALID) {		
		psy_ui_combobox_setcursel(self->comboline->combo,
			psy_property_item_int(self->comboline->property));
		self->combolevel = psy_INDEX_INVALID;
		self->comboline = NULL;
	}
}

void propertiesrenderer_rebuild(PropertiesRenderer* self,
	psy_Property* mainsection)
{
	psy_ui_Component* section;

	if (!mainsection) {
		return;
	}
	self->state.selected = NULL;
	self->state.selectedline = NULL;
	section = (psy_ui_Component*)psy_table_at(&self->sections,
		psy_property_index(mainsection));
	if (section) {
		psy_ui_Component* clients;

		clients = psy_ui_component_at(section, 1);
		if (clients) {
			psy_ui_component_clear(clients);
			self->rebuild_level = 1;
			self->curr = clients;
			self->comboline = NULL;
			self->combolevel = psy_INDEX_INVALID;
			psy_property_enumerate(mainsection, self, (psy_PropertyCallback)
				propertiesrenderer_onpropertiesbuild);
			psy_ui_component_align(clients);
			psy_ui_component_align_full(&self->component);
			psy_ui_component_invalidate(section);
			self->rebuild_level = 0;
		}		
	}
}

void propertiesrenderer_updateline(PropertiesRenderer* self,
	PropertiesRenderLine* line)
{
	if (!line) {
		return;
	}
	if (psy_property_ischoiceitem(line->property)) {
		psy_ui_Component* lines;
		psy_List* q;
		psy_List* p;

		lines = psy_ui_component_parent(&line->component);
		if (lines) {
			q = psy_ui_component_children(lines, psy_ui_NONRECURSIVE);
			for (p = q; p != NULL; p = p->next) {
				propertiesrenderline_update((PropertiesRenderLine*)p->entry);
			}
			psy_list_free(q);
		}
		return;
	}
	propertiesrenderline_update(line);
}

int propertiesrenderer_onpropertiesbuild(PropertiesRenderer* self,
	psy_Property* property, uintptr_t level)
{
	if (psy_property_hint(property) == PSY_PROPERTY_HINT_HIDE) {
		return 2;
	}	
	if (self->rebuild_level == 0 && level == 0) {
		propertiesrenderer_buildmainsection(self, property);
	} else {
		if (!self->curr) {
			self->curr = &self->client;
		}
		if (self->combolevel != psy_INDEX_INVALID) {
			if (level != self->combolevel + 1) {				
				psy_ui_combobox_setcursel(self->comboline->combo,
					psy_property_item_int(self->comboline->property));
				self->combolevel = psy_INDEX_INVALID;				
				self->comboline = NULL;
			} else {
				if (self->comboline && self->comboline->combo) {
					if (psy_property_translation_prevented(property)) {
						psy_ui_combobox_addtext(self->comboline->combo,
							psy_property_text(property));
					} else {
						psy_ui_combobox_addtext(self->comboline->combo,
							psy_ui_translate(psy_property_text(property)));
					}
					return 1;
				}
			}
		}
		if (psy_property_hint(property) != PSY_PROPERTY_HINT_HIDE) {
			PropertiesRenderLine* line;
						
			line = propertiesrenderline_allocinit(self->curr,
				&self->state, property, level + self->rebuild_level);			
			psy_ui_component_setstyletype(&line->key.component,
				self->keystyle);
			psy_ui_component_setstyletype_hover(&line->key.component,
				self->keystyle_hover);
			psy_ui_component_setstyletype_select(&line->component,
				self->linestyle_select);
			if (psy_property_hint(property) == PSY_PROPERTY_HINT_COMBO) {				
				self->combolevel = level;
				self->comboline = line;
			}
		}
	}
	return 1;
}

void propertiesrenderer_buildmainsection(PropertiesRenderer* self,
	psy_Property* section)
{
	if (psy_property_issection(section)) {
		psy_ui_Component* currsection;
		psy_ui_Component* lines;
		psy_ui_Label* label;

		currsection = psy_ui_component_allocinit(&self->client, NULL);
		psy_ui_component_setdefaultalign(currsection, psy_ui_ALIGN_TOP,
			psy_ui_margin_zero());		
		psy_ui_component_setstyletype(currsection, self->mainsectionstyle);
		label = psy_ui_label_allocinit(currsection);		
		psy_ui_component_set_margin(psy_ui_label_base(label),
			psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));
		psy_ui_component_set_padding(psy_ui_label_base(label),
			psy_ui_margin_make_em(0.5, 0.0, 0.5, 1.0));
		psy_ui_component_setstyletype(psy_ui_label_base(label),
			self->mainsectionheaderstyle);		
		lines = psy_ui_component_allocinit(currsection, NULL);
		psy_ui_component_setdefaultalign(lines, psy_ui_ALIGN_TOP,
			psy_ui_margin_zero());
		psy_ui_label_settext(label, psy_property_text(section));
		psy_table_insert(&self->sections, psy_property_index(section),
			currsection);
		self->curr = lines;
	}
}

void propertiesrenderer_ontimer(PropertiesRenderer* self, uintptr_t timerid)
{
	if (self->state.comboselect) {
		psy_signal_emit(&self->signal_changed, self, 1, self->state.selected);
		self->state.comboselect = FALSE;
	}
}

void propertiesrenderer_onmousedown(PropertiesRenderer* self,
	psy_ui_MouseEvent* ev)
{
	psy_Property* selected;

	if (psy_ui_mouseevent_button(ev) != 1) {
		if (self->state.preventmousepropagation) {
			psy_ui_mouseevent_stop_propagation(ev);
		}
		return;
	}
	selected = self->state.selected;
	if (!selected || psy_ui_component_visible(psy_ui_textinput_base(&self->edit))) {
		if (self->state.preventmousepropagation) {
			psy_ui_mouseevent_stop_propagation(ev);
		}
		return;
	}	
	psy_signal_emit(&self->signal_selected, self, 1, selected);
	if (!self->state.property || psy_property_readonly(selected)) {
		if (self->state.preventmousepropagation) {
			psy_ui_mouseevent_stop_propagation(ev);
		}
		return;
	}
	if (psy_property_isaction(selected) || psy_property_isbool(selected) ||
			psy_property_ischoiceitem(selected)) {
		psy_signal_emit(&self->signal_changed, self, 1, selected);
		if (self->state.preventmousepropagation) {
			psy_ui_mouseevent_stop_propagation(ev);
		}
		return;
	}
	if (self->state.dialogbutton) {
		propertiesrenderer_checkdialog(self, selected);				
	} else {
		propertiesrenderer_checkedit(self, selected);
	}
	if (self->state.preventmousepropagation) {
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void propertiesrenderer_checkdialog(PropertiesRenderer* self,
	psy_Property* selected)
{
	switch (psy_property_hint(selected)) {
	case PSY_PROPERTY_HINT_SHORTCUT:
		psy_property_setitem_int(selected, 0);
		psy_signal_emit(&self->signal_changed, self, 1, selected);
		break;
	case PSY_PROPERTY_HINT_EDITCOLOR: {
		psy_ui_ColourDialog dialog;

		psy_ui_colourdialog_init(&dialog, &self->component);
		psy_ui_colourdialog_setcolour(&dialog,
			psy_ui_colour_make((uint32_t)psy_property_item_int(selected)));
		if (psy_ui_colourdialog_execute(&dialog)) {
			psy_ui_Colour colour;

			colour = psy_ui_colourdialog_colour(&dialog);
			psy_property_setitem_int(selected, psy_ui_colour_colorref(&colour));
			psy_signal_emit(&self->signal_changed, self, 1, selected);
		}
		psy_ui_colourdialog_dispose(&dialog);
		break; }
	case PSY_PROPERTY_HINT_EDITDIR: {
		psy_ui_FolderDialog dialog;

		psy_ui_folderdialog_init_all(&dialog, 0, psy_ui_translate(
			psy_property_text(selected)), "");
		if (psy_ui_folderdialog_execute(&dialog)) {
			psy_property_setitem_str(selected,
				psy_ui_folderdialog_path(&dialog));
			psy_signal_emit(&self->signal_changed, self, 1, selected);
		}
		break; }
	default:
		if (psy_property_type(selected) == PSY_PROPERTY_TYPE_FONT) {
			psy_ui_FontDialog dialog;
			psy_ui_FontInfo fontinfo;

			psy_ui_fontdialog_init(&dialog, &self->component);
			psy_ui_fontinfo_init_string(&fontinfo,
				psy_property_item_str(selected));
			psy_ui_fontdialog_setfontinfo(&dialog, fontinfo);
			if (psy_ui_fontdialog_execute(&dialog)) {
				psy_ui_FontInfo fontinfo;
				char fontstr[256];

				fontinfo = psy_ui_fontdialog_fontinfo(&dialog);
				psy_ui_fontinfo_string(&fontinfo, fontstr, 256);
				psy_property_setitem_font(selected, fontstr);
				psy_signal_emit(&self->signal_changed, self, 1, selected);
			}
			psy_ui_fontdialog_dispose(&dialog);
		}
		break;
	}
}

void propertiesrenderer_checkedit(PropertiesRenderer* self,
	psy_Property* selected)
{
	switch (psy_property_type(selected)) {
	case PSY_PROPERTY_TYPE_INTEGER:
		if (psy_property_hint(selected) == PSY_PROPERTY_HINT_SHORTCUT) {
			if (self->state.selectedline) {
				inputdefiner_setinput(&self->inputdefiner, (uint32_t)
					psy_property_item_int(selected));
				propertiesrenderer_showedit(self, self->state.selectedline,
					&self->inputdefiner.component);
			}
		} else {
			char text[64];

			psy_snprintf(text, 40,
				(psy_property_ishex(selected)) ? "%X" : "%d",
				(int)psy_property_item_int(selected));
			psy_ui_textinput_settext(&self->edit, text);
			propertiesrenderer_showedit(self, self->state.selectedline,
				psy_ui_textinput_base(&self->edit));
		}
		break;
	case PSY_PROPERTY_TYPE_STRING:
		psy_ui_textinput_settext(&self->edit, psy_property_item_str(selected));
		propertiesrenderer_showedit(self, self->state.selectedline,
			psy_ui_textinput_base(&self->edit));
		break;
	default:
		break;
	}
}

void propertiesrenderer_showedit(PropertiesRenderer* self,
	PropertiesRenderLine* line, psy_ui_Component* edit)
{
	if (line && edit) {
		psy_ui_RealRectangle lineposition;
		psy_ui_RealRectangle clientposition;
		psy_ui_RealRectangle editposition;
		const psy_ui_TextMetric* tm;
		
		lineposition = psy_ui_component_screenposition(&line->component);		
		clientposition = psy_ui_component_screenposition(&self->client);
		tm = psy_ui_component_textmetric(&line->component);
		psy_ui_realrectangle_init_all(&editposition,
			psy_ui_realpoint_make(
				lineposition.left + tm->tmAveCharWidth * 80.0 -
					clientposition.left,
				lineposition.top - clientposition.top +
				(lineposition.bottom - lineposition.top - tm->tmHeight) / 2.0),
			psy_ui_realsize_make(tm->tmAveCharWidth * 80.0, tm->tmHeight));
		psy_ui_component_setposition(edit,
			psy_ui_rectangle_make_px(&editposition));
		psy_ui_component_show(edit);
		psy_ui_component_setfocus(edit);
	}
}

void propertiesrenderer_oneditkeydown(PropertiesRenderer* self,
	psy_ui_Component* sender, psy_ui_KeyboardEvent* ev)
{	
	if (self->state.selected && psy_property_ishex(self->state.selected)) {
		if ((psy_ui_keyboardevent_keycode(ev) >= psy_ui_KEY_DIGIT0 && psy_ui_keyboardevent_keycode(ev) <= psy_ui_KEY_DIGIT9) ||
			(psy_ui_keyboardevent_keycode(ev) >= psy_ui_KEY_A && psy_ui_keyboardevent_keycode(ev) <= psy_ui_KEY_F) ||
			(psy_ui_keyboardevent_keycode(ev) < psy_ui_KEY_HELP)) {
			return;
		}
		psy_ui_keyboardevent_prevent_default(ev);		
	}
	psy_ui_keyboardevent_stop_propagation(ev);
}

void propertiesrenderer_oneditaccept(PropertiesRenderer* self,
	psy_ui_TextInput* sender)
{
	psy_ui_component_hide(psy_ui_textinput_base(sender));
	psy_ui_component_setfocus(&self->component);
	if (self->state.selected) {
		if (psy_property_isint(self->state.selected)) {
			psy_property_setitem_int(self->state.selected,
				(psy_property_ishex(self->state.selected))
				? strtol(psy_ui_textinput_text(&self->edit), NULL, 16)
				: atoi(psy_ui_textinput_text(&self->edit)));
		} else if (psy_property_isstr(self->state.selected)) {
			psy_property_setitem_str(self->state.selected,
				psy_ui_textinput_text(&self->edit));
		}
		psy_signal_emit(&self->signal_changed, self, 1, self->state.selected);
	}
}

void propertiesrenderer_oneditreject(PropertiesRenderer* self,
	psy_ui_TextInput* sender)
{
	psy_ui_component_hide(psy_ui_textinput_base(sender));
	psy_ui_component_setfocus(&self->component);
}

void propertiesrenderer_oninputdefineraccept(PropertiesRenderer* self,
	InputDefiner* sender)
{
	if (self->state.selected && psy_property_isint(self->state.selected)) {
		psy_property_setitem_int(self->state.selected,
			inputdefiner_input(&self->inputdefiner));
		psy_ui_component_hide(&self->inputdefiner.component);
		psy_signal_emit(&self->signal_changed, self, 1, self->state.selected);
	}
}

/* PropertiesView */
/* prototypes */
static void propertiesview_ondestroy(PropertiesView*);
static void propertiesview_selectsection(PropertiesView*,
	psy_ui_Component* sender, uintptr_t section, uintptr_t options);
static void propertiesview_updatetabbarsections(PropertiesView*);
static void propertiesview_ontabbarchange(PropertiesView*,
	psy_ui_Component* sender, uintptr_t tabindex);
static void propertiesview_onpropertiesrendererchanged(PropertiesView*,
	PropertiesRenderer* sender, psy_Property*);
static void propertiesview_onpropertiesrendererselected(PropertiesView*,
	PropertiesRenderer* sender, psy_Property*);
static bool propertiesview_oninput(PropertiesView*, InputHandler* sender);
static double propertiesview_checkrange(PropertiesView*, double position);
static void propertiesview_onmousedown(PropertiesView*, psy_ui_MouseEvent*);
static void propertiesview_onmouseup(PropertiesView*, psy_ui_MouseEvent*);
static void propertiesview_onscrollpanealign(PropertiesView*,
	psy_ui_Component* sender);
/* vtable */
static psy_ui_ComponentVtable propertiesview_vtable;
static bool propertiesview_vtable_initialized = FALSE;

static void propertiesview_vtable_init(PropertiesView* self)
{
	if (!propertiesview_vtable_initialized) {
		propertiesview_vtable = *(self->component.vtable);
		propertiesview_vtable.ondestroy =
			(psy_ui_fp_component_event)
			propertiesview_ondestroy;
		propertiesview_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			propertiesview_onmouseup;
		propertiesview_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			propertiesview_onmouseup;
	}
	self->component.vtable = &propertiesview_vtable;
}

/* implementation */
void propertiesview_init(PropertiesView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent,
	psy_Property* properties, uintptr_t numcols, Workspace* workspace)
{		
	psy_ui_component_init(&self->component, parent, NULL);
	propertiesview_vtable_init(self);
	self->maximizemainsections = TRUE;
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_selected);
	psy_ui_component_init(&self->viewtabbar, tabbarparent, NULL);	
	propertiesrenderer_init(&self->renderer, &self->component, properties,
		numcols);
	psy_ui_scroller_init(&self->scroller, &self->renderer.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_margin(&self->scroller.pane,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));
	psy_ui_component_set_margin(&self->scroller.component,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));
	psy_ui_component_setalign(&self->renderer.component, psy_ui_ALIGN_HCLIENT);
	psy_signal_connect(&self->component.signal_selectsection, self,
		propertiesview_selectsection);	
	psy_ui_tabbar_init(&self->tabbar, &self->component);	
	psy_ui_tabbar_settabalign(&self->tabbar, psy_ui_ALIGN_TOP);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_RIGHT);	
	propertiesview_updatetabbarsections(self);
	psy_signal_connect(&self->renderer.signal_changed, self,
		propertiesview_onpropertiesrendererchanged);
	psy_signal_connect(&self->renderer.signal_selected, self,
		propertiesview_onpropertiesrendererselected);	
	inputhandler_connect(&workspace->inputhandler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_CMD, "tracker", psy_INDEX_INVALID,
		self, (fp_inputhandler_input) propertiesview_oninput);
	psy_signal_connect(&self->tabbar.signal_change, self,
		propertiesview_ontabbarchange);
	psy_signal_connect(&self->scroller.pane.signal_beforealign, self,
		propertiesview_onscrollpanealign);
}

void propertiesview_ondestroy(PropertiesView* self)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_selected);
}

void propertiesview_selectsection(PropertiesView* self,
	psy_ui_Component* sender, uintptr_t section, uintptr_t options)
{			
	psy_ui_tabbar_select(&self->tabbar, section);
}

void propertiesview_updatetabbarsections(PropertiesView* self)
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
				psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());
			}
		}
	}	
	psy_ui_tabbar_select(&self->tabbar, 0);
}

void propertiesview_ontabbarchange(PropertiesView* self,
	psy_ui_Component* sender, uintptr_t tabindex)
{		
	propertiesview_gotosection(self, tabindex);	
}

void propertiesview_gotosection(PropertiesView* self, uintptr_t index)
{
	psy_ui_Component* section;

	section = (psy_ui_Component*)psy_table_at(&self->renderer.sections, index);
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
	psy_Property* mainsection;
	uintptr_t rebuild_level;	
	
	if (!selected) {
		return;
	}	
	mainsection = psy_property_parent_level(selected, 1);	
	rebuild_level = psy_INDEX_INVALID;
	psy_signal_emit(&self->signal_changed, self, 2, selected, &rebuild_level);
	if (rebuild_level == psy_INDEX_INVALID) {
		propertiesrenderer_updateline(&self->renderer,
			self->renderer.state.selectedline);
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
	psy_ui_Component* line;
	psy_ui_Component* section;
	uintptr_t idx;

	line = NULL;	
	if (property) {
		/* todo: this works only for one section (recentview)
				 add a line to property map to find any property */
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
	/* avoid downcast */
	self->renderer.state.selectedline = (PropertiesRenderLine*)line;
	if (property) {
		psy_signal_emit(&self->signal_selected, self, 1, property);
	}
	if (self->renderer.state.selectedline) {
		psy_ui_component_addstylestate(
			&self->renderer.state.selectedline->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

void propertiesview_mark(PropertiesView* self, psy_Property* property)
{
	psy_signal_preventall(&self->signal_selected);
	propertiesview_select(self, property);
	psy_signal_enableall(&self->signal_selected);
}

psy_Property* propertiesview_selected(PropertiesView* self)
{
	return self->renderer.state.selected;
}

void propertiesview_enablemousepropagation(PropertiesView* self)
{
	self->renderer.state.preventmousepropagation = FALSE;
}

void propertiesview_prevent_maximize_mainsections(PropertiesView* self)
{
	self->maximizemainsections = FALSE;
}

void propertiesview_onpropertiesrendererselected(PropertiesView* self,
	PropertiesRenderer* sender, psy_Property* selected)
{
	psy_signal_emit(&self->signal_selected, self, 1, selected);
}

bool propertiesview_oninput(PropertiesView* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;
	double step;
	double top;
	double newtop;	

	cmd = inputhandler_cmd(sender);
	if (cmd.id == -1) {
		return 0;
	}					
	step = psy_ui_component_scrollstep_height_px(
		&self->renderer.component);
	top = psy_ui_component_scrolltop_px(&self->renderer.component);
	newtop = -1.0;	
	switch (cmd.id) {
	case CMD_NAVTOP:
		newtop = 0.0;		
		break;
	case CMD_NAVBOTTOM:
		newtop = propertiesview_checkrange(self, (double)INT32_MAX);
		break;
	case CMD_NAVUP:
		newtop = psy_max(0, top - step);
		break;
	case CMD_NAVDOWN:					
		newtop = propertiesview_checkrange(self, top + step);
		break;
	case CMD_NAVPAGEUP:					
		newtop = psy_max(0, top - step * 16.0);
		break;
	case CMD_NAVPAGEDOWN:
		newtop = propertiesview_checkrange(self, top + step * 16.0);
		break;
	default:
		break;
	}
	if (newtop != -1.0) {
		psy_ui_component_setscrolltop(&self->renderer.component,
			psy_ui_value_make_px(psy_max(0.0, newtop)));
	}	
	return 1;
}

double propertiesview_checkrange(PropertiesView* self, double position)
{
	intptr_t steps;
	double scrollstepypx;	
	psy_ui_IntPoint scrollrange;
	
	scrollrange = psy_ui_component_verticalscrollrange(
		&self->renderer.component);
	scrollstepypx = psy_ui_component_scrollstep_height_px(
		&self->renderer.component);
	steps = (intptr_t)(position / scrollstepypx);
	steps = psy_min(scrollrange.y, steps);
	return (double)(steps * scrollstepypx);
}

void propertiesview_onmousedown(PropertiesView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_setfocus(&self->component);
	if (self->renderer.state.preventmousepropagation) {		
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void propertiesview_onmouseup(PropertiesView* self, psy_ui_MouseEvent* ev)
{
	if (self->renderer.state.preventmousepropagation) {
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void propertiesview_onscrollpanealign(PropertiesView* self,
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
			psy_ui_component_setminimumsize(component,
				psy_ui_size_make(
					psy_ui_value_make_ew(0.0),
					clientsize.height));
		}
	}
}

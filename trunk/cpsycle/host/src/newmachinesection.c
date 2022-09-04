/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "newmachinesection.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void newmachinesection_on_destroyed(NewMachineSection*);
static void newmachinesection_on_mouse_down(NewMachineSection*,
	psy_ui_MouseEvent*);
static void newmachinesection_on_edit_name_accept(NewMachineSection*, psy_ui_TextArea* sender);
static void newmachinesection_on_drag_over(NewMachineSection*, psy_ui_DragEvent*);
static void newmachinesection_on_drop(NewMachineSection*, psy_ui_DragEvent*);
static void newmachinesection_on_language_changed(NewMachineSection*);

/* vtable */
static psy_ui_ComponentVtable newmachinesection_vtable;
static bool newmachinesection_vtable_initialized = FALSE;

static void newmachinesection_vtable_init(NewMachineSection* self)
{
	if (!newmachinesection_vtable_initialized) {
		newmachinesection_vtable = *(self->component.vtable);
		newmachinesection_vtable.on_destroyed =
			(psy_ui_fp_component)
			newmachinesection_on_destroyed;
		newmachinesection_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			newmachinesection_on_mouse_down;
		newmachinesection_vtable.ondragover =
			(psy_ui_fp_component_on_drag_event)
			newmachinesection_on_drag_over;
		newmachinesection_vtable.ondrop =
			(psy_ui_fp_component_on_drag_event)
			newmachinesection_on_drop;
		newmachinesection_vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			newmachinesection_on_language_changed;
		newmachinesection_vtable_initialized = TRUE;
	}
	self->component.vtable = &newmachinesection_vtable;
}

/* implementation */
void newmachinesection_init(NewMachineSection* self, psy_ui_Component* parent,
	psy_Property* section, PluginFilter* filter,
	psy_audio_PluginCatcher* plugin_catcher)
{
	assert(self);
	assert(plugin_catcher);
	
	psy_ui_component_init(&self->component, parent, NULL);
	newmachinesection_vtable_init(self);	
	psy_signal_init(&self->signal_selected);
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_renamed);
	psy_ui_component_set_style_types(&self->component,
		STYLE_NEWMACHINE_SECTION, psy_INDEX_INVALID,
		STYLE_NEWMACHINE_SECTION_SELECTED, psy_INDEX_INVALID);	
	self->section  = section;		
	self->filter = filter;
	self->plugin_catcher = plugin_catcher;
	psy_ui_component_init(&self->header, &self->component, NULL);
	psy_ui_component_set_align(&self->header, psy_ui_ALIGN_TOP);
	psy_ui_component_set_style_type(&self->header,
		STYLE_NEWMACHINE_SECTION_HEADER);	
	psy_ui_component_set_default_align(&self->header, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_textarea_init_single_line(&self->name, &self->header);	
	psy_ui_textarea_set_text(&self->name, newmachinesection_name(self));
	psy_ui_textarea_enable_input_field(&self->name);
	pluginsview_init(&self->pluginview, &self->component);
	psy_ui_component_set_align(&self->pluginview.component, psy_ui_ALIGN_TOP);
	newmachinesection_find_plugins(self);
	pluginsview_set_filter(&self->pluginview, filter);	
	psy_signal_connect(&self->name.signal_accept, self,
		newmachinesection_on_edit_name_accept);		
}

void newmachinesection_on_destroyed(NewMachineSection* self)
{
	psy_signal_dispose(&self->signal_selected);
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_renamed);
}

NewMachineSection* newmachinesection_alloc(void)
{
	return (NewMachineSection*)malloc(sizeof(NewMachineSection));
}

NewMachineSection* newmachinesection_allocinit(psy_ui_Component* parent,
	psy_Property* property, PluginFilter* filter, psy_audio_PluginCatcher*
		plugin_catcher)
{
	NewMachineSection* rv;

	rv = newmachinesection_alloc();
	if (rv) {
		newmachinesection_init(rv, parent, property, filter, plugin_catcher);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void newmachinesection_find_plugins(NewMachineSection* self)
{
	psy_Property* plugins;
	
	if (self->section) {
		plugins = psy_property_at_section(self->section, "plugins");		
	} else {
		plugins = NULL;
	}
	pluginsview_setplugins(&self->pluginview, plugins);	
}

const char* newmachinesection_key(const NewMachineSection* self)
{
	if (self->section) {		
		return psy_property_key(self->section);
	}
	return "";
}

const char* newmachinesection_name(const NewMachineSection* self)
{
	if (self->section) {
		if (strcmp(psy_property_key(self->section), "all") == 0) {
			return psy_ui_translate("newmachine.all");
		}
		if (strcmp(psy_property_key(self->section), "favorites") == 0) {
			return psy_ui_translate("newmachine.favorites");
		}
		return psy_property_at_str(self->section, "name",
			psy_property_key(self->section));		
	}
	return "";	
}

void newmachinesection_on_language_changed(NewMachineSection* self)
{
	if (self->section) {
		if (strcmp(psy_property_key(self->section), "all") == 0) {
			psy_ui_textarea_set_text(&self->name,
				psy_ui_translate("newmachine.all"));
		} else if (strcmp(psy_property_key(self->section), "favorites") == 0) {
			psy_ui_textarea_set_text(&self->name,
				psy_ui_translate("newmachine.favorites"));
		}
	}
}


void newmachinesection_on_edit_name_accept(NewMachineSection* self, psy_ui_TextArea* sender)
{	
	psy_Property* name;
	
	name = psy_property_at(self->section, "name", PSY_PROPERTY_TYPE_STRING);
	if (name) {
		psy_property_set_item_str(name, psy_ui_textarea_text(sender));			
		psy_signal_emit(&self->signal_renamed, self, 0);
	}	
}

void newmachinesection_on_mouse_down(NewMachineSection* self,
	psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 1) {
		newmachinesection_mark(self);
		psy_signal_emit(&self->signal_selected, self, 0);
	}
}

void newmachinesection_mark(NewMachineSection* self)
{
	psy_ui_Component* parent;
	
	parent = psy_ui_component_parent(&self->component);
	psy_ui_component_removestylestate_children(
		psy_ui_component_parent(&self->component),
		psy_ui_STYLESTATE_SELECT);	
	psy_ui_component_add_style_state(&self->component, psy_ui_STYLESTATE_SELECT);
}

void newmachinsection_clear_selection(NewMachineSection* self)
{
	pluginsview_clearselection(&self->pluginview);
}

void newmachinesection_on_drag_over(NewMachineSection* self, psy_ui_DragEvent* ev)
{	
	if (ev->dataTransfer) {
		psy_List* p;

		for (p = psy_property_begin(ev->dataTransfer); p != NULL; p = p->next) {
			psy_Property* plugin;

			plugin = (psy_Property*)p->entry;
			if (!psy_audio_pluginsections_pluginbyid(
					&self->plugin_catcher->sections, self->section,
					psy_property_key(plugin))) {
				ev->mouse.event.default_prevented_ = TRUE;
				break;
			}
		}
	}
}

void newmachinesection_on_drop(NewMachineSection* self, psy_ui_DragEvent* ev)
{	
	ev->mouse.event.default_prevented_ = TRUE;
	if (ev->dataTransfer) {		
		psy_List* p;
		psy_audio_PluginSections* sections;
		bool drop;

		drop = FALSE;
		sections = &self->plugin_catcher->sections;
		for (p = psy_property_begin(ev->dataTransfer); p != NULL; p = p->next) {
			psy_Property* plugindrag;
			const char* plugid;

			plugindrag = (psy_Property*)p->entry;
			plugid = psy_property_key(plugindrag);
			if (!psy_audio_pluginsections_pluginbyid(sections, self->section, plugid)) {									
				psy_Property* section;
				psy_Property* plugin;

				section = psy_audio_pluginsections_section(sections,
				psy_property_at_str(plugindrag, "section", ""));
				if (section) {
					plugin = psy_audio_pluginsections_pluginbyid(sections, section,
						plugid);
				} else {
					plugin = psy_audio_plugincatcher_at(self->plugin_catcher,
						plugid);
				}
				if (plugin) {
					plugin = psy_audio_pluginsections_add_property(sections,
						self->section, plugin);
					drop = TRUE;
				}
			}			
		}
		if (drop) {			
			newmachinesection_find_plugins(self);
			psy_signal_emit(&self->signal_changed, self, 0);
		}
	}
}

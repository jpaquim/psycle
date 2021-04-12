// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "newmachine.h"
// host
#include "resources/resource.h"
#include "styles.h"
// audio
#include <plugin_interface.h>
// ui
#include <uicolours.h>
// container
#include <qsort.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/strcasestr.h"

static void plugindisplayname(psy_Property*, char* text);
static uintptr_t plugintype(psy_Property*, char* text);
static uintptr_t pluginmode(psy_Property*, char* text);
static psy_Property* search(psy_Property* source, NewMachineFilter*);
static void searchfilter(psy_Property* plugin, NewMachineFilter*,
	psy_Property* parent);

static void newmachinebar_onselectdirectories(NewMachineBar*, psy_ui_Component* sender);

psy_Property* newmachine_sort(psy_Property* source, psy_fp_comp);
psy_Property* newmachine_favorites(psy_Property* source);
static int newmachine_comp_favorite(psy_Property* p, psy_Property* q);
static int newmachine_comp_name(psy_Property* p, psy_Property* q);
static int newmachine_comp_type(psy_Property* p, psy_Property* q);
static int newmachine_comp_mode(psy_Property* p, psy_Property* q);
static int newmachine_isplugin(int type);

// NewMachineFilter
void newmachinefilter_init(NewMachineFilter* self)
{
	self->text = NULL;
	self->gen = TRUE;
	self->effect = TRUE;
	psy_table_init(&self->categories);
	newmachinefilter_setalltypes(self);		
	psy_signal_init(&self->signal_changed);
}

void newmachinefilter_dispose(NewMachineFilter* self)
{	
	psy_table_disposeall(&self->categories, NULL);
	psy_signal_dispose(&self->signal_changed);
	free(self->text);
}

void newmachinefilter_notify(NewMachineFilter* self)
{
	psy_signal_emit(&self->signal_changed, self, 0);
}

void newmachinefilter_reset(NewMachineFilter* self)
{
	self->gen = TRUE;
	self->effect = TRUE;
	newmachinefilter_setalltypes(self);
	psy_strreset(&self->text, "");
	psy_table_disposeall(&self->categories, NULL);
	psy_table_init(&self->categories);
	newmachinefilter_notify(self);
}

void newmachinefilter_settext(NewMachineFilter* self, const char* text)
{
	psy_strreset(&self->text, text);
	newmachinefilter_notify(self);
}

void newmachinefilter_setalltypes(NewMachineFilter* self)
{
	self->effect = TRUE;
	self->intern = TRUE;
	self->native = TRUE;
	self->vst = TRUE;
	self->lua = TRUE;
	self->ladspa = TRUE;
}

void newmachinefilter_cleartypes(NewMachineFilter* self)
{
	self->effect = FALSE;
	self->intern = FALSE;
	self->native = FALSE;
	self->vst = FALSE;
	self->lua = FALSE;
	self->ladspa = FALSE;
}

bool newmachinefilter_all(const NewMachineFilter* self)
{
	return (self->gen && self->effect);
}

void newmachinefilter_addcategory(NewMachineFilter* self, const char* category)
{
	if (psy_strlen(category) > 0) {
		if (!psy_table_exists_strhash(&self->categories, category)) {
			psy_table_insert_strhash(&self->categories, category,
				(void*)psy_strdup(category));
			newmachinefilter_notify(self);
		}
	}
}

void newmachinefilter_removecategory(NewMachineFilter* self, const char* category)
{
	if (psy_strlen(category) > 0) {
		if (psy_table_exists_strhash(&self->categories, category)) {
			char* item;
			
			item = psy_table_at(&self->categories, psy_strhash(category));
			free(item);
			psy_table_remove(&self->categories, psy_strhash(category));
			newmachinefilter_notify(self);
		}
	}
}

void newmachinefilter_anycategory(NewMachineFilter* self)
{
	psy_table_disposeall(&self->categories, NULL);
	psy_table_init(&self->categories);
	newmachinefilter_notify(self);
}

bool newmachinefilter_useanycategory(const NewMachineFilter* self)
{
	return psy_table_size(&self->categories) == 0;	
}

bool newmachinefilter_hascategory(const NewMachineFilter* self,
	const char* category)
{
	if (newmachinefilter_useanycategory(self)) {
		return TRUE;
	}
	if (!category) {
		return FALSE;
	}
	return psy_table_exists_strhash(&self->categories, category);
}

// NewMachineSearch
// prototypes
static void newmachinesearch_oneditfocus(NewMachineSearch*,
	psy_ui_Component* sender);
static void newmachinesearch_oneditchange(NewMachineSearch*,
	psy_ui_Component* sender);
static void newmachinesearch_onaccept(NewMachineSearch*,
	psy_ui_Component* sender);
static void newmachinesearch_onreject(NewMachineSearch*,
	psy_ui_Component* sender);
static void newmachinesearch_reset(NewMachineSearch*);
// implementation
void newmachinesearch_init(NewMachineSearch* self, psy_ui_Component* parent,
	NewMachineFilter* filter)
{
	psy_ui_Margin margin;
	psy_ui_Margin spacing;

	psy_ui_component_init(&self->component, parent, NULL);
	self->filter = filter;
	psy_ui_margin_init_all_em(&spacing, 0.5, 0.5, 0.5, 0.0);
	psy_ui_component_setspacing(&self->component, spacing);
	psy_ui_image_init(&self->image, &self->component);
	psy_ui_image_setbitmapalignment(&self->image,
		psy_ui_ALIGNMENT_CENTER_VERTICAL);
	psy_ui_component_setalign(psy_ui_image_base(&self->image),
		psy_ui_ALIGN_LEFT);	
	psy_ui_component_setpreferredsize(&self->image.component,
		psy_ui_size_make_px(11, 11));
	psy_ui_component_preventalign(psy_ui_image_base(&self->image));
	psy_ui_margin_init_all_em(&margin, 0.0, 1.0, 0.0, 1.0);
	psy_ui_component_setmargin(psy_ui_image_base(&self->image), margin);
	psy_ui_bitmap_loadresource(&self->image.bitmap, IDB_SEARCH_DARK);
	psy_ui_bitmap_settransparency(&self->image.bitmap, psy_ui_colour_make(psy_ui_RGB_WHITE));
	psy_ui_edit_init(&self->edit, &self->component);
	newmachinesearch_reset(self);
	psy_signal_connect(&self->edit.component.signal_focus,
		self, newmachinesearch_oneditfocus);
	psy_signal_connect(&self->edit.signal_change,
		self, newmachinesearch_oneditchange);
	psy_ui_edit_enableinputfield(&self->edit);
	psy_signal_connect(&self->edit.signal_accept,
		self, newmachinesearch_onaccept);
	psy_signal_connect(&self->edit.signal_reject,
		self, newmachinesearch_onreject);
	psy_ui_component_setalign(psy_ui_edit_base(&self->edit),
		psy_ui_ALIGN_CLIENT);	
}

void newmachinesearch_oneditfocus(NewMachineSearch* self, psy_ui_Component* sender)
{
	if (self->hasdefaulttext) {
		psy_ui_edit_settext(&self->edit, "");
	}
}

void newmachinesearch_oneditchange(NewMachineSearch* self, psy_ui_Component* sender)
{
	self->hasdefaulttext = FALSE;
	newmachinefilter_settext(self->filter, psy_ui_edit_text(&self->edit));
}

void newmachinesearch_onaccept(NewMachineSearch* self,
	psy_ui_Component* sender)
{
	if (psy_strlen(psy_ui_edit_text(&self->edit)) == 0) {
		newmachinesearch_reset(self);		
	}	
}

void newmachinesearch_onreject(NewMachineSearch* self,
	psy_ui_Component* sender)
{	
	newmachinesearch_reset(self);	
}

void newmachinesearch_reset(NewMachineSearch* self)
{		
	psy_ui_edit_settext(&self->edit, "Search Plugin");
	newmachinefilter_settext(self->filter, "");
	psy_ui_component_setfocus(psy_ui_component_parent(&self->component));
	self->hasdefaulttext = TRUE;
}

// NewMachineBar
void newmachinebar_init(NewMachineBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_Margin spacing;
					
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_margin_init_all_em(&spacing, 0.0, 1.0, 1.0, 0.5);
	psy_ui_component_setspacing(&self->component, spacing);
	self->workspace = workspace;
	psy_ui_button_init_text(&self->rescan, &self->component, NULL,
		"newmachine.rescan");
	psy_ui_button_setcharnumber(&self->rescan, 30.0);
	psy_ui_button_init_text(&self->selectdirectories, &self->component, NULL,
		"newmachine.select-plugin-directories");
	psy_ui_button_setcharnumber(&self->rescan, 30.0);
	psy_ui_button_init_text(&self->sortbyfavorite, &self->component, NULL,
		"newmachine.sort-by-favorite");
	psy_ui_button_init_text(&self->sortbyname, &self->component, NULL,
		"newmachine.sort-by-name");
	psy_ui_button_init_text(&self->sortbytype, &self->component, NULL,
		"newmachine.sort-by-type");
	psy_ui_button_init_text(&self->sortbymode, &self->component, NULL,
		"newmachine.sort-by-mode");
	psy_ui_button_init_text(&self->createsection, &self->component, NULL,
		"newmachine.create-section");
	psy_ui_button_init_text(&self->addtosection, &self->component, NULL,
		"newmachine.add-to-section");
	psy_ui_button_init_text(&self->removefromsection, &self->component, NULL,
		"newmachine.remove-from-section");
	psy_ui_button_init_text(&self->removesection, &self->component, NULL,
		"newmachine.remove-section");
	psy_signal_connect(&self->selectdirectories.signal_clicked, self,
		newmachinebar_onselectdirectories);	
	psy_ui_margin_init_all_em(&spacing, 0.25, 0.0, 0.25, 0.0);
	psy_ui_component_setspacing_children(&self->component, spacing);
	psy_ui_component_setalign_children(&self->component, psy_ui_ALIGN_TOP);	
}

void newmachinebar_onselectdirectories(NewMachineBar* self, psy_ui_Component* sender)
{
	workspace_selectview(self->workspace, VIEW_ID_SETTINGSVIEW, 3, 0);
}

// NewMachineDetail
static void newmachinedetail_onloadnewblitz(NewMachineDetail*, psy_ui_Component* sender);

void newmachinedetail_init(NewMachineDetail* self, psy_ui_Component* parent,
	NewMachineFilter* filter, Workspace* workspace)
{
	psy_ui_Margin margin;
	psy_ui_Margin spacing;

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_margin_init_all_em(&spacing, 0.5, 0.5, 0.5, 2.0);
	psy_ui_component_setspacing(&self->component, spacing);
	self->workspace = workspace;
	// search
	newmachinesearch_init(&self->search, &self->component, filter);
	psy_ui_component_setalign(&self->search.component, psy_ui_ALIGN_TOP);
	// buttons
	newmachinebar_init(&self->bar, &self->component, workspace);
	psy_ui_component_setalign(&self->bar.component, psy_ui_ALIGN_TOP);
	// plugin name
	labelpair_init_right(&self->plugname, &self->component, "Name", 12.0);
	psy_ui_component_setalign(&self->plugname.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 1.0, 0.0);
	psy_ui_component_setmargin(&self->plugname.component, margin);
	// description
	psy_ui_label_init(&self->desclabel, &self->component, NULL);
	psy_ui_label_settextalignment(&self->desclabel, psy_ui_ALIGNMENT_TOP);
	psy_ui_component_setalign(&self->desclabel.component, psy_ui_ALIGN_CLIENT);	
	psy_ui_component_setalign(&self->dllname.component, psy_ui_ALIGN_BOTTOM);
	// song loading compatibility
	psy_ui_component_init(&self->bottom, &self->component, NULL);
	psy_ui_component_setalign(&self->bottom, psy_ui_ALIGN_BOTTOM);
	psy_ui_margin_init_all_em(&spacing, 0.5, 1.0, 0.5, 0.0);	
	psy_ui_component_setspacing(&self->bottom, spacing);
	psy_ui_checkbox_init_multiline(&self->compatblitzgamefx, &self->bottom);
	psy_ui_checkbox_settext(&self->compatblitzgamefx,
		"newmachine.jme-version-unknown");	
	self->compatblitzgamefx.multiline = TRUE;
	if (compatconfig_loadnewblitz(psycleconfig_compat(
			workspace_conf(self->workspace)))) {
		psy_ui_checkbox_check(&self->compatblitzgamefx);
	}
	psy_signal_connect(&self->compatblitzgamefx.signal_clicked, self,
		newmachinedetail_onloadnewblitz);
	psy_ui_component_setalign(&self->compatblitzgamefx.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_label_init_text(&self->compatlabel, &self->bottom, NULL,
		"newmachine.song-loading-compatibility");
	psy_ui_label_settextalignment(&self->compatlabel, psy_ui_ALIGNMENT_LEFT);
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 0.5, 0.0);
	psy_ui_component_setmargin(psy_ui_label_base(&self->compatlabel), margin);
	psy_ui_component_setalign(&self->compatlabel.component, psy_ui_ALIGN_BOTTOM);
	// details
	labelpair_init_right(&self->apiversion, &self->component, "API Version", 12.0);
	labelpair_init_right(&self->version, &self->component, "Version", 12.0);	
	labelpair_init_right(&self->dllname, &self->component, "DllName", 12.0);
	newmachinedetail_reset(self);
}

void newmachinedetail_reset(NewMachineDetail* self)
{
	psy_ui_label_enabletranslation(&self->desclabel);
	psy_ui_label_settext(&self->desclabel, psy_ui_translate(
		"newmachine.select-plugin-to-view-description"));
	newmachinedetail_setplugname(self, "");
	newmachinedetail_setdllname(self, "");
	newmachinedetail_setapiversion(self, 0);
	newmachinedetail_setplugversion(self, 0);
}

void newmachinedetail_update(NewMachineDetail* self,
	psy_Property* property)
{
	if (property) {						
		psy_audio_MachineInfo machineinfo;
		psy_Path path;

		machineinfo_init(&machineinfo);
		psy_audio_machineinfo_from_property(property, &machineinfo);		
		newmachinedetail_setdescription(self, machineinfo.desc);		
		psy_path_init(&path, machineinfo.modulepath);
		newmachinedetail_setplugname(self, machineinfo.Name);
		newmachinedetail_setdllname(self, psy_path_filename(&path));
		psy_path_dispose(&path);
		machineinfo_dispose(&machineinfo);
		newmachinedetail_setapiversion(self, machineinfo.APIVersion);
		newmachinedetail_setplugversion(self, machineinfo.PlugVersion);
	} else {
		newmachinedetail_reset(self);
	}
}

void newmachinedetail_onloadnewblitz(NewMachineDetail* self, psy_ui_Component* sender)
{	
	compatconfig_setloadnewblitz(
		psycleconfig_compat(workspace_conf(self->workspace)),
		psy_ui_checkbox_checked(&self->compatblitzgamefx) != FALSE);	
}

void newmachinedetail_setdescription(NewMachineDetail* self, const char* text)
{
	psy_ui_label_preventtranslation(&self->desclabel);
	psy_ui_label_settext(&self->desclabel, text);
}

void newmachinedetail_setplugname(NewMachineDetail* self, const char* text)
{
	psy_ui_label_settext(&self->plugname.value, text);
}

void newmachinedetail_setdllname(NewMachineDetail* self, const char* text)
{	
	psy_ui_label_settext(&self->dllname.value, text);
}

void newmachinedetail_setplugversion(NewMachineDetail* self, int16_t version)
{
	char valstr[64];

	psy_snprintf(valstr, 64, "%d", (int)version);	
	psy_ui_label_settext(&self->version.value, valstr);
}

void newmachinedetail_setapiversion(NewMachineDetail* self, int16_t apiversion)
{
	char valstr[64];

	psy_snprintf(valstr, 64, "%d", (int)apiversion);
	psy_ui_label_settext(&self->apiversion.value, valstr);
}

// NewMachineFilterBar
// prototypes
static void newmachinefilterbar_onclicked(NewMachineFilterBar* self, psy_ui_Button* sender);
// implementation
void newmachinefilterbar_init(NewMachineFilterBar* self, psy_ui_Component* parent,
	NewMachineFilter* filters)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init_text_connect(&self->gen, &self->component, NULL,
		"Generators", self, newmachinefilterbar_onclicked);
	psy_ui_button_allowrightclick(&self->gen);
	psy_ui_button_init_text_connect(&self->effects, &self->component, NULL,
		"Effects", self, newmachinefilterbar_onclicked);
	psy_ui_button_allowrightclick(&self->effects);
	psy_ui_button_init_text_connect(&self->intern, &self->component, NULL,
		"Intern", self, newmachinefilterbar_onclicked);	
	psy_ui_button_allowrightclick(&self->intern);
	psy_ui_button_init_text_connect(&self->native, &self->component, NULL,
		"Native", self, newmachinefilterbar_onclicked);
	psy_ui_button_allowrightclick(&self->native);
	psy_ui_button_init_text_connect(&self->vst, &self->component, NULL,
		"VST", self, newmachinefilterbar_onclicked);
	psy_ui_button_allowrightclick(&self->vst);
	psy_ui_button_init_text_connect(&self->lua, &self->component, NULL,
		"LUA", self, newmachinefilterbar_onclicked);
	psy_ui_button_allowrightclick(&self->lua);
	psy_ui_button_init_text_connect(&self->ladspa, &self->component, NULL,
		"LADSPA", self, newmachinefilterbar_onclicked);	
	psy_ui_button_allowrightclick(&self->ladspa);
	self->filters = filters;
	newmachinefilterbar_update(self);
}

void newmachinefilterbar_setfilters(NewMachineFilterBar* self,
	NewMachineFilter* filters)
{
	self->filters = filters;
	newmachinefilterbar_update(self);
}

void newmachinefilterbar_onclicked(NewMachineFilterBar* self, psy_ui_Button* sender)
{		
	if (self->filters) {
		if (sender->buttonstate == 1) {
			if (sender == &self->effects) {
				self->filters->effect = !self->filters->effect;
			} else if (sender == &self->gen) {
				self->filters->gen = !self->filters->gen;
			} else if (sender == &self->intern) {
				self->filters->intern = !self->filters->intern;
			} else if (sender == &self->native) {
				self->filters->native = !self->filters->native;
			} else if (sender == &self->vst) {
				self->filters->vst = !self->filters->vst;
			} else if (sender == &self->lua) {
				self->filters->lua = !self->filters->lua;
			} else if (sender == &self->ladspa) {
				self->filters->ladspa = !self->filters->ladspa;
			}
		} else {			
			if (sender == &self->effects) {				
				self->filters->effect = TRUE;
				self->filters->gen = FALSE;				
			} else if (sender == &self->gen) {								
				self->filters->effect = FALSE;
				self->filters->gen = TRUE;				
			} else if (sender == &self->intern) {				
				newmachinefilter_cleartypes(self->filters);
				self->filters->intern = TRUE;				
			} else if (sender == &self->native) {				
				newmachinefilter_cleartypes(self->filters);
				self->filters->native = TRUE;				
			} else if (sender == &self->vst) {				
				newmachinefilter_cleartypes(self->filters);
				self->filters->vst = TRUE;				
			} else if (sender == &self->lua) {				
				newmachinefilter_cleartypes(self->filters);
				self->filters->lua = TRUE;				
			} else if (sender == &self->ladspa) {				
				newmachinefilter_cleartypes(self->filters);
				self->filters->ladspa = TRUE;				
			}
		}
		newmachinefilterbar_update(self);
		newmachinefilter_notify(self->filters);
	}	
}

void newmachinefilterbar_update(NewMachineFilterBar* self)
{
	if (self->filters) {
		if (self->filters->effect) {
			psy_ui_button_highlight(&self->effects);
		} else {
			psy_ui_button_disablehighlight(&self->effects);
		}
		if (self->filters->gen) {
			psy_ui_button_highlight(&self->gen);
		} else {
			psy_ui_button_disablehighlight(&self->gen);
		}
		if (self->filters->intern) {
			psy_ui_button_highlight(&self->intern);
		} else {
			psy_ui_button_disablehighlight(&self->intern);
		}
		if (self->filters->native) {
			psy_ui_button_highlight(&self->native);
		} else {
			psy_ui_button_disablehighlight(&self->native);
		}
		if (self->filters->vst) {
			psy_ui_button_highlight(&self->vst);
		} else {
			psy_ui_button_disablehighlight(&self->vst);
		}
		if (self->filters->lua) {
			psy_ui_button_highlight(&self->lua);
		} else {
			psy_ui_button_disablehighlight(&self->lua);
		}
		if (self->filters->ladspa) {
			psy_ui_button_highlight(&self->ladspa);
		} else {
			psy_ui_button_disablehighlight(&self->ladspa);
		}
	}
}

// NewMachineCategoryBar
// prototypes
static void newmachinecategorybar_ondestroy(NewMachineCategoryBar*);
static void newmachinecategorybar_findcategories(NewMachineCategoryBar*,
	psy_Property* source);
static void newmachinecategorybar_onclicked(NewMachineCategoryBar*,
	psy_ui_Button* sender);
// vtable
static psy_ui_ComponentVtable newmachinecategorybar_vtable;
static bool newmachinecategorybar_vtable_initialized = FALSE;

static void newmachinecategorybar_vtable_init(NewMachineCategoryBar* self)
{
	if (!newmachinecategorybar_vtable_initialized) {
		newmachinecategorybar_vtable = *(self->component.vtable);
		newmachinecategorybar_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			newmachinecategorybar_ondestroy;
		newmachinecategorybar_vtable_initialized = TRUE;
	}
	self->component.vtable = &newmachinecategorybar_vtable;
}
// implementation
void newmachinecategorybar_init(NewMachineCategoryBar* self, psy_ui_Component* parent,
	NewMachineFilter* filter, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	newmachinecategorybar_vtable_init(self);
	psy_ui_component_init(&self->client, &self->component, NULL);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setdefaultalign(&self->client, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;
	psy_table_init(&self->categories);
}

void newmachinecategorybar_ondestroy(NewMachineCategoryBar* self)
{
	psy_table_disposeall(&self->categories, NULL);
}

void newmachinecategorybar_build(NewMachineCategoryBar* self)
{
	psy_TableIterator it;
	psy_ui_Button* button;

	assert(self);

	psy_ui_component_clear(&self->client);	

	button = psy_ui_button_allocinit(&self->client, &self->client);
	psy_ui_button_preventtranslation(button);
	psy_ui_button_settext(button, "Any Category");
	if (newmachinefilter_useanycategory(self->filters)) {
		psy_ui_button_highlight(button);
	} else {
		psy_ui_button_disablehighlight(button);
	}
	psy_signal_connect(&button->signal_clicked, self,
		newmachinecategorybar_onclicked);
	newmachinecategorybar_findcategories(self, workspace_pluginlist(self->workspace));	
	for (it = psy_table_begin(&self->categories);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		const char* category;

		category = (const char*)psy_tableiterator_value(&it);		
		if (category) {
			psy_ui_Button* button;

			button = psy_ui_button_allocinit(&self->client, &self->client);
			psy_ui_button_preventtranslation(button);
			psy_ui_button_settext(button, category);
			if (!newmachinefilter_useanycategory(self->filters)) {
				if (!newmachinefilter_hascategory(self->filters, category)) {
					psy_ui_button_highlight(button);
				}
			}
			psy_signal_connect(&button->signal_clicked, self,
				newmachinecategorybar_onclicked);
		}
	}
}

void newmachinecategorybar_findcategories(NewMachineCategoryBar* self,
	psy_Property* source)
{	
	psy_List* p;
	uintptr_t num;
	uintptr_t i;	
		
	psy_table_disposeall(&self->categories, NULL);
	psy_table_init(&self->categories);
	if (!source) {
		return;
	}		
	num = psy_property_size(source);
	p = psy_property_begin(source);	
	for (i = 0; p != NULL && i < num; psy_list_next(&p), ++i) {
		psy_Property* q;
		psy_audio_MachineInfo machineinfo;

		q = (psy_Property*)psy_list_entry(p);
				
		machineinfo_init(&machineinfo);
		psy_audio_machineinfo_from_property(q, &machineinfo);
		if (psy_strlen(machineinfo.category) > 0) {
			if (!psy_table_exists_strhash(&self->categories, machineinfo.category)) {
				psy_table_insert_strhash(&self->categories, machineinfo.category,
					(void*)psy_strdup(machineinfo.category));
			}
		}
		machineinfo_dispose(&machineinfo);
	}	
}

void newmachinecategorybar_onclicked(NewMachineCategoryBar* self, psy_ui_Button* sender)
{	
	psy_List* p;
	psy_List* q;

	if (strcmp(psy_ui_button_text(sender), "Any Category") == 0) {
		newmachinefilter_anycategory(self->filters);
		q = psy_ui_component_children(&self->client, psy_ui_NONRECURSIVE);
		for (p = q; p != NULL; p = p->next) {
			psy_ui_Button* button;
		
			button = (psy_ui_Button*)psy_list_entry(p);
			psy_ui_button_disablehighlight(button);			
		}
		psy_list_free(q);
		psy_ui_button_highlight(sender);
		return;
	}
	if (!newmachinefilter_useanycategory(self->filters) &&
			newmachinefilter_hascategory(self->filters, psy_ui_button_text(sender))) {
		newmachinefilter_removecategory(self->filters, psy_ui_button_text(sender));
		psy_ui_button_disablehighlight(sender);
	} else {
		newmachinefilter_addcategory(self->filters, psy_ui_button_text(sender));
		psy_ui_button_highlight(sender);
	}
	q = psy_ui_component_children(&self->client, psy_ui_NONRECURSIVE);
	if (newmachinefilter_useanycategory(self->filters)) {
		psy_ui_button_highlight((psy_ui_Button*)psy_list_entry(q));
	} else {
		psy_ui_button_disablehighlight((psy_ui_Button*)psy_list_entry(q));
	}
	psy_list_free(q);
}


// PluginScanView
void pluginscanview_init(PluginScanView* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_label_init_text(&self->scan, &self->component, NULL,
		"Scanning");	
	psy_ui_component_setalign(psy_ui_label_base(&self->scan),
		psy_ui_ALIGN_CENTER);
}

// PluginsView
static void pluginsview_ondestroy(PluginsView*, psy_ui_Component* component);
static void pluginsview_ondraw(PluginsView*, psy_ui_Graphics*);
static void pluginsview_drawitem(PluginsView*, psy_ui_Graphics*, psy_Property*,
	psy_ui_RealPoint topleft);
static void pluginsview_onpreferredsize(PluginsView*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pluginsview_onkeydown(PluginsView*, psy_ui_KeyEvent*);
static void pluginsview_cursorposition(PluginsView*, psy_Property* plugin,
	intptr_t* col, intptr_t* row);
static psy_Property* pluginsview_pluginbycursorposition(PluginsView*,
	intptr_t col, intptr_t row);
static void pluginsview_onmousedown(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_onmousedoubleclick(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_hittest(PluginsView*, double x, double y);
static void pluginsview_computetextsizes(PluginsView*, const psy_ui_Size*);
static void pluginsview_onplugincachechanged(PluginsView*,
	psy_audio_PluginCatcher* sender);
static uintptr_t pluginsview_visilines(PluginsView*);
static uintptr_t pluginsview_topline(PluginsView*);
static void pluginsview_settopline(PluginsView*, intptr_t line);
static uintptr_t pluginsview_numlines(const PluginsView*);
static uintptr_t pluginenabled(const PluginsView*, psy_Property* property);
static void pluginsview_onfilterchanged(PluginsView*, NewMachineFilter* sender);

static psy_ui_ComponentVtable pluginsview_vtable;
static int pluginsview_vtable_initialized = 0;

static void pluginsview_vtable_init(PluginsView* self)
{
	if (!pluginsview_vtable_initialized) {
		pluginsview_vtable = *(self->component.vtable);				
		pluginsview_vtable.ondraw = (psy_ui_fp_component_ondraw) pluginsview_ondraw;		
		pluginsview_vtable.onkeydown = (psy_ui_fp_component_onkeyevent)
			pluginsview_onkeydown;
		pluginsview_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			pluginsview_onmousedown;
		pluginsview_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			pluginsview_onmousedoubleclick;
		pluginsview_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			pluginsview_onpreferredsize;		
	}
	self->component.vtable = &pluginsview_vtable;
}

void pluginsview_init(PluginsView* self, psy_ui_Component* parent,
	bool favorites, Workspace* workspace)
{	
	psy_ui_Size size;

	psy_ui_component_init(&self->component, parent, NULL);
	pluginsview_vtable_init(self);
	newmachinefilter_init(&self->filters);
	self->workspace = workspace;
	self->onlyfavorites = favorites;
	self->mode = NEWMACHINE_APPEND;
	if (workspace_pluginlist(workspace)) {
		if (favorites) {
			self->plugins = newmachine_favorites(
				workspace_pluginlist(workspace));
		} else {
			self->plugins = psy_property_clone(
				workspace_pluginlist(workspace));
		}
	} else {
		self->plugins = NULL;
	}	
	psy_ui_component_doublebuffer(&self->component);	
	psy_signal_connect(&self->component.signal_destroy, self,
		pluginsview_ondestroy);
	self->selectedplugin = NULL;
	self->generatorsenabled = TRUE;
	self->effectsenabled = TRUE;
	psy_signal_init(&self->signal_selected);
	psy_signal_init(&self->signal_changed);
	psy_signal_connect(&workspace->plugincatcher.signal_changed, self,
		pluginsview_onplugincachechanged);
	size = psy_ui_component_size(&self->component);
	pluginsview_computetextsizes(self, &size);	
	psy_signal_connect(&self->filters.signal_changed, self,
		pluginsview_onfilterchanged);
}

void pluginsview_ondestroy(PluginsView* self, psy_ui_Component* component)
{
	psy_signal_dispose(&self->signal_selected);
	psy_signal_dispose(&self->signal_changed);
	if (self->plugins) {
		psy_property_deallocate(self->plugins);
	}
	newmachinefilter_dispose(&self->filters);
}

void pluginsview_ondraw(PluginsView* self, psy_ui_Graphics* g)
{	
	if (self->plugins) {
		psy_ui_Size size;
		psy_List* p;
		psy_ui_RealPoint cp;		
		
		size = psy_ui_component_size(&self->component);
		pluginsview_computetextsizes(self, &size);
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_realpoint_init(&cp);
		for (p = psy_property_begin(self->plugins);
				p != NULL; psy_list_next(&p)) {
			pluginsview_drawitem(self, g, (psy_Property*)psy_list_entry(p),
				cp);
			cp.x += self->columnwidth;
			if (cp.x >= self->numparametercols * self->columnwidth) {
				cp.x = 0.0;
				cp.y += self->lineheight;
			}
		}
	}
}

void pluginsview_drawitem(PluginsView* self, psy_ui_Graphics* g,
	psy_Property* property, psy_ui_RealPoint topleft)
{
	char text[128];

	if (property == self->selectedplugin) {
		psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x009B7800));		
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00FFFFFF));
	} else {
		psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x00232323));
		if (pluginenabled(self, property)) {
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00CACACA));
		} else {
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00666666));
		}
	}		
	plugindisplayname(property, text);	
	psy_ui_textout(g, topleft.x, topleft.y + 2, text, strlen(text));
	plugintype(property, text);
	psy_ui_textout(g, topleft.x + self->columnwidth - self->avgcharwidth * 7,
		topleft.y + 2, text, strlen(text));
	if (pluginmode(property, text) == psy_audio_MACHMODE_FX) {
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00B1C8B0));
	} else {		
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00D1C5B6));
	}
	psy_ui_textout(g, topleft.x + self->columnwidth - 10 * self->avgcharwidth,
		topleft.y + 2, text, strlen(text));
}

void pluginsview_computetextsizes(PluginsView* self, const psy_ui_Size* size)
{
	const psy_ui_TextMetric* tm;
	
	tm = psy_ui_component_textmetric(&self->component);
	self->avgcharwidth = tm->tmAveCharWidth;
	self->lineheight = (int) (tm->tmHeight * 1.5);
	self->columnwidth = tm->tmAveCharWidth * 45;
	self->identwidth = tm->tmAveCharWidth * 4;
	self->numparametercols = 
		(uintptr_t)psy_max(1, psy_ui_value_px(&size->width, tm) / self->columnwidth);
	psy_ui_component_setscrollstep_height(&self->component,
		psy_ui_value_makepx(self->lineheight));
}

void plugindisplayname(psy_Property* property, char* text)
{	
	const char* label;

	label = psy_property_at_str(property, "shortname", "");
	if (strcmp(label, "") == 0) {
		label = psy_property_key(property);
	}
	psy_snprintf(text, 128, "%s", label);
}

uintptr_t plugintype(psy_Property* property, char* text)
{	
	uintptr_t rv;
	
	rv = (uintptr_t)psy_property_at_int(property, "type", -1);
	switch (rv) {
		case psy_audio_PLUGIN:
			strcpy(text, "psy");
		break;
		case psy_audio_LUA:
			strcpy(text, "lua");
		break;
		case psy_audio_VST:
			strcpy(text, "vst");
		break;
		case psy_audio_VSTFX:
			strcpy(text, "vst");
		break;
		case psy_audio_LADSPA:
			strcpy(text, "lad");
			break;
		default:
			strcpy(text, "int");
		break;
	}
	return rv;
}

uintptr_t pluginmode(psy_Property* property, char* text)
{			
	uintptr_t rv;

	rv = (uintptr_t)psy_property_at_int(property, "mode", -1);
	strcpy(text, rv == psy_audio_MACHMODE_FX ? "fx" : "gn");
	return rv;
}

uintptr_t pluginenabled(const PluginsView* self, psy_Property* property)
{
	uintptr_t mode;
	
	mode = psy_property_at_int(property, "mode", psy_audio_MACHMODE_FX);
	if (self->effectsenabled && mode == psy_audio_MACHMODE_FX) {
		return TRUE;
	}
	if (self->generatorsenabled && mode == psy_audio_MACHMODE_GENERATOR) {
		return TRUE;
	}
	return FALSE;
}

void pluginsview_onpreferredsize(PluginsView* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (self->plugins) {		
		psy_ui_Size parentsize;
		
		parentsize = psy_ui_component_size(psy_ui_component_parent(&self->component));
		pluginsview_computetextsizes(self, &parentsize);
		rv->width = parentsize.width;
		rv->height = psy_ui_value_makepx(self->lineheight *
			pluginsview_numlines(self));
	}
}

void pluginsview_onkeydown(PluginsView* self, psy_ui_KeyEvent* ev)
{
	if (self->selectedplugin) {
		psy_Property* plugin;
		uintptr_t col;
		uintptr_t row;

		col = 0;
		row = 0;
		plugin = NULL;
		pluginsview_cursorposition(self, self->selectedplugin, &col, &row);
		switch (ev->keycode) {
			case psy_ui_KEY_RETURN:
				if (self->selectedplugin) {
					psy_signal_emit(&self->signal_selected, self, 1,
						self->selectedplugin);
					workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
						SECTION_ID_MACHINEVIEW_WIRES, 0);
					psy_ui_keyevent_stoppropagation(ev);
				}
				break;
			case psy_ui_KEY_DELETE:
				if (self->selectedplugin) {
					psy_Property* p;
					p = psy_property_find(self->workspace->plugincatcher.plugins,
							psy_property_key(self->selectedplugin),
							PSY_PROPERTY_TYPE_NONE);
					if (!self->onlyfavorites && p) {
						psy_property_remove(self->workspace->plugincatcher.plugins, p);
					} else {						
						psy_property_set_int(p, "favorite", 0);
					}
					psy_audio_plugincatcher_save(&self->workspace->plugincatcher);
					psy_signal_emit(&self->workspace->plugincatcher.signal_changed,
						&self->workspace->plugincatcher, 0);					
				}
				break;
			case psy_ui_KEY_DOWN:
				if (row + 1 < pluginsview_numlines(self)) {
					++row;
					if (row > pluginsview_topline(self) + pluginsview_visilines(self)) {
						pluginsview_settopline(self, row - pluginsview_visilines(self));
					}
				} else {
					psy_ui_component_focus_next(psy_ui_component_parent(&self->component));
				}
				psy_ui_keyevent_stoppropagation(ev);
				break;		
			case psy_ui_KEY_UP:
				if (row > 0) {
					--row;
					if (row < pluginsview_topline(self)) {
						pluginsview_settopline(self, row);
					}
				} else {
					psy_ui_component_focus_prev(psy_ui_component_parent(
						&self->component));
				}
				psy_ui_keyevent_stoppropagation(ev);
				break;
			case psy_ui_KEY_PRIOR:
				if (row > 0) {
					row = psy_max(0, row - 4);
					if (row < pluginsview_topline(self)) {
						pluginsview_settopline(self, row);
					}
				}
				psy_ui_keyevent_stoppropagation(ev);
				break;
			case psy_ui_KEY_NEXT:
				row += 4;
				if (row >= pluginsview_numlines(self) - 1) {
					row = pluginsview_numlines(self) - 1;
				}
				if (row > pluginsview_topline(self) + pluginsview_visilines(self)) {
					pluginsview_settopline(self, row - pluginsview_visilines(self));
				}
				psy_ui_keyevent_stoppropagation(ev);
				break;
			case psy_ui_KEY_LEFT:			
				if (col > 0) {
					--col;		
				}
				psy_ui_keyevent_stoppropagation(ev);
				break;
			case psy_ui_KEY_RIGHT: {					
				++col;				
				psy_ui_keyevent_stoppropagation(ev);
				break;
			}
			default:			
				break;
		}		
		plugin = pluginsview_pluginbycursorposition(self, col, row);
		if (plugin) {
			self->selectedplugin = plugin;
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selectedplugin);
			psy_ui_component_invalidate(&self->component);
		}
	} else
	if (ev->keycode >= psy_ui_KEY_LEFT && ev->keycode <= psy_ui_KEY_DOWN) {
		if (self->plugins && !psy_property_empty(self->plugins)) {
			self->selectedplugin = psy_property_first(self->plugins);
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selectedplugin);
			psy_ui_component_invalidate(&self->component);
		}
	}	
}

uintptr_t pluginsview_visilines(PluginsView* self)
{
	psy_ui_RealSize size;

	size = psy_ui_component_sizepx(&self->component);
	return (uintptr_t)(size.height / self->lineheight);
}

uintptr_t pluginsview_topline(PluginsView* self)
{
	return (uintptr_t)(psy_ui_component_scrolltoppx(&self->component)
		/ self->lineheight);
}

uintptr_t pluginsview_numlines(const PluginsView* self)
{
	return psy_property_size(self->plugins) /
		self->numparametercols + 1;
}

void pluginsview_settopline(PluginsView* self, intptr_t line)
{
	
	psy_ui_component_setscrolltop(&self->component,
		psy_ui_value_makepx(line * self->lineheight));
}

void pluginsview_cursorposition(PluginsView* self, psy_Property* plugin,
	intptr_t* col, intptr_t* row)
{		
	*col = 0;
	*row = 0;
	if (plugin && self->plugins) {
		psy_List* p;
		
		for (p = psy_property_begin(self->plugins); p != NULL;
				psy_list_next(&p)) {	
			if (p->entry == plugin) {
				break;
			}
			++(*col);
			if (*col >= self->numparametercols) {
				*col = 0;
				++(*row);
			}
		}		
	}	
}

psy_Property* pluginsview_pluginbycursorposition(PluginsView* self, intptr_t col, intptr_t row)
{				
	if (self->plugins) {
		psy_Property* rv;
		psy_List* p;
		int currcol;
		int currrow;
		psy_ui_Size size;
		rv = NULL;

		currcol = 0;
		currrow = 0;
		size = psy_ui_component_size(&self->component);
		pluginsview_computetextsizes(self, &size);
		for (p = psy_property_begin(self->plugins); p != NULL;
				psy_list_next(&p)) {			
			if (currcol == col && currrow == row) {
				rv = (psy_Property*)p->entry;
				break;
			}
			++currcol;
			if (currcol >= self->numparametercols) {
				currcol = 0;
				++currrow;
			}
		}
		return rv;
	}
	return NULL;
}

void pluginsview_onmousedown(PluginsView* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		psy_ui_component_setfocus(&self->component);
		pluginsview_hittest(self, ev->pt.x, ev->pt.y);
		psy_ui_component_invalidate(&self->component);
		psy_signal_emit(&self->signal_changed, self, 1,
			self->selectedplugin);		
		psy_ui_component_setfocus(&self->component);		
	}
}

void pluginsview_hittest(PluginsView* self, double x, double y)
{				
	if (self->plugins) {
		psy_ui_Size size;
		psy_List* p;
		double cpx;
		double cpy;

		size = psy_ui_component_size(&self->component);
		pluginsview_computetextsizes(self, &size);
		for (p = psy_property_begin(self->plugins), cpx = 0, cpy = 0;
				p != NULL; psy_list_next(&p)) {
			psy_ui_RealRectangle r;

			psy_ui_setrectangle(&r, cpx, cpy, self->columnwidth,
				self->lineheight);
			if (psy_ui_realrectangle_intersect(&r,
					psy_ui_realpoint_make(x, y))) {
				if (pluginenabled(self, (psy_Property*)psy_list_entry(p))) {
					self->selectedplugin = (psy_Property*)psy_list_entry(p);
				}
				break;
			}		
			cpx += self->columnwidth;
			if (cpx >= self->numparametercols * self->columnwidth) {
				cpx = 0;
				cpy += self->lineheight;
			}
		}
	}
}

void pluginsview_onmousedoubleclick(PluginsView* self, psy_ui_MouseEvent* ev)
{
	if (self->selectedplugin) {		
		psy_signal_emit(&self->signal_selected, self, 1,
			self->selectedplugin);
//		workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
//			SECTION_ID_MACHINEVIEW_WIRES, 0);
		psy_ui_mouseevent_stoppropagation(ev);		
	}	
}

void pluginsview_onplugincachechanged(PluginsView* self,
	psy_audio_PluginCatcher* sender)
{
	psy_ui_component_setscrolltop(&self->component, psy_ui_value_zero());
	self->selectedplugin = 0;
	if (self->plugins) {
		psy_property_deallocate(self->plugins);
	}
	if (sender->plugins) {
		if (self->onlyfavorites) {
			self->plugins = newmachine_favorites(sender->plugins);
		} else {
			self->plugins = psy_property_clone(sender->plugins);
		}
	} else {
		self->plugins = 0;
	}	
	psy_ui_component_setscrolltop(&self->component, psy_ui_value_zero());	
	psy_ui_component_updateoverflow(&self->component);
	psy_ui_component_invalidate(&self->component);	
}

void pluginsview_onfilterchanged(PluginsView* self, NewMachineFilter* sender)
{
	self->selectedplugin = 0;
	if (self->plugins) {
		psy_property_deallocate(self->plugins);
	}
	self->plugins = search(workspace_pluginlist(self->workspace), &self->filters);
	psy_signal_emit(&self->signal_changed, self, 1,
		self->selectedplugin);
	psy_ui_component_setscrolltop(&self->component, psy_ui_value_zero());
	psy_ui_component_updateoverflow(&self->component);
	psy_ui_component_invalidate(&self->component);
}

// NewMachineSection
// prototypes
static void newmachinesection_ondestroy(NewMachineSection*, psy_ui_Component* sender);
static void newmachinesection_findplugins(NewMachineSection*);
static void newmachinesection_onlabelclick(NewMachineSection*, psy_ui_Label* sender,
	psy_ui_MouseEvent*);
static void newmachinesection_onmousedown(NewMachineSection*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void newmachinesection_oneditaccept(NewMachineSection*, psy_ui_Edit* sender);
static void newmachinesection_oneditreject(NewMachineSection*, psy_ui_Edit* sender);
// implementation
void newmachinesection_init(NewMachineSection* self, psy_ui_Component* parent,
	psy_Property* property, psy_ui_Edit* edit, NewMachine* newmachine, Workspace* workspace)
{	
	psy_ui_Margin margin;
	psy_ui_Margin spacing;
	psy_ui_Border border;

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletypes(&self->component, 
		psy_INDEX_INVALID, psy_INDEX_INVALID, STYLE_NEWMACHINE_SECTION_SELECTED,
		psy_INDEX_INVALID);
	psy_signal_connect(&self->component.signal_destroy, self,
		newmachinesection_ondestroy);
	psy_signal_connect(&self->component.signal_mousedown, self,
		newmachinesection_onmousedown);
	self->property  = property;
	self->edit = edit;	
	self->preventedit = TRUE;
	self->newmachine = newmachine;
	psy_ui_margin_init_all_em(&margin, 1.0, 0.0, 1.0, 0.0);
	psy_ui_component_init(&self->header, &self->component, NULL);
	psy_ui_component_setalign(&self->header, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->header, margin);
	psy_ui_margin_init_all_em(&spacing, 1.0, 0.0, 0.0, 0.0);
	psy_ui_component_setspacing(&self->header, spacing);
	psy_ui_component_setdefaultalign(&self->header, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_border_init_all(&border, psy_ui_BORDER_SOLID,
		psy_ui_BORDER_NONE, psy_ui_BORDER_NONE, psy_ui_BORDER_NONE);
	self->header.style.style.border = border;
	psy_ui_label_init(&self->label, &self->header, NULL);
	psy_ui_label_preventtranslation(&self->label);
	psy_ui_label_settext(&self->label, psy_property_key(self->property));
	psy_ui_label_settextalignment(&self->label,
		psy_ui_ALIGNMENT_LEFT |
		psy_ui_ALIGNMENT_CENTER_VERTICAL);
	pluginsview_init(&self->pluginview, &self->component, TRUE, workspace);
	psy_ui_component_setalign(&self->pluginview.component, psy_ui_ALIGN_TOP);
	newmachinesection_findplugins(self);
	if (self->edit) {
		psy_signal_connect(&self->edit->signal_accept, self,
			newmachinesection_oneditaccept);
		psy_signal_connect(&self->edit->signal_reject, self,
			newmachinesection_oneditreject);
		psy_signal_connect(&self->label.component.signal_mousedown, self,
			newmachinesection_onlabelclick);
	}
}

void newmachinesection_ondestroy(NewMachineSection* self, psy_ui_Component* sender)
{	
}

void newmachinesection_findplugins(NewMachineSection* self)
{
	if (self->pluginview.plugins) {
		psy_property_deallocate(self->pluginview.plugins);
	}	
	self->pluginview.plugins = psy_property_clone(self->property);
	self->pluginview.selectedplugin = NULL;
}

void newmachinesection_onlabelclick(NewMachineSection* self, psy_ui_Label* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->edit) {
		psy_ui_RealRectangle position;
		psy_ui_RealRectangle editposition;		
		psy_ui_RealRectangle labelposition;		
				
		position = psy_ui_component_screenposition(&self->header);
		editposition = psy_ui_component_screenposition(psy_ui_component_parent(&self->edit->component));		
		labelposition = psy_ui_component_position(&self->label.component);
		labelposition.top += (position.top - editposition.top);
		labelposition.bottom += (position.top - editposition.top);
		self->preventedit = FALSE;
		psy_ui_component_setposition(&self->edit->component,
			psy_ui_rectangle_make_px(&labelposition));
		psy_ui_edit_settext(self->edit, self->label.text);
		psy_ui_edit_setsel(self->edit, 0, -1);
		psy_ui_component_show(&self->edit->component);
		psy_ui_component_setfocus(&self->edit->component);
	}
}

void newmachinesection_oneditaccept(NewMachineSection* self, psy_ui_Edit* sender)
{
	if (!self->preventedit) {
		self->preventedit = TRUE;
		psy_property_change_key(self->property, psy_ui_edit_text(sender));
		psy_ui_label_settext(&self->label, psy_property_key(self->property));
		psy_ui_component_hide(psy_ui_edit_base(sender));
	}
}

void newmachinesection_oneditreject(NewMachineSection* self, psy_ui_Edit* sender)
{
	if (!self->preventedit) {
		self->preventedit = TRUE;
		psy_ui_component_hide(psy_ui_edit_base(sender));
		psy_ui_component_invalidate(&self->component);
	}
}

void newmachinesection_onmousedown(NewMachineSection* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_List* p;
	psy_List* q;

	q = psy_ui_component_children(&self->newmachine->usersections,
		psy_ui_NONRECURSIVE);
	for (p = q; p != NULL; p = p->next) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)psy_list_entry(p);
		psy_ui_component_removestylestate(component,
			psy_ui_STYLESTATE_SELECT);
	}
	psy_list_free(q);
	self->newmachine->selectedsection = self;
	psy_ui_component_addstylestate(&self->component,
		psy_ui_STYLESTATE_SELECT);
}

NewMachineSection* newmachinesection_alloc(void)
{
	return (NewMachineSection*)malloc(sizeof(NewMachineSection));
}

NewMachineSection* newmachinesection_allocinit(psy_ui_Component* parent,
	psy_Property* property, psy_ui_Edit* edit, NewMachine* newmachine,
	Workspace* workspace)
{
	NewMachineSection* rv;

	rv = newmachinesection_alloc();
	if (rv) {
		newmachinesection_init(rv, parent, property, edit, newmachine, workspace);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

// NewMachine
// prototypes
static void newmachine_ondestroy(NewMachine*, psy_ui_Component* component);
static void newmachine_onpluginselected(NewMachine*, psy_ui_Component* parent,
	psy_Property*);
static void newmachine_onpluginchanged(NewMachine*, psy_ui_Component* parent,
	psy_Property*);
static void newmachine_onplugincachechanged(NewMachine*, psy_audio_PluginCatcher*);
static void newmachine_onkeydown(NewMachine*, psy_ui_KeyEvent*);
static void newmachine_onmousedown(NewMachine*, psy_ui_MouseEvent*);
static void newmachine_onsortbyfavorite(NewMachine*, psy_ui_Component* sender);
static void newmachine_onsortbyname(NewMachine*, psy_ui_Component* sender);
static void newmachine_onsortbytype(NewMachine*, psy_ui_Component* sender);
static void newmachine_onsortbymode(NewMachine*, psy_ui_Component* sender);
static void newmachine_oncreatesection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onaddtosection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onremovefromsection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onremovesection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onfocus(NewMachine*, psy_ui_Component* sender);
static void newmachine_onrescan(NewMachine*, psy_ui_Component* sender);
static void newmachine_onpluginscanprogress(NewMachine*, Workspace*,
	int progress);
static void newmachine_ontimer(NewMachine*, uintptr_t timerid);
static void newmachine_buildsections(NewMachine*);

// vtable
static psy_ui_ComponentVtable newmachine_vtable;
static bool newmachine_vtable_initialized = FALSE;

static void newmachine_vtable_init(NewMachine* self)
{
	if (!newmachine_vtable_initialized) {
		newmachine_vtable = *(self->component.vtable);				
		newmachine_vtable.onkeydown = (psy_ui_fp_component_onkeyevent)
			newmachine_onkeydown;
		newmachine_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			newmachine_onmousedown;
		newmachine_vtable.ontimer = (psy_ui_fp_component_ontimer)
			newmachine_ontimer;
		newmachine_vtable_initialized = TRUE;
	}
	self->component.vtable = &newmachine_vtable;
}
// implementation
void newmachine_init(NewMachine* self, psy_ui_Component* parent,
	MachineViewSkin* skin, Workspace* workspace)
{
	psy_ui_Margin margin;
	psy_ui_Margin spacing;
	psy_ui_Border sectionborder;
	
	psy_ui_component_init(&self->component, parent, NULL);
	newmachine_vtable_init(self);	
	self->skin = skin;
	self->workspace = workspace;
	self->scanending = FALSE;
	self->mode = NEWMACHINE_APPEND;
	self->appendstack = FALSE;
	self->restoresection = SECTION_ID_MACHINEVIEW_WIRES;
	self->selectedplugin = NULL;
	self->selectedsection = NULL;
	self->newsectioncount = 0;	
	// Notebook	
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	// client
	psy_ui_component_init(&self->client, psy_ui_notebook_base(&self->notebook),
		NULL);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	// scanview
	pluginscanview_init(&self->scanview,
		psy_ui_notebook_base(&self->notebook));
	// header margin
	psy_ui_margin_init_all_em(&margin, 1.0, 0.0, 1.0, 0.0);
	// section border, define for the top line above a section label
	psy_ui_border_init_all(&sectionborder, psy_ui_BORDER_SOLID,
		psy_ui_BORDER_NONE, psy_ui_BORDER_NONE,psy_ui_BORDER_NONE);
	psy_ui_colour_set(&sectionborder.colour_top, psy_ui_colour_make(0x00666666));
	// sections
	psy_ui_component_init(&self->sections, &self->client, NULL);	
	// favorite view
	psy_ui_component_init(&self->favoriteheader, &self->sections, NULL);
	psy_ui_component_setalign(&self->favoriteheader, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->favoriteheader, margin);
	psy_ui_margin_init_all_em(&spacing, 1.0, 0.0, 0.0, 0.0);
	psy_ui_component_setspacing(&self->favoriteheader, spacing);
	psy_ui_component_setdefaultalign(&self->favoriteheader, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->favoriteheader.style.style.border = sectionborder;
	psy_ui_image_init(&self->favoriteicon, &self->favoriteheader);
	psy_ui_bitmap_loadresource(&self->favoriteicon.bitmap, IDB_HEART_DARK);
	psy_ui_bitmap_settransparency(&self->favoriteicon.bitmap, psy_ui_colour_make(0x00FFFFFF));
	psy_ui_image_setbitmapalignment(&self->favoriteicon, psy_ui_ALIGNMENT_CENTER_VERTICAL);
	psy_ui_component_setpreferredsize(&self->favoriteicon.component,
		psy_ui_size_make_px(16, 14));
	psy_ui_component_preventalign(&self->favoriteicon.component);
	psy_ui_label_init_text(&self->favoritelabel, &self->favoriteheader, NULL,
		"newmachine.favorites");	
	psy_ui_label_settextalignment(&self->favoritelabel,
		psy_ui_ALIGNMENT_LEFT |
		psy_ui_ALIGNMENT_CENTER_VERTICAL);	
	// Favorite View
	pluginsview_init(&self->favoriteview, &self->sections, TRUE, workspace);
	psy_ui_component_setalign(&self->favoriteview.component, psy_ui_ALIGN_TOP);
	// user sections
	psy_ui_component_init(&self->usersections, &self->sections, NULL);
	psy_ui_component_setalign(&self->usersections, psy_ui_ALIGN_TOP);
	// Edit
	psy_ui_edit_init(&self->edit, &self->sections);
	psy_ui_edit_enableinputfield(&self->edit);
	psy_ui_component_hide(&self->edit.component);	
	// section scroll
	psy_ui_component_setoverflow(&self->sections, psy_ui_OVERFLOW_VSCROLL);
	psy_ui_component_setscrollstep(&self->sections, 
		psy_ui_size_make_em(0.0, 1.0));
	psy_ui_scroller_init(&self->scroller_fav, &self->sections, &self->client, NULL);
	psy_ui_component_setalign(&self->sections, psy_ui_ALIGN_HCLIENT);
	psy_ui_component_settabindex(&self->scroller_fav.component, 0);
	psy_ui_component_setalign(&self->scroller_fav.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setbackgroundmode(&self->scroller_fav.pane, psy_ui_SETBACKGROUND);
	// all
	psy_ui_component_init(&self->all, &self->client, NULL);
	psy_ui_component_setalign(&self->all, psy_ui_ALIGN_CLIENT);
	// pluginview header
	psy_ui_component_init(&self->pluginsheader, &self->all, NULL);
	psy_ui_component_setalign(&self->pluginsheader, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->pluginsheader, margin);
	psy_ui_component_setspacing(&self->pluginsheader, spacing);
	psy_ui_component_setdefaultalign(&self->pluginsheader, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->pluginsheader.style.style.border = sectionborder;
	psy_ui_image_init(&self->pluginsicon, &self->pluginsheader);
	psy_ui_bitmap_loadresource(&self->pluginsicon.bitmap, IDB_TRAY_DARK);
	psy_ui_bitmap_settransparency(&self->pluginsicon.bitmap, psy_ui_colour_make(0x00FFFFFF));
	psy_ui_image_setbitmapalignment(&self->pluginsicon, psy_ui_ALIGNMENT_CENTER_VERTICAL);
	psy_ui_component_setpreferredsize(&self->pluginsicon.component,
		psy_ui_size_make_px(16, 14));
	psy_ui_component_preventalign(&self->pluginsicon.component);
	psy_ui_label_init_text(&self->pluginslabel, &self->pluginsheader, NULL,
		"newmachine.all");
	psy_ui_label_settextalignment(&self->pluginslabel,
		psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL);
	newmachinefilterbar_init(&self->filterbar, &self->pluginsheader, NULL);
	// all categeory bar
	newmachinecategorybar_init(&self->categorybar, &self->all, NULL,
		workspace);
	psy_ui_component_setalign(&self->categorybar.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->categorybar.component,
		psy_ui_margin_makeem(0.0, 0.0, 1.0, 0.0));	
	// Plugins View
	pluginsview_init(&self->pluginsview, &self->all, FALSE, workspace);
	psy_ui_component_setwheelscroll(&self->pluginsview.component, 1);
	psy_ui_component_setoverflow(&self->pluginsview.component, psy_ui_OVERFLOW_VSCROLL);
	newmachinefilterbar_setfilters(&self->filterbar, &self->pluginsview.filters);
	self->categorybar.filters = &self->pluginsview.filters;
	psy_ui_scroller_init(&self->scroller_main, &self->pluginsview.component,
		&self->all, NULL);
	psy_ui_component_setbackgroundmode(&self->scroller_main.pane, psy_ui_SETBACKGROUND);
	psy_ui_component_settabindex(&self->scroller_main.component, 1);
	psy_ui_component_setalign(&self->scroller_main.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->pluginsview.component, psy_ui_ALIGN_HCLIENT);
	// Details
	newmachinedetail_init(&self->detail, &self->component, &self->pluginsview.filters,
		self->workspace);
	psy_ui_component_setalign(&self->detail.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmaximumsize(&self->detail.component,
		psy_ui_size_make_em(40.0, 0.0));
	// signals
	psy_signal_init(&self->signal_selected);	
	psy_signal_connect(&self->pluginsview.signal_selected, self,
		newmachine_onpluginselected);
	psy_signal_connect(&self->pluginsview.signal_changed, self,
		newmachine_onpluginchanged);
	psy_signal_connect(&self->favoriteview.signal_selected, self,
		newmachine_onpluginselected);
	psy_signal_connect(&self->favoriteview.signal_changed, self,
		newmachine_onpluginchanged);
	psy_signal_connect(&workspace->plugincatcher.signal_changed, self,
		newmachine_onplugincachechanged);
	psy_signal_connect(&self->detail.bar.sortbyfavorite.signal_clicked, self,
		newmachine_onsortbyfavorite);
	psy_signal_connect(&self->detail.bar.sortbyname.signal_clicked, self,
		newmachine_onsortbyname);
	psy_signal_connect(&self->detail.bar.sortbytype.signal_clicked, self,
		newmachine_onsortbytype);
	psy_signal_connect(&self->detail.bar.sortbymode.signal_clicked, self,
		newmachine_onsortbymode);
	psy_signal_connect(&self->component.signal_focus, self,
		newmachine_onfocus);
	psy_signal_connect(&self->component.signal_destroy, self,
		newmachine_ondestroy);
	psy_signal_connect(&self->detail.bar.rescan.signal_clicked, self,
		newmachine_onrescan);
	psy_signal_connect(&self->detail.bar.createsection.signal_clicked, self,
		newmachine_oncreatesection);
	psy_signal_connect(&self->detail.bar.addtosection.signal_clicked, self,
		newmachine_onaddtosection);
	psy_signal_connect(&self->detail.bar.removefromsection.signal_clicked, self,
		newmachine_onremovefromsection);
	psy_signal_connect(&self->detail.bar.removesection.signal_clicked, self,
		newmachine_onremovesection);
	psy_signal_connect(&workspace->signal_scanprogress, self,
		newmachine_onpluginscanprogress);
	newmachine_updateskin(self);
	psy_ui_notebook_select(&self->notebook, 0);
	newmachinecategorybar_build(&self->categorybar);
	psy_ui_component_align(&self->categorybar.component);
	newmachine_buildsections(self);
	psy_ui_component_align(&self->client);
}

void newmachine_ondestroy(NewMachine* self, psy_ui_Component* component)
{
	psy_signal_dispose(&self->signal_selected);
}

void newmachine_updateskin(NewMachine* self)
{
	psy_ui_component_setbackgroundcolour(&self->component, self->skin->colour);
	psy_ui_component_setcolour(&self->component, self->skin->effect_fontcolour);
}

void newmachine_onpluginselected(NewMachine* self, psy_ui_Component* parent,
	psy_Property* selected)
{	
	self->selectedplugin = selected;
	newmachinedetail_update(&self->detail, selected);
	psy_signal_emit(&self->signal_selected, self, 1, selected);
	if (self->selectedplugin) {
		intptr_t favorite;

		favorite = psy_property_at_int(self->selectedplugin, "favorite", 0);
		psy_property_set_int(self->selectedplugin, "favorite", ++favorite);
	}
	psy_property_sync(workspace_pluginlist(self->pluginsview.workspace), self->pluginsview.plugins);
	psy_audio_plugincatcher_save(&self->pluginsview.workspace->plugincatcher);
	pluginsview_onplugincachechanged(&self->favoriteview,
		&self->favoriteview.workspace->plugincatcher);
	if (self->favoriteview.plugins) {
		psy_Property* sorted;

		sorted = newmachine_sort(self->favoriteview.plugins,
			newmachine_comp_favorite);
		psy_property_deallocate(self->favoriteview.plugins);
		self->favoriteview.plugins = sorted;
		newmachinedetail_reset(&self->detail);
		psy_ui_component_align(&self->component);
		psy_ui_component_setscrolltop(&self->favoriteview.component, psy_ui_value_zero());
		psy_ui_component_updateoverflow(&self->favoriteview.component);
		psy_ui_component_invalidate(&self->favoriteview.component);
	}	
}

void newmachine_onpluginchanged(NewMachine* self, psy_ui_Component* parent,
	psy_Property* selected)
{	
	self->selectedplugin = selected;
	newmachinedetail_update(&self->detail, selected);		
}

void newmachine_onplugincachechanged(NewMachine* self,
	psy_audio_PluginCatcher* sender)
{
	newmachinefilter_reset(&self->pluginsview.filters);
	newmachinedetail_reset(&self->detail);
	self->selectedplugin = NULL;
	newmachinecategorybar_build(&self->categorybar);
	psy_ui_component_align(&self->categorybar.component);
	psy_ui_component_align(&self->categorybar.client);
	psy_ui_component_align(&self->client);
}

void newmachine_onsortbyfavorite(NewMachine* self, psy_ui_Component* sender)
{
	psy_Property* sorted;

	if (self->pluginsview.plugins) {
		sorted = newmachine_sort(self->pluginsview.plugins,
			newmachine_comp_favorite);
		psy_property_deallocate(self->pluginsview.plugins);
		self->pluginsview.plugins = sorted;
		newmachinedetail_reset(&self->detail);
		psy_ui_component_setscrolltop(&self->pluginsview.component, psy_ui_value_zero());
		psy_ui_component_updateoverflow(&self->pluginsview.component);
		psy_ui_component_invalidate(&self->pluginsview.component);
	}
}

void newmachine_onsortbyname(NewMachine* self, psy_ui_Component* sender)
{
	psy_Property* sorted;
	
	if (self->pluginsview.plugins) {
		sorted = newmachine_sort(self->pluginsview.plugins,
			newmachine_comp_name);
		psy_property_deallocate(self->pluginsview.plugins);
		self->pluginsview.plugins = sorted;
		newmachinedetail_reset(&self->detail);
		psy_ui_component_setscrolltop(&self->pluginsview.component, psy_ui_value_zero());
		psy_ui_component_updateoverflow(&self->pluginsview.component);
		psy_ui_component_invalidate(&self->pluginsview.component);
	}
}

void newmachine_onsortbytype(NewMachine* self, psy_ui_Component* parent)
{
	psy_Property* sorted;
	
	if (self->pluginsview.plugins) {
		sorted = newmachine_sort(self->pluginsview.plugins,
			newmachine_comp_type);
		psy_property_deallocate(self->pluginsview.plugins);
		self->pluginsview.plugins = sorted;
		newmachinedetail_reset(&self->detail);
		psy_ui_component_setscrolltop(&self->pluginsview.component, psy_ui_value_zero());
		psy_ui_component_updateoverflow(&self->pluginsview.component);
		psy_ui_component_invalidate(&self->pluginsview.component);
	}
}

void newmachine_onsortbymode(NewMachine* self, psy_ui_Component* parent)
{
	psy_Property* sorted;
	
	if (self->pluginsview.plugins) {
		sorted = newmachine_sort(self->pluginsview.plugins,
			newmachine_comp_mode);
		psy_property_deallocate(self->pluginsview.plugins);
		self->pluginsview.plugins = sorted;
		newmachinedetail_reset(&self->detail);
		psy_ui_component_setscrolltop(&self->pluginsview.component, psy_ui_value_zero());
		psy_ui_component_updateoverflow(&self->pluginsview.component);
		psy_ui_component_invalidate(&self->pluginsview.component);
	}
}

void newmachine_oncreatesection(NewMachine* self, psy_ui_Component* sender)
{
	char sectionkey[64];

	psy_snprintf(sectionkey, 64, "section%d", (int)self->newsectioncount);
	psy_audio_pluginsections_add(&self->workspace->pluginsections,
		sectionkey, NULL);
	++self->newsectioncount;
	newmachine_buildsections(self);
}

void newmachine_buildsections(NewMachine* self)
{
	psy_List* p;
	psy_ui_component_clear(&self->usersections);	
	self->selectedsection = NULL;	
	p = psy_property_begin(self->workspace->pluginsections.sections);
	for (; p != 0; p = p->next) {
		psy_Property* property;
		NewMachineSection* section;

		property = (psy_Property*)psy_list_entry(p);
		section = newmachinesection_allocinit(&self->usersections, property,
			&self->edit, self, self->workspace);
		if (section) {
			psy_signal_connect(&section->pluginview.signal_selected, self,
				newmachine_onpluginselected);
			psy_signal_connect(&section->pluginview.signal_changed, self,
				newmachine_onpluginchanged);
			psy_ui_component_setalign(&section->component, psy_ui_ALIGN_TOP);
		}		
	}	
	psy_ui_app()->alignvalid = FALSE;
	psy_ui_component_align(&self->client);
	psy_ui_app()->alignvalid = TRUE;
	psy_ui_component_invalidate(&self->client);
}

void newmachine_onaddtosection(NewMachine* self, psy_ui_Component* sender)
{
	if (!self->selectedsection) {
		workspace_output(self->workspace, "Select/Create first a section");
	} else if (!self->selectedplugin) {
		workspace_output(self->workspace, "Select first a plugin");
	} else {
		psy_audio_MachineInfo macinfo;

		machineinfo_init(&macinfo);
		newmachine_selectedmachineinfo(self, &macinfo);
		psy_audio_pluginsections_add(&self->workspace->pluginsections,
			psy_property_key(self->selectedsection->property),
			&macinfo);
		self->selectedplugin = NULL;
		newmachinesection_findplugins(self->selectedsection);		
		machineinfo_dispose(&macinfo);		
		psy_ui_app()->alignvalid = FALSE;
		psy_ui_component_align(&self->client);
		psy_ui_app()->alignvalid = TRUE;
		psy_ui_component_invalidate(&self->client);
	}
}

void newmachine_onremovefromsection(NewMachine* self, psy_ui_Component* sender)
{
	if (!self->selectedsection) {
		workspace_output(self->workspace, "Select/Create first a section");
	} else if (!self->selectedplugin) {
		workspace_output(self->workspace, "Select first a plugin");
	} else {
		psy_audio_MachineInfo macinfo;

		machineinfo_init(&macinfo);
		newmachine_selectedmachineinfo(self, &macinfo);
		psy_audio_pluginsections_remove(&self->workspace->pluginsections,
			psy_property_key(self->selectedsection->property),
			&macinfo);
		self->selectedplugin = NULL;
		newmachinesection_findplugins(self->selectedsection);
		machineinfo_dispose(&macinfo);
		psy_ui_app()->alignvalid = FALSE;
		psy_ui_component_align(&self->client);
		psy_ui_app()->alignvalid = TRUE;
		psy_ui_component_invalidate(&self->client);
	}
}

void newmachine_onremovesection(NewMachine* self, psy_ui_Component* sender)
{
	if (!self->selectedsection) {
		workspace_output(self->workspace, "Select/Create first a section");
	} else {		
		self->selectedplugin = NULL;
		psy_audio_pluginsections_remove(&self->workspace->pluginsections,
			psy_property_key(self->selectedsection->property),
			NULL);
		newmachine_buildsections(self);
		psy_ui_app()->alignvalid = FALSE;
		psy_ui_component_align(&self->client);
		psy_ui_app()->alignvalid = TRUE;
		psy_ui_component_invalidate(&self->client);
	}
}


void newmachine_onkeydown(NewMachine* self, psy_ui_KeyEvent* ev)
{
	if (ev->keycode != psy_ui_KEY_ESCAPE) {	
		psy_ui_keyevent_stoppropagation(ev);
	}
}

psy_Property* newmachine_favorites(psy_Property* source)
{
	psy_Property* rv = 0;

	if (source && !psy_property_empty(source)) {
		psy_List* p;

		rv = psy_property_allocinit_key(NULL);
		for (p = psy_property_begin(source); p != NULL; psy_list_next(&p)) {
			psy_Property* property;

			property = (psy_Property*)psy_list_entry(p);
			if (psy_property_at_int(property, "favorite", 0) != FALSE) {				
				psy_property_append_property(rv, psy_property_clone(
					property));
			}
		}		
	}
	return rv;
}

psy_Property* newmachine_sort(psy_Property* source, psy_fp_comp comp)
{
	psy_Property* rv = NULL;

	if (source) {
		uintptr_t i;
		uintptr_t num;
		psy_List* p;
		psy_Property** propertiesptr;
		
		num = psy_property_size(source);
		propertiesptr = malloc(sizeof(psy_Property*) * num);
		if (propertiesptr) {
			p = psy_property_begin(source);
			for (i = 0; p != NULL && i < num; psy_list_next(&p), ++i) {
				propertiesptr[i] = (psy_Property*)psy_list_entry(p);
			}
			psy_qsort(propertiesptr, 0, (int)(num - 1), comp);
			rv = psy_property_allocinit_key(NULL);
			for (i = 0; i < num; ++i) {
				psy_property_append_property(rv, psy_property_clone(
					propertiesptr[i]));
			}
			free(propertiesptr);
		}
	}
	return rv;
}

psy_Property* search(psy_Property* source, NewMachineFilter* filter)
{
	psy_Property* rv;
	psy_List* p;
	uintptr_t num;
	uintptr_t i;
	char* text;

	assert(filter);

	rv = psy_property_allocinit_key(NULL);
	if (!source) {
		return rv;
	}	
	num = psy_property_size(source);		
	p = psy_property_begin(source);	
	text = filter->text;
	for (i = 0; p != NULL && i < num; psy_list_next(&p), ++i) {
		psy_Property* q;
		
		q = (psy_Property*)psy_list_entry(p);
		if (psy_strlen(text) == 0) {			
			searchfilter(q, filter, rv);			
		} else if (psy_property_text(q) &&
				strcasestr(psy_property_text(q), text)) {
			searchfilter(q, filter, rv);			
		}		
	}
	return rv;
}

void searchfilter(psy_Property* plugin, NewMachineFilter* filter,
	psy_Property* parent)
{
	psy_audio_MachineInfo machineinfo;
	bool isintern;
	
	machineinfo_init(&machineinfo);
	psy_audio_machineinfo_from_property(plugin, &machineinfo);	
	if (!filter->vst && (machineinfo.type == psy_audio_VST || machineinfo.type == psy_audio_VSTFX)) {
		return;
	}
	if (!filter->native && machineinfo.type == psy_audio_PLUGIN) {
		return;
	}
	if (!filter->ladspa && machineinfo.type == psy_audio_LADSPA) {
		return;
	}
	if (!filter->lua && machineinfo.type == psy_audio_LUA) {
		return;
	}
	isintern = FALSE;
	switch (machineinfo.type) {
	case psy_audio_PLUGIN:
	case psy_audio_LUA:						
	case psy_audio_VST:			
	case psy_audio_VSTFX:			
	case psy_audio_LADSPA:
		break;
	default:
		isintern = TRUE;
		break;
	}	
	if (!newmachinefilter_hascategory(filter, machineinfo.category)) {
		return;		
	}
	if (!filter->intern && isintern) {
		return;
	}	
	if (filter->effect && machineinfo.mode == psy_audio_MACHMODE_FX) {
		psy_property_append_property(parent, psy_property_clone(plugin));
	}
	if (filter->gen && machineinfo.mode == psy_audio_MACHMODE_GENERATOR) {
		psy_property_append_property(parent, psy_property_clone(plugin));
	}	
}

int newmachine_comp_favorite(psy_Property* p, psy_Property* q)
{
	int left;
	int right;
	
	left = (int)psy_property_at_int(p, "favorite", 0);
	right = (int)psy_property_at_int(q, "favorite", 0);
	return right - left;
}

int newmachine_comp_name(psy_Property* p, psy_Property* q)
{
	const char* left;
	const char* right;

	left = psy_property_at_str(p, "name", "");
	if (strlen(left) == 0) {
		left = psy_property_key(p);
	}
	right = psy_property_at_str(q, "name", "");
	if (strlen(right) == 0) {
		right = psy_property_key(q);
	}
	return strcmp(left, right);		
}

int newmachine_comp_type(psy_Property* p, psy_Property* q)
{
	int left;
	int right;
	
	left = (int)psy_property_at_int(p, "type", 128);
	left = newmachine_isplugin(left) ? left : 0;
	right = (int)psy_property_at_int(q, "type", 128);
	right = newmachine_isplugin(right) ? right : 0;
	return left - right;		
}

int newmachine_isplugin(int type)
{
	return (type == psy_audio_PLUGIN) ||
	   (type == psy_audio_VST) ||
	   (type == psy_audio_VSTFX) ||
	   (type == psy_audio_LUA) ||
	   (type == psy_audio_LADSPA);
}

int newmachine_comp_mode(psy_Property* p, psy_Property* q)
{	
	return (int)psy_property_at_int(p, "mode", 128) -
		(int)psy_property_at_int(q, "mode", 128);
}

void newmachine_onfocus(NewMachine* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(&self->pluginsview.component);
}

void newmachine_onrescan(NewMachine* self, psy_ui_Component* sender)
{
	self->scanending = FALSE;
	psy_ui_component_starttimer(newmachine_base(self), 0, 50);
	psy_ui_notebook_select(&self->notebook, 1);
	workspace_scanplugins(self->workspace);
}

void newmachine_onpluginscanprogress(NewMachine* self, Workspace* workspace,
	int progress)
{
	if (progress == 0) {
		self->scanending = TRUE;		
	} else {
		
	}
}

void  newmachine_ontimer(NewMachine* self, uintptr_t timerid)
{
	if (self->scanending) {
		psy_ui_notebook_select(&self->notebook, 0);
		psy_ui_component_stoptimer(newmachine_base(self), 0);
	}
}

void newmachine_enableall(NewMachine* self)
{
	self->pluginsview.effectsenabled = TRUE;
	self->pluginsview.generatorsenabled = TRUE;
	self->favoriteview.effectsenabled = TRUE;
	self->favoriteview.generatorsenabled = TRUE;
	psy_ui_component_invalidate(&self->pluginsview.component);
	psy_ui_component_invalidate(&self->favoriteview.component);
}

void newmachine_enablegenerators(NewMachine* self)
{	
	self->pluginsview.generatorsenabled = TRUE;	
	self->favoriteview.generatorsenabled = TRUE;
	psy_ui_component_invalidate(&self->pluginsview.component);
	psy_ui_component_invalidate(&self->favoriteview.component);
}

void newmachine_preventgenerators(NewMachine* self)
{
	self->pluginsview.generatorsenabled = FALSE;
	self->favoriteview.generatorsenabled = FALSE;
	psy_ui_component_invalidate(&self->pluginsview.component);
	psy_ui_component_invalidate(&self->favoriteview.component);
}

void newmachine_enableeffects(NewMachine* self)
{
	self->pluginsview.effectsenabled = TRUE;
	self->favoriteview.effectsenabled = TRUE;
	psy_ui_component_invalidate(&self->pluginsview.component);
	psy_ui_component_invalidate(&self->favoriteview.component);
}

void newmachine_preventeffects(NewMachine* self)
{
	self->pluginsview.effectsenabled = FALSE;
	self->favoriteview.effectsenabled = FALSE;
	psy_ui_component_invalidate(&self->pluginsview.component);
	psy_ui_component_invalidate(&self->favoriteview.component);
}

void newmachine_insertmode(NewMachine* self)
{
	uintptr_t index;

	index = psy_audio_machines_selected(&self->workspace->song->machines);
	if (index != psy_INDEX_INVALID) {
		if (index < 0x40) {
			newmachine_enablegenerators(self);
			newmachine_preventeffects(self);
		} else {
			newmachine_enableeffects(self);
			newmachine_preventgenerators(self);
		}
	}
	self->mode = NEWMACHINE_INSERT;
	self->pluginsview.mode = NEWMACHINE_INSERT;
	self->favoriteview.mode = NEWMACHINE_INSERT;
}

void newmachine_appendmode(NewMachine* self)
{
	newmachine_enableall(self);
	self->mode = NEWMACHINE_APPEND;
	self->pluginsview.mode = NEWMACHINE_APPEND;
	self->favoriteview.mode = NEWMACHINE_APPEND;
}

void newmachine_addeffectmode(NewMachine* self)
{	
	newmachine_preventgenerators(self);
	newmachine_enableeffects(self);
	self->mode = NEWMACHINE_ADDEFFECT;
	self->pluginsview.mode = NEWMACHINE_ADDEFFECT;
	self->favoriteview.mode = NEWMACHINE_ADDEFFECT;		
}

bool newmachine_selectedmachineinfo(const NewMachine* self,
	psy_audio_MachineInfo* rv)
{
	if (self->selectedplugin) {
		psy_audio_machineinfo_from_property(self->selectedplugin, rv);
		return TRUE;
	}
	return FALSE;
}

void newmachine_onmousedown(NewMachine* self, psy_ui_MouseEvent* ev)
{
	psy_ui_mouseevent_stoppropagation(ev);
}

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

static int newmachine_isplugin(int type);

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
	psy_ui_component_setstyletypes(&self->component,
		STYLE_NEWMACHINE_SEARCHFIELD, psy_INDEX_INVALID,
		psy_INDEX_INVALID, psy_INDEX_INVALID);
	self->filter = filter;
	psy_ui_margin_init_all_em(&spacing, 0.3, 0.3, 0.3, 0.0);
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
	psy_ui_edit_setcharnumber(&self->edit, 42);
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

void newmachinesearch_setfilter(NewMachineSearch* self,
	NewMachineFilter* filter)
{
	self->filter = filter;
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
	self->hasdefaulttext = TRUE;
	if (self->filter) {
		newmachinefilter_settext(self->filter, "");
		psy_ui_component_setfocus(psy_ui_component_parent(&self->component));		
	}
}

// NewMachineSearchBar
// implementation
void newmachinesearchbar_init(NewMachineSearchBar* self, psy_ui_Component* parent,
	NewMachineFilter* filter)
{
	psy_ui_Margin margin;
	psy_ui_Margin spacing;

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_margin_init_all_em(&spacing, 0.5, 0.0, 0.5, 0.0);
	psy_ui_component_setspacing(&self->component, spacing);
	// search field
	newmachinesearch_init(&self->search, &self->component, filter);
	psy_ui_component_setalign(&self->search.component, psy_ui_ALIGN_RIGHT);
	psy_ui_margin_init_all_em(&margin, 0.0, 8.0, 0.0, 0.0);
	psy_ui_component_setmargin(&self->search.component, margin);	
}

void newmachinesearchbar_setfilter(NewMachineSearchBar* self,
	NewMachineFilter* filter)
{
	newmachinesearch_setfilter(&self->search, filter);
}

// NewMachineRescanBar
// prototypes
static void newmachinerescanbar_onselectdirectories(NewMachineRescanBar*,
	psy_ui_Component* sender);
// implementation
void newmachinerescanbar_init(NewMachineRescanBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_Margin spacing;	

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setbackgroundcolour(&self->component,
		psy_ui_colour_make_overlay(4));
	psy_ui_margin_init_all_em(&spacing, 0.2, 0.0, 0.2, 0.0);
	psy_ui_component_setspacing(&self->component, spacing);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;
	psy_ui_margin_init_all_em(&spacing, 0.2, 0.2, 0.2, 0.2);
	psy_ui_button_init_text(&self->rescan, &self->component, NULL,
		"newmachine.rescan");
	psy_ui_component_setspacing(psy_ui_button_base(&self->rescan), spacing);
	psy_ui_label_init_text(&self->desc, &self->component, NULL,
		"newmachine.in");	
	psy_ui_component_setspacing(psy_ui_label_base(&self->desc), spacing);
	psy_ui_button_init_text(&self->selectdirectories, &self->component, NULL,
		"newmachine.plugin-directories");
	psy_ui_component_setspacing(psy_ui_button_base(&self->selectdirectories),
		spacing);
	psy_signal_connect(&self->selectdirectories.signal_clicked, self,
		newmachinerescanbar_onselectdirectories);
}

void newmachinerescanbar_onselectdirectories(NewMachineRescanBar* self,
	psy_ui_Component* sender)
{
	workspace_selectview(self->workspace, VIEW_ID_SETTINGSVIEW, 3, 0);
}

// NewMachineSectionBar
// implementation
void newmachinesectionbar_init(NewMachineSectionBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin spacing;

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setbackgroundcolour(&self->component,
		psy_ui_colour_make_overlay(4));
	psy_ui_margin_init_all_em(&spacing, 0.2, 0.0, 0.2, 0.0);
	psy_ui_component_setspacing(&self->component, spacing);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;
	psy_ui_label_init_text(&self->desc, &self->component, NULL,
		"newmachine.section");
	psy_ui_button_init_text(&self->createsection, &self->component, NULL,
		"newmachine.create");
	psy_ui_button_init_text(&self->removesection, &self->component, NULL,
		"newmachine.remove");
	psy_ui_button_init_text(&self->clearsection, &self->component, NULL,
		"newmachine.clear");
	psy_ui_label_init_text(&self->descitem, &self->component, NULL,
		"Plugin");
	psy_ui_button_init_text(&self->addtosection, &self->component, NULL,
		"newmachine.add-to");
	psy_ui_button_init_text(&self->removefromsection, &self->component, NULL,
		"newmachine.remove-from");	
	psy_ui_label_init_text(&self->descsection, &self->component, NULL,
		"newmachine.section");
	
	psy_ui_margin_init_all_em(&spacing, 0.2, 0.2, 0.2, 0.2);
	psy_ui_component_setspacing_children(&self->component, spacing);
}

// NewMachineSectionBar
// prototypes
static void newmachinesortbar_onsortbyfavorite(NewMachineSortBar*, psy_ui_Component* sender);
static void newmachinesortbar_onsortbyname(NewMachineSortBar*, psy_ui_Component* sender);
static void newmachinesortbar_onsortbytype(NewMachineSortBar*, psy_ui_Component* sender);
static void newmachinesortbar_onsortbymode(NewMachineSortBar*, psy_ui_Component* sender);
// implementation
void newmachinesortbar_init(NewMachineSortBar* self, psy_ui_Component* parent,
	NewMachineSort* sort, Workspace* workspace)
{
	psy_ui_Margin spacing;
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_margin_init_all_em(&spacing, 0.2, 0.0, 0.2, 0.0);
	psy_ui_component_setspacing(&self->component, spacing);
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 0.0, 4.0);
	psy_ui_component_setmargin(&self->component, margin);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;
	self->sort = sort;
	psy_ui_label_init_text(&self->desc, &self->component, NULL,
		"newmachine.sort");
	psy_ui_button_init_text_connect(&self->sortbyfavorite, &self->component, NULL,
		"newmachine.favorite", self, newmachinesortbar_onsortbyfavorite);
	psy_ui_button_init_text_connect(&self->sortbyname, &self->component, NULL,
		"newmachine.name", self, newmachinesortbar_onsortbyname);
	psy_ui_button_init_text_connect(&self->sortbytype, &self->component, NULL,
		"newmachine.type", self, newmachinesortbar_onsortbytype);
	psy_ui_button_init_text_connect(&self->sortbymode, &self->component, NULL,
		"newmachine.mode", self, newmachinesortbar_onsortbymode);
	psy_ui_margin_init_all_em(&spacing, 0.2, 0.2, 0.2, 0.2);
	psy_ui_component_setspacing_children(&self->component, spacing);
}

void newmachinesortbar_onsortbyfavorite(NewMachineSortBar* self,
	psy_ui_Component* sender)
{
	if (self->sort) {
		newmachinesort_sort(self->sort, NEWMACHINESORTMODE_FAVORITE);
	}
}

void newmachinesortbar_onsortbyname(NewMachineSortBar* self,
	psy_ui_Component* sender)
{
	if (self->sort) {
		newmachinesort_sort(self->sort, NEWMACHINESORTMODE_NAME);
	}
}

void newmachinesortbar_onsortbytype(NewMachineSortBar* self,
	psy_ui_Component* sender)
{
	if (self->sort) {
		newmachinesort_sort(self->sort, NEWMACHINESORTMODE_TYPE);
	}
}

void newmachinesortbar_onsortbymode(NewMachineSortBar* self,
	psy_ui_Component* sender)
{
	if (self->sort) {
		newmachinesort_sort(self->sort, NEWMACHINESORTMODE_MODE);
	}
}

// NewMachineDetail
// prototypes
static void newmachinedetail_onloadnewblitz(NewMachineDetail*, psy_ui_Component* sender);
static void newmachinedetail_oncategoryeditaccept(NewMachineDetail*,
	psy_ui_Component* sender);
static void newmachinedetail_oncategoryeditreject(NewMachineDetail*,
	psy_ui_Component* sender);
static void newmachinedetail_ondestroy(NewMachineDetail*);
// vtable
static psy_ui_ComponentVtable newmachinedetail_vtable;
static bool newmachinedetail_vtable_initialized = FALSE;

static void newmachinedetail_vtable_init(NewMachineDetail* self)
{
	if (!newmachinedetail_vtable_initialized) {
		newmachinedetail_vtable = *(self->component.vtable);
		newmachinedetail_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			newmachinedetail_ondestroy;
		newmachinedetail_vtable_initialized = TRUE;
	}
	self->component.vtable = &newmachinedetail_vtable;
}
// implementation
void newmachinedetail_init(NewMachineDetail* self, psy_ui_Component* parent,
	NewMachineFilter* filter, Workspace* workspace)
{
	psy_ui_Margin margin;
	psy_ui_Margin spacing;

	psy_ui_component_init(&self->component, parent, NULL);
	newmachinedetail_vtable_init(self);
	psy_signal_init(&self->signal_categorychanged);
	psy_ui_margin_init_all_em(&spacing, 0.5, 0.5, 0.5, 2.0);
	psy_ui_component_setspacing(&self->component, spacing);
	self->plugin = NULL;
	self->workspace = workspace;	
	// plugin name
	labelpair_init(&self->plugname, &self->component, "Name", 12.0);
	psy_ui_component_setalign(&self->plugname.value.component,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->plugname.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 1.0, 0.0);
	psy_ui_component_setmargin(&self->plugname.component, margin);
	// description
	psy_ui_label_init(&self->desclabel, &self->component, NULL);
	psy_ui_label_settextalignment(&self->desclabel, psy_ui_ALIGNMENT_TOP);
	psy_ui_component_setalign(&self->desclabel.component, psy_ui_ALIGN_CLIENT);	
	psy_ui_component_setalign(&self->dllname.component, psy_ui_ALIGN_BOTTOM);
	// bottom
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
	psy_ui_component_setalign(&self->compatblitzgamefx.component, psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->compatlabel, &self->bottom, NULL,
		"newmachine.song-loading-compatibility");
	psy_ui_label_settextalignment(&self->compatlabel, psy_ui_ALIGNMENT_LEFT);
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 0.5, 0.0);
	psy_ui_component_setmargin(psy_ui_label_base(&self->compatlabel), margin);
	psy_ui_component_setalign(&self->compatlabel.component, psy_ui_ALIGN_TOP);
	// details	
	psy_ui_component_init(&self->details, &self->component, NULL);
	psy_ui_component_setalign(&self->details, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setdefaultalign(&self->details,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());
	// category
	psy_ui_component_init(&self->category, &self->details, NULL);	
	psy_ui_label_init_text(&self->categorydesc, &self->category, NULL,
		"Category");
	psy_ui_label_settextalignment(&self->categorydesc,
		psy_ui_ALIGNMENT_RIGHT);
	psy_ui_label_setcharnumber(&self->categorydesc, 12.0);
	psy_ui_component_setalign(psy_ui_label_base(&self->categorydesc),
		psy_ui_ALIGN_LEFT);
	psy_ui_edit_init(& self->categoryedit, &self->category);
	psy_ui_edit_enableinputfield(&self->categoryedit);	
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 0.0, 1.5);
	psy_ui_component_setmargin(psy_ui_edit_base(&self->categoryedit), margin);
	psy_ui_component_setalign(psy_ui_edit_base(&self->categoryedit),
		psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->categoryedit.signal_accept,
		self, newmachinedetail_oncategoryeditaccept);
	psy_signal_connect(&self->categoryedit.signal_reject,
		self, newmachinedetail_oncategoryeditreject);
	labelpair_init_right(&self->apiversion, &self->details, "API Version", 12.0);
	labelpair_init_right(&self->version, &self->details, "Version", 12.0);
	labelpair_init_right(&self->dllname, &self->details, "DllName", 12.0);
	newmachinedetail_reset(self);
}

void newmachinedetail_ondestroy(NewMachineDetail* self)
{
	psy_signal_dispose(&self->signal_categorychanged);
}

void newmachinedetail_reset(NewMachineDetail* self)
{
	psy_ui_label_enabletranslation(&self->desclabel);
	psy_ui_label_settext(&self->desclabel, psy_ui_translate(
		"newmachine.select-plugin-to-view-description"));
	newmachinedetail_setplugname(self, "");
	newmachinedetail_setdllname(self, "");
	newmachinedetail_setcategoryname(self, "");
	newmachinedetail_setapiversion(self, 0);
	newmachinedetail_setplugversion(self, 0);
	self->plugin = NULL;
	psy_ui_component_align(&self->component);	
}

void newmachinedetail_update(NewMachineDetail* self,
	psy_Property* property)
{
	if (property) {		
		psy_audio_MachineInfo machineinfo;
		psy_Path path;

		self->plugin = property;
		machineinfo_init(&machineinfo);
		psy_audio_machineinfo_from_property(property, &machineinfo);		
		newmachinedetail_setdescription(self, machineinfo.desc);		
		psy_path_init(&path, machineinfo.modulepath);
		newmachinedetail_setplugname(self, machineinfo.Name);
		newmachinedetail_setdllname(self, psy_path_filename(&path));
		newmachinedetail_setapiversion(self, machineinfo.APIVersion);
		newmachinedetail_setplugversion(self, machineinfo.PlugVersion);
		newmachinedetail_setcategoryname(self, machineinfo.category);		
		psy_path_dispose(&path);
		machineinfo_dispose(&machineinfo);
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

void newmachinedetail_setcategoryname(NewMachineDetail* self, const char* text)
{
	if (text) {
		psy_ui_edit_settext(&self->categoryedit, text);
	} else {
		psy_ui_edit_settext(&self->categoryedit, "");
	}
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

void newmachinedetail_oncategoryeditaccept(NewMachineDetail* self,
	psy_ui_Component* sender)
{
	if (self->plugin) {		
		psy_signal_emit(&self->signal_categorychanged, self, 0);	
	}
	psy_ui_component_setfocus(psy_ui_component_parent(&self->component));
}

void newmachinedetail_oncategoryeditreject(NewMachineDetail* self,
	psy_ui_Component* sender)
{
	psy_ui_component_setfocus(psy_ui_component_parent(&self->component));
}

// NewMachineFilterBar
// prototypes
static void newmachinefilterbar_onclicked(NewMachineFilterBar* self, psy_ui_Button* sender);
// implementation
void newmachinefilterbar_init(NewMachineFilterBar* self, psy_ui_Component* parent,
	NewMachineFilter* filters)
{	
	psy_ui_Margin spacing;
	psy_ui_Margin margin;
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_margin_init_all_em(&spacing, 0.2, 0.0, 0.2, 0.0);
	psy_ui_component_setspacing(&self->component, spacing);
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 0.0, 4.0);
	psy_ui_component_setmargin(&self->component, margin);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_label_init_text(&self->desc, &self->component, NULL,
		"newmachine.filter");
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
	self->filter = filter;
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
	if (self->filter) {
		if (newmachinefilter_useanycategory(self->filter)) {
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
				if (!newmachinefilter_useanycategory(self->filter)) {
					if (!newmachinefilter_hascategory(self->filter, category)) {
						psy_ui_button_highlight(button);
					}
				}
				psy_signal_connect(&button->signal_clicked, self,
					newmachinecategorybar_onclicked);
			}
		}
	} else {
		psy_ui_button_highlight(button);
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
			if (!psy_table_exists_strhash(&self->categories,
					machineinfo.category)) {
				psy_table_insert_strhash(&self->categories,
					machineinfo.category,
					(void*)psy_strdup(machineinfo.category));
			}
		}
		machineinfo_dispose(&machineinfo);
	}	
}

void newmachinecategorybar_onclicked(NewMachineCategoryBar* self, psy_ui_Button* sender)
{
	if (self->filter) {
		psy_List* p;
		psy_List* q;

		if (strcmp(psy_ui_button_text(sender), "Any Category") == 0) {
			newmachinefilter_anycategory(self->filter);
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
		if (!newmachinefilter_useanycategory(self->filter) &&
			newmachinefilter_hascategory(self->filter, psy_ui_button_text(sender))) {
			newmachinefilter_removecategory(self->filter, psy_ui_button_text(sender));
			psy_ui_button_disablehighlight(sender);
		} else {
			newmachinefilter_addcategory(self->filter, psy_ui_button_text(sender));
			psy_ui_button_highlight(sender);
		}
		q = psy_ui_component_children(&self->client, psy_ui_NONRECURSIVE);
		if (newmachinefilter_useanycategory(self->filter)) {
			psy_ui_button_highlight((psy_ui_Button*)psy_list_entry(q));
		} else {
			psy_ui_button_disablehighlight((psy_ui_Button*)psy_list_entry(q));
		}
		psy_list_free(q);
	}
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

// NewMachineSection
// prototypes
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

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletypes(&self->component, 
		STYLE_NEWMACHINE_SECTION, psy_INDEX_INVALID, STYLE_NEWMACHINE_SECTION_SELECTED,
		psy_INDEX_INVALID);	
	psy_ui_margin_init_all_em(&spacing, 0.5, 0.5, 1.0, 1.0);
	psy_ui_component_setspacing(&self->component, spacing);
	psy_signal_connect(&self->component.signal_mousedown, self,
		newmachinesection_onmousedown);
	self->property  = property;
	self->edit = edit;	
	self->preventedit = TRUE;
	self->newmachine = newmachine;	
	psy_ui_component_init(&self->header, &self->component, NULL);
	psy_ui_component_setalign(&self->header, psy_ui_ALIGN_TOP);
	psy_ui_component_setstyletypes(&self->header,
		STYLE_NEWMACHINE_SECTION_HEADER, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 1.0, 0.0);	
	psy_ui_component_setmargin(&self->header, margin);
	psy_ui_margin_init_all_em(&spacing, 0.0, 0.0, 0.0, 0.0);	
	psy_ui_component_setdefaultalign(&self->header, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_label_init(&self->label, &self->header, NULL);
	psy_ui_label_preventtranslation(&self->label);
	psy_ui_label_settext(&self->label, psy_property_key(self->property));
	psy_ui_label_settextalignment(&self->label,
		psy_ui_ALIGNMENT_LEFT |
		psy_ui_ALIGNMENT_CENTER_VERTICAL);
	pluginsview_init(&self->pluginview, &self->component, workspace);
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

void newmachinesection_findplugins(NewMachineSection* self)
{
	pluginsview_setplugins(&self->pluginview, self->property);	
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
static void newmachine_onmousedown(NewMachine*, psy_ui_MouseEvent*);
static void newmachine_oncreatesection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onremovesection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onclearsection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onaddtosection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onremovefromsection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onfocus(NewMachine*, psy_ui_Component* sender);
static void newmachine_onrescan(NewMachine*, psy_ui_Component* sender);
static void newmachine_onpluginscanprogress(NewMachine*, Workspace*,
	int progress);
static void newmachine_ontimer(NewMachine*, uintptr_t timerid);
static void newmachine_buildsections(NewMachine*);
static void newmachine_updateplugins(NewMachine*);
static void newmachine_onplugincategorychanged(NewMachine*, NewMachineDetail* sender);

// vtable
static psy_ui_ComponentVtable newmachine_vtable;
static bool newmachine_vtable_initialized = FALSE;

static void newmachine_vtable_init(NewMachine* self)
{
	if (!newmachine_vtable_initialized) {
		newmachine_vtable = *(self->component.vtable);		
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
	// Filter
	newmachinefilter_init(&self->filter);
	// Sort
	newmachinesort_init(&self->sort);
	// Notebook	
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	// client
	psy_ui_component_init(&self->client, psy_ui_notebook_base(&self->notebook),
		NULL);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	// searchbar
	newmachinesearchbar_init(&self->searchbar, &self->client, &self->filter);
	psy_ui_component_setalign(&self->searchbar.component, psy_ui_ALIGN_TOP);
	// scanview
	pluginscanview_init(&self->scanview,
		psy_ui_notebook_base(&self->notebook));
	// header margin
	psy_ui_margin_init_all_em(&margin, 1.0, 0.0, 1.0, 0.0);
	// section border, define for the top line above a section label
	psy_ui_border_init_all(&sectionborder, psy_ui_BORDER_SOLID,
		psy_ui_BORDER_NONE, psy_ui_BORDER_NONE,psy_ui_BORDER_NONE);
	psy_ui_colour_set(&sectionborder.colour_top, psy_ui_colour_make(0x00666666));
	// sectiongroup
	psy_ui_component_init(&self->sectiongroup, &self->client, NULL);
	psy_ui_component_setalign(&self->sectiongroup, psy_ui_ALIGN_CLIENT);
	// sectionbar
	newmachinesectionbar_init(&self->sectionbar, &self->sectiongroup, self->workspace);
	psy_ui_component_setalign(&self->sectionbar.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_margin_init_all_em(&margin, 0.5, 0.0, 0.0, 0.0);
	psy_ui_component_setmargin(&self->sectionbar.component, margin);
	// sections
	psy_ui_component_init(&self->sections, &self->sectiongroup, NULL);
	// favorite view
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
	psy_ui_scroller_init(&self->scroller_fav, &self->sections, &self->sectiongroup, NULL);
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
	psy_ui_margin_init_all_em(&spacing, 1.0, 0.0, 0.0, 0.0);
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
	// filter bar
	newmachinefilterbar_init(&self->filterbar, &self->pluginsheader, &self->filter);
	// sort bar
	newmachinesortbar_init(&self->sortbar, &self->pluginsheader, &self->sort,
		self->workspace);	
	// all categeory bar
	newmachinecategorybar_init(&self->categorybar, &self->all, &self->filter,
		workspace);
	psy_ui_component_setalign(&self->categorybar.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->categorybar.component,
		psy_ui_margin_makeem(0.0, 0.0, 1.0, 0.0));	
	// Plugins View
	pluginsview_init(&self->pluginsview, &self->all, workspace);
	pluginsview_setfilter(&self->pluginsview, &self->filter);
	pluginsview_setsort(&self->pluginsview, &self->sort);
	psy_ui_component_setwheelscroll(&self->pluginsview.component, 1);
	psy_ui_component_setoverflow(&self->pluginsview.component, psy_ui_OVERFLOW_VSCROLL);
	newmachinefilterbar_setfilters(&self->filterbar, &self->filter);
	psy_ui_scroller_init(&self->scroller_main, &self->pluginsview.component,
		&self->all, NULL);
	psy_ui_component_setbackgroundmode(&self->scroller_main.pane, psy_ui_SETBACKGROUND);
	psy_ui_component_settabindex(&self->scroller_main.component, 1);
	psy_ui_component_setalign(&self->scroller_main.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->pluginsview.component, psy_ui_ALIGN_HCLIENT);
	// Details
	newmachinedetail_init(&self->detail, &self->component, &self->filter,
		self->workspace);
	psy_ui_component_setalign(&self->detail.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmaximumsize(&self->detail.component,
		psy_ui_size_make_em(40.0, 0.0));
	psy_signal_connect(&self->detail.signal_categorychanged, self,
		newmachine_onplugincategorychanged);	
	// Rescanbar
	newmachinerescanbar_init(&self->rescanbar, &self->component, self->workspace);
	psy_ui_component_setalign(&self->rescanbar.component, psy_ui_ALIGN_BOTTOM);
	// filter
	newmachinesearchbar_setfilter(&self->searchbar, &self->filter);
	// signals
	psy_signal_init(&self->signal_selected);	
	psy_signal_connect(&self->pluginsview.signal_selected, self,
		newmachine_onpluginselected);
	psy_signal_connect(&self->pluginsview.signal_changed, self,
		newmachine_onpluginchanged);	
	psy_signal_connect(&workspace->plugincatcher.signal_changed, self,
		newmachine_onplugincachechanged);
	psy_signal_connect(&self->component.signal_focus, self,
		newmachine_onfocus);
	psy_signal_connect(&self->component.signal_destroy, self,
		newmachine_ondestroy);
	psy_signal_connect(&self->rescanbar.rescan.signal_clicked, self,
		newmachine_onrescan);
	psy_signal_connect(&self->sectionbar.createsection.signal_clicked, self,
		newmachine_oncreatesection);
	psy_signal_connect(&self->sectionbar.addtosection.signal_clicked, self,
		newmachine_onaddtosection);
	psy_signal_connect(&self->sectionbar.removefromsection.signal_clicked, self,
		newmachine_onremovefromsection);
	psy_signal_connect(&self->sectionbar.removesection.signal_clicked, self,
		newmachine_onremovesection);
	psy_signal_connect(&self->sectionbar.clearsection.signal_clicked, self,
		newmachine_onclearsection);
	psy_signal_connect(&workspace->signal_scanprogress, self,
		newmachine_onpluginscanprogress);
	newmachine_updateskin(self);
	psy_ui_notebook_select(&self->notebook, 0);
	newmachinecategorybar_build(&self->categorybar);
	psy_ui_component_align(&self->categorybar.component);
	newmachine_buildsections(self);
	newmachine_updateplugins(self);
}

void newmachine_ondestroy(NewMachine* self, psy_ui_Component* component)
{
	newmachinefilter_dispose(&self->filter);
	newmachinesort_dispose(&self->sort);
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
	newmachine_updateplugins(self);
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
	newmachine_updateplugins(self);
	newmachinefilter_reset(&self->filter);
	newmachinedetail_reset(&self->detail);	
	newmachinecategorybar_build(&self->categorybar);
	psy_ui_component_align(&self->categorybar.component);
	psy_ui_component_align(&self->categorybar.client);
	psy_ui_component_align(&self->client);
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
			if (p == psy_property_begin(
					self->workspace->pluginsections.sections)) {
				self->selectedsection = section;
				psy_ui_component_addstylestate(&section->component,
					psy_ui_STYLESTATE_SELECT);
			}
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

void newmachine_onclearsection(NewMachine* self, psy_ui_Component* sender)
{
	if (!self->selectedsection) {
		workspace_output(self->workspace, "Select/Create first a section");
	} else {
		self->selectedplugin = NULL;
		psy_audio_pluginsections_clearsection(&self->workspace->pluginsections,
			psy_property_key(self->selectedsection->property));
		newmachine_buildsections(self);
		psy_ui_app()->alignvalid = FALSE;
		psy_ui_component_align(&self->client);
		psy_ui_app()->alignvalid = TRUE;
		psy_ui_component_invalidate(&self->client);
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


int newmachine_isplugin(int type)
{
	return (type == psy_audio_PLUGIN) ||
	   (type == psy_audio_VST) ||
	   (type == psy_audio_VSTFX) ||
	   (type == psy_audio_LUA) ||
	   (type == psy_audio_LADSPA);
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
	psy_ui_component_invalidate(&self->pluginsview.component);	
}

void newmachine_enablegenerators(NewMachine* self)
{	
	self->pluginsview.generatorsenabled = TRUE;		
	psy_ui_component_invalidate(&self->pluginsview.component);	
}

void newmachine_preventgenerators(NewMachine* self)
{
	self->pluginsview.generatorsenabled = FALSE;	
	psy_ui_component_invalidate(&self->pluginsview.component);	
}

void newmachine_enableeffects(NewMachine* self)
{
	self->pluginsview.effectsenabled = TRUE;	
	psy_ui_component_invalidate(&self->pluginsview.component);	
}

void newmachine_preventeffects(NewMachine* self)
{
	self->pluginsview.effectsenabled = FALSE;	
	psy_ui_component_invalidate(&self->pluginsview.component);
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
}

void newmachine_appendmode(NewMachine* self)
{
	newmachine_enableall(self);
	self->mode = NEWMACHINE_APPEND;
	self->pluginsview.mode = NEWMACHINE_APPEND;	
}

void newmachine_addeffectmode(NewMachine* self)
{	
	newmachine_preventgenerators(self);
	newmachine_enableeffects(self);
	self->mode = NEWMACHINE_ADDEFFECT;
	self->pluginsview.mode = NEWMACHINE_ADDEFFECT;	
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

void newmachine_updateplugins(NewMachine* self)
{
	self->selectedplugin = NULL;
	pluginsview_setplugins(&self->pluginsview,
		workspace_pluginlist(self->workspace));
	psy_ui_component_align(&self->client);	
}

void newmachine_onplugincategorychanged(NewMachine* self, NewMachineDetail* sender)
{
	if (self->selectedplugin && workspace_pluginlist(self->workspace)) {
		psy_Property* plugin;

		plugin = psy_property_find(workspace_pluginlist(self->workspace),
			psy_property_key(self->selectedplugin), PSY_PROPERTY_TYPE_SECTION);
		if (plugin) {
			psy_Property* category;

			category = psy_property_at(plugin, "category", PSY_PROPERTY_TYPE_STRING);
			if (category) {
				psy_property_setitem_str(category, psy_ui_edit_text(&sender->categoryedit));
			}
			psy_audio_plugincatcher_save(&self->pluginsview.workspace->plugincatcher);
			newmachine_updateplugins(self);
		}
	}
}

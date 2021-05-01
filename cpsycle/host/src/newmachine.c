// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "newmachine.h"
// host
#include "resources/resource.h"
#include "styles.h"
// platform
#include "../../detail/portable.h"

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
static void newmachinesearch_onlanguagechanged(NewMachineSearch*);

// vtable
static psy_ui_ComponentVtable newmachinesearch_vtable;
static bool newmachinesearch_vtable_initialized = FALSE;

static void newmachinesearch_vtable_init(NewMachineSearch* self)
{
	if (!newmachinesearch_vtable_initialized) {
		newmachinesearch_vtable = *(self->component.vtable);
		newmachinesearch_vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			newmachinesearch_onlanguagechanged;
		newmachinesearch_vtable_initialized = TRUE;
	}
	self->component.vtable = &newmachinesearch_vtable;
}
// implementation
void newmachinesearch_init(NewMachineSearch* self, psy_ui_Component* parent,
	NewMachineFilter* filter)
{
	psy_ui_component_init(&self->component, parent, NULL);
	newmachinesearch_vtable_init(self);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_SEARCHFIELD);
	psy_ui_component_setstyletype_select(&self->component,
		STYLE_NEWMACHINE_SEARCHFIELD_SELECT);	
	self->filter = filter;
	psy_ui_image_init_resource_transparency(&self->image, &self->component,
		IDB_SEARCH_DARK, psy_ui_colour_white());	
	psy_ui_component_setalign(psy_ui_image_base(&self->image),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(psy_ui_image_base(&self->image),
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 1.0));	
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
	psy_ui_component_addstylestate(&self->component, psy_ui_STYLESTATE_SELECT);
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
	psy_ui_edit_settext(&self->edit,
		psy_ui_translate("newmachine.search-plugin"));
	self->hasdefaulttext = TRUE;
	if (self->filter) {
		newmachinefilter_settext(self->filter, "");
		psy_ui_component_removestylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
		psy_ui_component_setfocus(psy_ui_component_parent(&self->component));
	}
}

void newmachinesearch_onlanguagechanged(NewMachineSearch* self)
{
	if (self->hasdefaulttext) {
		psy_ui_edit_settext(&self->edit,
			psy_ui_translate("newmachine.search-plugin"));
		self->hasdefaulttext = TRUE;
	}
}

// NewMachineSearchBar
// implementation
void newmachinesearchbar_init(NewMachineSearchBar* self, psy_ui_Component* parent,
	NewMachineFilter* filter)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_SEARCHBAR);	
	// search field
	newmachinesearch_init(&self->search, &self->component, filter);
	psy_ui_component_setalign(newmachinesearch_base(&self->search),
		psy_ui_ALIGN_RIGHT);
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
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_RESCANBAR);	
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;	
	psy_ui_button_init_text(&self->rescan, &self->component, NULL,
		"newmachine.rescan");
	psy_ui_margin_init_em(&spacing, 0.2, 0.2, 0.2, 0.2);
	psy_ui_component_setspacing(psy_ui_button_base(&self->rescan), spacing);
	psy_ui_label_init_text(&self->desc, &self->component, NULL,
		"newmachine.in");	
	psy_ui_component_setspacing(psy_ui_label_base(&self->desc), spacing);
	psy_ui_button_init_text(&self->selectdirectories, &self->component, NULL,
		"newmachine.plugin-directories");
	psy_ui_component_setspacing(psy_ui_button_base(&self->selectdirectories),
		spacing);
	psy_ui_button_setbitmapresource(&self->selectdirectories,
		IDB_SETTINGS_DARK);
	psy_ui_button_setbitmaptransparency(&self->selectdirectories,
		psy_ui_colour_white());
	psy_signal_connect(&self->selectdirectories.signal_clicked, self,
		newmachinerescanbar_onselectdirectories);
}

void newmachinerescanbar_onselectdirectories(NewMachineRescanBar* self,
	psy_ui_Component* sender)
{
	workspace_selectview(self->workspace, VIEW_ID_SETTINGSVIEW, 4, 0);
}

// NewMachineSectionBar
// implementation
void newmachinesectionbar_init(NewMachineSectionBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin spacing;

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_SECTIONBAR);
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
	psy_ui_margin_init_em(&spacing, 0.2, 0.2, 0.2, 0.2);
	psy_ui_component_setspacing_children(&self->component, spacing);
}

// NewMachineSortBar
// prototypes
static void newmachinesortbar_onsortbyfavorite(NewMachineSortBar*, psy_ui_Component* sender);
static void newmachinesortbar_onsortbyname(NewMachineSortBar*, psy_ui_Component* sender);
static void newmachinesortbar_onsortbytype(NewMachineSortBar*, psy_ui_Component* sender);
static void newmachinesortbar_onsortbymode(NewMachineSortBar*, psy_ui_Component* sender);
// implementation
void newmachinesortbar_init(NewMachineSortBar* self, psy_ui_Component* parent,
	NewMachineSort* sort)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_SORTBAR);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);	
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
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
	psy_ui_component_setspacing_children(&self->component,
		psy_ui_margin_make_em(0.2, 0.2, 0.2, 0.2));
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
static void newmachinedetail_onloadnewblitz(NewMachineDetail*,
	psy_ui_Component* sender);
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
	double numcol0;

	// component
	psy_ui_component_init(&self->component, parent, NULL);
	newmachinedetail_vtable_init(self);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_DETAIL);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(40.0, 0.0));
	psy_signal_init(&self->signal_categorychanged);	
	self->plugin = NULL;
	self->workspace = workspace;
	numcol0 = 12;
	// plugin name
	labelpair_init(&self->plugname, &self->component, "newmachine.name", numcol0);
	psy_ui_component_setalign(&self->plugname.second.component,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->plugname.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 1.0, 0.0);
	psy_ui_component_setmargin(&self->plugname.component, margin);
	// description
	psy_ui_label_init(&self->desclabel, &self->component, NULL);
	psy_ui_label_settextalignment(&self->desclabel, psy_ui_ALIGNMENT_TOP);
	psy_ui_component_setalign(&self->desclabel.component, psy_ui_ALIGN_CLIENT);
	// bottom
	psy_ui_component_init(&self->bottom, &self->component, NULL);
	psy_ui_component_setalign(&self->bottom, psy_ui_ALIGN_BOTTOM);
	psy_ui_margin_init_em(&spacing, 0.5, 1.0, 0.5, 0.0);
	psy_ui_component_setspacing(&self->bottom, spacing);
	psy_ui_component_setdefaultalign(&self->bottom,
		psy_ui_ALIGN_TOP, psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_label_init_text(&self->compatlabel, &self->bottom, NULL,
		"newmachine.song-loading-compatibility");
	psy_ui_label_settextalignment(&self->compatlabel, psy_ui_ALIGNMENT_LEFT);
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
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 0.5, 0.0);
	psy_ui_component_setmargin(psy_ui_label_base(&self->compatlabel), margin);	
	// details	
	psy_ui_component_init(&self->details, &self->component, NULL);
	psy_ui_component_setalign(&self->details, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setdefaultalign(&self->details, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults())); 
	// category
	psy_ui_component_init(&self->category, &self->details, NULL);	
	psy_ui_label_init_text(&self->categorydesc, &self->category, NULL,
		"newmachine.category");
	psy_ui_label_settextalignment(&self->categorydesc,
		psy_ui_ALIGNMENT_RIGHT);
	psy_ui_label_setcharnumber(&self->categorydesc, numcol0);
	psy_ui_component_setalign(psy_ui_label_base(&self->categorydesc),
		psy_ui_ALIGN_LEFT);
	psy_ui_edit_init(& self->categoryedit, &self->category);
	psy_ui_edit_enableinputfield(&self->categoryedit);	
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 0.0, 1.5);
	psy_ui_component_setmargin(psy_ui_edit_base(&self->categoryedit), margin);
	psy_ui_component_setalign(psy_ui_edit_base(&self->categoryedit),
		psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->categoryedit.signal_accept,
		self, newmachinedetail_oncategoryeditaccept);
	psy_signal_connect(&self->categoryedit.signal_reject,
		self, newmachinedetail_oncategoryeditreject);
	labelpair_init_right(&self->apiversion, &self->details, "newmachine.apiversion", numcol0);
	labelpair_init_right(&self->version, &self->details, "newmachine.version", numcol0);
	labelpair_init_right(&self->dllname, &self->details, "newmachine.dllname", numcol0);
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
	psy_ui_label_settext(&self->plugname.second, text);
}

void newmachinedetail_setdllname(NewMachineDetail* self, const char* text)
{	
	psy_ui_label_settext(&self->dllname.second, text);
}

void newmachinedetail_setcategoryname(NewMachineDetail* self, const char* text)
{	
	psy_ui_edit_settext(&self->categoryedit, text);	
}

void newmachinedetail_setplugversion(NewMachineDetail* self, int16_t version)
{
	char valstr[64];

	psy_snprintf(valstr, 64, "%d", (int)version);	
	psy_ui_label_settext(&self->version.second, valstr);
}

void newmachinedetail_setapiversion(NewMachineDetail* self, int16_t apiversion)
{
	char valstr[64];

	psy_snprintf(valstr, 64, "%d", (int)apiversion);
	psy_ui_label_settext(&self->apiversion.second, valstr);
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
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_FILTERBAR);
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
static void newmachinecategorybar_onclicked(NewMachineCategoryBar*,
	psy_ui_Button* sender);
static void newmachinecategorybar_ondragover(NewMachineCategoryBar*, psy_ui_DragEvent*);
static void newmachinecategorybar_ondrop(NewMachineCategoryBar*, psy_ui_DragEvent*);
// vtable
static psy_ui_ComponentVtable newmachinecategorybar_vtable;
static bool newmachinecategorybar_vtable_initialized = FALSE;

static void newmachinecategorybar_vtable_init(NewMachineCategoryBar* self)
{
	if (!newmachinecategorybar_vtable_initialized) {
		newmachinecategorybar_vtable = *(self->component.vtable);		
		newmachinecategorybar_vtable.ondragover =
			(psy_ui_fp_component_ondragover)
			newmachinecategorybar_ondragover;
		newmachinecategorybar_vtable.ondrop =
			(psy_ui_fp_component_ondrop)
			newmachinecategorybar_ondrop;		
		newmachinecategorybar_vtable_initialized = TRUE;
	}
	self->component.vtable = &newmachinecategorybar_vtable;
}
// implementation
void newmachinecategorybar_init(NewMachineCategoryBar* self,
	psy_ui_Component* parent, NewMachineFilter* filter,
	psy_audio_PluginCatcher* plugincatcher)
{	
	assert(self);
	assert(plugincatcher);

	psy_ui_component_init(&self->component, parent, NULL);
	newmachinecategorybar_vtable_init(self);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_CATEGORYBAR);
	psy_ui_component_init_align(&self->client, &self->component,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setdefaultalign(&self->client, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->plugincatcher = plugincatcher;
	self->filter = filter;
}

void newmachinecategorybar_build(NewMachineCategoryBar* self)
{
	psy_TableIterator it;
	psy_ui_Button* button;

	assert(self);

	psy_ui_component_clear(&self->client);	
	button = psy_ui_button_allocinit(&self->client, &self->client);	
	psy_ui_button_settext(button, "newmachine.anycategory");
	button->stoppropagation = FALSE;
	if (self->filter) {
		psy_audio_PluginCategories categories;

		if (newmachinefilter_useanycategory(self->filter)) {
			psy_ui_button_highlight(button);
		} else {
			psy_ui_button_disablehighlight(button);
		}
		psy_signal_connect(&button->signal_clicked, self,
			newmachinecategorybar_onclicked);
		psy_audio_plugincategories_init(&categories,
			psy_audio_plugincatcher_plugins(self->plugincatcher));		
		for (it = psy_audio_plugincategories_begin(&categories);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			const char* category;

			category = (const char*)psy_tableiterator_value(&it);
			if (category) {
				psy_ui_Button* button;

				button = psy_ui_button_allocinit(&self->client, &self->client);
				psy_ui_button_preventtranslation(button);
				psy_ui_button_settext(button, category);
				button->stoppropagation = FALSE;
				if (!newmachinefilter_useanycategory(self->filter) &&
						(newmachinefilter_hascategory(self->filter, category))) {
					psy_ui_button_highlight(button);					
				}
				psy_signal_connect(&button->signal_clicked, self,
					newmachinecategorybar_onclicked);
			}
		}
		psy_audio_plugincategories_dispose(&categories);
	} else {
		psy_ui_button_highlight(button);
	}	
}

void newmachinecategorybar_onclicked(NewMachineCategoryBar* self, psy_ui_Button* sender)
{
	if (self->filter) {		
		psy_ui_Component* first;

		if (strcmp(psy_ui_button_text(sender), "newmachine.anycategory") == 0) {
			newmachinefilter_anycategory(self->filter);
			psy_ui_component_removestylestate_children(&self->client,
				psy_ui_STYLESTATE_SELECT);				
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
		first = psy_ui_component_at(&self->client, 0);
		if (first) {
			if ((newmachinefilter_useanycategory(self->filter))) {
				psy_ui_component_addstylestate(first,
					psy_ui_STYLESTATE_SELECT);
			} else {
				psy_ui_component_removestylestate(first,
					psy_ui_STYLESTATE_SELECT);
			}
		}
	}
}

void newmachinecategorybar_ondragover(NewMachineCategoryBar* self, psy_ui_DragEvent* ev)
{
	psy_List* p;
	psy_List* q;
	psy_ui_Button* button;	

	button = NULL;
	for (p = q = psy_ui_component_children(&self->client, psy_ui_NONRECURSIVE);
		p != NULL; p = p->next) {
		psy_ui_Component* component;
		psy_ui_RealRectangle position;

		component = (psy_ui_Component*)p->entry;
		position = psy_ui_component_position(component);
		if (psy_ui_realrectangle_intersect(&position, ev->mouse.pt)) {
			ev->preventdefault = TRUE;
			break;
		}
	}	
}

void newmachinecategorybar_ondrop(NewMachineCategoryBar* self, psy_ui_DragEvent* ev)
{
	psy_List* p;
	psy_List* q;
	psy_ui_Button* button;
	bool changed;

	button = NULL;
	for (p = q = psy_ui_component_children(&self->client, psy_ui_NONRECURSIVE);
			p != NULL; p = p->next) {
		psy_ui_Component* component;
		psy_ui_RealRectangle position;

		component = (psy_ui_Component*)p->entry;
		position = psy_ui_component_position(component);
		if (psy_ui_realrectangle_intersect(&position, ev->mouse.pt)) {
			/* todo avoid downcast */
			button = (psy_ui_Button*)component;
			break;
		}
	}
	if (button) {
		psy_List* p;
		const char* categoryname;
		
		if (strcmp(psy_ui_button_text(button), "newmachine.anycategory") == 0) {
			categoryname = "";
		} else {
			categoryname = psy_ui_button_text(button);
		}
		changed = FALSE;
		for (p = psy_property_begin(ev->dataTransfer); p != NULL; p = p->next) {
			psy_Property* plugindrag;
			const char* plugid;			
			psy_Property* plugin;

			plugindrag = (psy_Property*)p->entry;
			plugid = psy_property_key(plugindrag);
			plugin = psy_audio_plugincatcher_at(self->plugincatcher, plugid);
			if (plugin) {
				psy_Property* category;

				category = psy_property_at(plugin, "category",
					PSY_PROPERTY_TYPE_NONE);
				if (category) {
					psy_property_setitem_str(category, categoryname);
				}
				changed = TRUE;
			}
		}
		if (changed) {
			psy_audio_plugincatcher_save(self->plugincatcher);
			psy_audio_plugincatcher_notifychange(self->plugincatcher);
		}			
	}	
}

// PluginScanView
// prototypes
void pluginscanview_inittasklist(PluginScanView*);
void pluginscanview_onabort(PluginScanView*, psy_ui_Button* sender);
// implementation
void pluginscanview_init(PluginScanView* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_component_setspacing(&self->component,
		psy_ui_margin_make_em(0.5, 0.5, 0.0, 0.0));
	psy_ui_label_init_text(&self->scan, &self->component, NULL,
		"newmachine.scanning");
	psy_ui_component_setalign(psy_ui_label_base(&self->scan),
		psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(psy_ui_label_base(&self->scan),
		psy_ui_margin_make_em(2.0, 0.0, 2.0, 0.0));
	psy_ui_component_init_align(&self->left, &self->component,
		psy_ui_ALIGN_LEFT);	
	psy_ui_component_setspacing(&self->left,
		psy_ui_margin_make_em(0.0, 4.0, 0.0, 4.0));
	psy_ui_component_init_align(&self->right, &self->component,
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_init_align(&self->client, &self->component,
		psy_ui_ALIGN_CLIENT);	
	// filename
	psy_ui_label_init(&self->scanfile, &self->client, NULL);
	psy_ui_label_preventtranslation(&self->scanfile);
	psy_ui_label_preventwrap(&self->scanfile);
	psy_ui_component_setalign(psy_ui_label_base(&self->scanfile),
		psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(psy_ui_label_base(&self->scanfile),
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.0));
	psy_ui_component_init(&self->abortbar, &self->client, NULL);
	psy_ui_component_setmargin(&self->abortbar,
		psy_ui_margin_make_em(2.0, 0.0, 0.0, 20.0));
	psy_ui_component_setalign(&self->abortbar, psy_ui_ALIGN_TOP);
	psy_ui_button_init_text_connect(&self->abort, &self->abortbar, NULL,
		"Stop", self, pluginscanview_onabort);
	psy_ui_component_setalign(psy_ui_button_base(&self->abort),
		psy_ui_ALIGN_LEFT);
	psy_ui_button_preventtranslation(&self->abort);
	pluginscanview_inittasklist(self);
}

void pluginscanview_inittasklist(PluginScanView* self)
{
	psy_List* p;

	for (p = self->workspace->plugincatcher.scantasks; p != NULL; p = p->next) {
		psy_audio_PluginScanTask* task;
		psy_ui_Label* label;
		
		task = (psy_audio_PluginScanTask*)p->entry;
		label = psy_ui_label_allocinit(&self->left, NULL);
		psy_ui_label_preventtranslation(label);
		psy_ui_label_settext(label, task->label);
		psy_ui_component_setstyletype_select(&label->component,
			psy_ui_STYLE_BUTTON_SELECT);
		psy_ui_component_setmargin(psy_ui_label_base(label),
			psy_ui_margin_make_em(0.0, 0.0, 1.0, 0.0));
		psy_ui_component_setalign(&label->component, psy_ui_ALIGN_TOP);
	}
}

void pluginscanview_onabort(PluginScanView* self, psy_ui_Button* sender)
{
	psy_audio_plugincatcher_abort(&self->workspace->plugincatcher);
}

void pluginscanview_selecttask(PluginScanView* self, psy_audio_PluginScanTask* task)
{
	uintptr_t index;
	psy_List* p;

	index = 0;
	for (p = self->workspace->plugincatcher.scantasks; p != NULL; p = p->next, ++index) {
		psy_audio_PluginScanTask* curr;

		curr = (psy_audio_PluginScanTask*)p->entry;
		if (strcmp(curr->label, task->label) == 0) {
			break;
		}
	}
	if (p) {
		psy_ui_Component* component;

		psy_ui_component_removestylestate_children(&self->left,
			psy_ui_STYLESTATE_SELECT);
		component = psy_ui_component_at(&self->left, index);
		if (component) {
			psy_ui_component_addstylestate(component, psy_ui_STYLESTATE_SELECT);
		}
	}
}

// NewMachineSection
// prototypes
static void newmachinesection_findplugins(NewMachineSection*);
static void newmachinesection_showedit(NewMachineSection*);
static void newmachinesection_onlabelclick(NewMachineSection*, psy_ui_Label* sender,
	psy_ui_MouseEvent*);
static void newmachinesection_onmousedown(NewMachineSection*,
	psy_ui_MouseEvent*);
static void newmachinesection_oneditaccept(NewMachineSection*, psy_ui_Edit* sender);
static void newmachinesection_oneditreject(NewMachineSection*, psy_ui_Edit* sender);
static void newmachinesection_ondragover(NewMachineSection*, psy_ui_DragEvent*);
static void newmachinesection_ondrop(NewMachineSection*, psy_ui_DragEvent*);
static void newmachinesection_onlanguagechanged(NewMachineSection*);
// vtable
static psy_ui_ComponentVtable newmachinesection_vtable;
static bool newmachinesection_vtable_initialized = FALSE;

static void newmachinesection_vtable_init(NewMachineSection* self)
{
	if (!newmachinesection_vtable_initialized) {
		newmachinesection_vtable = *(self->component.vtable);
		newmachinesection_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			newmachinesection_onmousedown;
		newmachinesection_vtable.ondragover =
			(psy_ui_fp_component_ondragover)
			newmachinesection_ondragover;
		newmachinesection_vtable.ondrop =
			(psy_ui_fp_component_ondrop)
			newmachinesection_ondrop;
		newmachinesection_vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			newmachinesection_onlanguagechanged;
		newmachinesection_vtable_initialized = TRUE;
	}
	self->component.vtable = &newmachinesection_vtable;
}
// implementation
void newmachinesection_init(NewMachineSection* self, psy_ui_Component* parent,
	psy_Property* section, psy_ui_Edit* edit, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	newmachinesection_vtable_init(self);	
	psy_signal_init(&self->signal_selected);
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_renamed);
	psy_ui_component_setstyletypes(&self->component,
		STYLE_NEWMACHINE_SECTION, psy_INDEX_INVALID,
		STYLE_NEWMACHINE_SECTION_SELECTED, psy_INDEX_INVALID);	
	self->section  = section;
	self->edit = edit;
	self->preventedit = TRUE;
	self->workspace = workspace;
	psy_ui_component_init(&self->header, &self->component, NULL);
	psy_ui_component_setalign(&self->header, psy_ui_ALIGN_TOP);
	psy_ui_component_setstyletype(&self->header,
		STYLE_NEWMACHINE_SECTION_HEADER);
	psy_ui_component_setdefaultalign(&self->header, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_label_init(&self->label, &self->header, NULL);
	psy_ui_label_preventtranslation(&self->label);
	psy_ui_label_settext(&self->label, newmachinesection_name(self));
	pluginsview_init(&self->pluginview, &self->component);
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

void newmachinesection_destroy(NewMachineSection* self)
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
	psy_Property* property, psy_ui_Edit* edit, Workspace* workspace)
{
	NewMachineSection* rv;

	rv = newmachinesection_alloc();
	if (rv) {
		newmachinesection_init(rv, parent, property, edit, workspace);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

void newmachinesection_findplugins(NewMachineSection* self)
{
	psy_Property* plugins;
	
	if (self->section) {
		plugins = psy_property_at(self->section, "plugins",
			PSY_PROPERTY_TYPE_SECTION);		
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
		if (strcmp(psy_property_key(self->section), "favorites") == 0) {
			return psy_ui_translate("newmachine.favorites");
		}
		return psy_property_at_str(self->section, "name",
			psy_property_key(self->section));		
	}
	return "";	
}

void newmachinesection_onlanguagechanged(NewMachineSection* self)
{
	if (self->section) {
		if (strcmp(psy_property_key(self->section), "favorites") == 0) {
			psy_ui_label_settext(&self->label,
				psy_ui_translate("newmachine.favorites"));
		}
	}
}

void newmachinesection_onlabelclick(NewMachineSection* self, psy_ui_Label* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->edit) {		
		self->preventedit = FALSE;		
		psy_ui_edit_settext(self->edit, self->label.text);
		psy_ui_edit_setsel(self->edit, 0, -1);
		newmachinesection_showedit(self);		
	}
}

void newmachinesection_showedit(NewMachineSection* self)
{
	psy_ui_RealRectangle colscreenposition;
	psy_ui_RealRectangle screenposition;
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->label.component);
	colscreenposition = psy_ui_component_screenposition(&self->label.component);
	colscreenposition.right = colscreenposition.left +
		40.0 * tm->tmAveCharWidth;
	screenposition = psy_ui_component_screenposition(
		psy_ui_component_parent(&self->edit->component));	
	psy_ui_component_setposition(psy_ui_edit_base(self->edit),
		psy_ui_rectangle_make(
			psy_ui_point_make(
				psy_ui_value_make_px(colscreenposition.left - screenposition.left),
				psy_ui_value_make_px(colscreenposition.top - screenposition.top +
					(colscreenposition.bottom - colscreenposition.top - tm->tmHeight) / 2.0)),
			psy_ui_size_make(
				psy_ui_value_make_px(colscreenposition.right - colscreenposition.left),
				psy_ui_value_make_eh(1.0))));
	psy_ui_component_show(psy_ui_edit_base(self->edit));
	psy_ui_component_invalidate(psy_ui_component_parent(psy_ui_edit_base(self->edit)));
	psy_ui_component_setfocus(psy_ui_edit_base(self->edit));
}


void newmachinesection_oneditaccept(NewMachineSection* self, psy_ui_Edit* sender)
{
	if (!self->preventedit) {
		psy_Property* name;

		self->preventedit = TRUE;
		psy_ui_component_hide(psy_ui_edit_base(sender));
		name = psy_property_at(self->section, "name",
			PSY_PROPERTY_TYPE_STRING);
		if (name) {
			psy_property_setitem_str(name, psy_ui_edit_text(sender));
			psy_ui_label_settext(&self->label, psy_ui_edit_text(sender));
			psy_signal_emit(&self->signal_renamed, self, 0);
		}				
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

void newmachinesection_onmousedown(NewMachineSection* self,
	psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		newmachinesection_mark(self);
		psy_signal_emit(&self->signal_selected, self, 0);
	}
}

void newmachinesection_mark(NewMachineSection* self)
{
	psy_ui_component_removestylestate_children(
		psy_ui_component_parent(&self->component),
		psy_ui_STYLESTATE_SELECT);	
	psy_ui_component_addstylestate(&self->component, psy_ui_STYLESTATE_SELECT);
}

void newmachinsection_clearselection(NewMachineSection* self)
{
	pluginsview_clearselection(&self->pluginview);
}

void newmachinesection_ondragover(NewMachineSection* self, psy_ui_DragEvent* ev)
{	
	if (ev->dataTransfer) {
		psy_List* p;

		for (p = psy_property_begin(ev->dataTransfer); p != NULL; p = p->next) {
			psy_Property* plugin;

			plugin = (psy_Property*)p->entry;
			if (!psy_audio_pluginsections_pluginbyid(
					&self->workspace->pluginsections, self->section,
					psy_property_key(plugin))) {
				ev->preventdefault = TRUE;				
				break;
			}
		}
	}
}

void newmachinesection_ondrop(NewMachineSection* self, psy_ui_DragEvent* ev)
{	
	ev->preventdefault = TRUE;
	if (ev->dataTransfer) {		
		psy_List* p;
		psy_audio_PluginSections* sections;
		bool drop;

		drop = FALSE;
		sections = &self->workspace->pluginsections;
		for (p = psy_property_begin(ev->dataTransfer); p != NULL; p = p->next) {
			psy_Property* plugindrag;
			const char* plugid;

			plugindrag = (psy_Property*)p->entry;
			plugid = psy_property_key(plugindrag);
			if (!psy_audio_pluginsections_pluginbyid(
					&self->workspace->pluginsections, self->section,
					plugid)) {									
				psy_Property* section;
				psy_Property* plugin;

				section = psy_audio_pluginsections_section(sections,
				psy_property_at_str(plugindrag, "section", ""));
				if (section) {
					plugin = psy_audio_pluginsections_pluginbyid(sections, section,
						plugid);
				} else {
					plugin = psy_audio_plugincatcher_at(&self->workspace->plugincatcher,
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
			newmachinesection_findplugins(self);
			psy_signal_emit(&self->signal_changed, self, 0);
		}
	}
}

// NewMachineSectionsHeader
void newmachinesectionsheader_init(NewMachineSectionsHeader* self,
	psy_ui_Component* parent, uintptr_t iconresourceid)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_SECTIONS_HEADER);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_appdefaults()));
	psy_ui_image_init_resource_transparency(&self->icon,
		&self->component, iconresourceid, psy_ui_colour_white());	
	psy_ui_button_init(&self->expand, &self->component, NULL);	
	psy_ui_button_setbitmapresource(&self->expand, IDB_EXPAND_DARK);
	psy_ui_button_setbitmaptransparency(&self->expand, psy_ui_colour_white());	
	psy_ui_component_setspacing(psy_ui_button_base(&self->expand),
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	psy_ui_component_setalign(psy_ui_button_base(&self->expand),
		psy_ui_ALIGN_RIGHT);
}

// NewMachine
// prototypes
static void newmachine_ondestroy(NewMachine*);
static void newmachine_onpluginselected(NewMachine*, PluginsView* sender);
static void newmachine_onpluginchanged(NewMachine*, PluginsView* parent);
static void newmachine_onplugincachechanged(NewMachine*, psy_audio_PluginCatcher*);
static void newmachine_onmousedown(NewMachine*, psy_ui_MouseEvent*);
static void newmachine_oncreatesection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onremovesection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onclearsection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onaddtosection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onremovefromsection(NewMachine*, psy_ui_Component* sender);
static bool newmachine_checkplugin(NewMachine*);
static bool newmachine_checksection(NewMachine*);
static void newmachine_onfocus(NewMachine*, psy_ui_Component* sender);
static void newmachine_onrescan(NewMachine*, psy_ui_Component* sender);
static void newmachine_onpluginscanprogress(NewMachine*, Workspace*,
	int progress);
static void newmachine_ontimer(NewMachine*, uintptr_t timerid);
static void newmachine_buildsections(NewMachine*);
static void newmachine_updateplugins(NewMachine*);
static void newmachine_onplugincategorychanged(NewMachine*, NewMachineDetail* sender);
static void newmachine_ontoggleexpandsections(NewMachine*, psy_ui_Button* sender);
static void newmachine_ontoggleexpandall(NewMachine*, psy_ui_Button* sender);
static void newmachine_ontabbarchanged(NewMachine*, psy_ui_TabBar* sender,
	uintptr_t index);
static void newmachine_checkselections(NewMachine*, PluginsView* sender);
static void newmachine_onsectionselected(NewMachine*,
	NewMachineSection* sender);
static void newmachine_onsectionchanged(NewMachine*,
	NewMachineSection* sender);
static void newmachine_onsectionrenamed(NewMachine*, NewMachineSection* sender);
static void newmachine_alignsections(NewMachine*);
static void newmachine_onlanguagechanged(NewMachine*);
static void newmachine_onscanfile(NewMachine*, psy_audio_PluginCatcher* sender, const char* path, int type);
static void newmachine_onscantaskstart(NewMachine*, psy_audio_PluginCatcher* sender,
	psy_audio_PluginScanTask*);
// vtable
static psy_ui_ComponentVtable newmachine_vtable;
static bool newmachine_vtable_initialized = FALSE;

static void newmachine_vtable_init(NewMachine* self)
{
	if (!newmachine_vtable_initialized) {
		newmachine_vtable = *(self->component.vtable);
		newmachine_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			newmachine_ondestroy;
		newmachine_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			newmachine_onmousedown;
		newmachine_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			newmachine_ontimer;
		newmachine_vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			newmachine_onlanguagechanged;
		newmachine_vtable_initialized = TRUE;
	}
	self->component.vtable = &newmachine_vtable;
}
// implementation
void newmachine_init(NewMachine* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	newmachine_vtable_init(self);	
	self->workspace = workspace;	
	self->mode = NEWMACHINE_APPEND;
	self->appendstack = FALSE;
	self->restoresection = SECTION_ID_MACHINEVIEW_WIRES;
	self->selectedplugin = NULL;
	self->selectedsection = NULL;
	self->newsectioncount = 0;	
	psy_table_init(&self->newmachinesections);	
	newmachinefilter_init(&self->filter);	
	newmachinesort_init(&self->sort);
	// Notebook	(switches scanprogress/pluginselectview)
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
		psy_ui_notebook_base(&self->notebook), self->workspace);	
	// sectiongroup
	psy_ui_component_init(&self->sectiongroup, &self->client, NULL);
	psy_ui_component_setalign(&self->sectiongroup, psy_ui_ALIGN_CLIENT);
	// sectionsheader
	newmachinesectionsheader_init(&self->sectionsheader, &self->sectiongroup,
		IDB_HEART_DARK);
	psy_ui_component_setalign(&self->sectionsheader.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->sectionsheader.expand.signal_clicked, self,
		newmachine_ontoggleexpandsections);	
	psy_ui_tabbar_init(&self->navsections, &self->sectionsheader.component);
	psy_signal_connect(&self->navsections.signal_change, self,
		newmachine_ontabbarchanged);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->navsections),
		psy_ui_ALIGN_CLIENT);
	// sectionbar
	newmachinesectionbar_init(&self->sectionbar, &self->sectiongroup,
		self->workspace);
	psy_ui_component_setalign(&self->sectionbar.component, psy_ui_ALIGN_BOTTOM);	
	// sections
	psy_ui_component_init(&self->sections, &self->sectiongroup, NULL);	
	// Edit
	psy_ui_edit_init(&self->edit, &self->sectiongroup);
	psy_ui_edit_enableinputfield(&self->edit);
	psy_ui_component_hide(&self->edit.component);	
	// section scroll
	psy_ui_component_setoverflow(&self->sections, psy_ui_OVERFLOW_VSCROLL);
	psy_ui_component_setscrollstep(&self->sections, 
		psy_ui_size_make_em(0.0, 1.0));
	psy_ui_scroller_init(&self->scroller_sections, &self->sections,
		&self->sectiongroup, NULL);
	psy_ui_component_setalign(&self->sections, psy_ui_ALIGN_HCLIENT);
	psy_ui_component_settabindex(&self->scroller_sections.component, 0);
	psy_ui_component_setalign(&self->scroller_sections.component,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setbackgroundmode(&self->scroller_sections.pane,
		psy_ui_SETBACKGROUND);
	// all	
	psy_ui_component_init_align(&self->all, &self->client,
		psy_ui_ALIGN_CLIENT);	
	psy_ui_component_setmargin(&self->all, 
		psy_ui_margin_make_em(0.5, 0.0, 1.0, 0.0));
	// pluginview header
	newmachinesectionsheader_init(&self->pluginsheader, &self->all,
		IDB_TRAY_DARK);	
	psy_ui_component_setalign(&self->pluginsheader.component, psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->pluginslabel, &self->pluginsheader.component, NULL,
		"newmachine.all");
	psy_signal_connect(&self->pluginsheader.expand.signal_clicked, self,
		newmachine_ontoggleexpandall);
	// plugin header bars
	psy_ui_component_init(&self->pluginsheaderbars, &self->pluginsheader.component, NULL);	
	psy_ui_component_setalign(&self->pluginsheaderbars, psy_ui_ALIGN_CLIENT);	
	// filter bar
	newmachinefilterbar_init(&self->filterbar, &self->pluginsheaderbars, &self->filter);
	psy_ui_component_setalign(&self->filterbar.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(&self->filterbar.component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 2.0));
	// sort bar
	newmachinesortbar_init(&self->sortbar, &self->pluginsheaderbars, &self->sort);
	psy_ui_component_setalign(&self->sortbar.component, psy_ui_ALIGN_LEFT);	
	// all categeory bar
	newmachinecategorybar_init(&self->categorybar, &self->all, &self->filter,
		workspace_plugincatcher(self->workspace));
	psy_ui_component_setalign(&self->categorybar.component, psy_ui_ALIGN_TOP);	
	// Plugins View
	pluginsview_init(&self->pluginsview, &self->all);
	pluginsview_setfilter(&self->pluginsview, &self->filter);
	pluginsview_setsort(&self->pluginsview, &self->sort);
	psy_ui_component_setwheelscroll(&self->pluginsview.component, 1);
	psy_ui_component_setoverflow(&self->pluginsview.component, psy_ui_OVERFLOW_VSCROLL);
	newmachinefilterbar_setfilters(&self->filterbar, &self->filter);
	psy_ui_scroller_init(&self->scroller_all, &self->pluginsview.component,
		&self->all, NULL);
	psy_ui_component_setbackgroundmode(&self->scroller_all.pane, psy_ui_SETBACKGROUND);
	psy_ui_component_settabindex(&self->scroller_all.component, 1);
	psy_ui_component_setalign(&self->scroller_all.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->pluginsview.component, psy_ui_ALIGN_HCLIENT);
	// Details
	newmachinedetail_init(&self->detail, &self->component, &self->filter,
		self->workspace);
	psy_ui_component_setalign(&self->detail.component, psy_ui_ALIGN_LEFT);	
	psy_signal_connect(&self->detail.signal_categorychanged, self,
		newmachine_onplugincategorychanged);	
	// Rescanbar
	newmachinerescanbar_init(&self->rescanbar, &self->component, self->workspace);
	psy_ui_component_setalign(&self->rescanbar.component, psy_ui_ALIGN_BOTTOM);
	// filter
	newmachinesearchbar_setfilter(&self->searchbar, &self->filter);
	// connect to signals
	psy_signal_init(&self->signal_selected);	
	psy_signal_connect(&self->pluginsview.signal_selected, self,
		newmachine_onpluginselected);
	psy_signal_connect(&self->pluginsview.signal_changed, self,
		newmachine_onpluginchanged);	
	psy_signal_connect(&workspace->plugincatcher.signal_changed, self,
		newmachine_onplugincachechanged);
	psy_signal_connect(&self->component.signal_focus, self,
		newmachine_onfocus);	
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
	psy_signal_connect(&workspace->signal_scantaskstart, self,
		newmachine_onscantaskstart);
	psy_signal_connect(&workspace->signal_scanfile, self,
		newmachine_onscanfile);
	psy_ui_notebook_select(&self->notebook, 0);
	newmachinecategorybar_build(&self->categorybar);
	psy_ui_component_align(&self->categorybar.component);
	newmachine_buildsections(self);
	newmachine_updateplugins(self);
}

void newmachine_ondestroy(NewMachine* self)
{
	newmachinefilter_dispose(&self->filter);
	newmachinesort_dispose(&self->sort);
	psy_signal_dispose(&self->signal_selected);
	psy_table_dispose(&self->newmachinesections);
}

void newmachine_onpluginselected(NewMachine* self, PluginsView* sender)
{		
	psy_Property* selected;

	selected = pluginsview_selectedplugin(sender);
	self->selectedplugin = selected;
	newmachinedetail_update(&self->detail, self->selectedplugin);
	newmachine_checkselections(self, sender);
	psy_signal_emit(&self->signal_selected, self, 1, selected);
	if (selected) {
		plugincatcher_incfavorite(workspace_plugincatcher(self->workspace),
			psy_property_key(selected));		
	}	
	newmachine_updateplugins(self);
}

void newmachine_onpluginchanged(NewMachine* self, PluginsView* sender)
{	
	self->selectedplugin = pluginsview_selectedplugin(sender);
	newmachine_checkselections(self, sender);
	newmachinedetail_update(&self->detail, self->selectedplugin);
}

void newmachine_checkselections(NewMachine* self, PluginsView* sender)
{
	psy_TableIterator it;

	assert(self);

	for (it = psy_table_begin(&self->newmachinesections);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		NewMachineSection* section;

		section = (NewMachineSection*)psy_tableiterator_value(&it);
		if (&section->pluginview != sender) {
			newmachinsection_clearselection(section);
		}
	}
	if (sender != &self->pluginsview) {
		pluginsview_clearselection(&self->pluginsview);
	}
}

void newmachine_onplugincachechanged(NewMachine* self,
	psy_audio_PluginCatcher* sender)
{
	newmachine_updateplugins(self);
	newmachinefilter_reset(&self->filter);
	newmachinedetail_reset(&self->detail);	
	newmachinecategorybar_build(&self->categorybar);
	psy_ui_component_align_full(&self->client);
	psy_ui_component_invalidate(&self->client);
}

void newmachine_oncreatesection(NewMachine* self, psy_ui_Component* sender)
{
	char sectionkey[64];

	psy_snprintf(sectionkey, 64, "section%d", (int)self->newsectioncount);
	while (psy_audio_pluginsections_section(&self->workspace->pluginsections,
			sectionkey)) {
		++self->newsectioncount;
		psy_snprintf(sectionkey, 64, "section%d", (int)self->newsectioncount);
	}		
	psy_audio_pluginsections_add(&self->workspace->pluginsections,
		sectionkey, NULL);
	++self->newsectioncount;
	newmachine_buildsections(self);
}

void newmachine_buildsections(NewMachine* self)
{
	psy_List* p;
	uintptr_t i;
	uintptr_t selidx;

	self->selectedsection = NULL;
	psy_table_clear(&self->newmachinesections);
	psy_ui_component_clear(&self->sections);	
	p = psy_property_begin(self->workspace->pluginsections.sections);
	selidx = psy_INDEX_INVALID;
	for (i = 0; p != 0; p = p->next, ++i) {
		psy_Property* property;
		NewMachineSection* section;

		property = (psy_Property*)psy_list_entry(p);
		section = newmachinesection_allocinit(&self->sections, property,
			&self->edit, self->workspace);		
		if (section) {
			psy_signal_connect(&section->pluginview.signal_selected, self,
				newmachine_onpluginselected);
			psy_signal_connect(&section->pluginview.signal_changed, self,
				newmachine_onpluginchanged);
			psy_signal_connect(&section->signal_selected, self,
				newmachine_onsectionselected);
			psy_signal_connect(&section->signal_changed, self,
				newmachine_onsectionchanged);
			psy_signal_connect(&section->signal_renamed, self,
				newmachine_onsectionrenamed);
			psy_ui_component_setalign(&section->component, psy_ui_ALIGN_TOP);
			if (p == psy_property_begin(
					self->workspace->pluginsections.sections)) {
				self->selectedsection = section;
				selidx = i;
				psy_ui_component_addstylestate(&section->component,
					psy_ui_STYLESTATE_SELECT);
			}
			psy_table_insert(&self->newmachinesections, i, section);
		}		
	}		
	newmachine_buildnavsections(self);
	psy_ui_tabbar_mark(&self->navsections, selidx);
	newmachine_alignsections(self);
}

void newmachine_onlanguagechanged(NewMachine* self)
{	
	psy_ui_Tab* first;

	first = psy_ui_tabbar_tab(&self->navsections, 0);
	if (first) {		
		psy_ui_tab_settext(first, psy_ui_translate("newmachine.favorites"));
	}
}

void newmachine_buildnavsections(NewMachine* self)
{
	psy_List* p;
	uintptr_t selidx;

	selidx = psy_ui_tabbar_selected(&self->navsections);
	psy_ui_tabbar_clear(&self->navsections);
	psy_ui_tabbar_preventtranslation(&self->navsections);
	self->selectedsection = NULL;	
	p = psy_property_begin(self->workspace->pluginsections.sections);
	for (; p != 0; p = p->next) {
		psy_Property* section;		

		section = (psy_Property*)psy_list_entry(p);
		if (strcmp(psy_property_key(section), "favorites") == 0) {
			psy_ui_tabbar_append(&self->navsections,
				psy_ui_translate("newmachine.favorites"));
		} else {
			psy_ui_tabbar_append(&self->navsections,
				psy_property_at_str(section, "name",
					psy_property_key(section)));
		}
	}
	psy_ui_tabbar_mark(&self->navsections, selidx);
}

void newmachine_ontabbarchanged(NewMachine* self, psy_ui_TabBar* sender, uintptr_t index)
{
	NewMachineSection* section;

	section = psy_table_at(&self->newmachinesections, index);
	if (section) {
		psy_ui_RealRectangle position;

		position = psy_ui_component_position(&section->component);
		psy_ui_component_setscrolltop(&self->sections, 
			psy_ui_value_make_px(position.top));
		self->selectedsection = section;
		newmachinesection_mark(section);
	}	
}

void newmachine_onaddtosection(NewMachine* self, psy_ui_Component* sender)
{
	if (newmachine_checkplugin(self)) {		
		psy_audio_MachineInfo macinfo;

		machineinfo_init(&macinfo);
		newmachine_selectedmachineinfo(self, &macinfo);
		psy_audio_pluginsections_add(&self->workspace->pluginsections,
			psy_property_key(self->selectedsection->section), &macinfo);
		self->selectedplugin = NULL;
		newmachinesection_findplugins(self->selectedsection);		
		machineinfo_dispose(&macinfo);				
		newmachine_alignsections(self);
	}
}

void newmachine_onremovefromsection(NewMachine* self, psy_ui_Component* sender)
{
	if (newmachine_checkplugin(self)) {
		psy_audio_pluginsections_remove(&self->workspace->pluginsections,
			self->selectedsection->section, psy_property_key(self->selectedplugin));
		self->selectedplugin = NULL;
		newmachinesection_findplugins(self->selectedsection);		
		newmachine_alignsections(self);
	}
}

bool newmachine_checkplugin(NewMachine* self)
{	
	if (!newmachine_checksection(self)) {
		return FALSE;
	}
	if (!self->selectedplugin) {
		workspace_outputstatus(self->workspace,
			psy_ui_translate("newmachine.select-first-plugin"));
		return FALSE;		
	}
	return TRUE;
}

void newmachine_onremovesection(NewMachine* self, psy_ui_Component* sender)
{
	if (newmachine_checksection(self)) {		
		self->selectedplugin = NULL;
		if (strcmp(psy_property_key(self->selectedsection->section), "favorites") == 0) {
			newmachine_onclearsection(self, sender);
			return;
		}
		psy_audio_pluginsections_removesection(
			&self->workspace->pluginsections, self->selectedsection->section);
		newmachine_buildsections(self);		
	}
}

void newmachine_onclearsection(NewMachine* self, psy_ui_Component* sender)
{
	if (newmachine_checksection(self)) {
		self->selectedplugin = NULL;
		psy_audio_pluginsections_clearplugins(&self->workspace->pluginsections,
			newmachinesection_key(self->selectedsection));
		newmachine_buildsections(self);				
	}
}

bool newmachine_checksection(NewMachine* self)
{
	if (!self->selectedsection || !self->selectedsection->section) {
		workspace_outputstatus(self->workspace,
			psy_ui_translate("newmachine.select-first-section"));
		return FALSE;
	}
	return TRUE;
}

void newmachine_onfocus(NewMachine* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(&self->pluginsview.component);
}

void newmachine_onrescan(NewMachine* self, psy_ui_Component* sender)
{	
	if (!psy_audio_plugincatcher_scanning(&self->workspace->plugincatcher)) {
		psy_ui_label_settext(&self->scanview.scanfile, "");		
		psy_ui_component_starttimer(newmachine_base(self), 0, 50);
		psy_ui_notebook_select(&self->notebook, 1);
		workspace_scanplugins(self->workspace);
	}
}

void newmachine_onpluginscanprogress(NewMachine* self, Workspace* workspace,
	int progress)
{	
}

void  newmachine_ontimer(NewMachine* self, uintptr_t timerid)
{
	if (!psy_audio_plugincatcher_scanning(&self->workspace->plugincatcher)) {
		psy_ui_notebook_select(&self->notebook, 0);
		psy_ui_component_stoptimer(newmachine_base(self), 0);
	}
}

void newmachine_enableall(NewMachine* self)
{
	pluginsview_enableall(&self->pluginsview);	
}

void newmachine_enablegenerators(NewMachine* self)
{	
	pluginsview_enablegenerators(&self->pluginsview);
}

void newmachine_preventgenerators(NewMachine* self)
{
	pluginsview_preventgenerators(&self->pluginsview);
}

void newmachine_enableeffects(NewMachine* self)
{
	pluginsview_enableeffects(&self->pluginsview);
}

void newmachine_preventeffects(NewMachine* self)
{
	pluginsview_preventeffects(&self->pluginsview);
}

void newmachine_insertmode(NewMachine* self)
{
	if (workspace_song(self->workspace)) {
		uintptr_t index;

		index = psy_audio_machines_selected(
			&workspace_song(self->workspace)->machines);
		if (index != psy_INDEX_INVALID) {
			if (index < 0x40) {
				newmachine_enablegenerators(self);
				newmachine_preventeffects(self);
			} else {
				newmachine_enableeffects(self);
				newmachine_preventgenerators(self);
			}
		}
	}
	self->mode = NEWMACHINE_INSERT;	
}

void newmachine_appendmode(NewMachine* self)
{
	newmachine_enableall(self);
	self->mode = NEWMACHINE_APPEND;	
}

void newmachine_addeffectmode(NewMachine* self)
{	
	newmachine_preventgenerators(self);
	newmachine_enableeffects(self);
	self->mode = NEWMACHINE_ADDEFFECT;	
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
	pluginsview_filter(&self->pluginsview);	
	psy_ui_component_align(&self->client);	
}

void newmachine_onplugincategorychanged(NewMachine* self, NewMachineDetail* sender)
{
	if (self->selectedplugin && workspace_pluginlist(self->workspace)) {
		psy_Property* plugin;

		plugin = psy_property_at(workspace_pluginlist(self->workspace),
			psy_property_key(self->selectedplugin), PSY_PROPERTY_TYPE_SECTION);
		if (plugin) {
			psy_Property* category;

			category = psy_property_at(plugin, "category",
				PSY_PROPERTY_TYPE_NONE);
			if (category) {				
				psy_property_setitem_str(category,
					psy_ui_edit_text(&sender->categoryedit));
			}
			psy_audio_plugincatcher_save(workspace_plugincatcher(
				self->workspace));
			newmachinecategorybar_build(&self->categorybar);
			newmachine_updateplugins(self);			
			newmachinefilter_reset(&self->filter);
			psy_ui_component_align_full(&self->client);
			psy_ui_component_invalidate(&self->client);
		}
	}
}

void newmachine_ontoggleexpandsections(NewMachine* self, psy_ui_Button* sender)
{	
	psy_ui_component_togglevisibility(&self->all);	
}

void newmachine_ontoggleexpandall(NewMachine* self, psy_ui_Button* sender)
{
	psy_ui_Margin margin;

	if (psy_ui_component_visible(&self->sectiongroup)) {
		margin = psy_ui_margin_make_em(0.0, 0.0, 1.0, 0.0);		
	} else {
		margin = psy_ui_margin_make_em(0.5, 0.0, 1.0, 0.0);		
	}
	psy_ui_component_setmargin(&self->all, margin);
	psy_ui_component_togglevisibility(&self->sectiongroup);	
}

void newmachine_onsectionrenamed(NewMachine* self, NewMachineSection* sender)
{
	newmachine_buildnavsections(self);
	newmachine_alignsections(self);
}

void newmachine_onsectionselected(NewMachine* self, NewMachineSection* sender)
{
	self->selectedsection = sender;	
	psy_ui_tabbar_mark(&self->navsections,
		psy_ui_component_index(&sender->component));
}

void newmachine_onsectionchanged(NewMachine* self, NewMachineSection* sender)
{
	newmachine_alignsections(self);
}

void newmachine_alignsections(NewMachine* self)
{
	psy_ui_component_align_full(&self->sectiongroup);
	psy_ui_component_invalidate(&self->sectiongroup);
}

void newmachine_onscanfile(NewMachine* self, psy_audio_PluginCatcher* sender,
	const char* path, int type)
{
	psy_ui_label_settext(&self->scanview.scanfile, path);	
}

void newmachine_onscantaskstart(NewMachine* self, psy_audio_PluginCatcher* sender,
	psy_audio_PluginScanTask* task)
{
	pluginscanview_selecttask(&self->scanview, task);
}

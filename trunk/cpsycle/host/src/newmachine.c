/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "newmachine.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

/* NewMachineSearchBar */

/* implementation */
static void newmachinesearchbar_on_search_field_change(
	NewMachineSearchBar*, SearchField* sender);

void newmachinesearchbar_init(NewMachineSearchBar* self,
	psy_ui_Component* parent, PluginFilter* filter)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component,
		STYLE_NEWMACHINE_SEARCHBAR);	
	self->filter = filter;	
	searchfield_init(&self->search, &self->component);
	psy_ui_component_set_align(searchfield_base(&self->search),
		psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->search.signal_changed, self,
		newmachinesearchbar_on_search_field_change);
	searchfield_set_default_text(&self->search, "newmachine.search-plugin");
}

void newmachinesearchbar_setfilter(NewMachineSearchBar* self,
	PluginFilter* filter)
{
	self->filter = filter;	
}

void newmachinesearchbar_on_search_field_change(NewMachineSearchBar* self,
	SearchField* sender)
{
	if (self->filter) {
		pluginfilter_set_search_text(self->filter, searchfield_text(sender));		
	}
}

/* NewMachineRescanBar */

/* prototypes */
static void newmachinerescanbar_onselectdirectories(NewMachineRescanBar*,
	psy_ui_Component* sender);
	
/* implementation */
void newmachinerescanbar_init(NewMachineRescanBar* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component,
		STYLE_NEWMACHINE_RESCANBAR);	
	psy_ui_component_set_default_align(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;	
	psy_ui_button_init_text(&self->rescan, &self->component,
		"newmachine.rescan");	
	psy_ui_label_init_text(&self->desc, &self->component, "newmachine.in");	
	psy_ui_button_init_text(&self->selectdirectories, &self->component,
		"newmachine.plugin-directories");	
	psy_ui_button_load_resource(&self->selectdirectories,
		IDB_SETTINGS_DARK, IDB_SETTINGS_DARK, psy_ui_colour_white());		
	psy_signal_connect(&self->selectdirectories.signal_clicked, self,
		newmachinerescanbar_onselectdirectories);
	psy_ui_button_init_text(&self->cancel, &self->component, "Cancel");		
	psy_ui_component_set_align(psy_ui_button_base(&self->cancel),
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_preferred_width(psy_ui_button_base(
		&self->cancel), psy_ui_value_make_ew(15.0));
	psy_ui_button_init_text(&self->add, &self->component, "OK");
	psy_ui_component_set_preferred_width(psy_ui_button_base(&self->add),
		psy_ui_value_make_ew(15.0));		
	psy_ui_component_set_align(psy_ui_button_base(&self->add),
		psy_ui_ALIGN_RIGHT);
}

void newmachinerescanbar_onselectdirectories(NewMachineRescanBar* self,
	psy_ui_Component* sender)
{
	workspace_select_view(self->workspace, viewindex_make_all(
		VIEW_ID_SETTINGSVIEW, 4, 0, psy_INDEX_INVALID));
}

/* NewMachineSectionBar */

/* prototypes */
static void newmachinefiltergroup_on_destroyed(NewMachineFilterGroup*);

/* implementation */
void newmachinesectionbar_init(NewMachineSectionBar* self,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component,
		STYLE_NEWMACHINE_SECTIONBAR);
	psy_ui_component_set_default_align(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_button_init_text(&self->removesection, &self->component,
		"newmachine.remove");
	psy_ui_button_init_text(&self->clearsection, &self->component,
		"newmachine.clear");	
	psy_ui_button_init_text(&self->addtosection, &self->component,
		"newmachine.copy");
	psy_ui_button_init_text(&self->createsection, &self->component,
		"newmachine.mksection");
	psy_ui_button_init_text(&self->removefromsection, &self->component,
		"newmachine.delete");
	psy_ui_button_init_text(&self->removepane, &self->component, "-");
	psy_ui_component_set_align(&self->removepane.component, psy_ui_ALIGN_RIGHT);
	psy_ui_button_init_text(&self->addpane, &self->component, "+");
	psy_ui_component_set_align(&self->addpane.component, psy_ui_ALIGN_RIGHT);
}

/* NewMachineFilterGroup */

/* prototypes */
static void newmachinefiltergroup_on_button(NewMachineFilterGroup*,
	psy_ui_Button* sender);
static void newmachinefiltergroup_on_destroyed(NewMachineFilterGroup*);
static void newmachinefiltergroup_build(NewMachineFilterGroup*);
static void newmachinefiltergroup_select(NewMachineFilterGroup*,
	uintptr_t id, uintptr_t state);
static void newmachinefiltergroup_update(NewMachineFilterGroup*);
	
/* vtable */
static psy_ui_ComponentVtable newmachinefiltergroup_vtable;
static bool newmachinefiltergroup_vtable_initialized = FALSE;

static void newmachinefiltergroup_vtable_init(NewMachineFilterGroup*
	self)
{
	if (!newmachinefiltergroup_vtable_initialized) {
		newmachinefiltergroup_vtable = *(self->component.vtable);		
		newmachinefiltergroup_vtable.on_destroyed =
			(psy_ui_fp_component)
			newmachinefiltergroup_on_destroyed;
		newmachinefiltergroup_vtable_initialized = TRUE;
	}
	self->component.vtable = &newmachinefiltergroup_vtable;
}
	
/* implementation */
void newmachinefiltergroup_init(NewMachineFilterGroup* self,
	psy_ui_Component* parent, PluginFilter* filter,
	PluginFilterGroup* group)
{
	psy_ui_component_init(&self->component, parent, NULL);
	newmachinefiltergroup_vtable_init(self);	
	psy_signal_init(&self->signal_selected);
	psy_ui_component_set_style_type(&self->component,
		STYLE_NEWMACHINE_FILTERBAR);
	psy_ui_component_init_align(&self->header, &self->component, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_label_init(&self->desc, &self->header);
	psy_ui_component_set_style_type(&self->desc.component,
		STYLE_NEWMACHINE_FILTERBAR_LABEL);
	psy_ui_component_set_align(&self->desc.component, psy_ui_ALIGN_TOP);
	psy_ui_component_init_align(&self->types, &self->component, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(&self->types,
		psy_ui_margin_make_em(0.5, 0.0, 0.0, 0.0));		
	psy_ui_component_set_default_align(&self->types,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());
	newmachinefiltergroup_set_filter(self, filter, group);
}

void newmachinefiltergroup_on_destroyed(NewMachineFilterGroup* self)
{	
	psy_signal_dispose(&self->signal_selected);
}

void newmachinefiltergroup_set_filter(NewMachineFilterGroup* self,
	PluginFilter* filter, PluginFilterGroup* group)
{	
	self->filter = filter;
	self->filter_group = group;	
	newmachinefiltergroup_build(self);
}

void newmachinefiltergroup_build(NewMachineFilterGroup* self)
{
	psy_List* p;
	
	psy_ui_component_clear(&self->types);
	if (!self->filter_group) {
		return;
	}
	for (p = self->filter_group->items; p != NULL; p = p->next) {
		PluginFilterItem* curr;
		
		curr = (PluginFilterItem*)p->entry;
		newmachinefiltergroup_add(self, curr->name, curr->key, curr->active);		
	}
	psy_ui_label_set_text(&self->desc, self->filter_group->label);
}

void newmachinefiltergroup_add(NewMachineFilterGroup* self,
	const char* label, uintptr_t id, bool active)
{
	psy_ui_Button* button;
	
	assert(self);
	
	button = psy_ui_button_allocinit(&self->types);
	psy_ui_component_set_id(psy_ui_button_base(button), id);
	psy_ui_button_set_text(button, label);
	psy_ui_button_set_text_alignment(button, psy_ui_ALIGNMENT_LEFT);
	psy_ui_button_allowrightclick(button);
	if (active) {
		psy_ui_component_add_style_state(psy_ui_button_base(button), 
			psy_ui_STYLESTATE_SELECT);
	}
	psy_signal_connect(&button->signal_clicked, self,
		newmachinefiltergroup_on_button);
}

void newmachinefiltergroup_set(NewMachineFilterGroup* self,
	uintptr_t id, bool active)
{
	if (active) {
		newmachinefiltergroup_mark(self, id);
	} else {
		newmachinefiltergroup_unmark(self, id);
	}
}

void newmachinefiltergroup_mark(NewMachineFilterGroup* self,
	uintptr_t id)
{
	psy_ui_Component* component;
	
	component = psy_ui_component_by_id(&self->types, id, 0);
	if (component) {
		psy_ui_component_add_style_state(component, 
			psy_ui_STYLESTATE_SELECT);
	}
}

void newmachinefiltergroup_unmark(NewMachineFilterGroup* self,
	uintptr_t id)
{
	psy_ui_Component* component;
	
	component = psy_ui_component_by_id(&self->types, id, 0);
	if (component) {
		psy_ui_component_remove_style_state(component, 
			psy_ui_STYLESTATE_SELECT);
	}
}

void newmachinefiltergroup_on_button(NewMachineFilterGroup* self,
	psy_ui_Button* sender)
{
	newmachinefiltergroup_select(self, 
		psy_ui_component_id(psy_ui_button_base(sender)),
		psy_ui_button_clickstate(sender));
	psy_signal_emit(&self->signal_selected, self, 2,
		psy_ui_component_id(psy_ui_button_base(sender)),
		psy_ui_button_clickstate(sender));
}

void newmachinefiltergroup_select(NewMachineFilterGroup* self,
	uintptr_t id, uintptr_t state)
{
	if (self->filter_group) {		
		if (state == 1) {
			pluginfiltergroup_toggle(self->filter_group, id);
		} else {
			pluginfiltergroup_deselect_all(self->filter_group);
			pluginfiltergroup_select(self->filter_group, id);			
		}
		newmachinefiltergroup_update(self);		
	}
	if (self->filter) {			
		pluginfilter_notify(self->filter);
	}
}

void newmachinefiltergroup_update(NewMachineFilterGroup* self)
{
	if (self->filter_group) {		
		psy_List* p;
		
		for (p = self->filter_group->items; p != NULL; p = p->next) {
			PluginFilterItem* item;
			
			item = (PluginFilterItem*)p->entry;
			newmachinefiltergroup_set(self,	item->key, item->active);
		}
	}
}

/* NewMachineCategoryBar */

/* implementation */
void newmachinecategorybar_init(NewMachineCategoryBar* self,
	psy_ui_Component* parent, PluginFilter* filter,
	psy_audio_PluginCatcher* plugincatcher)
{	
	assert(self);
	assert(plugincatcher);

	newmachinefiltergroup_init(&self->group, parent,
		NULL, NULL);
	self->plugincatcher = plugincatcher;
	newmachinecategorybar_setfilter(self, filter);
}

void newmachinecategorybar_setfilter(NewMachineCategoryBar* self,
	PluginFilter* filter)
{
	self->filter = filter;		
	newmachinecategorybar_build(self);
}

void newmachinecategorybar_build(NewMachineCategoryBar* self)
{
	assert(self);
			
	pluginfiltergroup_clear(&self->filter->categories);
	if (self->filter) {		
		psy_audio_PluginCategories categories;
		psy_TableIterator it;
		
		psy_audio_plugincategories_init(&categories, 
			psy_audio_plugincatcher_plugins(self->plugincatcher));		
		for (it = psy_audio_plugincategories_begin(&categories);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			const char* category;

			category = (const char*)psy_tableiterator_value(&it);
			if (category) {
				pluginfiltergroup_add(&self->filter->categories,
					psy_strhash(category), category, TRUE);				
			}
		}
		psy_audio_plugincategories_dispose(&categories);				
	}
	newmachinefiltergroup_set_filter(&self->group, self->filter,
		self->filter ? &self->filter->categories : NULL);	
}

/* NewMachineFiltersBar */

/* prototypes */
static void newmachinefiltersbar_on_destroyed(NewMachineFiltersBar*);
static void newmachinefiltersbar_on_toggle(NewMachineFiltersBar*,
	psy_ui_Button* sender);

/* vtable */
static psy_ui_ComponentVtable newmachinefiltersbar_vtable;
static bool newmachinefiltersbar_vtable_initialized = FALSE;

static void newmachinefiltersbar_vtable_init(NewMachineFiltersBar* self)
{
	if (!newmachinefiltersbar_vtable_initialized) {
		newmachinefiltersbar_vtable = *(self->component.vtable);		
		newmachinefiltersbar_vtable.on_destroyed =
			(psy_ui_fp_component)
			newmachinefiltersbar_on_destroyed;
		newmachinefiltersbar_vtable_initialized = TRUE;
	}
	self->component.vtable = &newmachinefiltersbar_vtable;
}

/* implementation */
void newmachinefiltersbar_init(NewMachineFiltersBar* self,
	psy_ui_Component* parent, PluginFilter* filter,
	psy_audio_PluginCatcher* plugincatcher)
{	
	assert(self);
	assert(plugincatcher);
	
	psy_ui_component_init_align(&self->component, parent, NULL,
		psy_ui_ALIGN_TOP);
	newmachinefiltersbar_vtable_init(self);
	self->plugincatcher = plugincatcher;		
	psy_ui_component_set_margin(&self->component,
		psy_ui_margin_make_em(0.0, 0.0, 0.25, 0.0));
	psy_ui_component_init_align(&self->expandfiltersbar,
		&self->component, NULL, psy_ui_ALIGN_TOP);	
	psy_ui_button_init_connect(&self->togglefilters,
		&self->expandfiltersbar, self,
		newmachinefiltersbar_on_toggle);
	psy_ui_component_set_align(&self->togglefilters.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_button_prevent_translation(&self->togglefilters);	
	psy_ui_button_set_text(&self->togglefilters, "Filters");
	/* filters */
	psy_ui_component_init_align(&self->filters, &self->component, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_align_expand(&self->filters, psy_ui_HEXPAND);
	psy_ui_component_hide(&self->filters);
	/* mode bar */
	newmachinefiltergroup_init(&self->modebar, &self->filters,
		filter, filter ? &filter->mode : NULL);
	psy_ui_component_set_align(newmachinefiltergroup_base(&self->modebar),
		psy_ui_ALIGN_LEFT);	
	psy_ui_component_set_preferred_width(newmachinefiltergroup_base(
		&self->modebar), psy_ui_value_make_pw(0.25));	
	/* type bar */
	newmachinefiltergroup_init(&self->typebar, &self->filters,
		filter, filter ? &filter->mode : NULL);	
	psy_ui_component_set_align(newmachinefiltergroup_base(
		&self->typebar), psy_ui_ALIGN_LEFT);	
	psy_ui_component_set_preferred_width(newmachinefiltergroup_base(
		&self->typebar), psy_ui_value_make_pw(0.25));
	/* all categeory bar */
	newmachinecategorybar_init(&self->categorybar, &self->filters,
		filter, plugincatcher);
	psy_ui_component_set_align(newmachinecategorybar_base(
		&self->categorybar), psy_ui_ALIGN_LEFT);
	psy_ui_component_set_preferred_width(newmachinecategorybar_base(
		&self->categorybar), psy_ui_value_make_pw(0.25));
	/* sort bar */
	newmachinefiltergroup_init(&self->sortbar, &self->filters,
		filter, filter ? &filter->sort : NULL);	
	psy_ui_component_set_align(newmachinefiltergroup_base(&self->sortbar),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_set_preferred_width(newmachinefiltergroup_base(
		&self->sortbar), psy_ui_value_make_pw(25));
}

void newmachinefiltersbar_on_destroyed(NewMachineFiltersBar* self)
{	
}

void newmachinefiltersbar_setfilter(NewMachineFiltersBar* self,
	PluginFilter* filter)
{		
	newmachinefiltergroup_set_filter(&self->modebar, filter,
		&filter->mode);
	newmachinefiltergroup_set_filter(&self->typebar, filter,
		&filter->types);
	newmachinecategorybar_setfilter(&self->categorybar, filter);
}

void newmachinefiltersbar_on_toggle(NewMachineFiltersBar* self,
	psy_ui_Button* sender)
{
	psy_ui_Component* align_base;
	
	if (psy_ui_component_visible(&self->filters)) {
		psy_ui_component_hide(&self->filters);
	} else {
		psy_ui_component_show(&self->filters);
	}
	align_base = psy_ui_component_parent(
		psy_ui_component_parent(&self->component));
	psy_ui_component_align(align_base);
	psy_ui_component_invalidate(align_base);	
}

/* NewMachineSectionsHeader */
/* implementation */
void newmachinesectionsheader_init(NewMachineSectionsHeader* self,
	psy_ui_Component* parent, uintptr_t iconresourceid)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component,
		STYLE_NEWMACHINE_SECTIONS_HEADER);
	psy_ui_component_set_default_align(&self->component,
		psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_appdefaults()));
	if (iconresourceid != psy_INDEX_INVALID) {
		psy_ui_image_init_resource_transparency(&self->icon,
			&self->component, iconresourceid, psy_ui_colour_white());
	}
	psy_ui_button_init(&self->expand, &self->component);	
	psy_ui_button_load_resource(&self->expand, IDB_EXPAND_LIGHT,
		IDB_EXPAND_DARK, psy_ui_colour_white());		
	psy_ui_component_set_align(psy_ui_button_base(&self->expand),
		psy_ui_ALIGN_RIGHT);
}

/* NewMachineSectionsPane */

/* prototypes */
static void newmachinesectionspane_on_destroyed(NewMachineSectionsPane*);
static void newmachinesectionspane_on_tab_bar_changed(NewMachineSectionsPane*,
	psy_ui_TabBar* sender, uintptr_t index);
static void newmachinesectionspane_build_nav_sections(NewMachineSectionsPane*);
static void newmachinesectionspane_align_sections(NewMachineSectionsPane*);
static void newmachinesectionspane_on_section_renamed(NewMachineSectionsPane*,
	NewMachineSection* sender);
static void newmachinesectionspane_on_section_changed(NewMachineSectionsPane*,
	NewMachineSection* sender);
static void newmachinesectionspane_on_language_changed(NewMachineSectionsPane*);
static void newmachinesectionpane_on_mouse_down(NewMachineSectionsPane*,
	psy_ui_MouseEvent*);
static void newmachinesectionspane_onplugincachechanged(NewMachineSectionsPane*,
	PluginScanThread* sender);

/* vtable */
static psy_ui_ComponentVtable newmachinesectionspane_vtable;
static bool newmachinesectionspane_vtable_initialized = FALSE;

static void newmachinesectionspane_vtable_init(NewMachineSectionsPane* self)
{
	if (!newmachinesectionspane_vtable_initialized) {
		newmachinesectionspane_vtable = *(self->component.vtable);
		newmachinesectionspane_vtable.on_destroyed =
			(psy_ui_fp_component)
			newmachinesectionspane_on_destroyed;
		newmachinesectionspane_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			newmachinesectionpane_on_mouse_down;
		newmachinesectionspane_vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			newmachinesectionspane_on_language_changed;
		newmachinesectionspane_vtable_initialized = TRUE;
	}
	self->component.vtable = &newmachinesectionspane_vtable;
}

/* implementation */
void newmachinesectionspane_init(NewMachineSectionsPane* self,
	psy_ui_Component* parent, NewMachine* newmachine,
	PluginScanThread* plugin_scan_thread)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	newmachinesectionspane_vtable_init(self);
	self->plugin_scan_thread = plugin_scan_thread;	
	self->newmachine = newmachine;
	psy_table_init(&self->newmachinesections);
	/* filter */
	pluginfilter_init(&self->filter);
	/* sectionsheader */
	newmachinesectionsheader_init(&self->sectionsheader,
		&self->component, psy_INDEX_INVALID);
	psy_ui_component_set_align(&self->sectionsheader.component,
		psy_ui_ALIGN_TOP);
	/* navbar */
	psy_ui_tabbar_init(&self->navsections,
		&self->sectionsheader.component);
	psy_ui_component_set_align(psy_ui_tabbar_base(&self->navsections),
		psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&self->navsections.signal_change, self,
		newmachinesectionspane_on_tab_bar_changed);	
	/* pane */
	psy_ui_component_init(&self->pane, &self->component, NULL);	
	psy_ui_component_set_overflow(&self->pane, psy_ui_OVERFLOW_VSCROLL);
	psy_ui_component_set_scroll_step(&self->pane,
		psy_ui_size_make_em(0.0, 1.0));
	psy_ui_component_set_wheel_scroll(&self->pane, 4);
	/* filtersbar */
	newmachinefiltersbar_init(&self->filtersbar, &self->pane,
		&self->filter, plugin_scan_thread->plugincatcher);	
	/* sections */	
	psy_ui_component_init(&self->sections, &self->pane, NULL);
	psy_ui_component_set_align(&self->sections, psy_ui_ALIGN_TOP);
	/* section scroll */	
	psy_ui_scroller_init(&self->scroller_sections, &self->component,
		NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller_sections, &self->pane);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_HCLIENT);
	psy_ui_component_set_tab_index(&self->scroller_sections.component, 0);
	psy_ui_component_set_align(&self->scroller_sections.component,
		psy_ui_ALIGN_CLIENT);
	/* connect */
	psy_signal_connect(&plugin_scan_thread->signal_plugincachechanged,
		self, newmachinesectionspane_onplugincachechanged);
	newmachinesectionspane_buildsections(self);
}

void newmachinesectionspane_on_destroyed(NewMachineSectionsPane* self)
{
	psy_table_dispose(&self->newmachinesections);
	pluginfilter_dispose(&self->filter);
}

NewMachineSectionsPane* newmachinesectionspane_alloc(void)
{
		return (NewMachineSectionsPane*)malloc(sizeof(
			NewMachineSectionsPane));
}

NewMachineSectionsPane* newmachinesectionspane_allocinit(
	psy_ui_Component* parent, NewMachine* newmachine,
	PluginScanThread* plugin_scan_thread)
{
	NewMachineSectionsPane* rv;

	rv = newmachinesectionspane_alloc();
	if (rv) {
		newmachinesectionspane_init(rv, parent, newmachine, plugin_scan_thread);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void newmachinesectionspane_on_tab_bar_changed(NewMachineSectionsPane* self,
	psy_ui_TabBar* sender, uintptr_t index)
{
	NewMachineSection* section;

	section = (NewMachineSection*)psy_table_at(&self->newmachinesections,
		index);
	if (section) {
		psy_ui_RealRectangle sections_position;
		psy_ui_RealRectangle position;

		sections_position = psy_ui_component_position(&self->sections);
		position = psy_ui_component_position(&section->component);
		psy_ui_component_set_scroll_top(&self->pane, psy_ui_value_make_px(
			position.top + sections_position.top));
		self->newmachine->selectedsection = section;
		newmachinesection_mark(section);
		psy_ui_component_invalidate(&self->component);
	}
}

void newmachinesectionspane_checkselections(NewMachineSectionsPane* self,
	PluginsView* sender)
{
	psy_TableIterator it;

	assert(self);

	for (it = psy_table_begin(&self->newmachinesections);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		NewMachineSection* section;

		section = (NewMachineSection*)psy_tableiterator_value(&it);
		if (&section->pluginview != sender) {
			newmachinsection_clear_selection(section);
		}
	}
}

void newmachinesectionspane_buildsections(NewMachineSectionsPane* self)
{
	psy_List* p;
	uintptr_t i;
	uintptr_t selidx;

	self->newmachine->selectedsection = NULL;
	psy_table_clear(&self->newmachinesections);
	psy_ui_component_clear(&self->sections);
	p = psy_property_begin(
		self->plugin_scan_thread->plugincatcher->sections.sections);
	selidx = psy_INDEX_INVALID;
	for (i = 0; p != 0; p = p->next, ++i) {
		psy_Property* property;
		NewMachineSection* section;

		property = (psy_Property*)psy_list_entry(p);			
		section = newmachinesection_allocinit(&self->sections, property,
			&self->filter, self->plugin_scan_thread->plugincatcher);
		if (section) {
			psy_signal_connect(&section->pluginview.signal_selected,
				self->newmachine, newmachine_onpluginselected);
			psy_signal_connect(&section->pluginview.signal_changed,
				self->newmachine, newmachine_onpluginchanged);
			psy_signal_connect(&section->signal_selected, self->newmachine,
				newmachine_onsectionselected);
			psy_signal_connect(&section->signal_changed, self,
				newmachinesectionspane_on_section_changed);
			psy_signal_connect(&section->signal_renamed, self,
				newmachinesectionspane_on_section_renamed);
			psy_ui_component_set_align(&section->component, psy_ui_ALIGN_TOP);
			if (p == psy_property_begin(
					self->plugin_scan_thread->plugincatcher->sections.sections)) {
				self->newmachine->selectedsection = section;
				selidx = i;
				psy_ui_component_add_style_state(&section->component,
					psy_ui_STYLESTATE_SELECT);
			}
			psy_table_insert(&self->newmachinesections, i, section);
		}
	}
	newmachinesectionspane_build_nav_sections(self);
	psy_ui_tabbar_mark(&self->navsections, selidx);
	newmachinesectionspane_align_sections(self);
}

void newmachinesectionspane_build_nav_sections(NewMachineSectionsPane* self)
{
	psy_List* p;
	uintptr_t selidx;

	selidx = psy_ui_tabbar_selected(&self->navsections);
	psy_ui_tabbar_clear(&self->navsections);
	psy_ui_tabbar_prevent_translation(&self->navsections);
	self->newmachine->selectedsection = NULL;
	p = psy_property_begin(self->plugin_scan_thread->plugincatcher->sections.sections);
	for (; p != 0; p = p->next) {
		psy_Property* section;

		section = (psy_Property*)psy_list_entry(p);
		if (strcmp(psy_property_key(section), "all") == 0) {
			psy_ui_tabbar_append(&self->navsections,
				psy_ui_translate("newmachine.all"),
				psy_INDEX_INVALID,
				psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());
		} else if (strcmp(psy_property_key(section), "favorites") == 0) {
			psy_ui_tabbar_append(&self->navsections,				
				psy_ui_translate("newmachine.favorites"),
				psy_INDEX_INVALID,
				psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());
		} else {
			psy_ui_tabbar_append(&self->navsections,
				psy_property_at_str(section, "name",
					psy_property_key(section)),
				psy_INDEX_INVALID,
				psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());
		}
	}
	psy_ui_tabbar_mark(&self->navsections, selidx);
}

void newmachinesectionspane_align_sections(NewMachineSectionsPane* self)
{
	psy_ui_component_align_full(&self->component);
	psy_ui_component_invalidate(&self->component);
}


void newmachinesectionspane_on_section_renamed(NewMachineSectionsPane* self,
	NewMachineSection* sender)
{
	newmachinesectionspane_build_nav_sections(self);
	newmachinesectionspane_align_sections(self);
}

void newmachinesectionspane_on_section_changed(NewMachineSectionsPane* self,
	NewMachineSection* sender)
{
	newmachinesectionspane_align_sections(self);
}

void newmachinesectionspane_on_language_changed(NewMachineSectionsPane* self)
{
	psy_ui_Tab* first;

	first = psy_ui_tabbar_tab(&self->navsections, 0);
	if (first) {
		psy_ui_tab_set_text(first, psy_ui_translate("newmachine.favorites"));
	}
}

void newmachinesectionpane_on_mouse_down(NewMachineSectionsPane* self,
	psy_ui_MouseEvent* ev)
{	
	newmachine_select_pane(self->newmachine, self);
}

void newmachinesectionspane_onplugincachechanged(
	NewMachineSectionsPane* self, PluginScanThread* sender)
{
	newmachinesectionspane_buildsections(self);
	newmachinefiltersbar_setfilter(&self->filtersbar, &self->filter);	
}

/* NewMachine */

/* prototypes */
static void newmachine_on_destroyed(NewMachine*);
static NewMachineSectionsPane* newmachine_add_sections_pane(NewMachine*);
static void newmachine_onplugincachechanged(NewMachine*, PluginScanThread*);
static void newmachine_on_mouse_down(NewMachine*, psy_ui_MouseEvent*);
static void newmachine_on_focus(NewMachine*, psy_ui_Component* sender);
static void newmachine_onrescan(NewMachine*, psy_ui_Component* sender);
static void newmachine_onscanstart(NewMachine*, PluginScanThread*);
static void newmachine_onscanend(NewMachine*, PluginScanThread*);
static void newmachine_onpluginscanprogress(NewMachine*, PluginScanThread*,
	int progress);
static void newmachine_onplugincategorychanged(NewMachine*, NewMachineDetail*
	sender);
static void newmachine_checkselections(NewMachine*, PluginsView* sender);
static void newmachine_onscanfile(NewMachine*, psy_audio_PluginCatcher* sender,
	const char* path, int type);
static void newmachine_onscantaskstart(NewMachine*, psy_audio_PluginCatcher*
	sender, psy_audio_PluginScanTask*);
/* sectionbar */
static void newmachine_oncreatesection(NewMachine*,
	psy_ui_Component* sender);
static void newmachine_onremovesection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onclearsection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onaddtosection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onremovefromsection(NewMachine*,
	psy_ui_Component* sender);
static void newmachine_onaddpane(NewMachine*, psy_ui_Component* sender);
static void newmachine_onremovepane(NewMachine*, psy_ui_Component* sender);
static bool newmachine_checksection(NewMachine*);
static bool newmachine_checkplugin(NewMachine*);
static void newmachine_emit_selected_plugin(NewMachine*);
static void newmachine_on_add_selected_plugin(NewMachine*,
	psy_ui_Button* sender);
static void newmachine_on_cancel(NewMachine*,
	psy_ui_Button* sender);
static void newmachine_add_machine(NewMachine*, psy_Property* plugininfo);

/* vtable */
static psy_ui_ComponentVtable newmachine_vtable;
static bool newmachine_vtable_initialized = FALSE;

static void newmachine_vtable_init(NewMachine* self)
{
	if (!newmachine_vtable_initialized) {
		newmachine_vtable = *(self->component.vtable);
		newmachine_vtable.on_destroyed =
			(psy_ui_fp_component)
			newmachine_on_destroyed;
		newmachine_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			newmachine_on_mouse_down;				
		newmachine_vtable_initialized = TRUE;
	}
	self->component.vtable = &newmachine_vtable;
}

/* implementation */
void newmachine_init(NewMachine* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	newmachine_vtable_init(self);
	psy_ui_component_set_id(&self->component,
		SECTION_ID_MACHINEVIEW_NEWMACHINE);
	psy_ui_component_set_style_type(&self->component, STYLE_MV_NEWMACHINE);
	psy_ui_component_set_padding(&self->component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	self->workspace = workspace;	
	self->restoresection = SECTION_ID_MACHINEVIEW_WIRES;
	self->selectedplugin = NULL;
	self->selectedsection = NULL;
	self->newsectioncount = 0;	
	/* Notebook	(switches scanprogress/pluginselectview) */
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	/* client */
	psy_ui_component_init(&self->client, psy_ui_notebook_base(
		&self->notebook), NULL);
	psy_ui_component_set_align(&self->client, psy_ui_ALIGN_CLIENT);
	/* searchbar */
	newmachinesearchbar_init(&self->searchbar, &self->client, NULL);
	psy_ui_component_set_align(&self->searchbar.component,
		psy_ui_ALIGN_TOP);
	/* scanview */
	pluginscanview_init(&self->scanview, psy_ui_notebook_base(
		&self->notebook), self->workspace);	
	/* Rescanbar */
	newmachinerescanbar_init(&self->rescanbar, &self->component,
		self->workspace);
	psy_ui_component_set_align(&self->rescanbar.component,
		psy_ui_ALIGN_BOTTOM);
	/* Details */
	newmachinedetail_init(&self->detail, &self->component, self->workspace);
	psy_ui_component_set_align(&self->detail.component,
		psy_ui_ALIGN_RIGHT);	
	psy_signal_connect(&self->detail.signal_categorychanged, self,
		newmachine_onplugincategorychanged);	
	/* sectionbar */
	newmachinesectionbar_init(&self->sectionbar, &self->component);
	psy_ui_component_set_align(&self->sectionbar.component,
		psy_ui_ALIGN_BOTTOM);
	/* sections */
	psy_ui_component_init(&self->sections, &self->client, NULL);
	psy_ui_component_set_align(&self->sections, psy_ui_ALIGN_CLIENT);	
	/* connect signals */
	psy_signal_connect(&self->sectionbar.createsection.signal_clicked,
		self, newmachine_oncreatesection);
	psy_signal_connect(&self->sectionbar.addtosection.signal_clicked,
		self, newmachine_onaddtosection);
	psy_signal_connect(&self->sectionbar.removefromsection.signal_clicked,
		self, newmachine_onremovefromsection);
	psy_signal_connect(&self->sectionbar.removesection.signal_clicked,
		self, newmachine_onremovesection);
	psy_signal_connect(&self->sectionbar.clearsection.signal_clicked,
		self, newmachine_onclearsection);	
	psy_signal_connect(&self->sectionbar.addpane.signal_clicked,
		self, newmachine_onaddpane);
	psy_signal_connect(&self->sectionbar.removepane.signal_clicked,
		self, newmachine_onremovepane);	
	/* connect to signals */
	psy_signal_connect(&workspace->pluginscanthread.signal_plugincachechanged,
		self, newmachine_onplugincachechanged);
	psy_signal_connect(&self->component.signal_focus, self,
		newmachine_on_focus);	
	psy_signal_connect(&self->rescanbar.rescan.signal_clicked, self,
		newmachine_onrescan);
	psy_signal_connect(&workspace->pluginscanthread.signal_scanstart, self,
		newmachine_onscanstart);
	psy_signal_connect(&workspace->pluginscanthread.signal_scanend, self,
		newmachine_onscanend);
	psy_signal_connect(&workspace->pluginscanthread.signal_scanprogress, self,
		newmachine_onpluginscanprogress);
	psy_signal_connect(&workspace->pluginscanthread.signal_scantaskstart, self,
		newmachine_onscantaskstart);
	psy_signal_connect(&workspace->pluginscanthread.signal_scanfile, self,
		newmachine_onscanfile);
	psy_signal_connect(&self->rescanbar.add.signal_clicked,
		self, newmachine_on_add_selected_plugin);
	psy_signal_connect(&self->rescanbar.cancel.signal_clicked,
		self, newmachine_on_cancel);	
	newmachine_select_pane(self, newmachine_add_sections_pane(self));	
	psy_ui_notebook_select(&self->notebook, 0);	
}

void newmachine_on_destroyed(NewMachine* self)
{			
}

NewMachineSectionsPane* newmachine_add_sections_pane(NewMachine* self)
{
	NewMachineSectionsPane* rv;
	
	rv = newmachinesectionspane_allocinit(&self->sections, self,
		&self->workspace->pluginscanthread);
	if (rv) {
		psy_ui_component_set_align(&rv->component, psy_ui_ALIGN_CLIENT);
	}	
	return rv;
}

void newmachine_setfilter(NewMachine* self, PluginFilter* filter)
{		
	newmachinesearchbar_setfilter(&self->searchbar, filter);
	if (self->curr_sections_pane) {
		newmachinefiltersbar_setfilter(
			&self->curr_sections_pane->filtersbar,
			&self->curr_sections_pane->filter);
	}
}

void newmachine_onpluginselected(NewMachine* self, PluginsView* sender)
{		
	psy_Property* selected;

	selected = pluginsview_selectedplugin(sender);
	self->selectedplugin = selected;
	newmachinedetail_update(&self->detail, self->selectedplugin);
	newmachine_checkselections(self, sender);
	newmachine_emit_selected_plugin(self);	
}

void newmachine_emit_selected_plugin(NewMachine* self)
{
	if (self->selectedplugin) {
		newmachine_add_machine(self, self->selectedplugin);
		plugincatcher_incfavorite(
			workspace_plugincatcher(self->workspace),
			psy_property_key(self->selectedplugin));		
	}
}

void newmachine_add_machine(NewMachine* self, psy_Property* plugininfo)
{
	psy_audio_Machine* machine;
	psy_audio_MachineInfo machineinfo;	
	
	psy_audio_machineinfo_init_property(&machineinfo, plugininfo);	
	/*if (self->newmachine.restoresection == SECTION_ID_MACHINEVIEW_STACK &&
			self->stackview.state.insertmachinemode ==
			NEWMACHINE_ADDEFFECTSTACK) {
		self->newmachine.restoresection = psy_INDEX_INVALID;
		machinestackview_addeffect(&self->stackview, &machineinfo);
		machineinfo_dispose(&machineinfo);
		psy_ui_tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_STACK);
		return;
	}*/	
	machine = psy_audio_machinefactory_make_info(
		&self->workspace->player.machinefactory, &machineinfo);
	machineinfo_dispose(&machineinfo);
	if (machine) {
		psy_audio_Machines* machines;
		
		machines = &workspace_song(self->workspace)->machines;
		if (self->workspace->insert.replace_mac != psy_INDEX_INVALID &&
			psy_audio_machine_mode(machine) == psy_audio_MACHMODE_FX) {
			psy_audio_machine_setbus(machine);
		}
		if (self->workspace->insert.replace_mac != psy_INDEX_INVALID) {
			psy_audio_machines_insert(machines,
				self->workspace->insert.replace_mac, machine);				
		} else {
			psy_audio_machines_select(machines, psy_audio_machines_append(
				machines, machine));
		}
	} else {
		workspace_output_error(self->workspace,
			self->workspace->player.machinefactory.errstr);
	}
	workspace_select_view(self->workspace,  viewindex_make_all(VIEW_ID_MACHINEVIEW,
		self->restoresection, psy_INDEX_INVALID, psy_INDEX_INVALID));	
}			

void newmachine_on_add_selected_plugin(NewMachine* self,
	psy_ui_Button* sender)
{	
	if (self->selectedplugin) {
		newmachine_emit_selected_plugin(self);
	}
}

void newmachine_on_cancel(NewMachine* self,
	psy_ui_Button* sender)
{	
	workspace_select_view(self->workspace, viewindex_make_all(
		VIEW_ID_MACHINEVIEW, self->restoresection, 0,
		psy_INDEX_INVALID));	
}

void newmachine_onpluginchanged(NewMachine* self, PluginsView* sender)
{	
	self->selectedplugin = pluginsview_selectedplugin(sender);
	newmachine_checkselections(self, sender);
	newmachinedetail_update(&self->detail, self->selectedplugin);
}

void newmachine_checkselections(NewMachine* self, PluginsView* sender)
{
	if (self->curr_sections_pane) {
		newmachinesectionspane_checkselections(self->curr_sections_pane,
			sender);
	}	
}

void newmachine_onplugincachechanged(NewMachine* self, PluginScanThread* sender)
{	
	newmachine_updateplugins(self);
	newmachinedetail_reset(&self->detail);	
	psy_ui_component_align_full(&self->client);
	psy_ui_component_invalidate(&self->client);
}


void newmachine_updateplugins(NewMachine* self)
{		
	self->selectedplugin = NULL;
	self->selectedsection = NULL;	
}

void newmachine_on_focus(NewMachine* self, psy_ui_Component* sender)
{
	if (self->curr_sections_pane) {
		psy_ui_component_set_focus(&self->curr_sections_pane->component);
	}
}

void newmachine_onrescan(NewMachine* self, psy_ui_Component* sender)
{	
	if (!psy_audio_plugincatcher_scanning(&self->workspace->player.plugincatcher)) {
		psy_ui_label_set_text(&self->scanview.processview.scanfile, "");		
		workspace_scan_plugins(self->workspace);
	}
}

void newmachine_onpluginscanprogress(NewMachine* self, PluginScanThread* sender,
	int progress)
{	
	pluginscanstatusview_inc_plugin_count(
		&self->scanview.processview.statusview);	
}

void newmachine_onscanstart(NewMachine* self, PluginScanThread* sender)
{
	pluginscanview_reset(&self->scanview);
	psy_ui_notebook_select(&self->notebook, 1);	
}

void newmachine_onscanend(NewMachine* self, PluginScanThread* sender)
{	
	pluginscanview_scanstop(&self->scanview);
	psy_ui_notebook_select(&self->notebook, 0);	
}

void newmachine_on_mouse_down(NewMachine* self, psy_ui_MouseEvent* ev)
{
	psy_ui_mouseevent_stop_propagation(ev);
}

void newmachine_onplugincategorychanged(NewMachine* self,
	NewMachineDetail* sender)
{
	psy_Property* all;

	all = psy_audio_pluginsections_section_plugins(
		&self->workspace->player.plugincatcher.sections, "all");
	if (self->selectedplugin && all) {
		psy_Property* plugin;

		plugin = psy_property_at_section(all, psy_property_key(
			self->selectedplugin));
		if (plugin) {
			psy_Property* category;

			category = psy_property_at(plugin, "category",
				PSY_PROPERTY_TYPE_NONE);
			if (category) {				
				psy_property_set_item_str(category,
					psy_ui_textarea_text(&sender->categoryedit));
			}
			psy_audio_plugincatcher_save(workspace_plugincatcher(
				self->workspace));
			if (self->curr_sections_pane) {
				newmachine_setfilter(self,
					&self->curr_sections_pane->filter);
			}
			newmachine_updateplugins(self);
			if (self->curr_sections_pane) {
				pluginfilter_select_all(&self->curr_sections_pane->filter);
			}
			psy_ui_component_align_full(&self->client);
			psy_ui_component_invalidate(&self->client);
		}
	}
}

void newmachine_onsectionselected(NewMachine* self, NewMachineSection* sender)
{
	self->selectedsection = sender;	
	/* psy_ui_tabbar_mark(&self->sectionspane.navsections,
		psy_ui_component_index(&sender->component)); */
}

void newmachine_onscanfile(NewMachine* self, psy_audio_PluginCatcher* sender,
	const char* path, int type)
{
	psy_ui_label_set_text(&self->scanview.processview.scanfile, path);
	workspace_output_status(self->workspace, path);
	pluginscanstatusview_inc_file_count(&self->scanview.processview.statusview);
}

void newmachine_onscantaskstart(NewMachine* self,
	psy_audio_PluginCatcher* sender, psy_audio_PluginScanTask* task)
{
	pluginscanview_selecttask(&self->scanview, task);
}

void newmachine_oncreatesection(NewMachine* self, psy_ui_Component* sender)
{
	char sectionkey[64];

	psy_snprintf(sectionkey, 64, "section%d", (int)self->newsectioncount);
	while (psy_audio_pluginsections_section(
			&self->workspace->player.plugincatcher.sections, sectionkey)) {
		++self->newsectioncount;
		psy_snprintf(sectionkey, 64, "section%d", (int)self->newsectioncount);
	}
	psy_audio_pluginsections_add(&self->workspace->player.plugincatcher.sections,
		sectionkey, NULL);
	++self->newsectioncount;
	if (self->curr_sections_pane) {
		newmachinesectionspane_buildsections(self->curr_sections_pane);
	}	
}

void newmachine_onaddtosection(NewMachine* self, psy_ui_Component* sender)
{
	if (newmachine_checkplugin(self) && self->curr_sections_pane &&
			self->selectedplugin) {
		psy_audio_MachineInfo macinfo;		

		psy_audio_machineinfo_init_property(&macinfo, self->selectedplugin);		
		psy_audio_pluginsections_add(&self->workspace->player.plugincatcher.sections,
			psy_property_key(self->selectedsection->section), &macinfo);
		self->selectedplugin = NULL;
		newmachinesection_find_plugins(self->selectedsection);
		machineinfo_dispose(&macinfo);
		newmachinesectionspane_align_sections(self->curr_sections_pane);		
	}
}

void newmachine_onremovefromsection(NewMachine* self, psy_ui_Component* sender)
{
	if (newmachine_checkplugin(self) && self->curr_sections_pane) {
		psy_audio_pluginsections_remove(
			&self->workspace->player.plugincatcher.sections,
			self->selectedsection->section,
			psy_property_key(self->selectedplugin));
		self->selectedplugin = NULL;
		newmachinesection_find_plugins(self->selectedsection);
		newmachinesectionspane_align_sections(self->curr_sections_pane);		
	}
}

void newmachine_onremovesection(NewMachine* self, psy_ui_Component* sender)
{
	if (newmachine_checksection(self) && self->curr_sections_pane) {
		self->selectedplugin = NULL;
		if (strcmp(psy_property_key(self->selectedsection->section),
			"all") == 0) {
			return;
		}
		if (strcmp(psy_property_key(self->selectedsection->section),
			"favorites") == 0) {
			newmachine_onclearsection(self, sender);
			return;
		}
		psy_audio_pluginsections_removesection(
			&self->workspace->player.plugincatcher.sections,
			self->selectedsection->section);
		newmachinesectionspane_buildsections(self->curr_sections_pane);		
	}
}

void newmachine_onclearsection(NewMachine* self, psy_ui_Component* sender)
{
	if (newmachine_checksection(self) && self->curr_sections_pane) {
		self->selectedplugin = NULL;
		psy_audio_pluginsections_clear_plugins(
			&self->workspace->player.plugincatcher.sections,
			newmachinesection_key(self->selectedsection));
		newmachinesectionspane_buildsections(self->curr_sections_pane);		
	}
}

void newmachine_onaddpane(NewMachine* self, psy_ui_Component* sender)
{
	newmachine_select_pane(self, newmachine_add_sections_pane(self));
	psy_ui_component_align_full(&self->client);
	psy_ui_component_invalidate(&self->client);
}

void newmachine_select_pane(NewMachine* self, NewMachineSectionsPane* pane)
{	
	self->curr_sections_pane = pane;
	if (pane) {	
		newmachinesearchbar_setfilter(&self->searchbar, &pane->filter);
	} else {
		newmachinesearchbar_setfilter(&self->searchbar, NULL);
	}
}

void newmachine_onremovepane(NewMachine* self, psy_ui_Component* sender)
{
	if (self->curr_sections_pane) {
		psy_ui_Component* prev;
		uintptr_t index;
		
		index = psy_ui_component_index(
			&self->curr_sections_pane->component);
		if (index > 0) {
			prev = psy_ui_component_at(
				psy_ui_component_parent(&self->curr_sections_pane->component),
				index - 1);
			psy_ui_component_destroy(&self->curr_sections_pane->component);
			newmachine_select_pane(self, (NewMachineSectionsPane*)prev);			
			psy_ui_component_align_full(&self->client);
			psy_ui_component_invalidate(&self->client);
		}				
	}	
}

bool newmachine_checksection(NewMachine* self)
{
	if (!self->selectedsection || !self->selectedsection->section) {
		workspace_output_status(self->workspace, psy_ui_translate(
			"newmachine.select-first-section"));
		return FALSE;
	}
	return TRUE;
}

bool newmachine_checkplugin(NewMachine* self)
{
	if (!newmachine_checksection(self)) {
		return FALSE;
	}
	if (!self->selectedplugin) {
		workspace_output_status(self->workspace, psy_ui_translate(
			"newmachine.select-first-plugin"));
		return FALSE;
	}
	return TRUE;
}

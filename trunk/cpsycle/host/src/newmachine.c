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
static void newmachinesearchbar_onsearchfieldchange(NewMachineSearchBar*,
	SearchField* sender);

void newmachinesearchbar_init(NewMachineSearchBar* self,
	psy_ui_Component* parent, NewMachineFilter* filter)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_SEARCHBAR);	
	self->filter = filter;	
	searchfield_init(&self->search, &self->component);
	psy_ui_component_setalign(searchfield_base(&self->search),
		psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->search.signal_changed, self,
		newmachinesearchbar_onsearchfieldchange);
	searchfield_setdefaulttext(&self->search, "newmachine.search-plugin");
}

void newmachinesearchbar_setfilter(NewMachineSearchBar* self,
	NewMachineFilter* filter)
{
	self->filter = filter;	
}

void newmachinesearchbar_onsearchfieldchange(NewMachineSearchBar* self,
	SearchField* sender)
{
	if (self->filter) {
		newmachinefilter_settext(self->filter, searchfield_text(sender));
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
	psy_ui_Margin spacing;	

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_RESCANBAR);	
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;	
	psy_ui_button_init_text(&self->rescan, &self->component,
		"newmachine.rescan");
	psy_ui_margin_init_em(&spacing, 0.2, 0.2, 0.2, 0.2);
	psy_ui_component_setspacing(psy_ui_button_base(&self->rescan), spacing);
	psy_ui_label_init_text(&self->desc, &self->component, "newmachine.in");
	psy_ui_component_setspacing(psy_ui_label_base(&self->desc), spacing);
	psy_ui_button_init_text(&self->selectdirectories, &self->component,
		"newmachine.plugin-directories");
	psy_ui_component_setspacing(psy_ui_button_base(&self->selectdirectories),
		spacing);
	psy_ui_button_loadresource(&self->selectdirectories, IDB_SETTINGS_DARK,
		IDB_SETTINGS_DARK, psy_ui_colour_white());
	psy_signal_connect(&self->selectdirectories.signal_clicked, self,
		newmachinerescanbar_onselectdirectories);
}

void newmachinerescanbar_onselectdirectories(NewMachineRescanBar* self,
	psy_ui_Component* sender)
{
	workspace_selectview(self->workspace, VIEW_ID_SETTINGSVIEW, 4, 0);
}

/* NewMachineSectionBar */
/* implementation */
void newmachinesectionbar_init(NewMachineSectionBar* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin spacing;

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_SECTIONBAR);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;	
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
	psy_ui_margin_init_em(&spacing, 0.2, 0.2, 0.2, 0.2);
	psy_ui_component_setpadding_children(&self->component, spacing);	
}

/* NewMachineSortBar */
/* prototypes */
static void newmachinesortbar_onsortbyfavorite(NewMachineSortBar*,
	psy_ui_Component* sender);
static void newmachinesortbar_onsortbyname(NewMachineSortBar*,
	psy_ui_Component* sender);
static void newmachinesortbar_onsortbytype(NewMachineSortBar*,
	psy_ui_Component* sender);
static void newmachinesortbar_onsortbymode(NewMachineSortBar*,
	psy_ui_Component* sender);
/* implementation */
void newmachinesortbar_init(NewMachineSortBar* self, psy_ui_Component* parent,
	NewMachineSort* sort)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_SORTBAR);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HEXPAND);	
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	self->sort = sort;
	psy_ui_label_init_text(&self->desc, &self->component, "newmachine.sort");
	psy_ui_button_init_text_connect(&self->sortbyfavorite, &self->component,
		"newmachine.favorite", self, newmachinesortbar_onsortbyfavorite);	
	psy_ui_button_init_text_connect(&self->sortbyname, &self->component,
		"newmachine.name", self, newmachinesortbar_onsortbyname);
	psy_ui_button_init_text_connect(&self->sortbytype, &self->component,
		"newmachine.type", self, newmachinesortbar_onsortbytype);
	psy_ui_button_init_text_connect(&self->sortbymode, &self->component,
		"newmachine.mode", self, newmachinesortbar_onsortbymode);	
	psy_ui_component_setpadding_children(&self->component,
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

/* NewMachineFilterBar */
/* prototypes */
static void newmachinefilterbar_onclicked(NewMachineFilterBar* self,
	psy_ui_Button* sender);
/* implementation */
void newmachinefilterbar_init(NewMachineFilterBar* self,
	psy_ui_Component* parent, NewMachineFilter* filter)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_FILTERBAR);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_label_init_text(&self->desc, &self->component, "newmachine.filter");
	psy_ui_button_init_text_connect(&self->gen, &self->component,
		"Generators", self, newmachinefilterbar_onclicked);
	psy_ui_button_allowrightclick(&self->gen);
	psy_ui_button_init_text_connect(&self->effects, &self->component,
		"Effects", self, newmachinefilterbar_onclicked);
	psy_ui_button_allowrightclick(&self->effects);
	psy_ui_button_init_text_connect(&self->intern, &self->component,
		"Intern", self, newmachinefilterbar_onclicked);	
	psy_ui_button_allowrightclick(&self->intern);
	psy_ui_button_init_text_connect(&self->native, &self->component,
		"Native", self, newmachinefilterbar_onclicked);
	psy_ui_button_allowrightclick(&self->native);
	psy_ui_button_init_text_connect(&self->vst, &self->component,
		"VST", self, newmachinefilterbar_onclicked);
	psy_ui_button_allowrightclick(&self->vst);
	psy_ui_button_init_text_connect(&self->lua, &self->component,
		"LUA", self, newmachinefilterbar_onclicked);
	psy_ui_button_allowrightclick(&self->lua);
	psy_ui_button_init_text_connect(&self->ladspa, &self->component,
		"LADSPA", self, newmachinefilterbar_onclicked);	
	psy_ui_button_allowrightclick(&self->ladspa);
	self->filter = filter;
	newmachinefilterbar_update(self);
}

void newmachinefilterbar_setfilter(NewMachineFilterBar* self,
	NewMachineFilter* filter)
{
	self->filter = filter;
	newmachinefilterbar_update(self);
}

void newmachinefilterbar_onclicked(NewMachineFilterBar* self, psy_ui_Button* sender)
{		
	if (self->filter) {
		if (sender->buttonstate == 1) {
			if (sender == &self->effects) {
				self->filter->effect = !self->filter->effect;
			} else if (sender == &self->gen) {
				self->filter->gen = !self->filter->gen;
			} else if (sender == &self->intern) {
				self->filter->intern = !self->filter->intern;
			} else if (sender == &self->native) {
				self->filter->native = !self->filter->native;
			} else if (sender == &self->vst) {
				self->filter->vst = !self->filter->vst;
			} else if (sender == &self->lua) {
				self->filter->lua = !self->filter->lua;
			} else if (sender == &self->ladspa) {
				self->filter->ladspa = !self->filter->ladspa;
			}
		} else {			
			if (sender == &self->effects) {				
				self->filter->effect = TRUE;
				self->filter->gen = FALSE;				
			} else if (sender == &self->gen) {								
				self->filter->effect = FALSE;
				self->filter->gen = TRUE;				
			} else if (sender == &self->intern) {				
				newmachinefilter_cleartypes(self->filter);
				self->filter->intern = TRUE;				
			} else if (sender == &self->native) {				
				newmachinefilter_cleartypes(self->filter);
				self->filter->native = TRUE;				
			} else if (sender == &self->vst) {				
				newmachinefilter_cleartypes(self->filter);
				self->filter->vst = TRUE;				
			} else if (sender == &self->lua) {				
				newmachinefilter_cleartypes(self->filter);
				self->filter->lua = TRUE;				
			} else if (sender == &self->ladspa) {				
				newmachinefilter_cleartypes(self->filter);
				self->filter->ladspa = TRUE;				
			}			
		}	
		newmachinefilterbar_update(self);
		newmachinefilter_notify(self->filter);
	}	
}

void newmachinefilterbar_update(NewMachineFilterBar* self)
{
	if (self->filter) {
		if (self->filter->effect) {
			psy_ui_button_highlight(&self->effects);
		} else {
			psy_ui_button_disablehighlight(&self->effects);
		}
		if (self->filter->gen) {
			psy_ui_button_highlight(&self->gen);
		} else {
			psy_ui_button_disablehighlight(&self->gen);
		}
		if (self->filter->intern) {
			psy_ui_button_highlight(&self->intern);
		} else {
			psy_ui_button_disablehighlight(&self->intern);
		}
		if (self->filter->native) {
			psy_ui_button_highlight(&self->native);
		} else {
			psy_ui_button_disablehighlight(&self->native);
		}
		if (self->filter->vst) {
			psy_ui_button_highlight(&self->vst);
		} else {
			psy_ui_button_disablehighlight(&self->vst);
		}
		if (self->filter->lua) {
			psy_ui_button_highlight(&self->lua);
		} else {
			psy_ui_button_disablehighlight(&self->lua);
		}
		if (self->filter->ladspa) {
			psy_ui_button_highlight(&self->ladspa);
		} else {
			psy_ui_button_disablehighlight(&self->ladspa);
		}
	}
}

/* NewMachineCategoryBar */
/* prototypes */
static void newmachinecategorybar_onclicked(NewMachineCategoryBar*,
	psy_ui_Button* sender);
static void newmachinecategorybar_ondragover(NewMachineCategoryBar*, psy_ui_DragEvent*);
static void newmachinecategorybar_ondrop(NewMachineCategoryBar*, psy_ui_DragEvent*);
/* vtable */
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
/* implementation */
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
	psy_ui_component_init_align(&self->client, &self->component, NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setdefaultalign(&self->client, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->plugincatcher = plugincatcher;
	self->filter = filter;
}

void newmachinecategorybar_setfilter(NewMachineCategoryBar* self, NewMachineFilter* filter)
{
	self->filter = filter;
}

void newmachinecategorybar_build(NewMachineCategoryBar* self)
{
	psy_TableIterator it;
	psy_ui_Button* button;

	assert(self);

	psy_ui_component_clear(&self->client);	
	button = psy_ui_button_allocinit(&self->client);	
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

				button = psy_ui_button_allocinit(&self->client);
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
			psy_ui_dragevent_prevent_default(ev);			
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

/* NewMachineSectionsHeader */
/* implementation */
void newmachinesectionsheader_init(NewMachineSectionsHeader* self,
	psy_ui_Component* parent, uintptr_t iconresourceid)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletype(&self->component,
		STYLE_NEWMACHINE_SECTIONS_HEADER);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_appdefaults()));
	if (iconresourceid != psy_INDEX_INVALID) {
		psy_ui_image_init_resource_transparency(&self->icon,
			&self->component, iconresourceid, psy_ui_colour_white());
	}
	psy_ui_button_init(&self->expand, &self->component);	
	psy_ui_button_loadresource(&self->expand, IDB_EXPAND_LIGHT,
		IDB_EXPAND_DARK, psy_ui_colour_white());		
	psy_ui_component_setalign(psy_ui_button_base(&self->expand),
		psy_ui_ALIGN_RIGHT);
}

/* NewMachineSectionsPane */
/* prototypes */
static void newmachinesectionspane_ondestroy(NewMachineSectionsPane*);
static void newmachinesectionspane_ontabbarchanged(NewMachineSectionsPane*,
	psy_ui_TabBar* sender, uintptr_t index);
static void newmachinesectionspane_buildnavsections(NewMachineSectionsPane*);
static void newmachinesectionspane_alignsections(NewMachineSectionsPane*);
static void newmachinesectionspane_onsectionrenamed(NewMachineSectionsPane*, NewMachineSection* sender);
static void newmachinesectionspane_onsectionchanged(NewMachineSectionsPane*, NewMachineSection* sender);
static void newmachinesectionspane_onlanguagechanged(NewMachineSectionsPane*);
static void newmachinesectionpane_onmousedown(NewMachineSectionsPane*, psy_ui_MouseEvent*);
/* vtable */
static psy_ui_ComponentVtable newmachinesectionspane_vtable;
static bool newmachinesectionspane_vtable_initialized = FALSE;

static void newmachinesectionspane_vtable_init(NewMachineSectionsPane* self)
{
	if (!newmachinesectionspane_vtable_initialized) {
		newmachinesectionspane_vtable = *(self->component.vtable);
		newmachinesectionspane_vtable.ondestroy =
			(psy_ui_fp_component_event)
			newmachinesectionspane_ondestroy;
		newmachinesectionspane_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			newmachinesectionpane_onmousedown;
		newmachinesectionspane_vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			newmachinesectionspane_onlanguagechanged;
		newmachinesectionspane_vtable_initialized = TRUE;
	}
	self->component.vtable = &newmachinesectionspane_vtable;
}
/* implementation */
void newmachinesectionspane_init(NewMachineSectionsPane* self, psy_ui_Component* parent,
	NewMachine* newmachine, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	newmachinesectionspane_vtable_init(self);
	self->workspace = workspace;	
	self->newmachine = newmachine;
	psy_table_init(&self->newmachinesections);
	newmachinefilter_init(&self->filter);
	/* sectionsheader */
	newmachinesectionsheader_init(&self->sectionsheader, &self->component, psy_INDEX_INVALID);
	psy_ui_component_setalign(&self->sectionsheader.component, psy_ui_ALIGN_TOP);
	/* navbar */
	psy_ui_tabbar_init(&self->navsections, &self->sectionsheader.component);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->navsections), psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&self->navsections.signal_change, self,
		newmachinesectionspane_ontabbarchanged);	
	/* sections */
	psy_ui_component_init(&self->sections, &self->component, NULL);
	/* edit */
	psy_ui_textinput_init(&self->edit, &self->component);
	psy_ui_textinput_enableinputfield(&self->edit);
	psy_ui_component_hide(&self->edit.component);
	/* section scroll */
	psy_ui_component_setoverflow(&self->sections, psy_ui_OVERFLOW_VSCROLL);
	psy_ui_component_setscrollstep(&self->sections,
		psy_ui_size_make_em(0.0, 1.0));
	psy_ui_scroller_init(&self->scroller_sections, &self->sections,
		&self->component);
	psy_ui_component_setalign(&self->sections, psy_ui_ALIGN_HCLIENT);
	psy_ui_component_settabindex(&self->scroller_sections.component, 0);
	psy_ui_component_setalign(&self->scroller_sections.component,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setbackgroundmode(&self->scroller_sections.pane,
		psy_ui_SETBACKGROUND);	
}

void newmachinesectionspane_ondestroy(NewMachineSectionsPane* self)
{
	psy_table_dispose(&self->newmachinesections);
	newmachinefilter_dispose(&self->filter);
}

void newmachinesectionspane_ontabbarchanged(NewMachineSectionsPane* self, psy_ui_TabBar* sender,
	uintptr_t index)
{
	NewMachineSection* section;

	section = (NewMachineSection*)psy_table_at(&self->newmachinesections, index);
	if (section) {
		psy_ui_RealRectangle position;

		position = psy_ui_component_position(&section->component);
		psy_ui_component_setscrolltop(&self->sections,
			psy_ui_value_make_px(position.top));
		self->newmachine->selectedsection = section;
		newmachinesection_mark(section);
	}
}

void newmachinesectionspane_checkselections(NewMachineSectionsPane* self, PluginsView* sender)
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
}

void newmachinesectionspane_buildsections(NewMachineSectionsPane* self)
{
	psy_List* p;
	uintptr_t i;
	uintptr_t selidx;

	self->newmachine->selectedsection = NULL;
	psy_table_clear(&self->newmachinesections);
	psy_ui_component_clear(&self->sections);
	p = psy_property_begin(self->workspace->plugincatcher.sections.sections);
	selidx = psy_INDEX_INVALID;
	for (i = 0; p != 0; p = p->next, ++i) {
		psy_Property* property;
		NewMachineSection* section;

		property = (psy_Property*)psy_list_entry(p);			
		section = newmachinesection_allocinit(&self->sections, property,
			&self->edit, &self->filter, self->workspace);		
		if (section) {
			psy_signal_connect(&section->pluginview.signal_selected, self->newmachine,
				newmachine_onpluginselected);
			psy_signal_connect(&section->pluginview.signal_changed, self->newmachine,
				newmachine_onpluginchanged);
			psy_signal_connect(&section->signal_selected, self->newmachine,
				newmachine_onsectionselected);
			psy_signal_connect(&section->signal_changed, self,
				newmachinesectionspane_onsectionchanged);
			psy_signal_connect(&section->signal_renamed, self,
				newmachinesectionspane_onsectionrenamed);
			psy_ui_component_setalign(&section->component, psy_ui_ALIGN_TOP);
			if (p == psy_property_begin(self->workspace->plugincatcher.sections.sections)) {
				self->newmachine->selectedsection = section;
				selidx = i;
				psy_ui_component_addstylestate(&section->component,
					psy_ui_STYLESTATE_SELECT);
			}
			psy_table_insert(&self->newmachinesections, i, section);
		}
	}
	newmachinesectionspane_buildnavsections(self);
	psy_ui_tabbar_mark(&self->navsections, selidx);
	newmachinesectionspane_alignsections(self);
}

void newmachinesectionspane_buildnavsections(NewMachineSectionsPane* self)
{
	psy_List* p;
	uintptr_t selidx;

	selidx = psy_ui_tabbar_selected(&self->navsections);
	psy_ui_tabbar_clear(&self->navsections);
	psy_ui_tabbar_preventtranslation(&self->navsections);
	self->newmachine->selectedsection = NULL;
	p = psy_property_begin(self->workspace->plugincatcher.sections.sections);
	for (; p != 0; p = p->next) {
		psy_Property* section;

		section = (psy_Property*)psy_list_entry(p);
		if (strcmp(psy_property_key(section), "all") == 0) {
			psy_ui_tabbar_append(&self->navsections,
				psy_ui_translate("newmachine.all"),
				psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());
		} else if (strcmp(psy_property_key(section), "favorites") == 0) {
			psy_ui_tabbar_append(&self->navsections,
				psy_ui_translate("newmachine.favorites"),
				psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());
		} else {
			psy_ui_tabbar_append(&self->navsections,
				psy_property_at_str(section, "name",
					psy_property_key(section)),
				psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());
		}
	}
	psy_ui_tabbar_mark(&self->navsections, selidx);
}

void newmachinesectionspane_alignsections(NewMachineSectionsPane* self)
{
	psy_ui_component_align_full(&self->component);
	psy_ui_component_invalidate(&self->component);
}


void newmachinesectionspane_onsectionrenamed(NewMachineSectionsPane* self,
	NewMachineSection* sender)
{
	newmachinesectionspane_buildnavsections(self);
	newmachinesectionspane_alignsections(self);
}

void newmachinesectionspane_onsectionchanged(NewMachineSectionsPane* self,
	NewMachineSection* sender)
{
	newmachinesectionspane_alignsections(self);
}

void newmachinesectionspane_onlanguagechanged(NewMachineSectionsPane* self)
{
	psy_ui_Tab* first;

	first = psy_ui_tabbar_tab(&self->navsections, 0);
	if (first) {
		psy_ui_tab_settext(first, psy_ui_translate("newmachine.favorites"));
	}
}

void newmachinesectionpane_onmousedown(NewMachineSectionsPane* self,
	psy_ui_MouseEvent* ev)
{
	if (self->newmachine->currfilter != &self->filter) {
		newmachine_setfilter(self->newmachine, &self->filter);
	}
}

/* NewMachine */
/* prototypes */
static void newmachine_ondestroy(NewMachine*);
static void newmachine_onplugincachechanged(NewMachine*, Workspace*);
static void newmachine_onmousedown(NewMachine*, psy_ui_MouseEvent*);
static void newmachine_onfocus(NewMachine*, psy_ui_Component* sender);
static void newmachine_onrescan(NewMachine*, psy_ui_Component* sender);
static void newmachine_onscanstart(NewMachine*, Workspace*);
static void newmachine_onscanend(NewMachine*, Workspace*);
static void newmachine_onpluginscanprogress(NewMachine*, Workspace*,
	int progress);
static void newmachine_onplugincategorychanged(NewMachine*, NewMachineDetail* sender);
static void newmachine_ontoggleexpandpane0(NewMachine*, psy_ui_Button* sender);
static void newmachine_ontoggleexpandpane1(NewMachine*, psy_ui_Button* sender);
static void newmachine_checkselections(NewMachine*, PluginsView* sender);
static void newmachine_onscanfile(NewMachine*, psy_audio_PluginCatcher* sender, const char* path, int type);
static void newmachine_onscantaskstart(NewMachine*, psy_audio_PluginCatcher* sender,
	psy_audio_PluginScanTask*);
static void newmachine_onhorizontal(NewMachine*, psy_ui_Button* sender);
static void newmachine_onvertical(NewMachine*, psy_ui_Button* sender);
/* sectionbar */
static void newmachine_oncreatesection(NewMachine*,
	psy_ui_Component* sender);
static void newmachine_onremovesection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onclearsection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onaddtosection(NewMachine*, psy_ui_Component* sender);
static void newmachine_onremovefromsection(NewMachine*, psy_ui_Component* sender);
static bool newmachine_checksection(NewMachine*);
static bool newmachine_checkplugin(NewMachine*);
/* vtable */
static psy_ui_ComponentVtable newmachine_vtable;
static bool newmachine_vtable_initialized = FALSE;

static void newmachine_vtable_init(NewMachine* self)
{
	if (!newmachine_vtable_initialized) {
		newmachine_vtable = *(self->component.vtable);
		newmachine_vtable.ondestroy =
			(psy_ui_fp_component_event)
			newmachine_ondestroy;
		newmachine_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			newmachine_onmousedown;		
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
	self->workspace = workspace;	
	self->mode = NEWMACHINE_APPEND;
	self->appendstack = FALSE;
	self->restoresection = SECTION_ID_MACHINEVIEW_WIRES;
	self->selectedplugin = NULL;
	self->selectedsection = NULL;
	self->newsectioncount = 0;
	self->currfilter = NULL;
	newmachinesort_init(&self->sort);
	/* Notebook	(switches scanprogress/pluginselectview) */
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	/* client */
	psy_ui_component_init(&self->client, psy_ui_notebook_base(&self->notebook),
		NULL);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	/* searchbar */
	newmachinesearchbar_init(&self->searchbar, &self->client, NULL);
	psy_ui_component_setalign(&self->searchbar.component, psy_ui_ALIGN_TOP);
	/* scanview */
	pluginscanview_init(&self->scanview,
		psy_ui_notebook_base(&self->notebook), self->workspace);	
	/* header bars */
	psy_ui_component_init(&self->pluginsheaderbars, &self->searchbar.component, NULL);
	psy_ui_component_setalign(&self->pluginsheaderbars, psy_ui_ALIGN_CLIENT);
	/* filter bar */
	newmachinefilterbar_init(&self->filterbar, &self->pluginsheaderbars, NULL);
	psy_ui_component_setalign(&self->filterbar.component, psy_ui_ALIGN_LEFT);	
	/* sort bar */
	newmachinesortbar_init(&self->sortbar, &self->pluginsheaderbars, &self->sort);
	psy_ui_component_setalign(&self->sortbar.component, psy_ui_ALIGN_LEFT);
	/* all categeory bar */
	newmachinecategorybar_init(&self->categorybar, &self->client, NULL,
		workspace_plugincatcher(self->workspace));
	psy_ui_component_setalign(&self->categorybar.component, psy_ui_ALIGN_TOP);
	/* sectionpane0 */
	newmachinesectionspane_init(&self->sectionspane0, &self->client, self, workspace);
	psy_ui_component_setalign(&self->sectionspane0.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->sectionspane0.sectionsheader.expand.signal_clicked, self,
		newmachine_ontoggleexpandpane0);
	/* vspacer */
	psy_ui_component_init(&self->spacer, &self->client, NULL);
	psy_ui_component_setpreferredsize(&self->spacer,
		psy_ui_size_make(psy_ui_value_make_pw(0.01), psy_ui_value_zero()));
	psy_ui_component_hide(&self->spacer);
	psy_ui_component_setalign(&self->spacer, psy_ui_ALIGN_LEFT);
	/* sectionpane1 */
	newmachinesectionspane_init(&self->sectionspane1, &self->client, self, workspace);
	psy_ui_component_setalign(&self->sectionspane1.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->sectionspane1.sectionsheader.expand.signal_clicked, self,
		newmachine_ontoggleexpandpane1);		
	/* Details */
	newmachinedetail_init(&self->detail, &self->component, self->workspace);
	psy_ui_component_setalign(&self->detail.component, psy_ui_ALIGN_LEFT);	
	psy_signal_connect(&self->detail.signal_categorychanged, self,
		newmachine_onplugincategorychanged);
	/* Rescanbar */
	newmachinerescanbar_init(&self->rescanbar, &self->component, self->workspace);
	psy_ui_component_setalign(&self->rescanbar.component, psy_ui_ALIGN_BOTTOM);
	/* sectionbar */
	newmachinesectionbar_init(&self->sectionbar, &self->component,
		self->workspace);
	psy_ui_component_setalign(&self->sectionbar.component, psy_ui_ALIGN_BOTTOM);
	/* connect signals */
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
	/* layout */
	psy_ui_button_init_text_connect(&self->horizontal, &self->rescanbar.component, "Horizontal",
		self, newmachine_onhorizontal);
	psy_ui_component_setalign(&self->horizontal.component, psy_ui_ALIGN_RIGHT);
	psy_ui_button_init_text_connect(&self->vertical, &self->rescanbar.component, "Vertical",
		self, newmachine_onvertical);
	psy_ui_component_setalign(&self->vertical.component, psy_ui_ALIGN_RIGHT);
	/* filter */
	newmachine_setfilter(self, &self->sectionspane1.filter);	
	/* connect to signals */
	psy_signal_init(&self->signal_selected);	
	psy_signal_connect(&workspace->signal_plugincachechanged, self,
		newmachine_onplugincachechanged);
	psy_signal_connect(&self->component.signal_focus, self,
		newmachine_onfocus);	
	psy_signal_connect(&self->rescanbar.rescan.signal_clicked, self,
		newmachine_onrescan);
	psy_signal_connect(&workspace->signal_scanstart, self,
		newmachine_onscanstart);
	psy_signal_connect(&workspace->signal_scanend, self,
		newmachine_onscanend);
	psy_signal_connect(&workspace->signal_scanprogress, self,
		newmachine_onpluginscanprogress);
	psy_signal_connect(&workspace->signal_scantaskstart, self,
		newmachine_onscantaskstart);
	psy_signal_connect(&workspace->signal_scanfile, self,
		newmachine_onscanfile);	
	psy_ui_notebook_select(&self->notebook, 0);
	newmachinecategorybar_build(&self->categorybar);
	psy_ui_component_align(&self->categorybar.component);	
	/* show vertical */
	psy_ui_component_setalign(&self->sectionspane0.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setpreferredsize(&self->sectionspane0.component,
		psy_ui_size_make(psy_ui_value_make_pw(0.49), psy_ui_value_zero()));
	psy_ui_component_setalign(&self->sectionspane1.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setpreferredsize(&self->sectionspane1.component,
		psy_ui_size_make(psy_ui_value_make_pw(0.49), psy_ui_value_zero()));
	psy_ui_component_show(&self->spacer);
}

void newmachine_ondestroy(NewMachine* self)
{	
	newmachinesort_dispose(&self->sort);
	psy_signal_dispose(&self->signal_selected);	
}

void newmachine_setfilter(NewMachine* self, NewMachineFilter* filter)
{	
	self->currfilter = filter;
	newmachinesearchbar_setfilter(&self->searchbar, self->currfilter);
	newmachinefilterbar_setfilter(&self->filterbar, self->currfilter);
	newmachinecategorybar_setfilter(&self->categorybar, self->currfilter);
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
}

void newmachine_onpluginchanged(NewMachine* self, PluginsView* sender)
{	
	self->selectedplugin = pluginsview_selectedplugin(sender);
	newmachine_checkselections(self, sender);
	newmachinedetail_update(&self->detail, self->selectedplugin);
}

void newmachine_checkselections(NewMachine* self, PluginsView* sender)
{
	newmachinesectionspane_checkselections(&self->sectionspane0, sender);
	newmachinesectionspane_checkselections(&self->sectionspane1, sender);	
}

void newmachine_onplugincachechanged(NewMachine* self,
	Workspace* sender)
{	
	newmachine_updateplugins(self);
	newmachinefilter_reset(&self->sectionspane0.filter);	
	newmachinefilter_reset(&self->sectionspane1.filter);
	newmachinedetail_reset(&self->detail);		
	newmachinecategorybar_build(&self->categorybar);	
	psy_ui_component_align_full(&self->client);
	psy_ui_component_invalidate(&self->client);
}


void newmachine_updateplugins(NewMachine* self)
{		
	self->selectedplugin = NULL;
	self->selectedsection = NULL;	
	newmachinesectionspane_buildsections(&self->sectionspane0);	
	newmachinesectionspane_buildsections(&self->sectionspane1);	
	psy_ui_component_align(&self->client);
}

void newmachine_onfocus(NewMachine* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(&self->sectionspane0.component);
}

void newmachine_onrescan(NewMachine* self, psy_ui_Component* sender)
{	
	if (!psy_audio_plugincatcher_scanning(&self->workspace->plugincatcher)) {
		psy_ui_label_settext(&self->scanview.scanfile, "");		
		workspace_scanplugins(self->workspace);
	}
}

void newmachine_onpluginscanprogress(NewMachine* self, Workspace* workspace,
	int progress)
{	
}

void newmachine_onscanstart(NewMachine* self, Workspace* sender)
{
	pluginscanview_reset(&self->scanview);
	psy_ui_notebook_select(&self->notebook, 1);	
}

void newmachine_onscanend(NewMachine* self, Workspace* sender)
{
	psy_ui_notebook_select(&self->notebook, 0);
}

void newmachine_enableall(NewMachine* self)
{
/*	pluginsview_enableall(&self->pluginsview);	*/
}

void newmachine_enablegenerators(NewMachine* self)
{	
/*	pluginsview_enablegenerators(&self->pluginsview); */
}

void newmachine_preventgenerators(NewMachine* self)
{
/*	pluginsview_preventgenerators(&self->pluginsview); */
}

void newmachine_enableeffects(NewMachine* self)
{
/*	pluginsview_enableeffects(&self->pluginsview); */
}

void newmachine_preventeffects(NewMachine* self)
{
/*	pluginsview_preventeffects(&self->pluginsview); */
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
	psy_ui_mouseevent_stop_propagation(ev);
}

void newmachine_onplugincategorychanged(NewMachine* self, NewMachineDetail* sender)
{
	psy_Property* all;

	all = psy_audio_pluginsections_section_plugins(
		&self->workspace->plugincatcher.sections, "all");
	if (self->selectedplugin && all) {
		psy_Property* plugin;

		plugin = psy_property_at_section(all, psy_property_key(self->selectedplugin));
		if (plugin) {
			psy_Property* category;

			category = psy_property_at(plugin, "category",
				PSY_PROPERTY_TYPE_NONE);
			if (category) {				
				psy_property_setitem_str(category,
					psy_ui_textinput_text(&sender->categoryedit));
			}
			psy_audio_plugincatcher_save(workspace_plugincatcher(
				self->workspace));
			newmachinecategorybar_build(&self->categorybar);
			newmachine_updateplugins(self);			
			newmachinefilter_reset(&self->sectionspane1.filter);
			psy_ui_component_align_full(&self->client);
			psy_ui_component_invalidate(&self->client);
		}
	}
}

void newmachine_ontoggleexpandpane0(NewMachine* self, psy_ui_Button* sender)
{	
	psy_ui_component_togglevisibility(&self->sectionspane1.component);	
	if (self->sectionspane0.component.align == psy_ui_ALIGN_LEFT) {
		if (psy_ui_component_visible(&self->sectionspane1.component)) {
			psy_ui_component_setpreferredsize(&self->sectionspane0.component,
				psy_ui_size_make(psy_ui_value_make_pw(0.49), psy_ui_value_zero()));
				psy_ui_component_show(&self->spacer);

		} else {
			psy_ui_component_setpreferredsize(&self->sectionspane0.component,
				psy_ui_size_make(psy_ui_value_make_pw(1.0), psy_ui_value_zero()));
				psy_ui_component_hide(&self->spacer);
		}
		psy_ui_component_align(&self->client);
		psy_ui_component_invalidate(&self->client);
	}
}

void newmachine_ontoggleexpandpane1(NewMachine* self, psy_ui_Button* sender)
{
	psy_ui_component_togglevisibility(&self->sectionspane0.component);
	if (self->sectionspane1.component.align == psy_ui_ALIGN_LEFT) {
		if (psy_ui_component_visible(&self->sectionspane0.component)) {
			psy_ui_component_setpreferredsize(&self->sectionspane1.component,
				psy_ui_size_make(psy_ui_value_make_pw(0.49), psy_ui_value_zero()));
				psy_ui_component_show(&self->spacer);
			
		} else {
			psy_ui_component_setpreferredsize(&self->sectionspane1.component,
				psy_ui_size_make(psy_ui_value_make_pw(1.0), psy_ui_value_zero()));
			psy_ui_component_hide(&self->spacer);
		}
		psy_ui_component_align(&self->client);
		psy_ui_component_invalidate(&self->client);
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
	psy_ui_label_settext(&self->scanview.scanfile, path);
}

void newmachine_onscantaskstart(NewMachine* self,
	psy_audio_PluginCatcher* sender, psy_audio_PluginScanTask* task)
{
	pluginscanview_selecttask(&self->scanview, task);
}

void newmachine_onhorizontal(NewMachine* self, psy_ui_Button* sender)
{
	psy_ui_component_show(&self->sectionspane0.component);
	psy_ui_component_show(&self->sectionspane1.component);
	psy_ui_component_setalign(&self->sectionspane0.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setpreferredsize(&self->sectionspane0.component,
		psy_ui_size_make(psy_ui_value_make_pw(0.49), psy_ui_value_zero()));
	psy_ui_component_setalign(&self->sectionspane1.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setpreferredsize(&self->sectionspane1.component,
		psy_ui_size_make(psy_ui_value_make_pw(0.49), psy_ui_value_zero()));
	psy_ui_component_show(&self->spacer);
	psy_ui_component_align_full(&self->client);
	psy_ui_component_invalidate(&self->client);
}

void newmachine_onvertical(NewMachine* self, psy_ui_Button* sender)
{
	psy_ui_component_show(&self->sectionspane0.component);
	psy_ui_component_show(&self->sectionspane1.component);
	psy_ui_component_setalign(&self->sectionspane0.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->sectionspane1.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_hide(&self->spacer);
	psy_ui_component_align_full(&self->client);
	psy_ui_component_invalidate(&self->client);
}

void newmachine_oncreatesection(NewMachine* self, psy_ui_Component* sender)
{
	char sectionkey[64];

	psy_snprintf(sectionkey, 64, "section%d", (int)self->newsectioncount);
	while (psy_audio_pluginsections_section(&self->workspace->plugincatcher.sections,
		sectionkey)) {
		++self->newsectioncount;
		psy_snprintf(sectionkey, 64, "section%d", (int)self->newsectioncount);
	}
	psy_audio_pluginsections_add(&self->workspace->plugincatcher.sections,
		sectionkey, NULL);
	++self->newsectioncount;
	newmachinesectionspane_buildsections(&self->sectionspane0);
	newmachinesectionspane_buildsections(&self->sectionspane1);
}

void newmachine_onaddtosection(NewMachine* self, psy_ui_Component* sender)
{
	if (newmachine_checkplugin(self)) {
		psy_audio_MachineInfo macinfo;

		machineinfo_init(&macinfo);
		newmachine_selectedmachineinfo(self, &macinfo);
		psy_audio_pluginsections_add(&self->workspace->plugincatcher.sections,
			psy_property_key(self->selectedsection->section), &macinfo);
		self->selectedplugin = NULL;
		newmachinesection_findplugins(self->selectedsection);
		machineinfo_dispose(&macinfo);
		newmachinesectionspane_alignsections(&self->sectionspane0);
		newmachinesectionspane_alignsections(&self->sectionspane1);
	}
}

void newmachine_onremovefromsection(NewMachine* self, psy_ui_Component* sender)
{
	if (newmachine_checkplugin(self)) {
		psy_audio_pluginsections_remove(&self->workspace->plugincatcher.sections,
			self->selectedsection->section,
			psy_property_key(self->selectedplugin));
		self->selectedplugin = NULL;
		newmachinesection_findplugins(self->selectedsection);
		newmachinesectionspane_alignsections(&self->sectionspane0);
		newmachinesectionspane_alignsections(&self->sectionspane1);
	}
}

void newmachine_onremovesection(NewMachine* self, psy_ui_Component* sender)
{
	if (newmachine_checksection(self)) {
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
			&self->workspace->plugincatcher.sections,
			self->selectedsection->section);
		newmachinesectionspane_buildsections(&self->sectionspane0);
		newmachinesectionspane_buildsections(&self->sectionspane1);
	}
}

void newmachine_onclearsection(NewMachine* self, psy_ui_Component* sender)
{
	if (newmachine_checksection(self)) {
		self->selectedplugin = NULL;
		psy_audio_pluginsections_clearplugins(
			&self->workspace->plugincatcher.sections,
			newmachinesection_key(self->selectedsection));
		newmachinesectionspane_buildsections(&self->sectionspane0);
		newmachinesectionspane_buildsections(&self->sectionspane1);
	}
}

bool newmachine_checksection(NewMachine* self)
{
	if (!self->selectedsection || !self->selectedsection->section) {
		workspace_outputstatus(self->workspace, psy_ui_translate(
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
		workspace_outputstatus(self->workspace, psy_ui_translate(
			"newmachine.select-first-plugin"));
		return FALSE;
	}
	return TRUE;
}

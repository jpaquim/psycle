/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "fileview.h"
#include "styles.h"
#include "viewindex.h"
/* ui */
#include <uiapp.h>
/* file */
#include <dir.h>
/* container */
#include <qsort.h>
/* platform */
#include "../../detail/os.h"
#include "../../detail/portable.h"

/* FileViewFilter */

/* prototypes */
static void fileviewfilter_on_destroyed(FileViewFilter*);
static void fileviewfilter_build(FileViewFilter*, psy_Property* types);
static void fileviewfilter_on_item(FileViewFilter* self, psy_Property* sender);

/* vtable */
static psy_ui_ComponentVtable fileviewfilter_vtable;
static bool fileviewfilter_vtable_initialized = FALSE;

static void fileviewfilter_vtable_init(FileViewFilter* self)
{
	if (!fileviewfilter_vtable_initialized) {
		fileviewfilter_vtable = *(self->component.vtable);
		fileviewfilter_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			fileviewfilter_on_destroyed;
		fileviewfilter_vtable_initialized = TRUE;
	}
	self->component.vtable = &fileviewfilter_vtable;
}

/* implementation */
void fileviewfilter_init(FileViewFilter* self, psy_ui_Component* parent)
{	
	assert(self);
	
	psy_ui_component_init(fileviewfilter_base(self), parent, NULL);
	fileviewfilter_vtable_init(self);	
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_appdefaults()));
	psy_ui_label_init_text(&self->desc, fileviewfilter_base(self), "Item:");
	psy_ui_component_init(&self->items, fileviewfilter_base(self), NULL);
	psy_ui_component_set_align(&self->items, psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(&self->items, psy_ui_margin_make_em(
		0.0, 0.0, 0.0, 2.0));
	psy_ui_component_set_default_align(&self->items, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_appdefaults()));
	psy_property_init(&self->filter);
	self->types = psy_property_append_choice(&self->filter, "types", 0);
	psy_property_set_text(psy_property_set_id(
		psy_property_append_str(self->types, "psy", "*.psy"),
		FILEVIEWFILTER_PSY), "Psycle");
	psy_property_set_text(psy_property_set_id(
		psy_property_append_str(self->types, "mod", "*.mod"),
		FILEVIEWFILTER_MOD), "Module");	
	fileviewfilter_build(self, self->types);	
	self->showall = FALSE;	
	psy_signal_init(&self->signal_changed);
	psy_property_connect(self->types, self, fileviewfilter_on_item);
}

void fileviewfilter_on_destroyed(FileViewFilter* self)
{
	psy_property_dispose(&self->filter);
	psy_signal_dispose(&self->signal_changed);
}

void fileviewfilter_build(FileViewFilter* self, psy_Property* types)
{
	psy_List* p;			
	
	for (p = psy_property_begin(types); p  != NULL; p = p->next) {
		psy_Property* curr;
		psy_ui_CheckBox* check;
		
		curr = (psy_Property*)p->entry;	
		check = psy_ui_checkbox_allocinit(&self->items);
		psy_ui_checkbox_data_exchange(check, curr);
	}
}

const char* fileviewfilter_type(const FileViewFilter* self)
{
	static const char* all = "*.*";	
	psy_Property* item;
	
	if (self->showall) {
		return all;
	}
	item = psy_property_at_choice(self->types);
	if (item) {
		return psy_property_item_str(item);
	}	
	return all;
}

void fileviewfilter_on_item(FileViewFilter* self, psy_Property* sender)
{	
	psy_signal_emit(&self->signal_changed, self, 0);
}

/* FileViewSaveFilter */
void fileviewsavefilter_init(FileViewSaveFilter* self, psy_ui_Component* parent)
{
	psy_ui_component_init(fileviewsavefilter_base(self), parent, NULL);
	self->filter = FILEVIEWFILTER_PSY;
	psy_ui_component_set_default_align(fileviewsavefilter_base(self),
		psy_ui_ALIGN_TOP, psy_ui_defaults_vmargin(psy_ui_appdefaults()));
	psy_ui_label_init_text(&self->desc, fileviewsavefilter_base(self),
		"Save as:");	
	psy_ui_checkbox_init_text(&self->psy, fileviewsavefilter_base(self),
		"psy");	
	psy_ui_component_set_margin(&self->psy.component, psy_ui_margin_make_em(
		0.0, 0.0, 0.0, 2.0));	
}


/* FileViewFolderLinks */

/* prototypes */
static void fileviewlinks_on_destroyed(FileViewLinks*);
static void fileviewlinks_on_button(FileViewLinks*, psy_ui_Button* sender);

/* vtable */
static psy_ui_ComponentVtable fileviewlinks_vtable;
static bool fileviewlinks_vtable_initialized = FALSE;

static void fileviewlinks_vtable_init(FileViewLinks* self)
{
	if (!fileviewlinks_vtable_initialized) {
		fileviewlinks_vtable = *(self->component.vtable);
		fileviewlinks_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			fileviewlinks_on_destroyed;
		fileviewlinks_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &fileviewlinks_vtable);
}

/* implementation */
void fileviewlinks_init(FileViewLinks* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	fileviewlinks_vtable_init(self);
	psy_ui_component_set_style_type(&self->component, STYLE_FILEVIEW_LINKS);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());
	psy_signal_init(&self->signal_selected);
	psy_table_init(&self->locations);
}

void fileviewlinks_on_destroyed(FileViewLinks* self)
{
	psy_table_dispose_all(&self->locations, NULL);
	psy_signal_dispose(&self->signal_selected);
}

void fileviewlinks_add(FileViewLinks* self, const char* label,
	const char* path)
{
	psy_ui_Button* button;

	button = psy_ui_button_allocinit(&self->component);
	psy_signal_connect(&button->signal_clicked, self, 
		fileviewlinks_on_button);
	psy_ui_button_prevent_translation(button);
	psy_ui_button_set_text(button, label);	
	psy_table_insert(&self->locations, psy_table_size(&self->locations), 
		psy_strdup(path));
}

const char* fileviewlinks_path(FileViewLinks* self, uintptr_t index)
{
	return (const char*)psy_table_at(&self->locations, index);
}

void fileviewlinks_on_button(FileViewLinks* self, psy_ui_Button* sender)
{
	uintptr_t index;

	index = psy_ui_component_index(&sender->component);
	psy_signal_emit(&self->signal_selected, self, 1, index);
}



/* FileView */

/* prototypes */
static void fileview_on_destroyed(FileView*);
static void fileview_build_drives(FileView*);
static void fileview_on_file_selected(FileView*, FileBox* sender);
static void fileview_on_dir_changed(FileView*, FileBox* sender);
static void fileview_on_link(FileView* self, FileViewLinks* sender, intptr_t index);
static void fileview_update_path(FileView*);
static void fileview_on_filter(FileView*, psy_ui_Component* sender);
static void fileview_on_hide(FileView*, psy_ui_Component* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(FileView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			fileview_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void fileview_init(FileView* self, psy_ui_Component* parent,
	DirConfig* dirconfig)
{		
	psy_ui_component_init(fileview_base(self), parent, NULL);
	vtable_init(self);
	self->dirconfig = dirconfig;
	self->property = NULL;
	psy_ui_component_set_id(fileview_base(self), VIEW_ID_FILEVIEW);
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(80.0, 25.0));
	psy_ui_component_set_padding(&self->component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	/* filename */
	psy_ui_component_init(&self->bottom, &self->component, NULL);	
	psy_ui_component_set_align(&self->bottom, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_margin(&self->bottom, psy_ui_margin_make_em(
		0.5, 0.0, 0.0, 0.0));
	psy_ui_label_init(&self->dir, &self->bottom);
	psy_ui_component_set_style_type(&self->dir.component,
		STYLE_FILEVIEW_DIRBAR);
	psy_ui_component_set_align(psy_ui_label_base(&self->dir),
		psy_ui_ALIGN_TOP);	
	psy_ui_component_init_align(&self->filebar, &self->bottom, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(&self->filebar,
		psy_ui_margin_make_em(0.5, 0.0, 0.0, 0.0));
	psy_ui_label_init_text(&self->filedesc, &self->filebar, "file.file");	
	psy_ui_component_set_align(&self->filedesc.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_textarea_init_single_line(&self->filename, &self->filebar);	
	psy_ui_textarea_set_text(&self->filename, PSYCLE_UNTITLED);
	psy_ui_component_set_align(&self->filename.component,
		psy_ui_ALIGN_CLIENT);
	/* left */			
	psy_ui_component_init(&self->left, fileview_base(self), NULL);	
	psy_ui_component_set_align(&self->left, psy_ui_ALIGN_LEFT);
	psy_ui_component_init(&self->options, &self->left, NULL);
	psy_ui_component_set_style_type(&self->options, STYLE_FILEVIEW_OPTIONS);
	psy_ui_component_set_align(&self->options, psy_ui_ALIGN_TOP);	
	/* filter panel */
	psy_ui_component_init(&self->filters, &self->options, NULL);
	psy_ui_component_set_style_type(&self->filters, STYLE_FILEVIEW_FILTERS);
	psy_ui_component_set_align(&self->filters, psy_ui_ALIGN_LEFT);
	fileviewfilter_init(&self->dirfilter, &self->filters);
	psy_ui_component_set_align(&self->dirfilter.component, psy_ui_ALIGN_TOP);
	fileviewsavefilter_init(&self->savefilter, &self->filters);
	psy_ui_component_set_align(&self->savefilter.component, psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(&self->savefilter.component,
		psy_ui_margin_make_em(1.0, 0.0, 0.0, 0.0));
	/* buttons */
	psy_ui_component_init_align(&self->buttons, &self->options, NULL,			
		psy_ui_ALIGN_LEFT);
	psy_ui_component_set_style_type(&self->buttons, STYLE_FILEVIEW_BUTTONS);
	psy_ui_component_set_margin(&self->buttons, psy_ui_margin_make_em(
		0.0, 0.5, 0.0, 0.5));
	psy_ui_button_init_text(&self->save, &self->buttons, "file.save");
	psy_ui_component_set_align(&self->save.component, psy_ui_ALIGN_TOP);
	psy_ui_button_init_text_connect(&self->refresh, &self->buttons,
		"file.refresh", self, fileview_on_filter);
	psy_ui_component_set_align(&self->refresh.component, psy_ui_ALIGN_TOP);
	psy_ui_button_init_text_connect(&self->showall, &self->buttons,
		"file.showall", self, fileview_on_filter);
	psy_ui_component_set_align(&self->showall.component, psy_ui_ALIGN_TOP);	
	psy_ui_button_init_text_connect(&self->exit, &self->buttons,
		"file.exit", self, fileview_on_hide);
	psy_ui_component_set_align(&self->exit.component, psy_ui_ALIGN_TOP);
	/* file links */
	fileviewlinks_init(&self->links, &self->options);	
	psy_ui_component_set_align(&self->links.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->links.signal_selected, self,
		fileview_on_link);
	/* files */
	filebox_init(&self->filebox, &self->component);	
	psy_ui_component_set_align(&self->filebox.component,
		psy_ui_ALIGN_CLIENT);			
	psy_ui_component_set_margin(&self->filebox.component, psy_ui_margin_make_em(
		0.0, 0.0, 0.0, 1.0));
	psy_ui_component_set_align(psy_ui_textarea_base(&self->filename),
		psy_ui_ALIGN_TOP);
	fileview_build_drives(self);		
	psy_signal_init(&self->signal_selected);
	psy_signal_connect(&self->filebox.signal_selected, self,
		fileview_on_file_selected);
	psy_signal_connect(&self->filebox.signal_dir_changed, self,
		fileview_on_dir_changed);	
	fileview_update_path(self);
	psy_signal_connect(&self->dirfilter.signal_changed, self,
		fileview_on_filter);
}

void fileview_on_destroyed(FileView* self)
{	
	psy_signal_dispose(&self->signal_selected);		
}

void fileview_build_drives(FileView* self)
{
	psy_List* p;
	psy_List* q;	
		
	fileviewlinks_add(&self->links, ".."psy_SLASHSTR, "..");	
	for (q = p = psy_drives(); p != NULL; psy_list_next(&p)) {		
		fileviewlinks_add(&self->links, (char*)psy_list_entry(p), 
			(char*)psy_list_entry(p));
	}
	psy_list_deallocate(&q, NULL);	
	fileviewlinks_add(&self->links, "Songs", dirconfig_songs(self->dirconfig));
	fileviewlinks_add(&self->links, "Home", psy_dir_home());	
}

void fileview_on_file_selected(FileView* self, FileBox* sender)
{		
	psy_ui_textarea_set_text(&self->filename,
		filebox_file_name(&self->filebox));
	if (self->property) {
		char path[4096];
		
		filebox_full_name(&self->filebox, path, 4096);
		psy_property_set_item_str(self->property, path);
	} else {
		psy_signal_emit(&self->signal_selected, self, 0);
	}
}

void fileview_on_dir_changed(FileView* self, FileBox* sender)
{
	psy_ui_label_set_text(&self->dir, psy_path_full(
		&self->filebox.curr_dir));
}

void fileview_update_path(FileView* self)
{	
	psy_ui_label_set_text(&self->dir, psy_path_full(
		&self->filebox.curr_dir));	
}

void fileview_filename(FileView* self, char* filename, uintptr_t maxsize)
{
	filename[0] = '\0';
	if (psy_strlen(psy_ui_textarea_text(&self->filename)) > 0) {	
		psy_snprintf(filename, maxsize, "%s%s%s",
			psy_path_prefix(&self->filebox.curr_dir),
			psy_SLASHSTR, psy_ui_textarea_text(&self->filename));
		printf("%s\n", filename);
	}	
}

void fileview_on_link(FileView* self, FileViewLinks* sender, intptr_t index)
{
	const char* path;
	
	path = fileviewlinks_path(sender, index);
	if (path) {
		fileview_set_directory(self, path);
	}	
}

void fileview_set_directory(FileView* self, const char* path)
{	
	filebox_set_directory(&self->filebox, path);	
	fileview_update_path(self);			
}

void fileview_on_filter(FileView* self, psy_ui_Component* sender)
{	
	if (sender == psy_ui_button_base(&self->showall)) {
		self->dirfilter.showall = TRUE;
	}		
	filebox_set_wildcard(&self->filebox, 
		fileviewfilter_type(&self->dirfilter));		
	psy_ui_component_align_full(&self->filebox.component);
}

void fileview_on_hide(FileView* self, psy_ui_Component* sender)
{
	psy_ui_component_hide_align(fileview_base(self));
}

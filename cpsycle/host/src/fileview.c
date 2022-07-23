/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "fileview.h"
#include "styles.h"
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
static void fileviewfilter_update(FileViewFilter*);
static void fileviewfilter_on_checkbox(FileViewFilter*,
	psy_ui_CheckBox* sender);
static void fileviewfilter_clear(FileViewFilter*);

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
	psy_ui_checkbox_init_text(&self->psy, &self->items, "psy");	
	psy_ui_checkbox_init_text(&self->mod, &self->items, "mod");
	self->filter = FILEVIEWFILTER_PSY;
	self->showall = FALSE;
	fileviewfilter_update(self);
	psy_signal_connect(&self->psy.signal_clicked, self,
		fileviewfilter_on_checkbox);
	psy_signal_connect(&self->mod.signal_clicked, self,
		fileviewfilter_on_checkbox);
	psy_signal_init(&self->signal_changed);
}

void fileviewfilter_on_destroyed(FileViewFilter* self)
{
	psy_signal_dispose(&self->signal_changed);
}

void fileviewfilter_update(FileViewFilter* self)
{
	fileviewfilter_clear(self);
	switch (self->filter) {
	case FILEVIEWFILTER_PSY:
		psy_ui_checkbox_check(&self->psy);
		break;
	case FILEVIEWFILTER_MOD:
		psy_ui_checkbox_check(&self->mod);
		break;
	default:;
	}
}

void fileviewfilter_on_checkbox(FileViewFilter* self, psy_ui_CheckBox* sender)
{
	fileviewfilter_clear(self);
	if (sender == &self->psy) {
		psy_ui_checkbox_check(&self->psy);
		self->showall = FALSE;
		self->filter = FILEVIEWFILTER_PSY;
		psy_signal_emit(&self->signal_changed, self, 0);
	} else if (sender == &self->mod) {
		psy_ui_checkbox_check(&self->mod);
		self->showall = FALSE;
		self->filter = FILEVIEWFILTER_MOD;
		psy_signal_emit(&self->signal_changed, self, 0);
	}
}

void fileviewfilter_clear(FileViewFilter* self)
{
	psy_ui_checkbox_disable_check(&self->psy);
	psy_ui_checkbox_disable_check(&self->mod);
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

const char* fileviewfilter_wildcard(const FileViewFilter* self)
{
	static const char* all = "*.*";
	static const char* psy = "*.psy";
	static const char* mod = "*.mod";
	static const char* xm = "*.xm";
	
	if (self->showall) {
		return all;
	}
	if (self->filter == FILEVIEWFILTER_PSY) {
		return psy;
	}
	if (self->filter == FILEVIEWFILTER_MOD) {
		 return mod;
	}
	return psy;
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


/* FileLine */

/* prototypes */
static void fileline_on_destroyed(FileLine*);

/* vtable */
static psy_ui_ComponentVtable fileline_vtable;
static bool fileline_vtable_initialized = FALSE;

static void fileline_vtable_init(FileLine* self)
{
	if (!fileline_vtable_initialized) {
		fileline_vtable = *(self->component.vtable);
		fileline_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			fileline_on_destroyed;
		fileline_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &fileline_vtable);
}

/* implementation */
void fileline_init(FileLine* self, psy_ui_Component* parent, const char* path, bool is_dir)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	fileline_vtable_init(self);
	self->path = psy_strdup(path);
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_set_align(&self->component, psy_ui_ALIGN_TOP);
	/* filename */
	psy_ui_button_init(&self->name, &self->component);
	psy_ui_component_set_align(psy_ui_button_base(&self->name), psy_ui_ALIGN_CLIENT);
	psy_ui_button_prevent_translation(&self->name);
	psy_ui_button_set_text_alignment(&self->name,
		psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL);
	psy_ui_component_set_margin(&self->name.component, psy_ui_margin_make_em(
		0.0, 1.0, 0.0, 0.0));
	if (is_dir) {
		psy_ui_button_set_text(&self->name, path);
	} else {
		psy_Path extract;

		psy_path_init(&extract, path);				
		psy_ui_button_set_text(&self->name, psy_path_filename(&extract));
		psy_path_dispose(&extract);		
	}
	if (!is_dir) {
		uintptr_t size;
		char str[256];

		/* size */
		psy_ui_label_init(&self->size, &self->component);
		psy_ui_component_set_align(psy_ui_label_base(&self->size),
			psy_ui_ALIGN_RIGHT);
		psy_ui_label_prevent_translation(&self->size);
		psy_ui_label_set_text_alignment(&self->size,
			psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL);
		size = psy_file_size(self->path);
		if (size != psy_INDEX_INVALID) {
			if (size > 10240) {
				psy_snprintf(str, 512, "%dk",
					(int)(ceil((double)size / 1024.0)));
			} else {
				psy_snprintf(str, 512, "%d",
					(int)size);
			}
			psy_ui_label_set_text(&self->size, str);
		}		
	}
}

FileLine* fileline_alloc(void)
{
	return (FileLine*)malloc(sizeof(FileLine));
}

FileLine* fileline_allocinit(psy_ui_Component* parent, const char* path, bool is_dir)
{
	FileLine* rv;

	rv = fileline_alloc();
	if (rv) {
		fileline_init(rv, parent, path, is_dir);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void fileline_on_destroyed(FileLine* self)
{
	free(self->path);
	self->path = NULL;
}

/* FileBox */

/* prototypes */
static void filebox_on_destroyed(FileBox*);
static void filebox_add(FileBox*, psy_ui_Component* parent, const char* path,
	bool is_dir);
static void filebox_on_button(FileBox*, psy_ui_Button* sender);
static void filebox_on_dir_button(FileBox*, psy_ui_Button* sender);
static void filebox_on_timer(FileBox*, uintptr_t timer_id);
static psy_List* filebox_sort(psy_List* source, psy_fp_comp comp);
static int filebox_comp_filename(psy_List* p, psy_List* q);

/* vtable */
static psy_ui_ComponentVtable filebox_vtable;
static bool filebox_vtable_initialized = FALSE;

static void filebox_vtable_init(FileBox* self)
{
	if (!filebox_vtable_initialized) {
		filebox_vtable = *(self->component.vtable);
		filebox_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			filebox_on_destroyed;
		filebox_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			filebox_on_timer;
		filebox_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &filebox_vtable);
}

/* implementation */
void filebox_init(FileBox* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	filebox_vtable_init(self);
	self->selindex = psy_INDEX_INVALID;
	self->rebuild = FALSE;
	self->wildcard = psy_strdup("*.psy");
	psy_path_init(&self->curr_dir, NULL);
#if defined(DIVERSALIS__OS__MICROSOFT)
	psy_path_set_prefix(&self->curr_dir, "C:\\");	
#else	
	psy_path_set_prefix(&self->curr_dir, "/");	
#endif
	psy_signal_init(&self->signal_selected);
	psy_signal_init(&self->signal_dir_changed);
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(80.0, 0.0));
	psy_ui_component_init(&self->pane, &self->component, NULL);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_overflow(&self->pane, psy_ui_OVERFLOW_SCROLL);
	psy_ui_component_setscrollstep(&self->pane,
		psy_ui_size_make_em(0.0, 1.0));	
	psy_ui_scroller_init(&self->scroller, &self->component, 
		NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->pane);
	psy_ui_component_set_style_type(&self->scroller.component, 
		STYLE_SEQLISTVIEW);
	psy_ui_component_set_padding(psy_ui_scroller_base(&self->scroller),
		psy_ui_margin_make_em(0.5, 0.5, 0.0, 1.0));
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_HCLIENT);
	psy_ui_component_init(&self->dirpane, &self->pane, NULL);
	psy_ui_component_set_align(&self->dirpane, psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->filepane, &self->pane, NULL);
	psy_ui_component_set_align(&self->filepane, psy_ui_ALIGN_TOP);
	filebox_read(self, psy_path_prefix(&self->curr_dir));
}

void filebox_on_destroyed(FileBox* self)
{
	psy_signal_dispose(&self->signal_selected);
	psy_signal_dispose(&self->signal_dir_changed);
	psy_path_dispose(&self->curr_dir);
	free(self->wildcard);
	self->wildcard = NULL;
}

void filebox_read(FileBox* self, const char* path)
{
	psy_List* p;
	psy_List* q;
	psy_List* files;
	psy_List* sorted;	

	self->selindex = psy_INDEX_INVALID;
	psy_ui_component_clear(&self->dirpane);
	psy_ui_component_clear(&self->filepane);
	files = psy_directories(path);
	sorted = filebox_sort(files, (psy_fp_comp)filebox_comp_filename);
	psy_list_deallocate(&files, (psy_fp_disposefunc)NULL);
	files = sorted;
	for (q = p = files; p != NULL; psy_list_next(&p)) {			
		filebox_add(self, &self->dirpane, (const char*)p->entry, TRUE);
	}
	psy_list_deallocate(&q, NULL);
	files = psy_files(path, self->wildcard, psy_ui_NONE_RECURSIVE);
	sorted = filebox_sort(files, (psy_fp_comp)filebox_comp_filename);
	psy_list_deallocate(&files, (psy_fp_disposefunc)NULL);
	files = sorted;
	for (q = p = files; p != NULL; psy_list_next(&p)) {			
		filebox_add(self, &self->filepane, (const char*)p->entry, FALSE);
	}
	psy_list_deallocate(&q, NULL);
	psy_path_set_prefix(&self->curr_dir, path);
	psy_signal_emit(&self->signal_dir_changed, self, 0);
}

void filebox_add(FileBox* self, psy_ui_Component* parent, const char* path, bool is_dir)
{	
	FileLine* file_line;
	
	file_line = fileline_allocinit(parent, path, is_dir);	
	if (parent == &self->dirpane) {
		psy_signal_connect(&file_line->name.signal_clicked,
			self, filebox_on_dir_button);
	} else {
		psy_signal_connect(&file_line->name.signal_clicked,
			self, filebox_on_button);
	}
}

psy_List* filebox_sort(psy_List* source, psy_fp_comp comp)
{
	psy_List* rv;

	rv = NULL;
	if (source) {
		uintptr_t num;
		psy_Table arrayptr;
		psy_List* p;
		uintptr_t i;

		num = psy_list_size(source);
		if (num == 0) {
			return NULL;
		}
		psy_table_init(&arrayptr);
		p = source;
		for (i = 0; p != NULL && i < num; p = p->next, ++i) {
			psy_table_insert(&arrayptr, i, p);
		}
		psy_qsort(&arrayptr,
			(psy_fp_set_index_double)psy_table_insert,
			(psy_fp_index_double)psy_table_at,
			0, (int)(num - 1), comp);
		for (i = 0; i < num; ++i) {
			psy_list_append(&rv,
				psy_strdup((const char*)(((psy_List*)
					(psy_table_at(&arrayptr, i)))->entry)));
		}
		psy_table_dispose(&arrayptr);
	}
	return rv;
}

int filebox_comp_filename(psy_List* p, psy_List* q)
{
	const char* left;
	const char* right;

	left = (const char*)p->entry;
	right = (const char*)q->entry;
	return strcmp(left, right);
}


uintptr_t filebox_selected(const FileBox* self)
{
	return self->selindex;
}

void filebox_set_wildcard(FileBox* self, const char* wildcard)
{
	psy_strreset(&self->wildcard, wildcard);
	filebox_read(self, psy_path_full(&self->curr_dir));
}

void filebox_set_directory(FileBox* self, const char* path)
{	
	if (strcmp(path, "..") == 0) {
		psy_path_remove_dir(&self->curr_dir);
		filebox_read(self, psy_path_prefix(&self->curr_dir));
	} else {
		filebox_read(self, path);
	}
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->component);
}

void filebox_on_dir_button(FileBox* self, psy_ui_Button* sender)
{
	if (strcmp(psy_ui_button_text(sender), psy_SLASHSTR"..") == 0) {
		psy_path_remove_dir(&self->curr_dir);
		self->rebuild = TRUE;
		psy_ui_component_start_timer(&self->component, 0, 50);
	} else {
		psy_path_append_dir(&self->curr_dir, psy_ui_button_text(sender));
		psy_ui_component_start_timer(&self->component, 0, 50);
		self->rebuild = TRUE;
	}
}

void filebox_on_timer(FileBox* self, uintptr_t timer_id)
{
	if (self->rebuild) {
		self->rebuild = FALSE;
		psy_ui_component_stop_timer(&self->component, 0);
		filebox_read(self, psy_path_prefix(&self->curr_dir));
		psy_ui_component_align(&self->component);
		psy_ui_component_invalidate(&self->component);
	}
}

void filebox_on_button(FileBox* self, psy_ui_Button* sender)
{
	self->selindex = psy_ui_component_index(
		psy_ui_component_parent(psy_ui_button_base(sender)));
	psy_signal_emit(&self->signal_selected, self, 0);
}

const char* filebox_file_name(FileBox* self)
{
	FileLine* file_line;

	file_line = (FileLine*)psy_ui_component_at(&self->filepane, self->selindex);
	if (file_line) {
		return psy_ui_button_text(&file_line->name);
	}
	return "";
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
void fileview_init(FileView* self, psy_ui_Component* parent)
{		
	psy_ui_component_init(fileview_base(self), parent, NULL);
	vtable_init(self);		
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
	psy_ui_component_set_align(psy_ui_label_base(&self->dir),
		psy_ui_ALIGN_TOP);
	psy_ui_component_init_align(&self->filebar, &self->bottom, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->filedesc, &self->filebar, "file.file");
	psy_ui_component_set_align(&self->filedesc.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_textarea_init_single_line(&self->filename, &self->filebar);	
	psy_ui_textarea_set_text(&self->filename, PSYCLE_UNTITLED);
	psy_ui_component_set_align(&self->filename.component,
		psy_ui_ALIGN_CLIENT);
	/* filter panel */
	psy_ui_component_init(&self->filters, fileview_base(self), NULL);
	psy_ui_component_set_align(&self->filters, psy_ui_ALIGN_LEFT);
	fileviewfilter_init(&self->dirfilter, &self->filters);
	psy_ui_component_set_align(&self->dirfilter.component, psy_ui_ALIGN_TOP);
	fileviewsavefilter_init(&self->savefilter, &self->filters);
	psy_ui_component_set_align(&self->savefilter.component, psy_ui_ALIGN_TOP);
	/* buttons */
	psy_ui_component_init_align(&self->buttons, fileview_base(self), NULL,			
		psy_ui_ALIGN_LEFT);
	psy_ui_component_set_margin(&self->buttons, psy_ui_margin_make_em(
		0.0, 3.0, 0.0, 3.0));
	psy_ui_button_init_text(&self->save, &self->buttons,
		"file.save");
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
	/* drives */
	fileviewlinks_init(&self->links, &self->component);
	psy_ui_component_set_align(&self->links.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->links.signal_selected, self,
		fileview_on_link);
	/* files */
	filebox_init(&self->filebox, &self->component);	
	psy_ui_component_set_align(&self->filebox.component,
		psy_ui_ALIGN_CLIENT);			
	psy_ui_component_set_margin(&self->filebox.component, psy_ui_margin_make_em(
		0.0, 0.0, 0.0, 3.0));
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
//	fileviewlinks_add(&self->links, psy_SLASHSTR, psy_SLASHSTR);
	for (q = p = psy_drives(); p != NULL; psy_list_next(&p)) {		
		fileviewlinks_add(&self->links, (char*)psy_list_entry(p), 
			(char*)psy_list_entry(p));
	}
	psy_list_deallocate(&q, NULL);
}

void fileview_on_file_selected(FileView* self, FileBox* sender)
{		
	psy_ui_textarea_set_text(&self->filename,
		filebox_file_name(&self->filebox));
	psy_signal_emit(&self->signal_selected, self, 0);	
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
		fileviewfilter_wildcard(&self->dirfilter));	
	psy_ui_component_align_full(&self->filebox.component);
}

void fileview_on_hide(FileView* self, psy_ui_Component* sender)
{
	psy_ui_component_hide_align(fileview_base(self));
}

/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "filebox.h"
/* host */
#include "styles.h"
/* container */
#include <qsort.h>
/* platform */
#include "../../detail/os.h"
#include "../../detail/portable.h"


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
void fileline_init(FileLine* self, psy_ui_Component* parent, const char* path,
	bool is_dir)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	fileline_vtable_init(self);
	psy_ui_component_set_preferred_height(&self->component, 
		psy_ui_value_make_eh(1.5));
	self->path = psy_strdup(path);
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_set_align(&self->component, psy_ui_ALIGN_TOP);
	/* filename */
	psy_ui_button_init(&self->name, &self->component);	
	psy_ui_component_set_align(psy_ui_button_base(&self->name),
		psy_ui_ALIGN_CLIENT);
	psy_ui_button_prevent_translation(&self->name);
	psy_ui_button_set_text_alignment(&self->name,
		psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL);
	self->name.stoppropagation = FALSE;	
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
		psy_ui_component_set_margin(&self->size.component,
			psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));
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
static void filebox_on_mouse_down(FileBox*, psy_ui_MouseEvent* ev);

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
		filebox_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			filebox_on_mouse_down;
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
	self->property = NULL;
	self->selindex = psy_INDEX_INVALID;
	self->rebuild = FALSE;
	self->wildcard = psy_strdup("*.psy");
	self->dirsonly = FALSE;
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
	psy_ui_component_set_wheel_scroll(&self->pane, 2);	
	psy_ui_component_setscrollstep(&self->pane,
		psy_ui_size_make_em(0.0, 1.0));	
	psy_ui_scroller_init(&self->scroller, &self->component, 
		NULL, NULL);
	self->scroller.prevent_mouse_down_propagation = FALSE;
	psy_ui_scroller_set_client(&self->scroller, &self->pane);
	psy_ui_component_set_style_type(&self->scroller.component, 
		STYLE_SEQLISTVIEW);
	psy_ui_component_set_padding(psy_ui_scroller_base(&self->scroller),
		psy_ui_margin_make_em(0.5, 0.5, 0.0, 1.0));
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_HCLIENT);
	psy_ui_component_init(&self->dirpane, &self->pane, NULL);
	psy_ui_component_set_style_type(&self->dirpane, STYLE_FILEBOX_DIR_PANE);
	psy_ui_component_set_align(&self->dirpane, psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->filepane, &self->pane, NULL);
	psy_ui_component_set_style_type(&self->filepane, STYLE_FILEBOX_FILE_PANE);
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
	psy_List* sorted;
	psy_List* files;

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
	if (!self->dirsonly) {		
		files = psy_files(path, self->wildcard, psy_ui_NONE_RECURSIVE);
		sorted = filebox_sort(files, (psy_fp_comp)filebox_comp_filename);
		psy_list_deallocate(&files, (psy_fp_disposefunc)NULL);
		files = sorted;
		for (q = p = files; p != NULL; psy_list_next(&p)) {			
			filebox_add(self, &self->filepane, (const char*)p->entry, FALSE);
		}
		psy_list_deallocate(&q, NULL);
	}
	psy_path_set_prefix(&self->curr_dir, path);
	psy_signal_emit(&self->signal_dir_changed, self, 0);
}

void filebox_add(FileBox* self, psy_ui_Component* parent, const char* path,
	bool is_dir)
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
	assert(self);
	
#if defined DIVERSALIS__OS__POSIX	
	char norm[4096];

	psy_dir_normalize(path, norm);
	psy_path_set_prefix(&self->curr_dir, norm);
	filebox_read(self, norm);
#else	
	if (strcmp(path, "..") == 0) {
		psy_path_remove_dir(&self->curr_dir);
		filebox_read(self, psy_path_prefix(&self->curr_dir));
	} else {
		filebox_read(self, path);
	}
#endif	
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->component);
}

void filebox_on_dir_button(FileBox* self, psy_ui_Button* sender)
{
	assert(self);
	
	if (strcmp(psy_ui_button_text(sender), psy_SLASHSTR"..") == 0 ||
			(strcmp(psy_ui_button_text(sender), "..") == 0)) {
		psy_path_remove_dir(&self->curr_dir);
#if defined DIVERSALIS__OS__POSIX
		if (psy_strlen(psy_path_prefix(&self->curr_dir)) == 0) {
			psy_path_set_prefix(&self->curr_dir, "/");
		}
#endif
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
	assert(self);
	
	self->selindex = psy_ui_component_index(
		psy_ui_component_parent(psy_ui_button_base(sender)));
	psy_signal_emit(&self->signal_selected, self, 0);
}

void filebox_on_mouse_down(FileBox* self, psy_ui_MouseEvent* ev)
{
	assert(self);
	
	if (psy_ui_mouseevent_button(ev) == 2) {
		psy_path_remove_dir(&self->curr_dir);
#if defined DIVERSALIS__OS__POSIX
		if (psy_strlen(psy_path_prefix(&self->curr_dir)) == 0) {
			psy_path_set_prefix(&self->curr_dir, "/");
		}
#endif
		filebox_read(self, psy_path_prefix(&self->curr_dir));
		psy_ui_component_align(&self->component);
		psy_ui_component_invalidate(&self->component);
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

const char* filebox_file_name(FileBox* self)
{
	FileLine* file_line;

	assert(self);
	
	file_line = (FileLine*)psy_ui_component_at(&self->filepane, self->selindex);
	if (file_line) {
		return psy_ui_button_text(&file_line->name);
	}
	return "";
}

const char* filebox_directory(const FileBox* self)
{
	return psy_path_full(&self->curr_dir);
}

void filebox_full_name(FileBox* self, char* rv, uintptr_t maxlen)
{
	psy_snprintf(rv, maxlen, "%s"psy_SLASHSTR"%s",
		psy_path_prefix(&self->curr_dir),
		filebox_file_name(self));
	printf("%s\n", rv);
}

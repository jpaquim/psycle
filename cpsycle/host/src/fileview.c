/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "fileview.h"
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
static void fileviewfilter_ondestroy(FileViewFilter*);
static void fileviewfilter_update(FileViewFilter*);
static void fileviewfilter_oncheckbox(FileViewFilter*,
	psy_ui_CheckBox* sender);
static void fileviewfilter_clear(FileViewFilter*);

/* vtable */
static psy_ui_ComponentVtable fileviewfilter_vtable;
static bool fileviewfilter_vtable_initialized = FALSE;

static void fileviewfilter_vtable_init(FileViewFilter* self)
{
	if (!fileviewfilter_vtable_initialized) {
		fileviewfilter_vtable = *(self->component.vtable);
		fileviewfilter_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			fileviewfilter_ondestroy;
		fileviewfilter_vtable_initialized = TRUE;
	}
	self->component.vtable = &fileviewfilter_vtable;
}

/* implementation */
void fileviewfilter_init(FileViewFilter* self, psy_ui_Component* parent)
{
	psy_ui_component_init(fileviewfilter_base(self), parent, NULL);
	fileviewfilter_vtable_init(self);
	psy_ui_component_setdefaultalign(fileviewfilter_base(self), psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_appdefaults()));
	psy_ui_label_init_text(&self->desc, fileviewfilter_base(self), NULL,
		"Item:");
	psy_ui_checkbox_init_text(&self->psy, fileviewfilter_base(self),
		"psy");	
	psy_ui_checkbox_init_text(&self->mod, fileviewfilter_base(self),
		"mod");
	self->filter = FILEVIEWFILTER_PSY;
	self->showall = FALSE;
	fileviewfilter_update(self);
	psy_signal_connect(&self->psy.signal_clicked, self,
		fileviewfilter_oncheckbox);
	psy_signal_connect(&self->mod.signal_clicked, self,
		fileviewfilter_oncheckbox);
	psy_signal_init(&self->signal_changed);
}

void fileviewfilter_ondestroy(FileViewFilter* self)
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

void fileviewfilter_oncheckbox(FileViewFilter* self, psy_ui_CheckBox* sender)
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
	psy_ui_checkbox_disablecheck(&self->psy);
	psy_ui_checkbox_disablecheck(&self->mod);
}

/* FileViewSaveFilter */
void fileviewsavefilter_init(FileViewSaveFilter* self, psy_ui_Component* parent)
{
	psy_ui_component_init(fileviewsavefilter_base(self), parent, NULL);
	psy_ui_component_setdefaultalign(fileviewsavefilter_base(self), psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_appdefaults()));
	psy_ui_label_init_text(&self->desc, fileviewsavefilter_base(self), NULL,
		"Save as:");
	psy_ui_checkbox_init_text(&self->psy, fileviewsavefilter_base(self),
		"psy");	
	self->filter = FILEVIEWFILTER_PSY;	
}

/* prototypes */
static void fileview_ondestroy(FileView*, psy_ui_Component* sender);
static void fileview_build(FileView*);
static void fileview_builddrives(FileView*);
static void fileview_builddirectories(FileView*);
static int fileview_onenumdir(FileView*, const char* path, int flag);
static void fileview_onfileboxselected(FileView*, psy_ui_ListBox* sender,
	intptr_t index);
static void fileview_ondrives(FileView* self, psy_ui_TabBar* sender, int index);
static psy_List* fileview_sort(psy_List* source, psy_fp_comp comp);
static int fileview_comp_filename(psy_List* p, psy_List* q);
static void fileview_updatepath(FileView*);
static void fileview_currfile(FileView*, char* filename, uintptr_t maxsize);
static void fileview_file(FileView*, char* filename, uintptr_t index, uintptr_t maxsize);
static void fileview_ondirfilter(FileView*, psy_ui_Component* sender);
static void fileview_onhide(FileView*, psy_ui_Component* sender);

/* implementation */
void fileview_init(FileView* self, psy_ui_Component* parent)
{		
	psy_ui_component_init(fileview_base(self), parent, NULL);
	/* filename */
	psy_ui_component_init(&self->bottom, &self->component, NULL);
	psy_ui_component_setalign(&self->bottom, psy_ui_ALIGN_BOTTOM);
	psy_ui_label_init(&self->dir, &self->bottom, NULL);
	psy_ui_component_setalign(psy_ui_label_base(&self->dir),
		psy_ui_ALIGN_TOP);
	psy_ui_component_init_align(&self->filebar, &self->bottom, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->filedesc, &self->filebar, NULL, "file.file");
	psy_ui_component_setalign(&self->filedesc.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_edit_init(&self->filename, &self->filebar);
	psy_ui_component_setalign(&self->filename.component,
		psy_ui_ALIGN_CLIENT);
	/* filter panel */
	psy_ui_component_init(&self->filters, fileview_base(self), NULL);
	psy_ui_component_setalign(&self->filters, psy_ui_ALIGN_LEFT);
	fileviewfilter_init(&self->dirfilter, &self->filters);
	psy_ui_component_setalign(&self->dirfilter.component, psy_ui_ALIGN_TOP);
	fileviewsavefilter_init(&self->savefilter, &self->filters);
	psy_ui_component_setalign(&self->savefilter.component, psy_ui_ALIGN_TOP);	
	/* buttons */
	psy_ui_component_init_align(&self->buttons, fileview_base(self), NULL,			
		psy_ui_ALIGN_LEFT);
	psy_ui_button_init_text(&self->save, &self->buttons, NULL,
		"file.save");
	psy_ui_component_setalign(&self->save.component, psy_ui_ALIGN_TOP);
	psy_ui_button_init_text_connect(&self->refresh, &self->buttons, NULL,
		"file.refresh", self, fileview_ondirfilter);
	psy_ui_component_setalign(&self->refresh.component, psy_ui_ALIGN_TOP);
	psy_ui_button_init_text_connect(&self->showall, &self->buttons, NULL,
		"file.showall", self, fileview_ondirfilter);
	psy_ui_component_setalign(&self->showall.component, psy_ui_ALIGN_TOP);	
	psy_ui_button_init_text_connect(&self->exit, &self->buttons, NULL,
		"file.exit", self, fileview_onhide);
	psy_ui_component_setalign(&self->exit.component, psy_ui_ALIGN_TOP);
	/* drives */
	psy_ui_tabbar_init(&self->drives, fileview_base(self));
	psy_ui_tabbar_settabalign(&self->drives, psy_ui_ALIGN_TOP);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->drives),
		psy_ui_ALIGN_LEFT);
	/* directory */
	psy_ui_listbox_init(&self->filebox, &self->component);
	psy_ui_listbox_setcharnumber(&self->filebox, 80.0);
	psy_ui_component_setalign(psy_ui_listbox_base(&self->filebox),
		psy_ui_ALIGN_CLIENT);			
	psy_ui_component_setalign(psy_ui_edit_base(&self->filename),
		psy_ui_ALIGN_TOP);
	psy_path_init(&self->curr, "");	
#if defined(DIVERSALIS__OS__MICROSOFT)
	psy_path_init(&self->curr, "C:\\");	
#else	
	psy_path_init(&self->curr, "//");	
#endif
	fileview_builddrives(self);	
	self->files = NULL;	
	self->numdirectories = 0;
	fileview_builddirectories(self);
	fileview_build(self);
	psy_signal_init(&self->signal_selected);
	psy_signal_connect(&self->filebox.signal_selchanged, self,
		fileview_onfileboxselected);
	psy_signal_connect(&self->drives.signal_change, self,
		fileview_ondrives);
	fileview_updatepath(self);
	psy_signal_connect(&self->dirfilter.signal_changed, self,
		fileview_ondirfilter);
}

void fileview_ondestroy(FileView* self, psy_ui_Component* sender)
{
	psy_path_dispose(&self->curr);	
	psy_signal_dispose(&self->signal_selected);
	psy_list_deallocate(&self->files, NULL);
}

void fileview_build(FileView* self)
{
#if defined(DIVERSALIS__OS__UNIX)
	psy_List* sorted;
#endif
	psy_ui_listbox_clear(&self->filebox);
	if (self->dirfilter.showall) {
		psy_dir_enumerate(self, psy_path_prefix(&self->curr), "*.*", 0,
			(psy_fp_findfile)fileview_onenumdir);
	} else if (self->dirfilter.filter == FILEVIEWFILTER_PSY) {
		psy_dir_enumerate(self, psy_path_prefix(&self->curr), "*.psy", 0,
			(psy_fp_findfile)fileview_onenumdir);
	} else if (self->dirfilter.filter == FILEVIEWFILTER_MOD) {
		psy_dir_enumerate(self, psy_path_prefix(&self->curr), "*.mod", 0,
			(psy_fp_findfile)fileview_onenumdir);
		psy_dir_enumerate(self, psy_path_prefix(&self->curr), "*.xm", 0,
			(psy_fp_findfile)fileview_onenumdir);
	}
#if defined(DIVERSALIS__OS__UNIX)
	sorted = fileview_sort(self->files, (psy_fp_comp)fileview_comp_filename);
	psy_list_deallocate(&self->files, (psy_fp_disposefunc)NULL);
	self->files = sorted;
#endif
	if (self->files) {
		psy_List* p;
		uintptr_t i;

		for (p = self->files, i = 0; p != NULL; p = p->next, ++i) {
			if (p->entry) {
				char filename[4096];
				char itemstr[512];
				psy_Path path;
				uintptr_t size;				

				if (psy_strlen((const char*)p->entry) == 0) {
					continue;
				}				
				psy_snprintf(filename, 4096, "%s%s%s", psy_path_prefix(&self->curr),
					psy_SLASHSTR, (const char*)p->entry);
				psy_path_init(&path, filename);
				size = psy_file_size(&path);				
				if (size != psy_INDEX_INVALID && !psy_file_is_directory(&path)) {
					if (size > 10240) {
						psy_snprintf(itemstr, 512, "%s %dk", (const char*)p->entry,
							(int)(ceil((double)size / 1024.0)));
					} else {
						psy_snprintf(itemstr, 512, "%s %d", (const char*)p->entry, (int)size);
					}
				} else {
					psy_snprintf(itemstr, 512, "%s", (const char*)p->entry);
				}
				psy_path_dispose(&path);
				psy_ui_listbox_addtext(&self->filebox, itemstr);				
			}
		}		
	}
}

int fileview_onenumdir(FileView* self, const char* filename, int flag)
{
	if (filename) {
		psy_Path extract;

		psy_path_init(&extract, filename);
		psy_list_append(&self->files, psy_strdup(
			psy_path_filename(&extract)));
		psy_path_dispose(&extract);
	}
	return 1;
}

void fileview_builddirectories(FileView* self)
{
	psy_List* p;
	psy_List* q;	
	
	psy_list_deallocate(&self->files, NULL);
	psy_ui_listbox_clear(&self->filebox);
	self->numdirectories = 0;
	for (q = p = psy_directories(psy_path_prefix(&self->curr)); p != NULL;
			psy_list_next(&p)) {		
		psy_list_append(&self->files, psy_strdup((char*)psy_list_entry(p)));
		++self->numdirectories;
	}
	psy_list_deallocate(&q, NULL);
}

void fileview_builddrives(FileView* self)
{
	psy_List* p;
	psy_List* q;
	
	psy_ui_tabbar_clear(&self->drives);
	for (q = p = psy_drives(); p != NULL; psy_list_next(&p)) {		
		psy_ui_tabbar_append(&self->drives, (char*)psy_list_entry(p),
			psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());
	}
	psy_list_deallocate(&q, NULL);
}

void fileview_onfileboxselected(FileView* self, psy_ui_ListBox* sender,
	intptr_t index)
{
	if (psy_ui_listbox_cursel(&self->filebox) != -1) {
		if (psy_ui_listbox_cursel(&self->filebox) < self->numdirectories) {
			const char* itemstr;

			itemstr = fileview_path(self);
			if (itemstr) {
				if ((itemstr + 1) && (itemstr[0] == psy_SLASH) &&
					strcmp(".", itemstr + 1) == 0) {
					/* skip */
				} else if ((itemstr + 1) && (itemstr[0] == psy_SLASH) &&
						strcmp("..", itemstr + 1) == 0) {
					psy_path_remove_dir(&self->curr);
				} else {
					psy_path_append_dir(&self->curr, fileview_path(self));
				}
				fileview_builddirectories(self);
				fileview_build(self);
				fileview_updatepath(self);
				psy_ui_component_align_full(&self->filebox.component);
			}
		} else {
			fileview_updatepath(self);
			psy_signal_emit(&self->signal_selected, self, 0);
		}
	}
}

void fileview_updatepath(FileView* self)
{
	char fname[256];
	
	psy_ui_label_settext(&self->dir,
		psy_path_full(&self->curr));
	fileview_currfile(self, fname, 256);
	psy_ui_edit_settext(&self->filename, fname);
}

const char* fileview_path(FileView* self)
{	
	if (psy_ui_listbox_cursel(&self->filebox) != -1) {
		psy_List* p;
		p = psy_list_at(self->files, psy_ui_listbox_cursel(&self->filebox));
		if (p) {
			return (const char*)psy_list_entry(p);
		}
	}
	return "";
}

void fileview_filename(FileView* self, char* filename, uintptr_t maxsize)
{
	filename[0] = '\0';
	if (psy_strlen(psy_ui_edit_text(&self->filename)) > 0) {	
		psy_snprintf(filename, maxsize, "%s%s%s", psy_path_prefix(&self->curr),
			psy_SLASHSTR, psy_ui_edit_text(&self->filename));
	}	
}

void fileview_currfile(FileView* self, char* filename, uintptr_t maxsize)
{
	fileview_file(self, filename, psy_ui_listbox_cursel(&self->filebox),
		maxsize);	
}

void fileview_file(FileView* self, char* filename, uintptr_t index, uintptr_t maxsize)
{	
	filename[0] = '\0';
	if (index != -1) {
		psy_List* p;

		p = psy_list_at(self->files, index);
		if (p) {
			const char* fname;

			fname = (const char*)psy_list_entry(p);
			psy_snprintf(filename, maxsize, "%s", fname);
		}
	}
}


void fileview_ondrives(FileView* self, psy_ui_TabBar* sender, int index)
{
	psy_ui_Tab* tab;

	tab = psy_ui_tabbar_tab(&self->drives, index);
	if (tab) {
		fileview_setdirectory(self, tab->text);
	}
}

psy_List* fileview_sort(psy_List* source, psy_fp_comp comp)
{
	psy_List* rv;

	rv = NULL;
	if (source) {		
		uintptr_t num;		
		psy_List** arrayptr;

		num = psy_list_size(source);
		if (num == 0) {
			return NULL;
		}
		arrayptr = (psy_List**)malloc(sizeof(char*) * num);
		if (arrayptr) {
			psy_List* p;
			uintptr_t i;
			
			p = source;
			for (i = 0; p != NULL && i < num; p = p->next, ++i) {
				arrayptr[i] = p;
			}
			psy_qsort((void **)arrayptr, 0, (int)(num - 1), comp);
			for (i = 0; i < num; ++i) {
				psy_list_append(&rv,
					psy_strdup((const char*)(arrayptr[i]->entry)));
			}			
			free(arrayptr);
			arrayptr = NULL;
		}
	}
	return rv;
}

int fileview_comp_filename(psy_List* p, psy_List* q)
{
	const char* left;
	const char* right;

	left = (const char*)p->entry;	
	right = (const char*)q->entry;	
	return strcmp(left, right);
}

void fileview_setdirectory(FileView* self, const char* directory)
{
	psy_path_dispose(&self->curr);
	psy_path_init(&self->curr, NULL);
	psy_path_append_dir(&self->curr, directory);
	fileview_builddirectories(self);
	fileview_build(self);
	fileview_updatepath(self);
	psy_ui_component_align_full(&self->filebox.component);
}

void fileview_ondirfilter(FileView* self, psy_ui_Component* sender)
{
	if (sender == psy_ui_button_base(&self->showall)) {
		self->dirfilter.showall = TRUE;
	}
	fileview_builddirectories(self);
	fileview_build(self);
	psy_ui_component_align_full(&self->filebox.component);
}

void fileview_onhide(FileView* self, psy_ui_Component* sender)
{
	psy_ui_component_hide_align(fileview_base(self));
}

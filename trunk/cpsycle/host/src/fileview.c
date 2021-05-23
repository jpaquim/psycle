/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "fileview.h"
/* file */
#include <dir.h>
// container
#include <qsort.h>
/* platform */
#include "../../detail/os.h"
#include "../../detail/portable.h"

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

/* implementation */
void fileview_init(FileView* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	psy_ui_component_init(fileview_base(self), parent, NULL);
	self->workspace = workspace;	
	psy_ui_tabbar_init(&self->drives, fileview_base(self));
	psy_ui_tabbar_settabalign(&self->drives, psy_ui_ALIGN_TOP);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->drives),
		psy_ui_ALIGN_LEFT);	
	psy_ui_listbox_init(&self->filebox, &self->component);
	psy_ui_listbox_setcharnumber(&self->filebox, 40);
	psy_ui_component_setalign(psy_ui_listbox_base(&self->filebox),
		psy_ui_ALIGN_CLIENT);		
#if defined(DIVERSALIS__OS__MICROSOFT)
	self->curr = strdup("");
	self->drive = strdup("C:\\");
#else	
	self->curr = strdup(""); /* workspace_songs_directory(workspace)); */
	self->drive = strdup("/");
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
}

void fileview_ondestroy(FileView* self, psy_ui_Component* sender)
{
	free(self->curr);
	free(self->drive);
	psy_signal_dispose(&self->signal_selected);
	psy_list_deallocate(&self->files, NULL);
}

void fileview_build(FileView* self)
{	
	char path[4096];
#if defined(DIVERSALIS__OS__UNIX)
	psy_List* sorted;
#endif

	psy_ui_listbox_clear(&self->filebox);
	psy_snprintf(path, 4096, "%s%s", self->drive, self->curr);	
	psy_dir_enumerate(self, path, "*.psy", 0, (psy_fp_findfile)
		fileview_onenumdir);
#if defined(DIVERSALIS__OS__UNIX)
	sorted = fileview_sort(self->files, (psy_fp_comp)fileview_comp_filename);
	psy_list_deallocate(&self->files, (psy_fp_disposefunc)NULL);
	self->files = sorted;
#endif
	if (self->files) {
		psy_List* p;
		
		for (p = self->files; p != NULL; p = p->next) {
			if (p->entry) {
				psy_Path path;

				psy_path_init(&path, (const char*)p->entry);	
				psy_ui_listbox_addtext(&self->filebox,
					psy_path_filename(&path));
				psy_path_dispose(&path);
			}
		}		
	}
}

int fileview_onenumdir(FileView* self, const char* filename, int flag)
{
	if (filename) {			
		psy_list_append(&self->files, psy_strdup(filename));		
	}
	return 1;
}

void fileview_builddirectories(FileView* self)
{
	psy_List* p;
	psy_List* q;
	char path[4096];

	psy_snprintf(path, 4096, "%s%s", self->drive, self->curr);	
	for (q = p = psy_directories(path); p != NULL; psy_list_next(&p)) {
		psy_ui_listbox_addtext(&self->filebox, (char*)psy_list_entry(p));
		psy_snprintf(path, 4096, "%s%s%s%s", self->drive, self->curr,
			psy_SLASHSTR,
			(char*)psy_list_entry(p));
		psy_list_append(&self->files, strdup(path));		
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
		psy_ui_tabbar_append(&self->drives, (char*)psy_list_entry(p));
	}
	psy_list_deallocate(&q, NULL);
}

void fileview_onfileboxselected(FileView* self, psy_ui_ListBox* sender,
	intptr_t index)
{
	if (psy_ui_listbox_cursel(&self->filebox) != -1) {
		if (psy_ui_listbox_cursel(&self->filebox) < self->numdirectories) {
			const char* path = fileview_path(self);
			while (*path != '\0') {
				if (*path == psy_SLASH) {
					++path;
					break;
				}
				++path;
			}
			if (*path != '\0') {
				self->curr = strdup(path);
				self->numdirectories = 0;
				psy_list_deallocate(&self->files, NULL);
				psy_ui_listbox_clear(&self->filebox);
				fileview_builddirectories(self);
				fileview_build(self);		
				psy_ui_component_align_full(&self->filebox.component);		
			}
		} else {
			psy_signal_emit(&self->signal_selected, self, 0);
		}
	}	
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

void fileview_ondrives(FileView* self, psy_ui_TabBar* sender, int index)
{
	psy_ui_Tab* tab;

	tab = psy_ui_tabbar_tab(&self->drives, index);
	if (tab) {
		free(self->drive);
		free(self->curr);

		self->drive = strdup(tab->text);
		self->curr = strdup("");
		self->numdirectories = 0;
		psy_list_deallocate(&self->files, NULL);
		psy_ui_listbox_clear(&self->filebox);
		fileview_builddirectories(self);
		fileview_build(self);
		psy_ui_component_align_full(&self->filebox.component);
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
		arrayptr = malloc(sizeof(char*) * num);
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

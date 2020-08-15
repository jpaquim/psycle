// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "fileview.h"

#include <dir.h>

#include <stdlib.h>
#include <string.h>

#include "../../detail/os.h"
#include "../../detail/portable.h"

// prototypes
static void fileview_ondestroy(FileView*, psy_ui_Component* sender);
static void fileview_build(FileView*);
static void fileview_builddrives(FileView*);
static void fileview_builddirectories(FileView*);
static int fileview_onenumdir(FileView*, const char* path, int flag);
static void fileview_onfileboxselected(FileView*, psy_ui_ListBox* sender, intptr_t index);
static void fileview_ondrives(FileView* self, TabBar* sender, int index);

// implementation
void fileview_init(FileView* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	psy_ui_component_init(fileview_base(self), parent);
	self->workspace = workspace;
	psy_ui_component_enablealign(fileview_base(self));		
	tabbar_init(&self->drives, fileview_base(self));
	psy_ui_component_setalign(tabbar_base(&self->drives), psy_ui_ALIGN_LEFT);
	self->drives.tabalignment = psy_ui_ALIGN_LEFT;
	psy_ui_listbox_init(&self->filebox, &self->component);
	psy_ui_listbox_setcharnumber(&self->filebox, 40);
	psy_ui_component_setalign(psy_ui_listbox_base(&self->filebox),
		psy_ui_ALIGN_CLIENT);		
#if defined(DIVERSALIS__OS__MICROSOFT)
	self->curr = strdup("");
	self->drive = strdup("C:\\");
#else	
	self->curr = strdup(workspace_songs_directory(workspace));
	self->drive = strdup("");
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

	psy_snprintf(path, 4096, "%s%s", self->drive, self->curr);
	psy_dir_enumerate(self, path, "*.psy", 0,
		(psy_fp_findfile)fileview_onenumdir);
}

void fileview_builddirectories(FileView* self)
{
	psy_List* p;
	psy_List* q;
	char path[4096];

	psy_snprintf(path, 4096, "%s%s", self->drive, self->curr);
	for (q = p = psy_directories(path); p != NULL; psy_list_next(&p)) {
		psy_ui_listbox_addtext(&self->filebox, (char*)psy_list_entry(p));
		psy_snprintf(path, 4096, "%s%s%s", self->drive, self->curr,
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
	
	tabbar_clear(&self->drives);
	for (q = p = psy_drives(); p != NULL; psy_list_next(&p)) {		
		tabbar_append(&self->drives, (char*)psy_list_entry(p));
	}
	psy_list_deallocate(&q, NULL);
}

int fileview_onenumdir(FileView* self, const char* filename, int flag)
{
	psy_Path path;

	psy_path_init(&path, filename);
	psy_ui_listbox_addtext(&self->filebox, psy_path_name(&path));
	psy_list_append(&self->files, strdup(filename));
	return 1;
}

void fileview_onfileboxselected(FileView* self, psy_ui_ListBox* sender, intptr_t index)
{
	if (psy_ui_listbox_cursel(&self->filebox) != -1) {
		if (psy_ui_listbox_cursel(&self->filebox) < self->numdirectories) {
			const char* path = fileview_path(self);
			while (*path != '\0') {
				if (*path == '\\') {
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

void fileview_ondrives(FileView* self, TabBar* sender, int index)
{
	Tab* tab;

	tab = tabbar_tab(&self->drives, index);
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
	}
}

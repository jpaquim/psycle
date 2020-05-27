// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "help.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXREADBUFFER 4096

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#endif

static void help_ondestroy(Help*, psy_ui_Component* sender);
static void help_registerfiles(Help*);
static void help_clearfiles(Help*);
static void help_updatefiles(Help*);
static void help_ontabbarchanged(Help*, psy_ui_Component* sender,
	uintptr_t tabindex);
static void help_loadpage(Help*, uintptr_t index);
static void help_load(Help*, const char* path);

void help_init(Help* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_Margin margin;
	psy_ui_Margin tabmargin;

	psy_ui_component_init(help_base(self), parent);	
	self->workspace = workspace;
	psy_ui_component_enablealign(help_base(self));	
	tabbar_init(&self->tabbar, help_base(self));
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_RIGHT);
	self->tabbar.tabalignment = psy_ui_ALIGN_RIGHT;	
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(1), psy_ui_value_makepx(0),
		psy_ui_value_makeew(1.5));
	psy_ui_component_setmargin(tabbar_base(&self->tabbar), &margin);
	psy_ui_margin_init_all(&tabmargin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0),
		psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	tabbar_setdefaulttabmargin(&self->tabbar, &tabmargin);
	psy_ui_editor_init(&self->editor, help_base(self));
	psy_ui_editor_preventedit(&self->editor);
	psy_ui_component_setalign(&self->editor.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&self->tabbar.signal_change, self,
		help_ontabbarchanged);
	psy_table_init(&self->files);
	psy_signal_connect(&self->component.signal_destroy, self,
		help_ondestroy);
	help_registerfiles(self);
	help_loadpage(self, 0);
}

void help_ondestroy(Help* self, psy_ui_Component* sender)
{
	help_clearfiles(self);
	psy_table_dispose(&self->files);
}

void help_registerfiles(Help* self)
{
	uintptr_t page = 0;

	psy_table_insert(&self->files, page++, strdup("./docs/readme.txt"));
	psy_table_insert(&self->files, page++, strdup("./docs/keys.txt"));
	psy_table_insert(&self->files, page++, strdup("./docs/tweaking.txt"));
	psy_table_insert(&self->files, page++, strdup("./docs/whatsnew.txt"));
	psy_table_insert(&self->files, page++, strdup("./docs/luascripting.txt"));
	help_updatefiles(self);
}

void help_clearfiles(Help* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->files);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		free(psy_tableiterator_value(&it));
	}
	psy_table_clear(&self->files);
}

void help_updatefiles(Help* self)
{
	psy_TableIterator it;

	tabbar_clear(&self->tabbar);
	for (it = psy_table_begin(&self->files);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		tabbar_append(&self->tabbar, (char*)psy_tableiterator_value(&it));
	}
}

void help_ontabbarchanged(Help* self, psy_ui_Component* sender,
	uintptr_t tabindex)
{
	help_loadpage(self, tabindex);
}

void help_loadpage(Help* self, uintptr_t index)
{
	char path[_MAX_PATH];

	strcpy(path, workspace_doc_directory(self->workspace));
	strcat(path, "\\");
	switch (index) {
		case 0:
			strcat(path, "readme.txt");
			help_load(self, path);
		break;
		case 1:
			strcat(path, "keys.txt");
			help_load(self, path);
		break;
		case 2:
			strcat(path, "tweaking.txt");
			help_load(self, path);
		break;
		case 3:
			strcat(path, "whatsnew.txt");
			help_load(self, path);
		break;
		case 4:
			strcat(path, "luascripting.txt");
			help_load(self, path);
		break;
		default:
		break;
	}
}

void help_load(Help* self, const char* path)
{
	FILE* fp;

	fp = fopen(path, "rb");
	if (fp) {		
		char c;
		int pos = 0;		
		char text[MAXREADBUFFER];

		psy_ui_editor_clear(&self->editor);
		psy_ui_editor_enableedit(&self->editor);
		memset(text, 0, MAXREADBUFFER);
		while ((c = fgetc(fp)) != EOF) {
			if (pos < MAXREADBUFFER) {
				text[pos] = c;
				++pos;
			} else {
				psy_ui_editor_addtext(&self->editor, text);
				pos = 0;
			}
		}		
		fclose(fp);
		if (pos > 0) {
			psy_ui_editor_addtext(&self->editor, text);
		}
		psy_ui_editor_preventedit(&self->editor);
	}
}

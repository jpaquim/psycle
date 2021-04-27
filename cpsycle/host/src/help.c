// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "help.h"
// file
#include <dir.h>
// std
#include <stdlib.h>
#include <string.h>
// platform
#include "../../detail/portable.h"

// prototypes
static void help_ondestroy(Help*, psy_ui_Component* sender);
static void help_registerfiles(Help*);
static void help_clearfilenames(Help*);
static void help_buildtabs(Help*);
static void help_ontabbarchanged(Help*, psy_ui_Component* sender,
	uintptr_t tabindex);
static void help_loadpage(Help*, uintptr_t index);
static void help_onalign(Help*, psy_ui_Component* sender);
	
// implementation
void help_init(Help* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_Margin margin;
	psy_ui_Margin leftmargin;
	//psy_ui_Margin tabmargin;

	psy_ui_component_init(help_base(self), parent, NULL);
	self->workspace = workspace;
	psy_ui_tabbar_init(&self->tabbar, help_base(self));
	self->lastalign = psy_ui_ALIGN_NONE;
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar), psy_ui_ALIGN_RIGHT);	
	psy_ui_margin_init_em(&margin, 0.0, 1.0, 0.0, 1.5);
	psy_ui_component_setmargin(psy_ui_tabbar_base(&self->tabbar), margin);
	// psy_ui_margin_init_em(&tabmargin, 0.0, 0.0, 0.5, 0.0);		
	// psy_ui_tabbar_setdefaulttabmargin(&self->tabbar, tabmargin);
	psy_ui_margin_init_em(&leftmargin, 0.0, 0.0, 0.0, 3.0);		
	psy_ui_editor_init(&self->editor, help_base(self));
	psy_ui_component_setmargin(&self->editor.component, leftmargin);
	psy_ui_editor_preventedit(&self->editor);
	psy_ui_editor_enablewrap(&self->editor);
	psy_ui_component_setalign(&self->editor.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&self->tabbar.signal_change, self,
		help_ontabbarchanged);
	psy_table_init(&self->filenames);
	psy_signal_connect(&self->component.signal_destroy, self,
		help_ondestroy);
	psy_signal_connect(&self->component.signal_align, self,
		help_onalign);
	help_registerfiles(self);
	help_loadpage(self, 0);
}

void help_ondestroy(Help* self, psy_ui_Component* sender)
{
	psy_table_disposeall(&self->filenames, (psy_fp_disposefunc)NULL);
}

void help_registerfiles(Help* self)
{	
#ifdef PSYCLE_HELPFILES
	char* text;
	char seps[] = " ";
	char* token;
			
	text = psy_strdup(PSYCLE_HELPFILES);
	token = strtok(text, seps);
	while (token != 0) {
		psy_table_insert(&self->filenames, psy_table_size(&self->filenames),
			psy_strdup(token));
		token = strtok(0, seps);
	}
	free(text);
	help_buildtabs(self);
#endif
}

void help_buildtabs(Help* self)
{
	psy_TableIterator it;

	psy_ui_tabbar_clear(&self->tabbar);
	for (it = psy_table_begin(&self->filenames);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_Path path;

		psy_path_init(&path, (char*)psy_tableiterator_value(&it));		
		psy_ui_tabbar_append(&self->tabbar, psy_path_name(&path));
		psy_path_dispose(&path);
	}
}

void help_clearfilenames(Help* self)
{
	psy_table_disposeall(&self->filenames, (psy_fp_disposefunc)NULL);	
	psy_table_init(&self->filenames);
}

void help_ontabbarchanged(Help* self, psy_ui_Component* sender,
	uintptr_t tabindex)
{
	help_loadpage(self, tabindex);
}

void help_loadpage(Help* self, uintptr_t index)
{
	psy_ui_editor_enableedit(&self->editor);
	psy_ui_editor_clear(&self->editor);
	if (psy_table_at(&self->filenames, index) != NULL) {
		psy_Path path;

		psy_path_init(&path, NULL);
		psy_path_setprefix(&path, dirconfig_doc(
			&self->workspace->config.directories));
		psy_path_setname(&path, psy_table_at(&self->filenames, index));
		psy_ui_editor_load(&self->editor, psy_path_full(&path));
		psy_path_dispose(&path);
	}
	psy_ui_editor_preventedit(&self->editor);
}

void help_onalign(Help* self, psy_ui_Component* sender)
{
	if (self->lastalign != psy_ui_component_parent(sender)->align) {
		// psy_ui_Margin tabmargin;

		if (psy_ui_component_parent(sender)->align == psy_ui_ALIGN_RIGHT) {						
			psy_ui_component_setalign(&self->tabbar.component, psy_ui_ALIGN_TOP);
			psy_ui_tabbar_settabalign(&self->tabbar, psy_ui_ALIGN_RIGHT);
		} else {
			psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar), psy_ui_ALIGN_RIGHT);
			psy_ui_tabbar_settabalign(&self->tabbar, psy_ui_ALIGN_TOP);
		}
		psy_ui_component_align(psy_ui_tabbar_base(&self->tabbar));
		self->lastalign = psy_ui_component_parent(sender)->align;
	}
}

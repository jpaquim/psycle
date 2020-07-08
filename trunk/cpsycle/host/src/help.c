// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "help.h"

#include <dir.h>

#include <stdlib.h>
#include <string.h>

static void help_ondestroy(Help*, psy_ui_Component* sender);
static void help_registerfiles(Help*);
static void help_clearfilenames(Help*);
static void help_buildtabs(Help*);
static void help_ontabbarchanged(Help*, psy_ui_Component* sender,
	uintptr_t tabindex);
static void help_loadpage(Help*, uintptr_t index);

void help_init(Help* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_Margin margin;
	psy_ui_Margin leftmargin;
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
	psy_ui_margin_init_all(&leftmargin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0), psy_ui_value_makepx(0),
		psy_ui_value_makeew(3));
	psy_ui_editor_init(&self->editor, help_base(self));
	psy_ui_component_setmargin(&self->editor.component, &leftmargin);
	psy_ui_editor_preventedit(&self->editor);
	psy_ui_editor_enablewrap(&self->editor);
	psy_ui_component_setalign(&self->editor.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&self->tabbar.signal_change, self,
		help_ontabbarchanged);
	psy_table_init(&self->filenames);
	psy_signal_connect(&self->component.signal_destroy, self,
		help_ondestroy);
	help_registerfiles(self);
	help_loadpage(self, 0);
}

void help_ondestroy(Help* self, psy_ui_Component* sender)
{
	psy_table_disposeall(&self->filenames, (psy_fp_disposefunc)NULL);
}

void help_registerfiles(Help* self)
{
	uintptr_t page = 0;

	psy_table_insert(&self->filenames, page++, strdup("readme.txt"));
	psy_table_insert(&self->filenames, page++, strdup("keys.txt"));
	psy_table_insert(&self->filenames, page++, strdup("tweaking.txt"));
	psy_table_insert(&self->filenames, page++, strdup("whatsnew.txt"));
	psy_table_insert(&self->filenames, page++, strdup("luascripting.txt"));
	help_buildtabs(self);
}

void help_buildtabs(Help* self)
{
	psy_TableIterator it;

	tabbar_clear(&self->tabbar);
	for (it = psy_table_begin(&self->filenames);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_Path path;

		psy_path_init(&path, (char*)psy_tableiterator_value(&it));		
		tabbar_append(&self->tabbar, psy_path_name(&path));
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
		psy_path_setprefix(&path, workspace_doc_directory(self->workspace));
		psy_path_setname(&path, psy_table_at(&self->filenames, index));
		psy_ui_editor_load(&self->editor, psy_path_path(&path));
		psy_path_dispose(&path);
	}
	psy_ui_editor_preventedit(&self->editor);
}

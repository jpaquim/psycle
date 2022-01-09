/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "help.h"
/* file */
#include <dir.h>
/* std */
#include <stdlib.h>
#include <string.h>
/* platform */
#include "../../detail/portable.h"

#define BLOCKSIZE 128 * 1024

/* prototypes */
static void help_ondestroy(Help*, psy_ui_Component* sender);
static void help_registerfiles(Help*);
static void help_clearfilenames(Help*);
static void help_buildtabs(Help*);
static void help_ontabbarchanged(Help*, psy_ui_Component* sender,
	uintptr_t tabindex);
static void help_loadpage(Help*, uintptr_t index);
static void help_load(Help*, const char* path);
static void help_onalign(Help*, psy_ui_Component* sender);

 /* implementation  */
void help_init(Help* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_Margin margin;
	psy_ui_Margin leftmargin;

	psy_ui_component_init(help_base(self), parent, NULL);
	self->workspace = workspace;
	psy_ui_tabbar_init(&self->tabbar, help_base(self));
	self->lastalign = psy_ui_ALIGN_NONE;
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar), psy_ui_ALIGN_RIGHT);	
	psy_ui_margin_init_em(&margin, 0.0, 1.0, 0.0, 1.5);
	psy_ui_component_setmargin(psy_ui_tabbar_base(&self->tabbar), margin);	
	psy_ui_margin_init_em(&leftmargin, 0.0, 0.0, 0.0, 3.0);		
	psy_ui_label_init(&self->text, help_base(self));
	psy_ui_component_setmargin(&self->text.component, leftmargin);
	psy_ui_label_enablewrap(&self->text);
	psy_ui_component_setalign(&self->text.component, psy_ui_ALIGN_CLIENT);	
	psy_ui_label_preventtranslation(&self->text);
	psy_ui_label_setcharnumber(&self->text, 120.0);
	psy_ui_label_settextalignment(&self->text, psy_ui_ALIGNMENT_LEFT);
	psy_ui_component_setscrollstep_height(
		psy_ui_label_base(&self->text),
		psy_ui_value_make_eh(1.0));
	psy_ui_component_setwheelscroll(&self->text.component, 4);
	psy_ui_component_setalign(psy_ui_label_base(&self->text),
		psy_ui_ALIGN_FIXED);
	psy_ui_component_setoverflow(&self->text.component,
		psy_ui_OVERFLOW_SCROLL);
	psy_ui_label_enablewrap(&self->text);
	psy_ui_scroller_init(&self->scroller, &self->text.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
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
		psy_ui_tabbar_append(&self->tabbar, psy_path_name(&path),
			psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());
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
	if (psy_table_at(&self->filenames, index) != NULL) {
		psy_Path path;

		psy_path_init(&path, NULL);
		psy_path_setprefix(&path, dirconfig_doc(
			&self->workspace->config.directories));
		psy_path_setname(&path, (const char*)psy_table_at(&self->filenames,
			index));
		help_load(self, psy_path_full(&path));
		psy_path_dispose(&path);
	}
	psy_ui_component_align(&self->scroller.pane);
}

void help_load(Help* self, const char* path)
{
	FILE* fp;	
	
	psy_ui_label_settext(&self->text, "");
	fp = fopen(path, "rb");
	if (fp) {
		char data[BLOCKSIZE];
		uintptr_t lenfile;

		memset(data, 0, BLOCKSIZE);
		lenfile = fread(data, 1, sizeof(data), fp);
		while (lenfile > 0) {
			psy_ui_label_addtext(&self->text, (char*)data);			
			lenfile = fread(data, 1, sizeof(data), fp);
		}
		fclose(fp);
		psy_ui_component_invalidate(psy_ui_label_base(&self->text));
	}	
}

void help_onalign(Help* self, psy_ui_Component* sender)
{
	if (self->lastalign != psy_ui_component_parent(sender)->align) {
		if (psy_ui_component_parent(sender)->align == psy_ui_ALIGN_RIGHT) {
			psy_ui_component_setalign(&self->tabbar.component,
				psy_ui_ALIGN_TOP);
			psy_ui_tabbar_settabalign(&self->tabbar, psy_ui_ALIGN_RIGHT);
		} else {
			psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar),
				psy_ui_ALIGN_RIGHT);
			psy_ui_tabbar_settabalign(&self->tabbar, psy_ui_ALIGN_TOP);
		}
		psy_ui_component_align(psy_ui_tabbar_base(&self->tabbar));
		self->lastalign = psy_ui_component_parent(sender)->align;
	}
}

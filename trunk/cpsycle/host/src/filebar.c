/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "filebar.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* audio */
#include <songio.h>
/* ui */
#include <uiopendialog.h>
#include <uisavedialog.h>

/* prototypes */
static void filebar_onnewsong(FileBar*, psy_ui_Component* sender);
static void filebar_ondiskop(FileBar*, psy_ui_Component* sender);
static void filebar_onloadsong(FileBar*, psy_ui_Component* sender);
static void filebar_onsavesong(FileBar*, psy_ui_Component* sender);

/* implementation */
void filebar_init(FileBar* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_component_init(filebar_base(self), parent, NULL);	
	self->workspace = workspace;
	psy_ui_component_set_style_type(filebar_base(self), STYLE_FILEBAR);
	psy_ui_component_set_defaultalign(filebar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init(&self->recentbutton, filebar_base(self));	
	psy_ui_button_seticon(&self->recentbutton, psy_ui_ICON_MORE);	
	psy_ui_label_init_text(&self->header, filebar_base(self), "file.song");	
	psy_ui_button_init_text_connect(&self->newbutton, filebar_base(self),
		"file.new", self, filebar_onnewsong);
	psy_ui_bitmap_loadresource(&self->newbutton.bitmapicon, IDB_NEW_DARK);
	psy_ui_bitmap_settransparency(&self->newbutton.bitmapicon, psy_ui_colour_make(0x00FFFFFF));	
	psy_ui_button_init_text_connect(&self->diskop, filebar_base(self),
		"file.diskop", self, filebar_ondiskop);	
	psy_ui_button_init_text(&self->loadbutton, filebar_base(self),
		"file.load");
	psy_ui_button_loadresource(&self->loadbutton, IDB_OPEN_LIGHT,
		IDB_OPEN_DARK, psy_ui_colour_white()); 
	psy_signal_connect(&self->loadbutton.signal_clicked, self,
		filebar_onloadsong);
	psy_ui_button_init_text_connect(&self->savebutton, filebar_base(self),
		"file.save", self, filebar_onsavesong);
	psy_ui_button_loadresource(&self->savebutton, IDB_SAVE_LIGHT,
		IDB_SAVE_DARK, psy_ui_colour_white());
	psy_ui_button_init_text(&self->exportbutton, filebar_base(self),
		"file.export");
	psy_ui_button_loadresource(&self->exportbutton, IDB_EARTH_LIGHT,
		IDB_EARTH_DARK, psy_ui_colour_white());
	psy_ui_button_init_text(&self->renderbutton, filebar_base(self),
		"file.render");	
	psy_ui_button_loadresource(&self->renderbutton, IDB_PULSE_LIGHT,
		IDB_PULSE_DARK, psy_ui_colour_white());
	if (keyboardmiscconfig_ft2fileexplorer(psycleconfig_misc(
		workspace_conf(workspace)))) {
		filebar_useft2fileexplorer(self);
	} else {
		filebar_usenativefileexplorer(self);
	}
}

void filebar_useft2fileexplorer(FileBar* self)
{
	psy_ui_Component* alignroot;

	psy_ui_component_show(&self->diskop.component);
	psy_ui_component_hide(&self->loadbutton.component);
	psy_ui_component_hide(&self->savebutton.component);
	alignroot = psy_ui_component_parent(&self->component);
	if (alignroot) {
		psy_ui_component_align(alignroot);
	}
}

void filebar_usenativefileexplorer(FileBar* self)
{
	psy_ui_Component* alignroot;
	
	psy_ui_component_hide(&self->diskop.component);
	psy_ui_component_show(&self->loadbutton.component);
	psy_ui_component_show(&self->savebutton.component);	
	alignroot = psy_ui_component_parent(&self->component);
	if (alignroot) {
		psy_ui_component_align(alignroot);
	}
}

void filebar_onnewsong(FileBar* self, psy_ui_Component* sender)
{
	if (keyboardmiscconfig_savereminder(&self->workspace->config.misc) &&
			workspace_songmodified(self->workspace)) {
		workspace_select_view(self->workspace, VIEW_ID_CHECKUNSAVED, 0,
			CONFIRM_NEW);
	} else {
		workspace_newsong(self->workspace);
	}
}

void filebar_ondiskop(FileBar* self, psy_ui_Component* sender)
{

}

void filebar_onloadsong(FileBar* self, psy_ui_Component* sender)
{	
	if (!keyboardmiscconfig_ft2fileexplorer(psycleconfig_misc(
		workspace_conf(self->workspace)))) {
		if (keyboardmiscconfig_savereminder(&self->workspace->config.misc) &&
			workspace_songmodified(self->workspace)) {
			workspace_select_view(self->workspace, VIEW_ID_CHECKUNSAVED, 0,
				CONFIRM_LOAD);
		} else {
			workspace_loadsong_fileselect(self->workspace);
		}
	}
}

void filebar_onsavesong(FileBar* self, psy_ui_Component* sender)
{		
	workspace_savesong_fileselect(self->workspace);
}

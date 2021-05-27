// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "filebar.h"
// host
#include "resources/resource.h"
// audio
#include <songio.h>
// ui
#include <uiopendialog.h>
#include <uisavedialog.h>

// prototypes
static void filebar_onnewsong(FileBar*, psy_ui_Component* sender);
static void filebar_onloadsong(FileBar*, psy_ui_Component* sender);
static void filebar_onsavesong(FileBar*, psy_ui_Component* sender);
// static void filebar_onexportsong(FileBar*, psy_ui_Component* sender);
// implementation
void filebar_init(FileBar* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_component_init(filebar_base(self), parent, NULL);
	self->workspace = workspace;	
	psy_ui_component_setdefaultalign(filebar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init(&self->recentbutton, filebar_base(self), NULL);
	psy_ui_button_seticon(&self->recentbutton, psy_ui_ICON_MORE);
	psy_ui_label_init_text(&self->header, filebar_base(self), NULL,
		"file.song");	
	psy_ui_button_init_text_connect(&self->newbutton, filebar_base(self), NULL,
		"file.new", self, filebar_onnewsong);
	psy_ui_bitmap_loadresource(&self->newbutton.bitmapicon, IDB_NEW_DARK);
	psy_ui_bitmap_settransparency(&self->newbutton.bitmapicon, psy_ui_colour_make(0x00FFFFFF));
	psy_ui_button_init_text(&self->loadbutton, filebar_base(self), NULL,
		"file.load");
	psy_ui_bitmap_loadresource(&self->loadbutton.bitmapicon, IDB_OPEN_DARK);
	psy_ui_bitmap_settransparency(&self->loadbutton.bitmapicon, psy_ui_colour_make(0x00FFFFFF));
	psy_signal_connect(&self->loadbutton.signal_clicked, self,
		filebar_onloadsong);
	psy_ui_button_init_text_connect(&self->savebutton, filebar_base(self), NULL,
		"file.save", self, filebar_onsavesong);
	psy_ui_bitmap_loadresource(&self->savebutton.bitmapicon, IDB_SAVE_DARK);
	psy_ui_bitmap_settransparency(&self->savebutton.bitmapicon, psy_ui_colour_make(0x00FFFFFF));
	psy_ui_button_init_text(&self->exportbutton, filebar_base(self), NULL,
		"file.export");
	psy_ui_bitmap_loadresource(&self->exportbutton.bitmapicon, IDB_EARTH_DARK);
	psy_ui_bitmap_settransparency(&self->exportbutton.bitmapicon, psy_ui_colour_make(0x00FFFFFF));
	psy_ui_button_init_text(&self->renderbutton, filebar_base(self), NULL,
		"file.render");	
	psy_ui_bitmap_loadresource(&self->renderbutton.bitmapicon, IDB_PULSE_DARK);
	psy_ui_bitmap_settransparency(&self->renderbutton.bitmapicon, psy_ui_colour_make(0x00FFFFFF));	
}

void filebar_useft2fileexplorer(FileBar* self)
{
	psy_signal_disconnect(&self->loadbutton.signal_clicked, self,
		filebar_onloadsong);
}

void filebar_usenativefileexplorer(FileBar* self)
{
	psy_signal_connect(&self->loadbutton.signal_clicked, self,
		filebar_onloadsong);
}

void filebar_onnewsong(FileBar* self, psy_ui_Component* sender)
{
	if (keyboardmiscconfig_savereminder(&self->workspace->config.misc) &&
			workspace_songmodified(self->workspace)) {
		workspace_selectview(self->workspace, VIEW_ID_CHECKUNSAVED, 0, CONFIRM_NEW);
	} else {
		workspace_newsong(self->workspace);
	}
}

void filebar_onloadsong(FileBar* self, psy_ui_Component* sender)
{	
	if (!keyboardmiscconfig_ft2fileexplorer(psycleconfig_misc(
		workspace_conf(self->workspace)))) {
		if (keyboardmiscconfig_savereminder(&self->workspace->config.misc) &&
			workspace_songmodified(self->workspace)) {
			workspace_selectview(self->workspace, VIEW_ID_CHECKUNSAVED, 0, CONFIRM_LOAD);
		} else {
			workspace_loadsong_fileselect(self->workspace);
		}
	}
}

void filebar_onsavesong(FileBar* self, psy_ui_Component* sender)
{		
	workspace_savesong_fileselect(self->workspace);
}

// void filebar_onexportsong(FileBar* self, psy_ui_Component* sender)
// {
// 	workspace_exportsong(self->workspace);
// }


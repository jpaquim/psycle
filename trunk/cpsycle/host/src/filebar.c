// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "filebar.h"
// audio
#include <songio.h>
// ui
#include <uiopendialog.h>
#include <uisavedialog.h>

// prototypes
static void filebar_onnewsong(FileBar*, psy_ui_Component* sender);
static void filebar_onloadsong(FileBar*, psy_ui_Component* sender);
static void filebar_onsavesong(FileBar*, psy_ui_Component* sender);
// implementation
void filebar_init(FileBar* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_component_init(filebar_base(self), parent);
	self->workspace = workspace;
	psy_ui_component_setalignexpand(filebar_base(self),
		psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setdefaultalign(filebar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init(&self->recentbutton, filebar_base(self));
	psy_ui_button_seticon(&self->recentbutton, psy_ui_ICON_MORE);
	psy_ui_label_init_text(&self->header, filebar_base(self),
		"file.song");
	psy_ui_button_init_connect(&self->newbutton, filebar_base(self),
		self, filebar_onnewsong);
	psy_ui_button_settext(&self->newbutton, "file.new");
	psy_ui_button_init(&self->loadbutton, filebar_base(self));
	psy_ui_button_settext(&self->loadbutton, "file.load");
#ifdef PSYCLE_USE_PLATFORM_FILEOPEN
	psy_signal_connect(&self->loadbutton.signal_clicked, self,
		filebar_onloadsong);
#endif
	psy_ui_button_init_connect(&self->savebutton, filebar_base(self),
		self, filebar_onsavesong);
	psy_ui_button_settext(&self->savebutton, "file.save");
	psy_ui_button_init(&self->renderbutton, filebar_base(self));
	psy_ui_button_settext(&self->renderbutton, "file.render");	
}

void filebar_onnewsong(FileBar* self, psy_ui_Component* sender)
{
	if (keyboardandmisc_savereminder(&self->workspace->config.misc) &&
			workspace_songmodified(self->workspace)) {
		workspace_selectview(self->workspace, VIEW_ID_CHECKUNSAVED, 0, CHECKUNSAVE_NEW);
	} else {
		workspace_newsong(self->workspace);
	}
}

void filebar_onloadsong(FileBar* self, psy_ui_Component* sender)
{	
	if (keyboardandmisc_savereminder(&self->workspace->config.misc) &&
			workspace_songmodified(self->workspace)) {
		workspace_selectview(self->workspace, VIEW_ID_CHECKUNSAVED, 0, CHECKUNSAVE_LOAD);
	} else {
		workspace_loadsong_fileselect(self->workspace);
	}
}

void filebar_onsavesong(FileBar* self, psy_ui_Component* sender)
{		
	workspace_savesong_fileselect(self->workspace);
}

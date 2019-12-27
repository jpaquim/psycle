// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "filebar.h"

static void filebar_initalign(FileBar*);
static void filebar_onnewsong(FileBar*, psy_ui_Component* sender);
static void filebar_onloadsong(FileBar*, psy_ui_Component* sender);
static void filebar_onsavesong(FileBar*, psy_ui_Component* sender);
static void filebar_onrendersong(FileBar*, psy_ui_Component* sender);

void filebar_init(FileBar* self, psy_ui_Component* parent, Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	ui_label_init(&self->header, &self->component);
	ui_label_settext(&self->header,	"Song  ");
	ui_button_init(&self->newbutton, &self->component);
	ui_button_settext(&self->newbutton,
		workspace_translate(workspace, "new"));
	psy_signal_connect(&self->newbutton.signal_clicked, self,
		filebar_onnewsong);
	ui_button_init(&self->loadbutton, &self->component);
	ui_button_settext(&self->loadbutton,
		workspace_translate(workspace, "load"));
	psy_signal_connect(&self->loadbutton.signal_clicked, self,
		filebar_onloadsong);
	ui_button_init(&self->savebutton, &self->component);
	ui_button_settext(&self->savebutton, 
		workspace_translate(workspace, "save"));
	psy_signal_connect(&self->savebutton.signal_clicked, self,
		filebar_onsavesong);
	ui_button_init(&self->renderbutton, &self->component);
	ui_button_settext(&self->renderbutton, 
		workspace_translate(workspace, "Render"));
	psy_signal_connect(&self->renderbutton.signal_clicked, self,
		filebar_onrendersong);
	filebar_initalign(self);
}

void filebar_initalign(FileBar* self)
{	
	ui_margin margin = { 0, 5, 0, 0 };

	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT,
		&margin));
}

void filebar_onnewsong(FileBar* self, psy_ui_Component* sender)
{
	workspace_newsong(self->workspace);
	// todo update statustbar with song title 
}

void filebar_onloadsong(FileBar* self, psy_ui_Component* sender)
{
	char path[MAX_PATH]	 = "";
	char title[MAX_PATH]	 = ""; 					
	static char filter[] = "All Songs (*.psy *.xm *.it *.s3m *.mod *.wav)" "\0*.psy;*.xm;*.it;*.s3m;*.mod;*.wav\0"
				"Songs (*.psy)"				        "\0*.psy\0"
				"FastTracker II Songs (*.xm)"       "\0*.xm\0"
				"Impulse Tracker Songs (*.it)"      "\0*.it\0"
				"Scream Tracker Songs (*.s3m)"      "\0*.s3m\0"
				"Original Mod Format Songs (*.mod)" "\0*.mod\0"
				"Wav Format Songs (*.wav)"			"\0*.wav\0";
	char  defaultextension[] = "PSY";	

	int showsonginfo = 0;	
	*path = '\0'; 
	if (ui_openfile(&self->component, title, filter, defaultextension, 
			workspace_songs_directory(self->workspace),
			path)) {
		workspace_loadsong(self->workspace, path);						
	}
}

void filebar_onsavesong(FileBar* self, psy_ui_Component* sender)
{
	char path[MAX_PATH]	 = "";
	char title[MAX_PATH]	 = ""; 					
	static char filter[] = "Songs (*.psy)\0*.psy\0";
	char  defaultextension[] = "PSY";
	int showsonginfo = 0;	
	*path = '\0'; 
	if (ui_savefile(&self->component, title, filter, defaultextension, 
			workspace_songs_directory(self->workspace), path)) {
		workspace_savesong(self->workspace, path);
	}
}

void filebar_onrendersong(FileBar* self, psy_ui_Component* sender)
{

}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "filebar.h"

static void filebar_initalign(FileBar*);
static void filebar_onnewsong(FileBar*, psy_ui_Component* sender);
static void filebar_onloadsong(FileBar*, psy_ui_Component* sender);
static void filebar_onsavesong(FileBar*, psy_ui_Component* sender);

void filebar_init(FileBar* self, psy_ui_Component* parent, Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(filebar_base(self), parent);	
	ui_component_enablealign(filebar_base(self));
	ui_component_setalignexpand(filebar_base(self), psy_ui_HORIZONTALEXPAND);
	psy_ui_label_init(&self->header, filebar_base(self));
	psy_ui_label_settext(&self->header,	"Song  ");
	psy_ui_button_init(&self->newbutton, filebar_base(self));
	psy_ui_button_settext(&self->newbutton,
		workspace_translate(workspace, "new"));
	psy_signal_connect(&self->newbutton.signal_clicked, self,
		filebar_onnewsong);
	psy_ui_button_init(&self->loadbutton, filebar_base(self));
	psy_ui_button_settext(&self->loadbutton,
		workspace_translate(workspace, "load"));
	psy_signal_connect(&self->loadbutton.signal_clicked, self,
		filebar_onloadsong);
	psy_ui_button_init(&self->savebutton, filebar_base(self));
	psy_ui_button_settext(&self->savebutton, 
		workspace_translate(workspace, "save"));
	psy_signal_connect(&self->savebutton.signal_clicked, self,
		filebar_onsavesong);
	psy_ui_button_init(&self->renderbutton, filebar_base(self));
	psy_ui_button_settext(&self->renderbutton, 
		workspace_translate(workspace, "Render"));	
	filebar_initalign(self);
}

void filebar_initalign(FileBar* self)
{	
	psy_ui_Margin margin;

	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_list_free(ui_components_setalign(
		ui_component_children(filebar_base(self), 0),
		psy_ui_ALIGN_LEFT,
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
	if (ui_openfile(filebar_base(self), title, filter, defaultextension, 
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
	if (ui_savefile(filebar_base(self), title, filter, defaultextension, 
			workspace_songs_directory(self->workspace), path)) {
		workspace_savesong(self->workspace, path);
	}
}

psy_ui_Component* filebar_base(FileBar* self)
{
	return &self->component;
}

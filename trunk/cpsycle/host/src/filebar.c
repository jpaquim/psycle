// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "filebar.h"

static void OnLoadSong(FileBar*, ui_component* sender);
static void OnNewSong(FileBar*, ui_component* sender);

void InitFileBar(FileBar* self, ui_component* parent, Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);		
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	ui_button_init(&self->newsongbutton, &self->component);
	ui_button_settext(&self->newsongbutton, "New Song");	
	signal_connect(&self->newsongbutton.signal_clicked, self, OnNewSong);
	ui_button_init(&self->loadsongbutton, &self->component);
	ui_button_settext(&self->loadsongbutton, "Load Song");	
	signal_connect(&self->loadsongbutton.signal_clicked, self, OnLoadSong);	
	{
		List* children;
		ui_margin margin = { 3, 10, 0, 3 };
		
		children = ui_component_children(&self->component, 0);
		ui_components_setalign(children, UI_ALIGN_LEFT);
		ui_components_setmargin(children, &margin);
	}
}

void OnNewSong(FileBar* self, ui_component* sender)
{
	workspace_newsong(self->workspace);
	if (self->workspace->song->properties) {
		Properties* title;
		title = properties_find(self->workspace->song->properties, "title");
		if (title) {
			char* titlestr = 0;
			properties_readstring(title, "title", &titlestr, "Untitled");			
			// ui_statusbar_settext(&self->statusbar, 0, titlestr);
		}
	}	
}

void OnLoadSong(FileBar* self, ui_component* sender)
{
	char path[MAX_PATH]	 = "";
	char title[MAX_PATH]	 = ""; 					
	static char filter[] = "All Songs (*.psy *.xm *.it *.s3m *.mod)" "\0*.psy;*.xm;*.it;*.s3m;*.mod\0"
				"Songs (*.psy)"				        "\0*.psy\0"
				"FastTracker II Songs (*.xm)"       "\0*.xm\0"
				"Impulse Tracker Songs (*.it)"      "\0*.it\0"
				"Scream Tracker Songs (*.s3m)"      "\0*.s3m\0"
				"Original Mod Format Songs (*.mod)" "\0*.mod\0";
	char  defaultextension[] = "PSY";
	int showsonginfo = 0;	
	*path = '\0'; 
	if (ui_openfile(&self->component, title, filter, defaultextension, path)) {		
		workspace_loadsong(self->workspace, path);						
	}
}

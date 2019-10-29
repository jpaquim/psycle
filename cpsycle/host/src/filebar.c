// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "filebar.h"

static void filebar_initalign(FileBar*);
static void filebar_onnewsong(FileBar*, ui_component* sender);
static void filebar_onloadsong(FileBar*, ui_component* sender);
static void filebar_onsavesong(FileBar*, ui_component* sender);

void filebar_init(FileBar* self, ui_component* parent, Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	ui_label_init(&self->header, &self->component);
	ui_label_settext(&self->header,	"Song  ");
	ui_button_init(&self->newsongbutton, &self->component);
	ui_button_settext(&self->newsongbutton,
		workspace_translate(workspace, "new"));
	signal_connect(&self->newsongbutton.signal_clicked, self,
		filebar_onnewsong);
	ui_button_init(&self->loadsongbutton, &self->component);
	ui_button_settext(&self->loadsongbutton,
		workspace_translate(workspace, "load"));
	signal_connect(&self->loadsongbutton.signal_clicked, self,
		filebar_onloadsong);
	ui_button_init(&self->savesongbutton, &self->component);
	ui_button_settext(&self->savesongbutton, 
		workspace_translate(workspace, "save"));	
	signal_connect(&self->savesongbutton.signal_clicked, self,
		filebar_onloadsong);
	filebar_initalign(self);
}

void filebar_initalign(FileBar* self)
{	
	ui_margin margin = { 0, 5, 0, 0 };

	list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT,
		&margin));
}

void filebar_onnewsong(FileBar* self, ui_component* sender)
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

void filebar_onloadsong(FileBar* self, ui_component* sender)
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
		ui_invalidate(self->workspace->mainhandle);
		UpdateWindow(self->workspace->mainhandle->hwnd);
		workspace_loadsong(self->workspace, path);						
	}
}

void filebar_onsavesong(FileBar* self, ui_component* sender)
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
		ui_invalidate(self->workspace->mainhandle);
		UpdateWindow(self->workspace->mainhandle->hwnd);
		workspace_loadsong(self->workspace, path);						
	}
}

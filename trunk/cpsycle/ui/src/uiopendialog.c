// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiopendialog.h"
#include "uiapp.h"
#include "../../detail/portable.h"

extern psy_ui_App app;

void psy_ui_opendialog_init(psy_ui_OpenDialog* self, psy_ui_Component* parent)
{
	self->parent = parent;
	self->title = strdup("");
	self->filter = strdup("");
	self->defaultextension = strdup("");
	self->initialdir = strdup("");
	self->filename = strdup("");	
}

void psy_ui_opendialog_init_all(psy_ui_OpenDialog* self,
	psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	self->parent = parent;
	self->title = strdup(title);
	self->filter = strdup(filter);	
	self->defaultextension = strdup(defaultextension);
	self->initialdir = strdup(initialdir);
	self->filename = strdup("");
}

void psy_ui_opendialog_dispose(psy_ui_OpenDialog* self)
{
	free(self->title);
	free(self->filter);
	free(self->defaultextension);
	free(self->initialdir);
	free(self->filename);
}

int psy_ui_opendialog_execute(psy_ui_OpenDialog* self)
{
	int rv;
	OPENFILENAME ofn;
	char filename[MAX_PATH];
	char filter[MAX_PATH];
	char title[MAX_PATH];
	char initialdir[MAX_PATH];
	char defextension[MAX_PATH];
	char* filtertoken;
				
	*filename = '\0';	
	psy_snprintf(title, MAX_PATH, "%s", self->title);
	psy_snprintf(initialdir, MAX_PATH, "%s", self->initialdir);
	psy_snprintf(defextension, MAX_PATH, "%s", self->defaultextension);
	psy_snprintf(filter, MAX_PATH - 1, "%s", self->filter);
	filter[strlen(filter) + 1] = '\0';
	filtertoken = strtok(filter, "|");
	while (filtertoken != 0) {
		filtertoken = strtok(0, "|");
	}
	ZeroMemory(&ofn, sizeof(OPENFILENAME));	 
	ofn.lStructSize= sizeof(OPENFILENAME); 
	ofn.hwndOwner= self->parent ? (HWND) self->parent->platform->hwnd : (HWND) 0; 
	ofn.lpstrFilter = (LPSTR) filter;
	ofn.lpstrCustomFilter= (LPSTR) NULL;
	ofn.nMaxCustFilter= 0L; 
	ofn.nFilterIndex= 1L; 
	ofn.lpstrFile= (LPSTR) filename; 
	ofn.nMaxFile= MAX_PATH; 
	ofn.lpstrFileTitle = (LPSTR) NULL;
	ofn.nMaxFileTitle= MAX_PATH; 
	ofn.lpstrTitle= strlen(title) ? (LPSTR) title : (LPSTR) NULL;
	ofn.lpstrInitialDir= (LPSTR) initialdir;
	ofn.Flags= OFN_HIDEREADONLY|OFN_FILEMUSTEXIST; 
	ofn.nFileOffset= 0; 
	ofn.nFileExtension= 0; 
	ofn.lpstrDefExt= (LPSTR) defextension;
	rv = GetOpenFileName(&ofn);
	if (app.main) {
		InvalidateRect((HWND) app.main->platform->hwnd, 0, FALSE);
		UpdateWindow((HWND) app.main->platform->hwnd);
	}
	if (rv) {
		free(self->filename);
		self->filename = strdup(filename);
	}
	return rv;
}

const char* psy_ui_opendialog_filename(psy_ui_OpenDialog* self)
{
	return self->filename;
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwinopendialogimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "uiapp.h"
#include <stdlib.h>
#include "uiapp.h"
#include <shlobj.h>
#include "uiwincomponentimp.h"
#include <commdlg.h>
#include <stdlib.h>
#include "../../detail/portable.h"

extern psy_ui_App app;

// VTable Prototypes
static void dev_dispose(psy_ui_win_OpenDialogImp*);
static int dev_execute(psy_ui_win_OpenDialogImp*);
static const psy_Path* dev_path(const psy_ui_win_OpenDialogImp*);

// VTable init
static psy_ui_OpenDialogImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_win_OpenDialogImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose = (psy_ui_fp_opendialogimp_dev_dispose)dev_dispose;
		imp_vtable.dev_execute = (psy_ui_fp_opendialogimp_dev_execute)dev_execute;
		imp_vtable.dev_path = (psy_ui_fp_opendialogimp_dev_path)dev_path;
		imp_vtable_initialized = TRUE;
	}
}

void psy_ui_win_opendialogimp_init(psy_ui_win_OpenDialogImp* self, psy_ui_Component* parent)
{
	psy_ui_opendialogimp_init(&self->imp);
	imp_vtable_init(self);	
	self->imp.vtable = &imp_vtable;	
	self->parent = parent;
	self->title = strdup("");
	self->filter = strdup("");
	self->defaultextension = strdup("");
	self->initialdir = strdup("");	
	psy_path_init(&self->path, NULL);
}

void psy_ui_win_opendialogimp_init_all(psy_ui_win_OpenDialogImp* self,
	psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	psy_ui_opendialogimp_init(&self->imp);
	imp_vtable_init(self);
	self->imp.vtable = &imp_vtable;
	self->parent = parent;
	self->title = strdup(title);
	self->filter = strdup(filter);
	self->defaultextension = strdup(defaultextension);
	self->initialdir = strdup(initialdir);
	psy_path_init(&self->path, NULL);
}

// win32 implementation method for psy_ui_OpenDialog
void dev_dispose(psy_ui_win_OpenDialogImp* self)
{
	free(self->title);
	free(self->filter);
	free(self->defaultextension);
	free(self->initialdir);
	psy_path_dispose(&self->path);
}

int dev_execute(psy_ui_win_OpenDialogImp* self)
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
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = 0; //  self->parent ? (HWND)psy_ui_win_component_details(self->parent)->hwnd : (HWND)0;
	ofn.lpstrFilter = (LPSTR)filter;
	ofn.lpstrCustomFilter = (LPSTR)NULL;
	ofn.nMaxCustFilter = 0L;
	ofn.nFilterIndex = 1L;
	ofn.lpstrFile = (LPSTR)filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = (LPSTR)NULL;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrTitle = strlen(title) ? (LPSTR)title : (LPSTR)NULL;
	ofn.lpstrInitialDir = (LPSTR)initialdir;
	ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = (LPSTR)defextension;
	rv = GetOpenFileName(&ofn);
	if (app.main) {
		//InvalidateRect((HWND)psy_ui_win_component_details(app.main)->hwnd, 0, FALSE);
		//UpdateWindow((HWND)psy_ui_win_component_details(app.main)->hwnd);
	}
	if (rv) {
		psy_path_setpath(&self->path, filename);		
	}
	return rv;
}

const psy_Path* dev_path(const psy_ui_win_OpenDialogImp* self)
{
	return &self->path;
}

#endif

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uifolderdialog.h"
#include "uiapp.h"
#include <shlobj.h>
#include "uiwincomponentimp.h"
#include <commdlg.h>
#include <stdlib.h>
#include "../../detail/portable.h"

extern psy_ui_App app;

static psy_ui_win_ComponentImp* psy_ui_win_component_details(psy_ui_Component* self)
{
	return (psy_ui_win_ComponentImp*)self->imp;
}

void psy_ui_folderdialog_init(psy_ui_FolderDialog* self, psy_ui_Component* parent)
{
	self->parent = parent;
	self->title = strdup("");	
	self->initialdir = strdup("");
	self->path = strdup("");	
}

void psy_ui_folderdialog_init_all(psy_ui_FolderDialog* self,
	psy_ui_Component* parent,
	const char* title,
	const char* initialdir)
{
	self->parent = parent;
	self->title = strdup(title);		
	self->initialdir = strdup(initialdir);
	self->path = strdup("");
}

void psy_ui_folderdialog_dispose(psy_ui_FolderDialog* self)
{
	free(self->title);	
	free(self->initialdir);
	free(self->path);
	self->title = 0;
	self->initialdir = 0;
	self->path = 0;
}

int psy_ui_folderdialog_execute(psy_ui_FolderDialog* self)
{	
	///\todo: alternate browser window for Vista/7: http://msdn.microsoft.com/en-us/library/bb775966%28v=VS.85%29.aspx
	// SHCreateItemFromParsingName(	
	char title[MAX_PATH];
	// char initialdir[MAX_PATH];		
	int val = 0;
	LPMALLOC pMalloc;

	psy_snprintf(title, MAX_PATH, "%s", self->title);	
	// Gets the Shell's default allocator		
	if (SHGetMalloc(&pMalloc) == NOERROR)
	{
		char pszBuffer[MAX_PATH];
		BROWSEINFO bi;
		LPITEMIDLIST pidl;

		pszBuffer[0] = '\0';		
		// Get help on BROWSEINFO struct - it's got all the bit settings.
		//
		bi.hwndOwner = (HWND)0; // psy_ui_win_component_details(self)->hwnd;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = pszBuffer;
		bi.lpszTitle = title;
#if defined _MSC_VER > 1200
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
#else
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
#endif
		bi.lpfn = NULL;
		bi.lParam = 0;
		// This next call issues the dialog box.
		//
		if ((pidl = SHBrowseForFolder(&bi)) != NULL) {
			if (SHGetPathFromIDList(pidl, pszBuffer)) {
				// At this point pszBuffer contains the selected path
				//
				val = 1;
				free(self->path);
				self->path = strdup(pszBuffer);				
			}
			// Free the PIDL allocated by SHBrowseForFolder.
			//
			pMalloc->lpVtbl->Free(pMalloc, pidl);
		}
		// Release the shell's allocator.
		//
		pMalloc->lpVtbl->Release(pMalloc);
	}
	return val;	
}

const char* psy_ui_folderdialog_path(psy_ui_FolderDialog* self)
{
	return self->path;
}

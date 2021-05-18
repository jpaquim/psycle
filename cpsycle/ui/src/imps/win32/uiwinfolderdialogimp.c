/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiwinfolderdialogimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "../../uiapp.h"
#include <stdlib.h>
#include <shlobj.h>
#include "uiwincomponentimp.h"
#include <commdlg.h>
#include "../../detail/portable.h"

/* prototypes */
static void dev_dispose(psy_ui_win_FolderDialogImp*);
static int dev_execute(psy_ui_win_FolderDialogImp*);
static const char* dev_path(psy_ui_win_FolderDialogImp*);
/* vtable */
static psy_ui_FolderDialogImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(psy_ui_win_FolderDialogImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose = (psy_ui_fp_folderdialogimp_dev_dispose) dev_dispose;
		imp_vtable.dev_execute = (psy_ui_fp_folderdialogimp_dev_execute) dev_execute;
		imp_vtable.dev_path = (psy_ui_fp_folderdialogimp_dev_path) dev_path;
		imp_vtable_initialized = 1;
	}
}
/* implementation */
void psy_ui_win_folderdialogimp_init(psy_ui_win_FolderDialogImp* self)
{
	psy_ui_folderdialogimp_init(&self->imp);
	imp_vtable_init(self);	
	self->imp.vtable = &imp_vtable;	
}

void psy_ui_win_folderdialogimp_init_all(psy_ui_win_FolderDialogImp* self,
	psy_ui_Component* parent,
	const char* title,
	const char* initialdir)
{
	psy_ui_folderdialogimp_init(&self->imp);
	imp_vtable_init(self);
	self->imp.vtable = &imp_vtable;
	self->parent = parent;
	self->title = strdup(title ? title : "");
	self->initialdir = strdup(initialdir ? initialdir : "");
	self->path = strdup("");
}

void dev_dispose(psy_ui_win_FolderDialogImp* self)
{
	free(self->title);
	free(self->initialdir);
	free(self->path);
	self->title = NULL;
	self->initialdir = NULL;
	self->path = NULL;
}

int dev_execute(psy_ui_win_FolderDialogImp* self)
{
	/*
	** \todo: alternate browser window for Vista/7: http://msdn.microsoft.com/en-us/library/bb775966%28v=VS.85%29.aspx
	** SHCreateItemFromParsingName
	*/
	char title[MAX_PATH];
	/* char initialdir[MAX_PATH]; */
	int val = 0;
	LPMALLOC pMalloc;

	psy_snprintf(title, MAX_PATH, "%s", self->title);
	/* Gets the Shell's default allocator */
	if (SHGetMalloc(&pMalloc) == NOERROR)
	{
		char pszBuffer[MAX_PATH];
		BROWSEINFO bi;
		LPITEMIDLIST pidl;

		pszBuffer[0] = '\0';
		/*
		** Get help on BROWSEINFO struct - it's got all the bit settings.
		*/
		bi.hwndOwner = (HWND)0; /* psy_ui_win_component_details(self)->hwnd; */
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
		/*
		** This next call issues the dialog box.
		*/
		if ((pidl = SHBrowseForFolder(&bi)) != NULL) {
			if (SHGetPathFromIDList(pidl, pszBuffer)) {
				/*
				** At this point pszBuffer contains the selected path
				*/
				val = 1;
				free(self->path);
				self->path = strdup(pszBuffer);
			}
			/*
			** Free the PIDL allocated by SHBrowseForFolder.
			*/
			pMalloc->lpVtbl->Free(pMalloc, pidl);
		}
		/*
		** Release the shell's allocator.
		*/
		pMalloc->lpVtbl->Release(pMalloc);
	}
	return val;
}

const char* dev_path(psy_ui_win_FolderDialogImp* self)
{
	return self->path;
}

#endif /* PSYCLE_TK_WIN32 */

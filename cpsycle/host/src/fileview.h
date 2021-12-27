/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(FILEVIEW_H)
#define FILEVIEW_H

/* ui */
#include <uibutton.h>
#include <uicheckbox.h>
#include <uilabel.h>
#include "uilistbox.h"
#include <uitabbar.h>
#include <uitextinput.h>
/* file */
#include <dir.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FileView
**
** ft2style fileexplorer
*/

typedef enum {
	FILEVIEWFILTER_ALL,
	FILEVIEWFILTER_PSY,
	FILEVIEWFILTER_MOD
} FileViewFilterType;

typedef struct FileViewFilter {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_changed;
	/* internal */
	psy_ui_Label desc;
	psy_ui_CheckBox psy;
	psy_ui_CheckBox mod;
	FileViewFilterType filter;
	bool showall;
} FileViewFilter;

void fileviewfilter_init(FileViewFilter*, psy_ui_Component* parent);

INLINE psy_ui_Component* fileviewfilter_base(FileViewFilter* self)
{
	return &self->component;
}

typedef struct FileViewSaveFilter {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label desc;
	psy_ui_CheckBox psy;
	FileViewFilterType filter;
} FileViewSaveFilter;

void fileviewsavefilter_init(FileViewSaveFilter*, psy_ui_Component* parent);

INLINE psy_ui_Component* fileviewsavefilter_base(FileViewSaveFilter* self)
{
	return &self->component;

}

typedef struct FileView {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_selected;
	/* internal */
	psy_ui_ListBox filebox;	
	psy_ui_Component filters;
	FileViewFilter dirfilter;
	FileViewSaveFilter savefilter;
	psy_ui_Component buttons;
	psy_ui_Button save;
	psy_ui_Button showall;
	psy_ui_Button refresh;
	psy_ui_Button exit;
	psy_ui_TabBar drives;
	psy_ui_Component bottom;	
	psy_ui_Label dir;
	psy_ui_Component filebar;
	psy_ui_Label filedesc;
	psy_ui_TextInput filename;
	psy_Table filenames;
	psy_Path curr;	
	psy_List* files;	
	intptr_t numdirectories;
} FileView;

void fileview_init(FileView*, psy_ui_Component* parent);

INLINE psy_ui_Component* fileview_base(FileView* self)
{
	return &self->component;
}

const char* fileview_path(FileView* self);
void fileview_filename(FileView* self, char* filename, uintptr_t maxsize);
void fileview_setdirectory(FileView* self, const char* directory);

#ifdef __cplusplus
}
#endif

#endif /* FILEVIEW_H */

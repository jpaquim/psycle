/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(FILEBOX_H)
#define FILEBOX_H

/* ui */
#include <uibutton.h>
#include <uicheckbox.h>
#include <uilabel.h>
#include "uilistbox.h"
#include <uitabbar.h>
#include <uitextarea.h>
/* file */
#include <dir.h>

typedef struct FileLine {
	psy_ui_Component component;
	psy_ui_Button name;
	psy_ui_Label size;
	char* path;
} FileLine;

void fileline_init(FileLine*, psy_ui_Component* parent, const char* path, bool is_dir);

FileLine* fileline_alloc(void);
FileLine* fileline_allocinit(psy_ui_Component* parent, const char* path, bool is_dir);


typedef struct FileBox {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Scroller scroller;	
	psy_ui_Component pane;
	psy_ui_Component filepane;
	psy_ui_Component dirpane;
	uintptr_t selindex;
	psy_Path curr_dir;
	bool rebuild;
	char* wildcard;
	bool dirsonly;	
	/* signal */
	psy_Signal signal_selected;	
	psy_Signal signal_dir_changed;
	/* references */
	psy_Property* property;	
} FileBox;

void filebox_init(FileBox*, psy_ui_Component* parent);

void filebox_read(FileBox*, const char* path);
uintptr_t filebox_selected(const FileBox*);
void filebox_set_wildcard(FileBox* self, const char* wildcard);
void filebox_set_directory(FileBox* self, const char* path);
const char* filebox_directory(const FileBox*);
const char* filebox_file_name(FileBox*);
void filebox_full_name(FileBox*, char* rv, uintptr_t maxlen);


#ifdef __cplusplus
}
#endif

#endif /* FILEBOX_H */

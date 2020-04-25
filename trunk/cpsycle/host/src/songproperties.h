// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SONGPROPERTIES_H)
#define SONGPROPERTIES_H

#include <uilabel.h>
#include <uiedit.h>
#include "workspace.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Component tabbar;
	psy_audio_Song* song;
	psy_ui_Component title;
	psy_ui_Label label_title;
	psy_ui_Edit edit_title;
	psy_ui_Component credits;
	psy_ui_Label label_credits;
	psy_ui_Edit edit_credits;
	psy_ui_Component speed;
	psy_ui_Label label_speed;	
	psy_ui_Label label_tpb;
	psy_ui_Edit edit_tpb;
	psy_ui_Component comments;
	psy_ui_Label label_comments;
	psy_ui_Edit edit_comments;			
} SongPropertiesView;

void songpropertiesview_init(SongPropertiesView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);
void songpropertiesview_enableedit(SongPropertiesView*);
void songpropertiesview_disableedit(SongPropertiesView*);

#endif

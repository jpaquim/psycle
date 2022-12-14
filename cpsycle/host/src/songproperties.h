/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SONGPROPERTIES_H)
#define SONGPROPERTIES_H

/* host */
#include "intedit.h"
#include "workspace.h"
/* ui */
#include <uibutton.h>
#include <uitextarea.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SongPropertiesView */

typedef struct SongPropertiesView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component viewtabbar;
	psy_ui_Component top;
	psy_ui_Component title;
	psy_ui_Label label_title;
	psy_ui_TextArea edit_title;
	psy_ui_Component credits;
	psy_ui_Label label_credits;
	psy_ui_TextArea edit_credits;
	/* speed */
	psy_ui_Component speed;
	psy_ui_Label label_speed;
	psy_ui_Component speedbar;
	IntEdit tempo;
	IntEdit lpb;
	IntEdit tpb;
	IntEdit etpb;	
	IntEdit samplerindex;
	psy_ui_Label realtempo_desc;
	psy_ui_Label realticksperbeat_desc;
	psy_ui_Label realtempo;
	psy_ui_Label realticksperbeat;
	/* text */
	psy_ui_Component comments;
	psy_ui_Label label_comments;
	psy_ui_TextArea edit_comments;
	/* references */
	psy_audio_Song* song;
	Workspace* workspace;
} SongPropertiesView;

void songpropertiesview_init(SongPropertiesView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);

INLINE psy_ui_Component* songpropertiesview_base(SongPropertiesView* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SONGPROPERTIES_H */

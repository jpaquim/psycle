/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(RECENTVIEW_H)
#define RECENTVIEW_H

#include "propertiesview.h"
/* host */
#include "inputhandler.h"
/* ui */
#include <uibutton.h>
#include <playlist.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PlaylistView {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	PropertiesView view;		
} PlaylistView;

void playlistview_init(PlaylistView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, InputHandler*, psy_Playlist*);

INLINE psy_ui_Component* playlistview_base(PlaylistView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* RECENTVIEW_H */

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "recentview.h"
/* host */
#include "styles.h"


/* prototypes */
static void playlistview_on_playlist_changed(PlaylistView*,
	psy_Playlist* sender);

/* implementation */
void playlistview_init(PlaylistView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, InputHandler* inputhandler,
	psy_Playlist* playlist)
{	
	assert(self);
	assert(playlist);
	
	psy_ui_component_init(&self->component, parent, NULL);
	propertiesview_init(&self->view, &self->component,
		tabbarparent, playlist->recentsongs, 1, FALSE,
		inputhandler);	
	propertiesrenderer_set_style(&self->view.renderer,
		STYLE_BOX,
		STYLE_RECENTVIEW_MAINSECTIONHEADER,
		STYLE_RECENTVIEW_LINE,
		STYLE_RECENTVIEW_LINE_HOVER,
		STYLE_RECENTVIEW_LINE_SELECT);
	psy_ui_component_set_preferred_size(&self->view.component,
		psy_ui_size_make_em(50.0, 0.0));
	psy_ui_component_hide(&self->view.tabbar.component);	
	psy_ui_component_set_align(&self->view.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&playlist->signal_changed, self,
		playlistview_on_playlist_changed);
}

void playlistview_on_playlist_changed(PlaylistView* self, psy_Playlist* sender)
{
	propertiesview_reload(&self->view);
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "renderprogressview.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

/* PluginScanView */
/* prototypes */
void renderprogressview_onabort(RenderProgressView*, psy_ui_Button* sender);
/* implementation */
void renderprogressview_init(RenderProgressView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_label_init_text(&self->render, &self->component,
		"Rendering");
	psy_ui_component_set_align(psy_ui_label_base(&self->render),
		psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->abortbar, &self->component, NULL);
	psy_ui_component_set_margin(&self->abortbar,
		psy_ui_margin_make_em(2.0, 0.0, 0.0, 20.0));
	psy_ui_component_set_align(&self->abortbar, psy_ui_ALIGN_TOP);
	psy_ui_button_init_text_connect(&self->abort, &self->abortbar,
		"newmachine.stop", self, renderprogressview_onabort);
	psy_ui_component_set_align(psy_ui_button_base(&self->abort),
		psy_ui_ALIGN_LEFT);
}

void renderprogressview_onabort(RenderProgressView* self, psy_ui_Button* sender)
{	
	psy_audio_player_stop(&self->workspace->player);
}

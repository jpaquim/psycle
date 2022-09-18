/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "wireframes.h"
/* host */
#include "wireview.h"
#include "styles.h"

void wireframes_init(WireFrames* self, Workspace* workspace,
	psy_ui_Component* view)
{
	assert(self);
	assert(workspace);
	assert(view);
	
	self->workspace = workspace;
	self->wireframes = NULL;
	self->view = view;
}

void wireframes_dispose(WireFrames* self)
{
	psy_list_deallocate(&self->wireframes, (psy_fp_disposefunc)
		psy_ui_component_destroy);
}

void wireframes_destroy(WireFrames* self)
{
	psy_List* p;
	psy_List* q;
	
	assert(self);

	for (p = self->wireframes; p != NULL; p = q) {
		WireFrame* frame;

		frame = (WireFrame*)psy_list_entry(p);
		q = p->next;
		if (!wireview_wireexists(&frame->wireview)) {			
			psy_ui_component_deallocate(&frame->component);		
			psy_list_remove(&self->wireframes, p);
		}
	}
}

void wireframes_show(WireFrames* self, psy_audio_Wire wire)
{	
	assert(self);
	
	if (workspace_song(self->workspace) && psy_audio_wire_valid(&wire)) {
		WireFrame* wireframe;
		
		wireframe = wireframes_frame(self, wire);
		if (!wireframe) {		
			wireframe = wireframe_allocinit(self->view, wire,
				self->workspace);
			if (wireframe) {
				psy_list_append(&self->wireframes, wireframe);
				psy_signal_connect(&wireframe->component.signal_destroyed,
					self, wireframes_on_frame_destroyed);
			}
		}
		if (wireframe != NULL) {
			psy_ui_component_show(&wireframe->component);
		}
	}
}

void wireframes_on_frame_destroyed(WireFrames* self, psy_ui_Component* sender)
{
	psy_List* p;
	psy_List* q;

	assert(self);
	
	for (p = self->wireframes; p != NULL; p = q) {
		WireFrame* frame;

		frame = (WireFrame*)psy_list_entry(p);
		q = p->next;
		if (&frame->component == sender) {
			psy_list_remove(&self->wireframes, p);
		}
	}
}

WireFrame* wireframes_frame(WireFrames* self, psy_audio_Wire wire)
{	
	WireFrame* rv;
	psy_List* p;

	assert(self);
	
	rv = NULL;
	p = self->wireframes;
	while (p != NULL) {
		WireFrame* frame;

		frame = (WireFrame*)psy_list_entry(p);
		if (psy_audio_wire_equal(wireframe_wire(frame), &wire)) {
			rv = frame;
			break;
		}
		psy_list_next(&p);
	}
	return rv;
}

/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "paramviews.h"


/* prototypes */
static void paramviews_on_song_changed(ParamViews*, Workspace* sender);
static psy_audio_Machines* paramviews_machines(ParamViews*);

/* implementation */
void paramviews_init(ParamViews* self, psy_ui_Component* view,
	Workspace* workspace)
{	
	assert(self);
	assert(view);
	assert(workspace);	

	self->view = view;
	self->workspace = workspace;	
	psy_table_init(&self->frames);
	psy_signal_connect(&workspace->signal_songchanged, self,
		paramviews_on_song_changed);
}

void paramviews_dispose(ParamViews* self)
{	
	assert(self);
	
	paramviews_remove_all(self);
	psy_table_dispose(&self->frames);
}

psy_audio_Machines* paramviews_machines(ParamViews* self)
{
	assert(self);
	
	return (workspace_song(self->workspace))
		? &workspace_song(self->workspace)->machines
		: NULL;
}

void paramviews_show(ParamViews* self, uintptr_t mac_id)
{
	psy_audio_Machine* machine;
	MachineFrame* frame;
	
	assert(self);
	
	if (!paramviews_machines(self)) {
		return;
	}	
	machine = psy_audio_machines_at(paramviews_machines(self), mac_id);
	if (!machine) {
		return;
	}
	frame = paramviews_frame(self, mac_id);
	if (!frame) {
		frame = machineframe_allocinit(self->view, machine, self,
			self->workspace);
		if (frame) {
			psy_table_insert(&self->frames, mac_id, (void*)frame);
		}
	}
	if (frame) {
		psy_ui_component_show(&frame->component);
	}
}

void paramviews_erase(ParamViews* self, uintptr_t mac_id)
{
	assert(self);
	
	psy_table_remove(&self->frames, mac_id);	
}

void paramviews_remove(ParamViews* self, uintptr_t mac_id)
{	
	MachineFrame* frame;

	assert(self);
	
	frame = paramviews_frame(self, mac_id);
	if (frame) {	
		psy_ui_component_destroy(&frame->component);		
	}
}

void paramviews_remove_all(ParamViews* self)
{	
	psy_TableIterator it;	
	psy_List* frames;
	psy_List* p;
	
	assert(self);
	
	frames = NULL;	
	/*
	** copy machine frame pointers to a list to prevent traversing with invalid
	** tableiterators after calling frame destroy
	*/
	for (it = psy_table_begin(&self->frames);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {		
		psy_list_append(&frames, psy_tableiterator_value(&it));
	}
	/* destroy frames */
	for (p = frames; p != NULL; p = p->next) {
		psy_ui_Component* frame;
		
		frame = (psy_ui_Component*)psy_list_entry(p);
		/* a frame will erase themself from paramviews in on_close */
		frame->vtable->onclose(frame);		
		psy_ui_component_destroy(frame);		
	}
	psy_list_free(frames);
	frames = NULL;
	/* assert if all frames were removed */
	assert(psy_table_size(&self->frames) == 0);
}

MachineFrame* paramviews_frame(ParamViews* self, uintptr_t mac_id)
{
	assert(self);
	
	return (MachineFrame*)psy_table_at(&self->frames, mac_id);
}

void paramviews_on_song_changed(ParamViews* self, Workspace* sender)
{
	assert(self);
	
	paramviews_remove_all(self);
}

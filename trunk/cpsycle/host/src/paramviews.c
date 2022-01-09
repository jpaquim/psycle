/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "paramviews.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void paramviews_onsongchanged(ParamViews*, Workspace* sender);
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
		paramviews_onsongchanged);
}

void paramviews_dispose(ParamViews* self)
{	
	paramviews_removeall(self);
	psy_table_dispose(&self->frames);
}

psy_audio_Machines* paramviews_machines(ParamViews* self)
{
	return (workspace_song(self->workspace))
		? &workspace_song(self->workspace)->machines
		: NULL;
}

void paramviews_show(ParamViews* self, uintptr_t macid)
{
	if (paramviews_machines(self)) {
		psy_audio_Machine* machine;

		machine = psy_audio_machines_at(paramviews_machines(self), macid);
		if (machine) {
			MachineFrame* frame;

			frame = paramviews_frame(self, macid);
			if (!frame) {
				frame = machineframe_allocinit(self->view, machine, self,
					self->workspace);
				if (frame) {
					psy_table_insert(&self->frames, macid, (void*)frame);
				}
			}
			if (frame) {
				psy_ui_component_show(&frame->component);
			}
		}
	}
}

void paramviews_erase(ParamViews* self, uintptr_t macid)
{
	psy_table_remove(&self->frames, macid);	
}

void paramviews_remove(ParamViews* self, uintptr_t macid)
{	
	MachineFrame* frame;

	frame = paramviews_frame(self, macid);
	if (frame) {	
		psy_ui_component_destroy(&frame->component);		
	}
}

void paramviews_removeall(ParamViews* self)
{	
	psy_TableIterator it;	
	psy_List* frames;
	psy_List* p;
	
	frames = NULL;
	// copy machine frame pointers to a list to prevent traversing with invalid
	// tableiterators after calling frame destroy
	for (it = psy_table_begin(&self->frames);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {		
		psy_list_append(&frames, psy_tableiterator_value(&it));
	}
	// destroy frames	
	for (p = frames; p != NULL; p = p->next) {
		// a frame will erase themself from paramviews (calling paramviews_erase)
		// in ondestroy
		psy_ui_component_destroy((psy_ui_Component*)psy_list_entry(p));
	}
	psy_list_free(frames);
	frames = NULL;
	// assert if all frames were removed
	assert(psy_table_size(&self->frames) == 0);
}

MachineFrame* paramviews_frame(ParamViews* self, uintptr_t macid)
{
	return (MachineFrame*)psy_table_at(&self->frames, macid);
}

void paramviews_onsongchanged(ParamViews* self, Workspace* sender)
{
	paramviews_removeall(self);
}

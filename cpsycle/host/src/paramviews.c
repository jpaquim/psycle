// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "paramviews.h"
// host
#include "machineeditorview.h"
#include "paramview.h"
// platform
#include "../../detail/portable.h"

// prototypes
static void paramviews_onsongchanged(ParamViews*, Workspace*, int flag,
	psy_audio_Song*);
static void paramviews_onframedestroyed(ParamViews*, psy_ui_Component* sender);
static psy_audio_Machines* paramviews_machines(ParamViews*);
// implementation
void paramviews_init(ParamViews* self, psy_ui_Component* view,
	Workspace* workspace)
{	
	assert(self);
	assert(view);
	assert(workspace);	

	self->view = view;
	self->workspace = workspace;
	self->removingall = FALSE;
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
	psy_audio_Machine* machine;

	if (paramviews_machines(self)) {
		machine = psy_audio_machines_at(paramviews_machines(self), macid);
	} else {
		machine = NULL;
	}
	if (machine) {
		MachineFrame* frame;

		frame = paramviews_frame(self, macid);
		if (!frame) {
			frame = machineframe_alloc();
			psy_table_insert(&self->frames, macid, (void*)frame);
			machineframe_init(frame, self->view, self->workspace);
			psy_signal_connect(&frame->component.signal_destroy,
				self, paramviews_onframedestroyed);
			if (psy_audio_machine_haseditor(machine)) {
				MachineEditorView* editorview;

				editorview = machineeditorview_allocinit(
					&frame->notebook.component, machine, self->workspace);
				if (editorview) {
					machineframe_setview(frame, &editorview->component,
						machine);
				}
			} else {
				ParamView* paramview;

				paramview = paramview_allocinit(
					&frame->notebook.component, machine, self->workspace);
				if (paramview) {
					machineframe_setparamview(frame, paramview, machine);
				}
			}
		}
		if (frame) {
			psy_ui_component_show(&frame->component);
		}
	}
}

void paramviews_remove(ParamViews* self, uintptr_t macid)
{	
	MachineFrame* frame;

	frame = paramviews_frame(self, macid);
	if (frame) {	
		psy_ui_component_destroy(&frame->component);
		free(frame);
	}
}

void paramviews_removeall(ParamViews* self)
{	
	psy_TableIterator it;	

	self->removingall = TRUE;
	for (it = psy_table_begin(&self->frames);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_ui_component_destroy(psy_tableiterator_value(&it));
	}
	psy_table_clear(&self->frames);
	self->removingall = FALSE;
}

void paramviews_onframedestroyed(ParamViews* self, psy_ui_Component* sender)
{
	if (!self->removingall) {
		psy_TableIterator it;
		uintptr_t macid;

		macid = psy_INDEX_INVALID;
		for (it = psy_table_begin(&self->frames);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			if (psy_tableiterator_value(&it) == sender) {
				macid = psy_tableiterator_key(&it);
				break;
			}
		}
		if (macid != psy_INDEX_INVALID) {
			psy_table_remove(&self->frames, macid);
		}
	}
}

MachineFrame* paramviews_frame(ParamViews* self, uintptr_t macid)
{
	return psy_table_at(&self->frames, macid);
}

void paramviews_onsongchanged(ParamViews* self, Workspace* workspace,
	int flag, psy_audio_Song* song)
{
	paramviews_removeall(self);
}

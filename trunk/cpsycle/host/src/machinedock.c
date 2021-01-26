// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinedock.h"
// audio
#include <songio.h>
// platform
#include "../../detail/portable.h"


void machinedockbox_init(MachineDockBox* self, psy_ui_Component* parent,
	uintptr_t slot, Workspace* workspace)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(&workspace->song->machines, slot);
	psy_ui_component_init(&self->component, parent);
	psy_ui_label_init(&self->header, &self->component);
	psy_ui_label_setcharnumber(&self->header, 20.0);
	psy_ui_component_setbackgroundcolour(psy_ui_label_base(&self->header),
		psy_ui_colour_make(0x00404040));
	if (machine) {
		psy_ui_label_settext(&self->header, psy_audio_machine_editname(machine));
	}
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	parameterlistbox_init(&self->parameters, &self->component,
		psy_audio_machines_at(&workspace->song->machines, slot), workspace);
	psy_ui_component_setalign(&self->parameters.component, psy_ui_ALIGN_LEFT);
}

static void ondestroy(MachineDockPane*);
static void machinedockpane_build(MachineDockPane*);
static void machinedockpane_insertbox(MachineDockPane*, uintptr_t slot);
static void machinedockpane_removebox(MachineDockPane*, uintptr_t slot);
static void machinedockpane_onsongchanged(MachineDockPane*, Workspace* sender,
	int flag, psy_audio_SongFile*);
static void machinedockpane_connectsong(MachineDockPane*);
static void machinedockpane_onmachineselected(MachineDockPane*,
	psy_audio_Machines*, uintptr_t slot);
static void machinedockpane_onmachinesinsert(MachineDockPane*,
	psy_audio_Machines*, uintptr_t slot);
static void machinedockpane_onmachinesremoved(MachineDockPane*,
	psy_audio_Machines*, uintptr_t slot);

static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MachineDockPane* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)ondestroy;		
		vtable_initialized = TRUE;
	}
}

void machinedockpane_init(MachineDockPane* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);	
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	if (workspace_song(workspace)) {
		self->machines = &workspace->song->machines;
	}
	self->workspace = workspace;
	psy_signal_connect(&workspace->signal_songchanged, self,
		machinedockpane_onsongchanged);
	machinedockpane_connectsong(self);
	psy_table_init(&self->boxes);
	machinedockpane_build(self);
}

void ondestroy(MachineDockPane* self)
{
	psy_table_dispose(&self->boxes);
}

void machinedockpane_onsongchanged(MachineDockPane* self, Workspace* sender,
	int flag, psy_audio_SongFile* songfile)
{
	self->machines = &sender->song->machines;	
	machinedockpane_connectsong(self);
	machinedockpane_build(self);
	psy_ui_component_align(&self->component);
	psy_ui_component_align(psy_ui_component_parent(&self->component));	
}

void machinedockpane_build(MachineDockPane* self)
{
	psy_TableIterator it;	

	psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
		psy_ui_component_destroy);
	psy_table_init(&self->boxes);
	for (it = psy_audio_machines_begin(self->machines);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		if (!(psy_tableiterator_key(&it) > 0x80 &&
				psy_tableiterator_key(&it) <= 0xFE)) {			
			machinedockpane_insertbox(self, psy_tableiterator_key(&it));
		}
	}
}

 void machinedockpane_insertbox(MachineDockPane* self, uintptr_t slot)
{
	if (psy_audio_machines_at(self->machines, slot)) {
		MachineDockBox* box;

		if (psy_table_exists(&self->boxes, slot)) {
			machinedockpane_removebox(self, slot);
		}
		box = (MachineDockBox*)malloc(sizeof(MachineDockBox));
		if (box) {
			machinedockbox_init(box, &self->component, slot, self->workspace);
			psy_ui_component_setalign(&box->component, psy_ui_ALIGN_LEFT);
			psy_table_insert(&self->boxes, slot, box);
		}		
	}
}

void machinedockpane_removebox(MachineDockPane* self, uintptr_t slot)
{
	if (psy_table_exists(&self->boxes, slot)) {
		MachineDockBox* box;

		box = psy_table_at(&self->boxes, slot);
		psy_ui_component_deallocate(&box->component);		
		psy_table_remove(&self->boxes, slot);		
	}
}

void machinedockpane_connectsong(MachineDockPane* self)
{
	if (self->machines) {
		psy_signal_connect(&self->machines->signal_slotchange, self,
			machinedockpane_onmachineselected);
		psy_signal_connect(&self->machines->signal_insert, self,
			machinedockpane_onmachinesinsert);
		psy_signal_connect(&self->machines->signal_removed, self,
			machinedockpane_onmachinesremoved);
		// psy_signal_connect(&self->machines->connections.signal_connected, self,
		//	machinedockpane_onconnected);
		// psy_signal_connect(&self->machines->connections.signal_disconnected, self,
		//	machinedockpane_ondisconnected);
	}
}


void machinedockpane_onmachineselected(MachineDockPane* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
}

void machinedockpane_onmachinesinsert(MachineDockPane* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	machinedockpane_insertbox(self, slot);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}

void machinedockpane_onmachinesremoved(MachineDockPane* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	machinedockpane_removebox(self, slot);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}

// MachineDock
static void machinedock_horizontal_onchanged(MachineDock*, psy_ui_ScrollBar* sender);
static void machinedock_scrollrangechanged(MachineDock*, psy_ui_Component* sender,
	psy_ui_Orientation orientation);
// implemnetation
void machinedock_init(MachineDock* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	// Scrollbar
	psy_ui_scrollbar_init(&self->hscroll, &self->component);
	psy_ui_scrollbar_setorientation(&self->hscroll, psy_ui_HORIZONTAL);
	psy_ui_component_setalign(psy_ui_scrollbar_base(&self->hscroll), psy_ui_ALIGN_BOTTOM);	
	// Pane
	machinedockpane_init(&self->pane, &self->component, workspace);		
	psy_ui_component_setalign(&self->pane.component,
		psy_ui_ALIGN_VCLIENT);
	psy_ui_component_setmaximumsize(&self->component,
		psy_ui_size_makeem(0.0, 10.0));
	psy_ui_component_setminimumsize(&self->component,
		psy_ui_size_makeem(0.0, 10.0));	
	// connect scrollbar
	psy_signal_connect(&self->hscroll.signal_changed, self,
		machinedock_horizontal_onchanged);
	psy_ui_scrollbar_setscrollrange(&self->hscroll, 0, 100);
	psy_signal_connect(&self->pane.component.signal_scrollrangechanged, self,
		machinedock_scrollrangechanged);
}

void machinedock_horizontal_onchanged(MachineDock* self, psy_ui_ScrollBar* sender)
{
	psy_ui_RealRectangle position;
	double nPos;

	nPos = psy_ui_scrollbar_position(sender);
	position = psy_ui_component_position(&self->pane.component);
	psy_ui_component_move(&self->pane.component,		
		psy_ui_point_makepx(-nPos * 100, 0));
}

void machinedock_scrollrangechanged(MachineDock* self, psy_ui_Component* sender,
	psy_ui_Orientation orientation)
{
	psy_ui_scrollbar_setscrollrange(&self->hscroll,
		(double)sender->hscrollrange.x,
		(double)sender->hscrollrange.y);
}

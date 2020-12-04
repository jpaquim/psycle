// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinebar.h"
#include <songio.h>
#include <plugin_interface.h>

#include "../../detail/portable.h"

static void machinebar_ondestroy(MachineBar*, psy_ui_Component* component);
static void machinebar_buildmachinebox(MachineBar*);
static void machinebar_onmachinesinsert(MachineBar*, psy_audio_Machines* machines, int slot);
static int machinebar_insertmachine(MachineBar*, size_t slot, psy_audio_Machine*);
static void machinebar_onmachinesremoved(MachineBar*, psy_audio_Machines* machines, int slot);
static void machinebar_onmachinesslotchange(MachineBar*, psy_audio_Machines* machines, int slot);
static void machinebar_onmachineboxselchange(MachineBar*, psy_ui_Component* sender, int sel);
static void machinebar_onselectinstparamselchange(MachineBar*, psy_ui_Component* sender, int sel);
static void machinebar_oninstparamboxselchange(MachineBar*, psy_ui_Component* sender, int sel);
static void machinebar_buildparaminstbox(MachineBar*);
static void machinebar_buildparamlist(MachineBar*);
static void machinebar_buildinstrumentlist(MachineBar*);
static void machinebar_oninstrumentnamechanged(MachineBar*, psy_ui_Component* sender);
static void machinebar_oninstrumentinsert(MachineBar*, psy_audio_Instruments* sender, int slot);
static void machinebar_oninstrumentslotchanged(MachineBar* self, psy_audio_Instrument* sender,
	const psy_audio_InstrumentIndex* slot);
static void machinebar_onsongchanged(MachineBar*, Workspace*, int flag, psy_audio_SongFile*);
static void machinebar_connectsongsignals(MachineBar*);
static void machinebar_connectinstrumentsignals(MachineBar*);
static void machinebar_clearmachinebox(MachineBar* self);
static void machinebar_onprevmachine(MachineBar*, psy_ui_Component* sender);
static void machinebar_onnextmachine(MachineBar*, psy_ui_Component* sender);
static bool machinebar_instrumentmode(MachineBar* self)
{
	return psy_ui_combobox_cursel(&self->selectinstparam) == 1;
}

void machinebar_init(MachineBar* self, psy_ui_Component* parent, Workspace* workspace)
{		
	psy_ui_Margin margin;

	self->workspace = workspace;
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0), psy_ui_value_makeew(2.0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));				
	self->selchange = 0;	
	self->machines = &workspace->song->machines;	
	self->instruments = &workspace->song->instruments;
	psy_table_init(&self->comboboxslots);
	psy_table_init(&self->slotscombobox);
	psy_ui_component_init(&self->component, parent);	
	psy_signal_connect(&self->component.signal_destroy, self,
		machinebar_ondestroy);
	psy_ui_combobox_init(&self->machinebox, &self->component);
	psy_ui_combobox_setcharnumber(&self->machinebox, 30);	
	psy_ui_button_init(&self->gear, &self->component);
	psy_ui_button_settext(&self->gear, "machinebar.gear");
	psy_ui_button_init(&self->editor, &self->component);
	psy_ui_button_settext(&self->editor, "machinebar.editor");
	psy_ui_button_init(&self->cpu, &self->component);
	psy_ui_button_settext(&self->cpu, "machinebar.cpu");
	psy_ui_button_init(&self->midi, &self->component);
	psy_ui_button_settext(&self->midi, "machinebar.midi");
	machinebar_buildmachinebox(self);
	psy_signal_connect(&self->machinebox.signal_selchanged, self,
		machinebar_onmachineboxselchange);
	self->prevent_selchange_notify = FALSE;
	psy_ui_combobox_init(&self->selectinstparam, &self->component);
	psy_ui_combobox_setcharnumber(&self->selectinstparam, 14);
	psy_ui_combobox_addtext(&self->selectinstparam, "Params");
	psy_ui_combobox_addtext(&self->selectinstparam,
		workspace_translate(workspace, "Instrument"));
	psy_signal_connect(&self->selectinstparam.signal_selchanged, self,
		machinebar_onselectinstparamselchange);
	// Combobox for Instruments or Parameters
	psy_ui_combobox_init(&self->instparambox, &self->component);
	psy_ui_combobox_setcharnumber(&self->instparambox, 30);
	machinebar_buildparaminstbox(self);
	psy_ui_combobox_setcursel(&self->selectinstparam, 1);	
	psy_ui_combobox_setcursel(&self->instparambox, 0);
	psy_signal_connect(&self->instparambox.signal_selchanged, self,
		machinebar_oninstparamboxselchange);
	psy_signal_connect(&workspace->signal_songchanged, self,
		machinebar_onsongchanged);
	machinebar_connectsongsignals(self);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));	
}

void machinebar_ondestroy(MachineBar* self, psy_ui_Component* component)
{
	psy_table_dispose(&self->comboboxslots);
	psy_table_dispose(&self->slotscombobox);
}

void machinebar_clearmachinebox(MachineBar* self)
{
	psy_ui_combobox_clear(&self->machinebox);
	psy_table_dispose(&self->comboboxslots);
	psy_table_init(&self->comboboxslots);
	psy_table_dispose(&self->slotscombobox);
	psy_table_init(&self->slotscombobox);
}

void machinebar_oninstrumentinsert(MachineBar* self, psy_audio_Instruments* sender,
	int slot)
{
	psy_audio_Instrument* instrument;

	machinebar_buildinstrumentlist(self);
	psy_ui_combobox_setcursel(&self->instparambox, slot);
	instrument = psy_audio_instruments_at(sender,
		psy_audio_instrumentindex_make(0, slot));
	if (instrument) {
		psy_signal_connect(&instrument->signal_namechanged, self,
			machinebar_oninstrumentnamechanged);
	}
}

void machinebar_oninstrumentslotchanged(MachineBar* self, psy_audio_Instrument* sender,
	const psy_audio_InstrumentIndex* slot)
{
	psy_ui_combobox_setcursel(&self->instparambox, slot->subslot);	
}

void machinebar_onmachinesinsert(MachineBar* self, psy_audio_Machines* machines, int slot)
{	
	machinebar_buildmachinebox(self);
	psy_ui_combobox_setcursel(&self->machinebox, machines->slot);
}

void machinebar_onmachinesremoved(MachineBar* self, psy_audio_Machines* machines, int slot)
{
	machinebar_buildmachinebox(self);
	psy_ui_combobox_setcursel(&self->machinebox, machines->slot);	
}

void machinebar_buildmachinebox(MachineBar* self)
{
	machinebar_clearmachinebox(self);
	if (psy_audio_machines_size(self->machines) == 1) {
		psy_ui_combobox_addtext(&self->machinebox,
			workspace_translate(self->workspace,
				"machineview.no-machines-loaded"));
		psy_ui_combobox_setcursel(&self->machinebox, 0);
	} else {
		psy_TableIterator it;
	
		for (it = psy_audio_machines_begin(self->machines);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {			
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
			machinebar_insertmachine(self, psy_tableiterator_key(&it),  machine);
		}
	}
}

int machinebar_insertmachine(MachineBar* self, size_t slot, psy_audio_Machine* machine)
{			
	if (slot != psy_audio_MASTER_INDEX && psy_audio_machine_info(machine) &&
			psy_audio_machine_info(machine)->ShortName) {
		intptr_t comboboxindex;

		char buffer[128];
		psy_snprintf(buffer, 128, "%02X: %s", slot, 
			machine->vtable->info(machine)->ShortName); 
		comboboxindex = psy_ui_combobox_addtext(&self->machinebox, buffer);
		psy_table_insert(&self->comboboxslots, comboboxindex, (void*)slot);
		psy_table_insert(&self->slotscombobox, slot, (void*) comboboxindex);
	}
	return 1;
}

void machinebar_onmachineboxselchange(MachineBar* self, psy_ui_Component* sender, int sel)
{	
	size_t slot;
	
	psy_List* slots = self->machinebox.signal_selchanged.slots;
	self->machinebox.signal_selchanged.slots = 0;
	slot = (size_t)psy_table_at(&self->comboboxslots, sel);
	psy_audio_machines_changeslot(self->machines, slot);	
	self->machinebox.signal_selchanged.slots = slots;
}

void machinebar_onmachinesslotchange(MachineBar* self, psy_audio_Machines* machines, int slot)
{	
	intptr_t comboboxindex;
	psy_audio_Machine* machine;

	comboboxindex = (intptr_t) psy_table_at(&self->slotscombobox, slot);
	psy_ui_combobox_setcursel(&self->machinebox, comboboxindex);
	machine = self->machines ? psy_audio_machines_at(self->machines, psy_audio_machines_slot(self->machines)) : NULL;
	if (machine && machine_supports(machine, MACH_SUPPORTS_INSTRUMENTS)) {
		psy_ui_combobox_setcursel(&self->selectinstparam, 1);
	} else {
		psy_ui_combobox_setcursel(&self->selectinstparam, 0);
	}
	machinebar_buildparaminstbox(self);
	psy_ui_combobox_setcursel(&self->instparambox, 0);
}

void machinebar_buildparaminstbox(MachineBar* self)
{
	if (machinebar_instrumentmode(self)) {
		machinebar_buildinstrumentlist(self);
	} else {
		machinebar_buildparamlist(self);
	}
}

void machinebar_buildparamlist(MachineBar* self)
{
	psy_audio_Machine* machine;

	psy_ui_combobox_clear(&self->instparambox);
	machine = (self->machines)
		? psy_audio_machines_at(self->machines, psy_audio_machines_slot(self->machines))
		: NULL;
	if (machine) {
		uintptr_t i;

		for (i = 0; i != psy_audio_machine_numtweakparameters(machine); ++i) {
			psy_audio_MachineParam* param;			

			param = psy_audio_machine_tweakparameter(machine, i);
			if (param) {
				char text[128];
				char label[128];

				if (!psy_audio_machineparam_label(param, label)) {
					if (!psy_audio_machineparam_name(param, label)) {
						psy_snprintf(text, 128, "Parameter", label);
					}
				}
				if ((psy_audio_machineparam_type(param) & MPF_HEADER) == MPF_HEADER) {
					psy_snprintf(text, 128, "-----%s-----", label);
				} else {
					psy_snprintf(text, 128, "%s", label);
				}
				psy_ui_combobox_addtext(&self->instparambox, text);
			}
		}
	} else {
		psy_ui_combobox_addtext(&self->instparambox,
			workspace_translate(self->workspace, "machineview.no-machine"));
	}
}

void machinebar_buildinstrumentlist(MachineBar* self)
{
	psy_audio_Instrument* instrument;
	int slot = 0;
	char text[20];

	psy_ui_combobox_clear(&self->instparambox);
	for ( ; slot < 256; ++slot) {		
		if (instrument = psy_audio_instruments_at(&self->workspace->song->instruments,
			psy_audio_instrumentindex_make(0, slot))) {
			psy_snprintf(text, 20, "%02X:%s", slot,
				psy_audio_instrument_name(instrument));
		} else {
			psy_snprintf(text, 20, "%02X:%s", slot, "");
		}
		psy_ui_combobox_addtext(&self->instparambox, text);
	}
}

void machinebar_onselectinstparamselchange(MachineBar* self, psy_ui_Component* sender, int sel)
{
	machinebar_buildparaminstbox(self);
}

void machinebar_oninstparamboxselchange(MachineBar* self, psy_ui_Component* sender, int sel)
{
	if (machinebar_instrumentmode(self)) {		
		psy_audio_instruments_select(self->instruments,
			psy_audio_instrumentindex_make(0, sel));		
	} else {
		psy_audio_machines_changetweakparam(self->machines, sel);
	}
}

void machinebar_onsongchanged(MachineBar* self, Workspace* workspace, int flag,
	psy_audio_SongFile* songfile)
{	
	self->machines = &workspace->song->machines;
	self->instruments = &workspace->song->instruments;	
	machinebar_connectsongsignals(self);
	machinebar_buildmachinebox(self);
	machinebar_buildinstrumentlist(self);
	psy_ui_combobox_setcursel(&self->instparambox,
		psy_audio_instruments_selected(&workspace->song->instruments).subslot);
}

void machinebar_connectsongsignals(MachineBar* self)
{
	psy_signal_connect(&self->machines->signal_insert, self,
		machinebar_onmachinesinsert);
	psy_signal_connect(&self->machines->signal_removed, self,
		machinebar_onmachinesremoved);
	psy_signal_connect(&self->machines->signal_slotchange, self,
		machinebar_onmachinesslotchange);
	psy_signal_connect(&self->instruments->signal_insert, self,
		machinebar_oninstrumentinsert);
	psy_signal_connect(&self->instruments->signal_slotchange, self,
		machinebar_oninstrumentslotchanged);
	machinebar_connectinstrumentsignals(self);
}

void machinebar_connectinstrumentsignals(MachineBar* self)
{
	psy_TableIterator it;

	for (it = psy_audio_instruments_groupbegin(self->instruments, 0);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_Instrument* instrument;
		
		instrument = (psy_audio_Instrument*)psy_tableiterator_value(&it);
		psy_signal_connect(&instrument->signal_namechanged, self,
			machinebar_oninstrumentnamechanged);
	}
}

void machinebar_onnextmachine(MachineBar* self, psy_ui_Component* sender)
{
	if (self->machines && psy_audio_machines_slot(self->machines) > 0) {
		psy_audio_machines_changeslot(self->machines,
			psy_audio_machines_slot(self->machines) - 1);
	}
}

void machinebar_onprevmachine(MachineBar* self, psy_ui_Component* sender)
{
	if (self->machines) {
		psy_audio_machines_changeslot(self->machines,
			psy_audio_machines_slot(self->machines) + 1);
	}
}

void machinebar_oninstrumentnamechanged(MachineBar* self, psy_ui_Component* sender)
{
	if (self->instruments) {
		machinebar_buildinstrumentlist(self);
		psy_ui_combobox_setcursel(&self->instparambox,
			psy_audio_instruments_selected(self->instruments).subslot);
	}
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "machinebar.h"
/* audio */
#include <songio.h>
#include <containerconvert.h>
#include <plugin_interface.h>
/* platform */ 
#include "../../detail/portable.h"


/* prototypes */
static void machinebar_on_destroyed(MachineBar*);
static MachineBarInstParamMode machinebar_mode(MachineBar*);
static void machinebar_updatemode(MachineBar*);
static void machinebar_onmachineboxselchange(MachineBar*,
	psy_ui_Component* sender, int sel);
static void machinebar_buildmachinebox(MachineBar*);
static void machinebar_onmachinesinsert(MachineBar*, psy_audio_Machines* sender, uintptr_t slot);
static int machinebar_insertmachine(MachineBar*, size_t slot, psy_audio_Machine*);
static void machinebar_onmachinesremoved(MachineBar*, psy_audio_Machines* sender, uintptr_t slot);
static void machinebar_onmachineselect(MachineBar*, psy_audio_Machines* sender, uintptr_t slot);
static void machinebar_onparamselected(MachineBar*, psy_audio_Machines* sender, uintptr_t slot);
static void machinebar_onselectinstparamselchange(MachineBar*,
	psy_ui_Component* sender, intptr_t sel);
static void machinebar_oninstparamboxselchange(MachineBar*,
	psy_ui_Component* sender, intptr_t sel);
static void machinebar_buildparaminstbox(MachineBar*);
static void machinebar_buildparamlist(MachineBar*);
static void machinebar_buildinstrumentlist(MachineBar*);
static void machinebar_buildauxlist(MachineBar*);
static void machinebar_oninstrumentnamechanged(MachineBar*, psy_ui_Component* sender);
static void machinebar_oninstrumentinsert(MachineBar*, psy_audio_Instruments* sender, uintptr_t slot);
static void machinebar_oninstrumentslotchanged(MachineBar* self, psy_audio_Instrument* sender,
	const psy_audio_InstrumentIndex* slot);
static void machinebar_on_song_changed(MachineBar*, psy_audio_Player* sender);
static void machinebar_connectsongsignals(MachineBar*);
static void machinebar_connectinstrumentsignals(MachineBar*);
static void machinebar_clearmachinebox(MachineBar* self);
static void machinebar_onprevmachine(MachineBar*, psy_ui_Component* sender);
static void machinebar_onnextmachine(MachineBar*, psy_ui_Component* sender);
static intptr_t machinebar_comboboxindex(MachineBar* self, uintptr_t slot);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MachineBar* self)
{
	if (!vtable_initialized) {
		vtable = *(machinebar_base(self)->vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			machinebar_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(machinebar_base(self), &vtable);
}

/* implementation */
void machinebar_init(MachineBar* self, psy_ui_Component* parent, Workspace* workspace)
{		
	psy_ui_Margin margin;
	GeneralConfig* general;

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);	
	self->workspace = workspace;
	general = &workspace->config.general;
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);			
	self->selchange = 0;	
	self->machines = &workspace->song->machines;	
	self->instruments = &workspace->song->instruments;
	psy_table_init(&self->comboboxslots);
	psy_table_init(&self->slotscombobox);	
	psy_ui_component_set_default_align(machinebar_base(self), psy_ui_ALIGN_LEFT,
		margin);	
	/* Machine ComboBox */
	psy_ui_combobox_init(&self->machinebox, &self->component);
	psy_ui_combobox_set_char_number(&self->machinebox, 30);
	psy_signal_connect(&self->machinebox.signal_selchanged, self,
		machinebar_onmachineboxselchange);
	machinebar_buildmachinebox(self);
	/* Tool Buttons */
	psy_ui_button_init(&self->gear, &self->component);
	psy_ui_button_data_exchange(&self->gear,
		generalconfig_property(general, "bench.showgear"));	
	psy_ui_button_set_text(&self->gear, "machinebar.gear");	
	psy_ui_button_init(&self->dock, &self->component);
	psy_ui_button_data_exchange(&self->dock,
		generalconfig_property(general, "bench.showparamrack"));	
	psy_ui_button_set_text(&self->dock, "machinebar.dock");
#ifdef PSYCLE_USE_PLUGIN_EDITOR	
	psy_ui_button_init(&self->editor, &self->component);
	psy_ui_button_data_exchange(&self->editor,
		generalconfig_property(general, "bench.showplugineditor"));	
	psy_ui_button_set_text(&self->editor, "machinebar.editor");
#endif	
	psy_ui_button_init(&self->cpu, &self->component);
	psy_ui_button_set_text(&self->cpu, "machinebar.cpu");
	psy_ui_button_data_exchange(&self->cpu,
		generalconfig_property(general, "bench.showcpu"));
	psy_ui_button_init(&self->midi, &self->component);
	psy_ui_button_set_text(&self->midi, "machinebar.midi");
	psy_ui_button_data_exchange(&self->midi,
		generalconfig_property(general, "bench.showmidi"));	
	self->prevent_selchange_notify = FALSE;
	psy_ui_combobox_init(&self->selectinstparam, &self->component);
	psy_ui_combobox_set_char_number(&self->selectinstparam, 14);
	psy_ui_combobox_add_text(&self->selectinstparam, "Params");
	psy_ui_combobox_add_text(&self->selectinstparam, psy_ui_translate(
		"Instrument"));
	psy_signal_connect(&self->selectinstparam.signal_selchanged, self,
		machinebar_onselectinstparamselchange);
	/* Combobox for Instruments or Parameters */
	psy_ui_combobox_init(&self->instparambox, &self->component);
	psy_ui_combobox_set_char_number(&self->instparambox, 30);
	machinebar_buildparaminstbox(self);
	psy_ui_combobox_select(&self->selectinstparam, 1);	
	psy_ui_combobox_select(&self->instparambox, 0);
	psy_signal_connect(&self->instparambox.signal_selchanged, self,
		machinebar_oninstparamboxselchange);
	psy_signal_connect(&workspace->player.signal_song_changed, self,
		machinebar_on_song_changed);
	machinebar_connectsongsignals(self);		
}

void machinebar_on_destroyed(MachineBar* self)
{
	psy_table_dispose(&self->comboboxslots);
	psy_table_dispose(&self->slotscombobox);
}

MachineBarInstParamMode machinebar_mode(MachineBar* self)
{
	if (psy_ui_combobox_cursel(&self->selectinstparam) == 0) {
		return MACHINEBAR_PARAM;
	}
	if (psy_audio_machines_selectedmachine(self->machines) &&
			psy_audio_machine_numauxcolumns(
				psy_audio_machines_selectedmachine(self->machines)) > 0) {
		return MACHINEBAR_AUX;
	}
	return MACHINEBAR_INST;
}

void machinebar_updatemode(MachineBar* self)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_selectedmachine(self->machines);	
	if (machine && (machine_supports(machine, psy_audio_SUPPORTS_INSTRUMENTS) ||
		psy_audio_machine_numauxcolumns(machine))) {
		psy_ui_combobox_select(&self->selectinstparam, 1);
	} else {
		psy_ui_combobox_select(&self->selectinstparam, 0);
	}
}

void machinebar_clearmachinebox(MachineBar* self)
{
	psy_ui_combobox_clear(&self->machinebox);
	psy_table_dispose(&self->comboboxslots);
	psy_table_init(&self->comboboxslots);
	psy_table_dispose(&self->slotscombobox);
	psy_table_init(&self->slotscombobox);
}

void machinebar_oninstrumentinsert(MachineBar* self,
	psy_audio_Instruments* sender, uintptr_t slot)
{
	psy_audio_Instrument* instrument;

	machinebar_buildinstrumentlist(self);
	psy_ui_combobox_select(&self->instparambox, slot);
	instrument = psy_audio_instruments_at(sender,
		psy_audio_instrumentindex_make(0, slot));
	if (instrument) {
		psy_signal_connect(&instrument->signal_namechanged, self,
			machinebar_oninstrumentnamechanged);
	}
}

void machinebar_oninstrumentslotchanged(MachineBar* self,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex* slot)
{
	psy_ui_combobox_select(&self->instparambox, slot->subslot);	
}

void machinebar_onmachinesinsert(MachineBar* self, psy_audio_Machines* sender,
	uintptr_t slot)
{	
	machinebar_buildmachinebox(self);
	psy_ui_combobox_select(&self->machinebox,
		machinebar_comboboxindex(self,
		psy_audio_machines_selected(sender)));
}

void machinebar_onmachinesremoved(MachineBar* self, psy_audio_Machines* sender,
	uintptr_t slot)
{
	machinebar_buildmachinebox(self);
	psy_ui_combobox_select(&self->machinebox,
		machinebar_comboboxindex(self,
		psy_audio_machines_selected(sender)));
}

void machinebar_buildmachinebox(MachineBar* self)
{
	machinebar_clearmachinebox(self);
	if (psy_audio_machines_size(self->machines) == 1) {
		psy_ui_combobox_add_text(&self->machinebox, psy_ui_translate(
			"machineview.no-machines-loaded"));
		psy_ui_combobox_select(&self->machinebox, 0);
	} else if (self->machines &&
			(psy_audio_machines_size(self->machines) > 0)) {
		uintptr_t i;

		for (i = 0; i <= psy_audio_machines_maxindex(self->machines); ++i) {
			psy_audio_Machine* machine;

			machine = psy_audio_machines_at(self->machines, i);
			if (machine) {
				machinebar_insertmachine(self, i, machine);
			}
		}
		i = psy_ui_combobox_add_text(&self->machinebox,
			"------------------------------");
		psy_table_insert(&self->comboboxslots, i, (void*)psy_INDEX_INVALID);
		psy_table_insert(&self->slotscombobox, psy_INDEX_INVALID, (void*)i);
	}
}

int machinebar_insertmachine(MachineBar* self, size_t slot, psy_audio_Machine* machine)
{			
	if (slot != psy_audio_MASTER_INDEX && psy_audio_machine_info(machine) &&
			psy_audio_machine_info(machine)->shortname) {
		intptr_t comboboxindex;

		char buffer[128];
		psy_snprintf(buffer, 128, "%02X: %s", slot, 
			machine->vtable->info(machine)->shortname); 
		comboboxindex = psy_ui_combobox_add_text(&self->machinebox, buffer);
		psy_table_insert(&self->comboboxslots, comboboxindex, (void*)slot);
		psy_table_insert(&self->slotscombobox, slot, (void*)comboboxindex);
	}
	return 1;
}

void machinebar_onmachineboxselchange(MachineBar* self,
	psy_ui_Component* sender, int sel)
{	
	size_t slot;
		
	slot = (size_t)psy_table_at(&self->comboboxslots, sel);
	psy_audio_machines_select(self->machines, slot);	
}

void machinebar_onmachineselect(MachineBar* self, psy_audio_Machines* machines,
	uintptr_t slot)
{		
	psy_ui_combobox_select(&self->machinebox,
		machinebar_comboboxindex(self, slot));
	if (workspace_song(self->workspace)) {
		machinebar_updatemode(self);
		machinebar_buildparaminstbox(self);
	}
}

void machinebar_onparamselected(MachineBar* self, psy_audio_Machines* sender,
	uintptr_t slot)
{	
	psy_ui_combobox_select(&self->instparambox, slot);
}

void machinebar_buildparaminstbox(MachineBar* self)
{
	switch (machinebar_mode(self)) {		
		case MACHINEBAR_AUX:
			psy_ui_combobox_select(&self->selectinstparam, 1);
			machinebar_buildauxlist(self);
			if (psy_audio_machines_selectedmachine(self->machines)) {
				psy_ui_combobox_select(&self->instparambox,
					psy_audio_machine_auxcolumnselected(
						psy_audio_machines_selectedmachine(self->machines)));
			} else {
				psy_ui_combobox_select(&self->selectinstparam, -1);
			}
			break;
		case MACHINEBAR_PARAM:
			psy_ui_combobox_select(&self->selectinstparam, 0);
			machinebar_buildparamlist(self);
			if (psy_audio_machines_selectedmachine(self->machines)) {
				psy_ui_combobox_select(&self->instparambox,
					psy_audio_machine_paramselected(
						psy_audio_machines_selectedmachine(self->machines)));
			} else {
				psy_ui_combobox_select(&self->instparambox,
					psy_audio_machines_paramselected(self->machines));
			}
			break;
		// fallthrough
		case MACHINEBAR_INST:
		default:
			psy_ui_combobox_select(&self->selectinstparam, 1);
			machinebar_buildinstrumentlist(self);
			psy_ui_combobox_select(&self->instparambox,
				psy_audio_instruments_selected(
					&self->workspace->song->instruments).subslot);
			break;
	}	
}

void machinebar_buildparamlist(MachineBar* self)
{
	psy_audio_Machine* machine;

	psy_ui_combobox_clear(&self->instparambox);
	machine = (self->machines)
		? psy_audio_machines_at(self->machines, psy_audio_machines_selected(self->machines))
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
				if ((psy_audio_machineparam_type(param) & MPF_HEADER) ==
						MPF_HEADER) {
					psy_snprintf(text, 128, "-----%s-----", label);
				} else {
					psy_snprintf(text, 128, "%s", label);
				}
				psy_ui_combobox_add_text(&self->instparambox, text);
			}
		}
	} else {
		psy_ui_combobox_add_text(&self->instparambox, psy_ui_translate(
			"machineview.no-machine"));
	}
}

void machinebar_buildinstrumentlist(MachineBar* self)
{
	psy_audio_Instrument* instrument;
	uintptr_t slot;	
	
	psy_ui_combobox_clear(&self->instparambox);
	for (slot = 0 ; slot < 256; ++slot) {
		char text[32];

		if (instrument = psy_audio_instruments_at(
				&workspace_song(self->workspace)->instruments,
				psy_audio_instrumentindex_make(0, slot))) {
			psy_snprintf(text, 32, "%02X:%s", slot,
				psy_audio_instrument_name(instrument));
		} else {
			psy_snprintf(text, 32, "%02X:%s", slot, "");
		}
		psy_ui_combobox_add_text(&self->instparambox, text);
	}
}

void machinebar_buildauxlist(MachineBar* self)
{
	psy_audio_Machine* machine;
	psy_ui_combobox_clear(&self->instparambox);
	machine = psy_audio_machines_selectedmachine(self->machines);
	if (machine) {
		uintptr_t index;

		for (index = 0 ; index < psy_audio_machine_numauxcolumns(machine);
				++index) {
			psy_ui_combobox_add_text(&self->instparambox,
				psy_audio_machine_auxcolumnname(machine, index));
		}
	}
}

void machinebar_onselectinstparamselchange(MachineBar* self, psy_ui_Component* sender,
	intptr_t sel)
{
	machinebar_buildparaminstbox(self);
}

void machinebar_oninstparamboxselchange(MachineBar* self, psy_ui_Component* sender,
	intptr_t sel)
{
	switch (machinebar_mode(self)) {
		case MACHINEBAR_PARAM:
			psy_audio_machines_selectparam(self->machines, sel);
			break;
		case MACHINEBAR_AUX:
			psy_audio_machines_selectauxcolumn(self->machines, sel);
			break;
		case MACHINEBAR_INST:
			psy_audio_instruments_select(self->instruments,
				psy_audio_instrumentindex_make(0, sel));
			break;
		default:
			break;
	}	
}

void machinebar_on_song_changed(MachineBar* self, psy_audio_Player* sender)
{	
	psy_audio_Song* song;
	
	song = psy_audio_player_song(sender);
	if (song) {
		self->machines = &song->machines;
		self->instruments = &song->instruments;
	} else {
		self->machines = NULL;
		self->instruments = NULL;
	}
	machinebar_connectsongsignals(self);
	machinebar_buildmachinebox(self);
	machinebar_buildinstrumentlist(self);
	if (self->machines) {		
		psy_ui_combobox_select(&self->machinebox,
			machinebar_comboboxindex(self,
			psy_audio_machines_selected(self->machines)));
	}
	if (self->instruments) {
		psy_ui_combobox_select(&self->instparambox,
			psy_audio_instruments_selected(self->instruments).subslot);
	} else {
		psy_ui_combobox_select(&self->instparambox, -1);
	}
}

void machinebar_connectsongsignals(MachineBar* self)
{
	if (self->machines) {
		psy_signal_connect(&self->machines->signal_insert, self,
			machinebar_onmachinesinsert);
		psy_signal_connect(&self->machines->signal_removed, self,
			machinebar_onmachinesremoved);
		psy_signal_connect(&self->machines->signal_slotchange, self,
			machinebar_onmachineselect);
		psy_signal_connect(&self->instruments->signal_insert, self,
			machinebar_oninstrumentinsert);
		psy_signal_connect(&self->instruments->signal_slotchange, self,
			machinebar_oninstrumentslotchanged);
		psy_signal_connect(&self->machines->signal_paramselected, self,
			machinebar_onparamselected);
		machinebar_connectinstrumentsignals(self);
	}
}

void machinebar_connectinstrumentsignals(MachineBar* self)
{
	if (self->instruments) {
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
}

void machinebar_onnextmachine(MachineBar* self, psy_ui_Component* sender)
{
	if (self->machines && psy_audio_machines_selected(self->machines) > 0) {
		psy_audio_machines_select(self->machines,
			psy_audio_machines_selected(self->machines) - 1);
	}
}

void machinebar_onprevmachine(MachineBar* self, psy_ui_Component* sender)
{
	if (self->machines) {
		psy_audio_machines_select(self->machines,
			psy_audio_machines_selected(self->machines) + 1);
	}
}

void machinebar_oninstrumentnamechanged(MachineBar* self, psy_ui_Component* sender)
{
	if (self->instruments) {
		machinebar_buildinstrumentlist(self);
		psy_ui_combobox_select(&self->instparambox,
			psy_audio_instruments_selected(self->instruments).subslot);
	}
}

intptr_t machinebar_comboboxindex(MachineBar* self, uintptr_t slot)
{	
	if (psy_table_exists(&self->slotscombobox, slot)) {
		return (intptr_t)psy_table_at(&self->slotscombobox, slot);
	}
	return 0;
}

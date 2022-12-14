/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "machineviewbar.h"
/* host */
#include "wireview.h"
#include "machinewireview.h"
/* audio */
#include <exclusivelock.h>
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

/* MachineViewBar */
static void machineviewbar_on_song_changed(MachineViewBar*,
	psy_audio_Player* sender);
static void machineviewbar_connect_song(MachineViewBar*, psy_audio_Song*);
static void machineviewbar_onmixerconnectmodeclick(MachineViewBar*,
	psy_ui_Component* sender);
static void machineviewbar_updateconnectasmixersend(MachineViewBar*);
static void machineviewbar_onmachineinsert(MachineViewBar*,
	psy_audio_Machines*, uintptr_t slot);
static void machineviewbar_onmachineremoved(MachineViewBar*,
	psy_audio_Machines*, uintptr_t slot);
static psy_audio_Machines* machineviewbar_machines(MachineViewBar*);

/* implementation */
void machineviewbar_init(MachineViewBar* self, psy_ui_Component* parent,
	psy_audio_Player* player)
{
	assert(self);
	assert(player);
	
	psy_ui_component_init(machineviewbar_base(self), parent, NULL);
	self->player = player;	
	psy_ui_component_set_default_align(machineviewbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_margin_make_em(0.0, 4.0, 0.0, 0.0));
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_checkbox_init_text(&self->mixersend, machineviewbar_base(self),
		"machineview.connect-to-mixer-send-return-input");
	psy_ui_component_hide(psy_ui_checkbox_base(&self->mixersend));	
	psy_signal_connect(&self->mixersend.signal_clicked, self,
		machineviewbar_onmixerconnectmodeclick);	
	psy_signal_connect(&player->signal_song_changed, self,
		machineviewbar_on_song_changed);
	machineviewbar_connect_song(self, psy_audio_player_song(player));
}

void machineviewbar_onmixerconnectmodeclick(MachineViewBar* self,
	psy_ui_Component* sender)
{			
	if (!machineviewbar_machines(self)) {
		return;
	}
	if (psy_ui_checkbox_checked(&self->mixersend)) {
		psy_audio_machines_connect_as_mixersend(
			machineviewbar_machines(self));		
	} else {
		psy_audio_machines_connect_as_mixerinput(
			machineviewbar_machines(self));		
	}    
}

void machineviewbar_updateconnectasmixersend(MachineViewBar* self)
{	
	if (machineviewbar_machines(self) &&
			psy_audio_machines_is_connect_as_mixersend(
				machineviewbar_machines(self))) {
		psy_ui_checkbox_check(&self->mixersend);
	} else {
		psy_ui_checkbox_disable_check(&self->mixersend);
	}
}

void machineviewbar_on_song_changed(MachineViewBar* self,
	psy_audio_Player* sender)
{		
	machineviewbar_connect_song(self, psy_audio_player_song(sender));			
}

void machineviewbar_connect_song(MachineViewBar* self, psy_audio_Song* song)
{	
	if (song) {		
		psy_signal_connect(&song->machines.signal_insert, self,
			machineviewbar_onmachineinsert);
		psy_signal_connect(&song->machines.signal_removed, self,
			machineviewbar_onmachineremoved);
	}
	machineviewbar_updateconnectasmixersend(self);
}

void machineviewbar_onmachineinsert(MachineViewBar* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	if (psy_audio_machines_hasmixer(sender)) {		
		psy_ui_component_show(psy_ui_checkbox_base(&self->mixersend));
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_component_parent(&self->component)));
	}
	if (psy_audio_machines_at(sender, slot)) {
		char text[128];

		psy_snprintf(text, 128, "%s inserted at slot %u",
			psy_audio_machine_editname(psy_audio_machines_at(sender, slot)),
			(unsigned int)slot);
		// workspace_output_status(self->workspace, text);		
	}	
}

void machineviewbar_onmachineremoved(MachineViewBar* self,
	psy_audio_Machines* sender, uintptr_t slot)
{	
	char text[128];

	if (!psy_audio_machines_hasmixer(sender)) {		
		psy_ui_component_hide(psy_ui_checkbox_base(&self->mixersend));
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_component_parent(&self->component)));
	}		
	psy_snprintf(text, 128, "Machine removed from slot %u",
		(unsigned int)slot);
	// workspace_output_status(self->workspace, text);
}

psy_audio_Machines* machineviewbar_machines(MachineViewBar* self)
{
	assert(self);
	
	if (psy_audio_player_song(self->player)) {
		return psy_audio_song_machines(psy_audio_player_song(self->player));
	}
	return NULL;	
}

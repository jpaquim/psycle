// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "greet.h"

static void greet_onsize(Greet*, psy_ui_Component* sender, psy_ui_Size*);
static void greet_addstring(Greet*, const char* text);
static void greet_build(Greet*);
static void greet_buildoriginal(Greet*);
static void greet_onoriginal(Greet*, psy_ui_Component* sender);

void greet_init(Greet* self, psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent);	
	psy_signal_connect(&self->component.signal_size, self, greet_onsize);
	self->current = 1;
	psy_ui_component_settitle(&self->component, "Greetings and info");	
	psy_ui_label_init(&self->header, &self->component);	
	psy_ui_label_settextalignment(&self->header, psy_ui_ALIGNMENT_CENTER_HORIZONTAL);
	psy_ui_label_settext(&self->header, "Psycledelics, the Community, wants to thank the following people\nfor their contributions in the developement of Psycle");
	psy_ui_listbox_init(&self->greetz, &self->component);	
	psy_ui_label_init(&self->thanks, &self->component);
	psy_ui_label_settextalignment(&self->thanks, psy_ui_ALIGNMENT_LEFT);
	psy_ui_label_settext(&self->thanks, "Thanks!");
	psy_ui_button_init(&self->original, &self->component);
	psy_ui_button_settext(&self->original, "Show Original Arguru's Greetings");
	psy_signal_connect(&self->original.signal_clicked, self, greet_onoriginal);	
/*
	//Original Arguru's Greetings.
	m_greetz.AddString("Hamarr Heylen 'Hymax' [Logo design]");
	m_greetz.AddString("Raul Reales 'DJLaser'");
	m_greetz.AddString("Fco. Portillo 'Titan3_4'");
	m_greetz.AddString("Juliole");
	m_greetz.AddString("Sergio 'Zuprimo'");
	m_greetz.AddString("Oskari Tammelin [buzz creator]");
	m_greetz.AddString("Amir Geva 'Photon'");
	m_greetz.AddString("WhiteNoize");
	m_greetz.AddString("Zephod");
	m_greetz.AddString("Felix Petrescu 'WakaX'");
	m_greetz.AddString("Spiril at #goa [EFNET]");
	m_greetz.AddString("Joselex 'Americano'");
	m_greetz.AddString("Lach-ST2");
	m_greetz.AddString("DrDestral");
	m_greetz.AddString("Ic3man");
	m_greetz.AddString("Osirix");
	m_greetz.AddString("Mulder3");
	m_greetz.AddString("HexDump");
	m_greetz.AddString("Robotico");
	m_greetz.AddString("Krzysztof Foltman [FSM]");

	m_greetz.AddString("All #track at Irc-Hispano");

*/
	greet_build(self);
}

void greet_build(Greet* self)
{
	greet_addstring(self, "All the people in the Forums");
	greet_addstring(self, "All at #psycle [EFnet]");

	greet_addstring(self, "Alk [Extreme testing + Coding]");
//	greet_addstring(self, "BigTick [for his excellent VSTs]");
	greet_addstring(self, "bohan");
	greet_addstring(self, "Byte");
	greet_addstring(self, "CyanPhase [for porting VibraSynth]");
	greet_addstring(self, "dazld");
	greet_addstring(self, "dj_d [Beta testing]");
	greet_addstring(self, "DJMirage");
//	greet_addstring(self, "Drax_D [for asking to be here ;D]");
	greet_addstring(self, "Druttis [psy_audio_Machines]");
	greet_addstring(self, "Erodix");
//	greet_addstring(self, "Felix Kaplan / Spirit Of India");
//	greet_addstring(self, "Felix Petrescu 'WakaX'");
//	greet_addstring(self, "Gerwin / FreeH2o");
//	greet_addstring(self, "Imagineer");
	greet_addstring(self, "Arguru/Guru R.I.P. [We follow your steps]");
//	greet_addstring(self, "KooPer");
//	greet_addstring(self, "Krzysztof Foltman / fsm [Coding help]");
//	greet_addstring(self, "krokpitr");
	greet_addstring(self, "ksn [Psycledelics WebMaster]");
	greet_addstring(self, "lastfuture");
	greet_addstring(self, "LegoStar [asio]");
//	greet_addstring(self, "Loby [for being away]");
	greet_addstring(self, "Pikari");
	greet_addstring(self, "pooplog [psy_audio_Machines + Coding]");
	greet_addstring(self, "sampler");
	greet_addstring(self, "[SAS] SOLARiS");
	greet_addstring(self, "hugo Vinagre [Extreme testing]");
	greet_addstring(self, "TAo-AAS");
	greet_addstring(self, "TimEr [Site Graphics and more]");
//	greet_addstring(self, "Vir|us");
}

void greet_buildoriginal(Greet* self)
{
	//Original Arguru's Greetings.
	greet_addstring(self, "Hamarr Heylen 'Hymax' [Logo design]");
	greet_addstring(self, "Raul Reales 'DJLaser'");
	greet_addstring(self, "Fco. Portillo 'Titan3_4'");
	greet_addstring(self, "Juliole");
	greet_addstring(self, "Sergio 'Zuprimo'");
	greet_addstring(self, "Oskari Tammelin [buzz creator]");
	greet_addstring(self, "Amir Geva 'Photon'");
	greet_addstring(self, "WhiteNoize");
	greet_addstring(self, "Zephod");
	greet_addstring(self, "Felix Petrescu 'WakaX'");
	greet_addstring(self, "Spiril at #goa [EFNET]");
	greet_addstring(self, "Joselex 'Americano'");
	greet_addstring(self, "Lach-ST2");
	greet_addstring(self, "DrDestral");
	greet_addstring(self, "Ic3man");
	greet_addstring(self, "Osirix");
	greet_addstring(self, "Mulder3");
	greet_addstring(self, "HexDump");
	greet_addstring(self, "Robotico");
	greet_addstring(self, "Krzysztof Foltman [FSM]");

	greet_addstring(self, "All #track at Irc-Hispano");
}


void greet_addstring(Greet* self, const char* text)
{
	psy_ui_listbox_addtext(&self->greetz, text);
}

void greet_onoriginal(Greet* self, psy_ui_Component* sender)
{
	psy_ui_listbox_clear(&self->greetz);
	self->current = self->current == 0;
	if (self->current) {
		greet_build(self);
		psy_ui_button_settext(&self->original, "Show Original Arguru's Greetings");
	} else {
		greet_buildoriginal(self);
		psy_ui_button_settext(&self->original, "Show Current Greetings");
	}	
}

void greet_onsize(Greet* self, psy_ui_Component* sender, psy_ui_Size* size)
{
	psy_ui_component_setposition(&self->header.component, 0, 10, size->width, 40);
	psy_ui_component_setposition(&self->thanks.component, 0, 45, size->width - 10, 15);
	psy_ui_component_setposition(&self->greetz.component, 10, 65, size->width - 30, size->height - 100);
	psy_ui_component_setposition(&self->original.component, 0, size->height - 25, size->width, 20);
}

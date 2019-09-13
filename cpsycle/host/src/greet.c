// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "greet.h"

static void AddString(Greet* self, const char* text);

void greet_init(Greet* self, ui_component* parent)
{
	ui_margin margin = { 3, 3, 0, 3 };

	ui_component_init(&self->component, parent);	
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_setbackgroundcolor(&self->component, 0x009a887c);
	ui_component_enablealign(&self->component);
	ui_component_settitle(&self->component, "Greetings and info");	
	ui_label_init(&self->header, &self->component);
	ui_component_resize(&self->header.component, 0, 40);
	ui_component_setalign(&self->header.component, UI_ALIGN_TOP);
	ui_component_setmargin(&self->header.component, &margin);
	ui_label_settext(&self->header, "Psycledelics, the Community, wants to thank the following people\nfor their contributions in the developement of Psycle");		
	ui_listbox_init(&self->greetz, &self->component);
	ui_component_setalign(&self->greetz.component, UI_ALIGN_TOP);
	margin.top = 25;
	margin.left = 5;
	margin.right = 5;
	ui_component_setmargin(&self->greetz.component, &margin);
	ui_component_resize(&self->greetz.component, 0, 174);	
	ui_groupbox_init(&self->groupbox, &self->component);
	ui_groupbox_settext(&self->groupbox, "Thanks!");	
	ui_component_setalign(&self->groupbox.component, UI_ALIGN_FILL);
	margin.top = 45;
	margin.left = 3;
	margin.right = 3;
	ui_component_setmargin(&self->groupbox.component, &margin);
		
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
	AddString(self, "All the people in the Forums");
	AddString(self, "All at #psycle [EFnet]");

	AddString(self, "Alk [Extreme testing + Coding]");
//	AddString(self, "BigTick [for his excellent VSTs]");
	AddString(self, "bohan");
	AddString(self, "Byte");
	AddString(self, "CyanPhase [for porting VibraSynth]");
	AddString(self, "dazld");
	AddString(self, "dj_d [Beta testing]");
	AddString(self, "DJMirage");
//	AddString(self, "Drax_D [for asking to be here ;D]");
	AddString(self, "Druttis [Machines]");
	AddString(self, "Erodix");
//	AddString(self, "Felix Kaplan / Spirit Of India");
//	AddString(self, "Felix Petrescu 'WakaX'");
//	AddString(self, "Gerwin / FreeH2o");
//	AddString(self, "Imagineer");
	AddString(self, "Arguru/Guru R.I.P. [We follow your steps]");
//	AddString(self, "KooPer");
//	AddString(self, "Krzysztof Foltman / fsm [Coding help]");
//	AddString(self, "krokpitr");
	AddString(self, "ksn [Psycledelics WebMaster]");
	AddString(self, "lastfuture");
	AddString(self, "LegoStar [asio]");
//	AddString(self, "Loby [for being away]");
	AddString(self, "Pikari");
	AddString(self, "pooplog [Machines + Coding]");
	AddString(self, "sampler");
	AddString(self, "[SAS] SOLARiS");
	AddString(self, "hugo Vinagre [Extreme testing]");
	AddString(self, "TAo-AAS");
	AddString(self, "TimEr [Site Graphics and more]");
//	AddString(self, "Vir|us");
}


void AddString(Greet* self, const char* text)
{
	ui_listbox_addstring(&self->greetz, text);
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "greet.h"
/* host */
#include "styles.h"

/* prototypes */
static void greet_add_string(Greet*, const char* text);
static void greet_build(Greet*);
static void greet_build_original(Greet*);
static void greet_on_current_original(Greet*, psy_ui_Component* sender);

/* implementation */
void greet_init(Greet* self, psy_ui_Component* parent)
{
	psy_ui_component_init(greet_base(self), parent, NULL);
	psy_ui_component_set_style_type(greet_base(self), STYLE_GREET);
	self->current_greets = TRUE;
	psy_ui_component_set_title(greet_base(self), "Greetings and info");
	psy_ui_label_init(&self->headerlabel, greet_base(self));	
	psy_ui_component_set_align(psy_ui_label_base(&self->headerlabel),
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(psy_ui_label_base(&self->headerlabel),
		psy_ui_margin_make_perc(0.05, 0.2, 0.025, 0.2));	
	psy_ui_component_set_style_type(psy_ui_label_base(&self->headerlabel),
		STYLE_GREET_TOP);
	psy_ui_label_set_text(&self->headerlabel, "greetings.wantstothank");
	psy_ui_label_enable_wrap(&self->headerlabel);
	psy_ui_label_set_text_alignment(&self->headerlabel,
		psy_ui_ALIGNMENT_CENTER);	
	psy_ui_component_set_padding(psy_ui_label_base(&self->headerlabel),
		psy_ui_margin_make_em(1.0, 0.0, 1.0, 2.0));
	psy_ui_component_init_align(&self->header, greet_base(self), NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(&self->header,
		psy_ui_margin_make_perc(0.0, 0.2, 0.0, 0.2));	
	psy_ui_label_init_text(&self->thanks, &self->header, "greetings.thanks");		
	psy_ui_component_set_align(psy_ui_label_base(&self->thanks),
		psy_ui_ALIGN_LEFT);	
	psy_ui_listbox_init(&self->greetz, greet_base(self));
	psy_ui_component_set_margin(psy_ui_listbox_base(&self->greetz),
		psy_ui_margin_make_perc(0.025, 0.2, 0.05, 0.2));
	psy_ui_component_set_align(psy_ui_listbox_base(&self->greetz),
		psy_ui_ALIGN_CLIENT);
	psy_ui_button_init_connect(&self->original, greet_base(self),
		self, greet_on_current_original);
	psy_ui_button_set_text(&self->original, "greetings.showargurus");
	psy_ui_component_set_align(psy_ui_button_base(&self->original),
		psy_ui_ALIGN_BOTTOM);
	greet_build(self);
}

void greet_build(Greet* self)
{
	greet_add_string(self, "All the people in the Forums");
	greet_add_string(self, "All at #psycle [EFnet]");

	greet_add_string(self, "Alk [Extreme testing + Coding]");
/*	greet_add_string(self, "BigTick [for his excellent VSTs]"); */
	greet_add_string(self, "bohan");
	greet_add_string(self, "Byte");
	greet_add_string(self, "CyanPhase [for porting VibraSynth]");
	greet_add_string(self, "dazld");
	greet_add_string(self, "dj_d [Beta testing]");
	greet_add_string(self, "DJMirage");
/*	greet_add_string(self, "Drax_D [for asking to be here ;D]"); */
	greet_add_string(self, "Druttis [psy_audio_Machines]");
	greet_add_string(self, "Erodix");
/*	greet_add_string(self, "Felix Kaplan / Spirit Of India"); */
/*	greet_add_string(self, "Felix Petrescu 'WakaX'"); */
/*	greet_add_string(self, "Gerwin / FreeH2o"); */
/*	greet_add_string(self, "Imagineer"); */
	greet_add_string(self, "Arguru/Guru R.I.P. [We follow your steps]");
/*	greet_add_string(self, "KooPer"); */
/*	greet_add_string(self, "Krzysztof Foltman / fsm [Coding help]"); */
/*	greet_add_string(self, "krokpitr"); */
	greet_add_string(self, "ksn [Psycledelics WebMaster]");
	greet_add_string(self, "lastfuture");
	greet_add_string(self, "LegoStar [asio]");
/*	greet_add_string(self, "Loby [for being away]"); */
	greet_add_string(self, "Pikari");
	greet_add_string(self, "pooplog [psy_audio_Machines + Coding]");
	greet_add_string(self, "sampler");
	greet_add_string(self, "[SAS] SOLARiS");
	greet_add_string(self, "hugo Vinagre [Extreme testing]");
	greet_add_string(self, "TAo-AAS");
	greet_add_string(self, "TimEr [Site Graphics and more]");
/*	greet_add_string(self, "Vir|us"); */
}

void greet_build_original(Greet* self)
{	
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
	greet_add_string(self, "Hamarr Heylen 'Hymax' [Logo design]");
	greet_add_string(self, "Raul Reales 'DJLaser'");
	greet_add_string(self, "Fco. Portillo 'Titan3_4'");
	greet_add_string(self, "Juliole");
	greet_add_string(self, "Sergio 'Zuprimo'");
	greet_add_string(self, "Oskari Tammelin [buzz creator]");
	greet_add_string(self, "Amir Geva 'Photon'");
	greet_add_string(self, "WhiteNoize");
	greet_add_string(self, "Zephod");
	greet_add_string(self, "Felix Petrescu 'WakaX'");
	greet_add_string(self, "Spiril at #goa [EFNET]");
	greet_add_string(self, "Joselex 'Americano'");
	greet_add_string(self, "Lach-ST2");
	greet_add_string(self, "DrDestral");
	greet_add_string(self, "Ic3man");
	greet_add_string(self, "Osirix");
	greet_add_string(self, "Mulder3");
	greet_add_string(self, "HexDump");
	greet_add_string(self, "Robotico");
	greet_add_string(self, "Krzysztof Foltman [FSM]");

	greet_add_string(self, "All #track at Irc-Hispano");
}

void greet_add_string(Greet* self, const char* text)
{
	psy_ui_listbox_add_text(&self->greetz, text);
}

void greet_on_current_original(Greet* self, psy_ui_Component* sender)
{
	self->current_greets = !self->current_greets;
	psy_ui_listbox_clear(&self->greetz);	
	if (self->current_greets) {
		greet_build(self);
		psy_ui_button_set_text(&self->original, "greetings.showargurus");
	} else {
		greet_build_original(self);
		psy_ui_button_set_text(&self->original, "greetings.showcurrent");
	}
	psy_ui_component_align(&self->greetz.component);
	psy_ui_component_invalidate(&self->greetz.component);
}

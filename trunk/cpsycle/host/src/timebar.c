#include "../../detail/prefix.h"

#include "timebar.h"
#include <stdio.h>
#include <portable.h>

#define TIMERID_TIMERBAR 500

static void timerbar_onlesslessclicked(TimeBar* self, ui_component* sender);
static void timerbar_onlessclicked(TimeBar* self, ui_component* sender);
static void timerbar_onmoreclicked(TimeBar* self, ui_component* sender);
static void timerbar_onmoremoreclicked(TimeBar* self, ui_component* sender);
static void timerbar_ontimer(TimeBar* self, ui_component* sender, int timerid);

void timerbar_init(TimeBar* self, ui_component* parent, psy_audio_Player* player)
{				
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	self->player = player;
	self->bpm = 0;
	ui_label_init(&self->bpmdesc, &self->component);
	ui_label_settext(&self->bpmdesc, "Tempo");	
	ui_label_init(&self->bpmlabel, &self->component);
	ui_label_settext(&self->bpmlabel, "125");
	ui_label_setcharnumber(&self->bpmlabel, 8);	
	ui_button_init(&self->lessless, &self->component);
	ui_button_seticon(&self->lessless, UI_ICON_LESSLESS);		
	psy_signal_connect(&self->lessless.signal_clicked, self,
		timerbar_onlesslessclicked);	
	ui_button_init(&self->less, &self->component);
	ui_button_seticon(&self->less, UI_ICON_LESS);	
	psy_signal_connect(&self->less.signal_clicked, self,
		timerbar_onlessclicked);		
	ui_button_init(&self->more, &self->component);
	ui_button_seticon(&self->more, UI_ICON_MORE);	
	psy_signal_connect(&self->more.signal_clicked, self,
		timerbar_onmoreclicked);	
	ui_button_init(&self->moremore, &self->component);
	ui_button_seticon(&self->moremore, UI_ICON_MOREMORE);	
	psy_signal_connect(&self->moremore.signal_clicked, self,
		timerbar_onmoremoreclicked);
	{
		ui_margin margin;

		ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(0.5),
		ui_value_makepx(0), ui_value_makepx(0));
		psy_list_free(ui_components_setalign(
			ui_component_children(&self->component, 0),
			UI_ALIGN_LEFT, &margin));
	}	
	ui_component_starttimer(&self->component, TIMERID_TIMERBAR, 50);
	psy_signal_connect(&self->component.signal_timer, self,
		timerbar_ontimer);
}

void timerbar_onlesslessclicked(TimeBar* self, ui_component* sender)
{		
	player_setbpm(self->player, player_bpm(self->player) - 10);
}

void timerbar_onlessclicked(TimeBar* self, ui_component* sender)
{		
	player_setbpm(self->player, player_bpm(self->player) - 1);
}

void timerbar_onmoreclicked(TimeBar* self, ui_component* sender)
{		
	player_setbpm(self->player, player_bpm(self->player) + 1);
}

void timerbar_onmoremoreclicked(TimeBar* self, ui_component* sender)
{	
	player_setbpm(self->player, player_bpm(self->player) + 10);
}

void timerbar_ontimer(TimeBar* self, ui_component* sender, int timerid)
{		
	if (self->bpm != player_bpm(self->player)) {
		char txt[20];

		self->bpm = self->player->sequencer.bpm;
		psy_snprintf(txt, 10, "%.2f", self->bpm);
		ui_label_settext(&self->bpmlabel, txt);
	}
}

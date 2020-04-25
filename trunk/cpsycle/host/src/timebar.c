#include "../../detail/prefix.h"

#include "timebar.h"
#include <stdio.h>
#include "../../detail/portable.h"

#define TIMERID_TIMERBAR 500

static void timerbar_onlesslessclicked(TimeBar*, psy_ui_Component* sender);
static void timerbar_onlessclicked(TimeBar*, psy_ui_Component* sender);
static void timerbar_onmoreclicked(TimeBar*, psy_ui_Component* sender);
static void timerbar_onmoremoreclicked(TimeBar*, psy_ui_Component* sender);
static void timerbar_ontimer(TimeBar*, psy_ui_Component* sender, int timerid);
static void timerbar_offsetbpm(TimeBar*, psy_dsp_beat_t bpm);

void timerbar_init(TimeBar* self, psy_ui_Component* parent, psy_audio_Player* player)
{				
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	self->player = player;
	self->bpm = 0;
	psy_ui_label_init(&self->bpmdesc, &self->component);
	psy_ui_label_settext(&self->bpmdesc, "Tempo");	
	psy_ui_label_init(&self->bpmlabel, &self->component);
	psy_ui_label_settext(&self->bpmlabel, "125");
	psy_ui_label_setcharnumber(&self->bpmlabel, 8);	
	psy_ui_button_init(&self->lessless, &self->component);
	psy_ui_button_seticon(&self->lessless, psy_ui_ICON_LESSLESS);		
	psy_signal_connect(&self->lessless.signal_clicked, self,
		timerbar_onlesslessclicked);	
	psy_ui_button_init(&self->less, &self->component);
	psy_ui_button_seticon(&self->less, psy_ui_ICON_LESS);	
	psy_signal_connect(&self->less.signal_clicked, self,
		timerbar_onlessclicked);		
	psy_ui_button_init(&self->more, &self->component);
	psy_ui_button_seticon(&self->more, psy_ui_ICON_MORE);	
	psy_signal_connect(&self->more.signal_clicked, self,
		timerbar_onmoreclicked);	
	psy_ui_button_init(&self->moremore, &self->component);
	psy_ui_button_seticon(&self->moremore, psy_ui_ICON_MOREMORE);	
	psy_signal_connect(&self->moremore.signal_clicked, self,
		timerbar_onmoremoreclicked);
	{
		psy_ui_Margin margin;

		psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
			psy_ui_value_makeew(0.5), psy_ui_value_makepx(0),
			psy_ui_value_makepx(0));
		psy_list_free(psy_ui_components_setalign(
			psy_ui_component_children(&self->component, 0),
			psy_ui_ALIGN_LEFT, &margin));
	}	
	psy_ui_component_starttimer(&self->component, TIMERID_TIMERBAR, 50);
	psy_signal_connect(&self->component.signal_timer, self,
		timerbar_ontimer);
}

void timerbar_onlesslessclicked(TimeBar* self, psy_ui_Component* sender)
{		
	timerbar_offsetbpm(self, -10);
}

void timerbar_onlessclicked(TimeBar* self, psy_ui_Component* sender)
{		
	timerbar_offsetbpm(self, -1);
}

void timerbar_onmoreclicked(TimeBar* self, psy_ui_Component* sender)
{			
	timerbar_offsetbpm(self, 1);
}

void timerbar_onmoremoreclicked(TimeBar* self, psy_ui_Component* sender)
{		
	timerbar_offsetbpm(self, 10);
}

void timerbar_offsetbpm(TimeBar* self, psy_dsp_beat_t delta)
{	
	if (self->player && player_song(self->player)) {
		player_setbpm(self->player, player_bpm(self->player) + delta);
		psy_audio_song_setbpm(player_song(self->player),
			player_bpm(self->player));
	}
}

void timerbar_ontimer(TimeBar* self, psy_ui_Component* sender, int timerid)
{		
	if (self->bpm != player_bpm(self->player)) {
		char txt[20];

		self->bpm = self->player->sequencer.bpm;
		psy_snprintf(txt, 10, "%.2f", self->bpm);
		psy_ui_label_settext(&self->bpmlabel, txt);
	}
}


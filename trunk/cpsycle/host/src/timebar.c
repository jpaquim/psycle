#include "timebar.h"
#include <stdio.h>

static void OnLessLessClicked(TimeBar* self, ui_component* sender);
static void OnLessClicked(TimeBar* self, ui_component* sender);
static void OnMoreClicked(TimeBar* self, ui_component* sender);
static void OnMoreMoreClicked(TimeBar* self, ui_component* sender);
static void OnTimer(TimeBar* self, ui_component* sender, int timerid);

void InitTimeBar(TimeBar* self, ui_component* parent, Player* player)
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
	signal_connect(&self->lessless.signal_clicked, self, OnLessLessClicked);	
	ui_button_init(&self->less, &self->component);
	ui_button_seticon(&self->less, UI_ICON_LESS);	
	signal_connect(&self->less.signal_clicked, self, OnLessClicked);		
	ui_button_init(&self->more, &self->component);
	ui_button_seticon(&self->more, UI_ICON_MORE);	
	signal_connect(&self->more.signal_clicked, self, OnMoreClicked);	
	ui_button_init(&self->moremore, &self->component);
	ui_button_seticon(&self->moremore, UI_ICON_MOREMORE);	
	signal_connect(&self->moremore.signal_clicked, self, OnMoreMoreClicked);
	{
		ui_margin margin = { 0, 3, 3, 0 };

		ui_components_setalign(ui_component_children(&self->component, 0),
			UI_ALIGN_LEFT, &margin);
	}	
	SetTimer(self->component.hwnd, 500, 50, 0);
	signal_connect(&self->component.signal_timer, self, OnTimer);
}

void OnLessLessClicked(TimeBar* self, ui_component* sender)
{		
	player_setbpm(self->player, self->player->sequencer.bpm - 10);
}

void OnLessClicked(TimeBar* self, ui_component* sender)
{		
	player_setbpm(self->player, self->player->sequencer.bpm - 1);
}

void OnMoreClicked(TimeBar* self, ui_component* sender)
{		
	player_setbpm(self->player, self->player->sequencer.bpm + 1);
}

void OnMoreMoreClicked(TimeBar* self, ui_component* sender)
{	
	player_setbpm(self->player, self->player->sequencer.bpm + 10);
}

void OnTimer(TimeBar* self, ui_component* sender, int timerid)
{	
	char buffer[20];
	if (self->bpm != self->player->sequencer.bpm) {
		_snprintf(buffer, 10, "%.2f", self->player->sequencer.bpm);
		ui_label_settext(&self->bpmlabel, buffer);
		self->bpm = self->player->sequencer.bpm;
	}
}

#include "timebar.h"
#include <stdio.h>

static void OnLessLessClicked(TimeBar* self, ui_component* sender);
static void OnLessClicked(TimeBar* self, ui_component* sender);
static void OnMoreClicked(TimeBar* self, ui_component* sender);
static void OnMoreMoreClicked(TimeBar* self, ui_component* sender);
static void OnTimer(TimeBar* self, ui_component* sender, int timerid);

void InitTimeBar(TimeBar* self, ui_component* parent, Player* player)
{			
	ui_margin margin = { 0, 3, 0, 0 };

	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	self->player = player;
	self->bpm = 0;
	ui_label_init(&self->bpmdesclabel, &self->component);
	ui_label_settext(&self->bpmdesclabel, "Tempo");	
	ui_label_init(&self->bpmlabel, &self->component);
	ui_label_settext(&self->bpmlabel, "125");	
	ui_button_init(&self->lessbutton, &self->component);
	ui_button_settext(&self->lessbutton, "<");	
	signal_connect(&self->lessbutton.signal_clicked, self, OnLessClicked);	
	ui_button_init(&self->lesslessbutton, &self->component);
	ui_button_settext(&self->lesslessbutton, "<<");	
	signal_connect(&self->lesslessbutton.signal_clicked, self, OnLessLessClicked);	
	ui_button_init(&self->morebutton, &self->component);
	ui_button_settext(&self->morebutton, ">");	
	signal_connect(&self->morebutton.signal_clicked, self, OnMoreClicked);	
	ui_button_init(&self->moremorebutton, &self->component);
	ui_button_settext(&self->moremorebutton, ">>");	
	signal_connect(&self->moremorebutton.signal_clicked, self, OnMoreMoreClicked);		
	ui_component_resize(&self->bpmdesclabel.component, 50, 0);	
	ui_component_resize(&self->lesslessbutton.component, 20, 0);		
	ui_component_resize(&self->lessbutton.component, 20, 0);		
	ui_component_resize(&self->bpmlabel.component, 40, 0);	
	ui_component_resize(&self->morebutton.component, 20, 0);		
	ui_component_resize(&self->moremorebutton.component, 20, 0);	
	{
		List* p;
		for (p = ui_component_children(&self->component, 0); p != 0; p = p->next)
		{
			ui_component_setalign((ui_component*)p->entry, UI_ALIGN_LEFT);
			ui_component_setmargin((ui_component*)p->entry, &margin);
		}
	}
	SetTimer(self->component.hwnd, 500, 50, 0);	signal_connect(&self->component.signal_timer, self, OnTimer);
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

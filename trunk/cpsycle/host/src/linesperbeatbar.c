#include "linesperbeatbar.h"
#include <stdio.h>

static void OnLessClicked(LinesPerBeatBar* self, ui_component* sender);
static void OnMoreClicked(LinesPerBeatBar* self, ui_component* sender);
static void OnTimer(LinesPerBeatBar* self, ui_component* sender, int timerid);

void InitLinesPerBeatBar(LinesPerBeatBar* self, ui_component* parent, Player* player)
{				
	ui_component_init(&self->component, parent);		
	self->player = player;	
	ui_label_init(&self->lpbdesclabel, &self->component);
	ui_label_settext(&self->lpbdesclabel, "Lines per beat");	
	ui_label_init(&self->lpblabel, &self->component);
	ui_label_settext(&self->lpblabel, "4");	
	ui_button_init(&self->lessbutton, &self->component);
	ui_button_settext(&self->lessbutton, "<");	
	signal_connect(&self->lessbutton.signal_clicked, self, OnLessClicked);		
	
	ui_button_init(&self->morebutton, &self->component);
	ui_button_settext(&self->morebutton, ">");	
	signal_connect(&self->morebutton.signal_clicked, self, OnMoreClicked);	
	ui_component_move(&self->lpbdesclabel.component, 0, 0);
	ui_component_resize(&self->lpbdesclabel.component, 50, 20);	
	ui_component_move(&self->lessbutton.component, 80, 0);
	ui_component_resize(&self->lessbutton.component, 20, 20);	
	ui_component_move(&self->lpblabel.component, 105, 0);
	ui_component_resize(&self->lpblabel.component, 40, 20);
	ui_component_move(&self->morebutton.component, 150, 0);
	ui_component_resize(&self->morebutton.component, 20, 20);		
	self->lpb = 0;
	signal_connect(&self->component.signal_timer, self, OnTimer);
	SetTimer(self->component.hwnd, 500, 50, 0);
}

void OnLessClicked(LinesPerBeatBar* self, ui_component* sender)
{		
	player_setlpb(self->player, self->player->lpb - 1);
}

void OnMoreClicked(LinesPerBeatBar* self, ui_component* sender)
{		
	player_setlpb(self->player, self->player->lpb + 1);
}

void OnTimer(LinesPerBeatBar* self, ui_component* sender, int timerid)
{	
	char buffer[20];
	if (self->lpb != self->player->lpb) {
		_snprintf(buffer, 10, "%2d", self->player->lpb);
		ui_label_settext(&self->lpblabel, buffer);
		self->lpb = self->player->lpb;
	}
}

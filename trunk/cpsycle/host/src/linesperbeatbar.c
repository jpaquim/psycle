#include "linesperbeatbar.h"
#include <stdio.h>

static void OnLessClicked(LinesPerBeatBar*, ui_component* sender);
static void OnMoreClicked(LinesPerBeatBar*, ui_component* sender);
static void OnTimer(LinesPerBeatBar*, ui_component* sender, int timerid);

void InitLinesPerBeatBar(LinesPerBeatBar* self, ui_component* parent, Player* player)
{				
	ui_margin margin = { 0, 3, 0, 0 };

	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	self->player = player;		
	ui_label_init(&self->lpbdesclabel, &self->component);	
	self->lpbdesclabel.component.debugflag = 20;
	ui_label_settext(&self->lpbdesclabel, "Lines per beat");	
	ui_label_init(&self->lpblabel, &self->component);
	ui_label_settext(&self->lpblabel, "4");	
	ui_button_init(&self->lessbutton, &self->component);
	ui_button_settext(&self->lessbutton, "<");	
	signal_connect(&self->lessbutton.signal_clicked, self, OnLessClicked);		
	ui_button_init(&self->morebutton, &self->component);
	ui_button_settext(&self->morebutton, ">");	
	signal_connect(&self->morebutton.signal_clicked, self, OnMoreClicked);		
	ui_component_resize(&self->lpbdesclabel.component, 70, 0);		
	ui_component_resize(&self->lessbutton.component, 20, 0);		
	ui_component_resize(&self->lpblabel.component, 30, 0);	
	ui_component_resize(&self->morebutton.component, 20, 0);
	self->lpb = 0;
	signal_connect(&self->component.signal_timer, self, OnTimer);
	SetTimer(self->component.hwnd, 500, 50, 0);
	{
		List* p;
		for (p = ui_component_children(&self->component, 0); p != 0; p = p->next)
		{
			ui_component_setalign((ui_component*)p->entry, UI_ALIGN_LEFT);
			ui_component_setmargin((ui_component*)p->entry, &margin);
		}
	}
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

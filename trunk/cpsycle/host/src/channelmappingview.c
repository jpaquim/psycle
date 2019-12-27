// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "channelmappingview.h"
#include <stdio.h>
#include <portable.h>

static void pinedit_ondraw(PinEdit*, psy_ui_Component* sender, psy_ui_Graphics*);
static void pinedit_drawpinsockets(PinEdit*, psy_ui_Graphics*);
static void pinedit_drawpinconnections(PinEdit*, psy_ui_Graphics*);

void pinedit_init(PinEdit* self, psy_ui_Component* parent, psy_audio_Wire wire,
	Workspace* workspace)
{					
	self->wire = wire;
	self->lineheight = 12;
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	psy_signal_connect(&self->component.signal_draw, self, pinedit_ondraw);	
}

void pinedit_ondraw(PinEdit* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{		
	pinedit_drawpinsockets(self, g);
	pinedit_drawpinconnections(self, g);	
}

void pinedit_drawpinsockets(PinEdit* self, psy_ui_Graphics* g)
{
	psy_audio_Machine* machine;
	uintptr_t numsrcpins;
	uintptr_t numdstpins;
	
	machine = machines_at(&self->workspace->song->machines, self->wire.src);
	if (machine) {
		machine = machines_at(&self->workspace->song->machines, self->wire.dst);
		numsrcpins = machine->vtable->numoutputs(machine);
		if (machine) {
			uintptr_t p;
			ui_rectangle r;
			ui_size pin = { 8, 8 };
			ui_size size;
			int cpy;
			int centerx;
			char text[128];

			size = ui_component_size(&self->component);
			centerx = (size.width - 100) / 2;
			numdstpins = machine->vtable->numinputs(machine);
			ui_setcolor(g, 0x00CACACA);
			ui_setbackgroundmode(g, TRANSPARENT);
			ui_settextcolor(g, 0x00CACACA);
			for (p = 0; p < numsrcpins; ++p) {
				cpy = p * self->lineheight;
				psy_snprintf(text, 40, "%.02d", p);
				text[39] = '\0';
				ui_textout(g, centerx, cpy, text, strlen(text));
				ui_setrectangle(&r, centerx + 20, cpy + (self->lineheight - pin.height) / 2, pin.width, pin.height);
				ui_drawrectangle(g, r);
				ui_setrectangle(&r, centerx + 100, cpy + (self->lineheight - pin.height) / 2, pin.width, pin.height);
				ui_drawrectangle(g, r);
				ui_textout(g, centerx + 120, cpy, text, strlen(text));
			}			
		}
	}	
}

void pinedit_drawpinconnections(PinEdit* self, psy_ui_Graphics* g)
{
	psy_audio_Connections* connections;
	psy_audio_WireSocketEntry* input;
	psy_List* pinpair;
	ui_size pin = { 8, 8 };
	ui_size size;
	int centerx;

	size = ui_component_size(&self->component);
	centerx = (size.width - 100) / 2;
	connections = &self->workspace->song->machines.connections;
	input = connection_input(connections, self->wire.src, self->wire.dst);
	for (pinpair = input->mapping; pinpair != 0; pinpair = pinpair->next) {
		psy_audio_PinConnection* pinconnection;		

		pinconnection = (psy_audio_PinConnection*)pinpair->entry;
		ui_setcolor(g, 0x00CACACA);		
		ui_drawline(g, centerx + 20 + pin.width / 2, (int)(pinconnection->src * self->lineheight + self->lineheight / 2),
			centerx + 100 + pin.width / 2, (int)(pinconnection->dst * self->lineheight + self->lineheight / 2));
	}
}

void channelmappingview_init(ChannelMappingView* self, psy_ui_Component* parent,
	psy_audio_Wire wire, Workspace* workspace)
{
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);	
	ui_component_init(&self->buttongroup, &self->component);
	ui_component_enablealign(&self->buttongroup);
	ui_component_setalign(&self->buttongroup, UI_ALIGN_RIGHT);
	ui_button_init(&self->autowire, &self->buttongroup);
	ui_component_setalign(&self->autowire.component, UI_ALIGN_TOP);
	ui_button_settext(&self->autowire, "AutoWire");
	ui_button_init(&self->unselectall, &self->buttongroup);
	ui_component_setalign(&self->unselectall.component, UI_ALIGN_TOP);
	ui_button_settext(&self->unselectall, "Unselect all");
	pinedit_init(&self->pinedit, &self->component, wire, workspace);
	ui_component_setalign(&self->pinedit.component, UI_ALIGN_CLIENT);
}

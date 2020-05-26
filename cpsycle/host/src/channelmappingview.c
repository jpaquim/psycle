// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "channelmappingview.h"

#include <exclusivelock.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"

static void pinedit_ondraw(PinEdit*, psy_ui_Graphics*);
static void pinedit_drawpinsockets(PinEdit*, psy_ui_Graphics*);
static void pinedit_drawpinconnections(PinEdit*, psy_ui_Graphics*);
static void pinedit_drawconnectiondrag(PinEdit*, psy_ui_Graphics*);
static void pinedit_drawpinoutput(PinEdit*, psy_ui_Graphics*, uintptr_t pin);
static void pinedit_drawpininput(PinEdit*, psy_ui_Graphics*, uintptr_t pin);
static psy_ui_Rectangle pinedit_pinposition_output(PinEdit* self, uintptr_t pin);
static psy_ui_Rectangle pinedit_pinposition_input(PinEdit* self, uintptr_t pin);
static void pinedit_onmousedown(PinEdit*, psy_ui_MouseEvent*);
static void pinedit_onmousemove(PinEdit*, psy_ui_MouseEvent*);
static void pinedit_onmouseup(PinEdit*, psy_ui_MouseEvent*);
static psy_List* pinedit_hittest_wire(PinEdit*, int x, int y);
static bool pinedit_hittest_pin(PinEdit*, int x, int y, uintptr_t* pin, bool* isout);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(PinEdit* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_ondraw)pinedit_ondraw;
		vtable.onmousedown = (psy_ui_fp_onmousedown)pinedit_onmousedown;
		vtable.onmousemove = (psy_ui_fp_onmousedown)pinedit_onmousemove;
		vtable.onmouseup = (psy_ui_fp_onmousedown)pinedit_onmouseup;
		vtable_initialized = 1;
	}
}

void pinedit_init(PinEdit* self, psy_ui_Component* parent, psy_audio_Wire wire,
	Workspace* workspace)
{					
	self->wire = wire;
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);
	self->dragmode = PINEDIT_DRAG_NONE;
	self->mx = 0;
	self->my = 0;
}

void pinedit_ondraw(PinEdit* self, psy_ui_Graphics* g)
{		
	pinedit_drawpinsockets(self, g);
	pinedit_drawpinconnections(self, g);	
	pinedit_drawconnectiondrag(self, g);
}

void pinedit_drawpinsockets(PinEdit* self, psy_ui_Graphics* g)
{
	psy_audio_Machine* srcmachine;
	psy_audio_Machine* dstmachine;
	
	srcmachine = machines_at(&self->workspace->song->machines, self->wire.src);
	dstmachine = machines_at(&self->workspace->song->machines, self->wire.dst);
	if (srcmachine && dstmachine) {	
		uintptr_t p;
		uintptr_t numsrcpins;
		uintptr_t numdstpins;

		psy_ui_setcolor(g, 0x00CACACA);
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_settextcolor(g, 0x00CACACA);
		numsrcpins = psy_audio_machine_numoutputs(srcmachine);
		for (p = 0; p < numsrcpins; ++p) {
			pinedit_drawpinoutput(self, g, p);
		}
		numdstpins = psy_audio_machine_numinputs(dstmachine);
		for (p = 0; p < numdstpins; ++p) {
			pinedit_drawpininput(self, g, p);
		}					
	}	
}

void pinedit_drawpinoutput(PinEdit* self, psy_ui_Graphics* g, uintptr_t pin)
{
	int cpy;
	char text[128];
	int height;
	int height2;
	int centery;
	int pinwidth;
	int pinheight;
	int numchars;
	psy_ui_TextMetric tm;
	psy_ui_Size size;
	psy_ui_Rectangle r;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size(&self->component);
	height = tm.tmHeight * 2;
	height2 = height / 2;
	centery = (height - tm.tmHeight) / 2;
	cpy = (int)(pin * height);
	numchars = 10;
	pinwidth = (int)(tm.tmAveCharWidth * 1.5);
	pinheight = (int)(tm.tmHeight * 0.75);
	psy_snprintf(text, 40, "Out %.02d", (int)pin);
	psy_ui_textout(g, 0, cpy + centery, text, strlen(text));
	psy_ui_setrectangle(&r, pinwidth + tm.tmAveCharWidth * numchars, cpy + height2 - pinheight / 2, pinwidth, pinheight);
	psy_ui_drawrectangle(g, r);
}

void pinedit_drawpininput(PinEdit* self, psy_ui_Graphics* g, uintptr_t pin)
{
	int cpy;
	char text[128];
	int height;
	int height2;
	int centery;
	int pinwidth;
	int pinheight;
	int numchars;
	psy_ui_TextMetric tm;
	psy_ui_Size size;
	psy_ui_Rectangle r;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size(&self->component);
	height = tm.tmHeight * 2;
	height2 = height / 2;
	centery = (height - tm.tmHeight) / 2;
	cpy = (int)(pin * height);
	numchars = 10;
	pinwidth = (int)(tm.tmAveCharWidth * 1.5);
	pinheight = (int)(tm.tmHeight * 0.75);
	psy_snprintf(text, 40, "In %.02d", (int)pin);
	psy_ui_textout(g, size.width - tm.tmAveCharWidth * 8, cpy + centery, text, strlen(text));
	psy_ui_setrectangle(&r, size.width - (pinwidth + tm.tmAveCharWidth * numchars),
		cpy + height2 - pinheight / 2, pinwidth, pinheight);
	psy_ui_drawrectangle(g, r);
}

psy_ui_Rectangle pinedit_pinposition_output(PinEdit* self, uintptr_t pin)
{
	int cpy;
	int height;
	int height2;
	int centery;
	int pinwidth;
	int pinheight;
	int numchars;
	psy_ui_TextMetric tm;
	psy_ui_Size size;
	psy_ui_Rectangle r;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size(&self->component);
	height = tm.tmHeight * 2;
	height2 = height / 2;
	centery = (height - tm.tmHeight) / 2;
	cpy = (int)(pin * height);
	numchars = 10;
	pinwidth = (int)(tm.tmAveCharWidth * 1.5);
	pinheight = (int)(tm.tmHeight * 0.75);	
	psy_ui_setrectangle(&r, pinwidth + tm.tmAveCharWidth * numchars,
		cpy + height2 - pinheight / 2, pinwidth, pinheight);
	return r;
}

psy_ui_Rectangle pinedit_pinposition_input(PinEdit* self, uintptr_t pin)
{
	int cpy;
	int height;
	int height2;
	int centery;
	int pinwidth;
	int pinheight;
	int numchars;
	psy_ui_TextMetric tm;
	psy_ui_Size size;
	psy_ui_Rectangle r;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size(&self->component);
	height = tm.tmHeight * 2;
	height2 = height / 2;
	centery = (height - tm.tmHeight) / 2;
	cpy = (int)(pin * height);
	numchars = 10;
	pinwidth = (int)(tm.tmAveCharWidth * 1.5);
	pinheight = (int)(tm.tmHeight * 0.75);
	psy_ui_setrectangle(&r, size.width - (pinwidth + tm.tmAveCharWidth * numchars),
		cpy + height2 - pinheight / 2, pinwidth, pinheight);
	return r;
}

void pinedit_drawpinconnections(PinEdit* self, psy_ui_Graphics* g)
{
	psy_audio_Connections* connections;
	psy_audio_WireSocketEntry* input;
	psy_List* pinpair;

	connections = &self->workspace->song->machines.connections;
	input = connection_input(connections, self->wire.src, self->wire.dst);
	for (pinpair = input->mapping; pinpair != 0; pinpair = pinpair->next) {
		psy_audio_PinConnection* pinconnection;
		psy_ui_Rectangle out;
		psy_ui_Rectangle in;

		pinconnection = (psy_audio_PinConnection*)(pinpair->entry);
		out = pinedit_pinposition_output(self, pinconnection->src);
		in = pinedit_pinposition_input(self, pinconnection->dst);
		
		psy_ui_setcolor(g, 0x00CACACA);		
		psy_ui_drawline(g, out.left + (out.right - out.left) / 2,
			out.top + (out.bottom - out.top) / 2,
			in.left + (in.right - in.left) / 2,
			in.top + (in.bottom - in.top) / 2);
	}
}

void pinedit_drawconnectiondrag(PinEdit* self, psy_ui_Graphics* g)
{
	if (self->dragmode == PINEDIT_DRAG_SRC) {
		psy_ui_Rectangle in;

		in = pinedit_pinposition_input(self, self->drag_dst);
		psy_ui_setcolor(g, 0x00CACACA);
		psy_ui_drawline(g, self->mx, self->my,
			in.left + (in.right - in.left) / 2,
			in.top + (in.bottom - in.top) / 2);
	} else
	if (self->dragmode == PINEDIT_DRAG_DST) {
		psy_ui_Rectangle out;

		out = pinedit_pinposition_output(self, self->drag_src);
		psy_ui_setcolor(g, 0x00CACACA);
		psy_ui_drawline(g, self->mx, self->my,
			out.left + (out.right - out.left) / 2,
			out.top + (out.bottom - out.top) / 2);
	} else
	if (self->dragmode == PINEDIT_DRAG_NEW_SRC) {
		psy_ui_Rectangle in;

		in = pinedit_pinposition_input(self, self->drag_dst);
		psy_ui_setcolor(g, 0x00CACACA);
		psy_ui_drawline(g, self->mx, self->my,
			in.left + (in.right - in.left) / 2,
			in.top + (in.bottom - in.top) / 2);
	} else
	if (self->dragmode == PINEDIT_DRAG_NEW_DST) {
		psy_ui_Rectangle out;

		out = pinedit_pinposition_output(self, self->drag_src);
		psy_ui_setcolor(g, 0x00CACACA);
		psy_ui_drawline(g, self->mx, self->my,
			out.left + (out.right - out.left) / 2,
			out.top + (out.bottom - out.top) / 2);
	}
}

void pinedit_onmousedown(PinEdit* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 2) {		
		psy_List* pinpair;		
		
		pinpair = pinedit_hittest_wire(self, ev->x, ev->y);
		if (pinpair) {
			psy_audio_Connections* connections;
			psy_audio_WireSocketEntry* input;

			psy_audio_exclusivelock_enter();
			connections = &self->workspace->song->machines.connections;
			input = connection_input(connections, self->wire.src, self->wire.dst);
			free(pinpair->entry);
			psy_list_remove(&input->mapping, pinpair);			
			psy_audio_exclusivelock_leave();
			psy_ui_component_invalidate(&self->component);
		}
		self->dragmode = PINEDIT_DRAG_NONE;
	} else
	if (ev->button == 1) {
		psy_List* pinpair;
		
		pinpair = pinedit_hittest_wire(self, ev->x, ev->y);
		self->mx = ev->x;
		self->my = ev->y;
		if (pinpair) {
			psy_ui_Size size;
			psy_audio_PinConnection* pinconnection;
			
			pinconnection = (psy_audio_PinConnection*)(pinpair->entry);
			self->pindragnode = pinpair;
			self->drag_dst = pinconnection->dst;
			self->drag_src = pinconnection->src;
			size = psy_ui_component_size(&self->component);
			if (ev->x < size.width / 2) {
				self->dragmode = PINEDIT_DRAG_SRC;
			} else {
				self->dragmode = PINEDIT_DRAG_DST;				
			}			
		} else {
			uintptr_t newpin;
			psy_ui_Size size;
			psy_ui_TextMetric tm;
			
			tm = psy_ui_component_textmetric(&self->component);
			newpin = ev->y / (tm.tmHeight * 2);
			size = psy_ui_component_size(&self->component);
			if (ev->x < size.width / 2) {
					self->dragmode = PINEDIT_DRAG_NEW_DST;
					self->drag_src = newpin;
			} else {
				self->dragmode = PINEDIT_DRAG_NEW_SRC;
				self->drag_dst = newpin;
			}			
		}
	}
}

psy_List* pinedit_hittest_wire(PinEdit* self, int x, int y)
{
	psy_audio_Connections* connections;
	psy_audio_WireSocketEntry* input;
	psy_List* pinpair;
	
	connections = &self->workspace->song->machines.connections;
	input = connection_input(connections, self->wire.src, self->wire.dst);
	for (pinpair = input->mapping; pinpair != 0; pinpair = pinpair->next) {
		psy_audio_PinConnection* pinconnection;
		psy_ui_Rectangle out;
		psy_ui_Rectangle in;
		psy_ui_Rectangle rect_mouse;
		int d = 2;

		pinconnection = (psy_audio_PinConnection*)(pinpair->entry);
		out = pinedit_pinposition_output(self, pinconnection->src);
		in = pinedit_pinposition_input(self, pinconnection->dst);
		psy_ui_setrectangle(&rect_mouse, x - d, y - d, d * 2, d * 2);
		if (psy_ui_rectangle_intersect_segment(&rect_mouse,
			out.left + (out.right - out.left) / 2,
			out.top + (out.bottom - out.top) / 2,
			in.left + (in.right - in.left) / 2,
			in.top + (in.bottom - in.top) / 2)) {
			return pinpair;
		}
	}
	return NULL;
}

bool pinedit_hittest_pin(PinEdit* self, int x, int y, uintptr_t* pin, bool* isout)
{
	psy_audio_Connections* connections;
	psy_audio_WireSocketEntry* input;
	psy_List* pinpair;

	connections = &self->workspace->song->machines.connections;
	input = connection_input(connections, self->wire.src, self->wire.dst);
	for (pinpair = input->mapping; pinpair != 0; pinpair = pinpair->next) {
		psy_audio_PinConnection* pinconnection;
		psy_ui_Rectangle out;
		psy_ui_Rectangle in;

		pinconnection = (psy_audio_PinConnection*)(pinpair->entry);
		out = pinedit_pinposition_output(self, pinconnection->src);
		in = pinedit_pinposition_input(self, pinconnection->dst);
		if (psy_ui_rectangle_intersect(&out, x, y)) {
			*pin = pinconnection->src;
			*isout = TRUE;
			return TRUE;
		}
		if (psy_ui_rectangle_intersect(&in, x, y)) {
			*pin = pinconnection->dst;
			*isout = FALSE;
			return TRUE;
		}	
	}	
	return FALSE;
}

void pinedit_onmousemove(PinEdit* self, psy_ui_MouseEvent* ev)
{
	if (self->dragmode != PINEDIT_DRAG_NONE) {
		self->mx = ev->x;
		self->my = ev->y;
		psy_ui_component_invalidate(&self->component);
	}
}

void pinedit_onmouseup(PinEdit* self, psy_ui_MouseEvent* ev)
{
	uintptr_t newpin;
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	bool isout;

	tm = psy_ui_component_textmetric(&self->component);
	newpin = ev->y / (tm.tmHeight * 2);
	size = psy_ui_component_size(&self->component);
	isout = (ev->x < size.width / 2);

	if (self->dragmode == PINEDIT_DRAG_SRC) {		
		if (isout) {
			psy_audio_PinConnection* pinpair;

			psy_audio_exclusivelock_enter();
			pinpair = (psy_audio_PinConnection*)self->pindragnode->entry;
			if (pinpair) {
				pinpair->src = newpin;
			}
			psy_audio_exclusivelock_leave();
		}
	} else 
	if (self->dragmode == PINEDIT_DRAG_DST) {
		if (!isout) {			
			psy_audio_PinConnection* pinpair;

			psy_audio_exclusivelock_enter();			
			pinpair = (psy_audio_PinConnection*)self->pindragnode->entry;
			if (pinpair) {
				pinpair->dst = newpin;
			}
			psy_audio_exclusivelock_leave();
		}
	} else
	if (self->dragmode == PINEDIT_DRAG_NEW_SRC) {
		if (isout) {
			psy_audio_Connections* connections;
			psy_audio_WireSocketEntry* input;
			psy_audio_PinConnection* pinpair;

			psy_audio_exclusivelock_enter();
			connections = &self->workspace->song->machines.connections;
			input = connection_input(connections, self->wire.src, self->wire.dst);
			pinpair = (psy_audio_PinConnection*)malloc(sizeof(psy_audio_PinConnection));
			if (pinpair) {
				pinpair->src = newpin;
				pinpair->dst = self->drag_dst;
				psy_list_append(&input->mapping, pinpair);
			}
			psy_audio_exclusivelock_leave();
		}
	} else
	if (self->dragmode == PINEDIT_DRAG_NEW_DST) {
		if (!isout) {
			psy_audio_Connections* connections;
			psy_audio_WireSocketEntry* input;
			psy_audio_PinConnection* pinpair;

			psy_audio_exclusivelock_enter();
			connections = &self->workspace->song->machines.connections;
			input = connection_input(connections, self->wire.src, self->wire.dst);
			pinpair = (psy_audio_PinConnection*)malloc(sizeof(psy_audio_PinConnection));
			if (pinpair) {
				pinpair->src = self->drag_src;
				pinpair->dst = newpin;
				psy_list_append(&input->mapping, pinpair);
			}
			psy_audio_exclusivelock_leave();
		}
	}
	self->dragmode = PINEDIT_DRAG_NONE;	
	psy_ui_component_invalidate(&self->component);
}

static void channelmappingview_onautowire(ChannelMappingView*, psy_ui_Component* sender);
static void channelmappingview_unselectallautowire(ChannelMappingView*, psy_ui_Component* sender);

void channelmappingview_init(ChannelMappingView* self, psy_ui_Component* parent,
	psy_audio_Wire wire, Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_init(&self->buttongroup, &self->component);
	psy_ui_component_enablealign(&self->buttongroup);
	psy_ui_component_setalign(&self->buttongroup, psy_ui_ALIGN_RIGHT);
	psy_ui_button_init(&self->autowire, &self->buttongroup);
	psy_ui_component_setalign(&self->autowire.component, psy_ui_ALIGN_TOP);
	psy_ui_button_settext(&self->autowire, "AutoWire");
	psy_signal_connect(&self->autowire.signal_clicked, self, channelmappingview_onautowire);
	psy_ui_button_init(&self->unselectall, &self->buttongroup);
	psy_ui_component_setalign(&self->unselectall.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->unselectall.signal_clicked, self,
		channelmappingview_unselectallautowire);
	psy_ui_button_settext(&self->unselectall, "Unselect all");
	pinedit_init(&self->pinedit, &self->component, wire, workspace);
	psy_ui_component_setalign(&self->pinedit.component, psy_ui_ALIGN_CLIENT);	
}

void channelmappingview_onautowire(ChannelMappingView* self, psy_ui_Component* sender)
{
	if (self->workspace->song) {
		psy_audio_Connections* connections;
		psy_audio_WireSocketEntry* input;

		connections = &self->workspace->song->machines.connections;
		input = connection_input(connections, self->pinedit.wire.src, self->pinedit.wire.dst);
		if (input) {
			psy_List* p;
			psy_audio_Machine* srcmachine;
			psy_audio_Machine* dstmachine;

			psy_audio_exclusivelock_enter();
			for (p = input->mapping; p != NULL; p = p->next) {
				free(p->entry);
			}
			psy_list_free(input->mapping);
			input->mapping = NULL;
			srcmachine = machines_at(&self->workspace->song->machines, self->pinedit.wire.src);
			dstmachine = machines_at(&self->workspace->song->machines, self->pinedit.wire.dst);
			if (srcmachine && dstmachine) {
				uintptr_t maxchannels;
				uintptr_t channel;

				maxchannels = psy_audio_machine_numoutputs(srcmachine);
				if (psy_audio_machine_numinputs(dstmachine) < maxchannels) {
					maxchannels = psy_audio_machine_numinputs(srcmachine);
				}
				
				for (channel = 0; channel < maxchannels; ++channel) {
					psy_audio_PinConnection* pinpair;

					pinpair = (psy_audio_PinConnection*)malloc(sizeof(psy_audio_PinConnection));
					if (pinpair) {
						pinpair->src = channel;
						pinpair->dst = channel;
						psy_list_append(&input->mapping, pinpair);
					}
				}			
			}
			psy_audio_exclusivelock_leave();
			psy_ui_component_invalidate(&self->pinedit.component);
		}
	}
}

void channelmappingview_unselectallautowire(ChannelMappingView* self, psy_ui_Component* sender)
{
	if (self->workspace->song) {
		psy_audio_Connections* connections;
		psy_audio_WireSocketEntry* input;

		connections = &self->workspace->song->machines.connections;
		input = connection_input(connections, self->pinedit.wire.src, self->pinedit.wire.dst);
		if (input) {
			psy_List* p;

			psy_audio_exclusivelock_enter();
			for (p = input->mapping; p != NULL; p = p->next) {
				free(p->entry);
			}
			psy_list_free(input->mapping);
			input->mapping = NULL;
			psy_audio_exclusivelock_leave();				
			psy_ui_component_invalidate(&self->pinedit.component);			
		}
	}
}

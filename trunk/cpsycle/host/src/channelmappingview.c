// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "channelmappingview.h"

#include <exclusivelock.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"

static void pinedit_ondraw(PinEdit*, psy_ui_Graphics*);
static void pinedit_drawsockets(PinEdit*, psy_ui_Graphics*);
static void pinedit_drawconnections(PinEdit*, psy_ui_Graphics*);
static void pinedit_drawdrag(PinEdit*, psy_ui_Graphics*);
static void pinedit_drawpinoutput(PinEdit*, psy_ui_Graphics*, uintptr_t pin);
static void pinedit_drawpininput(PinEdit*, psy_ui_Graphics*, uintptr_t pin);
static psy_ui_Rectangle pinedit_pinposition_output(PinEdit*, uintptr_t pin);
static psy_ui_Rectangle pinedit_pinposition_input(PinEdit*, uintptr_t pin);
static void pinedit_onmousedown(PinEdit*, psy_ui_MouseEvent*);
static void pinedit_onmousemove(PinEdit*, psy_ui_MouseEvent*);
static void pinedit_onmouseup(PinEdit*, psy_ui_MouseEvent*);
static psy_List* pinedit_hittest_wire(PinEdit*, double x, double y);
static bool pinedit_screentopin(PinEdit*, double x, double y, uintptr_t* pin, bool* isout);
static psy_audio_PinMapping* pinedit_mapping(PinEdit*);
static uintptr_t pinedit_numinputs(PinEdit*);
static uintptr_t pinedit_numoutputs(PinEdit*);

static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PinEdit* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_component_ondraw)pinedit_ondraw;
		vtable.onmousedown = (psy_ui_fp_component_onmousedown)pinedit_onmousedown;
		vtable.onmousemove = (psy_ui_fp_component_onmousedown)pinedit_onmousemove;
		vtable.onmouseup = (psy_ui_fp_component_onmousedown)pinedit_onmouseup;
		vtable_initialized = TRUE;
	}
}

void pinedit_init(PinEdit* self, psy_ui_Component* parent, psy_audio_Wire wire,
	Workspace* workspace)
{					
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);
	self->wire = wire;
	self->workspace = workspace;
	self->dragmode = PINEDIT_DRAG_NONE;
	self->mx = 0;
	self->my = 0;
	self->pincolour = psy_ui_colour_make(0x00444444);
	self->wirecolour = psy_ui_colour_make(0x00999999);
}

void pinedit_ondraw(PinEdit* self, psy_ui_Graphics* g)
{		
	pinedit_drawsockets(self, g);
	pinedit_drawconnections(self, g);	
	pinedit_drawdrag(self, g);
}

void pinedit_drawsockets(PinEdit* self, psy_ui_Graphics* g)
{
	psy_audio_Machine* srcmachine;
	psy_audio_Machine* dstmachine;
	
	srcmachine = psy_audio_machines_at(&workspace_song(self->workspace)->machines, self->wire.src);
	dstmachine = psy_audio_machines_at(&workspace_song(self->workspace)->machines, self->wire.dst);
	if (srcmachine && dstmachine) {	
		uintptr_t p;
		uintptr_t numsrcpins;
		uintptr_t numdstpins;

		psy_ui_setcolour(g, self->pincolour);
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_settextcolour(g, self->pincolour);
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
	double cpy;
	char text[128];
	double height;
	double height2;
	double centery;
	double pinwidth;
	double pinheight;
	double numchars;
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
	double cpy;
	char text[128];
	double height;
	double height2;
	double centery;
	double pinwidth;
	double pinheight;
	double numchars;
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
	psy_ui_textout(g, psy_ui_value_px(&size.width, &tm) - tm.tmAveCharWidth * 8, cpy + centery, text, strlen(text));
	psy_ui_setrectangle(&r, psy_ui_value_px(&size.width, &tm) - (pinwidth + tm.tmAveCharWidth * numchars),
		cpy + height2 - pinheight / 2, pinwidth, pinheight);
	psy_ui_drawrectangle(g, r);
}

psy_ui_Rectangle pinedit_pinposition_output(PinEdit* self, uintptr_t pin)
{
	double cpy;
	double height;
	double height2;
	double centery;
	double pinwidth;
	double pinheight;
	double numchars;
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
	double cpy;
	double height;
	double height2;
	double centery;
	double pinwidth;
	double pinheight;
	double numchars;
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
	psy_ui_setrectangle(&r, psy_ui_value_px(&size.width, &tm) - (pinwidth + tm.tmAveCharWidth * numchars),
		cpy + height2 - pinheight / 2, pinwidth, pinheight);
	return r;
}

void pinedit_drawconnections(PinEdit* self, psy_ui_Graphics* g)
{
	psy_audio_PinMapping* mapping;
	psy_List* pinpair;

	mapping = pinedit_mapping(self);
	for (pinpair = mapping->container; pinpair != 0; pinpair = pinpair->next) {
		psy_audio_PinConnection* pinconnection;
		psy_ui_Rectangle out;
		psy_ui_Rectangle in;
		psy_ui_RealPoint p0;
		psy_ui_RealPoint c1;
		psy_ui_RealPoint c2;
		psy_ui_RealPoint p1;

		pinconnection = (psy_audio_PinConnection*)(pinpair->entry);
		out = pinedit_pinposition_output(self, pinconnection->src);
		in = pinedit_pinposition_input(self, pinconnection->dst);		
		psy_ui_setcolour(g, self->wirecolour);
		p0 = psy_ui_realpoint_make(out.left + (out.right - out.left) / 2,
			out.top + (out.bottom - out.top) / 2);
		p1 = psy_ui_realpoint_make(in.left + (in.right - in.left) / 2,
			in.top + (in.bottom - in.top) / 2);
		c1.x = p0.x + (p1.x - p0.x) / 3;
		c1.y = p0.y;
		c2.x = p1.x - (p1.x - p0.x) / 3;
		c2.y = p1.y;
		psy_ui_moveto(g, p0);
		psy_ui_curveto(g, c1, c2, p1);		
	}
}

void pinedit_drawdrag(PinEdit* self, psy_ui_Graphics* g)
{
	if (self->dragmode == PINEDIT_DRAG_SRC ||
		self->dragmode == PINEDIT_DRAG_NEW_SRC) {
		psy_ui_Rectangle in;

		in = pinedit_pinposition_input(self, self->drag_dst);
		psy_ui_setcolour(g, self->wirecolour);
		psy_ui_drawline(g, self->mx, self->my,
			in.left + (in.right - in.left) / 2,
			in.top + (in.bottom - in.top) / 2);
	} else
	if (self->dragmode == PINEDIT_DRAG_DST ||
		self->dragmode == PINEDIT_DRAG_NEW_DST) {
		psy_ui_Rectangle out;

		out = pinedit_pinposition_output(self, self->drag_src);
		psy_ui_setcolour(g, self->wirecolour);
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
			psy_audio_PinMapping* mapping;

			mapping = pinedit_mapping(self);
			psy_audio_exclusivelock_enter();
			free(pinpair->entry);
			psy_list_remove(&mapping->container, pinpair);
			psy_audio_exclusivelock_leave();
			psy_ui_component_invalidate(&self->component);
		}
		self->dragmode = PINEDIT_DRAG_NONE;
	} else
	if (ev->button == 1) {
		psy_List* pinpair;
		uintptr_t newpin;
		bool isout;
		
		self->mx = ev->x;
		self->my = ev->y;
		if (pinedit_screentopin(self, ev->x, ev->y, &newpin, &isout)) {			
			if (isout) {
				self->dragmode = PINEDIT_DRAG_NEW_DST;
				self->drag_src = newpin;
			} else {
				self->dragmode = PINEDIT_DRAG_NEW_SRC;
				self->drag_dst = newpin;
			}
		} else {
			pinpair = pinedit_hittest_wire(self, ev->x, ev->y);
			if (pinpair) {
				psy_ui_Size size;
				psy_ui_TextMetric tm;
				psy_audio_PinConnection* pinconnection;

				pinconnection = (psy_audio_PinConnection*)(pinpair->entry);
				self->drag_dst = pinconnection->dst;
				self->drag_src = pinconnection->src;
				size = psy_ui_component_size(&self->component);
				tm = psy_ui_component_textmetric(&self->component);
				if (ev->x < psy_ui_value_px(&size.width, &tm) / 2) {
					self->dragmode = PINEDIT_DRAG_SRC;
				} else {
					self->dragmode = PINEDIT_DRAG_DST;
				}
			} 
		}
	}
}

psy_List* pinedit_hittest_wire(PinEdit* self, double x, double y)
{
	psy_audio_Connections* connections;
	psy_audio_WireSocket* input;
	psy_List* pinpair;
	
	connections = &workspace_song(self->workspace)->machines.connections;
	input = psy_audio_connections_input(connections, self->wire);
	for (pinpair = input->mapping.container; pinpair != 0; pinpair = pinpair->next) {
		psy_audio_PinConnection* pinconnection;
		psy_ui_Rectangle out;
		psy_ui_Rectangle in;
		psy_ui_Rectangle rect_mouse;
		double d = 4;

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

bool pinedit_screentopin(PinEdit* self, double x, double y, uintptr_t* pin, bool* isout)
{
	psy_ui_Rectangle r;
	uintptr_t row;
	double rowheight;
	double cpy;
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);	
	rowheight = tm.tmHeight * 2;
	row = (uintptr_t)(psy_max(0.0, y) / rowheight);
	cpy = row * rowheight;
	r = pinedit_pinposition_output(self, 0);
	psy_ui_rectangle_move(&r, 0, cpy);	
	if (psy_ui_rectangle_intersect(&r, x, y)) {
		if (row >= pinedit_numoutputs(self)) {
			return FALSE;
		}
		*isout = TRUE;
		*pin = row;
		return TRUE;
	}
	r = pinedit_pinposition_input(self, 0);
	psy_ui_rectangle_move(&r, 0, cpy);
	if (psy_ui_rectangle_intersect(&r, x, y)) {
		if (row >= pinedit_numinputs(self)) {
			return FALSE;
		}
		*isout = FALSE;
		*pin = row;
		return TRUE;
	}
	*pin = psy_INDEX_INVALID;
	*isout = FALSE;
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
	if (self->dragmode != PINEDIT_DRAG_NONE) {
		uintptr_t newpin;
		bool isout;

		if (pinedit_screentopin(self, ev->x, ev->y, &newpin, &isout)) {						
			switch (self->dragmode) {
				case PINEDIT_DRAG_SRC:
					if (isout) {
						psy_audio_PinMapping* mapping;

						mapping = pinedit_mapping(self);
						psy_audio_exclusivelock_enter();
						psy_audio_pinmapping_disconnect(mapping,
							self->drag_src, self->drag_dst);
						psy_audio_pinmapping_connect(mapping, newpin,
							self->drag_dst);
						psy_audio_exclusivelock_leave();
					}
				break;
				case PINEDIT_DRAG_DST:
					if (!isout) {
						psy_audio_PinMapping* mapping;

						mapping = pinedit_mapping(self);
						psy_audio_exclusivelock_enter();
						psy_audio_pinmapping_disconnect(mapping,
							self->drag_src, self->drag_dst);
						psy_audio_pinmapping_connect(mapping, self->drag_src,
							newpin);
						psy_audio_exclusivelock_leave();
					}
				break;
				case PINEDIT_DRAG_NEW_SRC:
					if (isout) {
						psy_audio_PinMapping* mapping;

						mapping = pinedit_mapping(self);
						psy_audio_exclusivelock_enter();
						psy_audio_pinmapping_connect(mapping, newpin,
							self->drag_dst);
						psy_audio_exclusivelock_leave();
					}
				break;
				case PINEDIT_DRAG_NEW_DST:
					if (!isout) {
						psy_audio_PinMapping* mapping;

						mapping = pinedit_mapping(self);
						psy_audio_exclusivelock_enter();
						psy_audio_pinmapping_connect(mapping, self->drag_src,
							newpin);
						psy_audio_exclusivelock_leave();
					}
				break;
				default:
				break;
			}			
		}
		self->dragmode = PINEDIT_DRAG_NONE;
		psy_ui_component_invalidate(&self->component);
	}
}

void pinedit_autowire(PinEdit* self)
{
	if (workspace_song(self->workspace)) {
		psy_audio_PinMapping* mapping;

		mapping = pinedit_mapping(self);
		if (mapping) {
			psy_audio_Machine* srcmachine;
			psy_audio_Machine* dstmachine;

			srcmachine = psy_audio_machines_at(&workspace_song(self->workspace)->machines,
				self->wire.src);
			dstmachine = psy_audio_machines_at(&workspace_song(self->workspace)->machines,
				self->wire.dst);
			if (srcmachine && dstmachine) {
				uintptr_t maxchannels;

				maxchannels = psy_audio_machine_numoutputs(srcmachine);
				if (psy_audio_machine_numinputs(dstmachine) < maxchannels) {
					maxchannels = psy_audio_machine_numinputs(srcmachine);
				}
				psy_audio_exclusivelock_enter();
				psy_audio_pinmapping_autowire(mapping, maxchannels);
				psy_audio_exclusivelock_leave();
			}
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void pinedit_unselectall(PinEdit* self)
{
	if (workspace_song(self->workspace)) {
		psy_audio_PinMapping* mapping;

		mapping = pinedit_mapping(self);
		if (mapping) {
			psy_audio_exclusivelock_enter();
			psy_audio_pinmapping_clear(mapping);
			psy_audio_exclusivelock_leave();
			psy_ui_component_invalidate(&self->component);
		}
	}
}

psy_audio_PinMapping* pinedit_mapping(PinEdit* self)
{
	psy_audio_Connections* connections;
	psy_audio_WireSocket* input;

	connections = &workspace_song(self->workspace)->machines.connections;
	input = psy_audio_connections_input(connections, self->wire);
	return (input)
		? &input->mapping
		: NULL;
}

uintptr_t pinedit_numinputs(PinEdit* self)
{	
	psy_audio_Machine* dstmachine;
	
	dstmachine = psy_audio_machines_at(&workspace_song(self->workspace)->machines,
		self->wire.dst);
	return (dstmachine)
		? psy_audio_machine_numinputs(dstmachine)
		: 0;
}

uintptr_t pinedit_numoutputs(PinEdit* self)
{
	psy_audio_Machine* srcmachine;

	srcmachine = psy_audio_machines_at(&workspace_song(self->workspace)->machines,
		self->wire.src);
	return (srcmachine)
		? psy_audio_machine_numoutputs(srcmachine)
		: 0;
}

// ChannelMappingView
// prototypes
static void channelmappingview_onautowire(ChannelMappingView*,
	psy_ui_Component* sender);
static void channelmappingview_unselectall(ChannelMappingView*,
	psy_ui_Component* sender);

void channelmappingview_init(ChannelMappingView* self, psy_ui_Component* parent,
	psy_audio_Wire wire, Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent);	
	self->workspace = workspace;
	psy_ui_component_init(&self->buttongroup, &self->component);
	psy_ui_component_setdefaultalign(&self->buttongroup, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_label_init_text(&self->help, &self->component,
		"channelmapping.remove-connection-with-right-click");
	psy_ui_label_settextalignment(&self->help, psy_ui_ALIGNMENT_LEFT);
	psy_ui_component_setalign(&self->help.component, psy_ui_ALIGN_BOTTOM);		
	psy_ui_component_setalign(&self->buttongroup, psy_ui_ALIGN_RIGHT);
	psy_ui_button_init_connect(&self->autowire, &self->buttongroup, self,
		channelmappingview_onautowire);
	psy_ui_button_settext(&self->autowire, "channelmapping.autowire");
	psy_ui_button_settextalignment(&self->autowire, psy_ui_ALIGNMENT_LEFT);
	psy_ui_button_init_connect(&self->unselectall, &self->buttongroup,
		self, channelmappingview_unselectall);
	psy_ui_button_settext(&self->unselectall, "channelmapping.unselect-all");
	psy_ui_button_settextalignment(&self->unselectall, psy_ui_ALIGNMENT_LEFT);	
	pinedit_init(&self->pinedit, &self->component, wire, workspace);
	psy_ui_component_setalign(&self->pinedit.component, psy_ui_ALIGN_CLIENT);
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0));
	psy_ui_component_setmargin(&self->pinedit.component, &margin);
	psy_ui_component_setmargin(&self->help.component, &margin);	
}

void channelmappingview_onautowire(ChannelMappingView* self,
	psy_ui_Component* sender)
{
	pinedit_autowire(&self->pinedit);
}

void channelmappingview_unselectall(ChannelMappingView* self,
	psy_ui_Component* sender)
{
	pinedit_unselectall(&self->pinedit);
}

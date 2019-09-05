// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "instrumentsview.h"

static void OnDraw(InstrumentsView* self, ui_component* sender, ui_graphics* g);
static void OnSize(InstrumentsView* self, ui_component* sender, int width, int height);
static void Align(InstrumentsView* self);
static void OnInstrumentsViewSize(InstrumentsView* self, ui_component* sender, int width, int height);
static void OnTabBarChange(InstrumentsView* self, ui_component* sender, int tabindex);
static void OnShow(InstrumentsView* self, ui_component* sender);
static void OnHide(InstrumentsView* self, ui_component* sender);

void InitInstrumentsView(InstrumentsView* self, ui_component* parent, Player* player)
{
	ui_component_init(&self->component, parent);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_size, self, OnSize);
	InitSampulseInstrumentView(&self->sampulseview, &self->component, player);
	InitSamplerInstrumentView(&self->samplerview, &self->component, player);	
	InitTabBar(&self->tabbar, parent);
	ui_component_move(&self->tabbar.component, 600, 50);
	ui_component_resize(&self->tabbar.component, 160, 20);
	ui_component_hide(&self->tabbar.component);	
	tabbar_append(&self->tabbar, "Sampulse");
	tabbar_append(&self->tabbar, "Sampler PS1");
	self->tabbar.tabwidth = 70;
	self->tabbar.selected = 0;
	ui_component_hide(&self->samplerview.component);
	ui_component_show(&self->sampulseview.component);	
	signal_connect(&self->tabbar.signal_change, self, OnTabBarChange);
	signal_connect(&self->component.signal_show, self, OnShow);
	signal_connect(&self->component.signal_hide, self, OnHide);
}

void OnInstrumentsViewSize(InstrumentsView* self, ui_component* sender, int width, int height)
{
	ui_size size = ui_component_size(&self->component);
	ui_component_resize(&self->sampulseview.component, size.width, size.height);
	ui_component_resize(&self->samplerview.component, size.width, size.height);
}

void OnTabBarChange(InstrumentsView* self, ui_component* sender, int tabindex)
{	
	switch (tabindex) {
		case 0:
			ui_component_hide(&self->samplerview.component);
			ui_component_show(&self->sampulseview.component);					
		break;
		case 1:			
			ui_component_hide(&self->sampulseview.component);
			ui_component_show(&self->samplerview.component);			
		break;
		default:;
		break;
	};
	Align(self);
}

void OnShow(InstrumentsView* self, ui_component* sender)
{	
	ui_component_show(&self->tabbar.component);
}

void OnHide(InstrumentsView* self, ui_component* sender)
{
	ui_component_hide(&self->tabbar.component);
}

void OnDraw(InstrumentsView* self, ui_component* sender, ui_graphics* g)
{
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);	
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_drawsolidrectangle(g, r, 0x009a887c);
}

void OnSize(InstrumentsView* self, ui_component* sender, int width, int height)
{
	Align(self);
}

void Align(InstrumentsView* self)
{
	ui_size size = ui_component_size(&self->component);
	ui_component_resize(&self->sampulseview.component, size.width, size.height);
	ui_component_setfocus(&self->sampulseview.notemapedit.component);
	ui_component_resize(&self->samplerview.component, size.width, size.height);
	ui_component_setfocus(&self->samplerview.component);
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "settingsview.h"
#include <driver.h>
#include <stdio.h>

static int cpy;
static int cpx;
static int fillchoice;

static void OnDraw(SettingsView* self, ui_component* sender, ui_graphics* g);
static int OnPropertiesEnum(SettingsView* self, Properties* property, int level);
static int OnPropertiesHitTestEnum(SettingsView* self, Properties* property, int level);
static void OnKeyDown(SettingsView* self, ui_component* sender, int keycode, int keydata);
static void OnMouseDown(SettingsView* self, ui_component* sender, int x, int y, int button);
static void OnMouseDoubleClick(SettingsView* self, ui_component* sender, int x, int y, int button);
static void OnEditChange(SettingsView* self, ui_edit* sender);

void InitSettingsView(SettingsView* self, ui_component* parent, Properties* properties)
{			
	ui_component_init(self, &self->component, parent);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_keydown, self, OnKeyDown);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_mousedoubleclick, self, OnMouseDoubleClick);
	ui_component_move(&self->component, 10, 10);
	ui_component_resize(&self->component, 400, 400);
	self->properties = properties;	
	self->selected = 0;
	ui_edit_init(&self->edit, &self->component, 0);
	self->edit.component.events.target = self;
	ui_component_hide(&self->edit.component);
	signal_connect(&self->edit.signal_change, self, OnEditChange);
}

void OnDraw(SettingsView* self, ui_component* sender, ui_graphics* g)
{	   	
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);    
    self->g = g;		
    ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_setbackgroundcolor(g, 0x009a887c);
	ui_settextcolor(g, 0x00000000);
	ui_drawsolidrectangle(g, r, 0x009a887c);    
	cpx = 0;
	cpy = 0;
	self->lastlevel = 0;
	properties_enumarate(self->properties, self, OnPropertiesEnum);
}

int OnPropertiesEnum(SettingsView* self, Properties* property, int level)
{		
	if (property->item.key && property->item.typ == PROPERTY_TYP_CHOICE) {
		fillchoice = 1;
		self->lastlevel = level;
		ui_textout(self->g, 20 + cpx, 20 + cpy, property->item.key, strlen(property->item.key));
		return 1;
	} else
	if (self->lastlevel < level) {
		cpx += 20;
	} else
	if (self->lastlevel > level) {
		cpx -= 20;
		fillchoice = 0;
	}
	self->lastlevel = level;
	if (property->item.key) {				
		if (fillchoice) {
			ui_rectangle r;
			ui_textout(self->g, 220, 20 + cpy, property->item.key, strlen(property->item.key));			
			r.left = 200;
			r.top = 20 + cpy;
			r.right = 210;
			r.bottom = 30 + cpy;
			ui_drawrectangle(self->g, r);
		} else
		{
			ui_textout(self->g, 20 + cpx, 20 + cpy, property->item.key, strlen(property->item.key));
			if (property->item.typ == PROPERTY_TYP_STRING) {
				if (self->selected == property) {
					ui_setbackgroundcolor(self->g, 0x00Aa988c);
				}
				ui_textout(self->g, 200, 20 + cpy, property->item.value.s, strlen(property->item.value.s));
				ui_setbackgroundcolor(self->g, 0x009a887c);
			} else
			if (property->item.typ == PROPERTY_TYP_INTEGER) {
				char buf[20];
				int c;
				c =_snprintf(buf, 20, "%d", property->item.value.i);
				ui_textout(self->g, 200, 20 + cpy, buf, c);
			}				
		}
		cpy += 20;
	}
	return 1;
}

void OnKeyDown(SettingsView* self, ui_component* sender, int keycode, int keydata)
{	
}

void OnMouseDown(SettingsView* self, ui_component* sender, int x, int y, int button)
{
	ui_component_hide(&self->edit.component);
	self->selected = 0;
	self->mx = x;
	self->my = y;
	self->lastlevel = 0;
	cpx = 0;
	cpy = 0;
	properties_enumarate(self->properties, self, OnPropertiesHitTestEnum);
	ui_invalidate(&self->component);
}

int Intersects(ui_rectangle* r, int x, int y)
{
	return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
}

int OnPropertiesHitTestEnum(SettingsView* self, Properties* property, int level)
{
	if (property->item.key && property->item.typ == PROPERTY_TYP_CHOICE) {
		fillchoice = 1;
		self->lastlevel = level;
	//	ui_textout(self->g, 20 + cpx, 20 + cpy, property->item.key, strlen(property->item.key));
		return 1;
	} else
	if (self->lastlevel < level) {
		cpx += 20;
	} else
	if (self->lastlevel > level) {
		cpx -= 20;
		fillchoice = 0;
	}
	self->lastlevel = level;
	if (property->item.key) {				
		if (fillchoice) {
			ui_rectangle r;
	//		ui_textout(self->g, 220, 20 + cpy, property->item.key, strlen(property->item.key));			
			r.left = 200;
			r.top = 20 + cpy;
			r.right = 210;
			r.bottom = 30 + cpy;
	//		ui_drawrectangle(self->g, r);
		} else
		{
	//		ui_textout(self->g, 20 + cpx, 20 + cpy, property->item.key, strlen(property->item.key));
			if (property->item.typ == PROPERTY_TYP_STRING) {
				ui_rectangle r;
				ui_setrectangle(&r, 200, 20 + cpy, 300, 20);
				if (Intersects(&r, self->mx, self->my)) {
					self->selected = property;
					self->selrect = r;
					return 0;
				}
	//			ui_textout(self->g, 200, 20 + cpy, property->item.value.s, strlen(property->item.value.s));
			} else
			if (property->item.typ == PROPERTY_TYP_INTEGER) {
	//			char buf[20];
	//			int c;
	//			c =_snprintf(buf, 20, "%d", property->item.value.i);
	//			ui_textout(self->g, 200, 20 + cpy, buf, c);
			}				
		}
		cpy += 20;
	}
	return 1;
}

void OnMouseDoubleClick(SettingsView* self, ui_component* sender, int x, int y, int button)
{
	if (self->selected) {
		ui_component_move(&self->edit.component, self->selrect.left, self->selrect.top);
		ui_component_resize(&self->edit.component, self->selrect.right - self->selrect.left, 
			self->selrect.bottom - self->selrect.top);
		ui_edit_settext(&self->edit, self->selected->item.value.s);
		ui_component_show(&self->edit.component);
	}
}

void OnEditChange(SettingsView* self, ui_edit* sender)
{
	if (self->selected) {
		properties_write_string(self->selected, self->selected->item.key, ui_edit_text(&self->edit));
	}
}


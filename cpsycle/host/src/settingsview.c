// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "settingsview.h"
#include <stdio.h>

static void OnDraw(SettingsView* self, ui_component* sender, ui_graphics* g);
static int OnPropertiesEnum(SettingsView* self, Properties* property, int level);
static int OnPropertiesHitTestEnum(SettingsView* self, Properties* property, int level);
static void OnKeyDown(SettingsView* self, ui_component* sender, int keycode, int keydata);
static void OnMouseDown(SettingsView* self, ui_component* sender, int x, int y, int button);
static void OnMouseDoubleClick(SettingsView* self, ui_component* sender, int x, int y, int button);
static void OnEditChange(SettingsView* self, ui_edit* sender);
static void OnDestroy(SettingsView* self, ui_component* sender);

void InitSettingsView(SettingsView* self, ui_component* parent, Properties* properties)
{			
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_keydown, self, OnKeyDown);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_mousedoubleclick, self, OnMouseDoubleClick);	
	self->properties = properties;	
	self->selected = 0;
	ui_edit_init(&self->edit, &self->component, 0);	
	ui_component_hide(&self->edit.component);
	signal_connect(&self->edit.signal_change, self, OnEditChange);
	self->cpx = 0;
	self->cpy = 0;
	self->fillchoice = 0;
	signal_init(&self->signal_changed);
}

void OnDestroy(SettingsView* self, ui_component* sender)
{
	signal_dispose(&self->signal_changed);
}

void OnDraw(SettingsView* self, ui_component* sender, ui_graphics* g)
{	
    self->g = g;		    	
	ui_setcolor(g, 0x00CACACA);
	ui_settextcolor(g, 0x00CACACA);
	ui_setbackgroundmode(g, TRANSPARENT);
	self->cpx = 0;
	self->cpy = 0;
	self->lastlevel = 0;
	properties_enumerate(self->properties, self, OnPropertiesEnum);
}

int OnPropertiesEnum(SettingsView* self, Properties* property, int level)
{		
	if (property->item.key && property->item.typ == PROPERTY_TYP_CHOICE) {
		self->fillchoice = 1;
		self->lastlevel = level;
		ui_textout(self->g, 20 + self->cpx, 20 + self->cpy, property->item.key, strlen(property->item.key));
		return 1;
	} else
	if (self->lastlevel < level) {
		self->cpx += 20;
	} else
	if (self->lastlevel > level) {
		self->cpx -= 20;
		self->fillchoice = 0;
	}
	self->lastlevel = level;
	if (property->item.key) {				
		if (self->fillchoice) {
			ui_rectangle r;
			ui_textout(self->g, 220, 20 + self->cpy, property->item.key, strlen(property->item.key));
			r.left = 200;
			r.top = 20 + self->cpy;
			r.right = 210;
			r.bottom = 30 + self->cpy;
			ui_drawrectangle(self->g, r);
		} else
		{
			ui_textout(self->g, 20 + self->cpx, 20 + self->cpy, property->item.key, strlen(property->item.key));
			if (property->item.typ == PROPERTY_TYP_BOOL) {
				ui_rectangle r;				
				r.left = 200;
				r.top = 20 + self->cpy;
				r.right = 211;
				r.bottom = 34 + self->cpy;				
				if (property->item.value.i == 1) {
					ui_textout(self->g, 203, 21 + self->cpy - 2,
						"x", strlen("x"));			
				}
				ui_drawrectangle(self->g, r);
			} else
			if (property->item.typ == PROPERTY_TYP_STRING) {
				if (self->selected == property) {					
					ui_setbackgroundmode(self->g, OPAQUE);
					ui_setbackgroundcolor(self->g, 0x009B7800);
					ui_settextcolor(self->g, 0x00FFFFFF);
				}
				ui_textout(self->g, 200, 20 + self->cpy, property->item.value.s, strlen(property->item.value.s));
				ui_setbackgroundcolor(self->g, 0x003E3E3E);
				ui_setbackgroundmode(self->g, TRANSPARENT);
				ui_settextcolor(self->g, 0x00CACACA);
			//	if (property->item.hint == PROPERTY_HINT_EDITDIR) {
			//		ui_textout(self->g, 500, 20 + self->cpy, "...", strlen("..."));
			//	}
			} else
			if (property->item.typ == PROPERTY_TYP_INTEGER) {
				char buf[20];
				int c;
				c =_snprintf(buf, 20, "%d", property->item.value.i);
				ui_textout(self->g, 200, 20 + self->cpy, buf, c);
			}				
		}
		self->cpy += 20;
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
	self->cpx = 0;
	self->cpy = 0;
	properties_enumerate(self->properties, self, OnPropertiesHitTestEnum);
	ui_invalidate(&self->component);
}

int Intersects(ui_rectangle* r, int x, int y)
{
	return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
}

int OnPropertiesHitTestEnum(SettingsView* self, Properties* property, int level)
{
	if (property->item.key && property->item.typ == PROPERTY_TYP_CHOICE) {
		self->fillchoice = 1;
		self->lastlevel = level;
	//	ui_textout(self->g, 20 + self->cpx, 20 + self->cpy, property->item.key, strlen(property->item.key));
		return 1;
	} else
	if (self->lastlevel < level) {
		self->cpx += 20;
	} else
	if (self->lastlevel > level) {
		self->cpx -= 20;
		self->fillchoice = 0;
	}
	self->lastlevel = level;
	if (property->item.key) {				
		if (self->fillchoice) {
			ui_rectangle r;
	//		ui_textout(self->g, 220, 20 + self->cpy, property->item.key, strlen(property->item.key));			
			r.left = 200;
			r.top = 20 + self->cpy;
			r.right = 210;
			r.bottom = 30 + self->cpy;
	//		ui_drawrectangle(self->g, r);
		} else
		{
	//		ui_textout(self->g, 20 + self->cpx, 20 + self->cpy, property->item.key, strlen(property->item.key));
			if (property->item.typ == PROPERTY_TYP_BOOL) {
				ui_rectangle r;				
				r.left = 200;
				r.top = 20 + self->cpy;
				r.right = 211;
				r.bottom = 34 + self->cpy;				
				if (Intersects(&r, self->mx, self->my)) {
					property->item.value.i = property->item.value.i == 0;
					signal_emit(&self->signal_changed, self, 1, property);
				}
			} else
			if (property->item.typ == PROPERTY_TYP_STRING) {
				ui_rectangle r;
				ui_setrectangle(&r, 200, 20 + self->cpy, 300, 20);
				if (Intersects(&r, self->mx, self->my)) {
					self->selected = property;
					self->selrect = r;
					return 0;
				}
	//			ui_textout(self->g, 200, 20 + self->cpy, property->item.value.s, strlen(property->item.value.s));
			} else
			if (property->item.typ == PROPERTY_TYP_INTEGER) {
	//			char buf[20];
	//			int c;
	//			c =_snprintf(buf, 20, "%d", property->item.value.i);
	//			ui_textout(self->g, 200, 20 + self->cpy, buf, c);
			}				
		}
		self->cpy += 20;
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
		signal_emit(&self->signal_changed, self, 1, self->selected);
	}
}


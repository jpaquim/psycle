// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "settingsview.h"
#include <stdio.h>

static void OnDraw(SettingsView*, ui_component* sender, ui_graphics* g);
static int OnPropertiesEnum(SettingsView*, Properties* property, int level);
static int OnPropertiesHitTestEnum(SettingsView*, Properties* property, int level);
static int OnEnumFindSectionPositions(SettingsView* self, Properties* property, int level);
static void OnKeyDown(SettingsView*, ui_component* sender, int keycode, int keydata);
static void OnMouseDown(SettingsView*, ui_component* sender, int x, int y, int button);
static void OnMouseDoubleClick(SettingsView*, ui_component* sender, int x, int y, int button);
static void OnEditChange(SettingsView*, ui_edit* sender);
static void OnDestroy(SettingsView*, ui_component* sender);
static void OnSize(SettingsView*, ui_component* sender, int width, int height);
static void OnScroll(SettingsView*, ui_component* sender, int cx, int cy);
static void DrawCurrentLineBackground(SettingsView*);
static void DrawCurrentKey(SettingsView*, Properties* property);
static void AppendTabbarSections(SettingsView*);
static void ontabbarchange(SettingsView*, ui_component* sender, int tabindex);

void InitSettingsView(SettingsView* self, ui_component* parent,
	ui_component* tabbarparent, Properties* properties)
{			
	ui_component_init(&self->component, parent);
	ui_component_init(&self->client, &self->component);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_setbackgroundmode(&self->client, BACKGROUND_SET);
	ui_component_showverticalscrollbar(&self->client);
	signal_connect(&self->client.signal_destroy, self, OnDestroy);
	signal_connect(&self->client.signal_draw, self, OnDraw);
	signal_connect(&self->client.signal_scroll, self, OnScroll);
	signal_connect(&self->client.signal_keydown, self, OnKeyDown);
	signal_connect(&self->client.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->client.signal_mousedoubleclick, self, OnMouseDoubleClick);
	signal_connect(&self->component.signal_size, self, OnSize);
	self->properties = properties;	
	self->selected = 0;
	ui_edit_init(&self->edit, &self->client, 0);	
	ui_component_hide(&self->edit.component);
	signal_connect(&self->edit.signal_change, self, OnEditChange);
	self->dy = 0;
	self->cpx = 0;
	self->cpy = 0;
	self->fillchoice = 0;
	self->currchoice = 0;
	signal_init(&self->signal_changed);
	InitTabBar(&self->tabbar, &self->component);
	self->tabbar.tabalignment = UI_ALIGN_RIGHT;	
	ui_component_resize(&self->tabbar.component, 130, 0);	
	AppendTabbarSections(self);
}

void AppendTabbarSections(SettingsView* self)
{
	Properties* p;
	
	for (p = self->properties->children; p != 0; p = p->next) {
		if (p->item.typ == PROPERTY_TYP_SECTION) {
			tabbar_append(&self->tabbar, p->item.text);
		}		
	}
	tabbar_select(&self->tabbar, 0);
	InitIntHashTable(&self->sectionpositions, 256);
	self->lastlevel = 0;
	self->cpx = 0;
	self->cpy = self->dy;
	self->sectioncount = 0;
	properties_enumerate(self->properties, self, OnEnumFindSectionPositions);
	signal_connect(&self->tabbar.signal_change, self, ontabbarchange);
}

void OnDestroy(SettingsView* self, ui_component* sender)
{
	signal_dispose(&self->signal_changed);
	DisposeIntHashTable(&self->sectionpositions);
}

void OnDraw(SettingsView* self, ui_component* sender, ui_graphics* g)
{	
    self->g = g;		    	
	ui_setcolor(g, 0x00EAEAEA);
	ui_settextcolor(g, 0x00CACACA);
	ui_setbackgroundmode(g, TRANSPARENT);
	self->cpx = 0;
	self->cpy = 0;
	self->lastlevel = 0;
	self->fillchoice = 0;
	self->currchoice = 0;
	self->choicecount = 0;
	properties_enumerate(self->properties, self, OnPropertiesEnum);
}

int OnPropertiesEnum(SettingsView* self, Properties* property, int level)
{	
	if (self->lastlevel > level) {
		self->cpx -= 20;
		self->fillchoice = 0;
		self->currchoice = 0;
		self->choicecount = 0;
	}
	if (property->item.key && property->item.hint == PROPERTY_HINT_HIDE) {
		return 1;
	}
	if (property->item.key && property->item.typ == PROPERTY_TYP_SECTION) {
		self->cpy += 5;
		if (strcmp(property->item.key, "inputoutput") == 0) {
			self->cpy = self->cpy;
		}
		DrawCurrentKey(self, property);
		self->lastlevel = level;
		self->fillchoice = 0;
		self->cpy += 5;
		// self->cpx += 20;
		self->cpy += 20;
		if (property->children) {
			DrawCurrentLineBackground(self);
			self->cpy += 20;
		}
		return 1;
	} else
	if (property->item.key && property->item.typ == PROPERTY_TYP_CHOICE) {		
		if (property->children != 0)
		{
			self->fillchoice = 1;
			self->currchoice = property->item.value.i;
			self->choicecount = 0;
		}
		self->lastlevel = level;
		DrawCurrentLineBackground(self);
		ui_textout(self->g, 20 + self->cpx, 20 + self->cpy + self->dy, property->item.key, strlen(property->item.key));		
		if (property->children == 0)
		{
			self->cpy += 20;
		}
		return 1;
	} else
	if (self->lastlevel < level) {
		self->cpx += 20;
	}
	
	self->lastlevel = level;
	if (property->item.key) {				
		if (self->fillchoice) {
			ui_rectangle r;
			if (self->choicecount > 0) {
				DrawCurrentLineBackground(self);
			}
			ui_textout(self->g, 220, 20 + self->cpy + self->dy, properties_text(property),
				strlen(properties_text(property)));
			r.left = 200;
			r.top = 20 + self->cpy + self->dy;
			r.right = 210;
			r.bottom = 30 + self->cpy + self->dy;
			ui_drawrectangle(self->g, r);
			if (self->currchoice == self->choicecount)
			{
				ui_textout(self->g, 203, 21 + self->cpy + self->dy - 2,
						"x", strlen("x"));			
			}
			++self->choicecount;
		} else
		{
			DrawCurrentLineBackground(self);
			DrawCurrentKey(self, property);
			
			if (property->item.typ == PROPERTY_TYP_BOOL) {
				ui_rectangle r;				
				r.left = 200;
				r.top = 20 + self->cpy + self->dy;
				r.right = 211;
				r.bottom = 34 + self->cpy + self->dy;				
				if (property->item.value.i == 1) {
					ui_textout(self->g, 203, 21 + self->cpy + self->dy - 2,
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
				ui_textout(self->g, 200, 20 + self->cpy + self->dy, property->item.value.s, strlen(property->item.value.s));
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
				ui_textout(self->g, 200, 20 + self->cpy + self->dy, buf, c);
			}				
		}
		self->cpy += 20;
	}
	return 1;
}

void DrawCurrentLineBackground(SettingsView* self)
{
	ui_rectangle r;
	
	ui_setrectangle(&r, 10, 20 + self->cpy + self->dy, 400, 20);
	ui_drawsolidrectangle(self->g, r, 0x00333333);
}

void DrawCurrentKey(SettingsView* self, Properties* property)
{
	ui_textout(self->g, 20 + self->cpx, 20 + self->cpy + self->dy, properties_text(property),
		strlen(properties_text(property)));
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
	self->cpy = self->dy;
	self->currchoice = 0;
	self->choicecount = 0;
	self->choiceproperty = 0;
	properties_enumerate(self->properties, self, OnPropertiesHitTestEnum);
	ui_invalidate(&self->component);
}

int Intersects(ui_rectangle* r, int x, int y)
{
	return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
}

int OnPropertiesHitTestEnum(SettingsView* self, Properties* property, int level)
{
	if (self->lastlevel > level) {
		self->cpx -= 20;
		self->fillchoice = 0;
		self->currchoice = 0;
		self->choicecount = 0;
		self->choiceproperty = 0;
	}
	if (property->item.key && property->item.hint == PROPERTY_HINT_HIDE) {
		return 1;
	}
	if (property->item.key && property->item.typ == PROPERTY_TYP_SECTION) {
		self->cpy += 5;		
		self->lastlevel = level;
		self->fillchoice = 0;		
		self->cpy += 20;
		self->cpy += 5;
		if (property->children) {			
			self->cpy += 20;
		}
		return 1;
	} else
	if (property->item.key && property->item.typ == PROPERTY_TYP_CHOICE) {		
		if (property->children != 0)
		{
			self->fillchoice = 1;
			self->currchoice = property->item.value.i;
			self->choicecount = 0;
			self->choiceproperty = property;
		}
		self->lastlevel = level;	
		return 1;
	} else
	if (self->lastlevel < level) {
		self->cpx += 20;
	}	
	self->lastlevel = level;
	if (property->item.key) {				
		if (self->fillchoice) {
			ui_rectangle r;	
			r.left = 200;
			r.top = 20 + self->cpy;
			r.right = 210;
			r.bottom = 30 + self->cpy;	
			if (Intersects(&r, self->mx, self->my)) {
				if (self->choiceproperty) {
					self->choiceproperty->item.value.i = self->choicecount;
				}
				self->selected = property;
				signal_emit(&self->signal_changed, self, 1, property);
				self->selrect = r;
				return 0;
			}
			++self->choicecount;
		} else
		{	
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
			} else
			if (property->item.typ == PROPERTY_TYP_INTEGER) {
				ui_rectangle r;
				ui_setrectangle(&r, 200, 20 + self->cpy, 300, 20);
				if (Intersects(&r, self->mx, self->my)) {
					self->selected = property;
					self->selrect = r;
					return 0;
				}	
			}				
		}
		self->cpy += 20;
	}
	return 1;
}

int OnEnumFindSectionPositions(SettingsView* self, Properties* property, int level)
{
	if (property->item.key && property->item.hint == PROPERTY_HINT_HIDE) {
		return 1;
	}
	if (property->item.key && property->item.typ == PROPERTY_TYP_SECTION) {
		self->cpy += 5;		
		InsertIntHashTable(&self->sectionpositions, self->sectioncount, (void*)self->cpy);
		++self->sectioncount;
		self->lastlevel = level;
		self->fillchoice = 0;		
		self->cpy += 20;
		self->cpy += 5;
		if (property->children) {			
			self->cpy += 20;
		}
		return 1;
	} else
	if (property->item.key && property->item.typ == PROPERTY_TYP_CHOICE) {		
		if (property->children != 0)
		{
			self->fillchoice = 1;
		}
		self->lastlevel = level;	
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
			r.left = 200;
			r.top = 20 + self->cpy;
			r.right = 210;
			r.bottom = 30 + self->cpy;	
		} else
		{	
			if (property->item.typ == PROPERTY_TYP_BOOL) {
				ui_rectangle r;				
				r.left = 200;
				r.top = 20 + self->cpy;
				r.right = 211;
				r.bottom = 34 + self->cpy;							
			} else
			if (property->item.typ == PROPERTY_TYP_STRING) {
				ui_rectangle r;
				ui_setrectangle(&r, 200, 20 + self->cpy, 300, 20);				
			} else
			if (property->item.typ == PROPERTY_TYP_INTEGER) {
				ui_rectangle r;
				ui_setrectangle(&r, 200, 20 + self->cpy, 300, 20);				
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
		if (self->selected->item.typ == PROPERTY_TYP_STRING) {
			properties_write_string(self->selected, self->selected->item.key, ui_edit_text(&self->edit));
		} else 
		if (self->selected->item.typ == PROPERTY_TYP_INTEGER) {
			properties_write_int(self->selected, self->selected->item.key, atoi(ui_edit_text(&self->edit)));
		}
		signal_emit(&self->signal_changed, self, 1, self->selected);
	}
}

void OnScroll(SettingsView* self, ui_component* sender, int cx, int cy)
{
	self->dy += cy;
}

void OnSize(SettingsView* self, ui_component* sender, int width, int height)
{
	ui_size size;
	ui_size tabbarsize;
	
	size = ui_component_size(&self->component);	
	tabbarsize = ui_component_size(&self->tabbar.component);	
	ui_component_setposition(&self->client, 0, 0, 
		size.width - tabbarsize.width, size.height);
	ui_component_setposition(&self->tabbar.component,
		size.width - tabbarsize.width, 0, tabbarsize.width, size.height);
}

void ontabbarchange(SettingsView* self, ui_component* sender, int tabindex)
{
	int cpy;

	cpy = (int) SearchIntHashTable(&self->sectionpositions, tabindex);
	self->dy = -cpy;
	ui_invalidate(&self->client);
}

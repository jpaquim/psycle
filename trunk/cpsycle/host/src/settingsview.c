// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "settingsview.h"
#include <stdio.h>
#include "inputmap.h"

static void OnDraw(SettingsView*, ui_component* sender, ui_graphics*);
static int OnPropertiesDrawEnum(SettingsView*, Properties*, int level);
static int OnPropertiesHitTestEnum(SettingsView*, Properties*, int level);
static int OnEnumPropertyPosition(SettingsView*, Properties*, int level);
static void PreparePropertiesEnum(SettingsView* self);
static void OnKeyDown(SettingsView*, ui_component* sender, int keycode, int keydata);
static void OnMouseDown(SettingsView*, ui_component* sender, int x, int y, int button);
static void OnMouseDoubleClick(SettingsView*, ui_component* sender, int x, int y, int button);
static void OnEditChange(SettingsView*, ui_edit* sender);
static void OnInputDefinerChange(SettingsView* self, InputDefiner* sender);
static void OnDestroy(SettingsView*, ui_component* sender);
static void OnSize(SettingsView*, ui_component* sender, int width, int height);
static void OnScroll(SettingsView*, ui_component* sender, int cx, int cy);
static void DrawLineBackground(SettingsView*,Properties*);
static void DrawKey(SettingsView*, Properties*, int column);
static void DrawValue(SettingsView*, Properties*, int column);
static void DrawString(SettingsView*, Properties*, int column);
static void DrawInteger(SettingsView*, Properties*, int column);
static void DrawCheckBox(SettingsView*, Properties*, int column);
static void AdvanceLine(SettingsView*);
static void AddRemoveIdent(SettingsView*, int level);
static void AddIdent(SettingsView*);
static void RemoveIdent(SettingsView*);
static int IntersectsValue(SettingsView*, Properties* property, int column);
static void AppendTabbarSections(SettingsView*);
static void ontabbarchange(SettingsView*, ui_component* sender, int tabindex);
static void AdjustScrollRange(SettingsView*);

void InitSettingsView(SettingsView* self, ui_component* parent,
	ui_component* tabbarparent, Properties* properties)
{			
	self->properties = properties;
	ui_component_init(&self->component, parent);
	ui_component_init(&self->client, &self->component);
	self->client.doublebuffered = 1;
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
	self->selected = 0;
	self->choiceproperty = 0;
	self->dy = 0;
	ui_edit_init(&self->edit, &self->client, 0);	
	ui_component_hide(&self->edit.component);
	inputdefiner_init(&self->inputdefiner, &self->client);	
	ui_component_hide(&self->inputdefiner.component);		
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
		if (properties_type(p) == PROPERTY_TYP_SECTION) {
			tabbar_append(&self->tabbar, properties_text(p));			
		}		
	}
	tabbar_select(&self->tabbar, 0);			
	signal_connect(&self->tabbar.signal_change, self, ontabbarchange);
}

void OnDestroy(SettingsView* self, ui_component* sender)
{
	signal_dispose(&self->signal_changed);	
}

void OnDraw(SettingsView* self, ui_component* sender, ui_graphics* g)
{	
	self->g = g;
	ui_setcolor(g, 0x00EAEAEA);
	ui_settextcolor(g, 0x00CACACA);
	ui_setbackgroundmode(g, TRANSPARENT);
	PreparePropertiesEnum(self);
	properties_enumerate(self->properties->children, self, OnPropertiesDrawEnum);
}

void PreparePropertiesEnum(SettingsView* self)
{
	TEXTMETRIC tm;
	
	tm = ui_component_textmetric(&self->client);
	self->lineheight = (int) (tm.tmHeight * 1.5);
	self->columnwidth = tm.tmAveCharWidth * 50;
	self->identwidth = tm.tmAveCharWidth * 4;
	self->cpx = 0;
	self->cpy = 0;
	self->lastlevel = 0;
}

int OnPropertiesDrawEnum(SettingsView* self, Properties* property, int level)
{		
	AddRemoveIdent(self, level);
	if (self->cpy != 0 && level == 0 && properties_type(property) ==  PROPERTY_TYP_SECTION) {
		AdvanceLine(self);
	}
	if (properties_hint(property) == PROPERTY_HINT_HIDE) {
		return 1;
	}	
	if (properties_type(property) == PROPERTY_TYP_CHOICE) {
		self->currchoice = properties_value(property);
		self->choicecount = 0;					
	}	
	DrawLineBackground(self, property);						
	DrawKey(self, property, 0);	
	DrawValue(self, property, 1);
	if (properties_ischoiceitem(property)) {
		++self->choicecount;	
	}
	AdvanceLine(self);
	return 1;
}

void AddRemoveIdent(SettingsView* self, int level)
{
	if (self->lastlevel < level) {
		AddIdent(self);
		self->lastlevel = level;
	} else
	while (self->lastlevel > level) {
		RemoveIdent(self);
		--self->lastlevel;
	}	
}

void AddIdent(SettingsView* self)
{
	self->cpx += self->identwidth;
}

void RemoveIdent(SettingsView* self)
{
	self->cpx -= self->identwidth;
}

void DrawLineBackground(SettingsView* self, Properties* property)
{	
	if (properties_type(property) != PROPERTY_TYP_SECTION) {
		ui_size size;
		ui_rectangle r;

		size = ui_component_size(&self->client);
		ui_setrectangle(&r, 10, self->cpy + self->dy, size.width - 20,
			self->lineheight);
		ui_drawsolidrectangle(self->g, r, 0x00333333);
	}
}

void DrawKey(SettingsView* self, Properties* property, int column)
{
	ui_textout(self->g,
		self->cpx + column * self->columnwidth,
		self->cpy + self->dy,
		properties_text(property),
		strlen(properties_text(property)));
}

void DrawValue(SettingsView* self, Properties* property, int column)
{	
	if (properties_type(property) == PROPERTY_TYP_BOOL ||
			properties_ischoiceitem(property)) {
		DrawCheckBox(self, property, column);			
	} else
	if (properties_type(property) == PROPERTY_TYP_STRING) {
		DrawString(self, property, column);
	} else
	if (properties_type(property) == PROPERTY_TYP_INTEGER) {
		DrawInteger(self, property, column);
	}
}

void DrawString(SettingsView* self, Properties* property, int column)
{
	if (self->selected == property) {					
		ui_setbackgroundmode(self->g, OPAQUE);
		ui_setbackgroundcolor(self->g, 0x009B7800);
		ui_settextcolor(self->g, 0x00FFFFFF);
	}				
	ui_textout(self->g, self->columnwidth * column, self->cpy + self->dy,
		properties_valuestring(property),
		strlen(properties_valuestring(property)));
	ui_setbackgroundcolor(self->g, 0x003E3E3E);
	ui_setbackgroundmode(self->g, TRANSPARENT);
	ui_settextcolor(self->g, 0x00CACACA);
//	if (property->item.hint == PROPERTY_HINT_EDITDIR) {
//		ui_textout(self->g, 500, 20 + self->cpy, "...", strlen("..."));
//	}		
}

void DrawInteger(SettingsView* self, Properties* property, int column)
{	
	char text[40];
	
	if (properties_hint(property) == PROPERTY_HINT_INPUT) {
		inputdefiner_setinput(&self->inputdefiner, properties_value(property));
		inputdefiner_text(&self->inputdefiner, text);
	} else {
		_snprintf(text, 20, "%d", properties_value(property));
	}
	ui_textout(self->g, self->columnwidth * column, self->cpy + self->dy, text, strlen(text));
}

void AdvanceLine(SettingsView* self)
{
	self->cpy += self->lineheight;
}

void DrawCheckBox(SettingsView* self, Properties* property, int column)
{
	ui_rectangle r;
	int checked = 0;
	ui_size size;	
	
	size = ui_component_textsize(&self->client, "x");
	r.left = self->columnwidth * column;
	r.top = self->cpy + self->dy;
	r.right = r.left + size.width + 5;
	r.bottom = r.top + size.height + 2;
	if (properties_ischoiceitem(property)) {
		checked = self->currchoice == self->choicecount;
	} else {
		checked = properties_value(property) != 0;
	}
	if (checked) {
		ui_textout(self->g, r.left + 3, r.top - 1, "x", strlen("x"));			
	}
	ui_drawrectangle(self->g, r);
}

void OnKeyDown(SettingsView* self, ui_component* sender, int keycode, int keydata)
{	
}

void OnMouseDown(SettingsView* self, ui_component* sender, int x, int y, int button)
{
	if (ui_component_visible(&self->edit.component)) {
		OnEditChange(self, &self->edit);
		ui_component_hide(&self->edit.component);
	}	
	if (ui_component_visible(&self->inputdefiner.component)) {
		OnInputDefinerChange(self, &self->inputdefiner);
		ui_component_hide(&self->inputdefiner.component);
	}	
	self->selected = 0;
	self->mx = x;
	self->my = y;
	self->choiceproperty = 0;
	PreparePropertiesEnum(self);
	properties_enumerate(self->properties->children, self, OnPropertiesHitTestEnum);
	if (self->selected) {
		if (properties_ischoiceitem(self->selected)) {
			self->choiceproperty = self->selected->parent;
			self->choiceproperty->item.value.i = self->choicecount;
			signal_emit(&self->signal_changed, self, 1, self->selected);
		} else
		if (properties_type(self->selected) == PROPERTY_TYP_BOOL) {
			self->selected->item.value.i = self->selected->item.value.i == 0;
			signal_emit(&self->signal_changed, self, 1, self->selected);
		}
	}
	ui_invalidate(&self->client);
}

int Intersects(ui_rectangle* r, int x, int y)
{
	return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
}

int OnPropertiesHitTestEnum(SettingsView* self, Properties* property, int level)
{
	if (self->cpy != 0 && level == 0 && properties_type(property) ==  PROPERTY_TYP_SECTION) {
		AdvanceLine(self);
	}
	AddRemoveIdent(self, level);
	if (properties_hint(property) == PROPERTY_HINT_HIDE) {
		return 1;
	}
	if (properties_type(property) == PROPERTY_TYP_CHOICE) {
		self->currchoice = properties_value(property);
		self->choicecount = 0;					
	}	
	if (IntersectsValue(self, property, 1)) {
		self->selected = property;		
		return 0;
	}
	if (properties_ischoiceitem(property)) {
		++self->choicecount;	
	}
	AdvanceLine(self);	
	return 1;	
}

int OnEnumPropertyPosition(SettingsView* self, Properties* property, int level)
{
	AddRemoveIdent(self, level);
	if (self->cpy != 0 && level == 0 && properties_type(property) ==  PROPERTY_TYP_SECTION) {
		AdvanceLine(self);
	}
	if (properties_hint(property) == PROPERTY_HINT_HIDE) {
		return 1;
	}
	if (self->search == property) {		
		return 0;
	}
	if (properties_type(property) == PROPERTY_TYP_CHOICE) {
		self->currchoice = properties_value(property);
		self->choicecount = 0;					
	}	
	if (properties_ischoiceitem(property)) {
		++self->choicecount;	
	}
	AdvanceLine(self);	
	return 1;	
}

int IntersectsValue(SettingsView* self, Properties* property, int column)
{
	int rv = 0;	

	if (properties_type(property) == PROPERTY_TYP_BOOL) {					
		ui_rectangle r;
		int checked = 0;
		ui_size size;	
		
		size = ui_component_textsize(&self->client, "x");
		r.left = self->columnwidth * column;
		r.top = self->cpy + self->dy;
		r.right = r.left + size.width + 5;
		r.bottom = r.top + size.height + 2;
	
		rv = Intersects(&r, self->mx, self->my);
	} else
	if (properties_type(property) == PROPERTY_TYP_INTEGER ||
			properties_type(property) == PROPERTY_TYP_STRING) {
		ui_rectangle r;
		ui_setrectangle(&r, self->columnwidth * column, 
			self->cpy + self->dy, 300, self->lineheight);
		self->selrect = r;
		rv = Intersects(&r, self->mx, self->my);
	}
	return rv;
}

void OnMouseDoubleClick(SettingsView* self, ui_component* sender, int x, int y, int button)
{
	if (self->selected) {
		ui_component* edit = 0;

		if (self->selected->item.typ == PROPERTY_TYP_INTEGER) {
			if (properties_hint(self->selected) == PROPERTY_HINT_INPUT) {
				inputdefiner_setinput(&self->inputdefiner,
					properties_value(self->selected));
				edit = &self->inputdefiner.component;				
			} else {
				char text[40];
				_snprintf(text, 40, "%d", properties_value(self->selected));
				ui_edit_settext(&self->edit, text);
				edit = &self->edit.component;				
			}
		} else
		if (self->selected->item.typ == PROPERTY_TYP_STRING) {
			ui_edit_settext(&self->edit, self->selected->item.value.s);
			edit = &self->edit.component;									
		}		
		if (edit) {
			ui_component_setposition(edit,
				self->selrect.left,
				self->selrect.top,
				self->selrect.right - self->selrect.left, 
				self->selrect.bottom - self->selrect.top);
			ui_component_show(edit);
			ui_component_setfocus(edit);
		}
	}
}

void OnInputDefinerChange(SettingsView* self, InputDefiner* sender)
{
	if (self->selected && self->selected->parent) {
		if (self->selected->item.typ == PROPERTY_TYP_INTEGER) {
			properties_write_int(self->selected->parent, self->selected->item.key, self->inputdefiner.input);
		}
		signal_emit(&self->signal_changed, self, 1, self->selected);
	}
}

void OnEditChange(SettingsView* self, ui_edit* sender)
{
	if (self->selected && self->selected->parent) {
		if (self->selected->item.typ == PROPERTY_TYP_STRING) {
			properties_write_string(self->selected->parent, self->selected->item.key, ui_edit_text(&self->edit));
		} else 
		if (self->selected->item.typ == PROPERTY_TYP_INTEGER) {
			properties_write_int(self->selected->parent, self->selected->item.key, atoi(ui_edit_text(&self->edit)));
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
	tabbarsize = ui_component_preferredsize(&self->tabbar.component, &size);	
	ui_component_resize(&self->client, size.width - tabbarsize.width,
		size.height);
	ui_component_setposition(&self->tabbar.component,
		size.width - tabbarsize.width, 0, tabbarsize.width, size.height);
	AdjustScrollRange(self);
}

void ontabbarchange(SettingsView* self, ui_component* sender, int tabindex)
{		
	Properties* p = 0;	
	Tab* tab;

	self->search = 0;
	if (self->properties) {
		p = self->properties->children;
		tab = tabbar_tab(&self->tabbar, tabindex);
		if (tab) {		
			while (p) {
				if (properties_type(p) == PROPERTY_TYP_SECTION) {				
					if (strcmp(properties_text(p), tab->text) == 0) {
						break;
					}				
				}		
				p = p->next;
			}
		}
		self->search = p;
		if (self->search) {			
			int scrollposition;
			int scrollmin;
			int scrollmax;

			PreparePropertiesEnum(self);
			properties_enumerate(self->properties->children, self, OnEnumPropertyPosition);
			ui_component_verticalscrollrange(&self->client, &scrollmin, &scrollmax);
			scrollposition = self->cpy / self->lineheight;
			if (scrollposition > scrollmax) {
				scrollposition = scrollmax;
			}
			self->dy = -scrollposition * self->lineheight;			
			ui_component_setverticalscrollposition(&self->client, scrollposition);
		}	
		ui_invalidate(&self->client);
	}
}

void AdjustScrollRange(SettingsView* self)
{
	ui_size size;
	int scrollmax;

	size = ui_component_size(&self->client);
	self->search = 0;
	PreparePropertiesEnum(self);
	properties_enumerate(self->properties->children, self, OnEnumPropertyPosition);
	self->client.scrollstepy = self->lineheight;
	scrollmax =  (self->cpy - size.height) / self->lineheight + 1;
	ui_component_setverticalscrollrange(&self->client, 0, scrollmax);
	if (-self->dy / self->lineheight > scrollmax - 1) {
		self->dy = -(scrollmax) * self->lineheight;
		ui_component_setverticalscrollposition(&self->client, scrollmax);
	}	
}

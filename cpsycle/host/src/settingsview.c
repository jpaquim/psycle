// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "settingsview.h"
#include <stdio.h>
#include "inputmap.h"
#include <portable.h>

static void settingsview_ondraw(SettingsView*, ui_component* sender, ui_graphics*);
static int settingsview_onpropertiesdrawenum(SettingsView*, Properties*, int level);
static int settingsview_onpropertieshittestenum(SettingsView*, Properties*, int level);
static int settingsview_onenumpropertyposition(SettingsView*, Properties*, int level);
static void settingsview_preparepropertiesenum(SettingsView* self);
static void settingsview_onkeydown(SettingsView*, ui_component* sender, KeyEvent*);
static void settingsview_onmousedown(SettingsView*, ui_component* sender, MouseEvent*);
static void settingsview_onmousedoubleclick(SettingsView*, ui_component* sender, MouseEvent*);
static void settingsview_oneditchange(SettingsView*, ui_edit* sender);
static void settingsview_oneditkeydown(SettingsView*, ui_component* sender, KeyEvent*);
static void settingsview_oninputdefinerchange(SettingsView* self, InputDefiner* sender);
static void settingsview_ondestroy(SettingsView*, ui_component* sender);
static void settingsview_onsize(SettingsView*, ui_component* sender, ui_size*);
static void settingsview_onscroll(SettingsView*, ui_component* sender, int stepx, int stepy);
static void settingsview_drawlinebackground(SettingsView*,Properties*);
static void settingsview_drawkey(SettingsView*, Properties*, int column);
static void settingsview_drawvalue(SettingsView*, Properties*, int column);
static void settingsview_drawstring(SettingsView*, Properties*, int column);
static void settingsview_drawinteger(SettingsView*, Properties*, int column);
static void settingsview_drawbutton(SettingsView*, Properties*, int column);
static void settingsview_drawcheckbox(SettingsView*, Properties*, int column);
static void settingsview_advanceline(SettingsView*);
static void settingsview_addremoveident(SettingsView*, int level);
static void settingsview_addident(SettingsView*);
static void settingsview_removeident(SettingsView*);
static int settingsview_intersectsvalue(SettingsView*, Properties* property,
	int column);
static void settingsview_appendtabbarsections(SettingsView*);
static void settingsview_ontabbarchange(SettingsView*, ui_component* sender,
	int tabindex);
static void settingsview_adjustscroll(SettingsView*);

void settingsview_init(SettingsView* self, ui_component* parent,
	ui_component* tabbarparent, Properties* properties)
{
	self->properties = properties;
	ui_component_init(&self->component, parent);
	ui_component_init(&self->client, &self->component);
	self->client.doublebuffered = 1;
	self->client.wheelscroll = 4;
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);	
	ui_component_showverticalscrollbar(&self->client);	
	signal_connect(&self->client.signal_destroy, self, settingsview_ondestroy);
	signal_connect(&self->client.signal_draw, self, settingsview_ondraw);
	signal_connect(&self->client.signal_scroll, self, settingsview_onscroll);
	signal_connect(&self->client.signal_keydown, self, settingsview_onkeydown);
	signal_connect(&self->component.signal_keydown, self,
		settingsview_onkeydown);
	signal_connect(&self->client.signal_mousedown, self,
		settingsview_onmousedown);
	signal_connect(&self->client.signal_mousedoubleclick, self,
		settingsview_onmousedoubleclick);
	signal_connect(&self->component.signal_size, self, settingsview_onsize);
	self->selected = 0;
	self->choiceproperty = 0;
	self->dy = 0;
	self->dirbutton = 0;
	ui_edit_init(&self->edit, &self->client, ES_AUTOHSCROLL);
	signal_connect(&self->edit.component.signal_keydown, self,
		settingsview_oneditkeydown);
	ui_component_hide(&self->edit.component);
	inputdefiner_init(&self->inputdefiner, &self->client);	
	ui_component_hide(&self->inputdefiner.component);		
	signal_init(&self->signal_changed);
	tabbar_init(&self->tabbar, &self->component);
	self->tabbar.tabalignment = UI_ALIGN_RIGHT;	
	ui_component_resize(&self->tabbar.component, 130, 0);	
	settingsview_appendtabbarsections(self);
}

void settingsview_appendtabbarsections(SettingsView* self)
{	
	Properties* p;	
	
	for (p = self->properties->children; p != 0;
			p = properties_next(p)) {
		if (properties_type(p) == PROPERTY_TYP_SECTION) {
			tabbar_append(&self->tabbar, properties_text(p));			
		}		
	}
	tabbar_select(&self->tabbar, 0);			
	signal_connect(&self->tabbar.signal_change, self,
		settingsview_ontabbarchange);
}

void settingsview_ondestroy(SettingsView* self, ui_component* sender)
{
	signal_dispose(&self->signal_changed);	
}

void settingsview_ondraw(SettingsView* self, ui_component* sender, ui_graphics* g)
{	
	self->g = g;
	ui_setcolor(g, 0x00EAEAEA);
	ui_settextcolor(g, 0x00CACACA);
	ui_setbackgroundmode(g, TRANSPARENT);
	settingsview_preparepropertiesenum(self);
	properties_enumerate(self->properties->children, self,
		settingsview_onpropertiesdrawenum);
}

void settingsview_preparepropertiesenum(SettingsView* self)
{
	ui_textmetric tm;
	
	tm = ui_component_textmetric(&self->client);
	self->lineheight = (int) (tm.tmHeight * 1.5);
	self->columnwidth = tm.tmAveCharWidth * 50;
	self->identwidth = tm.tmAveCharWidth * 4;
	self->cpx = 0;
	self->cpy = 0;
	self->lastlevel = 0;
}

int settingsview_onpropertiesdrawenum(SettingsView* self, Properties* property, int level)
{		
	settingsview_addremoveident(self, level);
	if (self->cpy != 0 && level == 0 && properties_type(property) ==
			PROPERTY_TYP_SECTION) {
		settingsview_advanceline(self);
	}
	if (properties_hint(property) == PROPERTY_HINT_HIDE) {
		return 1;
	}	
	if (properties_type(property) == PROPERTY_TYP_CHOICE) {
		self->currchoice = properties_value(property);
		self->choicecount = 0;					
	}		
	settingsview_drawlinebackground(self, property);						
	settingsview_drawkey(self, property, 0);	
	settingsview_drawvalue(self, property, 1);
	if (properties_ischoiceitem(property)) {
		++self->choicecount;	
	}
	settingsview_advanceline(self);
	return 1;
}

void settingsview_addremoveident(SettingsView* self, int level)
{
	if (self->lastlevel < level) {
		settingsview_addident(self);
		self->lastlevel = level;
	} else
	while (self->lastlevel > level) {
		settingsview_removeident(self);
		--self->lastlevel;
	}	
}

void settingsview_addident(SettingsView* self)
{
	self->cpx += self->identwidth;
}

void settingsview_removeident(SettingsView* self)
{
	self->cpx -= self->identwidth;
}

void settingsview_drawlinebackground(SettingsView* self, Properties* property)
{	
	if (properties_type(property) != PROPERTY_TYP_SECTION) {
		ui_size size;
		ui_rectangle r;

		size = ui_component_size(&self->client);
		ui_setrectangle(&r, 10, self->cpy + self->dy, size.width - 20,
			self->lineheight);
		ui_drawsolidrectangle(self->g, r, 0x00292929);
	}
}

void settingsview_drawkey(SettingsView* self, Properties* property, int column)
{	
	if (properties_type(property) == PROPERTY_TYP_ACTION) {
		settingsview_drawbutton(self, property, column + 1);
	} else {
		ui_textout(self->g,
		self->cpx + column * self->columnwidth,
		self->cpy + self->dy,
		properties_text(property),
		strlen(properties_text(property)));
	}
}

void settingsview_drawvalue(SettingsView* self, Properties* property,
	int column)
{	
	if (properties_type(property) == PROPERTY_TYP_BOOL ||
			properties_ischoiceitem(property)) {
		settingsview_drawcheckbox(self, property, column);
	} else
	if (properties_type(property) == PROPERTY_TYP_STRING) {
		settingsview_drawstring(self, property, column);
		if (properties_hint(property) == PROPERTY_HINT_EDITDIR) {
			settingsview_drawbutton(self, property, column + 1);
		}
	} else
	if (properties_type(property) == PROPERTY_TYP_INTEGER) {
		settingsview_drawinteger(self, property, column);
	}
}

void settingsview_drawstring(SettingsView* self, Properties* property,
	int column)
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

void settingsview_drawinteger(SettingsView* self, Properties* property,
	int column)
{	
	char text[40];
	
	if (properties_hint(property) == PROPERTY_HINT_INPUT) {
		inputdefiner_setinput(&self->inputdefiner, properties_value(property));
		inputdefiner_text(&self->inputdefiner, text);
	} else {
		psy_snprintf(text, 20, "%d", properties_value(property));
	}
	ui_textout(self->g, self->columnwidth * column, self->cpy + self->dy, text,
		strlen(text));
}

void settingsview_advanceline(SettingsView* self)
{
	self->cpy += self->lineheight;
}

void settingsview_drawbutton(SettingsView* self, Properties* property,
	int column)
{
	ui_size size;
	ui_rectangle r;	
	if (properties_hint(property) == PROPERTY_HINT_EDITDIR) {
		ui_textout(self->g, self->columnwidth * column + 3,
			self->cpy + self->dy, "...", 3);
	} else {
		ui_textout(self->g, self->columnwidth * column + 3,
			self->cpy + self->dy, properties_text(property),
			strlen(properties_text(property)));
	}
	ui_setbackgroundcolor(self->g, 0x003E3E3E);
	ui_setbackgroundmode(self->g, TRANSPARENT);
	ui_settextcolor(self->g, 0x00CACACA);	
	if (properties_hint(property) == PROPERTY_HINT_EDITDIR) {
		size = ui_component_textsize(&self->client, "...");
	} else {
		size = ui_component_textsize(&self->client, properties_text(property));
	}
	r.left = self->columnwidth * column ;
	r.top = self->cpy + self->dy ;
	r.right = r.left + size.width + 6;
	r.bottom = r.top + size.height + 2;
	ui_drawrectangle(self->g, r);
}

void settingsview_drawcheckbox(SettingsView* self, Properties* property,
	int column)
{
	ui_rectangle r;
	int checked = 0;
	ui_textmetric tm;
	ui_size size;
	ui_size cornersize;
	ui_size knobsize;
		
	tm = ui_component_textmetric(&self->component);
	size.width = tm.tmAveCharWidth * 4;
	size.height = tm.tmHeight;
	knobsize.width = (int) (tm.tmAveCharWidth * 2);
	knobsize.height = (int) (tm.tmHeight * 0.7 + 0.5);
	cornersize.width = (int) (tm.tmAveCharWidth * 0.6);
	cornersize.height = (int) (tm.tmHeight * 0.6);
	r.left = self->columnwidth * column;
	r.top = self->cpy + self->dy + (self->lineheight - size.height) / 2;
	r.right = r.left + (int)(tm.tmAveCharWidth * 4.8);
	r.bottom = r.top + size.height;
	ui_setcolor(self->g, 0x00555555);
	ui_drawroundrectangle(self->g, r, cornersize);
	if (properties_ischoiceitem(property)) {
		checked = self->currchoice == self->choicecount;
	} else {
		checked = properties_value(property) != 0;
	}
	if (!checked) {
		r.left = self->columnwidth * column + (int)(tm.tmAveCharWidth * 0.4);
		r.top = self->cpy + self->dy + (self->lineheight - knobsize.height) / 2;
		r.right = r.left + (int)(tm.tmAveCharWidth * 2.5);
		r.bottom = r.top + knobsize.height;
		ui_drawsolidroundrectangle(self->g, r, cornersize, 0x00555555);
	} else {
		r.left = self->columnwidth * column + tm.tmAveCharWidth * 2;
		r.top = self->cpy + self->dy + (self->lineheight - knobsize.height) / 2;
		r.right = r.left + (int)(tm.tmAveCharWidth * 2.5);
		r.bottom = r.top + knobsize.height;
		ui_drawsolidroundrectangle(self->g, r, cornersize, 0x00CACACA);
	}	
}

void settingsview_onkeydown(SettingsView* self, ui_component* sender,
	KeyEvent* keyevent)
{	
	ui_component_propagateevent(sender);
}

void settingsview_onmousedown(SettingsView* self, ui_component* sender,
	MouseEvent* ev)
{
	ui_component_setfocus(&self->client);
	if (ui_component_visible(&self->edit.component)) {
		settingsview_oneditchange(self, &self->edit);
		ui_component_hide(&self->edit.component);
	}	
	if (ui_component_visible(&self->inputdefiner.component)) {
		settingsview_oninputdefinerchange(self, &self->inputdefiner);
		ui_component_hide(&self->inputdefiner.component);
	}	
	self->selected = 0;
	self->mx = ev->x;
	self->my = ev->y;
	self->choiceproperty = 0;
	self->dirbutton = 0;
	settingsview_preparepropertiesenum(self);
	properties_enumerate(self->properties->children, self,
		settingsview_onpropertieshittestenum);
	if (self->selected) {
		if (self->dirbutton) {
			char path[MAX_PATH]	 = "";
			char title[MAX_PATH];
			
			psy_snprintf(title, MAX_PATH, "%s", properties_text(self->selected));
			title[MAX_PATH - 1] = '\0';
			if (ui_browsefolder(&self->component, title, path)) {
				properties_write_string(self->selected->parent,
					self->selected->item.key, path);
			}
		} else
		if (properties_ischoiceitem(self->selected)) {
			self->choiceproperty = self->selected->parent;
			self->choiceproperty->item.value.i = self->choicecount;
			signal_emit(&self->signal_changed, self, 1, self->selected);
		} else
		if (properties_type(self->selected) == PROPERTY_TYP_BOOL) {
			self->selected->item.value.i = self->selected->item.value.i == 0;
			signal_emit(&self->signal_changed, self, 1, self->selected);
		} else
		if (properties_type(self->selected) == PROPERTY_TYP_ACTION) {			
			signal_emit(&self->signal_changed, self, 1, self->selected);
		}
	}
	ui_component_invalidate(&self->client);
}

int settingsview_intersects(ui_rectangle* r, int x, int y)
{
	return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
}

int settingsview_onpropertieshittestenum(SettingsView* self,
	Properties* property, int level)
{
	if (self->cpy != 0 && level == 0 && properties_type(property) == 
			PROPERTY_TYP_SECTION) {
		settingsview_advanceline(self);
	}
	settingsview_addremoveident(self, level);
	if (properties_hint(property) == PROPERTY_HINT_HIDE) {
		return 1;
	}
	if (properties_type(property) == PROPERTY_TYP_CHOICE) {
		self->currchoice = properties_value(property);
		self->choicecount = 0;					
	}	
	if (settingsview_intersectsvalue(self, property, 1)) {
		self->selected = property;		
		return 0;
	}
	if (properties_ischoiceitem(property)) {
		++self->choicecount;	
	}
	settingsview_advanceline(self);
	return 1;	
}

int settingsview_onenumpropertyposition(SettingsView* self,
	Properties* property, int level)
{
	settingsview_addremoveident(self, level);
	if (self->cpy != 0 && level == 0 && properties_type(property) ==  PROPERTY_TYP_SECTION) {
		settingsview_advanceline(self);
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
	settingsview_advanceline(self);
	return 1;	
}

int settingsview_intersectsvalue(SettingsView* self, Properties* property,
	int column)
{
	int rv = 0;	

	self->dirbutton = 0;
	if (properties_type(property) == PROPERTY_TYP_BOOL) {					
		ui_rectangle r;
		int checked = 0;
		ui_size size;
		ui_textmetric tm;
		
		tm = ui_component_textmetric(&self->component);
		size = ui_component_textsize(&self->client, "x");
		r.left = self->columnwidth * column;
		r.top = self->cpy + self->dy;
		r.right = r.left + tm.tmAveCharWidth * 4;;
		r.bottom = r.top + size.height + 2;
	
		rv = settingsview_intersects(&r, self->mx, self->my);
	} else
	if (properties_type(property) == PROPERTY_TYP_INTEGER ||
		properties_type(property) == PROPERTY_TYP_STRING ||
		properties_type(property) == PROPERTY_TYP_ACTION) {
		ui_rectangle r;		
		ui_setrectangle(&r, self->columnwidth * column, 
			self->cpy + self->dy, self->columnwidth, self->lineheight);
		self->selrect = r;
		rv = settingsview_intersects(&r, self->mx, self->my);
		if (!rv && properties_hint(property) == PROPERTY_HINT_EDITDIR) {
			ui_setrectangle(&r, (self->columnwidth * (column + 1)), 
				self->cpy + self->dy, self->columnwidth, self->lineheight);
			self->selrect = r;
			rv = settingsview_intersects(&r, self->mx, self->my);
			if (rv) {
				self->dirbutton = 1;
			}
		}
	}
	return rv;
}

void settingsview_onmousedoubleclick(SettingsView* self, ui_component* sender,
	MouseEvent* ev)
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
				psy_snprintf(text, 40, "%d", properties_value(self->selected));
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

void settingsview_oninputdefinerchange(SettingsView* self,
	InputDefiner* sender)
{
	if (self->selected && self->selected->parent) {
		if (self->selected->item.typ == PROPERTY_TYP_INTEGER) {
			properties_write_int(self->selected->parent,
				self->selected->item.key, self->inputdefiner.input);
		}
		signal_emit(&self->signal_changed, self, 1, self->selected);
	}
}

void settingsview_oneditchange(SettingsView* self, ui_edit* sender)
{
	if (self->selected && self->selected->parent) {
		if (self->selected->item.typ == PROPERTY_TYP_STRING) {
			properties_write_string(self->selected->parent,
				self->selected->item.key, ui_edit_text(&self->edit));
		} else 
		if (self->selected->item.typ == PROPERTY_TYP_INTEGER) {
			properties_write_int(self->selected->parent,
				self->selected->item.key, atoi(ui_edit_text(&self->edit)));
		}
		signal_emit(&self->signal_changed, self, 1, self->selected);
	}
}

void settingsview_oneditkeydown(SettingsView* self, ui_component* sender,
	KeyEvent* keyevent)
{
	if (keyevent->keycode == VK_RETURN) {
		ui_component_hide(&self->edit.component);
		ui_component_setfocus(&self->client);
		settingsview_oneditchange(self, &self->edit);
	} else
	if (keyevent->keycode == VK_ESCAPE) {
		ui_component_hide(&self->edit.component);
		ui_component_setfocus(&self->client);		
	}
}

void settingsview_onscroll(SettingsView* self, ui_component* sender, int stepx,
	int stepy)
{
	self->dy += (stepy * sender->scrollstepy);
}

void settingsview_onsize(SettingsView* self, ui_component* sender,
	ui_size* size)
{	
	ui_size tabbarsize;	
		
	tabbarsize = ui_component_preferredsize(&self->tabbar.component, size);
	ui_component_resize(&self->client, size->width - tabbarsize.width,
		size->height);
	ui_component_setposition(&self->tabbar.component,
		size->width - tabbarsize.width, 0, tabbarsize.width, size->height);
	settingsview_adjustscroll(self);
}

void settingsview_ontabbarchange(SettingsView* self, ui_component* sender,
	int tabindex)
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
				p = properties_next(p);
			}
		}
		self->search = p;
		if (self->search) {			
			int scrollposition;
			int scrollmin;
			int scrollmax;

			settingsview_preparepropertiesenum(self);
			properties_enumerate(self->properties->children, self,
				settingsview_onenumpropertyposition);
			ui_component_verticalscrollrange(&self->client, &scrollmin, &scrollmax);
			scrollposition = self->cpy / self->lineheight;
			if (scrollposition > scrollmax) {
				scrollposition = scrollmax;
			}
			self->dy = -scrollposition * self->lineheight;			
			ui_component_setverticalscrollposition(&self->client, scrollposition);
		}	
		ui_component_invalidate(&self->client);
	}
}

void settingsview_adjustscroll(SettingsView* self)
{
	ui_size size;
	int scrollmax;

	size = ui_component_size(&self->client);
	self->search = 0;
	settingsview_preparepropertiesenum(self);
	properties_enumerate(self->properties->children, self,
		settingsview_onenumpropertyposition);
	self->client.scrollstepy = self->lineheight;
	scrollmax =  (self->cpy - size.height) / self->lineheight + 1;
	if (scrollmax < 0) {
		scrollmax = 0;
	}
	ui_component_setverticalscrollrange(&self->client, 0, scrollmax);
	if (-self->dy / self->lineheight > scrollmax - 1) {
		self->dy = -(scrollmax) * self->lineheight;
		ui_component_setverticalscrollposition(&self->client, scrollmax);
	}	
}

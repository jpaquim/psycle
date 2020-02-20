// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "settingsview.h"
#include <stdio.h>
#include "inputmap.h"
#include <stdlib.h>
#include <string.h>

#include <uifolderdialog.h>
#include <uifontdialog.h>
#include <uicolordialog.h>
#include "../../detail/portable.h"
#include "../../detail/os.h"

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#endif

static void settingsview_ondraw(SettingsView*, psy_ui_Component* sender,
	psy_ui_Graphics*);
static int settingsview_onpropertiesdrawenum(SettingsView*, psy_Properties*,
	int level);
static int settingsview_onpropertieshittestenum(SettingsView*, psy_Properties*,
	int level);
static int settingsview_onenumpropertyposition(SettingsView*, psy_Properties*,
	int level);
static void settingsview_preparepropertiesenum(SettingsView* self);
static void settingsview_onmousedown(SettingsView*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void settingsview_onmousedoubleclick(SettingsView*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void settingsview_oneditchange(SettingsView*, psy_ui_Edit* sender);
static void settingsview_oneditkeydown(SettingsView*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void settingsview_oninputdefinerchange(SettingsView* self,
	InputDefiner* sender);
static void settingsview_ondestroy(SettingsView*, psy_ui_Component* sender);
static void settingsview_onsize(SettingsView*, psy_ui_Component* sender, psy_ui_Size*);
static void settingsview_onscroll(SettingsView*, psy_ui_Component* sender,
	int stepx, int stepy);
static void settingsview_drawlinebackground(SettingsView*,psy_Properties*);
static void settingsview_drawkey(SettingsView*, psy_Properties*, int column);
static void settingsview_drawvalue(SettingsView*, psy_Properties*, int column);
static void settingsview_drawstring(SettingsView*, psy_Properties*,
	int column);
static void settingsview_drawinteger(SettingsView*, psy_Properties*,
	int column);
static void settingsview_drawbutton(SettingsView*, psy_Properties*,
	int column);
static void settingsview_drawcheckbox(SettingsView*, psy_Properties*,
	int column);
static void settingsview_advanceline(SettingsView*);
static void settingsview_addremoveident(SettingsView*, int level);
static void settingsview_addident(SettingsView*);
static void settingsview_removeident(SettingsView*);
static int settingsview_intersectsvalue(SettingsView*, psy_Properties*,
	int column);
static void settingsview_appendtabbarsections(SettingsView*);
static void settingsview_ontabbarchange(SettingsView*, psy_ui_Component* sender,
	int tabindex);
static void settingsview_adjustscroll(SettingsView*);

void settingsview_init(SettingsView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_Properties* properties)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_init(&self->viewtabbar, tabbarparent);
	self->properties = properties;
	psy_ui_component_init(&self->client, &self->component);
	psy_ui_component_doublebuffer(&self->client);	
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	self->client.wheelscroll = 4;
	psy_ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);	
	psy_ui_component_showverticalscrollbar(&self->client);	
	psy_signal_connect(&self->client.signal_destroy, self,
		settingsview_ondestroy);
	psy_signal_connect(&self->client.signal_draw, self, settingsview_ondraw);
	psy_signal_connect(&self->client.signal_scroll, self,
		settingsview_onscroll);		
	psy_signal_connect(&self->client.signal_mousedown, self,
		settingsview_onmousedown);
	psy_signal_connect(&self->client.signal_mousedoubleclick, self,
		settingsview_onmousedoubleclick);
	psy_signal_connect(&self->component.signal_size, self,
		settingsview_onsize);
	self->selected = 0;
	self->choiceproperty = 0;
	self->dy = 0;
	self->button = 0;
	psy_ui_edit_init(&self->edit, &self->client);
	psy_signal_connect(&self->edit.component.signal_keydown, self,
		settingsview_oneditkeydown);
	psy_ui_component_hide(&self->edit.component);
	inputdefiner_init(&self->inputdefiner, &self->client);	
	psy_ui_component_hide(&self->inputdefiner.component);		
	psy_signal_init(&self->signal_changed);
	tabbar_init(&self->tabbar, &self->component);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_RIGHT);
	self->tabbar.tabalignment = psy_ui_ALIGN_RIGHT;	
	settingsview_appendtabbarsections(self);
}

void settingsview_appendtabbarsections(SettingsView* self)
{	
	psy_Properties* p;	
	
	for (p = self->properties->children; p != 0;
			p = psy_properties_next(p)) {
		if (psy_properties_type(p) == PSY_PROPERTY_TYP_SECTION) {
			tabbar_append(&self->tabbar, psy_properties_text(p));			
		}		
	}
	tabbar_select(&self->tabbar, 0);			
	psy_signal_connect(&self->tabbar.signal_change, self,
		settingsview_ontabbarchange);
}

void settingsview_selectsection(SettingsView* self, const char* key)
{
	psy_Properties* p;
	int pageindex;
	int found = 0;

	for (pageindex = 0, p = self->properties->children; p != 0;
		p = psy_properties_next(p), ++pageindex) {
		if (psy_properties_type(p) == PSY_PROPERTY_TYP_SECTION &&
			(strcmp(psy_properties_key(p), key) == 0)) {
			found = 1;
			break;
		}
	}
	if (found) {
		tabbar_select(&self->tabbar, pageindex);
	}
}

void settingsview_ondestroy(SettingsView* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
}

void settingsview_ondraw(SettingsView* self, psy_ui_Component* sender,
	psy_ui_Graphics* g)
{	
	self->g = g;
	psy_ui_setcolor(g, 0x00EAEAEA);
	psy_ui_settextcolor(g, 0x00CACACA);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	settingsview_preparepropertiesenum(self);
	psy_properties_enumerate(self->properties->children, self,
		settingsview_onpropertiesdrawenum);
}

void settingsview_preparepropertiesenum(SettingsView* self)
{
	psy_ui_TextMetric tm;
	
	tm = psy_ui_component_textmetric(&self->client);
	self->lineheight = (int) (tm.tmHeight * 1.5);
	self->columnwidth = tm.tmAveCharWidth * 50;
	self->identwidth = tm.tmAveCharWidth * 4;
	self->cpx = 0;
	self->cpy = 0;
	self->lastlevel = 0;
}

int settingsview_onpropertiesdrawenum(SettingsView* self,
	psy_Properties* property, int level)
{			
	settingsview_addremoveident(self, level);
	if (self->cpy != 0 && level == 0 && psy_properties_type(property) ==
			PSY_PROPERTY_TYP_SECTION) {
		settingsview_advanceline(self);
	}
	if (psy_properties_hint(property) == PSY_PROPERTY_HINT_HIDE) {		
		return 2;
	}	
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_CHOICE) {
		self->currchoice = psy_properties_value(property);
		self->choicecount = 0;					
	}		
	settingsview_drawlinebackground(self, property);						
	settingsview_drawkey(self, property, 0);	
	settingsview_drawvalue(self, property, 1);
	if (psy_properties_ischoiceitem(property)) {
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

void settingsview_drawlinebackground(SettingsView* self,
	psy_Properties* property)
{	
	if (psy_properties_type(property) != PSY_PROPERTY_TYP_SECTION) {
		psy_ui_Size size;
		psy_ui_Rectangle r;

		size = psy_ui_component_size(&self->client);
		psy_ui_setrectangle(&r, 10, self->cpy + self->dy, size.width - 20,
			self->lineheight);
		psy_ui_drawsolidrectangle(self->g, r, 0x00292929);
	}
}

void settingsview_drawkey(SettingsView* self, psy_Properties* property,
	int column)
{	
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_ACTION) {
		settingsview_drawbutton(self, property, column + 1);
	} else {
		psy_ui_textout(self->g, self->cpx + column * self->columnwidth,
			self->cpy + self->dy, psy_properties_text(property),
			strlen(psy_properties_text(property)));
	}
}

void settingsview_drawvalue(SettingsView* self, psy_Properties* property,
	int column)
{	
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_BOOL ||
			psy_properties_ischoiceitem(property)) {
		settingsview_drawcheckbox(self, property, column);
	} else
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_STRING) {
		settingsview_drawstring(self, property, column);		
	} else
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_FONT) {
		settingsview_drawstring(self, property, column);		
	} else
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_INTEGER) {
		settingsview_drawinteger(self, property, column);
	}
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_FONT ||
			psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITDIR ||
			psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR) {
		settingsview_drawbutton(self, property, column + 1);
	}
}

void settingsview_drawstring(SettingsView* self, psy_Properties* property,
	int column)
{
	psy_ui_Rectangle r;
	if (self->selected == property) {					
		psy_ui_setbackgroundmode(self->g, psy_ui_OPAQUE);
		psy_ui_setbackgroundcolor(self->g, 0x009B7800);
		psy_ui_settextcolor(self->g, 0x00FFFFFF);
	}				
	psy_ui_setrectangle(&r, self->columnwidth * column, self->cpy + self->dy,
		self->columnwidth, self->lineheight);
	psy_ui_rectangle_expand(&r, 0, -5, 0, 0);
	psy_ui_textoutrectangle(self->g, self->columnwidth * column, self->cpy + self->dy,
		psy_ui_ETO_CLIPPED, r,
		psy_properties_valuestring(property),
		strlen(psy_properties_valuestring(property)));
	psy_ui_setbackgroundcolor(self->g, 0x003E3E3E);
	psy_ui_setbackgroundmode(self->g, psy_ui_TRANSPARENT);
	psy_ui_settextcolor(self->g, 0x00CACACA);
//	if (property->item.hint == PSY_PROPERTY_HINT_EDITDIR) {
//		ui_textout(self->g, 500, 20 + self->cpy, "...", strlen("..."));
//	}		
}

void settingsview_drawinteger(SettingsView* self, psy_Properties* property,
	int column)
{	
	char text[40];
	
	if (psy_properties_hint(property) == PSY_PROPERTY_HINT_INPUT) {
		inputdefiner_setinput(&self->inputdefiner, psy_properties_value(property));
		inputdefiner_text(&self->inputdefiner, text);
	} else
	if (psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR) {
		psy_snprintf(text, 20, "0x%d", psy_properties_value(property));
	} else {
		psy_snprintf(text, 20, "%d", psy_properties_value(property));
	}
	if (psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR) {
		psy_ui_Rectangle r;
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->client);		
		r.left = self->columnwidth * (column + 1) - tm.tmAveCharWidth * 6;
		r.top = self->cpy + self->dy;
		r.right = r.left + tm.tmAveCharWidth * 4;
		r.bottom = r.top + tm.tmHeight + 2;
		psy_ui_drawsolidrectangle(self->g, r, psy_properties_value(property));		
	}
	psy_ui_textout(self->g, self->columnwidth * column, self->cpy + self->dy,
		text, strlen(text));	
}

void settingsview_advanceline(SettingsView* self)
{
	self->cpy += self->lineheight;
}

void settingsview_drawbutton(SettingsView* self, psy_Properties* property,
	int column)
{
	psy_ui_Size size;
	psy_ui_Rectangle r;	
	
	if (psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITDIR ||
		psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR) {
			psy_ui_textout(self->g, self->columnwidth * column + 3,
				self->cpy + self->dy, "...", 3);
	} else
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_FONT) {
		psy_ui_textout(self->g, self->columnwidth * column + 3,
			self->cpy + self->dy, "Choose Font", strlen("Choose Font"));
	} else {
		psy_ui_textout(self->g, self->columnwidth * column + 3,
			self->cpy + self->dy, psy_properties_text(property),
			strlen(psy_properties_text(property)));
	}
	psy_ui_setbackgroundcolor(self->g, 0x003E3E3E);
	psy_ui_setbackgroundmode(self->g, psy_ui_TRANSPARENT);
	psy_ui_settextcolor(self->g, 0x00CACACA);
	if (psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITDIR ||
		(psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR)) {
		size = psy_ui_component_textsize(&self->client, "...");
	} else
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_FONT) {
		size = psy_ui_component_textsize(&self->client, "Choose Font");
	} else {
		size = psy_ui_component_textsize(&self->client,
			psy_properties_text(property));
	}
	r.left = self->columnwidth * column ;
	r.top = self->cpy + self->dy ;
	r.right = r.left + size.width + 6;
	r.bottom = r.top + size.height + 2;	
	psy_ui_drawrectangle(self->g, r);
}

void settingsview_drawcheckbox(SettingsView* self, psy_Properties* property,
	int column)
{
	psy_ui_Rectangle r;
	int checked = 0;
	psy_ui_TextMetric tm;
	psy_ui_Size size;
	psy_ui_Size cornersize;
	psy_ui_Size knobsize;
		
	tm = psy_ui_component_textmetric(&self->component);
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
	psy_ui_setcolor(self->g, 0x00555555);
	psy_ui_drawroundrectangle(self->g, r, cornersize);
	if (psy_properties_ischoiceitem(property)) {
		checked = self->currchoice == self->choicecount;
	} else {
		checked = psy_properties_value(property) != 0;
	}
	if (!checked) {
		r.left = self->columnwidth * column + (int)(tm.tmAveCharWidth * 0.4);
		r.top = self->cpy + self->dy + (self->lineheight - knobsize.height) / 2;
		r.right = r.left + (int)(tm.tmAveCharWidth * 2.5);
		r.bottom = r.top + knobsize.height;
		psy_ui_drawsolidroundrectangle(self->g, r, cornersize, 0x00555555);
	} else {
		r.left = self->columnwidth * column + tm.tmAveCharWidth * 2;
		r.top = self->cpy + self->dy + (self->lineheight - knobsize.height) / 2;
		r.right = r.left + (int)(tm.tmAveCharWidth * 2.5);
		r.bottom = r.top + knobsize.height;
		psy_ui_drawsolidroundrectangle(self->g, r, cornersize, 0x00CACACA);
	}	
}

void settingsview_onmousedown(SettingsView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_ui_component_setfocus(&self->client);
	if (psy_ui_component_visible(&self->edit.component)) {
		settingsview_oneditchange(self, &self->edit);
		psy_ui_component_hide(&self->edit.component);
	}	
	if (psy_ui_component_visible(&self->inputdefiner.component)) {
		settingsview_oninputdefinerchange(self, &self->inputdefiner);
		psy_ui_component_hide(&self->inputdefiner.component);
	}	
	self->selected = 0;
	self->mx = ev->x;
	self->my = ev->y;
	self->choiceproperty = 0;
	self->button = 0;	
	settingsview_preparepropertiesenum(self);
	psy_properties_enumerate(self->properties->children, self,
		settingsview_onpropertieshittestenum);
	if (self->selected) {
		if (self->button &&
				psy_properties_hint(self->selected) == PSY_PROPERTY_HINT_EDITDIR) {
			psy_ui_FolderDialog dialog;			
			char title[_MAX_PATH];
			
			psy_snprintf(title, _MAX_PATH, "%s", psy_properties_text(self->selected));
			title[_MAX_PATH - 1] = '\0';			
			psy_ui_folderdialog_init_all(&dialog, 0, title, "");
			if (psy_ui_folderdialog_execute(&dialog)) {
				psy_properties_write_string(self->selected->parent,
					self->selected->item.key, psy_ui_folderdialog_path(&dialog));
			}
			psy_ui_folderdialog_dispose(&dialog);							
		} else
		if (self->button && psy_properties_hint(self->selected) == PSY_PROPERTY_HINT_EDITCOLOR) {
			psy_ui_ColorDialog colordialog;

			psy_ui_colordialog_init(&colordialog, &self->component);
			if (psy_ui_colordialog_execute(&colordialog)) {
				psy_ui_Color color;

				color = psy_ui_colordialog_color(&colordialog);
				psy_properties_write_int(self->selected->parent,
					self->selected->item.key,
					color);
			}
			psy_ui_colordialog_dispose(&colordialog);
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selected);
		} else
		if (self->button && psy_properties_type(self->selected) == PSY_PROPERTY_TYP_FONT) {
			psy_ui_FontDialog fontdialog;

			psy_ui_fontdialog_init(&fontdialog, &self->component);
			if (psy_ui_fontdialog_execute(&fontdialog)) {				
				psy_ui_FontInfo fontinfo;
				
				fontinfo = psy_ui_fontdialog_fontinfo(&fontdialog);				
				psy_properties_write_font(self->selected->parent,
					self->selected->item.key,
					psy_ui_fontinfo_string(&fontinfo));
			}
			psy_ui_fontdialog_dispose(&fontdialog);
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selected);
		} else
		if (psy_properties_ischoiceitem(self->selected)) {
			self->choiceproperty = self->selected->parent;
			self->choiceproperty->item.value.i = self->choicecount;
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selected);
		} else
		if (psy_properties_type(self->selected) == PSY_PROPERTY_TYP_BOOL) {
			self->selected->item.value.i = self->selected->item.value.i == 0;
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selected);
		} else
		if (psy_properties_type(self->selected) == PSY_PROPERTY_TYP_ACTION) {			
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selected);
		}
	}
	psy_ui_component_invalidate(&self->client);
}

int settingsview_intersects(psy_ui_Rectangle* r, int x, int y)
{
	return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
}

int settingsview_onpropertieshittestenum(SettingsView* self,
	psy_Properties* property, int level)
{
	if (self->cpy != 0 && level == 0 && psy_properties_type(property) == 
			PSY_PROPERTY_TYP_SECTION) {
		settingsview_advanceline(self);
	}
	settingsview_addremoveident(self, level);
	if (psy_properties_hint(property) == PSY_PROPERTY_HINT_HIDE) {
		return 2;
	}
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_CHOICE) {
		self->currchoice = psy_properties_value(property);
		self->choicecount = 0;					
	}	
	if (settingsview_intersectsvalue(self, property, 1)) {
		self->selected = property;		
		return 0;
	}
	if (psy_properties_ischoiceitem(property)) {
		++self->choicecount;	
	}
	settingsview_advanceline(self);
	return 1;	
}

int settingsview_onenumpropertyposition(SettingsView* self,
	psy_Properties* property, int level)
{
	settingsview_addremoveident(self, level);
	if (self->cpy != 0 && level == 0 &&
			psy_properties_type(property) ==  PSY_PROPERTY_TYP_SECTION) {
		settingsview_advanceline(self);
	}
	if (psy_properties_hint(property) == PSY_PROPERTY_HINT_HIDE) {
		return 2;
	}
	if (self->search == property) {		
		return 0;
	}
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_CHOICE) {
		self->currchoice = psy_properties_value(property);
		self->choicecount = 0;					
	}	
	if (psy_properties_ischoiceitem(property)) {
		++self->choicecount;	
	}
	settingsview_advanceline(self);
	return 1;	
}

int settingsview_intersectsvalue(SettingsView* self, psy_Properties* property,
	int column)
{
	int rv = 0;	

	self->button = 0;
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_BOOL) {					
		psy_ui_Rectangle r;
		int checked = 0;
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		
		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_component_textsize(&self->client, "x");
		r.left = self->columnwidth * column;
		r.top = self->cpy + self->dy;
		r.right = r.left + tm.tmAveCharWidth * 4;;
		r.bottom = r.top + size.height + 2;
	
		rv = settingsview_intersects(&r, self->mx, self->my);
	} else
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_INTEGER ||
		psy_properties_type(property) == PSY_PROPERTY_TYP_STRING ||
		psy_properties_type(property) == PSY_PROPERTY_TYP_ACTION ||
		psy_properties_type(property) == PSY_PROPERTY_TYP_FONT) {
		psy_ui_Rectangle r;		
		psy_ui_setrectangle(&r, self->columnwidth * column, 
			self->cpy + self->dy, self->columnwidth, self->lineheight);
		self->selrect = r;
		rv = settingsview_intersects(&r, self->mx, self->my);
		if (!rv && (
				psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITDIR ||
				psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR ||
				psy_properties_type(property) == PSY_PROPERTY_TYP_FONT)) {
			psy_ui_setrectangle(&r, (self->columnwidth * (column + 1)), 
				self->cpy + self->dy, self->columnwidth, self->lineheight);
			self->selrect = r;
			rv = settingsview_intersects(&r, self->mx, self->my);
			if (rv) {
				self->button = 1;
			}
		}
	}
	return rv;
}

void settingsview_onmousedoubleclick(SettingsView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->selected) {
		psy_ui_Component* edit = 0;

		if (self->selected->item.typ == PSY_PROPERTY_TYP_INTEGER) {
			if (psy_properties_hint(self->selected) ==
					PSY_PROPERTY_HINT_INPUT) {
				inputdefiner_setinput(&self->inputdefiner,
					psy_properties_value(self->selected));
				edit = &self->inputdefiner.component;				
			} else {
				char text[40];
				psy_snprintf(text, 40, "%d",
					psy_properties_value(self->selected));
				psy_ui_edit_settext(&self->edit, text);
				edit = &self->edit.component;				
			}
		} else
		if (self->selected->item.typ == PSY_PROPERTY_TYP_STRING) {
			psy_ui_edit_settext(&self->edit, self->selected->item.value.s);
			edit = &self->edit.component;									
		}		
		if (edit) {
			psy_ui_component_setposition(edit,
				self->selrect.left,
				self->selrect.top,
				self->selrect.right - self->selrect.left, 
				self->selrect.bottom - self->selrect.top);
			if (psy_properties_hint(self->selected) !=
					PSY_PROPERTY_HINT_READONLY) {				
				psy_ui_component_show(edit);
				psy_ui_component_setfocus(edit);
			}			
		}
	}
}

void settingsview_oninputdefinerchange(SettingsView* self,
	InputDefiner* sender)
{
	if (self->selected && self->selected->parent) {
		if (self->selected->item.typ == PSY_PROPERTY_TYP_INTEGER) {
			psy_properties_write_int(self->selected->parent,
				self->selected->item.key, self->inputdefiner.input);
		}
		psy_signal_emit(&self->signal_changed, self, 1, self->selected);
	}
}

void settingsview_oneditchange(SettingsView* self, psy_ui_Edit* sender)
{
	if (self->selected && self->selected->parent) {
		if (self->selected->item.typ == PSY_PROPERTY_TYP_STRING) {
			psy_properties_write_string(self->selected->parent,
				self->selected->item.key, psy_ui_edit_text(&self->edit));
		} else 
		if (self->selected->item.typ == PSY_PROPERTY_TYP_INTEGER) {
			psy_properties_write_int(self->selected->parent,
				self->selected->item.key, atoi(psy_ui_edit_text(&self->edit)));
		}
		psy_signal_emit(&self->signal_changed, self, 1, self->selected);
	}
}

void settingsview_oneditkeydown(SettingsView* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_RETURN) {
		psy_ui_component_hide(&self->edit.component);
		psy_ui_component_setfocus(&self->client);
		settingsview_oneditchange(self, &self->edit);
	} else
	if (ev->keycode == psy_ui_KEY_ESCAPE) {
		psy_ui_component_hide(&self->edit.component);
		psy_ui_component_setfocus(&self->client);		
	}
}

void settingsview_onscroll(SettingsView* self, psy_ui_Component* sender,
	int stepx, int stepy)
{
	self->dy += (stepy * sender->scrollstepy);
}

void settingsview_onsize(SettingsView* self, psy_ui_Component* sender,
	psy_ui_Size* size)
{	
	settingsview_adjustscroll(self);
}

void settingsview_ontabbarchange(SettingsView* self, psy_ui_Component* sender,
	int tabindex)
{		
	psy_Properties* p = 0;	
	Tab* tab;

	self->search = 0;
	if (self->properties) {
		p = self->properties->children;
		tab = tabbar_tab(&self->tabbar, tabindex);
		if (tab) {		
			while (p) {
				if (psy_properties_type(p) == PSY_PROPERTY_TYP_SECTION) {				
					if (strcmp(psy_properties_text(p), tab->text) == 0) {
						break;
					}				
				}		
				p = psy_properties_next(p);
			}
		}
		self->search = p;
		if (self->search) {			
			int scrollposition;
			int scrollmin;
			int scrollmax;

			settingsview_preparepropertiesenum(self);
			psy_properties_enumerate(self->properties->children, self,
				settingsview_onenumpropertyposition);
			psy_ui_component_verticalscrollrange(&self->client, &scrollmin,
				&scrollmax);
			scrollposition = self->cpy / self->lineheight;
			if (scrollposition > scrollmax) {
				scrollposition = scrollmax;
			}
			self->dy = -scrollposition * self->lineheight;			
			psy_ui_component_setverticalscrollposition(&self->client,
				scrollposition);
		}	
		psy_ui_component_invalidate(&self->client);
	}
}

void settingsview_adjustscroll(SettingsView* self)
{
	psy_ui_Size size;
	int scrollmax;

	size = psy_ui_component_size(&self->client);
	self->search = 0;
	settingsview_preparepropertiesenum(self);
	psy_properties_enumerate(self->properties->children, self,
		settingsview_onenumpropertyposition);
	self->client.scrollstepy = self->lineheight;
	scrollmax =  (self->cpy - size.height) / self->lineheight + 1;
	if (scrollmax < 0) {
		scrollmax = 0;
	}
	psy_ui_component_setverticalscrollrange(&self->client, 0, scrollmax);
	if (-self->dy / self->lineheight > scrollmax - 1) {
		self->dy = -(scrollmax) * self->lineheight;
		psy_ui_component_setverticalscrollposition(&self->client, scrollmax);
	}	
}

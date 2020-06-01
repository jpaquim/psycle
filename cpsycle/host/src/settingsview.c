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

static void propertiesrenderer_ondraw(PropertiesRenderer*, psy_ui_Component* sender,
	psy_ui_Graphics*);
static int propertiesrenderer_onpropertiesdrawenum(PropertiesRenderer*, psy_Properties*,
	int level);
static int propertiesrenderer_onpropertieshittestenum(PropertiesRenderer*, psy_Properties*,
	int level);
static int propertiesrenderer_onenumpropertyposition(PropertiesRenderer*, psy_Properties*,
	int level);
static void propertiesrenderer_preparepropertiesenum(PropertiesRenderer* self);
static void propertiesrenderer_onmousedown(PropertiesRenderer*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void propertiesrenderer_onmousedoubleclick(PropertiesRenderer*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void propertiesrenderer_oneditchange(PropertiesRenderer*, psy_ui_Edit* sender);
static void propertiesrenderer_oneditkeydown(PropertiesRenderer*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void propertiesrenderer_oninputdefinerchange(PropertiesRenderer*,
	InputDefiner* sender);
static void propertiesrenderer_ondestroy(PropertiesRenderer*, psy_ui_Component* sender);
static void propertiesrenderer_onsize(PropertiesRenderer*, psy_ui_Component* sender, psy_ui_Size*);
static void propertiesrenderer_onscroll(PropertiesRenderer*, psy_ui_Component* sender,
	int stepx, int stepy);
static void propertiesrenderer_drawlinebackground(PropertiesRenderer*,psy_Properties*);
static void propertiesrenderer_drawkey(PropertiesRenderer*, psy_Properties*, int column);
static void propertiesrenderer_drawvalue(PropertiesRenderer*, psy_Properties*, int column);
static void propertiesrenderer_drawstring(PropertiesRenderer*, psy_Properties*,
	int column);
static void propertiesrenderer_drawinteger(PropertiesRenderer*, psy_Properties*,
	int column);
static void propertiesrenderer_drawbutton(PropertiesRenderer*, psy_Properties*,
	int column);
static void propertiesrenderer_drawcheckbox(PropertiesRenderer*, psy_Properties*,
	int column);
static void propertiesrenderer_advanceline(PropertiesRenderer*);
static void propertiesrenderer_countblocklines(PropertiesRenderer*, psy_Properties*, int column);
static void propertiesrenderer_addremoveident(PropertiesRenderer*, int level);
static void propertiesrenderer_addident(PropertiesRenderer*);
static void propertiesrenderer_removeident(PropertiesRenderer*);
static int propertiesrenderer_intersectsvalue(PropertiesRenderer*, psy_Properties*,
	int column);
static int propertiesrenderer_intersectskey(PropertiesRenderer*, psy_Properties*,
	int column);
static void propertiesrenderer_adjustscroll(PropertiesRenderer*);
static int propertiesrenderer_columnwidth(PropertiesRenderer*, int column);
static int propertiesrenderer_columnstart(PropertiesRenderer*, int column);
static void propertiesrenderer_computecolumns(PropertiesRenderer* self, psy_ui_Size*);
static char* strrchrpos(char* str, char c, uintptr_t pos);

void propertiesrenderer_init(PropertiesRenderer* self, psy_ui_Component* parent,
	psy_Properties* properties)
{	
	self->properties = properties;
	psy_ui_component_init(&self->client, parent);
	psy_ui_component_doublebuffer(&self->client);		
	psy_ui_component_setwheelscroll(&self->client, 4);		
	psy_ui_component_showverticalscrollbar(&self->client);	
	psy_signal_connect(&self->client.signal_destroy, self,
		propertiesrenderer_ondestroy);
	psy_signal_connect(&self->client.signal_draw, self, propertiesrenderer_ondraw);
	psy_signal_connect(&self->client.signal_scroll, self,
		propertiesrenderer_onscroll);		
	psy_signal_connect(&self->client.signal_mousedown, self,
		propertiesrenderer_onmousedown);
	psy_signal_connect(&self->client.signal_mousedoubleclick, self,
		propertiesrenderer_onmousedoubleclick);	
	self->selected = 0;
	self->keyselected = 0;
	self->choiceproperty = 0;
	self->dy = 0;
	self->button = 0;
	self->col_perc[0] = 0.4f;
	self->col_perc[1] = 0.4f;
	self->col_perc[2] = 0.2f;
	psy_ui_edit_init(&self->edit, &self->client);
	psy_signal_connect(&self->edit.component.signal_keydown, self,
		propertiesrenderer_oneditkeydown);
	psy_ui_component_hide(&self->edit.component);
	inputdefiner_init(&self->inputdefiner, &self->client);	
	psy_ui_component_hide(&self->inputdefiner.component);		
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_selected);	
	psy_signal_connect(&self->client.signal_size, self,
		propertiesrenderer_onsize);
}

void propertiesrenderer_ondestroy(PropertiesRenderer* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_selected);
}

void propertiesrenderer_ondraw(PropertiesRenderer* self, psy_ui_Component* sender,
	psy_ui_Graphics* g)
{	
	self->g = g;
	psy_ui_setcolor(g, 0x00EAEAEA);
	psy_ui_settextcolor(g, 0x00CACACA);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	propertiesrenderer_preparepropertiesenum(self);
	psy_properties_enumerate(self->properties->children, self,
		propertiesrenderer_onpropertiesdrawenum);
}

void propertiesrenderer_preparepropertiesenum(PropertiesRenderer* self)
{
	psy_ui_TextMetric tm;
	
	tm = psy_ui_component_textmetric(&self->client);
	self->lineheight = (int) (tm.tmHeight * 1.5);
	self->identwidth = tm.tmAveCharWidth * 4;
	self->cpx = 0;
	self->cpy = 0;
	self->numblocklines = 1;
	self->lastlevel = 0;
}

int propertiesrenderer_onpropertiesdrawenum(PropertiesRenderer* self,
	psy_Properties* property, int level)
{			
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	
	propertiesrenderer_addremoveident(self, level);
	if (self->cpy != 0 && level == 0 && psy_properties_type(property) ==
			PSY_PROPERTY_TYP_SECTION) {
		propertiesrenderer_advanceline(self);
	}
	if (psy_properties_hint(property) == PSY_PROPERTY_HINT_HIDE) {		
		return 2;
	}	
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_CHOICE) {
		self->currchoice = psy_properties_value(property);
		self->choicecount = 0;					
	}			
	propertiesrenderer_drawlinebackground(self, property);						
	propertiesrenderer_drawkey(self, property, 0);	
	propertiesrenderer_drawvalue(self, property, 1);
	if (psy_properties_ischoiceitem(property)) {
		++self->choicecount;	
	}
	propertiesrenderer_advanceline(self);
	size = psy_ui_component_size(&self->client);
	tm = psy_ui_component_textmetric(&self->client);
	return self->cpy + self->dy < psy_ui_value_px(&size.height, &tm);
}

void propertiesrenderer_addremoveident(PropertiesRenderer* self, int level)
{
	if (self->lastlevel < level) {
		propertiesrenderer_addident(self);
		self->lastlevel = level;
	} else
	while (self->lastlevel > level) {
		propertiesrenderer_removeident(self);
		--self->lastlevel;
	}	
}

void propertiesrenderer_addident(PropertiesRenderer* self)
{
	self->cpx += self->identwidth;
}

void propertiesrenderer_removeident(PropertiesRenderer* self)
{
	self->cpx -= self->identwidth;
}

void propertiesrenderer_drawlinebackground(PropertiesRenderer* self,
	psy_Properties* property)
{	
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_SECTION) {
		/*psy_ui_Size size;
		psy_ui_Rectangle r;

		size = psy_ui_component_size(&self->client);
		psy_ui_setrectangle(&r, 10, self->cpy + self->dy, size.width - 20,
			self->lineheight);
		psy_ui_drawsolidrectangle(self->g, r, 0x00292929);*/
		psy_ui_settextcolor(self->g, 0x00D1C5B6);
	} else {
		psy_ui_settextcolor(self->g, 0x00CACACA);
	}
}

void propertiesrenderer_drawkey(PropertiesRenderer* self, psy_Properties* property,
	int column)
{	
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_ACTION) {
		propertiesrenderer_drawbutton(self, property, column + 1);
	} else {
		uintptr_t count;
		const char* str;
		uintptr_t numcolumnavgchars;
		psy_ui_TextMetric tm;

		count = strlen(psy_properties_translation(property));
		str = psy_properties_translation(property);
		tm = psy_ui_component_textmetric(&self->client);
		
		numcolumnavgchars = (uintptr_t)(propertiesrenderer_columnwidth(self, column) / (int)(tm.tmAveCharWidth * 1.70));
		while (count > 0) {
			uintptr_t numoutput;
			char* wrap;

			numoutput = min(numcolumnavgchars, count);
			if (numoutput < count) {
				wrap = strrchrpos((char*)str, ' ', numoutput);
				if (wrap) {
					++wrap;
					numoutput = wrap - str;
				}
			}
			psy_ui_textout(self->g, self->cpx + column * propertiesrenderer_columnwidth(self, column),
				self->cpy + self->dy + (self->numblocklines - 1) * self->lineheight, str, numoutput);
			count -= numoutput;
			str += numoutput;
			if (count > 0) {
				++self->numblocklines;
			}
		}
	}
}

char* strrchrpos(char* str, char c, uintptr_t pos)
{
	uintptr_t count;

	if (pos >= strlen(str)) {
		return 0;
	}
	count = pos;
	while (1) {
		if (str[count] == c) {
			return str + count;
		}
		if (count == 0) {
			break;
		}
		--count;
	}
	return 0;
}

void propertiesrenderer_drawvalue(PropertiesRenderer* self, psy_Properties* property,
	int column)
{	
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_BOOL ||
			psy_properties_ischoiceitem(property)) {
		propertiesrenderer_drawcheckbox(self, property, column);
	} else
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_STRING) {
		propertiesrenderer_drawstring(self, property, column);		
	} else
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_FONT) {
		propertiesrenderer_drawstring(self, property, column);		
	} else
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_INTEGER) {
		propertiesrenderer_drawinteger(self, property, column);
	}
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_FONT ||
			psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITDIR ||
			psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR) {
		propertiesrenderer_drawbutton(self, property, column + 1);
	}
}

void propertiesrenderer_drawstring(PropertiesRenderer* self, psy_Properties* property,
	int column)
{
	psy_ui_Rectangle r;
	if (self->selected == property) {					
		psy_ui_setbackgroundmode(self->g, psy_ui_OPAQUE);
		psy_ui_setbackgroundcolor(self->g, 0x009B7800);
		psy_ui_settextcolor(self->g, 0x00FFFFFF);
	}				
	psy_ui_setrectangle(&r, propertiesrenderer_columnwidth(self, column) * column, self->cpy + self->dy,
		propertiesrenderer_columnstart(self, column), self->lineheight);
	psy_ui_rectangle_expand(&r, 0, -5, 0, 0);
	psy_ui_textoutrectangle(self->g, propertiesrenderer_columnstart(self, column), self->cpy + self->dy,
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

void propertiesrenderer_drawinteger(PropertiesRenderer* self, psy_Properties* property,
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
		r.left = propertiesrenderer_columnstart(self, column + 1) - tm.tmAveCharWidth * 6;
		r.top = self->cpy + self->dy;
		r.right = r.left + tm.tmAveCharWidth * 4;
		r.bottom = r.top + tm.tmHeight + 2;
		psy_ui_drawsolidrectangle(self->g, r, psy_properties_value(property));		
	}
	psy_ui_textout(self->g, propertiesrenderer_columnstart(self, column), self->cpy + self->dy,
		text, strlen(text));	
}

void propertiesrenderer_advanceline(PropertiesRenderer* self)
{
	self->cpy += (self->lineheight * self->numblocklines);
	self->numblocklines = 1;
}

void propertiesrenderer_drawbutton(PropertiesRenderer* self, psy_Properties* property,
	int column)
{
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	psy_ui_Rectangle r;	
	
	if (psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITDIR ||
		psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR) {
			psy_ui_textout(self->g, propertiesrenderer_columnstart(self, column) + 3,
				self->cpy + self->dy, "...", 3);
	} else
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_FONT) {
		psy_ui_textout(self->g, propertiesrenderer_columnstart(self, column) + 3,
			self->cpy + self->dy, "Choose Font", strlen("Choose Font"));
	} else {
		psy_ui_textout(self->g, propertiesrenderer_columnstart(self, column) + 3,
			self->cpy + self->dy, psy_properties_translation(property),
			strlen(psy_properties_translation(property)));
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
			psy_properties_translation(property));
	}
	tm = psy_ui_component_textmetric(&self->client);
	r.left = propertiesrenderer_columnstart(self, column);
	r.top = self->cpy + self->dy ;
	r.right = r.left + psy_ui_value_px(&size.width, &tm) + 6;
	r.bottom = r.top + psy_ui_value_px(&size.height, &tm) + 2;
	psy_ui_drawrectangle(self->g, r);
}

void propertiesrenderer_drawcheckbox(PropertiesRenderer* self, psy_Properties* property,
	int column)
{
	psy_ui_Rectangle r;
	int checked = 0;
	psy_ui_TextMetric tm;
	psy_ui_Size size;
	psy_ui_Size cornersize;
	psy_ui_Size knobsize;
	
	tm = psy_ui_component_textmetric(&self->client);
	size.width = psy_ui_value_makeew(4);
	size.height = psy_ui_value_makeeh(1);
	knobsize.width = psy_ui_value_makeew(2);
	knobsize.height = psy_ui_value_makeeh(0.7);
	cornersize.width = psy_ui_value_makeew(0.6);
	cornersize.height = psy_ui_value_makeeh(0.6);
	r.left = propertiesrenderer_columnstart(self, column);
	r.top = self->cpy + self->dy + (self->lineheight -
		psy_ui_value_px(&size.height, &tm)) / 2;
	r.right = r.left + (int)(tm.tmAveCharWidth * 4.8);
	r.bottom = r.top + psy_ui_value_px(&size.height, &tm);
	psy_ui_setcolor(self->g, 0x00555555);
	psy_ui_drawroundrectangle(self->g, r, cornersize);
	if (psy_properties_ischoiceitem(property)) {
		checked = self->currchoice == self->choicecount;
	} else {
		checked = psy_properties_value(property) != 0;
	}
	if (!checked) {
		r.left = propertiesrenderer_columnstart(self, column) + (int)(tm.tmAveCharWidth * 0.4);
		r.top = self->cpy + self->dy + (self->lineheight -
			psy_ui_value_px(&knobsize.height, &tm)) / 2;
		r.right = r.left + (int)(tm.tmAveCharWidth * 2.5);
		r.bottom = r.top + psy_ui_value_px(&knobsize.height, &tm);
		psy_ui_drawsolidroundrectangle(self->g, r, cornersize, 0x00555555);
	} else {
		r.left = propertiesrenderer_columnstart(self, column) + tm.tmAveCharWidth * 2;
		r.top = self->cpy + self->dy + (self->lineheight -
			psy_ui_value_px(&knobsize.height, &tm)) / 2;
		r.right = r.left + (int)(tm.tmAveCharWidth * 2.5);
		r.bottom = r.top + psy_ui_value_px(&knobsize.height, &tm);
		psy_ui_drawsolidroundrectangle(self->g, r, cornersize, 0x00CACACA);
	}	
}

void propertiesrenderer_onmousedown(PropertiesRenderer* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_ui_component_setfocus(&self->client);
	if (psy_ui_component_visible(&self->edit.component)) {
		propertiesrenderer_oneditchange(self, &self->edit);
		psy_ui_component_hide(&self->edit.component);
	}	
	if (psy_ui_component_visible(&self->inputdefiner.component)) {
		propertiesrenderer_oninputdefinerchange(self, &self->inputdefiner);
		psy_ui_component_hide(&self->inputdefiner.component);
	}	
	self->selected = 0;
	self->keyselected = 1;
	self->mx = ev->x;
	self->my = ev->y;
	self->choiceproperty = 0;
	self->button = 0;	
	propertiesrenderer_preparepropertiesenum(self);
	psy_properties_enumerate(self->properties->children, self,
		propertiesrenderer_onpropertieshittestenum);
	if (self->selected) {
		if (self->button &&
				psy_properties_hint(self->selected) == PSY_PROPERTY_HINT_EDITDIR) {
			psy_ui_FolderDialog dialog;			
			char title[_MAX_PATH];
			
			psy_snprintf(title, _MAX_PATH, "%s", psy_properties_translation(self->selected));
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

			psy_ui_colordialog_init(&colordialog, &self->client);
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

			psy_ui_fontdialog_init(&fontdialog, &self->client);
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
		psy_signal_emit(&self->signal_selected, self, 1, self->selected);
	}
	psy_ui_component_invalidate(&self->client);
}

int propertiesrenderer_intersects(psy_ui_Rectangle* r, int x, int y)
{
	return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
}

int propertiesrenderer_onpropertieshittestenum(PropertiesRenderer* self,
	psy_Properties* property, int level)
{
	if (self->cpy != 0 && level == 0 && psy_properties_type(property) == 
			PSY_PROPERTY_TYP_SECTION) {
		propertiesrenderer_advanceline(self);
	}
	propertiesrenderer_addremoveident(self, level);
	if (psy_properties_hint(property) == PSY_PROPERTY_HINT_HIDE) {
		return 2;
	}
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_CHOICE) {
		self->currchoice = psy_properties_value(property);
		self->choicecount = 0;					
	}	
	propertiesrenderer_countblocklines(self, property, 0);
	if (propertiesrenderer_intersectskey(self, property, 0)) {
		self->selected = property;
		self->keyselected = 1;
		return 0;
	}
	if (propertiesrenderer_intersectsvalue(self, property, 1)) {
		self->selected = property;		
		return 0;
	}
	if (psy_properties_ischoiceitem(property)) {
		++self->choicecount;	
	}
	propertiesrenderer_advanceline(self);
	return 1;	
}

int propertiesrenderer_onenumpropertyposition(PropertiesRenderer* self,
	psy_Properties* property, int level)
{
	propertiesrenderer_addremoveident(self, level);
	if (self->cpy != 0 && level == 0 &&
			psy_properties_type(property) ==  PSY_PROPERTY_TYP_SECTION) {
		propertiesrenderer_advanceline(self);
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
	propertiesrenderer_countblocklines(self, property, 0);
	if (psy_properties_ischoiceitem(property)) {
		++self->choicecount;	
	}
	propertiesrenderer_advanceline(self);
	return 1;	
}

void propertiesrenderer_countblocklines(PropertiesRenderer* self, psy_Properties* property, int column)
{
	unsigned int count;
	const char* str;
	uintptr_t numcolumnavgchars;
	psy_ui_TextMetric tm;

	if (propertiesrenderer_columnwidth(self, column) == 0) {
		++self->numblocklines;
		return;
	}

	count = strlen(psy_properties_translation(property));
	str = psy_properties_translation(property);
	tm = psy_ui_component_textmetric(&self->client);

	numcolumnavgchars = (uintptr_t)(propertiesrenderer_columnwidth(self, column) / (int)(tm.tmAveCharWidth * 1.70));	
	while (count > 0) {
		uintptr_t numoutput;
		char* wrap;

		numoutput = min(numcolumnavgchars, count);
		if (numoutput < count) {
			wrap = strrchrpos((char*)str, ' ', numoutput);
			if (wrap) {
				++wrap;
				numoutput = wrap - str;
			}
		}
		count -= numoutput;
		str += numoutput;
		if (count > 0) {
			++self->numblocklines;
		}
	}
}

int propertiesrenderer_intersectsvalue(PropertiesRenderer* self, psy_Properties* property,
	int column)
{
	int rv = 0;	

	self->button = 0;
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_BOOL) {					
		psy_ui_Rectangle r;
		int checked = 0;
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		
		tm = psy_ui_component_textmetric(&self->client);
		size = psy_ui_component_textsize(&self->client, "x");
		r.left = propertiesrenderer_columnstart(self, column);
		r.top = self->cpy + self->dy;
		r.right = r.left + tm.tmAveCharWidth * 4;;
		r.bottom = r.top + psy_ui_value_px(&size.height, &tm) + 2;
	
		rv = propertiesrenderer_intersects(&r, self->mx, self->my);
	} else
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_INTEGER ||
		psy_properties_type(property) == PSY_PROPERTY_TYP_STRING ||
		psy_properties_type(property) == PSY_PROPERTY_TYP_ACTION ||
		psy_properties_type(property) == PSY_PROPERTY_TYP_FONT) {
		psy_ui_Rectangle r;		
		psy_ui_setrectangle(&r, propertiesrenderer_columnstart(self, column),
			self->cpy + self->dy, propertiesrenderer_columnwidth(self, column), self->lineheight);
		self->selrect = r;
		rv = propertiesrenderer_intersects(&r, self->mx, self->my);
		if (!rv && (
				psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITDIR ||
				psy_properties_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR ||
				psy_properties_type(property) == PSY_PROPERTY_TYP_FONT)) {
			psy_ui_setrectangle(&r, (propertiesrenderer_columnstart(self, column + 1)),
				self->cpy + self->dy, propertiesrenderer_columnstart(self, column + 1), self->lineheight);
			self->selrect = r;
			rv = propertiesrenderer_intersects(&r, self->mx, self->my);
			if (rv) {
				self->button = 1;
			}
		}
	}
	return rv;
}

int propertiesrenderer_intersectskey(PropertiesRenderer* self, psy_Properties* property, int column)
{
	int rv = 0;
	psy_ui_Rectangle r;

	self->button = 0;
	psy_ui_setrectangle(&r, propertiesrenderer_columnstart(self, column),
		self->cpy + self->dy, propertiesrenderer_columnwidth(self, column), self->lineheight * self->numblocklines);
	self->selrect = r;
	rv = propertiesrenderer_intersects(&r, self->mx, self->my);
	return rv;
}

void propertiesrenderer_onmousedoubleclick(PropertiesRenderer* self, psy_ui_Component* sender,
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
			if (psy_properties_hint(self->selected) != PSY_PROPERTY_HINT_READONLY) {
				psy_ui_edit_settext(&self->edit, self->selected->item.value.s);
				edit = &self->edit.component;
			}
		}		
		if (edit) {
			psy_ui_component_setposition(edit,
				self->selrect.left,
				self->selrect.top,
				psy_ui_value_makepx(self->selrect.right - self->selrect.left),
				psy_ui_value_makepx(self->selrect.bottom - self->selrect.top));
			if (psy_properties_hint(self->selected) !=
					PSY_PROPERTY_HINT_READONLY) {				
				psy_ui_component_show(edit);
				psy_ui_component_setfocus(edit);
			}			
		}
	}
}

void propertiesrenderer_oninputdefinerchange(PropertiesRenderer* self,
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

void propertiesrenderer_oneditchange(PropertiesRenderer* self, psy_ui_Edit* sender)
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

void propertiesrenderer_oneditkeydown(PropertiesRenderer* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_RETURN) {
		psy_ui_component_hide(&self->edit.component);
		psy_ui_component_setfocus(&self->client);
		propertiesrenderer_oneditchange(self, &self->edit);
	} else
	if (ev->keycode == psy_ui_KEY_ESCAPE) {
		psy_ui_component_hide(&self->edit.component);
		psy_ui_component_setfocus(&self->client);		
	}
}

void propertiesrenderer_onscroll(PropertiesRenderer* self, psy_ui_Component* sender,
	int stepx, int stepy)
{
	self->dy += (stepy * sender->scrollstepy);
}

void propertiesrenderer_onsize(PropertiesRenderer* self, psy_ui_Component* sender,
	psy_ui_Size* size)
{	
	propertiesrenderer_computecolumns(self, size);
	propertiesrenderer_adjustscroll(self);
}

void propertiesrenderer_computecolumns(PropertiesRenderer* self, psy_ui_Size* size)
{
	int column;
	psy_ui_TextMetric tm;
	tm = psy_ui_component_textmetric(&self->client);

	for (column = 0; column < PROPERTIESRENDERER_NUMCOLS; ++column) {
		self->col_width[column] = (int)(self->col_perc[column] *
			psy_ui_value_px(&size->width, &tm));
		if (column == 0) {
			self->col_start[column] = 0;
		} else {
			self->col_start[column] = self->col_start[column - 1] +
				self->col_width[column - 1];
		} 
	}	
}

int propertiesrenderer_columnwidth(PropertiesRenderer* self, int column)
{
	return (column < PROPERTIESRENDERER_NUMCOLS)
		? self->col_width[column]
		: 0;
}

int propertiesrenderer_columnstart(PropertiesRenderer* self, int column)
{
	return (column < PROPERTIESRENDERER_NUMCOLS)
		? self->col_start[column]
		: 0;
}

void propertiesrenderer_adjustscroll(PropertiesRenderer* self)
{
	psy_ui_Size size;	
	psy_ui_TextMetric tm;
	int scrollmax;
	
	size = psy_ui_component_size(&self->client);	
	tm = psy_ui_component_textmetric(&self->client);
	self->search = 0;
	propertiesrenderer_preparepropertiesenum(self);
	psy_properties_enumerate(self->properties->children, self,
		propertiesrenderer_onenumpropertyposition);
	self->client.scrollstepy = self->lineheight;
	scrollmax =  (self->cpy - psy_ui_value_px(&size.height, &tm)) / self->lineheight + 1;
	if (scrollmax < 0) {
		scrollmax = 0;
	}
	psy_ui_component_setverticalscrollrange(&self->client, 0, scrollmax);
	if (-self->dy / self->lineheight > scrollmax - 1) {
		self->dy = -(scrollmax) * self->lineheight;
		psy_ui_component_setverticalscrollposition(&self->client, scrollmax);
	}	
}

static void propertiesview_ondestroy(PropertiesView*, psy_ui_Component* sender);
static void propertiesview_selectsection(PropertiesView*, psy_ui_Component* sender, uintptr_t section);
static void propertiesview_appendtabbarsections(PropertiesView*);
static void propertiesview_ontabbarchange(PropertiesView*, psy_ui_Component* sender,
	int tabindex);
static void propertiesview_onpropertiesrendererchanged(PropertiesView*,
	PropertiesRenderer* sender, psy_Properties*);
static void propertiesview_onpropertiesrendererselected(PropertiesView*,
	PropertiesRenderer* sender, psy_Properties*);

void propertiesview_init(PropertiesView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_Properties* properties)
{
	psy_ui_Margin tabmargin;

	psy_ui_component_init(&self->component, parent);
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_selected);
	psy_signal_connect(&self->component.signal_destroy, self,
		propertiesview_ondestroy);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	psy_ui_component_init(&self->viewtabbar, tabbarparent);
	propertiesrenderer_init(&self->renderer, &self->component, properties);
	psy_ui_component_setalign(&self->renderer.client, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->component.signal_selectsection, self,
		propertiesview_selectsection);
	tabbar_init(&self->tabbar, &self->component);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_RIGHT);
	self->tabbar.tabalignment = psy_ui_ALIGN_RIGHT;
	psy_ui_margin_init_all(&tabmargin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(1),
		psy_ui_value_makeeh(0.5),
		psy_ui_value_makeew(2));
	tabbar_setdefaulttabmargin(&self->tabbar, &tabmargin);
		propertiesview_appendtabbarsections(self);
	psy_signal_connect(&self->renderer.signal_changed, self,
		propertiesview_onpropertiesrendererchanged);
	psy_signal_connect(&self->renderer.signal_selected, self,
		propertiesview_onpropertiesrendererselected);	
}

void propertiesview_ondestroy(PropertiesView* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_selected);
}

void propertiesview_selectsection(PropertiesView* self, psy_ui_Component* sender, uintptr_t section)
{
	tabbar_select(&self->tabbar, (int)section);
}

void propertiesview_appendtabbarsections(PropertiesView* self)
{
	psy_Properties* p;

	for (p = self->renderer.properties->children; p != NULL;
		p = psy_properties_next(p)) {
		if (psy_properties_type(p) == PSY_PROPERTY_TYP_SECTION) {
			tabbar_append(&self->tabbar, psy_properties_translation(p));
		}
	}
	tabbar_select(&self->tabbar, 0);
	psy_signal_connect(&self->tabbar.signal_change, self,
		propertiesview_ontabbarchange);
}

void propertiesview_ontabbarchange(PropertiesView* self, psy_ui_Component* sender,
	int tabindex)
{
	psy_Properties* p = 0;
	Tab* tab;

	self->renderer.search = 0;
	if (self->renderer.properties) {
		p = self->renderer.properties->children;
		tab = tabbar_tab(&self->tabbar, tabindex);
		if (tab) {
			while (p) {
				if (psy_properties_type(p) == PSY_PROPERTY_TYP_SECTION) {
					if (strcmp(psy_properties_translation(p), tab->text) == 0) {
						break;
					}
				}
				p = psy_properties_next(p);
			}
		}
		self->renderer.search = p;
		if (self->renderer.search) {
			int scrollposition;
			int scrollmin;
			int scrollmax;

			propertiesrenderer_preparepropertiesenum(&self->renderer);
			psy_properties_enumerate(self->renderer.properties->children, &self->renderer,
				propertiesrenderer_onenumpropertyposition);
			psy_ui_component_verticalscrollrange(&self->renderer.client, &scrollmin,
				&scrollmax);
			scrollposition = self->renderer.cpy / self->renderer.lineheight;
			if (scrollposition > scrollmax) {
				scrollposition = scrollmax;
			}
			self->renderer.dy = -scrollposition * self->renderer.lineheight;
			psy_ui_component_setverticalscrollposition(&self->renderer.client,
				scrollposition);
		}
		psy_ui_component_invalidate(&self->renderer.client);
	}
}

void propertiesview_onpropertiesrendererchanged(PropertiesView* self,
	PropertiesRenderer* sender, psy_Properties* selected)
{
	psy_signal_emit(&self->signal_changed, self, 1,
		selected);
}

void propertiesview_onpropertiesrendererselected(PropertiesView* self,
	PropertiesRenderer* sender, psy_Properties* selected)
{
	psy_signal_emit(&self->signal_selected, self, 1, selected);
}

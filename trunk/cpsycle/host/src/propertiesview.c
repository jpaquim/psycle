// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "propertiesview.h"

#include <uicolordialog.h>
#include <uifolderdialog.h>
#include <uifontdialog.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../detail/portable.h"

void propertiesrenderlinestate_init(PropertiesRenderLineState* self)
{
	self->cpy = 0;
	self->level = 0;
	self->numlines = 1;
	self->properties = NULL;
}

void propertiesrenderlinestate_dispose(PropertiesRenderLineState* self)
{
}

static void propertiesrenderer_ondraw(PropertiesRenderer*, psy_ui_Graphics*);
static PropertiesRenderLineState* propertiesrenderer_findfirstlinestate(
	PropertiesRenderer*, int y);
static void propertiesrenderer_updatelinestates(PropertiesRenderer*);
static int propertiesrenderer_onpropertiesupdatelinestates(PropertiesRenderer*,
	psy_Property*, int level);
static int propertiesrenderer_onpropertiesdrawenum(PropertiesRenderer*, psy_Property*,
	int level);
static int propertiesrenderer_onpropertieshittestenum(PropertiesRenderer*, psy_Property*,
	int level);
static int propertiesrenderer_onenumpropertyposition(PropertiesRenderer*, psy_Property*,
	int level);
static void propertiesrenderer_preparepropertiesenum(PropertiesRenderer* self);
static void propertiesrenderer_onmousedown(PropertiesRenderer*, psy_ui_MouseEvent*);
static void propertiesrenderer_onmousedoubleclick(PropertiesRenderer*, psy_ui_MouseEvent*);
static void propertiesrenderer_oneditchange(PropertiesRenderer*, psy_ui_Edit* sender);
static void propertiesrenderer_oneditkeydown(PropertiesRenderer*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void propertiesrenderer_oninputdefinerchange(PropertiesRenderer*,
	InputDefiner* sender);
static void propertiesrenderer_ondestroy(PropertiesRenderer*, psy_ui_Component* sender);
static void propertiesrenderer_onalign(PropertiesRenderer*, psy_ui_Component* sender);
static void propertiesrenderer_setlinebackground(PropertiesRenderer*,psy_Property*);
static void propertiesrenderer_drawkey(PropertiesRenderer*, psy_Property*, int column);
static void propertiesrenderer_drawvalue(PropertiesRenderer*, psy_Property*, int column);
static void propertiesrenderer_drawstring(PropertiesRenderer*, psy_Property*,
	int column);
static void propertiesrenderer_drawinteger(PropertiesRenderer*, psy_Property*,
	int column);
static void propertiesrenderer_drawbutton(PropertiesRenderer*, psy_Property*,
	int column);
static void propertiesrenderer_drawcheckbox(PropertiesRenderer*, psy_Property*,
	int column);
static void propertiesrenderer_advanceline(PropertiesRenderer*);
static void propertiesrenderer_countblocklines(PropertiesRenderer*,
	psy_Property*, int column);
static void propertiesrenderer_addremoveident(PropertiesRenderer*, int level);
static void propertiesrenderer_addident(PropertiesRenderer*);
static void propertiesrenderer_removeident(PropertiesRenderer*);
static int propertiesrenderer_intersectsvalue(PropertiesRenderer*, psy_Property*,
	int column);
static int propertiesrenderer_intersectskey(PropertiesRenderer*, psy_Property*,
	int column);
static int propertiesrenderer_columnwidth(PropertiesRenderer*, int column);
static int propertiesrenderer_columnstart(PropertiesRenderer*, int column);
static void propertiesrenderer_computecolumns(PropertiesRenderer* self,
	const psy_ui_Size*);
static char* strrchrpos(char* str, char c, uintptr_t pos);
static void propertiesrenderer_onpreferredsize(PropertiesRenderer*,
	const psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable propertiesrenderer_vtable;
static bool propertiesrenderer_vtable_initialized = FALSE;

static void propertiesrenderer_vtable_init(PropertiesRenderer* self)
{
	if (!propertiesrenderer_vtable_initialized) {
		propertiesrenderer_vtable = *(self->component.vtable);
		propertiesrenderer_vtable.ondraw = (psy_ui_fp_component_ondraw)
			propertiesrenderer_ondraw;
		propertiesrenderer_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			propertiesrenderer_onmousedown;
		propertiesrenderer_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmousedoubleclick)
			propertiesrenderer_onmousedoubleclick;
		propertiesrenderer_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			propertiesrenderer_onpreferredsize;
		propertiesrenderer_vtable_initialized = TRUE;
	}
}

void propertiesrenderer_init(PropertiesRenderer* self, psy_ui_Component* parent,
	psy_Property* properties, Workspace* workspace)
{	
	self->workspace = workspace;
	self->properties = properties;
	psy_ui_component_init(&self->component, parent);
	propertiesrenderer_vtable_init(self);
	self->component.vtable = &propertiesrenderer_vtable;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	psy_signal_connect(&self->component.signal_destroy, self,
		propertiesrenderer_ondestroy);
	self->selected = 0;
	self->keyselected = 0;
	self->choiceproperty = 0;
	self->button = 0;
	self->col_perc[0] = 0.4f;
	self->col_perc[1] = 0.4f;
	self->col_perc[2] = 0.2f;
	self->usefixedwidth = FALSE;
	psy_table_init(&self->linestates);
	psy_ui_edit_init(&self->edit, &self->component);
	psy_signal_connect(&self->edit.component.signal_keydown, self,
		propertiesrenderer_oneditkeydown);
	psy_ui_component_hide(&self->edit.component);
	inputdefiner_init(&self->inputdefiner, &self->component);
	psy_ui_component_hide(&self->inputdefiner.component);
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_selected);
	psy_signal_connect(&self->component.signal_align, self,
		propertiesrenderer_onalign);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
	self->valuecolour = psy_ui_colour_make(0x00CACACA);
	self->sectioncolour = psy_ui_colour_make(0x00CACACA);
	self->separatorcolour = psy_ui_colour_make(0x00333333);
	self->valueselcolour = psy_ui_colour_make(0x00FFFFFF);
	self->valueselbackgroundcolour = psy_ui_colour_make(0x009B7800);
}

void propertiesrenderer_ondestroy(PropertiesRenderer* self, psy_ui_Component* sender)
{
	psy_table_disposeall(&self->linestates, (psy_fp_disposefunc)
		propertiesrenderlinestate_dispose);
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_selected);
}

void propertiesrenderer_setfixedwidth(PropertiesRenderer* self, psy_ui_Value width)
{
	self->fixedwidth = width;
	self->usefixedwidth = TRUE;
}

void propertiesrenderer_ondraw(PropertiesRenderer* self, psy_ui_Graphics* g)
{	
	self->g = g;
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	self->linestate_clipstart = propertiesrenderer_findfirstlinestate(
		self, g->clip.top);
	propertiesrenderer_preparepropertiesenum(self);
	if (self->linestate_clipstart) {
		self->cpy = self->linestate_clipstart->cpy;
		self->cpx = self->linestate_clipstart->cpx;
	}
	psy_property_enumerate(self->properties, self,
		(psy_PropertyCallback)propertiesrenderer_onpropertiesdrawenum);	
}

void propertiesrenderer_preparepropertiesenum(PropertiesRenderer* self)
{
	psy_ui_TextMetric tm;
	
	tm = psy_ui_component_textmetric(&self->component);
	self->textheight = tm.tmHeight;
	self->lineheight = (int) (self->textheight * 1.5);
	self->centery = (self->lineheight - self->textheight) / 2;
	self->identwidth = tm.tmAveCharWidth * 4;
	self->cpx = tm.tmAveCharWidth * 2;
	self->cpy = 0;
	self->numblocklines = 1;
	self->lastlevel = 0;
}

void propertiesrenderer_updatelinestates(PropertiesRenderer* self)
{
	psy_table_disposeall(&self->linestates, (psy_fp_disposefunc)
		propertiesrenderlinestate_dispose);
	psy_table_init(&self->linestates);
	propertiesrenderer_preparepropertiesenum(self);
	self->currlinestatecount = 0;
	psy_property_enumerate(self->properties, self,
		(psy_PropertyCallback)
		propertiesrenderer_onpropertiesupdatelinestates);
}

PropertiesRenderLineState* propertiesrenderer_findfirstlinestate(
	PropertiesRenderer* self, int y)
{
	PropertiesRenderLineState* rv;
	uintptr_t i;
	
	rv = NULL;
	for (i = 0; i < psy_table_size(&self->linestates); ++i) {
		PropertiesRenderLineState* linestate;

		linestate = (PropertiesRenderLineState*)psy_table_at(&self->linestates,
			i);
		if (linestate && linestate->cpy + (linestate->numlines + 1) *
				self->lineheight >= y) {
			rv = linestate;
			break;
		}
	}
	return rv;
}

int propertiesrenderer_onpropertiesupdatelinestates(PropertiesRenderer* self,
	psy_Property* property, int level)
{	
	PropertiesRenderLineState* linestate;

	linestate = (PropertiesRenderLineState*)
		malloc(sizeof(PropertiesRenderLineState));
	assert(linestate);
	propertiesrenderlinestate_init(linestate);
	linestate->cpy = self->cpy;
	linestate->cpx = self->cpx;
	linestate->level = level;
	linestate->properties = property;
	psy_table_insert(&self->linestates, self->currlinestatecount,
		(void*)linestate);
	++self->currlinestatecount;	
	propertiesrenderer_addremoveident(self, level);
	if (self->cpy != 0 && level == 0 && psy_property_type(property) ==
			PSY_PROPERTY_TYPE_SECTION) {
		propertiesrenderer_advanceline(self);
	}
	if (psy_property_hint(property) == PSY_PROPERTY_HINT_HIDE) {
		return 2;
	}
	propertiesrenderer_countblocklines(self, property, 0);
	linestate->numlines = self->numblocklines;
	propertiesrenderer_advanceline(self);	
	return 1;
}

int propertiesrenderer_onpropertiesdrawenum(PropertiesRenderer* self,
	psy_Property* property, int level)
{			
	psy_ui_Size size;
	psy_ui_TextMetric tm;		

	if (self->linestate_clipstart && self->linestate_clipstart->properties != property) {
		if (self->lastlevel < level) {
			self->lastlevel = level;
		} else
		while (self->lastlevel > level) {
			--self->lastlevel;
		}
		return 1;
	}
	self->linestate_clipstart = NULL;
	propertiesrenderer_addremoveident(self, level);
	if (self->cpy != 0 && level == 0 && psy_property_type(property) ==
			PSY_PROPERTY_TYPE_SECTION) {
		propertiesrenderer_advanceline(self);
	}
	if (psy_property_hint(property) == PSY_PROPERTY_HINT_HIDE) {		
		return 2;
	}			
	propertiesrenderer_setlinebackground(self, property);
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_SECTION) {		
		psy_ui_IntSize intsize;

		intsize = psy_ui_intsize_init_size(psy_ui_component_size(&self->component),
			&tm);
		psy_ui_setcolour(self->g, self->separatorcolour);
		psy_ui_drawline(self->g, self->cpx, self->cpy, intsize.width, self->cpy);
	}
	propertiesrenderer_drawkey(self, property, 0);
	if (self->col_perc[1] > 0.0) {
		propertiesrenderer_drawvalue(self, property, 1);
	}	
	propertiesrenderer_advanceline(self);
	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	return (self->cpy - psy_ui_component_scrolltop(&self->component))
		< psy_ui_value_px(&size.height, &tm);
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

void propertiesrenderer_setlinebackground(PropertiesRenderer* self,
	psy_Property* property)
{	
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_SECTION) {		
		psy_ui_settextcolour(self->g, psy_ui_component_colour(&self->component)); // 0x00D1C5B6);
	} else {
		psy_ui_settextcolour(self->g, psy_ui_component_colour(&self->component));
	}
}

void propertiesrenderer_drawkey(PropertiesRenderer* self, psy_Property* property,
	int column)
{	
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_ACTION) {
		propertiesrenderer_drawbutton(self, property, column + 1);
	} else {
		uintptr_t count;
		const char* str;
		uintptr_t numcolumnavgchars;
		psy_ui_TextMetric tm;

		count = strlen(psy_property_translation(property));
		str = psy_property_translation(property);
		tm = psy_ui_component_textmetric(&self->component);

		psy_ui_setbackgroundmode(self->g, psy_ui_TRANSPARENT);
		psy_ui_settextcolour(self->g, psy_ui_component_colour(&self->component));
		
		numcolumnavgchars = (uintptr_t)(propertiesrenderer_columnwidth(self, column) /
			(int)(tm.tmAveCharWidth * 1.70));
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
			if (numoutput == 0) {
				break;
			}
			psy_ui_textout(self->g, self->cpx + column * propertiesrenderer_columnwidth(self, column),
				self->cpy + (self->numblocklines - 1) * self->lineheight + self->centery, str, numoutput);
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

void propertiesrenderer_drawvalue(PropertiesRenderer* self, psy_Property* property,
	int column)
{
	if (self->selected == property) {
		psy_ui_setbackgroundmode(self->g, psy_ui_OPAQUE);
		psy_ui_setbackgroundcolour(self->g, self->valueselbackgroundcolour);
		psy_ui_settextcolour(self->g, self->valueselcolour);
	} else {
		psy_ui_setbackgroundmode(self->g, psy_ui_TRANSPARENT);
		if (psy_property_readonly(property)) {
			psy_ui_settextcolour(self->g, psy_ui_component_colour(
				&self->component));
		} else {
			psy_ui_settextcolour(self->g, self->valuecolour);
		}
	}
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_BOOL ||
			psy_property_ischoiceitem(property)) {
		propertiesrenderer_drawcheckbox(self, property, column);
	} else
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_STRING) {
		propertiesrenderer_drawstring(self, property, column);		
	} else
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_FONT) {
		propertiesrenderer_drawstring(self, property, column);		
	} else
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_INTEGER) {
		propertiesrenderer_drawinteger(self, property, column);
	}
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_FONT ||
			psy_property_hint(property) == PSY_PROPERTY_HINT_EDITDIR ||
			psy_property_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR) {
		propertiesrenderer_drawbutton(self, property, column + 1);
	}
}

void propertiesrenderer_drawstring(PropertiesRenderer* self, psy_Property* property,
	int column)
{
	psy_ui_Rectangle r;
	
	psy_ui_setrectangle(&r, propertiesrenderer_columnwidth(self, column) * column,
		self->cpy + self->centery,
		propertiesrenderer_columnstart(self, column), self->textheight);
	psy_ui_rectangle_expand(&r, 0, -5, 0, 0);
	psy_ui_textoutrectangle(self->g, propertiesrenderer_columnstart(self, column),
		self->cpy + self->centery,
		psy_ui_ETO_CLIPPED, r,
		psy_property_item_str(property),
		strlen(psy_property_item_str(property)));
	//psy_ui_setbackgroundcolour(self->g, psy_ui_colour_make(0x003E3E3E));		
}

void propertiesrenderer_drawinteger(PropertiesRenderer* self, psy_Property* property,
	int column)
{	
	char text[256];
	
	if (psy_property_hint(property) == PSY_PROPERTY_HINT_SHORTCUT) {
		inputdefiner_setinput(&self->inputdefiner, psy_property_item_int(property));
		inputdefiner_text(&self->inputdefiner, text);
	} else
	if (psy_property_hint(property) == PSY_PROPERTY_HINT_EDITHEX) {
		psy_snprintf(text, 20, "%X", psy_property_item_int(property));
	} else
	if (psy_property_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR) {
		psy_snprintf(text, 20, "0x%d", psy_property_item_int(property));
	} else {
		psy_snprintf(text, 20, "%d", psy_property_item_int(property));
	}
	if (psy_property_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR) {
		psy_ui_Rectangle r;
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->component);		
		r.left = propertiesrenderer_columnstart(self, column + 1) - tm.tmAveCharWidth * 6;
		r.top = self->cpy + self->centery;
		r.right = r.left + tm.tmAveCharWidth * 4;
		r.bottom = r.top + self->textheight + 2;
		psy_ui_drawsolidrectangle(self->g, r, psy_ui_colour_make(psy_property_item_int(property)));
	}
	psy_ui_textout(self->g, propertiesrenderer_columnstart(self, column),
		self->cpy + self->centery,
		text, strlen(text));
	if (psy_property_int_hasrange(property) && !psy_property_readonly(property)) {
		psy_snprintf(text, 256, "from %d to %d", property->item.min, property->item.max);
		psy_ui_textout(self->g, propertiesrenderer_columnstart(self, column + 1),
			self->cpy + self->centery,
			text, strlen(text));
	}
}

void propertiesrenderer_advanceline(PropertiesRenderer* self)
{
	self->cpy += (self->lineheight * self->numblocklines);
	self->numblocklines = 1;
}

void propertiesrenderer_drawbutton(PropertiesRenderer* self, psy_Property* property,
	int column)
{
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	psy_ui_Rectangle r;	
	
	psy_ui_setcolour(self->g, psy_ui_component_colour(&self->component));
	psy_ui_setbackgroundcolour(self->g, psy_ui_component_backgroundcolour(
		&self->component));
	psy_ui_setbackgroundmode(self->g, psy_ui_TRANSPARENT);
	psy_ui_settextcolour(self->g, psy_ui_component_colour(&self->component));

	if (psy_property_hint(property) == PSY_PROPERTY_HINT_EDITDIR ||
		psy_property_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR) {
			psy_ui_textout(self->g, propertiesrenderer_columnstart(self, column) + 3,
				self->cpy, "...", 3);
	} else
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_FONT) {
		const char* choosefonttext;

		choosefonttext = workspace_translate(self->workspace, "settingsview.choose-font");
		psy_ui_textout(self->g, propertiesrenderer_columnstart(self, column) + 3,
			self->cpy, choosefonttext, strlen(choosefonttext));
	} else {
		psy_ui_textout(self->g, propertiesrenderer_columnstart(self, column) + 3,
			self->cpy, psy_property_translation(property),
			strlen(psy_property_translation(property)));
	}	
	if (psy_property_hint(property) == PSY_PROPERTY_HINT_EDITDIR ||
		(psy_property_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR)) {
		size = psy_ui_component_textsize(&self->component, "...");
	} else
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_FONT) {
		const char* choosefonttext;

		choosefonttext = workspace_translate(self->workspace, "settingsview.choose-font");
		size = psy_ui_component_textsize(&self->component, choosefonttext);
	} else {
		size = psy_ui_component_textsize(&self->component,
			psy_property_translation(property));
	}
	tm = psy_ui_component_textmetric(&self->component);
	r.left = propertiesrenderer_columnstart(self, column);
	r.top = self->cpy;
	r.right = r.left + psy_ui_value_px(&size.width, &tm) + 6;
	r.bottom = r.top + psy_ui_value_px(&size.height, &tm) + 2;
	psy_ui_drawrectangle(self->g, r);
}

void propertiesrenderer_drawcheckbox(PropertiesRenderer* self, psy_Property* property,
	int column)
{
	psy_ui_Rectangle r;
	int checked = 0;
	psy_ui_TextMetric tm;
	psy_ui_Size size;
	psy_ui_Size cornersize;
	psy_ui_Size knobsize;
	
	tm = psy_ui_component_textmetric(&self->component);
	size.width = psy_ui_value_makeew(4);
	size.height = psy_ui_value_makeeh(1);
	knobsize.width = psy_ui_value_makeew(2);
	knobsize.height = psy_ui_value_makeeh(0.7);
	cornersize.width = psy_ui_value_makeew(0.6);
	cornersize.height = psy_ui_value_makeeh(0.6);
	r.left = propertiesrenderer_columnstart(self, column);
	r.top = self->cpy + (self->lineheight -
		psy_ui_value_px(&size.height, &tm)) / 2;
	r.right = r.left + (int)(tm.tmAveCharWidth * 4.8);
	r.bottom = r.top + psy_ui_value_px(&size.height, &tm);
	psy_ui_setcolour(self->g, psy_ui_colour_make(0x00555555));
	psy_ui_drawroundrectangle(self->g, r, cornersize);
	if (psy_property_ischoiceitem(property)) {
		if (psy_property_parent(property)) {
			checked = psy_property_at_choice(psy_property_parent(property)) ==
				property;
		} else {
			checked = FALSE;
		}
	} else {
		checked = psy_property_item_int(property) != 0;
	}
	if (!checked) {
		r.left = propertiesrenderer_columnstart(self, column) + (int)(tm.tmAveCharWidth * 0.4);
		r.top = self->cpy + (self->lineheight -
			psy_ui_value_px(&knobsize.height, &tm)) / 2;
		r.right = r.left + (int)(tm.tmAveCharWidth * 2.5);
		r.bottom = r.top + psy_ui_value_px(&knobsize.height, &tm);
		psy_ui_drawsolidroundrectangle(self->g, r, cornersize,
			psy_ui_colour_make(0x00555555));
	} else {
		r.left = propertiesrenderer_columnstart(self, column) + tm.tmAveCharWidth * 2;
		r.top = self->cpy + (self->lineheight -
			psy_ui_value_px(&knobsize.height, &tm)) / 2;
		r.right = r.left + (int)(tm.tmAveCharWidth * 2.5);
		r.bottom = r.top + psy_ui_value_px(&knobsize.height, &tm);
		psy_ui_drawsolidroundrectangle(self->g, r, cornersize,
			psy_ui_colour_make(0x00CACACA));
	}	
}

void propertiesrenderer_onmousedown(PropertiesRenderer* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_setfocus(&self->component);
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
	psy_property_enumerate(self->properties, self,
		(psy_PropertyCallback)propertiesrenderer_onpropertieshittestenum);
	if (self->selected) {
		printf("selected\n");
		if (self->button && psy_property_hint(self->selected) ==
				PSY_PROPERTY_HINT_EDITDIR) {
			psy_ui_FolderDialog dialog;
						
			psy_ui_folderdialog_init_all(&dialog, 0, psy_property_translation(
				self->selected), "");
			if (psy_ui_folderdialog_execute(&dialog)) {
				psy_property_set_str(self->selected->parent,
					self->selected->item.key, psy_ui_folderdialog_path(&dialog));
			}
			psy_ui_folderdialog_dispose(&dialog);							
		} else if (self->button && psy_property_hint(self->selected) ==
				PSY_PROPERTY_HINT_EDITCOLOR) {
			psy_ui_ColourDialog colourdialog;

			psy_ui_colourdialog_init(&colourdialog, &self->component);
			if (psy_ui_colourdialog_execute(&colourdialog)) {
				psy_ui_Colour colour;

				colour = psy_ui_colourdialog_colour(&colourdialog);
				psy_property_set_int(self->selected->parent,
					self->selected->item.key,
					colour.value);
			}
			psy_ui_colourdialog_dispose(&colourdialog);
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selected);
		} else if (self->button && psy_property_type(self->selected) ==
				PSY_PROPERTY_TYPE_FONT) {
			psy_ui_FontDialog fontdialog;
			psy_ui_FontInfo fontinfo;

			psy_ui_fontdialog_init(&fontdialog, &self->component);
			psy_ui_fontinfo_init_string(&fontinfo,
				psy_property_item_str(self->selected));
			psy_ui_fontdialog_setfontinfo(&fontdialog, fontinfo);
			if (psy_ui_fontdialog_execute(&fontdialog)) {				
				psy_ui_FontInfo fontinfo;
				
				fontinfo = psy_ui_fontdialog_fontinfo(&fontdialog);				
				psy_property_set_font(self->selected->parent,
					self->selected->item.key,
					psy_ui_fontinfo_string(&fontinfo));
			}
			psy_ui_fontdialog_dispose(&fontdialog);
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selected);
		} else if (psy_property_ischoiceitem(self->selected)) {
			psy_List* p;
			int choicecount;

			self->choiceproperty = psy_property_parent(self->selected);
			choicecount = 0;
			p = psy_property_children(self->choiceproperty);
			while (p != NULL) {
				if (psy_list_entry(p) == self->selected) {
					break;
				}
				++choicecount;
				psy_list_next(&p);				
			}
			psy_property_setitem_int(self->choiceproperty, choicecount);
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selected);
		} else if (psy_property_type(self->selected) ==
				PSY_PROPERTY_TYPE_BOOL) {
			psy_property_setitem_bool(self->selected, !psy_property_item_bool(
				self->selected));
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selected);
		} else if (psy_property_type(self->selected) ==
				PSY_PROPERTY_TYPE_ACTION) {
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selected);
		}
		psy_signal_emit(&self->signal_selected, self, 1, self->selected);
	}
	psy_ui_component_invalidate(&self->component);
}

int propertiesrenderer_onpropertieshittestenum(PropertiesRenderer* self,
	psy_Property* property, int level)
{
	if (self->cpy != 0 && level == 0 && psy_property_type(property) == 
			PSY_PROPERTY_TYPE_SECTION) {
		propertiesrenderer_advanceline(self);
	}
	propertiesrenderer_addremoveident(self, level);
	if (psy_property_hint(property) == PSY_PROPERTY_HINT_HIDE) {
		return 2;
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
	propertiesrenderer_advanceline(self);
	return 1;	
}

int propertiesrenderer_onenumpropertyposition(PropertiesRenderer* self,
	psy_Property* property, int level)
{
	propertiesrenderer_addremoveident(self, level);
	if (self->cpy != 0 && level == 0 && psy_property_type(property) ==
			PSY_PROPERTY_TYPE_SECTION) {
		propertiesrenderer_advanceline(self);
	}
	if (psy_property_hint(property) == PSY_PROPERTY_HINT_HIDE) {
		return 2;
	}
	if (self->search == property) {		
		return 0;
	}	
	propertiesrenderer_countblocklines(self, property, 0);	
	propertiesrenderer_advanceline(self);
	return 1;	
}

void propertiesrenderer_countblocklines(PropertiesRenderer* self, psy_Property*
	property, int column)
{
	unsigned int count;
	const char* str;
	uintptr_t numcolumnavgchars;
	psy_ui_TextMetric tm;

	if (propertiesrenderer_columnwidth(self, column) == 0) {
		++self->numblocklines;
		return;
	}

	count = strlen(psy_property_translation(property));
	str = psy_property_translation(property);
	tm = psy_ui_component_textmetric(&self->component);
	numcolumnavgchars = (uintptr_t)
		(propertiesrenderer_columnwidth(self, column) /
		(int)(tm.tmAveCharWidth * 1.70));
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

int propertiesrenderer_intersectsvalue(PropertiesRenderer* self, psy_Property*
	property, int column)
{
	int rv = 0;	

	self->button = 0;
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_BOOL) {					
		psy_ui_Rectangle r;
		int checked = 0;
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		
		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_component_textsize(&self->component, "x");
		r.left = propertiesrenderer_columnstart(self, column);
		r.top = self->cpy;
		r.right = r.left + tm.tmAveCharWidth * 4;;
		r.bottom = r.top + psy_ui_value_px(&size.height, &tm) + 2;
	
		rv = psy_ui_rectangle_intersect(&r, self->mx, self->my);
	} else if (psy_property_type(property) == PSY_PROPERTY_TYPE_INTEGER ||
			psy_property_type(property) == PSY_PROPERTY_TYPE_STRING ||
			psy_property_type(property) == PSY_PROPERTY_TYPE_ACTION ||
			psy_property_type(property) == PSY_PROPERTY_TYPE_FONT) {
		psy_ui_Rectangle r;		
		psy_ui_setrectangle(&r, propertiesrenderer_columnstart(self, column),
			self->cpy,
			propertiesrenderer_columnwidth(self, column), self->lineheight);
		self->selrect = r;
		rv = psy_ui_rectangle_intersect(&r, self->mx, self->my);
		if (!rv && (
				psy_property_hint(property) == PSY_PROPERTY_HINT_EDITDIR ||
				psy_property_hint(property) == PSY_PROPERTY_HINT_EDITCOLOR ||
				psy_property_type(property) == PSY_PROPERTY_TYPE_FONT)) {
			psy_ui_setrectangle(&r,
				(propertiesrenderer_columnstart(self, column + 1)),
				self->cpy, propertiesrenderer_columnstart(self, column + 1),
				self->lineheight);
			self->selrect = r;
			rv = psy_ui_rectangle_intersect(&r, self->mx, self->my);
			if (rv) {
				self->button = 1;
			}
		}
	}
	return rv;
}

int propertiesrenderer_intersectskey(PropertiesRenderer* self, psy_Property*
	property, int column)
{
	int rv = 0;
	psy_ui_Rectangle r;

	self->button = 0;
	psy_ui_setrectangle(&r,
		propertiesrenderer_columnstart(self, column),
		self->cpy,
		propertiesrenderer_columnwidth(self, column),
		self->lineheight * self->numblocklines);
	self->selrect = r;
	rv = psy_ui_rectangle_intersect(&r, self->mx, self->my);
	return rv;
}

void propertiesrenderer_onmousedoubleclick(PropertiesRenderer* self,
	psy_ui_MouseEvent* ev)
{
	if (self->selected) {
		psy_ui_Component* edit = 0;

		if (psy_property_type(self->selected) == PSY_PROPERTY_TYPE_INTEGER) {
			if (psy_property_hint(self->selected) ==
					PSY_PROPERTY_HINT_SHORTCUT) {
				inputdefiner_setinput(&self->inputdefiner,
					psy_property_item_int(self->selected));
				edit = &self->inputdefiner.component;				
			} else {
				char text[40];
				if (psy_property_hint(self->selected) == PSY_PROPERTY_HINT_EDITHEX) {
					psy_snprintf(text, 40, "%X",
						psy_property_item_int(self->selected));
				} else {
					psy_snprintf(text, 40, "%d",
						psy_property_item_int(self->selected));
				}
				psy_ui_edit_settext(&self->edit, text);
				edit = &self->edit.component;				
			}
		} else if (psy_property_type(self->selected) == PSY_PROPERTY_TYPE_STRING) {
			if (!psy_property_readonly(self->selected)) {
				psy_ui_edit_settext(&self->edit, psy_property_item_str(
					self->selected));
				edit = &self->edit.component;
			}
		}		
		if (edit) {
			psy_ui_component_setposition(edit,
				psy_ui_point_make(
					psy_ui_value_makepx(self->selrect.left -
						psy_ui_component_scrollleft(&self->component)),
					psy_ui_value_makepx(self->selrect.top + self->centery -
						psy_ui_component_scrolltop(&self->component))),
				psy_ui_size_make(
					psy_ui_value_makepx(self->selrect.right - self->selrect.left),
					psy_ui_value_makepx(self->textheight + 2)));
			if (!psy_property_readonly(self->selected)) {				
				psy_ui_component_show(edit);
				psy_ui_component_setfocus(edit);
			}			
		}
	}
}

void propertiesrenderer_oninputdefinerchange(PropertiesRenderer* self,
	InputDefiner* sender)
{
	if (self->selected && psy_property_type(self->selected) ==
			PSY_PROPERTY_TYPE_INTEGER) {		
		psy_property_setitem_int(self->selected,
			inputdefiner_input(&self->inputdefiner));
	}
	psy_signal_emit(&self->signal_changed, self, 1, self->selected);
}

void propertiesrenderer_oneditchange(PropertiesRenderer* self, psy_ui_Edit* sender)
{
	if (self->selected) {
		switch (psy_property_type(self->selected)) {
			case PSY_PROPERTY_TYPE_INTEGER:
				if (psy_property_hint(self->selected) == PSY_PROPERTY_HINT_EDITHEX) {
					psy_property_setitem_int(self->selected,
						strtol(psy_ui_edit_text(&self->edit), NULL, 16));
				} else {
					psy_property_setitem_int(self->selected,
						atoi(psy_ui_edit_text(&self->edit)));
				}
			break;
			case PSY_PROPERTY_TYPE_STRING:
				psy_property_setitem_str(self->selected,
					psy_ui_edit_text(&self->edit));
				break;		
			default:
				break;
		}		
		psy_signal_emit(&self->signal_changed, self, 1, self->selected);
		self->selected = NULL;
	}
}

void propertiesrenderer_oneditkeydown(PropertiesRenderer* self,
	psy_ui_Component* sender, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_RETURN) {
		psy_ui_component_hide(&self->edit.component);
		psy_ui_component_setfocus(&self->component);
		propertiesrenderer_oneditchange(self, &self->edit);
	} else if (ev->keycode == psy_ui_KEY_ESCAPE) {
		psy_ui_component_hide(&self->edit.component);
		psy_ui_component_setfocus(&self->component);		
	} else if (psy_property_hint(self->selected) == PSY_PROPERTY_HINT_EDITHEX) {
		if ((ev->keycode >= psy_ui_KEY_DIGIT0 && ev->keycode <= psy_ui_KEY_DIGIT9) ||
			(ev->keycode >= psy_ui_KEY_A && ev->keycode <= psy_ui_KEY_F) ||
			(ev->keycode < psy_ui_KEY_HELP)) {
			return;
		} else {
			psy_ui_keyevent_preventdefault(ev);
		}
	}	
}

void propertiesrenderer_onalign(PropertiesRenderer* self, psy_ui_Component*
	sender)
{	
	psy_ui_Size size;

	size = psy_ui_component_size(&self->component);
	propertiesrenderer_computecolumns(self, &size);
	propertiesrenderer_updatelinestates(self);	
}

void propertiesrenderer_computecolumns(PropertiesRenderer* self,
	const psy_ui_Size* size)
{
	int column;
	psy_ui_TextMetric tm;
	tm = psy_ui_component_textmetric(&self->component);

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

void propertiesrenderer_onpreferredsize(PropertiesRenderer* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	float col_perc[PROPERTIESRENDERER_NUMCOLS];
	int col_width[PROPERTIESRENDERER_NUMCOLS];
	int col_start[PROPERTIESRENDERER_NUMCOLS];

	memcpy(col_perc, self->col_perc, sizeof(col_perc));
	memcpy(col_width, self->col_width, sizeof(col_width));
	memcpy(col_start, self->col_start, sizeof(col_start));	
	if (!self->usefixedwidth) {
		propertiesrenderer_computecolumns(self, limit);
		rv->width = limit->width;
	} else {
		psy_ui_Size fixedsize;

		fixedsize.width = self->fixedwidth;
		fixedsize.height = limit->width;
		rv->width = self->fixedwidth;
		propertiesrenderer_computecolumns(self, limit);
	}	
	self->search = 0;
	propertiesrenderer_preparepropertiesenum(self);
	psy_property_enumerate(self->properties, self,
		(psy_PropertyCallback)propertiesrenderer_onenumpropertyposition);
	self->component.scrollstepy = self->lineheight;
	rv->height = psy_ui_value_makepx(self->cpy);	
	memcpy(self->col_perc, col_perc, sizeof(col_perc));
	memcpy(self->col_width, col_width, sizeof(col_width));
	memcpy(self->col_start, col_start, sizeof(col_start));
}

// PropertiesView
// prototypes
static void propertiesview_ondestroy(PropertiesView*, psy_ui_Component* sender);
static void propertiesview_selectsection(PropertiesView*, psy_ui_Component* sender,
	uintptr_t section);
static void propertiesview_updatetabbarsections(PropertiesView*);
static void propertiesview_ontabbarchange(PropertiesView*, psy_ui_Component* sender,
	int tabindex);
static void propertiesview_onpropertiesrendererchanged(PropertiesView*,
	PropertiesRenderer* sender, psy_Property*);
static void propertiesview_onpropertiesrendererselected(PropertiesView*,
	PropertiesRenderer* sender, psy_Property*);
static void propertiesview_onlanguagechanged(PropertiesView*, Translator*);
static void propertiesview_translate(PropertiesView*);
static int propertiesview_onchangelanguageenum(PropertiesView*,
	psy_Property*, int level);
// implementation
void propertiesview_init(PropertiesView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_Property* properties,
	Workspace* workspace)
{
	psy_ui_Margin tabmargin;

	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_selected);
	psy_signal_connect(&self->component.signal_destroy, self,
		propertiesview_ondestroy);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	psy_ui_component_init(&self->viewtabbar, tabbarparent);
	propertiesrenderer_init(&self->renderer, &self->component, properties,
		workspace);
	psy_ui_scroller_init(&self->scroller, &self->renderer.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->component.signal_selectsection, self,
		propertiesview_selectsection);
	propertiesview_translate(self);
	tabbar_init(&self->tabbar, &self->component);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_RIGHT);
	self->tabbar.tabalignment = psy_ui_ALIGN_RIGHT;
	psy_ui_margin_init_all(&tabmargin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(1),
		psy_ui_value_makeeh(0.5),
		psy_ui_value_makeew(2));
	tabbar_setdefaulttabmargin(&self->tabbar, &tabmargin);
	propertiesview_updatetabbarsections(self);
	psy_signal_connect(&self->renderer.signal_changed, self,
		propertiesview_onpropertiesrendererchanged);
	psy_signal_connect(&self->renderer.signal_selected, self,
		propertiesview_onpropertiesrendererselected);
	psy_signal_connect(&workspace->signal_languagechanged, self,
		propertiesview_onlanguagechanged);	
}

void propertiesview_ondestroy(PropertiesView* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_selected);
}

void propertiesview_selectsection(PropertiesView* self,
	psy_ui_Component* sender, uintptr_t section)
{
	tabbar_select(&self->tabbar, (int)section);
}

void propertiesview_updatetabbarsections(PropertiesView* self)
{	
	tabbar_clear(&self->tabbar);
	if (self->renderer.properties) {
		psy_List* p;

		for (p = psy_property_children(self->renderer.properties); p != NULL;
				psy_list_next(&p)) {
			psy_Property* property;

			property = (psy_Property*)p->entry;
			if (psy_property_type(property) == PSY_PROPERTY_TYPE_SECTION) {
				tabbar_append(&self->tabbar,
					psy_property_translation(property));
			}
		}
	}
	tabbar_select(&self->tabbar, 0);
	psy_signal_connect(&self->tabbar.signal_change, self,
			propertiesview_ontabbarchange);
}

void propertiesview_ontabbarchange(PropertiesView* self, psy_ui_Component* sender,
	int tabindex)
{	
	Tab* tab;

	self->renderer.search = 0;
	if (self->renderer.properties) {	
		psy_Property* property = NULL;
		
		tab = tabbar_tab(&self->tabbar, tabindex);
		if (tab) {
			psy_List* p;

			p = psy_property_children(self->renderer.properties);
			while (p) {
				property = (psy_Property*)p->entry;
				if (psy_property_type(property) == PSY_PROPERTY_TYPE_SECTION) {
					if (strcmp(psy_property_translation(property), tab->text) == 0) {						
						break;
					}
				}
				property = NULL;
				psy_list_next(&p);
			}			
		}
		self->renderer.search = property;
		if (self->renderer.search) {
			int scrollposition;
			int scrollmin;
			int scrollmax;

			propertiesrenderer_preparepropertiesenum(&self->renderer);
			psy_property_enumerate(self->renderer.properties,
				&self->renderer, (psy_PropertyCallback)
				propertiesrenderer_onenumpropertyposition);
			psy_ui_component_verticalscrollrange(&self->renderer.component,
				&scrollmin, &scrollmax);
			scrollposition = self->renderer.cpy / self->renderer.lineheight;
			if (scrollposition > scrollmax) {
				scrollposition = scrollmax;
			}
			psy_ui_component_setscrolltop(&self->renderer.component,
				scrollposition * self->renderer.lineheight);			
		}
		psy_ui_component_invalidate(&self->renderer.component);
	}
}

void propertiesview_onpropertiesrendererchanged(PropertiesView* self,
	PropertiesRenderer* sender, psy_Property* selected)
{		
	psy_signal_emit(&self->signal_changed, self, 1,
		selected);
	psy_ui_component_align(propertiesrenderer_base(&self->renderer));
	psy_ui_component_updateoverflow(propertiesrenderer_base(&self->renderer));
}

void propertiesview_onpropertiesrendererselected(PropertiesView* self,
	PropertiesRenderer* sender, psy_Property* selected)
{
	psy_signal_emit(&self->signal_selected, self, 1, selected);
}

void propertiesview_onlanguagechanged(PropertiesView* self, Translator* sender)
{
	propertiesview_translate(self);
	propertiesview_updatetabbarsections(self);
}

void propertiesview_translate(PropertiesView* self)
{
	psy_property_enumerate(self->renderer.properties, self,
		(psy_PropertyCallback)propertiesview_onchangelanguageenum);
}

int propertiesview_onchangelanguageenum(PropertiesView* self,
	psy_Property* property, int level)
{	
	if (!property->item.translate) {
		return 2;
	} else {
		psy_property_settranslation(property,
			translator_translate(&self->workspace->translator,
				psy_property_text(property)));
	}
	return TRUE;
}

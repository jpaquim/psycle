// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicomponent.h"
#include "uialigner.h"
#include "uiapp.h"
#include "uiimpfactory.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../detail/portable.h"

static void enableinput_internal(psy_ui_Component*, int enable, int recursive);
static void psy_ui_updatesyles(psy_ui_Component* main);
static void psy_ui_component_updatefont(psy_ui_Component*);
static void psy_ui_component_dispose_signals(psy_ui_Component*);

void psy_ui_updatesyles(psy_ui_Component* main)
{
	if (main) {
		psy_List* p;
		psy_List* q;		
		
		// merge
		psy_ui_component_updatefont(main);
		for (p = q = psy_ui_component_children(main, 1); p != NULL; p = p->next) {
			psy_ui_Component* child;			
			child = (psy_ui_Component*) p->entry;			
			psy_ui_component_updatefont(child);						
		}
		// align
		psy_ui_updatealign(main, q);
	}
}

void psy_ui_updatealign(psy_ui_Component* main, psy_List* children)
{
	if (main) {
		psy_List* p;
		psy_List* q;

		psy_ui_component_align(main);
		if (!children) {
			q = psy_ui_component_children(main, 1);
		} else {
			q = children;
		}
		for (p = q; p != NULL; p = p->next) {
			psy_ui_Component* child;

			child = (psy_ui_Component*)p->entry;
			if ((!psy_ui_component_parent(child) ||
				(psy_ui_component_parent(child) && !psy_ui_component_parent(child)->alignchildren))
				&& child->alignchildren) {
				psy_ui_component_align(child);
			}
		}
		psy_list_free(q);
	}
}

psy_ui_Font* psy_ui_component_font(psy_ui_Component* self)
{
	psy_ui_Font* rv;
	psy_ui_Style* common;	

	common = &app.defaults.style_common;
	if (self->style.use_font) {
		rv = &self->style.font;
	} else {
		rv = &app.defaults.style_common.font;
	}
	return rv;
}

void psy_ui_component_setbackgroundcolor(psy_ui_Component* self,
	psy_ui_Color color)
{		
	psy_ui_color_set(&self->style.backgroundcolor, color);
}

psy_ui_Color psy_ui_component_backgroundcolor(psy_ui_Component* self)
{
	psy_ui_Component* curr;

	curr = self;
	while (curr) {
		if (curr->style.backgroundcolor.mode.set) {
			break;
		}
		if (self->style.backgroundcolor.mode.inherited == FALSE) {
			curr = NULL;
			break;
		}
		curr = psy_ui_component_parent(curr);
	}
	if (curr) {
		return curr->style.backgroundcolor;
	}
	return app.defaults.style_common.backgroundcolor;
}

void psy_ui_component_setcolor(psy_ui_Component* self, psy_ui_Color color)
{
	psy_ui_color_set(&self->style.color, color);
}

psy_ui_Color psy_ui_component_color(psy_ui_Component* self)
{	
	psy_ui_Component* curr;

	curr = self;
	while (curr) {
		if (curr->style.color.mode.set) {
			break;
		}
		if (self->style.color.mode.inherited == FALSE) {
			curr = NULL;
			break;
		}
		curr = psy_ui_component_parent(curr);
	}
	if (curr) {
		return curr->style.color;
	}
	return app.defaults.style_common.color;
}

psy_ui_Border psy_ui_component_border(psy_ui_Component* self)
{
	if (self->style.border.mode.set) {
		return self->style.border;
	}
	return app.defaults.style_common.border;
}

void psy_ui_replacedefaultfont(psy_ui_Component* main, psy_ui_Font* font)
{		
	if (main) {
		psy_ui_Style* common;
		psy_ui_Font old_default_font;

		common = &app.defaults.style_common;
		old_default_font = common->font;
		psy_ui_font_init(&common->font, 0);
		psy_ui_font_copy(&common->font, font);
		psy_ui_updatesyles(main);
		psy_ui_font_dispose(&old_default_font);
	}
}

// vtable
static void dispose(psy_ui_Component*);
static void destroy(psy_ui_Component*);
static void show(psy_ui_Component*);
static void showstate(psy_ui_Component*, int state);
static void hide(psy_ui_Component*);
static int visible(psy_ui_Component*);
static void move(psy_ui_Component*, psy_ui_Point);
static void resize(psy_ui_Component*, psy_ui_Size);
static void clientresize(psy_ui_Component*, psy_ui_Size);
static void setposition(psy_ui_Component*, psy_ui_Point, psy_ui_Size);
static psy_ui_Size framesize(psy_ui_Component*);
static void scrollto(psy_ui_Component*, intptr_t dx, intptr_t dy);
static void setfont(psy_ui_Component*, psy_ui_Font*);
static psy_List* children(psy_ui_Component*, int recursive);
static void enableinput(psy_ui_Component*);
static void preventinput(psy_ui_Component*);
// events
static void onsize(psy_ui_Component* self, const psy_ui_Size* size) { }
static void onalign(psy_ui_Component* self) { }
static void onpreferredsize(psy_ui_Component*, const psy_ui_Size* limit, psy_ui_Size* rv);
static bool onclose(psy_ui_Component* self) { return TRUE; }
static void onmousedown(psy_ui_Component* self, psy_ui_MouseEvent* ev) { }
static void onmousemove(psy_ui_Component* self, psy_ui_MouseEvent* ev) { }
static void onmousewheel(psy_ui_Component* self, psy_ui_MouseEvent* ev) { }
static void onmouseup(psy_ui_Component* self, psy_ui_MouseEvent* ev) { }
static void onmousedoubleclick(psy_ui_Component* self, psy_ui_MouseEvent* ev) { }
static void onmouseenter(psy_ui_Component* self) { }
static void onmouseleave(psy_ui_Component* self) { }
static void onkeydown(psy_ui_Component* self, psy_ui_KeyEvent* ev) { }
static void onkeyup(psy_ui_Component* self, psy_ui_KeyEvent* ev) { }
static void ontimer(psy_ui_Component* self, uintptr_t timerid) { }

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.dispose = dispose;
		vtable.destroy = destroy;
		vtable.show = show;
		vtable.showstate = showstate;
		vtable.hide = hide;
		vtable.visible = visible;
		vtable.move = move;
		vtable.resize = resize;
		vtable.clientresize = clientresize;		
		vtable.setposition = setposition;		
		vtable.framesize = framesize;
		vtable.scrollto = scrollto;		
		vtable.setfont = setfont;		
		vtable.children = children;
		// events
		vtable.ondraw = 0;
		vtable.onalign = onalign;
		vtable.onpreferredsize = onpreferredsize;
		vtable.onsize = onsize;
		vtable.onclose = onclose;
		vtable.onmousedown = onmousedown;
		vtable.onmousemove = onmousemove;
		vtable.onmousewheel = onmousewheel;
		vtable.onmouseup = onmouseup;
		vtable.onmousedoubleclick = onmousedoubleclick;
		vtable.onmouseenter = onmouseenter;
		vtable.onmouseleave = onmouseleave;
		vtable.onkeyup = onkeyup;
		vtable.onkeydown = onkeydown;
		vtable.onkeydown = onkeyup;
		vtable.ontimer = ontimer;
		vtable.enableinput = enableinput;
		vtable.preventinput = preventinput;
		vtable_initialized = 1;
	}
}

void psy_ui_component_init_imp(psy_ui_Component* self, psy_ui_Component* parent,
	psy_ui_ComponentImp* imp)
{
	vtable_init();
	self->vtable = &vtable;
	if (!parent) {
		app.main = self;
	}
	self->imp = imp;
	psy_ui_component_init_base(self);
	psy_ui_component_init_signals(self);
}

void psy_ui_component_init(psy_ui_Component* self, psy_ui_Component* parent)
{	
	vtable_init();
	self->vtable = &vtable;
	if (!parent) {
		app.main = self;
	}
	self->imp = psy_ui_impfactory_allocinit_componentimp(psy_ui_app_impfactory(&app),
		self, parent);
	psy_ui_component_init_base(self);
	psy_ui_component_init_signals(self);	
}

void dispose(psy_ui_Component* self)
{	
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = 0;
	psy_ui_style_dispose(&self->style);
}

void destroy(psy_ui_Component* self)
{
	self->imp->vtable->dev_destroy(self->imp);
}

void psy_ui_component_updatefont(psy_ui_Component* self)
{
	// assert(self->imp);   
	if (self->imp) {
		self->imp->vtable->dev_setfont(self->imp,
			psy_ui_component_font(self));
		if (self->vtable->setfont != setfont) {
			self->vtable->setfont(self, NULL);
		}
	}
}

void show(psy_ui_Component* self)
{
	self->visible = 1;
	self->imp->vtable->dev_show(self->imp);	
}

void showstate(psy_ui_Component* self, int state)
{
	self->visible = 1;
	self->imp->vtable->dev_showstate(self->imp, state);
	psy_ui_component_align(self);
}

void hide(psy_ui_Component* self)
{
	self->visible = 0;
	self->imp->vtable->dev_hide(self->imp);	
}

int visible(psy_ui_Component* self)
{	
	return self->imp->vtable->dev_visible(self->imp);
}

void move(psy_ui_Component* self, psy_ui_Point topleft)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(self);
	self->imp->vtable->dev_move(self->imp, 
		psy_ui_value_px(&topleft.x, &tm),
		psy_ui_value_px(&topleft.y, &tm));
}

void resize(psy_ui_Component* self, psy_ui_Size size)
{			
	self->imp->vtable->dev_resize(self->imp, size);	
}

void clientresize(psy_ui_Component* self, psy_ui_Size size)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(self);
	self->imp->vtable->dev_clientresize(self->imp,
		psy_ui_value_px(&size.width, &tm),
		psy_ui_value_px(&size.height, &tm));
}

void setposition(psy_ui_Component* self, psy_ui_Point topleft,
	psy_ui_Size size)
{	
	self->imp->vtable->dev_setposition(self->imp, topleft, size);
}

psy_ui_Size framesize(psy_ui_Component* self)
{
	return self->imp->vtable->dev_framesize(self->imp);
}

void scrollto(psy_ui_Component* self, intptr_t dx, intptr_t dy)
{
	self->imp->vtable->dev_scrollto(self->imp, dx, dy);
}

void setfont(psy_ui_Component* self, psy_ui_Font* font)
{
	if (font) {
		int dispose;

		dispose = self->style.use_font;
		self->imp->vtable->dev_setfont(self->imp, font);
		if (dispose) {
			psy_ui_font_dispose(&self->style.font);
		}
		psy_ui_font_init(&self->style.font, 0);
		psy_ui_font_copy(&self->style.font, font);
		self->style.use_font = 1;
	} else {
		int dispose;

		dispose = self->style.use_font;
		self->style.use_font = 0;
		psy_ui_component_updatefont(self);
		self->imp->vtable->dev_setfont(self->imp, psy_ui_component_font(self));		
		if (dispose) {
			psy_ui_font_dispose(&self->style.font);
		}
	}
}

psy_List* children(psy_ui_Component* self, int recursive)
{
	return self->imp->vtable->dev_children(self->imp, recursive);
}

void psy_ui_component_init_signals(psy_ui_Component* self)
{
	psy_signal_init(&self->signal_size);
	psy_signal_init(&self->signal_draw);
	psy_signal_init(&self->signal_timer);
	psy_signal_init(&self->signal_keydown);
	psy_signal_init(&self->signal_keyup);
	psy_signal_init(&self->signal_mousedown);
	psy_signal_init(&self->signal_mouseup);
	psy_signal_init(&self->signal_mousemove);
	psy_signal_init(&self->signal_mousewheel);
	psy_signal_init(&self->signal_mousedoubleclick);
	psy_signal_init(&self->signal_mouseenter);
	psy_signal_init(&self->signal_mousehover);
	psy_signal_init(&self->signal_mouseleave);
	psy_signal_init(&self->signal_scroll);
	psy_signal_init(&self->signal_create);
	psy_signal_init(&self->signal_close);
	psy_signal_init(&self->signal_destroy);
	psy_signal_init(&self->signal_destroyed);
	psy_signal_init(&self->signal_show);
	psy_signal_init(&self->signal_hide);
	psy_signal_init(&self->signal_focus);
	psy_signal_init(&self->signal_focuslost);
	psy_signal_init(&self->signal_align);
//	psy_signal_init(&self->signal_preferredsize);	
	psy_signal_init(&self->signal_preferredsizechanged);
	psy_signal_init(&self->signal_command);
	psy_signal_init(&self->signal_selectsection);
	psy_signal_init(&self->signal_scrollrangechanged);
}

void psy_ui_component_init_base(psy_ui_Component* self) {	
	self->scrollstepx = 100;
	self->scrollstepy = 12;
	self->overflow = psy_ui_OVERFLOW_HIDDEN;	
	self->vscrollrange.x = 0;
	self->vscrollrange.y = 0;
	self->preventdefault = 0;
	self->preventpreferredsize = 0;
	self->preventpreferredsizeatalign = FALSE;
	self->align = psy_ui_ALIGN_NONE;
	self->justify = psy_ui_JUSTIFY_EXPAND;
	self->alignchildren = 1;
	self->alignexpandmode = psy_ui_NOEXPAND;	
	self->preferredsize = psy_ui_component_size(self);
	self->maxsize = psy_ui_size_zero();
	self->minsize = psy_ui_size_zero();
	psy_ui_style_init(&self->style);
	psy_ui_margin_init(&self->margin);
	psy_ui_margin_init(&self->spacing);	
	self->debugflag = 0;	
	self->visible = 1;
	self->doublebuffered = FALSE;
	self->wheelscroll = 0;
	self->accumwheeldelta = 0;
	self->handlevscroll = TRUE;
	self->handlehscroll = TRUE;
	self->backgroundmode = psy_ui_BACKGROUND_SET;
	self->mousetracking = 0;
	self->cursor = psy_ui_CURSOR_DEFAULT;
	psy_ui_intpoint_init(&self->scroll);	
	psy_ui_component_updatefont(self);
	if (self->imp) {
		self->imp->vtable->dev_setbackgroundcolor(self->imp,
			psy_ui_component_backgroundcolor(self));
	}
}

void psy_ui_component_dispose(psy_ui_Component* self)
{
	self->vtable->dispose(self);
	psy_ui_component_dispose_signals(self);
}

void psy_ui_component_dispose_signals(psy_ui_Component* self)
{	
	psy_signal_dispose(&self->signal_size);
	psy_signal_dispose(&self->signal_draw);
	psy_signal_dispose(&self->signal_timer);
	psy_signal_dispose(&self->signal_keydown);
	psy_signal_dispose(&self->signal_keyup);
	psy_signal_dispose(&self->signal_mousedown);
	psy_signal_dispose(&self->signal_mouseup);
	psy_signal_dispose(&self->signal_mousemove);
	psy_signal_dispose(&self->signal_mousewheel);
	psy_signal_dispose(&self->signal_mousedoubleclick);
	psy_signal_dispose(&self->signal_mouseenter);
	psy_signal_dispose(&self->signal_mousehover);
	psy_signal_dispose(&self->signal_mouseleave);
	psy_signal_dispose(&self->signal_scroll);
	psy_signal_dispose(&self->signal_create);
	psy_signal_dispose(&self->signal_close);
	psy_signal_dispose(&self->signal_destroy);
	psy_signal_dispose(&self->signal_destroyed);
	psy_signal_dispose(&self->signal_show);
	psy_signal_dispose(&self->signal_hide);
	psy_signal_dispose(&self->signal_focus);
	psy_signal_dispose(&self->signal_focuslost);
	psy_signal_dispose(&self->signal_align);
	// psy_signal_dispose(self->signal_preferredsize);	
	psy_signal_dispose(&self->signal_command);
	psy_signal_dispose(&self->signal_preferredsizechanged);
	psy_signal_dispose(&self->signal_selectsection);
	psy_signal_dispose(&self->signal_scrollrangechanged);
}

void psy_ui_component_destroy(psy_ui_Component* self)
{
	self->vtable->destroy(self);
}

void psy_ui_component_scrollstep(psy_ui_Component* self, intptr_t stepx,
	intptr_t stepy)
{
	if (stepy != 0 || stepx != 0) {
		self->vtable->scrollto(self, self->scrollstepx * stepx,
			self->scrollstepy * stepy);
	}
}

void psy_ui_component_showstate(psy_ui_Component* self, int state)
{
	self->vtable->showstate(self, state);	
}

void psy_ui_component_move(psy_ui_Component* self, psy_ui_Point topleft)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(self);
	self->vtable->move(self, topleft);
}

void psy_ui_component_resize(psy_ui_Component* self, psy_ui_Size size)
{		
	self->vtable->resize(self, size);	
}

void psy_ui_component_setposition(psy_ui_Component* self, psy_ui_Point topleft,
	psy_ui_Size size)
{		
	self->vtable->setposition(self, topleft, size);	
}

psy_List* psy_ui_component_children(psy_ui_Component* self, int recursive)
{	
	return self->vtable->children(self, recursive);	
}

void psy_ui_component_setfont(psy_ui_Component* self, psy_ui_Font* source)
{	
	self->vtable->setfont(self, source);
}

void psy_ui_component_preventdefault(psy_ui_Component* self)
{
	self->preventdefault = 1;
}

int psy_ui_component_visible(psy_ui_Component* self)
{
	return self->vtable->visible(self);	
}

void psy_ui_component_align(psy_ui_Component* self)
{		
	psy_ui_Aligner aligner;

	psy_ui_aligner_init(&aligner, self);
	psy_ui_aligner_align(&aligner);
	psy_signal_emit(&self->signal_align, self, 0);
	self->vtable->onalign(self);
}

void psy_ui_component_alignall(psy_ui_Component* self)
{
	psy_List* p;
	psy_List* q;

	// align
	psy_ui_Aligner aligner;

	psy_ui_aligner_init(&aligner, self);
	psy_ui_aligner_align(&aligner);
	psy_signal_emit(&self->signal_align, self, 0);
	self->vtable->onalign(self);
	for (p = q = psy_ui_component_children(self, 1); p != NULL; psy_list_next(&p)) {
		psy_ui_Component* child;

		child = (psy_ui_Component*)psy_list_entry(p);
		psy_ui_component_align(child);
	}	
	psy_list_free(q);
}

void onpreferredsize(psy_ui_Component* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	if (!self->preventpreferredsize) {
		psy_ui_Aligner aligner;

		psy_ui_aligner_init(&aligner, self);
		psy_ui_aligner_preferredsize(&aligner, limit, rv);
	} else {
		psy_ui_Size size;
		
		size = psy_ui_component_size(self);
		*rv = size;
	}
}

void psy_ui_component_doublebuffer(psy_ui_Component* self)
{
	self->doublebuffered = 1;
}

void psy_ui_component_setmargin(psy_ui_Component* self,
	const psy_ui_Margin* margin)
{	
	if (margin) {
		self->margin = *margin;		
	} else {
		memset(&self->margin, 0, sizeof(psy_ui_Margin));
	}
}

void psy_ui_component_setspacing(psy_ui_Component* self,
	const psy_ui_Margin* spacing)
{	
	if (spacing) {
		self->spacing = *spacing;
	} else {
		memset(&self->spacing, 0, sizeof(psy_ui_Margin));
	}
}

void psy_ui_component_setalign(psy_ui_Component* self, psy_ui_AlignType align)
{
	self->align = align;
}

void psy_ui_component_enablealign(psy_ui_Component* self)
{
	self->alignchildren = 1;	
}

void psy_ui_component_preventalign(psy_ui_Component* self)
{
	self->alignchildren = 0;
}

void psy_ui_component_setalignexpand(psy_ui_Component* self, psy_ui_ExpandMode mode)
{
	self->alignexpandmode = mode;
}

void psy_ui_component_enableinput(psy_ui_Component* self, int recursive)
{
	enableinput_internal(self, TRUE, recursive);
}

void psy_ui_component_preventinput(psy_ui_Component* self, int recursive)
{
	enableinput_internal(self, FALSE, recursive);
}

void enableinput_internal(psy_ui_Component* self, int enable, int recursive)
{	
	if (enable) {
		self->vtable->enableinput(self);
	} else {
		self->vtable->preventinput(self);
	}	
	if (recursive) {
		psy_List* p;
		psy_List* q;
		
		for (p = q = psy_ui_component_children(self, recursive); p != NULL;
				p = p->next) {
			psy_ui_Component* child;

			child = (psy_ui_Component*) p->entry;
			if (enable) {
				child->vtable->enableinput(child);
			} else {
				child->vtable->preventinput(child);
			}			
		}
		psy_list_free(q);
	}
}

static void enableinput(psy_ui_Component* self)
{
	self->imp->vtable->dev_enableinput(self->imp);
}

static void preventinput(psy_ui_Component* self)
{
	self->imp->vtable->dev_preventinput(self->imp);
}

void psy_ui_component_setbackgroundmode(psy_ui_Component* self,
	psy_ui_BackgroundMode mode)
{
	self->backgroundmode = mode;	
}

psy_List* psy_ui_components_setalign(psy_List* list, psy_ui_AlignType align,
	const psy_ui_Margin* margin)
{
	psy_List* p;

	for (p = list; p != NULL; p = p->next) {
		psy_ui_component_setalign((psy_ui_Component*) p->entry, align);
		if (margin) {
			psy_ui_component_setmargin((psy_ui_Component*) p->entry, margin);
		}
	}
	return list;
}

psy_List* psy_ui_components_setmargin(psy_List* list, const psy_ui_Margin* margin)
{
	psy_List* p;

	for (p = list; p != NULL; p = p->next) {
		psy_ui_component_setmargin((psy_ui_Component*) p->entry, margin);
	}
	return list;
}

void psy_ui_component_setpreferredsize(psy_ui_Component* self, psy_ui_Size size)
{
	self->preferredsize = size;
}

psy_ui_Size psy_ui_component_preferredsize(psy_ui_Component* self,
	const psy_ui_Size* limit)
{
	if (self->preventpreferredsize) {
		return psy_ui_component_size(self);
	} else {
		psy_ui_Size rv;

		rv = self->preferredsize;
		self->vtable->onpreferredsize(self, limit, &rv);
			// psy_signal_emit(&self->signal_preferredsize, self, 2, limit, &rv);		
		return rv;
	}
}

void psy_ui_component_preventpreferredsize(psy_ui_Component* self)
{
	self->preventpreferredsize = TRUE;
}

void psy_ui_component_enablepreferredsize(psy_ui_Component* self)
{
	self->preventpreferredsize = FALSE;
}

void psy_ui_component_setmaximumsize(psy_ui_Component* self, psy_ui_Size size)
{
	self->maxsize = size;
}

psy_ui_Size psy_ui_component_maximumsize(psy_ui_Component* self)
{
	return self->maxsize;
}

void psy_ui_component_setminimumsize(psy_ui_Component* self, psy_ui_Size size)
{
	self->minsize = size;
}

psy_ui_Size psy_ui_component_minimumsize(psy_ui_Component* self)
{
	return self->minsize;
}

void psy_ui_component_seticonressource(psy_ui_Component* self, int ressourceid)
{
	self->imp->vtable->dev_seticonressource(self->imp, ressourceid);
}

void psy_ui_component_clientresize(psy_ui_Component* self, psy_ui_Size size)
{
	self->vtable->clientresize(self, size);
}

void psy_ui_component_setcursor(psy_ui_Component* self, psy_ui_CursorStyle style)
{
	self->imp->vtable->dev_setcursor(self->imp, style);
}

// psy_ui_ComponentImp vtable
static void dev_dispose(psy_ui_ComponentImp* self) { }
static void dev_destroy(psy_ui_ComponentImp* self) { }
static void dev_show(psy_ui_ComponentImp* self) { }
static void dev_showstate(psy_ui_ComponentImp* self, int state) { }
static void dev_hide(psy_ui_ComponentImp* self) { }
static int dev_visible(psy_ui_ComponentImp* self) { return 0; }
static psy_ui_Rectangle dev_position(psy_ui_ComponentImp* self) { psy_ui_Rectangle rv = { 0, 0, 0, 0 }; return rv; }
static void dev_move(psy_ui_ComponentImp* self, int x, int y) { }
static void dev_resize(psy_ui_ComponentImp* self, psy_ui_Size size) { }
static void dev_clientresize(psy_ui_ComponentImp* self, int width, int height) { }
static void dev_setposition(psy_ui_ComponentImp* self, psy_ui_Point topleft, psy_ui_Size size) { }
static psy_ui_Size dev_size(psy_ui_ComponentImp* self) { return psy_ui_size_zero(); }
static void dev_updatesize(psy_ui_ComponentImp* self) { }
static psy_ui_Size dev_framesize(psy_ui_ComponentImp* self) { return psy_ui_size_zero(); }
static void dev_scrollto(psy_ui_ComponentImp* self, intptr_t dx, intptr_t dy) { }
static psy_ui_Component* dev_parent(psy_ui_ComponentImp* self) { return 0;  }
static void dev_setparent(psy_ui_ComponentImp* self, psy_ui_Component* parent) { }
static void dev_insert(psy_ui_ComponentImp* self, psy_ui_ComponentImp* child, psy_ui_ComponentImp* insertafter) { }
static void dev_setorder(psy_ui_ComponentImp* self, psy_ui_ComponentImp* insertafter) { }
static void dev_capture(psy_ui_ComponentImp* self) { }
static void dev_releasecapture(psy_ui_ComponentImp* self) { }
static void dev_invalidate(psy_ui_ComponentImp* self) { }
static void dev_invalidaterect(psy_ui_ComponentImp* self, const psy_ui_Rectangle* r) { }
static void dev_update(psy_ui_ComponentImp* self) { }
static void dev_setfont(psy_ui_ComponentImp* self, psy_ui_Font* font) { }
static void dev_showhorizontalscrollbar(psy_ui_ComponentImp* self) { }
static void dev_hidehorizontalscrollbar(psy_ui_ComponentImp* self) { }
static void dev_sethorizontalscrollrange(psy_ui_ComponentImp* self, int min, int max) { }
static void dev_horizontalscrollrange(psy_ui_ComponentImp* self, int* scrollmin, int* scrollmax) { *scrollmin = 0; *scrollmax = 0; }
static int dev_horizontalscrollposition(psy_ui_ComponentImp* self) { return 0;  }
static void dev_sethorizontalscrollposition(psy_ui_ComponentImp* self, int position) { }
static void dev_showverticalscrollbar(psy_ui_ComponentImp* self) { }
static void dev_hideverticalscrollbar(psy_ui_ComponentImp* self) { }
static void dev_setverticalscrollrange(psy_ui_ComponentImp* self, int min, int max) { }
static void dev_verticalscrollrange(psy_ui_ComponentImp* self, int* scrollmin, int* scrollmax) { *scrollmin = 0; *scrollmax = 0; }
static int dev_verticalscrollposition(psy_ui_ComponentImp* self) { return 0; }
static void dev_setverticalscrollposition(psy_ui_ComponentImp* self, int position) { }
static psy_List* dev_children(psy_ui_ComponentImp* self, int recursive) { return 0; }
static void dev_enableinput(psy_ui_ComponentImp* self) { }
static void dev_preventinput(psy_ui_ComponentImp* self) { }
static void dev_setcursor(psy_ui_ComponentImp* self, psy_ui_CursorStyle cursorstyle) { }
static void dev_starttimer(psy_ui_ComponentImp* self, uintptr_t id, uintptr_t interval) { }
static void dev_stoptimer(psy_ui_ComponentImp* self, uintptr_t id) { }
static void dev_seticonressource(psy_ui_ComponentImp* self, int ressourceid) { }
static psy_ui_TextMetric dev_textmetric(psy_ui_ComponentImp* self)
{
	psy_ui_TextMetric tm;
	memset(&tm, 0, sizeof(tm));
	return tm;
}
static psy_ui_Size dev_textsize(psy_ui_ComponentImp* self, const char* text, psy_ui_Font* font) { psy_ui_Size rv = { 0, 0 }; return rv; }
static void dev_setbackgroundcolor(psy_ui_ComponentImp* self, psy_ui_Color color) { }
static void dev_settitle(psy_ui_ComponentImp* self, const char* title) { }
static void dev_setfocus(psy_ui_ComponentImp* self) { }
static int dev_hasfocus(psy_ui_ComponentImp* self) { return 0;  }
static void* dev_platform(psy_ui_ComponentImp* self) { return (void*) self; }

static psy_ui_ComponentImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = dev_dispose;
		imp_vtable.dev_destroy = dev_destroy;
		imp_vtable.dev_show = dev_show;
		imp_vtable.dev_showstate = dev_showstate;
		imp_vtable.dev_hide = dev_hide;
		imp_vtable.dev_visible = dev_visible;
		imp_vtable.dev_move = dev_move;
		imp_vtable.dev_resize = dev_resize;
		imp_vtable.dev_clientresize = dev_clientresize;
		imp_vtable.dev_position = dev_position;
		imp_vtable.dev_setposition = dev_setposition;
		imp_vtable.dev_size = dev_size;
		imp_vtable.dev_updatesize = dev_updatesize;
		imp_vtable.dev_framesize = dev_framesize;
		imp_vtable.dev_scrollto = dev_scrollto;
		imp_vtable.dev_parent = dev_parent;
		imp_vtable.dev_setparent = dev_setparent;
		imp_vtable.dev_insert = dev_insert;
		imp_vtable.dev_setorder = dev_setorder;
		imp_vtable.dev_capture = dev_capture;
		imp_vtable.dev_releasecapture = dev_releasecapture;
		imp_vtable.dev_invalidate = dev_invalidate;
		imp_vtable.dev_invalidaterect = dev_invalidaterect;
		imp_vtable.dev_update = dev_update;
		imp_vtable.dev_setfont = dev_setfont;		
		imp_vtable.dev_children = dev_children;
		imp_vtable.dev_enableinput = dev_enableinput;
		imp_vtable.dev_preventinput = dev_preventinput;
		imp_vtable.dev_setcursor = dev_setcursor;
		imp_vtable.dev_starttimer = dev_starttimer;
		imp_vtable.dev_stoptimer = dev_stoptimer;
		imp_vtable.dev_seticonressource = dev_seticonressource;
		imp_vtable.dev_textmetric = dev_textmetric;
		imp_vtable.dev_textsize = dev_textsize;
		imp_vtable.dev_setbackgroundcolor = dev_setbackgroundcolor;
		imp_vtable.dev_settitle = dev_settitle;
		imp_vtable.dev_setfocus = dev_setfocus;
		imp_vtable.dev_hasfocus = dev_hasfocus;
		imp_vtable.dev_platform = dev_platform;
	}
}

void psy_ui_componentimp_init(psy_ui_ComponentImp* self)
{
	imp_vtable_init();
	self->vtable = &imp_vtable;
	psy_signal_init(&self->signal_command);
}

void psy_ui_componentimp_dispose(psy_ui_ComponentImp* self)
{
	psy_signal_dispose(&self->signal_command);
}

psy_ui_Component* psy_ui_component_at(psy_ui_Component* self, uintptr_t index)
{
	psy_ui_Component* rv = 0;
	psy_List* p;
	psy_List* q;
	uintptr_t c = 0;

	p = q = psy_ui_component_children(self, 0);
	while (p) {
		if (c == index) {
			rv = (psy_ui_Component*)p->entry;
			break;
		}
		p = p->next;
		++c;
	}
	psy_list_free(q);
	return rv;
}

void psy_ui_component_setscroll(psy_ui_Component* self,
	psy_ui_IntPoint position)
{
	psy_ui_component_setscrollleft(self, position.x);
	psy_ui_component_setscrolltop(self, position.y);
}

void psy_ui_component_setscrollleft(psy_ui_Component* self, int left)
{	
	int oldscrollx;

	oldscrollx = self->scroll.x;
	self->scroll.x = left;
	if (self->scroll.x != oldscrollx) {		
		if (self->signal_scroll.slots) {
			psy_signal_emit(&self->signal_scroll, self, 0);
		}
		psy_ui_component_scrollstep(self, (oldscrollx - self->scroll.x) /
			self->scrollstepx, 0);
	}
}

void psy_ui_component_setscrolltop(psy_ui_Component* self, int top)
{
	int oldscrolly;
	
	oldscrolly = self->scroll.y;
	self->scroll.y = top;
	if (self->scroll.y != oldscrolly) {		
		if (self->signal_scroll.slots) {			
			psy_signal_emit(&self->signal_scroll, self, 0);
		}
		psy_ui_component_scrollstep(self, 0, (oldscrolly - self->scroll.y) /
			self->scrollstepy);
	}
}

void psy_ui_component_updateoverflow(psy_ui_Component* self)
{		
	if ((self->overflow & psy_ui_OVERFLOW_VSCROLL) == psy_ui_OVERFLOW_VSCROLL) {
		psy_ui_Size preferredsize;
		psy_ui_TextMetric tm;
		int maxlines;
		int visilines;
		int currline;
		psy_ui_Size size;

		tm = psy_ui_component_textmetric(self);
		size = psy_ui_component_size(self);
		preferredsize = psy_ui_component_preferredsize(self, &size);
		maxlines = (int)(psy_ui_value_px(&preferredsize.height, &tm) / (double)self->scrollstepy);
		visilines = (psy_ui_value_px(&size.height, &tm) / self->scrollstepy);
		currline = psy_ui_component_scrolltop(self) / self->scrollstepy;
		if ((self->overflow & psy_ui_OVERFLOW_VSCROLLCENTER) ==
				psy_ui_OVERFLOW_VSCROLLCENTER) {
			psy_ui_component_setverticalscrollrange(self, -visilines / 2,
				maxlines - visilines / 2 - 1);
			if (currline > maxlines - visilines / 2) {
				currline = max(-visilines / 2, maxlines -visilines / 2);
				psy_ui_component_setscrolltop(self, currline * self->scrollstepy);
			}
		} else {
			psy_ui_component_setverticalscrollrange(self, 0, maxlines - visilines);
			if (currline > maxlines - visilines) {
				currline = max(0, maxlines - visilines);
				psy_ui_component_setscrolltop(self, currline * self->scrollstepy);
			}
		}		
	}
	if ((self->overflow & psy_ui_OVERFLOW_HSCROLL) == psy_ui_OVERFLOW_HSCROLL) {
		psy_ui_Size preferredsize;
		psy_ui_TextMetric tm;
		int maxrows;
		int visirows;
		int currrow;
		psy_ui_Size size;

		tm = psy_ui_component_textmetric(self);
		size = psy_ui_component_size(self);
		preferredsize = psy_ui_component_preferredsize(self, &size);
		maxrows = (int)(psy_ui_value_px(&preferredsize.width, &tm) /
			(double)self->scrollstepx + 0.5);
		visirows = psy_ui_value_px(&size.width, &tm) / self->scrollstepx;
		currrow = psy_ui_component_scrollleft(self) / self->scrollstepx;		
		psy_ui_component_sethorizontalscrollrange(self, 0, maxrows - visirows);			
		if (currrow > maxrows - visirows) {
			currrow = max(0, maxrows - visirows);
			psy_ui_component_setscrollleft(self, currrow * self->scrollstepx);
		}		
	}
}

void psy_ui_component_drawborder(psy_ui_Component* self, psy_ui_Graphics* g)
{
	psy_ui_Border border;

	border = psy_ui_component_border(self);
	if (border.mode.set) {
		psy_ui_TextMetric tm;
		psy_ui_IntSize size;

		tm = psy_ui_component_textmetric(self);
		size = psy_ui_intsize_init_size(
			psy_ui_component_size(self), &tm);
		if (border.top != psy_ui_BORDER_NONE) {
			if (border.color_top.mode.set) {
				psy_ui_setcolor(g, border.color_top);
			} else {
				psy_ui_setcolor(g,
					app.defaults.style_common.border.color_top);
			}
			psy_ui_drawline(g, 0, 0, size.width, 0);
		}
		if (border.bottom != psy_ui_BORDER_NONE) {
			if (border.color_top.mode.set) {
				psy_ui_setcolor(g, border.color_top);
			} else {
				psy_ui_setcolor(g,
					app.defaults.style_common.border.color_top);
			}
			psy_ui_drawline(g, 0, size.height - 1, size.width,
				size.height - 1);
		}
	}
}

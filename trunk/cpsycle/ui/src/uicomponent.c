// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicomponent.h"
// local
#include "uiapp.h"
#include "uigridaligner.h"
#include "uiimpfactory.h"
#include "uilclaligner.h"
#include "uiviewcomponentimp.h"
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

typedef void (*psy_fp_int)(psy_ui_Component*, uintptr_t);
typedef void (*psy_fp_margin)(psy_ui_Component*, psy_ui_Margin);

static void psy_ui_component_traverse_int(psy_ui_Component*,
	psy_fp_int, uintptr_t value);
static void psy_ui_component_traverse_margin(psy_ui_Component*,
	psy_fp_margin fp, psy_ui_Margin value);
static double lines(double pos, double size, double scrollsize, double step,
	bool round, double* rv_max, double* rv_visi);

void psy_ui_componentcontaineralign_init(psy_ui_ComponentContainerAlign* self)
{
	self->alignexpandmode = psy_ui_NOEXPAND;
	self->containeralign = psy_ui_CONTAINER_ALIGN_LCL;
	self->insertaligntype = psy_ui_ALIGN_NONE;	
	psy_ui_margin_init(&self->insertmargin);
}

psy_ui_ComponentContainerAlign* psy_ui_componentcontaineralign_alloc(void)
{
	return (psy_ui_ComponentContainerAlign*)malloc(sizeof(
		psy_ui_ComponentContainerAlign));
}

psy_ui_ComponentContainerAlign* psy_ui_componentcontaineralign_allocinit(void)
{
	psy_ui_ComponentContainerAlign* rv;

	rv = psy_ui_componentcontaineralign_alloc();
	if (rv) {
		psy_ui_componentcontaineralign_init(rv);
	}
	return rv;
}

void psy_ui_componentcontaineralign_deallocate(psy_ui_ComponentContainerAlign* self)
{
	free(self);
}

static bool componentscroll_initialized = FALSE;
static psy_ui_ComponentScroll componentscroll;

static bool sizehints_initialized = FALSE;
static psy_ui_ComponentSizeHints sizehints;

static bool containeralign_initialized = FALSE;
static psy_ui_ComponentContainerAlign containeralign;

static void enableinput_internal(psy_ui_Component*, int enable, int recursive);
static void psy_ui_component_dispose_signals(psy_ui_Component*);

const psy_ui_Font* psy_ui_component_font(const psy_ui_Component* self)
{
	const psy_ui_Font* rv;
	const psy_ui_Style* common;	

	common = psy_ui_style(psy_ui_STYLE_ROOT);
	if (self->style.currstyle->use_font) {
		rv = &self->style.currstyle->font;
	} else {
		rv = &psy_ui_style(psy_ui_STYLE_ROOT)->font;
	}
	return rv;
}

void psy_ui_component_setbackgroundcolour(psy_ui_Component* self,
	psy_ui_Colour colour)
{		
	self->style.currstyle->backgroundcolour = colour;	
}

psy_ui_Colour psy_ui_component_backgroundcolour(psy_ui_Component* self)
{
	psy_ui_Component* curr;
	psy_ui_Colour base;

	assert(self);

	curr = self;	
	while (curr) {
		if (curr->style.currstyle->backgroundcolour.mode.set) {			
			base = curr->style.currstyle->backgroundcolour;
			if (base.overlay != 0) {
				uint8_t overlay;
				psy_ui_Colour overlaycolour;

				if (psy_ui_app_hasdarktheme(psy_ui_app())) {
					overlaycolour = psy_ui_colour_make(0xFFFFFF);
				} else {
					overlaycolour = psy_ui_colour_make(0x0000000);
				}
				overlay = base.overlay;
				base = psy_ui_component_backgroundcolour(
					psy_ui_component_parent(curr));
				base = psy_ui_colour_overlayed(&base, &overlaycolour, overlay / 100.0);
			}
			break;
		}
		curr = psy_ui_component_parent(curr);
	}
	if (!curr) {		
		base = psy_ui_style(psy_ui_STYLE_ROOT)->backgroundcolour;
	}	
	return base;
}

void psy_ui_component_setcolour(psy_ui_Component* self, psy_ui_Colour colour)
{
	psy_ui_colour_set(&self->style.currstyle->colour, colour);
}

psy_ui_Colour psy_ui_component_colour(psy_ui_Component* self)
{	
	psy_ui_Component* curr;
	psy_ui_Colour base;

	assert(self);

	curr = self;
	while (curr) {
		if (curr->style.currstyle->colour.mode.set) {
			base = curr->style.currstyle->colour;
			if (base.overlay != 0) {
				uint8_t overlay;
				psy_ui_Colour overlaycolour;

				if (psy_ui_app_hasdarktheme(psy_ui_app())) {
					overlaycolour = psy_ui_colour_make(0xFFFFFF);
				} else {
					overlaycolour = psy_ui_colour_make(0x0000000);
				}
				overlay = base.overlay;
				base = psy_ui_component_colour(
					psy_ui_component_parent(curr));
				base = psy_ui_colour_overlayed(&base, &overlaycolour, overlay / 100.0);
			}
			break;
		}
		curr = psy_ui_component_parent(curr);
	}
	if (!curr) {
		base = psy_ui_style(psy_ui_STYLE_ROOT)->colour;
	}
	return base;
}

void psy_ui_component_setborder(psy_ui_Component* self,
	const psy_ui_Border* border)
{
	if (border) {
		self->style.currstyle->border = *border;
	} else {
		self->style.currstyle->border = self->style.style.border;
	}
}

const psy_ui_Border* psy_ui_component_border(const psy_ui_Component* self)
{
	if (self->style.currstyle->border.mode.set) {
		return &self->style.currstyle->border;
	}
	return &psy_ui_style(psy_ui_STYLE_ROOT)->border;
}

uintptr_t psy_ui_component_backgroundimageid(const psy_ui_Component* self)
{
	return self->style.currstyle->backgroundid;		
}

void psy_ui_replacedefaultfont(psy_ui_Component* main, psy_ui_Font* font)
{		
	if (main) {
		psy_ui_Style* common;		

		common = (psy_ui_Style*)psy_ui_style(psy_ui_STYLE_ROOT);		
psy_ui_font_dispose(&common->font);
psy_ui_font_init(&common->font, NULL);
psy_ui_font_copy(&common->font, font);
psy_ui_app_updatesyles(psy_ui_app());
	}
}

void psy_ui_component_capture(psy_ui_Component* self)
{
	self->imp->vtable->dev_capture(self->imp);
}

void psy_ui_component_releasecapture(psy_ui_Component* self)
{
	psy_ui_app()->capture = NULL;
	self->imp->vtable->dev_releasecapture(self->imp);
}

static void psy_ui_component_checkbackgroundanimation(psy_ui_Component*);
static psy_ui_Size psy_ui_component_sub_border(const psy_ui_Component*,
	psy_ui_Size);

// vtable
static void dispose(psy_ui_Component*);
static void destroy(psy_ui_Component*);
static void show(psy_ui_Component*);
static void showstate(psy_ui_Component*, int state);
static void hide(psy_ui_Component*);
static int visible(psy_ui_Component*);
static int drawvisible(psy_ui_Component*);
static void move(psy_ui_Component*, psy_ui_Point);
static void resize(psy_ui_Component*, psy_ui_Size);
static void clientresize(psy_ui_Component*, psy_ui_Size);
static void setposition(psy_ui_Component*, psy_ui_Point, psy_ui_Size);
static psy_ui_Size framesize(psy_ui_Component*);
static void scrollto(psy_ui_Component*, intptr_t dx, intptr_t dy);
static void setfont(psy_ui_Component*, const psy_ui_Font*);
static psy_List* children(psy_ui_Component*, int recursive);
static void enableinput(psy_ui_Component*);
static void preventinput(psy_ui_Component*);
static void invalidate(psy_ui_Component*);
static uintptr_t section(const psy_ui_Component* self) { return 0; }
static void setalign(psy_ui_Component* self, psy_ui_AlignType align) {  }

// events
static void ondestroy(psy_ui_Component* self) {	}
static void ondestroyed(psy_ui_Component* self) { }
static void onsize(psy_ui_Component* self, const psy_ui_Size* size) { }
static void onalign(psy_ui_Component* self) { }
static void onpreferredsize(psy_ui_Component*, const psy_ui_Size* limit, psy_ui_Size* rv);
static void onpreferredscrollsize(psy_ui_Component*, const psy_ui_Size* limit, psy_ui_Size* rv);
static bool onclose(psy_ui_Component* self) { return TRUE; }

static void onmousedown(psy_ui_Component* self, psy_ui_MouseEvent* ev)
{
	assert(ev);

	psy_ui_component_addstylestate(self, psy_ui_STYLESTATE_ACTIVE);
	if (self->draggable) {
		psy_ui_app()->dragevent.target = self;
		psy_ui_app()->dragevent.mouse = *ev;
		psy_ui_app_startdrag(psy_ui_app());
		self->vtable->ondragstart(self, &psy_ui_app()->dragevent);
	}
}

static void onmousemove(psy_ui_Component* self, psy_ui_MouseEvent* ev)
{
	assert(ev);

	if (psy_ui_app()->dragevent.active) {
		psy_ui_app()->dragevent.preventdefault = FALSE;
		psy_ui_app()->dragevent.mouse = *ev;
		self->vtable->ondragover(self, &psy_ui_app()->dragevent);
		if (psy_ui_app()->dragevent.preventdefault) {
			psy_ui_component_setcursor(self, psy_ui_CURSORSTYLE_GRAB);
			psy_ui_mouseevent_stoppropagation(ev);
		}
	}
}

static void onmousewheel(psy_ui_Component* self, psy_ui_MouseEvent* ev) { }

static void onmouseup(psy_ui_Component* self, psy_ui_MouseEvent* ev)
{
	assert(ev);

	psy_ui_component_removestylestate(self, psy_ui_STYLESTATE_ACTIVE);
	if (psy_ui_app()->dragevent.active) {
		psy_ui_app()->dragevent.mouse = *ev;
		self->vtable->ondrop(self, &psy_ui_app()->dragevent);
		if (ev->preventdefault) {
			psy_ui_mouseevent_stoppropagation(ev);
		}
	}
}

static void onmousedoubleclick(psy_ui_Component* self, psy_ui_MouseEvent* ev) { }

static void onmouseenter(psy_ui_Component* self)
{
	psy_ui_component_addstylestate(self, psy_ui_STYLESTATE_HOVER);
}

static void onmouseleave(psy_ui_Component* self)
{
	psy_ui_component_removestylestate(self, psy_ui_STYLESTATE_HOVER);
}

static void onkeydown(psy_ui_Component* self, psy_ui_KeyEvent* ev) { }
static void onkeyup(psy_ui_Component* self, psy_ui_KeyEvent* ev) { }
static void ontimer(psy_ui_Component* self, uintptr_t timerid)
{
	if (self->style.currstyle->backgroundid &&
		self->style.currstyle->backgroundanimation.animate) {
		self->bgframetimer++;
		if (self->bgframetimer == self->style.currstyle->backgroundanimation.animatetime) {
			psy_ui_component_invalidate(self);			
			self->currbgframe++;
			self->bgframetimer = 0;
		}		
	}
	
}
static void onlanguagechanged(psy_ui_Component* self) { }

static void onfocus(psy_ui_Component* self)
{
	psy_ui_component_addstylestate(self, psy_ui_STYLESTATE_FOCUS);
}

static void onfocuslost(psy_ui_Component* self)
{
	psy_ui_component_removestylestate(self, psy_ui_STYLESTATE_FOCUS);
}

static void onupdatestyles(psy_ui_Component* self) { }
static void ondragstart(psy_ui_Component* self, psy_ui_DragEvent* ev) { }
static void ondragover(psy_ui_Component* self, psy_ui_DragEvent* ev) { }
static void ondrop(psy_ui_Component* self, psy_ui_DragEvent* ev) { }

static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.dispose = dispose;
		vtable.destroy = destroy;		
		vtable.show = show;
		vtable.showstate = showstate;
		vtable.hide = hide;
		vtable.visible = visible;
		vtable.drawvisible = drawvisible;
		vtable.move = move;
		vtable.resize = resize;
		vtable.clientresize = clientresize;		
		vtable.setposition = setposition;		
		vtable.framesize = framesize;
		vtable.scrollto = scrollto;		
		vtable.setfont = setfont;		
		vtable.children = children;
		vtable.section = section;
		vtable.setalign = setalign;
		// events
		vtable.ondestroy = ondestroy;
		vtable.ondestroyed = ondestroyed;
		vtable.ondraw = 0;
		vtable.onalign = onalign;
		vtable.onpreferredsize = onpreferredsize;
		vtable.onpreferredscrollsize = onpreferredscrollsize;
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
		vtable.onlanguagechanged = onlanguagechanged;
		vtable.enableinput = enableinput;
		vtable.preventinput = preventinput;
		vtable.onfocus = onfocus;
		vtable.onfocuslost = onfocuslost;
		vtable.onupdatestyles = onupdatestyles;
		vtable.invalidate = invalidate;
		vtable.ondragstart = ondragstart;
		vtable.ondragover = ondragover;
		vtable.ondrop = ondrop;
		vtable_initialized = TRUE;
	}
}

void psy_ui_component_init_imp(psy_ui_Component* self, psy_ui_Component* parent,
	psy_ui_ComponentImp* imp)
{
	assert(self);
	assert(self != parent);

	vtable_init();
	self->vtable = &vtable;
	if (!parent) {
		psy_ui_app()->main = self;
	}
	self->imp = imp;
	psy_ui_component_init_base(self);
	psy_ui_component_init_signals(self);
	if (parent && parent->containeralign->insertaligntype != psy_ui_ALIGN_NONE) {
		psy_ui_component_setalign(self, parent->containeralign->insertaligntype);
		psy_ui_component_setmargin(self, parent->containeralign->insertmargin);
	}
}

void psy_ui_component_init(psy_ui_Component* self, psy_ui_Component* parent, psy_ui_Component* view)
{	
	assert(self);
	assert(self != parent);

	vtable_init();

	self->vtable = &vtable;
	if (!parent) {
		psy_ui_app()->main = self;
	}
	if (view) {
		self->imp = (psy_ui_ComponentImp*)
			psy_ui_viewcomponentimp_allocinit(
				self, parent, view, "",
				0, 0, 100, 100, 0, 0);		
	} else {
		self->imp = psy_ui_impfactory_allocinit_componentimp(psy_ui_app_impfactory(psy_ui_app()),
			self, parent);
	}
	psy_ui_component_init_base(self);
	psy_ui_component_init_signals(self);
	if (parent && parent->containeralign->insertaligntype != psy_ui_ALIGN_NONE) {
		psy_ui_component_setalign(self, parent->containeralign->insertaligntype);
		psy_ui_component_setmargin(self, parent->containeralign->insertmargin);
	}
}

void dispose(psy_ui_Component* self)
{	
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = 0;	
	psy_ui_componentstyle_dispose(&self->style);	
}

void destroy(psy_ui_Component* self)
{
	if (self->imp) {
		self->imp->vtable->dev_destroy(self->imp);
	}
}

void invalidate(psy_ui_Component* self)
{
	self->imp->vtable->dev_invalidate(self->imp);
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

int drawvisible(psy_ui_Component* self)
{
	return self->imp->vtable->dev_drawvisible(self->imp);
}

void move(psy_ui_Component* self, psy_ui_Point origin)
{	
	self->imp->vtable->dev_move(self->imp, origin);		
}

void resize(psy_ui_Component* self, psy_ui_Size size)
{			
	self->imp->vtable->dev_resize(self->imp, size);	
}

void clientresize(psy_ui_Component* self, psy_ui_Size size)
{	
	self->imp->vtable->dev_clientresize(self->imp,
		(intptr_t)psy_ui_value_px(&size.width,
			psy_ui_component_textmetric(self)),
		(intptr_t)psy_ui_value_px(&size.height,
			psy_ui_component_textmetric(self)));
}

void setposition(psy_ui_Component* self, psy_ui_Point topleft,
	psy_ui_Size size)
{	
	self->imp->vtable->dev_setposition(self->imp, topleft, size);
	if (!psy_ui_app()->alignvalid || 
		((self->imp->vtable->dev_flags && self->imp->vtable->dev_flags(self->imp) & psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) ==
			psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN)) {				
		psy_ui_component_align(self);				
		if (self->scroll->overflow != psy_ui_OVERFLOW_HIDDEN) {
			psy_ui_component_updateoverflow(self);
		}		
	}
}

psy_ui_Size framesize(psy_ui_Component* self)
{
	return self->imp->vtable->dev_framesize(self->imp);
}

void scrollto(psy_ui_Component* self, intptr_t dx, intptr_t dy)
{
	self->imp->vtable->dev_scrollto(self->imp, dx, dy);
}

void setfont(psy_ui_Component* self, const psy_ui_Font* font)
{
	if (font) {
		int dispose;

		dispose = self->style.currstyle->use_font;
		self->imp->vtable->dev_setfont(self->imp, font);
		if (dispose) {
			psy_ui_font_dispose(&self->style.currstyle->font);
		}
		psy_ui_font_init(&self->style.currstyle->font, 0);
		psy_ui_font_copy(&self->style.currstyle->font, font);
		self->style.currstyle->use_font = 1;
	} else {
		int dispose;

		dispose = self->style.currstyle->use_font;
		self->style.currstyle->use_font = 0;
		psy_ui_component_updatefont(self);
		self->imp->vtable->dev_setfont(self->imp, psy_ui_component_font(self));		
		if (dispose) {
			psy_ui_font_dispose(&self->style.currstyle->font);
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
	psy_signal_init(&self->signal_languagechanged);
}

void psy_ui_component_init_base(psy_ui_Component* self) {
	if (!containeralign_initialized) {
		psy_ui_componentcontaineralign_init(&containeralign);
		containeralign_initialized = TRUE;
	}
	self->containeralign = &containeralign;
	if (!componentscroll_initialized) {
		psy_ui_componentscroll_init(&componentscroll);
		componentscroll_initialized = TRUE;
	}
	self->scroll = &componentscroll;
	if (!sizehints_initialized) {
		psy_ui_componentsizehints_init(&sizehints);
		sizehints_initialized = TRUE;
	}
	self->sizehints = &sizehints;
	psy_ui_componentstyle_init(&self->style);	
	self->align = psy_ui_ALIGN_NONE;
	self->alignsorted = psy_ui_ALIGN_NONE;
	self->deallocate = FALSE;	
	psy_ui_componentstyle_init(&self->style);	
	self->debugflag = 0;
	self->visible = 1;
	self->doublebuffered = FALSE;	
	self->backgroundmode = psy_ui_SETBACKGROUND;
	self->tabindex = psy_INDEX_INVALID;
	self->opcount = 0;
	self->draggable = FALSE;
	self->bgframetimer = FALSE;
	self->currbgframe = 0;
	psy_ui_component_updatefont(self);
	if (self->imp) {
		self->imp->vtable->dev_setbackgroundcolour(self->imp,
			psy_ui_component_backgroundcolour(self));
	}
}

void psy_ui_component_dispose(psy_ui_Component* self)
{	
	self->vtable->dispose(self);
	psy_ui_component_dispose_signals(self);	
	if (psy_ui_app()->hover == self) {
		psy_ui_app_sethover(psy_ui_app(), NULL);
	}	
	if (self->scroll != &componentscroll) {		
		free(self->scroll);
		self->scroll = NULL;
	}
	if (self->sizehints != &sizehints) {
		free(self->sizehints);
		self->sizehints = NULL;
	}
	if (self->containeralign != &containeralign) {
		free(self->containeralign);
		self->containeralign = NULL;
	}
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
	psy_signal_dispose(&self->signal_languagechanged);
}

void psy_ui_component_destroy(psy_ui_Component* self)
{
	self->vtable->destroy(self);
}

void psy_ui_component_deallocate(psy_ui_Component* self)
{
	psy_ui_component_destroy(self);
	free(self);
}

void psy_ui_component_deallocateafterdestroyed(psy_ui_Component* self)
{
	self->deallocate = TRUE;
}

psy_ui_Component* psy_ui_component_alloc(void)
{
	return (psy_ui_Component*)malloc(sizeof(psy_ui_Component));	
}

psy_ui_Component* psy_ui_component_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view)
{
	psy_ui_Component* rv;

	rv = psy_ui_component_alloc();
	if (rv) {
		psy_ui_component_init(rv, parent, view);
		psy_ui_component_deallocateafterdestroyed(rv);		
	}
	return rv;
}

void psy_ui_component_scrollstep(psy_ui_Component* self, double stepx,
	double stepy)
{
	if (stepy != 0 || stepx != 0) {		
		self->vtable->scrollto(self,
			(intptr_t)(psy_ui_component_scrollstep_width_px(self) * stepx),
			(intptr_t)(psy_ui_component_scrollstep_height_px(self) * stepy));
	}
}

void psy_ui_component_show_align(psy_ui_Component* self)
{
	assert(self);

	if (!psy_ui_component_visible(self)) {
		if (psy_ui_component_parent(self)) {
			self->visible = 1;
			psy_ui_component_align(psy_ui_component_parent(self));
		}
		self->vtable->show(self);
	}
}

void psy_ui_component_hide_align(psy_ui_Component* self)
{
	assert(self);

	if (psy_ui_component_visible(self)) {
		self->vtable->hide(self);
		if (psy_ui_component_parent(self)) {
			psy_ui_component_align(psy_ui_component_parent(self));
		}
	}
}

void psy_ui_component_showstate(psy_ui_Component* self, int state)
{
	self->vtable->showstate(self, state);	
}

void psy_ui_component_move(psy_ui_Component* self, psy_ui_Point topleft)
{	
	self->vtable->move(self, topleft);
}

void psy_ui_component_resize(psy_ui_Component* self, psy_ui_Size size)
{		
	self->vtable->resize(self, size);	
}

void psy_ui_component_setposition(psy_ui_Component* self, psy_ui_Rectangle position)
{	
	self->vtable->setposition(self, position.topleft, position.size);	
}

psy_List* psy_ui_component_children(psy_ui_Component* self, int recursive)
{	
	return self->vtable->children(self, recursive);	
}

void psy_ui_component_setfont(psy_ui_Component* self, const psy_ui_Font* source)
{	
	self->vtable->setfont(self, source);
}

void psy_ui_component_setfontinfo(psy_ui_Component* self,
	psy_ui_FontInfo fontinfo)
{
	psy_ui_Font font;

	psy_ui_font_init(&font, &fontinfo);
	psy_ui_component_setfont(self, &font);	
	psy_ui_font_dispose(&font);	
}

int psy_ui_component_visible(psy_ui_Component* self)
{
	return self->vtable->visible(self);	
}

int psy_ui_component_drawvisible(psy_ui_Component* self)
{
	return self->vtable->drawvisible(self);
}


void psy_ui_component_align(psy_ui_Component* self)
{
	switch (self->containeralign->containeralign) {
	case psy_ui_CONTAINER_ALIGN_LCL: {
		psy_ui_LCLAligner aligner;

		psy_ui_lclaligner_init(&aligner, self);
		psy_ui_aligner_align(psy_ui_lclaligner_base(&aligner));
		psy_signal_emit(&self->signal_align, self, 0);
		self->vtable->onalign(self);
		psy_ui_aligner_dispose(psy_ui_lclaligner_base(&aligner));
		break; }
	case psy_ui_CONTAINER_ALIGN_GRID: {
		psy_ui_GridAligner aligner;

		psy_ui_gridaligner_init(&aligner, self);
		psy_ui_aligner_align(psy_ui_gridaligner_base(&aligner));
		psy_signal_emit(&self->signal_align, self, 0);
		self->vtable->onalign(self);
		psy_ui_aligner_dispose(psy_ui_gridaligner_base(&aligner));
		break; }
	default:
		break;
	}
}

void psy_ui_component_align_full(psy_ui_Component* self)
{
	psy_ui_app()->alignvalid = FALSE;
	psy_ui_component_align(self);
	psy_ui_app()->alignvalid = TRUE;
}

void onpreferredsize(psy_ui_Component* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	psy_ui_LCLAligner aligner;

	psy_ui_lclaligner_init(&aligner, self);
	psy_ui_aligner_preferredsize(psy_ui_lclaligner_base(&aligner), limit, rv);	
}

void onpreferredscrollsize(psy_ui_Component* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	onpreferredsize(self, limit, rv);
}

void psy_ui_component_doublebuffer(psy_ui_Component* self)
{
	self->doublebuffered = TRUE;
}

void psy_ui_component_setmargin_children(psy_ui_Component* self,
	psy_ui_Margin margin)
{
	psy_ui_component_traverse_margin(self,
		(psy_fp_margin)psy_ui_component_setmargin, margin);
}

psy_ui_Margin psy_ui_component_bordermargin(const psy_ui_Component* self)
{
	psy_ui_Margin rv;
	const psy_ui_Border* border;

	psy_ui_margin_init(&rv);
	border = psy_ui_component_border(self);
	if (border->left == psy_ui_BORDER_SOLID) {		
		rv.left = psy_ui_value_make_px(1);
	}
	if (border->top == psy_ui_BORDER_SOLID) {
		rv.top = psy_ui_value_make_px(1);		
	}
	if (border->right == psy_ui_BORDER_SOLID) {
		rv.right = psy_ui_value_make_px(1);
	}
	if (border->bottom == psy_ui_BORDER_SOLID) {
		rv.bottom = psy_ui_value_make_px(1);
	}
	return rv;
}

void psy_ui_component_setspacing_children(psy_ui_Component* self,
	psy_ui_Margin spacing)
{
	psy_ui_component_traverse_margin(self,
		(psy_fp_margin)psy_ui_component_setspacing, spacing);	
}

uintptr_t psy_ui_component_index(psy_ui_Component* self)
{
	uintptr_t rv;
	uintptr_t i;
	psy_List* p;
	psy_List* q;

	rv = psy_INDEX_INVALID;
	q = psy_ui_component_children(psy_ui_component_parent(self),
		psy_ui_NONRECURSIVE);
	for (i = 0, p = q; p != NULL; p = p->next, ++i) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
		if (component == self) {
			rv = i;
			break;
		}
	}
	psy_list_free(q);
	return rv;
}

void psy_ui_component_setalign_children(psy_ui_Component* self,
	psy_ui_AlignType align)
{
	psy_ui_component_traverse_int(self,
		(psy_fp_int)psy_ui_component_setalign, align);	
}

void psy_ui_component_checksortedalign(psy_ui_Component* self,
	psy_ui_AlignType align)
{
	if (self->alignsorted != align) {
		psy_List* p;
		psy_List* q;

		q = psy_ui_component_children(self, psy_ui_NONRECURSIVE);
		for (p = q; p != NULL; p = p->next) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)p->entry;
			if (component->align != align) {
				self->alignsorted = psy_ui_ALIGN_NONE;
				break;
			}
		}
		if (p == NULL) {
			self->alignsorted = align;
		}
		psy_list_free(q);
	}
}

void psy_ui_component_setalign(psy_ui_Component* self, psy_ui_AlignType align)
{
	psy_ui_Component* parent;

	self->align = align;
	if (parent = psy_ui_component_parent(self)) {
		psy_ui_component_checksortedalign(parent, align);
	}
	self->vtable->setalign(self, align);
}

void psy_ui_component_setcontaineralign(psy_ui_Component* self, 
	psy_ui_ContainerAlignType containeralign)
{
	psy_ui_component_usecontaineralign(self);
	self->containeralign->containeralign = containeralign;
}

void psy_ui_component_preventalign(psy_ui_Component* self)
{
	psy_ui_component_usecontaineralign(self);
	self->containeralign->containeralign = psy_ui_CONTAINER_ALIGN_NONE;
}

void psy_ui_component_setalignexpand(psy_ui_Component* self, psy_ui_ExpandMode mode)
{
	psy_ui_component_usecontaineralign(self);
	self->containeralign->alignexpandmode = mode;
}

void psy_ui_component_enableinput(psy_ui_Component* self, int recursive)
{
	if (psy_ui_component_inputprevented(self)) {
		enableinput_internal(self, TRUE, recursive);		
		psy_ui_component_invalidate(self);
	}
}

void psy_ui_component_preventinput(psy_ui_Component* self, int recursive)
{
	if (!psy_ui_component_inputprevented(self)) {
		enableinput_internal(self, FALSE, recursive);		
		psy_ui_component_invalidate(self);
	}
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
	psy_ui_component_removestylestate(self, psy_ui_STYLESTATE_DISABLED);
}

static void preventinput(psy_ui_Component* self)
{
	self->imp->vtable->dev_preventinput(self->imp);
	psy_ui_component_addstylestate(self, psy_ui_STYLESTATE_DISABLED);	
}

bool psy_ui_component_inputprevented(const psy_ui_Component* self)
{
	return self->imp->vtable->dev_inputprevented(self->imp);
}

void psy_ui_component_setbackgroundmode(psy_ui_Component* self,
	psy_ui_BackgroundMode mode)
{
	self->backgroundmode = mode;	
}

psy_List* psy_ui_components_setalign(psy_List* list, psy_ui_AlignType align,
	psy_ui_Margin margin)
{
	psy_List* p;

	for (p = list; p != NULL; p = p->next) {
		psy_ui_component_setalign((psy_ui_Component*) p->entry, align);		
		psy_ui_component_setmargin((psy_ui_Component*) p->entry, margin);		
	}
	return list;
}

psy_List* psy_ui_components_setmargin(psy_List* list, psy_ui_Margin margin)
{
	psy_List* p;

	for (p = list; p != NULL; p = p->next) {
		psy_ui_component_setmargin((psy_ui_Component*)p->entry, margin);
	}
	return list;
}

void psy_ui_component_setpreferredsize(psy_ui_Component* self, psy_ui_Size size)
{
	psy_ui_component_usesizehints(self);
	self->sizehints->preferredsize = size;
	self->sizehints->preferredwidthset = TRUE;
	self->sizehints->preferredheightset = TRUE;
}

psy_ui_Size psy_ui_component_preferredsize(psy_ui_Component* self,
	const psy_ui_Size* limit)
{		
	psy_ui_Size rv;

	rv = self->sizehints->preferredsize;
	if (self->sizehints->preferredwidthset &&
		self->sizehints->preferredheightset) {
		return rv;
	}
	self->vtable->onpreferredsize(self, limit, &rv);
	if (self->sizehints->preferredwidthset) {
		rv.width = self->sizehints->preferredsize.width;
	}
	if (self->sizehints->preferredheightset) {
		rv.height = self->sizehints->preferredsize.height;
	}
	return rv;	
}

psy_ui_Size psy_ui_component_preferredscrollsize(psy_ui_Component* self,
	const psy_ui_Size* limit)
{
	return psy_ui_component_preferredsize(self, limit);	
}

psy_ui_RealSize psy_ui_component_preferredscrollsize_px(psy_ui_Component* self,
	const psy_ui_Size* limit)
{	
	psy_ui_Size size;
	
	size = psy_ui_component_preferredscrollsize(self, limit);
	return psy_ui_size_px(&size, psy_ui_component_textmetric(self));
}

void psy_ui_component_setmaximumsize(psy_ui_Component* self, psy_ui_Size size)
{
	psy_ui_component_usesizehints(self);
	self->sizehints->maxsize = size;
}

const psy_ui_Size psy_ui_component_maximumsize(const psy_ui_Component* self)
{
	return self->sizehints->maxsize;
}

void psy_ui_component_setminimumsize(psy_ui_Component* self, psy_ui_Size size)
{
	psy_ui_component_usesizehints(self);
	self->sizehints->minsize = size;	
}

const psy_ui_Size psy_ui_component_minimumsize(const psy_ui_Component* self)
{
	return self->sizehints->minsize;
}

psy_ui_Size psy_ui_component_size(const psy_ui_Component* self)
{
	psy_ui_Size rv;
	const psy_ui_TextMetric* tm;
	psy_ui_Margin spacing;
	psy_ui_Value spacing_width;
	psy_ui_Value spacing_height;


	rv = psy_ui_component_scrollsize(self);
	spacing = psy_ui_component_spacing(self);
	tm = psy_ui_component_textmetric(self);
	spacing_width = psy_ui_margin_width(&spacing, tm);
	psy_ui_value_sub(&rv.width, &spacing_width, tm);
	spacing_height = psy_ui_margin_height(&spacing, tm);
	psy_ui_value_sub(&rv.height, &spacing_height, tm);
	return rv;
}

psy_ui_Size psy_ui_component_sub_border(const psy_ui_Component* self,
	psy_ui_Size size)
{
	psy_ui_Size rv;
	const psy_ui_Border* border;
	psy_ui_Size bordersize;
	const psy_ui_TextMetric* tm;

	rv = size;
	border = psy_ui_component_border(self);
	bordersize = psy_ui_border_size(border);
	tm = psy_ui_component_textmetric(self);
	psy_ui_value_sub(&rv.width, &bordersize.width, tm);	
	psy_ui_value_sub(&rv.height, &bordersize.height, tm);
	return rv;
}

psy_ui_Size psy_ui_component_clientsize(const psy_ui_Component* self)
{		
	if (self->scroll->overflow == psy_ui_OVERFLOW_HIDDEN) {		
		return psy_ui_component_sub_border(self,
			self->imp->vtable->dev_size(self->imp));
	} else {		
		psy_ui_Size rv;
		psy_ui_Size size;
		psy_ui_Size scrollpane;

		scrollpane = self->imp->vtable->dev_size(
			psy_ui_component_parent_const(self)->imp);
		size = self->imp->vtable->dev_size(self->imp);		
		if (self->scroll->overflow & psy_ui_OVERFLOW_HSCROLL) {
			rv.width = scrollpane.width;
		} else {
			rv.width = size.width;
		}
		if (self->scroll->overflow & psy_ui_OVERFLOW_VSCROLL) {
			rv.height = scrollpane.height;
		} else {
			rv.height = size.height;
		}
		return rv;
	}
}

void psy_ui_component_seticonressource(psy_ui_Component* self, int ressourceid)
{
	self->imp->vtable->dev_seticonressource(self->imp, ressourceid);
}

void psy_ui_component_clientresize(psy_ui_Component* self, psy_ui_Size size)
{
	self->vtable->clientresize(self, size);
}

psy_ui_Size psy_ui_component_scrollsize(const psy_ui_Component* self)
{
	return self->imp->vtable->dev_size(self->imp);
}

psy_ui_Size psy_ui_component_offsetsize(const psy_ui_Component* self)
{
	return self->imp->vtable->dev_size(
		psy_ui_component_parent_const(self)->imp);	
}

void psy_ui_component_setcursor(psy_ui_Component* self, psy_ui_CursorStyle style)
{
	self->imp->vtable->dev_setcursor(self->imp, style);
}

static psy_ui_TextMetric default_tm;
static bool default_tm_initialized = FALSE;

// psy_ui_ComponentImp vtable
static void dev_dispose(psy_ui_ComponentImp* self) { }
static void dev_destroy(psy_ui_ComponentImp* self) { }
static void dev_destroyed(psy_ui_ComponentImp* self) { }
static void dev_show(psy_ui_ComponentImp* self) { }
static void dev_showstate(psy_ui_ComponentImp* self, int state) { }
static void dev_hide(psy_ui_ComponentImp* self) { }
static int dev_visible(psy_ui_ComponentImp* self) { return 0; }
static int dev_drawvisible(psy_ui_ComponentImp* self) { return 0; }

static psy_ui_RealRectangle dev_position(const psy_ui_ComponentImp* self)
{
	return psy_ui_realrectangle_zero();
}

static psy_ui_RealRectangle dev_screenposition(const psy_ui_ComponentImp* self)
{
	return psy_ui_realrectangle_zero();
}

static void dev_move(psy_ui_ComponentImp* self, psy_ui_Point origin) { }
static void dev_resize(psy_ui_ComponentImp* self, psy_ui_Size size) { }
static void dev_clientresize(psy_ui_ComponentImp* self, intptr_t width, intptr_t height) { }
static void dev_setposition(psy_ui_ComponentImp* self, psy_ui_Point topleft, psy_ui_Size size) { }
static psy_ui_Size dev_size(const psy_ui_ComponentImp* self) { return psy_ui_size_zero(); }
static psy_ui_Size dev_preferredsize(psy_ui_ComponentImp* self, const psy_ui_Size* limit) { return psy_ui_size_zero(); }
static void dev_updatesize(psy_ui_ComponentImp* self) { }
static psy_ui_Size dev_framesize(psy_ui_ComponentImp* self) { return psy_ui_size_zero(); }
static void dev_scrollto(psy_ui_ComponentImp* self, intptr_t dx, intptr_t dy) { }
static psy_ui_Component* dev_parent(psy_ui_ComponentImp* self) { return 0;  }
static void dev_setparent(psy_ui_ComponentImp* self, psy_ui_Component* parent) { }
static void dev_insert(psy_ui_ComponentImp* self, psy_ui_ComponentImp* child, psy_ui_ComponentImp* insertafter) { }
static void dev_remove(psy_ui_ComponentImp* self, psy_ui_ComponentImp* child) { }
static void dev_erase(psy_ui_ComponentImp* self, psy_ui_ComponentImp* child) { }
static void dev_setorder(psy_ui_ComponentImp* self, psy_ui_ComponentImp* insertafter) { }
static void dev_capture(psy_ui_ComponentImp* self) { }
static void dev_releasecapture(psy_ui_ComponentImp* self) { }
static void dev_invalidate(psy_ui_ComponentImp* self) { }
static void dev_invalidaterect(psy_ui_ComponentImp* self, const psy_ui_RealRectangle* r) { }
static void dev_update(psy_ui_ComponentImp* self) { }
static void dev_setfont(psy_ui_ComponentImp* self, const psy_ui_Font* font) { }
static void dev_showhorizontalscrollbar(psy_ui_ComponentImp* self) { }
static void dev_hidehorizontalscrollbar(psy_ui_ComponentImp* self) { }
static void dev_sethorizontalscrollrange(psy_ui_ComponentImp* self, intptr_t min, intptr_t max) { }
static void dev_horizontalscrollrange(psy_ui_ComponentImp* self, intptr_t* scrollmin, intptr_t* scrollmax) { *scrollmin = 0; *scrollmax = 0; }
static int dev_horizontalscrollposition(psy_ui_ComponentImp* self) { return 0;  }
static void dev_sethorizontalscrollposition(psy_ui_ComponentImp* self, intptr_t position) { }
static void dev_showverticalscrollbar(psy_ui_ComponentImp* self) { }
static void dev_hideverticalscrollbar(psy_ui_ComponentImp* self) { }
static void dev_setverticalscrollrange(psy_ui_ComponentImp* self, intptr_t min, intptr_t max) { }
static void dev_verticalscrollrange(psy_ui_ComponentImp* self, intptr_t* scrollmin, intptr_t* scrollmax) { *scrollmin = 0; *scrollmax = 0; }
static int dev_verticalscrollposition(psy_ui_ComponentImp* self) { return 0; }
static void dev_setverticalscrollposition(psy_ui_ComponentImp* self, intptr_t position) { }
static psy_List* dev_children(psy_ui_ComponentImp* self, int recursive) { return 0; }
static void dev_enableinput(psy_ui_ComponentImp* self) { }
static void dev_preventinput(psy_ui_ComponentImp* self) { }
static bool dev_inputprevented(const psy_ui_ComponentImp* self) { return FALSE; }
static void dev_setcursor(psy_ui_ComponentImp* self, psy_ui_CursorStyle cursorstyle) { }
static void dev_starttimer(psy_ui_ComponentImp* self, uintptr_t id, uintptr_t interval) { }
static void dev_stoptimer(psy_ui_ComponentImp* self, uintptr_t id) { }
static void dev_seticonressource(psy_ui_ComponentImp* self, int ressourceid) { }
static const psy_ui_TextMetric* dev_textmetric(const psy_ui_ComponentImp* self)
{
	if (!default_tm_initialized) {
		memset(&default_tm, 0, sizeof(default_tm));
		default_tm.tmHeight = 12;
		default_tm.tmAveCharWidth = 8;
		default_tm_initialized = TRUE;
	}
	return &default_tm;
}
static psy_ui_Size dev_textsize(const psy_ui_ComponentImp* self, const char* text,
	const psy_ui_Font* font)
{
	psy_ui_Size rv = { 0, 0 }; return rv;
}

static void dev_setbackgroundcolour(psy_ui_ComponentImp* self, psy_ui_Colour colour) { }
static void dev_settitle(psy_ui_ComponentImp* self, const char* title) { }
static void dev_setfocus(psy_ui_ComponentImp* self) { }
static int dev_hasfocus(psy_ui_ComponentImp* self) { return 0;  }
static void* dev_platform(psy_ui_ComponentImp* self) { return (void*) self; }
static uintptr_t dev_flags(const psy_ui_ComponentImp* self) { return 0; }
static void dev_clear(psy_ui_ComponentImp* self) {  }
static void dev_draw(psy_ui_ComponentImp* self, psy_ui_Graphics* g) { }
static void dev_mousedown(psy_ui_ComponentImp* self, psy_ui_MouseEvent* ev) { }
static void dev_mouseup(psy_ui_ComponentImp* self, psy_ui_MouseEvent* ev) { }
static void dev_mousemove(psy_ui_ComponentImp* self, psy_ui_MouseEvent* ev) { }
static void dev_mousedoubleclick(psy_ui_ComponentImp* self, psy_ui_MouseEvent* ev) { }
static void dev_mouseenter(psy_ui_ComponentImp* self) { }
static void dev_mouseleave(psy_ui_ComponentImp* self) { }

static psy_ui_ComponentImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = dev_dispose;
		imp_vtable.dev_destroy = dev_destroy;
		imp_vtable.dev_destroyed = dev_destroyed;
		imp_vtable.dev_show = dev_show;
		imp_vtable.dev_showstate = dev_showstate;
		imp_vtable.dev_hide = dev_hide;
		imp_vtable.dev_visible = dev_visible;
		imp_vtable.dev_drawvisible = dev_drawvisible;
		imp_vtable.dev_move = dev_move;
		imp_vtable.dev_resize = dev_resize;
		imp_vtable.dev_clientresize = dev_clientresize;
		imp_vtable.dev_position = dev_position;
		imp_vtable.dev_screenposition = dev_position;
		imp_vtable.dev_setposition = dev_setposition;
		imp_vtable.dev_size = dev_size;
		imp_vtable.dev_preferredsize = dev_preferredsize;
		imp_vtable.dev_updatesize = dev_updatesize;
		imp_vtable.dev_framesize = dev_framesize;
		imp_vtable.dev_scrollto = dev_scrollto;
		imp_vtable.dev_parent = dev_parent;
		imp_vtable.dev_setparent = dev_setparent;
		imp_vtable.dev_insert = dev_insert;
		imp_vtable.dev_remove = dev_remove;
		imp_vtable.dev_erase = dev_erase;
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
		imp_vtable.dev_inputprevented = dev_inputprevented;
		imp_vtable.dev_setcursor = dev_setcursor;
		imp_vtable.dev_starttimer = dev_starttimer;
		imp_vtable.dev_stoptimer = dev_stoptimer;
		imp_vtable.dev_seticonressource = dev_seticonressource;
		imp_vtable.dev_textmetric = dev_textmetric;
		imp_vtable.dev_textsize = dev_textsize;
		imp_vtable.dev_setbackgroundcolour = dev_setbackgroundcolour;
		imp_vtable.dev_settitle = dev_settitle;
		imp_vtable.dev_setfocus = dev_setfocus;
		imp_vtable.dev_hasfocus = dev_hasfocus;
		imp_vtable.dev_platform = dev_platform;
		imp_vtable.dev_flags = dev_flags;
		imp_vtable.dev_clear = dev_clear;
		imp_vtable.dev_draw = dev_draw;
		imp_vtable.dev_mouseup = dev_mouseup;
		imp_vtable.dev_mousedown = dev_mousedown;
		imp_vtable.dev_mousemove = dev_mousemove;
		imp_vtable.dev_mousedoubleclick = dev_mousedoubleclick;
		imp_vtable.dev_mouseenter = dev_mouseenter;
		imp_vtable.dev_mouseleave = dev_mouseleave;
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
	psy_ui_Point position)
{		
	psy_ui_component_setscrollleft(self, position.x);
	psy_ui_component_setscrolltop(self, position.y);
}

void psy_ui_component_setscrollleft(psy_ui_Component* self, psy_ui_Value left)
{		
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(self);
	psy_ui_component_move(self,
		psy_ui_point_make(
			psy_ui_value_make_px(
				-psy_ui_value_px(&left, psy_ui_component_textmetric(self))),
			psy_ui_value_make_px(position.top)));
	psy_signal_emit(&self->signal_scroll, self, 0);	
}

void psy_ui_component_setscrolltop(psy_ui_Component* self, psy_ui_Value top)
{	
	psy_ui_RealRectangle position;
	psy_ui_RealSize parentsize;
	double newtop;

	position = psy_ui_component_position(self);	
	newtop = -psy_ui_value_px(&top, psy_ui_component_textmetric(self));
	/*if (psy_ui_component_overflow(self) == psy_ui_OVERFLOW_VSCROLL) {
		psy_ui_IntPoint range;
		double miny;
		double maxy;

		range = psy_ui_component_verticalscrollrange(self);
		miny = range.x * psy_ui_component_scrollstep_width_px(self);
		maxy = range.y * psy_ui_component_scrollstep_width_px(self);
		if (newtop < miny) {
			newtop = miny;
		}
	}*/	
	psy_ui_component_move(self, psy_ui_point_make_px(position.left, newtop));
	parentsize = psy_ui_component_scrollsize_px(psy_ui_component_parent(self));
	position = psy_ui_component_position(self);
	if (parentsize.height > position.bottom) {
		psy_ui_RealPoint translate;
		psy_ui_Component* bgcomponent;

		bgcomponent = psy_ui_component_parent(self);
		psy_ui_realpoint_init(&translate);
		while (bgcomponent && bgcomponent->backgroundmode == psy_ui_NOBACKGROUND) {
			bgcomponent = psy_ui_component_parent(bgcomponent);
			if (bgcomponent) {
				psy_ui_RealRectangle position;

				position = psy_ui_component_position(bgcomponent);
				translate.x += position.left;
				translate.y += position.top;
			}
		}
		if (bgcomponent) {			
			psy_ui_component_invalidaterect(
				bgcomponent,
				psy_ui_realrectangle_make(
					psy_ui_realpoint_make(
						position.left + translate.x,
						position.bottom + translate.y),
					psy_ui_realsize_make(
						position.right - position.left,
						parentsize.height - position.bottom)));
		}
	}
	psy_signal_emit(&self->signal_scroll, self, 0);	
}

void psy_ui_component_updateoverflow(psy_ui_Component* self)
{	
	if ((self->scroll->overflow & psy_ui_OVERFLOW_VSCROLL) ==
			psy_ui_OVERFLOW_VSCROLL) {
		psy_ui_Size scrollsize;
		const psy_ui_TextMetric* tm;
		double scrollmax;
		double visi;
		double curr;
		psy_ui_Size size;		
		double scrollstepy_px;		
		
		size = psy_ui_component_scrollsize(psy_ui_component_parent(self));
		scrollsize = psy_ui_component_preferredscrollsize(self, &size);
		scrollstepy_px = psy_ui_component_scrollstep_height_px(self);
		tm = psy_ui_component_textmetric(self);
		curr = lines(
			psy_ui_component_scrolltoppx(self),
			psy_ui_value_px(&size.height, tm),
			psy_ui_value_px(&scrollsize.height, tm),			
			scrollstepy_px,
			TRUE, /* round */
			&scrollmax,
			&visi);
		if ((self->scroll->overflow & psy_ui_OVERFLOW_VSCROLLCENTER) ==
				psy_ui_OVERFLOW_VSCROLLCENTER) {
			psy_ui_component_setverticalscrollrange(self,
				psy_ui_intpoint_make((intptr_t)(-visi / 2),
				(intptr_t)(scrollmax - visi / 2 - 1)));
			if (curr > scrollmax - visi / 2) {
				curr = psy_max(-visi / 2, scrollmax -visi / 2);
				psy_ui_component_setscrolltop(self,
					psy_ui_value_make_px((double)(curr * scrollstepy_px)));
			}
		} else {		
			psy_ui_component_setverticalscrollrange(self,
				psy_ui_intpoint_make(0, (intptr_t)(scrollmax - visi)));
			if (curr > scrollmax - visi) {
				curr = psy_max(0, scrollmax - visi);
				psy_ui_component_setscrolltop(self,
					psy_ui_value_make_px(curr * scrollstepy_px));
			}
		}		
	}
	if ((self->scroll->overflow & psy_ui_OVERFLOW_HSCROLL) ==
			psy_ui_OVERFLOW_HSCROLL) {
		psy_ui_Size scrollsize;
		const psy_ui_TextMetric* tm;		
		double scrollmax;
		double visi;
		double currrow;
		psy_ui_Size size;
		double scrollstepx_px;		
		
		size = psy_ui_component_scrollsize(psy_ui_component_parent(self));
		scrollsize = psy_ui_component_preferredscrollsize(self, &size);
		scrollstepx_px = floor(psy_ui_component_scrollstep_width_px(self));
		tm = psy_ui_component_textmetric(self);
		currrow = lines(
			psy_ui_component_scrollleftpx(self),
			psy_ui_value_px(&size.width, tm),
			psy_ui_value_px(&scrollsize.width, tm),			
			scrollstepx_px,
			FALSE, /* no round */
			&scrollmax,
			&visi);		
		psy_ui_component_sethorizontalscrollrange(self,
			psy_ui_intpoint_make(0, (intptr_t)(scrollmax - visi)));
		if (currrow > scrollmax - visi) {
			currrow = psy_max(0, scrollmax - visi);
			psy_ui_component_setscrollleft(self,
				psy_ui_value_make_px(currrow * scrollstepx_px));
		}		
	}
}

double lines(double pos, double size, double scrollsize, double step,
	bool round, double* rv_max, double* rv_visi)
{
	*rv_visi = floor(size / step);
	*rv_max = floor(scrollsize / step + ((round) ? 0.5 : 0.0));
	return floor(pos / step);
}

void psy_ui_component_drawborder(psy_ui_Component* self, psy_ui_Graphics* g)
{	
	psy_ui_drawborder(g,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_zero(),
			psy_ui_component_scrollsize_px(self)),
		psy_ui_component_border(self),
		psy_ui_component_textmetric(self));
}

void psy_ui_component_drawbackgroundimage(psy_ui_Component* self,
	psy_ui_Graphics* g, psy_ui_Bitmap* bitmap, psy_ui_BackgroundRepeat repeat,
	psy_ui_Alignment alignment)
{
	if (bitmap && !psy_ui_bitmap_empty(bitmap)) {
		psy_ui_RealSize size;
		psy_ui_RealPoint cp;
		psy_ui_RealSize bmpsize;
		psy_ui_RealRectangle position;
		psy_ui_RealPoint src;

		size = psy_ui_component_size_px(self);
		psy_ui_realpoint_init(&cp);
		bmpsize = psy_ui_bitmap_size(bitmap);
		if (self->style.currstyle->backgroundanimation.animate) {
			src = psy_ui_realpoint_make(
				self->currbgframe *
				self->style.currstyle->backgroundanimation.framesize.width,
				0);
			if (src.x >= bmpsize.width) {
				src.x = 0;
				self->currbgframe = 0;
			}
			bmpsize.width = self->style.currstyle->backgroundanimation.framesize.width;
		} else {
			src = psy_ui_realpoint_zero();
		}
		if (repeat == psy_ui_REPEAT) {
			while (cp.y < size.height) {
				position = psy_ui_realrectangle_make(cp, bmpsize);
				psy_ui_drawbitmap(g, bitmap, position, src);
				cp.x += bmpsize.width;
				if (cp.x >= size.width) {
					cp.x = 0.0;
					cp.y += bmpsize.height;
				}
			}
		} else {
			if (alignment & psy_ui_ALIGNMENT_CENTER_HORIZONTAL) {
				cp.x = (size.width - bmpsize.width) / 2.0;
			}
			if (alignment & psy_ui_ALIGNMENT_CENTER_VERTICAL) {
				cp.y = (size.height - bmpsize.height) / 2.0;
			}
			position = psy_ui_realrectangle_make(cp, bmpsize);
			psy_ui_drawbitmap(g, bitmap, position, src);
		}
	}
}

void psy_ui_component_drawbackground(psy_ui_Component* self,
	psy_ui_Graphics* g)
{
	if (self->backgroundmode == psy_ui_SETBACKGROUND) {				
		const psy_ui_Border* b;
		
		b = psy_ui_component_border(self);
		if (psy_ui_border_isround(b)) {
			psy_ui_RealRectangle r;
			const psy_ui_TextMetric* tm;

			r = psy_ui_realrectangle_make(
				psy_ui_realpoint_zero(),
				psy_ui_component_scrollsize_px(self));
			tm = psy_ui_component_textmetric(self);
			psy_ui_drawsolidrectangle(g, r,
				psy_ui_component_backgroundcolour(
					psy_ui_component_parent(self)));
			psy_ui_drawsolidroundrectangle(g, r,
				psy_ui_realsize_make(
					psy_ui_value_px(&b->border_bottom_left_radius, tm),
					psy_ui_value_px(&b->border_bottom_left_radius, tm)),
				psy_ui_component_backgroundcolour(self));
		} else {		
			uintptr_t image_id;
			bool success;
			
			success = FALSE;
			image_id = psy_ui_component_backgroundimageid(self);
			if (image_id != psy_INDEX_INVALID) {
				psy_ui_Bitmap bitmap;

				psy_ui_bitmap_init(&bitmap);
				psy_ui_bitmap_loadresource(&bitmap, image_id);
				success = !psy_ui_bitmap_empty(&bitmap);
				if (success && self->style.currstyle->backgroundrepeat == psy_ui_NOREPEAT) {
					psy_ui_drawsolidrectangle(g, g->clip,
						psy_ui_component_backgroundcolour(self));
				}
				psy_ui_component_drawbackgroundimage(self, g, &bitmap,
					self->style.currstyle->backgroundrepeat,
					self->style.currstyle->backgroundposition);
				psy_ui_bitmap_dispose(&bitmap);
			}
			if (!success) {
				psy_ui_drawsolidrectangle(g, g->clip,
					psy_ui_component_backgroundcolour(self));
			}
		}
	}
}

void psy_ui_component_usescroll(psy_ui_Component* self)
{
	if (self->scroll == &componentscroll) {
		self->scroll = psy_ui_componentscroll_allocinit();
	}
}

void psy_ui_component_usesizehints(psy_ui_Component* self)
{
	if (self->sizehints == &sizehints) {
		self->sizehints = psy_ui_componentsizehints_allocinit();
	}
}

void psy_ui_component_usecontaineralign(psy_ui_Component* self)
{
	if (self->containeralign == &containeralign) {
		self->containeralign = psy_ui_componentcontaineralign_allocinit();
	}
}

bool psy_ui_component_togglevisibility(psy_ui_Component* self)
{
	assert(self);
	if (psy_ui_component_parent(self)) {
		if (psy_ui_component_visible(self)) {
			psy_ui_component_hide(self);
			psy_ui_component_align(psy_ui_component_parent(self));
			return FALSE;
		} else {
			psy_ui_component_hide(self);
			self->visible = 1;
			psy_ui_component_align(psy_ui_component_parent(self));
			psy_ui_component_show(self);
			return TRUE;
		}
	}
	return FALSE;
}

psy_ui_RealRectangle psy_ui_component_scrolledposition(psy_ui_Component* self)
{	
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			psy_ui_component_scrollleftpx(self),
			psy_ui_component_scrolltoppx(self)),
		psy_ui_component_scrollsize_px(self));
}

void psy_ui_component_focus_next(psy_ui_Component* self)
{
	if (self->tabindex != psy_INDEX_INVALID) {
		psy_ui_Component* parent;		
		
		parent = psy_ui_component_parent(self);
		if (parent) {
			psy_ui_Component* focus;
			psy_List* p;
			psy_List* q;
			uintptr_t tabindex;

			tabindex = psy_ui_component_tabindex(self);			
			focus = NULL;
			for (q = p = psy_ui_component_children(parent, 0); p != NULL;
					psy_list_next(&p)) {
				psy_ui_Component* component;

				component = (psy_ui_Component*)psy_list_entry(p);
				if (tabindex < psy_ui_component_tabindex(component)) {
					focus = component;
					tabindex = psy_ui_component_tabindex(focus);
				}
			}
			psy_list_free(q);
			if (focus) {
				psy_ui_component_setfocus(focus);
			}
		}
	}
}

void psy_ui_component_focus_prev(psy_ui_Component* self)
{
	if (self->tabindex != psy_INDEX_INVALID) {
		psy_ui_Component* parent;

		parent = psy_ui_component_parent(self);
		if (parent) {
			psy_ui_Component* focus;
			psy_List* p;
			psy_List* q;
			uintptr_t tabindex;

			tabindex = psy_ui_component_tabindex(self);
			focus = NULL;
			for (q = p = psy_ui_component_children(parent, 0); p != NULL;
					psy_list_next(&p)) {
				psy_ui_Component* component;

				component = (psy_ui_Component*)psy_list_entry(p);				
				if (psy_ui_component_tabindex(component) != psy_INDEX_INVALID &&
						tabindex > psy_ui_component_tabindex(component)) {
					focus = component;
					tabindex = psy_ui_component_tabindex(focus);
				}
			}
			psy_list_free(q);
			if (focus) {
				psy_ui_component_setfocus(focus);
			}
		}
	}
}

int psy_ui_component_level(const psy_ui_Component* self)
{
	int rv;
	const psy_ui_Component* component;

	rv = 0;
	component = self;
	while (psy_ui_component_parent_const(component)) {
		component = psy_ui_component_parent_const(component);
		++rv;
	}
	return rv;
}

void psy_ui_component_setdefaultalign(psy_ui_Component* self,
	psy_ui_AlignType aligntype, psy_ui_Margin margin)
{
	psy_ui_component_usecontaineralign(self);
	self->containeralign->insertaligntype = aligntype;
	self->containeralign->insertmargin = margin;
}

void psy_ui_component_updatelanguage(psy_ui_Component* self)
{
	assert(self);

	self->vtable->onlanguagechanged(self);
	psy_signal_emit(&self->signal_languagechanged, self, 0);
}

psy_Translator* psy_ui_translator(void)
{
	return &psy_ui_app()->translator;
}

const char* psy_ui_translate(const char* key)
{
	return psy_translator_translate(psy_ui_translator(), key);
}

void psy_ui_component_setstyletypes(psy_ui_Component* self,
	uintptr_t standard, uintptr_t hover, uintptr_t select,
	uintptr_t disabled)
{
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_NONE, standard);
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_HOVER, hover);
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_SELECT, select);
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_DISABLED, disabled);	
	psy_ui_componentstyle_readstyles(&self->style);
	psy_ui_component_checkbackgroundanimation(self);
}

void psy_ui_component_checkbackgroundanimation(psy_ui_Component* self)
{
	if (self->style.currstyle->backgroundanimation.animate) {
		psy_ui_component_starttimer(self, 65535, 50);
	} else {
		psy_ui_component_stoptimer(self, 65535);
	}
}

void psy_ui_component_setstyletype(psy_ui_Component* self,
	uintptr_t standard)
{
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_NONE, standard);
	psy_ui_componentstyle_readstyles(&self->style);
	psy_ui_component_checkbackgroundanimation(self);
}

void psy_ui_component_setstyletype_hover(psy_ui_Component* self,
	uintptr_t hover)
{
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_HOVER, hover);
	psy_ui_componentstyle_readstyles(&self->style);
	psy_ui_component_checkbackgroundanimation(self);
}

void psy_ui_component_setstyletype_focus(psy_ui_Component* self,
	uintptr_t focus)
{
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_FOCUS, focus);
	psy_ui_componentstyle_readstyle(&self->style,
		psy_ui_STYLESTATE_FOCUS, focus);
	psy_ui_component_checkbackgroundanimation(self);
}

void psy_ui_component_setstyletype_active(psy_ui_Component* self,
	uintptr_t active)
{
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_ACTIVE, active);
	psy_ui_componentstyle_readstyle(&self->style,
		psy_ui_STYLESTATE_ACTIVE, active);
	psy_ui_component_checkbackgroundanimation(self);
}

void psy_ui_component_setstyletype_select(psy_ui_Component* self,
	uintptr_t select)
{
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_SELECT, select);
	psy_ui_componentstyle_readstyle(&self->style,
		psy_ui_STYLESTATE_SELECT, select);
	psy_ui_component_checkbackgroundanimation(self);
}

void psy_ui_component_setstylestate(psy_ui_Component* self,
	psy_ui_StyleState state)
{		
	if (psy_ui_componentstyle_setcurrstate(&self->style, state)) {
		psy_ui_component_checkbackgroundanimation(self);
		psy_ui_component_invalidate(self);
	}	
}

void psy_ui_component_addstylestate(psy_ui_Component* self,
	psy_ui_StyleState state)
{
	if (psy_ui_componentstyle_addstate(&self->style, state)) {
		psy_ui_component_checkbackgroundanimation(self);
		psy_ui_component_invalidate(self);
	}
}

void psy_ui_component_addstylestate_children(psy_ui_Component* self,
	psy_ui_StyleState state)
{
	psy_ui_component_traverse_int(self,
		(psy_fp_int)psy_ui_component_addstylestate,
		(uintptr_t)state);
}

void psy_ui_component_removestylestate(psy_ui_Component* self,
	psy_ui_StyleState state)
{
	if (psy_ui_componentstyle_removestate(&self->style, state)) {
		psy_ui_component_invalidate(self);
	}
}

void psy_ui_component_removestylestate_children(psy_ui_Component* self,
	psy_ui_StyleState state)
{
	psy_ui_component_traverse_int(self,
		(psy_fp_int)psy_ui_component_removestylestate,
		(uintptr_t)state);	
}

void psy_ui_notifystyleupdate(psy_ui_Component* main)
{
	if (main) {
		psy_List* p;
		psy_List* q;

		// merge		
		psy_ui_componentstyle_readstyles(&main->style);
		main->vtable->onupdatestyles(main);
		for (p = q = psy_ui_component_children(main, psy_ui_RECURSIVE); p != NULL; psy_list_next(&p)) {
			psy_ui_Component* child;

			child = (psy_ui_Component*)psy_list_entry(p);
			psy_ui_componentstyle_readstyles(&child->style);
			child->vtable->onupdatestyles(child);
			if (child->imp) {
				child->imp->vtable->dev_setbackgroundcolour(child->imp,
					psy_ui_component_backgroundcolour(child));
				psy_ui_component_updatefont(child);
			}

		}
	}
	psy_ui_component_invalidate(main);
}

void psy_ui_component_drawchildren(psy_ui_Component* self, psy_ui_Graphics* g,
	psy_List* children)
{
	if (children) {
		psy_List* p;
		psy_ui_RealRectangle clip;

		clip = g->clip;
		for (p = children; p != NULL; p = p->next) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)psy_list_entry(p);			
			if ((component->imp->vtable->dev_flags(component->imp)
				& psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) ==
				psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) {
				psy_ui_RealRectangle position;
				psy_ui_RealRectangle intersection;

				position = psy_ui_component_position(component);				
				if ((self->alignsorted == psy_ui_ALIGN_TOP && position.bottom < clip.top) ||
					(self->alignsorted == psy_ui_ALIGN_LEFT && position.right < clip.left)) {
					continue;
				}
				intersection = clip;
				if (psy_ui_realrectangle_intersection(&intersection, &position)) {
					psy_ui_RealPoint origin;

					// translate graphics clip and origin
					psy_ui_realrectangle_settopleft(&intersection,
						psy_ui_realpoint_make(
							intersection.left - position.left,
							intersection.top - position.top));
					g->clip = intersection;
					origin = psy_ui_origin(g);
					psy_ui_setorigin(g, psy_ui_realpoint_make(-position.left + origin.x,
						-position.top + origin.y));					
					component->imp->vtable->dev_draw(component->imp, g);
					psy_ui_setorigin(g, psy_ui_realpoint_make(origin.x, origin.y));
					if ((self->alignsorted == psy_ui_ALIGN_TOP && position.top > clip.bottom) ||						
					    (self->alignsorted == psy_ui_ALIGN_LEFT && position.left > clip.right)) {
						break;
					}
				}
			}
		}
		g->clip = clip;		
		psy_ui_resetorigin(g);
	}
}

void psy_ui_component_traverse_int(psy_ui_Component* self,
	psy_fp_int fp, uintptr_t value)
{
	psy_List* p;
	psy_List* q;
	
	for (p = q = psy_ui_component_children(self, psy_ui_NONRECURSIVE);
			p != NULL; p = p->next) {
		fp((psy_ui_Component*)p->entry, value);
	}
	psy_list_free(q);
}

void psy_ui_component_traverse_margin(psy_ui_Component* self,
	psy_fp_margin fp, psy_ui_Margin value)
{
	psy_List* p;
	psy_List* q;

	for (p = q = psy_ui_component_children(self, psy_ui_NONRECURSIVE);
		p != NULL; p = p->next) {
		fp((psy_ui_Component*)p->entry, value);
	}
	psy_list_free(q);
}

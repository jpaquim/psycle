/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uicomponent.h"
/* local */
#include "uiapp.h"
#include "uigridaligner.h"
#include "uiimpfactory.h"
#include "uilclaligner.h"
#include "uiviewcomponentimp.h"
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"

/* todo: includes for SW_MAXIMIZE */
#ifdef DIVERSALIS__OS__MICROSOFT
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objbase.h>
#endif

typedef void (*psy_fp_int)(psy_ui_Component*, uintptr_t);
typedef void (*psy_fp_margin)(psy_ui_Component*, psy_ui_Margin);

static void psy_ui_component_traverse_int(psy_ui_Component*,
	psy_fp_int, uintptr_t value);
static double lines(double pos, double size, double scrollsize, double step,
	bool round, double* rv_max, double* rv_visi);
static void psy_ui_component_draw_children(psy_ui_Component*, psy_ui_Graphics*);
static void psy_ui_component_draw_border(psy_ui_Component*, psy_ui_Graphics*);

static bool componentscroll_initialized = FALSE;
static psy_ui_ComponentScroll componentscroll;

static void enableinput_internal(psy_ui_Component*, int enable, int recursive);
static void psy_ui_component_dispose_signals(psy_ui_Component*);

void psy_ui_component_set_background_colour(psy_ui_Component* self,
	psy_ui_Colour colour)
{			
	psy_ui_componentstyle_setbackgroundcolour(&self->style, colour);	
	psy_ui_component_invalidate(self);
}

const psy_ui_Font* psy_ui_component_font(const psy_ui_Component* self)
{
	const psy_ui_Component* curr;
	const psy_ui_Font* rv;

	assert(self);

	curr = self;
	rv = NULL;
	while (curr) {
		const psy_ui_Font* font;

		font = &psy_ui_componentstyle_currstyle_const(&curr->style)->font;
		if (font->imp) {
			rv = font;
			break;
		}
		curr = psy_ui_component_parent_const(curr);
	}
	if (!rv) {		
		rv = &psy_ui_style_const(psy_ui_STYLE_ROOT)->font;		
	}
	return rv;
}

psy_ui_Colour psy_ui_component_background_colour(psy_ui_Component* self)
{
	psy_ui_Component* curr;
	psy_ui_Colour base;
	
	assert(self);
	
	curr = self;	
	while (curr) {
		psy_ui_Colour colour;

		colour = psy_ui_componentstyle_backgroundcolour(&curr->style);		
		base = colour;
		if (colour.overlay != 0) {
			psy_ui_Colour overlaycolour;
			uint8_t overlay;

			if (psy_ui_app_hasdarktheme(psy_ui_app())) {
				overlaycolour = psy_ui_colour_make(0xFFFFFF);
			} else {
				overlaycolour = psy_ui_colour_make(0x0000000);
			}
			overlay = 0;
			while (curr && (colour.overlay != 0 || colour.mode.transparent)) {				
				overlay += colour.overlay;
				curr = psy_ui_component_parent(curr);
				if (curr) {
					colour = psy_ui_componentstyle_backgroundcolour(&curr->style);
				}
			}				
			if (!curr) {				
				colour = psy_ui_style_const(psy_ui_STYLE_ROOT)->background.colour;
			}
			return psy_ui_colour_overlayed(&colour, &overlaycolour,
				overlay / 100.0);																
		} else if (!colour.mode.inherit) {
			break;
		}
		curr = psy_ui_component_parent(curr);
	}
	if (!curr) {		
		base = psy_ui_style_const(psy_ui_STYLE_ROOT)->background.colour;
	}	
	return base;
}

void psy_ui_component_setcolour(psy_ui_Component* self, psy_ui_Colour colour)
{
	psy_ui_componentstyle_setcolour(&self->style, colour);	
}

psy_ui_Colour psy_ui_component_colour(psy_ui_Component* self)
{
	psy_ui_Component* curr;
	psy_ui_Colour base;

	assert(self);

	curr = self;	
	while (curr) {
		psy_ui_Colour colour;

		colour = psy_ui_componentstyle_colour(&curr->style);
		base = colour;
		if (colour.overlay != 0) {
			psy_ui_Colour overlaycolour;
			uint8_t overlay;

			if (psy_ui_app_hasdarktheme(psy_ui_app())) {
				overlaycolour = psy_ui_colour_make(0xFFFFFF);
			} else {
				overlaycolour = psy_ui_colour_make(0x0000000);
			}
			overlay = 0;
			while (curr && (colour.overlay != 0 || colour.mode.transparent)) {
				overlay += colour.overlay;
				curr = psy_ui_component_parent(curr);
				if (curr) {
					colour = psy_ui_componentstyle_colour(&curr->style);
				}
			}
			if (!curr) {
				colour = psy_ui_style_const(psy_ui_STYLE_ROOT)->colour;
			}
			return psy_ui_colour_overlayed(&colour, &overlaycolour,
				overlay / 100.0);
		} else if (!colour.mode.inherit) {
			break;
		}
		curr = psy_ui_component_parent(curr);
	}
	if (!curr) {
		base = psy_ui_style_const(psy_ui_STYLE_ROOT)->colour;
	}
	return base;
}

void psy_ui_component_setborder(psy_ui_Component* self,
	const psy_ui_Border* border)
{
	psy_ui_componentstyle_setborder(&self->style, border);	
}

const psy_ui_Border* psy_ui_component_border(const psy_ui_Component* self)
{	
	return psy_ui_componentstyle_border(&self->style);	
}

uintptr_t psy_ui_component_backgroundimageid(const psy_ui_Component* self)
{
	return psy_ui_componentstyle_currstyle_const(&self->style)->background.image_id;
}

const char* psy_ui_component_backgroundimagepath(const psy_ui_Component* self)
{
	return psy_ui_componentstyle_currstyle_const(&self->style)->background.image_path;
}

void psy_ui_replacedefaultfont(psy_ui_Component* main,
	const psy_ui_Font* font)
{		
	if (main) {
		psy_ui_Style* root;

		root = (psy_ui_Style*)psy_ui_style_const(psy_ui_STYLE_ROOT);
		psy_ui_font_dispose(&root->font);		
		psy_ui_font_init(&root->font, NULL);
		psy_ui_font_copy(&root->font, font);		
		psy_ui_notify_style_update(main);
	}
}

void psy_ui_component_set_focus(psy_ui_Component* self)
{	
	if (self->tabindex != psy_INDEX_INVALID) {
		psy_ui_app_set_focus(psy_ui_app(), self);
	}
}

void psy_ui_component_capture(psy_ui_Component* self)
{
	self->imp->vtable->dev_capture(self->imp);
}

void psy_ui_component_release_capture(psy_ui_Component* self)
{
	psy_ui_app_setcapture(psy_ui_app(), NULL);
	self->imp->vtable->dev_releasecapture(self->imp);
}

static void psy_ui_component_checkbackgroundanimation(psy_ui_Component*);
static psy_ui_Size psy_ui_component_sub_border(const psy_ui_Component*,
	psy_ui_Size);

/* vtable */
static void dispose(psy_ui_Component*);
static void destroy(psy_ui_Component*);
static void show(psy_ui_Component*);
static void showstate(psy_ui_Component*, int state);
static void hide(psy_ui_Component*);
static int visible(psy_ui_Component*);
static int draw_visible(psy_ui_Component*);
static void move(psy_ui_Component*, psy_ui_Point);
static void resize(psy_ui_Component*, psy_ui_Size);
static void clientresize(psy_ui_Component*, psy_ui_Size);
static void setposition(psy_ui_Component*, psy_ui_Point, psy_ui_Size);
static psy_ui_Size framesize(psy_ui_Component*);
static void scrollto(psy_ui_Component*, intptr_t dx, intptr_t dy,
	const psy_ui_RealRectangle*);
static void setfont(psy_ui_Component*, const psy_ui_Font*);
static psy_List* children(psy_ui_Component*, int recursive);
static void enableinput(psy_ui_Component*);
static void preventinput(psy_ui_Component*);
static void invalidate(psy_ui_Component*);
static uintptr_t section(const psy_ui_Component* self) { return 0; }
static void setalign(psy_ui_Component* self, psy_ui_AlignType align) {  }
/* events */
static void on_destroyed(psy_ui_Component* self) { }
static void onsize(psy_ui_Component* self) { }
static void beforealign(psy_ui_Component* self) { }
static void onalign(psy_ui_Component* self)
{
	psy_List* p;
	psy_List* q;
	
	q = psy_ui_component_children(self, psy_ui_NONE_RECURSIVE);
	for (p = q; p != NULL; p = p->next) {
		psy_ui_Component* component;
		psy_ui_Style* style;

		component = (psy_ui_Component*)p->entry;
		if (component->align == psy_ui_ALIGN_NONE) {
			style = psy_ui_componentstyle_currstyle(&component->style);
			if (psy_ui_position_is_active(&style->position)) {
				psy_ui_component_setposition(component, *style->position.rectangle);
			}
		}
	}
	psy_list_free(q);
	q = NULL;
}

static void onpreferredsize(psy_ui_Component*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void onpreferredscrollsize(psy_ui_Component*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static bool onclose(psy_ui_Component* self) { return TRUE; }

static void on_mouse_down(psy_ui_Component* self, psy_ui_MouseEvent* ev)
{
	assert(ev);

	psy_ui_component_add_style_state(self, psy_ui_STYLESTATE_ACTIVE);
	if (self->draggable) {
		psy_ui_DragEvent* dragevent;

		dragevent = &psy_ui_app()->dragevent;
		psy_ui_event_settarget(&dragevent->mouse.event, self);
		dragevent->mouse = *ev;
		psy_ui_app_startdrag(psy_ui_app());
		self->vtable->ondragstart(self, dragevent);
		psy_signal_emit(&self->signal_dragstart, self, 1, dragevent);
	}
}

static void on_mouse_move(psy_ui_Component* self, psy_ui_MouseEvent* ev)
{
	psy_ui_DragEvent* dragevent;

	assert(ev);	

	dragevent = &psy_ui_app()->dragevent;
	if (dragevent->active) {
		dragevent->mouse.event.default_prevented_ = FALSE;
		dragevent->mouse = *ev;
		self->vtable->ondragover(self, dragevent);
		if (psy_ui_event_default_prevented(&dragevent->mouse.event)) {
			psy_ui_component_setcursor(self, psy_ui_CURSORSTYLE_GRAB);
			psy_ui_mouseevent_stop_propagation(ev);
		}
	}
}

static void onmousewheel(psy_ui_Component* self, psy_ui_MouseEvent* ev) { }

static void on_mouse_up(psy_ui_Component* self, psy_ui_MouseEvent* ev)
{
	psy_ui_DragEvent* dragevent;

	assert(ev);
	

	psy_ui_component_remove_style_state(self, psy_ui_STYLESTATE_ACTIVE);
	dragevent = &psy_ui_app()->dragevent;
	if (dragevent->active) {
		dragevent->mouse = *ev;
		self->vtable->ondrop(self, dragevent);
		if (psy_ui_event_default_prevented(&ev->event)) {
			psy_ui_mouseevent_stop_propagation(ev);
		}
	}
}

static void on_mouse_double_click(psy_ui_Component* self, psy_ui_MouseEvent* ev) { }

static void onmouseenter(psy_ui_Component* self)
{		
	psy_ui_component_add_style_state(self, psy_ui_STYLESTATE_HOVER);
}

static void onmouseleave(psy_ui_Component* self)
{
	psy_ui_component_remove_style_state(self, psy_ui_STYLESTATE_HOVER);
}

static void on_key_down(psy_ui_Component* self, psy_ui_KeyboardEvent* ev) { }
static void onkeyup(psy_ui_Component* self, psy_ui_KeyboardEvent* ev) { }
static void on_timer(psy_ui_Component* self, uintptr_t timerid)
{	
	psy_ui_componentbackground_idle(&self->componentbackground);
}
static void onlanguagechanged(psy_ui_Component* self) { }

static void on_focus(psy_ui_Component* self)
{
	psy_ui_component_add_style_state(self, psy_ui_STYLESTATE_FOCUS);
}

static void on_focuslost(psy_ui_Component* self)
{
	psy_ui_component_remove_style_state(self, psy_ui_STYLESTATE_FOCUS);
	psy_ui_app()->focus = NULL;
}

static void on_focusin(psy_ui_Component* self, psy_ui_Event* ev)
{
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
		vtable.draw_visible = draw_visible;
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
		/* events */		
		vtable.on_destroyed = on_destroyed;
		vtable.ondraw = NULL;
		vtable.beforealign = beforealign;
		vtable.onalign = onalign;
		vtable.onpreferredsize = onpreferredsize;
		vtable.onpreferredscrollsize = onpreferredscrollsize;
		vtable.onsize = onsize;
		vtable.onclose = onclose;
		vtable.on_mouse_down = on_mouse_down;
		vtable.on_mouse_move = on_mouse_move;
		vtable.onmousewheel = onmousewheel;
		vtable.on_mouse_up = on_mouse_up;
		vtable.on_mouse_double_click = on_mouse_double_click;
		vtable.onmouseenter = onmouseenter;
		vtable.onmouseleave = onmouseleave;
		vtable.onkeyup = onkeyup;
		vtable.on_key_down = on_key_down;
		vtable.on_key_down = onkeyup;
		vtable.on_timer = on_timer;
		vtable.onlanguagechanged = onlanguagechanged;
		vtable.enableinput = enableinput;
		vtable.preventinput = preventinput;
		vtable.on_focus = on_focus;
		vtable.on_focuslost = on_focuslost;
		vtable.on_focusin = on_focusin;
		vtable.onupdatestyles = onupdatestyles;
		vtable.invalidate = invalidate;
		vtable.ondragstart = ondragstart;
		vtable.ondragover = ondragover;
		vtable.ondrop = ondrop;		
		vtable_initialized = TRUE;
	}
}

void psy_ui_component_init_imp(psy_ui_Component* self, psy_ui_Component* parent,
	psy_ui_Component* view, psy_ui_ComponentImp* imp)
{
	assert(self);
	assert(self != parent);

	vtable_init();
	self->vtable = &vtable;
	if (view) {
		self->view = view;
	} else if (parent && parent->view) {
		self->view = parent->view;
	} else {
		self->view = NULL;
	}
	self->imp = imp;
	psy_ui_component_init_base(self);
	psy_ui_component_init_signals(self);
	if (parent && parent->aligner && parent->aligner->insertaligntype != psy_ui_ALIGN_NONE) {
		psy_ui_component_set_align(self, parent->aligner->insertaligntype);
		psy_ui_component_set_margin(self, parent->aligner->insertmargin);
	}
}

void psy_ui_component_init(psy_ui_Component* self, psy_ui_Component* parent, psy_ui_Component* view)
{	
	assert(self);
	assert(self != parent);

	vtable_init();
	self->vtable = &vtable;	
	if (view) {
		self->view = view;
	} else if (parent && parent->view) {
		self->view = parent->view;
	} else {
		self->view = NULL;
	}
	if (self->view) {
		self->imp = (psy_ui_ComponentImp*)
			psy_ui_viewcomponentimp_allocinit(
				self, parent, self->view, "",
				0, 0, 100, 100, 0, 0);		
	} else {
		self->imp = psy_ui_impfactory_allocinit_componentimp(
			psy_ui_app_impfactory(psy_ui_app()), self, parent);
	}
	psy_ui_component_init_base(self);
	psy_ui_component_init_signals(self);
	if (parent && parent->aligner && parent->aligner->insertaligntype != psy_ui_ALIGN_NONE) {
		psy_ui_component_set_align(self, parent->aligner->insertaligntype);
		psy_ui_component_set_margin(self, parent->aligner->insertmargin);
	}
	self->imp->vtable->dev_initialized(self->imp);
}

void dispose(psy_ui_Component* self)
{	
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = 0;	
	psy_ui_componentstyle_dispose(&self->style);
	psy_ui_componentbackground_dispose(&self->componentbackground);
	if (psy_ui_app()->focus == self) {
		psy_ui_app()->focus = NULL;
	}
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
	
}

void showstate(psy_ui_Component* self, int state)
{	
	self->imp->vtable->dev_showstate(self->imp, state);
	psy_ui_component_align(self);
}

void hide(psy_ui_Component* self)
{	
}

int visible(psy_ui_Component* self)
{	
	assert(self);
	assert(self->imp);

	return self->imp->vtable->dev_visible(self->imp);
}

int draw_visible(psy_ui_Component* self)
{
	assert(self);	
	assert(self->imp);

	return self->imp->vtable->dev_drawvisible(self->imp);
}

void move(psy_ui_Component* self, psy_ui_Point origin)
{	
	assert(self);
	assert(self->imp);

	self->imp->vtable->dev_move(self->imp, origin);		
}

void resize(psy_ui_Component* self, psy_ui_Size size)
{			
	self->imp->vtable->dev_resize(self->imp, size);	
	if (!psy_ui_app()->alignvalid ||
		((self->imp->vtable->dev_flags && self->imp->vtable->dev_flags(self->imp) & psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) ==
			psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN)) {
		psy_ui_component_align(self);
		if (self->scroll->overflow != psy_ui_OVERFLOW_HIDDEN) {
			psy_ui_component_updateoverflow(self);
		}
	}
}

void clientresize(psy_ui_Component* self, psy_ui_Size size)
{	
	self->imp->vtable->dev_clientresize(self->imp,
		(intptr_t)psy_ui_value_px(&size.width,
			psy_ui_component_textmetric(self), NULL),
		(intptr_t)psy_ui_value_px(&size.height,
			psy_ui_component_textmetric(self), NULL));
}

void setposition(psy_ui_Component* self, psy_ui_Point topleft,
	psy_ui_Size size)
{	
	self->imp->vtable->dev_setposition(self->imp, topleft, size);	
	if (!psy_ui_app()->alignvalid || 
		((self->imp->vtable->dev_flags && self->imp->vtable->dev_flags(self->imp)
			& psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) ==
			psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN)) {		
		self->vtable->onsize(self);
		psy_ui_component_align(self);				
		if (self->scroll->overflow != psy_ui_OVERFLOW_HIDDEN) {
			psy_ui_component_updateoverflow(self);
		}		
		psy_signal_emit(&self->signal_size, self, 0);
	}
}

psy_ui_Size framesize(psy_ui_Component* self)
{
	return self->imp->vtable->dev_framesize(self->imp);
}

void scrollto(psy_ui_Component* self, intptr_t dx, intptr_t dy,
	const psy_ui_RealRectangle* r)
{
	self->imp->vtable->dev_scrollto(self->imp, dx, dy, r);
}

void setfont(psy_ui_Component* self, const psy_ui_Font* font)
{
	if (font) {		
		psy_ui_font_copy(&psy_ui_componentstyle_currstyle(&self->style)->font, font);
	} else {
		psy_ui_font_dispose(&psy_ui_componentstyle_currstyle(&self->style)->font);
	}
	self->imp->vtable->dev_setfont(self->imp, psy_ui_component_font(self));
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
	psy_signal_init(&self->signal_scrolled);
	psy_signal_init(&self->signal_create);
	psy_signal_init(&self->signal_close);
	psy_signal_init(&self->signal_destroyed);	
	psy_signal_init(&self->signal_show);
	psy_signal_init(&self->signal_hide);	
	psy_signal_init(&self->signal_focus);
	psy_signal_init(&self->signal_focuslost);
	psy_signal_init(&self->signal_align);
	psy_signal_init(&self->signal_beforealign);
	psy_signal_init(&self->signal_preferredsizechanged);
	psy_signal_init(&self->signal_command);
	psy_signal_init(&self->signal_selectsection);
	psy_signal_init(&self->signal_scrollrangechanged);
	psy_signal_init(&self->signal_languagechanged);
	psy_signal_init(&self->signal_dragstart);
	psy_signal_init(&self->signal_styleupdate);
}

void psy_ui_component_init_base(psy_ui_Component* self) {
	psy_table_insert(&psy_ui_app()->components,
		(uintptr_t)self, (void*)self);
	self->aligner = (psy_ui_Aligner*)psy_ui_lclaligner_allocinit();
	if (!componentscroll_initialized) {
		psy_ui_componentscroll_init(&componentscroll);
		componentscroll_initialized = TRUE;
	}
	self->scroll = &componentscroll;	
	self->id = psy_INDEX_INVALID;
	self->dbg = psy_INDEX_INVALID;
	psy_ui_componentstyle_init(&self->style);
	psy_ui_componentbackground_init(&self->componentbackground, self);
	self->align = psy_ui_ALIGN_NONE;	
	self->deallocate = FALSE;	
	self->doublebuffered = FALSE;	
	self->tabindex = psy_INDEX_INVALID;
	self->opcount = 0;
	self->draggable = FALSE;
	self->dropdown = FALSE;
	self->ncpaint = FALSE;
	self->blitscroll = FALSE;
	psy_ui_bitmap_init(&self->bufferbitmap);
	self->drawtobuffer = FALSE;
	psy_ui_component_updatefont(self);	
	if (self->imp) {
		self->imp->vtable->dev_setbackgroundcolour(self->imp,
			psy_ui_component_background_colour(self));
	}
}

void psy_ui_component_dispose(psy_ui_Component* self)
{		
	self->vtable->dispose(self);
	psy_signal_emit(&self->signal_destroyed, self, 0);
	self->vtable->on_destroyed(self);
	psy_ui_component_dispose_signals(self);	
	if (psy_ui_app()->hover == self) {
		psy_ui_app_set_hover(psy_ui_app(), NULL);
	}	
	if (self->scroll != &componentscroll) {		
		free(self->scroll);
		self->scroll = NULL;
	}
	if (self->aligner) {
		psy_ui_aligner_dispose(self->aligner);
		free(self->aligner);
		self->aligner = NULL;
	}
	psy_ui_bitmap_dispose(&self->bufferbitmap);
	psy_ui_app_stoptimer(psy_ui_app(), self, psy_INDEX_INVALID);
	psy_table_remove(&psy_ui_app()->components, (uintptr_t)self);	
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
	psy_signal_dispose(&self->signal_scrolled);
	psy_signal_dispose(&self->signal_create);
	psy_signal_dispose(&self->signal_close);
	psy_signal_dispose(&self->signal_destroyed);
	psy_signal_dispose(&self->signal_show);
	psy_signal_dispose(&self->signal_hide);	
	psy_signal_dispose(&self->signal_focus);
	psy_signal_dispose(&self->signal_focuslost);
	psy_signal_dispose(&self->signal_align);	
	psy_signal_dispose(&self->signal_beforealign);
	psy_signal_dispose(&self->signal_command);
	psy_signal_dispose(&self->signal_preferredsizechanged);
	psy_signal_dispose(&self->signal_selectsection);
	psy_signal_dispose(&self->signal_scrollrangechanged);
	psy_signal_dispose(&self->signal_languagechanged);
	psy_signal_dispose(&self->signal_dragstart);
	psy_signal_dispose(&self->signal_styleupdate);
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

void psy_ui_component_deallocate_after_destroyed(psy_ui_Component* self)
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
		psy_ui_component_deallocate_after_destroyed(rv);		
	}
	return rv;
}

void psy_ui_component_activate_resize(psy_ui_Component* self, int recursive)
{		
	if (recursive == psy_ui_RECURSIVE) {
		psy_List* q;
		psy_List* p;

		q = psy_ui_component_children(self, recursive);
		for (p = q; p != NULL; p = p->next) {
			psy_ui_Component* curr;

			curr = (psy_ui_Component*)p->entry;
			psy_ui_component_activate_resize(curr, psy_ui_NONE_RECURSIVE);
		}
		psy_list_free(q);
		q = NULL;
	}
}


void psy_ui_component_scrollstep(psy_ui_Component* self, double stepx,
	double stepy)
{
	if (stepy != 0 || stepx != 0) {		
		self->vtable->scrollto(self,
			(intptr_t)(psy_ui_component_scrollstep_width_px(self) * stepx),
			(intptr_t)(psy_ui_component_scroll_step_height_px(self) * stepy),
			NULL);
	}
}

void psy_ui_component_show(psy_ui_Component* self)
{	
	self->imp->vtable->dev_show(self->imp);
	self->vtable->show(self);
}


void psy_ui_component_show_align(psy_ui_Component* self)
{
	assert(self);

	if (!psy_ui_component_visible(self)) {
		psy_ui_component_show(self);
		if (psy_ui_component_parent(self)) {			
			psy_ui_component_align(psy_ui_component_parent(self));
		}		
		psy_ui_component_invalidate(psy_ui_component_parent(self));
	}
}

void psy_ui_component_hide(psy_ui_Component* self)
{		
	self->imp->vtable->dev_hide(self->imp);
	self->vtable->hide(self);
}

void psy_ui_component_hide_align(psy_ui_Component* self)
{
	assert(self);

	if (psy_ui_component_visible(self)) {
		psy_ui_component_hide(self);		
		if (psy_ui_component_parent(self)) {
			psy_ui_component_align(psy_ui_component_parent(self));
			psy_ui_component_invalidate(psy_ui_component_parent(self));
		}
	}
}

void psy_ui_component_showstate(psy_ui_Component* self, int state)
{
	self->vtable->showstate(self, state);	
}

void psy_ui_component_showmaximized(psy_ui_Component* self)
{
#ifdef DIVERSALIS__OS__MICROSOFT	
	psy_ui_component_showstate(self, SW_MAXIMIZE);
#else	
	psy_ui_component_showstate(self, 1);	
#endif	
}

void psy_ui_component_togglefullscreen(psy_ui_Component* self)
{
#ifdef DIVERSALIS__OS__MICROSOFT	
	psy_ui_Component* curr_focus;
	psy_ui_ComponentState state;

	curr_focus = psy_ui_app_focus(psy_ui_app());
	state = psy_ui_component_state(self);
	if (state == psy_ui_COMPONENTSTATE_FULLSCREEN) {
		psy_ui_component_set_state(self, psy_ui_COMPONENTSTATE_NORMAL);
	} else {
		psy_ui_component_set_state(self, psy_ui_COMPONENTSTATE_FULLSCREEN);
	}
	if (curr_focus) {		
		psy_ui_component_set_focus(curr_focus);
	}
#else
	psy_ui_component_resize(self,
		psy_ui_size_make_px(1024.0, 768.0));
	psy_ui_component_showstate(self, 0);
#endif	
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

void psy_ui_component_applyposition(psy_ui_Component* component, bool children)
{	
	psy_List* p;
	psy_List* q;
		
	if (!children) {
		component->imp->vtable->dev_applyposition(component->imp);
	}
	for (p = q = psy_ui_component_children(component, psy_ui_RECURSIVE); p != NULL; psy_list_next(&p)) {
		psy_ui_Component* child;

		child = (psy_ui_Component*)psy_list_entry(p);		
		child->imp->vtable->dev_applyposition(child->imp);		
	}
	psy_list_free(q);	
}

psy_List* psy_ui_component_children(psy_ui_Component* self, int recursive)
{	
	return self->vtable->children(self, recursive);	
}

void psy_ui_component_set_font(psy_ui_Component* self, const psy_ui_Font* source)
{	
	self->vtable->setfont(self, source);
}

void psy_ui_component_set_font_info(psy_ui_Component* self,
	psy_ui_FontInfo fontinfo)
{
	psy_ui_Font font;

	psy_ui_font_init(&font, &fontinfo);
	psy_ui_component_set_font(self, &font);	
	psy_ui_font_dispose(&font);	
}

int psy_ui_component_visible(psy_ui_Component* self)
{
	return self->vtable->visible(self);	
}

bool psy_ui_component_draw_visible(psy_ui_Component* self)
{
	return self->vtable->draw_visible(self);
}

void psy_ui_component_align(psy_ui_Component* self)
{	
	if (self->aligner) {
		self->vtable->beforealign(self);
		psy_signal_emit(&self->signal_beforealign, self, 0);
		psy_ui_aligner_align(self->aligner, self);
		psy_signal_emit(&self->signal_align, self, 0);
		self->vtable->onalign(self);		
	}	
}

void psy_ui_component_align_full(psy_ui_Component* self)
{
	if (self->aligner) {
		psy_ui_app()->alignvalid = FALSE;
		psy_ui_component_align(self);
		psy_ui_app()->alignvalid = TRUE;
	}
}

void onpreferredsize(psy_ui_Component* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	if (self->aligner) {
		psy_ui_aligner_preferred_size(self->aligner, self, limit, rv);
	}
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

uintptr_t psy_ui_component_index(psy_ui_Component* self)
{
	uintptr_t rv;
	uintptr_t i;
	psy_List* p;
	psy_List* q;

	rv = psy_INDEX_INVALID;
	q = psy_ui_component_children(psy_ui_component_parent(self),
		psy_ui_NONE_RECURSIVE);
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

void psy_ui_component_check_sorted_align(psy_ui_Component* self,
	psy_ui_AlignType align)
{
	if (self->aligner && self->aligner->alignsorted != align) {
		psy_List* p;
		psy_List* q;

		q = psy_ui_component_children(self, psy_ui_NONE_RECURSIVE);
		for (p = q; p != NULL; p = p->next) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)p->entry;
			if (component->align != align) {
				self->aligner->alignsorted = psy_ui_ALIGN_NONE;
				break;
			}
		}
		if (p == NULL) {
			self->aligner->alignsorted = align;
		}
		psy_list_free(q);
	}
}

psy_ui_Component* psy_ui_component_set_align(psy_ui_Component* self,
	psy_ui_AlignType align)
{
	if (self->align != align) {
		psy_ui_Component* parent;

		self->align = align;
		if (parent = psy_ui_component_parent(self)) {
			psy_ui_component_check_sorted_align(parent, align);
		}
		self->vtable->setalign(self, align);
	}
	return self;
}

void psy_ui_component_set_aligner(psy_ui_Component* self, 
	psy_ui_Aligner* aligner)
{	
	if (self->aligner) {
		psy_ui_aligner_dispose(self->aligner);
		free(self->aligner);
	}
	self->aligner = aligner;	
}

void psy_ui_component_set_align_expand(psy_ui_Component* self, psy_ui_ExpandMode mode)
{
	if (self->aligner) {
		self->aligner->alignexpandmode = mode;
	}
}

void psy_ui_component_enable_input(psy_ui_Component* self, int recursive)
{
	if (psy_ui_component_input_prevented(self)) {
		enableinput_internal(self, TRUE, recursive);		
		psy_ui_component_invalidate(self);
	}
}

void psy_ui_component_prevent_input(psy_ui_Component* self, int recursive)
{
	if (!psy_ui_component_input_prevented(self)) {
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
	psy_ui_component_remove_style_state(self, psy_ui_STYLESTATE_DISABLED);
}

static void preventinput(psy_ui_Component* self)
{
	self->imp->vtable->dev_preventinput(self->imp);
	psy_ui_component_add_style_state(self, psy_ui_STYLESTATE_DISABLED);	
}

bool psy_ui_component_input_prevented(const psy_ui_Component* self)
{	
	assert(self->imp);
	
	return self->imp->vtable->dev_inputprevented(self->imp);	
}

psy_List* psy_ui_components_setalign(psy_List* list, psy_ui_AlignType align,
	psy_ui_Margin margin)
{
	psy_List* p;

	for (p = list; p != NULL; p = p->next) {
		psy_ui_component_set_align((psy_ui_Component*) p->entry, align);		
		psy_ui_component_set_margin((psy_ui_Component*) p->entry, margin);		
	}
	return list;
}

psy_List* psy_ui_components_setmargin(psy_List* list, psy_ui_Margin margin)
{
	psy_List* p;

	for (p = list; p != NULL; p = p->next) {
		psy_ui_component_set_margin((psy_ui_Component*)p->entry, margin);
	}
	return list;
}

void psy_ui_component_set_preferred_size(psy_ui_Component* self, psy_ui_Size size)
{
	psy_ui_componentstyle_setpreferredsize(&self->style, size);	
}

void psy_ui_component_set_preferred_height(psy_ui_Component* self, psy_ui_Value height)
{
	psy_ui_componentstyle_setpreferredheight(&self->style, height);	
}

void psy_ui_component_set_preferred_width(psy_ui_Component* self, psy_ui_Value width)
{
	psy_ui_componentstyle_setpreferredwidth(&self->style, width);
}

psy_ui_Size psy_ui_component_preferred_size(psy_ui_Component* self,
	const psy_ui_Size* limit)
{		
	psy_ui_Size rv;
	psy_ui_Size preferredsize;
	psy_ui_Size parentsize;
	psy_ui_Size* pparentsize;
	psy_ui_Margin padding;
	const psy_ui_TextMetric* tm;
	
	rv = preferredsize = psy_ui_componentstyle_preferredsize(&self->style);	
	padding = psy_ui_component_padding(self);
	tm = psy_ui_component_textmetric(self);
	if (preferredsize.width.set && preferredsize.height.set) {
		rv = preferredsize;
	} else {
		rv.width.set = TRUE;
		rv.height.set = TRUE;
		self->vtable->onpreferredsize(self, limit, &rv);
		if (preferredsize.width.set) {
			rv.width = self->style.sizehints->preferredsize.width;
		}
		if (preferredsize.height.set) {
			rv.height = self->style.sizehints->preferredsize.height;
		}		
	}
	if (rv.height.unit == psy_ui_UNIT_PH ||
		rv.width.unit == psy_ui_UNIT_PW) {
		parentsize = psy_ui_component_parent_size(self);
		pparentsize = &parentsize;
	} else {
		pparentsize = NULL;
	}	
	rv.height = psy_ui_add_values(rv.height, psy_ui_margin_height(
		&padding, tm, pparentsize), tm, pparentsize);	
	rv.width = psy_ui_add_values(rv.width, psy_ui_margin_width(
		&padding, tm, pparentsize), tm, pparentsize);
	return rv;	
}

psy_ui_Size psy_ui_component_frame_size(psy_ui_Component* self)
{
	return self->vtable->framesize(self);
}

psy_ui_Size psy_ui_component_preferred_scrollsize(psy_ui_Component* self,
	const psy_ui_Size* limit)
{
	return psy_ui_component_preferred_size(self, limit);	
}

psy_ui_RealSize psy_ui_component_preferredscrollsize_px(psy_ui_Component* self,
	const psy_ui_Size* limit)
{	
	psy_ui_Size size;
	
	size = psy_ui_component_preferred_scrollsize(self, limit);
	return psy_ui_size_px(&size, psy_ui_component_textmetric(self), NULL);
}

void psy_ui_component_setmaximumsize(psy_ui_Component* self, psy_ui_Size size)
{
	psy_ui_componentstyle_setmaximumsize(&self->style, size);
}

const psy_ui_Size psy_ui_component_maximumsize(const psy_ui_Component* self)
{
	return psy_ui_componentstyle_maximumsize(&self->style);
}

void psy_ui_component_set_minimum_size(psy_ui_Component* self, psy_ui_Size size)
{
	psy_ui_componentstyle_setminimumsize(&self->style, size);
}

const psy_ui_Size psy_ui_component_minimumsize(const psy_ui_Component* self)
{
	return psy_ui_componentstyle_minimumsize(&self->style);
}

psy_ui_Size psy_ui_component_size(const psy_ui_Component* self)
{
	psy_ui_Size rv;
	const psy_ui_TextMetric* tm;
	psy_ui_Margin spacing;
	psy_ui_Value spacing_width;
	psy_ui_Value spacing_height;


	rv = psy_ui_component_scroll_size(self);
	spacing = psy_ui_component_padding(self);
	tm = psy_ui_component_textmetric(self);
	spacing_width = psy_ui_margin_width(&spacing, tm, NULL);
	psy_ui_value_sub(&rv.width, &spacing_width, tm, NULL);
	spacing_height = psy_ui_margin_height(&spacing, tm, NULL);
	psy_ui_value_sub(&rv.height, &spacing_height, tm, NULL);
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
	psy_ui_value_sub(&rv.width, &bordersize.width, tm, NULL);
	psy_ui_value_sub(&rv.height, &bordersize.height, tm, NULL);
	return rv;
}

psy_ui_Size psy_ui_component_client_size(const psy_ui_Component* self)
{			
	psy_ui_Size rv;
	psy_ui_RealRectangle parent_position;
	psy_ui_RealRectangle position;
	psy_ui_RealSize parent_size;

	parent_size = psy_ui_component_scroll_size_px(
		psy_ui_component_parent_const(self));
	parent_position = psy_ui_realrectangle_make(
		psy_ui_realpoint_zero(),
		parent_size);
	position = psy_ui_component_position(self);
	psy_ui_realrectangle_intersection(&position, &parent_position);			
	rv.width = psy_ui_value_make_px(psy_ui_realrectangle_width(&position));
	rv.height = psy_ui_value_make_px(psy_ui_realrectangle_height(&position));		
	return rv;	
}

void psy_ui_component_seticonressource(psy_ui_Component* self, int ressourceid)
{
	self->imp->vtable->dev_seticonressource(self->imp, ressourceid);
}

void psy_ui_component_clientresize(psy_ui_Component* self, psy_ui_Size size)
{
	self->vtable->clientresize(self, size);
}

psy_ui_Size psy_ui_component_scroll_size(const psy_ui_Component* self)
{
	return self->imp->vtable->dev_size(self->imp);
}

psy_ui_Size psy_ui_component_offsetsize(const psy_ui_Component* self)
{
	return self->imp->vtable->dev_size(
		psy_ui_component_parent_const(self)->imp);	
}

void psy_ui_component_clear(psy_ui_Component* self)
{
	psy_List* c;
	psy_List* p;
	psy_List* q;

	c = psy_ui_component_children(self, psy_ui_NONE_RECURSIVE);
	for (p = c; p != NULL; p = q) {
		psy_ui_Component* component;
		bool deallocate;

		q = p->next;
		component = (psy_ui_Component*)psy_list_entry(p);
		deallocate = component->deallocate;
		psy_ui_component_destroy(component);
		if (deallocate) {
			free(component);
		}
	}
	psy_list_free(c);
	c = NULL;
}

void psy_ui_component_setcursor(psy_ui_Component* self, psy_ui_CursorStyle style)
{
	self->imp->vtable->dev_setcursor(self->imp, style);
}

static psy_ui_TextMetric default_tm;
static bool default_tm_initialized = FALSE;

/* psy_ui_ComponentImp vtable */
static void dev_dispose(psy_ui_ComponentImp* self) { }
static void dev_destroy(psy_ui_ComponentImp* self) { }
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
static void dev_applyposition(psy_ui_ComponentImp* self) { }
static psy_ui_Size dev_framesize(psy_ui_ComponentImp* self) { return psy_ui_size_zero(); }
static void dev_scrollto(psy_ui_ComponentImp* self, intptr_t dx, intptr_t dy,
	const psy_ui_RealRectangle* r) { }
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

static void dev_setbackgroundcolour(psy_ui_ComponentImp* self, psy_ui_Colour colour) { }
static void dev_settitle(psy_ui_ComponentImp* self, const char* title) { }
static const char* dev_title(const psy_ui_ComponentImp* self) { return ""; }
static void dev_setfocus(psy_ui_ComponentImp* self) { }
static int dev_hasfocus(psy_ui_ComponentImp* self) { return 0;  }
static bool dev_issystem(psy_ui_ComponentImp* self) { return FALSE; }
static void* dev_platform(psy_ui_ComponentImp* self) { return (void*) self; }
static uintptr_t dev_platform_handle(psy_ui_ComponentImp* self) { return psy_INDEX_INVALID; }
static uintptr_t dev_flags(const psy_ui_ComponentImp* self) { return 0; }
static void dev_initialized(psy_ui_ComponentImp* self) { }

static psy_ui_ComponentState dev_component_state(const psy_ui_ComponentImp* self)
{
	return psy_ui_COMPONENTSTATE_NORMAL;
}
static void dev_set_component_state(psy_ui_ComponentImp* self,
	psy_ui_ComponentState state)
{
}

static psy_ui_ComponentImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = dev_dispose;
		imp_vtable.dev_destroy = dev_destroy;
		imp_vtable.dev_show = dev_show;
		imp_vtable.dev_showstate = dev_showstate;
		imp_vtable.dev_hide = dev_hide;
		imp_vtable.dev_visible = dev_visible;
		imp_vtable.dev_drawvisible = dev_drawvisible;
		imp_vtable.dev_move = dev_move;
		imp_vtable.dev_resize = dev_resize;
		imp_vtable.dev_clientresize = dev_clientresize;
		imp_vtable.dev_position = dev_position;
		imp_vtable.dev_restore_position = dev_position;
		imp_vtable.dev_screenposition = dev_position;
		imp_vtable.dev_setposition = dev_setposition;
		imp_vtable.dev_size = dev_size;
		imp_vtable.dev_preferredsize = dev_preferredsize;
		imp_vtable.dev_updatesize = dev_updatesize;
		imp_vtable.dev_applyposition = dev_applyposition;
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
		imp_vtable.dev_seticonressource = dev_seticonressource;
		imp_vtable.dev_textmetric = dev_textmetric;		
		imp_vtable.dev_setbackgroundcolour = dev_setbackgroundcolour;
		imp_vtable.dev_settitle = dev_settitle;
		imp_vtable.dev_title = dev_title;
		imp_vtable.dev_setfocus = dev_setfocus;
		imp_vtable.dev_hasfocus = dev_hasfocus;
		imp_vtable.dev_issystem = dev_issystem;
		imp_vtable.dev_platform = dev_platform;
		imp_vtable.dev_platform_handle = dev_platform_handle;
		imp_vtable.dev_flags = dev_flags;		
		imp_vtable.dev_initialized = dev_initialized;
		imp_vtable.dev_component_state = dev_component_state;
		imp_vtable.dev_set_component_state = dev_set_component_state;
		imp_vtable_initialized = TRUE;
	}
}

void psy_ui_componentimp_init(psy_ui_ComponentImp* self)
{
	imp_vtable_init();
	self->vtable = &imp_vtable;
	psy_signal_init(&self->signal_command);
	self->extended_vtable = NULL;
	self->extended_imp = NULL;
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

psy_ui_Component* psy_ui_component_intersect(psy_ui_Component* self, psy_ui_RealPoint pt, uintptr_t* index)
{
	psy_ui_Component* rv = 0;
	psy_List* p;
	psy_List* q;
	uintptr_t c = 0;

	p = q = psy_ui_component_children(self, 0);
	while (p) {
		psy_ui_RealRectangle position;
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
		if (psy_ui_component_visible(component)) {
			position = psy_ui_component_position(component);
			if (psy_ui_realrectangle_intersect(&position, pt)) {
				rv = (psy_ui_Component*)p->entry;
				break;
			}			
		}
		++c;
		p = p->next;
	}
	psy_list_free(q);
	if (rv) {
		*index = c;
	} else {
		*index = psy_INDEX_INVALID;
	}
	return rv;
}

void psy_ui_component_set_scroll(psy_ui_Component* self,
	psy_ui_Point position)
{		
	psy_ui_component_set_scroll_left(self, position.x);
	psy_ui_component_set_scroll_top(self, position.y);
}

void psy_ui_component_set_scroll_left(psy_ui_Component* self, psy_ui_Value left)
{		
	psy_ui_RealRectangle position;
	double newleft;
	psy_ui_RealRectangle r;

	position = psy_ui_component_position(self);
	newleft = -psy_ui_value_px(&left, psy_ui_component_textmetric(self), NULL);
	if (newleft == position.left) {
		return;
	}
	psy_ui_component_move(self,
		psy_ui_point_make(
			psy_ui_value_make_px(
				-psy_ui_value_px(&left, psy_ui_component_textmetric(self), NULL)),
			psy_ui_value_make_px(position.top)));
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			-newleft,
			psy_ui_component_scroll_top_px(self)),
		psy_ui_component_scroll_size_px(psy_ui_component_parent(self)));
	if (!self->blitscroll) {
		psy_ui_component_invalidate(self);
	} else {		
		psy_ui_component_scroll_to(self, newleft - position.left, 0.0, &r);
	}
	if (newleft - position.left < 0) {
		double delta;

		delta = psy_ui_realrectangle_width(&r) - (newleft + psy_ui_realrectangle_width(&position));
		if (delta > 0) {			
			psy_ui_component_invalidate_rect(psy_ui_component_parent(self),
				psy_ui_realrectangle_make(
					psy_ui_realpoint_make(psy_ui_realrectangle_width(&r) - delta, 0.0),
					psy_ui_realsize_make(delta, psy_ui_realrectangle_height(&r))));
		}
	}
	psy_signal_emit(&self->signal_scrolled, self, 0);	
}

void psy_ui_component_set_scroll_top(psy_ui_Component* self, psy_ui_Value top)
{	
	psy_ui_RealRectangle position;	
	double newtop;
	psy_ui_RealRectangle r;
	psy_ui_IntPoint range;

	position = psy_ui_component_position(self);		
	newtop = -psy_ui_value_px(&top, psy_ui_component_textmetric(self), NULL);
	if (newtop == position.top) {
		return;
	}
	if (psy_ui_componentscroll_has_vertical_overflow(self->scroll)) {
		psy_ui_IntPoint range;
		double miny;
		double maxy;

		range = psy_ui_component_verticalscrollrange(self);
		miny = range.x * psy_ui_component_scroll_step_height_px(self);
		maxy = range.y * psy_ui_component_scroll_step_height_px(self);
		if (-newtop > maxy) {
			newtop = -maxy;
		}
	}
	psy_ui_component_move(self, psy_ui_point_make_px(position.left, newtop));
	psy_signal_emit(&self->signal_scroll, self, 0);
	if (!psy_ui_component_draw_visible(self)) {
		psy_signal_emit(&self->signal_scrolled, self, 0);
		return;
	}
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			psy_ui_component_scroll_left_px(self),
			-newtop),
		psy_ui_component_scroll_size_px(psy_ui_component_parent(self)));
	if (!self->blitscroll) {
		psy_ui_component_invalidate(self);
	} else {
		psy_ui_component_scroll_to(self, 0.0, newtop - position.top, &r);		
	}
	range = psy_ui_component_verticalscrollrange(self);
	if (newtop > 0) {
		psy_ui_component_invalidate_rect(psy_ui_component_parent(self),
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(0.0, 0.0),
				psy_ui_realsize_make(psy_ui_realrectangle_width(&r), newtop)));
	} else if (newtop - position.top < 0) {
		double delta;

		delta = psy_ui_realrectangle_height(&r) - (newtop + psy_ui_realrectangle_height(&position));
		if (delta > 0) {			
			psy_ui_component_invalidate_rect(psy_ui_component_parent(self), psy_ui_realrectangle_make(
				psy_ui_realpoint_make(0.0, psy_ui_realrectangle_height(&r) - delta),
				psy_ui_realsize_make(psy_ui_realrectangle_width(&r), delta)));
		}
	}
	psy_signal_emit(&self->signal_scrolled, self, 0);
}

void psy_ui_component_updateoverflow(psy_ui_Component* self)
{	
	if (psy_ui_componentscroll_has_vertical_overflow(self->scroll)) {
		psy_ui_Size scrollsize;
		const psy_ui_TextMetric* tm;
		double scrollmax;
		double visi;
		double curr;
		psy_ui_Size size;		
		double scrollstepy_px;		
		
		size = psy_ui_component_scroll_size(psy_ui_component_parent(self));
		scrollsize = psy_ui_component_preferred_scrollsize(self, &size);
		scrollstepy_px = psy_ui_component_scroll_step_height_px(self);
		tm = psy_ui_component_textmetric(self);
		curr = lines(
			psy_ui_component_scroll_top_px(self),
			psy_ui_value_px(&size.height, tm, NULL),
			psy_ui_value_px(&scrollsize.height, tm, NULL),
			scrollstepy_px,
			TRUE, /* round */
			&scrollmax,
			&visi);
		if (psy_ui_componentscroll_has_vertical_center_overflow(self->scroll)) {
			psy_ui_component_setverticalscrollrange(self,
				psy_ui_intpoint_make((intptr_t)(-visi / 2),
				(intptr_t)(scrollmax - visi / 2 - 1)));
			if (curr > scrollmax - visi / 2) {
				curr = psy_max(-visi / 2, scrollmax -visi / 2);
				psy_ui_component_set_scroll_top(self,
					psy_ui_value_make_px((double)(curr * scrollstepy_px)));
			}
		} else {			
			psy_ui_component_setverticalscrollrange(self,
				psy_ui_intpoint_make(0, (intptr_t)(scrollmax - visi)));
			if (curr > scrollmax - visi) {
				curr = psy_max(0, scrollmax - visi);
				psy_ui_component_set_scroll_top(self,
					psy_ui_value_make_px(curr * scrollstepy_px));
			}
		}		
	}
	if (psy_ui_componentscroll_has_horizontal_overflow(self->scroll)) {
		psy_ui_Size scrollsize;
		const psy_ui_TextMetric* tm;		
		double scrollmax;
		double visi;
		double currrow;
		psy_ui_Size size;
		double scrollstepx_px;		
		
		size = psy_ui_component_scroll_size(psy_ui_component_parent(self));
		scrollsize = psy_ui_component_preferred_scrollsize(self, &size);
		scrollstepx_px = floor(psy_ui_component_scrollstep_width_px(self));
		tm = psy_ui_component_textmetric(self);
		currrow = lines(
			psy_ui_component_scroll_left_px(self),
			psy_ui_value_px(&size.width, tm, NULL),
			psy_ui_value_px(&scrollsize.width, tm, NULL),
			scrollstepx_px,
			FALSE, /* no round */
			&scrollmax,
			&visi);		
		psy_ui_component_sethorizontalscrollrange(self,
			psy_ui_intpoint_make(0, (intptr_t)(scrollmax - visi)));
		if (currrow > scrollmax - visi) {
			currrow = psy_max(0, scrollmax - visi);
			psy_ui_component_set_scroll_left(self,
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

void psy_ui_component_draw_border(psy_ui_Component* self, psy_ui_Graphics* g)
{	
	psy_ui_RealSize size;

	size = psy_ui_component_scroll_size_px(self);	
	psy_ui_drawborder(g,
		psy_ui_realrectangle_make(psy_ui_realpoint_zero(), size),
		psy_ui_component_border(self), psy_ui_component_textmetric(self));
}

void psy_ui_component_usescroll(psy_ui_Component* self)
{
	if (self->scroll == &componentscroll) {
		self->scroll = psy_ui_componentscroll_allocinit();
	}
}

bool psy_ui_component_toggle_visibility(psy_ui_Component* self)
{
	assert(self);
	if (psy_ui_component_parent(self)) {
		if (psy_ui_component_visible(self)) {
			psy_ui_component_hide(self);
			psy_ui_component_align(psy_ui_component_parent(self));
			psy_ui_component_invalidate(psy_ui_component_parent(self));
			return FALSE;
		} else {			
			psy_ui_component_show(self);
			psy_ui_component_align(psy_ui_component_parent(self));			
			psy_ui_component_invalidate(psy_ui_component_parent(self));
			return TRUE;
		}		
	}
	return FALSE;
}

psy_ui_ComponentState psy_ui_component_state(const psy_ui_Component* self)
{
	assert(self->imp);

	return self->imp->vtable->dev_component_state(self->imp);
}

void psy_ui_component_set_state(psy_ui_Component* self, psy_ui_ComponentState state)
{
	assert(self->imp);

	self->imp->vtable->dev_set_component_state(self->imp, state);
}

psy_ui_RealRectangle psy_ui_component_scrolledposition(psy_ui_Component* self)
{	
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			psy_ui_component_scroll_left_px(self),
			psy_ui_component_scroll_top_px(self)),
		psy_ui_component_scroll_size_px(self));
}

void psy_ui_component_focus_next(psy_ui_Component* self)
{
	if (self->tabindex != -1) {
		psy_ui_Component* parent;		
		
		parent = psy_ui_component_parent(self);
		if (parent) {
			psy_ui_Component* focus;
			psy_List* p;
			psy_List* q;
			uintptr_t tabindex;

			tabindex = psy_ui_component_tab_index(self);			
			focus = NULL;
			for (q = p = psy_ui_component_children(parent, 0); p != NULL;
					psy_list_next(&p)) {
				psy_ui_Component* component;

				component = (psy_ui_Component*)psy_list_entry(p);
				if (tabindex < psy_ui_component_tab_index(component)) {
					focus = component;
					tabindex = psy_ui_component_tab_index(focus);
				}
			}
			psy_list_free(q);
			if (focus) {
				psy_ui_component_set_focus(focus);
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

			tabindex = psy_ui_component_tab_index(self);
			focus = NULL;
			for (q = p = psy_ui_component_children(parent, 0); p != NULL;
					psy_list_next(&p)) {
				psy_ui_Component* component;

				component = (psy_ui_Component*)psy_list_entry(p);				
				if (psy_ui_component_tab_index(component) != psy_INDEX_INVALID &&
						tabindex > psy_ui_component_tab_index(component)) {
					focus = component;
					tabindex = psy_ui_component_tab_index(focus);
				}
			}
			psy_list_free(q);
			if (focus) {
				psy_ui_component_set_focus(focus);
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

void psy_ui_component_set_default_align(psy_ui_Component* self,
	psy_ui_AlignType aligntype, psy_ui_Margin margin)
{
	if (self->aligner) {
		self->aligner->insertaligntype = aligntype;
		self->aligner->insertmargin = margin;
	}
}

void psy_ui_component_update_language(psy_ui_Component* self)
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

void psy_ui_component_set_style_types(psy_ui_Component* self,
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
	psy_ui_component_checkbackgroundanimation(self);
}

void psy_ui_component_checkbackgroundanimation(psy_ui_Component* self)
{	
	if (psy_ui_componentstyle_currstyle_const(&self->style)->background.animation.enabled) {
		psy_ui_component_start_timer(self, 65535, 50);
	} else {		
		psy_ui_component_stop_timer(self, 65535);
	}
}

void psy_ui_component_set_style_type(psy_ui_Component* self,
	uintptr_t standard)
{
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_NONE, standard);	
	psy_ui_component_checkbackgroundanimation(self);
}

void psy_ui_component_set_style_type_hover(psy_ui_Component* self,
	uintptr_t hover)
{
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_HOVER, hover);	
	psy_ui_component_checkbackgroundanimation(self);
}

void psy_ui_component_set_style_type_focus(psy_ui_Component* self,
	uintptr_t focus)
{
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_FOCUS, focus);	
	psy_ui_component_checkbackgroundanimation(self);
}

void psy_ui_component_set_style_type_active(psy_ui_Component* self,
	uintptr_t active)
{
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_ACTIVE, active);	
	psy_ui_component_checkbackgroundanimation(self);
}

void psy_ui_component_set_style_type_select(psy_ui_Component* self,
	uintptr_t select)
{
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_SELECT, select);	
	psy_ui_component_checkbackgroundanimation(self);
}

void psy_ui_component_set_style_type_disabled(psy_ui_Component* self,
	uintptr_t disabled)
{
	psy_ui_componentstyle_setstyle(&self->style,
		psy_ui_STYLESTATE_DISABLED, disabled);
	psy_ui_component_checkbackgroundanimation(self);
}

void psy_ui_component_set_style_state(psy_ui_Component* self,
	psy_ui_StyleState state)
{		
	if (psy_ui_componentstyle_setcurrstate(&self->style, state)) {
		psy_ui_component_checkbackgroundanimation(self);
		psy_ui_component_invalidate(self);
	}	
}

void psy_ui_component_add_style_state(psy_ui_Component* self,
	psy_ui_StyleState state)
{	
	if (psy_ui_componentstyle_addstate(&self->style, state)) {
		psy_ui_component_checkbackgroundanimation(self);
		psy_ui_component_invalidate(self);
	}
}

bool psy_ui_component_stylestate_active(const psy_ui_Component* self,
	psy_ui_StyleState state)
{
	return (self->style.states & state) == state;
}

void psy_ui_component_addstylestate_children(psy_ui_Component* self,
	psy_ui_StyleState state)
{
	psy_ui_component_traverse_int(self,
		(psy_fp_int)psy_ui_component_add_style_state,
		(uintptr_t)state);
}

void psy_ui_component_remove_style_state(psy_ui_Component* self,
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
		(psy_fp_int)psy_ui_component_remove_style_state,
		(uintptr_t)state);	
}

void psy_ui_notify_style_update(psy_ui_Component* main)
{
	if (main) {
		psy_List* p;
		psy_List* q;

		psy_ui_componentstyle_updatecurrstate(&main->style);
		main->vtable->onupdatestyles(main);
		psy_signal_emit(&main->signal_styleupdate, main, 0);
		for (p = q = psy_ui_component_children(main, psy_ui_RECURSIVE); p != NULL; p = p->next) {
			psy_ui_Component* child;

			child = (psy_ui_Component*)psy_list_entry(p);			
			psy_ui_componentstyle_updatecurrstate(&child->style);
			child->vtable->onupdatestyles(child);
			psy_signal_emit(&child->signal_styleupdate, child, 0);
			if (child->imp) {
				child->imp->vtable->dev_setbackgroundcolour(child->imp,
					psy_ui_component_background_colour(child));
				psy_ui_component_updatefont(child);
			}
		}
		psy_list_free(q);
	}
	/* align */
	psy_ui_component_align_full(main);	
	psy_ui_component_invalidate(main);	
}

void psy_ui_component_draw(psy_ui_Component* self, psy_ui_Graphics* g)
{	
	psy_ui_Graphics bitmap_g;
	psy_ui_Graphics* temp_g;	
	psy_ui_RealRectangle oldclip;
	psy_ui_RealRectangle clip;
	psy_ui_RealSize size;
			
	temp_g = NULL;	
	oldclip = psy_ui_graphics_cliprect(g);
	if (self->drawtobuffer) {
		if (psy_ui_bitmap_empty(&self->bufferbitmap)) {
			psy_ui_RealSize size;

			psy_ui_bitmap_dispose(&self->bufferbitmap);
			size = psy_ui_component_scroll_size_px(self);
			psy_ui_bitmap_init_size(&self->bufferbitmap, size);
			psy_ui_graphics_init_bitmap(&bitmap_g, &self->bufferbitmap);			
			temp_g = g;
			g = &bitmap_g;
		} else {			
			psy_ui_drawfullbitmap(g, &self->bufferbitmap,
				psy_ui_realpoint_zero());			
			return;
		}
	}		
	psy_ui_setfont(g, psy_ui_component_font(self));
	size = psy_ui_component_scroll_size_px(self);
	clip = psy_ui_realrectangle_make(psy_ui_realpoint_zero(), size);
	if ((oldclip.bottom - oldclip.top != 0.0)) {
		psy_ui_realrectangle_intersection(&clip, &oldclip);
	}
	psy_ui_graphics_set_clip_rect(g, clip);
	/* draw background */		
	psy_ui_componentbackground_draw(&self->componentbackground, g);		
	psy_ui_component_draw_border(self, g);
	/* prepare colours */
	psy_ui_setcolour(g, psy_ui_component_colour(self));
	psy_ui_set_text_colour(g, psy_ui_component_colour(self));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	if (self->vtable->ondraw) {
		psy_ui_Margin padding;
		const psy_ui_TextMetric* tm;
		psy_ui_RealPoint origin;

		origin = psy_ui_origin(g);
		/* spacing */
		padding = psy_ui_component_padding(self);
		if (!psy_ui_margin_iszero(&padding)) {
			tm = psy_ui_component_textmetric(self);			
			psy_ui_setorigin(g, psy_ui_realpoint_make(
				origin.x - (int)psy_ui_value_px(&padding.left, tm, NULL),
				origin.y - (int)psy_ui_value_px(&padding.top, tm, NULL)));
		}
		self->vtable->ondraw(self, g);
		psy_signal_emit(&self->signal_draw, self, 1, g);
		psy_ui_setorigin(g, origin);
	}	
	psy_ui_component_draw_children(self, g);	
	if (self->drawtobuffer && temp_g) {		
		psy_ui_graphics_dispose(&bitmap_g);	
		g = temp_g;
		if (!psy_ui_bitmap_empty(&self->bufferbitmap)) {
			psy_ui_drawfullbitmap(g, &self->bufferbitmap,
				psy_ui_realpoint_zero());
		}
	}
	psy_ui_graphics_set_clip_rect(g, oldclip);
}

void psy_ui_component_draw_children(psy_ui_Component* self, psy_ui_Graphics* g)
{		
	psy_List* q;		

	q = psy_ui_component_children(self, psy_ui_NONE_RECURSIVE);
	if (q) {		
		psy_ui_RealRectangle clip;
		psy_ui_RealPoint origin;
		psy_List* p;

		clip = psy_ui_graphics_cliprect(g);
		origin = psy_ui_origin(g);
		for (p = q; p != NULL; p = p->next) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)psy_list_entry(p);			
			if (psy_ui_component_visible(component) &&
					psy_ui_component_islightweight(component)) {
				psy_ui_RealRectangle position;
				psy_ui_RealRectangle intersection;				

				position = psy_ui_component_position(component);
				if (self->aligner &&
					(self->aligner->alignsorted == psy_ui_ALIGN_TOP && position.bottom < clip.top) ||
					(self->aligner->alignsorted == psy_ui_ALIGN_LEFT && position.right < clip.left)) {
					continue;
				}
				intersection = clip;				
				if (psy_ui_realrectangle_intersection(&intersection, &position)) {
					/* translate graphics clip and origin */
					psy_ui_realrectangle_set_topleft(&intersection,
						psy_ui_realpoint_make(
							intersection.left - position.left,
							intersection.top - position.top));
					psy_ui_graphics_set_clip_rect(g, clip);
					psy_ui_setorigin(g,
						psy_ui_realpoint_make(
							-position.left + origin.x,
							-position.top + origin.y));
					/* draw */
					psy_ui_component_draw(component, g);
					/* reset origin */
					psy_ui_setorigin(g, origin);
					/* terminate? */
					if (self->aligner &&
						(self->aligner->alignsorted == psy_ui_ALIGN_TOP && position.top > clip.bottom) ||
						(self->aligner->alignsorted == psy_ui_ALIGN_LEFT && position.left > clip.right)) {
						break;
					}
				}
			}
		}
		psy_list_free(q);		
	}	
}

void psy_ui_component_traverse_int(psy_ui_Component* self, psy_fp_int fp,
	uintptr_t value)
{
	psy_List* p;
	psy_List* q;
	
	for (p = q = psy_ui_component_children(self, psy_ui_NONE_RECURSIVE);
			p != NULL; p = p->next) {
		fp((psy_ui_Component*)p->entry, value);
	}
	psy_list_free(q);
}

void psy_ui_component_set_id(psy_ui_Component* self, uintptr_t id)
{
	self->id = id;
}

uintptr_t psy_ui_component_id(const psy_ui_Component* self)
{
	return self->id;
}

psy_ui_Component* psy_ui_component_by_id(psy_ui_Component* self, uintptr_t id, int recursive)
{
	psy_ui_Component* rv;
	psy_List* p;
	psy_List* q;

	rv = NULL;
	/* is id from self? */
	if (self->id != psy_INDEX_INVALID && (self->id == id)) {
		return self;		
	}
	/* is id from direct children */
	for (p = q = psy_ui_component_children(self, psy_ui_NONE_RECURSIVE); p != NULL; p = p->next) {
		psy_ui_Component* component;
		
		component = (psy_ui_Component*)p->entry;
		if (psy_ui_component_id(component) == psy_INDEX_INVALID) {
			continue;
		}
		if (psy_ui_component_id(component) == id) {
			rv = component;
			break;
		}
	}	
	if (rv == NULL && recursive) {
		/* search recursive */
		for (p = q; p != NULL; p = p->next) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)p->entry;
			rv = psy_ui_component_by_id(component, id, recursive);
			if (rv) {
				break;
			}			
		}
	}
	psy_list_free(q);
	return rv;
}

void psy_ui_component_buffer(psy_ui_Component* self)
{
	self->drawtobuffer = TRUE;	
}

void psy_ui_component_clearbuffer(psy_ui_Component* self)
{
	psy_ui_bitmap_dispose(&self->bufferbitmap);
	psy_ui_bitmap_init(&self->bufferbitmap);
}

void* psy_ui_component_platform(psy_ui_Component* self)
{
	assert(self);
	assert(self->imp);

	return (void*)self->imp->vtable->dev_platform_handle(self->imp);
}

void psy_ui_component_start_timer(psy_ui_Component* self, uintptr_t id,
	uintptr_t interval)
{
	psy_ui_app_starttimer(psy_ui_app(), self, id, interval);
}

void psy_ui_component_stop_timer(psy_ui_Component* self, uintptr_t id)
{
	psy_ui_app_stoptimer(psy_ui_app(), self, id);
}

psy_ui_RealRectangle psy_ui_component_bounds(psy_ui_Component* self)
{
	psy_ui_RealRectangle rv;	
	psy_List* p;
	psy_List* q;
	
	for (p = q = psy_ui_component_children(self, psy_ui_NONE_RECURSIVE);
			p != NULL; p = p->next) {
		psy_ui_Component* component;
		psy_ui_RealRectangle r;

		component = (psy_ui_Component*)p->entry;
		if (p == q) {
			rv = psy_ui_component_position(component);
		} else {
			r = psy_ui_component_position(component);
			psy_ui_realrectangle_union(&rv, &r);
		}
	}
	if (!q) {
		return psy_ui_realrectangle_zero();
	}
	psy_list_free(q);	
	return rv;
}

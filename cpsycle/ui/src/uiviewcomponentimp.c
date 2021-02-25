// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiviewcomponentimp.h"
// platform
#include "../../detail/portable.h"

// ViewComponentImp
// prototypes
static void view_dev_dispose(psy_ui_ViewComponentImp*);
static void view_dev_destroy(psy_ui_ViewComponentImp*);
static void view_dev_show(psy_ui_ViewComponentImp*);
static void view_dev_showstate(psy_ui_ViewComponentImp*, int state);
static void view_dev_hide(psy_ui_ViewComponentImp*);
static int view_dev_visible(psy_ui_ViewComponentImp*);
static int view_dev_drawvisible(psy_ui_ViewComponentImp*);
static void view_dev_move(psy_ui_ViewComponentImp*, psy_ui_Point origin);
static void view_dev_resize(psy_ui_ViewComponentImp*, psy_ui_Size);
static void view_dev_clientresize(psy_ui_ViewComponentImp*, intptr_t width, intptr_t height);
static psy_ui_RealRectangle view_dev_position(psy_ui_ViewComponentImp*);
static void view_dev_setposition(psy_ui_ViewComponentImp*, psy_ui_Point topleft,
	psy_ui_Size);
static psy_ui_Size view_dev_size(const psy_ui_ViewComponentImp*);
static void view_dev_updatesize(psy_ui_ViewComponentImp*);
static psy_ui_Size view_dev_framesize(psy_ui_ViewComponentImp*);
static void view_dev_scrollto(psy_ui_ViewComponentImp*, intptr_t dx, intptr_t dy);
static psy_ui_Component* view_dev_parent(psy_ui_ViewComponentImp*);
static void view_dev_setparent(psy_ui_ViewComponentImp*, psy_ui_Component* parent);
static void view_dev_insert(psy_ui_ViewComponentImp*, psy_ui_ViewComponentImp* child,
	psy_ui_ViewComponentImp* insertafter);
static void view_dev_setorder(psy_ui_ViewComponentImp*, psy_ui_ViewComponentImp*
	insertafter);
static void view_dev_capture(psy_ui_ViewComponentImp*);
static void view_dev_releasecapture(psy_ui_ViewComponentImp*);
static void view_dev_invalidate(psy_ui_ViewComponentImp*);
static void view_dev_invalidaterect(psy_ui_ViewComponentImp*,
	const psy_ui_RealRectangle*);
static void view_dev_update(psy_ui_ViewComponentImp*);
static void view_dev_setfont(psy_ui_ViewComponentImp*, psy_ui_Font*);
static void view_dev_showhorizontalscrollbar(psy_ui_ViewComponentImp*);
static void view_dev_hidehorizontalscrollbar(psy_ui_ViewComponentImp*);
static psy_List* view_dev_children(psy_ui_ViewComponentImp*, int recursive);
static void view_dev_enableinput(psy_ui_ViewComponentImp*);
static void view_dev_preventinput(psy_ui_ViewComponentImp*);
static void view_dev_setcursor(psy_ui_ViewComponentImp*, psy_ui_CursorStyle);
static void view_dev_starttimer(psy_ui_ViewComponentImp*, uintptr_t id,
	uintptr_t interval);
static void view_dev_stoptimer(psy_ui_ViewComponentImp*, uintptr_t id);
static void view_dev_seticonressource(psy_ui_ViewComponentImp*, int ressourceid);
static const psy_ui_TextMetric* view_dev_textmetric(const psy_ui_ViewComponentImp*);
static psy_ui_Size view_dev_textsize(psy_ui_ViewComponentImp*, const char* text,
	psy_ui_Font*);
static void view_dev_setbackgroundcolour(psy_ui_ViewComponentImp*, psy_ui_Colour);
static void view_dev_settitle(psy_ui_ViewComponentImp*, const char* title);
static void view_dev_setfocus(psy_ui_ViewComponentImp*);
static int view_dev_hasfocus(psy_ui_ViewComponentImp*);

// VTable init
static psy_ui_ComponentImpVTable view_imp_vtable;
static bool view_imp_vtable_initialized = FALSE;

static void view_imp_vtable_init(psy_ui_ViewComponentImp* self)
{
	if (!view_imp_vtable_initialized) {
		view_imp_vtable = *self->imp.vtable;
		view_imp_vtable.dev_dispose = (psy_ui_fp_componentimp_dev_dispose)view_dev_dispose;
		view_imp_vtable.dev_destroy = (psy_ui_fp_componentimp_dev_destroy)view_dev_destroy;
		view_imp_vtable.dev_show = (psy_ui_fp_componentimp_dev_show)view_dev_show;
		view_imp_vtable.dev_showstate = (psy_ui_fp_componentimp_dev_showstate)
			view_dev_showstate;
		view_imp_vtable.dev_hide = (psy_ui_fp_componentimp_dev_hide)view_dev_hide;
		view_imp_vtable.dev_visible = (psy_ui_fp_componentimp_dev_visible)view_dev_visible;
		view_imp_vtable.dev_drawvisible = (psy_ui_fp_componentimp_dev_drawvisible)view_dev_drawvisible;
		view_imp_vtable.dev_move = (psy_ui_fp_componentimp_dev_move)view_dev_move;
		view_imp_vtable.dev_resize = (psy_ui_fp_componentimp_dev_resize)view_dev_resize;
		view_imp_vtable.dev_clientresize = (psy_ui_fp_componentimp_dev_clientresize)
			view_dev_clientresize;
		view_imp_vtable.dev_position = (psy_ui_fp_componentimp_dev_position)view_dev_position;
		view_imp_vtable.dev_setposition = (psy_ui_fp_componentimp_dev_setposition)
			view_dev_setposition;
		view_imp_vtable.dev_size = (psy_ui_fp_componentimp_dev_size)view_dev_size;
		view_imp_vtable.dev_updatesize = (psy_ui_fp_componentimp_dev_size)view_dev_updatesize;
		view_imp_vtable.dev_framesize = (psy_ui_fp_componentimp_dev_framesize)
			view_dev_framesize;
		view_imp_vtable.dev_scrollto = (psy_ui_fp_componentimp_dev_scrollto)view_dev_scrollto;
		view_imp_vtable.dev_parent = (psy_ui_fp_componentimp_dev_parent)view_dev_parent;
		view_imp_vtable.dev_setparent = (psy_ui_fp_componentimp_dev_setparent)
			view_dev_setparent;
		view_imp_vtable.dev_insert = (psy_ui_fp_componentimp_dev_insert)view_dev_insert;
		view_imp_vtable.dev_capture = (psy_ui_fp_componentimp_dev_capture)view_dev_capture;
		view_imp_vtable.dev_releasecapture = (psy_ui_fp_componentimp_dev_releasecapture)
			view_dev_releasecapture;
		view_imp_vtable.dev_invalidate = (psy_ui_fp_componentimp_dev_invalidate)
			view_dev_invalidate;
		view_imp_vtable.dev_invalidaterect = (psy_ui_fp_componentimp_dev_invalidaterect)
			view_dev_invalidaterect;
		view_imp_vtable.dev_update = (psy_ui_fp_componentimp_dev_update)view_dev_update;
		view_imp_vtable.dev_setfont = (psy_ui_fp_componentimp_dev_setfont)view_dev_setfont;
		view_imp_vtable.dev_children = (psy_ui_fp_componentimp_dev_children)view_dev_children;
		view_imp_vtable.dev_enableinput = (psy_ui_fp_componentimp_dev_enableinput)
			view_dev_enableinput;
		view_imp_vtable.dev_preventinput = (psy_ui_fp_componentimp_dev_preventinput)
			view_dev_preventinput;
		view_imp_vtable.dev_setcursor = (psy_ui_fp_componentimp_dev_setcursor)
			view_dev_setcursor;
		view_imp_vtable.dev_starttimer = (psy_ui_fp_componentimp_dev_starttimer)
			view_dev_starttimer;
		view_imp_vtable.dev_stoptimer = (psy_ui_fp_componentimp_dev_stoptimer)
			view_dev_stoptimer;
		view_imp_vtable.dev_seticonressource =
			(psy_ui_fp_componentimp_dev_seticonressource)view_dev_seticonressource;
		view_imp_vtable.dev_textmetric = (psy_ui_fp_componentimp_dev_textmetric)
			view_dev_textmetric;
		view_imp_vtable.dev_textsize = (psy_ui_fp_componentimp_dev_textsize)view_dev_textsize;
		view_imp_vtable.dev_setbackgroundcolour =
			(psy_ui_fp_componentimp_dev_setbackgroundcolour)
			view_dev_setbackgroundcolour;
		view_imp_vtable.dev_settitle = (psy_ui_fp_componentimp_dev_settitle)view_dev_settitle;
		view_imp_vtable.dev_setfocus = (psy_ui_fp_componentimp_dev_setfocus)view_dev_setfocus;
		view_imp_vtable.dev_hasfocus = (psy_ui_fp_componentimp_dev_hasfocus)view_dev_hasfocus;
		view_imp_vtable_initialized = TRUE;
	}
}


void psy_ui_viewcomponentimp_init(psy_ui_ViewComponentImp* self,
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent,
	psy_ui_Component* view,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand)
{
	// psy_ui_ViewComponentImp* parent_imp;
	psy_ui_componentimp_init(&self->imp);
	view_imp_vtable_init(self);
	self->imp.vtable = &view_imp_vtable;
	self->view = view;
	self->component = component;
	psy_ui_realrectangle_init_all(&self->position,
		psy_ui_realpoint_make(x, y),
		psy_ui_realsize_make(width, height));
}

void view_dev_dispose(psy_ui_ViewComponentImp* self)
{
	psy_ui_componentimp_dispose(&self->imp);
}

psy_ui_ViewComponentImp* psy_ui_viewcomponentimp_alloc(void)
{
	return (psy_ui_ViewComponentImp*)malloc(sizeof(psy_ui_ViewComponentImp));
}

psy_ui_ViewComponentImp* psy_ui_viewcomponentimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent,
	psy_ui_Component* view,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand)
{
	psy_ui_ViewComponentImp* rv;

	rv = psy_ui_viewcomponentimp_alloc();
	if (rv) {
		psy_ui_viewcomponentimp_init(rv,
			component,
			parent,
			view,
			classname,
			x, y, width, height,
			dwStyle,
			usecommand
		);
	}
	return rv;
}

void view_dev_destroy(psy_ui_ViewComponentImp* self)
{
}

void view_dev_show(psy_ui_ViewComponentImp* self)
{
}

void view_dev_showstate(psy_ui_ViewComponentImp* self, int state)
{
}

void view_dev_hide(psy_ui_ViewComponentImp* self)
{
}

int view_dev_visible(psy_ui_ViewComponentImp* self)
{
	return TRUE;
}

int view_dev_drawvisible(psy_ui_ViewComponentImp* self)
{
	return TRUE;
}

void view_dev_move(psy_ui_ViewComponentImp* self, psy_ui_Point origin)
{
	psy_ui_realrectangle_settopleft(&self->position,
		psy_ui_realpoint_make(
			psy_ui_value_px(&origin.x, NULL),
			psy_ui_value_px(&origin.y, NULL)));
}

void view_dev_resize(psy_ui_ViewComponentImp* self, psy_ui_Size size)
{
	psy_ui_RealPoint topleft;

	topleft = psy_ui_realrectangle_topleft(&self->position);
	self->position = psy_ui_realrectangle_make(
		topleft,
		psy_ui_realsize_make(
			psy_ui_value_px(&size.width, NULL),
			psy_ui_value_px(&size.height, NULL)));
}

void view_dev_clientresize(psy_ui_ViewComponentImp* self, intptr_t width, intptr_t height)
{
}


psy_ui_RealRectangle view_dev_position(psy_ui_ViewComponentImp* self)
{
	return self->position;
}

void view_dev_setposition(psy_ui_ViewComponentImp* self, psy_ui_Point topleft,
	psy_ui_Size size)
{
	self->position = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			psy_ui_value_px(&topleft.x, NULL),
			psy_ui_value_px(&topleft.y, NULL)),
		psy_ui_realsize_make(
			psy_ui_value_px(&size.width, NULL),
			psy_ui_value_px(&size.height, NULL)));
}

psy_ui_Size view_dev_size(const psy_ui_ViewComponentImp* self)
{
	return psy_ui_size_makeem(self->position.right - self->position.left,
		self->position.bottom - self->position.top);
}

void view_dev_updatesize(psy_ui_ViewComponentImp* self)
{
}

psy_ui_Size view_dev_framesize(psy_ui_ViewComponentImp* self)
{
	return psy_ui_size_makeem(self->position.right - self->position.left,
		self->position.bottom - self->position.top);
}

void view_dev_scrollto(psy_ui_ViewComponentImp* self, intptr_t dx, intptr_t dy)
{
}

psy_ui_Component* view_dev_parent(psy_ui_ViewComponentImp* self)
{
	return NULL;
}

void view_dev_setparent(psy_ui_ViewComponentImp* self, psy_ui_Component* parent)
{
}

void view_dev_insert(psy_ui_ViewComponentImp* self, psy_ui_ViewComponentImp* child,
	psy_ui_ViewComponentImp* insertafter)
{
}

void view_dev_setorder(psy_ui_ViewComponentImp* self, psy_ui_ViewComponentImp*
	insertafter)
{
}

void view_dev_capture(psy_ui_ViewComponentImp* self)
{
}

void view_dev_releasecapture(psy_ui_ViewComponentImp* self)
{
}

void view_dev_invalidate(psy_ui_ViewComponentImp* self)
{	
	psy_ui_component_invalidaterect(self->view, 
		psy_ui_component_position(self->component));
}

void view_dev_invalidaterect(psy_ui_ViewComponentImp* self,
	const psy_ui_RealRectangle* r)
{
	if (r) {
		psy_ui_component_invalidaterect(self->view, *r);
	}
}

void view_dev_update(psy_ui_ViewComponentImp* self)
{
	psy_ui_component_update(self->view);
}

void view_dev_setfont(psy_ui_ViewComponentImp* self, psy_ui_Font* source)
{
}

psy_List* view_dev_children(psy_ui_ViewComponentImp* self, int recursive)
{
	psy_List* rv = 0;
	return rv;
}

void view_dev_enableinput(psy_ui_ViewComponentImp* self)
{
}

void view_dev_preventinput(psy_ui_ViewComponentImp* self)
{
}

const psy_ui_TextMetric* view_dev_textmetric(const psy_ui_ViewComponentImp* self)
{
	return NULL;
}

void view_dev_setcursor(psy_ui_ViewComponentImp* self, psy_ui_CursorStyle
	cursorstyle)
{
}

void view_dev_starttimer(psy_ui_ViewComponentImp* self, uintptr_t id,
	uintptr_t interval)
{
}

void view_dev_stoptimer(psy_ui_ViewComponentImp* self, uintptr_t id)
{
}

void view_dev_seticonressource(psy_ui_ViewComponentImp* self, int ressourceid)
{
}

psy_ui_Size view_dev_textsize(psy_ui_ViewComponentImp* self, const char* text,
	psy_ui_Font* font)
{
	return psy_ui_size_makeem(1.0, 1.0);
}

void view_dev_setbackgroundcolour(psy_ui_ViewComponentImp* self, psy_ui_Colour colour)
{
}

void view_dev_settitle(psy_ui_ViewComponentImp* self, const char* title)
{
}

void view_dev_setfocus(psy_ui_ViewComponentImp* self)
{	
}

int view_dev_hasfocus(psy_ui_ViewComponentImp* self)
{
	return FALSE;
}

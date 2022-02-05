/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiviewcomponentimp.h"
/* local */
#include "uiapp.h"
#include "uicomponent.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

/* ViewComponentImp */
/* prototypes */
static void view_dev_destroyed(psy_ui_ViewComponentImp*);
static void view_dev_dispose(psy_ui_ViewComponentImp*);
static void view_dev_destroy(psy_ui_ViewComponentImp*);
static void view_dev_clear(psy_ui_ViewComponentImp*);
static void view_dev_show(psy_ui_ViewComponentImp*);
static void view_dev_showstate(psy_ui_ViewComponentImp*, int state);
static void view_dev_hide(psy_ui_ViewComponentImp*);
static int view_dev_visible(psy_ui_ViewComponentImp*);
static int view_dev_drawvisible(psy_ui_ViewComponentImp*);
static void view_dev_move(psy_ui_ViewComponentImp*, psy_ui_Point origin);
static void view_dev_resize(psy_ui_ViewComponentImp*, psy_ui_Size);
static void view_dev_clientresize(psy_ui_ViewComponentImp*, intptr_t width,
	intptr_t height);
static psy_ui_RealRectangle view_dev_position(psy_ui_ViewComponentImp*);
static psy_ui_RealRectangle view_dev_screenposition(psy_ui_ViewComponentImp*);
static void view_dev_setposition(psy_ui_ViewComponentImp*,
	psy_ui_Point topleft, psy_ui_Size);
static psy_ui_Size view_dev_size(const psy_ui_ViewComponentImp*);
static void view_dev_updatesize(psy_ui_ViewComponentImp*);
static void view_dev_applyposition(psy_ui_ViewComponentImp* self) { }
static psy_ui_Size view_dev_framesize(psy_ui_ViewComponentImp*);
static void view_dev_scrollto(psy_ui_ViewComponentImp*, intptr_t dx,
	intptr_t dy, const psy_ui_RealRectangle*);
static psy_ui_Component* view_dev_parent(psy_ui_ViewComponentImp*);
static void view_dev_setparent(psy_ui_ViewComponentImp*,
	psy_ui_Component* parent);
static void view_dev_insert(psy_ui_ViewComponentImp*,
	psy_ui_ViewComponentImp* child, psy_ui_ViewComponentImp* insertafter);
static void view_dev_remove(psy_ui_ViewComponentImp*,
	psy_ui_ViewComponentImp* child);
static void view_dev_erase(psy_ui_ViewComponentImp*,
	psy_ui_ViewComponentImp* child);
static void view_dev_setorder(psy_ui_ViewComponentImp*,
	psy_ui_ViewComponentImp* insertafter);
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
bool view_dev_inputprevented(const psy_ui_ViewComponentImp*);
static void view_dev_setcursor(psy_ui_ViewComponentImp*, psy_ui_CursorStyle);
static void view_dev_seticonressource(psy_ui_ViewComponentImp*,
	int ressourceid);
static const psy_ui_TextMetric* view_dev_textmetric(
	const psy_ui_ViewComponentImp*);
static psy_ui_Size view_dev_textsize(psy_ui_ViewComponentImp*,
	const char* text, psy_ui_Font*);
static void view_dev_setbackgroundcolour(psy_ui_ViewComponentImp*,
	psy_ui_Colour);
static void view_dev_settitle(psy_ui_ViewComponentImp*, const char* title);
static void view_dev_setfocus(psy_ui_ViewComponentImp*);
static int view_dev_hasfocus(psy_ui_ViewComponentImp*);
static uintptr_t view_dev_flags(const psy_ui_ComponentImp*);
static psy_ui_RealRectangle view_translation(psy_ui_ViewComponentImp*,
	const psy_ui_RealRectangle*);
static psy_ui_RealPoint translatecoords(psy_ui_ViewComponentImp*,
	psy_ui_Component* src, psy_ui_Component* dst);

/* vtable */
static psy_ui_ComponentImpVTable view_imp_vtable;
static bool view_imp_vtable_initialized = FALSE;

static void view_imp_vtable_init(psy_ui_ViewComponentImp* self)
{
	if (!view_imp_vtable_initialized) {
		view_imp_vtable = *self->imp.vtable;
		view_imp_vtable.dev_destroy =
			(psy_ui_fp_componentimp_dev_destroy)
			view_dev_destroy;
		view_imp_vtable.dev_destroyed =
			(psy_ui_fp_componentimp_dev_destroyed)
			view_dev_destroyed;
		view_imp_vtable.dev_dispose =
			(psy_ui_fp_componentimp_dev_dispose)
			view_dev_dispose;
		view_imp_vtable.dev_clear =
			(psy_ui_fp_componentimp_dev_clear)
			view_dev_clear;
		view_imp_vtable.dev_show =
			(psy_ui_fp_componentimp_dev_show)
			view_dev_show;
		view_imp_vtable.dev_showstate =
			(psy_ui_fp_componentimp_dev_showstate)
			view_dev_showstate;
		view_imp_vtable.dev_hide =
			(psy_ui_fp_componentimp_dev_hide)
			view_dev_hide;
		view_imp_vtable.dev_visible =
			(psy_ui_fp_componentimp_dev_visible)
			view_dev_visible;
		view_imp_vtable.dev_drawvisible =
			(psy_ui_fp_componentimp_dev_drawvisible)
			view_dev_drawvisible;
		view_imp_vtable.dev_move =
			(psy_ui_fp_componentimp_dev_move)
			view_dev_move;
		view_imp_vtable.dev_resize =
			(psy_ui_fp_componentimp_dev_resize)
			view_dev_resize;
		view_imp_vtable.dev_clientresize =
			(psy_ui_fp_componentimp_dev_clientresize)
			view_dev_clientresize;
		view_imp_vtable.dev_position =
			(psy_ui_fp_componentimp_dev_position)
			view_dev_position;
		view_imp_vtable.dev_screenposition =
			(psy_ui_fp_componentimp_dev_position)
			view_dev_screenposition;
		view_imp_vtable.dev_setposition =
			(psy_ui_fp_componentimp_dev_setposition)
			view_dev_setposition;
		view_imp_vtable.dev_size =
			(psy_ui_fp_componentimp_dev_size)
			view_dev_size;
		view_imp_vtable.dev_updatesize =
			(psy_ui_fp_componentimp_dev_updatesize)
			view_dev_updatesize;
		view_imp_vtable.dev_applyposition =
			(psy_ui_fp_componentimp_dev_applyposition)
			view_dev_applyposition;
		view_imp_vtable.dev_framesize =
			(psy_ui_fp_componentimp_dev_framesize)
			view_dev_framesize;
		view_imp_vtable.dev_scrollto =
			(psy_ui_fp_componentimp_dev_scrollto)
			view_dev_scrollto;
		view_imp_vtable.dev_parent =
			(psy_ui_fp_componentimp_dev_parent)
			view_dev_parent;
		view_imp_vtable.dev_setparent =
			(psy_ui_fp_componentimp_dev_setparent)
			view_dev_setparent;
		view_imp_vtable.dev_insert =
			(psy_ui_fp_componentimp_dev_insert)
			view_dev_insert;
		view_imp_vtable.dev_remove =
			(psy_ui_fp_componentimp_dev_remove)
			view_dev_remove;
		view_imp_vtable.dev_erase =
			(psy_ui_fp_componentimp_dev_erase)
			view_dev_erase;
		view_imp_vtable.dev_capture =
			(psy_ui_fp_componentimp_dev_capture)
			view_dev_capture;
		view_imp_vtable.dev_releasecapture =
			(psy_ui_fp_componentimp_dev_releasecapture)
			view_dev_releasecapture;
		view_imp_vtable.dev_invalidate =
			(psy_ui_fp_componentimp_dev_invalidate)
			view_dev_invalidate;
		view_imp_vtable.dev_invalidaterect =
			(psy_ui_fp_componentimp_dev_invalidaterect)
			view_dev_invalidaterect;
		view_imp_vtable.dev_update =
			(psy_ui_fp_componentimp_dev_update)
			view_dev_update;
		view_imp_vtable.dev_setfont =
			(psy_ui_fp_componentimp_dev_setfont)
			view_dev_setfont;
		view_imp_vtable.dev_children =
			(psy_ui_fp_componentimp_dev_children)
			view_dev_children;
		view_imp_vtable.dev_enableinput =
			(psy_ui_fp_componentimp_dev_enableinput)
			view_dev_enableinput;
		view_imp_vtable.dev_preventinput =
			(psy_ui_fp_componentimp_dev_preventinput)
			view_dev_preventinput;
		view_imp_vtable.dev_inputprevented =
			(psy_ui_fp_componentimp_dev_inputprevented)
			view_dev_inputprevented;
		view_imp_vtable.dev_setcursor =
			(psy_ui_fp_componentimp_dev_setcursor)
			view_dev_setcursor;		
		view_imp_vtable.dev_seticonressource =
			(psy_ui_fp_componentimp_dev_seticonressource)
			view_dev_seticonressource;
		view_imp_vtable.dev_textmetric =
			(psy_ui_fp_componentimp_dev_textmetric)
			view_dev_textmetric;
		view_imp_vtable.dev_textsize =
			(psy_ui_fp_componentimp_dev_textsize)
			view_dev_textsize;
		view_imp_vtable.dev_setbackgroundcolour =
			(psy_ui_fp_componentimp_dev_setbackgroundcolour)
			view_dev_setbackgroundcolour;
		view_imp_vtable.dev_settitle =
			(psy_ui_fp_componentimp_dev_settitle)
			view_dev_settitle;
		view_imp_vtable.dev_setfocus =
			(psy_ui_fp_componentimp_dev_setfocus)
			view_dev_setfocus;
		view_imp_vtable.dev_hasfocus =
			(psy_ui_fp_componentimp_dev_hasfocus)
			view_dev_hasfocus;
		view_imp_vtable.dev_flags =
			(psy_ui_fp_componentimp_dev_flags)
			view_dev_flags;		
		view_imp_vtable_initialized = TRUE;
	}
}

/* implementation */
void psy_ui_viewcomponentimp_init(psy_ui_ViewComponentImp* self,
	struct psy_ui_Component* component,
	psy_ui_Component* parent,
	psy_ui_Component* view,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand)
{
	psy_ui_componentimp_init(&self->imp);
	view_imp_vtable_init(self);
	self->imp.vtable = &view_imp_vtable;
	self->view = view;
	self->component = component;	
	self->parent = parent;
	self->visible = TRUE;
	self->tmcachevalid = FALSE;
	if (parent && parent->imp) {
		parent->imp->vtable->dev_insert(parent->imp, &self->imp, NULL);		
	}
	psy_ui_realrectangle_init_all(&self->position,
		psy_ui_realpoint_make(x, y),
		psy_ui_realsize_make(width, height));
	self->viewcomponents = NULL;
}

void view_dev_dispose(psy_ui_ViewComponentImp* self)
{	
	if (self->component == psy_ui_app_capture(psy_ui_app())) {
		psy_ui_component_releasecapture(self->view);		
	}
	psy_ui_componentimp_dispose(&self->imp);
	self->parent = NULL;
	psy_list_free(self->viewcomponents);
	self->viewcomponents = NULL;
}

void view_dev_clear(psy_ui_ViewComponentImp* self)
{
	psy_List* p;
	psy_List* q;

	for (p = self->viewcomponents; p != NULL; p = q) {
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
	psy_list_free(self->viewcomponents);
	self->viewcomponents = NULL;

}

psy_ui_ViewComponentImp* psy_ui_viewcomponentimp_alloc(void)
{
	return (psy_ui_ViewComponentImp*)malloc(sizeof(psy_ui_ViewComponentImp));
}

psy_ui_ViewComponentImp* psy_ui_viewcomponentimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_Component* parent,
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
	psy_ui_Component* component;
	psy_ui_Component* parent;
	psy_List* c;	
	psy_List* p;	
	psy_List* q;

	component = self->component;
	if (!component) {
		return;
	}	
	parent = psy_ui_component_parent(component);
	psy_signal_emit(&component->signal_destroy,
		self->component, 0);
	component->vtable->ondestroy(component);	
	if (parent) {
		psy_ui_component_erase(parent, component);
	}
	c = psy_ui_component_children(self->component, psy_ui_NONRECURSIVE);
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
	psy_list_free(self->viewcomponents);
	self->viewcomponents = NULL;	
	component->imp->vtable->dev_destroyed(component->imp);			
}

void view_dev_destroyed(psy_ui_ViewComponentImp* self)
{		
	self->component->vtable->ondestroyed(self->component);	
	psy_ui_component_dispose(self->component);	
}

void view_dev_show(psy_ui_ViewComponentImp* self)
{
	self->visible = TRUE;
}

void view_dev_showstate(psy_ui_ViewComponentImp* self, int state)
{
	self->visible = TRUE;
}

void view_dev_hide(psy_ui_ViewComponentImp* self)
{
	self->visible = FALSE;
}

int view_dev_visible(psy_ui_ViewComponentImp* self)
{
	return self->visible;
}

int view_dev_drawvisible(psy_ui_ViewComponentImp* self)
{
	return self->visible;
}

void view_dev_move(psy_ui_ViewComponentImp* self, psy_ui_Point origin)
{
	const psy_ui_TextMetric* tm;	
		
	tm = view_dev_textmetric(self);
	psy_ui_realrectangle_settopleft(&self->position,
		psy_ui_realpoint_make(
			psy_ui_value_px(&origin.x, tm, NULL),
			psy_ui_value_px(&origin.y, tm, NULL)));	
//	view_dev_invalidate(self);
}

void view_dev_resize(psy_ui_ViewComponentImp* self, psy_ui_Size size)
{
	psy_ui_RealPoint topleft;
	const psy_ui_TextMetric* tm;
	
	topleft = psy_ui_realrectangle_topleft(&self->position);
	tm = view_dev_textmetric(self);
	self->position = psy_ui_realrectangle_make(
		topleft,
		psy_ui_realsize_make(
			psy_ui_value_px(&size.width, tm, NULL),
			psy_ui_value_px(&size.height, tm, NULL)));
}

void view_dev_clientresize(psy_ui_ViewComponentImp* self, intptr_t width,
	intptr_t height)
{
	
}

psy_ui_RealRectangle view_dev_position(psy_ui_ViewComponentImp* self)
{
	return self->position;
}

psy_ui_RealRectangle view_dev_screenposition(psy_ui_ViewComponentImp* self)
{
	psy_ui_RealPoint translation;
	psy_ui_RealRectangle position;
	psy_ui_RealRectangle viewscreenposition;

	assert(self->view);

	translation = translatecoords(self, self->component, self->view);
	position = psy_ui_component_position(self->component);
	viewscreenposition = psy_ui_component_screenposition(self->view);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			viewscreenposition.left + position.left + translation.x,
			viewscreenposition.top + position.top + translation.y),
		psy_ui_realrectangle_size(&position));	
}

void view_dev_setposition(psy_ui_ViewComponentImp* self, psy_ui_Point topleft,
	psy_ui_Size size)
{
	const psy_ui_TextMetric* tm;
	psy_ui_Size parentsize;
	psy_ui_Size* pparentsize;
					
	tm = view_dev_textmetric(self);
	if (psy_ui_size_has_percent(&size)) {
		if (psy_ui_component_parent_const(self->component)) {
			parentsize = psy_ui_component_scrollsize(
				psy_ui_component_parent_const(self->component));
		} else {
			parentsize = psy_ui_component_scrollsize(self->component);
		}
		pparentsize = &parentsize;
	} else {
		pparentsize = NULL;		
	}		
	self->position = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			psy_ui_value_px(&topleft.x, tm, pparentsize),
			psy_ui_value_px(&topleft.y, tm, pparentsize)),
		psy_ui_realsize_make(
			psy_ui_value_px(&size.width, tm, pparentsize),
			psy_ui_value_px(&size.height, tm, pparentsize)));	
}

psy_ui_Size view_dev_size(const psy_ui_ViewComponentImp* self)
{
	return psy_ui_size_make_px(self->position.right - self->position.left,
		self->position.bottom - self->position.top);
}

void view_dev_updatesize(psy_ui_ViewComponentImp* self)
{
}

psy_ui_Size view_dev_framesize(psy_ui_ViewComponentImp* self)
{
	return psy_ui_size_make_px(self->position.right - self->position.left,
		self->position.bottom - self->position.top);
}

psy_ui_Component* view_dev_parent(psy_ui_ViewComponentImp* self)
{
	return self->parent;
}

void view_dev_setparent(psy_ui_ViewComponentImp* self, psy_ui_Component* parent)
{
	if (self->parent) {
		psy_ui_component_erase(self->parent, self->component);
	}
	self->parent = parent;
	if (parent) {
		psy_ui_component_insert(parent, self->component, NULL);
	}
}

void view_dev_insert(psy_ui_ViewComponentImp* self, psy_ui_ViewComponentImp* child,
	psy_ui_ViewComponentImp* insertafter)
{
	if ((child->imp.vtable->dev_flags(&child->imp) & psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) ==
		psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) {
		psy_list_append(&self->viewcomponents, child->component);
	}
}

void view_dev_remove(psy_ui_ViewComponentImp* self, psy_ui_ViewComponentImp* child)
{
	if ((child->imp.vtable->dev_flags(&child->imp) & psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) ==
		psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) {
		psy_List* p;

		p = psy_list_findentry(self->viewcomponents, child->component);
		if (p) {
			psy_list_remove(&self->viewcomponents, p);
			if (child->component->deallocate) {
				psy_ui_component_deallocate(child->component);
			} else {
				psy_ui_component_dispose(child->component);
			}
		}
	} else {
		assert(0);
		/* todo */
	}
}

void view_dev_erase(psy_ui_ViewComponentImp* self, psy_ui_ViewComponentImp* child)
{	
	if ((child->imp.vtable->dev_flags(&child->imp) & psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) ==
		psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) {
		psy_List* p;

		p = psy_list_findentry(self->viewcomponents, child->component);
		if (p) {
			psy_list_remove(&self->viewcomponents, p);			
		}
	} else {
		assert(0);
		/* todo */
	}
}

void view_dev_setorder(psy_ui_ViewComponentImp* self, psy_ui_ViewComponentImp*
	insertafter)
{
}

void view_dev_capture(psy_ui_ViewComponentImp* self)
{
	psy_ui_app_setcapture(psy_ui_app(), self->component);
	self->view->imp->vtable->dev_capture(self->view->imp);
}

void view_dev_releasecapture(psy_ui_ViewComponentImp* self)
{
	self->view->imp->vtable->dev_releasecapture(self->view->imp);
}


void view_dev_scrollto(psy_ui_ViewComponentImp* self, intptr_t dx, intptr_t dy,
	const psy_ui_RealRectangle* r)
{
	psy_ui_RealSize size;
	psy_ui_RealRectangle rc;

	size = psy_ui_component_scrollsize_px(self->component);
	if (r) {
		rc = view_translation(self, r);		
	} else {
		rc = psy_ui_realrectangle_make(
			psy_ui_realpoint_zero(),
			size);
		rc = view_translation(self, &rc);
	}
	psy_ui_component_scrollto(self->view, dx, dy, &rc);
}


void view_dev_invalidate(psy_ui_ViewComponentImp* self)
{	
	psy_ui_RealSize size;
	psy_ui_RealRectangle r;

	size = psy_ui_component_scrollsize_px(self->component);
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_zero(),
		size);
	view_dev_invalidaterect(self, &r);
}

void view_dev_invalidaterect(psy_ui_ViewComponentImp* self,
	const psy_ui_RealRectangle* r)
{
	if (psy_ui_component_drawvisible(self->view) && r) {		
		psy_ui_component_invalidaterect(self->view,
			view_translation(self, r));
	}
}

psy_ui_RealRectangle view_translation(psy_ui_ViewComponentImp* self, 
	const psy_ui_RealRectangle* r)
{
	psy_ui_RealPoint translation;
	psy_ui_RealRectangle position;
	psy_ui_RealMargin spacing;

	translation = translatecoords(self, self->component, self->view);
	position = psy_ui_component_position(self->component);
	spacing = psy_ui_component_spacing_px(self->view);
	return psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				position.left + r->left + translation.x + spacing.left,
				position.top + r->top + translation.y + spacing.top),
			psy_ui_realrectangle_size(r));
}

psy_ui_RealPoint translatecoords(psy_ui_ViewComponentImp* self, psy_ui_Component* src,
	psy_ui_Component* dst)
{
	psy_ui_RealPoint rv;
	psy_ui_Component* curr;
	psy_ui_RealRectangle r;

	curr = psy_ui_component_parent(src);
	psy_ui_realpoint_init(&rv);
	while (dst != curr && curr != NULL) {
		r = psy_ui_component_position(curr);
		psy_ui_realpoint_add(&rv, psy_ui_realrectangle_topleft(&r));
		curr = psy_ui_component_parent(curr);
	}
	return rv;
}


void view_dev_update(psy_ui_ViewComponentImp* self)
{
	psy_ui_component_update(self->view);
}

void view_dev_setfont(psy_ui_ViewComponentImp* self, psy_ui_Font* source)
{
	if (source && self->tmcachevalid) {
		const psy_ui_Font* font;

		font = psy_ui_component_font(self->component);
		if (font) {
			self->tmcachevalid = psy_ui_font_equal(font, source);
			return;
		}
	}
	self->tmcachevalid = FALSE;
}

void view_dev_enableinput(psy_ui_ViewComponentImp* self)
{
}

void view_dev_preventinput(psy_ui_ViewComponentImp* self)
{
}

bool view_dev_inputprevented(const psy_ui_ViewComponentImp* self)
{
	return FALSE;
}

const psy_ui_TextMetric* view_dev_textmetric(const psy_ui_ViewComponentImp* self)
{	
	if (self->component && !self->tmcachevalid) {			
		const psy_ui_Font* font;

		font = psy_ui_component_font(self->component);
		/* mutable */
		((psy_ui_ViewComponentImp*)self)->tm = psy_ui_font_textmetric(font);
		((psy_ui_ViewComponentImp*)self)->tmcachevalid = TRUE;
	}
	return &self->tm;	
}

void view_dev_setcursor(psy_ui_ViewComponentImp* self, psy_ui_CursorStyle
	cursorstyle)
{
	psy_ui_component_setcursor(self->view, cursorstyle);
}

void view_dev_seticonressource(psy_ui_ViewComponentImp* self, int ressourceid)
{
}

psy_ui_Size view_dev_textsize(psy_ui_ViewComponentImp* self, const char* text,
	psy_ui_Font* font)
{
	if (self->view) {
		return psy_ui_component_textsize(self->view, text);
	}	
	return psy_ui_size_make_em(1.0 * psy_strlen(text), 1.0);
}

void view_dev_setbackgroundcolour(psy_ui_ViewComponentImp* self, psy_ui_Colour colour)
{
}

void view_dev_settitle(psy_ui_ViewComponentImp* self, const char* title)
{
}

void view_dev_setfocus(psy_ui_ViewComponentImp* self)
{
	assert(self);
	assert(self->view);
	
	if (!psy_ui_component_hasfocus(self->view)) {
		psy_ui_component_setfocus(self->view);
	} else if (self->component) {
		self->component->vtable->onfocus(self->component);
		psy_signal_emit(&self->component->signal_focus, self, 0);		
	}
}

int view_dev_hasfocus(psy_ui_ViewComponentImp* self)
{
	return psy_ui_app()->focus == self->component;
}

uintptr_t view_dev_flags(const psy_ui_ComponentImp* self)
{
	return psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN;
}

psy_List* view_dev_children(psy_ui_ViewComponentImp* self, int recursive)
{
	psy_List* rv = 0;
	psy_List* p;
	
	for (p = self->viewcomponents; p != NULL; psy_list_next(&p)) {
		psy_list_append(&rv, (psy_ui_Component*)psy_list_entry(p));
		if (recursive == psy_ui_RECURSIVE) {
			psy_List* r;
			psy_List* q;

			q = psy_ui_component_children((psy_ui_Component*)psy_list_entry(p), recursive);
			for (r = q; r != NULL; psy_list_next(&r)) {
				psy_list_append(&rv, (psy_ui_Component*)psy_list_entry(r));
			}
			psy_list_free(q);
		}
	}
	return rv;
}

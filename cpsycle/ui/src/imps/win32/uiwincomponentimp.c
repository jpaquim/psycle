// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwincomponentimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "uiwinimpfactory.h"
#include "../../uicomponent.h"
#include "uiwinfontimp.h"
#include "uiwinbitmapimp.h"
#include "../../uiapp.h"
#include "uiwinapp.h"
#include "uiwingraphicsimp.h"
// common control header
#include <commctrl.h>
// details
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static BOOL CALLBACK allchildenumproc(HWND hwnd, LPARAM lParam);

static int windowstyle(psy_ui_win_ComponentImp*);
static int windowexstyle(psy_ui_win_ComponentImp*);

static void psy_ui_win_component_create_window(psy_ui_win_ComponentImp*,
	psy_ui_win_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand);
static HINSTANCE psy_ui_win_component_instance(psy_ui_win_ComponentImp*
	parent);
static void psy_ui_win_component_init_wndproc(psy_ui_win_ComponentImp*,
	LPCSTR classname);

// prototypes
static void dev_dispose(psy_ui_win_ComponentImp*);
static void dev_destroy(psy_ui_win_ComponentImp*);
static void dev_show(psy_ui_win_ComponentImp*);
static void dev_showstate(psy_ui_win_ComponentImp*, int state);
static void dev_hide(psy_ui_win_ComponentImp*);
static int dev_visible(psy_ui_win_ComponentImp*);
static int dev_drawvisible(psy_ui_win_ComponentImp*);
static void dev_move(psy_ui_win_ComponentImp*, psy_ui_Point origin);
static void dev_resize(psy_ui_win_ComponentImp*, psy_ui_Size);
static void dev_clientresize(psy_ui_win_ComponentImp*, intptr_t width, intptr_t height);
static psy_ui_RealRectangle dev_position(psy_ui_win_ComponentImp*);
static psy_ui_RealRectangle dev_screenposition(psy_ui_win_ComponentImp*);
static void dev_setposition(psy_ui_win_ComponentImp*, psy_ui_Point topleft,
	psy_ui_Size);
static psy_ui_Size dev_size(const psy_ui_win_ComponentImp*);
static void dev_updatesize(psy_ui_win_ComponentImp*);
static psy_ui_Size dev_framesize(psy_ui_win_ComponentImp*);
static void dev_scrollto(psy_ui_win_ComponentImp*, intptr_t dx, intptr_t dy);
static psy_ui_Component* dev_parent(psy_ui_win_ComponentImp*);
static void dev_setparent(psy_ui_win_ComponentImp*, psy_ui_Component* parent);
static void dev_insert(psy_ui_win_ComponentImp*, psy_ui_win_ComponentImp* child,
	psy_ui_win_ComponentImp* insertafter);
static void dev_remove(psy_ui_win_ComponentImp*, psy_ui_win_ComponentImp* child);
static void dev_erase(psy_ui_win_ComponentImp*, psy_ui_win_ComponentImp* child);
static void dev_setorder(psy_ui_win_ComponentImp*, psy_ui_win_ComponentImp*
	insertafter);
static void dev_capture(psy_ui_win_ComponentImp*);
static void dev_releasecapture(psy_ui_win_ComponentImp*);
static void dev_invalidate(psy_ui_win_ComponentImp*);
static void dev_invalidaterect(psy_ui_win_ComponentImp*,
	const psy_ui_RealRectangle*);
static void dev_update(psy_ui_win_ComponentImp*);
static void dev_setfont(psy_ui_win_ComponentImp*, psy_ui_Font*);
static void dev_showhorizontalscrollbar(psy_ui_win_ComponentImp*);
static void dev_hidehorizontalscrollbar(psy_ui_win_ComponentImp*);
static psy_List* dev_children(psy_ui_win_ComponentImp*, int recursive);
static void dev_enableinput(psy_ui_win_ComponentImp*);
static void dev_preventinput(psy_ui_win_ComponentImp*);
static bool dev_inputprevented(const psy_ui_win_ComponentImp* self);
static void dev_setcursor(psy_ui_win_ComponentImp*, psy_ui_CursorStyle);
static void dev_starttimer(psy_ui_win_ComponentImp*, uintptr_t id,
	uintptr_t interval);
static void dev_stoptimer(psy_ui_win_ComponentImp*, uintptr_t id);
static void dev_seticonressource(psy_ui_win_ComponentImp*, int ressourceid);
static const psy_ui_TextMetric* dev_textmetric(const psy_ui_win_ComponentImp*);
static psy_ui_Size dev_textsize(psy_ui_win_ComponentImp*, const char* text,
	psy_ui_Font*);
static void dev_setbackgroundcolour(psy_ui_win_ComponentImp*, psy_ui_Colour);
static void dev_settitle(psy_ui_win_ComponentImp*, const char* title);
static void dev_setfocus(psy_ui_win_ComponentImp*);
static int dev_hasfocus(psy_ui_win_ComponentImp*);
static void dev_clear(psy_ui_win_ComponentImp*);
static void dev_draw(psy_ui_win_ComponentImp*, psy_ui_Graphics*);
static void dev_mousedown(psy_ui_win_ComponentImp*, psy_ui_MouseEvent*);
static void dev_mouseup(psy_ui_win_ComponentImp*, psy_ui_MouseEvent*);
static void dev_mousemove(psy_ui_win_ComponentImp*, psy_ui_MouseEvent*);
static void dev_mousedoubleclick(psy_ui_win_ComponentImp*, psy_ui_MouseEvent*);
static void dev_mouseenter(psy_ui_win_ComponentImp*);
static void dev_mouseleave(psy_ui_win_ComponentImp*);
static psy_ui_RealPoint translatecoords(psy_ui_win_ComponentImp*, psy_ui_Component* src,
	psy_ui_Component* dst);
static psy_ui_RealPoint mapcoords(psy_ui_win_ComponentImp* self, psy_ui_Component* src,
	psy_ui_Component* dst);

// VTable init
static psy_ui_ComponentImpVTable vtable;
static bool vtable_initialized = FALSE;

static void win_imp_vtable_init(psy_ui_win_ComponentImp* self)
{
	if (!vtable_initialized) {
		vtable = *self->imp.vtable;
		vtable.dev_dispose = (psy_ui_fp_componentimp_dev_dispose)dev_dispose;
		vtable.dev_destroy = (psy_ui_fp_componentimp_dev_destroy)dev_destroy;
		vtable.dev_show = (psy_ui_fp_componentimp_dev_show)dev_show;
		vtable.dev_showstate = (psy_ui_fp_componentimp_dev_showstate)
			dev_showstate;
		vtable.dev_hide = (psy_ui_fp_componentimp_dev_hide)dev_hide;
		vtable.dev_visible = (psy_ui_fp_componentimp_dev_visible)dev_visible;
		vtable.dev_drawvisible = (psy_ui_fp_componentimp_dev_drawvisible)dev_drawvisible;
		vtable.dev_move = (psy_ui_fp_componentimp_dev_move)dev_move;
		vtable.dev_resize = (psy_ui_fp_componentimp_dev_resize)dev_resize;
		vtable.dev_clientresize = (psy_ui_fp_componentimp_dev_clientresize)
			dev_clientresize;
		vtable.dev_position = (psy_ui_fp_componentimp_dev_position)dev_position;
		vtable.dev_screenposition = (psy_ui_fp_componentimp_dev_screenposition)dev_screenposition;
		vtable.dev_setposition = (psy_ui_fp_componentimp_dev_setposition)
			dev_setposition;
		vtable.dev_size = (psy_ui_fp_componentimp_dev_size)dev_size;
		vtable.dev_updatesize = (psy_ui_fp_componentimp_dev_size)dev_updatesize;
		vtable.dev_framesize = (psy_ui_fp_componentimp_dev_framesize)
			dev_framesize;
		vtable.dev_scrollto = (psy_ui_fp_componentimp_dev_scrollto)dev_scrollto;
		vtable.dev_parent = (psy_ui_fp_componentimp_dev_parent)dev_parent;
		vtable.dev_setparent = (psy_ui_fp_componentimp_dev_setparent)
			dev_setparent;
		vtable.dev_insert = (psy_ui_fp_componentimp_dev_insert)dev_insert;
		vtable.dev_remove = (psy_ui_fp_componentimp_dev_remove)dev_remove;
		vtable.dev_erase = (psy_ui_fp_componentimp_dev_erase)dev_erase;
		vtable.dev_capture = (psy_ui_fp_componentimp_dev_capture)dev_capture;
		vtable.dev_releasecapture = (psy_ui_fp_componentimp_dev_releasecapture)
			dev_releasecapture;
		vtable.dev_invalidate = (psy_ui_fp_componentimp_dev_invalidate)
			dev_invalidate;
		vtable.dev_invalidaterect = (psy_ui_fp_componentimp_dev_invalidaterect)
			dev_invalidaterect;
		vtable.dev_update = (psy_ui_fp_componentimp_dev_update)dev_update;
		vtable.dev_setfont = (psy_ui_fp_componentimp_dev_setfont)dev_setfont;		
		vtable.dev_children = (psy_ui_fp_componentimp_dev_children)dev_children;
		vtable.dev_enableinput = (psy_ui_fp_componentimp_dev_enableinput)
			dev_enableinput;
		vtable.dev_preventinput = (psy_ui_fp_componentimp_dev_preventinput)
			dev_preventinput;
		vtable.dev_inputprevented = (psy_ui_fp_componentimp_dev_inputprevented)
			dev_inputprevented;		
		vtable.dev_setcursor = (psy_ui_fp_componentimp_dev_setcursor)
			dev_setcursor;
		vtable.dev_starttimer = (psy_ui_fp_componentimp_dev_starttimer)
			dev_starttimer;
		vtable.dev_stoptimer = (psy_ui_fp_componentimp_dev_stoptimer)
			dev_stoptimer;
		vtable.dev_seticonressource =
			(psy_ui_fp_componentimp_dev_seticonressource) dev_seticonressource;
		vtable.dev_textmetric = (psy_ui_fp_componentimp_dev_textmetric)
			dev_textmetric;
		vtable.dev_textsize = (psy_ui_fp_componentimp_dev_textsize)dev_textsize;
		vtable.dev_setbackgroundcolour =
			(psy_ui_fp_componentimp_dev_setbackgroundcolour)
			dev_setbackgroundcolour;
		vtable.dev_settitle = (psy_ui_fp_componentimp_dev_settitle)dev_settitle;
		vtable.dev_setfocus = (psy_ui_fp_componentimp_dev_setfocus)dev_setfocus;
		vtable.dev_hasfocus = (psy_ui_fp_componentimp_dev_hasfocus)dev_hasfocus;
		vtable.dev_clear = (psy_ui_fp_componentimp_dev_clear)dev_clear;
		vtable.dev_draw = (psy_ui_fp_componentimp_dev_draw)dev_draw;
		vtable.dev_mousedown = (psy_ui_fp_componentimp_dev_mousedown)
			dev_mousedown;
		vtable.dev_mouseup = (psy_ui_fp_componentimp_dev_mouseup)
			dev_mouseup;
		vtable.dev_mousemove = (psy_ui_fp_componentimp_dev_mousemove)
			dev_mousemove;
		vtable.dev_mousedoubleclick = (psy_ui_fp_componentimp_dev_mousedoubleclick)
			dev_mousedoubleclick;
		vtable.dev_mouseenter = (psy_ui_fp_componentimp_dev_mouseenter)
			dev_mouseenter;
		vtable.dev_mouseleave = (psy_ui_fp_componentimp_dev_mouseleave)
			dev_mouseleave;
		vtable_initialized = TRUE;
	}
}

void psy_ui_win_componentimp_init(psy_ui_win_ComponentImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand)
{	
	psy_ui_win_ComponentImp* parent_imp;	

	psy_ui_componentimp_init(&self->imp);
	win_imp_vtable_init(self);
	self->imp.vtable = &vtable;
	self->component = component;
	self->background = 0;
	self->winid = -1;
	self->hwnd = 0;
	self->wndproc = 0;	
	self->preventwmchar = 0;
	self->sizecachevalid = FALSE;
	self->tmcachevalid = FALSE;
	self->dbg = 0;	
	self->visible = parent ? TRUE : FALSE;
	self->viewcomponents = NULL;
	parent_imp = (parent)
		? (psy_ui_win_ComponentImp*)parent
		: NULL;	
	psy_ui_win_component_create_window(self, parent_imp, classname, x, y,
		width, height, dwStyle, usecommand);
	if (self->hwnd) {
		psy_ui_win_component_init_wndproc(self, classname);
	}
}

void psy_ui_win_component_create_window(psy_ui_win_ComponentImp* self,
	psy_ui_win_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand)
{
	psy_ui_WinApp* winapp;
	HINSTANCE instance;
	int err = 0;

	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
	instance = psy_ui_win_component_instance(parent);
	self->hwnd = CreateWindow(
		classname,
		NULL,
		dwStyle,
		x, y, width, height,
		parent ? parent->hwnd : (HWND)NULL,
		usecommand ? (HMENU) winapp->winid : NULL,
		instance,
		NULL);
	if (self->hwnd == NULL) {
		char text[256];
		unsigned long err;

		err = GetLastError();
		psy_snprintf(text, 256, "Failed To Create Component (Error %u)", err);
		MessageBox(NULL, text, "Error",
			MB_OK | MB_ICONERROR);
		err = 1;
	} else {
		psy_table_insert(&winapp->selfmap, (uintptr_t)self->hwnd, self);
	}
	if (err == 0 && usecommand) {
		psy_table_insert(&winapp->winidmap, winapp->winid, self);
		++winapp->winid;
	}
}

HINSTANCE psy_ui_win_component_instance(psy_ui_win_ComponentImp* parent)
{
	HINSTANCE rv;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
	if (parent) {
#if defined(_WIN64)		
		rv = (HINSTANCE)GetWindowLongPtr(parent->hwnd, GWLP_HINSTANCE);
#else
		rv = (HINSTANCE)GetWindowLong(parent->hwnd, GWL_HINSTANCE);
#endif
	} else {
		rv = winapp->instance;
	}
	return rv;
}

void psy_ui_win_component_init_wndproc(psy_ui_win_ComponentImp* self,
	LPCSTR classname)
{
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
#if defined(_WIN64)		
	self->wndproc = (winproc) GetWindowLongPtr(self->hwnd, GWLP_WNDPROC);
#else		
	self->wndproc = (winproc) GetWindowLong(self->hwnd, GWL_WNDPROC);
#endif
	if (classname != winapp->componentclass && classname != winapp->appclass) {
#if defined(_WIN64)		
		SetWindowLongPtr(self->hwnd, GWLP_WNDPROC,
			(LONG_PTR)winapp->comwinproc);
#else	
		SetWindowLong(self->hwnd, GWL_WNDPROC, (LONG)winapp->comwinproc);
#endif
	}
}

// win32 implementation method for psy_ui_Component
void dev_dispose(psy_ui_win_ComponentImp* self)
{
	if (self->background) {
		DeleteObject(self->background);
	}
	psy_ui_componentimp_dispose(&self->imp);
	dev_clear(self);
}

void dev_clear(psy_ui_win_ComponentImp* self)
{ 
	psy_List* p;
	psy_List* q;

	for (p = self->viewcomponents; p != NULL; psy_list_next(&p)) {
		psy_ui_Component* component;
		bool deallocate;

		component = (psy_ui_Component*)psy_list_entry(p);
		deallocate = component->deallocate;
		psy_ui_component_destroy(component);		
		if (deallocate) {			
			free(component);
		}
	}
	psy_list_free(self->viewcomponents);
	self->viewcomponents = NULL;	

	if (self->component) {
		q = psy_ui_component_children(self->component, psy_ui_NONRECURSIVE);
		for (p = q; p != NULL; psy_list_next(&p)) {
			psy_ui_Component* component;			

			component = (psy_ui_Component*)psy_list_entry(p);			
			psy_ui_component_destroy(component);			
		}
		psy_list_free(q);
	}
}

psy_ui_win_ComponentImp* psy_ui_win_componentimp_alloc(void)
{
	return (psy_ui_win_ComponentImp*)malloc(sizeof(psy_ui_win_ComponentImp));
}

psy_ui_win_ComponentImp* psy_ui_win_componentimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand)
{
	psy_ui_win_ComponentImp* rv;

	rv = psy_ui_win_componentimp_alloc();
	if (rv) {
		psy_ui_win_componentimp_init(rv,
			component,
			parent,
			classname,
			x, y, width, height,
			dwStyle,
			usecommand
		);
	}	
	return rv;
}

void dev_destroy(psy_ui_win_ComponentImp* self)
{	
	DestroyWindow(self->hwnd);
}

void dev_show(psy_ui_win_ComponentImp* self)
{
	self->visible = TRUE;
	if (!IsWindowVisible(self->hwnd)) {
		ShowWindow(self->hwnd, SW_SHOW);
		UpdateWindow(self->hwnd);
	}
}

void dev_showstate(psy_ui_win_ComponentImp* self, int state)
{
	self->visible = TRUE;
	ShowWindow(self->hwnd, state);
	UpdateWindow(self->hwnd);
}

void dev_hide(psy_ui_win_ComponentImp* self)
{
	self->visible = FALSE;
	ShowWindow(self->hwnd, SW_HIDE);
	UpdateWindow(self->hwnd);
}

int dev_visible(psy_ui_win_ComponentImp* self)
{
	return self->visible;
	//return IsWindowVisible(self->hwnd);
}

int dev_drawvisible(psy_ui_win_ComponentImp* self)
{
	return IsWindowVisible(self->hwnd);
}

void dev_move(psy_ui_win_ComponentImp* self, psy_ui_Point topleft)
{
	SetWindowPos(self->hwnd, NULL,
		(int)psy_ui_value_px(&topleft.x, dev_textmetric(self)),
		(int)psy_ui_value_px(&topleft.y, dev_textmetric(self)),
		0, 0,
		SWP_NOZORDER | SWP_NOSIZE);	
}

void dev_resize(psy_ui_win_ComponentImp* self, psy_ui_Size size)
{	
	self->sizecachevalid = FALSE;
	SetWindowPos(self->hwnd, NULL,
		0, 0,
		(int)psy_ui_value_px(&size.width, dev_textmetric(self)),
		(int)psy_ui_value_px(&size.height, dev_textmetric(self)),
		SWP_NOZORDER | SWP_NOMOVE);
	
	self->sizecache = size;
	self->sizecachevalid = TRUE;
}

void dev_clientresize(psy_ui_win_ComponentImp* self, intptr_t width, intptr_t height)
{
	RECT rc;
	
	rc.left = 0;
	rc.top = 0;
	rc.right = (int)width;
	rc.bottom = (int)height;
	AdjustWindowRectEx(&rc, windowstyle(self),
		GetMenu(self->hwnd) != NULL,
		windowexstyle(self));
	dev_resize(self,
		psy_ui_size_make(
			psy_ui_value_makepx((double)rc.right - (double)rc.left),
			psy_ui_value_makepx((double)rc.bottom - (double)rc.top)));
}


psy_ui_RealRectangle dev_position(psy_ui_win_ComponentImp* self)
{
	psy_ui_RealRectangle rv;
	RECT rc;
	POINT pt;
	
	GetWindowRect(self->hwnd, &rc);	
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(GetParent(self->hwnd), &pt);	
	GetClientRect(self->hwnd, &rc);
	rv.left = pt.x;
	rv.top = pt.y;
	rv.right = rv.left + rc.right;
	rv.bottom = rv.top + rc.bottom;	
	return rv;
}

psy_ui_RealRectangle dev_screenposition(psy_ui_win_ComponentImp* self)
{
	POINT pt;
	
	pt.x = 0;
	pt.y = 0;
	ClientToScreen(self->hwnd, &pt);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(pt.x, pt.y),
		psy_ui_component_offsetsizepx(self->component));
}

void dev_setposition(psy_ui_win_ComponentImp* self, psy_ui_Point topleft,
	psy_ui_Size size)
{
	const psy_ui_TextMetric* tm;
		
	tm = dev_textmetric(self);
	self->sizecachevalid = FALSE;
	SetWindowPos(self->hwnd, 0,
		(int)psy_ui_value_px(&topleft.x, tm),
		(int)psy_ui_value_px(&topleft.y, tm),
		(int)(psy_ui_value_px(&size.width, tm)),
		(int)(psy_ui_value_px(&size.height, tm)),
		SWP_NOZORDER);	
	dev_updatesize(self);	
}

psy_ui_Size dev_size(const psy_ui_win_ComponentImp* self)
{
	if (!self->sizecachevalid) {	
		psy_ui_Size rv;
		RECT rect;

		GetClientRect(self->hwnd, &rect);
		rv.width = psy_ui_value_makepx(rect.right);
		rv.height = psy_ui_value_makepx(rect.bottom);		
		((psy_ui_win_ComponentImp*)self)->sizecache = rv;
		((psy_ui_win_ComponentImp*)self)->sizecachevalid = TRUE;		
	}
	return self->sizecache;
}

void dev_updatesize(psy_ui_win_ComponentImp* self)
{	
	RECT rect;

	GetClientRect(self->hwnd, &rect);
	psy_ui_size_setpx(&self->sizecache, rect.right, rect.bottom);
	self->sizecachevalid = TRUE;
}

psy_ui_Size dev_framesize(psy_ui_win_ComponentImp* self)
{	
	RECT rect;

	GetWindowRect(self->hwnd, &rect);
	return psy_ui_size_make_px(rect.right, rect.bottom);	
}

void dev_scrollto(psy_ui_win_ComponentImp* self, intptr_t dx, intptr_t dy)
{
	ScrollWindow(self->hwnd, (int)dx, (int)dy, NULL, NULL);
	UpdateWindow(self->hwnd);
}

psy_ui_Component* dev_parent(psy_ui_win_ComponentImp* self)
{
	psy_ui_WinApp* winapp;
	psy_ui_win_ComponentImp* imp;

	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
	imp = (psy_ui_win_ComponentImp*) psy_table_at(&winapp->selfmap,
		(uintptr_t)GetParent(self->hwnd));	
	return imp ? imp->component : 0;
}

void dev_setparent(psy_ui_win_ComponentImp* self, psy_ui_Component* parent)
{
	psy_ui_WinApp* winapp;
	psy_ui_win_ComponentImp* parentimp;

	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
	parentimp = (psy_ui_win_ComponentImp*) parent->imp;
	if (parentimp) {
		SetParent(self->hwnd, parentimp->hwnd);
	}
}

void dev_insert(psy_ui_win_ComponentImp* self, psy_ui_win_ComponentImp* child,
	psy_ui_win_ComponentImp* insertafter)
{
	assert(child);

	if ((child->imp.vtable->dev_flags(&child->imp) & psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) ==
			psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) {
		psy_list_append(&self->viewcomponents, child->component);
	} else {
		SetParent(child->hwnd, self->hwnd);
		if (insertafter) {
			// psy_ui_WinApp* winapp;
			// HINSTANCE instance;
			int err = 0;
			SetWindowPos(
				child->hwnd,
				(insertafter) ? insertafter->hwnd : (HWND)NULL,
				0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE
			);
			// winapp = (psy_ui_WinApp*)app.platform;
			// psy_list_free(winapp->targetids);
			// winapp->targetids = NULL;
		}
	}
}

void dev_remove(psy_ui_win_ComponentImp* self, psy_ui_win_ComponentImp* child)
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
		// todo
	}
}

void dev_erase(psy_ui_win_ComponentImp* self, psy_ui_win_ComponentImp* child)
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
		// todo
	}
}

void dev_setorder(psy_ui_win_ComponentImp* self, psy_ui_win_ComponentImp*
	insertafter)
{
	SetWindowPos(
		self->hwnd, insertafter->hwnd,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE
	);
}

void dev_capture(psy_ui_win_ComponentImp* self)
{
	SetCapture(self->hwnd);
}

void dev_releasecapture(psy_ui_win_ComponentImp* self)
{
	ReleaseCapture();
}

void dev_invalidate(psy_ui_win_ComponentImp* self)
{
	InvalidateRect(self->hwnd, NULL, FALSE);
}

void dev_invalidaterect(psy_ui_win_ComponentImp* self,
	const psy_ui_RealRectangle* r)
{
	RECT rc;
	const psy_ui_TextMetric* tm;	

	tm = psy_ui_component_textmetric(self->component);	
	rc.left = (int)(r->left);
	rc.top = (int)(r->top);
	rc.right = (int)(r->right);
	rc.bottom = (int)(r->bottom);
	InvalidateRect(self->hwnd, &rc, FALSE);
}

void dev_update(psy_ui_win_ComponentImp* self)
{
	UpdateWindow(self->hwnd);
}

void dev_setfont(psy_ui_win_ComponentImp* self, psy_ui_Font* source)
{
	if (source) {
		HFONT hfont;
		
		hfont = ((psy_ui_win_FontImp*)(source->imp))->hfont;
		SendMessage(self->hwnd, WM_SETFONT, (WPARAM)hfont, 0);
		self->tmcachevalid = FALSE;
	}	
}

psy_List* dev_children(psy_ui_win_ComponentImp* self, int recursive)
{	
	psy_List* rv = 0;
	psy_List* p = 0;
	// if (recursive == psy_ui_RECURSIVE) {
	//	EnumChildWindows(self->hwnd, allchildenumproc, (LPARAM) &rv);		
	// } else
	{
		uintptr_t hwnd = (uintptr_t)GetWindow(self->hwnd, GW_CHILD);
		if (hwnd) {
			psy_ui_WinApp* winapp;
			psy_ui_win_ComponentImp* imp;
			psy_ui_Component* child;

			winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
			imp = psy_table_at(&winapp->selfmap, hwnd);
			child = imp ? imp->component : 0;
			if (child) {
				rv = psy_list_create(child);
				if (recursive == psy_ui_RECURSIVE) {
					psy_List* r;
					psy_List* q;

					q = psy_ui_component_children(child, recursive);
					for (r = q; r != NULL; psy_list_next(&r)) {
						psy_list_append(&rv, (psy_ui_Component*)psy_list_entry(r));
					}
					psy_list_free(q);
				}
			}
		}
		while (hwnd) {
			hwnd = (uintptr_t)GetNextWindow((HWND)hwnd, GW_HWNDNEXT);
			if (hwnd) {
				psy_ui_WinApp* winapp;
				psy_ui_win_ComponentImp* imp;
				psy_ui_Component* child;

				winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
				imp = psy_table_at(&winapp->selfmap, hwnd);
				child = imp ? imp->component : NULL;
				if (child) {
					psy_list_append(&rv, child);
					if (recursive == psy_ui_RECURSIVE) {
						psy_List* r;
						psy_List* q;

						q = psy_ui_component_children(child, recursive);
						for (r = q; r != NULL; psy_list_next(&r)) {
							psy_list_append(&rv, (psy_ui_Component*)psy_list_entry(r));
						}
						psy_list_free(q);
					}
				}
			}
		}
	}
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

void dev_enableinput(psy_ui_win_ComponentImp* self)
{
	EnableWindow(self->hwnd, 1);
}

void dev_preventinput(psy_ui_win_ComponentImp* self)
{
	EnableWindow(self->hwnd, 0);
}

bool dev_inputprevented(const psy_ui_win_ComponentImp* self)
{
	return !IsWindowEnabled(self->hwnd);
}

const psy_ui_TextMetric* dev_textmetric(const psy_ui_win_ComponentImp* self)
{
	if (!self->tmcachevalid) {	
		psy_ui_TextMetric rv;
		TEXTMETRIC tm;
		HDC hdc;
		HFONT hPrevFont = 0;
		HFONT hfont = 0;
		const psy_ui_Font* font = NULL;

		hdc = GetDC(self->hwnd);
		SaveDC(hdc);
		if (self->component) {
			font = psy_ui_component_font(self->component);
		}
		if (font) {
			hfont = ((psy_ui_win_FontImp*)font->imp)->hfont;
			if (hfont) {
				hPrevFont = SelectObject(hdc, hfont);
			}
		}
		GetTextMetrics(hdc, &tm);
		if (font) {
			if (hPrevFont) {
				SelectObject(hdc, hPrevFont);
			}
		}
		RestoreDC(hdc, -1);
		ReleaseDC(self->hwnd, hdc);
		rv.tmHeight = tm.tmHeight;
		rv.tmAscent = tm.tmAscent;
		rv.tmDescent = tm.tmDescent;
		rv.tmInternalLeading = tm.tmInternalLeading;
		rv.tmExternalLeading = tm.tmExternalLeading;
		rv.tmAveCharWidth = tm.tmAveCharWidth;
		rv.tmMaxCharWidth = tm.tmMaxCharWidth;
		rv.tmWeight = tm.tmWeight;
		rv.tmOverhang = tm.tmOverhang;
		rv.tmDigitizedAspectX = tm.tmDigitizedAspectX;
		rv.tmDigitizedAspectY = tm.tmDigitizedAspectY;
		rv.tmFirstChar = tm.tmFirstChar;
		rv.tmLastChar = tm.tmLastChar;
		rv.tmDefaultChar = tm.tmDefaultChar;
		rv.tmBreakChar = tm.tmBreakChar;
		rv.tmItalic = tm.tmItalic;
		rv.tmUnderlined = tm.tmUnderlined;
		rv.tmStruckOut = tm.tmStruckOut;
		rv.tmPitchAndFamily = tm.tmPitchAndFamily;
		rv.tmCharSet = tm.tmCharSet;
		// mutable
		((psy_ui_win_ComponentImp*)(self))->tm = rv;
		((psy_ui_win_ComponentImp*)(self))->tmcachevalid = TRUE;
	}
	return &self->tm;
}

void dev_setcursor(psy_ui_win_ComponentImp* self, psy_ui_CursorStyle
	cursorstyle)
{
	HCURSOR hc;

	switch (cursorstyle) {
	case psy_ui_CURSORSTYLE_AUTO:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_MOVE:
		hc = LoadCursor(NULL, IDC_SIZEALL);
		break;
	case psy_ui_CURSORSTYLE_NODROP:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_COL_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_ALL_SCROLL:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_POINTER:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_NOT_ALLOWED:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_ROW_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZENS);
		break;
	case psy_ui_CURSORSTYLE_CROSSHAIR:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_PROGRESS:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_E_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_NE_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_DEFAULT_TEXT:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_N_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_NW_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_HELP:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_VERTICAL_TEXT:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_S_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_SE_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_INHERIT:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_WAIT:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_W_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case psy_ui_CURSORSTYLE_SW_RESIZE:
		hc = LoadCursor(NULL, IDC_SIZEWE);
		break;
	default:
		hc = 0;
		break;
	}
	if (hc) {
		SetCursor(hc);
	}
}

void dev_starttimer(psy_ui_win_ComponentImp* self, uintptr_t id,
	uintptr_t interval)
{
	SetTimer(self->hwnd, id, (UINT)interval, 0);
}

void dev_stoptimer(psy_ui_win_ComponentImp* self, uintptr_t id)
{
	KillTimer(self->hwnd, id);
}

void dev_seticonressource(psy_ui_win_ComponentImp* self, int ressourceid)
{
	psy_ui_WinApp* winapp;
	LPARAM hicon;

	assert(self);

	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
	hicon = (intptr_t)LoadIcon(winapp->instance, MAKEINTRESOURCE(ressourceid));
	SendMessage(self->hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hicon);
	//SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hiconbig);
}

psy_ui_Size dev_textsize(psy_ui_win_ComponentImp* self, const char* text,
	psy_ui_Font* font)
{
	psy_ui_Size rv;
	psy_ui_Graphics g;
	HDC hdc;

	hdc = GetDC(self->hwnd);
	SaveDC(hdc);
	psy_ui_graphics_init(&g, hdc);
	psy_ui_setfont(&g, font);
	rv = psy_ui_textsize(&g, text);
	psy_ui_graphics_dispose(&g);
	RestoreDC(hdc, -1);
	ReleaseDC(self->hwnd, hdc);
	return rv;
}

void dev_setbackgroundcolour(psy_ui_win_ComponentImp* self, psy_ui_Colour colour)
{	
	if (self->background) {
		DeleteObject(self->background);
	}
	self->background = CreateSolidBrush(colour.value);
}

void dev_settitle(psy_ui_win_ComponentImp* self, const char* title)
{
	SetWindowText(self->hwnd, title);
}

void dev_setfocus(psy_ui_win_ComponentImp* self)
{
	SetFocus(self->hwnd);
	if (self->component) {
		self->component->vtable->onfocus(self->component);
		psy_signal_emit(&self->component->signal_focus, self, 0);
	}
}

int dev_hasfocus(psy_ui_win_ComponentImp* self)
{
	return self->hwnd == GetFocus();
}

BOOL CALLBACK allchildenumproc(HWND hwnd, LPARAM lParam)
{
	psy_List** pChildren = (psy_List**)lParam;
	psy_ui_WinApp* winapp;
	psy_ui_win_ComponentImp* imp;
	psy_ui_Component* child;

	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
	imp = psy_table_at(&winapp->selfmap, (uintptr_t)hwnd);
	child = imp ? imp->component : 0;
	if (child) {
		psy_list_append(pChildren, child);
	}
	return TRUE;
}

int windowstyle(psy_ui_win_ComponentImp* self)
{
	int rv;
#if defined(_WIN64)		
	rv = (int)GetWindowLongPtr(self->hwnd, GWL_STYLE);
#else
	rv = (int)GetWindowLong(self->hwnd, GWL_STYLE);
#endif
	return rv;
}

int windowexstyle(psy_ui_win_ComponentImp* self)
{
	int rv;
#if defined(_WIN64)		
	rv = (int)GetWindowLongPtr(self->hwnd, GWL_EXSTYLE);
#else
	rv = (int)GetWindowLong(self->hwnd, GWL_EXSTYLE);
#endif
	return rv;
}

void dev_draw(psy_ui_win_ComponentImp* self, psy_ui_Graphics* g)
{
	psy_List* p;
	psy_List* q;
	const psy_ui_TextMetric* tm;
	psy_ui_win_GraphicsImp* win_g;
	POINT origin;
	POINT org;
	psy_ui_RealRectangle clip;	
	psy_ui_Margin spacing;
	
	tm = psy_ui_component_textmetric(self->component);
	// draw background						
	if (self->component->backgroundmode != psy_ui_NOBACKGROUND) {
		psy_ui_component_drawbackground(self->component, g);
	}
	psy_ui_component_drawborder(self->component, g);
	// prepare a clip rect that can be used by a component
	// to optimize the draw amount	
	psy_ui_realrectangle_settopleft(&g->clip,
		psy_ui_realpoint_make(g->clip.left, g->clip.top));
	// add scroll coords
	tm = psy_ui_component_textmetric(self->component);
	win_g = (psy_ui_win_GraphicsImp*)g->imp;			
	// spacing
	spacing = psy_ui_component_spacing(self->component);
	if (!psy_ui_margin_iszero(&spacing)) {
		tm = psy_ui_component_textmetric(self->component);
		
		origin.x = -(int)psy_ui_value_px(&spacing.left, tm);
		origin.y = -(int)psy_ui_value_px(&spacing.top, tm);
		SetWindowOrgEx(win_g->hdc, origin.x, origin.y, NULL);
	}
	// prepare colours
	psy_ui_setcolour(g, psy_ui_component_colour(
		self->component));
	psy_ui_settextcolour(g, psy_ui_component_colour(
		self->component));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	// update graphics origin
	GetWindowOrgEx(win_g->hdc, &org);
	win_g->orgx = org.x;
	win_g->orgy = org.y;
	// call specialization methods (vtable, then signals)			
	if (self->component->vtable->ondraw) {
		self->component->vtable->ondraw(self->component, g);
	}
	psy_signal_emit(&self->component->signal_draw,
		self->component, 1, g);
	q = self->viewcomponents;
	clip = g->clip;
	for (p = q; p != NULL; psy_list_next(&p)) {		
		psy_ui_Component* component;
		
		component = (psy_ui_Component*)psy_list_entry(p);		
		if ((component->imp->vtable->dev_flags(component->imp) &
				psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) ==
				psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) {
			psy_ui_RealRectangle position;
			psy_ui_RealRectangle intersection;
			
			position = psy_ui_component_position(component);
			intersection = clip;
			if (psy_ui_realrectangle_intersection(&intersection, &position)) {
				// translate graphics clip and origin
				psy_ui_realrectangle_settopleft(&intersection,
					psy_ui_realpoint_make(
						intersection.left - position.left,
						intersection.top - position.top));
				g->clip = intersection;				
				psy_ui_setorigin(g,
					psy_ui_realpoint_make(-position.left, -position.top));
				component->imp->vtable->dev_draw(component->imp, g);
			}
		}
	}
	// graphics clip and origin
	g->clip = clip;
	psy_ui_resetorigin(g);
}

void dev_mousedown(psy_ui_win_ComponentImp* self, psy_ui_MouseEvent* ev)
{
	psy_List* p;

	for (p = self->viewcomponents; p != NULL; psy_list_next(&p)) {
		psy_ui_Component* child;
		psy_ui_RealRectangle r;

		child = (psy_ui_Component*)p->entry;
		if (psy_ui_component_visible(child)) {
			r = psy_ui_component_position(child);
			if (psy_ui_realrectangle_intersect(&r, ev->pt)) {
				ev->pt.x -= r.left;
				ev->pt.y -= r.top;
				child->imp->vtable->dev_mousedown(child->imp, ev);
				ev->pt.x += r.left;
				ev->pt.y += r.top;
				break;
			}
		}
	}	
}

void dev_mouseup(psy_ui_win_ComponentImp* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_app()->capture) {
		psy_ui_RealPoint translation;

		translation = mapcoords(self, psy_ui_app()->capture,
			self->component);
		psy_ui_realpoint_sub(&ev->pt, translation);
		psy_ui_app()->capture->vtable->onmouseup(psy_ui_app()->capture, ev);
		psy_ui_realpoint_add(&ev->pt, translation);
	} else {
		psy_List* p;

		for (p = self->viewcomponents; p != NULL; psy_list_next(&p)) {
			psy_ui_Component* child;
			psy_ui_RealRectangle r;

			child = (psy_ui_Component*)p->entry;
			if (psy_ui_component_visible(child)) {
				r = psy_ui_component_position(child);
				if (psy_ui_realrectangle_intersect(&r, ev->pt)) {
					ev->pt.x -= r.left;
					ev->pt.y -= r.top;
					child->imp->vtable->dev_mouseup(child->imp, ev);
					ev->pt.x += r.left;
					ev->pt.y += r.top;
					break;
				}
			}
		}
	}
	psy_ui_app()->mousetracking = FALSE;
}

psy_ui_RealPoint translatecoords(psy_ui_win_ComponentImp* self, psy_ui_Component* src,
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

psy_ui_RealPoint mapcoords(psy_ui_win_ComponentImp* self, psy_ui_Component* src,
	psy_ui_Component* dst)
{
	psy_ui_RealPoint rv;
	psy_ui_Component* curr;
	psy_ui_RealRectangle r;

	curr = src;
	psy_ui_realpoint_init(&rv);
	while (dst != curr && curr != NULL) {
		r = psy_ui_component_position(curr);
		psy_ui_realpoint_add(&rv, psy_ui_realrectangle_topleft(&r));
		curr = psy_ui_component_parent(curr);
	}
	return rv;
}

void dev_mousemove(psy_ui_win_ComponentImp* self, psy_ui_MouseEvent* ev)
{	
	psy_List* p;

	if (!self->viewcomponents && psy_ui_app()->hover) {
		psy_ui_app()->hover->vtable->onmouseleave(psy_ui_app()->hover);
		psy_ui_app_sethover(psy_ui_app(), NULL);
	}

	if (!psy_ui_app()->mousetracking) {
		TRACKMOUSEEVENT tme;

		self->imp.vtable->dev_mouseenter(&self->imp);
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = 200;
		tme.hwndTrack = self->hwnd;
		if (_TrackMouseEvent(&tme)) {
			psy_ui_app()->mousetracking = TRUE;
		}		
	}
	if (psy_ui_app()->capture) {
		psy_ui_Component* capture;
		psy_ui_RealPoint translation;
		
		capture = psy_ui_app()->capture;
		translation = mapcoords(self, capture, self->component);
		psy_ui_realpoint_sub(&ev->pt, translation);			
		capture->vtable->onmousemove(capture, ev);
		psy_ui_realpoint_add(&ev->pt, translation);		
	} else {
		bool intersect;

		intersect = FALSE;
		for (p = self->viewcomponents; p != NULL; psy_list_next(&p)) {
			psy_ui_Component* child;
			psy_ui_RealRectangle r;

			child = (psy_ui_Component*)p->entry;
			if (psy_ui_component_visible(child)) {
				r = psy_ui_component_position(child);
				intersect = psy_ui_realrectangle_intersect(&r, ev->pt);
				if (intersect) {
					psy_ui_realpoint_sub(&ev->pt, psy_ui_realrectangle_topleft(&r));
					child->imp->vtable->dev_mousemove(child->imp, ev);
					psy_ui_realpoint_add(&ev->pt, psy_ui_realrectangle_topleft(&r));
					break;
				}
			}
		}
		if (!intersect) {
			if (psy_ui_app()->hover != self->component) {
				psy_ui_Component* hover;

				hover = psy_ui_app()->hover;				
				if (hover) {
					hover->vtable->onmouseleave(hover);
				}
				self->component->vtable->onmouseenter(self->component);
				psy_ui_app_sethover(psy_ui_app(), self->component);
			}
		}
	}
	if (ev->bubble) {
		self->component->vtable->onmousemove(self->component, ev);
		psy_signal_emit(&self->component->signal_mousemove,
			self->component, 1, ev);
	}

}

void dev_mousedoubleclick(psy_ui_win_ComponentImp* self, psy_ui_MouseEvent* ev)
{
	psy_List* p;	

	for (p = self->viewcomponents; p != NULL; psy_list_next(&p)) {
		psy_ui_Component* child;
		psy_ui_RealRectangle r;

		child = (psy_ui_Component*)p->entry;
		r = psy_ui_component_position(child);
		if (psy_ui_component_visible(child)) {
			if (psy_ui_realrectangle_intersect(&r, ev->pt)) {
				ev->pt.x -= r.left;
				ev->pt.y -= r.top;
				child->imp->vtable->dev_mousedoubleclick(child->imp, ev);
				ev->pt.x += r.left;
				ev->pt.y += r.top;
				break;
			}
		}
	}	
}

void dev_mouseenter(psy_ui_win_ComponentImp* self)
{
//	if (psy_ui_app()->hover) {
//		psy_ui_app()->hover->vtable->onmouseleave(psy_ui_app()->hover);
//	}
//	psy_ui_app_sethover(psy_ui_app(), NULL);
	self->component->vtable->onmouseenter(self->component);
	psy_signal_emit(&self->component->signal_mouseenter,
		self->component, 0);
}

void dev_mouseleave(psy_ui_win_ComponentImp* self)
{	
	psy_ui_app()->mousetracking = FALSE;
	if (psy_ui_app()->hover) {
		psy_ui_Component* hover;

		hover = psy_ui_app()->hover;
		hover->vtable->onmouseleave(hover);
		psy_signal_emit(&hover->signal_mouseleave, hover, 0);
		psy_ui_app_sethover(psy_ui_app(), NULL);		
	}
	self->component->vtable->onmouseleave(self->component);		
	psy_signal_emit(&self->component->signal_mouseleave, self->component, 0);		
}

#endif

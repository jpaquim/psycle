/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

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
#include <tchar.h>
/* common control header */
#include <commctrl.h>
/* details */
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

/* prototypes */
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
static psy_ui_RealRectangle dev_position(const psy_ui_win_ComponentImp*);
static psy_ui_RealRectangle dev_screenposition(const psy_ui_win_ComponentImp*);
static void dev_setposition(psy_ui_win_ComponentImp*, psy_ui_Point topleft,
	psy_ui_Size);
static psy_ui_Size dev_size(const psy_ui_win_ComponentImp*);
static void dev_updatesize(psy_ui_win_ComponentImp*);
static void dev_applyposition(psy_ui_win_ComponentImp*);
static void dev_updatetopleft(psy_ui_win_ComponentImp*);
static psy_ui_Size dev_framesize(psy_ui_win_ComponentImp*);
static void dev_scrollto(psy_ui_win_ComponentImp*, intptr_t dx, intptr_t dy,
	const psy_ui_RealRectangle*);
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
static psy_List* dev_children(psy_ui_win_ComponentImp*, int recursive);
static void dev_enableinput(psy_ui_win_ComponentImp*);
static void dev_preventinput(psy_ui_win_ComponentImp*);
static bool dev_inputprevented(const psy_ui_win_ComponentImp* self);
static void dev_setcursor(psy_ui_win_ComponentImp*, psy_ui_CursorStyle);
static void dev_seticonressource(psy_ui_win_ComponentImp*, int ressourceid);
static const psy_ui_TextMetric* dev_textmetric(const psy_ui_win_ComponentImp*);
static psy_ui_Size dev_textsize(psy_ui_win_ComponentImp*, const char* text,
	psy_ui_Font*);
static void dev_setbackgroundcolour(psy_ui_win_ComponentImp*, psy_ui_Colour);
static void dev_settitle(psy_ui_win_ComponentImp*, const char* title);
static void dev_setfocus(psy_ui_win_ComponentImp*);
static int dev_hasfocus(psy_ui_win_ComponentImp*);

static void dev_clear(psy_ui_win_ComponentImp*);
static void dev_initialized(psy_ui_win_ComponentImp* self) { }
static uintptr_t dev_platform_handle(psy_ui_win_ComponentImp* self) { return (uintptr_t)self->hwnd; }
static bool dev_issystem(psy_ui_win_ComponentImp* self) { return TRUE; }

static void dev_setshowfullscreen(psy_ui_win_ComponentImp*, bool fullscreen);
static void dev_showtaskbar(psy_ui_win_ComponentImp*, bool show);

/* vtable */
static psy_ui_ComponentImpVTable vtable;
static bool vtable_initialized = FALSE;

static void win_imp_vtable_init(psy_ui_win_ComponentImp* self)
{
	if (!vtable_initialized) {
		vtable = *self->imp.vtable;
		vtable.dev_dispose =
			(psy_ui_fp_componentimp_dev_dispose)
			dev_dispose;
		vtable.dev_destroy =
			(psy_ui_fp_componentimp_dev_destroy)
			dev_destroy;
		vtable.dev_show =
			(psy_ui_fp_componentimp_dev_show)
			dev_show;
		vtable.dev_showstate =
			(psy_ui_fp_componentimp_dev_showstate)
			dev_showstate;
		vtable.dev_hide =
			(psy_ui_fp_componentimp_dev_hide)
			dev_hide;
		vtable.dev_visible =
			(psy_ui_fp_componentimp_dev_visible)
			dev_visible;
		vtable.dev_drawvisible =
			(psy_ui_fp_componentimp_dev_drawvisible)
			dev_drawvisible;
		vtable.dev_move =
			(psy_ui_fp_componentimp_dev_move)
			dev_move;
		vtable.dev_resize =
			(psy_ui_fp_componentimp_dev_resize)
			dev_resize;
		vtable.dev_clientresize =
			(psy_ui_fp_componentimp_dev_clientresize)
			dev_clientresize;
		vtable.dev_position =
			(psy_ui_fp_componentimp_dev_position)
			dev_position;
		vtable.dev_screenposition =
			(psy_ui_fp_componentimp_dev_screenposition)
			dev_screenposition;
		vtable.dev_setposition =
			(psy_ui_fp_componentimp_dev_setposition)
			dev_setposition;
		vtable.dev_size =
			(psy_ui_fp_componentimp_dev_size)
			dev_size;
		vtable.dev_updatesize =
			(psy_ui_fp_componentimp_dev_size)
			dev_updatesize;
		vtable.dev_framesize =
			(psy_ui_fp_componentimp_dev_framesize)
			dev_framesize;
		vtable.dev_scrollto =
			(psy_ui_fp_componentimp_dev_scrollto)
			dev_scrollto;
		vtable.dev_parent =
			(psy_ui_fp_componentimp_dev_parent)
			dev_parent;
		vtable.dev_setparent =
			(psy_ui_fp_componentimp_dev_setparent)
			dev_setparent;
		vtable.dev_insert =
			(psy_ui_fp_componentimp_dev_insert)
			dev_insert;
		vtable.dev_remove =
			(psy_ui_fp_componentimp_dev_remove)
			dev_remove;
		vtable.dev_erase =
			(psy_ui_fp_componentimp_dev_erase)
			dev_erase;
		vtable.dev_capture =
			(psy_ui_fp_componentimp_dev_capture)
			dev_capture;
		vtable.dev_releasecapture =
			(psy_ui_fp_componentimp_dev_releasecapture)
			dev_releasecapture;
		vtable.dev_invalidate =
			(psy_ui_fp_componentimp_dev_invalidate)
			dev_invalidate;
		vtable.dev_invalidaterect =
			(psy_ui_fp_componentimp_dev_invalidaterect)
			dev_invalidaterect;
		vtable.dev_update =
			(psy_ui_fp_componentimp_dev_update)
			dev_update;
		vtable.dev_applyposition =
			(psy_ui_fp_componentimp_dev_applyposition)
			dev_applyposition;
		vtable.dev_setfont =
			(psy_ui_fp_componentimp_dev_setfont)
			dev_setfont;
		vtable.dev_children =
			(psy_ui_fp_componentimp_dev_children)
			dev_children;
		vtable.dev_enableinput =
			(psy_ui_fp_componentimp_dev_enableinput)
			dev_enableinput;
		vtable.dev_preventinput =
			(psy_ui_fp_componentimp_dev_preventinput)
			dev_preventinput;
		vtable.dev_inputprevented =
			(psy_ui_fp_componentimp_dev_inputprevented)
			dev_inputprevented;		
		vtable.dev_setcursor =
			(psy_ui_fp_componentimp_dev_setcursor)
			dev_setcursor;		
		vtable.dev_seticonressource =
			(psy_ui_fp_componentimp_dev_seticonressource)
			dev_seticonressource;
		vtable.dev_textmetric =
			(psy_ui_fp_componentimp_dev_textmetric)
			dev_textmetric;
		vtable.dev_textsize =
			(psy_ui_fp_componentimp_dev_textsize)
			dev_textsize;
		vtable.dev_setbackgroundcolour =
			(psy_ui_fp_componentimp_dev_setbackgroundcolour)
			dev_setbackgroundcolour;
		vtable.dev_settitle =
			(psy_ui_fp_componentimp_dev_settitle)
			dev_settitle;
		vtable.dev_setfocus =
			(psy_ui_fp_componentimp_dev_setfocus)
			dev_setfocus;
		vtable.dev_hasfocus =
			(psy_ui_fp_componentimp_dev_hasfocus)
			dev_hasfocus;
		vtable.dev_clear =
			(psy_ui_fp_componentimp_dev_clear)
			dev_clear;
		vtable.dev_initialized =
			(psy_ui_fp_componentimp_dev_initialized)
			dev_initialized;
		vtable.dev_platform_handle =
			(psy_ui_fp_componentimp_dev_platform_handle)
			dev_platform_handle;
		vtable.dev_issystem =
			(psy_ui_fp_componentimp_dev_issystem)
			dev_issystem;
		vtable_initialized = TRUE;
	}
}
/* implementation */
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
	self->topleftcachevalid = FALSE;
	self->sizecachevalid = FALSE;	
	self->dbg = 0;	
	self->visible = parent ? TRUE : FALSE;
	self->viewcomponents = NULL;
	self->fullscreen = FALSE;
	self->restore_style = 0;
	self->restore_exstyle = 0;
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
	self->visible = (dwStyle & WS_VISIBLE) == WS_VISIBLE;
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

/* win32 implementation method for psy_ui_Component */
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

	if (self->component) {
		psy_List* c;
		
		c = psy_ui_component_children(self->component, psy_ui_NONRECURSIVE);
		for (p = c; p != NULL; p = q) {
			psy_ui_Component* component;			

			q = p->next;
			component = (psy_ui_Component*)psy_list_entry(p);			
			psy_ui_component_destroy(component);			
		}
		psy_list_free(c);
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
	if (state == 20) {
		dev_setshowfullscreen(self, !self->fullscreen);
	} else {
		ShowWindow(self->hwnd, state);
		UpdateWindow(self->hwnd);
	}
}

void dev_setshowfullscreen(psy_ui_win_ComponentImp* self, bool fullscreen)	
{
	RECT fullrect = { 0 };	

	dev_showtaskbar(self, !fullscreen);				
	SetRect(&fullrect, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	if (fullscreen) {				
		self->restore_style = windowstyle(self);
		self->restore_exstyle = windowexstyle(self);		
		SetWindowLong(self->hwnd, GWL_STYLE,
			self->restore_style & ~(WS_CAPTION | WS_THICKFRAME));
		SetWindowLong(self->hwnd, GWL_EXSTYLE,
			self->restore_exstyle & ~(WS_EX_DLGMODALFRAME |
				WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
		SetWindowPos(self->hwnd, NULL, 0, 0,
			GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN),				
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		ShowWindow(self->hwnd, SW_SHOW);
		UpdateWindow(self->hwnd);
	} else {
		SetWindowLong(self->hwnd, GWL_STYLE, self->restore_style);
		SetWindowLong(self->hwnd, GWL_EXSTYLE, self->restore_exstyle);
		SetWindowPos(self->hwnd, NULL, 0, 0,
			GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN),
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		UpdateWindow(self->hwnd);
	}
	self->fullscreen = fullscreen;
	self->sizecachevalid = FALSE;
	self->topleftcachevalid = FALSE;
}

void dev_showtaskbar(psy_ui_win_ComponentImp* self, bool show)
{
	HWND taskbar = FindWindow(_T("Shell_TrayWnd"), NULL);
	HWND start = FindWindow(_T("Button"), NULL);

	if (taskbar != NULL) {
		ShowWindow(taskbar, show ? SW_SHOW : SW_HIDE);
		UpdateWindow(taskbar);
	}
	if (start != NULL) {
		/* Vista */
		ShowWindow(start, show ? SW_SHOW : SW_HIDE);
		UpdateWindow(start);
	}
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
	/* return IsWindowVisible(self->hwnd); */
}

int dev_drawvisible(psy_ui_win_ComponentImp* self)
{
	return IsWindowVisible(self->hwnd);
}

void dev_move(psy_ui_win_ComponentImp* self, psy_ui_Point topleft)
{
	self->topleftcachevalid = FALSE;
	SetWindowPos(self->hwnd, NULL,
		(int)psy_ui_value_px(&topleft.x, dev_textmetric(self), NULL),
		(int)psy_ui_value_px(&topleft.y, dev_textmetric(self), NULL),
		0, 0,
		SWP_NOZORDER | SWP_NOSIZE);
	self->topleftcache = topleft;
	self->topleftcachevalid = TRUE;
}

void dev_resize(psy_ui_win_ComponentImp* self, psy_ui_Size size)
{	
	self->sizecachevalid = FALSE;
	SetWindowPos(self->hwnd, NULL,
		0, 0,
		(int)psy_ui_value_px(&size.width, dev_textmetric(self), NULL),
		(int)psy_ui_value_px(&size.height, dev_textmetric(self), NULL),
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
			psy_ui_value_make_px((double)rc.right - (double)rc.left),
			psy_ui_value_make_px((double)rc.bottom - (double)rc.top)));
}


psy_ui_RealRectangle dev_position(const psy_ui_win_ComponentImp* self)
{
	psy_ui_RealRectangle rv;
	RECT rc;
	POINT pt;

	/*if (self->sizecachevalid && self->topleftcachevalid) {
		return psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_value_px(&self->topleftcache.x, dev_textmetric(self), &self->sizecache),
				psy_ui_value_px(&self->topleftcache.y, dev_textmetric(self), &self->sizecache)),
			psy_ui_realsize_make(
				psy_ui_value_px(&self->sizecache.width, dev_textmetric(self), &self->sizecache),
				psy_ui_value_px(&self->sizecache.height, dev_textmetric(self), &self->sizecache)));
	}*/	
	GetWindowRect(self->hwnd, &rc);
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(GetParent(self->hwnd), &pt);
	GetClientRect(self->hwnd, &rc);
	rv.left = pt.x;
	rv.top = pt.y;
	rv.right = rv.left + rc.right;
	rv.bottom = rv.top + rc.bottom;
	dev_updatetopleft((psy_ui_win_ComponentImp*)self);
	return rv;		
}

psy_ui_RealRectangle dev_screenposition(const psy_ui_win_ComponentImp* self)
{
	POINT pt;
	
	pt.x = 0;
	pt.y = 0;
	ClientToScreen(self->hwnd, &pt);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(pt.x, pt.y),
		psy_ui_component_scrollsize_px(self->component));
}

void dev_setposition(psy_ui_win_ComponentImp* self, psy_ui_Point topleft,
	psy_ui_Size size)
{
	const psy_ui_TextMetric* tm;	
	
	tm = dev_textmetric(self);
	self->sizecachevalid = FALSE;
	self->topleftcachevalid = FALSE;	
	if (psy_ui_size_has_percent(&size)) {
		psy_ui_Size parentsize;

		if (psy_ui_component_parent_const(self->component)) {
			parentsize = psy_ui_component_scrollsize(psy_ui_component_parent_const(self->component));
		} else {
			parentsize = psy_ui_component_scrollsize(self->component);
		}
		SetWindowPos(self->hwnd, 0,
			(int)psy_ui_value_px(&topleft.x, tm, &parentsize),
			(int)psy_ui_value_px(&topleft.y, tm, &parentsize),
			(int)(psy_ui_value_px(&size.width, tm, &parentsize)),
			(int)(psy_ui_value_px(&size.height, tm, &parentsize)),
			/* SWP_NOREDRAW | SWP_NOZORDER); */
			SWP_NOZORDER);
	} else {
		if (!psy_ui_app()->setpositioncacheonly) {
			SetWindowPos(self->hwnd, 0,
				(int)psy_ui_value_px(&topleft.x, tm, NULL),
				(int)psy_ui_value_px(&topleft.y, tm, NULL),
				(int)(psy_ui_value_px(&size.width, tm, NULL)),
				(int)(psy_ui_value_px(&size.height, tm, NULL)),
				/* SWP_NOREDRAW | SWP_NOZORDER); */
				SWP_NOZORDER);
		} else {
			self->topleftcache = topleft;			
			self->sizecache = size;
			self->sizecachevalid = TRUE;
			return;
		}
	}	
	dev_updatesize(self);
	dev_updatetopleft(self);
}

psy_ui_Size dev_size(const psy_ui_win_ComponentImp* self)
{
	if (!self->sizecachevalid) {
		psy_ui_Size rv;
		RECT rect;

		GetClientRect(self->hwnd, &rect);
		psy_ui_size_init_px(&rv,
			rect.right, rect.bottom);		
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

void dev_updatetopleft(psy_ui_win_ComponentImp* self)
{	
	RECT rc;
	POINT pt;

	GetWindowRect(self->hwnd, &rc);
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(GetParent(self->hwnd), &pt);
	GetClientRect(self->hwnd, &rc);
	self->topleftcache = psy_ui_point_make_px((double)pt.x, (double)pt.y);
	self->topleftcachevalid = TRUE;
}

psy_ui_Size dev_framesize(psy_ui_win_ComponentImp* self)
{	
	RECT rect;

	GetWindowRect(self->hwnd, &rect);
	return psy_ui_size_make_px(rect.right, rect.bottom);	
}

void dev_scrollto(psy_ui_win_ComponentImp* self, intptr_t dx, intptr_t dy,
	const psy_ui_RealRectangle* r)
{
	RECT* prc;
	RECT rc;

	prc = NULL;
	if (r) {
		rc.left = (int)(r->left);
		rc.top = (int)(r->top);
		rc.right = (int)(r->right);
		rc.bottom = (int)(r->bottom);
		prc = &rc;
	}
	ScrollWindow(self->hwnd, (int)dx, (int)dy, prc, prc);
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
			int err = 0;
			SetWindowPos(
				child->hwnd,
				(insertafter) ? insertafter->hwnd : (HWND)NULL,
				0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE
			);			
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
		/* todo */
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
		/* todo */
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

void dev_applyposition(psy_ui_win_ComponentImp* self)
{
	const psy_ui_TextMetric* tm;
	int left;
	int top;
	int w;
	int h;
	RECT rc;
	POINT pt;

	tm = dev_textmetric(self);
	left = (int)psy_ui_value_px(&self->topleftcache.x, tm, NULL);
	top = (int)psy_ui_value_px(&self->topleftcache.y, tm, NULL);
	w = (int)(psy_ui_value_px(&self->sizecache.width, tm, NULL));
	h = (int)(psy_ui_value_px(&self->sizecache.height, tm, NULL));
	GetWindowRect(self->hwnd, &rc);
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(GetParent(self->hwnd), &pt);
	GetClientRect(self->hwnd, &rc);
	if (left != pt.x || top != pt.y || w != rc.right || h != rc.bottom) {
		SetWindowPos(self->hwnd, 0,
			left, top, w, h,			
			SWP_NOREDRAW | SWP_NOZORDER);
	}
}

void dev_setfont(psy_ui_win_ComponentImp* self, psy_ui_Font* source)
{
	if (source) {
		HFONT hfont;
		
		hfont = ((psy_ui_win_FontImp*)(source->imp))->hfont;
		SendMessage(self->hwnd, WM_SETFONT, (WPARAM)hfont, 0);		
	}	
}

psy_List* dev_children(psy_ui_win_ComponentImp* self, int recursive)
{	
	psy_List* rv = 0;
	psy_List* p = 0;
	
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
	if (self->component) {
		const psy_ui_Font* font;

		font = psy_ui_component_font(self->component);
		if (font) {
			return psy_ui_font_textmetric(font);
		}
	}
	return NULL;
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
		hc = LoadCursor(NULL, IDC_NO);
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
	case psy_ui_CURSORSTYLE_GRAB:
		hc = LoadCursor(NULL, IDC_HAND);
		break;
	default:
		hc = 0;
		break;
	}
	if (hc) {
		SetCursor(hc);
	}
}

void dev_seticonressource(psy_ui_win_ComponentImp* self, int ressourceid)
{
	psy_ui_WinApp* winapp;
	LPARAM hicon;

	assert(self);

	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
	hicon = (intptr_t)LoadIcon(winapp->instance, MAKEINTRESOURCE(ressourceid));
	SendMessage(self->hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hicon);	
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
	rv = psy_ui_textsize(&g, text, psy_strlen(text));
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
	self->background = CreateSolidBrush(psy_ui_colour_colorref(&colour));
}

void dev_settitle(psy_ui_win_ComponentImp* self, const char* title)
{
	SetWindowText(self->hwnd, title);
}

void dev_setfocus(psy_ui_win_ComponentImp* self)
{
	if (self->hwnd != GetFocus()) {
		SetFocus(self->hwnd);
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

#endif /* PSYCLE_TK_WIN32 */

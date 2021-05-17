// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uix11componentimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT

#include "uix11impfactory.h"
#include "uicomponent.h"
#include "uix11fontimp.h"
#include "uix11bitmapimp.h"
#include "uix11graphicsimp.h"
#include "uiapp.h"
#include "uix11app.h"
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#include <stdlib.h>

static void dev_rec_children(psy_ui_x11_ComponentImp*,
	psy_List** children);
static int windowstyle(psy_ui_x11_ComponentImp*);
static int windowexstyle(psy_ui_x11_ComponentImp*);

static void psy_ui_x11_component_create_window(psy_ui_x11_ComponentImp*,
	psy_ui_x11_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand);

// prototypes
static void dev_dispose(psy_ui_x11_ComponentImp*);
static void dev_destroy(psy_ui_x11_ComponentImp*);
static void dev_show(psy_ui_x11_ComponentImp*);
static void dev_showstate(psy_ui_x11_ComponentImp*, int state);
static void dev_hide(psy_ui_x11_ComponentImp*);
static int dev_visible(psy_ui_x11_ComponentImp*);
static int dev_drawvisible(psy_ui_x11_ComponentImp*);
static void dev_move(psy_ui_x11_ComponentImp*, psy_ui_Point origin);
static void dev_resize(psy_ui_x11_ComponentImp*, psy_ui_Size);
static void dev_clientresize(psy_ui_x11_ComponentImp*, intptr_t width, intptr_t height);
static psy_ui_RealRectangle dev_position(const psy_ui_x11_ComponentImp*);
static psy_ui_RealRectangle dev_screenposition(const psy_ui_x11_ComponentImp*);
static void dev_setposition(psy_ui_x11_ComponentImp*, psy_ui_Point topleft,
	psy_ui_Size);
static psy_ui_Size dev_size(const psy_ui_x11_ComponentImp*);
static void dev_updatesize(psy_ui_x11_ComponentImp*);
static psy_ui_Size dev_framesize(psy_ui_x11_ComponentImp*);
static void dev_scrollto(psy_ui_x11_ComponentImp*, intptr_t dx, intptr_t dy);
static psy_ui_Component* dev_parent(psy_ui_x11_ComponentImp*);
static void dev_setparent(psy_ui_x11_ComponentImp* self, psy_ui_Component* parent);
static void dev_insert(psy_ui_x11_ComponentImp* self, psy_ui_x11_ComponentImp* child,
	psy_ui_x11_ComponentImp* insertafter);
static void dev_remove(psy_ui_x11_ComponentImp*, psy_ui_x11_ComponentImp* child);
static void dev_erase(psy_ui_x11_ComponentImp*, psy_ui_x11_ComponentImp* child);
static void dev_setorder(psy_ui_x11_ComponentImp*, psy_ui_x11_ComponentImp*
	insertafter);
static void dev_capture(psy_ui_x11_ComponentImp*);
static void dev_releasecapture(psy_ui_x11_ComponentImp*);
static void dev_invalidate(psy_ui_x11_ComponentImp*);
static void dev_invalidaterect(psy_ui_x11_ComponentImp*,
	const psy_ui_RealRectangle*);
static void dev_update(psy_ui_x11_ComponentImp*);
static void dev_setfont(psy_ui_x11_ComponentImp*, psy_ui_Font*);
static psy_List* dev_children(psy_ui_x11_ComponentImp*, int recursive);
static void dev_enableinput(psy_ui_x11_ComponentImp*);
static void dev_preventinput(psy_ui_x11_ComponentImp*);
static bool dev_inputprevented(const psy_ui_x11_ComponentImp* self);
static void dev_setcursor(psy_ui_x11_ComponentImp*, psy_ui_CursorStyle);
static void dev_starttimer(psy_ui_x11_ComponentImp*, uintptr_t id, uintptr_t
	interval);
static void dev_stoptimer(psy_ui_x11_ComponentImp*, uintptr_t id);
static void dev_seticonressource(psy_ui_x11_ComponentImp*, int ressourceid);
static const psy_ui_TextMetric* dev_textmetric(const psy_ui_x11_ComponentImp*);
static psy_ui_Size dev_textsize(psy_ui_x11_ComponentImp*, const char* text,
	psy_ui_Font*);
static void dev_setbackgroundcolour(psy_ui_x11_ComponentImp*, psy_ui_Colour);
static void dev_settitle(psy_ui_x11_ComponentImp*, const char* title);
static void dev_setfocus(psy_ui_x11_ComponentImp*);
static int dev_hasfocus(psy_ui_x11_ComponentImp*);

static void dev_clear(psy_ui_x11_ComponentImp*);
static void dev_draw(psy_ui_x11_ComponentImp*, psy_ui_Graphics*);
static void dev_mousedown(psy_ui_x11_ComponentImp*, psy_ui_MouseEvent*);
static void dev_mouseup(psy_ui_x11_ComponentImp*, psy_ui_MouseEvent*);
static void dev_mousemove(psy_ui_x11_ComponentImp*, psy_ui_MouseEvent*);
static void dev_mousedoubleclick(psy_ui_x11_ComponentImp*, psy_ui_MouseEvent*);
static void dev_mouseenter(psy_ui_x11_ComponentImp*);
static void dev_mouseleave(psy_ui_x11_ComponentImp*);
static void dev_initialized(psy_ui_x11_ComponentImp* self) { }

static psy_ui_RealPoint translatecoords(psy_ui_x11_ComponentImp*, psy_ui_Component* src,
	psy_ui_Component* dst);
static psy_ui_RealPoint mapcoords(psy_ui_x11_ComponentImp* self, psy_ui_Component* src,
	psy_ui_Component* dst);

// VTable init
static psy_ui_ComponentImpVTable vtable;
static bool vtable_initialized = FALSE;

static void xt_imp_vtable_init(psy_ui_x11_ComponentImp* self)
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
			(psy_ui_fp_componentimp_dev_updatesize)
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
		vtable.dev_starttimer =
			(psy_ui_fp_componentimp_dev_starttimer)
			dev_starttimer;
		vtable.dev_stoptimer =
			(psy_ui_fp_componentimp_dev_stoptimer)
			dev_stoptimer;
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
		vtable.dev_draw =
			(psy_ui_fp_componentimp_dev_draw)
			dev_draw;
		vtable.dev_mousedown =
			(psy_ui_fp_componentimp_dev_mouseevent)
			dev_mousedown;
		vtable.dev_mouseup =
			(psy_ui_fp_componentimp_dev_mouseevent)
			dev_mouseup;
		vtable.dev_mousemove =
			(psy_ui_fp_componentimp_dev_mouseevent)
			dev_mousemove;
		vtable.dev_mousedoubleclick =
			(psy_ui_fp_componentimp_dev_mouseevent)
			dev_mousedoubleclick;
		vtable.dev_mouseenter =
			(psy_ui_fp_componentimp_dev_mouseenter)
			dev_mouseenter;
		vtable.dev_mouseleave =
			(psy_ui_fp_componentimp_dev_mouseleave)
			dev_mouseleave;
		vtable.dev_initialized =
			(psy_ui_fp_componentimp_dev_initialized)
			dev_initialized;
		vtable_initialized = TRUE;
	}
}

void psy_ui_x11_componentimp_init(psy_ui_x11_ComponentImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand)
{	
	psy_ui_x11_ComponentImp* parent_imp;	

	psy_ui_componentimp_init(&self->imp);
	xt_imp_vtable_init(self);
	self->imp.vtable = &vtable;
	self->component = component;
	self->backgroundcolor = psy_ui_colour_make(0);
	self->winid = -1;
	self->hwnd = 0;
	// self->wndproc = 0;
	// self->preventwmchar = 0;
	self->sizecachevalid = FALSE;
	self->tmcachevalid = FALSE;
	self->dbg = 0;
	self->visible = parent ? TRUE : FALSE;
	self->viewcomponents = NULL;
	psy_ui_realrectangle_init(&self->exposearea);
	parent_imp = parent
		? (psy_ui_x11_ComponentImp*)parent
		: NULL;	
	psy_ui_x11_component_create_window(self, parent_imp, classname, x, y,
		width, height, dwStyle, usecommand);
	if (self->hwnd) {
//		psy_ui_x11_component_init_wndproc(self, classname);
	}
}

void psy_ui_x11_component_create_window(psy_ui_x11_ComponentImp* self,
	psy_ui_x11_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand)
{	
	psy_ui_X11App* x11app;
	//
	int err = 0;
			
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;	
	self->prev_w = width;
	self->prev_h = height;
	self->d_backBuf = 0;
	if (parent == 0) {
		XSetWindowAttributes xAttr;
		unsigned long xAttrMask = CWBackPixel;
		
		xAttr.background_pixel = 0x00232323;		     
        self->hwnd = XCreateWindow(
			x11app->dpy, XDefaultRootWindow(x11app->dpy),
				x, y, width, height, 0,
				CopyFromParent, CopyFromParent, x11app->visual,
				xAttrMask, &xAttr);		      
        //self->hwnd = XCreateSimpleWindow(
		//	x11app->dpy, XDefaultRootWindow(x11app->dpy),
		//		x, y, width, height, 4, 0, 0x00232323);
		XSelectInput(x11app->dpy, self->hwnd,
			ExposureMask | KeyPressMask | KeyReleaseMask |
			StructureNotifyMask |
			EnterWindowMask | LeaveWindowMask);
		self->mapped = FALSE;
    } else {   
		XSetWindowAttributes xAttr;
		unsigned long xAttrMask = CWBackPixel;
		
		xAttr.background_pixel = 0x00232323;		     
        self->hwnd = XCreateWindow(
			x11app->dpy, parent->hwnd,
				x, y, width, height, 0,
				CopyFromParent, CopyFromParent, x11app->visual,
				xAttrMask, &xAttr);
				// 0, 0x00232323);
			XMapWindow(x11app->dpy, self->hwnd);
			XSelectInput(x11app->dpy, self->hwnd,
				KeyPressMask | KeyReleaseMask |
				ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
				ExposureMask | StructureNotifyMask |
				EnterWindowMask | LeaveWindowMask);
			self->mapped = TRUE;        
    }
    self->parent = parent;    
    if (self->parent) {
		psy_list_free(self->parent->children_nonrec_cache);
		self->parent->children_nonrec_cache = NULL;
	}
    self->children_nonrec_cache = NULL;
    if (self->hwnd) {		       
        GC gc;
		PlatformXtGC xgc;
		psy_ui_Graphics g;
		XColor dummy;
		XFontStruct *fontinfo;
		Window root;
		Window window;
		int     screen;	
		
		XSetWMProtocols(x11app->dpy, self->hwnd, &x11app->wmDeleteMessage, 1);
		if (x11app->dbe) {		
			self->d_backBuf = XdbeAllocateBackBufferName(x11app->dpy, self->hwnd,
				XdbeBackground);
			xgc.window = self->d_backBuf;
		} else {
			xgc.window = self->hwnd;
		}
        gc = XCreateGC(x11app->dpy, xgc.window, 0, NULL);
        xgc.display =x11app->dpy;		
		xgc.gc = gc;
		xgc.visual = x11app->visual;		
		psy_ui_graphics_init(&self->g, &xgc);	
    }
	
	//self->hwnd = CreateWindow(
		//classname,
		//NULL,
		//dwStyle,
		//x, y, width, height,
		//parent ? parent->hwnd : (HWND)NULL,
		//usecommand ? (HMENU) winapp->winid : NULL,
		//instance,
		//NULL);
	if (self->hwnd == 0) {
		printf("Failed To Create Component\n");
		//char text[256];
		//unsigned long err;

		//err = GetLastError();
		//psy_snprintf(text, 256, "Failed To Create Component (Error %u)", err);
		//MessageBox(NULL, text, "Error",
			//MB_OK | MB_ICONERROR);
		err = 1;
	} else {
		psy_table_insert(&x11app->selfmap, (uintptr_t) self->hwnd, self);
	}
	//if (err == 0 && usecommand) {
		//psy_table_insert(&winapp->winidmap, winapp->winid, self);
		//++winapp->winid;
	//}
}

//HINSTANCE psy_ui_x11_component_instance(psy_ui_x11_ComponentImp* parent)
//{
	//HINSTANCE rv;
	//psy_ui_WinApp* winapp;

	//winapp = (psy_ui_WinApp*) app.platform;
	//if (parent) {
//#if defined(_WIN64)		
		//rv = (HINSTANCE)GetWindowLongPtr(parent->hwnd, GWLP_HINSTANCE);
//#else
		//rv = (HINSTANCE)GetWindowLong(parent->hwnd, GWL_HINSTANCE);
//#endif
	//} else {
		//rv = winapp->instance;
	//}
	//return rv;
// }

//void psy_ui_x11_component_init_wndproc(psy_ui_x11_ComponentImp* self,
	//LPCSTR classname)
//{
	//psy_ui_WinApp* winapp;

	//winapp = (psy_ui_WinApp*) app.platform;	
//#if defined(_WIN64)		
	//self->wndproc = (winproc) GetWindowLongPtr(self->hwnd, GWLP_WNDPROC);
//#else		
	//self->wndproc = (winproc) GetWindowLong(self->hwnd, GWL_WNDPROC);
//#endif
	//if (classname != winapp->componentclass && classname != winapp->appclass) {
//#if defined(_WIN64)		
		//SetWindowLongPtr(self->hwnd, GWLP_WNDPROC, (LONG_PTR)winapp->comwinproc);
//#else	
		//SetWindowLong(self->hwnd, GWL_WNDPROC, (LONG)winapp->comwinproc);
//#endif
	//}
// }

// xt implementation method for psy_ui_Component
void dev_dispose(psy_ui_x11_ComponentImp* self)
{	
	psy_ui_X11App* x11app;		
	psy_List* p;
	psy_List* q;	
	
    x11app = (psy_ui_X11App*)psy_ui_app()->imp;    
	psy_ui_componentimp_dispose(&self->imp);
	psy_ui_graphics_dispose(&self->g);    
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
	psy_list_free(self->children_nonrec_cache);
	self->children_nonrec_cache = NULL;	
}

void dev_clear(psy_ui_x11_ComponentImp* self)
{
	psy_List* p;
	psy_List* q;
	psy_ui_X11App* x11app;		

    x11app = (psy_ui_X11App*)psy_ui_app()->imp;
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
	psy_list_free(self->children_nonrec_cache);
	self->children_nonrec_cache = NULL;
}

psy_ui_x11_ComponentImp* psy_ui_x11_componentimp_alloc(void)
{
	return (psy_ui_x11_ComponentImp*)malloc(sizeof(psy_ui_x11_ComponentImp));
}

psy_ui_x11_ComponentImp* psy_ui_x11_componentimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand)
{
	psy_ui_x11_ComponentImp* rv;

	rv = psy_ui_x11_componentimp_alloc();
	if (rv) {
		psy_ui_x11_componentimp_init(rv,
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

void dev_destroy(psy_ui_x11_ComponentImp* self)
{	
	psy_ui_X11App* x11app;		

    x11app = (psy_ui_X11App*)psy_ui_app()->imp;    
	self->mapped = FALSE;
	self->visible = FALSE;
	XDestroyWindow(x11app->dpy, self->hwnd);
}

void dev_show(psy_ui_x11_ComponentImp* self)
{
	psy_ui_X11App* x11app;		

	self->visible = TRUE;
    x11app = (psy_ui_X11App*)psy_ui_app()->imp;        
	XMapWindow(x11app->dpy, self->hwnd);
	self->mapped = TRUE;	
}

void dev_showstate(psy_ui_x11_ComponentImp* self, int state)
{
	psy_ui_X11App* x11app;		

	self->visible = TRUE;
    x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	XMapWindow(x11app->dpy, self->hwnd);
	self->mapped = TRUE;
}

void dev_hide(psy_ui_x11_ComponentImp* self)
{
	psy_ui_X11App* x11app;		

	self->visible = FALSE;
    x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	XUnmapWindow(x11app->dpy, self->hwnd);
	self->mapped = FALSE;
}

int dev_visible(psy_ui_x11_ComponentImp* self)
{
	return self->mapped;
}

int dev_drawvisible(psy_ui_x11_ComponentImp* self)
{
	/* TODO */
	return self->mapped;
}

void dev_move(psy_ui_x11_ComponentImp* self, psy_ui_Point origin)
{
	psy_ui_X11App* x11app;		

    x11app = (psy_ui_X11App*)psy_ui_app()->imp;
    XMoveWindow(x11app->dpy, self->hwnd,		
		(int)psy_ui_value_px(&origin.x, dev_textmetric(self), NULL),
		(int)psy_ui_value_px(&origin.y, dev_textmetric(self), NULL));		
}

void dev_resize(psy_ui_x11_ComponentImp* self, psy_ui_Size size)
{    
	const psy_ui_TextMetric* tm;
	psy_ui_X11App* x11app;
		
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	tm = dev_textmetric(self);	
	self->sizecachevalid = FALSE;	
	XResizeWindow(x11app->dpy, self->hwnd,
		(psy_ui_value_px(&size.width, tm, NULL) > 0)
			? psy_ui_value_px(&size.width, tm, NULL)
			: 1,
		(psy_ui_value_px(&size.height, tm, NULL) > 0)
			? psy_ui_value_px(&size.height, tm, NULL)
			: 1);	
	self->sizecache = size;
	self->sizecachevalid = TRUE;	
}

void dev_clientresize(psy_ui_x11_ComponentImp* self, intptr_t width, intptr_t height)
{
	//RECT rc;

	//rc.left = 0;
	//rc.top = 0;
	//rc.right = width;
	//rc.bottom = height;
	//AdjustWindowRectEx(&rc, windowstyle(self),
		//GetMenu(self->hwnd) != NULL,
		//windowexstyle(self));
	//dev_resize(self, rc.right - rc.left, rc.bottom - rc.top);
	psy_ui_X11App* x11app;
		
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	self->sizecachevalid = FALSE;
	XResizeWindow(x11app->dpy, self->hwnd,
		width,
		height);	
	self->sizecachevalid = FALSE;
}

psy_ui_RealRectangle dev_position(const psy_ui_x11_ComponentImp* self)
{
	psy_ui_RealRectangle rv;
	psy_ui_Size size;

	Window root;
	unsigned int temp;
	XWindowAttributes win_attr;
	psy_ui_X11App* x11app;

	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	XGetWindowAttributes(x11app->dpy, self->hwnd, &win_attr);
	rv.left = win_attr.x;
	rv.top = win_attr.y;
	rv.right = win_attr.x + win_attr.width;
	rv.bottom = win_attr.y + win_attr.height;
	return rv;
}

psy_ui_RealRectangle dev_screenposition(const psy_ui_x11_ComponentImp* self)
{
	psy_ui_RealRectangle rv;
	psy_ui_Size size;

	/* todo */
	Window root;
	unsigned int temp;
	XWindowAttributes win_attr;
	psy_ui_X11App* x11app;

	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	XGetWindowAttributes(x11app->dpy, self->hwnd, &win_attr);
	rv.left = win_attr.x;
	rv.top = win_attr.y;
	rv.right = win_attr.x + win_attr.width;
	rv.bottom = win_attr.y + win_attr.height;
	return rv;
}


void dev_setposition(psy_ui_x11_ComponentImp* self, psy_ui_Point topleft,
	psy_ui_Size size)
{
	const psy_ui_TextMetric* tm;
	psy_ui_X11App* x11app;
		
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	tm = dev_textmetric(self);
	self->sizecachevalid = FALSE;	
	if (psy_ui_size_has_percent(&size)) {
		psy_ui_Size parentsize;

		if (psy_ui_component_parent_const(self->component)) {
			parentsize = psy_ui_component_scrollsize(psy_ui_component_parent_const(self->component));
		} else {
			parentsize = psy_ui_component_scrollsize(self->component);
		}
		XMoveResizeWindow(x11app->dpy, self->hwnd,
			psy_ui_value_px(&topleft.x, tm, &parentsize),
			psy_ui_value_px(&topleft.y, tm, &parentsize),
			(psy_ui_value_px(&size.width, tm, &parentsize) > 0)
			? psy_ui_value_px(&size.width, tm, &parentsize)
			: 1,
			(psy_ui_value_px(&size.height, tm, &parentsize) > 0)
			? psy_ui_value_px(&size.height, tm, &parentsize)
			: 1);
	} else {	
		XMoveResizeWindow(x11app->dpy, self->hwnd,
			psy_ui_value_px(&topleft.x, tm, NULL),
			psy_ui_value_px(&topleft.y, tm, NULL),
			(psy_ui_value_px(&size.width, tm, NULL) > 0)
			? psy_ui_value_px(&size.width, tm, NULL)
			: 1,
			(psy_ui_value_px(&size.height, tm, NULL) > 0)
			? psy_ui_value_px(&size.height, tm, NULL)
			: 1);
	}	
	dev_updatesize(self);	
}

psy_ui_Size dev_size(const psy_ui_x11_ComponentImp* self)
{    
	psy_ui_Size rv;
 
    if (self->hwnd) {
        Window root;
        unsigned int temp;
        unsigned int width = 0;
        unsigned int height = 0;        
        XWindowAttributes win_attr;		
		psy_ui_X11App* x11app;
		
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
        XGetWindowAttributes(x11app->dpy, self->hwnd, &win_attr);       
        rv.width = psy_ui_value_make_px(win_attr.width);
        rv.height = psy_ui_value_make_px(win_attr.height);
    } else {
        rv.width = psy_ui_value_make_px(0);
        rv.height = psy_ui_value_make_px(0);
    }
	return rv;
}

void dev_updatesize(psy_ui_x11_ComponentImp* self)
{
	psy_ui_Size size;		
	Window root;
    unsigned int temp;
    unsigned int width = 0;
    unsigned int height = 0;    
    XWindowAttributes win_attr;			
	psy_ui_X11App* x11app;
		
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;        
	XGetWindowAttributes(x11app->dpy, self->hwnd, &win_attr); 	
    size.width = psy_ui_value_make_px(win_attr.width);
    size.height = psy_ui_value_make_px(win_attr.height);	
	self->sizecache = size;
	self->sizecachevalid = TRUE;
}

psy_ui_Size dev_framesize(psy_ui_x11_ComponentImp* self)
{
	psy_ui_Size rv;
 
    if (self->hwnd) {   
        Window root;
        unsigned int temp;
        unsigned int width = 0;
        unsigned int height = 0;        
        XWindowAttributes win_attr;			
		psy_ui_X11App* x11app;
		
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;        
        XGetWindowAttributes(x11app->dpy, self->hwnd, &win_attr); 
        rv.width = psy_ui_value_make_px(win_attr.width);
        rv.height = psy_ui_value_make_px(win_attr.height);
    } else {
        rv.width = psy_ui_value_make_px(0);
        rv.height = psy_ui_value_make_px(0);
    }
	return rv;
}

void dev_scrollto(psy_ui_x11_ComponentImp* self, intptr_t dx, intptr_t dy)
{	
	if (dx != 0 || dy != 0) {		
		XWindowAttributes win_attr;			
		psy_ui_x11_GraphicsImp* gx11;
		XExposeEvent xev;
		Region region;
		XRectangle rectangle;
		psy_ui_X11App* x11app;
		
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		XGetWindowAttributes(x11app->dpy, self->hwnd, &win_attr); 
		gx11 = (psy_ui_x11_GraphicsImp*)self->g.imp;	
		region = XCreateRegion();
		rectangle.x = 0;
		rectangle.y = 0;
		rectangle.width = (unsigned short) win_attr.width;
		rectangle.height = (unsigned short) win_attr.height;
		XUnionRectWithRegion(&rectangle, region, region);
		XSetRegion(x11app->dpy, gx11->gc, region);		
		XDestroyRegion(region);		
		XCopyArea(x11app->dpy, self->hwnd, self->hwnd,
			gx11->gc,
			(dx < 0) ? -dx : 0, // srcx
			(dy < 0) ? -dy : 0, // srcy
			(dx < 0) ? win_attr.width + dx : win_attr.width - dx, // srcwidth
			(dy < 0) ? win_attr.height + dy : win_attr.height - dy, // srcheight
			(dx < 0) ? 0 : dx, // destx
			(dy < 0) ? 0 : dy); // desty
		XSync(x11app->dpy, FALSE);
		// printf("%d\n", dy);
		xev.type = Expose;
		xev.display = x11app->dpy;
		xev.window = self->hwnd;
		xev.count = 0;
		if (dy < 0) {
			xev.x = 0;
			xev.y = win_attr.height + dy;
			xev.width = win_attr.width; 
			xev.height = -dy;
		} else if (dy > 0) {
			xev.x = 0;
			xev.y = 0;
			xev.width = win_attr.width; 
			xev.height = dy;
		}
		// printf("scrollto: %d, %d, %d, %d\n", xev.x, xev.y, xev.width, xev.height);
		XSendEvent (x11app->dpy, self->hwnd, True, ExposureMask, (XEvent *)&xev);
	}
}

psy_ui_Component* dev_parent(psy_ui_x11_ComponentImp* self)
{
	return (self->parent)
		? self->parent->component
		: NULL;
}

void dev_setparent(psy_ui_x11_ComponentImp* self, psy_ui_Component* parent)
{	
	if (parent) {
		psy_ui_x11_ComponentImp* parentimp;
		psy_ui_X11App* x11app;
		
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		parentimp = (psy_ui_x11_ComponentImp*)parent->imp;
		if (parentimp) {	
			self->parent = parentimp;	
			psy_list_free(self->parent->children_nonrec_cache);
			self->parent->children_nonrec_cache = NULL;		
			XReparentWindow(x11app->dpy,
				self->hwnd,
				parentimp->hwnd,
				0, 0);					
		}
	}
}

void dev_insert(psy_ui_x11_ComponentImp* self, psy_ui_x11_ComponentImp* child,
	psy_ui_x11_ComponentImp* insertafter)
{
	assert(child);

	if ((child->imp.vtable->dev_flags(&child->imp) & psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) ==
			psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) {
		psy_list_append(&self->viewcomponents, child->component);
	} else {
	  dev_setparent(child, self->component);
	}
	//SetParent(child->hwnd, self->hwnd);
	//SetWindowPos(
		//child->hwnd,
		//insertafter->hwnd,
		//0, 0, 0, 0,
		//SWP_NOMOVE | SWP_NOSIZE
	//);
}

void dev_remove(psy_ui_x11_ComponentImp* self, psy_ui_x11_ComponentImp* child)
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

void dev_erase(psy_ui_x11_ComponentImp* self, psy_ui_x11_ComponentImp* child)
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

void dev_setorder(psy_ui_x11_ComponentImp* self, psy_ui_x11_ComponentImp*
	insertafter)
{
	/* TODO */
}

void dev_capture(psy_ui_x11_ComponentImp* self)
{
	psy_ui_X11App* x11app;
		
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	XGrabPointer(x11app->dpy,
		   self->hwnd,
		   0,
		   ButtonPressMask|ButtonReleaseMask|
		   ButtonMotionMask|PointerMotionMask,
		   GrabModeAsync,
		   GrabModeAsync, 
		   None,
		   0,
		   CurrentTime);
}

void dev_releasecapture(psy_ui_x11_ComponentImp* self)
{
	psy_ui_X11App* x11app;
		
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	XUngrabPointer(x11app->dpy, CurrentTime);
	XFlush(x11app->dpy);
}

void dev_invalidate(psy_ui_x11_ComponentImp* self)
{
	psy_ui_X11App* x11app;
	XWindowAttributes win_attr;
	XExposeEvent xev;	
		
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;    
    XGetWindowAttributes(x11app->dpy, self->hwnd, &win_attr);	
	xev.type = Expose;
	xev.display = x11app->dpy;
	xev.window = self->hwnd;
	xev.count = 0;
	xev.x = 0;
	xev.y = 0;
	xev.width = win_attr.width; 
	xev.height = win_attr.height;
	XSendEvent(x11app->dpy, self->hwnd, True, ExposureMask, (XEvent*)&xev);
}

void dev_invalidaterect(psy_ui_x11_ComponentImp* self,
	const psy_ui_RealRectangle* r)
{	
	psy_ui_X11App* x11app;
	XExposeEvent xev;	
	
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	xev.type = Expose;
	xev.display = x11app->dpy;
	xev.window = self->hwnd;
	xev.count = 0;	
	xev.x = (int)r->left;
	xev.y = (int)r->top;		
	xev.width = (int)r->right - (int)r->left; 
	xev.height = (int)r->bottom - (int)r->top;		
	XSendEvent(x11app->dpy, self->hwnd, True, ExposureMask, (XEvent*)&xev);	
}

void dev_update(psy_ui_x11_ComponentImp* self)
{
	psy_ui_X11App* x11app;
		
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	XFlush(x11app->dpy);
	XSync(x11app->dpy, FALSE);
}

void dev_setfont(psy_ui_x11_ComponentImp* self, psy_ui_Font* source)
{
	self->tmcachevalid = FALSE;
}

void dev_rec_children(psy_ui_x11_ComponentImp* self,
	psy_List** children)
{		
	Window root_win;
	Window parent_win;
	Window* child_windows;
	int i;
	int num_child_windows;	
	psy_ui_x11_ComponentImp* imp;
	psy_ui_X11App* x11app;
		
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;		
	XQueryTree(x11app->dpy, self->hwnd,
           &root_win,
           &parent_win,
           &child_windows, &num_child_windows);   
	for (i = 0; i < num_child_windows; ++i) {
		uintptr_t hwnd;
		psy_ui_x11_ComponentImp* imp;
		psy_ui_Component* child;
			
        hwnd = child_windows[i];
        imp = psy_table_at(&x11app->selfmap, hwnd);
		child = imp ? imp->component : 0;
		if (child) {
			psy_list_append(children, child);
			dev_rec_children(imp, children);
		}
	}                                                   
	XFree(child_windows);
}

psy_List* dev_children(psy_ui_x11_ComponentImp* self, int recursive)
{	
	psy_List* rv = NULL;
	psy_List* p = 0;
	
	if (recursive == psy_ui_RECURSIVE) {		
		dev_rec_children(self, &rv);
	} else {
		if (self->children_nonrec_cache) {
			psy_List* p;
			
			for (p = self->children_nonrec_cache; p != NULL;
				psy_list_next(&p)) {
					psy_list_append(&rv, (psy_ui_Component*)
						psy_list_entry(p));
			}
		} else {			
			Window root_win;
			Window parent_win;
			Window* child_windows;
			int i;
			int num_child_windows;	
			psy_ui_x11_ComponentImp* imp;
			psy_ui_X11App* x11app;
		
			x11app = (psy_ui_X11App*)psy_ui_app()->imp;
			XQueryTree(x11app->dpy, self->hwnd,
				   &root_win,
				   &parent_win,
				   &child_windows, &num_child_windows);
			for (i = 0; i < num_child_windows; ++i) {
				uintptr_t hwnd;
				psy_ui_x11_ComponentImp* imp;
				psy_ui_Component* child;
					
				hwnd = child_windows[i];
				imp = psy_table_at(&x11app->selfmap, hwnd);
				child = imp ? imp->component : 0;
				if (child) {
					psy_list_append(&rv, child);
					psy_list_append(&self->children_nonrec_cache, child);
				}
			}                                                   
			XFree(child_windows);			
			
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

void dev_enableinput(psy_ui_x11_ComponentImp* self)
{
	/* TODO */
}

void dev_preventinput(psy_ui_x11_ComponentImp* self)
{
	/* TODO */
}

bool dev_inputprevented(const psy_ui_x11_ComponentImp* self)
{
	/* TODO */
	return FALSE;
}

const psy_ui_TextMetric* dev_textmetric(const psy_ui_x11_ComponentImp* self)
{	
	if (!self->tmcachevalid) {
		psy_ui_TextMetric rv;
		psy_ui_X11App* x11app;
		GC gc;
		PlatformXtGC xgc;
		psy_ui_Graphics g;
		psy_ui_x11_GraphicsImp* gx11;
									
		rv.tmAveCharWidth = 10;				
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		gc = XCreateGC(x11app->dpy, self->hwnd, 0, 0);
		xgc.display =x11app->dpy;
		if (x11app->dbe) {
			xgc.window = self->d_backBuf;
		} else {
			xgc.window = self->hwnd;
		}
		xgc.visual = x11app->visual;
		xgc.gc = gc;
		psy_ui_graphics_init(&g, &xgc);
		gx11 = (psy_ui_x11_GraphicsImp*)g.imp;
		rv.tmHeight = gx11->xftfont->height;
		rv.tmAscent = gx11->xftfont->ascent;
		rv.tmDescent = gx11->xftfont->descent;
		rv.tmMaxCharWidth = gx11->xftfont->max_advance_width;
		rv.tmAveCharWidth = gx11->xftfont->max_advance_width / 4;
		psy_ui_graphics_dispose(&g);		
		// mutable
		((psy_ui_x11_ComponentImp*)(self))->tm = rv;
		((psy_ui_x11_ComponentImp*)(self))->tmcachevalid = TRUE;
	}	
	return &self->tm;	
}

void dev_setcursor(psy_ui_x11_ComponentImp* self, psy_ui_CursorStyle cursorstyle)
{
	//HCURSOR hc;

	//switch (cursorstyle) {
	//case psy_ui_CURSORSTYLE_AUTO:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_MOVE:
		//hc = LoadCursor(NULL, IDC_SIZEALL);
		//break;
	//case psy_ui_CURSORSTYLE_NODROP:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_COL_RESIZE:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_ALL_SCROLL:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_POINTER:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_NOT_ALLOWED:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_ROW_RESIZE:
		//hc = LoadCursor(NULL, IDC_SIZENS);
		//break;
	//case psy_ui_CURSORSTYLE_CROSSHAIR:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_PROGRESS:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_E_RESIZE:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_NE_RESIZE:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_DEFAULT_TEXT:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_N_RESIZE:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_NW_RESIZE:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_HELP:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_VERTICAL_TEXT:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_S_RESIZE:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_SE_RESIZE:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_INHERIT:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_WAIT:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_W_RESIZE:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//case psy_ui_CURSORSTYLE_SW_RESIZE:
		//hc = LoadCursor(NULL, IDC_SIZEWE);
		//break;
	//default:
		//hc = 0;
		//break;
	//}
	//if (hc) {
		//SetCursor(hc);
	//}
}

void dev_starttimer(psy_ui_x11_ComponentImp* self, uintptr_t id,
	uintptr_t interval)
{
	psy_ui_X11App* x11app;
	
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	psy_ui_x11app_starttimer(x11app, self->hwnd, id, interval);
}

void dev_stoptimer(psy_ui_x11_ComponentImp* self, uintptr_t id)
{
	psy_ui_X11App* x11app;
	
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	psy_ui_x11app_stoptimer(x11app, self->hwnd, id);
}

void dev_seticonressource(psy_ui_x11_ComponentImp* self, int ressourceid)
{
	//psy_ui_WinApp* winapp;

	//winapp = (psy_ui_WinApp*)app.platform;
//#if defined(_WIN64)	
	//SetClassLongPtr(self->hwnd, GCLP_HICON,
		//(intptr_t)LoadIcon(winapp->instance, MAKEINTRESOURCE(ressourceid)));
//#else	
	//SetClassLong(self->hwnd, GCL_HICON,
		//(intptr_t)LoadIcon(winapp->instance, MAKEINTRESOURCE(ressourceid)));
//#endif
}

psy_ui_Size dev_textsize(psy_ui_x11_ComponentImp* self, const char* text,
	psy_ui_Font* font)
{
    psy_ui_Size rv;    
	psy_ui_X11App* x11app;
	GC gc;
	PlatformXtGC xgc;
	psy_ui_Graphics g;	
									
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	gc = XCreateGC(x11app->dpy, self->hwnd, 0, 0);
	xgc.display =x11app->dpy;
	if (x11app->dbe) {
		xgc.window = self->d_backBuf;
	} else {
		xgc.window = self->hwnd;
	}
	xgc.visual = x11app->visual;    
	xgc.gc = gc;
	psy_ui_graphics_init(&g, &xgc);
	rv = psy_ui_textsize(&g, text, psy_strlen(text));
	psy_ui_graphics_dispose(&g);        
	return rv;
}

void dev_setbackgroundcolour(psy_ui_x11_ComponentImp* self, psy_ui_Colour colour)
{
	self->backgroundcolor = colour;
}

void dev_settitle(psy_ui_x11_ComponentImp* self, const char* title)
{
	psy_ui_X11App* x11app;	
		  
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	XStoreName(x11app->dpy, self->hwnd, title);   
}

void dev_setfocus(psy_ui_x11_ComponentImp* self)
{
	if (self->mapped) {
		psy_ui_X11App* x11app;
		XWindowAttributes win_attr;			

		x11app = (psy_ui_X11App*)psy_ui_app()->imp;       
		XGetWindowAttributes(x11app->dpy, self->hwnd, &win_attr);
		if (win_attr.map_state == IsViewable) {
			psy_ui_X11App* x11app;

			x11app = (psy_ui_X11App*)psy_ui_app()->imp;	
			XSync(x11app->dpy, FALSE);
			XSetInputFocus(x11app->dpy, self->hwnd, RevertToNone, CurrentTime);
			psy_signal_emit(&self->component->signal_focus, self, 0);
		}
	}
}

int dev_hasfocus(psy_ui_x11_ComponentImp* self)
{
	psy_ui_X11App* x11app;
	Window window_focus;
	int revert;
	
	if (!self->mapped) {
		return FALSE;
	}		
	x11app = (psy_ui_X11App*)psy_ui_app()->imp;
	XGetInputFocus(x11app->dpy, &window_focus, &revert);
	return self->hwnd == window_focus;
}

int windowstyle(psy_ui_x11_ComponentImp* self)
{
	int rv = 0;
/*	
#if defined(_WIN64)		
	rv = (int)GetWindowLongPtr(self->hwnd, GWLP_STYLE);
#else
	rv = (int)GetWindowLong(self->hwnd, GWL_STYLE);
#endif
*/
	return rv;
}

int windowexstyle(psy_ui_x11_ComponentImp* self)
{
	int rv = 0;
/*	
#if defined(_WIN64)		
	rv = (int) GetWindowLongPtr(self->hwnd, GWLP_EXSTYLE);
#else
	rv = (int)GetWindowLong(self->hwnd, GWL_EXSTYLE);
#endif 
*/
	return rv;
}

void dev_draw(psy_ui_x11_ComponentImp* self, psy_ui_Graphics* g)
{		
	psy_ui_component_draw(self->component, g, self->viewcomponents);
}

void dev_mousedown(psy_ui_x11_ComponentImp* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_mousedown(self->component, ev, self->viewcomponents);
}

void dev_mouseup(psy_ui_x11_ComponentImp* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_mouseup(self->component, ev, self->viewcomponents);
}

psy_ui_RealPoint translatecoords(psy_ui_x11_ComponentImp* self, psy_ui_Component* src,
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

psy_ui_RealPoint mapcoords(psy_ui_x11_ComponentImp* self, psy_ui_Component* src,
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

void dev_mousemove(psy_ui_x11_ComponentImp* self, psy_ui_MouseEvent* ev)
{
}

void dev_mousedoubleclick(psy_ui_x11_ComponentImp* self, psy_ui_MouseEvent* ev)
{
}

void dev_mouseenter(psy_ui_x11_ComponentImp* self)
{
	self->component->vtable->onmouseenter(self->component);
	psy_signal_emit(&self->component->signal_mouseenter,
		self->component, 0);
}

void dev_mouseleave(psy_ui_x11_ComponentImp* self)
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

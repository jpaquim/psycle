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
#include <stdlib.h>
#include <stdio.h>
#include "../../detail/portable.h"
#include "../../detail/trace.h"

extern psy_ui_App app;

static void dev_rec_children(psy_ui_x11_ComponentImp*,
	psy_List** children);
static int windowstyle(psy_ui_x11_ComponentImp*);
static int windowexstyle(psy_ui_x11_ComponentImp*);

static void psy_ui_x11_component_create_window(psy_ui_x11_ComponentImp* self,
	psy_ui_x11_ComponentImp* parent,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand);
//static HINSTANCE psy_ui_x11_component_instance(psy_ui_x11_ComponentImp*
	//parent);
//static void psy_ui_x11_component_init_wndproc(psy_ui_x11_ComponentImp* self,
	//LPCSTR classname);

// VTable Prototypes
static void dev_dispose(psy_ui_x11_ComponentImp*);
static void dev_destroy(psy_ui_x11_ComponentImp*);
static void dev_show(psy_ui_x11_ComponentImp*);
static void dev_showstate(psy_ui_x11_ComponentImp*, int state);
static void dev_hide(psy_ui_x11_ComponentImp*);
static int dev_visible(psy_ui_x11_ComponentImp*);
static void dev_move(psy_ui_x11_ComponentImp*, int left, int top);
static void dev_resize(psy_ui_x11_ComponentImp*, psy_ui_Size);
static void dev_clientresize(psy_ui_x11_ComponentImp*, int width, int height);
static psy_ui_Rectangle dev_position(psy_ui_x11_ComponentImp*);
static void dev_setposition(psy_ui_x11_ComponentImp*, psy_ui_Point topleft,
	psy_ui_Size);
static psy_ui_Size dev_size(psy_ui_x11_ComponentImp*);
static void dev_updatesize(psy_ui_x11_ComponentImp*);
static psy_ui_Size dev_framesize(psy_ui_x11_ComponentImp*);
static void dev_scrollto(psy_ui_x11_ComponentImp*, intptr_t dx, intptr_t dy);
static psy_ui_Component* dev_parent(psy_ui_x11_ComponentImp*);
static void dev_setparent(psy_ui_x11_ComponentImp* self, psy_ui_Component*
	parent);
static void dev_insert(psy_ui_x11_ComponentImp* self, psy_ui_x11_ComponentImp*
	child, psy_ui_x11_ComponentImp* insertafter);
static void dev_capture(psy_ui_x11_ComponentImp*);
static void dev_releasecapture(psy_ui_x11_ComponentImp*);
static void dev_invalidate(psy_ui_x11_ComponentImp*);
static void dev_invalidaterect(psy_ui_x11_ComponentImp*, const psy_ui_Rectangle*);
static void dev_update(psy_ui_x11_ComponentImp*);
static void dev_setfont(psy_ui_x11_ComponentImp*, psy_ui_Font*);
static psy_List* dev_children(psy_ui_x11_ComponentImp*, int recursive);
static void dev_enableinput(psy_ui_x11_ComponentImp*);
static void dev_preventinput(psy_ui_x11_ComponentImp*);
static void dev_setcursor(psy_ui_x11_ComponentImp*, psy_ui_CursorStyle);
static void dev_starttimer(psy_ui_x11_ComponentImp*, uintptr_t id, uintptr_t interval);
static void dev_stoptimer(psy_ui_x11_ComponentImp*, uintptr_t id);
static void dev_seticonressource(psy_ui_x11_ComponentImp*, int ressourceid);
static psy_ui_TextMetric dev_textmetric(psy_ui_x11_ComponentImp*);
static psy_ui_Size dev_textsize(psy_ui_x11_ComponentImp*, const char* text,
	psy_ui_Font*);
static void dev_setbackgroundcolor(psy_ui_x11_ComponentImp*, psy_ui_Color color);
static void dev_settitle(psy_ui_x11_ComponentImp*, const char* title);
static void dev_setfocus(psy_ui_x11_ComponentImp*);
static int dev_hasfocus(psy_ui_x11_ComponentImp*);

// VTable init
static psy_ui_ComponentImpVTable vtable;
static int vtable_initialized = 0;

static void xt_imp_vtable_init(psy_ui_x11_ComponentImp* self)
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
		vtable.dev_move = (psy_ui_fp_componentimp_dev_move)dev_move;
		vtable.dev_resize = (psy_ui_fp_componentimp_dev_resize)dev_resize;
		vtable.dev_clientresize = (psy_ui_fp_componentimp_dev_clientresize)
			dev_clientresize;
		vtable.dev_position = (psy_ui_fp_componentimp_dev_position)dev_position;
		vtable.dev_setposition = (psy_ui_fp_componentimp_dev_setposition)
			dev_setposition;
		vtable.dev_size = (psy_ui_fp_componentimp_dev_size) dev_size;
		vtable.dev_framesize = (psy_ui_fp_componentimp_dev_framesize)
			dev_framesize;
		vtable.dev_scrollto = (psy_ui_fp_componentimp_dev_scrollto)dev_scrollto;
		vtable.dev_parent = (psy_ui_fp_componentimp_dev_parent)dev_parent;
		vtable.dev_setparent = (psy_ui_fp_componentimp_dev_setparent)
			dev_setparent;
		vtable.dev_insert = (psy_ui_fp_componentimp_dev_insert)dev_insert;
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
		vtable.dev_setcursor = (psy_ui_fp_componentimp_dev_setcursor)
			dev_setcursor;
		vtable.dev_starttimer = (psy_ui_fp_componentimp_dev_starttimer)
			dev_starttimer;
		vtable.dev_stoptimer = (psy_ui_fp_componentimp_dev_stoptimer)
			dev_stoptimer;
		vtable.dev_seticonressource =
			(psy_ui_fp_componentimp_dev_seticonressource)
			dev_seticonressource;
		vtable.dev_textmetric = (psy_ui_fp_componentimp_dev_textmetric)
			dev_textmetric;
		vtable.dev_textsize = (psy_ui_fp_componentimp_dev_textsize)
			dev_textsize;
		vtable.dev_setbackgroundcolor =
			(psy_ui_fp_componentimp_dev_setbackgroundcolor)
			dev_setbackgroundcolor;
		vtable.dev_settitle = (psy_ui_fp_componentimp_dev_settitle)dev_settitle;
		vtable.dev_setfocus = (psy_ui_fp_componentimp_dev_setfocus)dev_setfocus;
		vtable.dev_hasfocus = (psy_ui_fp_componentimp_dev_hasfocus)dev_hasfocus;
		vtable_initialized = 1;
	}
}

// prototypes

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
	self->hwnd = 0;
	self->winid = -1;
	self->sizecachevalid = FALSE;
	self->tmcachevalid = FALSE;
	self->backgroundcolor = psy_ui_color_make(0);
//	self->wndproc = 0;
//	self->background = 0;
	parent_imp = parent ? (psy_ui_x11_ComponentImp*)parent : 0;	
	psy_ui_x11_component_create_window(self, parent_imp, classname, x, y, width,
		height, dwStyle, usecommand);			
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
	psy_ui_X11App* xtapp;
	//HINSTANCE instance;
	int err = 0;	

	xtapp = (psy_ui_X11App*) app.platform;
	//instance = psy_ui_x11_component_instance(parent);
	
	if (parent == 0) {
		 printf("Create Main Frame\n");
        // self->hwnd = XtAppCreateShell(NULL, NULL, 
        // topLevelShellWidgetClass, xtapp->dpy, NULL, 0);
        // self->bulletin = XtCreateWidget(NULL, 
        //    xmBulletinBoardWidgetClass, self->hwnd, NULL, 0);
        // XtManageChild(self->bulletin);        
        self->hwnd = XCreateSimpleWindow(
			xtapp->dpy, XDefaultRootWindow(xtapp->dpy),
				x, y, width, height, 4, 0, 0);
		XSelectInput(xtapp->dpy, self->hwnd,
			ExposureMask | KeyPressMask | KeyReleaseMask |
			StructureNotifyMask);
		self->mapped = FALSE;
		app.main = self->hwnd;
    } else {        
        self->hwnd = XCreateSimpleWindow(
			xtapp->dpy, parent->hwnd,
				x, y, width, height, 0, 0, 0);
			XMapWindow(xtapp->dpy, self->hwnd);
			XSelectInput(xtapp->dpy, self->hwnd,
				KeyPressMask | ButtonPressMask | ButtonReleaseMask |
				PointerMotionMask |
				ExposureMask | StructureNotifyMask);
			self->mapped = TRUE;
        // self->hwnd = XtCreateWidget(NULL, 
        //    xmBulletinBoardWidgetClass,
        //    parent->bulletin ? parent->bulletin : parent->hwnd,
        //    NULL, 0);
        //    XtResizeWidget(self->hwnd, 200, 200, 0);	
            //XtRealizeWidget(self->hwnd);
            //XmCreatePushButton(parent->hwnd, "button", NULL, 0);
        //    XtManageChild(self->hwnd);
        //self->bulletin = 0;
    }
    self->parent = parent;    
    if (self->parent) {
		psy_list_free(self->parent->children_nonrec_cache);
		self->parent->children_nonrec_cache = NULL;
	}
    self->children_nonrec_cache = NULL;
    if (self->hwnd) {		
		XSetWMProtocols(xtapp->dpy, self->hwnd, &xtapp->wmDeleteMessage, 1);
       /* GC gc;
        XGCValues gcv;

        gcv.foreground = BlackPixelOfScreen(XtScreen(self->hwnd));
        gc = XCreateGC(XtDisplay(self->hwnd), 
        RootWindowOfScreen(XtScreen(self->hwnd)),
        GCForeground, &gcv);
        XtVaSetValues(self->hwnd, XmNuserData, gc, NULL);
        XtAddEventHandler(self->hwnd, ExposureMask, True,
        widget_expose, (void*)self);        
        XtResizeWidget(self->hwnd, 200, 200, 0);	
        XtRealizeWidget(self->hwnd);	*/
        GC gc;
		PlatformXtGC xgc;
		XGCValues gcv;
		psy_ui_Graphics g;
		XColor dummy;
		XFontStruct *fontinfo;
		Window root;
		Window window;
		int     screen;	
				
		screen  = XDefaultScreen(xtapp->dpy);			
        gcv.function =   GXcopy;
        gcv.plane_mask = AllPlanes;
            //fontinfo = XLoadQueryFont(self->dpy,"6x10"); 
            //XSetFont(self->dpy,gc,fontinfo->fid); 
        gc = XCreateGC(xtapp->dpy, self->hwnd ,
			GCFunction | GCPlaneMask, // | GCForeground | GCBackground,
			&gcv);
        xgc.display =xtapp->dpy;
		xgc.window = self->hwnd;	    
		xgc.gc = gc;			
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
		psy_table_insert(&xtapp->selfmap, (uintptr_t) self->hwnd, self);
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
	//if (self->background) {
		//DeleteObject(self->background);
	//}
	psy_ui_componentimp_dispose(&self->imp);
	psy_ui_graphics_dispose(&self->g);
	psy_list_free(self->children_nonrec_cache);
	self->children_nonrec_cache = NULL;
}

psy_ui_x11_ComponentImp* psy_ui_x11_componentimp_alloc(void)
{
	return (psy_ui_x11_ComponentImp*) malloc(sizeof(psy_ui_x11_ComponentImp));
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
	psy_ui_X11App* xtapp;		

    xtapp = (psy_ui_X11App*) app.platform;
	XDestroyWindow(xtapp->dpy, self->hwnd);
}

void dev_show(psy_ui_x11_ComponentImp* self)
{
	psy_ui_X11App* xtapp;		

    xtapp = (psy_ui_X11App*) app.platform;        
	XMapWindow(xtapp->dpy, self->hwnd);
	self->mapped = TRUE;
}

void dev_showstate(psy_ui_x11_ComponentImp* self, int state)
{
	psy_ui_X11App* xtapp;		

    xtapp = (psy_ui_X11App*) app.platform;
	XMapWindow(xtapp->dpy, self->hwnd);
	self->mapped = TRUE;
}

void dev_hide(psy_ui_x11_ComponentImp* self)
{
	psy_ui_X11App* xtapp;		

    xtapp = (psy_ui_X11App*) app.platform;
	XUnmapWindow(xtapp->dpy, self->hwnd);
	self->mapped = FALSE;
}

int dev_visible(psy_ui_x11_ComponentImp* self)
{
	return self->mapped;
}

void dev_move(psy_ui_x11_ComponentImp* self, int left, int top)
{
	psy_ui_X11App* xtapp;		

    xtapp = (psy_ui_X11App*) app.platform;
	XMoveWindow(xtapp->dpy, self->hwnd,
		left,
		top);    
}

void dev_resize(psy_ui_x11_ComponentImp* self, psy_ui_Size size)
{    
	psy_ui_TextMetric tm;
	psy_ui_X11App* xtapp;		

    xtapp = (psy_ui_X11App*) app.platform;
	tm = dev_textmetric(self);
	self->sizecachevalid = FALSE;	
	XResizeWindow(xtapp->dpy, self->hwnd,
		(psy_ui_value_px(&size.width, &tm) > 0)
			? psy_ui_value_px(&size.width, &tm)
			: 1,
		(psy_ui_value_px(&size.height, &tm) > 0)
			? psy_ui_value_px(&size.height, &tm)
			: 1);
	self->sizecache = size;
	self->sizecachevalid = TRUE;
}

void dev_clientresize(psy_ui_x11_ComponentImp* self, int width, int height)
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
	psy_ui_X11App* xtapp;		

    xtapp = (psy_ui_X11App*) app.platform;	
	self->sizecachevalid = FALSE;
	XResizeWindow(xtapp->dpy, self->hwnd,
		width,
		height);	
	self->sizecachevalid = FALSE;
}

void dev_setposition(psy_ui_x11_ComponentImp* self, psy_ui_Point topleft,
	psy_ui_Size size)
{
	psy_ui_TextMetric tm;
	psy_ui_X11App* xtapp;		

    xtapp = (psy_ui_X11App*) app.platform;
	tm = dev_textmetric(self);
	self->sizecachevalid = FALSE;
	//printf("setposition-%d:%d,%d,%d,%d\n",
	//	self->component->debugflag,
	//	psy_ui_value_px(&topleft.x, &tm),
	//	psy_ui_value_px(&topleft.y, &tm),
	//	psy_ui_value_px(&size.width, &tm),
	//	psy_ui_value_px(&size.height, &tm));		
	XMoveResizeWindow(xtapp->dpy, self->hwnd,
		psy_ui_value_px(&topleft.x, &tm),
		psy_ui_value_px(&topleft.y, &tm),		
		(psy_ui_value_px(&size.width, &tm) > 0)
			? psy_ui_value_px(&size.width, &tm)
			: 1,
		(psy_ui_value_px(&size.height, &tm) > 0)
			? psy_ui_value_px(&size.height, &tm)
			: 1);	
	dev_updatesize(self);	
}

psy_ui_Rectangle dev_position(psy_ui_x11_ComponentImp* self)
{
	psy_ui_Rectangle rv;
    psy_ui_Size size;
    
    Window root;
    unsigned int temp;
    unsigned int x = 0;
    unsigned int y = 0;
    unsigned int width = 0;
    unsigned int height = 0;
    psy_ui_X11App* xtapp;
    XWindowAttributes win_attr;	

    xtapp = (psy_ui_X11App*) app.platform;
    XGetWindowAttributes(xtapp->dpy, self->hwnd, &win_attr);
    rv.left = win_attr.x;
	rv.top = win_attr.y;
	rv.right = x + win_attr.width;
	rv.bottom = y + win_attr.height;
	return rv;
}

psy_ui_Size dev_size(psy_ui_x11_ComponentImp* self)
{    
	psy_ui_Size rv;
 
    if (self->hwnd) {
        Window root;
        unsigned int temp;
        unsigned int width = 0;
        unsigned int height = 0;
        psy_ui_X11App* xtapp;
        XWindowAttributes win_attr;		

        xtapp = (psy_ui_X11App*) app.platform;
        XGetWindowAttributes(xtapp->dpy, self->hwnd, &win_attr);       
        rv.width = psy_ui_value_makepx(win_attr.width);
        rv.height = psy_ui_value_makepx(win_attr.height);
    } else {
        rv.width = psy_ui_value_makepx(0);
        rv.height = psy_ui_value_makepx(0);
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
    psy_ui_X11App* xtapp;
    XWindowAttributes win_attr;			

    xtapp = (psy_ui_X11App*) app.platform;        
	XGetWindowAttributes(xtapp->dpy, self->hwnd, &win_attr); 	
    size.width = psy_ui_value_makepx(win_attr.width);
    size.height = psy_ui_value_makepx(win_attr.height);	
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
        psy_ui_X11App* xtapp;
        XWindowAttributes win_attr;			

        xtapp = (psy_ui_X11App*) app.platform;        
        XGetWindowAttributes(xtapp->dpy, self->hwnd, &win_attr); 
        rv.width = psy_ui_value_makepx(win_attr.width);
        rv.height = psy_ui_value_makepx(win_attr.height);
    } else {
        rv.width = psy_ui_value_makepx(0);
        rv.height = psy_ui_value_makepx(0);
    }
	return rv;
}

void dev_scrollto(psy_ui_x11_ComponentImp* self, intptr_t dx, intptr_t dy)
{
	//ScrollWindow(self->hwnd, dx, dy, NULL, NULL);
	//UpdateWindow(self->hwnd);
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
		psy_ui_X11App* xtapp;

		xtapp = (psy_ui_X11App*)app.platform;   
		parentimp = (psy_ui_x11_ComponentImp*)parent->imp;
		if (parentimp) {	
			self->parent = parentimp;	
			psy_list_free(self->parent->children_nonrec_cache);
			self->parent->children_nonrec_cache = NULL;		
			XReparentWindow(xtapp->dpy,
				self->hwnd,
				parentimp->hwnd,
				0, 0);					
		}
	}
}

void dev_insert(psy_ui_x11_ComponentImp* self, psy_ui_x11_ComponentImp* child,
	psy_ui_x11_ComponentImp* insertafter)
{
	dev_setparent(child, self->component);
	//SetParent(child->hwnd, self->hwnd);
	//SetWindowPos(
		//child->hwnd,
		//insertafter->hwnd,
		//0, 0, 0, 0,
		//SWP_NOMOVE | SWP_NOSIZE
	//);
}

void dev_capture(psy_ui_x11_ComponentImp* self)
{
//	SetCapture(self->hwnd);
}

void dev_releasecapture(psy_ui_x11_ComponentImp* self)
{
//	ReleaseCapture();
}

void dev_invalidate(psy_ui_x11_ComponentImp* self)
{
	XExposeEvent xev;
	Window root;
	unsigned int temp;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	psy_ui_X11App* xtapp;		

	xtapp = (psy_ui_X11App*) app.platform;        
	XGetGeometry(xtapp->dpy, self->hwnd, &root, &x, &y,
			&width, &height, &temp, &temp);
	xev.type    = Expose ;
	xev.display = xtapp->dpy;
	xev.window = self->hwnd;
	xev.count = 0;
	xev.x = x;
	xev.y = y;
	xev.width = width; 
	xev.height = height;	
	XSendEvent (xtapp->dpy, self->hwnd, True, ExposureMask, (XEvent *)&xev);
}

void dev_invalidaterect(psy_ui_x11_ComponentImp* self, const psy_ui_Rectangle* r)
{	
	XExposeEvent xev;
	Window root;
	unsigned int temp;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	psy_ui_X11App* xtapp;		

	xtapp = (psy_ui_X11App*) app.platform;        
	XGetGeometry(xtapp->dpy, self->hwnd, &root, &x, &y,
			&width, &height, &temp, &temp);
	xev.type    = Expose ;
	xev.display = xtapp->dpy;
	xev.window = self->hwnd;
	xev.count = 0;
	xev.x = r->left;
	xev.y = r->top;
	xev.width = r->right - r->left; 
	xev.height = r->bottom - r->top;	
	XSendEvent (xtapp->dpy, self->hwnd, True, ExposureMask, (XEvent *)&xev);
}

void dev_update(psy_ui_x11_ComponentImp* self)
{
//	UpdateWindow(self->hwnd);
}

void dev_setfont(psy_ui_x11_ComponentImp* self, psy_ui_Font* source)
{
	self->tmcachevalid = FALSE;
}

void dev_rec_children(psy_ui_x11_ComponentImp* self,
	psy_List** children)
{	
	psy_ui_X11App* xtapp;
	Window root_win;
	Window parent_win;
	Window* child_windows;
	int i;
	int num_child_windows;	
		    
    xtapp = (psy_ui_X11App*) app.platform; 
	psy_ui_x11_ComponentImp* imp;		
	XQueryTree(xtapp->dpy, self->hwnd,
           &root_win,
           &parent_win,
           &child_windows, &num_child_windows);   
	for (i = 0; i < num_child_windows; ++i) {
		uintptr_t hwnd;
		psy_ui_x11_ComponentImp* imp;
		psy_ui_Component* child;
			
        hwnd = child_windows[i];
        imp = psy_table_at(&xtapp->selfmap, hwnd);
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
	psy_List* children = NULL;
	
	if (recursive == psy_ui_RECURSIVE) {		
		dev_rec_children(self, &children);
	} else {
		if (self->children_nonrec_cache) {
			psy_List* p;
			
			for (p = self->children_nonrec_cache; p != NULL;
				psy_list_next(&p)) {
					psy_list_append(&children, (psy_ui_Component*)
						psy_list_entry(p));
			}
		} else {
			psy_ui_X11App* xtapp;
			Window root_win;
			Window parent_win;
			Window* child_windows;
			int i;
			int num_child_windows;	
					
			xtapp = (psy_ui_X11App*) app.platform; 
			psy_ui_x11_ComponentImp* imp;		
			XQueryTree(xtapp->dpy, self->hwnd,
				   &root_win,
				   &parent_win,
				   &child_windows, &num_child_windows);
			for (i = 0; i < num_child_windows; ++i) {
				uintptr_t hwnd;
				psy_ui_x11_ComponentImp* imp;
				psy_ui_Component* child;
					
				hwnd = child_windows[i];
				imp = psy_table_at(&xtapp->selfmap, hwnd);
				child = imp ? imp->component : 0;
				if (child) {
					psy_list_append(&children, child);
					psy_list_append(&self->children_nonrec_cache, child);
				}
			}                                                   
			XFree(child_windows);			
			
		}
	}	
	return children;	
}

void dev_enableinput(psy_ui_x11_ComponentImp* self)
{
//	EnableWindow(self->hwnd, 1);
}

void dev_preventinput(psy_ui_x11_ComponentImp* self)
{
//	EnableWindow(self->hwnd, 0);
}

psy_ui_TextMetric dev_textmetric(psy_ui_x11_ComponentImp* self)
{
	if (self->tmcachevalid) {
		return self->tm;
	} else {
		psy_ui_TextMetric rv;
		psy_ui_X11App* xtapp;
		GC gc;
		PlatformXtGC xgc;
		psy_ui_Graphics g;
		psy_ui_x11_GraphicsImp* gx11;
									
		rv.tmAveCharWidth = 10;
		xtapp = (psy_ui_X11App*) app.platform;
		gc = XCreateGC(xtapp->dpy, self->hwnd, 0, 0);
		xgc.display =xtapp->dpy;
		xgc.window = self->hwnd;	    
		xgc.gc = gc;
		psy_ui_graphics_init(&g, &xgc);
		gx11 = (psy_ui_x11_GraphicsImp*)g.imp;
		rv.tmHeight = gx11->xftfont->height;
		rv.tmAscent = gx11->xftfont->ascent;
		rv.tmDescent = gx11->xftfont->descent;
		rv.tmMaxCharWidth = gx11->xftfont->max_advance_width;
		rv.tmAveCharWidth = gx11->xftfont->max_advance_width / 4;
		// printf("avcharwidth %d\n", rv.tmAveCharWidth);
		psy_ui_graphics_dispose(&g);
		self->tm = rv;
		self->tmcachevalid = TRUE;
		return rv;
	}
	//TEXTMETRIC tm;
	//HDC hdc;
	//HFONT hPrevFont = 0;
	//HFONT hfont = 0;

	//hdc = GetDC(self->hwnd);
	//SaveDC(hdc);
	//hfont = ((psy_ui_x11_FontImp*) font->imp)->hfont;
	//if (hfont) {
		//hPrevFont = SelectObject(hdc, hfont);
	//}
	//GetTextMetrics(hdc, &tm);
	//if (hPrevFont) {
		//SelectObject(hdc, hPrevFont);
	//}
	//RestoreDC(hdc, -1);
	//ReleaseDC(self->hwnd, hdc);
	//rv.tmHeight = tm.tmHeight;
	//rv.tmAscent = tm.tmAscent;
	//rv.tmDescent = tm.tmDescent;
	//rv.tmInternalLeading = tm.tmInternalLeading;
	//rv.tmExternalLeading = tm.tmExternalLeading;
	//rv.tmAveCharWidth = tm.tmAveCharWidth;
	//rv.tmMaxCharWidth = tm.tmMaxCharWidth;
	//rv.tmWeight = tm.tmWeight;
	//rv.tmOverhang = tm.tmOverhang;
	//rv.tmDigitizedAspectX = tm.tmDigitizedAspectX;
	//rv.tmDigitizedAspectY = tm.tmDigitizedAspectY;
	//rv.tmFirstChar = tm.tmFirstChar;
	//rv.tmLastChar = tm.tmLastChar;
	//rv.tmDefaultChar = tm.tmDefaultChar;
	//rv.tmBreakChar = tm.tmBreakChar;
	//rv.tmItalic = tm.tmItalic;
	//rv.tmUnderlined = tm.tmUnderlined;
	//rv.tmStruckOut = tm.tmStruckOut;
	//rv.tmPitchAndFamily = tm.tmPitchAndFamily;
	//rv.tmCharSet = tm.tmCharSet;
	
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
//	SetTimer(self->hwnd, id, interval, 0);
}

void dev_stoptimer(psy_ui_x11_ComponentImp* self, uintptr_t id)
{
//	KillTimer(self->hwnd, id);
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
	psy_ui_X11App* xtapp;
	GC gc;
	PlatformXtGC xgc;
	psy_ui_Graphics g;	
									
	xtapp = (psy_ui_X11App*) app.platform;
	gc = XCreateGC(xtapp->dpy, self->hwnd, 0, 0);
	xgc.display =xtapp->dpy;
	xgc.window = self->hwnd;	    
	xgc.gc = gc;
	psy_ui_graphics_init(&g, &xgc);
	rv = psy_ui_textsize(&g, text);
	psy_ui_graphics_dispose(&g);        
	return rv;
}

void dev_setbackgroundcolor(psy_ui_x11_ComponentImp* self, psy_ui_Color color)
{
	self->backgroundcolor = color;
}

void dev_settitle(psy_ui_x11_ComponentImp* self, const char* title)
{
	psy_ui_X11App* xtapp;	
		  
	xtapp = (psy_ui_X11App*) app.platform;
	XStoreName(xtapp->dpy, self->hwnd, title);   
}

void dev_setfocus(psy_ui_x11_ComponentImp* self)
{
//	SetFocus(self->hwnd);
	psy_signal_emit(&self->component->signal_focus, self, 0);
}

int dev_hasfocus(psy_ui_x11_ComponentImp* self)
{
	// return self->hwnd == GetFocus();
	return 0;
}

/*
BOOL CALLBACK allchildenumproc(HWND hwnd, LPARAM lParam)
{
	psy_List** pChildren = (psy_List**)lParam;
	psy_ui_WinApp* winapp;
	psy_ui_x11_ComponentImp* imp;
	psy_ui_Component* child;

	winapp = (psy_ui_WinApp*)app.platform;
	imp = psy_table_at(&winapp->selfmap, (uintptr_t)hwnd);
	child = imp ? imp->component : 0;
	if (child) {
		psy_list_append(pChildren, child);
	}
	return TRUE;
}
*/

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

#endif

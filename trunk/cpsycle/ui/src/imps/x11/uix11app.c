/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uix11app.h"
#if PSYCLE_USE_TK == PSYCLE_TK_X11

/* local */
#include "uicomponent.h"
#include "uix11componentimp.h"
#include "uix11fontimp.h"
#include "uix11graphicsimp.h"
#include "uix11keyboardevent.h"
/* X11 */
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xdbe.h>
#include <X11/keysym.h>
/* std */
#include <stdio.h>

static psy_ui_MouseEvent buttonpressevent;
/* prototypes */
static void psy_ui_x11app_initdbe(psy_ui_X11App*);
static void psy_ui_x11app_dispose(psy_ui_X11App*);
static int psy_ui_x11app_run(psy_ui_X11App*);
static void psy_ui_x11app_stop(psy_ui_X11App*);
static void psy_ui_x11app_close(psy_ui_X11App*);
static void psy_ui_x11app_sendevent(psy_ui_X11App*, psy_ui_Component*,
	psy_ui_Event*);
static psy_ui_Component* psy_ui_x11app_component(psy_ui_X11App*,
	uintptr_t platformhandle);

static int psy_ui_x11app_handle_event(psy_ui_X11App*, XEvent*);
static void psy_ui_x11app_mousewheel(psy_ui_X11App*, psy_ui_x11_ComponentImp*,
	XEvent*);
static int psy_ui_x11app_translate_x11button(int button);
static int psy_ui_x11app_make_x11button(int button);
static bool psy_ui_x11app_sendeventtoparent(psy_ui_X11App*,
	psy_ui_x11_ComponentImp*, int mask, XEvent*);
static void psy_ui_x11app_adjustcoordinates(psy_ui_Component*,
	psy_ui_RealPoint*);
static int psy_ui_x11app_timertick(psy_ui_X11App*);
static void psy_ui_x11app_update_keyevent_mods(psy_ui_X11App*,
	psy_ui_KeyboardEvent*);
static void psy_ui_x11app_update_mouseevent_mods(psy_ui_X11App*,
	psy_ui_MouseEvent*);
static XButtonEvent psy_ui_X11app_make_x11buttonevent(psy_ui_MouseEvent*,
	Display*, int hwnd);	
static void psy_ui_x11app_sendx11event(psy_ui_X11App*, int mask, int hwnd,
	XEvent*);
static void psy_ui_x11app_sync(psy_ui_X11App*);

/* vtable */
static psy_ui_AppImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_X11App* self)
{
	assert(self);

	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose =
			(psy_ui_fp_appimp_dispose)
			psy_ui_x11app_dispose;
		imp_vtable.dev_run =
			(psy_ui_fp_appimp_run)
			psy_ui_x11app_run;
		imp_vtable.dev_stop =
			(psy_ui_fp_appimp_stop)
			psy_ui_x11app_stop;
		imp_vtable.dev_close =
			(psy_ui_fp_appimp_close)
			psy_ui_x11app_close;
		imp_vtable.dev_onappdefaultschange =
			(psy_ui_fp_appimp_onappdefaultschange)
			psy_ui_x11app_onappdefaultschange;
		imp_vtable.dev_sendevent =
			(psy_ui_fp_appimp_sendevent)
			psy_ui_x11app_sendevent;
		imp_vtable.dev_component =
			(psy_ui_fp_appimp_component)
			psy_ui_x11app_component;				
		imp_vtable_initialized = TRUE;
	}
	self->imp.vtable = &imp_vtable;
}
/* implementation */
void psy_ui_x11app_init(psy_ui_X11App* self, psy_ui_App* app, void* instance)
{
	static const char szAppClass[] = "PsycleApp";
	static const char szComponentClass[] = "PsycleComponent";	
	bool shape_extension;
	int shapeEventBase;
	int shapeErrorBase;

	psy_ui_appimp_init(&self->imp);
	imp_vtable_init(self);
	self->app = app;
	printf("open display\n");
	self->dpy = XOpenDisplay(NULL);
	self->appclass = szAppClass;
	self->componentclass = szComponentClass;
	self->winid = 20000;
	psy_table_init(&self->selfmap);
	psy_table_init(&self->winidmap);
	self->wmDeleteMessage = XInternAtom(self->dpy, "WM_DELETE_WINDOW", False);
	self->running = FALSE;
	shape_extension = XShapeQueryExtension (self->dpy,
		&shapeEventBase,
		&shapeErrorBase);
	if (!shape_extension) {
		printf("XShapeQueryExtension error\n");
	}
	self->dbe = TRUE;
	if (self->dbe) {
		psy_ui_x11app_initdbe(self);
	} else {
		self->visual = DefaultVisual(self->dpy,
			DefaultScreen(self->dpy));
	}
	psy_ui_x11colours_init(&self->colourmap, self->dpy);
	self->dograb = FALSE;
	self->grabwin = 0;
	self->targetids = NULL;
	psy_timers_init(&self->wintimers);
	self->shiftstate = FALSE;
	self->controlstate = FALSE;
	self->altstate = FALSE;
}

void psy_ui_x11app_initdbe(psy_ui_X11App* self)
{
	int major, minor;

	self->vinfo = 0;
	self->visual = 0;
	if (XdbeQueryExtension(self->dpy, &major, &minor)) {
		XVisualInfo xvisinfo_templ;
		int numScreens = 1;
		int matches;
		Drawable screens[] = { DefaultRootWindow(self->dpy) };
		XdbeScreenVisualInfo *info;
		
		printf("Xdbe (%d.%d) supported, using double buffering\n",
			major, minor);		
		info = XdbeGetVisualInfo(self->dpy, screens,
			&numScreens);
		if (!info || numScreens < 1 || info->count < 1) {
			fprintf(stderr, "No visuals support Xdbe\n");
			return;
		}
		/*
		** Choosing the first one, seems that they have all perflevel of 0,
		** and the depth varies.
		*/
		/* We know there's at least one */
		xvisinfo_templ.visualid = info->visinfo[0].visual;
		/*
		** As far as I know, screens are densely packed, so we can assume that
		** if at least 1 exists, it's screen 0.
		*/
		xvisinfo_templ.screen = 0;
		xvisinfo_templ.depth = info->visinfo[0].depth;
		XdbeFreeVisualInfo(info);
		self->vinfo = XGetVisualInfo(self->dpy,
			VisualIDMask|VisualScreenMask|VisualDepthMask, &xvisinfo_templ,
			&matches);
		if (!self->vinfo || matches < 1) {
			fprintf(stderr, "Couldn't match a Visual with double buffering\n");
			return;
		}		
		self->visual = self->vinfo->visual;
	} else {
		fprintf(stderr, "No Xdbe support\n");
		return;
	}
}

void psy_ui_x11app_dispose(psy_ui_X11App* self)
{
	if (self->vinfo) {
		XFree(self->vinfo);
	}
	psy_table_dispose(&self->selfmap);
	psy_table_dispose(&self->winidmap);
	XCloseDisplay(self->dpy);
	psy_ui_x11colours_dispose(&self->colourmap);
	psy_list_free(self->targetids);
	psy_timers_dispose(&self->wintimers);
}

int psy_ui_x11app_run(psy_ui_X11App* self)
{
	XEvent event;
	int x11_fd;
	struct timeval tv;
	fd_set in_fds;

	x11_fd = ConnectionNumber(self->dpy);
    self->running = TRUE;
    tv.tv_sec = 0;
	tv.tv_usec = 10000;
	FD_ZERO(&in_fds);
	FD_SET(x11_fd, &in_fds);
	while (self->running) {
		if (XPending(self->dpy)) {
			XNextEvent(self->dpy, &event);
			psy_ui_x11app_handle_event(self, &event);
		} else {
			if (select(x11_fd + 1, &in_fds, 0, 0, &tv) == 0) {
				psy_ui_x11app_timertick(self);
			}
			if (tv.tv_usec == 0) {
				tv.tv_sec = 0;
				tv.tv_usec = 10000;
			}
		}
    }
    return 0;
}

void psy_ui_x11app_stop(psy_ui_X11App* self)
{
	self->running = FALSE;
	/* PostQuitMessage(0); */
}

void psy_ui_x11app_close(psy_ui_X11App* self)
{
	self->running = FALSE;
}

int psy_ui_x11app_timertick(psy_ui_X11App* self)
{	
	psy_timers_tick(&self->wintimers);
}

void psy_ui_x11app_starttimer(psy_ui_X11App* self, uintptr_t hwnd, uintptr_t id,
	uintptr_t interval)
{
	psy_ui_x11_ComponentImp* imp;

	imp = (psy_ui_x11_ComponentImp*)psy_table_at(&self->selfmap, hwnd);
	if (!imp && !imp->component) {
		return;
	}
	psy_timers_addtimer(&self->wintimers, hwnd, imp->component,
		(psy_fp_timerwork)imp->component->vtable->ontimer, id, interval);
}

void psy_ui_x11app_stoptimer(psy_ui_X11App* self, uintptr_t hwnd, uintptr_t id)
{
	psy_timers_removetimer(&self->wintimers, hwnd, id);
}

int psy_ui_x11app_handle_event(psy_ui_X11App* self, XEvent* event)
{
	Window id;
	psy_ui_x11_ComponentImp* imp;
	psy_ui_x11_ComponentImp* rootimp;

	imp = (psy_ui_x11_ComponentImp*)psy_table_at(&self->selfmap,
		(uintptr_t) event->xany.window);
	if (!imp) {
		return 0;
	}
	switch (event->type) {
	case DestroyNotify: {
		psy_ui_x11app_destroy_window(self, imp->hwnd);
		break; }
	case NoExpose:
		/* expose_window(self, imp,
		   event->xnoexpose.x, event->xnoexpose.y,
		   event->xnoexpose.width, event->xnoexpose.height); */
		break;
	case GraphicsExpose:
		/* expose_window(self, imp,
		   event->xgraphicsexpose.x, event->xgraphicsexpose.y,
		   event->xgraphicsexpose.width, event->xgraphicsexpose.height); */
		break;
	case Expose: {
		const psy_ui_Border* border;
		psy_ui_RealRectangle r;

		border = psy_ui_component_border(imp->component);
		r = psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					event->xexpose.x,
					event->xexpose.y),
				psy_ui_realsize_make(
					event->xexpose.width,
					event->xexpose.height));
		if (!imp->exposeareavalid) {
			imp->exposearea = r;
			imp->exposeareavalid = TRUE;
		} else {
			psy_ui_realrectangle_union(&imp->exposearea, &r);
		}
		if (event->xexpose.count > 0) {
			return 0;
		}
		if (imp->component->vtable->ondraw ||
				imp->component->signal_draw.slots ||
				imp->component->backgroundmode != psy_ui_NOBACKGROUND ||
				psy_ui_border_isset(border)) {
			psy_ui_x11_GraphicsImp* gx11;
			XRectangle rectangle;
			psy_ui_RealMargin spacing;

			if (!psy_ui_component_visible(imp->component)) {
				return 0;
			}

			gx11 = (psy_ui_x11_GraphicsImp*)imp->g.imp;
			/* reset scroll origin */
			gx11->org.x = 0;
			gx11->org.y = 0;
			/*
			** prepare a clip rect that can be used by a component to
			** optimize the draw amount
			*/
			imp->g.clip = imp->exposearea;
			/* set gc/xfd clip */
			rectangle.x = (short)imp->exposearea.left;
			rectangle.y = (short)imp->exposearea.top;
			rectangle.width = (unsigned short)(imp->exposearea.right -
				imp->exposearea.left);
			rectangle.height = (unsigned short)(imp->exposearea.bottom -
				imp->exposearea.top);
			XUnionRectWithRegion(&rectangle, gx11->region, gx11->region);
			XSetRegion(self->dpy, gx11->gc, gx11->region);
			XftDrawSetClipRectangles(gx11->xfd,0,0,&rectangle, 1);
			XDestroyRegion(gx11->region);
			gx11->region = XCreateRegion();
			/* draw */
			imp->imp.vtable->dev_draw(&imp->imp, &imp->g);
		}
		if (self->dbe) {
			int w;
			int h;

			w  = imp->exposearea.right - imp->exposearea.left;
			h  = imp->exposearea.bottom - imp->exposearea.top;
			if (w != 0 && h != 0) {
				psy_ui_x11_GraphicsImp* gx11;

				gx11 = (psy_ui_x11_GraphicsImp*)imp->g.imp;
				XCopyArea(self->dpy, imp->d_backBuf,
					imp->hwnd, gx11->gc,
					imp->exposearea.left, imp->exposearea.top,
					w, h,
					imp->exposearea.left, imp->exposearea.top);
			}
		}
		imp->exposeareavalid = FALSE;
		break; }
	case MapNotify:
		psy_signal_emit(&imp->component->signal_show,
			imp->component, 0);
		if (self->dograb && imp->hwnd == self->grabwin) {
			XGrabPointer(self->dpy,self->grabwin,True,
			PointerMotionMask | ButtonReleaseMask | ButtonPressMask,
			GrabModeAsync,
			GrabModeAsync,None,None,CurrentTime);
		}
		break;
	case UnmapNotify:
		psy_signal_emit(&imp->component->signal_hide,
			imp->component, 0);
		if (self->dograb && imp->hwnd == self->grabwin) {
			self->dograb = FALSE;
		}
		break;
	case ConfigureNotify: {
		XConfigureEvent xce = event->xconfigure;

		if (xce.width != imp->prev_w || xce.height != imp->prev_h) {
			imp->prev_w = xce.width;
			imp->prev_h = xce.height;
			if (self->dbe) {
				psy_ui_x11_GraphicsImp* gx11;

				gx11 = (psy_ui_x11_GraphicsImp*)imp->g.imp;
				psy_ui_x11_graphicsimp_updatexft(gx11);
			}							
			if (imp->component) {
				psy_ui_Event ev;

				imp->sizecachevalid = FALSE;
				psy_ui_event_init(&ev, psy_ui_RESIZE);
				psy_ui_event_stop_propagation(&ev);				
				psy_ui_eventdispatch_send(&self->app->eventdispatch,
					imp->component, &ev);
			}			
		}
		return 0; }
	case ClientMessage:
        if (event->xclient.data.l[0] == self->wmDeleteMessage) {
			XEvent e;			
			uintptr_t hwnd;
            bool close;
            bool ismain;

			ismain = (imp->component == psy_ui_app()->main);
			close = imp->component->vtable->onclose(imp->component);
			if (imp->component->signal_close.slots) {
				psy_signal_emit(&imp->component->signal_close,
					imp->component, 1, (void*)&close);
			}
			if (!close) {
				return 0;
			}
			hwnd = event->xclient.window;
			XDestroyWindow(self->dpy, event->xclient.window);
			while (TRUE) {
                if (XPending(self->dpy)) {
                    XNextEvent(self->dpy, event);
                    if (event->type ==  DestroyNotify) {
                        psy_ui_x11app_destroy_window(self, event->xany.window);
                        if (hwnd == event->xany.window) {             
                            break;
                        }
                    }
                } else {
                    break;
                }
			}			
			if (ismain) {
				self->running = FALSE;
			}
		}
        break;
    case KeyPress: {	
		if (imp->component) {
			psy_ui_KeyboardEvent ev;

			ev = psy_ui_x11_keyboardevent_make(&event->xkey);
			ev.event.type = psy_ui_KEYDOWN;
			psy_ui_eventdispatch_send(&self->app->eventdispatch,
				imp->component, &ev.event);			
			return 0;
		}
		break; }
    case KeyRelease:
		if (imp->component) {
			psy_ui_KeyboardEvent ev;

			ev = psy_ui_x11_keyboardevent_make(&event->xkey);			
			ev.event.type = psy_ui_KEYUP;
			psy_ui_eventdispatch_send(&self->app->eventdispatch,
				imp->component, &ev.event);			
			return 0;
		}
		break;
    case ButtonPress: {		
		psy_ui_MouseEvent ev;
				
		if (self->dograb) {
			psy_ui_Component* grab;
			psy_ui_Component* curr;

			grab = psy_ui_x11app_component(self, self->grabwin);
			curr = imp->component;
			while (curr && curr != grab) {
				curr = psy_ui_component_parent(curr);
			}
			if (!curr) {
				psy_ui_component_hide(grab);
				return 0;
			}
		}
		/* mousewheel */
		if (event->xbutton.button == 4 || event->xbutton.button == 5) {
			psy_ui_x11app_mousewheel(self, imp, event);				
			return 0;
		}		
		psy_ui_mouseevent_init_all(&ev,	
				psy_ui_realpoint_make(event->xbutton.x, event->xbutton.y),
				psy_ui_realpoint_make(event->xbutton.x, event->xbutton.y),
				psy_ui_x11app_translate_x11button(event->xbutton.button),
				0, 0, 0);
		ev.event.timestamp = (uintptr_t)event->xbutton.time;		
		ev.event.type = psy_ui_MOUSEDOWN;
		psy_ui_x11app_update_mouseevent_mods(self, &ev);
		psy_ui_x11app_adjustcoordinates(imp->component, &ev.pt);		
		psy_ui_eventdispatch_send(&self->app->eventdispatch,
			imp->component, &ev.event);		
		break; }
	case ButtonRelease: { 
		psy_ui_MouseEvent ev;							

		psy_ui_mouseevent_init_all(&ev,	
			psy_ui_realpoint_make(event->xbutton.x, event->xbutton.y),
			psy_ui_realpoint_make(event->xbutton.x, event->xbutton.y),
			psy_ui_x11app_translate_x11button(event->xbutton.button),
			0, 0, 0);
		ev.event.timestamp = (uintptr_t)event->xbutton.time;			
		ev.event.type = psy_ui_MOUSEUP;
		psy_ui_x11app_update_mouseevent_mods(self, &ev);
		psy_ui_x11app_adjustcoordinates(imp->component, &ev.pt);		
		psy_ui_eventdispatch_send(&self->app->eventdispatch,
			imp->component, &ev.event);
		return 0;		
		break; }
	case MotionNotify: {
		psy_ui_MouseEvent ev;
		XMotionEvent xme;
		int button;
		
		xme = event->xmotion;
		if (xme.state & Button1Mask) {
			button = 1;
		} else if (xme.state & Button2Mask) {
			button = 3;
		} else if (xme.state & Button3Mask) {
			button = 2;
		}
		psy_ui_mouseevent_init_all(&ev,
			psy_ui_realpoint_make(xme.x, xme.y),
			psy_ui_realpoint_make(xme.x, xme.y),
			button, 0, 0, 0);
		ev.event.type = psy_ui_MOUSEMOVE;
		psy_ui_x11app_update_mouseevent_mods(self, &ev);
		psy_ui_x11app_adjustcoordinates(imp->component, &ev.pt);		
		ev.event.timestamp = (uintptr_t)event->xbutton.time;					
		psy_ui_eventdispatch_send(&self->app->eventdispatch,
			imp->component, &ev.event);		
		return 0; }
	case EnterNotify: {
		imp->imp.vtable->dev_mouseenter(&imp->imp);
		break; }
	case LeaveNotify: {
		if (imp->component) {
			psy_ui_Event ev;

			psy_ui_event_init(&ev, psy_ui_MOUSELEAVE);
			psy_ui_event_stop_propagation(&ev);
			psy_ui_eventdispatch_send(&self->app->eventdispatch,
				imp->component, &ev);
		}
		return 0; }
	case FocusOut: {
		if (imp->component) {
			psy_ui_Event ev;

			psy_ui_event_init(&ev, psy_ui_FOCUSOUT);
			ev.bubbles = FALSE;
			psy_ui_eventdispatch_send(&self->app->eventdispatch,
				imp->component, &ev);					
		}		
		break; }
	default:
		break;
	}
	return 0;
}

void psy_ui_x11app_destroy_window(psy_ui_X11App* self, Window window)
{
	psy_ui_x11_ComponentImp* imp;
	
	imp = (psy_ui_x11_ComponentImp*)psy_table_at(
		&self->selfmap, (uintptr_t)window);
	if (imp) {
        psy_ui_Component* component;
        bool deallocate;

		component = imp->component;
		deallocate = FALSE;
		if (imp->component) {

            deallocate = imp->component->deallocate;
            psy_signal_emit(&imp->component->signal_destroy,
					imp->component, 0);
			imp->component->vtable->ondestroy(imp->component);
            psy_signal_emit(&imp->component->signal_destroyed,
				imp->component, 0);
				imp->component->vtable->ondestroyed(imp->component);
			psy_ui_component_dispose(imp->component);
		} else {
			imp->imp.vtable->dev_dispose(&imp->imp);
		}
		psy_table_remove(&self->selfmap, (uintptr_t)window);
		if (component && deallocate) {
            free(component);
        }
	}
}

void psy_ui_x11app_adjustcoordinates(psy_ui_Component* component,
	psy_ui_RealPoint* pt)
{
	psy_ui_RealMargin spacing;

	spacing = psy_ui_component_spacing_px(component);
	if (!psy_ui_realmargin_iszero(&spacing)) {
		pt->x -= spacing.left;
		pt->y -= spacing.top;
	}
}

void psy_ui_x11app_update_keyevent_mods(psy_ui_X11App* self,
	psy_ui_KeyboardEvent* ev)
{
	ev->shift_key = self->shiftstate;
	ev->ctrl_key = self->controlstate;
	ev->alt_key = self->altstate;
}

void psy_ui_x11app_update_mouseevent_mods(psy_ui_X11App* self,
	psy_ui_MouseEvent* ev)
{
	ev->shift_key = self->shiftstate;
	ev->ctrl_key = self->controlstate;
	/* ev->alt_key = self->altstate; */
}

int psy_ui_x11app_translate_x11button(int button)
{
	static const int map[] = { 0, 1, 3, 2 };
	
	if (button >= 0 && button < 4) {
		return map[button];
	}
	return button;
}

int psy_ui_x11app_make_x11button(int button)
{
	static const int map[] = { 0, 1, 3, 2 };
	
	if (button >= 0 && button < 4) {
		return map[button];
	}
	return 0;	
}

void psy_ui_x11app_mousewheel(psy_ui_X11App* self,
	psy_ui_x11_ComponentImp* imp, XEvent* xe)
{
	int delta;
	psy_ui_MouseEvent ev;

	if (xe->xbutton.button == 4) {
		delta = 120;
	} else if (xe->xbutton.button == 5) {
		delta = -120;
	} else {
		delta = 0;
	}
	psy_ui_mouseevent_init_all(&ev,
		psy_ui_realpoint_make(xe->xbutton.x, xe->xbutton.y),
		psy_ui_realpoint_make(xe->xbutton.x, xe->xbutton.y),
		0, delta, 0, 0);
	psy_ui_x11app_update_mouseevent_mods(self, &ev);
	psy_ui_x11app_adjustcoordinates(imp->component, &ev.pt);
	psy_ui_component_mousewheel(imp->component, &ev, delta /* 120 or -120 */);
}

bool psy_ui_x11app_sendeventtoparent(psy_ui_X11App* self,
	psy_ui_x11_ComponentImp* imp, int mask, XEvent* xev)
{
	if (xev && psy_table_at(&self->selfmap,
			(uintptr_t)imp->parent->hwnd)) {
		XEvent event;

		psy_list_append(&self->targetids, (void*)imp->hwnd);
		self->eventretarget = imp->component;
		event = *xev;
		event.xany.window = imp->parent->hwnd;		
		XSendEvent(self->dpy, imp->parent->hwnd, True, mask, xev);		
		self->eventretarget = 0;
		return TRUE;
	} else {
		psy_list_free(self->targetids);
		self->targetids = NULL;
	}
	self->eventretarget = 0;
	return FALSE;
}

void psy_ui_x11app_onappdefaultschange(psy_ui_X11App* self)
{
}

void psy_ui_x11app_sendevent(psy_ui_X11App* self, psy_ui_Component* component,
	psy_ui_Event* ev)
{
	psy_ui_x11_ComponentImp* imp;
	
	assert(component);	

	imp = (psy_ui_x11_ComponentImp*)component->imp;	
	if (!imp) {
		return;
	}	
	switch (ev->type) {
	case psy_ui_KEYDOWN: {
		XKeyEvent xkey;
				
		xkey = psy_ui_x11_xkeyevent_make((psy_ui_KeyboardEvent*)ev,
			self->dpy, imp->hwnd, DefaultRootWindow(self->dpy));
		xkey.type = KeyPress;		
		psy_ui_x11app_sendx11event(self, KeyPressMask,
			imp->hwnd, (XEvent*)&xkey);
		break; }
	case psy_ui_KEYUP: {		
		XKeyEvent xkey;
				
		xkey = psy_ui_x11_xkeyevent_make((psy_ui_KeyboardEvent*)ev,
			self->dpy, imp->hwnd, DefaultRootWindow(self->dpy));
		xkey.type = KeyRelease;		
		psy_ui_x11app_sendx11event(self, KeyReleaseMask,
			imp->hwnd, (XEvent*)&xkey);
		break; }
	case psy_ui_MOUSEDOWN:
	case psy_ui_DBLCLICK : {				
		XButtonEvent xbutton;		
				
		xbutton = psy_ui_X11app_make_x11buttonevent((psy_ui_MouseEvent*)ev,
			self->dpy, imp->hwnd);		
		psy_ui_x11app_sendx11event(self, ButtonPressMask,
			imp->hwnd, (XEvent*)&xbutton);
		break; }
	case psy_ui_MOUSEUP: {		
		XButtonEvent xbutton;		

		xbutton = psy_ui_X11app_make_x11buttonevent((psy_ui_MouseEvent*)ev,
			self->dpy, imp->hwnd);
		psy_ui_x11app_sendx11event(self, ButtonReleaseMask,
			imp->hwnd, (XEvent*)&xbutton);
		break; }		
	case psy_ui_MOUSEMOVE: {
		psy_ui_MouseEvent* mouseevent;
		XMotionEvent xme;		

		mouseevent = (psy_ui_MouseEvent*)ev;
		xme.serial = MotionNotify;
		xme.send_event = False;
		xme.display = self->dpy;
		xme.time = CurrentTime;
		xme.same_screen = True;
		xme.is_hint = NotifyNormal,
		xme.root = DefaultRootWindow(self->dpy);
		xme.window = imp->hwnd;
		xme.subwindow = None;		
		xme.state = 0x00;
		xme.x = mouseevent->pt.x;
		xme.y = mouseevent->pt.y;
		xme.x_root = 0;
		xme.y_root = 0;	
		xme.type = MotionNotify;			
		XSendEvent(self->dpy, imp->hwnd, True, Button1MotionMask,
			(XEvent*)&xme);		
		break; }
	default:
		break;
	}
}

void psy_ui_x11app_sendx11event(psy_ui_X11App* self, int mask, int hwnd,
	XEvent* ev)
{	
	assert(ev);
	
	XSendEvent(self->dpy,hwnd, FALSE, mask, ev);
	psy_ui_x11app_sync(self);	
}

void psy_ui_x11app_sync(psy_ui_X11App* self)
{
	XEvent event;
	
	XSync(self->dpy, 0);
	while (XPending(self->dpy)) {		
		XNextEvent(self->dpy, &event);
		psy_ui_x11app_handle_event(self, &event);
	}
}

XButtonEvent psy_ui_X11app_make_x11buttonevent(psy_ui_MouseEvent* ev,
	Display* dpy, int hwnd)
{
	XButtonEvent rv;
	
	rv.display      = dpy;
	rv.root         = DefaultRootWindow(dpy);
	rv.time         = (Time)ev->event.timestamp;
	rv.same_screen  = True;
	rv.button       = psy_ui_x11app_make_x11button(ev->button);
	rv.state        = 0;
	rv.x            = ev->pt.x;
	rv.y            = ev->pt.y;
	rv.x_root       = ev->pt.x;;
	rv.y_root       = ev->pt.y;
	rv.window       = hwnd;   
	if (ev->event.type == psy_ui_MOUSEUP) {
		rv.type = ButtonRelease;
	} else {
		rv.type = ButtonPress;
	}							
	return rv;
}

psy_ui_Component* psy_ui_x11app_component(psy_ui_X11App* self,
	uintptr_t handle)
{	
	psy_ui_x11_ComponentImp* imp;
	
	imp = (psy_ui_x11_ComponentImp*)psy_table_at(&self->selfmap, handle);
	if (imp) {
		return imp->component;
	}
	return NULL;
}

#endif /* PSYCLE_TK_X11 */

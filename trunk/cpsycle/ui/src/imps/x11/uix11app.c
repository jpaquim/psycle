// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uix11app.h"
#if PSYCLE_USE_TK == PSYCLE_TK_XT

#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xdbe.h>

#include "uix11graphicsimp.h"
#include "uix11fontimp.h"
#include "uicomponent.h"
#include "uiapp.h"
#include "uix11componentimp.h"
#include <stdlib.h>
#include <stdio.h>

// int iDeltaPerLine = 120;

static psy_ui_X11App* x11app = NULL;

static int shapeEventBase, shapeErrorBase;

// double click
static int buttonclicks = 0;
static int buttonclickcounter = 0;
static int doubleclicktime = 200;
static psy_ui_MouseEvent buttonpressevent;

// prototypes

// virtual
static void psy_ui_x11app_dispose(psy_ui_X11App*);
static int psy_ui_x11app_run(psy_ui_X11App*);
static void psy_ui_x11app_stop(psy_ui_X11App*);
static void psy_ui_x11app_close(psy_ui_X11App*);



static int handleevent(psy_ui_X11App*, XEvent*);
static void handle_mouseevent(psy_ui_Component* component,
	psy_ui_x11_ComponentImp* x11imp,
	uintptr_t hwnd, uintptr_t message, uintptr_t wParam, uintptr_t lParam,
	int button, psy_ui_fp_component_onmouseevent fp, psy_Signal* signal);
static void dispose_window(psy_ui_X11App*, Window);
static void expose_window(psy_ui_X11App*, psy_ui_x11_ComponentImp*,
	int x, int y, int width, int height);	
void buttonpress_single(psy_ui_X11App*, psy_ui_x11_ComponentImp*,
	psy_ui_MouseEvent*);
static int translate_x11button(int button);	
static psy_ui_KeyboardEvent translate_keyevent(XKeyEvent*);
static void sendeventtoparent(psy_ui_X11App*, psy_ui_x11_ComponentImp*,
	int mask, XEvent*);
static void adjustcoordinates(psy_ui_Component*, psy_ui_RealPoint* pt);
static int timertick(psy_ui_X11App*);
static void psy_ui_x11app_initdbe(psy_ui_X11App*);

// vtable
static psy_ui_AppImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_X11App* self)
{
	assert(self);

	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose = (psy_ui_fp_appimp_dispose)
			psy_ui_x11app_dispose;
		imp_vtable.dev_run = (psy_ui_fp_appimp_run)psy_ui_x11app_run;
		imp_vtable.dev_stop = (psy_ui_fp_appimp_stop)psy_ui_x11app_stop;
		imp_vtable.dev_close = (psy_ui_fp_appimp_close)psy_ui_x11app_close;
		imp_vtable.dev_onappdefaultschange = (psy_ui_fp_appimp_onappdefaultschange)
			psy_ui_x11app_onappdefaultschange;		
		imp_vtable_initialized = TRUE;
	}
}
	
// implementation
void psy_ui_x11app_init(psy_ui_X11App* self, psy_ui_App* app, void* instance)
{
	static const char szAppClass[] = "PsycleApp";	
	static const char szComponentClass[] = "PsycleComponent";
	int argc = 0;
	bool shape_extension;
	
	psy_ui_appimp_init(&self->imp);
	imp_vtable_init(self);
	self->imp.vtable = &imp_vtable;
	
	x11app = self;
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
	shape_extension = XShapeQueryExtension (self->dpy, &shapeEventBase,
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
	self->timers = NULL;
	psy_table_init(&self->colormap);
	self->dograb = FALSE;
	self->grabwin = 0;
}

void psy_ui_x11app_initdbe(psy_ui_X11App* self)
{
	int major, minor;
	
	self->visual = 0;
	if (XdbeQueryExtension(self->dpy, &major, &minor)) {
		printf("Xdbe (%d.%d) supported, using double buffering\n", major, minor);
		int numScreens = 1;
		Drawable screens[] = { DefaultRootWindow(self->dpy) };
		XdbeScreenVisualInfo *info = XdbeGetVisualInfo(self->dpy, screens, &numScreens);
		if (!info || numScreens < 1 || info->count < 1) {
			fprintf(stderr, "No visuals support Xdbe\n");
			return;
		}

		// Choosing the first one, seems that they have all perflevel of 0,
		// and the depth varies.
		XVisualInfo xvisinfo_templ;
		xvisinfo_templ.visualid = info->visinfo[0].visual; // We know there's at least one
		// As far as I know, screens are densely packed, so we can assume that if at least 1 exists, it's screen 0.
		xvisinfo_templ.screen = 0;
		xvisinfo_templ.depth = info->visinfo[0].depth;

		int matches;
		XVisualInfo *xvisinfo_match =
			XGetVisualInfo(self->dpy, VisualIDMask|VisualScreenMask|VisualDepthMask, &xvisinfo_templ, &matches);

		if (!xvisinfo_match || matches < 1) {
			fprintf(stderr, "Couldn't match a Visual with double buffering\n");
			return;
		}

		/*
		printf("%d supported visuals\n", info->count);
		for (int i = 0; i < info->count; ++i) {
			printf("visual %d/%d: id %d, depth %d, perf %d\n",
					i, info->count,
					info->visinfo[i].visual,
					info->visinfo[i].depth,
					info->visinfo[i].perflevel);
		}
		printf("We got xvisinfo: id: %d, screen %d, depth %d\n",
				xvisinfo_match->visualid, xvisinfo_match->screen, xvisinfo_match->depth);
		*/

		// We can use Visual from the match
		self->visual = xvisinfo_match->visual;
	} else {
		fprintf(stderr, "No Xdbe support\n");
		return;
	}	
}

void psy_ui_x11app_dispose(psy_ui_X11App* self)
{	
	psy_table_dispose(&self->selfmap);
	psy_table_dispose(&self->winidmap);
	XCloseDisplay(self->dpy);
	psy_list_deallocate(&self->timers, NULL);
	psy_table_dispose(&self->colormap);	
//	DeleteObject(self->defaultbackgroundbrush);
}

void widget_expose(Widget w, XtPointer clientdata,
  XmDrawingAreaCallbackStruct* cbs)
{
}

int psy_ui_x11app_run(psy_ui_X11App* self) 
{
	XEvent event;	
	int x11_fd;
	fd_set in_fds;
	struct timeval tv;	
	
	x11_fd = ConnectionNumber(self->dpy);
	
	self->running = TRUE;
	while (self->running) {
		// create a file description set containing x11_fd
        FD_ZERO(&in_fds);
        FD_SET(x11_fd, &in_fds);
        // set timer to 1 ms
        tv.tv_usec = 1000;
        tv.tv_sec = 0;        
		// wait for X event or a timer
        int num_ready_fds = select(x11_fd + 1, &in_fds, NULL, NULL, &tv);
        if (num_ready_fds > 0) {
            // X11 event
		} else if (num_ready_fds == 0) {
            timertick(self);            
        } else {
            printf("X11 select: An error occured!\n");
		}
		while(XPending(self->dpy)) {
            XNextEvent(self->dpy, &event);	      
			handleevent(self, &event);
		}      
    }
    return 0;
}

void psy_ui_x11app_stop(psy_ui_X11App* self)
{
	self->running = FALSE;
//	PostQuitMessage(0);
}

void psy_ui_x11app_close(psy_ui_X11App* self)
{
	self->running = FALSE;
}

int timertick(psy_ui_X11App* self)
{
	psy_List* p;
	
	for (p = self->timers; p != NULL; p = p->next) {
		psy_ui_X11TickCounter* counter;
		
		counter = (psy_ui_X11TickCounter*)psy_list_entry(p);
		if (counter->tick == 0) {
			psy_ui_x11_ComponentImp* imp;
	
			imp = (psy_ui_x11_ComponentImp*)psy_table_at(&self->selfmap,
				(uintptr_t)counter->hwnd);
			if (imp && imp->component) {
				if (imp->component->signal_timer.slots) {
					psy_signal_emit(&imp->component->signal_timer,
						imp->component, 1, counter->id);
				}
			}			
			counter->tick = counter->numticks;			
		}
		if (counter->tick > 0) {
			--counter->tick;
		}	
	}
	if (buttonclicks == 1 && buttonclickcounter > 0) {
		--buttonclickcounter;
	} else {
		buttonclicks = 0;
	}
}

void psy_ui_x11app_starttimer(psy_ui_X11App* self, uintptr_t hwnd, uintptr_t id,
	uintptr_t interval)
{
	psy_ui_X11TickCounter* counter;
		
	counter = (psy_ui_X11TickCounter*)malloc(sizeof(psy_ui_X11TickCounter));
	counter->doubleclick = FALSE;
	counter->hwnd = hwnd;
	counter->id = id;
	counter->numticks = interval;
	counter->tick = 0;
	psy_list_append(&self->timers, counter);
}

void psy_ui_x11app_stoptimer(psy_ui_X11App* self, uintptr_t hwnd, uintptr_t id)
{
	psy_List* p;
	
	for (p = self->timers; p != NULL; p = p->next) {
		psy_ui_X11TickCounter* counter;
		
		counter = (psy_ui_X11TickCounter*)psy_list_entry(p);
		if (counter->hwnd == hwnd && counter->id == id) {
			psy_list_remove(&self->timers, p);
			free(counter);
			break;
		}
	}
}

int handleevent(psy_ui_X11App* self, XEvent* event)
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
		case DestroyNotify:
			dispose_window(self, imp->hwnd);
			break;
		case NoExpose:
			//expose_window(self, imp,
				//event->xnoexpose.x, event->xnoexpose.y,
				//event->xnoexpose.width, event->xnoexpose.height);
			break;
		case GraphicsExpose:
			//printf("GraphicsExpose\n");
			//expose_window(self, imp,
				//event->xgraphicsexpose.x, event->xgraphicsexpose.y,
				//event->xgraphicsexpose.width, event->xgraphicsexpose.height);
			break;
		case Expose: {	
			const psy_ui_Border* border;							
			psy_ui_RealRectangle r;
			
			 r = psy_ui_realrectangle_make(
				   psy_ui_realpoint_make(
						event->xexpose.x,
						event->xexpose.y),
				   psy_ui_realsize_make(
					event->xexpose.width,
					event->xexpose.height));
			psy_ui_realrectangle_union(&imp->exposearea, &r);
			/*if (event->xexpose.count > 0) {			
				return 0;
			}*/
			border = psy_ui_component_border(imp->component);
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
				// reset scroll origin
				gx11->org.x = 0;
				gx11->org.y = 0;			
				// prepare a clip rect that can be used by a component to
				// optimize the draw amount
				psy_ui_setrectangle(&imp->g.clip, event->xexpose.x,
					event->xexpose.y, event->xexpose.width,
					event->xexpose.height);
				// set gc/xfd clip
				rectangle.x = (short)imp->exposearea.left;
				rectangle.y = (short)imp->exposearea.top;
				rectangle.width = (unsigned short)(imp->exposearea.right -
					imp->exposearea.left);
				rectangle.height = (unsigned short)(imp->exposearea.bottom -
					imp->exposearea.top);
				XUnionRectWithRegion(&rectangle, gx11->region, gx11->region);
				XSetRegion(self->dpy, gx11->gc, gx11->region);
				XftDrawSetClipRectangles(gx11->xfd,0,0,&rectangle,1);
				XDestroyRegion(gx11->region);
				gx11->region = XCreateRegion();	
				// draw						
				imp->imp.vtable->dev_draw(&imp->imp, &imp->g);					
				psy_ui_realrectangle_init(&imp->exposearea);
			}							
			if (self->dbe) {
				psy_ui_x11_GraphicsImp* gx11;
										
				gx11 = (psy_ui_x11_GraphicsImp*)imp->g.imp;
				XCopyArea(self->dpy, imp->d_backBuf, 
					imp->hwnd, gx11->gc, event->xexpose.x, event->xexpose.y,
				event->xexpose.width, event->xexpose.height, event->xexpose.x,
				event->xexpose.y);
			}			
			break; }
		case MapNotify:
			if (self->dograb && imp->hwnd == self->grabwin) {			
				XGrabPointer(self->dpy,self->grabwin,True,
				PointerMotionMask | ButtonReleaseMask | ButtonPressMask,
				GrabModeAsync,
			    GrabModeAsync,None,None,CurrentTime);			    
			}	
			break;
		case UnmapNotify:
			if (self->dograb && imp->hwnd == self->grabwin) {
				self->dograb = FALSE;
			}
			break;
		case ConfigureNotify: {			
			XConfigureEvent xce = event->xconfigure;
										
			if (xce.width != imp->prev_w || xce.height != imp->prev_h) {
				psy_ui_Size size;				
					
				imp->prev_w = xce.width;
				imp->prev_h = xce.height;
				if (self->dbe) {
					psy_ui_x11_GraphicsImp* gx11;
										
					gx11 = (psy_ui_x11_GraphicsImp*)imp->g.imp;
					psy_ui_x11_graphicsimp_updatexft(gx11);
				}
				// if (imp->component->alignchildren) {
					psy_ui_component_align(imp->component);
				// }
				size.width = psy_ui_value_make_px(xce.width);
				size.height = psy_ui_value_make_px(xce.height);				
				imp->component->vtable->onsize(imp->component, &size);
				if (psy_ui_component_overflow(imp->component) !=
						psy_ui_OVERFLOW_HIDDEN) {
					psy_ui_component_updateoverflow(imp->component);						
				}
				psy_signal_emit(&imp->component->signal_size, imp->component, 1,
					(void*)&size);
			}
			return 0 ;			
		break; }
		case ClientMessage:
            if (event->xclient.data.l[0] == self->wmDeleteMessage) {
				XEvent e;
				
				printf("close request\n");
				uintptr_t hwnd;
                bool close;

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
					XNextEvent(self->dpy, event);
					if (event->type ==  DestroyNotify) {
						dispose_window(self, event->xany.window);
						if (hwnd == event->xany.window) {
							printf("cleaned up\n");
							break;
						}
					}
				}
				self->running = FALSE;				
			}		
            break;
        case KeyPress: {
			psy_ui_KeyboardEvent ev;
			
			ev = translate_keyevent(&event->xkey);			
			imp->component->vtable->onkeydown(imp->component, &ev);
			psy_signal_emit(&imp->component->signal_keydown, imp->component,
				1, &ev);
			if (ev.event.bubbles != FALSE && imp->parent && imp->parent->hwnd) {
				XKeyEvent xkevent;

				xkevent = event->xkey;
				xkevent.window      = imp->parent->hwnd;				
				XSendEvent(self->dpy, imp->parent->hwnd, True, KeyPressMask,
					(XEvent*)&xkevent);
			}			
			return 0;
			break; }
        case KeyRelease: {
			psy_ui_KeyboardEvent ev;
							
			ev = translate_keyevent(&event->xkey);		
			//(int)wParam, lParam, 
			//	GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
			//	(lParam & 0x40000000) == 0x40000000);
			imp->component->vtable->onkeyup(imp->component, &ev);
			psy_signal_emit(&imp->component->signal_keyup, imp->component,
				1, &ev);
			if (ev.event.bubbles != FALSE && imp->parent && imp->parent->hwnd) {
				XKeyEvent xkevent;

				xkevent = event->xkey;
				xkevent.window      = imp->parent->hwnd;				
				XSendEvent(self->dpy, imp->parent->hwnd, True, KeyReleaseMask,
					(XEvent*)&xkevent);
			}
			return 0;
			break; }
        case ButtonPress: {	
			psy_ui_MouseEvent ev;
			
			if (self->dograb) {
				psy_ui_x11_ComponentImp* grabimp;
				psy_ui_Component* curr;
				
				grabimp = (psy_ui_x11_ComponentImp*)psy_table_at(&self->selfmap,
					(uintptr_t)self->grabwin);
				
				curr = imp->component;
				while (curr && curr != grabimp->component) {
					curr = psy_ui_component_parent(curr);
				}
				if (!curr) {					
					psy_ui_component_hide(grabimp->component);
					return 0;
				}			
			}							
			handle_mouseevent(
				imp->component, imp, event->xany.window,
				ButtonPress, event->xbutton.x, event->xbutton.y,
				translate_x11button(event->xbutton.button),					
				imp->component->vtable->onmousedown,
				&imp->component->signal_mousedown);									
			/*psy_ui_mouseevent_init(&ev,
				event->xbutton.x,
				event->xbutton.y,
				translate_x11button(event->xbutton.button),
				0,
				0,
				0);
				//(SHORT)LOWORD (lParam), 
				//(SHORT)HIWORD (lParam), MK_RBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);*/
			adjustcoordinates(imp->component, &ev.pt);
			/*if (buttonclicks == 0) {
				buttonpressevent = ev;										
				buttonpress_single(self, imp, &ev);
				buttonclicks = 1;				
				buttonclickcounter = doubleclicktime;
			} else {
				// No timeout
				// stop click timer
				buttonclicks = 0;
				// check distance
				if (ev.pt.x != buttonpressevent.pt.x ||
						ev.pt.y != buttonpressevent.pt.y) {
					// single click
					buttonpress_single(self, imp, &ev);
				} else {
					// double click			
					imp->component->vtable->onmousedoubleclick(imp->component, &ev);
					psy_signal_emit(&imp->component->signal_mousedoubleclick, imp->component, 1,
						&ev);
					while (ev.event.bubble != FALSE && imp->parent && imp->parent->hwnd) {
						imp = imp->parent;
						imp->component->vtable->onmousedoubleclick(imp->component, &ev);
						psy_signal_emit(&imp->component->signal_mousedoubleclick, imp->component, 1,
							&ev);
					}
				}			
			}*/					
			return 0;			
			break; }
		case ButtonRelease: {			
			psy_ui_MouseEvent ev;

			psy_ui_mouseevent_init_all(
				&ev,
				psy_ui_realpoint_make(event->xbutton.x, event->xbutton.y),
				translate_x11button(event->xbutton.button),
				0, 0, 0);
				//(SHORT)LOWORD (lParam), 
				//(SHORT)HIWORD (lParam), MK_RBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
			adjustcoordinates(imp->component, &ev.pt);
			imp->component->vtable->onmouseup(imp->component, &ev);
			psy_signal_emit(&imp->component->signal_mouseup, imp->component, 1,
					&ev);			
			return 0;
			break; }
		case MotionNotify: {
			psy_ui_MouseEvent ev;
			XMotionEvent xme;
			
			if (buttonclicks == 1) {
				buttonclicks = 0;
			}
			xme = event->xmotion;
			psy_ui_mouseevent_init_all(&ev,
				psy_ui_realpoint_make(xme.x, xme.y),
				0, // button
				0,
				0,
				0);
				//(SHORT)LOWORD (lParam), 
				//(SHORT)HIWORD (lParam), MK_RBUTTON, 0,
					//GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
			adjustcoordinates(imp->component, &ev.pt);
			imp->component->vtable->onmousemove(imp->component, &ev);
			psy_signal_emit(&imp->component->signal_mousemove, imp->component, 1,
					&ev);
			return 0;
			break; }	
		case EnterNotify: {
			imp->imp.vtable->dev_mouseenter(&imp->imp);
		break; }
		case LeaveNotify: {
			imp->imp.vtable->dev_mouseleave(&imp->imp);
		break; }
		  default:
			break;
	  }
	return 0;
}

void buttonpress_single(psy_ui_X11App* self, psy_ui_x11_ComponentImp* imp,
	psy_ui_MouseEvent* ev)
{
	imp->component->vtable->onmousedown(imp->component, ev);
	psy_signal_emit(&imp->component->signal_mousedown, imp->component, 1, ev);
}

void dispose_window(psy_ui_X11App* self, Window window)
{
	psy_ui_x11_ComponentImp* imp;
		
	imp = (psy_ui_x11_ComponentImp*)psy_table_at(
		&self->selfmap, (uintptr_t) window);	
	if (imp) {		
		if (imp->component) {
			psy_ui_component_dispose(imp->component);
		} else {
			imp->imp.vtable->dev_dispose(&imp->imp);
		}
		psy_table_remove(&self->selfmap, (uintptr_t)imp->hwnd);		
	}
}

void adjustcoordinates(psy_ui_Component* component, psy_ui_RealPoint* pt)
{	
	psy_ui_RealMargin spacing;
	
	spacing = psy_ui_component_spacing_px(component);	
	if (!psy_ui_realmargin_iszero(&spacing)) {				
		pt->x -= spacing.left;
		pt->y -= spacing.top;
	}
}

int translate_x11button(int button)
{
	int rv;
	
	switch (button) {
		case 1: // left button
			rv = 1;
			break;
		case 2: // middle button
			rv = 3;
			break;
		case 3: // right button
			rv = 2;
			break;
		default:
			rv = 1;
			break;
	}
	return rv;
}	

psy_ui_KeyboardEvent translate_keyevent(XKeyEvent* event)
{
	psy_ui_KeyboardEvent rv;
	KeySym keysym = NoSymbol;
	int repeat = 0;					
	static unsigned char buf[64];
	static unsigned char bufnomod[2];
	int ret;
	XKeyEvent xkevent;
	bool shift;
	bool ctrl;
	
	xkevent = *event;	
	shift = (xkevent.state & ShiftMask) == ShiftMask;
	ctrl = (xkevent.state & ControlMask) == ControlMask;					
	ret = XLookupString(&xkevent, buf, sizeof buf, &keysym, 0);
	switch (keysym) {
		case XK_Home:
			keysym = psy_ui_KEY_HOME;
			break;
		case XK_Escape:
			keysym = psy_ui_KEY_ESCAPE;
			break;
		case XK_Return:
			keysym = psy_ui_KEY_RETURN;
			break;
		case XK_Tab:
			keysym = psy_ui_KEY_TAB;
			break;
		case XK_Prior:
			keysym = psy_ui_KEY_PRIOR;
			break;
		case XK_Next:
			keysym = psy_ui_KEY_NEXT;
			break;
		case XK_Left:
			keysym = psy_ui_KEY_LEFT;
			break;
		case XK_Up:
			keysym = psy_ui_KEY_UP;
			break;
		case XK_Right:
			keysym = psy_ui_KEY_RIGHT;
			break;
		case XK_Down:
			keysym = psy_ui_KEY_DOWN;
			break;
		case XK_Delete:
			keysym = psy_ui_KEY_DELETE;
			break;
		case XK_BackSpace:
			keysym = psy_ui_KEY_BACK;
			break;
		case XK_F1:
			keysym = psy_ui_KEY_F1;
			break;
		case XK_F2:
			keysym = psy_ui_KEY_F2;
			break;
		case XK_F3:
			keysym = psy_ui_KEY_F3;
			break;
		case XK_F4:
			keysym = psy_ui_KEY_F4;
			break;
		case XK_F5:
			keysym = psy_ui_KEY_F5;
			break;
		case XK_F6:
			keysym = psy_ui_KEY_F6;
			break;
		case XK_F7:
			keysym = psy_ui_KEY_F7;
			break;
		case XK_F8:
			keysym = psy_ui_KEY_F8;
			break;
		case XK_F9:
			keysym = psy_ui_KEY_F9;
			break;
		case XK_F10:
			keysym = psy_ui_KEY_F10;
			break;
		case XK_F11:
			keysym = psy_ui_KEY_F11;
			break;
		case XK_F12:
			keysym = psy_ui_KEY_F12;
			break;
		default:
			if (ret && buf[0] != '\0') {
				if (buf[0] >= 'A' && buf[0] <= 'Z') {					
					keysym = psy_ui_KEY_A +
						buf[0] - 'A';
				} else if (buf[0] >= 'a' && buf[0] <= 'z') {
					keysym = psy_ui_KEY_A +
						buf[0] - 'a';
				} else if (buf[0] >= '0' && buf[0] <= '9') {
					keysym = psy_ui_KEY_DIGIT0 +
						buf[0] - '0';
				} else {
					keysym = psy_ui_KEY_A; //buf[0];
				}
			}
			break;		
	}
	// if (ret && buf[0] != '\0') {
	// 	keysym = buf[0];
	// 	printf("%d,%d\n", ret, (int)buf[0]);
	// } else {
	// 	printf("no lookup %d\n", keysym);
	// }	
	psy_ui_keyboardevent_init_all(&rv,
		keysym,
		0,
		shift,
		ctrl,
		0,
		repeat);
	return rv;
}

void handle_mouseevent(psy_ui_Component* component,
	psy_ui_x11_ComponentImp* x11imp,
	uintptr_t hwnd, uintptr_t message, uintptr_t wParam, uintptr_t lParam, int button,
	psy_ui_fp_component_onmouseevent fp,
	psy_Signal* signal)
{
	psy_ui_MouseEvent ev;	
	bool up;	

	psy_ui_mouseevent_init_all(&ev,
		psy_ui_realpoint_make(wParam, lParam),
		button, 0, 0, 0); /* GetKeyState(VK_SHIFT) < 0 */
		//GetKeyState(VK_CONTROL) < 0);	
	adjustcoordinates(component, &ev.pt);
	ev.event.target = component; // eventtarget(component));
	up = FALSE;
	if (message == ButtonPress) {
		
		x11imp->imp.vtable->dev_mousedown(&x11imp->imp, &ev);
	}	
	if (ev.event.bubbles != FALSE) {
		fp(component, &ev);
		psy_signal_emit(signal, component, 1, &ev);
	}
	/*if (ev.event.bubble != FALSE) {
		bool bubble;
		
		bubble = sendmessagetoparent(winimp, message, wParam, lParam);
		if (up && !bubble) {
			psy_ui_app_stopdrag(psy_ui_app());
		} else if (message == WM_MOUSEMOVE && !bubble) {
			if (!psy_ui_app()->dragevent.mouse.event.preventdefault) {
				psy_ui_component_setcursor(psy_ui_app()->main,
					psy_ui_CURSORSTYLE_NODROP);
			} else {
				psy_ui_component_setcursor(psy_ui_app()->main,
					psy_ui_CURSORSTYLE_GRAB);
			}
			psy_ui_app()->dragevent.mouse.event.preventdefault = FALSE;
		}
	} else if (up) {		
		psy_ui_app_stopdrag(psy_ui_app());
	}*/
	
}


void sendeventtoparent(psy_ui_X11App* self, psy_ui_x11_ComponentImp* imp,
	int mask,
	XEvent* xev)
{	
	if (xev && psy_table_at(&self->selfmap,
			(uintptr_t)imp->parent->hwnd)) {
		//XEvent event;
		
		//event = *xev;
		//event.xany.window = imp->parent->hwnd;
		// psy_list_append(&winapp->targetids, imp->hwnd);
		// winapp->eventretarget = imp->component;
		//xev->xany.window = imp->parent->hwnd;
		XSendEvent(self->dpy, imp->parent->hwnd, True, mask,
			xev);
	} else {
		//psy_list_free(winapp->targetids);
		//winapp->targetids = NULL;
	}
	//winapp->eventretarget = 0;
}

int psy_ui_x11app_colourindex(psy_ui_X11App* self, psy_ui_Colour color)
{	
	int rv;
	XColor xcolor;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	
	if (psy_table_exists(&self->colormap,
			(uintptr_t)psy_ui_colour_colorref(&color))) {
		rv = (int)(intptr_t)psy_table_at(&self->colormap,
			(uintptr_t)psy_ui_colour_colorref(&color));
	} else {					
		psy_ui_colour_rgb(&color, &r, &g, &b);
		xcolor.red = r * 256;
		xcolor.green = g * 256;
		xcolor.blue = b * 256;	
		if (XAllocColor(self->dpy, DefaultColormap(self->dpy,
				DefaultScreen(self->dpy)), &xcolor)) {
			rv = xcolor.pixel;
		} else {
			rv = BlackPixel(self->dpy, DefaultScreen(self->dpy));
			printf("Warning: can't allocate requested colour\n");
		}
		psy_table_insert(&self->colormap,
			(uintptr_t)psy_ui_colour_colorref(&color),
			(void*)(uintptr_t)rv);
	}
	return rv;
}

void psy_ui_x11app_onappdefaultschange(psy_ui_X11App* self)
{
	
}

#endif

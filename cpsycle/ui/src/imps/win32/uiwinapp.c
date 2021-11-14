/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiwinapp.h"
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
/* local */
#include "../../uicomponent.h"
/* windows */
#include "uiwincomponentimp.h"
#include <excpt.h>
/* common control header */
#include <commctrl.h>
/* platform */
#include "../../detail/trace.h"

static psy_ui_WinApp* winapp = NULL;

/* prototypes */
static psy_ui_KeyboardEvent keyboardevent(psy_ui_EventType, WPARAM, LPARAM);
static psy_ui_MouseEvent mouseevent(int msg, WPARAM, LPARAM);
static bool handle_ctlcolor(int msg, HWND, WPARAM, LPARAM, LRESULT* rv);
static void handle_scroll(psy_ui_win_ComponentImp* imp, HWND hwnd, WPARAM,
	LPARAM, int bar);
static void handle_scrollparam(SCROLLINFO*, WPARAM);
static void adjustcoordinates(psy_ui_Component*, psy_ui_RealPoint*);
static void psy_ui_winapp_onappdefaultschange(psy_ui_WinApp*);
static psy_ui_EventType translate_win_event_type(int message);
static int translate_win_button(int message);

static LPARAM psy_ui_winapp_pack_pt(psy_ui_RealPoint pt)
{
	return (int16_t)pt.x | ((int16_t)pt.y << 16);
}

LRESULT CALLBACK ui_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
static void psy_ui_winapp_registerclasses(psy_ui_WinApp*);

static psy_ui_win_ComponentImp* psy_ui_win_component_details(psy_ui_Component*
	self)
{
	return (psy_ui_win_ComponentImp*)self->imp->vtable->dev_platform(self->imp);
}

/* virtual */
static void psy_ui_winapp_dispose(psy_ui_WinApp*);
static int psy_ui_winapp_run(psy_ui_WinApp*);
static void psy_ui_winapp_stop(psy_ui_WinApp*);
static void psy_ui_winapp_close(psy_ui_WinApp*);
static void psy_ui_winapp_startmousehook(psy_ui_WinApp*);
static void psy_ui_winapp_stopmousehook(psy_ui_WinApp*);
static void psy_ui_winapp_sendevent(psy_ui_WinApp*, psy_ui_Component*,
	psy_ui_Event*);
static psy_ui_Component* psy_ui_winapp_component(psy_ui_WinApp*,
	uintptr_t handle);
psy_ui_win_ComponentImp* psy_ui_winapp_componentimp(psy_ui_WinApp*,
	uintptr_t handle);

/* vtable */
static psy_ui_AppImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_WinApp* self)
{
	assert(self);

	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose =
			(psy_ui_fp_appimp_dispose)
			psy_ui_winapp_dispose;
		imp_vtable.dev_run =
			(psy_ui_fp_appimp_run)
			psy_ui_winapp_run;
		imp_vtable.dev_stop =
			(psy_ui_fp_appimp_stop)
			psy_ui_winapp_stop;
		imp_vtable.dev_close =
			(psy_ui_fp_appimp_close)
			psy_ui_winapp_close;
		imp_vtable.dev_onappdefaultschange =
			(psy_ui_fp_appimp_onappdefaultschange)
			psy_ui_winapp_onappdefaultschange;		
		imp_vtable.dev_startmousehook =
			(psy_ui_fp_appimp_startmousehook)
			psy_ui_winapp_startmousehook;
		imp_vtable.dev_stopmousehook =
			(psy_ui_fp_appimp_stopmousehook)
			psy_ui_winapp_stopmousehook;
		imp_vtable.dev_sendevent =
			(psy_ui_fp_appimp_sendevent)
			psy_ui_winapp_sendevent;
		imp_vtable.dev_component =
			(psy_ui_fp_appimp_component)
			psy_ui_winapp_component;		
		imp_vtable_initialized = TRUE;
	}
	self->imp.vtable = &imp_vtable;
}

void psy_ui_winapp_init(psy_ui_WinApp* self, psy_ui_App* app, HINSTANCE instance)
{
	static TCHAR szAppClass[] = TEXT("PsycleApp");	
	static TCHAR szComponentClass[] = TEXT("PsycleComponent");	
	HRESULT hr;

	assert(self);

	psy_ui_appimp_init(&self->imp);
	imp_vtable_init(self);
	/* init static winapp reference */
	winapp = self;
	self->app = app;
	self->instance = instance;
	self->appclass = szAppClass;
	self->componentclass = szComponentClass;
	self->winproc = ui_winproc;
	self->comwinproc = ui_com_winproc;
	self->winid = 20000;
	self->mousehook = 0;
	psy_ui_winapp_registerclasses(self);
	hr = CoInitialize(NULL);	
	if (hr == S_FALSE) {
		psy_ui_error(
			"The COM library is already initialized on this thread. ",
			"Warning! psy_ui_winapp_init: CoInitialize already initialized");
	} else if (hr == RPC_E_CHANGED_MODE) {
		psy_ui_error(
			"A previous call to CoInitializeEx specified the concurrency model "
			"for this thread as multithread apartment (MTA). This could also "
			"indicate that a change from neutral-threaded apartment to "
			"single-threaded apartment has occurred. ",
			"Warning! psy_ui_winapp_init: CoInitialize RPC_E_CHANGED_MODE");
	}
	psy_table_init(&self->selfmap);
	psy_table_init(&self->winidmap);
	self->defaultbackgroundbrush = CreateSolidBrush(0x00232323);	
}

void psy_ui_winapp_dispose(psy_ui_WinApp* self)
{
	psy_ui_winapp_stopmousehook(self);
	psy_table_dispose(&self->selfmap);
	psy_table_dispose(&self->winidmap);	
	DeleteObject(self->defaultbackgroundbrush);
	CoUninitialize();	
}

void psy_ui_winapp_registerclasses(psy_ui_WinApp* self)
{
	WNDCLASS     wndclass ;
	INITCOMMONCONTROLSEX icex;
	int succ;
		
	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
    wndclass.lpfnWndProc   = self->winproc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = (HINSTANCE) self->instance;
    wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
    wndclass.hbrBackground = (HBRUSH)GetStockObject (NULL_BRUSH) ;
    wndclass.lpszMenuName  = NULL ;
    wndclass.lpszClassName = self->appclass;
	if (!RegisterClass (&wndclass))
    {
		MessageBox (NULL, TEXT ("This program requires Windows NT!"), 
                      self->appclass, MB_ICONERROR) ;		
    }
	
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = self->winproc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = sizeof (long); 
	wndclass.hInstance     = self->instance;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject (NULL_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = self->componentclass;
     
	RegisterClass (&wndclass) ;	
	/* Ensure that the common control DLL is loaded. */
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_USEREX_CLASSES;
    succ = InitCommonControlsEx(&icex);	
}

LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	psy_ui_win_ComponentImp* imp;
	psy_ui_Component* component;
	psy_ui_fp_winproc winproc;
	bool preventdefault;

	preventdefault = 0;	
	imp = psy_ui_winapp_componentimp(winapp, (uintptr_t)hwnd);		
	if (imp) {		
		winproc = imp->wndproc;
		component = imp->component;
		switch (message) {
		case WM_NCDESTROY:
			/* restore default winproc */
			if (component) {
				psy_signal_emit(&component->signal_destroyed,
					component, 0);					
				component->vtable->ondestroyed(component);					
			}
#if defined(_WIN64)		
			SetWindowLongPtr(imp->hwnd, GWLP_WNDPROC, (LONG_PTR)
				imp->wndproc);
#else	
			SetWindowLong(imp->hwnd, GWL_WNDPROC, (LONG)imp->wndproc);
#endif				
			if (component) {
				psy_ui_component_dispose(component);
			} else {
				imp->imp.vtable->dev_dispose(&imp->imp);
			}
			psy_table_remove(&winapp->selfmap, (uintptr_t)hwnd);
			break;
		case WM_DESTROY:
			if (component) {					
				psy_signal_emit(&component->signal_destroy, component, 0);
				component->vtable->ondestroy(component);
			}								
			break;			
		case WM_TIMER:				
			if (component) {
				psy_ui_eventdispatch_timer(&winapp->app->eventdispatch,
					component, (uintptr_t)wParam);
			}				
			break;
		case WM_CHAR:
			if (imp->preventwmchar) {
				imp->preventwmchar = 0;
				preventdefault = 1;
			}
			break;
		case WM_KEYDOWN:
		case WM_KEYUP:
			if (imp->component) {
				psy_ui_KeyboardEvent keyevent;

				keyevent = keyboardevent(translate_win_event_type(message),
					wParam, lParam);
				psy_ui_eventdispatch_send(&winapp->app->eventdispatch,
					imp->component, psy_ui_keyboardevent_base(&keyevent));
				if (keyevent.event.default_prevented) {
					imp->preventwmchar = 1;
				}
				preventdefault = keyevent.event.default_prevented;					
			}
			break;			
		case WM_KILLFOCUS:
			if (component) {
				psy_ui_Event ev;

				psy_ui_event_init(&ev, psy_ui_FOCUSOUT);
				psy_ui_event_stop_propagation(&ev);				
				psy_ui_eventdispatch_send(&winapp->app->eventdispatch,
					component, &ev);
			}
			break;	
		case WM_NCPAINT:
			if (component && component->ncpaint) {					
#ifndef DCX_USESTYLE
#define DCX_USESTYLE 0x00010000
#endif
				psy_ui_Colour bgcolour;

				bgcolour = psy_ui_component_backgroundcolour(imp->component);
				if (bgcolour.mode.set) {
					HDC hdc = GetDCEx(hwnd, 0, DCX_WINDOW | DCX_USESTYLE);
					if (hdc) {
						RECT rcclient;
						RECT rcwin;
						POINT ptupleft;
						HRGN rgntemp;
						HBRUSH hbrush;

						GetClientRect(hwnd, &rcclient);							
						GetWindowRect(hwnd, &rcwin);							
						ptupleft.x = rcwin.left;
						ptupleft.y = rcwin.top;
						MapWindowPoints(0, hwnd, (LPPOINT)&rcwin, (sizeof(RECT) / sizeof(POINT)));
						OffsetRect(&rcclient, -rcwin.left, -rcwin.top);
						OffsetRect(&rcwin, -rcwin.left, -rcwin.top);
						rgntemp = NULL;
						if (wParam == NULLREGION || wParam == ERROR) {
							ExcludeClipRect(hdc, rcclient.left, rcclient.top, rcclient.right, rcclient.bottom);
						} else {
							rgntemp = CreateRectRgn(rcclient.left + ptupleft.x, rcclient.top + ptupleft.y,
								rcclient.right + ptupleft.x, rcclient.bottom + ptupleft.y);
							if (CombineRgn(rgntemp, (HRGN)wParam, rgntemp, RGN_DIFF) == NULLREGION) {
								/* nothing to paint */
							}
							OffsetRgn(rgntemp, -ptupleft.x, -ptupleft.y);
							ExtSelectClipRgn(hdc, rgntemp, RGN_AND);
						}
						hbrush = CreateSolidBrush(RGB(bgcolour.r, bgcolour.g, bgcolour.b));
						FillRect(hdc, &rcwin, hbrush);
						DeleteObject(hbrush);
						ReleaseDC(hwnd, hdc);
						if (rgntemp != 0) {
							DeleteObject(rgntemp);
						}
					}
					return 0;
				}
			}
			break;
		case WM_NCCALCSIZE:
			if (component && component->ncpaint && wParam != 0) {
				NCCALCSIZE_PARAMS* nc;					

				nc = (NCCALCSIZE_PARAMS*)lParam;					
				if (nc) {	
					const psy_ui_TextMetric* tm;
					RECT* rc;
					int dy;

					tm = psy_ui_component_textmetric(component);
					rc = &nc->rgrc[0];
					dy = -((((nc->rgrc[0].bottom - nc->rgrc[0].top) -
						(tm->tmHeight - 1)) / 2));
					rc->top -= dy + 1;
					rc->bottom += dy;
				}
				return 0;
			}
			break;
		case WM_MOUSEWHEEL: {
			if (component) {
				int preventdefault = 0;
				psy_ui_MouseEvent ev;
				POINT pt_client;

				pt_client.x = (SHORT)LOWORD(lParam);
				pt_client.y = (SHORT)HIWORD(lParam);
				ScreenToClient(imp->hwnd, &pt_client);
				psy_ui_mouseevent_init_all(&ev,
					psy_ui_realpoint_make(pt_client.x, pt_client.y),
					psy_ui_realpoint_make(pt_client.x, pt_client.y),
					(short)LOWORD(wParam), (short)HIWORD(wParam),
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				component->vtable->onmousewheel(component, &ev);
				psy_signal_emit(&component->signal_mousewheel, component, 1,
					&ev);
				preventdefault = ev.event.default_prevented;
			}
			break; }			
		default:
			break;
		}
		if (preventdefault) {
			return 0;
		}
		return CallWindowProc(winproc, hwnd, message, wParam, lParam);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK ui_winproc (HWND hwnd, UINT message, 
                               WPARAM wParam, LPARAM lParam)
{    
	psy_ui_win_ComponentImp* imp;
	psy_ui_Component* component;	
	LRESULT lres;

	if (handle_ctlcolor(message, hwnd, wParam, lParam, &lres)) {
		return lres;
	}	
	imp = psy_ui_winapp_componentimp(winapp, (uintptr_t)hwnd);	
	if (imp) {
		component = imp->component;
		switch (message) {
		case WM_SHOWWINDOW:
			if (component) {
				if (wParam != FALSE) {
					psy_signal_emit(&component->signal_show, component, 0);
				} else {
					psy_signal_emit(&component->signal_hide, component, 0);
				}
			}
			return 0;
		case WM_SIZE:
			if (component) {
				psy_ui_Event ev;

				imp->sizecachevalid = FALSE;
				psy_ui_event_init(&ev, psy_ui_RESIZE);
				psy_ui_event_stop_propagation(&ev);				
				psy_ui_eventdispatch_send(&winapp->app->eventdispatch,
					component, &ev);
			}
			return 0;
		case WM_TIMER:
			if (component) {
				psy_ui_eventdispatch_timer(&winapp->app->eventdispatch,
					component, (uintptr_t)wParam);
			}
			return 0;		
		case WM_ERASEBKGND:
			return 1;
		case WM_COMMAND:
			imp = psy_table_at(&winapp->winidmap, (uintptr_t)LOWORD(wParam));
			if (imp && imp->component &&
				imp->component->signal_command.slots) {
				psy_signal_emit(&imp->component->signal_command,
					imp->component, 2, wParam, lParam);				
			} else if (imp && imp->imp.signal_command.slots) {
				psy_signal_emit(&imp->imp.signal_command, imp->component, 2,
					wParam, lParam);				
			}
			return 0;			
		case WM_CREATE:
			psy_signal_emit(&imp->component->signal_create, component, 0);
			return 0;			
		case WM_PAINT: {
			const psy_ui_Border* border;

			border = psy_ui_component_border(imp->component);
			if (imp->component->vtable->ondraw ||
					imp->component->signal_draw.slots ||
					imp->component->backgroundmode != psy_ui_NOBACKGROUND ||
					psy_ui_border_isset(border)) {
				HDC hdc;
				PAINTSTRUCT ps;
				psy_ui_RealSize clipsize;

				hdc = BeginPaint(hwnd, &ps);
				/* store clip / repaint size of paint request */
				clipsize = psy_ui_realsize_make(
					(double)ps.rcPaint.right - (double)ps.rcPaint.left,
					(double)ps.rcPaint.bottom - (double)ps.rcPaint.top);
				/* anything to paint ? */				
				if (clipsize.width > 0.0 && clipsize.height > 0.0) {
					psy_ui_Graphics	g;					
					psy_ui_Bitmap dblbuffer;

					if (psy_ui_component_doublebuffered(component)) {
						/*
						** create a graphics context with back buffer bitmap
						** with origin (0; 0) and size of the paint request
						*/
						psy_ui_bitmap_init_size(&dblbuffer, clipsize);							
						psy_ui_graphics_init_bitmap(&g, &dblbuffer);						
						/* translate paint request to buffer 0, 0 origin */
						psy_ui_setorigin(&g, psy_ui_realpoint_make(
							ps.rcPaint.left, ps.rcPaint.top));						
					} else {
						/* create graphics handle with the paint hdc */
						psy_ui_graphics_init(&g, hdc);						
					}
					/* update graphics font with component font */
					psy_ui_setfont(&g, psy_ui_component_font(imp->component));
					/* set clip */
					psy_ui_setrectangle(&g.clip, ps.rcPaint.left,
						ps.rcPaint.top, clipsize.width, clipsize.height);
					/* draw */
					imp->imp.vtable->dev_draw(&imp->imp, &g);
					if (psy_ui_component_doublebuffered(component)) {
						/* copy the double buffer bitmap to the paint hdc */						
						BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
							(int)clipsize.width, (int)clipsize.height,
							(HDC)psy_ui_graphics_dev_gc(&g), 0, 0, SRCCOPY);
						psy_ui_bitmap_dispose(&dblbuffer);
					}
					psy_ui_graphics_dispose(&g);
				}
				EndPaint(hwnd, &ps);
				return 0;
			}
			break; }
		case WM_NCDESTROY: {
			bool deallocate;

			deallocate = FALSE;
			if (component) {
				deallocate = component->deallocate;
				psy_signal_emit(&component->signal_destroyed, component, 0);
				component->vtable->ondestroyed(component);
			}
			psy_ui_component_dispose(component);
			psy_table_remove(&winapp->selfmap, (uintptr_t)hwnd);
			if (deallocate) {
				free(component);
			}
			return 0;
			break; }
		case WM_DESTROY:
			if (component) {
				psy_signal_emit(&component->signal_destroy, component, 0);
				component->vtable->ondestroy(component);			
				return 0;
			}
			break;
		case WM_CLOSE: {
			if (component) {
				bool close;

				close = component->vtable->onclose(component);
				psy_signal_emit(&component->signal_close,
					component, 1, (void*)&close);
				if (!close) {
					return 0;
				}
			}
			break; }
		case WM_SYSKEYDOWN:
			if (!(component && (wParam >= VK_F10 && wParam <= VK_F12 ||
					wParam >= 0x41 && wParam <= psy_ui_KEY_Z ||
					wParam >= psy_ui_KEY_DIGIT0 &&
					wParam <= psy_ui_KEY_DIGIT9))) {
				return 0;
			}
			/* fallthrough */
		case WM_KEYDOWN:
		case WM_KEYUP:
			if (component) {
				psy_ui_KeyboardEvent keyevent;

				keyevent = keyboardevent(translate_win_event_type(message),
					wParam, lParam);
				psy_ui_eventdispatch_send(&winapp->app->eventdispatch,
					component, psy_ui_keyboardevent_base(&keyevent));
				if (psy_ui_event_default_prevented(&keyevent.event)) {
					imp->preventwmchar = 1;
				}		
				return 0;
			}
			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_LBUTTONDOWN: 
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:	
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_MOUSEMOVE:
			if (component) {
				psy_ui_MouseEvent ev;

				ev = mouseevent(message, wParam, lParam);
				adjustcoordinates(component, &ev.pt);
				if (component->id == 100) {
					component = component;
				}
				psy_ui_eventdispatch_send(&winapp->app->eventdispatch,
					component, psy_ui_mouseevent_base(&ev));
				return 0;
			}
			break;
		case WM_SETTINGCHANGE: {
			int ulScrollLines;

			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0,
				&ulScrollLines, 0);      
			/*
			** ulScrollLines usually equals 3 or 0 (for no scrolling)
			** WHEEL_DELTA equals 120, so deltaperline will be 40
			*/
			if (ulScrollLines) {
				psy_ui_app()->deltaperline = WHEEL_DELTA / ulScrollLines;
			} else {
				psy_ui_app()->deltaperline = 0;
			}
			return 0;
			break; }
		case WM_MOUSEWHEEL:	{
			if (component) {
				psy_ui_MouseEvent ev;
				POINT pt_client;

				pt_client.x = (SHORT)LOWORD(lParam);
				pt_client.y = (SHORT)HIWORD(lParam);
				ScreenToClient(imp->hwnd, &pt_client);
				psy_ui_mouseevent_init_all(&ev,
					psy_ui_realpoint_make(pt_client.x, pt_client.y),
					psy_ui_realpoint_make(pt_client.x, pt_client.y),
					(short)LOWORD(wParam), (short)HIWORD(wParam),
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(component, &ev.pt);
				psy_ui_component_mousewheel(component, &ev,
					(short)HIWORD(wParam) /* 120 or -120 */);
			}
			break; }
		case WM_MOUSEHOVER:
			if (component) {
				psy_signal_emit(&component->signal_mousehover, component, 0);
				return 0;
			}
			break;
		case WM_MOUSELEAVE:
			if (component) {
				psy_ui_Event ev;

				psy_ui_event_init(&ev, psy_ui_MOUSELEAVE);
				psy_ui_event_stop_propagation(&ev);
				psy_ui_eventdispatch_send(&winapp->app->eventdispatch,
					component, &ev);				
			}			
			return 0;
		case WM_VSCROLL:			
			if (component) {
				handle_scroll(imp, hwnd, wParam, lParam, SB_VERT);
				return 0;
			}
			break;
		case WM_HSCROLL:
			if (component) {
				handle_scroll(imp, hwnd, wParam, lParam, SB_HORZ);
				return 0;
			}
			break;
		case WM_KILLFOCUS:
			if (component) {
				psy_ui_Event ev;

				psy_ui_event_init(&ev, psy_ui_FOCUSOUT);
				psy_ui_event_stop_propagation(&ev);
				psy_ui_eventdispatch_send(&winapp->app->eventdispatch,
					component, &ev);
				return 0;
			}							
			break;
		case WM_NCACTIVATE:
			if (component && component->dropdown && (wParam == 0)) {
				EnableWindow(GetParent(imp->hwnd), TRUE);						
				psy_ui_component_hide(component);				
			}
			break;
		default:			
			break;
		}	
	}
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}

psy_ui_EventType translate_win_event_type(int message)
{	
	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		return psy_ui_MOUSEDOWN;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		return psy_ui_MOUSEUP;
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
		return psy_ui_DBLCLICK;
	case WM_MOUSEMOVE:
		return psy_ui_MOUSEMOVE;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		return psy_ui_KEYDOWN;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		return psy_ui_KEYUP;
	default:
		return psy_ui_UNKNOWNEVENT;
	}
}

bool handle_ctlcolor(int msg, HWND hwnd, WPARAM wparam, LPARAM lparam,
	LRESULT* rv)
{
	psy_ui_win_ComponentImp* imp;	

	*rv = 0;
	switch (msg) {
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLOREDIT: {
		uint32_t colorref;
		uint32_t bgcolorref;
		HBRUSH brush;

		imp = psy_ui_winapp_componentimp(winapp, (uintptr_t)lparam);
		if (!imp) {
			imp = psy_ui_winapp_componentimp(winapp, (uintptr_t)hwnd);
		}
		if (imp && imp->component) {
			psy_ui_Colour colour;

			colour = psy_ui_component_colour(imp->component);
			colorref = psy_ui_colour_colorref(&colour);
			colour = psy_ui_component_backgroundcolour(imp->component);
			bgcolorref = psy_ui_colour_colorref(&colour);
			if (((imp->component->backgroundmode & psy_ui_SETBACKGROUND)
				== psy_ui_SETBACKGROUND) && colour.mode.set) {
				DeleteObject(psy_ui_win_component_details(imp->component)->background);
				psy_ui_win_component_details(imp->component)->background =
					CreateSolidBrush(RGB(colour.r, colour.g, colour.b));
			} else {
				brush = (HBRUSH)GetStockObject(NULL_BRUSH);
			}
			brush = ((imp->component->backgroundmode & psy_ui_SETBACKGROUND)
				== psy_ui_SETBACKGROUND)
				? psy_ui_win_component_details(imp->component)->background
				: (HBRUSH)GetStockObject(NULL_BRUSH);
		} else {
			colorref = psy_ui_colour_colorref(&psy_ui_style_const(psy_ui_STYLE_ROOT)->colour);
			bgcolorref = psy_ui_colour_colorref(&psy_ui_style_const(psy_ui_STYLE_ROOT)->backgroundcolour);
			brush = winapp->defaultbackgroundbrush;
		}
		SetTextColor((HDC)wparam, colorref);
		SetBkColor((HDC)wparam, bgcolorref);
		*rv = (LRESULT)brush;
		return TRUE;
		break; }
	default:
		break;
	}
	return FALSE;
}

int translate_win_button(int message)
{
	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		return 1;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
		return 2;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
		return 3;
	default:
		return 1;
	}
}

psy_ui_KeyboardEvent keyboardevent(psy_ui_EventType type, WPARAM wparam, LPARAM lparam)
{
	psy_ui_KeyboardEvent rv;

	psy_ui_keyboardevent_init_all(&rv, (int)wparam, lparam,
		GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
		GetKeyState(VK_MENU) < 0, (lparam & 0x40000000) == 0x40000000);
	psy_ui_keyboardevent_settype(&rv, type);
	return rv;
}

psy_ui_MouseEvent mouseevent(int msg, WPARAM wparam, LPARAM lparam)
{
	psy_ui_MouseEvent rv;

	psy_ui_mouseevent_init_all(&rv,
		psy_ui_realpoint_make((SHORT)LOWORD(lparam), (SHORT)HIWORD(lparam)),
		psy_ui_realpoint_make((SHORT)LOWORD(lparam), (SHORT)HIWORD(lparam)),
		translate_win_button(msg), 0, GetKeyState(VK_SHIFT) < 0,
		GetKeyState(VK_CONTROL) < 0);
	if (msg == WM_MOUSEMOVE) {
		rv.button = wparam;
	}
	psy_ui_mouseevent_settype(&rv, translate_win_event_type(msg));
	return rv;
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

void handle_scroll(psy_ui_win_ComponentImp* imp, HWND hwnd, WPARAM wParam,
	LPARAM lParam, int bar)
{
	SCROLLINFO si;
	int pos;	

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(hwnd, bar, &si);
	/* Save the position for comparison later on */
	pos = si.nPos;
	handle_scrollparam(&si, wParam);
	/*
	** Set the positionand then retrieve it. Due to adjustments
	** by Windows it may not be the same as the value set.
	*/
	si.fMask = SIF_POS;
	SetScrollInfo(hwnd, bar, &si, TRUE);
	GetScrollInfo(hwnd, bar, &si);
	/* If the position has changed, scroll the windowand update it */
	if (si.nPos != pos) {
		const psy_ui_TextMetric* tm;
		psy_ui_Value scrollpos;		

		tm = psy_ui_component_textmetric(imp->component);		
		if (bar == SB_VERT) {
			scrollpos = psy_ui_component_scrolltop(imp->component);
			psy_ui_component_setscrolltop(imp->component,
				psy_ui_value_make_px(
					psy_ui_value_px(&scrollpos, tm, NULL) -
					psy_ui_component_scrollstep_height_px(imp->component) *
					(pos - si.nPos)));
		} else {
			scrollpos = psy_ui_component_scrollleft(imp->component);
			psy_ui_component_setscrollleft(imp->component,
				psy_ui_value_make_px(
					psy_ui_value_px(&scrollpos, tm, NULL) -
					psy_ui_component_scrollstep_width_px(imp->component) *
					(pos - si.nPos)));
		}
	}
}

void handle_scrollparam(SCROLLINFO* si, WPARAM wparam)
{
	switch (LOWORD(wparam)) {
	case SB_TOP:
		si->nPos = si->nMin ;
		break;
	case SB_BOTTOM:
		si->nPos = si->nMax ;
		break;
	case SB_LINEUP:
		si->nPos -= 1 ;
		break;
	case SB_LINEDOWN:
		si->nPos += 1 ;
		break;
	case SB_PAGEUP:
		si->nPos -= si->nPage ;
		break;
	case SB_PAGEDOWN:
		si->nPos += si->nPage ;
		break;
	case SB_THUMBTRACK:
		si->nPos = (short)HIWORD(wparam);
		break;
	default:
		break;   		
	}	
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0 && winapp && winapp->app && 
			(wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN)) {
		psy_ui_MouseEvent ev;
		MOUSEHOOKSTRUCT* pMouseStruct = (MOUSEHOOKSTRUCT*)lParam;

		psy_ui_mouseevent_init_all(&ev,
			psy_ui_realpoint_make(pMouseStruct->pt.x, pMouseStruct->pt.y),
			psy_ui_realpoint_make(pMouseStruct->pt.x, pMouseStruct->pt.y),
			(wParam == WM_LBUTTONDOWN) ? 0 : 1,
			0, GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
		psy_signal_emit(&winapp->app->signal_mousehook, winapp->app,
			1, &ev);		
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

int psy_ui_winapp_run(psy_ui_WinApp* self) 
{
	MSG msg;
		
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}	
    return (int)msg.wParam ;
}

void psy_ui_winapp_startmousehook(psy_ui_WinApp* self)
{
	if (self->mousehook == 0) {
		self->mousehook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);
	}
}

void psy_ui_winapp_stopmousehook(psy_ui_WinApp* self)
{
	if (self->mousehook) {
		UnhookWindowsHookEx(self->mousehook);
		self->mousehook = NULL;
	}
}

void psy_ui_winapp_stop(psy_ui_WinApp* self)
{
	PostQuitMessage(0);
}

void psy_ui_winapp_close(psy_ui_WinApp* self)
{		
	assert(self);

	if (psy_ui_app_main(winapp->app)) {
		assert(psy_ui_app_main(winapp->app)->imp);

		PostMessage(((psy_ui_win_ComponentImp*)
			(psy_ui_app_main(winapp->app)->imp))->hwnd,
			WM_CLOSE, 0, 0);
	}
}

void psy_ui_winapp_onappdefaultschange(psy_ui_WinApp* self)
{
	DeleteObject(self->defaultbackgroundbrush);
	self->defaultbackgroundbrush = CreateSolidBrush(
		psy_ui_colour_colorref(&psy_ui_style_const(psy_ui_STYLE_ROOT)->backgroundcolour));
}

void psy_ui_winapp_sendevent(psy_ui_WinApp* self, psy_ui_Component* component,
	psy_ui_Event* ev)
{
	psy_ui_win_ComponentImp* imp;

	assert(self);
	assert(component);
	assert(ev);

	imp = (psy_ui_win_ComponentImp*)component->imp;
	if (!imp) {
		return;
	}
	switch (ev->type) {
	case psy_ui_KEYDOWN: {
		psy_ui_KeyboardEvent* keyevent;

		keyevent = (psy_ui_KeyboardEvent*)ev;
		SendMessage(imp->hwnd, (UINT)WM_KEYDOWN, (WPARAM)keyevent->keycode,
			(LPARAM)keyevent->keydata);	
		break; }
	case psy_ui_KEYUP: {
		psy_ui_KeyboardEvent* keyevent;

		keyevent = (psy_ui_KeyboardEvent*)ev;
		SendMessage(imp->hwnd, (UINT)WM_KEYUP, (WPARAM)keyevent->keycode,
			(LPARAM)keyevent->keydata);
		break; }
	case psy_ui_MOUSEDOWN: {
		psy_ui_MouseEvent* mouseevent;
		UINT msg;

		mouseevent = (psy_ui_MouseEvent*)ev;		
		switch (mouseevent->button) {
		case 1: msg = (UINT)WM_LBUTTONDOWN; break;
		case 2: msg = (UINT)WM_RBUTTONDOWN; break;
		case 3: msg = (UINT)WM_MBUTTONDOWN; break;
		default: msg = 0; break;
		}
		if (msg != 0) {		
			SendMessage(imp->hwnd, msg, (WPARAM)mouseevent->button,
				psy_ui_winapp_pack_pt(mouseevent->pt));
		}
		break; }
	case psy_ui_MOUSEUP: {
		psy_ui_MouseEvent* mouseevent;
		UINT msg;

		mouseevent = (psy_ui_MouseEvent*)ev;		
		switch (mouseevent->button) {
		case 1: msg = (UINT)WM_LBUTTONUP; break;
		case 2: msg = (UINT)WM_RBUTTONUP; break;
		case 3: msg = (UINT)WM_MBUTTONUP; break;
		default: msg = 0; break;
		}		
		if (msg != 0) {
			SendMessage(imp->hwnd, msg, (WPARAM)mouseevent->button,
				psy_ui_winapp_pack_pt(mouseevent->pt));
		}		
		break; }
	case psy_ui_DBLCLICK : {
		psy_ui_MouseEvent* mouseevent;
		UINT msg;

		mouseevent = (psy_ui_MouseEvent*)ev;
		switch (mouseevent->button) {
		case 1: msg = (UINT)WM_LBUTTONDBLCLK; break;
		case 2: msg = (UINT)WM_RBUTTONDBLCLK; break;
		case 3: msg = (UINT)WM_MBUTTONDBLCLK; break;
		default: msg = 0; break;
		}
		if (msg != 0) {			
			SendMessage(imp->hwnd, msg, (WPARAM)mouseevent->button,
				psy_ui_winapp_pack_pt(mouseevent->pt));
		}
		break; }
	case psy_ui_MOUSEMOVE: {
		psy_ui_MouseEvent* mouseevent;		

		mouseevent = (psy_ui_MouseEvent*)ev;		
		SendMessage(imp->hwnd, (UINT)WM_MOUSEMOVE,
			(WPARAM)mouseevent->button,
			psy_ui_winapp_pack_pt(mouseevent->pt));
		break; }
	default:
		break;
	}
}

psy_ui_Component* psy_ui_winapp_component(psy_ui_WinApp* self,
	uintptr_t handle)
{	
	psy_ui_win_ComponentImp* imp;
	
	imp = psy_ui_winapp_componentimp(self, handle);
	if (imp) {
		return imp->component;
	}
	return NULL;
}

psy_ui_win_ComponentImp* psy_ui_winapp_componentimp(psy_ui_WinApp* self,
	uintptr_t handle)
{
	return (psy_ui_win_ComponentImp*)psy_table_at(&winapp->selfmap, handle);
}

#endif /* PSYCLE_TK_WIN32 */

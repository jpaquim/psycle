// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwinapp.h"
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
// local
#include "../../uicomponent.h"
// windows
#include "uiwincomponentimp.h"
#include "uiwinfontimp.h"
#include "uiwingraphicsimp.h"
#include <excpt.h>
// common control header
#include <commctrl.h>
// platform
#include "../../detail/trace.h"

static psy_ui_WinApp* winapp = NULL;

static psy_ui_KeyboardEvent keyboardevent(psy_ui_EventType, WPARAM wParam, LPARAM lParam);
static void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_scrollparam(HWND hwnd, SCROLLINFO* si, WPARAM wParam);
static void adjustcoordinates(psy_ui_Component*, psy_ui_RealPoint* pt);
static void psy_ui_winapp_onappdefaultschange(psy_ui_WinApp* self);

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
	uintptr_t platformhandle);
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
}

void psy_ui_winapp_init(psy_ui_WinApp* self, psy_ui_App* app, HINSTANCE instance)
{
	static TCHAR szAppClass[] = TEXT("PsycleApp");	
	static TCHAR szComponentClass[] = TEXT("PsycleComponent");	
	HRESULT hr;

	assert(self);

	psy_ui_appimp_init(&self->imp);
	imp_vtable_init(self);
	self->imp.vtable = &imp_vtable;
	// init static winapp reference
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
    wndclass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH) ;
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
	wndclass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = self->componentclass;
     
	RegisterClass (&wndclass) ;	
	// Ensure that the common control DLL is loaded.     		
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_USEREX_CLASSES;
    succ = InitCommonControlsEx(&icex);	
}

LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	psy_ui_win_ComponentImp* imp;	
	psy_ui_fp_winproc winproc;
	bool preventdefault;

	preventdefault = 0;	
	imp = (psy_ui_win_ComponentImp*)psy_table_at(&winapp->selfmap, (uintptr_t)
		hwnd);
	if (imp) {		
		winproc = imp->wndproc;
		switch (message)
		{
			case WM_NCDESTROY:
				// restore default winproc
				if (imp->component) {					
					psy_signal_emit(&imp->component->signal_destroyed,
						imp->component, 0);					
					imp->component->vtable->ondestroyed(imp->component);
					
				}
#if defined(_WIN64)		
				SetWindowLongPtr(imp->hwnd, GWLP_WNDPROC, (LONG_PTR)
					imp->wndproc);
#else	
				SetWindowLong(imp->hwnd, GWL_WNDPROC, (LONG)imp->wndproc);
#endif				
				if (imp->component) {
					psy_ui_component_dispose(imp->component);
				} else {
					imp->imp.vtable->dev_dispose(&imp->imp);
				}
				psy_table_remove(&winapp->selfmap, (uintptr_t)hwnd);
				break;
			case WM_DESTROY:
				if (imp->component) {					
					psy_signal_emit(&imp->component->signal_destroy,
						imp->component, 0);					
					imp->component->vtable->ondestroy(imp->component);
				}								
				break;			
			case WM_TIMER:				
				if (imp->component) {
					psy_ui_eventdispatch_timer(&winapp->app->eventdispatch,
						imp->component, (uintptr_t)wParam);
				}				
				break;
			case WM_CHAR:
				if (imp->preventwmchar) {
					imp->preventwmchar = 0;
					preventdefault = 1;
				}
				break;
			case WM_KEYDOWN:
				if (imp->component) {
					psy_ui_KeyboardEvent ev;

					ev = keyboardevent(psy_ui_KeyPress, wParam, lParam);
					psy_ui_eventdispatch_keydown(&winapp->app->eventdispatch,
						imp->component, &ev);
					if (ev.event.default_prevented) {
						imp->preventwmchar = 1;
					}
					preventdefault = ev.event.default_prevented;
				}				
				break;
			case WM_KEYUP:
				if (imp->component) {
					psy_ui_KeyboardEvent ev;

					ev = keyboardevent(psy_ui_KeyRelease, wParam, lParam);
					psy_ui_eventdispatch_keyup(&winapp->app->eventdispatch,
						imp->component, &ev);
					if (ev.event.default_prevented) {
						imp->preventwmchar = 1;
					}
					preventdefault = ev.event.default_prevented;
				}
				break;
			case WM_KILLFOCUS:
				if (imp->component) {
					psy_ui_eventdispatch_focuslost(&winapp->app->eventdispatch,
						imp->component);
				}
				break;	
			case WM_MOUSEWHEEL:
			{
				int preventdefault = 0;
				psy_ui_MouseEvent ev;
				POINT pt_client;
				const psy_ui_TextMetric* tm;				

				pt_client.x = (SHORT)LOWORD(lParam);
				pt_client.y = (SHORT)HIWORD(lParam);				
				ScreenToClient(imp->hwnd, &pt_client);				
				tm = psy_ui_component_textmetric(imp->component);				
				psy_ui_mouseevent_init_all(&ev,
					psy_ui_realpoint_make(pt_client.x, pt_client.y),
					(short)LOWORD(wParam),
					(short)HIWORD(wParam),
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);				
				imp->component->vtable->onmousewheel(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousewheel,
					imp->component, 1, &ev);
				preventdefault = ev.event.default_prevented;
			}
			break;
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
	
	imp = (psy_ui_win_ComponentImp*)psy_table_at(&winapp->selfmap,
		(uintptr_t) hwnd);	
	if (imp) {
		switch (message) {		
			case WM_SHOWWINDOW:					
				if (wParam == TRUE) {
					psy_signal_emit(&imp->component->signal_show,
						imp->component, 0);
				} else {
					psy_signal_emit(&imp->component->signal_hide,
						imp->component, 0);
				}
				return 0;					
			case WM_SIZE:
				if (imp->component) {					
					imp->sizecachevalid = FALSE;	
					psy_ui_eventdispatch_size(&winapp->app->eventdispatch,
						imp->component,
						psy_ui_size_make_px(LOWORD(lParam), (HIWORD(lParam))));
				}
				return 0;				
			case WM_TIMER:
				if (imp->component) {
					psy_ui_eventdispatch_timer(&winapp->app->eventdispatch,
						imp->component, (uintptr_t)wParam);
				}
				return 0;
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLORSTATIC:
			case WM_CTLCOLOREDIT: {
				uint32_t colorref;
				uint32_t bgcolorref;
				HBRUSH brush;

				imp = psy_table_at(&winapp->selfmap, (uintptr_t)lParam);
				if (imp && imp->component) {
					psy_ui_Colour colour;

					colour = psy_ui_component_colour(imp->component);
					colorref = psy_ui_colour_colorref(&colour);
					colour = psy_ui_component_backgroundcolour(imp->component);
					bgcolorref = psy_ui_colour_colorref(&colour);
					brush = ((imp->component->backgroundmode & psy_ui_SETBACKGROUND)
							== psy_ui_SETBACKGROUND)
						? psy_ui_win_component_details(imp->component)->background
						: (HBRUSH)GetStockObject(NULL_BRUSH);
				} else {
					colorref = psy_ui_colour_colorref(&psy_ui_style_const(psy_ui_STYLE_ROOT)->colour);
					bgcolorref = psy_ui_colour_colorref(&psy_ui_style_const(psy_ui_STYLE_ROOT)->backgroundcolour);
					brush = winapp->defaultbackgroundbrush;
				}
				SetTextColor((HDC)wParam, colorref);
				SetBkColor((HDC)wParam, bgcolorref);
				return (LRESULT)brush;
				break; }
			case WM_ERASEBKGND:
				return 1;
				break;
			case WM_COMMAND:
				imp = psy_table_at(&winapp->winidmap,
					(uintptr_t) LOWORD(wParam));
				if (imp && imp->component &&
						imp->component->signal_command.slots) {
					psy_signal_emit(&imp->component->signal_command,
						imp->component, 2,  wParam, lParam);
					return 0;
				}
				if (imp && imp->imp.signal_command.slots) {
				  psy_signal_emit(&imp->imp.signal_command, imp->component, 2,
					  wParam, lParam);
				  return 0;
				}
				return 0;
				break;          
			case WM_CREATE:							
				psy_signal_emit(&imp->component->signal_create,
					imp->component, 0);				
				return 0;
				break;
			case WM_PAINT: {
				const psy_ui_Border* border;							

				border = psy_ui_component_border(imp->component);
				if (imp->component->vtable->ondraw ||
						imp->component->signal_draw.slots ||
						imp->component->backgroundmode != psy_ui_NOBACKGROUND ||
					psy_ui_border_isset(border)) {
					HDC hdc;
					POINT clipsize;
					PAINTSTRUCT ps;
					
					hdc = BeginPaint(hwnd, &ps);					
					// store clip/repaint size of paint request
					clipsize.x = ps.rcPaint.right - ps.rcPaint.left;
					clipsize.y = ps.rcPaint.bottom - ps.rcPaint.top;
					// anything to paint?
					if (clipsize.x > 0 && clipsize.y > 0) {
						psy_ui_Graphics	g;
						HDC bufferDC;
						HBITMAP bufferBmp;
						HBITMAP oldBmp;
						psy_ui_win_GraphicsImp* win_g;
						POINT dblbuffer_offset;
						HFONT hfont = 0;
						HFONT hPrevFont = 0;						
						POINT origin;
						const psy_ui_TextMetric* tm;

						tm = psy_ui_component_textmetric(imp->component);						
						if (imp->component->doublebuffered) {
							// create a graphics context with back buffer bitmap
							// with origin (0; 0) and size of the paint request
							bufferDC = CreateCompatibleDC(hdc);
							bufferBmp = CreateCompatibleBitmap(hdc, clipsize.x,
								clipsize.y);
							oldBmp = SelectObject(bufferDC, bufferBmp);
							psy_ui_graphics_init(&g, bufferDC);
							win_g = (psy_ui_win_GraphicsImp*)g.imp;
							// back buffer bitmap starts at 0, 0
							// set offset to paint request origin
							// to translate it to the buffer DC 0, 0 origin
							dblbuffer_offset.x = ps.rcPaint.left;
							dblbuffer_offset.y = ps.rcPaint.top;
						} else {
							// create graphics handle with the paint hdc
							psy_ui_graphics_init(&g, hdc);
							win_g = (psy_ui_win_GraphicsImp*)g.imp;
							// no translation needed
							dblbuffer_offset.x = 0;
							dblbuffer_offset.y = 0;
						}
						// update graphics font with component font 
						hfont = ((psy_ui_win_FontImp*)
							psy_ui_component_font(imp->component)->imp)->hfont;
						hPrevFont = SelectObject(win_g->hdc, hfont);
						// set coordinates origin to fit bufferDC if used
						// (DPtoLP?)
						origin.x = dblbuffer_offset.x;
						origin.y = dblbuffer_offset.y;
						// draw border
						psy_ui_setrectangle(&g.clip,
							ps.rcPaint.left, ps.rcPaint.top,
							clipsize.x, clipsize.y);
						win_g->org.x = origin.x;
						win_g->org.y = origin.y;						
						// draw						
						imp->imp.vtable->dev_draw(&imp->imp, &g);
						// clean up font
						if (hPrevFont) {
							SelectObject(win_g->hdc, hPrevFont);
						}
						if (imp->component->doublebuffered) {
							// copy the double buffer bitmap to the paint hdc
							win_g->hdc = hdc;
							// DPtoLP ?							
							BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
								clipsize.x, clipsize.y, bufferDC, 0, 0,
								SRCCOPY);
							// clean the double buffer bitmap
							SelectObject(bufferDC, oldBmp);
							DeleteObject(bufferBmp);
							DeleteDC(bufferDC);
						}
						psy_ui_graphics_dispose(&g);
					}
					EndPaint(hwnd, &ps);
					return 0;
				}
				break; }
			case WM_NCDESTROY: {
				bool deallocate;
				psy_ui_Component* component;

				component = imp->component;
				deallocate = imp->component->deallocate;
				if (imp->component) {
					psy_signal_emit(&imp->component->signal_destroyed,
						imp->component, 0);
					imp->component->vtable->ondestroyed(imp->component);
				}
				psy_ui_component_dispose(imp->component);
				psy_table_remove(&winapp->selfmap, (uintptr_t)hwnd);
				if (deallocate) {					
					free(component);
				}
				return 0;
				break; }
			case WM_DESTROY:				
				if (imp->component) {
					psy_signal_emit(&imp->component->signal_destroy,
						imp->component, 0);					
					imp->component->vtable->ondestroy(imp->component);
				}
				return 0;
				break;
			case WM_CLOSE: {
				bool close;

				close = imp->component->vtable->onclose(imp->component);				
				psy_signal_emit(&imp->component->signal_close,
					imp->component, 1, (void*)&close);				
				if (!close) {
					return 0;
				}				
				break; }
			case WM_SYSKEYDOWN:
				if (!(imp->component &&
						(wParam >= VK_F10 && wParam <= VK_F12 ||
						wParam >= 0x41 && wParam <= psy_ui_KEY_Z ||
						wParam >= psy_ui_KEY_DIGIT0 && wParam <= psy_ui_KEY_DIGIT9))) {					
					return 0;
				}
				/* fallthrough */
			case WM_KEYDOWN:							
				if (imp->component) {
					psy_ui_KeyboardEvent ev;

					ev = keyboardevent(psy_ui_KeyPress, wParam, lParam);										
					psy_ui_eventdispatch_keydown(&winapp->app->eventdispatch,
						imp->component, &ev);					
					if (ev.event.default_prevented) {
						imp->preventwmchar = 1;
					}					
					return 0;
				}
				break;
			case WM_KEYUP:
				if (imp->component) {
					psy_ui_KeyboardEvent ev;

					ev = keyboardevent(psy_ui_KeyRelease, wParam, lParam);
					psy_ui_eventdispatch_keyup(&winapp->app->eventdispatch,
						imp->component, &ev);
					if (ev.event.default_prevented) {
						imp->preventwmchar = 1;
					}
					return 0;
				}
				break;
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP: {
				psy_ui_MouseEvent ev;				
				int button;
			
				button = 1;
				if (message == WM_LBUTTONUP) {
					button = 1;
				} else if (message == WM_RBUTTONUP) {					
					button = 2;
				} else {
					button = 3;
				}			
				psy_ui_mouseevent_init_all(&ev,
					psy_ui_realpoint_make((SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam)),
					button, 0, GetKeyState(VK_SHIFT) < 0,
					GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.pt);
				psy_ui_eventdispatch_buttonup(&winapp->app->eventdispatch,
					imp->component, &ev);
				return 0;
				break; }			
			case WM_LBUTTONDOWN: 
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN: {
				psy_ui_MouseEvent ev;				
				int button;

				button = 1;
				if (message == WM_LBUTTONDOWN) {
					button = 1;
				} else if (message == WM_RBUTTONDOWN) {
					button = 2;
				} else {
					button = 3;
				}
				psy_ui_mouseevent_init_all(&ev,
					psy_ui_realpoint_make((SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam)),
					button, 0, GetKeyState(VK_SHIFT) < 0,
					GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.pt);
				psy_ui_eventdispatch_buttondown(&winapp->app->eventdispatch,
					imp->component, &ev);
				return 0;
				break; }			
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDBLCLK: {
				psy_ui_MouseEvent ev;
				int button;

				button = 1;
				if (message == WM_LBUTTONDBLCLK) {
					button = 1;
				} else if (message == WM_RBUTTONDBLCLK) {
					button = 2;
				} else {
					button = 3;
				}
				psy_ui_mouseevent_init_all(&ev,
					psy_ui_realpoint_make((SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam)),
					button, 0, GetKeyState(VK_SHIFT) < 0,
					GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.pt);
				psy_ui_eventdispatch_doubleclick(&winapp->app->eventdispatch,
					imp->component, &ev);
				return 0;
				break; }
			case WM_MOUSEMOVE: {
				if (psy_ui_app()->dragevent.active) {
					psy_ui_MouseEvent ev;

					psy_ui_mouseevent_init_all(&ev,
						psy_ui_realpoint_make((SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam)),
						wParam, 0, GetKeyState(VK_SHIFT) < 0,
						GetKeyState(VK_CONTROL) < 0);
					adjustcoordinates(imp->component, &ev.pt);
					psy_ui_eventdispatch_mousemove(&winapp->app->eventdispatch,
						imp->component, &ev);					
				} else {
					psy_ui_MouseEvent ev;

					psy_ui_mouseevent_init_all(&ev,
						psy_ui_realpoint_make((SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam)),
						wParam, 0, GetKeyState(VK_SHIFT) < 0,
						GetKeyState(VK_CONTROL) < 0);
					adjustcoordinates(imp->component, &ev.pt);
					// psy_ui_mouseevent_settarget(&ev, eventtarget(imp->component));
					imp->imp.vtable->dev_mousemove(&imp->imp, &ev);			
				}			
				return 0;
				break; }
			case WM_SETTINGCHANGE: {
				static int ulScrollLines;

				SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0,
					&ulScrollLines, 0) ;
      
			   // ulScrollLines usually equals 3 or 0 (for no scrolling)
			   // WHEEL_DELTA equals 120, so deltaperline will be 40
				if (ulScrollLines) {
					psy_ui_app()->deltaperline = WHEEL_DELTA / ulScrollLines;
				} else {
					psy_ui_app()->deltaperline = 0;
				}
				return 0;
				break; }
			case WM_MOUSEWHEEL:	{				
				psy_ui_MouseEvent ev;
				POINT pt_client;

				pt_client.x = (SHORT)LOWORD(lParam);
				pt_client.y = (SHORT)HIWORD(lParam);
				ScreenToClient(imp->hwnd, &pt_client);
				psy_ui_mouseevent_init_all(&ev,
					psy_ui_realpoint_make(pt_client.x, pt_client.y),
					(short)LOWORD(wParam),
					(short)HIWORD(wParam),
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.pt);
				psy_ui_component_mousewheel(imp->component, &ev,
					(short)HIWORD(wParam) /* 120 or -120 */);
				break; }
			case WM_MOUSEHOVER:							
				psy_signal_emit(&imp->component->signal_mousehover, imp->component, 0);
				return 0;				
				break;
			case WM_MOUSELEAVE:							
				imp->imp.vtable->dev_mouseleave(&imp->imp);
				return 0;			
				break;
			case WM_VSCROLL:				
				handle_vscroll(hwnd, wParam, lParam);
				return 0;
			break;
			case WM_HSCROLL:				
				handle_hscroll(hwnd, wParam, lParam);
				return 0;
			break;
			case WM_KILLFOCUS:
				if (imp->component) {
					psy_ui_eventdispatch_focuslost(&winapp->app->eventdispatch,
						imp->component);
				}				
				return 0;				
			break;
			case WM_NCACTIVATE:
				if (imp->component->dropdown) {
					if (wParam == 0) {
						EnableWindow(GetParent(imp->hwnd), TRUE);
						ShowWindow(imp->hwnd, FALSE);
					}
				}
				break;
			default:			
			break;
		}	
	}
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}

psy_ui_KeyboardEvent keyboardevent(psy_ui_EventType type, WPARAM wParam, LPARAM lParam)
{
	psy_ui_KeyboardEvent rv;

	psy_ui_keyboardevent_init_all(&rv, (int)wParam, lParam,
		GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_MENU) < 0,
		(lParam & 0x40000000) == 0x40000000);
	rv.event.type = type;
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

void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO		si;	
    int				pos; //, iHorzPos;	
	psy_ui_win_ComponentImp* imp;
     
	si.cbSize = sizeof(si);
    si.fMask  = SIF_ALL;
    GetScrollInfo (hwnd, SB_VERT, &si);	
	// Save the position for comparison later on
	pos = si.nPos;
	handle_scrollparam(hwnd, &si, wParam);	
	// Set the position and then retrieve it.  Due to adjustments
	// by Windows it may not be the same as the value set.
	si.fMask = SIF_POS;
	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
	GetScrollInfo(hwnd, SB_VERT, &si);
	// If the position has changed, scroll the window and update it
	if (si.nPos != pos)
	{
		const psy_ui_TextMetric* tm;
		psy_ui_Value scrolltop;
		imp = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);					

		tm = psy_ui_component_textmetric(imp->component);
		scrolltop = psy_ui_component_scrolltop(imp->component);
		psy_ui_component_setscrolltop(imp->component,
			psy_ui_value_make_px(
				psy_ui_value_px(&scrolltop, tm, NULL) -
				psy_ui_component_scrollstep_height_px(imp->component) *
					(pos - si.nPos)));
	}
}
void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO		si;	
    int				pos; 
	psy_ui_win_ComponentImp* imp;
     
	si.cbSize = sizeof (si) ;
    si.fMask  = SIF_ALL ;
    GetScrollInfo (hwnd, SB_HORZ, &si) ;	
	// Save the position for comparison later on
	pos = si.nPos ;
	handle_scrollparam(hwnd, &si, wParam);
	// Set the position and then retrieve it.  Due to adjustments
	// by Windows it may not be the same as the value set.
	si.fMask = SIF_POS ;
	SetScrollInfo (hwnd, SB_HORZ, &si, TRUE) ;
	GetScrollInfo (hwnd, SB_HORZ, &si) ;
	// If the position has changed, scroll the window and update it
	if (si.nPos != pos) {       
		const psy_ui_TextMetric* tm;
		psy_ui_Value scrollleft;

		imp = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);							
		tm = psy_ui_component_textmetric(imp->component);
		scrollleft = psy_ui_component_scrollleft(imp->component);
		psy_ui_component_setscrollleft(imp->component,
			psy_ui_value_make_px(
				psy_ui_value_px(&scrollleft, tm, NULL) -
				psy_ui_component_scrollstep_width_px(imp->component) *
					(pos - si.nPos)));
	}
}

void handle_scrollparam(HWND hwnd, SCROLLINFO* si, WPARAM wParam)
{
	switch (LOWORD (wParam)) {
		case SB_TOP:
		   si->nPos = si->nMin ;
		break ;
		case SB_BOTTOM:
		   si->nPos = si->nMax ;
		break ;
		case SB_LINEUP:
		   si->nPos -= 1 ;
		break ;
		case SB_LINEDOWN:
		   si->nPos += 1 ;
		break ;
		case SB_PAGEUP:
		   si->nPos -= si->nPage ;
		break ;
		case SB_PAGEDOWN:
		   si->nPos += si->nPage ;
		break ;
		case SB_THUMBTRACK:
		   si->nPos = (short)HIWORD(wParam);
		break ;
		default:
		break ;   		
	}	
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0 && winapp && winapp->app) {
		if (wParam == WM_LBUTTONDOWN) {
			psy_ui_MouseEvent ev;
			MOUSEHOOKSTRUCT* pMouseStruct = (MOUSEHOOKSTRUCT*)lParam;

			psy_ui_mouseevent_init_all(&ev,
				psy_ui_realpoint_make(pMouseStruct->pt.x, pMouseStruct->pt.y),
				0, 0, GetKeyState(VK_SHIFT) < 0,
				GetKeyState(VK_CONTROL) < 0);
			psy_signal_emit(&winapp->app->signal_mousehook, winapp->app,
				1, &ev);			
		} else if (wParam == WM_RBUTTONDOWN) {
			psy_ui_MouseEvent ev;

			psy_ui_mouseevent_init_all(&ev,
				psy_ui_realpoint_make((SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam)),
				1, 0, GetKeyState(VK_SHIFT) < 0,
				GetKeyState(VK_CONTROL) < 0);
			psy_signal_emit(&winapp->app->signal_mousehook, winapp->app,
				1, &ev);
		}	
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

int psy_ui_winapp_run(psy_ui_WinApp* self) 
{
	MSG msg;
	
	// __try
	// {
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	// }
	// __except(FilterException("app loop", GetExceptionCode(),
	//		GetExceptionInformation())) {
	// }
    return (int) msg.wParam ;
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

	imp = (psy_ui_win_ComponentImp*)component->imp;
	switch (ev->type) {
	case psy_ui_KeyPress: {
		psy_ui_KeyboardEvent* keyevent;

		keyevent = (psy_ui_KeyboardEvent*)ev;
		SendMessage(imp->hwnd, (UINT)WM_KEYDOWN, (WPARAM)keyevent->keycode,
			(LPARAM)keyevent->keydata);	
		break; }
	case psy_ui_KeyRelease: {
		psy_ui_KeyboardEvent* keyevent;

		keyevent = (psy_ui_KeyboardEvent*)ev;
		SendMessage(imp->hwnd, (UINT)WM_KEYUP, (WPARAM)keyevent->keycode,
			(LPARAM)keyevent->keydata);
		break; }
	case psy_ui_ButtonPress: {
		psy_ui_MouseEvent* mouseevent;
		int16_t loword;
		int16_t hiword;
		LPARAM lparam;
		
		mouseevent = (psy_ui_MouseEvent*)ev;
		loword = (int16_t)mouseevent->pt.x;
		hiword = (int16_t)mouseevent->pt.y;
		lparam = loword | (hiword << 16);
		if (mouseevent->button == 1) {
			SendMessage(imp->hwnd, (UINT)WM_LBUTTONDOWN, (WPARAM)mouseevent->button, (LPARAM)lparam);
		} else if (mouseevent->button == 2) {
			SendMessage(imp->hwnd, (UINT)WM_RBUTTONDOWN, (WPARAM)mouseevent->button, (LPARAM)lparam);
		} else if (mouseevent->button == 3) {
			SendMessage(imp->hwnd, (UINT)WM_MBUTTONDOWN, (WPARAM)mouseevent->button, (LPARAM)lparam);
		}
		break; }
	case psy_ui_ButtonRelease: {
		psy_ui_MouseEvent* mouseevent;
		int16_t loword;
		int16_t hiword;
		LPARAM lparam;

		mouseevent = (psy_ui_MouseEvent*)ev;
		loword = (int16_t)mouseevent->pt.x;
		hiword = (int16_t)mouseevent->pt.y;
		lparam = loword | (hiword << 16);
		if (mouseevent->button == 1) {
			SendMessage(imp->hwnd, (UINT)WM_LBUTTONUP, (WPARAM)mouseevent->button, (LPARAM)lparam);
		} else if (mouseevent->button == 2) {
			SendMessage(imp->hwnd, (UINT)WM_RBUTTONUP, (WPARAM)mouseevent->button, (LPARAM)lparam);
		} else if (mouseevent->button == 3) {
			SendMessage(imp->hwnd, (UINT)WM_MBUTTONUP, (WPARAM)mouseevent->button, (LPARAM)lparam);
		}
		break; }
	case psy_ui_DoubleClick: {
		psy_ui_MouseEvent* mouseevent;
		int16_t loword;
		int16_t hiword;
		LPARAM lparam;

		mouseevent = (psy_ui_MouseEvent*)ev;
		loword = (int16_t)mouseevent->pt.x;
		hiword = (int16_t)mouseevent->pt.y;
		lparam = loword | (hiword << 16);
		if (mouseevent->button == 1) {
			SendMessage(imp->hwnd, (UINT)WM_LBUTTONDBLCLK, (WPARAM)mouseevent->button, (LPARAM)lparam);
		} else if (mouseevent->button == 2) {
			SendMessage(imp->hwnd, (UINT)WM_RBUTTONDBLCLK, (WPARAM)mouseevent->button, (LPARAM)lparam);
		} else if (mouseevent->button == 3) {
			SendMessage(imp->hwnd, (UINT)WM_MBUTTONDBLCLK, (WPARAM)mouseevent->button, (LPARAM)lparam);
		}
		break; }
	case psy_ui_MotionNotify: {
		psy_ui_MouseEvent* mouseevent;
		int16_t loword;
		int16_t hiword;
		LPARAM lparam;

		mouseevent = (psy_ui_MouseEvent*)ev;
		loword = (int16_t)mouseevent->pt.x;
		hiword = (int16_t)mouseevent->pt.y;
		lparam = loword | (hiword << 16);		
		SendMessage(imp->hwnd, (UINT)WM_MOUSEMOVE, (WPARAM)mouseevent->button, (LPARAM)lparam);		
		break; }
	default:
		break;
	}
}

psy_ui_Component* psy_ui_winapp_component(psy_ui_WinApp* self,
	uintptr_t handle)
{	
	psy_ui_win_ComponentImp* imp;
	
	imp = (psy_ui_win_ComponentImp*)psy_table_at(&winapp->selfmap, handle);
	if (imp) {
		return imp->component;
	}
	return NULL;
}

#endif /* PSYCLE_TK_WIN32 */

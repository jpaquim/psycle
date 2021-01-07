// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/trace.h"

#include "uiwinapp.h"
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32


#include "uiwingraphicsimp.h"
#include "uiwinfontimp.h"
#include "uicomponent.h"
#include "uiapp.h"
#include "uiwincomponentimp.h"
#include <excpt.h>
#include <stdlib.h>
#include <commctrl.h> // common control header

int iDeltaPerLine = 120;
extern psy_Table menumap;
extern psy_ui_App app;

static psy_ui_Component* eventtarget(psy_ui_Component* component);
static void sendmessagetoparent(psy_ui_win_ComponentImp* imp, uintptr_t message,
	WPARAM wparam, LPARAM lparam);
static void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_scrollparam(HWND hwnd, SCROLLINFO* si, WPARAM wParam);
static void adjustcoordinates(psy_ui_Component*, intptr_t* x, intptr_t* y);

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

static int FilterException(const char* msg, int code,
	struct _EXCEPTION_POINTERS *ep) 
{	
	// char txt[512];				
	MessageBox(0, msg, "Psycle Ui Exception", MB_OK | MB_ICONERROR);
	return EXCEPTION_EXECUTE_HANDLER;
}

static void onlanguagechanged(psy_ui_WinApp*, psy_Translator* sender);

void psy_ui_winapp_init(psy_ui_WinApp* self, HINSTANCE instance)
{
	static TCHAR szAppClass[] = TEXT("PsycleApp");	
	static TCHAR szComponentClass[] = TEXT("PsycleComponent");

	self->instance = instance;
	self->appclass = szAppClass;
	self->componentclass = szComponentClass;
	self->winproc = ui_winproc;
	self->comwinproc = ui_com_winproc;
	self->winid = 20000;
	self->eventretarget = 0;
	psy_ui_winapp_registerclasses(self);
	CoInitialize(NULL);
	psy_table_init(&self->selfmap);
	psy_table_init(&self->winidmap);
	self->defaultbackgroundbrush = CreateSolidBrush(
		app.defaults.style_common.backgroundcolour.value);
	self->targetids = NULL;
}

void psy_ui_winapp_dispose(psy_ui_WinApp* self)
{
	psy_table_dispose(&self->selfmap);
	psy_table_dispose(&self->winidmap);
	psy_list_free(self->targetids);
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
	psy_ui_WinApp* winapp;
	psy_ui_fp_winproc winproc;
	bool preventdefault;

	preventdefault = 0;
	winapp = (psy_ui_WinApp*) app.platform;
	imp = (psy_ui_win_ComponentImp*)psy_table_at(&winapp->selfmap, (uintptr_t)
		hwnd);
	if (imp) {		
		winproc = imp->wndproc;
		switch (message)
		{
			case WM_NCDESTROY:
				// restore default winproc				
				if (imp->component) {
					if (imp->component->signal_destroyed.slots) {
						psy_signal_emit(&imp->component->signal_destroyed,
							imp->component, 0);
					}
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
					if (imp->component->signal_destroy.slots) {
						psy_signal_emit(&imp->component->signal_destroy,
							imp->component, 0);
					}
					imp->component->vtable->ondestroy(imp->component);
				}								
				break;			
			case WM_TIMER:								
				imp->component->vtable->ontimer(imp->component,
					(uintptr_t)wParam);
				if (imp->component->signal_timer.slots) {
					psy_signal_emit(&imp->component->signal_timer,
						imp->component, 1, (uintptr_t)wParam);
				}
				break;
			case WM_CHAR:
				if (imp->preventwmchar) {
					imp->preventwmchar = 0;
					preventdefault = 1;
				}
				break;
			case WM_KEYDOWN: {
				psy_ui_KeyEvent ev;

				if (imp->component) {
					psy_ui_keyevent_init(&ev, (int)wParam, lParam,
						GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_MENU) < 0,
						(lParam & 0x40000000) == 0x40000000);
					imp->component->vtable->onkeydown(imp->component, &ev);
					psy_signal_emit(&imp->component->signal_keydown, imp->component,
						1, &ev);
					preventdefault = ev.preventdefault;
					if (preventdefault) {
						imp->preventwmchar = 1;
					}
					if (ev.bubble != FALSE) {
						sendmessagetoparent(imp, message, wParam, lParam);
					}
				}
				break; }
			case WM_KEYUP: {
				psy_ui_KeyEvent ev;

				if (imp->component) {
					psy_ui_keyevent_init(&ev, (int)wParam, lParam,
						GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_MENU) < 0,
						(lParam & 0x40000000) == 0x40000000);
					imp->component->vtable->onkeyup(imp->component, &ev);
					psy_signal_emit(&imp->component->signal_keyup, imp->component,
						1, &ev);
					preventdefault = ev.preventdefault;
					if (preventdefault) {
						imp->preventwmchar = 1;
					}
					if (ev.bubble != FALSE) {
						sendmessagetoparent(imp, message, wParam, lParam);
					}
				}
				break; }			
			case WM_KILLFOCUS:
				if (imp->component) {
					imp->component->vtable->onfocuslost(imp->component);
					psy_signal_emit(&imp->component->signal_focuslost,
						imp->component, 0);
				}				
			break;	
			case WM_MOUSEWHEEL:
			{
				int preventdefault = 0;
				psy_ui_MouseEvent ev;
				POINT pt_client;
				psy_ui_TextMetric tm;

				pt_client.x = (SHORT)LOWORD(lParam);
				pt_client.y = (SHORT)HIWORD(lParam);
				ScreenToClient(imp->hwnd, &pt_client);				
				tm = psy_ui_component_textmetric(imp->component);
				psy_ui_mouseevent_init(&ev,
					pt_client.x + psy_ui_value_px(&imp->component->scroll.x, &tm),
					pt_client.y + psy_ui_value_px(&imp->component->scroll.y, &tm),
					(short)LOWORD(wParam),
					(short)HIWORD(wParam),
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);				
				imp->component->vtable->onmousewheel(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousewheel,
					imp->component, 1, &ev);
				preventdefault = ev.preventdefault;				
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
    PAINTSTRUCT  ps ;     
	psy_ui_win_ComponentImp* imp;
	psy_ui_Graphics	 g;
	// HMENU		 hMenu;
	// psy_ui_Menu* menu;
	// int			 menu_id;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*) app.platform;
	imp = (psy_ui_win_ComponentImp*) psy_table_at(&winapp->selfmap,
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
				return 0 ;				
				break;		
			case WM_SIZE: {
				psy_ui_Size size;
				
				if (imp->component->alignchildren) {
					psy_ui_component_align(imp->component);
				}
				size.width = psy_ui_value_makepx(LOWORD(lParam));
				size.height = psy_ui_value_makepx(HIWORD(lParam));					
				imp->component->vtable->onsize(imp->component, &size);
				if (imp->component->overflow != psy_ui_OVERFLOW_HIDDEN) {
					psy_ui_component_updateoverflow(imp->component);						
				}
				psy_signal_emit(&imp->component->signal_size, imp->component, 1,
					(void*)&size);

				return 0;
			break; }
			case WM_TIMER:				
				imp->component->vtable->ontimer(imp->component, (int) wParam);
				if (imp->component->signal_timer.slots) {
					psy_signal_emit(&imp->component->signal_timer,
						imp->component, 1, (int)wParam);
				}
				return 0;
			break;		
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLORSTATIC:
			case WM_CTLCOLOREDIT:
				imp = psy_table_at(&winapp->selfmap, (uintptr_t) lParam);
				if (imp && imp->component) {					
					SetTextColor((HDC) wParam, psy_ui_component_colour(
						imp->component).value);
					SetBkColor((HDC) wParam, psy_ui_component_backgroundcolour(
						imp->component).value);
					if ((imp->component->backgroundmode & psy_ui_BACKGROUND_SET)
							== psy_ui_BACKGROUND_SET) {
						return (intptr_t) psy_ui_win_component_details(
							imp->component)->background;
					} else {
						return (intptr_t) GetStockObject(NULL_BRUSH);
					}
				} else {				
					SetTextColor((HDC) wParam,
						app.defaults.style_common.colour.value);
					SetBkColor((HDC)wParam,
						app.defaults.style_common.backgroundcolour.value);
					return (intptr_t) winapp->defaultbackgroundbrush;
				}
				break;
			case WM_ERASEBKGND:
				return 1;
				break;
			case WM_COMMAND:
			  /*hMenu = GetMenu (hwnd) ;
			  menu_id = LOWORD (wParam);
			  menu = psy_table_at(&menumap, (uintptr_t) menu_id);
			  if (menu && menu->execute) {	
				menu->execute(menu);
			  }*/
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
				if (imp->component->signal_create.slots) {	
					psy_signal_emit(&imp->component->signal_create,
						imp->component, 0);
				}
				return 0;
				break;
			case WM_PAINT :			
				if (imp->component->vtable->ondraw || 
						imp->component->signal_draw.slots ||
						imp->component->backgroundmode !=
							psy_ui_BACKGROUND_NONE) {
					HDC hdc;					
					POINT clipsize;														

					hdc = BeginPaint(hwnd, &ps);
					// store clip/repaint size of paint request
					clipsize.x = ps.rcPaint.right - ps.rcPaint.left;
					clipsize.y = ps.rcPaint.bottom - ps.rcPaint.top;
					// anything to paint?
					if (clipsize.x > 0 && clipsize.y > 0) {
						HDC bufferDC;
						HBITMAP bufferBmp;
						HBITMAP oldBmp;
						psy_ui_win_GraphicsImp* win_g;
						POINT dblbuffer_offset;
						HFONT hfont = 0;
						HFONT hPrevFont = 0;																		
						POINT org;
						psy_ui_TextMetric tm;						

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
						if (imp->component->backgroundmode ==
								psy_ui_BACKGROUND_SET) {
							//int level;
							// psy_ui_Colour c;
							// float gain;
							// draw background
							psy_ui_Rectangle r;
							psy_ui_setrectangle(&r,
								ps.rcPaint.left - dblbuffer_offset.x,
								ps.rcPaint.top - dblbuffer_offset.y,
								clipsize.x, clipsize.y);
							// level = psy_ui_component_level(imp->component);
							// c = psy_ui_component_backgroundcolour(
							//	imp->component);
							// gain = 1.f + level * 0.03f;
							// psy_ui_colour_mul_rgb(&c, gain, gain, gain);
							psy_ui_drawsolidrectangle(&g, r, // c);
								psy_ui_component_backgroundcolour(
									imp->component));
						}						
						// prepare a clip rect that can be used by a component
						// to optimize the draw amount
						psy_ui_setrectangle(&g.clip,
							ps.rcPaint.left + psy_ui_value_px(&imp->component->scroll.x, &tm),
							ps.rcPaint.top + psy_ui_value_px(&imp->component->scroll.y, &tm),
							clipsize.x, clipsize.y);												
						// translate coordinates 
						// 1. to fit bufferDC bitmap if used
						// 2. to handle scroll coords
						// DPtoLP ?												
						if (!psy_ui_margin_iszero(&imp->component->spacing)) {
							tm = psy_ui_component_textmetric(imp->component);
							
							/*
							// exclude padding from the clipping region
							if (!psy_ui_value_iszero(&imp->component->spacing.top)) {
								ExcludeClipRect(win_g->hdc,
									0, 0, clipsize.x,
									psy_ui_value_px(&imp->component->spacing.top, &tm));
							}
							if (!psy_ui_value_iszero(&imp->component->spacing.bottom)) {
								ExcludeClipRect(win_g->hdc,
									0, clipsize.y - psy_ui_value_px(&imp->component->spacing.bottom, &tm),
									clipsize.x, clipsize.y);
							}
							if (!psy_ui_value_iszero(&imp->component->spacing.left)) {
								ExcludeClipRect(win_g->hdc,
									0, 0,
									psy_ui_value_px(&imp->component->spacing.left, &tm), clipsize.y);
							}
							if (!psy_ui_value_iszero(&imp->component->spacing.right)) {
								ExcludeClipRect(win_g->hdc,
									psy_ui_value_px(&imp->component->spacing.right, &tm), 0,
									clipsize.x, clipsize.y);
							}*/
							SetWindowOrgEx(win_g->hdc,
								(int)dblbuffer_offset.x + (int)
									psy_ui_value_px(&imp->component->scroll.x, &tm) -
								(int)psy_ui_value_px(&imp->component->spacing.left,
									&tm),
								(int)dblbuffer_offset.y + (int)
									psy_ui_value_px(&imp->component->scroll.y, &tm) -
									(int)psy_ui_value_px(&imp->component->spacing.top,
									&tm),
								NULL);							
						} else {
							SetWindowOrgEx(win_g->hdc,
								(int)dblbuffer_offset.x +
									(int)psy_ui_value_px(&imp->component->scroll.x, &tm),
								(int)dblbuffer_offset.y +
									(int)psy_ui_value_px(&imp->component->scroll.y, &tm),
								NULL);							
						}
						// update graphics font with component font 
						hfont = ((psy_ui_win_FontImp*)
							psy_ui_component_font(imp->component)->imp)->hfont;
						hPrevFont = SelectObject(win_g->hdc, hfont);						
						// prepare colours
						psy_ui_setcolour(&g, psy_ui_component_colour(
							imp->component));
						psy_ui_settextcolour(&g, psy_ui_component_colour(
							imp->component));
						psy_ui_setbackgroundmode(&g, psy_ui_TRANSPARENT);						
						// draw border						
						psy_ui_component_drawborder(imp->component, &g);
						// update graphics origin
						GetWindowOrgEx(win_g->hdc, &org);
						win_g->orgx = org.x;
						win_g->orgy = org.y;
						// call specialization methods (vtable, then signals)						
						if (imp->component->vtable->ondraw) {
							imp->component->vtable->ondraw(imp->component, &g);
						}												
						psy_signal_emit(&imp->component->signal_draw,
							imp->component, 1, &g);						
						// clean up font
						if (hPrevFont) {
							SelectObject(win_g->hdc, hPrevFont);
						}
						if (imp->component->doublebuffered) {
							// copy the double buffer bitmap to the paint hdc
							win_g->hdc = hdc;
							// DPtoLP ?
							SetWindowOrgEx(bufferDC, 0, 0, NULL);
							SetWindowOrgEx(hdc, 0, 0, NULL);
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
					return 0 ;
				}
				break;
			case WM_NCDESTROY:
				if (imp->component) {
					if (imp->component->signal_destroyed.slots) {
						psy_signal_emit(&imp->component->signal_destroyed,
							imp->component, 0);
					}
					imp->component->vtable->ondestroyed(imp->component);
				}
				psy_ui_component_dispose(imp->component);
				psy_table_remove(&winapp->selfmap, (uintptr_t)hwnd);
				return 0;
				break;
			case WM_DESTROY:
				if (imp->component) {
					if (imp->component->signal_destroy.slots) {
						psy_signal_emit(&imp->component->signal_destroy,
							imp->component, 0);
					}
					imp->component->vtable->ondestroy(imp->component);
				}
				return 0;
				break;
			case WM_CLOSE: {
				bool close;

				close = imp->component->vtable->onclose(imp->component);
				if (imp->component->signal_close.slots) {
					psy_signal_emit(&imp->component->signal_close,
						imp->component, 1, (void*)&close);
				}
				if (!close) {
					return 0;
				}				
				break; }
			case WM_SYSKEYDOWN:
				if (wParam >= VK_F10 && wParam <= VK_F12 ||
					wParam >= 0x41 && wParam <= psy_ui_KEY_Z) {
					psy_ui_KeyEvent ev;
						
					psy_ui_keyevent_init(&ev, (int)wParam, lParam, 
						GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_MENU) < 0,
						(lParam & 0x40000000) == 0x40000000);
					psy_ui_keyevent_settarget(&ev, eventtarget(imp->component));
					imp->component->vtable->onkeydown(imp->component, &ev);
					psy_signal_emit(&imp->component->signal_keydown,
						imp->component, 1, &ev);					
					if (ev.bubble != FALSE) {
						sendmessagetoparent(imp, message, wParam, lParam);
					} else {
						psy_list_free(winapp->targetids);
						winapp->targetids = NULL;
					}					
					return 0;
				}
				break;
			case WM_KEYDOWN: {
				psy_ui_KeyEvent ev;
				
				psy_ui_keyevent_init(&ev, (int)wParam, lParam, 
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_MENU) < 0,
					(lParam & 0x40000000) == 0x40000000);
				psy_ui_keyevent_settarget(&ev, eventtarget(imp->component));
				imp->component->vtable->onkeydown(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_keydown, imp->component,
					1, &ev);
				if (ev.bubble != FALSE) {										
					sendmessagetoparent(imp, message, wParam, lParam);
				} else {
					psy_list_free(winapp->targetids);
					winapp->targetids = NULL;
				}
				return 0;
				break; }
			case WM_KEYUP: {
				psy_ui_KeyEvent ev;
								
				psy_ui_keyevent_init(&ev, (int)wParam, lParam, 
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_MENU) < 0,
					(lParam & 0x40000000) == 0x40000000);
				psy_ui_keyevent_settarget(&ev, eventtarget(imp->component));
				imp->component->vtable->onkeyup(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_keyup, imp->component,
					1, &ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
				break; }
			case WM_LBUTTONUP: {
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev,
					(SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam),
					MK_LBUTTON, 0, GetKeyState(VK_SHIFT) < 0,
					GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.x, &ev.y);
				psy_ui_mouseevent_settarget(&ev, eventtarget(imp->component));
				imp->component->vtable->onmouseup(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mouseup, imp->component,
					1, &ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}				
				return 0;
				break; }
			case WM_RBUTTONUP: {
				psy_ui_MouseEvent ev;
			
				psy_ui_mouseevent_init(&ev,
					(SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam),
					MK_RBUTTON, 0, GetKeyState(VK_SHIFT) < 0,
					GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.x, &ev.y);
				psy_ui_mouseevent_settarget(&ev, eventtarget(imp->component));
				imp->component->vtable->onmouseup(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mouseup, imp->component,
					1, &ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
				break; }
			case WM_MBUTTONUP: {			
				psy_ui_MouseEvent ev;
		
				psy_ui_mouseevent_init(&ev,
					(SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam),
					MK_MBUTTON, 0, GetKeyState(VK_SHIFT) < 0,
					GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.x, &ev.y);
				psy_ui_mouseevent_settarget(&ev, eventtarget(imp->component));
				imp->component->vtable->onmouseup(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mouseup, imp->component,
					1, &ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
				break; }
			case WM_LBUTTONDOWN: {
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev,
					(SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam),
					MK_LBUTTON, 0, GetKeyState(VK_SHIFT) < 0,
					GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.x, &ev.y);
				psy_ui_mouseevent_settarget(&ev, eventtarget(imp->component));
				imp->component->vtable->onmousedown(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousedown,
					imp->component, 1, &ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
				break; }
			case WM_RBUTTONDOWN: {
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev,
					(SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam),
					MK_RBUTTON, 0, GetKeyState(VK_SHIFT) < 0,
					GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.x, &ev.y);
				psy_ui_mouseevent_settarget(&ev, eventtarget(imp->component));
				imp->component->vtable->onmousedown(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousedown,
					imp->component, 1, &ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
				break; }
			case WM_MBUTTONDOWN: {		
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev,
					(SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam),
					MK_MBUTTON, 0, GetKeyState(VK_SHIFT) < 0,
					GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.x, &ev.y);
				psy_ui_mouseevent_settarget(&ev, eventtarget(imp->component));
				imp->component->vtable->onmousedown(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousedown,
					imp->component, 1, &ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
				break; }
			case WM_LBUTTONDBLCLK: {
				psy_ui_MouseEvent ev;
				
				psy_ui_mouseevent_init(&ev,
					(SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam),
					MK_LBUTTON, 0, GetKeyState(VK_SHIFT) < 0,
					GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.x, &ev.y);
				psy_ui_mouseevent_settarget(&ev, eventtarget(imp->component));
				imp->component->vtable->onmousedoubleclick(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousedoubleclick,
					imp->component, 1, &ev);				
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;			
				break; }
			case WM_MBUTTONDBLCLK: {
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev,
					(SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam),
					MK_MBUTTON, 0, GetKeyState(VK_SHIFT) < 0,
					GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.x, &ev.y);
				psy_ui_mouseevent_settarget(&ev, eventtarget(imp->component));
				imp->component->vtable->onmousedoubleclick(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousedoubleclick,
					imp->component, 1, &ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
				break; }
			case WM_RBUTTONDBLCLK: {				
				psy_ui_MouseEvent ev;

				psy_ui_mouseevent_init(&ev,
					(SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam),
					MK_RBUTTON, 0, GetKeyState(VK_SHIFT) < 0,
					GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.x, &ev.y);
				psy_ui_mouseevent_settarget(&ev, eventtarget(imp->component));
				imp->component->vtable->onmousedoubleclick(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousedoubleclick,
					imp->component, 1, &ev);
				if (ev.bubble != FALSE) {
					sendmessagetoparent(imp, message, wParam, lParam);
				}
				return 0;
				break; }
			case WM_MOUSEMOVE:
				if (!imp->component->mousetracking) {
					TRACKMOUSEEVENT tme;
					
					imp->component->vtable->onmouseenter(imp->component);
					psy_signal_emit(&imp->component->signal_mouseenter,
						imp->component, 0);					
					tme.cbSize = sizeof(TRACKMOUSEEVENT);
					tme.dwFlags = TME_LEAVE | TME_HOVER;
					tme.dwHoverTime = 200;
					tme.hwndTrack = hwnd;
					if (_TrackMouseEvent(&tme)) {
						imp->component->mousetracking = 1;
					} 
					return 0;
				}								
				{
					psy_ui_MouseEvent ev;

					psy_ui_mouseevent_init(&ev,
						(SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam),
						wParam, 0, GetKeyState(VK_SHIFT) < 0,
						GetKeyState(VK_CONTROL) < 0);
					adjustcoordinates(imp->component, &ev.x, &ev.y);
					//psy_ui_mouseevent_settarget(&ev, eventtarget(imp->component));
					imp->component->vtable->onmousemove(imp->component, &ev);
					psy_signal_emit(&imp->component->signal_mousemove,
						imp->component, 1, &ev);					
					return 0 ;
				}
				break;			
			case WM_SETTINGCHANGE: {
				static int ulScrollLines;

				SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0,
					&ulScrollLines, 0) ;
      
			   // ulScrollLines usually equals 3 or 0 (for no scrolling)
			   // WHEEL_DELTA equals 120, so iDeltaPerLine will be 40
				if (ulScrollLines) {
					iDeltaPerLine = WHEEL_DELTA / ulScrollLines;
				} else {
					iDeltaPerLine = 0;
				}
				return 0;
				break; }
			case WM_MOUSEWHEEL:	
			{
				int preventdefault = 0;				
				psy_ui_MouseEvent ev;
				POINT pt_client;

				pt_client.x = (SHORT)LOWORD(lParam);
				pt_client.y = (SHORT)HIWORD(lParam);
				ScreenToClient(imp->hwnd, &pt_client);				
				psy_ui_mouseevent_init(&ev,
					pt_client.x,
					pt_client.y,
					(short)LOWORD(wParam),
					(short)HIWORD(wParam),
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
				adjustcoordinates(imp->component, &ev.x, &ev.y);
				imp->component->vtable->onmousewheel(imp->component, &ev);
				psy_signal_emit(&imp->component->signal_mousewheel, imp->component, 1,
					&ev);
				preventdefault = ev.preventdefault;
				if (!preventdefault && imp->component->wheelscroll > 0) {
					if (iDeltaPerLine != 0) {
						imp->component->accumwheeldelta += (short)HIWORD(wParam); // 120 or -120
						while (imp->component->accumwheeldelta >= iDeltaPerLine)
						{
							double iPos;
							intptr_t scrollmin;
							intptr_t scrollmax;
							psy_ui_Value scrolltop;
							psy_ui_TextMetric tm;

							tm = psy_ui_component_textmetric(imp->component);
							psy_ui_component_verticalscrollrange(imp->component, &scrollmin,
								&scrollmax);							
							scrolltop = psy_ui_component_scrolltop(imp->component);
							iPos =  psy_ui_value_px(&scrolltop, &tm) / 
								psy_ui_value_px(&imp->component->scrollstepy, &tm) -
								imp->component->wheelscroll;
							if (iPos < scrollmin) {
								iPos = scrollmin;
							}
							if (imp->component->handlevscroll) {
								psy_ui_component_setscrolltop(imp->component,
									psy_ui_mul_value_real(imp->component->scrollstepy, iPos));
							}							
							imp->component->accumwheeldelta -= iDeltaPerLine;
						}
						while (imp->component->accumwheeldelta <= -iDeltaPerLine)
						{
							double iPos;
							intptr_t scrollmin;
							intptr_t scrollmax;
							psy_ui_Value scrolltop;
							psy_ui_TextMetric tm;

							tm = psy_ui_component_textmetric(imp->component);
							psy_ui_component_verticalscrollrange(imp->component, &scrollmin,
								&scrollmax);		
							scrolltop = psy_ui_component_scrolltop(imp->component);
							iPos = psy_ui_value_px(&scrolltop, &tm) /
								psy_ui_value_px(&imp->component->scrollstepy, &tm) +
								imp->component->wheelscroll;
							if (iPos > scrollmax) {
								iPos = scrollmax;
							}
							if (imp->component->handlevscroll) {
								psy_ui_component_setscrolltop(imp->component,
									psy_ui_mul_value_real(imp->component->scrollstepy, iPos));
							}							
							imp->component->accumwheeldelta += iDeltaPerLine;
						}
					}
				}
			}
			break;
			case WM_MOUSEHOVER:			
				if (imp->component->signal_mousehover.slots) {	                    
					psy_signal_emit(&imp->component->signal_mousehover, imp->component, 0);
					return 0;
				}
			break;
			case WM_MOUSELEAVE:
			{
				imp->component->mousetracking = 0;
				imp->component->vtable->onmouseleave(imp->component);
				psy_signal_emit(&imp->component->signal_mouseleave, imp->component, 0);
				return 0;
			}				
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
				imp->component->vtable->onfocuslost(imp->component);
				psy_signal_emit(&imp->component->signal_focuslost, imp->component, 0);
				return 0;				
			break;
			default:			
			break;
		}	
	}
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}

psy_ui_Component* eventtarget(psy_ui_Component* component)
{
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	if (winapp->targetids) {
		HWND targethwnd;
		psy_ui_win_ComponentImp* targetimp;

		targethwnd = (winapp->targetids)
			? (HWND)(uintptr_t)(winapp->targetids->entry)
			: NULL;
		targetimp = (psy_ui_win_ComponentImp*)psy_table_at(
			&winapp->selfmap,
			(uintptr_t)targethwnd);
		if (targetimp) {
			return targetimp->component;
		}
	}
	return component;
}

void sendmessagetoparent(psy_ui_win_ComponentImp* imp, uintptr_t message, WPARAM wparam, LPARAM lparam)
{
	psy_ui_WinApp* winapp;
	
	winapp = (psy_ui_WinApp*)app.platform;	
	if (psy_table_at(&winapp->selfmap,
			(uintptr_t)GetParent(imp->hwnd))) {
		psy_list_append(&winapp->targetids, imp->hwnd);
		winapp->eventretarget = imp->component;
		SendMessage(GetParent(imp->hwnd), (UINT)message, wparam, lparam);
	} else {
		psy_list_free(winapp->targetids);
		winapp->targetids = NULL;
	}
	winapp->eventretarget = 0;
}

void adjustcoordinates(psy_ui_Component* component, intptr_t* x, intptr_t* y)
{		
	psy_ui_TextMetric tm;
	tm = psy_ui_component_textmetric(component);

	*x += psy_ui_value_px(&component->scroll.x, &tm);
	*y += psy_ui_value_px(&component->scroll.y, &tm);
	if (!psy_ui_margin_iszero(&component->spacing)) {				
		*x -= psy_ui_value_px(&component->spacing.left, &tm);
		*y -= psy_ui_value_px(&component->spacing.top, &tm);
	}
}

void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO		si;	
    int				iPos; //, iHorzPos;	
	psy_ui_win_ComponentImp* imp;
     
	si.cbSize = sizeof (si) ;
    si.fMask  = SIF_ALL ;
    GetScrollInfo (hwnd, SB_VERT, &si) ;	
	// Save the position for comparison later on
	iPos = si.nPos ;

	handle_scrollparam(hwnd, &si, wParam);	
	// Set the position and then retrieve it.  Due to adjustments
	// by Windows it may not be the same as the value set.
	si.fMask = SIF_POS ;
	SetScrollInfo (hwnd, SB_VERT, &si, TRUE) ;
	GetScrollInfo (hwnd, SB_VERT, &si) ;
	// If the position has changed, scroll the window and update it
	if (si.nPos != iPos)
	{
		psy_ui_WinApp* winapp;

		winapp = (psy_ui_WinApp*) app.platform;
		imp = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);	
		if (imp->component->handlevscroll) {
			psy_ui_TextMetric tm;
			psy_ui_Value scrolltop;

			psy_ui_component_textmetric(imp->component);
			scrolltop = psy_ui_component_scrolltop(imp->component);
			psy_ui_component_setscrolltop(imp->component,
				psy_ui_value_makepx(
					psy_ui_value_px(&scrolltop, &tm) -
					psy_ui_value_px(&imp->component->scrollstepy, &tm) * (iPos - si.nPos)));
		}			
	}
}
void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO		si;	
    int				iPos; 
	psy_ui_win_ComponentImp* imp;
     
	si.cbSize = sizeof (si) ;
    si.fMask  = SIF_ALL ;
    GetScrollInfo (hwnd, SB_HORZ, &si) ;	
	// Save the position for comparison later on
	iPos = si.nPos ;
	handle_scrollparam(hwnd, &si, wParam);
	// Set the position and then retrieve it.  Due to adjustments
	// by Windows it may not be the same as the value set.
	si.fMask = SIF_POS ;
	SetScrollInfo (hwnd, SB_HORZ, &si, TRUE) ;
	GetScrollInfo (hwnd, SB_HORZ, &si) ;
	// If the position has changed, scroll the window and update it
	if (si.nPos != iPos)
	{                    
		psy_ui_WinApp* winapp;

		winapp = (psy_ui_WinApp*) app.platform;
		imp = psy_table_at(&winapp->selfmap, (uintptr_t) hwnd);
		if (imp->component->handlehscroll) {
			psy_ui_TextMetric tm;
			psy_ui_Value scrollleft;

			tm = psy_ui_component_textmetric(imp->component);
			scrollleft = psy_ui_component_scrollleft(imp->component);
			psy_ui_component_setscrollleft(imp->component,
				psy_ui_value_makepx(
					psy_ui_value_px(&scrollleft, &tm) -
					psy_ui_value_px(&imp->component->scrollstepx, &tm) *
						(iPos - si.nPos)));
		}		
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

int psy_ui_winapp_run(psy_ui_WinApp* self) 
{
	MSG msg; 

	// __try
	// {
		while (GetMessage (&msg, NULL, 0, 0))
		{
			  TranslateMessage (&msg) ;
			  DispatchMessage (&msg) ;
		}
	// }
	// __except(FilterException("app loop", GetExceptionCode(), GetExceptionInformation())) {			
	// }
    return (int) msg.wParam ;
}

void psy_ui_winapp_stop(psy_ui_WinApp* self)
{
	PostQuitMessage(0);
}

void psy_ui_winapp_close(psy_ui_WinApp* self, psy_ui_Component* main)
{	
	PostMessage(((psy_ui_win_ComponentImp*)(main->imp))->hwnd, WM_CLOSE, 0, 0);	
}

void onlanguagechanged(psy_ui_WinApp* self, psy_Translator* sender)
{

}

#endif

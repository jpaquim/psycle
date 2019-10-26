// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UICOMPONENT_H)
#define UICOMPONENT_H

#include <windows.h>
#include "uidef.h"
#include "graphics.h"
#include <signal.h>
#include "uimenu.h"
#include <list.h>

typedef enum {
	UI_ALIGN_NONE,
	UI_ALIGN_CLIENT,
	UI_ALIGN_TOP,
	UI_ALIGN_LEFT,
	UI_ALIGN_BOTTOM,
	UI_ALIGN_RIGHT,
	UI_ALIGN_FILL
} UiAlignType;

typedef enum {
	UI_JUSTIFY_NONE,	
	UI_JUSTIFY_EXPAND	
} UiJustifyType;

typedef enum {
	BACKGROUND_NONE,
	BACKGROUND_SET,
	BACKGROUND_PARENT,
} BackgroundMode;

typedef enum {
	UI_NOEXPAND = 1,	
	UI_HORIZONTALEXPAND = 2,
	UI_VERTICALEXPAND = 4	
} UiExpandMode;

typedef struct {		               
   int (*childenum)(void*, void*);   
   void* context;
} EnumCallback;

typedef LRESULT(CALLBACK *winproc)(HWND hwnd, UINT message, WPARAM wParam,
	LPARAM lParam);

typedef struct {
	HWND hwnd;
	HMENU winid;	
	Signal signal_size;
	Signal signal_draw;
	Signal signal_timer;
	Signal signal_keydown;
	Signal signal_keyup;
	Signal signal_mousedown;
	Signal signal_mouseup;
	Signal signal_mousemove;
	Signal signal_mousedoubleclick;
	Signal signal_mouseenter;
	Signal signal_mousehover;
	Signal signal_mouseleave;
	Signal signal_scroll;
	Signal signal_create;
	Signal signal_destroy;
	Signal signal_childenum;
	Signal signal_show;
	Signal signal_hide;
	Signal signal_windowproc;
	Signal signal_align;
	Signal signal_preferredsize;
	Signal signal_command;
	EnumCallback childenum;
	UiAlignType align;
	UiJustifyType justify;
	int alignexpandmode;
	int alignchildren;
	ui_margin margin;
	int doublebuffered;
	int defaultpropagation;
	int propagateevent;
	int preventdefault;
	int scrollstepx;
	int scrollstepy;
	int debugflag;
	unsigned int backgroundcolor;
	unsigned int color;
	BackgroundMode backgroundmode;
	ui_font font;
	HBRUSH background;
	int visible;
	winproc wndproc;
} ui_component;

void ui_init(HINSTANCE hInstance);
void ui_dispose();
void ui_replacedefaultfont(ui_component* main, ui_font* font);

void ui_component_init(ui_component*, ui_component* parent);
void ui_component_dispose(ui_component*);
void ui_component_destroy(ui_component*);
void ui_frame_init(ui_component*, ui_component* parent);
int ui_win32_component_init(ui_component*, ui_component* parent,
	LPCTSTR classname, int x, int y, int width, int height,
	DWORD dwStyle, int usecommand);
void ui_component_show(ui_component*);
void ui_component_hide(ui_component*);
void ui_component_show_state(ui_component*, int cmd);
void ui_component_showhorizontalscrollbar(ui_component*);
void ui_component_hidehorizontalscrollbar(ui_component*);
void ui_component_sethorizontalscrollrange(ui_component*, int min, int max);
void ui_component_showverticalscrollbar(ui_component*);
void ui_component_hideverticalscrollbar(ui_component*);
void ui_component_setverticalscrollrange(ui_component*, int min, int max);
void ui_component_verticalscrollrange(ui_component* self, int* scrollmin,
	int* scrollmax);
void ui_component_setverticalscrollposition(ui_component*, int position);
void ui_component_sethorizontalscrollposition(ui_component*, int position);
void ui_component_move(ui_component*, int left, int top);
void ui_component_resize(ui_component*, int width, int height);
void ui_component_setposition(ui_component*, int x, int y, int width, int height);
void ui_component_setmenu(ui_component*, ui_menu* menu);
void ui_component_settitle(ui_component*, const char* title);
ui_component* ui_component_parent(ui_component* component);
void ui_component_enumerate_children(ui_component*, void* context, int (*childenum)(void*, void*));
List* ui_component_children(ui_component*, int recursive);
void ui_component_capture(ui_component*);
void ui_component_releasecapture();
ui_size ui_component_size(ui_component*);
ui_size ui_component_frame_size(ui_component*);
void ui_invalidate(ui_component*);
void ui_invalidaterect(ui_component*, const ui_rectangle* rect);
void ui_component_setfocus(ui_component*);
int ui_component_hasfocus(ui_component*);
void ui_component_setfont(ui_component*, ui_font* font);
void ui_component_propagateevent(ui_component*);
void ui_component_preventdefault(ui_component*);
void ui_component_init_base(ui_component*);
void ui_component_init_signals(ui_component*);
int ui_component_visible(ui_component*);
void ui_component_align(ui_component*);
void ui_component_setmargin(ui_component*, const ui_margin*);
void ui_component_setalign(ui_component*, UiAlignType align);
void ui_component_enablealign(ui_component*);
void ui_component_setalignexpand(ui_component*, UiExpandMode);
void ui_component_preventalign(ui_component*);
void ui_component_enableinput(ui_component*, int recursive);
void ui_component_preventinput(ui_component*, int recursive);
void ui_component_setbackgroundmode(ui_component*, BackgroundMode);
void ui_component_setbackgroundcolor(ui_component*, unsigned int color);
void ui_component_setcolor(ui_component*, unsigned int color);
ui_size ui_component_textsize(ui_component*, const char*);
TEXTMETRIC ui_component_textmetric(ui_component*);
ui_size ui_component_preferredsize(ui_component*, ui_size* limit);

List* ui_components_setalign(List*, UiAlignType align, const ui_margin* margin);
List* ui_components_setmargin(List* list, const ui_margin* margin);

int ui_openfile(ui_component* self, char* title, char* filter,
	char* defextension, char* filename);

#endif
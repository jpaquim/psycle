// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(UICOMPONENT)
#define UICOMPONENT

#include <windows.h>
#include "uidef.h"
#include "uievents.h"
#include "uimenu.h"


void ui_init(HINSTANCE hInstance);
void ui_dispose();

typedef struct {
	HWND hwnd;
	HMENU winid;
	ui_events events;
	Signal signal_size;
	Signal signal_draw;
	Signal signal_timer;
	Signal signal_keydown;
	Signal signal_keyup;
	Signal signal_mousedown;
	Signal signal_mouseup;
	Signal signal_mousemove;
	Signal signal_mousedoubleclick;
	Signal signal_scroll;
	Signal signal_create;
	Signal signal_destroy;
	Signal signal_childenum;
	Signal signal_show;
	Signal signal_hide;
	int align;   
	int doublebuffered;
	int propagateevent;
	int scrollstepx;
	int scrollstepy;
} ui_component;

void ui_component_init(ui_component* component, ui_component* parent);
void ui_component_dispose(ui_component* component);
void ui_frame_init(ui_component* frame, ui_component* parent);
void ui_classcomponent_init(ui_component* component, ui_component* parent, const char* classname);

void ui_component_show(ui_component* self);
void ui_component_hide(ui_component* self);
void ui_component_show_state(ui_component* self, int cmd);
void ui_component_showhorizontalscrollbar(ui_component* self);
void ui_component_hidehorizontalscrollbar(ui_component* self);
void ui_component_sethorizontalscrollrange(ui_component* self, int min, int max);
void ui_component_showverticalscrollbar(ui_component* self);
void ui_component_hideverticalscrollbar(ui_component* self);
void ui_component_setverticalscrollrange(ui_component* self, int min, int max);
void ui_component_move(ui_component* self, int left, int top);
void ui_component_resize(ui_component* self, int width, int height);
void ui_component_setmenu(ui_component* self, ui_menu* menu);
void ui_component_settitle(ui_component* self, const char* title);
void ui_component_enumerate_children(ui_component* self, void* context, int (*childenum)(void*, void*));
void ui_component_capture(ui_component* self);
void ui_component_releasecapture();
ui_size ui_component_size(ui_component* self);
void ui_invalidate(ui_component* self);
void ui_component_setfocus(ui_component* self);
void ui_component_setfont(ui_component* self, HFONT font);
void ui_component_propagateevent(ui_component* self);
void ui_component_init_signals(ui_component* component);
int ui_component_visible(ui_component* component);


#endif
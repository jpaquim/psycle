// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(UICOMPONENT_H)
#define UICOMPONENT_H

#include "uidef.h"
#include "uievents.h"
#include "uigraphics.h"
#include <signal.h>
#include "uimenu.h"
#include <list.h>
#include "../../detail/stdint.h"

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

typedef enum {
	UI_HORIZONTAL,
	UI_VERTICAL
} UiOrientation;

typedef struct {		               
   int (*childenum)(void*, void*);   
   void* context;
} EnumCallback;


typedef LRESULT(CALLBACK *winproc)(HWND hwnd, UINT message, WPARAM wParam,
	LPARAM lParam);

// vtable function pointers
typedef void (*psy_ui_fp_preferredsize)(struct psy_ui_Component*,
	ui_size* limit, ui_size* rv);
typedef void (*psy_ui_fp_draw)(struct psy_ui_Component*, psy_ui_Graphics*);

typedef struct psy_ui_ComponentVTable {
	psy_ui_fp_draw draw;
	psy_ui_fp_preferredsize preferredsize;
} psy_ui_ComponentVtable;

typedef struct psy_ui_Component {
	psy_ui_ComponentVtable* vtable;
	uintptr_t hwnd;
	uintptr_t winid;	
	psy_Signal signal_size;
	psy_Signal signal_draw;
	psy_Signal signal_timer;
	psy_Signal signal_keydown;
	psy_Signal signal_keyup;	
	psy_Signal signal_mousedown;
	psy_Signal signal_mouseup;
	psy_Signal signal_mousemove;
	psy_Signal signal_mousewheel;
	psy_Signal signal_mousedoubleclick;
	psy_Signal signal_mouseenter;
	psy_Signal signal_mousehover;
	psy_Signal signal_mouseleave;
	psy_Signal signal_scroll;
	psy_Signal signal_create;
	psy_Signal signal_destroy;
	psy_Signal signal_childenum;
	psy_Signal signal_show;
	psy_Signal signal_hide;
	psy_Signal signal_windowproc;
	psy_Signal signal_align;
	//psy_Signal signal_preferredsize;
	psy_Signal signal_command;
	psy_Signal signal_focuslost;
	psy_Signal signal_focus;
	EnumCallback childenum;
	UiAlignType align;
	UiJustifyType justify;
	int alignexpandmode;
	int alignchildren;
	ui_margin margin;
	ui_margin spacing;
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
	int accumwheeldelta;
	int wheelscroll;
	int handlevscroll;
	int handlehscroll;
	int cursor;	
} psy_ui_Component;

void ui_replacedefaultfont(psy_ui_Component* main, ui_font* font);

void ui_component_init(psy_ui_Component*, psy_ui_Component* parent);
void ui_component_dispose(psy_ui_Component*);
void ui_component_destroy(psy_ui_Component*);
void ui_frame_init(psy_ui_Component*, psy_ui_Component* parent);
int ui_win32_component_init(psy_ui_Component*, psy_ui_Component* parent,
	LPCTSTR classname, int x, int y, int width, int height,
	DWORD dwStyle, int usecommand);
void ui_component_show(psy_ui_Component*);
void ui_component_hide(psy_ui_Component*);
void ui_component_show_state(psy_ui_Component*, int cmd);
void ui_component_showhorizontalscrollbar(psy_ui_Component*);
void ui_component_hidehorizontalscrollbar(psy_ui_Component*);
void ui_component_sethorizontalscrollrange(psy_ui_Component*, int min, int max);
void ui_component_showverticalscrollbar(psy_ui_Component*);
void ui_component_hideverticalscrollbar(psy_ui_Component*);
void ui_component_setverticalscrollrange(psy_ui_Component*, int min, int max);
void ui_component_verticalscrollrange(psy_ui_Component* self, int* scrollmin,
	int* scrollmax);
int ui_component_verticalscrollposition(psy_ui_Component* self);
void ui_component_setverticalscrollposition(psy_ui_Component*, int position);
void ui_component_sethorizontalscrollposition(psy_ui_Component*, int position);
int ui_component_horizontalscrollposition(psy_ui_Component*);
void ui_component_scrollstep(psy_ui_Component*, intptr_t stepx, intptr_t stepy);
void ui_component_move(psy_ui_Component*, int left, int top);
void ui_component_resize(psy_ui_Component*, int width, int height);
void ui_component_setposition(psy_ui_Component*, int x, int y, int width, int height);
void ui_component_setmenu(psy_ui_Component*, ui_menu* menu);
void ui_component_settitle(psy_ui_Component*, const char* title);
psy_ui_Component* ui_component_parent(psy_ui_Component* component);
void ui_component_enumerate_children(psy_ui_Component*, void* context, int (*childenum)(void*, void*));
psy_List* ui_component_children(psy_ui_Component*, int recursive);
void ui_component_capture(psy_ui_Component*);
void ui_component_releasecapture();
ui_size ui_component_size(psy_ui_Component*);
ui_rectangle ui_component_position(psy_ui_Component*);
ui_size ui_component_frame_size(psy_ui_Component*);
void ui_component_invalidate(psy_ui_Component*);
void ui_component_invalidaterect(psy_ui_Component*, const ui_rectangle* rect);
void ui_component_update(psy_ui_Component*);
void ui_component_setfocus(psy_ui_Component*);
int ui_component_hasfocus(psy_ui_Component*);
void ui_component_setfont(psy_ui_Component*, ui_font* font);
void ui_component_propagateevent(psy_ui_Component*);
void ui_component_preventdefault(psy_ui_Component*);
void ui_component_init_base(psy_ui_Component*);
void ui_component_init_signals(psy_ui_Component*);
int ui_component_visible(psy_ui_Component*);
void ui_component_align(psy_ui_Component*);
void ui_component_setmargin(psy_ui_Component*, const ui_margin*);
void ui_component_setspacing(psy_ui_Component*, const ui_margin*);
void ui_component_setalign(psy_ui_Component*, UiAlignType align);
void ui_component_enablealign(psy_ui_Component*);
void ui_component_setalignexpand(psy_ui_Component*, UiExpandMode);
void ui_component_preventalign(psy_ui_Component*);
void ui_component_enableinput(psy_ui_Component*, int recursive);
void ui_component_preventinput(psy_ui_Component*, int recursive);
void ui_component_setbackgroundmode(psy_ui_Component*, BackgroundMode);
void ui_component_setbackgroundcolor(psy_ui_Component*, uint32_t color);
void ui_component_setcolor(psy_ui_Component*, uint32_t color);
void ui_component_starttimer(psy_ui_Component*, unsigned int id, unsigned int interval);
void ui_component_stoptimer(psy_ui_Component*, unsigned int id);
ui_size ui_component_textsize(psy_ui_Component*, const char*);
ui_textmetric ui_component_textmetric(psy_ui_Component*);
ui_size ui_component_preferredsize(psy_ui_Component*, ui_size* limit);
void ui_component_seticonressource(psy_ui_Component*, int ressourceid);

psy_List* ui_components_setalign(psy_List*, UiAlignType, const ui_margin*);
psy_List* ui_components_setmargin(psy_List*, const ui_margin*);

int ui_openfile(psy_ui_Component* self, char* title, char* filter,
	char* defextension, const char* szInitialDir, char* filename);
int ui_savefile(psy_ui_Component* self, char* title, char* filter,
	char* defextension, const char* szInitialDir, char* filename);
int ui_browsefolder(psy_ui_Component* self, const char* title, char* path);


#endif

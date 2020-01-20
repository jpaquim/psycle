// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_COMPONENT_H
#define psy_ui_COMPONENT_H

#include "uidef.h"
#include "uievents.h"
#include "uigraphics.h"
#include <signal.h>
#include "uimenu.h"
#include <list.h>
#include "../../detail/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	BACKGROUND_NONE,
	BACKGROUND_SET,
	BACKGROUND_PARENT,
} BackgroundMode;

typedef LRESULT(CALLBACK *winproc)(HWND hwnd, UINT message, WPARAM wParam,
	LPARAM lParam);

// vtable function pointers
typedef void (*psy_ui_fp_onpreferredsize)(struct psy_ui_Component*,
	psy_ui_Size* limit, psy_ui_Size* rv);
typedef void (*psy_ui_fp_ondraw)(struct psy_ui_Component*, psy_ui_Graphics*);
typedef void (*psy_ui_fp_onsize)(struct psy_ui_Component*, const psy_ui_Size*);
typedef void (*psy_ui_fp_onmousedown)(struct psy_ui_Component*, psy_ui_MouseEvent*);
typedef void (*psy_ui_fp_onmousemove)(struct psy_ui_Component*, psy_ui_MouseEvent*);
typedef void (*psy_ui_fp_onmouseup)(struct psy_ui_Component*, psy_ui_MouseEvent*);
typedef void (*psy_ui_fp_onmousedoubleclick)(struct psy_ui_Component*, psy_ui_MouseEvent*);
typedef void (*psy_ui_fp_onmouseenter)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_onmouseleave)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_onkeydown)(struct psy_ui_Component*, psy_ui_KeyEvent*);
typedef void (*psy_ui_fp_onkeyup)(struct psy_ui_Component*, psy_ui_KeyEvent*);

typedef struct psy_ui_ComponentVTable {
	psy_ui_fp_ondraw ondraw;
	psy_ui_fp_onsize onsize;
	psy_ui_fp_onpreferredsize onpreferredsize;
	psy_ui_fp_onmousedown onmousedown;
	psy_ui_fp_onmousemove onmousemove;
	psy_ui_fp_onmouseup onmouseup;
	psy_ui_fp_onmousedoubleclick onmousedoubleclick;
	psy_ui_fp_onmouseenter onmouseenter;
	psy_ui_fp_onmouseleave onmouseleave;
	psy_ui_fp_onkeydown onkeydown;
	psy_ui_fp_onkeydown onkeyup;
} psy_ui_ComponentVtable;

typedef struct {
	uintptr_t hwnd;
	uintptr_t winid;
	HBRUSH background;
	winproc wndproc;	
} psy_ui_win_ComponentDetails;

typedef struct psy_ui_Component {
	psy_ui_ComponentVtable* vtable;	
	psy_ui_win_ComponentDetails* platform;
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
	psy_Signal signal_destroyed;
	psy_Signal signal_childenum;
	psy_Signal signal_show;
	psy_Signal signal_hide;	
	psy_Signal signal_align;
	//psy_Signal signal_preferredsize;
	psy_Signal signal_command;
	psy_Signal signal_focuslost;
	psy_Signal signal_focus;	
	psy_ui_AlignType align;
	psy_ui_JustifyType justify;
	int alignexpandmode;
	int alignchildren;
	psy_ui_Margin margin;
	psy_ui_Margin spacing;
	int doublebuffered;	
	int preventdefault;
	int preventpreferredsize;
	int scrollstepx;
	int scrollstepy;
	int debugflag;
	unsigned int backgroundcolor;
	unsigned int color;
	BackgroundMode backgroundmode;
	psy_ui_Font font;	
	int visible;	
	int accumwheeldelta;
	int wheelscroll;
	int handlevscroll;
	int handlehscroll;
	int cursor;	
} psy_ui_Component;

void psy_ui_replacedefaultfont(psy_ui_Component* main, psy_ui_Font*);

void psy_ui_component_init(psy_ui_Component*, psy_ui_Component* parent);
void psy_ui_component_dispose(psy_ui_Component*);
void psy_ui_component_destroy(psy_ui_Component*);
int psy_ui_win32_component_init(psy_ui_Component*, psy_ui_Component* parent,
	LPCTSTR classname, int x, int y, int width, int height,
	DWORD dwStyle, int usecommand);
void psy_ui_component_show(psy_ui_Component*);
void psy_ui_component_hide(psy_ui_Component*);
void psy_ui_component_show_state(psy_ui_Component*, int cmd);
void psy_ui_component_showhorizontalscrollbar(psy_ui_Component*);
void psy_ui_component_hidehorizontalscrollbar(psy_ui_Component*);
void psy_ui_component_sethorizontalscrollrange(psy_ui_Component*, int min, int max);
void psy_ui_component_showverticalscrollbar(psy_ui_Component*);
void psy_ui_component_hideverticalscrollbar(psy_ui_Component*);
void psy_ui_component_setverticalscrollrange(psy_ui_Component*, int min, int max);
void psy_ui_component_verticalscrollrange(psy_ui_Component* self, int* scrollmin,
	int* scrollmax);
int psy_ui_component_verticalscrollposition(psy_ui_Component* self);
void psy_ui_component_setverticalscrollposition(psy_ui_Component*, int position);
void psy_ui_component_sethorizontalscrollposition(psy_ui_Component*, int position);
int psy_ui_component_horizontalscrollposition(psy_ui_Component*);
void psy_ui_component_scrollstep(psy_ui_Component*, intptr_t stepx, intptr_t stepy);
void psy_ui_component_move(psy_ui_Component*, int left, int top);
void psy_ui_component_resize(psy_ui_Component*, int width, int height);
void psy_ui_component_clientresize(psy_ui_Component*, int width, int height);
void psy_ui_component_setposition(psy_ui_Component*, int x, int y, int width, int height);
void psy_ui_component_setmenu(psy_ui_Component*, psy_ui_Menu* menu);
void psy_ui_component_settitle(psy_ui_Component*, const char* title);
psy_ui_Component* psy_ui_component_parent(psy_ui_Component* component);
void psy_ui_component_enumerate_children(psy_ui_Component*, void* context, int (*childenum)(void*, void*));
psy_List* psy_ui_component_children(psy_ui_Component*, int recursive);
void psy_ui_component_capture(psy_ui_Component*);
void psy_ui_component_releasecapture();
psy_ui_Size psy_ui_component_size(psy_ui_Component*);
psy_ui_Rectangle psy_ui_component_position(psy_ui_Component*);
psy_ui_Size psy_ui_component_frame_size(psy_ui_Component*);
void psy_ui_component_invalidate(psy_ui_Component*);
void psy_ui_component_invalidaterect(psy_ui_Component*, const psy_ui_Rectangle*);
void psy_ui_component_update(psy_ui_Component*);
void psy_ui_component_setfocus(psy_ui_Component*);
int psy_ui_component_hasfocus(psy_ui_Component*);
void psy_ui_component_setfont(psy_ui_Component*, psy_ui_Font*);
void psy_ui_component_preventdefault(psy_ui_Component*);
void psy_ui_component_init_base(psy_ui_Component*);
void psy_ui_component_init_signals(psy_ui_Component*);
int psy_ui_component_visible(psy_ui_Component*);
void psy_ui_component_align(psy_ui_Component*);
void psy_ui_component_setmargin(psy_ui_Component*, const psy_ui_Margin*);
void psy_ui_component_setspacing(psy_ui_Component*, const psy_ui_Margin*);
void psy_ui_component_setalign(psy_ui_Component*, psy_ui_AlignType align);
void psy_ui_component_enablealign(psy_ui_Component*);
void psy_ui_component_setalignexpand(psy_ui_Component*, psy_ui_ExpandMode);
void psy_ui_component_preventalign(psy_ui_Component*);
void psy_ui_component_enableinput(psy_ui_Component*, int recursive);
void psy_ui_component_preventinput(psy_ui_Component*, int recursive);
void psy_ui_component_setbackgroundmode(psy_ui_Component*, BackgroundMode);
void psy_ui_component_setbackgroundcolor(psy_ui_Component*, uint32_t color);
void psy_ui_component_setcolor(psy_ui_Component*, uint32_t color);
void psy_ui_component_starttimer(psy_ui_Component*, unsigned int id, unsigned int interval);
void psy_ui_component_stoptimer(psy_ui_Component*, unsigned int id);
psy_ui_Size psy_ui_component_textsize(psy_ui_Component*, const char*);
psy_ui_TextMetric psy_ui_component_textmetric(psy_ui_Component*);
psy_ui_Size psy_ui_component_preferredsize(psy_ui_Component*, psy_ui_Size* limit);
void psy_ui_component_seticonressource(psy_ui_Component*, int ressourceid);
void psy_ui_component_doublebuffer(psy_ui_Component*);

psy_List* psy_ui_components_setalign(psy_List*, psy_ui_AlignType,
	const psy_ui_Margin*);
psy_List* psy_ui_components_setmargin(psy_List*, const psy_ui_Margin*);

int psy_ui_browsefolder(psy_ui_Component* self, const char* title, char* path);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMPONENT_H */

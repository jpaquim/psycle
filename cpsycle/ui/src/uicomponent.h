// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_COMPONENT_H
#define psy_ui_COMPONENT_H

#include "uidef.h"
#include "uievents.h"
#include "uigraphics.h"
#include <signal.h>
#include "uimenu.h"
#include "uistyle.h"
#include <list.h>
#include "../../detail/stdint.h"

// psy_ui_Component
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_Component
// Implementor			psy_ui_ComponentImp
// Concrete Implementor	psy_ui_win_ComponentImp
//
// Backreference imp to component for imp/component mapping and events
//
// psy_ui_Component <>--------<> psy_ui_ComponentImp
//     imp->dev_invalidate           ^
//     ...                           |
//                          psy_ui_win_ComponentImp

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	psy_ui_CURSORSTYLE_AUTO,
	psy_ui_CURSORSTYLE_MOVE,
	psy_ui_CURSORSTYLE_NODROP,
	psy_ui_CURSORSTYLE_COL_RESIZE,
	psy_ui_CURSORSTYLE_ALL_SCROLL,
	psy_ui_CURSORSTYLE_POINTER,
	psy_ui_CURSORSTYLE_NOT_ALLOWED,
	psy_ui_CURSORSTYLE_ROW_RESIZE,
	psy_ui_CURSORSTYLE_CROSSHAIR,
	psy_ui_CURSORSTYLE_PROGRESS,
	psy_ui_CURSORSTYLE_E_RESIZE,
	psy_ui_CURSORSTYLE_NE_RESIZE,
	psy_ui_CURSORSTYLE_DEFAULT_TEXT,
	psy_ui_CURSORSTYLE_N_RESIZE,
	psy_ui_CURSORSTYLE_NW_RESIZE,
	psy_ui_CURSORSTYLE_HELP,
	psy_ui_CURSORSTYLE_VERTICAL_TEXT,
	psy_ui_CURSORSTYLE_S_RESIZE,
	psy_ui_CURSORSTYLE_SE_RESIZE,
	psy_ui_CURSORSTYLE_INHERIT,
	psy_ui_CURSORSTYLE_WAIT,
	psy_ui_CURSORSTYLE_W_RESIZE,
	psy_ui_CURSORSTYLE_SW_RESIZE
} psy_ui_CursorStyle;

typedef enum {
	BACKGROUND_NONE,
	BACKGROUND_SET,
	BACKGROUND_PARENT,
} BackgroundMode;

struct psy_ui_Component;

// vtable function pointers
typedef void (*psy_ui_fp_component_dispose)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_destroy)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_show)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_showstate)(struct psy_ui_Component*, int state);
typedef void (*psy_ui_fp_component_hide)(struct psy_ui_Component*);
typedef int (*psy_ui_fp_component_visible)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_move)(struct psy_ui_Component*, int left, int top);
typedef void (*psy_ui_fp_component_resize)(struct psy_ui_Component*, int width, int height);
typedef void (*psy_ui_fp_component_clientresize)(struct psy_ui_Component*, int width, int height);
typedef void (*psy_ui_fp_component_setposition)(struct psy_ui_Component*, int x, int y, int width, int height);
typedef psy_ui_Size(*psy_ui_fp_component_framesize)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_scrollto)(struct psy_ui_Component*, intptr_t dx, intptr_t dy);
typedef void (*psy_ui_fp_component_setfont)(struct psy_ui_Component*, psy_ui_Font*);
typedef void (*psy_ui_fp_component_showhorizontalscrollbar)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_hidehorizontalscrollbar)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_sethorizontalscrollrange)(struct psy_ui_Component*, int min, int max);
typedef void (*psy_ui_fp_component_horizontalscrollrange)(struct psy_ui_Component* self, int* scrollmin,
	int* scrollmax);
typedef int (*psy_ui_fp_component_horizontalscrollposition)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_sethorizontalscrollposition)(struct psy_ui_Component*, int position);
typedef void (*psy_ui_fp_component_showverticalscrollbar)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_hideverticalscrollbar)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_setverticalscrollrange)(struct psy_ui_Component*, int min, int max);
typedef void (*psy_ui_fp_component_verticalscrollrange)(struct psy_ui_Component* self, int* scrollmin,
	int* scrollmax);
typedef int (*psy_ui_fp_component_verticalscrollposition)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_setverticalscrollposition)(struct psy_ui_Component*, int position);
typedef psy_List* (*psy_ui_fp_component_children)(struct psy_ui_Component* self, int recursive);
// vtable events function pointers 
typedef void (*psy_ui_fp_onalign)(struct psy_ui_Component*);
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
	psy_ui_fp_component_dispose dispose;
	psy_ui_fp_component_destroy destroy;
	psy_ui_fp_component_show show;
	psy_ui_fp_component_showstate showstate;
	psy_ui_fp_component_hide hide;
	psy_ui_fp_component_visible visible;
	psy_ui_fp_component_move move;
	psy_ui_fp_component_resize resize;
	psy_ui_fp_component_clientresize clientresize;	
	psy_ui_fp_component_setposition setposition;
	psy_ui_fp_component_framesize framesize;
	psy_ui_fp_component_scrollto scrollto;
	psy_ui_fp_component_setfont setfont;
	psy_ui_fp_component_showhorizontalscrollbar showhorizontalscrollbar;
	psy_ui_fp_component_hidehorizontalscrollbar hidehorizontalscrollbar;
	psy_ui_fp_component_sethorizontalscrollrange sethorizontalscrollrange;
	psy_ui_fp_component_horizontalscrollrange horizontalscrollrange;
	psy_ui_fp_component_horizontalscrollposition horizontalscrollposition;
	psy_ui_fp_component_sethorizontalscrollposition sethorizontalscrollposition;
	psy_ui_fp_component_showverticalscrollbar showverticalscrollbar;
	psy_ui_fp_component_hideverticalscrollbar hideverticalscrollbar;
	psy_ui_fp_component_setverticalscrollrange setverticalscrollrange;
	psy_ui_fp_component_verticalscrollrange verticalscrollrange;
	psy_ui_fp_component_verticalscrollposition verticalscrollposition;
	psy_ui_fp_component_setverticalscrollposition setverticalscrollposition;
	psy_ui_fp_component_children children;
	// events
	psy_ui_fp_ondraw ondraw;
	psy_ui_fp_onsize onsize;
	psy_ui_fp_onalign onalign;
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

typedef void* psy_ui_ComponentDetails;

struct psy_ui_ComponentImp;

typedef struct psy_ui_Component {
	psy_ui_ComponentVtable* vtable;		
	struct psy_ui_ComponentImp* imp;
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
	psy_Signal signal_preferredsizechanged;
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
	int visible;	
	int accumwheeldelta;
	int wheelscroll;
	int handlevscroll;
	int handlehscroll;
	int cursor;	
	int mousetracking;
	psy_ui_Style style;
} psy_ui_Component;

void psy_ui_replacedefaultfont(psy_ui_Component* main, psy_ui_Font*);

void psy_ui_component_init(psy_ui_Component*, psy_ui_Component* parent);
void psy_ui_component_init_imp(psy_ui_Component*, psy_ui_Component* parent, struct psy_ui_ComponentImp*);
void psy_ui_component_dispose(psy_ui_Component*);
void psy_ui_component_destroy(psy_ui_Component*);
/*int psy_ui_win32_component_init(psy_ui_Component*, psy_ui_Component* parent,
	const char* classname, int x, int y, int width, int height,
	uint32_t dwStyle, int usecommand);*/

INLINE void psy_ui_component_show(psy_ui_Component* self)
{
	self->vtable->show(self);
}

INLINE void psy_ui_component_hide(psy_ui_Component* self)
{
	self->vtable->hide(self);
}

void psy_ui_component_showstate(psy_ui_Component*, int cmd);
void psy_ui_component_showhorizontalscrollbar(psy_ui_Component*);
void psy_ui_component_hidehorizontalscrollbar(psy_ui_Component*);
void psy_ui_component_sethorizontalscrollrange(psy_ui_Component*, int min, int max);
void psy_ui_component_horizontalscrollrange(psy_ui_Component*, int* scrollmin,
	int* scrollmax);
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
psy_List* psy_ui_component_children(psy_ui_Component*, int recursive);
psy_ui_Size psy_ui_component_frame_size(psy_ui_Component*);

void psy_ui_component_setfont(psy_ui_Component*, psy_ui_Font*);
psy_ui_Font* psy_ui_component_font(psy_ui_Component*);
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
psy_ui_Size psy_ui_component_preferredsize(psy_ui_Component*, psy_ui_Size* limit);
void psy_ui_component_seticonressource(psy_ui_Component*, int ressourceid);
void psy_ui_component_doublebuffer(psy_ui_Component*);
void psy_ui_component_setcursor(psy_ui_Component*, psy_ui_CursorStyle);

psy_List* psy_ui_components_setalign(psy_List*, psy_ui_AlignType,
	const psy_ui_Margin*);
psy_List* psy_ui_components_setmargin(psy_List*, const psy_ui_Margin*);

// uicomponentimp

// vtable function pointers
typedef void (*psy_ui_fp_componentimp_dev_dispose)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_destroy)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_show)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_showstate)(struct psy_ui_ComponentImp*, int state);
typedef void (*psy_ui_fp_componentimp_dev_hide)(struct psy_ui_ComponentImp*);
typedef int (*psy_ui_fp_componentimp_dev_visible)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_move)(struct psy_ui_ComponentImp*, int left, int top);
typedef void (*psy_ui_fp_componentimp_dev_resize)(struct psy_ui_ComponentImp*, int width, int height);
typedef void (*psy_ui_fp_componentimp_dev_clientresize)(struct psy_ui_ComponentImp*, int width, int height);
typedef psy_ui_Rectangle (*psy_ui_fp_componentimp_dev_position)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setposition)(struct psy_ui_ComponentImp*, int x, int y, int width, int height);
typedef psy_ui_Size (*psy_ui_fp_componentimp_dev_size)(struct psy_ui_ComponentImp*);
typedef psy_ui_Size (*psy_ui_fp_componentimp_dev_framesize)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_scrollto)(struct psy_ui_ComponentImp*, intptr_t dx, intptr_t dy);
typedef struct psy_ui_Component* (*psy_ui_fp_componentimp_dev_parent)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setparent)(struct psy_ui_ComponentImp*, struct psy_ui_Component*);
typedef void (*psy_ui_fp_componentimp_dev_insert)(struct psy_ui_ComponentImp*, struct psy_ui_ComponentImp*, struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_capture)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_releasecapture)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_invalidate)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_invalidaterect)(struct psy_ui_ComponentImp*, const psy_ui_Rectangle*);
typedef void (*psy_ui_fp_componentimp_dev_update)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setfont)(struct psy_ui_ComponentImp*, psy_ui_Font*);
typedef void (*psy_ui_fp_componentimp_dev_showhorizontalscrollbar)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_hidehorizontalscrollbar)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_sethorizontalscrollrange)(struct psy_ui_ComponentImp*, int min, int max);
typedef void (*psy_ui_fp_componentimp_dev_horizontalscrollrange)(struct psy_ui_ComponentImp*, int* scrollmin,
	int* scrollmax);
typedef int (*psy_ui_fp_componentimp_dev_horizontalscrollposition)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_sethorizontalscrollposition)(struct psy_ui_ComponentImp*, int position);
typedef void (*psy_ui_fp_componentimp_dev_showverticalscrollbar)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_hideverticalscrollbar)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setverticalscrollrange)(struct psy_ui_ComponentImp*, int min, int max);
typedef void (*psy_ui_fp_componentimp_dev_verticalscrollrange)(struct psy_ui_ComponentImp*, int* scrollmin,
	int* scrollmax);
typedef int (*psy_ui_fp_componentimp_dev_verticalscrollposition)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setverticalscrollposition)(struct psy_ui_ComponentImp*, int position);
typedef psy_List* (*psy_ui_fp_componentimp_dev_children)(struct psy_ui_ComponentImp*, int recursive);
typedef void (*psy_ui_fp_componentimp_dev_enableinput)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_preventinput)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setcursor)(struct psy_ui_ComponentImp*, psy_ui_CursorStyle);
typedef void (*psy_ui_fp_componentimp_dev_starttimer)(struct psy_ui_ComponentImp*, unsigned int id,
	unsigned int interval);
typedef void (*psy_ui_fp_componentimp_dev_stoptimer)(struct psy_ui_ComponentImp*, unsigned int id);
typedef void (*psy_ui_fp_componentimp_dev_seticonressource)(struct psy_ui_ComponentImp*, int ressourceid);
typedef psy_ui_TextMetric (*psy_ui_fp_componentimp_dev_textmetric)(struct psy_ui_ComponentImp*, psy_ui_Font* font);
typedef psy_ui_Size (*psy_ui_fp_componentimp_dev_textsize)(struct psy_ui_ComponentImp*, const char* text, psy_ui_Font*);
typedef void (*psy_ui_fp_componentimp_dev_setbackgroundcolor)(struct psy_ui_ComponentImp*, uint32_t color);
typedef void (*psy_ui_fp_componentimp_dev_settitle)(struct psy_ui_ComponentImp*, const char* title);
typedef void (*psy_ui_fp_componentimp_dev_setfocus)(struct psy_ui_ComponentImp*);
typedef int (*psy_ui_fp_componentimp_dev_hasfocus)(struct psy_ui_ComponentImp*);
typedef void* (*psy_ui_fp_componentimp_dev_platform)(struct psy_ui_ComponentImp*);

typedef struct {	
	psy_ui_fp_componentimp_dev_dispose dev_dispose;
	psy_ui_fp_componentimp_dev_destroy dev_destroy;
	psy_ui_fp_componentimp_dev_show dev_show;
	psy_ui_fp_componentimp_dev_showstate dev_showstate;
	psy_ui_fp_componentimp_dev_hide dev_hide;
	psy_ui_fp_componentimp_dev_visible dev_visible;
	psy_ui_fp_componentimp_dev_move dev_move;
	psy_ui_fp_componentimp_dev_resize dev_resize;
	psy_ui_fp_componentimp_dev_resize dev_clientresize;
	psy_ui_fp_componentimp_dev_position dev_position;
	psy_ui_fp_componentimp_dev_setposition dev_setposition;
	psy_ui_fp_componentimp_dev_size dev_size;
	psy_ui_fp_componentimp_dev_framesize dev_framesize;
	psy_ui_fp_componentimp_dev_scrollto dev_scrollto;
	psy_ui_fp_componentimp_dev_parent dev_parent;
	psy_ui_fp_componentimp_dev_setparent dev_setparent;
	psy_ui_fp_componentimp_dev_insert dev_insert;
	psy_ui_fp_componentimp_dev_capture dev_capture;
	psy_ui_fp_componentimp_dev_releasecapture dev_releasecapture;
	psy_ui_fp_componentimp_dev_invalidate dev_invalidate;	
	psy_ui_fp_componentimp_dev_invalidaterect dev_invalidaterect;
	psy_ui_fp_componentimp_dev_update dev_update;
	psy_ui_fp_componentimp_dev_setfont dev_setfont;
	psy_ui_fp_componentimp_dev_showhorizontalscrollbar dev_showhorizontalscrollbar;
	psy_ui_fp_componentimp_dev_hidehorizontalscrollbar dev_hidehorizontalscrollbar;
	psy_ui_fp_componentimp_dev_sethorizontalscrollrange dev_sethorizontalscrollrange;
	psy_ui_fp_componentimp_dev_horizontalscrollposition dev_horizontalscrollposition;
	psy_ui_fp_componentimp_dev_sethorizontalscrollposition dev_sethorizontalscrollposition;
	psy_ui_fp_componentimp_dev_horizontalscrollrange dev_horizontalscrollrange;
	psy_ui_fp_componentimp_dev_showverticalscrollbar dev_showverticalscrollbar;
	psy_ui_fp_componentimp_dev_hideverticalscrollbar dev_hideverticalscrollbar;
	psy_ui_fp_componentimp_dev_setverticalscrollrange dev_setverticalscrollrange;
	psy_ui_fp_componentimp_dev_verticalscrollrange dev_verticalscrollrange;
	psy_ui_fp_componentimp_dev_verticalscrollposition dev_verticalscrollposition;
	psy_ui_fp_componentimp_dev_setverticalscrollposition dev_setverticalscrollposition;
	psy_ui_fp_componentimp_dev_children dev_children;
	psy_ui_fp_componentimp_dev_enableinput dev_enableinput;
	psy_ui_fp_componentimp_dev_preventinput dev_preventinput;
	psy_ui_fp_componentimp_dev_setcursor dev_setcursor;
	psy_ui_fp_componentimp_dev_starttimer dev_starttimer;
	psy_ui_fp_componentimp_dev_stoptimer dev_stoptimer;
	psy_ui_fp_componentimp_dev_seticonressource dev_seticonressource;
	psy_ui_fp_componentimp_dev_textmetric dev_textmetric;
	psy_ui_fp_componentimp_dev_textsize dev_textsize;
	psy_ui_fp_componentimp_dev_setbackgroundcolor dev_setbackgroundcolor;
	psy_ui_fp_componentimp_dev_settitle dev_settitle;
	psy_ui_fp_componentimp_dev_setfocus dev_setfocus;
	psy_ui_fp_componentimp_dev_hasfocus dev_hasfocus;
	psy_ui_fp_componentimp_dev_platform dev_platform;
} psy_ui_ComponentImpVTable;

typedef struct psy_ui_ComponentImp {
	psy_ui_ComponentImpVTable* vtable;
	psy_Signal signal_command;
} psy_ui_ComponentImp;

void psy_ui_componentimp_init(psy_ui_ComponentImp*);
void psy_ui_componentimp_dispose(psy_ui_ComponentImp*);

INLINE void psy_ui_component_invalidate(psy_ui_Component* self)
{
	self->imp->vtable->dev_invalidate(self->imp);
}

INLINE void psy_ui_component_invalidaterect(psy_ui_Component* self, const psy_ui_Rectangle* r)
{
	self->imp->vtable->dev_invalidaterect(self->imp, r);
}

INLINE void psy_ui_component_update(psy_ui_Component* self)
{
	self->imp->vtable->dev_update(self->imp);
}

INLINE psy_ui_Size psy_ui_component_size(psy_ui_Component* self)
{
	return self->imp->vtable->dev_size(self->imp);
}

INLINE psy_ui_Rectangle psy_ui_component_position(psy_ui_Component* self)
{
	return self->imp->vtable->dev_position(self->imp);
}

INLINE void psy_ui_component_starttimer(psy_ui_Component* self, unsigned int id,
	unsigned int interval)
{
	self->imp->vtable->dev_starttimer(self->imp, id, interval);
}

INLINE void psy_ui_component_stoptimer(psy_ui_Component* self, unsigned int id)
{
	self->imp->vtable->dev_stoptimer(self->imp, id);
}

INLINE psy_ui_TextMetric psy_ui_component_textmetric(psy_ui_Component* self)
{
	return self->imp->vtable->dev_textmetric(self->imp, 
		psy_ui_component_font(self));
}

INLINE psy_ui_Size psy_ui_component_textsize(psy_ui_Component* self, const char* text)
{
	return self->imp->vtable->dev_textsize(self->imp, text,
		psy_ui_component_font(self));
}

INLINE void psy_ui_component_setcolor(psy_ui_Component* self, uint32_t color)
{
	self->color = color;
}

INLINE void psy_ui_component_setbackgroundcolor(psy_ui_Component* self, uint32_t color)
{
	self->backgroundcolor = color;
	self->imp->vtable->dev_setbackgroundcolor(self->imp, color);
}

INLINE void psy_ui_component_settitle(psy_ui_Component* self, const char* text)
{	
	self->imp->vtable->dev_settitle(self->imp, text);
}

INLINE void psy_ui_component_capture(psy_ui_Component* self)
{
	self->imp->vtable->dev_capture(self->imp);
}

INLINE void psy_ui_component_releasecapture(psy_ui_Component* self)
{
	self->imp->vtable->dev_releasecapture(self->imp);
}

INLINE void psy_ui_component_setfocus(psy_ui_Component* self)
{
	self->imp->vtable->dev_setfocus(self->imp);
}

INLINE int psy_ui_component_hasfocus(psy_ui_Component* self)
{
	return self->imp->vtable->dev_hasfocus(self->imp);
}

INLINE psy_ui_Component* psy_ui_component_parent(psy_ui_Component* self)
{
	return self->imp->vtable->dev_parent(self->imp);
}

INLINE void psy_ui_component_setparent(psy_ui_Component* self, psy_ui_Component* parent)
{
	self->imp->vtable->dev_setparent(self->imp, parent);
}

INLINE void psy_ui_component_insert(psy_ui_Component* self, psy_ui_Component* child, psy_ui_Component* insertafter)
{
	self->imp->vtable->dev_insert(self->imp, child->imp, insertafter->imp);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMPONENT_H */

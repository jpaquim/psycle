// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_COMPONENT_H
#define psy_ui_COMPONENT_H

#include "uidef.h"
#include "uievents.h"
#include "uigraphics.h"
#include <signal.h>
#include "uimenu.h"
#include "uistyle.h"
#include "uidefaults.h"
#include <list.h>
#include <translator.h>
#include "../../detail/stdint.h"
#include <math.h>

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
//     imp->dev_invalidate                ^
//     ...                                |
//                             psy_ui_win_ComponentImp

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	psy_ui_BACKGROUND_NONE,
	psy_ui_BACKGROUND_SET,
	psy_ui_BACKGROUND_PARENT,
} psy_ui_BackgroundMode;

struct psy_ui_Component;

// vtable function pointers
typedef void (*psy_ui_fp_component_dispose)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_destroy)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_show)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_showstate)(struct psy_ui_Component*, int state);
typedef void (*psy_ui_fp_component_hide)(struct psy_ui_Component*);
typedef int (*psy_ui_fp_component_visible)(struct psy_ui_Component*);
typedef int (*psy_ui_fp_component_drawvisible)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_move)(struct psy_ui_Component*, psy_ui_Point);
typedef void (*psy_ui_fp_component_resize)(struct psy_ui_Component*, psy_ui_Size);
typedef void (*psy_ui_fp_component_clientresize)(struct psy_ui_Component*, psy_ui_Size);
typedef void (*psy_ui_fp_component_setposition)(struct psy_ui_Component*, psy_ui_Point, psy_ui_Size);
typedef psy_ui_Size (*psy_ui_fp_component_framesize)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_scrollto)(struct psy_ui_Component*, intptr_t dx, intptr_t dy);
typedef void (*psy_ui_fp_component_setfont)(struct psy_ui_Component*, psy_ui_Font*);
typedef void (*psy_ui_fp_component_sethorizontalscrollrange)(struct psy_ui_Component*, int min, int max);
typedef void (*psy_ui_fp_component_horizontalscrollrange)(struct psy_ui_Component* self, int* scrollmin,
	int* scrollmax);
typedef void (*psy_ui_fp_component_showverticalscrollbar)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_hideverticalscrollbar)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_setverticalscrollrange)(struct psy_ui_Component*, int min, int max);
typedef void (*psy_ui_fp_component_verticalscrollrange)(struct psy_ui_Component* self, int* scrollmin,
	int* scrollmax);
typedef psy_List* (*psy_ui_fp_component_children)(struct psy_ui_Component*, int recursive);
typedef void (*psy_ui_fp_component_enableinput)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_preventinput)(struct psy_ui_Component*);
// vtable events function pointers 
typedef void (*psy_ui_fp_component_ondestroy)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_ondestroyed)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_onalign)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_onpreferredsize)(struct psy_ui_Component*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
typedef void (*psy_ui_fp_component_ondraw)(struct psy_ui_Component*, psy_ui_Graphics*);
typedef void (*psy_ui_fp_component_onsize)(struct psy_ui_Component*, const psy_ui_Size*);
typedef bool (*psy_ui_fp_component_onclose)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_onmousedown)(struct psy_ui_Component*, psy_ui_MouseEvent*);
typedef void (*psy_ui_fp_component_onmousemove)(struct psy_ui_Component*, psy_ui_MouseEvent*);
typedef void (*psy_ui_fp_component_onmousewheel)(struct psy_ui_Component*, psy_ui_MouseEvent*);
typedef void (*psy_ui_fp_component_onmouseup)(struct psy_ui_Component*, psy_ui_MouseEvent*);
typedef void (*psy_ui_fp_component_onmousedoubleclick)(struct psy_ui_Component*, psy_ui_MouseEvent*);
typedef void (*psy_ui_fp_component_onmouseenter)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_onmouseleave)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_onkeydown)(struct psy_ui_Component*, psy_ui_KeyEvent*);
typedef void (*psy_ui_fp_component_onkeyup)(struct psy_ui_Component*, psy_ui_KeyEvent*);
typedef void (*psy_ui_fp_component_ontimer)(struct psy_ui_Component*, uintptr_t);
typedef void (*psy_ui_fp_component_onlanguagechanged)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_onfocus)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_onfocuslost)(struct psy_ui_Component*);

typedef struct psy_ui_ComponentVTable {
	psy_ui_fp_component_dispose dispose;
	psy_ui_fp_component_destroy destroy;
	psy_ui_fp_component_show show;
	psy_ui_fp_component_showstate showstate;
	psy_ui_fp_component_hide hide;
	psy_ui_fp_component_visible visible;
	psy_ui_fp_component_visible drawvisible;
	psy_ui_fp_component_move move;
	psy_ui_fp_component_resize resize;
	psy_ui_fp_component_clientresize clientresize;	
	psy_ui_fp_component_setposition setposition;
	psy_ui_fp_component_framesize framesize;
	psy_ui_fp_component_scrollto scrollto;
	psy_ui_fp_component_setfont setfont;
	psy_ui_fp_component_children children;
	psy_ui_fp_component_enableinput enableinput;
	psy_ui_fp_component_preventinput preventinput;
	// events
	psy_ui_fp_component_ondestroy ondestroy;
	psy_ui_fp_component_ondestroyed ondestroyed;
	psy_ui_fp_component_ondraw ondraw;
	psy_ui_fp_component_onsize onsize;
	psy_ui_fp_component_onclose onclose;
	psy_ui_fp_component_onalign onalign;
	psy_ui_fp_component_onpreferredsize onpreferredsize;
	psy_ui_fp_component_onmousedown onmousedown;
	psy_ui_fp_component_onmousemove onmousemove;
	psy_ui_fp_component_onmousewheel onmousewheel;
	psy_ui_fp_component_onmouseup onmouseup;
	psy_ui_fp_component_onmousedoubleclick onmousedoubleclick;
	psy_ui_fp_component_onmouseenter onmouseenter;
	psy_ui_fp_component_onmouseleave onmouseleave;
	psy_ui_fp_component_onkeydown onkeydown;
	psy_ui_fp_component_onkeydown onkeyup;
	psy_ui_fp_component_ontimer ontimer;
	psy_ui_fp_component_onlanguagechanged onlanguagechanged;
	psy_ui_fp_component_onfocus onfocus;
	psy_ui_fp_component_onfocus onfocuslost;
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
	psy_Signal signal_close;
	psy_Signal signal_destroy;
	psy_Signal signal_destroyed;
	psy_Signal signal_childenum;
	psy_Signal signal_show;
	psy_Signal signal_hide;	
	psy_Signal signal_align;
	psy_Signal signal_selectsection;
	//psy_Signal signal_preferredsize;
	psy_Signal signal_command;
	psy_Signal signal_focuslost;
	psy_Signal signal_focus;	
	psy_Signal signal_preferredsizechanged;
	psy_Signal signal_scrollrangechanged;
	psy_Signal signal_languagechanged;
	psy_ui_AlignType align;
	psy_ui_JustifyType justify;
	int alignexpandmode;
	int alignchildren;
	psy_ui_Margin margin;
	psy_ui_Margin spacing;	
	bool doublebuffered;	
	bool preventdefault;
	bool preventpreferredsize;
	psy_ui_Value scrollstepx;
	psy_ui_Value scrollstepy;
	intptr_t debugflag;
	psy_ui_BackgroundMode backgroundmode;
	bool visible;	
	int accumwheeldelta;
	int wheelscroll;
	bool handlevscroll;
	bool handlehscroll;
	int cursor;	
	bool mousetracking;
	psy_ui_Style style;
	psy_ui_Size preferredsize;
	psy_ui_Size minsize;
	psy_ui_Size maxsize;
	psy_ui_Point scroll;	
	psy_ui_IntPoint vscrollrange;
	psy_ui_IntPoint hscrollrange;
	psy_ui_Overflow overflow;
	intptr_t tabindex;
	intptr_t preventpreferredsizeatalign;
	psy_ui_AlignType insertaligntype;
	psy_ui_Margin insertmargin;	
} psy_ui_Component;

void psy_ui_replacedefaultfont(psy_ui_Component* main, psy_ui_Font*);
void psy_ui_updatealign(psy_ui_Component* main, psy_List* children);

void psy_ui_component_init(psy_ui_Component*, psy_ui_Component* parent);
void psy_ui_component_init_imp(psy_ui_Component*, psy_ui_Component* parent,
	struct psy_ui_ComponentImp*);

void psy_ui_component_dispose(psy_ui_Component*);
void psy_ui_component_destroy(psy_ui_Component*);
void psy_ui_component_deallocate(psy_ui_Component*);


INLINE void psy_ui_component_setvtable(psy_ui_Component* self,
	psy_ui_ComponentVtable* vtable)
{
	self->vtable = vtable;
}

INLINE void psy_ui_component_show(psy_ui_Component* self)
{
	self->vtable->show(self);
}

void psy_ui_component_show_align(psy_ui_Component* self);

INLINE void psy_ui_component_hide(psy_ui_Component* self)
{
	self->visible = 0;
	self->vtable->hide(self);
}

void psy_ui_component_hide_align(psy_ui_Component* self);

void psy_ui_component_showstate(psy_ui_Component*, int cmd);
void psy_ui_component_togglevisibility(psy_ui_Component*);

INLINE void psy_ui_component_horizontalscrollrange(psy_ui_Component* self, intptr_t* scrollmin,
	intptr_t* scrollmax)
{
	*scrollmin = self->hscrollrange.x;
	*scrollmax = self->hscrollrange.y;
}

void psy_ui_component_showverticalscrollbar(psy_ui_Component*);
void psy_ui_component_hideverticalscrollbar(psy_ui_Component*);

INLINE void psy_ui_component_setverticalscrollrange(psy_ui_Component* self, intptr_t minval, intptr_t maxval)
{
	self->vscrollrange.x = minval;
	self->vscrollrange.y = psy_max(minval, maxval);
	psy_signal_emit(&self->signal_scrollrangechanged, self, 1, psy_ui_VERTICAL);	
}

INLINE void psy_ui_component_sethorizontalscrollrange(psy_ui_Component* self, intptr_t minval, intptr_t maxval)
{	
	self->hscrollrange.x = minval;
	self->hscrollrange.y = psy_max(minval, maxval);	
	psy_signal_emit(&self->signal_scrollrangechanged, self, 1, psy_ui_HORIZONTAL);	
}

INLINE void psy_ui_component_verticalscrollrange(psy_ui_Component* self, intptr_t* scrollmin,
	intptr_t* scrollmax)
{
	*scrollmin = self->vscrollrange.x;
	*scrollmax = self->vscrollrange.y;
}

void psy_ui_component_scrollstep(psy_ui_Component*, double stepx, double stepy);
void psy_ui_component_move(psy_ui_Component*, psy_ui_Point origin);
void psy_ui_component_resize(psy_ui_Component*, psy_ui_Size);
void psy_ui_component_clientresize(psy_ui_Component*, psy_ui_Size);
void psy_ui_component_setposition(psy_ui_Component*, psy_ui_Point,
	psy_ui_Size);
psy_List* psy_ui_component_children(psy_ui_Component*, int recursive);
psy_ui_Size psy_ui_component_frame_size(psy_ui_Component*);
psy_ui_Component* psy_ui_component_at(psy_ui_Component*, uintptr_t index);
void psy_ui_component_setfont(psy_ui_Component*, psy_ui_Font*);
psy_ui_Font* psy_ui_component_font(psy_ui_Component*);
void psy_ui_component_preventdefault(psy_ui_Component*);
void psy_ui_component_init_base(psy_ui_Component*);
void psy_ui_component_init_signals(psy_ui_Component*);
int psy_ui_component_visible(psy_ui_Component*);
int psy_ui_component_drawvisible(psy_ui_Component*);
void psy_ui_component_align(psy_ui_Component*);
void psy_ui_component_alignall(psy_ui_Component*);
void psy_ui_component_setmargin(psy_ui_Component*, const psy_ui_Margin*);
void psy_ui_component_setspacing(psy_ui_Component*, const psy_ui_Margin*);

INLINE void psy_ui_component_setborder(psy_ui_Component* self, psy_ui_Border border)
{
	self->style.border = border;
	self->style.border.mode.set = TRUE;
}

psy_ui_Border psy_ui_component_border(psy_ui_Component*);
void psy_ui_component_setalign(psy_ui_Component*, psy_ui_AlignType align);
INLINE void psy_ui_component_init_align(psy_ui_Component* self,
	psy_ui_Component* parent, psy_ui_AlignType aligntype)
{
	psy_ui_component_init(self, parent);
	psy_ui_component_setalign(self, aligntype);
}
void psy_ui_component_enablealign(psy_ui_Component*);
void psy_ui_component_preventalign(psy_ui_Component*);
void psy_ui_component_setalignexpand(psy_ui_Component*, psy_ui_ExpandMode);
void psy_ui_component_enableinput(psy_ui_Component*, int recursive);
void psy_ui_component_preventinput(psy_ui_Component*, int recursive);
void psy_ui_component_setbackgroundmode(psy_ui_Component*, psy_ui_BackgroundMode);
void psy_ui_component_setpreferredsize(psy_ui_Component*, psy_ui_Size size);
psy_ui_Size psy_ui_component_preferredsize(psy_ui_Component*, const psy_ui_Size* limit);
void psy_ui_component_setmaximumsize(psy_ui_Component*, psy_ui_Size size);
psy_ui_Size psy_ui_component_maximumsize(psy_ui_Component*);
void psy_ui_component_setminimumsize(psy_ui_Component*, psy_ui_Size size);
psy_ui_Size psy_ui_component_minimumsize(psy_ui_Component*);
void psy_ui_component_preventpreferredsize(psy_ui_Component*);
void psy_ui_component_enablepreferredsize(psy_ui_Component*);
void psy_ui_component_seticonressource(psy_ui_Component*, int ressourceid);
void psy_ui_component_doublebuffer(psy_ui_Component*);
void psy_ui_component_setcursor(psy_ui_Component*, psy_ui_CursorStyle);

psy_List* psy_ui_components_setalign(psy_List*, psy_ui_AlignType,
	const psy_ui_Margin*);
psy_List* psy_ui_components_setmargin(psy_List*, const psy_ui_Margin*);
void psy_ui_component_close(psy_ui_Component*);

// psy_ui_ComponentImp
// vtable function pointers
typedef void (*psy_ui_fp_componentimp_dev_dispose)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_destroy)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_show)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_showstate)(struct psy_ui_ComponentImp*, int state);
typedef void (*psy_ui_fp_componentimp_dev_hide)(struct psy_ui_ComponentImp*);
typedef int (*psy_ui_fp_componentimp_dev_visible)(struct psy_ui_ComponentImp*);
typedef int (*psy_ui_fp_componentimp_dev_drawvisible)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_move)(struct psy_ui_ComponentImp*, psy_ui_Point origin);
typedef void (*psy_ui_fp_componentimp_dev_resize)(struct psy_ui_ComponentImp*, psy_ui_Size);
typedef void (*psy_ui_fp_componentimp_dev_clientresize)(struct psy_ui_ComponentImp*, intptr_t width, intptr_t height);
typedef psy_ui_RealRectangle (*psy_ui_fp_componentimp_dev_position)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setposition)(struct psy_ui_ComponentImp*, psy_ui_Point, psy_ui_Size);
typedef psy_ui_Size (*psy_ui_fp_componentimp_dev_size)(const struct psy_ui_ComponentImp*);
typedef psy_ui_Size(*psy_ui_fp_componentimp_dev_preferredsize)(struct psy_ui_ComponentImp*, const psy_ui_Size* limits);
typedef psy_ui_Size (*psy_ui_fp_componentimp_dev_framesize)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_updatesize)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_scrollto)(struct psy_ui_ComponentImp*, intptr_t dx, intptr_t dy);
typedef struct psy_ui_Component* (*psy_ui_fp_componentimp_dev_parent)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setparent)(struct psy_ui_ComponentImp*, struct psy_ui_Component*);
typedef void (*psy_ui_fp_componentimp_dev_insert)(struct psy_ui_ComponentImp*, struct psy_ui_ComponentImp*, struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setorder)(struct psy_ui_ComponentImp*, struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_capture)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_releasecapture)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_invalidate)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_invalidaterect)(struct psy_ui_ComponentImp*, const psy_ui_RealRectangle*);
typedef void (*psy_ui_fp_componentimp_dev_update)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setfont)(struct psy_ui_ComponentImp*, psy_ui_Font*);
typedef psy_List* (*psy_ui_fp_componentimp_dev_children)(struct psy_ui_ComponentImp*, int recursive);
typedef void (*psy_ui_fp_componentimp_dev_enableinput)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_preventinput)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setcursor)(struct psy_ui_ComponentImp*, psy_ui_CursorStyle);
typedef void (*psy_ui_fp_componentimp_dev_starttimer)(struct psy_ui_ComponentImp*, uintptr_t id,
	uintptr_t interval);
typedef void (*psy_ui_fp_componentimp_dev_stoptimer)(struct psy_ui_ComponentImp*, uintptr_t id);
typedef void (*psy_ui_fp_componentimp_dev_seticonressource)(struct psy_ui_ComponentImp*, int ressourceid);
typedef const psy_ui_TextMetric* (*psy_ui_fp_componentimp_dev_textmetric)(const struct psy_ui_ComponentImp*);
typedef psy_ui_Size (*psy_ui_fp_componentimp_dev_textsize)(struct psy_ui_ComponentImp*, const char* text, psy_ui_Font*);
typedef void (*psy_ui_fp_componentimp_dev_setbackgroundcolour)(struct psy_ui_ComponentImp*, psy_ui_Colour);
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
	psy_ui_fp_componentimp_dev_drawvisible dev_drawvisible;
	psy_ui_fp_componentimp_dev_move dev_move;
	psy_ui_fp_componentimp_dev_resize dev_resize;
	psy_ui_fp_componentimp_dev_clientresize dev_clientresize;
	psy_ui_fp_componentimp_dev_position dev_position;
	psy_ui_fp_componentimp_dev_setposition dev_setposition;
	psy_ui_fp_componentimp_dev_size dev_size;
	psy_ui_fp_componentimp_dev_preferredsize dev_preferredsize;
	psy_ui_fp_componentimp_dev_updatesize dev_updatesize;
	psy_ui_fp_componentimp_dev_framesize dev_framesize;
	psy_ui_fp_componentimp_dev_scrollto dev_scrollto;
	psy_ui_fp_componentimp_dev_parent dev_parent;
	psy_ui_fp_componentimp_dev_setparent dev_setparent;
	psy_ui_fp_componentimp_dev_insert dev_insert;
	psy_ui_fp_componentimp_dev_setorder dev_setorder;
	psy_ui_fp_componentimp_dev_capture dev_capture;
	psy_ui_fp_componentimp_dev_releasecapture dev_releasecapture;
	psy_ui_fp_componentimp_dev_invalidate dev_invalidate;	
	psy_ui_fp_componentimp_dev_invalidaterect dev_invalidaterect;
	psy_ui_fp_componentimp_dev_update dev_update;
	psy_ui_fp_componentimp_dev_setfont dev_setfont;
	psy_ui_fp_componentimp_dev_children dev_children;
	psy_ui_fp_componentimp_dev_enableinput dev_enableinput;
	psy_ui_fp_componentimp_dev_preventinput dev_preventinput;
	psy_ui_fp_componentimp_dev_setcursor dev_setcursor;
	psy_ui_fp_componentimp_dev_starttimer dev_starttimer;
	psy_ui_fp_componentimp_dev_stoptimer dev_stoptimer;
	psy_ui_fp_componentimp_dev_seticonressource dev_seticonressource;
	psy_ui_fp_componentimp_dev_textmetric dev_textmetric;
	psy_ui_fp_componentimp_dev_textsize dev_textsize;
	psy_ui_fp_componentimp_dev_setbackgroundcolour dev_setbackgroundcolour;
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

INLINE void psy_ui_component_invalidaterect(psy_ui_Component* self, psy_ui_RealRectangle r)
{
	self->imp->vtable->dev_invalidaterect(self->imp, &r);
}

INLINE void psy_ui_component_update(psy_ui_Component* self)
{
	self->imp->vtable->dev_update(self->imp);
}

INLINE psy_ui_Size psy_ui_component_size(const psy_ui_Component* self)
{	
	return self->imp->vtable->dev_size(self->imp);	
}

INLINE psy_ui_RealRectangle psy_ui_component_position(psy_ui_Component* self)
{
	return self->imp->vtable->dev_position(self->imp);
}

psy_ui_RealRectangle psy_ui_component_scrolledposition(psy_ui_Component*);

INLINE void psy_ui_component_starttimer(psy_ui_Component* self, uintptr_t id,
	uintptr_t interval)
{
	self->imp->vtable->dev_starttimer(self->imp, id, interval);
}

INLINE void psy_ui_component_stoptimer(psy_ui_Component* self, uintptr_t id)
{
	self->imp->vtable->dev_stoptimer(self->imp, id);
}

INLINE const psy_ui_TextMetric* psy_ui_component_textmetric(const psy_ui_Component* self)
{
	assert(self->imp);

	return self->imp->vtable->dev_textmetric(self->imp);
}

INLINE psy_ui_Size psy_ui_component_textsize(psy_ui_Component* self, const char* text)
{
	return self->imp->vtable->dev_textsize(self->imp, text,
		psy_ui_component_font(self));
}

void psy_ui_component_setcolour(psy_ui_Component*, psy_ui_Colour);
psy_ui_Colour psy_ui_component_colour(psy_ui_Component*);
void psy_ui_component_setbackgroundcolour(psy_ui_Component*, psy_ui_Colour);
psy_ui_Colour psy_ui_component_backgroundcolour(psy_ui_Component*);

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

INLINE const psy_ui_Component* psy_ui_component_parent_const(const psy_ui_Component* self)
{
	return ((psy_ui_Component*)self)->imp->vtable->dev_parent(
		((psy_ui_Component*)(self))->imp);
}

INLINE void psy_ui_component_setparent(psy_ui_Component* self, psy_ui_Component* parent)
{
	self->imp->vtable->dev_setparent(self->imp, parent);
}

INLINE void psy_ui_component_insert(psy_ui_Component* self, psy_ui_Component* child,
	psy_ui_Component* insertafter)
{
	self->imp->vtable->dev_insert(self->imp, child->imp, (insertafter) ? insertafter->imp : NULL);
}

INLINE void psy_ui_component_setorder(psy_ui_Component* self, psy_ui_Component* insertafter)
{
	self->imp->vtable->dev_setorder(self->imp, insertafter->imp);
}

INLINE void psy_ui_component_setwheelscroll(psy_ui_Component* self, int lines)
{
	self->wheelscroll = lines;
}

INLINE void psy_ui_component_selectsection(psy_ui_Component* self, uintptr_t section, uintptr_t options)
{
	psy_signal_emit(&self->signal_selectsection, self, 2, section, options);
}

INLINE psy_ui_Point psy_ui_component_scroll(psy_ui_Component* self)
{
	return self->scroll;
}

void psy_ui_component_setscroll(psy_ui_Component*, psy_ui_Point);
void psy_ui_component_setscrollleft(psy_ui_Component*, psy_ui_Value left);

INLINE psy_ui_Value psy_ui_component_scrollleft(psy_ui_Component* self)
{
	return self->scroll.x;
}

INLINE double psy_ui_component_scrollleftpx(psy_ui_Component* self)
{	
	return floor(psy_ui_value_px(&self->scroll.x,
		psy_ui_component_textmetric(self)));
}

void psy_ui_component_setscrolltop(psy_ui_Component*, psy_ui_Value top);

INLINE psy_ui_Value psy_ui_component_scrolltop(psy_ui_Component* self)
{
	return self->scroll.y;
}

INLINE double psy_ui_component_scrolltoppx(psy_ui_Component* self)
{	
	return floor(psy_ui_value_px(&self->scroll.y,
		psy_ui_component_textmetric(self)));
}

void psy_ui_component_updateoverflow(psy_ui_Component*);
void psy_ui_component_drawborder(psy_ui_Component*, psy_ui_Graphics*);

INLINE void psy_ui_component_setoverflow(psy_ui_Component* self, psy_ui_Overflow overflow)
{
	if (self->overflow != overflow) {
		self->overflow = overflow;
		if (overflow == psy_ui_OVERFLOW_HIDDEN) {
			psy_ui_component_sethorizontalscrollrange(self, 0, 0);
			psy_ui_component_setverticalscrollrange(self, 0, 0);
		} else {
			psy_ui_component_updateoverflow(self);
		}		
	}
}

INLINE psy_ui_Overflow psy_ui_component_overflow(psy_ui_Component* self)
{
	assert(self);

	return self->overflow;
}

INLINE void psy_ui_component_setscrollstep(psy_ui_Component* self, psy_ui_Value stepx, psy_ui_Value stepy)
{
	assert(self);

	self->scrollstepx = stepx;
	self->scrollstepy = stepy;
}

INLINE void psy_ui_component_setscrollstepx(psy_ui_Component* self, psy_ui_Value step)
{
	assert(self);

	self->scrollstepx = step;
}

INLINE psy_ui_Value psy_ui_component_scrollstepx(const psy_ui_Component* self)
{
	assert(self);

	return self->scrollstepx;
}

INLINE void psy_ui_component_setscrollstepy(psy_ui_Component* self, psy_ui_Value step)
{
	assert(self);

	self->scrollstepy = step;
}

INLINE psy_ui_Value psy_ui_component_scrollstepy(const psy_ui_Component* self)
{
	assert(self);

	return self->scrollstepy;
}

INLINE void psy_ui_component_settabindex(psy_ui_Component* self, intptr_t index)
{
	assert(self);

	self->tabindex = index;
}

INLINE intptr_t psy_ui_component_tabindex(const psy_ui_Component* self)
{
	assert(self);

	return self->tabindex;
}

void psy_ui_component_focus_next(psy_ui_Component*);
void psy_ui_component_focus_prev(psy_ui_Component*);

INLINE psy_ui_IntSize psy_ui_component_intsize(psy_ui_Component* self)
{	
	return psy_ui_intsize_init_size(psy_ui_component_size(self),
		psy_ui_component_textmetric(self));
}

INLINE psy_ui_RealSize psy_ui_component_sizepx(const psy_ui_Component* self)
{
	const psy_ui_TextMetric* tm;
	psy_ui_Size size;

	tm = psy_ui_component_textmetric(self);
	size = psy_ui_component_size(self);
	return psy_ui_realsize_make(
		psy_ui_value_px(&size.width, tm),
		psy_ui_value_px(&size.height, tm));
		
}

int psy_ui_component_level(const psy_ui_Component* self);

void psy_ui_component_setdefaultalign(psy_ui_Component* self,
	psy_ui_AlignType, psy_ui_Margin margin);

const struct psy_ui_Defaults* psy_ui_defaults(void);

void psy_ui_component_updatelanguage(psy_ui_Component* self);
psy_Translator* psy_ui_translator(void);

const char* psy_ui_translate(const char* key);


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMPONENT_H */

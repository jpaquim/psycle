/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_COMPONENT_H
#define psy_ui_COMPONENT_H

/* local */
#include "uicomponentscroll.h"
#include "uicomponentstyle.h"
#include "uicomponentbackground.h"
#include "uidefaults.h"
#include "uievents.h"
#include "uigraphics.h"
#include "uimenu.h"
#include "uistyles.h"
/* container */
#include <list.h>
#include <signal.h>
#include <translator.h>
/* std */
#include <math.h>
#include <stdio.h>

/*
** psy_ui_Component
**  Bridge
**  Aim: avoid coupling to one platform (win32, xt/motif, etc)
**  Abstraction/Refined  psy_ui_Component
**  Implementor			psy_ui_ComponentImp
**  Concrete Implementor	psy_ui_win_ComponentImp
**
** Backreference imp to component for imp/component mapping and events
**
** psy_ui_Component <>--------<> psy_ui_ComponentImp
**     imp->dev_invalidate                ^
**     ...                                |
**                             psy_ui_win_ComponentImp
*/

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_Component;

typedef struct psy_ui_ComponentContainerAlign {
	int alignexpandmode;
	psy_ui_ContainerAlignType containeralign;
	psy_ui_AlignType insertaligntype;
	psy_ui_Margin insertmargin;
} psy_ui_ComponentContainerAlign;

void psy_ui_componentcontaineralign_init(psy_ui_ComponentContainerAlign*);

psy_ui_ComponentContainerAlign* psy_ui_componentcontaineralign_alloc(void);
psy_ui_ComponentContainerAlign* psy_ui_componentcontaineralign_allocinit(void);
void psy_ui_componentcontaineralign_deallocate(psy_ui_ComponentContainerAlign*);

/* vtable function pointers */
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
typedef void (*psy_ui_fp_component_scrollto)(struct psy_ui_Component*, intptr_t dx, intptr_t dy,
	const psy_ui_RealRectangle*);
typedef void (*psy_ui_fp_component_setfont)(struct psy_ui_Component*, const psy_ui_Font*);
typedef void (*psy_ui_fp_component_sethorizontalscrollrange)(struct psy_ui_Component*, int min, int max);
typedef void (*psy_ui_fp_component_horizontalscrollrange)(struct psy_ui_Component*, int* scrollmin,
	int* scrollmax);
typedef void (*psy_ui_fp_component_showverticalscrollbar)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_hideverticalscrollbar)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_setverticalscrollrange)(struct psy_ui_Component*, int min, int max);
typedef void (*psy_ui_fp_component_verticalscrollrange)(struct psy_ui_Component*, int* scrollmin,
	int* scrollmax);
typedef psy_List* (*psy_ui_fp_component_children)(struct psy_ui_Component*, int recursive);
typedef void (*psy_ui_fp_component_enableinput)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_preventinput)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_invalidate)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_setalign)(struct psy_ui_Component*, psy_ui_AlignType);
/* vtable events function pointers */
typedef void (*psy_ui_fp_component_event)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_focusin)(struct psy_ui_Component*, psy_ui_Event*);
typedef bool (*psy_ui_fp_component_onclose)(struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_onpreferredsize)(struct psy_ui_Component*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
typedef void (*psy_ui_fp_component_onpreferredscrollsize)(struct psy_ui_Component*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
typedef void (*psy_ui_fp_component_ondraw)(struct psy_ui_Component*, psy_ui_Graphics*);
typedef void (*psy_ui_fp_component_onmouseevent)(struct psy_ui_Component*, psy_ui_MouseEvent*);
typedef void (*psy_ui_fp_component_onkeyevent)(struct psy_ui_Component*, psy_ui_KeyboardEvent*);
typedef void (*psy_ui_fp_component_ontimer)(struct psy_ui_Component*, uintptr_t);
typedef void (*psy_ui_fp_component_onlanguagechanged)(struct psy_ui_Component*);
typedef uintptr_t (*psy_ui_fp_component_section)(const struct psy_ui_Component*);
typedef void (*psy_ui_fp_component_ondragstart)(struct psy_ui_Component*, psy_ui_DragEvent*);
typedef void (*psy_ui_fp_component_ondragover)(struct psy_ui_Component*, psy_ui_DragEvent*);
typedef void (*psy_ui_fp_component_ondrop)(struct psy_ui_Component*, psy_ui_DragEvent*);

typedef struct psy_ui_ComponentVTable {
	psy_ui_fp_component_event dispose;
	psy_ui_fp_component_event destroy;
	psy_ui_fp_component_event show;
	psy_ui_fp_component_showstate showstate;
	psy_ui_fp_component_event hide;
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
	psy_ui_fp_component_invalidate invalidate;
	psy_ui_fp_component_section section;
	psy_ui_fp_component_setalign setalign;
	/* events */
	psy_ui_fp_component_event ondestroy;
	psy_ui_fp_component_event ondestroyed;
	psy_ui_fp_component_ondraw ondraw;
	psy_ui_fp_component_event onsize;
	psy_ui_fp_component_onclose onclose;
	psy_ui_fp_component_event beforealign;
	psy_ui_fp_component_event onalign;
	psy_ui_fp_component_onpreferredsize onpreferredsize;
	psy_ui_fp_component_onpreferredscrollsize onpreferredscrollsize;
	psy_ui_fp_component_onmouseevent onmousedown;
	psy_ui_fp_component_onmouseevent onmousemove;
	psy_ui_fp_component_onmouseevent onmousewheel;
	psy_ui_fp_component_onmouseevent onmouseup;
	psy_ui_fp_component_onmouseevent onmousedoubleclick;
	psy_ui_fp_component_event onmouseenter;
	psy_ui_fp_component_event onmouseleave;
	psy_ui_fp_component_onkeyevent onkeydown;
	psy_ui_fp_component_onkeyevent onkeyup;
	psy_ui_fp_component_ontimer ontimer;
	psy_ui_fp_component_onlanguagechanged onlanguagechanged;
	psy_ui_fp_component_event onfocus;
	psy_ui_fp_component_event onfocuslost;
	psy_ui_fp_component_focusin onfocusin;
	psy_ui_fp_component_event onupdatestyles;
	psy_ui_fp_component_ondragstart ondragstart;
	psy_ui_fp_component_ondragover ondragover;
	psy_ui_fp_component_ondrop ondrop;
} psy_ui_ComponentVtable;

typedef void* psy_ui_ComponentDetails;

struct psy_ui_ComponentImp;

/* psy_ui_Component */
typedef struct psy_ui_Component {
	psy_ui_ComponentVtable* vtable;		
	struct psy_ui_ComponentImp* imp;
	struct psy_ui_Component* view;
	/* signals */
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
	psy_Signal signal_childenum;
	psy_Signal signal_show;
	psy_Signal signal_hide;	
	psy_Signal signal_align;
	psy_Signal signal_beforealign;
	psy_Signal signal_selectsection;	
	psy_Signal signal_command;
	psy_Signal signal_focuslost;
	psy_Signal signal_focus;	
	psy_Signal signal_preferredsizechanged;
	psy_Signal signal_scrollrangechanged;
	psy_Signal signal_languagechanged;
	psy_Signal signal_dragstart;
	psy_Signal signal_styleupdate;
	/* internal */
	uintptr_t id;
	bool doublebuffered;	
	psy_ui_ComponentStyle style;
	psy_ui_ComponentScroll* scroll;
	psy_ui_AlignType align;
	psy_ui_AlignType alignsorted;
	psy_ui_ComponentContainerAlign* containeralign;	
	uintptr_t tabindex;
	bool deallocate;
	uintptr_t opcount;	
	bool draggable;		
	bool dropdown;
	psy_ui_Bitmap bufferbitmap;
	bool drawtobuffer;
	bool ncpaint;
	bool blitscroll;
	psy_ui_ComponentBackground componentbackground;
} psy_ui_Component;

void psy_ui_replacedefaultfont(psy_ui_Component* main, psy_ui_Font*);
void psy_ui_notifystyleupdate(psy_ui_Component* main);
void psy_ui_component_updatefont(psy_ui_Component*);

void psy_ui_component_init(psy_ui_Component*, psy_ui_Component* parent,
	psy_ui_Component* view);
void psy_ui_component_init_imp(psy_ui_Component*, psy_ui_Component* parent,
	psy_ui_Component* view, struct psy_ui_ComponentImp*);

void psy_ui_component_dispose(psy_ui_Component*);
void psy_ui_component_destroy(psy_ui_Component*);
void psy_ui_component_deallocate(psy_ui_Component*);

psy_ui_Component* psy_ui_component_alloc(void);
psy_ui_Component* psy_ui_component_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view);
void psy_ui_component_deallocateafterdestroyed(psy_ui_Component*);

INLINE void psy_ui_component_setvtable(psy_ui_Component* self,
	psy_ui_ComponentVtable* vtable)
{
	self->vtable = vtable;
}

void psy_ui_component_show(psy_ui_Component* self);
void psy_ui_component_show_align(psy_ui_Component* self);
void psy_ui_component_hide(psy_ui_Component* self);
void psy_ui_component_hide_align(psy_ui_Component* self);
void psy_ui_component_showstate(psy_ui_Component*, int cmd);
void psy_ui_component_showmaximized(psy_ui_Component*);
void psy_ui_component_togglefullscreen(psy_ui_Component*);
bool psy_ui_component_togglevisibility(psy_ui_Component*);

void psy_ui_component_usescroll(psy_ui_Component*);
void psy_ui_component_usecontaineralign(psy_ui_Component*);

INLINE psy_ui_IntPoint psy_ui_component_horizontalscrollrange(
	const psy_ui_Component* self)
{
	return psy_ui_componentscroll_hrange(self->scroll);
}

void psy_ui_component_showverticalscrollbar(psy_ui_Component*);
void psy_ui_component_hideverticalscrollbar(psy_ui_Component*);

INLINE void psy_ui_component_setverticalscrollrange(psy_ui_Component* self,
	psy_ui_IntPoint range)
{			
	psy_ui_component_usescroll(self);
	range.y = psy_max(range.x, range.y);
	psy_ui_componentscroll_setvrange(self->scroll, range);
	psy_signal_emit(&self->signal_scrollrangechanged, self, 1,
		psy_ui_VERTICAL);
}

INLINE void psy_ui_component_sethorizontalscrollrange(psy_ui_Component* self,
	psy_ui_IntPoint range)
{		
	psy_ui_component_usescroll(self);
	range.y = psy_max(range.x, range.y);
	psy_ui_componentscroll_sethrange(self->scroll, range);
	psy_signal_emit(&self->signal_scrollrangechanged, self, 1,
		psy_ui_HORIZONTAL);
}

INLINE psy_ui_IntPoint psy_ui_component_verticalscrollrange(
	const psy_ui_Component* self)
{
	return psy_ui_componentscroll_vrange(self->scroll);
}

void psy_ui_component_scrollstep(psy_ui_Component*, double stepx, double stepy);
void psy_ui_component_move(psy_ui_Component*, psy_ui_Point origin);
void psy_ui_component_resize(psy_ui_Component*, psy_ui_Size);
void psy_ui_component_clientresize(psy_ui_Component*, psy_ui_Size);
void psy_ui_component_setposition(psy_ui_Component*, psy_ui_Rectangle position);
void psy_ui_component_applyposition(psy_ui_Component*, bool children);
psy_List* psy_ui_component_children(psy_ui_Component*, int recursive);
uintptr_t psy_ui_component_index(psy_ui_Component*);
psy_ui_Size psy_ui_component_frame_size(psy_ui_Component*);
psy_ui_Component* psy_ui_component_at(psy_ui_Component*, uintptr_t index);
psy_ui_Component* psy_ui_component_intersect(psy_ui_Component*, psy_ui_RealPoint,
	uintptr_t* index);
void psy_ui_component_setfont(psy_ui_Component*, const psy_ui_Font*);
void psy_ui_component_setfontinfo(psy_ui_Component*, psy_ui_FontInfo);
const psy_ui_Font* psy_ui_component_font(const psy_ui_Component*);
void psy_ui_component_init_base(psy_ui_Component*);
void psy_ui_component_init_signals(psy_ui_Component*);
int psy_ui_component_visible(psy_ui_Component*);
bool psy_ui_component_drawvisible(psy_ui_Component*);
void psy_ui_component_align(psy_ui_Component*);
void psy_ui_component_align_full(psy_ui_Component*);
void psy_ui_component_align_cached(psy_ui_Component*);

INLINE bool psy_ui_component_hasalign(const psy_ui_Component* self)
{
	return (self->containeralign &&
		self->containeralign->containeralign != psy_ui_CONTAINER_ALIGN_NONE);
}

psy_ui_Component* psy_ui_component_preferredsize_parent(psy_ui_Component*);
psy_ui_Component* psy_ui_component_root(psy_ui_Component*);

INLINE void psy_ui_component_setmargin(psy_ui_Component* self, psy_ui_Margin margin)
{	
	psy_ui_componentstyle_setmargin(&self->style, margin);	
}

INLINE psy_ui_Margin psy_ui_component_margin(const psy_ui_Component* self)
{
	return psy_ui_componentstyle_margin(&self->style);	
}

void psy_ui_component_setmargin_children(psy_ui_Component*,
	psy_ui_Margin);

INLINE void psy_ui_component_setspacing(psy_ui_Component* self, psy_ui_Margin spacing)
{	
	psy_ui_componentstyle_setspacing(&self->style, spacing);	
}

INLINE psy_ui_Margin psy_ui_component_spacing(const psy_ui_Component* self)
{	
	return psy_ui_componentstyle_spacing(&self->style);
}

void psy_ui_component_setpadding_children(psy_ui_Component*, psy_ui_Margin);
void psy_ui_component_setalign_children(psy_ui_Component*, psy_ui_AlignType);
void psy_ui_component_checksortedalign(psy_ui_Component*, psy_ui_AlignType);
const psy_ui_Border* psy_ui_component_border(const psy_ui_Component*);
void psy_ui_component_setborder(psy_ui_Component*, const psy_ui_Border*);
uintptr_t psy_ui_component_backgroundimageid(const psy_ui_Component*);
const char* psy_ui_component_backgroundimagepath(const psy_ui_Component*);

psy_ui_Component* psy_ui_component_setalign(psy_ui_Component*,
	psy_ui_AlignType);

INLINE void psy_ui_component_init_align(psy_ui_Component* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_ui_AlignType aligntype)
{
	psy_ui_component_init(self, parent, view);
	psy_ui_component_setalign(self, aligntype);
}

void psy_ui_component_setcontaineralign(psy_ui_Component*,
	psy_ui_ContainerAlignType);
void psy_ui_component_preventalign(psy_ui_Component*);
void psy_ui_component_setalignexpand(psy_ui_Component*, psy_ui_ExpandMode);
void psy_ui_component_enableinput(psy_ui_Component*, int recursive);
void psy_ui_component_preventinput(psy_ui_Component*, int recursive);
bool psy_ui_component_inputprevented(const psy_ui_Component*);
void psy_ui_component_setbackgroundmode(psy_ui_Component*, psy_ui_BackgroundMode);
void psy_ui_component_setpreferredsize(psy_ui_Component*, psy_ui_Size);
void psy_ui_component_setpreferredheight(psy_ui_Component*, psy_ui_Value);
void psy_ui_component_setpreferredwidth(psy_ui_Component*, psy_ui_Value);
psy_ui_Size psy_ui_component_preferredsize(psy_ui_Component*, const psy_ui_Size* limit);
psy_ui_Size psy_ui_component_preferredscrollsize(psy_ui_Component*, const psy_ui_Size* limit);
psy_ui_RealSize psy_ui_component_preferredscrollsize_px(psy_ui_Component*,
	const psy_ui_Size* limit);
void psy_ui_component_setmaximumsize(psy_ui_Component*, psy_ui_Size);
const psy_ui_Size psy_ui_component_maximumsize(const psy_ui_Component*);
void psy_ui_component_setminimumsize(psy_ui_Component*, psy_ui_Size);
const psy_ui_Size psy_ui_component_minimumsize(const psy_ui_Component*);
void psy_ui_component_seticonressource(psy_ui_Component*, int ressourceid);
void psy_ui_component_doublebuffer(psy_ui_Component*);

INLINE bool psy_ui_component_doublebuffered(const psy_ui_Component* self)
{
	return self->doublebuffered;
}

void psy_ui_component_setcursor(psy_ui_Component*, psy_ui_CursorStyle);

psy_List* psy_ui_components_setalign(psy_List*, psy_ui_AlignType,
	psy_ui_Margin);
psy_List* psy_ui_components_setmargin(psy_List*, psy_ui_Margin);
void psy_ui_component_close(psy_ui_Component*);

void psy_ui_component_setid(psy_ui_Component*, uintptr_t id);
uintptr_t psy_ui_component_id(const psy_ui_Component*);
psy_ui_Component* psy_ui_component_byid(psy_ui_Component*, uintptr_t id);

void* psy_ui_component_platform(psy_ui_Component*);

/* psy_ui_ComponentImp */
/* flags */
typedef enum psy_ui_ComponentImpFlags {
	psy_ui_COMPONENTIMPFLAGS_NONE = 0,
	psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN = 1
} psy_ui_ComponentImpFlags;

/* vtable function pointers */
typedef void (*psy_ui_fp_componentimp_dev_dispose)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_destroy)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_destroyed)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_show)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_showstate)(struct psy_ui_ComponentImp*, int state);
typedef void (*psy_ui_fp_componentimp_dev_hide)(struct psy_ui_ComponentImp*);
typedef int (*psy_ui_fp_componentimp_dev_visible)(struct psy_ui_ComponentImp*);
typedef int (*psy_ui_fp_componentimp_dev_drawvisible)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_move)(struct psy_ui_ComponentImp*, psy_ui_Point origin);
typedef void (*psy_ui_fp_componentimp_dev_resize)(struct psy_ui_ComponentImp*, psy_ui_Size);
typedef void (*psy_ui_fp_componentimp_dev_clientresize)(struct psy_ui_ComponentImp*, intptr_t width, intptr_t height);
typedef psy_ui_RealRectangle (*psy_ui_fp_componentimp_dev_position)(const struct psy_ui_ComponentImp*);
typedef psy_ui_RealRectangle(*psy_ui_fp_componentimp_dev_screenposition)(const struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setposition)(struct psy_ui_ComponentImp*, psy_ui_Point, psy_ui_Size);
typedef psy_ui_Size (*psy_ui_fp_componentimp_dev_size)(const struct psy_ui_ComponentImp*);
typedef psy_ui_Size(*psy_ui_fp_componentimp_dev_preferredsize)(struct psy_ui_ComponentImp*, const psy_ui_Size* limits);
typedef psy_ui_Size (*psy_ui_fp_componentimp_dev_framesize)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_updatesize)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_applyposition)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_scrollto)(struct psy_ui_ComponentImp*, intptr_t dx, intptr_t dy,
	const psy_ui_RealRectangle*);
typedef struct psy_ui_Component* (*psy_ui_fp_componentimp_dev_parent)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setparent)(struct psy_ui_ComponentImp*, struct psy_ui_Component*);
typedef void (*psy_ui_fp_componentimp_dev_insert)(struct psy_ui_ComponentImp*, struct psy_ui_ComponentImp*, struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_remove)(struct psy_ui_ComponentImp*, struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_erase)(struct psy_ui_ComponentImp*, struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setorder)(struct psy_ui_ComponentImp*, struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_capture)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_releasecapture)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_invalidate)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_invalidaterect)(struct psy_ui_ComponentImp*, const psy_ui_RealRectangle*);
typedef void (*psy_ui_fp_componentimp_dev_update)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setfont)(struct psy_ui_ComponentImp*, const psy_ui_Font*);
typedef psy_List* (*psy_ui_fp_componentimp_dev_children)(struct psy_ui_ComponentImp*, int recursive);
typedef void (*psy_ui_fp_componentimp_dev_enableinput)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_preventinput)(struct psy_ui_ComponentImp*);
typedef bool (*psy_ui_fp_componentimp_dev_inputprevented)(const struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_setcursor)(struct psy_ui_ComponentImp*, psy_ui_CursorStyle);
typedef void (*psy_ui_fp_componentimp_dev_seticonressource)(struct psy_ui_ComponentImp*, int ressourceid);
typedef const psy_ui_TextMetric* (*psy_ui_fp_componentimp_dev_textmetric)(const struct psy_ui_ComponentImp*);
typedef psy_ui_Size (*psy_ui_fp_componentimp_dev_textsize)(const struct psy_ui_ComponentImp*,
	const char* text, const psy_ui_Font*);
typedef void (*psy_ui_fp_componentimp_dev_setbackgroundcolour)(struct psy_ui_ComponentImp*, psy_ui_Colour);
typedef void (*psy_ui_fp_componentimp_dev_settitle)(struct psy_ui_ComponentImp*, const char* title);
typedef void (*psy_ui_fp_componentimp_dev_setfocus)(struct psy_ui_ComponentImp*);
typedef int (*psy_ui_fp_componentimp_dev_hasfocus)(struct psy_ui_ComponentImp*);
typedef void* (*psy_ui_fp_componentimp_dev_platform)(struct psy_ui_ComponentImp*);
typedef bool (*psy_ui_fp_componentimp_dev_issystem)(struct psy_ui_ComponentImp*);
typedef uintptr_t (*psy_ui_fp_componentimp_dev_platform_handle)(struct psy_ui_ComponentImp*);
typedef uintptr_t (*psy_ui_fp_componentimp_dev_flags)(const struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_clear)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_componentimp_dev_mouseevent)(struct psy_ui_ComponentImp*, psy_ui_MouseEvent*);
typedef void (*psy_ui_fp_componentimp_dev_initialized)(struct psy_ui_ComponentImp*);

typedef struct psy_ui_ComponentImpVTable {
	psy_ui_fp_componentimp_dev_dispose dev_dispose;
	psy_ui_fp_componentimp_dev_destroy dev_destroy;
	psy_ui_fp_componentimp_dev_destroyed dev_destroyed;
	psy_ui_fp_componentimp_dev_show dev_show;
	psy_ui_fp_componentimp_dev_showstate dev_showstate;	
	psy_ui_fp_componentimp_dev_hide dev_hide;
	psy_ui_fp_componentimp_dev_visible dev_visible;
	psy_ui_fp_componentimp_dev_drawvisible dev_drawvisible;
	psy_ui_fp_componentimp_dev_move dev_move;
	psy_ui_fp_componentimp_dev_resize dev_resize;
	psy_ui_fp_componentimp_dev_clientresize dev_clientresize;
	psy_ui_fp_componentimp_dev_position dev_position;
	psy_ui_fp_componentimp_dev_screenposition dev_screenposition;
	psy_ui_fp_componentimp_dev_setposition dev_setposition;
	psy_ui_fp_componentimp_dev_size dev_size;
	psy_ui_fp_componentimp_dev_preferredsize dev_preferredsize;
	psy_ui_fp_componentimp_dev_updatesize dev_updatesize;
	psy_ui_fp_componentimp_dev_applyposition dev_applyposition;
	psy_ui_fp_componentimp_dev_framesize dev_framesize;
	psy_ui_fp_componentimp_dev_scrollto dev_scrollto;
	psy_ui_fp_componentimp_dev_parent dev_parent;
	psy_ui_fp_componentimp_dev_setparent dev_setparent;
	psy_ui_fp_componentimp_dev_insert dev_insert;
	psy_ui_fp_componentimp_dev_remove dev_remove;
	psy_ui_fp_componentimp_dev_remove dev_erase;
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
	psy_ui_fp_componentimp_dev_inputprevented dev_inputprevented;
	psy_ui_fp_componentimp_dev_setcursor dev_setcursor;	
	psy_ui_fp_componentimp_dev_seticonressource dev_seticonressource;
	psy_ui_fp_componentimp_dev_textmetric dev_textmetric;
	psy_ui_fp_componentimp_dev_textsize dev_textsize;
	psy_ui_fp_componentimp_dev_setbackgroundcolour dev_setbackgroundcolour;
	psy_ui_fp_componentimp_dev_settitle dev_settitle;
	psy_ui_fp_componentimp_dev_setfocus dev_setfocus;
	psy_ui_fp_componentimp_dev_hasfocus dev_hasfocus;
	psy_ui_fp_componentimp_dev_issystem dev_issystem;
	psy_ui_fp_componentimp_dev_platform dev_platform;
	psy_ui_fp_componentimp_dev_platform_handle dev_platform_handle;
	psy_ui_fp_componentimp_dev_flags dev_flags;
	psy_ui_fp_componentimp_dev_clear dev_clear;	
	psy_ui_fp_componentimp_dev_initialized dev_initialized;
} psy_ui_ComponentImpVTable;

typedef struct psy_ui_ComponentImp {
	psy_ui_ComponentImpVTable* vtable;
	psy_Signal signal_command;
	void* extended_vtable;
	void* extended_imp;
} psy_ui_ComponentImp;

void psy_ui_componentimp_init(psy_ui_ComponentImp*);
void psy_ui_componentimp_dispose(psy_ui_ComponentImp*);

psy_ui_Component* psy_ui_mainwindow(void);

INLINE void psy_ui_component_invalidate(psy_ui_Component* self)
{	
#if PSYCLE_USE_TK == PSYCLE_TK_X11	
	// if (!psy_ui_mainwindow()->visible) {
	//	return;
	//}
#endif	
	self->vtable->invalidate(self);	
}

INLINE void psy_ui_component_invalidaterect(psy_ui_Component* self, psy_ui_RealRectangle r)
{
	self->imp->vtable->dev_invalidaterect(self->imp, &r);
}

INLINE void psy_ui_component_update(psy_ui_Component* self)
{
	self->imp->vtable->dev_update(self->imp);
}

INLINE void psy_ui_component_clear(psy_ui_Component* self)
{
	self->imp->vtable->dev_clear(self->imp);
}

/* returns the element’s size that includes padding and border */
psy_ui_Size psy_ui_component_offsetsize(const psy_ui_Component* self);

INLINE psy_ui_RealRectangle psy_ui_component_position(const psy_ui_Component* self)
{
	return self->imp->vtable->dev_position(self->imp);
}

INLINE psy_ui_RealRectangle psy_ui_component_screenposition(const psy_ui_Component* self)
{
	return self->imp->vtable->dev_screenposition(self->imp);
}

psy_ui_RealRectangle psy_ui_component_scrolledposition(psy_ui_Component*);

void psy_ui_component_starttimer(psy_ui_Component* self, uintptr_t id,
	uintptr_t interval);
void psy_ui_component_stoptimer(psy_ui_Component* self, uintptr_t id);

INLINE const psy_ui_TextMetric* psy_ui_component_textmetric(const psy_ui_Component* self)
{
	assert(self->imp);
	
	return self->imp->vtable->dev_textmetric(self->imp);
}

INLINE psy_ui_Size psy_ui_component_textsize(const psy_ui_Component* self, const char* text)
{
	return self->imp->vtable->dev_textsize(self->imp, text,
		psy_ui_component_font(self));
}

/* returns the content's size(excludes padding and border) */
psy_ui_Size psy_ui_component_size(const psy_ui_Component*);

/* returns the element’s size that include padding but without the border */
psy_ui_Size psy_ui_component_clientsize(const psy_ui_Component*);

/*
** returns the element’s entire size that includes padding but
** not border, not margin and not scrollbars
*/
psy_ui_Size psy_ui_component_scrollsize(const psy_ui_Component*);

void psy_ui_component_setcolour(psy_ui_Component*, psy_ui_Colour);
psy_ui_Colour psy_ui_component_colour(psy_ui_Component*);
void psy_ui_component_setbackgroundcolour(psy_ui_Component*, psy_ui_Colour);
psy_ui_Colour psy_ui_component_backgroundcolour(psy_ui_Component*);

INLINE void psy_ui_component_settitle(psy_ui_Component* self, const char* text)
{	
	self->imp->vtable->dev_settitle(self->imp, text);
}

void psy_ui_component_capture(psy_ui_Component* self);
void psy_ui_component_releasecapture(psy_ui_Component* self);

void psy_ui_component_setfocus(psy_ui_Component*);
INLINE int psy_ui_component_hasfocus(psy_ui_Component* self)
{
	return self->imp->vtable->dev_hasfocus(self->imp);
}

INLINE uintptr_t psy_ui_component_section(const psy_ui_Component* self)
{
	assert(self);

	return self->vtable->section(self);
}

INLINE psy_ui_Component* psy_ui_component_parent(psy_ui_Component* self)
{
	if (self->imp) {
		return self->imp->vtable->dev_parent(self->imp);
	}
	return NULL;
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

INLINE void psy_ui_component_remove(psy_ui_Component* self, psy_ui_Component* child)
{	
	self->imp->vtable->dev_remove(self->imp, child->imp);	
}

INLINE void psy_ui_component_erase(psy_ui_Component* self, psy_ui_Component* child)
{
	self->imp->vtable->dev_erase(self->imp, child->imp);
}

INLINE void psy_ui_component_setorder(psy_ui_Component* self, psy_ui_Component* insertafter)
{
	self->imp->vtable->dev_setorder(self->imp, insertafter->imp);
}

INLINE void psy_ui_component_setwheelscroll(psy_ui_Component* self, int lines)
{
	psy_ui_component_usescroll(self);
	psy_ui_componentscroll_setwheel(self->scroll, lines);	
}

INLINE int psy_ui_component_wheelscroll(const psy_ui_Component* self)
{
	return psy_ui_componentscroll_wheel(self->scroll);
}

INLINE void psy_ui_component_selectsection(psy_ui_Component* self, uintptr_t section, uintptr_t options)
{
	psy_signal_emit(&self->signal_selectsection, self, 2, section, options);
}

void psy_ui_component_setscroll(psy_ui_Component*, psy_ui_Point);
void psy_ui_component_setscrollleft(psy_ui_Component*, psy_ui_Value left);

INLINE psy_ui_Value psy_ui_component_scrollleft(psy_ui_Component* self)
{	
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(self);
	return psy_ui_value_make_px(-position.left);	
}

INLINE double psy_ui_component_scrollleft_px(psy_ui_Component* self)
{		
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(self);
	return -floor(position.left);
}

void psy_ui_component_setscrolltop(psy_ui_Component*, psy_ui_Value top);

INLINE psy_ui_Value psy_ui_component_scrolltop(psy_ui_Component* self)
{		
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(self);
	return psy_ui_value_make_px(-position.top);
}

INLINE double psy_ui_component_scrolltop_px(psy_ui_Component* self)
{		
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(self);
	return -floor(position.top);	
}

INLINE void psy_ui_component_setscrolltop_px(psy_ui_Component* self,
	double top)
{
	psy_ui_component_setscrolltop(self, psy_ui_value_make_px(top));
}

void psy_ui_component_updateoverflow(psy_ui_Component*);


INLINE void psy_ui_component_setoverflow(psy_ui_Component* self, psy_ui_Overflow overflow)
{
	if (self->scroll->overflow != overflow) {
		psy_ui_component_usescroll(self);
		self->scroll->overflow = overflow;
		if (overflow == psy_ui_OVERFLOW_HIDDEN) {
			psy_ui_component_sethorizontalscrollrange(self, psy_ui_intpoint_zero());
			psy_ui_component_setverticalscrollrange(self, psy_ui_intpoint_zero());
		} else {
			psy_ui_component_updateoverflow(self);
		}		
	}
}

INLINE psy_ui_Overflow psy_ui_component_overflow(psy_ui_Component* self)
{
	assert(self);

	return self->scroll->overflow;
}

INLINE void psy_ui_component_setscrollstep(psy_ui_Component* self,
	psy_ui_Size step)
{
	assert(self);

	psy_ui_component_usescroll(self);
	self->scroll->step = step;	
}

INLINE void psy_ui_component_setscrollstep_width(psy_ui_Component* self,
	psy_ui_Value step)
{
	assert(self);

	psy_ui_component_usescroll(self);
	self->scroll->step.width = step;
}

INLINE psy_ui_Value psy_ui_component_scrollstep_width(const psy_ui_Component* self)
{
	assert(self);

	return self->scroll->step.width;
}

INLINE double psy_ui_component_scrollstep_width_px(const psy_ui_Component* self)
{	
	return psy_ui_value_px(&self->scroll->step.width,
		psy_ui_component_textmetric(self), NULL);
}

INLINE double psy_ui_component_scrollstep_height_px(const psy_ui_Component* self)
{	
	return psy_ui_value_px(&self->scroll->step.height,
		psy_ui_component_textmetric(self), NULL);
}

INLINE void psy_ui_component_setscrollstep_height(psy_ui_Component* self,
	psy_ui_Value step)
{
	assert(self);

	psy_ui_component_usescroll(self);	
	self->scroll->step.height = step;
}

INLINE psy_ui_Value psy_ui_component_scrollstep_height(const psy_ui_Component* self)
{
	assert(self);

	return self->scroll->step.height;
}

INLINE void psy_ui_component_settabindex(psy_ui_Component* self, uintptr_t index)
{
	assert(self);

	self->tabindex = index;
}

INLINE uintptr_t psy_ui_component_tabindex(const psy_ui_Component* self)
{
	assert(self);

	return self->tabindex;
}

void psy_ui_component_focus_next(psy_ui_Component*);
void psy_ui_component_focus_prev(psy_ui_Component*);

INLINE psy_ui_IntSize psy_ui_component_intsize(psy_ui_Component* self)
{	
	return psy_ui_intsize_init_size(psy_ui_component_scrollsize(self),
		psy_ui_component_textmetric(self), NULL);
}

INLINE psy_ui_Size psy_ui_component_parentsize(const psy_ui_Component* self)
{	
	if (psy_ui_component_parent_const(self)) {
		return psy_ui_component_scrollsize(psy_ui_component_parent_const(self));
	}
	return psy_ui_component_scrollsize(self);	
}

INLINE psy_ui_RealSize psy_ui_component_scrollsize_px(const psy_ui_Component* self)
{
	psy_ui_Size size;
	psy_ui_Size parentsize;
	
	parentsize = psy_ui_component_parentsize(self);
	size = psy_ui_component_scrollsize(self);	
	return psy_ui_size_px(&size, psy_ui_component_textmetric(self), &parentsize);
}

INLINE psy_ui_RealSize psy_ui_component_clientsize_px(const psy_ui_Component* self)
{
	psy_ui_Size size;
	psy_ui_Size parentsize;

	parentsize = psy_ui_component_parentsize(self);
	size = psy_ui_component_clientsize(self);
	return psy_ui_size_px(&size, psy_ui_component_textmetric(self), &parentsize);
}

INLINE psy_ui_RealSize psy_ui_component_offsetsize_px(const psy_ui_Component* self)
{
	const psy_ui_TextMetric* tm;
	psy_ui_Size size;
	psy_ui_Size parentsize;
	
	tm = psy_ui_component_textmetric(self);
	size = psy_ui_component_offsetsize(self);
	parentsize = psy_ui_component_parentsize(self);
	return psy_ui_realsize_make(
		psy_ui_value_px(&size.width, tm, &parentsize),
		psy_ui_value_px(&size.height, tm, &parentsize));
}

INLINE psy_ui_RealSize psy_ui_component_size_px(const psy_ui_Component* self)
{
	psy_ui_Size size;
	psy_ui_Size parentsize;

	size = psy_ui_component_size(self);
	parentsize = psy_ui_component_parentsize(self);
	return psy_ui_size_px(&size, psy_ui_component_textmetric(self), &parentsize);
}


int psy_ui_component_level(const psy_ui_Component*);
void psy_ui_component_setdefaultalign(psy_ui_Component*,
	psy_ui_AlignType, psy_ui_Margin);

void psy_ui_component_updatelanguage(psy_ui_Component*);
psy_Translator* psy_ui_translator(void);
const char* psy_ui_translate(const char* key);

void psy_ui_component_setstyletypes(psy_ui_Component*,
	uintptr_t standard, uintptr_t hover, uintptr_t select, uintptr_t disabled);
void psy_ui_component_setstyletype(psy_ui_Component*, uintptr_t standard);
void psy_ui_component_setstyletype_hover(psy_ui_Component*, uintptr_t hover);
void psy_ui_component_setstyletype_focus(psy_ui_Component*, uintptr_t focus);
void psy_ui_component_setstyletype_active(psy_ui_Component*, uintptr_t active);
void psy_ui_component_setstyletype_select(psy_ui_Component*, uintptr_t select);
void psy_ui_component_setstylestate(psy_ui_Component*, psy_ui_StyleState);
void psy_ui_component_addstylestate(psy_ui_Component*, psy_ui_StyleState);
void psy_ui_component_removestylestate(psy_ui_Component*, psy_ui_StyleState);
void psy_ui_component_addstylestate_children(psy_ui_Component*, psy_ui_StyleState);
void psy_ui_component_removestylestate_children(psy_ui_Component*, psy_ui_StyleState);
bool psy_ui_component_stylestate_active(const psy_ui_Component*, psy_ui_StyleState);

INLINE psy_ui_RealMargin psy_ui_component_margin_px(const psy_ui_Component* self)
{
	psy_ui_RealMargin rv;
	psy_ui_Margin margin;
	psy_ui_Size parentsize;
	
	margin = psy_ui_component_margin(self);
	parentsize = psy_ui_component_parentsize(self);
	psy_ui_realmargin_init_margin(&rv, &margin,
		psy_ui_component_textmetric(self), &parentsize);
	return rv;
}

INLINE psy_ui_RealMargin psy_ui_component_spacing_px(const psy_ui_Component* self)
{
	psy_ui_RealMargin rv;
	psy_ui_Margin margin;
	psy_ui_Size parentsize;
	
	margin = psy_ui_component_spacing(self);
	parentsize = psy_ui_component_parentsize(self);
	psy_ui_realmargin_init_margin(&rv, &margin,
		psy_ui_component_textmetric(self), &parentsize);
	return rv;
}

void psy_ui_component_draw(psy_ui_Component*, psy_ui_Graphics*);

INLINE void psy_ui_component_scrollto(psy_ui_Component* self,
	intptr_t dx, intptr_t dy, const psy_ui_RealRectangle* r)
{	
	self->vtable->scrollto(self, dx, dy, r);
}

void psy_ui_component_buffer(psy_ui_Component*);
void psy_ui_component_clearbuffer(psy_ui_Component*);
psy_ui_RealRectangle psy_ui_component_bounds(psy_ui_Component*);

INLINE bool psy_ui_component_islightweight(psy_ui_Component* self)
{
	return ((self->imp->vtable->dev_flags(self->imp)
		& psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN));
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMPONENT_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicursescomponentimp.h"
#if PSYCLE_USE_TK == PSYCLE_TK_CURSES
#include "uicursesgraphicsimp.h"
#include <string.h>


#include "uicomponent.h"
#include "uiapp.h"
#include <stdlib.h>
#include "../../detail/portable.h"
#include "uicursesdef.h"


static void psy_ui_curses_component_create_window(psy_ui_curses_ComponentImp* self,
	psy_ui_curses_ComponentImp* parent,	
	int x, int y, int width, int height);

// VTable Prototypes
static void dev_dispose(psy_ui_curses_ComponentImp*);
static void dev_destroy(psy_ui_curses_ComponentImp*);
static void dev_show(psy_ui_curses_ComponentImp*);
static void dev_showstate(psy_ui_curses_ComponentImp*, int state);
static void dev_hide(psy_ui_curses_ComponentImp*);
static int dev_visible(psy_ui_curses_ComponentImp*);
static void dev_move(psy_ui_curses_ComponentImp*, psy_ui_Point origin);
static void dev_resize(psy_ui_curses_ComponentImp*, psy_ui_Size);
static void dev_clientresize(psy_ui_curses_ComponentImp*, int width, int height);
static psy_ui_RealRectangle dev_position(psy_ui_curses_ComponentImp*);
static void dev_setposition(psy_ui_curses_ComponentImp*, psy_ui_Point, psy_ui_Size);
static psy_ui_Size dev_size(const psy_ui_curses_ComponentImp*);
static psy_ui_Size dev_framesize(psy_ui_curses_ComponentImp*);
static void dev_scrollto(psy_ui_curses_ComponentImp*, intptr_t dx, intptr_t dy);
static psy_ui_Component* dev_parent(psy_ui_curses_ComponentImp*);
static void dev_capture(psy_ui_curses_ComponentImp*);
static void dev_releasecapture(psy_ui_curses_ComponentImp*);
static void dev_invalidate(psy_ui_curses_ComponentImp*);
static void dev_invalidaterect(psy_ui_curses_ComponentImp*, const psy_ui_RealRectangle*);
static void dev_update(psy_ui_curses_ComponentImp*);
static void dev_setfont(psy_ui_curses_ComponentImp*, psy_ui_Font*);
static void dev_showhorizontalscrollbar(psy_ui_curses_ComponentImp*);
static void dev_hidehorizontalscrollbar(psy_ui_curses_ComponentImp*);
static void dev_sethorizontalscrollrange(psy_ui_curses_ComponentImp*, int min, int max);
static void dev_horizontalscrollrange(psy_ui_curses_ComponentImp*, int* scrollmin,
	int* scrollmax);
static int dev_horizontalscrollposition(psy_ui_curses_ComponentImp* self);
static void dev_sethorizontalscrollposition(psy_ui_curses_ComponentImp* self, int position);
static void dev_showverticalscrollbar(psy_ui_curses_ComponentImp*);
static void dev_hideverticalscrollbar(psy_ui_curses_ComponentImp*);
static void dev_setverticalscrollrange(psy_ui_curses_ComponentImp*, int min, int max);
static void dev_verticalscrollrange(psy_ui_curses_ComponentImp*, int* scrollmin,
	int* scrollmax);
static int dev_verticalscrollposition(psy_ui_curses_ComponentImp*);
static void dev_setverticalscrollposition(psy_ui_curses_ComponentImp*, int position);
static psy_List* dev_children(psy_ui_curses_ComponentImp*, int recursive);
static void dev_enableinput(psy_ui_curses_ComponentImp*);
static void dev_preventinput(psy_ui_curses_ComponentImp*);
static void dev_setcursor(psy_ui_curses_ComponentImp*, psy_ui_CursorStyle);
static void dev_starttimer(psy_ui_curses_ComponentImp*, uintptr_t id, uintptr_t interval);
static void dev_stoptimer(psy_ui_curses_ComponentImp*, uintptr_t id);
static void dev_seticonressource(psy_ui_curses_ComponentImp*, int ressourceid);
static const psy_ui_TextMetric* dev_textmetric(const psy_ui_curses_ComponentImp*, psy_ui_Font*);
static psy_ui_Size dev_textsize(psy_ui_curses_ComponentImp*, const char* text, psy_ui_Font*);
static void dev_setbackgroundcolour(psy_ui_curses_ComponentImp*, uint32_t colour);
static void dev_settitle(psy_ui_curses_ComponentImp*, const char* title);
static void dev_setfocus(psy_ui_curses_ComponentImp*);
static int dev_hasfocus(psy_ui_curses_ComponentImp*);

// VTable init
static psy_ui_ComponentImpVTable vtable;
static int vtable_initialized = 0;

static void win_imp_vtable_init(psy_ui_curses_ComponentImp* self)
{
	if (!vtable_initialized) {
		vtable = *self->imp.vtable;
		vtable.dev_dispose = (psy_ui_fp_componentimp_dev_dispose)dev_dispose;
		vtable.dev_destroy = (psy_ui_fp_componentimp_dev_destroy)dev_destroy;
		vtable.dev_show = (psy_ui_fp_componentimp_dev_show)dev_show;
		vtable.dev_showstate = (psy_ui_fp_componentimp_dev_showstate)dev_showstate;
		vtable.dev_hide = (psy_ui_fp_componentimp_dev_hide)dev_hide;
		vtable.dev_visible = (psy_ui_fp_componentimp_dev_visible)dev_visible;
		vtable.dev_move = (psy_ui_fp_componentimp_dev_move)dev_move;
		vtable.dev_resize = (psy_ui_fp_componentimp_dev_resize)dev_resize;
		vtable.dev_clientresize = (psy_ui_fp_componentimp_dev_clientresize)dev_clientresize;
		vtable.dev_position = (psy_ui_fp_componentimp_dev_position)dev_position;
		vtable.dev_setposition = (psy_ui_fp_componentimp_dev_setposition)dev_setposition;
		vtable.dev_size = (psy_ui_fp_componentimp_dev_size)dev_size;
		vtable.dev_framesize = (psy_ui_fp_componentimp_dev_framesize)dev_framesize;
		vtable.dev_scrollto = (psy_ui_fp_componentimp_dev_scrollto)dev_scrollto;
		vtable.dev_parent = (psy_ui_fp_componentimp_dev_parent)dev_parent;
		vtable.dev_capture = (psy_ui_fp_componentimp_dev_capture)dev_capture;
		vtable.dev_releasecapture = (psy_ui_fp_componentimp_dev_releasecapture)dev_releasecapture;
		vtable.dev_invalidate = (psy_ui_fp_componentimp_dev_invalidate)dev_invalidate;
		vtable.dev_invalidaterect = (psy_ui_fp_componentimp_dev_invalidaterect)dev_invalidaterect;
		vtable.dev_update = (psy_ui_fp_componentimp_dev_update)dev_update;
		vtable.dev_setfont = (psy_ui_fp_componentimp_dev_setfont)dev_setfont;
		vtable.dev_showhorizontalscrollbar = (psy_ui_fp_componentimp_dev_showhorizontalscrollbar)dev_showhorizontalscrollbar;
		vtable.dev_hidehorizontalscrollbar = (psy_ui_fp_componentimp_dev_hidehorizontalscrollbar)dev_hidehorizontalscrollbar;
		vtable.dev_sethorizontalscrollrange = (psy_ui_fp_componentimp_dev_sethorizontalscrollrange)dev_sethorizontalscrollrange;
		vtable.dev_horizontalscrollrange = (psy_ui_fp_componentimp_dev_horizontalscrollrange)dev_horizontalscrollrange;
		vtable.dev_horizontalscrollposition = (psy_ui_fp_componentimp_dev_horizontalscrollposition)dev_horizontalscrollposition;
		vtable.dev_sethorizontalscrollposition = (psy_ui_fp_componentimp_dev_sethorizontalscrollposition)dev_sethorizontalscrollposition;
		vtable.dev_showverticalscrollbar = (psy_ui_fp_componentimp_dev_showverticalscrollbar)dev_showverticalscrollbar;
		vtable.dev_hideverticalscrollbar = (psy_ui_fp_componentimp_dev_hideverticalscrollbar)dev_hideverticalscrollbar;
		vtable.dev_setverticalscrollrange = (psy_ui_fp_componentimp_dev_setverticalscrollrange)dev_setverticalscrollrange;
		vtable.dev_verticalscrollrange = (psy_ui_fp_componentimp_dev_verticalscrollrange)dev_verticalscrollrange;
		vtable.dev_verticalscrollposition = (psy_ui_fp_componentimp_dev_verticalscrollposition)dev_verticalscrollposition;
		vtable.dev_setverticalscrollposition = (psy_ui_fp_componentimp_dev_setverticalscrollposition)dev_setverticalscrollposition;
		vtable.dev_children = (psy_ui_fp_componentimp_dev_children)dev_children;
		vtable.dev_enableinput = (psy_ui_fp_componentimp_dev_enableinput)dev_enableinput;
		vtable.dev_preventinput = (psy_ui_fp_componentimp_dev_preventinput)dev_preventinput;
		vtable.dev_setcursor = (psy_ui_fp_componentimp_dev_setcursor)dev_setcursor;
		vtable.dev_starttimer = (psy_ui_fp_componentimp_dev_starttimer)dev_starttimer;
		vtable.dev_stoptimer = (psy_ui_fp_componentimp_dev_stoptimer)dev_stoptimer;
		vtable.dev_seticonressource = (psy_ui_fp_componentimp_dev_seticonressource)dev_seticonressource;
		vtable.dev_textmetric = (psy_ui_fp_componentimp_dev_textmetric)dev_textmetric;
		vtable.dev_textsize = (psy_ui_fp_componentimp_dev_textsize)dev_textsize;
		vtable.dev_setbackgroundcolour = (psy_ui_fp_componentimp_dev_setbackgroundcolour)dev_setbackgroundcolour;
		vtable.dev_settitle = (psy_ui_fp_componentimp_dev_settitle)dev_settitle;
		vtable.dev_setfocus = (psy_ui_fp_componentimp_dev_setfocus)dev_setfocus;
		vtable.dev_hasfocus = (psy_ui_fp_componentimp_dev_hasfocus)dev_hasfocus;
		vtable_initialized = 1;
	}
}

void psy_ui_curses_componentimp_init(psy_ui_curses_ComponentImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent,	
	int x, int y, int width, int height)
{	
	psy_ui_curses_ComponentImp* parent_imp;	

	psy_ui_componentimp_init(&self->imp);
	win_imp_vtable_init(self);
	self->imp.vtable = &vtable;
	self->component = component;	
	self->hwnd = 0;	
	parent_imp = parent ? (psy_ui_curses_ComponentImp*)parent : 0;	
	psy_ui_curses_component_create_window(self, parent_imp, x, y, width, height);	
}

void psy_ui_curses_component_create_window(psy_ui_curses_ComponentImp* self,
	psy_ui_curses_ComponentImp* parent,	
	int x, int y, int width, int height)
{	
	int err = 0;
	int maxyscr;
	int maxxscr;		
	int cx;
	int cy;
	int cw;
	int ch;

	getmaxyx(stdscr, maxyscr, maxxscr);
	cx = (int)((float)maxxscr / psy_ui_curses_VSCREENX * x);
	cy = (int)((float)maxyscr / psy_ui_curses_VSCREENY * y);
	cw = (int)((float)maxxscr / psy_ui_curses_VSCREENX * width);
	ch = (int)((float)maxyscr / psy_ui_curses_VSCREENY * height);
	self->hwnd = newwin(ch, cw, cy, cx);
	wborder(self->hwnd, '|', '|', '-', '-', '+', '+', '+', '+');
	wrefresh(self->hwnd);	
	if (self->hwnd == NULL) {	
		err = 1;
	} else {
		// psy_table_insert(&winapp->selfmap, (uintptr_t) self->hwnd, self);
	}	
}

// win32 implementation method for psy_ui_Component
void dev_dispose(psy_ui_curses_ComponentImp* self)
{
}

psy_ui_curses_ComponentImp* psy_ui_curses_componentimp_alloc(void)
{
	return (psy_ui_curses_ComponentImp*) malloc(sizeof(psy_ui_curses_ComponentImp));
}

psy_ui_curses_ComponentImp* psy_ui_curses_componentimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent,	
	int x, int y, int width, int height)
{
	psy_ui_curses_ComponentImp* rv;

	rv = psy_ui_curses_componentimp_alloc();
	if (rv) {
		psy_ui_curses_componentimp_init(rv,
			component,
			parent,			
			x, y, width, height			
		);
	}	
	return rv;
}

void dev_destroy(psy_ui_curses_ComponentImp* self)
{	
	wborder(self->hwnd, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(self->hwnd);
	delwin(self->hwnd);
}

void dev_show(psy_ui_curses_ComponentImp* self)
{	
}

void dev_showstate(psy_ui_curses_ComponentImp* self, int state)
{
}

void dev_hide(psy_ui_curses_ComponentImp* self)
{
}

int dev_visible(psy_ui_curses_ComponentImp* self)
{
	return 1; 
}

void dev_move(psy_ui_curses_ComponentImp* self, psy_ui_Point origin)
{
	int maxyscr;
	int maxxscr;
	int maxy;
	int maxx;
	int cx;
	int cy;

	getmaxyx(stdscr, maxyscr, maxxscr);
	getmaxyx(self->hwnd, maxy, maxx);
	cx = (int)((float)maxxscr / psy_ui_curses_VSCREENX * left);
	cy = (int)((float)maxyscr / psy_ui_curses_VSCREENY * top);	
	// wresize(self->hwnd, min(maxy, maxyscr - top), min(maxy, maxxscr - left));
	mvwin(self->hwnd, cy, cx);
	wrefresh(self->hwnd);
	dev_invalidate(self);
}

void dev_resize(psy_ui_curses_ComponentImp* self, psy_ui_Size)
{
	int maxyscr;
	int maxxscr;	
	int cw;
	int ch;

	getmaxyx(stdscr, maxyscr, maxxscr);	
	cw = (int)((float)maxxscr / psy_ui_curses_VSCREENX * width);
	ch = (int)((float)maxyscr / psy_ui_curses_VSCREENY * height);
	wresize(self->hwnd, ch, cw);
	wrefresh(self->hwnd);
	dev_invalidate(self);
}

void dev_clientresize(psy_ui_curses_ComponentImp* self, int width, int height)
{
}

psy_ui_RealRectangle dev_position(psy_ui_curses_ComponentImp* self)
{
	psy_ui_RealRectangle rv;
	psy_ui_setrectangle(&rv, 0, 0, 0, 0);
	return rv;
}

void dev_setposition(psy_ui_curses_ComponentImp* self, psy_ui_Point topleft, psy_ui_Size size)
{
	
}

psy_ui_Size dev_size(const psy_ui_curses_ComponentImp* self)
{
	return psy_ui_size_make_em(0.0, 0.0);
}

psy_ui_Size dev_framesize(psy_ui_curses_ComponentImp* self)
{
	return psy_ui_size_make_em(0.0, 0.0);
}

void dev_scrollto(psy_ui_curses_ComponentImp* self, intptr_t dx, intptr_t dy)
{
}

psy_ui_Component* dev_parent(psy_ui_curses_ComponentImp* self)
{
/*	psy_ui_WinApp* winapp;
	psy_ui_curses_ComponentImp* imp;

	winapp = (psy_ui_WinApp*) app.platform;
	imp = (psy_ui_curses_ComponentImp*) psy_table_at(&winapp->selfmap,
		(uintptr_t)GetParent(self->hwnd));
	return imp ? imp->component : 0;*/
	return 0;
}

void dev_capture(psy_ui_curses_ComponentImp* self)
{	
}

void dev_releasecapture(psy_ui_curses_ComponentImp* self)
{	
}

void dev_invalidate(psy_ui_curses_ComponentImp* self)
{
	psy_ui_curses_GraphicsImp g;
	
	psy_ui_curses_graphicsimp_init(&g, (uintptr_t*) self->hwnd);
	psy_signal_emit(&self->component->signal_draw, self, 1, &g);	
}

void dev_invalidaterect(psy_ui_curses_ComponentImp* self, const psy_ui_RealRectangle* r)
{
}

void dev_update(psy_ui_curses_ComponentImp* self)
{	
}

void dev_setfont(psy_ui_curses_ComponentImp* self, psy_ui_Font* source)
{
}

void dev_showhorizontalscrollbar(psy_ui_curses_ComponentImp* self)
{
}

void dev_hidehorizontalscrollbar(psy_ui_curses_ComponentImp* self)
{
}

void dev_sethorizontalscrollrange(psy_ui_curses_ComponentImp* self, int min, int max)
{
}

void dev_horizontalscrollrange(psy_ui_curses_ComponentImp* self, int* scrollmin,
	int* scrollmax)
{
}

void dev_showverticalscrollbar(psy_ui_curses_ComponentImp* self)
{
}

void dev_hideverticalscrollbar(psy_ui_curses_ComponentImp* self)
{
}

void dev_setverticalscrollrange(psy_ui_curses_ComponentImp* self, int min, int max)
{
}

int dev_verticalscrollposition(psy_ui_curses_ComponentImp* self)
{
	return 0;
}

void dev_setverticalscrollposition(psy_ui_curses_ComponentImp* self, int position)
{
}

int dev_horizontalscrollposition(psy_ui_curses_ComponentImp* self)
{
	return 0;
}

void dev_sethorizontalscrollposition(psy_ui_curses_ComponentImp* self, int position)
{
}

void dev_verticalscrollrange(psy_ui_curses_ComponentImp* self, int* scrollmin,
	int* scrollmax)
{
}

psy_List* dev_children(psy_ui_curses_ComponentImp* self, int recursive)
{	
	return 0;
}

void dev_enableinput(psy_ui_curses_ComponentImp* self)
{	
}

void dev_preventinput(psy_ui_curses_ComponentImp* self)
{
}

const psy_ui_TextMetric* dev_textmetric(const psy_ui_curses_ComponentImp* self, psy_ui_Font* font)
{
	static psy_ui_TextMetric rv;

	memset(&rv, 0, sizeof(psy_ui_TextMetric));
	return &rv;
}

void dev_setcursor(psy_ui_curses_ComponentImp* self, psy_ui_CursorStyle cursorstyle)
{
}

void dev_starttimer(psy_ui_curses_ComponentImp* self, uintptr_t id,
	uintptr_t interval)
{
}

void dev_stoptimer(psy_ui_curses_ComponentImp* self, unsigned int id)
{
}

void dev_seticonressource(psy_ui_curses_ComponentImp* self, int ressourceid)
{
}

psy_ui_Size dev_textsize(psy_ui_curses_ComponentImp* self, const char* text, psy_ui_Font* font)
{
	psy_ui_Size rv = { 0, 0 };
	return rv;
}

void dev_setbackgroundcolour(psy_ui_curses_ComponentImp* self, uint32_t colour)
{		
}

void dev_settitle(psy_ui_curses_ComponentImp* self, const char* title)
{
}

void dev_setfocus(psy_ui_curses_ComponentImp* self)
{
}

int dev_hasfocus(psy_ui_curses_ComponentImp* self)
{
	return 0;
}

#endif

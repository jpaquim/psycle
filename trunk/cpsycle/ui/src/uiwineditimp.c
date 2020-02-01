// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwineditimp.h"
#include "uiwincomponentimp.h"
#include "uicomponent.h"
#include "uiapp.h"
#include "uiwinapp.h"
#include <stdlib.h>
#include "../../detail/portable.h"

// WinComponentImp VTable Delegation
static void dev_dispose(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_dispose(&self->win_component_imp.imp); }
static void dev_destroy(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_destroy(&self->win_component_imp.imp); }
static void dev_show(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_show(&self->win_component_imp.imp); }
static void dev_showstate(psy_ui_win_EditImp* self, int state) { self->win_component_imp.imp.vtable->dev_showstate(&self->win_component_imp.imp, state); }
static void dev_hide(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_hide(&self->win_component_imp.imp); }
static int dev_visible(psy_ui_win_EditImp* self) { return self->win_component_imp.imp.vtable->dev_visible(&self->win_component_imp.imp); }
static void dev_move(psy_ui_win_EditImp* self, int left, int top) { self->win_component_imp.imp.vtable->dev_move(&self->win_component_imp.imp, left, top); }
static void dev_resize(psy_ui_win_EditImp* self, int width, int height) { self->win_component_imp.imp.vtable->dev_resize(&self->win_component_imp.imp, width, height); }
static void dev_clientresize(psy_ui_win_EditImp* self, int width, int height) { self->win_component_imp.imp.vtable->dev_clientresize(&self->win_component_imp.imp, width, height); }
static psy_ui_Rectangle dev_position(psy_ui_win_EditImp* self) { return self->win_component_imp.imp.vtable->dev_position(&self->win_component_imp.imp); }
static void dev_setposition(psy_ui_win_EditImp* self, int x, int y, int width, int height) { self->win_component_imp.imp.vtable->dev_setposition(&self->win_component_imp.imp, x, y, width, height); }
static psy_ui_Size dev_size(psy_ui_win_EditImp* self) { return self->win_component_imp.imp.vtable->dev_size(&self->win_component_imp.imp); }
static psy_ui_Size dev_framesize(psy_ui_win_EditImp* self) { return self->win_component_imp.imp.vtable->dev_framesize(&self->win_component_imp.imp); }
static void dev_scrollto(psy_ui_win_EditImp* self, intptr_t dx, intptr_t dy) { self->win_component_imp.imp.vtable->dev_scrollto(&self->win_component_imp.imp, dx, dy); }
static psy_ui_Component* dev_parent(psy_ui_win_EditImp* self) { return self->win_component_imp.imp.vtable->dev_parent(&self->win_component_imp.imp); }
static void dev_capture(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_capture(&self->win_component_imp.imp); }
static void dev_releasecapture(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_releasecapture(&self->win_component_imp.imp); }
static void dev_invalidate(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_invalidate(&self->win_component_imp.imp); }
static void dev_invalidaterect(psy_ui_win_EditImp* self, const psy_ui_Rectangle* r) { self->win_component_imp.imp.vtable->dev_invalidaterect(&self->win_component_imp.imp, r); }
static void dev_update(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_update(&self->win_component_imp.imp); }
static void dev_setfont(psy_ui_win_EditImp* self, psy_ui_Font* font) { self->win_component_imp.imp.vtable->dev_setfont(&self->win_component_imp.imp, font); }
static void dev_showhorizontalscrollbar(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_showhorizontalscrollbar(&self->win_component_imp.imp); }
static void dev_hidehorizontalscrollbar(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_hidehorizontalscrollbar(&self->win_component_imp.imp); }
static void dev_sethorizontalscrollrange(psy_ui_win_EditImp* self, int min, int max) { self->win_component_imp.imp.vtable->dev_sethorizontalscrollrange(&self->win_component_imp.imp, min, max); }
static void dev_horizontalscrollrange(psy_ui_win_EditImp* self, int* scrollmin, int* scrollmax) { self->win_component_imp.imp.vtable->dev_horizontalscrollrange(&self->win_component_imp.imp, scrollmin, scrollmax); }
static int dev_horizontalscrollposition(psy_ui_win_EditImp* self) { return self->win_component_imp.imp.vtable->dev_horizontalscrollposition(&self->win_component_imp.imp); }
static void dev_sethorizontalscrollposition(psy_ui_win_EditImp* self, int position) { self->win_component_imp.imp.vtable->dev_sethorizontalscrollposition(&self->win_component_imp.imp, position); }
static void dev_showverticalscrollbar(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_showverticalscrollbar(&self->win_component_imp.imp); }
static void dev_hideverticalscrollbar(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_hideverticalscrollbar(&self->win_component_imp.imp); }
static void dev_setverticalscrollrange(psy_ui_win_EditImp* self, int min, int max) { self->win_component_imp.imp.vtable->dev_setverticalscrollrange(&self->win_component_imp.imp, min, max); }
static void dev_verticalscrollrange(psy_ui_win_EditImp* self, int* scrollmin, int* scrollmax) { self->win_component_imp.imp.vtable->dev_verticalscrollrange(&self->win_component_imp.imp, scrollmin, scrollmax); }
static int dev_verticalscrollposition(psy_ui_win_EditImp* self) { return self->win_component_imp.imp.vtable->dev_verticalscrollposition(&self->win_component_imp.imp); }
static void dev_setverticalscrollposition(psy_ui_win_EditImp* self, int position) { self->win_component_imp.imp.vtable->dev_setverticalscrollposition(&self->win_component_imp.imp, position); }
static psy_List* dev_children(psy_ui_win_EditImp* self, int recursive) { return self->win_component_imp.imp.vtable->dev_children(&self->win_component_imp.imp, recursive); }
static void dev_enableinput(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_enableinput(&self->win_component_imp.imp); }
static void dev_preventinput(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_preventinput(&self->win_component_imp.imp); }
static void dev_setcursor(psy_ui_win_EditImp* self, psy_ui_CursorStyle style) { self->win_component_imp.imp.vtable->dev_setcursor(&self->win_component_imp.imp, style); }
static void dev_starttimer(psy_ui_win_EditImp* self, unsigned int id, unsigned int interval) { self->win_component_imp.imp.vtable->dev_starttimer(&self->win_component_imp.imp, id, interval); }
static void dev_stoptimer(psy_ui_win_EditImp* self, unsigned int id) { self->win_component_imp.imp.vtable->dev_stoptimer(&self->win_component_imp.imp, id); }
static void dev_seticonressource(psy_ui_win_EditImp* self, int ressourceid) { self->win_component_imp.imp.vtable->dev_seticonressource(&self->win_component_imp.imp, ressourceid); }
static psy_ui_TextMetric dev_textmetric(psy_ui_win_EditImp* self, psy_ui_Font* font) { return self->win_component_imp.imp.vtable->dev_textmetric(&self->win_component_imp.imp, font); }
static psy_ui_Size dev_textsize(psy_ui_win_EditImp* self, const char* text, psy_ui_Font* font) { return self->win_component_imp.imp.vtable->dev_textsize(&self->win_component_imp.imp, text, font); }
static void dev_setbackgroundcolor(psy_ui_win_EditImp* self, uint32_t color) { self->win_component_imp.imp.vtable->dev_setbackgroundcolor(&self->win_component_imp.imp, color); }
static void dev_settitle(psy_ui_win_EditImp* self, const char* title) { self->win_component_imp.imp.vtable->dev_settitle(&self->win_component_imp.imp, title); }
static void dev_setfocus(psy_ui_win_EditImp* self) { self->win_component_imp.imp.vtable->dev_setfocus(&self->win_component_imp.imp); }
static int dev_hasfocus(psy_ui_win_EditImp* self) { return self->win_component_imp.imp.vtable->dev_hasfocus(&self->win_component_imp.imp); }
static void* dev_platform(psy_ui_win_EditImp* self) { return (void*) &self->win_component_imp; }

// VTable init
static psy_ui_ComponentImpVTable vtable;
static int vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!vtable_initialized) {		
		vtable.dev_dispose = (psy_ui_fp_componentimp_dev_dispose) dev_dispose;
		vtable.dev_destroy = (psy_ui_fp_componentimp_dev_destroy) dev_destroy;
		vtable.dev_show = (psy_ui_fp_componentimp_dev_show) dev_show;
		vtable.dev_showstate = (psy_ui_fp_componentimp_dev_showstate) dev_showstate;
		vtable.dev_hide = (psy_ui_fp_componentimp_dev_hide) dev_hide;
		vtable.dev_visible = (psy_ui_fp_componentimp_dev_visible) dev_visible;
		vtable.dev_move = (psy_ui_fp_componentimp_dev_move) dev_move;
		vtable.dev_resize = (psy_ui_fp_componentimp_dev_resize) dev_resize;
		vtable.dev_clientresize = (psy_ui_fp_componentimp_dev_clientresize) dev_clientresize;
		vtable.dev_position = (psy_ui_fp_componentimp_dev_position) dev_position;
		vtable.dev_setposition = (psy_ui_fp_componentimp_dev_setposition) dev_setposition;
		vtable.dev_size = (psy_ui_fp_componentimp_dev_size) dev_size;
		vtable.dev_framesize = (psy_ui_fp_componentimp_dev_framesize) dev_framesize;
		vtable.dev_scrollto = (psy_ui_fp_componentimp_dev_scrollto) dev_scrollto;
		vtable.dev_parent = (psy_ui_fp_componentimp_dev_parent) dev_parent;
		vtable.dev_capture = (psy_ui_fp_componentimp_dev_capture) dev_capture;
		vtable.dev_releasecapture = (psy_ui_fp_componentimp_dev_releasecapture) dev_releasecapture;
		vtable.dev_invalidate = (psy_ui_fp_componentimp_dev_invalidate) dev_invalidate;
		vtable.dev_invalidaterect = (psy_ui_fp_componentimp_dev_invalidaterect) dev_invalidaterect;
		vtable.dev_update = (psy_ui_fp_componentimp_dev_update) dev_update;
		vtable.dev_setfont = (psy_ui_fp_componentimp_dev_setfont) dev_setfont;
		vtable.dev_showhorizontalscrollbar = (psy_ui_fp_componentimp_dev_showhorizontalscrollbar) dev_showhorizontalscrollbar;
		vtable.dev_hidehorizontalscrollbar = (psy_ui_fp_componentimp_dev_hidehorizontalscrollbar) dev_hidehorizontalscrollbar;
		vtable.dev_sethorizontalscrollrange = (psy_ui_fp_componentimp_dev_sethorizontalscrollrange) dev_sethorizontalscrollrange;
		vtable.dev_horizontalscrollrange = (psy_ui_fp_componentimp_dev_horizontalscrollrange) dev_horizontalscrollrange;
		vtable.dev_horizontalscrollposition = (psy_ui_fp_componentimp_dev_horizontalscrollposition) dev_horizontalscrollposition;
		vtable.dev_sethorizontalscrollposition = (psy_ui_fp_componentimp_dev_sethorizontalscrollposition) dev_sethorizontalscrollposition;
		vtable.dev_showverticalscrollbar = (psy_ui_fp_componentimp_dev_showverticalscrollbar) dev_showverticalscrollbar;
		vtable.dev_hideverticalscrollbar = (psy_ui_fp_componentimp_dev_hideverticalscrollbar) dev_hideverticalscrollbar;
		vtable.dev_setverticalscrollrange = (psy_ui_fp_componentimp_dev_setverticalscrollrange) dev_setverticalscrollrange;
		vtable.dev_verticalscrollrange = (psy_ui_fp_componentimp_dev_verticalscrollrange) dev_verticalscrollrange;
		vtable.dev_verticalscrollposition = (psy_ui_fp_componentimp_dev_verticalscrollposition) dev_verticalscrollposition;
		vtable.dev_setverticalscrollposition = (psy_ui_fp_componentimp_dev_setverticalscrollposition) dev_setverticalscrollposition;
		vtable.dev_children = (psy_ui_fp_componentimp_dev_children) dev_children;
		vtable.dev_enableinput = (psy_ui_fp_componentimp_dev_enableinput) dev_enableinput;
		vtable.dev_preventinput = (psy_ui_fp_componentimp_dev_preventinput) dev_preventinput;
		vtable.dev_setcursor = (psy_ui_fp_componentimp_dev_setcursor) dev_setcursor;
		vtable.dev_starttimer = (psy_ui_fp_componentimp_dev_starttimer) dev_starttimer;
		vtable.dev_stoptimer = (psy_ui_fp_componentimp_dev_stoptimer) dev_stoptimer;
		vtable.dev_seticonressource = (psy_ui_fp_componentimp_dev_seticonressource) dev_seticonressource;
		vtable.dev_textmetric = (psy_ui_fp_componentimp_dev_textmetric) dev_textmetric;
		vtable.dev_textsize = (psy_ui_fp_componentimp_dev_textsize) dev_textsize;
		vtable.dev_setbackgroundcolor = (psy_ui_fp_componentimp_dev_setbackgroundcolor) dev_setbackgroundcolor;
		vtable.dev_settitle = (psy_ui_fp_componentimp_dev_settitle) dev_settitle;
		vtable.dev_setfocus = (psy_ui_fp_componentimp_dev_setfocus) dev_setfocus;
		vtable.dev_hasfocus = (psy_ui_fp_componentimp_dev_hasfocus) dev_hasfocus;
		vtable.dev_platform = (psy_ui_fp_componentimp_dev_platform) dev_platform;
		vtable_initialized = 1;
	}
}

static void oncommand(psy_ui_Edit*, psy_ui_Component* sender, WPARAM wParam, LPARAM lParam);

// EditImp VTable

static void dev_settext(psy_ui_win_EditImp*, const char* text);
static void dev_text(psy_ui_win_EditImp* self, char* text);
static void dev_setstyle(psy_ui_win_EditImp*, int style);
static void dev_enableedit(psy_ui_win_EditImp*);
static void dev_preventedit(psy_ui_win_EditImp*);

static psy_ui_EditImpVTable editimp_vtable;
static int editimp_vtable_initialized = 0;

static void editimp_imp_vtable_init(psy_ui_win_EditImp* self)
{
	if (!editimp_vtable_initialized) {
		editimp_vtable.dev_settext = (psy_ui_fp_editimp_dev_settext) dev_settext;
		editimp_vtable.dev_text = (psy_ui_fp_editimp_dev_text) dev_text;
		editimp_vtable.dev_setstyle = (psy_ui_fp_editimp_dev_setstyle) dev_setstyle;
		editimp_vtable.dev_enableedit = (psy_ui_fp_editimp_dev_preventedit) dev_enableedit;
		editimp_vtable.dev_preventedit = (psy_ui_fp_editimp_dev_preventedit) dev_preventedit;
		editimp_vtable_initialized = 1;
	}
}

void psy_ui_win_editimp_init(psy_ui_win_EditImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{	
	psy_ui_win_componentimp_init(&self->win_component_imp,
		component,
		parent,
		TEXT("EDIT"),
		0, 0, 100, 20,		
		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LEFT,
		1);
	imp_vtable_init();
	self->imp.component_imp.vtable = &vtable;
	psy_ui_editimp_init(&self->imp);
	editimp_imp_vtable_init(self);
	self->imp.vtable = &editimp_vtable;
	psy_signal_connect(&self->win_component_imp.imp.signal_command, component, oncommand);
}

void psy_ui_win_editimp_multiline_init(psy_ui_win_EditImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_componentimp_init(&self->win_component_imp,
		component,
		parent,
		TEXT("EDIT"),
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT,
		1);
	imp_vtable_init();
	self->imp.component_imp.vtable = &vtable;
	psy_ui_editimp_init(&self->imp);
	editimp_imp_vtable_init(self);
	self->imp.vtable = &editimp_vtable;
	psy_signal_connect(&self->win_component_imp.imp.signal_command, component, oncommand);
}

psy_ui_win_EditImp* psy_ui_win_editimp_alloc(void)
{
	return (psy_ui_win_EditImp*) malloc(sizeof(psy_ui_win_EditImp));
}

psy_ui_win_EditImp* psy_ui_win_editimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_EditImp* rv;

	rv = psy_ui_win_editimp_alloc();
	if (rv) {
		psy_ui_win_editimp_init(rv, component, parent);
	}
	return rv;
}

psy_ui_win_EditImp* psy_ui_win_editimp_multiline_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_EditImp* rv;

	rv = psy_ui_win_editimp_alloc();
	if (rv) {
		psy_ui_win_editimp_multiline_init(rv, component, parent);
	}
	return rv;
}

void dev_settext(psy_ui_win_EditImp* self, const char* text)
{
	SetWindowText(self->win_component_imp.hwnd, text);		
}

void dev_setstyle(psy_ui_win_EditImp* self, int style)
{
#if defined(_WIN64)
	SetWindowLongPtr(self->win_component_imp.hwnd, GWL_STYLE, style);
#else
	SetWindowLong(self->win_component_imp.hwnd, GWL_STYLE, style);
#endif
}

void dev_text(psy_ui_win_EditImp* self, char* text)
{	
	GetWindowText(self->win_component_imp.hwnd, text, 256);
}

void dev_enableedit(psy_ui_win_EditImp* self)
{
	SendMessage(self->win_component_imp.hwnd, EM_SETREADONLY, (WPARAM)0,
		(LPARAM)0);
}

void dev_preventedit(psy_ui_win_EditImp* self)
{
	SendMessage(self->win_component_imp.hwnd, EM_SETREADONLY, (WPARAM)1,
		(LPARAM)0);
}

void oncommand(psy_ui_Edit* self, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam) {
	switch (HIWORD(wParam))
	{
	case EN_CHANGE:
	{
		if (self->signal_change.slots) {
			psy_signal_emit(&self->signal_change, self, 0);
		}
	}
	break;
	default:
		break;
	}
}

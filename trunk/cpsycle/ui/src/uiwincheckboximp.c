// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwincheckboximp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "uiwincomponentimp.h"
#include "uicomponent.h"
#include "uiapp.h"
#include "uiwinapp.h"
#include <stdlib.h>
#include "../../detail/portable.h"

// WinComponentImp VTable Delegation
static void dev_dispose(psy_ui_win_CheckBoxImp* self) { self->win_component_imp.imp.vtable->dev_dispose(&self->win_component_imp.imp); }
static void dev_destroy(psy_ui_win_CheckBoxImp* self) { self->win_component_imp.imp.vtable->dev_destroy(&self->win_component_imp.imp); }
static void dev_show(psy_ui_win_CheckBoxImp* self) { self->win_component_imp.imp.vtable->dev_show(&self->win_component_imp.imp); }
static void dev_showstate(psy_ui_win_CheckBoxImp* self, int state) { self->win_component_imp.imp.vtable->dev_showstate(&self->win_component_imp.imp, state); }
static void dev_hide(psy_ui_win_CheckBoxImp* self) { self->win_component_imp.imp.vtable->dev_hide(&self->win_component_imp.imp); }
static int dev_visible(psy_ui_win_CheckBoxImp* self) { return self->win_component_imp.imp.vtable->dev_visible(&self->win_component_imp.imp); }
static void dev_move(psy_ui_win_CheckBoxImp* self, double left, double top) { self->win_component_imp.imp.vtable->dev_move(&self->win_component_imp.imp, left, top); }
static void dev_resize(psy_ui_win_CheckBoxImp* self, psy_ui_Size size) { self->win_component_imp.imp.vtable->dev_resize(&self->win_component_imp.imp, size); }
static void dev_clientresize(psy_ui_win_CheckBoxImp* self, int width, int height) { self->win_component_imp.imp.vtable->dev_clientresize(&self->win_component_imp.imp, width, height); }
static psy_ui_Rectangle dev_position(psy_ui_win_CheckBoxImp* self) { return self->win_component_imp.imp.vtable->dev_position(&self->win_component_imp.imp); }
static void dev_setposition(psy_ui_win_CheckBoxImp* self, psy_ui_Point topleft, psy_ui_Size size) { self->win_component_imp.imp.vtable->dev_setposition(&self->win_component_imp.imp, topleft, size); }
static psy_ui_Size dev_size(psy_ui_win_CheckBoxImp* self) { return self->win_component_imp.imp.vtable->dev_size(&self->win_component_imp.imp); }
static psy_ui_Size dev_preferredsize(psy_ui_win_CheckBoxImp* self, const psy_ui_Size* limits);
static psy_ui_Size dev_framesize(psy_ui_win_CheckBoxImp* self) { return self->win_component_imp.imp.vtable->dev_framesize(&self->win_component_imp.imp); }
static void dev_scrollto(psy_ui_win_CheckBoxImp* self, intptr_t dx, intptr_t dy) { self->win_component_imp.imp.vtable->dev_scrollto(&self->win_component_imp.imp, dx, dy); }
static psy_ui_Component* dev_parent(psy_ui_win_CheckBoxImp* self) { return self->win_component_imp.imp.vtable->dev_parent(&self->win_component_imp.imp); }
static void dev_capture(psy_ui_win_CheckBoxImp* self) { self->win_component_imp.imp.vtable->dev_capture(&self->win_component_imp.imp); }
static void dev_releasecapture(psy_ui_win_CheckBoxImp* self) { self->win_component_imp.imp.vtable->dev_releasecapture(&self->win_component_imp.imp); }
static void dev_invalidate(psy_ui_win_CheckBoxImp* self) { self->win_component_imp.imp.vtable->dev_invalidate(&self->win_component_imp.imp); }
static void dev_invalidaterect(psy_ui_win_CheckBoxImp* self, const psy_ui_Rectangle* r) { self->win_component_imp.imp.vtable->dev_invalidaterect(&self->win_component_imp.imp, r); }
static void dev_update(psy_ui_win_CheckBoxImp* self) { self->win_component_imp.imp.vtable->dev_update(&self->win_component_imp.imp); }
static void dev_setfont(psy_ui_win_CheckBoxImp* self, psy_ui_Font* font) { self->win_component_imp.imp.vtable->dev_setfont(&self->win_component_imp.imp, font); }
static psy_List* dev_children(psy_ui_win_CheckBoxImp* self, int recursive) { return self->win_component_imp.imp.vtable->dev_children(&self->win_component_imp.imp, recursive); }
static void dev_enableinput(psy_ui_win_CheckBoxImp* self) { self->win_component_imp.imp.vtable->dev_enableinput(&self->win_component_imp.imp); }
static void dev_preventinput(psy_ui_win_CheckBoxImp* self) { self->win_component_imp.imp.vtable->dev_preventinput(&self->win_component_imp.imp); }
static void dev_setcursor(psy_ui_win_CheckBoxImp* self, psy_ui_CursorStyle style) { self->win_component_imp.imp.vtable->dev_setcursor(&self->win_component_imp.imp, style); }
static void dev_starttimer(psy_ui_win_CheckBoxImp* self, uintptr_t id, uintptr_t interval) { self->win_component_imp.imp.vtable->dev_starttimer(&self->win_component_imp.imp, id, interval); }
static void dev_stoptimer(psy_ui_win_CheckBoxImp* self, uintptr_t id) { self->win_component_imp.imp.vtable->dev_stoptimer(&self->win_component_imp.imp, id); }
static void dev_seticonressource(psy_ui_win_CheckBoxImp* self, int ressourceid) { self->win_component_imp.imp.vtable->dev_seticonressource(&self->win_component_imp.imp, ressourceid); }
static const psy_ui_TextMetric* dev_textmetric(psy_ui_win_CheckBoxImp* self) { return self->win_component_imp.imp.vtable->dev_textmetric(&self->win_component_imp.imp); }
static psy_ui_Size dev_textsize(psy_ui_win_CheckBoxImp* self, const char* text, psy_ui_Font* font) { return self->win_component_imp.imp.vtable->dev_textsize(&self->win_component_imp.imp, text, font); }
static void dev_setbackgroundcolour(psy_ui_win_CheckBoxImp* self, psy_ui_Colour colour) { self->win_component_imp.imp.vtable->dev_setbackgroundcolour(&self->win_component_imp.imp, colour); }
static void dev_settitle(psy_ui_win_CheckBoxImp* self, const char* title) { self->win_component_imp.imp.vtable->dev_settitle(&self->win_component_imp.imp, title); }
static void dev_setfocus(psy_ui_win_CheckBoxImp* self) { self->win_component_imp.imp.vtable->dev_setfocus(&self->win_component_imp.imp); }
static int dev_hasfocus(psy_ui_win_CheckBoxImp* self) { return self->win_component_imp.imp.vtable->dev_hasfocus(&self->win_component_imp.imp); }
static void* dev_platform(psy_ui_win_CheckBoxImp* self) { return (void*) &self->win_component_imp; }

// VTable init
static psy_ui_ComponentImpVTable vtable;
static bool vtable_initialized = 0;

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
		vtable.dev_preferredsize = (psy_ui_fp_componentimp_dev_preferredsize)dev_preferredsize;
		vtable.dev_framesize = (psy_ui_fp_componentimp_dev_framesize) dev_framesize;
		vtable.dev_scrollto = (psy_ui_fp_componentimp_dev_scrollto) dev_scrollto;
		vtable.dev_parent = (psy_ui_fp_componentimp_dev_parent) dev_parent;
		vtable.dev_capture = (psy_ui_fp_componentimp_dev_capture) dev_capture;
		vtable.dev_releasecapture = (psy_ui_fp_componentimp_dev_releasecapture) dev_releasecapture;
		vtable.dev_invalidate = (psy_ui_fp_componentimp_dev_invalidate) dev_invalidate;
		vtable.dev_invalidaterect = (psy_ui_fp_componentimp_dev_invalidaterect) dev_invalidaterect;
		vtable.dev_update = (psy_ui_fp_componentimp_dev_update) dev_update;
		vtable.dev_setfont = (psy_ui_fp_componentimp_dev_setfont) dev_setfont;
		vtable.dev_children = (psy_ui_fp_componentimp_dev_children) dev_children;
		vtable.dev_enableinput = (psy_ui_fp_componentimp_dev_enableinput) dev_enableinput;
		vtable.dev_preventinput = (psy_ui_fp_componentimp_dev_preventinput) dev_preventinput;
		vtable.dev_setcursor = (psy_ui_fp_componentimp_dev_setcursor) dev_setcursor;
		vtable.dev_starttimer = (psy_ui_fp_componentimp_dev_starttimer) dev_starttimer;
		vtable.dev_stoptimer = (psy_ui_fp_componentimp_dev_stoptimer) dev_stoptimer;
		vtable.dev_seticonressource = (psy_ui_fp_componentimp_dev_seticonressource) dev_seticonressource;
		vtable.dev_textmetric = (psy_ui_fp_componentimp_dev_textmetric) dev_textmetric;
		vtable.dev_textsize = (psy_ui_fp_componentimp_dev_textsize) dev_textsize;
		vtable.dev_setbackgroundcolour = (psy_ui_fp_componentimp_dev_setbackgroundcolour) dev_setbackgroundcolour;
		vtable.dev_settitle = (psy_ui_fp_componentimp_dev_settitle) dev_settitle;
		vtable.dev_setfocus = (psy_ui_fp_componentimp_dev_setfocus) dev_setfocus;
		vtable.dev_hasfocus = (psy_ui_fp_componentimp_dev_hasfocus) dev_hasfocus;
		vtable.dev_platform = (psy_ui_fp_componentimp_dev_platform) dev_platform;
		vtable_initialized = 1;
	}
}

static void oncommand(psy_ui_CheckBox*, psy_ui_Component*,
	WPARAM wParam, LPARAM lParam);

// CheckBoxImp VTable
static void dev_settext(psy_ui_win_CheckBoxImp*, const char* text);
static void dev_text(psy_ui_win_CheckBoxImp*, char* text);
static void dev_check(psy_ui_win_CheckBoxImp*);
static void dev_disablecheck(psy_ui_win_CheckBoxImp*);
static int dev_checked(psy_ui_win_CheckBoxImp*);

static psy_ui_CheckBoxImpVTable checkboximp_vtable;
static int checkboximp_vtable_initialized = 0;

static void checkboximp_imp_vtable_init(psy_ui_win_CheckBoxImp* self)
{
	if (!checkboximp_vtable_initialized) {
		checkboximp_vtable.dev_settext = (psy_ui_fp_checkboximp_dev_settext) dev_settext;
		checkboximp_vtable.dev_text = (psy_ui_fp_checkboximp_dev_text) dev_text;
		checkboximp_vtable.dev_check = (psy_ui_fp_checkboximp_dev_check) dev_check;
		checkboximp_vtable.dev_disablecheck = (psy_ui_fp_checkboximp_dev_disablecheck) dev_disablecheck;
		checkboximp_vtable.dev_checked = (psy_ui_fp_checkboximp_dev_checked) dev_checked;
		checkboximp_vtable_initialized = 1;
	}
}

void psy_ui_win_checkboximp_init(psy_ui_win_CheckBoxImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_componentimp_init(&self->win_component_imp,
		component,
		parent,
		TEXT("BUTTON"),
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_FLAT,
		1);
	imp_vtable_init();
	self->imp.component_imp.vtable = &vtable;
	psy_ui_checkboximp_init(&self->imp);
	checkboximp_imp_vtable_init(self);
	self->imp.vtable = &checkboximp_vtable;
	psy_signal_connect(&self->win_component_imp.imp.signal_command, component, oncommand);
}

void psy_ui_win_checkboximp_init_multiline(psy_ui_win_CheckBoxImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_componentimp_init(&self->win_component_imp,
		component,
		parent,
		TEXT("BUTTON"),
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_MULTILINE | BS_FLAT,
		1);
	imp_vtable_init();
	self->imp.component_imp.vtable = &vtable;
	psy_ui_checkboximp_init(&self->imp);
	checkboximp_imp_vtable_init(self);
	self->imp.vtable = &checkboximp_vtable;
	psy_signal_connect(&self->win_component_imp.imp.signal_command, component, oncommand);
}

psy_ui_win_CheckBoxImp* psy_ui_win_checkboximp_alloc(void)
{
	return (psy_ui_win_CheckBoxImp*) malloc(sizeof(psy_ui_win_CheckBoxImp));
}

psy_ui_win_CheckBoxImp* psy_ui_win_checkboximp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_CheckBoxImp* rv;

	rv = psy_ui_win_checkboximp_alloc();
	if (rv) {
		psy_ui_win_checkboximp_init(rv, component, parent);
	}
	return rv;
}

psy_ui_win_CheckBoxImp* psy_ui_win_checkboximp_allocinit_multiline(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent)
{
	psy_ui_win_CheckBoxImp* rv;

	rv = psy_ui_win_checkboximp_alloc();
	if (rv) {
		psy_ui_win_checkboximp_init_multiline(rv, component, parent);
	}
	return rv;
}

void dev_settext(psy_ui_win_CheckBoxImp* self, const char* text)
{
	SetWindowText(self->win_component_imp.hwnd, text);		
}

void dev_text(psy_ui_win_CheckBoxImp* self, char* text)
{	
	GetWindowText(self->win_component_imp.hwnd, text, 256);
}

void dev_check(psy_ui_win_CheckBoxImp* self)
{
	if (!dev_checked(self)) {
		SendMessage(self->win_component_imp.hwnd, BM_SETCHECK, (WPARAM)BST_CHECKED,
			(LPARAM)0);
	}
}

void dev_disablecheck(psy_ui_win_CheckBoxImp* self)
{
	if (dev_checked(self)) {
		SendMessage(self->win_component_imp.hwnd, BM_SETCHECK, (WPARAM)0, (LPARAM)0);
	}
}

int dev_checked(psy_ui_win_CheckBoxImp* self)
{
	return SendMessage(self->win_component_imp.hwnd, BM_GETCHECK, (WPARAM)0,
		(LPARAM)0) != 0;
}

psy_ui_Size dev_preferredsize(psy_ui_win_CheckBoxImp* self, const psy_ui_Size* limits)
{
	psy_ui_Size rv;
	psy_ui_Graphics g;
	HDC hdc;
	HWND hwnd;
	char text[512];
	RECT rect = { 0 };
	const psy_ui_TextMetric* tm;

	hwnd = self->win_component_imp.hwnd;

	hdc = GetDC(NULL);	
	psy_ui_graphics_init(&g, hdc);
	//psy_ui_setfont(&g, font);
	dev_text(self, text);	
	tm = dev_textmetric(self);
	rect.left = 0;
	rect.right = (int)psy_ui_value_px(&limits->width, tm) - 40;
	DrawText(hdc, text, -1, &rect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);	
	psy_ui_graphics_dispose(&g);	
	ReleaseDC(NULL, hdc);
	rv.width = psy_ui_value_makepx(rect.right);
	rv.height = psy_ui_value_makepx(rect.bottom);
	return rv;
}

void oncommand(psy_ui_CheckBox* self, psy_ui_Component* sender,
	WPARAM wParam, LPARAM lParam)
{
	switch (HIWORD(wParam))
	{
	case BN_CLICKED:
	{
		if (self->signal_clicked.slots) {
			psy_signal_emit(&self->signal_clicked, self, 0);
		}
	}
	break;
	default:
		break;
	}
}

#endif

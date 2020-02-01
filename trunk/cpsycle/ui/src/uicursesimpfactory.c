// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_CURSES

#include "uicursesimpfactory.h"
#include "uiapp.h"
#include "uicursescomponentimp.h"

#include <stdlib.h>

// psy_ui_curses_ImpFactory

static struct psy_ui_BitmapImp* allocinit_bitmapimp(psy_ui_curses_ImpFactory*);
static struct psy_ui_GraphicsImp* allocinit_graphicsimp(psy_ui_curses_ImpFactory*, uintptr_t* platformdc);
static struct psy_ui_ComponentImp* allocinit_componentimp(psy_ui_curses_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ComponentImp* allocinit_frameimp(psy_ui_curses_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_EditImp* allocinit_editimp(psy_ui_curses_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_EditImp* allocinit_editimp_multiline(psy_ui_curses_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_LabelImp* allocinit_labelimp(psy_ui_curses_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ListBoxImp* allocinit_listboximp(psy_ui_curses_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ListBoxImp* allocinit_listboximp_multiselect(psy_ui_curses_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_CheckBoxImp* allocinit_checkboximp(psy_ui_curses_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);

// VTable init
static psy_ui_ImpFactoryVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_curses_ImpFactory* self)
{
	if (!vtable_initialized) {
		vtable = *self->imp.vtable;
		vtable.allocinit_bitmapimp = (psy_ui_fp_impfactory_allocinit_bitmapimp) allocinit_bitmapimp;
		vtable.allocinit_graphicsimp = (psy_ui_fp_impfactory_allocinit_graphicsimp) allocinit_graphicsimp;
		vtable.allocinit_componentimp = (psy_ui_fp_impfactory_allocinit_componentimp) allocinit_componentimp;
		vtable.allocinit_frameimp = (psy_ui_fp_impfactory_allocinit_frameimp) allocinit_frameimp;
		vtable.allocinit_editimp = (psy_ui_fp_impfactory_allocinit_editimp) allocinit_editimp;
		vtable.allocinit_editimp_multiline = (psy_ui_fp_impfactory_allocinit_editimp_multiline) allocinit_editimp_multiline;
		vtable.allocinit_labelimp = (psy_ui_fp_impfactory_allocinit_labelimp) allocinit_labelimp;
		vtable.allocinit_listboximp = (psy_ui_fp_impfactory_allocinit_listboximp) allocinit_listboximp;
		vtable.allocinit_listboximp_multiselect = (psy_ui_fp_impfactory_allocinit_listboximp_multiselect) allocinit_listboximp_multiselect;
		vtable.allocinit_checkboximp = (psy_ui_fp_impfactory_allocinit_checkboximp) allocinit_checkboximp;
		vtable_initialized = 1;
	}
}

void psy_ui_curses_impfactory_init(psy_ui_curses_ImpFactory* self)
{
	psy_ui_impfactory_init(&self->imp);
	vtable_init(self);
	self->imp.vtable = &vtable;
}

psy_ui_curses_ImpFactory* psy_ui_curses_impfactory_alloc(void)
{
	return (psy_ui_curses_ImpFactory*) malloc(sizeof(psy_ui_curses_ImpFactory));
}

psy_ui_curses_ImpFactory* psy_ui_curses_impfactory_allocinit(void)
{
	psy_ui_curses_ImpFactory* rv;

	rv = psy_ui_curses_impfactory_alloc();
	if (rv) {
		psy_ui_curses_impfactory_init(rv);
	}
	return rv;
}

struct psy_ui_BitmapImp* allocinit_bitmapimp(psy_ui_curses_ImpFactory* self)
{
	return 0;
}

struct psy_ui_GraphicsImp* allocinit_graphicsimp(psy_ui_curses_ImpFactory* self, uintptr_t* platformdc)
{
	return 0;
}

struct psy_ui_ComponentImp* allocinit_componentimp(psy_ui_curses_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return 0;
}

struct psy_ui_ComponentImp* allocinit_frameimp(psy_ui_curses_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_curses_ComponentImp* rv;
	rv = psy_ui_curses_componentimp_allocinit(
		component,
		parent ? parent->imp : 0,
		0, 0,
		80, 25);
	if (rv->hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ComponentImp*)rv;
}

struct psy_ui_EditImp* allocinit_editimp(psy_ui_curses_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return 0;
}

struct psy_ui_EditImp* allocinit_editimp_multiline(psy_ui_curses_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return 0;
}

struct psy_ui_LabelImp* allocinit_labelimp(psy_ui_curses_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return 0;
}

struct psy_ui_ListBoxImp* allocinit_listboximp(psy_ui_curses_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return 0;
}

struct psy_ui_ListBoxImp* allocinit_listboximp_multiselect(psy_ui_curses_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return 0;
}

struct psy_ui_CheckBoxImp* allocinit_checkboximp(psy_ui_curses_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return 0;
}

#endif

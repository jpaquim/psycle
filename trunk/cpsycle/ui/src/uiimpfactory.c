// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiimpfactory.h"
#include <stdlib.h>

struct psy_ui_BitmapImp* psy_ui_impfactory_allocinit_bitmapimp(psy_ui_ImpFactory* self)
{
	return self->vtable->allocinit_bitmapimp(self);
}

struct psy_ui_GraphicsImp* psy_ui_impfactory_allocinit_graphicsimp(psy_ui_ImpFactory* self, uintptr_t* platformdc)
{
	return self->vtable->allocinit_graphicsimp(self, platformdc);
}

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_componentimp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_componentimp(self, component, parent);
}

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_frameimp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_frameimp(self, component, parent);
}

struct psy_ui_LabelImp* psy_ui_impfactory_allocinit_labelimp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_labelimp(self, component, parent);
}

struct psy_ui_ListBoxImp* psy_ui_impfactory_allocinit_listboximp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_listboximp(self, component, parent);
}

struct psy_ui_ListBoxImp* psy_ui_impfactory_allocinit_listboximp_multiselect(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_listboximp_multiselect(self, component, parent);
}

struct psy_ui_EditImp* psy_ui_impfactory_allocinit_editimp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_editimp(self, component, parent);
}

struct psy_ui_EditImp* psy_ui_impfactory_allocinit_editimp_multiline(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_editimp_multiline(self, component, parent);
}

struct psy_ui_CheckBoxImp* psy_ui_impfactory_allocinit_checkboximp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_checkboximp(self, component, parent);
}

static struct psy_ui_BitmapImp* allocinit_bitmapimp(struct psy_ui_ImpFactory* self) { return 0; }
static struct psy_ui_GraphicsImp* allocinit_graphicsimp(struct psy_ui_ImpFactory* self, uintptr_t* platformdc) { return 0; }
static struct psy_ui_ComponentImp* allocinit_componentimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_ComponentImp* allocinit_frameimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_EditImp* allocinit_editimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_EditImp* allocinit_editimp_multiline(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_LabelImp* allocinit_labelimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_ListBoxImp* allocinit_listboximp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_ListBoxImp* allocinit_listboximp_multiselect(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_CheckBoxImp* allocinit_checkboximp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }

// VTable init
static psy_ui_ImpFactoryVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.allocinit_bitmapimp = vtable.allocinit_bitmapimp;
		vtable.allocinit_graphicsimp = allocinit_graphicsimp;
		vtable.allocinit_componentimp = allocinit_componentimp;
		vtable.allocinit_frameimp = allocinit_frameimp;
		vtable.allocinit_editimp = allocinit_editimp;
		vtable.allocinit_editimp_multiline = allocinit_editimp_multiline;
		vtable.allocinit_labelimp = allocinit_labelimp;
		vtable.allocinit_listboximp = allocinit_listboximp;
		vtable.allocinit_listboximp_multiselect = allocinit_listboximp_multiselect;
		vtable.allocinit_checkboximp = allocinit_checkboximp;
		vtable_initialized = 1;
	}
}

void psy_ui_impfactory_init(psy_ui_ImpFactory* self)
{
	vtable_init();
	self->vtable = &vtable;
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uinativeeditimp.h"
/* local */
#include "../../uicomponent.h"
#include "../../uiviewcomponentimp.h"
#include "../../uiapp.h"
#include "../../uiimpfactory.h"
/* portable */
#include "../../detail/portable.h"

/* EditImp VTable */
static void psy_ui_native_editimp_initialized(psy_ui_ComponentImp*);
static void dev_settext(psy_ui_native_EditImp*, const char* text);
static void dev_text(psy_ui_native_EditImp*, char* text);
static void dev_check(psy_ui_native_EditImp*);
static void dev_disablecheck(psy_ui_native_EditImp*);
static int dev_checked(psy_ui_native_EditImp*);

static psy_ui_EditImpVTable editimp_vtable;
static bool editimp_vtable_initialized = FALSE;

static void editimp_imp_vtable_init(psy_ui_native_EditImp* self)
{	
	if (!editimp_vtable_initialized) {
		editimp_vtable = *((psy_ui_EditImpVTable*)self->imp->extended_vtable);
		editimp_vtable.dev_settext =
			(psy_ui_fp_editimp_dev_settext)
			dev_settext;
		editimp_vtable.dev_text =
			(psy_ui_fp_editimp_dev_text)
			dev_text;		
		editimp_vtable_initialized = TRUE;
	}
	self->imp->extended_vtable = (void*)&editimp_vtable;
}

static psy_ui_Size dev_preferredsize(psy_ui_ComponentImp*,
	const psy_ui_Size* limits);
static void dev_draw(psy_ui_ComponentImp*, psy_ui_Graphics*);
static void dev_mousedown(psy_ui_ComponentImp*, psy_ui_MouseEvent*);
static void dev_dispose(psy_ui_ComponentImp*);

/* imp vtable */
static psy_ui_ComponentImpVTable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_native_EditImp* self)
{
	if (!vtable_initialized) {
		vtable = *self->imp->vtable;
		vtable.dev_initialized =
			(psy_ui_fp_componentimp_dev_initialized)
			psy_ui_native_editimp_initialized;
		vtable.dev_dispose =
			(psy_ui_fp_componentimp_dev_dispose)
			dev_dispose;
		vtable.dev_preferredsize =
			(psy_ui_fp_componentimp_dev_preferredsize)
			dev_preferredsize;
		vtable.dev_draw =
			(psy_ui_fp_componentimp_dev_draw)
			dev_draw;
		vtable.dev_mousedown =
			(psy_ui_fp_componentimp_dev_mouseevent)
			dev_mousedown;
		vtable_initialized = TRUE;
	}
	self->imp->vtable = &vtable;
}

static void onkeydown(psy_ui_native_EditImp*, psy_ui_Component* sender, psy_ui_KeyboardEvent*);
static void onfocus(psy_ui_native_EditImp*, psy_ui_Component* sender);
static void onfocuslost(psy_ui_native_EditImp*, psy_ui_Component* sender);
static void insertchar(psy_ui_native_EditImp*, char c);
static void deletechar(psy_ui_native_EditImp*);
static void removechar(psy_ui_native_EditImp*);
static char_dyn_t* lefttext(psy_ui_native_EditImp*, uintptr_t split);
static char_dyn_t* righttext(psy_ui_native_EditImp*, uintptr_t split);

void psy_ui_native_editimp_init(psy_ui_native_EditImp* self,
	psy_ui_Component* component,
	psy_ui_Component* parent,
	psy_ui_Component* view)
{	
	if (view) {
		self->imp = (psy_ui_ComponentImp*)
			psy_ui_viewcomponentimp_allocinit(
				component, parent, view, "",
				0, 0, 100, 100, 0, 0);
	} else {
		self->imp = psy_ui_impfactory_allocinit_componentimp(
			psy_ui_app_impfactory(psy_ui_app()), component, parent);
	}	
	vtable_init(self);	
	psy_ui_editimp_extend(self->imp);	
	editimp_imp_vtable_init(self);
	self->imp->extended_imp = (void*)self;
	self->component = component;
	self->text = NULL;	
	self->multiline = 0;	
	self->active = FALSE;
	self->cp = 0;
	self->ident = 5;
}

void psy_ui_native_editimp_init_multiline(psy_ui_native_EditImp* self,
	psy_ui_Component* component,
	psy_ui_Component* parent)
{
	self->imp = psy_ui_impfactory_allocinit_componentimp(
		psy_ui_app_impfactory(psy_ui_app()), component, parent);
	vtable_init(self);	
	psy_ui_editimp_extend(self->imp);
	self->imp->extended_imp = (void*)self;
	self->component = component;
	editimp_imp_vtable_init(self);		
	self->text = NULL;
	self->multiline = 1;
	self->active = FALSE;
	self->cp = 0;
	self->ident = 5;
}

void psy_ui_native_editimp_initialized(psy_ui_ComponentImp* context)
{	
	psy_ui_native_EditImp* self;

	self = (psy_ui_native_EditImp*)context->extended_imp;
	psy_signal_connect(&self->component->signal_keydown, self, onkeydown);
	psy_signal_connect(&self->component->signal_focus, self, onfocus);
	psy_signal_connect(&self->component->signal_focuslost, self, onfocuslost);
}

void dev_dispose(psy_ui_ComponentImp* context)
{
	psy_ui_native_EditImp* self;	

	self = (psy_ui_native_EditImp*)context->extended_imp;
	free(self->text);
	self->text = 0;
	free(self);
}

psy_ui_native_EditImp* psy_ui_native_editimp_alloc(void)
{
	return (psy_ui_native_EditImp*)malloc(sizeof(psy_ui_native_EditImp));
}

psy_ui_native_EditImp* psy_ui_native_editimp_allocinit(
	struct psy_ui_Component* component, struct psy_ui_Component* view,
	psy_ui_Component* parent)
{
	psy_ui_native_EditImp* rv;

	rv = psy_ui_native_editimp_alloc();
	if (rv) {
		psy_ui_native_editimp_init(rv, component, parent, view);
	}
	return rv;
}

psy_ui_native_EditImp* psy_ui_native_editimp_allocinit_multiline(
	struct psy_ui_Component* component,
	psy_ui_Component* parent)
{
	psy_ui_native_EditImp* rv;

	rv = psy_ui_native_editimp_alloc();
	if (rv) {
		psy_ui_native_editimp_init_multiline(rv, component, parent);
	}
	return rv;
}

void dev_settext(psy_ui_native_EditImp* self, const char* text)
{	
	psy_strreset(&self->text, text);	
	psy_ui_component_invalidate(self->component);
}

void dev_text(psy_ui_native_EditImp* self, char* text)
{	
	if (self->text) {
		psy_snprintf(text, 256, "%s", self->text);
	} else {
		text = '\0';
	}
}

void dev_draw(psy_ui_ComponentImp* context, psy_ui_Graphics* g)
{	
	psy_ui_native_EditImp* self;
	psy_ui_RealSize size;	
	const psy_ui_TextMetric* tm;	
		
	self = (psy_ui_native_EditImp*)context->extended_imp;
	psy_ui_component_draw(self->component, g, NULL);	
	tm = psy_ui_component_textmetric(self->component);
	size = psy_ui_component_scrollsize_px(self->component);	
	if (psy_strlen(self->text) > 0) {
		psy_ui_textout(g, self->ident, (size.height - tm->tmHeight) / 2,
			self->text, strlen(self->text));
	}
	if (self->active) {
		psy_ui_Size textsize;
		double x;

		textsize = psy_ui_textsize(g, self->text, self->cp);		
		x = psy_ui_value_px(&textsize.width, tm, NULL) + self->ident;
		psy_ui_drawline(g,
			psy_ui_realpoint_make(x, 0),
			psy_ui_realpoint_make(x, size.height));
	}
}

psy_ui_Size dev_preferredsize(psy_ui_ComponentImp* context, const psy_ui_Size* limit)
{
	psy_ui_native_EditImp* self;
	psy_ui_Size rv;
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;	

	self = (psy_ui_native_EditImp*)context->extended_imp;		
	if (!self->text) {
		return psy_ui_size_zero();
	}
	tm = psy_ui_component_textmetric(self->component);
	size = psy_ui_component_textsize(self->component, self->text);
	rv.width = psy_ui_value_make_px(psy_ui_value_px(&size.width, tm, NULL) + 5);
	rv.height = size.height;
	return rv;
}

void dev_mousedown(psy_ui_ComponentImp* context, psy_ui_MouseEvent* ev)
{
	psy_ui_native_EditImp* self;

	self = (psy_ui_native_EditImp*)context->extended_imp;	
	psy_ui_component_setfocus(self->component);	
}

void onfocus(psy_ui_native_EditImp* self, psy_ui_Component* sender)
{
	self->active = TRUE;
	psy_ui_component_invalidate(self->component);
}

void onfocuslost(psy_ui_native_EditImp* self, psy_ui_Component* sender)
{
	self->active = FALSE;
	psy_ui_component_invalidate(self->component);
}

void onkeydown(psy_ui_native_EditImp* self, psy_ui_Component* sender, psy_ui_KeyboardEvent* ev)
{
	switch (ev->keycode) {
	case psy_ui_KEY_LEFT:
		if (self->cp > 0) {
			--self->cp;
		}
		break;
	case psy_ui_KEY_RIGHT:
		if (self->cp < psy_strlen(self->text)) {
			++self->cp;
		}
		break;
	case psy_ui_KEY_UP:
		break;
	case psy_ui_KEY_DOWN:
		break;
	case psy_ui_KEY_HOME:
		self->cp = 0;
		break;
	case psy_ui_KEY_END:		
		self->cp = psy_strlen(self->text);		
		break;
	case psy_ui_KEY_BACK:
		deletechar(self);
		break;
	case psy_ui_KEY_DELETE:
		removechar(self);
		break;
	case psy_ui_KEY_SPACE:
		insertchar(self, ' ');
		break;
	case psy_ui_KEY_SHIFT:				
	case psy_ui_KEY_CONTROL:		
	case psy_ui_KEY_MENU:
		break;
	default:		
		if (ev->shift_key) {
			insertchar(self, ev->keycode);
		} else if (ev->keycode >= psy_ui_KEY_DIGIT0 && ev->keycode <= psy_ui_KEY_DIGIT9) {
			insertchar(self, ev->keycode);
		} else {
			insertchar(self, ev->keycode - 'A' + 'a');
		}
		break;
	}	
	psy_ui_component_invalidate(self->component);
}

void insertchar(psy_ui_native_EditImp* self, char c)
{
	char insert[2];
	char* left;
	char* right;

	insert[0] = c;
	insert[1] = '\0';
	left = lefttext(self, self->cp + 1);
	right = righttext(self, self->cp);
	left = psy_strcat_realloc(left, insert);
	if (psy_strlen(right) > 0) {
		left = psy_strcat_realloc(left, right);
	}
	psy_strreset(&self->text, left);
	free(left);
	left = NULL;
	free(right);
	right = NULL;
	++self->cp;
}

void deletechar(psy_ui_native_EditImp* self)
{	
	char* left;
	char* right;
	
	left = lefttext(self, self->cp);	
	right = righttext(self, self->cp);	
	if (psy_strlen(right) > 0) {
		left = psy_strcat_realloc(left, right);
	}
	psy_strreset(&self->text, left);
	free(left);
	left = NULL;
	free(right);
	right = NULL;
	if (self->cp > 0) {
		--self->cp;
	}
}

void removechar(psy_ui_native_EditImp* self)
{
	char* left;
	char* right;

	left = lefttext(self, self->cp + 1);
	right = righttext(self, self->cp + 1);
	if (psy_strlen(right) > 0) {
		left = psy_strcat_realloc(left, right);
	}
	psy_strreset(&self->text, left);
	free(left);
	left = NULL;
	free(right);
	right = NULL;	
}

char_dyn_t* lefttext(psy_ui_native_EditImp* self, uintptr_t split)
{
	char* rv;	

	rv = malloc(split + 1);
	if (split > 0) {
		psy_snprintf(rv, split, "%s", self->text);
	} else {
		rv[0] = '\0';
	}
	return rv;
}

char_dyn_t* righttext(psy_ui_native_EditImp* self, uintptr_t split)
{
	char* rv;	

	if (psy_strlen(self->text) > split) {
		uintptr_t num;

		num = psy_strlen(self->text) - split;
		rv = malloc(num + 1);
		psy_snprintf(rv, num + 1, "%s", self->text + split);
	} else {
		rv = NULL;
	}
	return rv;
}

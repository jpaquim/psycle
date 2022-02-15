/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "checkui.h"

/* host */
#include "styles.h"
#include "machineparamconfig.h"
/* ui */
#include <uiapp.h>
/* audio */
#include <machine.h>
#include <plugin_interface.h>
/* platform */
#include "../../detail/portable.h"

/* CheckUi */

/* prototypes */
static void checkui_ondraw(CheckUi*, psy_ui_Graphics*);
static void checkui_onpreferredsize(CheckUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void checkui_onmousedown(CheckUi*, psy_ui_MouseEvent*);
static void checkui_onmouseup(CheckUi*, psy_ui_MouseEvent*);
static void checkui_updateparam(CheckUi*);

/* vtable */
static psy_ui_ComponentVtable checkui_vtable;
static bool checkui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* checkui_vtable_init(CheckUi* self)
{
	assert(self);

	if (!checkui_vtable_initialized) {
		checkui_vtable = *(self->component.vtable);
		checkui_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			checkui_ondraw;
		checkui_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			checkui_onpreferredsize;
		checkui_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			checkui_onmousedown;
		checkui_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			checkui_onmouseup;
		checkui_vtable_initialized = TRUE;
	}
	return &checkui_vtable;
}

/* implementation */
void checkui_init(CheckUi* self, psy_ui_Component* parent,	
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	assert(self);			

	psy_ui_component_init(&self->component, parent, NULL);
	checkui_vtable_init(self);
	self->component.vtable = &checkui_vtable;
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);
	self->machine = machine;
	self->paramidx = paramidx;
	self->param = param;
	paramtweak_init(&self->paramtweak);
}

CheckUi* checkui_alloc(void)
{
	return (CheckUi*)malloc(sizeof(CheckUi));
}

CheckUi* checkui_allocinit(psy_ui_Component* parent,	
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	CheckUi* rv;

	rv = checkui_alloc();
	if (rv) {
		checkui_init(rv, parent, machine, paramidx, param);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void checkui_ondraw(CheckUi* self, psy_ui_Graphics* g)
{	
	double centery;
	psy_ui_RealRectangle r;
	char label[128];
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;	
	psy_ui_Style* style;

	if (!self->param || psy_audio_machineparam_normvalue(self->param) == 0.f) {
		style = psy_ui_style(STYLE_MACPARAM_CHECKOFF);
	} else {
		style = psy_ui_style(STYLE_MACPARAM_CHECKON);
	}	
	label[0] = '\0';
	checkui_updateparam(self);
	size = psy_ui_component_scrollsize_px(&self->component);
	centery = (size.height - style->background.size.height) / 2;	
	psy_ui_component_drawbackground_style(&self->component,
		g, style, psy_ui_realpoint_make(0.0, centery));	
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(20, 0),
		psy_ui_realsize_make(size.width - 20, size.height));
	if (self->param && !psy_audio_machineparam_name(self->param, label)) {
		if (!psy_audio_machineparam_label(self->param, label)) {
			psy_snprintf(label, 128, "%s", "");
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	centery = (size.height - tm->tmHeight) / 2;
	if (self->param) {
		psy_ui_textoutrectangle(g, psy_ui_realpoint_make(20, centery),
			psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
			r, label, psy_strlen(label));
	}
}

void checkui_onpreferredsize(CheckUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	checkui_updateparam(self);
	if (self->param) {
		if (psy_audio_machineparam_type(self->param) & MPF_SMALL) {
			psy_ui_size_setem(rv, PARAMWIDTH_SMALL, 1.0);
			return;
		}
	}
	psy_ui_size_setem(rv, PARAMWIDTH, 1.0);
}

void checkui_onmousedown(CheckUi* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 1) {
		paramtweak_begin(&self->paramtweak, self->machine, self->paramidx,
			self->param);		
		paramtweak_onmousedown(&self->paramtweak, ev);		
		psy_ui_component_capture(&self->component);		
	}
}

void checkui_onmouseup(CheckUi* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);
	if ((paramtweak_active(&self->paramtweak))) {
		paramtweak_end(&self->paramtweak);		
	}
}

void checkui_updateparam(CheckUi* self)
{
	if (self->machine && self->paramidx != psy_INDEX_INVALID) {
		self->param = psy_audio_machine_parameter(self->machine,
			self->paramidx);
	}
}

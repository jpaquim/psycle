/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "switchui.h"
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

/* SwitchUi */

/* prototypes */
static void switchui_ondraw(SwitchUi*, psy_ui_Graphics*);
static void switchui_onpreferredsize(SwitchUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void switchui_onmousedown(SwitchUi*, psy_ui_MouseEvent*);
static void switchui_onmouseup(SwitchUi*, psy_ui_MouseEvent*);
static void switchui_onmousemove(SwitchUi*, psy_ui_MouseEvent*);
static void switchui_updateparam(SwitchUi*);

/* vtable */
static psy_ui_ComponentVtable switchui_vtable;
static bool switchui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* switchui_vtable_init(SwitchUi* self)
{
	assert(self);

	if (!switchui_vtable_initialized) {
		switchui_vtable = *(self->component.vtable);
		switchui_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			switchui_ondraw;
		switchui_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			switchui_onpreferredsize;
		switchui_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			switchui_onmousedown;
		switchui_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			switchui_onmouseup;
		switchui_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			switchui_onmousemove;
		switchui_vtable_initialized = TRUE;
	}
	return &switchui_vtable;
}

/* implementation */
void switchui_init(SwitchUi* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	assert(self);	

	psy_ui_component_init(&self->component, parent, NULL);
	switchui_vtable_init(self);
	self->component.vtable = &switchui_vtable;
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);	
	self->machine = machine;
	self->paramidx = paramidx;
	self->param = param;	
	paramtweak_init(&self->paramtweak);
}

SwitchUi* switchui_alloc(void)
{
	return (SwitchUi*)malloc(sizeof(SwitchUi));
}

SwitchUi* switchui_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	SwitchUi* rv;

	rv = switchui_alloc();
	if (rv) {
		switchui_init(rv, parent, machine, paramidx, param);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void switchui_ondraw(SwitchUi* self, psy_ui_Graphics* g)
{	
	psy_ui_RealSize size;				
	psy_ui_RealRectangle r;
	char label[128];
	psy_ui_Style* top_style;
	psy_ui_Style* bottom_style;	
	psy_ui_Style* style;	
	
	top_style = psy_ui_style(STYLE_MACPARAM_TOP);
	bottom_style = psy_ui_style(STYLE_MACPARAM_BOTTOM);		
	if (!self->param || psy_audio_machineparam_normvalue(self->param) == 0.f) {
		style = psy_ui_style(STYLE_MACPARAM_SWITCHOFF);
	} else {
		style = psy_ui_style(STYLE_MACPARAM_SWITCHON);
	}
	switchui_updateparam(self);
	size = psy_ui_component_scrollsize_px(&self->component);
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, 0),
		psy_ui_realsize_make(size.width, size.height));
	psy_ui_drawsolidrectangle(g, r, bottom_style->background.colour);		
	psy_ui_component_drawbackground_style(&self->component,
		g, style, psy_ui_realpoint_zero());	
	psy_snprintf(label, 128, "%s", "");
	if (self->param && !psy_audio_machineparam_name(self->param, label)) {
		if (!psy_audio_machineparam_label(self->param, label)) {
			psy_snprintf(label, 128, "%s", "");
		}
	}
	psy_ui_setbackgroundcolour(g, top_style->background.colour);
	psy_ui_settextcolour(g, top_style->colour);
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(	
			style->background.size.width, 0),
		psy_ui_realsize_make(
			size.width - style->background.size.width,
			size.height / 2));
	psy_ui_textoutrectangle(g,
		psy_ui_realpoint_make(
			style->background.size.width, 0.0),
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, label, strlen(label));	
}

void switchui_onpreferredsize(SwitchUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	switchui_updateparam(self);
	if (self->param) {
		if (psy_audio_machineparam_type(self->param) & MPF_SMALL) {
			psy_ui_size_setem(rv, PARAMWIDTH_SMALL, 2.0);
			return;
		}
	}
	psy_ui_size_setem(rv, PARAMWIDTH, 2.0);
}

void switchui_onmousedown(SwitchUi* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 1) {
		paramtweak_begin(&self->paramtweak, NULL, psy_INDEX_INVALID, self->param);		
		paramtweak_onmousedown(&self->paramtweak, ev);
		psy_ui_component_capture(&self->component);		
	}
}

void switchui_onmousemove(SwitchUi* self, psy_ui_MouseEvent* ev)
{
	if ((paramtweak_active(&self->paramtweak))) {
		paramtweak_onmousemove(&self->paramtweak, ev);
		psy_ui_component_invalidate(&self->component);
	}
}

void switchui_onmouseup(SwitchUi* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);
	if ((paramtweak_active(&self->paramtweak))) {
		paramtweak_end(&self->paramtweak);
		psy_ui_component_invalidate(&self->component);
	}
}

void switchui_updateparam(SwitchUi* self)
{
	if (self->machine && self->paramidx != psy_INDEX_INVALID) {
		self->param = psy_audio_machine_parameter(self->machine,
			self->paramidx);
	}
}

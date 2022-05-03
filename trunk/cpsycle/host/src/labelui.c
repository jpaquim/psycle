/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "labelui.h"
/* host */
#include "machineparamconfig.h"
#include "styles.h"
/* ui */
#include <uiapp.h>
/* audio */
#include <machine.h>
#include <plugin_interface.h>
/* platform */
#include "../../detail/portable.h"


/* LabelUi */
/* prototypes */
static void labelui_ondraw(LabelUi*, psy_ui_Graphics*);
static void labelui_invalidate(LabelUi*);
static void labelui_onpreferredsize(LabelUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void labelui_updateparam(LabelUi*);

/* vtable */
static psy_ui_ComponentVtable labelui_vtable;
static bool labelui_vtable_initialized = FALSE;

static void labelui_vtable_init(LabelUi* self)
{
	assert(self);

	if (!labelui_vtable_initialized) {
		labelui_vtable = *(self->component.vtable);
		labelui_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			labelui_ondraw;
		labelui_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			labelui_onpreferredsize;		
		labelui_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &labelui_vtable);
}

/* implementation */
void labelui_init(LabelUi* self, psy_ui_Component* parent,	
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	assert(self);	

	psy_ui_component_init(&self->component, parent, NULL);
	labelui_vtable_init(self);
	psy_ui_component_set_style_type(&self->component, STYLE_MV_LABEL);
	self->param = param;
	self->machine = machine;
	self->paramidx = paramidx;
}

LabelUi* labelui_alloc(void)
{
	return (LabelUi*)malloc(sizeof(LabelUi));
}

LabelUi* labelui_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	LabelUi* rv;

	rv = labelui_alloc();
	if (rv) {
		labelui_init(rv, parent, machine, paramidx, param);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void labelui_ondraw(LabelUi* self, psy_ui_Graphics* g)
{		
	double half;
	psy_ui_RealRectangle r;
	char str[128];
	psy_ui_RealSize size;
	psy_ui_Style* title_style;
	psy_ui_Style* bottom_style;

	title_style = psy_ui_style(STYLE_MACPARAM_TITLE);
	bottom_style = psy_ui_style(STYLE_MACPARAM_BOTTOM);
	str[0] = '\0';
	labelui_updateparam(self);
	size = psy_ui_component_scroll_size_px(&self->component);
	half = size.height / 2;
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0.0, 0.0),
		psy_ui_realsize_make(size.width, half));
	psy_ui_setbackgroundcolour(g, title_style->background.colour);
	psy_ui_settextcolour(g, title_style->colour);
	if (self->param) {
		if (!psy_audio_machineparam_name(self->param, str)) {
			if (!psy_audio_machineparam_label(self->param, str)) {
				psy_snprintf(str, 128, "%s", "");
			}
		}
	}
	psy_ui_textoutrectangle(g,
		psy_ui_realpoint_zero(), psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, psy_strlen(str));
	if (self->param && psy_audio_machineparam_describe(self->param, str) == FALSE) {
		psy_snprintf(str, 128, "%s", "");
	}
	psy_ui_setbackgroundcolour(g, bottom_style->background.colour);
	psy_ui_settextcolour(g, bottom_style->colour);
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, half),
		psy_ui_realsize_make(size.width, half));
	psy_ui_textoutrectangle(g, psy_ui_realpoint_make(0, half),
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, psy_strlen(str));
}

void labelui_onpreferredsize(LabelUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	labelui_updateparam(self);
	if (self->param) {
		if (psy_audio_machineparam_type(self->param) & MPF_SMALL) {
			psy_ui_size_setem(rv, PARAMWIDTH_SMALL, 2.0);
			return;
		}
	}
	psy_ui_size_setem(rv, PARAMWIDTH, 2.0);
}

void labelui_updateparam(LabelUi* self)
{
	if (self->machine && self->paramidx != psy_INDEX_INVALID) {
		self->param = psy_audio_machine_parameter(self->machine,
			self->paramidx);
	}
}

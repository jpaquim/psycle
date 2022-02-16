/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "headerui.h"
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

/* HeaderUi */
/* prototypes */
static void headerui_ondraw(HeaderUi*, psy_ui_Graphics*);
static void headerui_onpreferredsize(HeaderUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void headerui_updateparam(HeaderUi*);

/* vtable */
static psy_ui_ComponentVtable headerui_vtable;
static bool headerui_vtable_initialized = FALSE;

static void headerui_vtable_init(HeaderUi* self)
{
	assert(self);

	if (!headerui_vtable_initialized) {
		headerui_vtable = *(self->component.vtable);	
		headerui_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			headerui_ondraw;
		headerui_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			headerui_onpreferredsize;		
		headerui_vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(&self->component, &headerui_vtable);	
}

/* implementation */
void headerui_init(HeaderUi* self, psy_ui_Component* parent,	
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	assert(self);	

	psy_ui_component_init(&self->component, parent, NULL);
	headerui_vtable_init(self);	
	psy_ui_component_setstyletype(&self->component, STYLE_MV_HEADER);
	self->machine = machine;
	self->paramidx = paramidx;
	self->param = param;	
}

HeaderUi* headerui_alloc(void)
{
	return (HeaderUi*)malloc(sizeof(HeaderUi));
}

HeaderUi* headerui_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	HeaderUi* rv;

	rv = headerui_alloc();
	if (rv) {
		headerui_init(rv, parent, machine, paramidx, param);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void headerui_ondraw(HeaderUi* self, psy_ui_Graphics* g)
{		
	double half;
	double quarter;
	psy_ui_RealRectangle r;
	char str[128];
	psy_ui_RealSize size;
	psy_ui_Style* top_style;
	psy_ui_Style* bottom_style;	
	psy_ui_Style* title_style;

	top_style = psy_ui_style(STYLE_MACPARAM_TOP);
	bottom_style = psy_ui_style(STYLE_MACPARAM_BOTTOM);	
	title_style = psy_ui_style(STYLE_MACPARAM_TITLE);
	headerui_updateparam(self);
	size = psy_ui_component_size_px(&self->component);
	half = size.height / 2;
	quarter = half / 2;
	psy_ui_realrectangle_init_all(&r, psy_ui_realpoint_zero(), size);
	psy_ui_drawsolidrectangle(g, r, top_style->background.colour);
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, half + quarter),
		psy_ui_realsize_make(size.width, quarter));
	psy_ui_drawsolidrectangle(g, r, bottom_style->background.colour);
	if (!psy_audio_machineparam_name(self->param, str)) {
		if (!psy_audio_machineparam_label(self->param, str)) {
			psy_snprintf(str, 128, "%s", "");
		}
	}
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, quarter),
		psy_ui_realsize_make(size.width, half));
	psy_ui_setbackgroundcolour(g, title_style->background.colour);
	psy_ui_settextcolour(g, title_style->colour);
	/* todo font_bold */
	psy_ui_textoutrectangle(g, psy_ui_realpoint_make(0, quarter),
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r, str,
		psy_strlen(str));
}

void headerui_onpreferredsize(HeaderUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	headerui_updateparam(self);
	if (self->param) {
		if (psy_audio_machineparam_type(self->param) & MPF_SMALL) {
			psy_ui_size_setem(rv, PARAMWIDTH_SMALL, 2.0);
			return;
		}
	}
	psy_ui_size_setem(rv, PARAMWIDTH, 2.0);
}

void headerui_updateparam(HeaderUi* self)
{
	if (self->machine && self->paramidx != psy_INDEX_INVALID) {
		self->param = psy_audio_machine_parameter(self->machine,
			self->paramidx);
	}
}

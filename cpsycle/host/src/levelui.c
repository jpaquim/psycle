/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "levelui.h"
/* host */
#include "styles.h"
/* ui */
#include <uiapp.h>
/* audio */
#include <machine.h>

/* LevelUi */

/* prototypes */
static void levelui_ondraw(LevelUi*, psy_ui_Graphics*);
static void levelui_vumeterdraw(LevelUi*, psy_ui_Graphics*,
	psy_ui_RealPoint topleft, double value);
static void levelui_updateparam(LevelUi*);

/* vtable */
static psy_ui_ComponentVtable levelui_vtable;
static bool levelui_vtable_initialized = FALSE;

static void levelui_vtable_init(LevelUi* self)
{
	assert(self);

	if (!levelui_vtable_initialized) {
		levelui_vtable = *(self->component.vtable);
		levelui_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			levelui_ondraw;		
		levelui_vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(&self->component, &levelui_vtable);
}

/* implementation */
void levelui_init(LevelUi* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	assert(self);		

	psy_ui_component_init(&self->component, parent, NULL);
	levelui_vtable_init(self);	
	psy_ui_component_set_style_type(&self->component, STYLE_MV_LEVEL);
	self->machine = machine;
	self->paramidx = paramidx;
	self->param = param;	
}

LevelUi* levelui_alloc(void)
{
	return (LevelUi*)malloc(sizeof(LevelUi));
}

LevelUi* levelui_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	LevelUi* rv;

	rv = levelui_alloc();
	if (rv) {
		levelui_init(rv, parent, machine, paramidx, param);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void levelui_ondraw(LevelUi* self, psy_ui_Graphics* g)
{		
	levelui_updateparam(self);
	if (self->param) {
		levelui_vumeterdraw(self, g, psy_ui_realpoint_zero(),
			(double)psy_audio_machineparam_normvalue(self->param));
	} else {
		levelui_vumeterdraw(self, g, psy_ui_realpoint_zero(), 0.0);
	}
}

void levelui_vumeterdraw(LevelUi* self, psy_ui_Graphics* g,
	psy_ui_RealPoint topleft, double value)
{
	double ypos;
	psy_ui_Style* vuoff_style;	
	psy_ui_Style* vuon_style;
	
	vuoff_style = psy_ui_style(STYLE_MACPARAM_VUOFF);
	vuon_style = psy_ui_style(STYLE_MACPARAM_VUON);
	if (value < 0.0) value = 0.0;
	if (value > 1.0) value = 1.0;
	ypos = (1.0 - value) * vuoff_style->background.size.height;
	psy_ui_drawbitmap(g, &vuoff_style->background.bitmap,
		psy_ui_realrectangle_make(
			topleft,
			psy_ui_realsize_make(
				vuoff_style->background.size.width,
				ypos)),
		psy_ui_realpoint_make(
			-vuoff_style->background.position.x,
			-vuoff_style->background.position.y));		
	psy_ui_drawbitmap(g, &vuoff_style->background.bitmap,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(topleft.x, topleft.y + ypos),
			psy_ui_realsize_make(
				vuoff_style->background.size.width,
				vuoff_style->background.size.height - ypos)),
		psy_ui_realpoint_make(
			-vuon_style->background.position.x,
			-vuon_style->background.position.y + ypos));
}

void levelui_updateparam(LevelUi* self)
{
	if (self->machine && self->paramidx != psy_INDEX_INVALID) {
		self->param = psy_audio_machine_parameter(self->machine,
			self->paramidx);
	}
}

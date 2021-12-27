/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "effectui.h"
/* host */
#include "skingraphics.h"
#include "paramview.h"
#include "wireview.h"
/* audio */
#include <exclusivelock.h>
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

/* prototypes */
static void effectui_dispose(EffectUi*);
static void effectui_initsize(EffectUi*);
static int effectui_hittestpan(EffectUi*, psy_ui_RealPoint, double* dx);
static bool effectui_hittestcoord(EffectUi*, psy_ui_RealPoint, SkinCoord*);
static void effectui_ondraw(EffectUi*, psy_ui_Graphics*);
static void effectui_draweditname(EffectUi*, psy_ui_Graphics*);
static void effectui_drawbackground(EffectUi*, psy_ui_Graphics*);
static void effectui_drawpanning(EffectUi*, psy_ui_Graphics*);
static void effectui_drawmute(EffectUi*, psy_ui_Graphics*);
static void effectui_drawbypassed(EffectUi*, psy_ui_Graphics*);
static psy_dsp_amp_t effectui_panvalue(EffectUi*, double dx, uintptr_t slot);
static void effectui_onmousedown(EffectUi*, psy_ui_MouseEvent*);
static void effectui_onmouseup(EffectUi*, psy_ui_MouseEvent*);
static void effectui_onmousemove(EffectUi*, psy_ui_MouseEvent*);
static void effectui_onmousedoubleclick(EffectUi*, psy_ui_MouseEvent*);
static void effectui_drawvu(EffectUi*, psy_ui_Graphics*);
static void effectui_onframedestroyed(EffectUi*, psy_ui_Component* sender);
static void effectui_move(EffectUi*, psy_ui_Point topleft);
static void effectui_updatevolumedisplay(EffectUi*);
static void effectui_invalidate(EffectUi*);
static void effectui_onshowparameters(EffectUi*, Workspace* sender,
	uintptr_t slot);
static void effectui_showparameters(EffectUi*, psy_ui_Component* parent);
static void effectui_onpreferredsize(EffectUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
/* vtable */
static psy_ui_ComponentVtable effectui_vtable;
static psy_ui_ComponentVtable effectui_super_vtable;
static bool effectui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* effectui_vtable_init(EffectUi* self)
{
	assert(self);

	if (!effectui_vtable_initialized) {
		effectui_vtable = *(self->component.vtable);
		effectui_super_vtable = effectui_vtable;
		effectui_vtable.dispose =
			(psy_ui_fp_component_dispose)
			effectui_dispose;
		effectui_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			effectui_ondraw;
		effectui_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			effectui_onmousedown;
		effectui_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			effectui_onmouseup;
		effectui_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			effectui_onmousemove;
		effectui_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			effectui_onmousedoubleclick;
		effectui_vtable.move =
			(psy_ui_fp_component_move)
			effectui_move;
		effectui_vtable.invalidate =
			(psy_ui_fp_component_invalidate)
			effectui_invalidate;
		effectui_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			effectui_onpreferredsize;
		effectui_vtable_initialized = TRUE;
	}
	return &effectui_vtable;
}

/* implementation */
void effectui_init(EffectUi* self, psy_ui_Component* parent,
	uintptr_t slot, MachineViewSkin* skin, ParamViews* paramviews,
	Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(workspace->song);	

	psy_ui_component_init(&self->component, parent, NULL);	
	effectui_vtable_init(self);
	self->component.vtable = &effectui_vtable;
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	machineuicommon_init(&self->intern, &self->component, slot, skin,
		paramviews, workspace);
	self->intern.coords = &skin->effect;	
	self->intern.font = skin->effect_fontcolour;
	self->intern.bgcolour = psy_ui_colour_make(0x003E2f25);
	effectui_initsize(self);
	psy_signal_connect(&workspace->signal_showparameters, self,
		effectui_onshowparameters);	
}

void effectui_setdrawmode(EffectUi* self, MachineUiMode drawmode)
{
	switch (drawmode) {
		case MACHINEUIMODE_DRAW:
			self->intern.drawmode = drawmode;
			psy_ui_component_setbackgroundmode(&self->component,
				psy_ui_SETBACKGROUND);
			psy_ui_component_setbackgroundcolour(&self->component,
				self->intern.bgcolour);
			break;
		case MACHINEUIMODE_DRAWSMALL:
			self->intern.drawmode = drawmode;
			psy_ui_component_setbackgroundmode(&self->component,
				psy_ui_SETBACKGROUND);
			psy_ui_component_setbackgroundcolour(&self->component,
				self->intern.bgcolour);
			break;
		case MACHINEUIMODE_BITMAP:
		default:
			self->intern.drawmode = drawmode;
			psy_ui_component_setbackgroundmode(&self->component,
				psy_ui_NOBACKGROUND);					
			self->intern.drawmode = MACHINEUIMODE_BITMAP;
			break;
	}
}

void effectui_dispose(EffectUi* self)
{
	assert(self);
	
	free(self->intern.restorename);
	effectui_super_vtable.dispose(&self->component);
}

void effectui_initsize(EffectUi* self)
{
	psy_ui_RealPoint topleft;
	psy_ui_RealSize size;

	size = psy_ui_realrectangle_size(&self->intern.coords->background.dest);
	if (self->intern.machine && self->intern.machinepos) {
		psy_audio_machine_position(self->intern.machine, &topleft.x, &topleft.y);
	} else {
		topleft = psy_ui_realpoint_zero();
	}
	psy_ui_component_setposition(&self->component,
		psy_ui_rectangle_make(
			psy_ui_point_make_px(topleft.x, topleft.y),
			psy_ui_size_make_px(size.width, size.height)));
	vudisplay_init(&self->intern.vu, self->intern.skin, self->intern.coords);	
}

void effectui_move(EffectUi* self, psy_ui_Point topleft)
{
	assert(self);

	effectui_super_vtable.move(&self->component, topleft);
	machineuicommon_move(&self->intern, topleft);	
}

void effectui_ondraw(EffectUi* self, psy_ui_Graphics* g)
{
	assert(self);

	effectui_drawbackground(self, g);
	if (!machineui_vuupdate()) {
		effectui_draweditname(self, g);		
		effectui_drawpanning(self, g);		
	}
	effectui_drawmute(self, g);
	effectui_drawbypassed(self, g);
	if (self->intern.skin->drawvumeters) {
		effectui_drawvu(self, g);
	}
}

void effectui_drawbackground(EffectUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (self->intern.drawmode == MACHINEUIMODE_BITMAP &&
			!psy_ui_bitmap_empty(&self->intern.skin->skinbmp)) {
		skin_blitcoord(g, &self->intern.skin->skinbmp, psy_ui_realpoint_zero(),
			&self->intern.coords->background);
	} else {		
		psy_ui_drawsolidrectangle(g, self->intern.coords->background.dest,
			self->intern.bgcolour);		
	}	
}

void effectui_draweditname(EffectUi* self, psy_ui_Graphics* g)
{	
	char editname[130];

	assert(self);

	editname[0] = '\0';
	if (psy_audio_machine_editname(self->intern.machine)) {
		if (self->intern.skin->drawmachineindexes) {
			psy_snprintf(editname, 130, "%.2X:%s", (int)self->intern.slot,
				psy_audio_machine_editname(self->intern.machine));
		} else {
			psy_snprintf(editname, 130, "%s",
				psy_audio_machine_editname(self->intern.machine));
		}
	}
	if (psy_strlen(editname) > 0) {
		psy_ui_settextcolour(g, self->intern.font);
		psy_ui_textoutrectangle(g,
			psy_ui_realrectangle_topleft(&self->intern.coords->name.dest),
			psy_ui_ETO_CLIPPED, self->intern.coords->name.dest, editname,
			psy_strlen(editname));
	}	
}

void effectui_drawpanning(EffectUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (self->intern.drawmode == MACHINEUIMODE_BITMAP) {
		skin_blitcoord(g, &self->intern.skin->skinbmp,
			psy_ui_realpoint_make(
				skincoord_position(&self->intern.coords->pan,
					psy_audio_machine_panning(self->intern.machine)),
				0),
			&self->intern.coords->pan);
	}
}

void effectui_drawmute(EffectUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (psy_audio_machine_muted(self->intern.machine)) {
		if (self->intern.drawmode == MACHINEUIMODE_BITMAP) {
			skin_blitcoord(g, &self->intern.skin->skinbmp,
				psy_ui_realpoint_zero(),
				&self->intern.coords->mute);
		}
	}
}

void effectui_drawbypassed(EffectUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (psy_audio_machine_bypassed(self->intern.machine)) {
		if (self->intern.drawmode == MACHINEUIMODE_BITMAP) {
			skin_blitcoord(g, &self->intern.skin->skinbmp,
				psy_ui_realpoint_zero(),
				&self->intern.coords->bypass);
		}
	}
}

void effectui_drawvu(EffectUi* self, psy_ui_Graphics* g)
{
	assert(self);
	
	effectui_updatevolumedisplay(self);
	if (self->intern.drawmode == MACHINEUIMODE_BITMAP) {
		vudisplay_draw(&self->intern.vu, g);
	}
}

void effectui_updatevolumedisplay(EffectUi* self)
{
	if (self->intern.machine) {
		vudisplay_update(&self->intern.vu,
			psy_audio_machine_buffermemory(self->intern.machine));
	}
}

void effectui_showparameters(EffectUi* self, psy_ui_Component* parent)
{
	if (self->intern.paramviews) {
		paramviews_show(self->intern.paramviews, self->intern.slot);
	}	
}

void effectui_onmousedown(EffectUi* self, psy_ui_MouseEvent* ev)
{	
	if (ev->button == 1) {
		if (self->intern.slot != psy_audio_machines_selected(self->intern.machines)) {
			if (ev->ctrl_key) {
				psy_audio_machineselection_select(&self->intern.machines->selection,
					psy_audio_machineindex_make(self->intern.slot));
			} else {
				psy_audio_machines_select(self->intern.machines, self->intern.slot);
			}
		}
		if (effectui_hittestcoord(self, ev->pt,
			&self->intern.skin->effect.bypass)) {
			if (psy_audio_machine_bypassed(self->intern.machine)) {
				psy_audio_machine_unbypass(self->intern.machine);
			} else {
				psy_audio_machine_bypass(self->intern.machine);
			}
			psy_ui_mouseevent_stop_propagation(ev);
		} else if (effectui_hittestcoord(self, ev->pt,
			&self->intern.skin->generator.mute) ||
			effectui_hittestcoord(self, ev->pt,
				&self->intern.skin->effect.mute)) {
			if (psy_audio_machine_muted(self->intern.machine)) {
				psy_audio_machine_unmute(self->intern.machine);
			} else {
				psy_audio_machine_mute(self->intern.machine);
			}
			psy_ui_mouseevent_stop_propagation(ev);
		} else if (effectui_hittestpan(self, ev->pt, &self->intern.mx)) {
			self->intern.dragmode = MACHINEVIEW_DRAG_PAN;
			psy_ui_mouseevent_stop_propagation(ev);
		}
		if (!ev->event.bubbles) {
			psy_ui_component_invalidate(&self->component);
		}
	}
}

bool effectui_hittestcoord(EffectUi* self, psy_ui_RealPoint pt,
	SkinCoord* coord)
{
	assert(self);

	return psy_ui_realrectangle_intersect(&coord->dest, pt);	
}

void effectui_onmousemove(EffectUi* self, psy_ui_MouseEvent* ev)
{
	if (self->intern.dragmode == MACHINEVIEW_DRAG_PAN) {
		psy_audio_machine_setpanning(self->intern.machine,
			effectui_panvalue(self, ev->pt.x, self->intern.slot));
		psy_ui_component_invalidate(&self->component);
	}
}

int effectui_hittestpan(EffectUi* self, psy_ui_RealPoint pt, double* dx)
{
	psy_ui_RealRectangle r;
	double offset;
	
	offset = psy_audio_machine_panning(self->intern.machine) *
		self->intern.coords->pan.range;
	r = skincoord_destposition(&self->intern.coords->pan);
	psy_ui_realrectangle_move(&r, offset, 0);
	*dx = pt.x - r.left;
	return psy_ui_realrectangle_intersect(&r, pt);			
}

psy_dsp_amp_t effectui_panvalue(EffectUi* self, double dx, uintptr_t slot)
{
	psy_dsp_amp_t rv;
	MachineCoords* coords;
	
	rv = 0.f;
	coords = self->intern.coords;
	if (coords && coords->pan.range != 0) {
		rv = (psy_dsp_amp_t)(
			(dx - coords->pan.dest.left - (double)self->intern.mx) /
			(double)coords->pan.range);
	}
	return rv;
}

void effectui_onmouseup(EffectUi* self, psy_ui_MouseEvent* ev)
{
	self->intern.dragmode = MACHINEVIEW_DRAG_NONE;
}

void effectui_onmousedoubleclick(EffectUi* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		psy_ui_RealPoint dragpt;

		if (effectui_hittestcoord(self, ev->pt,
				&self->intern.skin->effect.bypass) ||
			effectui_hittestcoord(self, ev->pt,
				 &self->intern.skin->generator.mute) ||
			effectui_hittestcoord(self, ev->pt,
				&self->intern.skin->effect.mute) ||
			effectui_hittestpan(self, ev->pt, &dragpt.x)) {
		} else {
			if (self->component.view) {
				effectui_showparameters(self, self->component.view);
			}
		}
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void effectui_invalidate(EffectUi* self)
{
	if (machineui_vuupdate()) {		
		psy_ui_component_invalidaterect(&self->component,
			self->intern.coords->vu0.dest);
		psy_ui_component_invalidaterect(&self->component,
			self->intern.coords->bypass.dest);
		psy_ui_component_invalidaterect(&self->component,
			self->intern.coords->mute.dest);
	} else {
		effectui_super_vtable.invalidate(&self->component);
	}
}

void effectui_onshowparameters(EffectUi* self, Workspace* sender,
	uintptr_t slot)
{
	if (slot == self->intern.slot) {
		if (self->component.view) {
			effectui_showparameters(self, self->component.view);
		}
	}
}

void effectui_onpreferredsize(EffectUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	if (self->intern.drawmode == MACHINEUIMODE_DRAWSMALL) {
		*rv = psy_ui_size_make(
			psy_ui_value_make_px(self->intern.coords->background.dest.right),
			psy_ui_value_make_eh(1.0));				
	} else {
		psy_ui_size_setreal(rv,
			psy_ui_realrectangle_size(&
				self->intern.coords->background.dest));
	}
}

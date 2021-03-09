// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "generatorui.h"
// host
#include "skingraphics.h"
#include "paramview.h"
#include "wireview.h"
#include "masterui.h"
// audio
#include <exclusivelock.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// GeneratorUi
static void generatorui_dispose(GeneratorUi*);
static void generatorui_initsize(GeneratorUi*);
static int generatorui_hittestpan(GeneratorUi*, psy_ui_RealPoint, double* dx);
static bool generatorui_hittestcoord(GeneratorUi*, psy_ui_RealPoint,
	SkinCoord*);
static void generatorui_ondraw(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_draweditname(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_drawbackground(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_drawpanning(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_drawmute(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_drawsoloed(GeneratorUi*, psy_ui_Graphics*);
static bool generatorui_hittesteditname(GeneratorUi*, psy_ui_RealPoint);
static psy_ui_RealRectangle generatorui_coordposition(GeneratorUi*, SkinCoord*);
static void generatorui_oneditchange(GeneratorUi*, psy_ui_Edit* sender);
static void generatorui_oneditfocuslost(GeneratorUi*, psy_ui_Component* sender);
static psy_dsp_amp_t generatorui_panvalue(GeneratorUi*, double dx, uintptr_t slot);
static void generatorui_editname(GeneratorUi*, psy_ui_Edit*,
	psy_ui_RealPoint scrolloffset);
static void generatorui_onmousedown(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_onmouseup(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_onmousemove(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_onmousedoubleclick(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_oneditkeydown(GeneratorUi*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void generatorui_drawvu(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_onframedestroyed(GeneratorUi*, psy_ui_Component* sender);
static void generatorui_move(GeneratorUi*, psy_ui_Point topleft);
static void generatorui_updatevolumedisplay(GeneratorUi*);
static void generatorui_invalidate(GeneratorUi*);
static void generatorui_onshowparameters(GeneratorUi*, Workspace* sender,
	uintptr_t slot);
static void generatorui_showparameters(GeneratorUi*, psy_ui_Component* parent);
static void generatorui_onpreferredsize(GeneratorUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable generatorui_vtable;
static psy_ui_ComponentVtable generatorui_super_vtable;
static bool generatorui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* generatorui_vtable_init(GeneratorUi* self)
{
	assert(self);

	if (!generatorui_vtable_initialized) {
		generatorui_vtable = *(self->component.vtable);
		generatorui_super_vtable = generatorui_vtable;
		generatorui_vtable.dispose = (psy_ui_fp_component_dispose)generatorui_dispose;
		generatorui_vtable.ondraw = (psy_ui_fp_component_ondraw)generatorui_ondraw;
		generatorui_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)generatorui_onmousedown;
		generatorui_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)generatorui_onmouseup;
		generatorui_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)generatorui_onmousemove;
		generatorui_vtable.onmousedoubleclick = (psy_ui_fp_component_onmouseevent)
			generatorui_onmousedoubleclick;
		generatorui_vtable.move = (psy_ui_fp_component_move)generatorui_move;
		generatorui_vtable.invalidate = (psy_ui_fp_component_invalidate)generatorui_invalidate;
		generatorui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			generatorui_onpreferredsize;
		generatorui_vtable_initialized = TRUE;
	}
	return &generatorui_vtable;
}
// implementation
void generatorui_init(GeneratorUi* self, psy_ui_Component* parent,
	uintptr_t slot, MachineViewSkin* skin,
	psy_ui_Component* view, psy_ui_Edit* editname, Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(workspace->song);
	assert(skin);
	assert(view);

	psy_ui_component_init(&self->component, parent, view);	
	generatorui_vtable_init(self);
	self->component.vtable = &generatorui_vtable;
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	machineuicommon_init(&self->intern, slot, skin, view, editname, workspace);
	self->intern.coords = &skin->generator;
	self->intern.font = skin->generator_fontcolour;
	self->intern.bgcolour = psy_ui_colour_make(0x002f3E25);
	generatorui_initsize(self);
	psy_signal_connect(&workspace->signal_showparameters, self,
		generatorui_onshowparameters);
}

void generatorui_dispose(GeneratorUi* self)
{
	assert(self);

	if (self->intern.paramview) {
		psy_ui_component_destroy(&self->intern.paramview->component);
		free(self->intern.paramview);
	}
	if (self->intern.machineframe) {
		psy_ui_component_destroy(&self->intern.machineframe->component);
		free(self->intern.machineframe);
	}
	free(self->intern.restorename);
	psy_signal_disconnect(&self->intern.workspace->signal_showparameters, self,
		generatorui_onshowparameters);
	generatorui_super_vtable.dispose(&self->component);
}

void generatorui_initsize(GeneratorUi* self)
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
			psy_ui_point_makepx(topleft.x, topleft.y),
			psy_ui_size_makepx(size.width, size.height)));
	vudisplay_init(&self->intern.vu, self->intern.skin, self->intern.coords);
	self->intern.vu.position = generatorui_coordposition(self, &self->intern.coords->vu0);
}

void generatorui_move(GeneratorUi* self, psy_ui_Point topleft)
{
	assert(self);

	generatorui_super_vtable.move(&self->component, topleft);
	machineuicommon_move(&self->intern, topleft);
	self->intern.vu.position = generatorui_coordposition(self,
		&self->intern.coords->vu0);
}

psy_ui_RealRectangle generatorui_coordposition(GeneratorUi* self, SkinCoord* coord)
{
	psy_ui_RealRectangle r;

	assert(self);

	r = psy_ui_component_position(&self->component);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			r.left + coord->dest.left,
			r.top + coord->dest.top),
		psy_ui_realrectangle_size(&coord->dest));
}

void generatorui_editname(GeneratorUi* self, psy_ui_Edit* edit,
	psy_ui_RealPoint scroll)
{
	assert(self);

	if (self->intern.machine) {
		psy_ui_RealRectangle r;

		psy_strreset(&self->intern.restorename,
			psy_audio_machine_editname(self->intern.machine));
		psy_signal_disconnectall(&edit->component.signal_focuslost);
		psy_signal_disconnectall(&edit->component.signal_keydown);
		psy_signal_disconnectall(&edit->signal_change);
		psy_signal_connect(&edit->signal_change, self, generatorui_oneditchange);
		psy_signal_connect(&edit->component.signal_keydown, self,
			generatorui_oneditkeydown);
		psy_signal_connect(&edit->component.signal_focuslost, self,
			generatorui_oneditfocuslost);
		psy_ui_edit_settext(edit, psy_audio_machine_editname(self->intern.machine));
		r = generatorui_coordposition(self, &self->intern.coords->name);
		psy_ui_realrectangle_move(&r, -scroll.x, -scroll.y);
		psy_ui_component_setposition(psy_ui_edit_base(edit),
			psy_ui_rectangle_make_px(&r));
		psy_ui_component_show(&edit->component);
	}
}

void generatorui_oneditkeydown(GeneratorUi* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{
	assert(self);

	switch (ev->keycode) {
	case psy_ui_KEY_ESCAPE:
		if (self->intern.machine) {
			psy_audio_machine_seteditname(self->intern.machine, self->intern.restorename);
			free(self->intern.restorename);
			self->intern.restorename = NULL;
		}
		psy_ui_component_hide(sender);
		psy_ui_keyevent_preventdefault(ev);
		psy_ui_component_invalidate(&self->component);
		break;
	case psy_ui_KEY_RETURN:
		psy_ui_component_hide(sender);
		psy_ui_keyevent_preventdefault(ev);
		psy_ui_component_invalidate(&self->component);
		break;
	default:
		break;
	}
	psy_ui_keyevent_stoppropagation(ev);
}

void generatorui_oneditchange(GeneratorUi* self, psy_ui_Edit* sender)
{
	assert(self);

	if (self->intern.machine) {
		psy_audio_machine_seteditname(self->intern.machine,
			psy_ui_edit_text(sender));
	}
}

void generatorui_oneditfocuslost(GeneratorUi* self, psy_ui_Component* sender)
{
	assert(self);

	psy_ui_component_hide(sender);
}

void generatorui_ondraw(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	generatorui_drawbackground(self, g);
	if (!machineui_vuupdate()) {
		generatorui_draweditname(self, g);		
		generatorui_drawpanning(self, g);
		generatorui_drawmute(self, g);
		generatorui_drawsoloed(self, g);		
	}
	if (self->intern.skin->drawvumeters) {
		generatorui_drawvu(self, g);
	}
}

void generatorui_drawbackground(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (!psy_ui_bitmap_empty(&self->intern.skin->skinbmp)) {
		skin_blitcoord(g, &self->intern.skin->skinbmp, psy_ui_realpoint_zero(),
			&self->intern.coords->background);
	} else {
		psy_ui_RealRectangle r;

		r = psy_ui_component_position(&self->component);
		psy_ui_drawsolidrectangle(g, r, self->intern.bgcolour);		
	}
}

void generatorui_draweditname(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (self->intern.mode != psy_audio_MACHMODE_MASTER) {
		char editname[130];

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
				strlen(editname));
		}
	}
}

void generatorui_drawpanning(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	skin_blitcoord(g, &self->intern.skin->skinbmp,
		psy_ui_realpoint_make(
			skincoord_position(&self->intern.coords->pan,
				psy_audio_machine_panning(self->intern.machine)),
			0),
		&self->intern.coords->pan);
}

void generatorui_drawmute(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (psy_audio_machine_muted(self->intern.machine)) {
		skin_blitcoord(g, &self->intern.skin->skinbmp,
			psy_ui_realpoint_zero(),
			&self->intern.coords->mute);
	}
}

void generatorui_drawsoloed(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (psy_audio_machines_soloed(self->intern.machines) ==
			self->intern.slot) {
		skin_blitcoord(g, &self->intern.skin->skinbmp,
			psy_ui_realpoint_zero(),
			&self->intern.coords->solo);
	}
}

void generatorui_drawvu(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);
	
	generatorui_updatevolumedisplay(self);
	vudisplay_draw(&self->intern.vu, g);	
}

void generatorui_updatevolumedisplay(GeneratorUi* self)
{
	if (self->intern.machine) {
		vudisplay_update(&self->intern.vu,
			psy_audio_machine_buffermemory(self->intern.machine));
	}
}

void generatorui_showparameters(GeneratorUi* self, psy_ui_Component* parent)
{
	if (self->intern.machine) {
		if (!self->intern.machineframe) {
			self->intern.machineframe = machineframe_alloc();
			machineframe_init(self->intern.machineframe, parent, self->intern.workspace);
			psy_signal_connect(&self->intern.machineframe->component.signal_destroy,
				self, generatorui_onframedestroyed);
			if (psy_audio_machine_haseditor(self->intern.machine)) {
				MachineEditorView* editorview;

				editorview = machineeditorview_allocinit(
					psy_ui_notebook_base(&self->intern.machineframe->notebook),
					self->intern.machine, self->intern.workspace);
				if (editorview) {
					machineframe_setview(self->intern.machineframe,
						&editorview->component, self->intern.machine);
				}
			} else {
				ParamView* paramview;

				paramview = paramview_allocinit(
					&self->intern.machineframe->notebook.component,
					self->intern.machine, self->intern.workspace);
				if (paramview) {
					machineframe_setparamview(self->intern.machineframe, paramview,
						self->intern.machine);
				}
			}
		}
		if (self->intern.machineframe) {
			psy_ui_component_show(&self->intern.machineframe->component);
		}
	}
}

void generatorui_onframedestroyed(GeneratorUi* self, psy_ui_Component* sender)
{
	self->intern.machineframe = NULL;
}

void generatorui_onmousedown(GeneratorUi* self, psy_ui_MouseEvent* ev)
{	
	if (self->intern.slot != psy_audio_machines_selected(self->intern.machines)) {
		psy_audio_machines_select(self->intern.machines, self->intern.slot);
	}
	if (generatorui_hittestcoord(self, ev->pt,
			&self->intern.skin->generator.solo)) {
		psy_audio_machines_solo(self->intern.machines, self->intern.slot);
		psy_ui_mouseevent_stoppropagation(ev);
	} else if (generatorui_hittestcoord(self, ev->pt,
			&self->intern.skin->generator.mute)) {
		if (psy_audio_machine_muted(self->intern.machine)) {
			psy_audio_machine_unmute(self->intern.machine);
		} else {
			psy_audio_machine_mute(self->intern.machine);
		}
		psy_ui_mouseevent_stoppropagation(ev);
	} else if (generatorui_hittestpan(self, ev->pt, &self->intern.mx)) {
		self->intern.dragmode = MACHINEVIEW_DRAG_PAN;
		psy_ui_mouseevent_stoppropagation(ev);
	}
	if (!ev->bubble) {
		psy_ui_component_invalidate(&self->component);
	}
}

bool generatorui_hittesteditname(GeneratorUi* self, psy_ui_RealPoint pt)
{
	psy_ui_RealRectangle r;

	r = generatorui_coordposition(self, &self->intern.coords->name);
	return psy_ui_realrectangle_intersect(&r, pt);
}

bool generatorui_hittestcoord(GeneratorUi* self, psy_ui_RealPoint pt,
	SkinCoord* coord)
{
	assert(self);
	
	psy_ui_RealRectangle r;

	r = generatorui_coordposition(self, coord);
	return psy_ui_realrectangle_intersect(&r, pt);	
}

void generatorui_onmousemove(GeneratorUi* self, psy_ui_MouseEvent* ev)
{	
	if (self->intern.dragmode == MACHINEVIEW_DRAG_PAN) {
		psy_audio_machine_setpanning(self->intern.machine,
			generatorui_panvalue(self, ev->pt.x, self->intern.slot));
		psy_ui_component_invalidate(&self->component);
	}
}

int generatorui_hittestpan(GeneratorUi* self, psy_ui_RealPoint pt, double* dx)
{
	psy_ui_RealRectangle r;
	double offset;
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(&self->component);
	offset = psy_audio_machine_panning(self->intern.machine) *
		self->intern.coords->pan.range;
	r = skincoord_destposition(&self->intern.coords->pan);
	psy_ui_realrectangle_move(&r, offset, 0);
	*dx = pt.x - position.left - r.left;
	return psy_ui_realrectangle_intersect(&r,
		psy_ui_realpoint_make(pt.x - position.left,
			pt.y - position.top));
}

psy_dsp_amp_t generatorui_panvalue(GeneratorUi* self, double dx, uintptr_t slot)
{
	psy_dsp_amp_t rv = 0.f;
	MachineCoords* coords;
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(&self->component);
	coords = self->intern.coords;
	if (coords && coords->pan.range != 0) {
		rv = (psy_dsp_amp_t)(
			(dx - (double)position.left -
				coords->pan.dest.left - (double)self->intern.mx) /
			(double)coords->pan.range);
	}
	return rv;
}

void generatorui_onmouseup(GeneratorUi* self, psy_ui_MouseEvent* ev)
{
	self->intern.dragmode = MACHINEVIEW_DRAG_NONE;
}

void generatorui_onmousedoubleclick(GeneratorUi* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		psy_ui_RealPoint dragpt;

		if (generatorui_hittesteditname(self, ev->pt)) {
			if (self->intern.editname) {
				generatorui_editname(self, self->intern.editname,
					psy_ui_component_scrollpx(self->intern.view));
			}
		} else if (generatorui_hittestcoord(self, ev->pt,
				&self->intern.skin->generator.solo) ||
			generatorui_hittestcoord(self, ev->pt,
				&self->intern.skin->effect.bypass) ||
			generatorui_hittestcoord(self, ev->pt,
				&self->intern.skin->generator.mute) ||
			generatorui_hittestcoord(self, ev->pt,
				&self->intern.skin->effect.mute) ||
			generatorui_hittestpan(self, ev->pt, &dragpt.x)) {
		} else {
			generatorui_showparameters(self, self->intern.view);
		}
		psy_ui_mouseevent_stoppropagation(ev);
	}
}

void generatorui_invalidate(GeneratorUi* self)
{
	if (machineui_vuupdate()) {
		psy_ui_component_invalidaterect(self->intern.view,
			self->intern.vu.position);		
	} else {
		generatorui_super_vtable.invalidate(&self->component);
	}
}

void generatorui_onshowparameters(GeneratorUi* self, Workspace* sender,
	uintptr_t slot)
{
	if (slot == self->intern.slot) {
		generatorui_showparameters(self, self->intern.view);
	}
}

void generatorui_onpreferredsize(GeneratorUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{			
	psy_ui_size_setreal(rv, psy_ui_realrectangle_size(
		&self->intern.coords->background.dest));
}

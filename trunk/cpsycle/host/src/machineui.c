// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineui.h"
// host
#include "skingraphics.h"
#include "wireview.h"
// audio
#include <exclusivelock.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static void drawmachineline(psy_ui_Graphics*, psy_ui_RealPoint dir,
	psy_ui_RealPoint edge);

// MachineUi
void machineui_init(MachineUi* self, uintptr_t slot, MachineViewSkin* skin,
	psy_ui_Component* view, Workspace* workspace)
{	
	assert(self);
	assert(workspace);
	assert(workspace->song);
	assert(skin);
	assert(view);

	self->machines = &workspace->song->machines;
	self->machine = psy_audio_machines_at(self->machines, slot);
	self->workspace = workspace;
	self->view = view;
	self->skin = skin;	
	self->mode = psy_audio_machine_mode(self->machine);
	self->volumedisplay = (psy_dsp_amp_t)0.f;
	self->volumemaxdisplay = (psy_dsp_amp_t)0.f;
	self->volumemaxcounterlife = 0;
	self->coords = NULL;
	self->slot = slot;
	self->machineframe = NULL;
	self->paramview = NULL;
	self->editorview = NULL;
	self->restorename = NULL;
	self->machinepos = TRUE;
	self->dragmode = MACHINEVIEW_DRAG_NONE;
	machineui_update(self);	
}

void machineui_update(MachineUi* self)
{	
	psy_ui_RealPoint topleft;
	psy_ui_RealSize size;

	assert(self);
			
	switch (self->mode) {
		case MACHMODE_MASTER:
			self->coords = &self->skin->master;
			self->font = self->skin->effect_fontcolour;
			self->bgcolour = psy_ui_colour_make(0x00333333);
			break;
		case MACHMODE_GENERATOR:
			self->coords = &self->skin->generator;
			self->font = self->skin->generator_fontcolour;
			self->bgcolour = psy_ui_colour_make(0x002f3E25);
			break;
			// fallthrough
		case MACHMODE_FX:
		default:
			self->coords = &self->skin->effect;
			self->font = self->skin->effect_fontcolour;
			self->bgcolour = psy_ui_colour_make(0x003E2f25);
			break;
	}		
	size = psy_ui_realrectangle_size(&self->coords->background.dest);	
	if (self->machine && self->machinepos) {
		psy_audio_machine_position(self->machine, &topleft.x, &topleft.y);
	} else {
		topleft = psy_ui_realpoint_zero();
	}
	self->position = psy_ui_realrectangle_make(topleft, size);
	self->vuposition = machineui_coordposition(self, &self->coords->vu0);
	self->topleft = topleft;
}

void machineui_dispose(MachineUi* self)
{
	assert(self);

	if (self->paramview) {
		psy_ui_component_destroy(&self->paramview->component);
		free(self->paramview);
	}
	if (self->machineframe) {
		psy_ui_component_destroy(&self->machineframe->component);
		free(self->machineframe);
	}	
	free(self->restorename);
}

psy_ui_RealSize machineui_size(const MachineUi* self)
{	
	assert(self);

	return psy_ui_realrectangle_size(&self->position);
}

psy_ui_RealPoint machineui_centerposition(MachineUi* self)
{
	assert(self);

	return psy_ui_realpoint_make(
		self->position.left + psy_ui_realrectangle_width(&self->position) / 2,
		self->position.top + psy_ui_realrectangle_height(&self->position) / 2);
}

const psy_ui_RealRectangle* machineui_position(const MachineUi* self)
{
	assert(self);

	return &self->position;
}

void machineui_move(MachineUi* self, psy_ui_RealPoint topleft)
{
	assert(self);

	if (self->machine && self->machinepos) {
		psy_audio_machine_setposition(self->machine, topleft.x, topleft.y);			
	}
	psy_ui_realrectangle_settopleft(&self->position, topleft);
	self->topleft = topleft;
	self->vuposition = machineui_coordposition(self, &self->coords->vu0);
}

psy_ui_RealRectangle machineui_coordposition(MachineUi* self, SkinCoord* coord)
{	
	assert(self);	
		
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			self->position.left + coord->dest.left,
			self->position.top + coord->dest.top),
		psy_ui_realrectangle_size(&coord->dest));
}

void machineui_editname(MachineUi* self, psy_ui_Edit* edit,
	psy_ui_RealPoint scroll)
{
	assert(self);

	if (self->machine) {
		psy_ui_RealRectangle r;		
				
		psy_strreset(&self->restorename,
			psy_audio_machine_editname(self->machine));
		psy_signal_disconnectall(&edit->component.signal_focuslost);
		psy_signal_disconnectall(&edit->component.signal_keydown);
		psy_signal_disconnectall(&edit->signal_change);
		psy_signal_connect(&edit->signal_change, self, machineui_oneditchange);
		psy_signal_connect(&edit->component.signal_keydown, self,
			machineui_onkeydown);
		psy_signal_connect(&edit->component.signal_focuslost, self,
			machineui_oneditfocuslost);
		psy_ui_edit_settext(edit, psy_audio_machine_editname(self->machine));
		r = machineui_coordposition(self, &self->coords->name);		
		psy_ui_realrectangle_move(&r, -scroll.x, -scroll.y);
		psy_ui_component_setposition(psy_ui_edit_base(edit),
			psy_ui_point_makepx(r.left, r.top),
			psy_ui_size_makepx(
				psy_ui_realrectangle_width(&r),
				psy_ui_realrectangle_height(&r)));
		psy_ui_component_show(&edit->component);
	}
}

void machineui_onkeydown(MachineUi* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{	
	assert(self);

	switch (ev->keycode) {		
		case psy_ui_KEY_ESCAPE:			
			if (self->machine) {
				psy_audio_machine_seteditname(self->machine, self->restorename);
				free(self->restorename);
				self->restorename = NULL;			
			}
			psy_ui_component_hide(sender);
			psy_ui_keyevent_preventdefault(ev);
			machineui_invalidate(self, FALSE);
			break;
		case psy_ui_KEY_RETURN:
			psy_ui_component_hide(sender);
			psy_ui_keyevent_preventdefault(ev);
			machineui_invalidate(self, FALSE);
			break;
		default:
			break;
	}
	psy_ui_keyevent_stoppropagation(ev);
}

void machineui_oneditchange(MachineUi* self, psy_ui_Edit* sender)
{
	assert(self);

	if (self->machine) {
		psy_audio_machine_seteditname(self->machine, psy_ui_edit_text(sender));
	}
}

void machineui_oneditfocuslost(MachineUi* self, psy_ui_Component* sender)
{
	assert(self);

	psy_ui_component_hide(sender);
}

void machineui_invalidate(MachineUi* self, bool vuupdate)
{	
	if (vuupdate) {
		psy_ui_component_invalidaterect(self->view, self->vuposition);		
	} else {
		psy_ui_component_invalidaterect(self->view, self->position);
	}		
}

void machineui_draw(MachineUi* self, psy_ui_Graphics* g, uintptr_t slot,
	bool vuupdate)
{	
	assert(self);
		
	machineui_drawbackground(self, g);		
	if (!vuupdate) {
		machineui_draweditname(self, g);
		if (self->mode != MACHMODE_MASTER) {				
			machineui_drawpanning(self, g);
			machineui_drawmute(self, g);
			machineui_drawbypassed(self, g);
			machineui_drawsoloed(self, g,
				&self->workspace->song->machines);
		}
	}	
	if (self->skin->drawvumeters) {
		machineui_drawvu(self, g);	
	}
}

void machineui_drawbackground(MachineUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (!psy_ui_bitmap_empty(&self->skin->skinbmp)) {
		skin_blitcoord(g, &self->skin->skinbmp, psy_ui_realpoint_zero(),
			&self->coords->background);
	} else {
		psy_ui_drawsolidrectangle(g, self->position, self->bgcolour);
		if (self->mode == MACHMODE_MASTER) {			
			psy_ui_textoutrectangle(g, 
				psy_ui_realrectangle_topleft(&self->coords->name.dest),
				psy_ui_ETO_CLIPPED, self->coords->name.dest, "Master",
				strlen("Master"));
		}
	}
}

void machineui_draweditname(MachineUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (self->mode != MACHMODE_MASTER) {
		char editname[130];		

		editname[0] = '\0';
		if (psy_audio_machine_editname(self->machine)) {		
			if (self->skin->drawmachineindexes) {
				psy_snprintf(editname, 130, "%.2X:%s", (int)self->slot,
					psy_audio_machine_editname(self->machine));
			} else {
				psy_snprintf(editname, 130, "%s",
					psy_audio_machine_editname(self->machine));
			}
		}
		if (psy_strlen(editname) > 0) {			
			psy_ui_settextcolour(g, self->font);
			psy_ui_textoutrectangle(g,
				psy_ui_realrectangle_topleft(&self->coords->name.dest),
				psy_ui_ETO_CLIPPED, self->coords->name.dest, editname,
				strlen(editname));
		}
	}
}

void machineui_drawpanning(MachineUi* self, psy_ui_Graphics* g)
{
	assert(self);

	skin_blitcoord(g, &self->skin->skinbmp,
		psy_ui_realpoint_make(
			skincoord_position(&self->coords->pan,
				psy_audio_machine_panning(self->machine)),
			0),
		&self->coords->pan);
}

void machineui_drawmute(MachineUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (psy_audio_machine_muted(self->machine)) {
		skin_blitcoord(g, &self->skin->skinbmp,
			psy_ui_realpoint_zero(),
			&self->coords->mute);
	}
}

void machineui_drawbypassed(MachineUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if ((psy_audio_machine_mode(self->machine) == MACHMODE_FX) &&
		psy_audio_machine_bypassed(self->machine)) {
		skin_blitcoord(g, &self->skin->skinbmp,
			psy_ui_realpoint_zero(),
			&self->coords->bypass);
	}
}

void machineui_drawsoloed(MachineUi* self, psy_ui_Graphics* g,
	psy_audio_Machines* machines)
{
	assert(self);

	if ((psy_audio_machine_mode(self->machine) == MACHMODE_GENERATOR) &&
			psy_audio_machines_soloed(machines) == self->slot) {
		skin_blitcoord(g, &self->skin->skinbmp,
			psy_ui_realpoint_zero(),
			&self->coords->solo);
	}
}

void machineui_drawvu(MachineUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (self->mode != MACHMODE_MASTER) {		
		machineui_drawvudisplay(self, g);
		machineui_drawvupeak(self, g);
	}
}

void machineui_drawvudisplay(MachineUi* self, psy_ui_Graphics* g)
{
	if (self->machine) {		
		psy_ui_drawbitmap(g, &self->skin->skinbmp,
			psy_ui_realrectangle_make(
				psy_ui_realrectangle_topleft(&self->coords->vu0.dest),
			psy_ui_realsize_make(
				self->volumedisplay *
					psy_ui_realrectangle_width(&self->coords->vu0.dest),
				psy_ui_realrectangle_height(&self->coords->vu0.dest))),
			psy_ui_realrectangle_topleft(&self->coords->vu0.src));
	}
}

void machineui_drawvupeak(MachineUi* self, psy_ui_Graphics* g)
{
	if (self->machine && (self->volumemaxdisplay > 0.01f)) {		
		SkinCoord* vupeak;
		SkinCoord* vu;

		vupeak = &self->coords->vupeak;
		vu = &self->coords->vu0;		
		psy_ui_drawbitmap(g, &self->skin->skinbmp,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					vu->dest.left + self->volumemaxdisplay *
						psy_ui_realrectangle_width(&vu->dest),
					vu->dest.top),
				psy_ui_realrectangle_size(&vupeak->src)),
			psy_ui_realrectangle_topleft(&vupeak->src));
	}
}

void machineui_updatevolumedisplay(MachineUi* self)
{
	if (self->machine) {
		psy_audio_Buffer* memory;

		memory = psy_audio_machine_buffermemory(self->machine);		
		self->volumedisplay = (memory)
			? psy_audio_buffer_rmsdisplay(memory)
			: (psy_dsp_amp_t)0.f;		
		machineui_updatemaxvolumedisplay(self);
	}
}

void machineui_updatemaxvolumedisplay(MachineUi* self)
{
	--self->volumemaxcounterlife;
	if ((self->volumedisplay > self->volumemaxdisplay)
			|| (self->volumemaxcounterlife <= 0)) {
		self->volumemaxdisplay = self->volumedisplay - 1/32768.f;
		self->volumemaxcounterlife = 60;
	}
}

void machineui_showparameters(MachineUi* self, psy_ui_Component* parent)
{
	if (self->machine) {		
		if (!self->machineframe) {
			self->machineframe = machineframe_alloc();
			machineframe_init(self->machineframe, parent, self->workspace);
			psy_signal_connect(&self->machineframe->component.signal_destroy,
				self, machineui_onframedestroyed);
			if (psy_audio_machine_haseditor(self->machine)) {
				MachineEditorView* editorview;

				editorview = machineeditorview_allocinit(
					psy_ui_notebook_base(&self->machineframe->notebook),
					self->machine, self->workspace);
				if (editorview) {
					machineframe_setview(self->machineframe,
						&editorview->component, self->machine);
				}
			} else {
				ParamView* paramview;

				paramview = paramview_allocinit(
					&self->machineframe->notebook.component,
					self->machine, self->workspace);
				if (paramview) {
					machineframe_setparamview(self->machineframe, paramview,
						self->machine);
				}
			}
		}
		if (self->machineframe) {
			psy_ui_component_show(&self->machineframe->component);
		}		
	}
}

void machineui_onframedestroyed(MachineUi* self, psy_ui_Component* sender)
{	
	self->machineframe = NULL;
}

void machineui_onmousedown(MachineUi* self, psy_ui_MouseEvent* ev)
{	
	if (self->slot == psy_audio_MASTER_INDEX) {
		return;
	}	
	if (machineui_hittestcoord(self, ev->pt, MACHMODE_GENERATOR,
			&self->skin->generator.solo)) {				
		psy_audio_machines_solo(self->machines, self->slot);		
		psy_ui_mouseevent_stoppropagation(ev);
	} else if (machineui_hittestcoord(self, ev->pt,
			MACHMODE_FX, &self->skin->effect.bypass)) {				
		if (psy_audio_machine_bypassed(self->machine)) {
			psy_audio_machine_unbypass(self->machine);
		} else {
			psy_audio_machine_bypass(self->machine);
		}		
		psy_ui_mouseevent_stoppropagation(ev);
	} else if (machineui_hittestcoord(self, ev->pt, MACHMODE_GENERATOR,
			&self->skin->generator.mute) ||
		machineui_hittestcoord(self, ev->pt,
		MACHMODE_FX, &self->skin->effect.mute)) {				
		if (psy_audio_machine_muted(self->machine)) {
			psy_audio_machine_unmute(self->machine);
		} else {
			psy_audio_machine_mute(self->machine);
		}		
		psy_ui_mouseevent_stoppropagation(ev);
	} else if (machineui_hittestpan(self, ev->pt, self->slot, &self->mx)) {
		self->dragmode = MACHINEVIEW_DRAG_PAN;		
		psy_ui_mouseevent_stoppropagation(ev);
	}
	if (!ev->bubble) {
		machineui_invalidate(self, FALSE);
	}
}

bool machineui_hittestcoord(MachineUi* self, psy_ui_RealPoint pt, int mode,
	SkinCoord* coord)
{	
	assert(self);

	if (self->mode == mode) {
		psy_ui_RealRectangle r;

		r = machineui_coordposition(self, coord);
		return psy_ui_realrectangle_intersect(&r, pt);
	}
	return FALSE;
}

void machineui_onmousemove(MachineUi* self, psy_ui_MouseEvent* ev)
{
	if (self->dragmode == MACHINEVIEW_DRAG_PAN) {		
		psy_audio_machine_setpanning(self->machine,
			machineui_panvalue(self, ev->pt.x, self->slot));
		machineui_invalidate(self, FALSE);		
	}
}

int machineui_hittestpan(MachineUi* self, psy_ui_RealPoint pt,
	uintptr_t slot, double* dx)
{
	psy_ui_RealRectangle r;
	double offset;

	offset = psy_audio_machine_panning(self->machine) *
		self->coords->pan.range;
	r = skincoord_destposition(&self->coords->pan);
	psy_ui_realrectangle_move(&r, offset, 0);
	*dx = pt.x - self->position.left - r.left;
	return psy_ui_realrectangle_intersect(&r,
		psy_ui_realpoint_make(pt.x - self->position.left,
			pt.y - self->position.top));
}

psy_dsp_amp_t machineui_panvalue(MachineUi* self, double dx, uintptr_t slot)
{
	psy_dsp_amp_t rv = 0.f;	
	MachineCoords* coords;
	
	coords = self->coords;
	if (coords && coords->pan.range != 0) {		
		rv = (psy_dsp_amp_t)(
			(dx - (double)self->position.left -
				coords->pan.dest.left - (double)self->mx) /
			(double)coords->pan.range);
	}	
	return rv;
}

void machineui_onmouseup(MachineUi* self, psy_ui_MouseEvent* ev)
{

}

void machineui_onmousedoubleclick(MachineUi* self, psy_ui_MouseEvent* ev)
{

}

void machineui_drawhighlight(MachineUi* self, psy_ui_Graphics* g)
{
	static intptr_t d = 5; // the distance of the highlight from the machine
	static psy_ui_RealPoint dirs[] = {
		{ 1.0, 0.0 }, { 0.0, 1.0 }, { -1.0, 0.0 }, { 0.0, -1.0 }
	};
	psy_ui_RealPoint edges[4];

	edges[0] = psy_ui_realpoint_make(self->position.left - d,
		self->position.top - d);
	edges[1] = psy_ui_realpoint_make(self->position.right + d,
		self->position.top - d);
	edges[2] = psy_ui_realpoint_make(self->position.right + d,
		self->position.bottom + d);
	edges[3] = psy_ui_realpoint_make(self->position.left - d,
		self->position.bottom + d);
	psy_ui_setcolour(g, self->skin->wirecolour);
	drawmachineline(g, dirs[0], edges[0]);
	drawmachineline(g, dirs[1], edges[0]);
	drawmachineline(g, dirs[1], edges[1]);
	drawmachineline(g, dirs[2], edges[1]);
	drawmachineline(g, dirs[2], edges[2]);
	drawmachineline(g, dirs[3], edges[2]);
	drawmachineline(g, dirs[3], edges[3]);
	drawmachineline(g, dirs[0], edges[3]);
}

void drawmachineline(psy_ui_Graphics* g, psy_ui_RealPoint dir,
	psy_ui_RealPoint edge)
{
	static intptr_t hlength = 9; // the length of the selected machine highlight	

	psy_ui_drawline(g, edge, psy_ui_realpoint_make(
		edge.x + dir.x * hlength, edge.y + dir.y * hlength));
}

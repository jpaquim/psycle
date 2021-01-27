// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineview.h"
// host
#include "skingraphics.h"
#include "wireview.h"
// audio
#include <exclusivelock.h>
#include <songio.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static void machineui_init(MachineUi*, uintptr_t slot, MachineViewSkin*,
	Workspace*);
static void machineui_update(MachineUi*);
static void machineui_dispose(MachineUi*);
static void machineui_onframedestroyed(MachineUi*, psy_ui_Component* sender);
static psy_ui_RealSize machineui_size(const MachineUi*);
static void machineui_move(MachineUi*, psy_ui_RealPoint dest);
static const psy_ui_RealRectangle* machineui_position(const MachineUi*);
static psy_ui_RealRectangle machineui_coordposition(MachineUi*, SkinCoord*);
static psy_ui_RealPoint machineui_centerposition(MachineUi*);
static void machineui_draw(MachineUi*, psy_ui_Graphics*, uintptr_t slot,
	bool vuupdate);
static void machineui_drawbackground(MachineUi*, psy_ui_Graphics*);
static void machineui_draweditname(MachineUi*, psy_ui_Graphics*);
static void machineui_drawpanning(MachineUi*, psy_ui_Graphics*);
static void machineui_drawmute(MachineUi*, psy_ui_Graphics*);
static void machineui_drawbypassed(MachineUi*, psy_ui_Graphics*);
static void machineui_drawsoloed(MachineUi*, psy_ui_Graphics*,
	psy_audio_Machines*);
static void machineui_drawvu(MachineUi*, psy_ui_Graphics*);
static void machineui_drawvudisplay(MachineUi*, psy_ui_Graphics*);
static void machineui_drawvupeak(MachineUi*, psy_ui_Graphics*);
static void machineui_drawhighlight(MachineUi*, psy_ui_Graphics*);
static void machineui_updatevolumedisplay(MachineUi*);
static void machineui_updatemaxvolumedisplay(MachineUi*);
static void machineui_showparameters(MachineUi*, psy_ui_Component* parent);
static void machineui_editname(MachineUi*, psy_ui_Edit*,
	psy_ui_RealPoint scrolloffset);
static void machineui_onkeydown(MachineUi*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void machineui_oneditchange(MachineUi*, psy_ui_Edit* sender);
static void machineui_oneditfocuslost(MachineUi*, psy_ui_Component* sender);
static void machineui_invalidate(MachineUi*, psy_ui_Component* view,
	bool vuupdate);

// MachineUi
void machineui_init(MachineUi* self, uintptr_t slot, MachineViewSkin* skin,
	Workspace* workspace)
{	
	assert(self);
	assert(workspace);
	assert(workspace->song);
	assert(skin);	

	self->machine = psy_audio_machines_at(&workspace->song->machines, slot);	
	self->workspace = workspace;	
	self->skin = skin;	
	self->mode = psy_audio_machine_mode(self->machine);
	self->volumedisplay = (psy_dsp_amp_t)0.f;
	self->volumemaxdisplay = (psy_dsp_amp_t)0.f;
	self->volumemaxcounterlife = 0;
	self->coords = NULL;
	self->slot = slot;
	self->frame = NULL;
	self->paramview = NULL;
	self->editorview = NULL;
	self->restorename = NULL;
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
	if (self->machine) {
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
	if (self->frame) {		
		psy_ui_component_destroy(&self->frame->component);
		free(self->frame);		
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

	if (self->machine) {
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

	if (ev->keycode == psy_ui_KEY_RETURN) {		
		psy_ui_component_hide(sender);
		psy_ui_keyevent_preventdefault(ev);
	} else if (ev->keycode == psy_ui_KEY_ESCAPE) {
		if (self->machine) {
			psy_audio_machine_seteditname(self->machine, self->restorename);
			free(self->restorename);
			self->restorename = NULL;			
		}
		psy_ui_keyevent_preventdefault(ev);
		psy_ui_component_hide(sender);
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

void machineui_invalidate(MachineUi* self, psy_ui_Component* view,
	bool vuupdate)
{
	psy_ui_component_invalidaterect(view,
		(vuupdate)
		? self->vuposition
		: self->position);
}

void machineui_draw(MachineUi* self, psy_ui_Graphics* g, uintptr_t slot,
	bool vuupdate)
{	
	assert(self);
	
	if (psy_ui_realrectangle_intersect_rectangle(&g->clip, &self->position)) {		
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
}

void machineui_drawbackground(MachineUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (!psy_ui_bitmap_empty(&self->skin->skinbmp)) {
		skin_blitpart(g, &self->skin->skinbmp, self->topleft,
			&self->coords->background);
	} else {
		psy_ui_drawsolidrectangle(g, self->position, self->bgcolour);
		if (self->mode == MACHMODE_MASTER) {
			psy_ui_RealRectangle clip;

			clip = machineui_coordposition(self, &self->coords->name);
			psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&clip),
				psy_ui_ETO_CLIPPED, clip, "Master", strlen("Master"));
		}
	}
}

void machineui_draweditname(MachineUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (self->mode != MACHMODE_MASTER) {
		char editname[130];
		psy_ui_RealRectangle r;

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
			r = machineui_coordposition(self, &self->coords->name);
			psy_ui_settextcolour(g, self->font);
			psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r),
				psy_ui_ETO_CLIPPED, r, editname, strlen(editname));
		}
	}
}

void machineui_drawpanning(MachineUi* self, psy_ui_Graphics* g)
{
	assert(self);

	skin_blitpart(g, &self->skin->skinbmp,
		psy_ui_realpoint_make(
			self->topleft.x + skincoord_position(&self->coords->pan,
				psy_audio_machine_panning(self->machine)),
			self->topleft.y),
		&self->coords->pan);
}

void machineui_drawmute(MachineUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (psy_audio_machine_muted(self->machine)) {
		skin_blitpart(g, &self->skin->skinbmp, self->topleft,
			&self->coords->mute);
	}
}

void machineui_drawbypassed(MachineUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if ((psy_audio_machine_mode(self->machine) == MACHMODE_FX) &&
		psy_audio_machine_bypassed(self->machine)) {
		skin_blitpart(g, &self->skin->skinbmp, self->topleft,
			&self->coords->bypass);
	}
}

void machineui_drawsoloed(MachineUi* self, psy_ui_Graphics* g,
	psy_audio_Machines* machines)
{
	assert(self);

	if ((psy_audio_machine_mode(self->machine) == MACHMODE_GENERATOR) &&
			psy_audio_machines_soloed(machines) == self->slot) {
		skin_blitpart(g, &self->skin->skinbmp, self->topleft,
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
		psy_ui_RealRectangle vu;

		vu = machineui_coordposition(self, &self->coords->vu0);
		psy_ui_drawbitmap(g, &self->skin->skinbmp,
			psy_ui_realrectangle_make(
				psy_ui_realrectangle_topleft(&vu),
			psy_ui_realsize_make(
				self->volumedisplay * psy_ui_realrectangle_width(&vu),
				psy_ui_realrectangle_height(&vu))),
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
					self->topleft.x + vu->dest.left + self->volumemaxdisplay *
						psy_ui_realrectangle_width(&vu->dest),
					self->topleft.y + psy_ui_realrectangle_height(&vu->dest)),
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
		if (!self->frame) {
			self->frame = machineframe_alloc();
			machineframe_init(self->frame, parent, self->workspace);			
			psy_signal_connect(&self->frame->component.signal_destroy, self,
				machineui_onframedestroyed);
			if (psy_audio_machine_haseditor(self->machine)) {
				MachineEditorView* editorview;

				editorview = machineeditorview_allocinit(
					psy_ui_notebook_base(&self->frame->notebook),
					self->machine, self->workspace);
				if (editorview) {
					machineframe_setview(self->frame, &editorview->component,
						self->machine);
				}
			} else {
				ParamView* paramview;

				paramview = paramview_allocinit(
					&self->frame->notebook.component,
					self->machine, self->workspace);
				if (paramview) {
					machineframe_setparamview(self->frame, paramview,
						self->machine);
				}
			}			
		}
		if (self->frame) {
			psy_ui_component_show(&self->frame->component);			
		}		
	}
}

void machineui_onframedestroyed(MachineUi* self, psy_ui_Component* sender)
{	
	self->frame = NULL;	
}

// MachineWireView
// prototypes
static void machinewireview_ondestroy(MachineWireView*);
static psy_ui_RealRectangle machinewireview_bounds(MachineWireView*);
static void machinewireview_connectsong(MachineWireView*);
static void machinewireview_ondraw(MachineWireView*, psy_ui_Graphics*);
static void machinewireview_drawmachines(MachineWireView*, psy_ui_Graphics*);
static void drawmachineline(psy_ui_Graphics*, psy_ui_RealPoint dir,
	psy_ui_RealPoint edge);
static void machinewireview_drawdragwire(MachineWireView*, psy_ui_Graphics*);
static void machinewireview_drawwires(MachineWireView*, psy_ui_Graphics*);
static void machinewireview_drawwire(MachineWireView*, psy_ui_Graphics*,
	MachineUi*);
static void machinewireview_drawwirearrow(MachineWireView*, psy_ui_Graphics*,
	double x1, double y1, double x2, double y2);
static psy_ui_RealPoint rotate_point(psy_ui_RealPoint, double phi);
static psy_ui_RealPoint move_point(psy_ui_RealPoint pt, psy_ui_RealPoint d);
static void machinewireview_onmousedown(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onmouseup(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onmousemove(MachineWireView*, psy_ui_MouseEvent*);
static bool machinewireview_movemachine(MachineWireView*, uintptr_t slot,
	double dx, double dy);
static bool machinewireview_dragmachine(MachineWireView*, uintptr_t slot,
	double x, double y);
static void machinewireview_onmousedoubleclick(MachineWireView*,
	psy_ui_MouseEvent*);
static void machinewireview_onkeydown(MachineWireView*, psy_ui_KeyEvent*);
static uintptr_t machinewireview_machineleft(MachineWireView*, uintptr_t src);
static uintptr_t machinewireview_machineright(MachineWireView*, uintptr_t src);
static uintptr_t machinewireview_machineup(MachineWireView*, uintptr_t src);
static uintptr_t machinewireview_machinedown(MachineWireView*, uintptr_t src);
static void machinewireview_hittest(MachineWireView*);
static int machinewireview_hittestpan(MachineWireView*, double x, double y,
	uintptr_t slot, double* dx);
static bool machinewireview_hittestcoord(MachineWireView*, double x, double y,
	int mode, SkinCoord*);
static psy_audio_Wire machinewireview_hittestwire(MachineWireView*,
	double x, double y);
static int machinewireview_hittesteditname(MachineWireView*, psy_ui_RealPoint,
	uintptr_t slot);
static psy_dsp_amp_t machinewireview_panvalue(MachineWireView*,
	double x, double y, uintptr_t slot);
static bool machinewireview_dragging_machine(const MachineWireView*);
static bool machinewireview_dragging_connection(const MachineWireView*);
static bool machinewireview_dragging_newconnection(const MachineWireView*);
static void machinewireview_onnewmachineselected(MachineView*,
	psy_ui_Component* sender, psy_Property*);
static void machinewireview_onmachineselected(MachineWireView*,
	psy_audio_Machines*, uintptr_t slot);
static void machinewireview_onwireselected(MachineWireView*,
	psy_audio_Machines* sender, psy_audio_Wire);
static void machinewireview_onmachinesinsert(MachineWireView*,
	psy_audio_Machines*, uintptr_t slot);
static void machinewireview_onmachinesremoved(MachineWireView*,
	psy_audio_Machines*, uintptr_t slot);
static void machinewireview_onconnected(MachineWireView*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machinewireview_ondisconnected(MachineWireView*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machinewireview_onsongchanged(MachineWireView*, Workspace*,
	int flag, psy_audio_SongFile*);
static void machinewireview_buildmachineuis(MachineWireView*);
static void machinewireview_updateskin(MachineWireView*);
static void machinewireview_onshowparameters(MachineWireView*, Workspace*,
	uintptr_t slot);
static void machinewireview_onmasterworked(MachineWireView*,
	psy_audio_Machine*, uintptr_t slot, psy_audio_BufferContext*);
static void machinewireview_ontimer(MachineWireView*, uintptr_t timerid);
static void machinewireview_destroywireframes(MachineWireView*);
static void machinewireview_floatparamview(MachineWireView*, MachineUi*);
static void machinewireview_dockparamview(MachineWireView*, MachineUi*);
static void machinewireview_closeparamview(MachineWireView*, MachineUi*);
static void machinewireview_showwireview(MachineWireView*, psy_audio_Wire);
static void machinewireview_onwireframedestroyed(MachineWireView*,
	psy_ui_Component* sender);
static WireFrame* machinewireview_wireframe(MachineWireView*, psy_audio_Wire);
static psy_ui_RealRectangle machinewireview_updaterect(MachineWireView*,
	uintptr_t slot);
static void machinewireview_onpreferredsize(MachineWireView*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static MachineUi* machineuis_insert(MachineWireView*, uintptr_t slot);
static MachineUi* machineuis_at(MachineWireView*, uintptr_t slot);
static void machineuis_remove(MachineWireView*, uintptr_t slot);
static void machineuis_removeall(MachineWireView*);

static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(MachineWireView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			machinewireview_ondestroy;
		vtable.ondraw = (psy_ui_fp_component_ondraw)machinewireview_ondraw;
		vtable.ontimer = (psy_ui_fp_component_ontimer)machinewireview_ontimer;
		vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			machinewireview_onmousedown;
		vtable.onmouseup = (psy_ui_fp_component_onmouseup)
			machinewireview_onmouseup;
		vtable.onmousemove = (psy_ui_fp_component_onmousemove)
			machinewireview_onmousemove;
		vtable.onmousedoubleclick = (psy_ui_fp_component_onmousedoubleclick)
			machinewireview_onmousedoubleclick;
		vtable.onkeydown = (psy_ui_fp_component_onkeydown)
			machinewireview_onkeydown;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			machinewireview_onpreferredsize;
		vtable_initialized = TRUE;
	}
	return &vtable;
}

void machinewireview_init(MachineWireView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_ui_Scroller* scroller,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component, vtable_init(self));	
	self->component.vtable = &vtable;
	self->component.scrollstepx = psy_ui_value_makepx(10);
	self->component.scrollstepy = psy_ui_value_makepx(10);
	self->statusbar = NULL;
	self->machines = NULL;
	self->workspace = workspace;
	self->machines = &workspace->song->machines;	
	self->wireframes = 0;
	self->randominsert = 1;
	self->addeffect = 0;
	self->vudrawupdate = FALSE;
	self->showwirehover = FALSE;
	self->drawvirtualgenerators = FALSE;
	self->scroller = scroller;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	// skin init
	machineviewskin_init(&self->skin);
	machinewireview_updateskin(self);
	psy_table_init(&self->machineuis);
	machineuis_insert(self, psy_audio_MASTER_INDEX);
	psy_ui_component_setfont(&self->component, &self->skin.font);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);		
	self->dragslot = psy_INDEX_INVALID;
	self->dragmode = MACHINEWIREVIEW_DRAG_MACHINE;
	self->selectedslot = psy_audio_MASTER_INDEX;	
	psy_audio_wire_init(&self->selectedwire);
	psy_audio_wire_init(&self->hoverwire);
	psy_signal_connect(&workspace->signal_songchanged, self,
		machinewireview_onsongchanged);	
	machinewireview_connectsong(self);
	psy_signal_connect(&workspace->signal_showparameters, self,
		machinewireview_onshowparameters);	
	psy_ui_edit_init(&self->editname, &self->component);
	psy_ui_component_hide(&self->editname.component);
	psy_ui_component_starttimer(&self->component, 0, 50);	
}

void machinewireview_connectsong(MachineWireView* self)
{	
	if (self->machines) {
		psy_signal_connect(&self->machines->signal_slotchange, self,
			machinewireview_onmachineselected);
		psy_signal_connect(&self->machines->signal_wireselected, self,
			machinewireview_onwireselected);		
		psy_signal_connect(&self->machines->signal_insert, self,
			machinewireview_onmachinesinsert);
		psy_signal_connect(&self->machines->signal_removed, self,
			machinewireview_onmachinesremoved);
		psy_signal_connect(&self->machines->connections.signal_connected, self,
			machinewireview_onconnected);
		psy_signal_connect(&self->machines->connections.signal_disconnected, self,
			machinewireview_ondisconnected);
		psy_signal_connect(
			&psy_audio_machines_master(self->machines)->signal_worked, self,
			machinewireview_onmasterworked);
	}
}

void machinewireview_ondestroy(MachineWireView* self)
{	
	if (self->machines) {
		psy_signal_disconnect_context(
			&psy_audio_machines_master(self->machines)->signal_worked, self);
	}	
	psy_table_disposeall(&self->machineuis, (psy_fp_disposefunc)
		machineui_dispose);
	psy_list_deallocate(&self->wireframes, (psy_fp_disposefunc)
		psy_ui_component_destroy);		
	machineviewskin_dispose(&self->skin);
}

void machinewireview_configure(MachineWireView* self, MachineViewConfig* config)
{	
	self->skin.drawvumeters = machineviewconfig_vumeters(config);
	self->skin.drawmachineindexes = machineviewconfig_machineindexes(config);
	self->showwirehover = machineviewconfig_wirehover(config);
	self->drawvirtualgenerators = machineviewconfig_virtualgenerators(config);
}

void machinewireview_updateskin(MachineWireView* self)
{		
	machineviewskin_settheme(&self->skin, 
		psycleconfig_macview(workspace_conf(self->workspace))->theme,
		dirconfig_skins(&self->workspace->config.directories));	
	psy_ui_component_setbackgroundcolour(&self->component, self->skin.colour);	
}

void machinewireview_ondraw(MachineWireView* self, psy_ui_Graphics* g)
{		
	if (!self->vudrawupdate) {
		machinewireview_drawwires(self, g);
	}
	machinewireview_drawmachines(self, g);
	if (!self->vudrawupdate) {
		machinewireview_drawdragwire(self, g);
	}
}

void machinewireview_drawwires(MachineWireView* self, psy_ui_Graphics* g)
{
	psy_TableIterator it;
	
	for (it = psy_table_begin(&self->machineuis); 
			!psy_tableiterator_equal(&it, psy_table_end()); 
			psy_tableiterator_inc(&it)) {
		machinewireview_drawwire(self, g,
			(MachineUi*)psy_tableiterator_value(&it));
	}	
}

void machinewireview_drawwire(MachineWireView* self, psy_ui_Graphics* g,
	MachineUi* machineui)
{		
	psy_audio_MachineSockets* sockets;	
	
	sockets	= psy_audio_connections_at(&self->machines->connections,
		machineui->slot);
	if (sockets) {
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->outputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);		
			if (socket->slot != psy_INDEX_INVALID) {
				MachineUi* inmachineui;				

				inmachineui = machineuis_at(self, socket->slot);
				if (inmachineui && machineui) {
					psy_ui_RealPoint out;
					psy_ui_RealPoint in;
										
					if (self->hoverwire.src == machineui->slot &&
							self->hoverwire.dst == socket->slot) {
						psy_ui_setcolour(g, self->skin.hoverwirecolour);
					} else if (self->selectedwire.src == machineui->slot &&
							self->selectedwire.dst == socket->slot) {
						psy_ui_setcolour(g, self->skin.selwirecolour);
					} else {
						psy_ui_setcolour(g, self->skin.wirecolour);
					}
					out = machineui_centerposition(machineui);
					in = machineui_centerposition(inmachineui);
					psy_ui_drawline(g, out, in);
					machinewireview_drawwirearrow(self, g, out.x, out.y,
						in.x, in.y);
				}
			}
		}
	}
}

void machinewireview_drawwirearrow(MachineWireView* self, psy_ui_Graphics* g,
	double x1, double y1, double x2, double y2)
{			
	psy_ui_RealPoint center;
	psy_ui_RealPoint a, b, c;	
	psy_ui_RealPoint tri[4];
	double polysize;
	float deltaColR = ((self->skin.polycolour.value     & 0xFF) / 510.0f) + .45f;
	float deltaColG = ((self->skin.polycolour.value >>8  & 0xFF) / 510.0f) + .45f;
	float deltaColB = ((self->skin.polycolour.value>>16 & 0xFF) / 510.0f) + .45f;	
	unsigned int polyInnards;
	double phi;
	
	polyInnards = psy_ui_colour_make_rgb((uint8_t)(192 * deltaColR),
		(uint8_t)(192 * deltaColG), (uint8_t)(192 * deltaColB)).value;
			
	center.x = (x2 - x1) / 2 + x1;
	center.y = (y2 - y1) / 2 + y1;

	polysize = self->skin.triangle_size;
	a.x = -polysize/2;
	a.y = polysize/2;
	b.x = polysize/2;
	b.y = polysize/2;
	c.x = 0;
	c.y = -polysize/2;

	phi = atan2(x2 - x1, y1 - y2);
	
	tri[0] = move_point(rotate_point(a, phi), center);
	tri[1] = move_point(rotate_point(b, phi), center);
	tri[2] = move_point(rotate_point(c, phi), center);
	tri[3] = tri[0];
	
	psy_ui_drawsolidpolygon(g, tri, 4, polyInnards, self->skin.wireaacolour.value);
}

psy_ui_RealPoint rotate_point(psy_ui_RealPoint pt, double phi)
{	
	return psy_ui_realpoint_make(
		cos(phi) * pt.x - sin(phi) * pt.y,
		sin(phi) * pt.x + cos(phi) * pt.y);	
}

psy_ui_RealPoint move_point(psy_ui_RealPoint pt, psy_ui_RealPoint d)
{
	return psy_ui_realpoint_make(pt.x + d.x, pt.y + d.y);	
}

void machinewireview_drawdragwire(MachineWireView* self, psy_ui_Graphics* g)
{
	if (self->dragslot != psy_INDEX_INVALID &&
			self->dragmode >= MACHINEWIREVIEW_DRAG_NEWCONNECTION &&			
			self->dragmode <= MACHINEWIREVIEW_DRAG_RIGHTCONNECTION) {
		MachineUi* machineui;

		machineui = machineuis_at(self, self->dragslot);
		if (machineui) {
			psy_ui_RealPoint center;			
			
			center = machineui_centerposition(machineui);
			psy_ui_setcolour(g, self->skin.wirecolour);			
			psy_ui_drawline(g, center, psy_ui_realpoint_make(self->mx, self->my));
		}
	}
}

void machinewireview_drawmachines(MachineWireView* self, psy_ui_Graphics* g)
{
	psy_TableIterator it;
	
	for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {				
		MachineUi* machineui;

		machineui = (MachineUi*)psy_tableiterator_value(&it);
		machineui_draw(machineui, g, psy_tableiterator_key(&it), 
			self->vudrawupdate);
		if (self->selectedwire.src == psy_INDEX_INVALID &&
				self->selectedslot == psy_tableiterator_key(&it)) {
			machineui_drawhighlight(machineui, g);
		}
	}
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

void machinewireview_centermaster(MachineWireView* self)
{
	MachineUi* machineui;	

	machineui = machineuis_at(self, psy_audio_MASTER_INDEX);
	if (machineui) {		
		psy_ui_RealSize machinesize;
		psy_ui_RealSize size;
						
		size = psy_ui_component_sizepx(&self->component);
		machinesize = machineui_size(machineui);
		machineui_move(machineui,
			psy_ui_realpoint_make(
				(size.width - machinesize.width) / 2,
				(size.height - machinesize.height) / 2));
		psy_ui_component_invalidate(machinewireview_base(self));
	}
}

void machinewireview_onmousedoubleclick(MachineWireView* self,
	psy_ui_MouseEvent* ev)
{	
	if (ev->button == 1) {
		self->mx = ev->x;
		self->my = ev->y;
		machinewireview_hittest(self);
		if (self->dragslot == psy_INDEX_INVALID) {
			self->selectedwire = machinewireview_hittestwire(self, ev->x,
				ev->y);
			if (self->selectedwire.dst != psy_INDEX_INVALID) {
				machinewireview_showwireview(self, self->selectedwire);
				psy_ui_component_invalidate(&self->component);
			} else {
				self->dragslot = psy_INDEX_INVALID;
				self->randominsert = 0;
				return;
			}
		} else if (machinewireview_hittesteditname(self,
				psy_ui_realpoint_make(ev->x, ev->y), self->dragslot)) {
			if (machineuis_at(self, self->dragslot)) {
				psy_ui_Point scroll;
				psy_ui_RealPoint scroll_px;
				const psy_ui_TextMetric* tm;

				tm = psy_ui_component_textmetric(&self->component);
				scroll = psy_ui_component_scroll(&self->component);
				scroll_px.x = psy_ui_value_px(&scroll.x, tm);
				scroll_px.y = psy_ui_value_px(&scroll.y, tm);						
				machineui_editname(machineuis_at(self, self->dragslot),
					&self->editname, scroll_px);
				psy_ui_component_scrolltop(&self->component);
			}
		} else if (machinewireview_hittestcoord(self, ev->x, ev->y,
					MACHMODE_GENERATOR, &self->skin.generator.solo) ||
				machinewireview_hittestcoord(self, ev->x, ev->y, MACHMODE_FX,
					&self->skin.effect.bypass) ||
				machinewireview_hittestcoord(self, ev->x, ev->y,
					MACHMODE_GENERATOR, &self->skin.generator.mute) ||
				machinewireview_hittestcoord(self, ev->x, ev->y, MACHMODE_FX,
					&self->skin.effect.mute) ||
				machinewireview_hittestpan(self, ev->x, ev->y,
					self->dragslot, &self->mx)) {
			} else {
				workspace_showparameters(self->workspace, self->dragslot);
			}
		self->dragslot = psy_INDEX_INVALID;
		psy_ui_mouseevent_stoppropagation(ev);
	}
}

void machinewireview_onmousedown(MachineWireView* self, psy_ui_MouseEvent* ev)
{	
	self->mx = ev->x;
	self->my = ev->y;
	self->mousemoved = FALSE;
	psy_ui_component_hide(&self->editname.component);
	psy_ui_component_setfocus(&self->component);	
	self->dragmode = MACHINEWIREVIEW_DRAG_NONE;
	machinewireview_hittest(self);
	if (self->dragslot == psy_INDEX_INVALID) {
		if (ev->button == 1) {
			self->selectedwire = machinewireview_hittestwire(self, ev->x,
				ev->y);
			if (self->selectedwire.src != -1 && ev->shift) {				
					self->dragmode = MACHINEWIREVIEW_DRAG_LEFTCONNECTION;
					self->dragslot = self->selectedwire.src;
			}
			if (self->selectedwire.dst != -1 && ev->ctrl) {
				self->dragmode = MACHINEWIREVIEW_DRAG_RIGHTCONNECTION;
				self->dragslot = self->selectedwire.dst;				
			}
			psy_ui_component_invalidate(&self->component);
		}
	} else {		
		if (ev->button == 1) {
			if (self->dragslot != psy_audio_MASTER_INDEX) {
				self->selectedslot = self->dragslot;
				psy_audio_wire_invalidate(&self->selectedwire);				
				psy_audio_machines_select(self->machines, self->selectedslot);
			}			
			if (machinewireview_hittestcoord(self, ev->x, ev->y,
					MACHMODE_GENERATOR, &self->skin.generator.solo)) {
				psy_audio_Machine* machine = psy_audio_machines_at(
					self->machines, self->dragslot);
				if (machine) {
					psy_audio_machines_solo(self->machines, self->dragslot);
					psy_ui_component_invalidate(&self->component);
				}
				self->dragslot = psy_INDEX_INVALID;
			} else if (machinewireview_hittestcoord(self, ev->x, ev->y,
					MACHMODE_FX, &self->skin.effect.bypass)) {
				psy_audio_Machine* machine = psy_audio_machines_at(
					self->machines, self->dragslot);
				if (machine) {
					if (psy_audio_machine_bypassed(machine)) {
						psy_audio_machine_unbypass(machine);
					} else {
						psy_audio_machine_bypass(machine);
					}
					self->dragslot = psy_INDEX_INVALID;
				}
			} else if (machinewireview_hittestcoord(self, ev->x, ev->y,
					MACHMODE_GENERATOR, &self->skin.generator.mute) ||
				machinewireview_hittestcoord(self, ev->x, ev->y,
					MACHMODE_FX, &self->skin.effect.mute)) {
				psy_audio_Machine* machine;
				
				machine = psy_audio_machines_at(self->machines,
					self->dragslot);
				if (machine) {
					if (psy_audio_machine_muted(machine)) {
						psy_audio_machine_unmute(machine);
					} else {
						psy_audio_machine_mute(machine);
					}
				}
				self->dragslot = psy_INDEX_INVALID;
			} else if (machinewireview_hittestpan(self, ev->x, ev->y,
					self->dragslot, &self->mx)) {
				self->dragmode = MACHINEWIREVIEW_DRAG_PAN;				
			} else {
				MachineUi* machineui;

				machineui = machineuis_at(self, self->dragslot);
				if (machineui) {
					double x;
					double y;

					self->dragmode = MACHINEWIREVIEW_DRAG_MACHINE;
					psy_audio_machine_position(machineui->machine, &x, &y);
					self->mx = ev->x - x;
					self->my = ev->y - y;
					psy_ui_component_capture(&self->component);
				}
			}
			
		} else if (ev->button == 2) {
			psy_audio_Machine* machine;

			machine = psy_audio_machines_at(self->machines, self->dragslot);
			if (machine && psy_audio_machine_numoutputs(machine) > 0) {
				self->dragmode = MACHINEWIREVIEW_DRAG_NEWCONNECTION;
				psy_ui_component_capture(&self->component);
			} else {
				self->dragslot = psy_INDEX_INVALID;
			}			
		}
	}
}

int machinewireview_hittestpan(MachineWireView* self, double x, double y,
	uintptr_t slot, double* dx)
{	
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui && machineui->coords && machineui->machine) {
		double mx;
		double my;
		psy_ui_RealRectangle r;
		double offset;

		psy_audio_machine_position(machineui->machine, &mx, &my);
		offset = psy_audio_machine_panning(machineui->machine) *
			machineui->coords->pan.range;
		r = skincoord_destposition(&machineui->coords->pan);
		psy_ui_realrectangle_move(&r, offset, 0);		
		*dx = x - mx - r.left;
		return psy_ui_realrectangle_intersect(&r, x - mx, y - my);		
	}
	return FALSE;
}

int machinewireview_hittesteditname(MachineWireView* self, psy_ui_RealPoint pt,
	uintptr_t slot)
{		
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui && machineui->coords) {
		psy_ui_RealRectangle r;

		r = machineui_coordposition(machineui, &machineui->coords->name);		
		return psy_ui_realrectangle_intersect(&r, pt.x, pt.y);
	}
	return FALSE;
}

bool machinewireview_hittestcoord(MachineWireView* self, double x, double y,
	int mode, SkinCoord* coord)
{
	MachineUi* machineui;

	machineui = machineuis_at(self, self->dragslot);
	if (machineui && machineui->mode == mode) {		
		psy_ui_RealRectangle r;

		r = machineui_coordposition(machineui, coord);		
		return psy_ui_realrectangle_intersect(&r, x, y);
	}
	return FALSE;
}

psy_dsp_amp_t machinewireview_panvalue(MachineWireView* self, double x, double y,
	uintptr_t slot)
{
	psy_dsp_amp_t rv = 0.f;	
	MachineUi* machineui;
	
	machineui = machineuis_at(self, slot);
	if (machineui) {
		MachineCoords* coords;
		double mx;
		double my;

		psy_audio_machine_position(machineui->machine, &mx, &my);
		coords = machineui->coords;
		if (coords && coords->pan.range != 0) {
			rv = (psy_dsp_amp_t)(
				(x - (double)mx - coords->pan.dest.left - (double)self->mx) /
				(double)coords->pan.range);
		}		
	}
	return rv;
}

void machinewireview_hittest(MachineWireView* self)
{	
	psy_TableIterator it;
		
	for (it = psy_table_begin(&self->machineuis); it.curr != 0; 
			psy_tableiterator_inc(&it)) {		
		if (psy_ui_realrectangle_intersect(machineui_position((MachineUi*)
				psy_tableiterator_value(&it)), self->mx, self->my)) {
			self->dragslot = psy_tableiterator_key(&it);
			break;	
		}
	}	
}

void machinewireview_onmousemove(MachineWireView* self, psy_ui_MouseEvent* ev)
{		
	if (!self->mousemoved) {
		if (self->mx != ev->x || self->my != ev->y) {
			self->mousemoved = TRUE;
		} else {
			return;
		}
	}	
	if (self->dragslot != psy_INDEX_INVALID) {
		if (self->dragmode == MACHINEWIREVIEW_DRAG_PAN) {
			MachineUi* machineui;
			
			machineui = machineuis_at(self, self->dragslot);
			if (machineui) {
				psy_audio_machine_setpanning(machineui->machine,
					machinewireview_panvalue(self, ev->x, ev->y,
						self->dragslot));
				machineui_invalidate(machineui, machinewireview_base(self), FALSE);
			}
		} else if (machinewireview_dragging_machine(self)) {
			if (!machinewireview_dragmachine(self, self->dragslot,
					ev->x - self->mx, ev->y - self->my)) {
				self->dragmode = MACHINEWIREVIEW_DRAG_NONE;
			}			
		} else if (machinewireview_dragging_connection(self)) {
			self->mx = ev->x;
			self->my = ev->y;
			psy_ui_component_invalidate(&self->component);			
		}		
	} else if (self->showwirehover) {
		psy_audio_Wire hoverwire;
		
		hoverwire = machinewireview_hittestwire(self, ev->x, ev->y);
		if (psy_audio_wire_valid(&hoverwire)) {
			MachineUi* machineui;

			machineui = machineuis_at(self, hoverwire.dst);
			if (machineui) {				
				if (psy_ui_realrectangle_intersect(machineui_position(machineui),
						ev->x, ev->y)) {
					psy_audio_wire_invalidate(&self->hoverwire);
					psy_ui_component_invalidate(&self->component);
					return;
				}
			}
			machineui = machineuis_at(self, hoverwire.src);
			if (machineui) {				
				if (psy_ui_realrectangle_intersect(machineui_position(machineui),
						ev->x, ev->y)) {
					psy_audio_wire_invalidate(&self->hoverwire);
					psy_ui_component_invalidate(&self->component);
					return;
				}
			}
		}
		if (!psy_audio_wire_equal(&hoverwire, &self->hoverwire)) {
			self->hoverwire = hoverwire;
			psy_ui_component_invalidate(&self->component);
		}		
	}
}

bool machinewireview_dragging_machine(const MachineWireView* self)
{
	return self->dragmode == MACHINEWIREVIEW_DRAG_MACHINE;
}

bool machinewireview_dragging_connection(const MachineWireView* self)
{
	return (self->dragmode >= MACHINEWIREVIEW_DRAG_NEWCONNECTION &&
		self->dragmode <= MACHINEWIREVIEW_DRAG_RIGHTCONNECTION);
}

bool machinewireview_dragging_newconnection(const MachineWireView* self)
{
	return (self->dragmode == MACHINEWIREVIEW_DRAG_NEWCONNECTION);
}

bool machinewireview_movemachine(MachineWireView* self, uintptr_t slot,
	double dx, double dy)
{
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		return machinewireview_dragmachine(self, slot,
			machineui_position(machineui)->left + dx,
			machineui_position(machineui)->top + dy);
	}
	return FALSE;
}

bool machinewireview_dragmachine(MachineWireView* self, uintptr_t slot,
	double x, double y)
{
	MachineUi* machineui;
	psy_ui_RealRectangle r_old;
	psy_ui_RealRectangle r_new;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		r_old = *machineui_position(machineui);
		psy_ui_realrectangle_expand(&r_old, 10.0, 10.0, 10.0, 10.0);
		machineui_move(machineui,
			psy_ui_realpoint_make(
				psy_max(0.0, x), psy_max(0.0, y)));
		if (self->statusbar && machineui->machine) {
			static char txt[128];
			double mx;
			double my;

			psy_audio_machine_position(machineui->machine, &mx, &my);
			psy_snprintf(txt, 128, "%s (%d, %d)",
				(psy_audio_machine_editname(machineui->machine))
				? psy_audio_machine_editname(machineui->machine)
				: "",
				(int)mx, (int)my);
			machineviewbar_settext(self->statusbar, txt);
		}
		r_new = machinewireview_updaterect(self, self->dragslot);
		psy_ui_realrectangle_union(&r_new, &r_old);
		psy_ui_realrectangle_expand(&r_new, 10.0, 10.0, 10.0, 10.0);
		psy_ui_component_invalidaterect(&self->component, r_new);
		return TRUE;
	}
	return FALSE;
}

void machinewireview_onmouseup(MachineWireView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_releasecapture(&self->component);
	if (self->dragslot != psy_INDEX_INVALID) {
		if (machinewireview_dragging_machine(self)) {
			psy_ui_component_updateoverflow(&self->component);
			psy_ui_mouseevent_stoppropagation(ev);
		} else if (machinewireview_dragging_connection(self)) {
			if (self->mousemoved) {
				uintptr_t slot = self->dragslot;
				self->dragslot = psy_INDEX_INVALID;
				machinewireview_hittest(self);
				if (self->dragslot != psy_INDEX_INVALID) {
					if (!machinewireview_dragging_newconnection(self)) {
						psy_audio_machines_disconnect(self->machines,
							self->selectedwire);						
					}
					if (self->dragmode < MACHINEWIREVIEW_DRAG_RIGHTCONNECTION) {
						if (psy_audio_machines_valid_connection(self->machines,
							psy_audio_wire_make(slot, self->dragslot))) {
							psy_audio_machines_connect(self->machines,
								psy_audio_wire_make(slot, self->dragslot));							
						}
					} else if (psy_audio_machines_valid_connection(
						self->machines, psy_audio_wire_make(self->dragslot,
							slot))) {
						psy_audio_machines_connect(self->machines,
							psy_audio_wire_make(self->dragslot, slot));						
					}
				}
				psy_ui_mouseevent_stoppropagation(ev);
			} else if (ev->button == 2) {
				if (!self->workspace->gearvisible) {
					workspace_togglegear(self->workspace);					
				}
				psy_ui_mouseevent_stoppropagation(ev);
			}			
		}
	}
	self->dragslot = psy_INDEX_INVALID;
	psy_ui_component_invalidate(&self->component);
}

void machinewireview_onkeydown(MachineWireView* self, psy_ui_KeyEvent* ev)
{		
	if (ev->ctrl) {
		if (ev->keycode == psy_ui_KEY_B) {
			self->dragwire.src = self->selectedslot;
		} else if (ev->keycode == psy_ui_KEY_E) {
			if (self->dragwire.src != psy_INDEX_INVALID &&
					self->selectedslot != psy_INDEX_INVALID) {
				self->dragwire.dst = self->selectedslot;
				if (!psy_audio_machines_connected(self->machines,
						self->dragwire)) {				
					psy_audio_machines_connect(self->machines, self->dragwire);
				} else {
					self->selectedwire = self->dragwire;
					psy_ui_component_invalidate(&self->component);
				}
			}
		}
	} else if (ev->keycode == psy_ui_KEY_UP) {
		if (ev->shift) {
			machinewireview_movemachine(self, self->selectedslot, 0, -10);
		} else {
			uintptr_t index;

			index = machinewireview_machineup(self, self->selectedslot);
			if (index != psy_INDEX_INVALID) {
				psy_audio_machines_select(self->machines, index);
			}
		}
	} else if (ev->keycode == psy_ui_KEY_DOWN) {
		if (ev->shift) {
			machinewireview_movemachine(self, self->selectedslot, 0, 10);
		} else {
			uintptr_t index;

			index = machinewireview_machinedown(self, self->selectedslot);
			if (index != psy_INDEX_INVALID) {
				psy_audio_machines_select(self->machines, index);
			}
		}
	} else if (ev->keycode == psy_ui_KEY_LEFT) {
		if (ev->shift) {
			machinewireview_movemachine(self, self->selectedslot, -10, 0);
		} else {
			uintptr_t index;

			index = machinewireview_machineleft(self, self->selectedslot);
			if (index != psy_INDEX_INVALID) {
				psy_audio_machines_select(self->machines, index);
			}
		}
	} else if (ev->keycode == psy_ui_KEY_RIGHT) {
		if (ev->shift) {
			machinewireview_movemachine(self, self->selectedslot, 10, 0);
		} else {
			uintptr_t index;

			index = machinewireview_machineright(self, self->selectedslot);
			if (index != psy_INDEX_INVALID) {
				psy_audio_machines_select(self->machines, index);
			}
		}
	} else if (ev->keycode == psy_ui_KEY_DELETE &&
			psy_audio_wire_valid(&self->selectedwire)) {
		psy_audio_machines_disconnect(self->machines, self->selectedwire);
	} else if (ev->keycode == psy_ui_KEY_DELETE && self->selectedslot != - 1 &&
			self->selectedslot != psy_audio_MASTER_INDEX) {
		psy_audio_machines_remove(self->machines, self->selectedslot);		
		self->selectedslot = psy_INDEX_INVALID;
	} else if (ev->repeat) {
		psy_ui_keyevent_stoppropagation(ev);
	}
}

uintptr_t machinewireview_machineleft(MachineWireView* self, uintptr_t src)
{
	uintptr_t rv;
	MachineUi* srcmachineui;
	double currpos;	

	rv = psy_INDEX_INVALID;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		double srcpos;

		srcpos = machineui_position(srcmachineui)->left;
		currpos = (double)INTPTR_MAX;
		for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			MachineUi* machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			if (machineui_position(machineui)->left < srcpos) {
				if (currpos == INTPTR_MAX ||
						currpos < machineui_position(machineui)->left) {
					rv = psy_tableiterator_key(&it);
					currpos = machineui_position(machineui)->left;
				}
			}
		}
	}
	return rv;
}

uintptr_t machinewireview_machineright(MachineWireView* self, uintptr_t src)
{
	uintptr_t rv;
	MachineUi* srcmachineui;
	double currpos;

	rv = psy_INDEX_INVALID;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		double srcpos;

		srcpos = machineui_position(srcmachineui)->left;
		currpos = (double)INTPTR_MIN;
		for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			MachineUi* machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			if (machineui_position(machineui)->left > srcpos) {
				if (currpos == (double)INTPTR_MIN ||
						currpos > machineui_position(machineui)->left) {
					rv = psy_tableiterator_key(&it);
					currpos = machineui_position(machineui)->left;
				}
			}
		}
	}
	return rv;
}

uintptr_t machinewireview_machineup(MachineWireView* self, uintptr_t src)
{
	uintptr_t rv;
	MachineUi* srcmachineui;
	double currpos;

	rv = psy_INDEX_INVALID;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		double srcpos;

		srcpos = machineui_position(srcmachineui)->top;
		currpos = (double)INTPTR_MAX;
		for (it = psy_table_begin(&self->machineuis);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			MachineUi* machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			if (machineui_position(machineui)->top < srcpos) {
				if (currpos == INTPTR_MAX ||
					currpos < machineui_position(machineui)->top) {
					rv = psy_tableiterator_key(&it);
					currpos = machineui_position(machineui)->top;
				}
			}
		}
	}
	return rv;
}

uintptr_t machinewireview_machinedown(MachineWireView* self, uintptr_t src)
{
	uintptr_t rv;
	MachineUi* srcmachineui;
	double currpos;

	rv = psy_INDEX_INVALID;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		double srcpos;

		srcpos = machineui_position(srcmachineui)->bottom;
		currpos = INTPTR_MIN;
		for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			MachineUi* machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			if (machineui_position(machineui)->bottom > srcpos) {
				if (currpos == (double)INTPTR_MIN ||
					currpos > machineui_position(machineui)->bottom) {
					rv = psy_tableiterator_key(&it);
					currpos = machineui_position(machineui)->bottom;
				}
			}
		}
	}
	return rv;
}

psy_audio_Wire machinewireview_hittestwire(MachineWireView* self, double x,
	double y)
{		
	psy_audio_Wire rv;
	psy_TableIterator it;
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);	
	psy_audio_wire_init(&rv);
	for (it = psy_audio_machines_begin(self->machines); it.curr != 0; 
			psy_tableiterator_inc(&it)) {
		psy_audio_MachineSockets* sockets;			
		uintptr_t slot = it.curr->key;
	
		sockets	= psy_audio_connections_at(&self->machines->connections, slot);
		if (sockets) {
			psy_TableIterator it;

			for (it = psy_audio_wiresockets_begin(&sockets->outputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
				psy_audio_WireSocket* socket;

				socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);											
				if (socket->slot != psy_INDEX_INVALID) {
					MachineUi* inmachineui;
					MachineUi* outmachineui;

					inmachineui = machineuis_at(self, socket->slot);
					outmachineui = machineuis_at(self, slot);
					if (inmachineui && outmachineui) {
						psy_ui_RealRectangle r;
						psy_ui_RealSize out;
						psy_ui_RealSize in;
						double d = 4;
						double mxout;
						double myout;
						double mxin;
						double myin;

						psy_audio_machine_position(outmachineui->machine,
							&mxout, &myout);
						psy_audio_machine_position(inmachineui->machine,
							&mxin, &myin);

						out = machineui_size(outmachineui);
						in = machineui_size(inmachineui);
						psy_ui_setrectangle(&r, x - d, y - d, 2 * d, 2 * d);
						if (psy_ui_realrectangle_intersect_segment(&r,
								mxout + out.width / 2, myout + out.height / 2,
								mxin + in.width / 2, myin + in.height / 2)) {
							psy_audio_wire_set(&rv, slot, socket->slot);
						}						
					}
				}
				if (psy_audio_wire_valid(&rv)) {
					break;
				}				
			}
		}
	}
	return rv;
}

void machinewireview_onmachineselected(MachineWireView* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	self->selectedslot = slot;
	psy_audio_wire_invalidate(&self->selectedwire);	
	psy_ui_component_invalidate(&self->component);
	psy_ui_component_setfocus(&self->component);
}

void machinewireview_onwireselected(MachineWireView* self,
	psy_audio_Machines* sender, psy_audio_Wire wire)
{
	self->selectedwire = wire;
	psy_ui_component_invalidate(&self->component);
}

void machinewireview_onmachinesinsert(MachineWireView* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(self->machines, slot);
	if (machine) {
		MachineUi* machineui;

		machineui = machineuis_insert(self, slot);
		if (machineui && !self->randominsert) {
			double width;
			double height;

			width = machineui_position(machineui)->right -
				machineui_position(machineui)->left;
			height = machineui_position(machineui)->bottom -
				machineui_position(machineui)->top;
			machineui_move(machineui,
				psy_ui_realpoint_make(
					psy_max(0.0, self->mx - width / 2),
					psy_max(0.0, self->my - height / 2)));
		}
		psy_ui_component_updateoverflow(&self->component);
		psy_ui_component_invalidate(&self->component);
		self->randominsert = 1;
	}
}

void machinewireview_onmachinesremoved(MachineWireView* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	machineuis_remove(self, slot);
	psy_ui_component_updateoverflow(&self->component);
	psy_ui_component_invalidate(&self->component);
}

void machinewireview_onconnected(MachineWireView* self,
	psy_audio_Connections* sender, uintptr_t src, uintptr_t dst)
{
	psy_ui_component_invalidate(&self->component);
}

void machinewireview_ondisconnected(MachineWireView* self,
	psy_audio_Connections* sender, uintptr_t src, uintptr_t dst)
{
	psy_ui_component_invalidate(&self->component);
}

void machinewireview_buildmachineuis(MachineWireView* self)
{
	if (self->machines) {
		psy_TableIterator it;

		machineuis_removeall(self);
		for (it = psy_audio_machines_begin(self->machines);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			if (self->drawvirtualgenerators ||
					!(psy_tableiterator_key(&it) > 0x80 &&
					psy_tableiterator_key(&it) <= 0xFE)) {
				psy_audio_Machine* machine;

				machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
				machineuis_insert(self, psy_tableiterator_key(&it));
			}
		}
		if (psy_audio_machines_size(self->machines) == 1) {
			// if only master exists, center
			machinewireview_centermaster(self);
		}
	}
}

void machinewireview_onsongchanged(MachineWireView* self, Workspace* sender,
	int flag, psy_audio_SongFile* songfile)
{	
	self->machines = &sender->song->machines;
	machinewireview_buildmachineuis(self);	
	machinewireview_connectsong(self);
	psy_ui_component_setscroll(&self->component, psy_ui_point_zero());	
	psy_ui_component_updateoverflow(&self->component);
	psy_ui_component_invalidate(&self->component);	
}

void machinewireview_onshowparameters(MachineWireView* self, Workspace* sender,
	uintptr_t slot)
{	
	if (machineuis_at(self, slot)) {
		machineui_showparameters(machineuis_at(self, slot), &self->component);
	}
}

void machinewireview_onmasterworked(MachineWireView* self,
	psy_audio_Machine* machine, uintptr_t slot, psy_audio_BufferContext* bc)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		MachineUi* machineui;
		
		machineui = psy_tableiterator_value(&it);
		if (machineui->slot != psy_audio_MASTER_INDEX) {		
			machineui_updatevolumedisplay(machineui);
		}
	}
}

void machinewireview_ontimer(MachineWireView* self, uintptr_t timerid)
{	
	psy_TableIterator it;
	bool updatevus;
	
	machinewireview_destroywireframes(self);
	self->vudrawupdate = TRUE;
	updatevus = psy_ui_component_drawvisible(&self->component) &&
		self->skin.drawvumeters;
	if (updatevus) {
		psy_ui_component_setbackgroundmode(&self->component,
			psy_ui_BACKGROUND_NONE);
	}	
	for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		MachineUi* machineui;

		machineui = psy_tableiterator_value(&it);
		if (machineui->frame && machineui->frame->doclose) {
			machinewireview_closeparamview(self, machineui);			
		}
		if (updatevus) {
			machineui_invalidate(machineui, machinewireview_base(self), TRUE);
		}
	}	
	if (updatevus) {
		psy_ui_component_update(machinewireview_base(self));		
		psy_ui_component_setbackgroundmode(machinewireview_base(self),
			psy_ui_BACKGROUND_SET);
	}
	self->vudrawupdate = FALSE;
}

void machinewireview_destroywireframes(MachineWireView* self)
{
	psy_List* p;
	psy_List* q;

	for (p = self->wireframes; p != NULL; p = q) {
		WireFrame* frame;

		frame = (WireFrame*)psy_list_entry(p);
		q = p->next;
		if (frame->wireview && !wireview_wireexists(frame->wireview)) {
			psy_ui_component_destroy(&frame->wireview->component);
			free(frame->wireview);
			psy_ui_component_destroy(&frame->component);
			free(frame);
			psy_list_remove(&self->wireframes, p);
		}
	}
}

void machinewireview_closeparamview(MachineWireView* self, MachineUi* machineui)
{
	psy_ui_Component* dockparent;

	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_SET);	
	dockparent = psy_ui_component_parent(
		&machineui->frame->component);
	psy_ui_component_destroy(&machineui->frame->component);
	psy_ui_component_align(psy_ui_component_parent(dockparent));
	psy_ui_component_align(dockparent);
}

void machinewireview_showwireview(MachineWireView* self, psy_audio_Wire wire)
{			
	WireFrame* wireframe;

	if (!self->workspace->song) {
		return;
	}
	wireframe = machinewireview_wireframe(self, wire);
	if (!wireframe) {
		WireView* wireview;
		char buf[128];
		psy_audio_Machines* machines;
		psy_audio_Machine* srcmachine;
		psy_audio_Machine* dstmachine;

		// WireFrame
		wireframe = (WireFrame*)malloc(sizeof(WireFrame));
		if (!wireframe) {
			return;
		}
		psy_list_append(&self->wireframes, wireframe);	
		wireframe_init(wireframe, &self->component, 0);		
		machines = &self->workspace->song->machines;		
		srcmachine = psy_audio_machines_at(machines, wire.src);
		dstmachine = psy_audio_machines_at(machines, wire.dst);
		psy_snprintf(buf, 128, "[%d] %s -> %s Connection Volume",
			(int)psy_audio_connections_wireindex(&machines->connections, wire),
			(srcmachine) ? psy_audio_machine_editname(srcmachine) : "ERR",
			(dstmachine) ? psy_audio_machine_editname(dstmachine) : "ERR");
		psy_ui_component_settitle(wireframe_base(wireframe), buf);
		psy_signal_connect(&wireframe->component.signal_destroyed, self,
			machinewireview_onwireframedestroyed);
		// WireView
		wireview = (WireView*) malloc(sizeof(WireView));
		if (wireview) {
			wireview_init(wireview, &wireframe->component, wire,
				self->workspace);
			wireframe->wireview = wireview;						
		} else {
			psy_ui_component_destroy(&wireframe->component);
			free(wireframe);
			wireframe = NULL;
		}
	}
	if (wireframe != NULL) {
		psy_ui_component_show(&wireframe->component);
	}
}

void machinewireview_onwireframedestroyed(MachineWireView* self,
	psy_ui_Component* sender)
{
	psy_List* p;
	psy_List* q;

	for (p = self->wireframes; p != NULL; p = q) {
		WireFrame* frame;

		frame = (WireFrame*)psy_list_entry(p);
		q = p->next;
		if (&frame->component == sender) {
			psy_list_remove(&self->wireframes, p);
		}
	}
}

WireFrame* machinewireview_wireframe(MachineWireView* self,
	psy_audio_Wire wire)
{
	WireFrame* rv;
	psy_List* framenode;

	rv = NULL;
	framenode = self->wireframes;
	while (framenode != NULL) {
		WireFrame* frame;

		frame = (WireFrame*)framenode->entry;
		if (frame->wireview && frame->wireview->wire.dst == wire.dst && 
				frame->wireview->wire.src == wire.src) {
			rv = frame;
			break;
		}
		psy_list_next(&framenode);
	}
	return rv;
}

void machinewireview_onnewmachineselected(MachineView* self,
	psy_ui_Component* sender, psy_Property* plugininfo)
{		
	psy_audio_Machine* machine;
	
	machine = psy_audio_machinefactory_makemachinefrompath(
		&self->workspace->machinefactory,
		psy_property_at_int(plugininfo, "type", UINTPTR_MAX),
		psy_property_at_str(plugininfo, "path", ""),
		psy_property_at_int(plugininfo, "shellidx", 0),
		UINTPTR_MAX);
	if (machine) {
		intptr_t favorite;

		favorite = psy_property_at_int(plugininfo, "favorite", 0);
		psy_property_set_int(plugininfo, "favorite", ++favorite);
		if (self->wireview.addeffect) {
			uintptr_t slot;

			slot = psy_audio_machines_append(self->wireview.machines, machine);
			psy_audio_machines_disconnect(self->wireview.machines,
				self->wireview.selectedwire);
			psy_audio_machines_connect(self->wireview.machines,
				psy_audio_wire_make(self->wireview.selectedwire.src, slot));
			psy_audio_machines_connect(self->wireview.machines,
				psy_audio_wire_make(slot, self->wireview.selectedwire.dst));
			psy_audio_machines_select(self->wireview.machines, slot);
			self->wireview.addeffect = 0;
		} else if (self->newmachine.mode == NEWMACHINE_INSERT) {
			psy_audio_machines_insert(self->wireview.machines,
				psy_audio_machines_selected(self->wireview.machines), machine);
		} else {
			psy_audio_machines_select(self->wireview.machines,
				psy_audio_machines_append(self->wireview.machines, machine));
		}
		tabbar_select(&self->tabbar, 0);
	} else {
		workspace_outputerror(self->workspace,
			self->workspace->machinefactory.errstr);
	}
}

MachineUi* machineuis_insert(MachineWireView* self, uintptr_t slot)
{	
	if (psy_audio_machines_at(self->machines, slot)) {
		MachineUi* rv;

		if (psy_table_exists(&self->machineuis, slot)) {
			machineuis_remove(self, slot);
		}	
		rv = (MachineUi*)malloc(sizeof(MachineUi));
		if (rv) {
			machineui_init(rv, slot, &self->skin, self->workspace);
			psy_table_insert(&self->machineuis, slot, rv);
		}
		return rv;
	}
	return NULL;
}

MachineUi* machineuis_at(MachineWireView* self, uintptr_t slot)
{
	return psy_table_at(&self->machineuis, slot);
}

void machineuis_remove(MachineWireView* self, uintptr_t slot)
{
	MachineUi* machineui;

	machineui = (MachineUi*) psy_table_at(&self->machineuis, slot);
	if (machineui) {
		machineui_dispose(machineui);
		free(machineui);
		psy_table_remove(&self->machineuis, slot);
	}
}

void machineuis_removeall(MachineWireView* self)
{
	psy_table_disposeall(&self->machineuis, (psy_fp_disposefunc)
		machineui_dispose);	
	psy_table_init(&self->machineuis);
}

// MachineViewBar
static void machineviewbar_onsongchanged(MachineViewBar*, Workspace*,
	int flag, psy_audio_SongFile*);
static void machineviewbar_onmixerconnectmodeclick(MachineViewBar*,
	psy_ui_Component* sender);
static void machineviewbar_onmachinesinsert(MachineViewBar*,
	psy_audio_Machines*, uintptr_t slot);
static void machineviewbar_onmachinesremoved(MachineViewBar*,
	psy_audio_Machines*, uintptr_t slot);

void machineviewbar_init(MachineViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(machineviewbar_base(self), parent);
	self->workspace = workspace;				
	psy_ui_component_setdefaultalign(machineviewbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_margin_make(
			psy_ui_value_makepx(0), psy_ui_value_makeew(4),
			psy_ui_value_makepx(0), psy_ui_value_makepx(0)));
	psy_ui_checkbox_init(&self->mixersend, machineviewbar_base(self));
	psy_ui_checkbox_check(&self->mixersend);
	psy_ui_checkbox_settext(&self->mixersend,
		"machineview.connect-to-mixer-send-return-input");
	psy_ui_component_hide(psy_ui_checkbox_base(&self->mixersend));
	psy_signal_connect(&self->mixersend.signal_clicked, self,
		machineviewbar_onmixerconnectmodeclick);		
	psy_ui_label_init(&self->status, machineviewbar_base(self));
	psy_ui_label_preventtranslation(&self->status);
	psy_ui_label_setcharnumber(&self->status, 44);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		machineviewbar_onsongchanged);
	if (workspace->song) {
		psy_signal_connect(&workspace->song->machines.signal_insert, self,
			machineviewbar_onmachinesinsert);
		psy_signal_connect(&workspace->song->machines.signal_removed, self,
			machineviewbar_onmachinesremoved);
	}
}

void machineviewbar_settext(MachineViewBar* self, const char* text)
{	
	psy_ui_label_settext(&self->status, text);
	psy_ui_label_fadeout(&self->status);
}

void machineviewbar_onmixerconnectmodeclick(MachineViewBar* self,
	psy_ui_Component* sender)
{
	if (psy_ui_checkbox_checked(&self->mixersend)) {
		workspace_connectasmixersend(self->workspace);
	} else {
		workspace_connectasmixerinput(self->workspace);
	}    
}

void machineviewbar_onsongchanged(MachineViewBar* self, Workspace* workspace,
	int flag, psy_audio_SongFile* songfile)
{
	if (!workspace_song(self->workspace) ||
			psy_audio_machines_isconnectasmixersend(
				psy_audio_song_machines(workspace_song(self->workspace)))) {
		psy_ui_checkbox_check(&self->mixersend);
	} else {
		psy_ui_checkbox_disablecheck(&self->mixersend);		
	}
	if (workspace->song) {
		psy_signal_connect(&workspace->song->machines.signal_insert, self,
			machineviewbar_onmachinesinsert);
		psy_signal_connect(&workspace->song->machines.signal_removed, self,
			machineviewbar_onmachinesremoved);
	}
}

void machineviewbar_onmachinesinsert(MachineViewBar* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	if (psy_audio_machines_hasmixer(sender)) {		
		psy_ui_component_show_align(psy_ui_checkbox_base(&self->mixersend));
	}
	if (psy_audio_machines_at(sender, slot)) {
		char text[128];

		psy_snprintf(text, 128, "%s inserted at slot %u",
			psy_audio_machine_editname(psy_audio_machines_at(sender, slot)),
			(unsigned int)slot);
		machineviewbar_settext(self, text);
	}	
}

void machineviewbar_onmachinesremoved(MachineViewBar* self,
	psy_audio_Machines* sender, uintptr_t slot)
{	
	char text[128];

	if (!psy_audio_machines_hasmixer(sender)) {		
		psy_ui_component_hide_align(psy_ui_checkbox_base(&self->mixersend));
	}		
	psy_snprintf(text, 128, "Machine removed from slot %u",
		(unsigned int)slot);
	machineviewbar_settext(self, text);	
}

// MachineView
static void machineview_updatetext(MachineView*, psy_Translator*);
static void machineview_onsongchanged(MachineView*, Workspace*, int flag,
	psy_audio_SongFile*);
static void machineview_onmousedown(MachineView*, psy_ui_MouseEvent*);
static void machineview_onmouseup(MachineView*, psy_ui_MouseEvent*);
static void machineview_onmousedoubleclick(MachineView*, psy_ui_MouseEvent*);
static void machineview_onkeydown(MachineView*, psy_ui_KeyEvent*);
static void machineview_onfocus(MachineView*, psy_ui_Component* sender);
static void machineview_selectsection(MachineView*, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options);
static void machineview_onconfigure(MachineView*, MachineViewConfig*,
	psy_Property*);
static void machineview_onthemechanged(MachineView*, MachineViewConfig*,
	psy_Property* theme);

static psy_ui_ComponentVtable machineview_vtable;
static bool machineview_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machineview_vtable_init(MachineView* self)
{
	if (!machineview_vtable_initialized) {
		machineview_vtable = *(self->component.vtable);
		machineview_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			machineview_onmousedown;
		machineview_vtable.onmouseup = (psy_ui_fp_component_onmouseup)
			machineview_onmouseup;
		machineview_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmousedoubleclick)
			machineview_onmousedoubleclick;
		machineview_vtable.onkeydown = (psy_ui_fp_component_onkeydown)
			machineview_onkeydown;
		machineview_vtable_initialized = TRUE;
	}
	return &machineview_vtable;
}

void machineview_init(MachineView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	psy_ui_Margin leftmargin;
	
	psy_ui_component_init(machineview_base(self), parent);
	machineview_vtable_init(self);
	psy_ui_component_setvtable(machineview_base(self),
		machineview_vtable_init(self));
	self->workspace = workspace;	
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		machineview_onsongchanged);
	psy_ui_notebook_init(&self->notebook, &self->component);	
	psy_ui_margin_init_all(&leftmargin,
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeew(3.0));
	psy_ui_component_setmargin(psy_ui_notebook_base(&self->notebook),
		&leftmargin);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);	
	machinewireview_init(&self->wireview,
		psy_ui_notebook_base(&self->notebook), tabbarparent, NULL, workspace);
	psy_ui_scroller_init(&self->scroller, &self->wireview.component,
		psy_ui_notebook_base(&self->notebook));
	self->wireview.scroller = &self->scroller;
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	newmachine_init(&self->newmachine, psy_ui_notebook_base(&self->notebook),
		&self->wireview.skin, self->workspace);		
	psy_ui_component_setalign(&self->newmachine.component,
		psy_ui_ALIGN_CLIENT);
	tabbar_init(&self->tabbar, tabbarparent);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	tabbar_append_tabs(&self->tabbar, "machineview.wires",
		"machineview.new-machine", NULL);
	psy_signal_connect(&self->component.signal_selectsection, self,
		machineview_selectsection);
	psy_ui_notebook_select(&self->notebook, SECTION_ID_MACHINEVIEW_WIRES);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	psy_signal_connect(&self->newmachine.signal_selected, self,
		machinewireview_onnewmachineselected);			
	psy_signal_connect(&self->component.signal_focus, self,
		machineview_onfocus);
	psy_signal_connect(
		&psycleconfig_macview(workspace_conf(workspace))->signal_themechanged,
		self, machineview_onthemechanged);
	psy_signal_connect(
		&psycleconfig_macview(workspace_conf(workspace))->signal_changed,
		self, machineview_onconfigure);		
}

void machineview_onmousedoubleclick(MachineView* self, psy_ui_MouseEvent* ev)
{		
	if (ev->button == 1 && tabbar_selected(&self->tabbar) ==
			SECTION_ID_MACHINEVIEW_WIRES) {
		psy_ui_component_selectsection(machineview_base(self),
			SECTION_ID_MACHINEVIEW_NEWMACHINE,
			NEWMACHINE_APPEND);
	}
}

void machineview_onmousedown(MachineView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_mouseevent_stoppropagation(ev);
}

void machineview_onmouseup(MachineView* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 2) {
		if (tabbar_selected(&self->tabbar) == SECTION_ID_MACHINEVIEW_WIRES) {
			workspace_togglegear(self->workspace);
		} else if (tabbar_selected(&self->tabbar) ==
				SECTION_ID_MACHINEVIEW_NEWMACHINE) {
			tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);			
		}
	}
}

void machineview_onkeydown(MachineView* self, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_ESCAPE) {
		if (tabbar_selected(&self->tabbar) ==
				SECTION_ID_MACHINEVIEW_NEWMACHINE) {
			tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);
			psy_ui_component_setfocus(machinewireview_base(&self->wireview));
		} else if (self->workspace->gearvisible) {
			workspace_togglegear(self->workspace);
		}
		psy_ui_keyevent_stoppropagation(ev);
	} 
}

void machineview_onfocus(MachineView* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(machinewireview_base(&self->wireview));
}

psy_ui_RealRectangle machinewireview_bounds(MachineWireView* self)
{
	psy_ui_RealRectangle rv;
	psy_TableIterator it;

	rv = psy_ui_realrectangle_zero();
	for (it = psy_table_begin(&self->machineuis); 
			!psy_tableiterator_equal(&it, psy_table_end()); 
			psy_tableiterator_inc(&it)) {		
		psy_ui_realrectangle_union(&rv, machineui_position(
			((MachineUi*)psy_tableiterator_value(&it))));
	}
	psy_ui_realrectangle_expand(&rv, 0.0, 10.0, 10.0, 0.0);
	return rv;
}

psy_ui_RealRectangle machinewireview_updaterect(MachineWireView* self,
	uintptr_t slot)
{		
	psy_ui_RealRectangle rv;	
	MachineUi* machineui;
	
	machineui = machineuis_at(self, slot);
	if (machineui) {		
		psy_audio_MachineSockets* sockets;

		rv = *machineui_position(machineui);
		sockets = psy_audio_connections_at(&self->machines->connections, slot);
		if (sockets) {
			psy_TableIterator it;

			for (it = psy_audio_wiresockets_begin(&sockets->outputs);
					!psy_tableiterator_equal(&it, psy_table_end());
					psy_tableiterator_inc(&it)) {
				psy_audio_WireSocket* socket;

				socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);			
				if (socket->slot != UINTPTR_MAX) {
					MachineUi* inmachineui;

					inmachineui = machineuis_at(self, socket->slot);
					if (inmachineui && machineui) {												
						psy_ui_realrectangle_union(&rv,
							machineui_position(inmachineui));
					}
				}
			}
			for(it = psy_audio_wiresockets_begin(&sockets->inputs);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
				psy_audio_WireSocket* socket;

				socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
				if (socket->slot != psy_INDEX_INVALID) {
					MachineUi* outmachineui;

					outmachineui = machineuis_at(self, socket->slot);
					if (outmachineui && machineui) {						
						psy_ui_realrectangle_union(&rv,
							machineui_position(outmachineui));
					}
				}
			}
		}
		return rv;
	}
	return psy_ui_realrectangle_zero();
}

void machinewireview_onpreferredsize(MachineWireView* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_RealRectangle bounds;

	bounds = machinewireview_bounds(self);
	psy_ui_size_setpx(rv, bounds.right, bounds.bottom);	
}

void machineview_selectsection(MachineView* self, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options)
{
	if (section == SECTION_ID_MACHINEVIEW_NEWMACHINE) {		
		if (options & NEWMACHINE_INSERT) {
			newmachine_insertmode(&self->newmachine);
		} else if (options & NEWMACHINE_APPEND) {
			newmachine_appendmode(&self->newmachine);
		} else if (options & NEWMACHINE_ADDEFFECT) {
			self->wireview.addeffect = 1;
			self->wireview.randominsert = 0;
			newmachine_addeffectmode(&self->newmachine);
		}
	}
	tabbar_select(&self->tabbar, section);
}

void machineview_onsongchanged(MachineView* self, Workspace* workspace,
	int flag, psy_audio_SongFile* songfile)
{
	tabbar_select(&self->tabbar, SECTION_ID_MACHINEVIEW_WIRES);
}

void machineview_onconfigure(MachineView* self, MachineViewConfig* sender,
	psy_Property* property)
{
	machinewireview_configure(&self->wireview, sender);
	if (psy_property_hasid(property, PROPERTY_ID_DRAWVIRTUALGENERATORS)) {
		machinewireview_buildmachineuis(&self->wireview);
	}
	psy_ui_component_invalidate(&self->component);
}

void machineview_onthemechanged(MachineView* self, MachineViewConfig* sender,
	psy_Property* theme)
{
	machinewireview_updateskin(&self->wireview);
	newmachine_updateskin(&self->newmachine);
	psy_ui_component_invalidate(&self->component);
}

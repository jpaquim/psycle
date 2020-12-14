// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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
#include <stdlib.h>
#include <string.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static void machineui_init(MachineUi*, psy_audio_Machine*, uintptr_t slot,
	MachineViewSkin*, Workspace*);
static void machineui_updatecoords(MachineUi*);
static void machineui_dispose(MachineUi*);
static void machineui_onframedestroyed(MachineUi*, psy_ui_Component* sender);
static psy_ui_IntSize machineui_size(MachineUi*);
static psy_ui_Rectangle machineui_position(MachineUi*);
static psy_ui_Rectangle machineui_vuposition(MachineUi*);
static void machineui_draw(MachineUi*, psy_ui_Graphics*, MachineWireView*, uintptr_t slot);
static void machineui_drawvu(MachineUi*, psy_ui_Graphics*, MachineWireView*);
static void machineui_drawvudisplay(MachineUi*, psy_ui_Graphics*, MachineWireView*);
static void machineui_drawvupeak(MachineUi*, psy_ui_Graphics*, MachineWireView*);
static void machineui_drawhighlight(MachineUi*, psy_ui_Graphics*, MachineWireView*);
static void machineui_updatevolumedisplay(MachineUi*);
static void machineui_updatemaxvolumedisplay(MachineUi*);
static void machineui_showparameters(MachineUi*, psy_ui_Component* parent);
static void machineui_editname(MachineUi*, psy_ui_Edit*, psy_ui_IntPoint scroll);
static void machineui_onkeydown(MachineUi*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void machineui_oneditchange(MachineUi*, psy_ui_Edit* sender);
static void machineui_oneditfocuslost(MachineUi*, psy_ui_Component* sender);
static void machineui_invalidate(MachineUi*, MachineWireView*, bool vu);

// MachineUi
void machineui_init(MachineUi* self, psy_audio_Machine* machine,
	uintptr_t slot, MachineViewSkin* skin, Workspace* workspace)
{	
	assert(self);
	assert(machine);
	assert(workspace);

	self->machine = machine;	
	self->workspace = workspace;	
	self->skin = skin;	
	self->mode = psy_audio_machine_mode(machine);
	self->volumedisplay = (psy_dsp_amp_t)0.f;
	self->volumemaxdisplay = (psy_dsp_amp_t)0.f;
	self->volumemaxcounterlife = 0;
	self->coords = NULL;
	self->slot = slot;
	self->frame = NULL;
	self->paramview = NULL;
	self->editorview = NULL;
	self->restorename = NULL;
	machineui_updatecoords(self);	
}

void machineui_updatecoords(MachineUi* self)
{	
	switch (self->mode) {
		case MACHMODE_MASTER: 
			self->coords = &self->skin->master;
		break;	
		case MACHMODE_GENERATOR:
			self->coords = &self->skin->generator;
		break;
		default:
			self->coords = &self->skin->effect;
		break;
	}	
}

void machineui_dispose(MachineUi* self)
{
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

psy_ui_IntSize machineui_size(MachineUi* self)
{	
	if (self->coords) {
		return  psy_ui_intsize_make(
			self->coords->background.destwidth,
			self->coords->background.destheight);
	}
	return psy_ui_intsize_make(200, 20);
}

psy_ui_Rectangle machineui_position(MachineUi* self)
{
	if (self->coords && self->machine) {
		intptr_t x;
		intptr_t y;

		psy_audio_machine_position(self->machine, &x, &y);	
		return psy_ui_rectangle_make(x, y, self->coords->background.destwidth,
			self->coords->background.destheight);
	}
	return psy_ui_rectangle_make(0, 0, 200, 20);
}

psy_ui_Rectangle machineui_vuposition(MachineUi* self)
{
	if (self->coords && self->machine) {
		intptr_t x;
		intptr_t y;

		psy_audio_machine_position(self->machine, &x, &y);
		return psy_ui_rectangle_make(x + self->coords->vu0.destx,
			y + self->coords->vu0.desty, self->coords->vu0.destwidth,
			self->coords->vu0.destheight);
	}
	return psy_ui_rectangle_make(0, 0, 200, 20);
}

void machineui_editname(MachineUi* self, psy_ui_Edit* edit,
	psy_ui_IntPoint scroll)
{
	if (self->machine) {
		psy_ui_Rectangle r;
		intptr_t x;
		intptr_t y;
		
		free(self->restorename);
		self->restorename = (psy_audio_machine_editname(self->machine))
				? strdup(psy_audio_machine_editname(self->machine))
				: NULL;
		psy_signal_disconnectall(&edit->component.signal_focuslost);
		psy_signal_disconnectall(&edit->component.signal_keydown);
		psy_signal_disconnectall(&edit->signal_change);
		psy_signal_connect(&edit->signal_change, self,
			machineui_oneditchange);
		psy_signal_connect(&edit->component.signal_keydown, self,
			machineui_onkeydown);		
		psy_signal_connect(&edit->component.signal_focuslost, self,
			machineui_oneditfocuslost);
		psy_ui_edit_settext(edit, psy_audio_machine_editname(self->machine));
		r = skincoord_destposition(&self->coords->name);
		psy_audio_machine_position(self->machine, &x, &y);
		psy_ui_rectangle_move(&r, x - scroll.x, y - scroll.y);
		psy_ui_component_setposition(&edit->component,
			psy_ui_point_makepx(r.left, r.top),
			psy_ui_size_makepx(r.right - r.left, r.bottom - r.top));
		psy_ui_component_show(&edit->component);
	}
}

void machineui_onkeydown(MachineUi* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{	
	if (ev->keycode == psy_ui_KEY_RETURN) {		
		psy_ui_component_hide(sender);
		psy_ui_keyevent_preventdefault(ev);
	} else
	if (ev->keycode == psy_ui_KEY_ESCAPE) {
		if (self->machine) {
			psy_audio_machine_seteditname(self->machine, self->restorename);
			free(self->restorename);
			self->restorename = 0;			
		}
		psy_ui_keyevent_preventdefault(ev);
		psy_ui_component_hide(sender);
	}
	psy_ui_keyevent_stoppropagation(ev);
}

void machineui_oneditchange(MachineUi* self, psy_ui_Edit* sender)
{
	if (self->machine) {
		psy_audio_machine_seteditname(self->machine, psy_ui_edit_text(sender));
	}
}

void machineui_oneditfocuslost(MachineUi* self, psy_ui_Component* sender)
{
	psy_ui_component_hide(sender);
}

void machineui_draw(MachineUi* self, psy_ui_Graphics* g,
	MachineWireView* wireview, uintptr_t slot)
{	
	psy_ui_Rectangle r;

	r = machineui_position(self);	
	if (self->coords && psy_ui_rectangle_intersect_rectangle(&g->clip, &r)) {
		MachineCoords* coords;
		char editname[130];

		editname[0] = '\0';
		coords = self->coords;		
		
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		if (!psy_ui_bitmap_empty(&wireview->skin.skinbmp)) {
			skin_blitpart(g, &wireview->skin.skinbmp, r.left, r.top,
				&self->coords->background);
		} else {
			psy_ui_Colour bgcolour;

			if (self->mode == MACHMODE_MASTER) {
				bgcolour = psy_ui_colour_make(0x00333333);
			} else
				if (self->mode == MACHMODE_FX) {
					bgcolour = psy_ui_colour_make(0x003E2f25);
				} else {
					bgcolour = psy_ui_colour_make(0x002f3E25);
				}
			psy_ui_drawsolidrectangle(g, r, bgcolour);
			if (self->mode == MACHMODE_MASTER) {
				psy_ui_Rectangle clip;

				clip = psy_ui_rectangle_make(r.left + coords->name.destx,
					r.top + coords->name.desty, coords->name.destwidth,
					coords->name.destheight);
				psy_ui_textoutrectangle(g, r.left + coords->name.destx,
					r.top + coords->name.desty,
					psy_ui_ETO_CLIPPED, clip,
					"Master", strlen("Master"));
			}
		}
		if (!wireview->drawvumode) {
			if (psy_audio_machine_editname(self->machine)) {
				if (self->skin->drawmachineindexes) {
					psy_snprintf(editname, 130, "%.2X:%s", (int)slot,
						psy_audio_machine_editname(self->machine));
				} else {
					psy_snprintf(editname, 130, "%s",
						psy_audio_machine_editname(self->machine));
				}
			}
			if (self->mode == MACHMODE_FX) {
				psy_ui_settextcolour(g, wireview->skin.effect_fontcolour);
			} else {
				psy_ui_settextcolour(g, wireview->skin.generator_fontcolour);;
			}
			if (self->mode != MACHMODE_MASTER) {
				psy_ui_Rectangle clip;

				clip = psy_ui_rectangle_make(r.left + coords->name.destx,
					r.top + coords->name.desty, coords->name.destwidth,
					coords->name.destheight);
				psy_ui_textoutrectangle(g, r.left + coords->name.destx,
					r.top + coords->name.desty,
					psy_ui_ETO_CLIPPED, clip,
					editname, strlen(editname));
				skin_blitpart(g, &wireview->skin.skinbmp,
					r.left + skincoord_position(&coords->pan,
						psy_audio_machine_panning(self->machine)), r.top,
					&coords->pan);
				if (psy_audio_machine_muted(self->machine)) {
					skin_blitpart(g, &wireview->skin.skinbmp, r.left, r.top,
						&coords->mute);
				}
				if ((psy_audio_machine_mode(self->machine) == MACHMODE_FX) &&
					psy_audio_machine_bypassed(self->machine)) {
					skin_blitpart(g, &wireview->skin.skinbmp, r.left, r.top,
						&coords->bypass);
				}
				if ((psy_audio_machine_mode(self->machine) == MACHMODE_GENERATOR) &&
					psy_audio_machines_soloed(wireview->machines) == self->slot) {
					skin_blitpart(g, &wireview->skin.skinbmp, r.left, r.top,
						&coords->solo);
				}
			}
		}
		if (wireview->drawvumeters) {
			machineui_drawvu(self, g, wireview);
		}
	}
}

void machineui_drawvu(MachineUi* self, psy_ui_Graphics* g,
	MachineWireView* wireview)
{				
	if (self->coords && self->mode != MACHMODE_MASTER) {
		machineui_drawvudisplay(self, g, wireview);
		machineui_drawvupeak(self, g, wireview);
	}
}

void machineui_drawvudisplay(MachineUi* self, psy_ui_Graphics* g,
	MachineWireView* wireview)
{
	if (self->machine) {
		SkinCoord* vu;
		intptr_t x;
		intptr_t y;

		psy_audio_machine_position(self->machine, &x, &y);
		vu = &self->coords->vu0;
		psy_ui_drawbitmap(g, &wireview->skin.skinbmp,
			x + vu->destx, y + vu->desty,
			(int)(self->volumedisplay * vu->destwidth), vu->destheight,
			vu->srcx, vu->srcy);
	}
}

void machineui_drawvupeak(MachineUi* self, psy_ui_Graphics* g,
	MachineWireView* wireview)
{
	if (self->machine && (self->volumemaxdisplay > 0.01f)) {
		intptr_t x;
		intptr_t y;
		SkinCoord* vupeak;
		SkinCoord* vu;

		vupeak = &self->coords->vupeak;
		vu = &self->coords->vu0;
		psy_audio_machine_position(self->machine, &x, &y);
		psy_ui_drawbitmap(g, &wireview->skin.skinbmp,
			x + vu->destx + (int)(self->volumemaxdisplay * vu->destwidth),
			y + vu->desty,
			vupeak->srcwidth, vupeak->srcheight,
			vupeak->srcx, vupeak->srcy);
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
			machineframe_init(self->frame, parent,
				machineparamconfig_showfloated(
					psycleconfig_macparam(workspace_conf(self->workspace))),
				self->workspace);			
			psy_signal_connect(&self->frame->component.signal_destroy, self,
				machineui_onframedestroyed);
			if (psy_audio_machine_haseditor(self->machine)) {
				MachineEditorView* editorview;

				editorview = machineeditorview_allocinit(
					psy_ui_notebook_base(&self->frame->notebook),
					self->machine, self->workspace);
				if (editorview) {
					machineframe_setview(self->frame, &editorview->component, self->machine);
				}
			} else {
				ParamView* paramview;

				paramview = paramview_allocinit(&self->frame->notebook.component,
					self->machine, self->workspace);
				if (paramview) {
					machineframe_setparamview(self->frame, paramview, self->machine);
				}
			}			
		}
		if (self->frame) {
			psy_ui_component_show(&self->frame->component);
			if (!machineparamconfig_showfloated(
					psycleconfig_macparam(workspace_conf(self->workspace)))) {
				workspace_dockview(self->workspace, &self->frame->component);
			}
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
static psy_ui_Rectangle machinewireview_bounds(MachineWireView*);
static void machinewireview_connectmachinessignals(MachineWireView*);
static void machinewireview_ondraw(MachineWireView*, psy_ui_Graphics*);
static void machinewireview_drawmachines(MachineWireView*, psy_ui_Graphics*);
static void drawmachineline(psy_ui_Graphics* g, int xdir, int ydir, int x, int y);
static void machinewireview_drawdragwire(MachineWireView*, psy_ui_Graphics*);
static void machinewireview_drawwires(MachineWireView*, psy_ui_Graphics*);
static void machinewireview_drawwire(MachineWireView*, psy_ui_Graphics*,
	uintptr_t slot, MachineUi*);
static void machinewireview_drawwirearrow(MachineWireView*, psy_ui_Graphics*,
	int x1, int y1, int x2, int y2);
static psy_ui_IntPoint rotate_point(psy_ui_IntPoint, double phi);
static psy_ui_IntPoint move_point(psy_ui_IntPoint pt, psy_ui_IntPoint d);
static void machinewireview_onmousedown(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onmouseup(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onmousemove(MachineWireView*, psy_ui_MouseEvent*);
static bool machinewireview_movemachine(MachineWireView*, uintptr_t slot,
	int dx, int dy);
static bool machinewireview_dragmachine(MachineWireView*, uintptr_t slot,
	int x, int y);
static void machinewireview_onmousedoubleclick(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onkeydown(MachineWireView*, psy_ui_KeyEvent*);
static uintptr_t machinewireview_machineleft(MachineWireView*, uintptr_t src);
static uintptr_t machinewireview_machineright(MachineWireView* self, uintptr_t src);
static uintptr_t machinewireview_machineup(MachineWireView*, uintptr_t src);
static uintptr_t machinewireview_machinedown(MachineWireView* self, uintptr_t src);
static void machinewireview_hittest(MachineWireView*);
static int machinewireview_hittestpan(MachineWireView*, int x, int y,
	uintptr_t slot, int* dx);
static bool machinewireview_hittestcoord(MachineWireView*, int x, int y,
	int mode, SkinCoord*);
static psy_audio_Wire machinewireview_hittestwire(MachineWireView*, int x,
	int y);
static int machinewireview_hittesteditname(MachineWireView*, int x, int y,
	uintptr_t slot);
static psy_dsp_amp_t machinewireview_panvalue(MachineWireView*, int x, int y,
	uintptr_t slot);
static bool machinewireview_dragging_machine(const MachineWireView*);
static bool machinewireview_dragging_connection(const MachineWireView*);
static bool machinewireview_dragging_newconnection(const MachineWireView*);
static void machinewireview_onnewmachineselected(MachineView*,
	psy_ui_Component* sender, psy_Property*);
static void machinewireview_onmachineschangeslot(MachineWireView*,
	psy_audio_Machines*, uintptr_t slot);
static void machinewireview_onmachinesinsert(MachineWireView*,
	psy_audio_Machines*, uintptr_t slot);
static void machinewireview_onmachinesremoved(MachineWireView*,
	psy_audio_Machines*, uintptr_t slot);
static void machinewireview_onconnected(MachineWireView*, psy_audio_Connections*,
	uintptr_t outputslot, uintptr_t inputslot);
static void machinewireview_ondisconnected(MachineWireView*, psy_audio_Connections*,
	uintptr_t outputslot, uintptr_t inputslot);
static void machinewireview_onsongchanged(MachineWireView*, Workspace*,
	int flag, psy_audio_SongFile*);
static void machinewireview_buildmachineuis(MachineWireView*);
static void machinewireview_updateskin(MachineWireView*);
static void machinewireview_onshowparameters(MachineWireView*, Workspace*,
	uintptr_t slot);
static void machinewireview_onmasterworked(MachineWireView*,
	psy_audio_Machine*, uintptr_t slot, psy_audio_BufferContext*);
static void machinewireview_ontimer(MachineWireView*, uintptr_t timerid);
static void machinewireview_showwireview(MachineWireView*, psy_audio_Wire);
static void machinewireview_onwireframedestroyed(MachineWireView*,
	psy_ui_Component* sender);
static WireFrame* machinewireview_wireframe(MachineWireView*, psy_audio_Wire);
static psy_ui_Rectangle machinewireview_updaterect(MachineWireView*,
	uintptr_t slot);
static void machinewireview_onpreferredsize(MachineWireView* self, const psy_ui_Size* limit,
	psy_ui_Size* rv);
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
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)machinewireview_ondestroy;
		vtable.ondraw = (psy_ui_fp_component_ondraw)machinewireview_ondraw;
		vtable.ontimer = (psy_ui_fp_component_ontimer)machinewireview_ontimer;
		vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			machinewireview_onmousedown;
		vtable.onmouseup = (psy_ui_fp_component_onmouseup)machinewireview_onmouseup;
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
	psy_ui_Component* tabbarparent, psy_ui_Scroller* scroller, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component, vtable_init(self));	
	self->component.vtable = &vtable;
	self->component.scrollstepy = 10;
	self->statusbar = 0;
	self->workspace = workspace;
	self->machines = &workspace->song->machines;
	self->drawvumeters = 1;
	self->wireframes = 0;
	self->randominsert = 1;
	self->addeffect = 0;
	self->drawvumode = FALSE;
	self->showwirehover = FALSE;
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
	self->dragslot = UINTPTR_MAX;
	self->dragmode = MACHINEWIREVIEW_DRAG_MACHINE;
	self->selectedslot = psy_audio_MASTER_INDEX;	
	psy_audio_wire_init(&self->selectedwire);
	psy_audio_wire_init(&self->hoverwire);
	psy_signal_connect(&workspace->signal_songchanged, self,
		machinewireview_onsongchanged);	
	machinewireview_connectmachinessignals(self);
	psy_signal_connect(&workspace->signal_showparameters, self,
		machinewireview_onshowparameters);	
	psy_ui_edit_init(&self->editname, &self->component);
	psy_ui_component_hide(&self->editname.component);
	psy_ui_component_starttimer(&self->component, 0, 50);	
}

void machinewireview_connectmachinessignals(MachineWireView* self)
{	
	psy_signal_connect(&self->machines->signal_slotchange, self,
		machinewireview_onmachineschangeslot);
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

void machinewireview_ondestroy(MachineWireView* self)
{	
	machineuis_removeall(self);
	psy_table_dispose(&self->machineuis);
	psy_list_deallocate(&self->wireframes, (psy_fp_disposefunc)
		psy_ui_component_destroy);		
	machineviewskin_dispose(&self->skin);
}

void machinewireview_configure(MachineWireView* self, MachineViewConfig* macview)
{	
	self->drawvumeters = machineviewconfig_vumeters(macview);
	self->skin.drawmachineindexes = machineviewconfig_machineindexes(macview);
	self->showwirehover = machineviewconfig_wirehover(macview);	
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
	machinewireview_drawwires(self, g);
	machinewireview_drawmachines(self, g);
	machinewireview_drawdragwire(self, g);
}

void machinewireview_drawwires(MachineWireView* self, psy_ui_Graphics* g)
{
	psy_TableIterator it;
	
	for (it = psy_table_begin(&self->machineuis); 
			!psy_tableiterator_equal(&it, psy_table_end()); 
			psy_tableiterator_inc(&it)) {
		machinewireview_drawwire(self, g, psy_tableiterator_key(&it),
			(MachineUi*)psy_tableiterator_value(&it));
	}	
}

void machinewireview_drawwire(MachineWireView* self, psy_ui_Graphics* g,
	uintptr_t slot, MachineUi* outmachineui)
{		
	psy_audio_MachineSockets* sockets;	
	
	sockets	= psy_audio_connections_at(&self->machines->connections, slot);
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
				if (inmachineui && outmachineui) {
					psy_ui_Rectangle out;
					psy_ui_Rectangle in;					

					out = machineui_position(inmachineui);
					in = machineui_position(outmachineui);
					if (self->hoverwire.src == slot &&
						self->hoverwire.dst == socket->slot) {
						psy_ui_setcolour(g, self->skin.hoverwirecolour);
					} else
					if (self->selectedwire.src == slot &&
							self->selectedwire.dst == socket->slot) {
						psy_ui_setcolour(g, self->skin.selwirecolour);
					} else {
						psy_ui_setcolour(g, self->skin.wirecolour);
					}
					out.left = out.left + (out.right - out.left) / 2;
					out.top = out.top + (out.bottom - out.top) / 2,
					in.left = in.left + (in.right - in.left) / 2,
					in.top = in.top + (in.bottom - in.top) / 2;
					psy_ui_drawline(g, out.left, out.top, in.left, in.top);
					machinewireview_drawwirearrow(self, g, in.left, in.top,
						out.left, out.top);
				}
			}		
		}
	}
}

void machinewireview_drawwirearrow(MachineWireView* self, psy_ui_Graphics* g,
	int x1, int y1, int x2, int y2)
{			
	psy_ui_IntPoint center;
	psy_ui_IntPoint a, b, c;	
	psy_ui_IntPoint tri[4];
	int polysize;
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

psy_ui_IntPoint rotate_point(psy_ui_IntPoint pt, double phi)
{
	psy_ui_IntPoint rv;
	
	rv.x = (int) (cos(phi) * pt.x - sin(phi) * pt.y);
	rv.y = (int) (sin(phi) * pt.x + cos(phi) * pt.y);
	return rv;
}

psy_ui_IntPoint move_point(psy_ui_IntPoint pt, psy_ui_IntPoint d)
{
	psy_ui_IntPoint rv;
	
	rv.x = pt.x + d.x;
	rv.y = pt.y + d.y;
	return rv;
}

void machinewireview_drawdragwire(MachineWireView* self, psy_ui_Graphics* g)
{
	if (self->dragslot != UINTPTR_MAX && (
			self->dragmode == MACHINEWIREVIEW_DRAG_NEWCONNECTION ||
			self->dragmode == MACHINEWIREVIEW_DRAG_LEFTCONNECTION ||
			self->dragmode == MACHINEWIREVIEW_DRAG_RIGHTCONNECTION)) {
		MachineUi* machineui;

		machineui = machineuis_at(self, self->dragslot);
		if (machineui) {
			psy_ui_IntSize machinesize;			
			intptr_t x;
			intptr_t y;

			psy_audio_machine_position(machineui->machine, &x, &y);			
			machinesize = machineui_size(machineui);
			psy_ui_setcolour(g, self->skin.wirecolour);			
			psy_ui_drawline(g, 
				x + machinesize.width / 2, y + machinesize.height / 2,
				self->mx, self->my);
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
		machineui_draw(machineui, g, self, psy_tableiterator_key(&it));
		if (self->selectedwire.src == UINTPTR_MAX &&
				self->selectedslot == psy_tableiterator_key(&it)) {
			machineui_drawhighlight(machineui, g, self);	
		}
	}
}

void machineui_drawhighlight(MachineUi* self, psy_ui_Graphics* g,
	MachineWireView* wireview)
{	
	psy_ui_Rectangle r;
	static int d = 5; // the distance of the highlight from the machine

	r = machineui_position(self);	
	psy_ui_setcolour(g, wireview->skin.wirecolour);
	drawmachineline(g, 1, 0, r.left - d, r.top - d);
	drawmachineline(g, 0, 1, r.left - d, r.top - d);
	drawmachineline(g, -1, 0, r.right + d, r.top - d);
	drawmachineline(g, 0, 1, r.right + d, r.top - d);
	drawmachineline(g, 0, -1, r.right + d, r.bottom + d);
	drawmachineline(g, -1, 0, r.right + d, r.bottom + d);
	drawmachineline(g, 1, 0, r.left - d, r.bottom + d);
	drawmachineline(g, 0, -1, r.left - d, r.bottom + d);	
}

void drawmachineline(psy_ui_Graphics* g, int xdir, int ydir, int x, int y)
{
	int hlength = 9; // the length of the selected machine highlight	

	psy_ui_drawline(g, x, y, x + xdir * hlength, y + ydir * hlength);
}

void machinewireview_centermaster(MachineWireView* self)
{
	MachineUi* machineui;	

	machineui = machineuis_at(self, psy_audio_MASTER_INDEX);
	if (machineui) {
		psy_ui_TextMetric tm;
		psy_ui_IntSize machinesize;
		psy_ui_IntSize size;
				
		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_intsize_init_size(
			psy_ui_component_size(&self->component), &tm);
		machinesize = machineui_size(machineui);
		psy_audio_machine_setposition(machineui->machine,
			(size.width - machinesize.width) / 2,
			(size.height - machinesize.height) / 2);
	}
}

void machinewireview_onmousedoubleclick(MachineWireView* self, psy_ui_MouseEvent* ev)
{	
	self->mx = ev->x;
	self->my = ev->y;
	machinewireview_hittest(self);
	if (self->dragslot == UINTPTR_MAX) {
		self->selectedwire = machinewireview_hittestwire(self, ev->x, ev->y);
		if (self->selectedwire.dst != UINTPTR_MAX) {			
			machinewireview_showwireview(self, self->selectedwire);
			psy_ui_component_invalidate(&self->component);
		} else {
			self->dragslot = UINTPTR_MAX;
			self->randominsert = 0;
			return;
		}
	} else		 
	if (machinewireview_hittesteditname(self, ev->x, ev->y, self->dragslot)) {
		if (machineuis_at(self, self->dragslot)) {
			machineui_editname(machineuis_at(self, self->dragslot),
				&self->editname, psy_ui_component_scroll(&self->component));
				psy_ui_component_scrolltop(&self->component);
		}
	} else
	if (machinewireview_hittestcoord(self, ev->x, ev->y, MACHMODE_GENERATOR,
			&self->skin.generator.solo) ||				
	    machinewireview_hittestcoord(self, ev->x, ev->y, MACHMODE_FX,
			&self->skin.effect.bypass) ||	
	    machinewireview_hittestcoord(self, ev->x, ev->y, MACHMODE_GENERATOR,
			&self->skin.generator.mute) ||
		machinewireview_hittestcoord(self, ev->x, ev->y, MACHMODE_FX,
			&self->skin.effect.mute) ||
	    machinewireview_hittestpan(self, ev->x, ev->y,
			self->dragslot, &self->mx)) {
	} else {
		workspace_showparameters(self->workspace, self->dragslot);
	}		
	self->dragslot = UINTPTR_MAX;
	psy_ui_mouseevent_stoppropagation(ev);
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
	if (self->dragslot == UINTPTR_MAX) {
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
				self->selectedwire.src = UINTPTR_MAX;
				self->selectedwire.dst = UINTPTR_MAX;
				psy_audio_machines_changeslot(self->machines, self->selectedslot);
			}			
			if (machinewireview_hittestcoord(self, ev->x, ev->y,
					MACHMODE_GENERATOR, &self->skin.generator.solo)) {
				psy_audio_Machine* machine = psy_audio_machines_at(self->machines,
					self->dragslot);
				if (machine) {
					psy_audio_machines_solo(self->machines, self->dragslot);
					psy_ui_component_invalidate(&self->component);
				}
				self->dragslot = UINTPTR_MAX;
			} else
			if (machinewireview_hittestcoord(self, ev->x, ev->y,
					MACHMODE_FX, &self->skin.effect.bypass)) {
				psy_audio_Machine* machine = psy_audio_machines_at(self->machines,
					self->dragslot);
				if (machine) {
					if (psy_audio_machine_bypassed(machine)) {
						psy_audio_machine_unbypass(machine);
					} else {
						psy_audio_machine_bypass(machine);
					}
					self->dragslot = UINTPTR_MAX;
				}
			} else
			if (machinewireview_hittestcoord(self, ev->x, ev->y,
					MACHMODE_GENERATOR, &self->skin.generator.mute) ||
				machinewireview_hittestcoord(self, ev->x, ev->y,
					MACHMODE_FX, &self->skin.effect.mute)) {
				psy_audio_Machine* machine = psy_audio_machines_at(self->machines,
					self->dragslot);
				if (machine) {
					if (psy_audio_machine_muted(machine)) {
						psy_audio_machine_unmute(machine);
					} else {
						psy_audio_machine_mute(machine);
					}
				}
				self->dragslot = UINTPTR_MAX;
			} else
			if (machinewireview_hittestpan(self, ev->x, ev->y,
					self->dragslot, &self->mx)) {
				self->dragmode = MACHINEWIREVIEW_DRAG_PAN;				
			} else  {
				MachineUi* machineui;

				machineui = machineuis_at(self, self->dragslot);
				if (machineui) {
					intptr_t x;
					intptr_t y;

					self->dragmode = MACHINEWIREVIEW_DRAG_MACHINE;
					psy_audio_machine_position(machineui->machine, &x, &y);
					self->mx = ev->x - x;
					self->my = ev->y - y;
					psy_ui_component_capture(&self->component);
				}
			}
			
		} else
		if (ev->button == 2) {
			psy_audio_Machine* machine;

			machine = psy_audio_machines_at(self->machines, self->dragslot);
			if (machine && psy_audio_machine_numoutputs(machine) > 0) {
				self->dragmode = MACHINEWIREVIEW_DRAG_NEWCONNECTION;
				psy_ui_component_capture(&self->component);
			} else {
				self->dragslot = UINTPTR_MAX;
			}			
		}
	}
}

int machinewireview_hittestpan(MachineWireView* self, int x, int y,
	uintptr_t slot, int* dx)
{	
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui && machineui->coords && machineui->machine) {
		intptr_t mx;
		intptr_t my;		
		psy_ui_Rectangle r;
		int offset;

		psy_audio_machine_position(machineui->machine, &mx, &my);
		offset = (int) (psy_audio_machine_panning(machineui->machine) *
			machineui->coords->pan.range);
		r = skincoord_destposition(&machineui->coords->pan);
		psy_ui_rectangle_move(&r, offset, 0);		
		*dx = x - mx - r.left;
		return psy_ui_rectangle_intersect(&r, x - mx, y - my);		
	}
	return FALSE;
}

int machinewireview_hittesteditname(MachineWireView* self, int x, int y,
	uintptr_t slot)
{	
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui && machineui->coords) {
		intptr_t mx;
		intptr_t my;
		psy_ui_Rectangle r;

		psy_audio_machine_position(machineui->machine, &mx, &my);		
		r = skincoord_destposition(&machineui->coords->name);
		return psy_ui_rectangle_intersect(&r, x - mx, y - my);		
	}
	return FALSE;
}

bool machinewireview_hittestcoord(MachineWireView* self, int x, int y, int mode,
	SkinCoord* coord)
{
	MachineUi* machineui;

	machineui = machineuis_at(self, self->dragslot);
	if (machineui && machineui->mode == mode) {
		psy_ui_Rectangle r;
		intptr_t mx;
		intptr_t my;		

		psy_audio_machine_position(machineui->machine, &mx, &my);
		r = skincoord_destposition(coord);
		return psy_ui_rectangle_intersect(&r, x - mx, y - my);
	}
	return FALSE;
}

psy_dsp_amp_t machinewireview_panvalue(MachineWireView* self, int x, int y,
	uintptr_t slot)
{
	psy_dsp_amp_t rv = 0.f;	
	MachineUi* machineui;
	
	machineui = machineuis_at(self, slot);
	if (machineui) {
		MachineCoords* coords;
		intptr_t mx;
		intptr_t my;

		psy_audio_machine_position(machineui->machine, &mx, &my);
		coords = machineui->coords;
		if (coords && coords->pan.range != 0) {
			rv =  (x - mx - coords->pan.destx - self->mx) /
				(float)coords->pan.range;
		}		
	}
	return rv;
}

void machinewireview_hittest(MachineWireView* self)
{	
	psy_TableIterator it;
		
	for (it = psy_table_begin(&self->machineuis); it.curr != 0; 
			psy_tableiterator_inc(&it)) {
		psy_ui_Rectangle position;	

		position = machineui_position((MachineUi*)
			psy_tableiterator_value(&it));
		if (psy_ui_rectangle_intersect(&position, self->mx, self->my)) {
			self->dragslot = it.curr->key;
			break;	
		}
	}	
}

void machinewireview_onmousemove(MachineWireView* self, psy_ui_MouseEvent* ev)
{		
	self->mousemoved = TRUE;
	if (self->dragslot != UINTPTR_MAX) {
		if (self->dragmode == MACHINEWIREVIEW_DRAG_PAN) {
			MachineUi* machineui;
			
			machineui = machineuis_at(self, self->dragslot);
			if (machineui) {
				psy_audio_machine_setpanning(machineui->machine,
					machinewireview_panvalue(self, ev->x, ev->y,
						self->dragslot));
				machineui_invalidate(machineui, self, FALSE);
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
	} else 
	if (self->showwirehover) {
		psy_audio_Wire hoverwire;
		
		hoverwire = machinewireview_hittestwire(self, ev->x, ev->y);
		if (psy_audio_wire_valid(&hoverwire)) {
			MachineUi* machineui;

			machineui = machineuis_at(self, hoverwire.dst);
			if (machineui) {
				psy_ui_Rectangle position;

				position = machineui_position(machineui);
				if (psy_ui_rectangle_intersect(&position, ev->x, ev->y)) {
					psy_audio_wire_init(&self->hoverwire);
					psy_ui_component_invalidate(&self->component);
					return;
				}
			}
			machineui = machineuis_at(self, hoverwire.src);
			if (machineui) {
				psy_ui_Rectangle position;

				position = machineui_position(machineui);
				if (psy_ui_rectangle_intersect(&position, ev->x, ev->y)) {
					psy_audio_wire_init(&self->hoverwire);
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
	int dx, int dy)
{
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		return machinewireview_dragmachine(self, slot,
			machineui_position(machineui).left + dx,
			machineui_position(machineui).top + dy);
	}
	return FALSE;
}

bool machinewireview_dragmachine(MachineWireView* self, uintptr_t slot,
	int x, int y)
{
	MachineUi* machineui;
	psy_ui_Rectangle r_old;
	psy_ui_Rectangle r_new;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		r_old = machineui_position(machineui);
		psy_ui_rectangle_expand(&r_old, 10, 10, 10, 10);
		psy_audio_machine_setposition(machineui->machine,
			psy_max(0, x), psy_max(0, y));
		if (self->statusbar && machineui->machine) {
			static char txt[128];
			intptr_t mx;
			intptr_t my;

			psy_audio_machine_position(machineui->machine, &mx, &my);
			psy_snprintf(txt, 128, "%s (%d, %d)",
				(psy_audio_machine_editname(machineui->machine))
				? psy_audio_machine_editname(machineui->machine)
				: "",
				mx, my);
			machineviewbar_settext(self->statusbar, txt);
		}
		r_new = machinewireview_updaterect(self, self->dragslot);
		psy_ui_rectangle_union(&r_new, &r_old);
		psy_ui_rectangle_expand(&r_new, 10, 10, 10, 10);
		psy_ui_component_invalidaterect(&self->component, r_new);
		return TRUE;
	}
	return FALSE;
}

void machinewireview_onmouseup(MachineWireView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_releasecapture(&self->component);
	if (self->dragslot != UINTPTR_MAX) {
		if (machinewireview_dragging_machine(self)) {
			psy_ui_component_updateoverflow(&self->component);
		} else if (machinewireview_dragging_connection(self)) {
			if (self->mousemoved) {
				uintptr_t slot = self->dragslot;
				self->dragslot = UINTPTR_MAX;
				machinewireview_hittest(self);
				if (self->dragslot != UINTPTR_MAX) {
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
					} else if (psy_audio_machines_valid_connection(self->machines,
							psy_audio_wire_make(self->dragslot, slot))) {
						psy_audio_machines_connect(self->machines,
							psy_audio_wire_make(self->dragslot, slot));						
					}
				}
			} else if (ev->button == 2) {
				workspace_togglegear(self->workspace);
			}			
		}
	}
	self->dragslot = UINTPTR_MAX;
	psy_ui_component_invalidate(&self->component);
}

void machinewireview_onkeydown(MachineWireView* self, psy_ui_KeyEvent* ev)
{		
	if (ev->ctrl) {
		if (ev->keycode == psy_ui_KEY_B) {
			self->dragwire.src = self->selectedslot;
		} else if (ev->keycode == psy_ui_KEY_E) {
			if (self->dragwire.src != UINTPTR_MAX &&
					self->selectedslot != UINTPTR_MAX) {
				self->dragwire.dst = self->selectedslot;
				if (!psy_audio_machines_connected(self->machines, self->dragwire)) {					
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
			if (index != UINTPTR_MAX) {
				psy_audio_machines_changeslot(self->machines, index);
			}
		}
	} else if (ev->keycode == psy_ui_KEY_DOWN) {
		if (ev->shift) {
			machinewireview_movemachine(self, self->selectedslot, 0, 10);
		} else {
			uintptr_t index;

			index = machinewireview_machinedown(self, self->selectedslot);
			if (index != UINTPTR_MAX) {
				psy_audio_machines_changeslot(self->machines, index);
			}
		}
	} else if (ev->keycode == psy_ui_KEY_LEFT) {
		if (ev->shift) {
			machinewireview_movemachine(self, self->selectedslot, -10, 0);
		} else {
			uintptr_t index;

			index = machinewireview_machineleft(self, self->selectedslot);
			if (index != UINTPTR_MAX) {
				psy_audio_machines_changeslot(self->machines, index);
			}
		}
	} else if (ev->keycode == psy_ui_KEY_RIGHT) {
		if (ev->shift) {
			machinewireview_movemachine(self, self->selectedslot, 10, 0);
		} else {
			uintptr_t index;

			index = machinewireview_machineright(self, self->selectedslot);
			if (index != UINTPTR_MAX) {
				psy_audio_machines_changeslot(self->machines, index);
			}
		}
	} else if (ev->keycode == psy_ui_KEY_DELETE &&
			psy_audio_wire_valid(&self->selectedwire)) {
		psy_audio_machines_disconnect(self->machines, self->selectedwire);
	} else if (ev->keycode == psy_ui_KEY_DELETE && self->selectedslot != - 1 &&
			self->selectedslot != psy_audio_MASTER_INDEX) {
		psy_audio_machines_remove(self->machines, self->selectedslot);		
		self->selectedslot = UINTPTR_MAX;
	} else if (ev->repeat) {
		psy_ui_keyevent_stoppropagation(ev);
	}
}

uintptr_t machinewireview_machineleft(MachineWireView* self, uintptr_t src)
{
	uintptr_t rv;
	MachineUi* srcmachineui;
	intptr_t currpos;	

	rv = UINTPTR_MAX;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		intptr_t srcpos;

		srcpos = machineui_position(srcmachineui).left;
		currpos = INTPTR_MAX;
		for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			MachineUi* machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			if (machineui_position(machineui).left < srcpos) {
				if (currpos == INTPTR_MAX ||
						currpos < machineui_position(machineui).left) {
					rv = psy_tableiterator_key(&it);
					currpos = machineui_position(machineui).left;
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
	intptr_t currpos;

	rv = UINTPTR_MAX;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		intptr_t srcpos;

		srcpos = machineui_position(srcmachineui).left;
		currpos = INTPTR_MIN;
		for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			MachineUi* machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			if (machineui_position(machineui).left > srcpos) {
				if (currpos == INTPTR_MIN ||
						currpos > machineui_position(machineui).left) {
					rv = psy_tableiterator_key(&it);
					currpos = machineui_position(machineui).left;
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
	intptr_t currpos;

	rv = UINTPTR_MAX;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		intptr_t srcpos;

		srcpos = machineui_position(srcmachineui).top;
		currpos = INTPTR_MAX;
		for (it = psy_table_begin(&self->machineuis);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			MachineUi* machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			if (machineui_position(machineui).top < srcpos) {
				if (currpos == INTPTR_MAX ||
					currpos < machineui_position(machineui).top) {
					rv = psy_tableiterator_key(&it);
					currpos = machineui_position(machineui).top;
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
	intptr_t currpos;

	rv = UINTPTR_MAX;
	srcmachineui = machineuis_at(self, src);
	if (srcmachineui) {
		psy_TableIterator it;
		intptr_t srcpos;

		srcpos = machineui_position(srcmachineui).bottom;
		currpos = INTPTR_MIN;
		for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			MachineUi* machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			if (machineui_position(machineui).bottom > srcpos) {
				if (currpos == INTPTR_MIN ||
					currpos > machineui_position(machineui).bottom) {
					rv = psy_tableiterator_key(&it);
					currpos = machineui_position(machineui).bottom;
				}
			}
		}
	}
	return rv;
}

psy_audio_Wire machinewireview_hittestwire(MachineWireView* self, int x, int y)
{		
	psy_audio_Wire rv;
	psy_TableIterator it;
	psy_ui_TextMetric tm;

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
				if (socket->slot != UINTPTR_MAX) {
					MachineUi* inmachineui;
					MachineUi* outmachineui;

					inmachineui = machineuis_at(self, socket->slot);
					outmachineui = machineuis_at(self, slot);
					if (inmachineui && outmachineui) {
						psy_ui_Rectangle r;
						psy_ui_IntSize out;
						psy_ui_IntSize in;
						int d = 4;
						intptr_t mxout;
						intptr_t myout;
						intptr_t mxin;
						intptr_t myin;

						psy_audio_machine_position(outmachineui->machine, &mxout, &myout);
						psy_audio_machine_position(inmachineui->machine, &mxin, &myin);

						out = machineui_size(outmachineui);
						in = machineui_size(inmachineui);
						psy_ui_setrectangle(&r, x - d, y - d, 2 * d, 2 * d);
						if (psy_ui_rectangle_intersect_segment(&r,
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

void machinewireview_onmachineschangeslot(MachineWireView* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	self->selectedslot = slot;
	psy_audio_wire_invalidate(&self->selectedwire);	
	psy_ui_component_invalidate(&self->component);
	psy_ui_component_setfocus(&self->component);
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
			int width;
			int height;

			width = machineui_position(machineui).right -
				machineui_position(machineui).left;
			height = machineui_position(machineui).bottom -
				machineui_position(machineui).top;
			psy_audio_machine_setposition(machine,
				psy_max(0, self->mx - width / 2),
				psy_max(0, self->my - height / 2));
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
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
			machineuis_insert(self, psy_tableiterator_key(&it));			
		}
		if (psy_audio_machines_size(self->machines) == 1) {
			// if only master exists, center
			machinewireview_centermaster(self);
		}
	}
}

void machinewireview_onsongchanged(MachineWireView* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
{	
	self->machines = &workspace->song->machines;	
	machinewireview_buildmachineuis(self);	
	machinewireview_connectmachinessignals(self);
	psy_ui_component_setscroll(&self->component, psy_ui_intpoint_make(0, 0));	
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
	psy_List* p;
	psy_List* q;
	psy_TableIterator it;
	bool updatevus;

	self->drawvumode = TRUE;
	updatevus = psy_ui_component_visible(&self->component) && self->drawvumeters;
	if (updatevus) {
		psy_ui_component_setbackgroundmode(&self->component, psy_ui_BACKGROUND_NONE);
	}
	for (p = self->wireframes; p != NULL; p = q) {
		WireFrame* frame;

		frame = (WireFrame*)p->entry;
		q = p->next;
		if (frame->wireview && !wireview_wireexists(frame->wireview)) {
			psy_ui_component_destroy(&frame->wireview->component);
			free(frame->wireview);
			psy_ui_component_destroy(&frame->component);
			free(frame);
			psy_list_remove(&self->wireframes, p);
		}
	}
	for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		MachineUi* machineui;

		machineui = psy_tableiterator_value(&it);
		if (machineui->frame) {
			if (machineui->frame->dofloat) {
				MachineFrame* frame;
				psy_ui_Component temp;
				psy_ui_Component* view;
				ParamView* paramview;
				psy_audio_Machine* machine;
				psy_ui_Component* dockparent;

				frame = machineui->frame;
				psy_ui_component_init(&temp, &self->component);
				view = frame->view;
				paramview = frame->paramview;
				dockparent = psy_ui_component_parent(&frame->component);
				machine = frame->machine;
				psy_ui_component_setparent(frame->view, &temp);
				psy_signal_disconnectall(&view->signal_preferredsizechanged);
				frame->view = 0;
				psy_ui_component_destroy(&frame->component);
				frame = machineframe_alloc();
				machineframe_init(frame, &self->component, TRUE, self->workspace);
				psy_ui_component_insert(&frame->notebook.component, view, &frame->help.component);
				if (paramview) {
					machineframe_setparamview(frame, paramview, machine);
				} else {
					machineframe_setview(frame, view, machine);
				}
				psy_ui_component_show(&frame->component);
				psy_ui_component_destroy(&temp);
				machineui->frame = frame;
				psy_signal_connect(&frame->component.signal_destroy, machineui,
					machineui_onframedestroyed);
				psy_ui_component_align(psy_ui_component_parent(dockparent));
				psy_ui_component_align(dockparent);
			} else if (machineui->frame->dodock) {
				MachineFrame* frame;
				psy_ui_Component temp;
				psy_ui_Component* view;
				ParamView* paramview;
				psy_audio_Machine* machine;

				frame = machineui->frame;
				psy_ui_component_init(&temp, &self->component);
				view = frame->view;
				paramview = frame->paramview;
				machine = frame->machine;
				psy_ui_component_setparent(frame->view, &temp);
				frame->view = 0;
				psy_signal_disconnectall(&view->signal_preferredsizechanged);
				psy_ui_component_destroy(&frame->component);
				frame = machineframe_alloc();
				machineframe_init(frame, &self->component, FALSE, self->workspace);
				psy_ui_component_insert(&frame->notebook.component, view, &frame->help.component);
				if (paramview) {
					machineframe_setparamview(frame, paramview, machine);
				} else {
					machineframe_setview(frame, view, machine);
				}
				psy_ui_component_show(&frame->component);
				psy_ui_component_destroy(&temp);
				machineui->frame = frame;
				workspace_dockview(self->workspace, &machineui->frame->component);
				psy_signal_connect(&frame->component.signal_destroy, machineui,
					machineui_onframedestroyed);
			} else if (machineui->frame->doclose) {
				psy_ui_Component* dockparent;

				dockparent = psy_ui_component_parent(&machineui->frame->component);
				psy_ui_component_destroy(&machineui->frame->component);
				psy_ui_component_align(psy_ui_component_parent(dockparent));
				psy_ui_component_align(dockparent);
			}
		}
		if (updatevus) {
			machineui_invalidate(machineui, self, TRUE);
		}
	}	
	if (updatevus) {
		psy_ui_component_update(&self->component);
		self->drawvumode = FALSE;
		psy_ui_component_setbackgroundmode(&self->component, psy_ui_BACKGROUND_SET);
	}
}

void machineui_invalidate(MachineUi* self, MachineWireView* wireview, bool vu)
{
	if (vu) {
		psy_ui_component_invalidaterect(&wireview->component,
			machineui_vuposition(self));
	} else {
		psy_ui_component_invalidaterect(&wireview->component,
			machineui_position(self));
	}
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

		frame = (WireFrame*) p->entry;
		q = p->next;
		if (&frame->component == sender) {
			psy_list_remove(&self->wireframes, p);
		}
	}
}

WireFrame* machinewireview_wireframe(MachineWireView* self,
	psy_audio_Wire wire)
{
	WireFrame* rv = 0;
	psy_List* framenode;

	framenode = self->wireframes;
	while (framenode != 0) {
		WireFrame* frame;

		frame = (WireFrame*)framenode->entry;
		if (frame->wireview && frame->wireview->wire.dst == wire.dst && 
				frame->wireview->wire.src == wire.src) {
			rv = frame;
			break;
		}
		framenode = framenode->next;
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
		int favorite;

		favorite = psy_property_at_int(plugininfo, "favorite", 0);
		psy_property_set_int(plugininfo, "favorite", ++favorite);
		if (self->wireview.addeffect) {
			uintptr_t slot;

			slot = psy_audio_machines_append(self->wireview.machines, machine);
			psy_audio_machines_disconnect(self->wireview.machines, self->wireview.selectedwire);
			psy_audio_machines_connect(self->wireview.machines,
				psy_audio_wire_make(self->wireview.selectedwire.src, slot));
			psy_audio_machines_connect(self->wireview.machines,
				psy_audio_wire_make(slot, self->wireview.selectedwire.dst));
			psy_audio_machines_changeslot(self->wireview.machines, slot);
			self->wireview.addeffect = 0;
		} else
			if (self->newmachine.pluginsview.calledby == 10) {
				psy_audio_machines_insert(self->wireview.machines,
					psy_audio_machines_slot(self->wireview.machines), machine);
			} else {
				psy_audio_machines_changeslot(self->wireview.machines,
					psy_audio_machines_append(self->wireview.machines, machine));
			}
		tabbar_select(&self->tabbar, 0);
	} else {
		workspace_outputerror(self->workspace, self->workspace->machinefactory.errstr);
	}
}

MachineUi* machineuis_insert(MachineWireView* self, uintptr_t slot)
{	
	MachineUi* rv = 0;
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(self->machines, slot);
	if (machine) {
		if (psy_table_exists(&self->machineuis, slot)) {
			machineuis_remove(self, slot);
		}	
		rv = (MachineUi*) malloc(sizeof(MachineUi));
		machineui_init(rv, machine, slot, &self->skin,
			self->workspace);		
		psy_table_insert(&self->machineuis, slot, rv);
	}
	return rv;
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

void machineviewbar_init(MachineViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;		
	psy_ui_checkbox_init(&self->mixersend, &self->component);	
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(4), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->mixersend.component, &margin);
	psy_ui_checkbox_check(&self->mixersend);
	psy_ui_checkbox_settext(&self->mixersend,
		"machineview.connect-to-mixer-send-return-input");
	psy_signal_connect(&self->mixersend.signal_clicked, self,
		machineviewbar_onmixerconnectmodeclick);	
	psy_ui_component_setalign(&self->mixersend.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->status, &self->component);
	psy_ui_label_preventtranslation(&self->status);
	psy_ui_label_setcharnumber(&self->status, 44);		
	psy_ui_component_setalign(psy_ui_label_base(&self->status),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_doublebuffer(psy_ui_label_base(&self->status));
	psy_signal_connect(&workspace->signal_songchanged, self,
		machineviewbar_onsongchanged);	
}

void machineviewbar_settext(MachineViewBar* self, const char* text)
{	
	psy_ui_label_settext(&self->status, text);
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
			psy_audio_machines_isconnectasmixersend(&workspace_song(self->workspace)->machines)) {
		psy_ui_checkbox_check(&self->mixersend);
	} else {
		psy_ui_checkbox_disablecheck(&self->mixersend);		
	}
}

// MachineView
static void machineview_updatetext(MachineView*, psy_Translator*);
static void machineview_onsongchanged(MachineView*, Workspace*, int flag, psy_audio_SongFile*);
static void machineview_onmousedown(MachineView*, psy_ui_MouseEvent*);
static void machineview_onmouseup(MachineView*, psy_ui_MouseEvent*);
static void machineview_onmousedoubleclick(MachineView*, psy_ui_MouseEvent*);
static void machineview_onkeydown(MachineView*, psy_ui_KeyEvent*);
static void machineview_onfocus(MachineView*, psy_ui_Component* sender);
static void machineview_onskinchanged(MachineView*, Workspace*);
static void machineview_onconfigure(MachineView*, Workspace*, psy_Property*);
static void machineview_selectsection(MachineView*, psy_ui_Component* sender, uintptr_t section);

static psy_ui_ComponentVtable machineview_vtable;
static bool machineview_vtable_initialized = FALSE;

static void machineview_vtable_init(MachineView* self)
{
	if (!machineview_vtable_initialized) {
		machineview_vtable = *(self->component.vtable);
		machineview_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			machineview_onmousedown;
		machineview_vtable.onmouseup = (psy_ui_fp_component_onmouseup)
			machineview_onmouseup;
		machineview_vtable.onmousedoubleclick = (psy_ui_fp_component_onmousedoubleclick)
			machineview_onmousedoubleclick;
		machineview_vtable.onkeydown = (psy_ui_fp_component_onkeydown)
			machineview_onkeydown;
		machineview_vtable_initialized = TRUE;
	}
}

void machineview_init(MachineView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	psy_ui_Margin leftmargin;
	
	psy_ui_component_init(&self->component, parent);
	machineview_vtable_init(self);
	self->component.vtable = &machineview_vtable;
	self->workspace = workspace;	
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		machineview_onsongchanged);
	psy_ui_notebook_init(&self->notebook, &self->component);	
	psy_ui_margin_init_all(&leftmargin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makeew(3));
	psy_ui_component_setmargin(&self->notebook.component, &leftmargin);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook), psy_ui_ALIGN_CLIENT);	
	//machinewireview_init(&self->wireview,psy_ui_notebook_base(&self->notebook),
		//tabbarparent, workspace);
	machinewireview_init(&self->wireview, psy_ui_notebook_base(&self->notebook),
		tabbarparent, NULL, workspace);
	psy_ui_scroller_init(&self->scroller, &self->wireview.component,
		psy_ui_notebook_base(&self->notebook));
	self->wireview.scroller = &self->scroller;
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	newmachine_init(&self->newmachine, psy_ui_notebook_base(&self->notebook),
		&self->wireview.skin, self->workspace);		
	psy_ui_component_setalign(&self->newmachine.component, psy_ui_ALIGN_CLIENT);
	tabbar_init(&self->tabbar, tabbarparent);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	tabbar_append_tabs(&self->tabbar, "machineview.wires",
		"machineview.new-machine", NULL);
	psy_signal_connect(&self->component.signal_selectsection, self,
		machineview_selectsection);
	psy_ui_notebook_select(&self->notebook, 0);	
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	psy_signal_connect(&self->newmachine.signal_selected, self,
		machinewireview_onnewmachineselected);			
	psy_signal_connect(&self->component.signal_focus, self,
		machineview_onfocus);
	psy_signal_connect(&self->workspace->signal_skinchanged, self,
		machineview_onskinchanged);
	psy_signal_connect(&workspace->signal_configchanged, self,
		machineview_onconfigure);		
}

void machineview_onmousedoubleclick(MachineView* self, psy_ui_MouseEvent* ev)
{	
	self->newmachine.pluginsview.calledby = 0;	
	machineview_selectsection(self, &self->component, 1);
}

void machineview_onmousedown(MachineView* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 2 && tabbar_selected(&self->tabbar) == 1) {
		tabbar_select(&self->tabbar, 0);		
	}
	psy_ui_mouseevent_stoppropagation(ev);
}

void machineview_onmouseup(MachineView* self, psy_ui_MouseEvent* ev)
{
}

void machineview_onkeydown(MachineView* self, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_ESCAPE &&
			tabbar_selected(&self->tabbar) == 1) {
		tabbar_select(&self->tabbar, 0);
		psy_ui_component_setfocus(&self->wireview.component);
		psy_ui_keyevent_stoppropagation(ev);
	}	
}

void machineview_onfocus(MachineView* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(&self->wireview.component);
}

void machineview_onskinchanged(MachineView* self, Workspace* sender)
{			
	machinewireview_updateskin(&self->wireview);
	newmachine_updateskin(&self->newmachine);
}

psy_ui_Rectangle machinewireview_bounds(MachineWireView* self)
{
	psy_ui_Rectangle rv;
	psy_TableIterator it;

	rv = psy_ui_rectangle_zero();
	for (it = psy_table_begin(&self->machineuis); 
			!psy_tableiterator_equal(&it, psy_table_end()); 
			psy_tableiterator_inc(&it)) {		
		psy_ui_Rectangle r;

		r = machineui_position(((MachineUi*) psy_tableiterator_value(&it)));
		psy_ui_rectangle_union(&rv, &r);
	}
	psy_ui_rectangle_expand(&rv, 0, 10, 10, 0);
	return rv;
}

psy_ui_Rectangle machinewireview_updaterect(MachineWireView* self, uintptr_t slot)
{		
	psy_ui_Rectangle rv;	
	MachineUi* machineui;
	
	machineui = machineuis_at(self, slot);
	if (machineui) {		
		psy_audio_MachineSockets* sockets;

		rv = machineui_position(machineui);
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
						psy_ui_Rectangle out;						

						out = machineui_position(inmachineui);
						psy_ui_rectangle_union(&rv, &out);
					}
				}
			}
			for(it = psy_audio_wiresockets_begin(&sockets->inputs);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
				psy_audio_WireSocket* socket;

				socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
				if (socket->slot != UINTPTR_MAX) {
					MachineUi* outmachineui;

					outmachineui = machineuis_at(self, socket->slot);
					if (outmachineui && machineui) {						
						psy_ui_Rectangle in;

						in = machineui_position(outmachineui);
						psy_ui_rectangle_union(&rv, &in);
					}
				}
			}
		}
		return rv;
	}
	return psy_ui_rectangle_zero();
}

void machinewireview_onpreferredsize(MachineWireView* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_Rectangle bounds;

	bounds = machinewireview_bounds(self);
	rv->width = psy_ui_value_makepx(bounds.right);
	rv->height = psy_ui_value_makepx(bounds.bottom);
}

void machineview_selectsection(MachineView* self, psy_ui_Component* sender,
	uintptr_t section)
{
	tabbar_select(&self->tabbar, (int) section);
	if (section == 0) {
		psy_ui_component_setfocus(&self->wireview.component);
	} else {
		psy_ui_component_setfocus(&self->newmachine.component);
	}
}

void machineview_onsongchanged(MachineView* self, Workspace* workspace,
	int flag, psy_audio_SongFile* songfile)
{
	tabbar_select(&self->tabbar, 0);
}

void machineview_onconfigure(MachineView* self, Workspace* sender, psy_Property*
	property)
{
	machinewireview_configure(&self->wireview,
		psycleconfig_macview(workspace_conf(sender)));
}

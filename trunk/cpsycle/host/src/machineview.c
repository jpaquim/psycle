// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

// prefix file for each .c file
#include "../../detail/prefix.h"

#include "machineview.h"

// host imports
#include "skingraphics.h"
#include "skinio.h"
#include "resources/resource.h"
#include "wireview.h"
// audio imports
#include <exclusivelock.h>
#include <songio.h>
// helper imports
#include <dir.h>
// standard c imports
#include <math.h>
#include <stdlib.h>
#include <string.h>
// platform tweaks
#include "../../detail/os.h"
#include "../../detail/portable.h"

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#endif

static void machineui_init(MachineUi*, psy_audio_Machine* machine,
	uintptr_t slot, MachineSkin*, Workspace*);
static void machineui_updatecoords(MachineUi*);
static void machineui_dispose(MachineUi*);
static void machineui_onframedestroyed(MachineUi*, psy_ui_Component* sender);
static psy_ui_IntSize machineui_size(MachineUi*);
static psy_ui_Rectangle machineui_position(MachineUi*);
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
static void machineui_invalidate(MachineUi*, MachineWireView*);

// MachineUi
void machineui_init(MachineUi* self, psy_audio_Machine* machine,
	uintptr_t slot, MachineSkin* skin, Workspace* workspace)
{	
	assert(machine);
	self->machine = machine;	
	self->workspace = workspace;	
	self->skin = skin;	
	self->mode = psy_audio_machine_mode(machine);
	self->coords = NULL;
	machineui_updatecoords(self);	
	self->volumedisplay = (psy_dsp_amp_t)0.f;
	self->volumemaxdisplay = (psy_dsp_amp_t)0.f;
	self->volumemaxcounterlife = 0;	
	self->slot = slot;
	self->frame = NULL;
	self->paramview = NULL;
	self->editorview = NULL;
	self->restorename = NULL;
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

void machineui_editname(MachineUi* self, psy_ui_Edit* edit,
	psy_ui_IntPoint scroll)
{
	if (self->machine) {
		psy_ui_Rectangle r;
		int x;
		int y;
		
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
		if (psy_audio_machine_editname(self->machine)) {
			if (self->skin->drawmachineindexes) {
				psy_snprintf(editname, 130, "%.2X:%s", (int)slot, 
					psy_audio_machine_editname(self->machine));
			} else {
				psy_snprintf(editname, 130, "%s", 
					psy_audio_machine_editname(self->machine));
			}
		}
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		skin_blitpart(g, &wireview->skin.skinbmp, r.left, r.top,
			&self->coords->background);
		if (self->mode == MACHMODE_FX) {			
			psy_ui_settextcolor(g, wireview->skin.effect_fontcolour);
		} else {		
			psy_ui_settextcolor(g, wireview->skin.generator_fontcolour);;
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
			x + vupeak->destx + (int)(self->volumemaxdisplay * vu->destwidth),
			y + vupeak->desty,
			vupeak->destwidth, vupeak->destheight,
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
			: 0;		
		machineui_updatemaxvolumedisplay(self);
	}
}

void machineui_updatemaxvolumedisplay(MachineUi* self)
{
	self->volumemaxcounterlife--;
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
			machineframe_init(self->frame, parent, workspace_showparamviewaswindow(self->workspace),
				self->workspace);
			psy_signal_connect(&self->frame->component.signal_destroy, self,
				machineui_onframedestroyed);
			if (psy_audio_machine_haseditor(self->machine)) {
				MachineEditorView* editorview;

				editorview = machineeditorview_allocinit(&self->frame->notebook.component,
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
			if (!workspace_showparamviewaswindow(self->workspace)) {
				workspace_dockview(self->workspace, &self->frame->component);
			}
		}		
	}
}

void machineui_onframedestroyed(MachineUi* self, psy_ui_Component* sender)
{	
	self->frame = 0;	
}

// MachineWireView
static psy_ui_Rectangle machinewireview_bounds(MachineWireView*);
static void machinewireview_connectuisignals(MachineWireView*);
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
static void machinewireview_ondestroy(MachineWireView*,
	psy_ui_Component* component);
static void machinewireview_onmousedown(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onmouseup(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onmousemove(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onmousedoubleclick(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onkeydown(MachineWireView*, psy_ui_KeyEvent*);
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
static void machinewireview_onnewmachineselected(MachineView*,
	psy_ui_Component* sender, psy_Properties*);
static void machinewireview_initmachinecoords(MachineWireView*);
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
static void machinewireview_applyproperties(MachineWireView*, psy_Properties*);
static void machinewireview_onshowparameters(MachineWireView*, Workspace*,
	uintptr_t slot);
static void machinewireview_onmasterworked(MachineWireView*,
	psy_audio_Machine*, uintptr_t slot, psy_audio_BufferContext*);
static void machinewireview_ontimer(MachineWireView*, uintptr_t timerid);
static void machinewireview_invalidateallmacvus(MachineWireView*);
static void machinewireview_onconfigchanged(MachineWireView*, Workspace*,
	psy_Properties*);
static void machinewireview_readconfig(MachineWireView*);
static void machinewireview_showwireview(MachineWireView*,
	psy_audio_Wire wire);
static void machinewireview_onwireframedestroyed(MachineWireView*,
	psy_ui_Component* sender);
static WireFrame* machinewireview_wireframe(MachineWireView*,
	psy_audio_Wire wire);
static void machinewireview_setcoords(MachineWireView*, psy_Properties*);
static void machinewireview_onsize(MachineWireView*, psy_ui_Component* sender,
	psy_ui_Size* size);
static psy_ui_Rectangle machinewireview_updaterect(MachineWireView* self,
	uintptr_t slot);
static void machinewireview_onpreferredsize(MachineWireView* self, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static MachineUi* machineuis_insert(MachineWireView*, uintptr_t slot);
static MachineUi* machineuis_at(MachineWireView*, uintptr_t slot);
static void machineuis_remove(MachineWireView*, uintptr_t slot);
static void machineuis_removeall(MachineWireView*);

static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MachineWireView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_ondraw)machinewireview_ondraw;
		vtable.ontimer = (psy_ui_fp_ontimer)machinewireview_ontimer;
		vtable.onmousedown = (psy_ui_fp_onmousedown)
			machinewireview_onmousedown;
		vtable.onmouseup = (psy_ui_fp_onmouseup)machinewireview_onmouseup;
		vtable.onmousemove = (psy_ui_fp_onmousemove)
			machinewireview_onmousemove;
		vtable.onmousedoubleclick = (psy_ui_fp_onmousedoubleclick)
			machinewireview_onmousedoubleclick;
		vtable.onkeydown = (psy_ui_fp_onkeydown)
			machinewireview_onkeydown;
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			machinewireview_onpreferredsize;
		vtable_initialized = TRUE;
	}
}

void machinewireview_init(MachineWireView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	psy_ui_FontInfo fontinfo;

	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->firstsize = 0;
	self->statusbar = 0;
	self->workspace = workspace;
	self->machines = &workspace->song->machines;
	self->drawvumeters = 1;
	self->wireframes = 0;
	self->randominsert = 1;
	self->addeffect = 0;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);	
	// skin init
	psy_ui_bitmap_init(&self->skin.skinbmp);
	psy_ui_bitmap_loadresource(&self->skin.skinbmp, IDB_MACHINESKIN);
	psy_ui_fontinfo_init(&fontinfo, "Tahoma", 16);
	psy_ui_font_init(&self->skin.font, &fontinfo);
	machinewireview_initmachinecoords(self);	
	psy_table_init(&self->machineuis);
	machineuis_insert(self, psy_audio_MASTER_INDEX);
	psy_ui_component_setfont(&self->component, &self->skin.font);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);
	machinewireview_connectuisignals(self);	
	self->dragslot = UINTPTR_MAX;
	self->dragmode = MACHINEWIREVIEW_DRAG_MACHINE;
	self->selectedslot = psy_audio_MASTER_INDEX;
	self->showwirehover = FALSE;
	psy_audio_wire_init(&self->selectedwire);
	psy_audio_wire_init(&self->hoverwire);
	psy_signal_connect(&workspace->signal_songchanged, self,
		machinewireview_onsongchanged);	
	machinewireview_connectmachinessignals(self);
	psy_signal_connect(&workspace->signal_configchanged, self,
		machinewireview_onconfigchanged);	
	psy_signal_connect(&workspace->signal_showparameters, self,
		machinewireview_onshowparameters);	
	psy_signal_connect(&self->component.signal_size, self,
		machinewireview_onsize);
	psy_ui_edit_init(&self->editname, &self->component);
	psy_ui_component_hide(&self->editname.component);
	psy_ui_component_starttimer(&self->component, 0, 50);
	if (workspace->machineviewtheme) {
		machinewireview_applyproperties(self, workspace->machineviewtheme);
	}
	self->firstsize = 1;	
}

void machinewireview_connectuisignals(MachineWireView* self)
{
	psy_signal_connect(&self->component.signal_destroy, self,
		machinewireview_ondestroy);
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

void machinewireview_ondestroy(MachineWireView* self, psy_ui_Component* sender)
{	
	machineuis_removeall(self);
	psy_table_dispose(&self->machineuis);
	psy_list_deallocate(&self->wireframes, (psy_fp_disposefunc)
		psy_ui_component_destroy);	
	psy_ui_font_dispose(&self->skin.font);
}

void machinewireview_initmachinecoords(MachineWireView* self)
{	
	MachineCoords master = {
		{ 0, 52, 138, 35, 0, 0, 138, 35, 0 },		// background
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 },
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 },
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 },
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 },
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 },
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 },
		{ 0,  0,   0,  0, 0, 0,   0,  0, 0 }
	};
	MachineCoords generator = {
		{ 0, 87, 138, 52, 0, 0, 138, 52, 0 },		// background
		{ 0, 156, 0, 7, 4, 20, 129, 7, 129},		// vu0
		{ 108, 156, 1, 7, 4, 20, 1, 7, 82 },		// vupeak
		{ 0, 139, 6, 13, 6, 33, 6, 13, 82 },		// pan
		{ 23, 139, 17, 17, 117, 31, 17, 17, 0 },	// mute
		{ 40, 139, 17, 17, 98, 31, 17, 17, 0 },		// bypass
		{ 6, 139, 17, 17, 98, 31, 17, 17, 0 },		// solo
		{ 0, 0, 0, 0, 20, 3, 117, 15, 0 },			// name
	};			
	MachineCoords effect = {
		{ 0, 0, 138, 52, 0, 0, 138, 52, 0 },		// background
		{ 0, 163, 0, 7, 4, 20, 129, 7, 129 },		// vu0
		{ 96, 144, 1, 7, 4, 20, 1, 7, 0 },			// vupeak
		{ 57, 139, 6, 13, 6, 33, 6, 13, 82 },		// pan
		{ 23, 139, 17, 17, 117, 31, 17, 17, 0 },	// mute
		{ 40, 139, 17, 17, 98, 31, 17, 17, 0 },		// bypass
		{ 40, 139, 17, 17, 98, 31, 17, 17, 0 },		// solo
		{ 0, 0, 0, 0, 20, 3, 117, 15, 0 },			// name 
	};									
	self->skin.master = master;
	self->skin.generator = generator;
	self->skin.effect = effect;		
	machinewireview_applyproperties(self, 0);
}

void machinewireview_onconfigchanged(MachineWireView* self,
	Workspace* workspace, psy_Properties* property)
{	
	machinewireview_readconfig(self);
}

void machinewireview_readconfig(MachineWireView* self)
{
	psy_Properties* mv;
	
	mv = psy_properties_findsection(self->workspace->config,
		"visual.machineview");
	if (mv) {		
		self->drawvumeters = psy_properties_bool(mv, "drawvumeters", 1);
		psy_ui_component_stoptimer(&self->component, 0);
		if (self->drawvumeters) {
			psy_ui_component_starttimer(&self->component, 0, 50);
		}
		self->skin.drawmachineindexes = psy_properties_bool(mv,
			"drawmachineindexes", 1);
		self->showwirehover = workspace_showwirehover(self->workspace);
	}
}

void machinewireview_applyproperties(MachineWireView* self, psy_Properties* p)
{
	const char* machine_skin_name;

	self->skin.drawmachineindexes = workspace_showmachineindexes(self->workspace);
	self->skin.colour = psy_properties_int(p, "mv_colour", 0x00232323);
	self->skin.wirecolour = psy_properties_int(p, "mv_wirecolour", 0x005F5F5F);
	self->skin.selwirecolour = psy_properties_int(p, "mv_selwirecolour", 0x007F7F7F);
	self->skin.hoverwirecolour = psy_properties_int(p, "mv_hoverwirecolour", 0x007F7F7F);
	self->skin.polycolour = psy_properties_int(p, "mv_wireaacolour2", 0x005F5F5F);
	self->skin.polycolour = psy_properties_int(p, "mv_polycolour", 0x00B1C8B0);
	self->skin.generator_fontcolour = 
		psy_properties_int(p, "mv_generator_fontcolour", 0x00B1C8B0); // 0x00B1C8B0
	self->skin.effect_fontcolour = 
		psy_properties_int(p, "mv_effect_fontcolour", 0x00D1C5B6);
	self->skin.triangle_size = psy_properties_int(p, "mv_triangle_size", 10);
	self->skin.wireaacolour 
		= ((((self->skin.wirecolour&0x00ff0000) 
			+ ((self->skin.colour&0x00ff0000)*4))/5)&0x00ff0000) +
		  ((((self->skin.wirecolour&0x00ff00)
			+ ((self->skin.colour&0x00ff00)*4))/5)&0x00ff00) +
		  ((((self->skin.wirecolour&0x00ff) 
			+ ((self->skin.colour&0x00ff)*4))/5)&0x00ff);
	self->skin.wireaacolour2
		= (((((self->skin.wirecolour&0x00ff0000))
			+ ((self->skin.colour&0x00ff0000)))/2)&0x00ff0000) +
		  (((((self->skin.wirecolour&0x00ff00))
			+ ((self->skin.colour&0x00ff00)))/2)&0x00ff00) +
		  (((((self->skin.wirecolour&0x00ff))
			+ ((self->skin.colour&0x00ff)))/2)&0x00ff);
	psy_ui_component_setbackgroundcolor(&self->component, self->skin.colour);
	machine_skin_name = psy_properties_readstring(p, "machine_skin", 0);
	if (machine_skin_name && strlen(machine_skin_name)) {
		char path[_MAX_PATH];
		char filename[_MAX_PATH];

		strcpy(filename, machine_skin_name);
		strcat(filename, ".bmp");
		psy_dir_findfile(workspace_skins_directory(self->workspace),
			filename, path);
		if (path[0] != '\0') {
			psy_ui_Bitmap bmp;

			psy_ui_bitmap_init(&bmp);
			if (psy_ui_bitmap_load(&bmp, path) == 0) {
				psy_ui_bitmap_dispose(&self->skin.skinbmp);
				self->skin.skinbmp = bmp; 
			}
		}
		strcpy(filename, machine_skin_name);
		strcat(filename, ".psm");
		psy_dir_findfile(workspace_skins_directory(self->workspace),
			filename, path);
		if (path[0] != '\0') {
			psy_Properties* coords;

			coords = psy_properties_create();
			skin_loadpsh(coords, path);
			machinewireview_setcoords(self, coords);
			properties_free(coords);
		}
	}
}

void machinewireview_setcoords(MachineWireView* self, psy_Properties* p)
{
	const char* s;
	int vals[4];	
	
	// master
	if (s = psy_properties_readstring(p, "master_source", 0)) {	
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->skin.master.background, vals);
	}
	// generator
	if (s = psy_properties_readstring(p, "generator_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->skin.generator.background, vals);		
	}
	if (s = psy_properties_readstring(p, "generator_vu0_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->skin.generator.vu0, vals);
	}
	if (s = psy_properties_readstring(p, "generator_vu_peak_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->skin.generator.vupeak, vals);
	}
	if (s = psy_properties_readstring(p, "generator_pan_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->skin.generator.pan, vals);
	}
	if (s = psy_properties_readstring(p, "generator_mute_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->skin.generator.mute, vals);		
	}
	if (s = psy_properties_readstring(p, "generator_solo_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->skin.generator.solo, vals);
	}
	if (s = psy_properties_readstring(p, "generator_vu_dest", 0)) {
		skin_psh_values(s, 3, vals);
		skincoord_setdest(&self->skin.generator.vu0, vals);		
	}
	if (s = psy_properties_readstring(p, "generator_pan_dest", 0)) {
		skin_psh_values(s, 3, vals);
		skincoord_setdest(&self->skin.generator.pan, vals);
	}
	if (s = psy_properties_readstring(p, "generator_mute_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->skin.generator.mute, vals);
	}
	if (s = psy_properties_readstring(p, "generator_solo_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->skin.generator.solo, vals);
	}
	if (s = psy_properties_readstring(p, "generator_name_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->skin.generator.name, vals);
	}
	// effect
	if (s = psy_properties_readstring(p, "effect_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->skin.effect.background, vals);		
	}
	if (s = psy_properties_readstring(p, "effect_vu0_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->skin.effect.vu0, vals);
	}
	if (s = psy_properties_readstring(p, "effect_vu_peak_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->skin.effect.vupeak, vals);
	}
	if (s = psy_properties_readstring(p, "effect_pan_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->skin.effect.pan, vals);
	}
	if (s = psy_properties_readstring(p, "effect_mute_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->skin.effect.mute, vals);		
	}
	if (s = psy_properties_readstring(p, "effect_bypass_source", 0)) {
		skin_psh_values(s, 4, vals);
		skincoord_setsource(&self->skin.effect.bypass, vals);
	}
	if (s = psy_properties_readstring(p, "effect_vu_dest", 0)) {
		skin_psh_values(s, 3, vals);
		skincoord_setdest(&self->skin.effect.vu0, vals);		
	}
	if (s = psy_properties_readstring(p, "effect_pan_dest", 0)) {
		skin_psh_values(s, 3, vals);
		skincoord_setdest(&self->skin.effect.pan, vals);
	}
	if (s = psy_properties_readstring(p, "effect_mute_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->skin.effect.mute, vals);
	}
	if (s = psy_properties_readstring(p, "effect_bypass_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->skin.effect.bypass, vals);
	}
	if (s = psy_properties_readstring(p, "effect_name_dest", 0)) {
		skin_psh_values(s, 2, vals);
		skincoord_setdest(&self->skin.effect.name, vals);
	}
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
	WireSocket* p;
	
	sockets	= connections_at(&self->machines->connections, slot);
	if (sockets) {
		for (p = sockets->outputs; p != NULL; p = p->next) {
			psy_audio_WireSocketEntry* entry =
				(psy_audio_WireSocketEntry*)p->entry;
			if (entry->slot != UINTPTR_MAX) {
				MachineUi* inmachineui;				

				inmachineui = machineuis_at(self, entry->slot);
				if (inmachineui && outmachineui) {
					psy_ui_Rectangle out;
					psy_ui_Rectangle in;					

					out = machineui_position(inmachineui);
					in = machineui_position(outmachineui);
					if (self->hoverwire.src == slot &&
						self->hoverwire.dst == entry->slot) {
						psy_ui_setcolor(g, self->skin.hoverwirecolour);
					} else
					if (self->selectedwire.src == slot &&
							self->selectedwire.dst == entry->slot) {
						psy_ui_setcolor(g, self->skin.selwirecolour);
					} else {
						psy_ui_setcolor(g, self->skin.wirecolour);
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
	float deltaColR = ((self->skin.polycolour     & 0xFF) / 510.0f) + .45f;
	float deltaColG = ((self->skin.polycolour>>8  & 0xFF) / 510.0f) + .45f;
	float deltaColB = ((self->skin.polycolour>>16 & 0xFF) / 510.0f) + .45f;	
	unsigned int polyInnards;
	double phi;
	
	polyInnards = psy_ui_color_make_rgb((uint8_t)(192 * deltaColR),
		(uint8_t)(192 * deltaColG), (uint8_t)(192 * deltaColB));
			
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
	
	psy_ui_drawsolidpolygon(g, tri, 4, polyInnards, self->skin.wireaacolour);
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
			psy_ui_setcolor(g, self->skin.wirecolour);			
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
	psy_ui_setcolor(g, wireview->skin.wirecolour);
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

void machinewireview_onsize(MachineWireView* self, psy_ui_Component* sender,
	psy_ui_Size* size)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	if (self->firstsize && psy_ui_value_px(&size->width, &tm) > 0) {
		self->firstsize = 0;
		machinewireview_align(self);
	}
}

void machinewireview_align(MachineWireView* self)
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
				machineui_invalidate(machineui, self);
			}
		} else if (self->dragmode == MACHINEWIREVIEW_DRAG_MACHINE) {
			MachineUi* machineui;
			psy_ui_Rectangle r_old;
			psy_ui_Rectangle r_new;

			machineui = machineuis_at(self, self->dragslot);
			if (machineui) {
				r_old = machineui_position(machineui);
				psy_ui_rectangle_expand(&r_old, 10, 10, 10, 10);
				psy_audio_machine_setposition(machineui->machine,
					max(0, ev->x - self->mx), max(0, ev->y - self->my));
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
			} else {
				self->dragmode = MACHINEWIREVIEW_DRAG_NONE;
			}
		} else if (self->dragmode >= MACHINEWIREVIEW_DRAG_NEWCONNECTION &&
				self->dragmode <= MACHINEWIREVIEW_DRAG_RIGHTCONNECTION) {			
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

void machinewireview_onmouseup(MachineWireView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_releasecapture(&self->component);
	if (self->dragslot != UINTPTR_MAX) {
		if (self->dragmode == MACHINEWIREVIEW_DRAG_MACHINE) {
			psy_ui_component_updateoverflow(&self->component);
		} else if (self->dragmode >= MACHINEWIREVIEW_DRAG_NEWCONNECTION &&
			self->dragmode <= MACHINEWIREVIEW_DRAG_RIGHTCONNECTION) {
			if (self->mousemoved) {
				uintptr_t slot = self->dragslot;
				self->dragslot = UINTPTR_MAX;
				machinewireview_hittest(self);
				if (self->dragslot != UINTPTR_MAX) {
					if (self->dragmode != MACHINEWIREVIEW_DRAG_NEWCONNECTION) {
						psy_audio_machines_disconnect(self->machines,
							self->selectedwire);
					}
					if (self->dragmode < MACHINEWIREVIEW_DRAG_RIGHTCONNECTION) {
						psy_audio_machines_connect(self->machines,
							psy_audio_wire_make(slot, self->dragslot));
					} else {
						psy_audio_machines_connect(self->machines,
							psy_audio_wire_make(self->dragslot, slot));
					}
				}
			} else if (ev->button == 2) {
				workspace_showgear(self->workspace);
			}			
		}
	}
	self->dragslot = UINTPTR_MAX;
	psy_ui_component_invalidate(&self->component);
}

void machinewireview_onkeydown(MachineWireView* self, psy_ui_KeyEvent* ev)
{		
	if (ev->keycode == psy_ui_KEY_DELETE &&
			psy_audio_wire_valid(&self->selectedwire)) {
		psy_audio_machines_disconnect(self->machines, self->selectedwire);
	} else 
	if (ev->keycode == psy_ui_KEY_DELETE && self->selectedslot != - 1 &&
			self->selectedslot != psy_audio_MASTER_INDEX) {
		psy_audio_machines_remove(self->machines, self->selectedslot);		
		self->selectedslot = UINTPTR_MAX;
	} else 
	if (ev->repeat) {
		psy_ui_keyevent_stoppropagation(ev);
	}
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
		WireSocket* p;			
		uintptr_t slot = it.curr->key;
	
		sockets	= connections_at(&self->machines->connections, slot);
		if (sockets) {
			p = sockets->outputs;	
			while (p != NULL) {
				psy_audio_WireSocketEntry* entry;

				entry =	(psy_audio_WireSocketEntry*) p->entry;
				if (entry->slot != UINTPTR_MAX) {					
					MachineUi* inmachineui;
					MachineUi* outmachineui;

					inmachineui = machineuis_at(self, entry->slot);
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
							psy_audio_wire_set(&rv, slot, entry->slot);							
						}						
					}
				}
				if (psy_audio_wire_valid(&rv)) {
					break;
				}
				p = p->next;
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
				max(0, self->mx - width / 2),
				max(0, self->my - height / 2));
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
			machinewireview_align(self);
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
	
	for (p = self->wireframes; p != NULL; p = q) {
		WireFrame* frame;

		frame = (WireFrame*) p->entry;
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
		machineui  = psy_tableiterator_value(&it);
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
			} else
			if (machineui->frame->dodock) {
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
			} else
			if (machineui->frame->doclose) {
				psy_ui_Component* dockparent;

				dockparent = psy_ui_component_parent(&machineui->frame->component);
				psy_ui_component_destroy(&machineui->frame->component);
				psy_ui_component_align(psy_ui_component_parent(dockparent));
				psy_ui_component_align(dockparent);
			}
		}
	}
	machinewireview_invalidateallmacvus(self);
}

void machinewireview_invalidateallmacvus(MachineWireView* self)
{
	psy_TableIterator it;
	
	for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {		
		machineui_invalidate(psy_tableiterator_value(&it), self);
	}	
}

void machineui_invalidate(MachineUi* self, MachineWireView* wireview)
{	
	psy_ui_component_invalidaterect(&wireview->component,
		machineui_position(self));
}

void machinewireview_showwireview(MachineWireView* self, psy_audio_Wire wire)
{			
	WireFrame* wireframe;

	wireframe = machinewireview_wireframe(self, wire);
	if (!wireframe) {
		WireView* wireview;

		wireframe = (WireFrame*)malloc(sizeof(WireFrame));					
		psy_list_append(&self->wireframes, wireframe);		
		wireframe_init(wireframe, &self->component, 0);
		psy_signal_connect(&wireframe->component.signal_destroyed, self,
			machinewireview_onwireframedestroyed);
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
	psy_ui_Component* sender, psy_Properties* plugininfo)
{		
	psy_audio_Machine* machine;
	
	machine = psy_audio_machinefactory_makemachinefrompath(
		&self->workspace->machinefactory,
		psy_properties_int(plugininfo, "type", UINTPTR_MAX),
		psy_properties_readstring(plugininfo, "path", ""),
		psy_properties_int(plugininfo, "shellidx", 0));
	if (machine) {
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
static void machineviewbar_onlanguagechanged(MachineViewBar*,
	Workspace* sender);
static void machineviewbar_updatetext(MachineViewBar* self,
	Workspace* workspace);

void machineviewbar_init(MachineViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;
	psy_ui_component_enablealign(&self->component);	
	psy_ui_checkbox_init(&self->mixersend, &self->component);	
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(4), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->mixersend.component, &margin);
	psy_ui_checkbox_check(&self->mixersend);
	psy_signal_connect(&self->mixersend.signal_clicked, self,
		machineviewbar_onmixerconnectmodeclick);	
	psy_ui_component_setalign(&self->mixersend.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->status, &self->component);
	psy_ui_label_setcharnumber(&self->status, 44);		
	psy_ui_component_setalign(psy_ui_label_base(&self->status),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_doublebuffer(psy_ui_label_base(&self->status));
	psy_signal_connect(&workspace->signal_songchanged, self,
		machineviewbar_onsongchanged);
	machineviewbar_updatetext(self, workspace);
	psy_signal_connect(&workspace->signal_languagechanged, self,
		machineviewbar_onlanguagechanged);	
}

void machineviewbar_updatetext(MachineViewBar* self, Workspace* workspace)
{
	psy_ui_checkbox_settext(&self->mixersend,
		workspace_translate(workspace,
			"machineview.Connect to Mixer Send/Return Input"));
}

void machineviewbar_onlanguagechanged(MachineViewBar* self, Workspace* sender)
{
	machineviewbar_updatetext(self, sender);
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
	if (!self->workspace->song ||
			psy_audio_machines_isconnectasmixersend(&self->workspace->song->machines)) {
		psy_ui_checkbox_check(&self->mixersend);
	} else {
		psy_ui_checkbox_disablecheck(&self->mixersend);		
	}
}

// MachineView
static void machineview_updatetext(MachineView*, Workspace*);
static void machineview_onsongchanged(MachineView*, Workspace*, int flag, psy_audio_SongFile*);
static void machineview_onmousedown(MachineView*, psy_ui_MouseEvent*);
static void machineview_onmousedoubleclick(MachineView*, psy_ui_MouseEvent*);
static void machineview_onkeydown(MachineView*, psy_ui_KeyEvent*);
static void machineview_onfocus(MachineView*, psy_ui_Component* sender);
static void machineview_onskinchanged(MachineView*, Workspace*);
static void machineview_onlanguagechanged(MachineView*, Workspace* workspace);
static void selectsection(MachineView*, psy_ui_Component* sender, uintptr_t section);

static psy_ui_ComponentVtable machineview_vtable;
static bool machineview_vtable_initialized = FALSE;

static void machineview_vtable_init(MachineView* self)
{
	if (!machineview_vtable_initialized) {
		machineview_vtable = *(self->component.vtable);
		machineview_vtable.onmousedown = (psy_ui_fp_onmousedown)
			machineview_onmousedown;	
		machineview_vtable.onmousedoubleclick = (psy_ui_fp_onmousedoubleclick)
			machineview_onmousedoubleclick;
		machineview_vtable.onkeydown = (psy_ui_fp_onkeydown)
			machineview_onkeydown;
		machineview_vtable_initialized = TRUE;
	}
}

void machineview_init(MachineView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	machineview_vtable_init(self);
	self->component.vtable = &machineview_vtable;
	self->workspace = workspace;
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	psy_ui_component_enablealign(&self->component);
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		machineview_onsongchanged);
	psy_ui_notebook_init(&self->notebook, &self->component);	
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook), psy_ui_ALIGN_CLIENT);	
	machinewireview_init(&self->wireview,psy_ui_notebook_base(&self->notebook),
		tabbarparent, workspace);
	psy_ui_component_setalign(&self->wireview.component, psy_ui_ALIGN_CLIENT);
	newmachine_init(&self->newmachine, psy_ui_notebook_base(&self->notebook),
		self->workspace);
	psy_ui_component_setalign(&self->newmachine.component, psy_ui_ALIGN_CLIENT);
	tabbar_init(&self->tabbar, tabbarparent);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	tabbar_append_tabs(&self->tabbar, "", "", NULL); // Wires, New Machine
	psy_signal_connect(&self->component.signal_selectsection, self, selectsection);
	psy_ui_notebook_setpageindex(&self->notebook, 0);	
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	psy_signal_connect(&self->newmachine.pluginsview.signal_selected, self,
		machinewireview_onnewmachineselected);			
	psy_signal_connect(&self->component.signal_focus, self,
		machineview_onfocus);
	psy_signal_connect(&self->workspace->signal_skinchanged, self,
		machineview_onskinchanged);	
	machineview_updatetext(self, workspace);
	psy_signal_connect(&workspace->signal_languagechanged, self,
		machineview_onlanguagechanged);
	if (workspace_showwirehover(workspace)) {
		self->wireview.showwirehover = TRUE;
	}
	self->wireview.firstsize = 1;
}

void machineview_updatetext(MachineView* self, Workspace* workspace)
{
	tabbar_rename_tabs(&self->tabbar,
		workspace_translate(self->workspace, "machineview.Wires"),
		workspace_translate(self->workspace, "machineview.New Machine"),
		NULL);
}

void machineview_onlanguagechanged(MachineView* self, Workspace* workspace)
{
	machineview_updatetext(self, workspace);
	psy_ui_component_align(&self->component);
}

void machineview_onmousedoubleclick(MachineView* self, psy_ui_MouseEvent* ev)
{	
	self->newmachine.pluginsview.calledby = 0;	
	tabbar_select(&self->tabbar, 1);
}

void machineview_onmousedown(MachineView* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 2 && tabbar_selected(&self->tabbar) == 1) {
		tabbar_select(&self->tabbar, 0);		
	}
	psy_ui_mouseevent_stoppropagation(ev);
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
	machinewireview_applyproperties(&self->wireview,
		sender->machineviewtheme);	
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
	return rv;
}

psy_ui_Rectangle machinewireview_updaterect(MachineWireView* self, uintptr_t slot)
{		
	psy_ui_Rectangle rv;	
	MachineUi* machineui;
	
	machineui = machineuis_at(self, slot);
	if (machineui) {
		WireSocket* p;
		psy_audio_MachineSockets* sockets;

		rv = machineui_position(machineui);
		sockets = connections_at(&self->machines->connections, slot);
		if (sockets) {
			for (p = sockets->outputs; p != NULL; p = p->next) {
				psy_audio_WireSocketEntry* entry =
					(psy_audio_WireSocketEntry*)p->entry;
				if (entry->slot != UINTPTR_MAX) {
					MachineUi* inmachineui;

					inmachineui = machineuis_at(self, entry->slot);
					if (inmachineui && machineui) {
						psy_ui_Rectangle out;						

						out = machineui_position(inmachineui);
						psy_ui_rectangle_union(&rv, &out);
					}
				}
			}
			for (p = sockets->inputs; p != NULL; p = p->next) {
				psy_audio_WireSocketEntry* entry =
					(psy_audio_WireSocketEntry*)p->entry;
				if (entry->slot != UINTPTR_MAX) {
					MachineUi* outmachineui;

					outmachineui = machineuis_at(self, entry->slot);
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

void selectsection(MachineView* self, psy_ui_Component* sender, uintptr_t section)
{
	tabbar_select(&self->tabbar, (int) section);
	if (psy_ui_notebook_activepage(&self->notebook)) {
		psy_ui_component_setfocus(psy_ui_notebook_activepage(&self->notebook));
	}
}

void machineview_onsongchanged(MachineView* self, Workspace* workspace,
	int flag, psy_audio_SongFile* songfile)
{
	tabbar_select(&self->tabbar, 0);
}

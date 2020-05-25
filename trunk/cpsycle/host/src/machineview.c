// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "machineview.h"
#include "wireview.h"
#include "skingraphics.h"
#include "skinio.h"
#include "resources/resource.h"
#include <math.h>
#include <dir.h>
#include <stdlib.h>
#include <string.h>
#include <exclusivelock.h>
#include "../../detail/portable.h"
#include <songio.h>

#if defined DIVERSALIS__OS__UNIX
#define _MAX_PATH 4096
#endif

#define RGB(r,g,b)  ((uint32_t)(((uint16_t)(r)|((uint16_t)((uint8_t)(g))<<8))|(((uint16_t)(uint8_t)(b))<<16)))

#define TIMERID_UPDATEVUMETERS 300

static void machineviewbar_settext(MachineViewBar*, const char* text);
static void machineviewbar_ondestroy(MachineViewBar*, psy_ui_Component* sender);
static void machineviewbar_ondraw(MachineViewBar* self, psy_ui_Graphics*);
static void machineviewbar_onpreferredsize(MachineViewBar*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void machineviewbar_onsongchanged(MachineViewBar*, Workspace*,
	int flag, psy_audio_SongFile*);


static MachineUi* machineuis_insert(MachineWireView*, uintptr_t slot, int x, int y,
	MachineSkin*);
static MachineUi* machineuis_at(MachineWireView*, uintptr_t slot);
static void machineuis_remove(MachineWireView*, uintptr_t slot);
static void machineuis_removeall(MachineWireView*);

static void machineui_init(MachineUi*, int x, int y,
	psy_audio_Machine* machine, uintptr_t slot, MachineSkin*, Workspace*);
static void machineui_updatecoords(MachineUi*);
static void machineui_dispose(MachineUi*);
static void machineui_onframedestroyed(MachineUi*, psy_ui_Component* sender);
static psy_ui_Size machineui_size(MachineUi*);
static psy_ui_Rectangle machineui_position(MachineUi*);
static void machineui_draw(MachineUi*, psy_ui_Graphics*, MachineWireView*, uintptr_t slot);
static void machineui_drawvu(MachineUi*, psy_ui_Graphics*, MachineWireView*);
static void machineui_drawvudisplay(MachineUi*, psy_ui_Graphics*, MachineWireView*);
static void machineui_drawvupeak(MachineUi*, psy_ui_Graphics*, MachineWireView*);
static void machineui_drawhighlight(MachineUi*, psy_ui_Graphics*, MachineWireView*);
static void machineui_updatevolumedisplay(MachineUi*);
static void machineui_updatemaxvolumedisplay(MachineUi*);
static void machineui_showparameters(MachineUi*, psy_ui_Component* parent);
static void machineui_editname(MachineUi*, psy_ui_Edit* edit);
static void machineui_onkeydown(MachineUi*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void machineui_oneditchange(MachineUi*, psy_ui_Edit* sender);
static void machineui_oneditfocuslost(MachineUi*, psy_ui_Component* sender);
static int slidercoord(SkinCoord*, float value);
static void machineui_invalidate(MachineUi*, MachineWireView*);

static void machinewireview_adjustscroll(MachineWireView*);
static psy_ui_Rectangle machinewireview_bounds(MachineWireView*);
static void machinewireview_onscroll(MachineWireView*, psy_ui_Component* sender,
	int stepx,	int stepy);
static void machinewireview_initmasterui(MachineWireView*);
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
	MachineUi* out, MachineUi* in);
static psy_ui_Point rotate_point(psy_ui_Point, double phi);
static psy_ui_Point move_point(psy_ui_Point pt, psy_ui_Point d);
static void machinewireview_ondestroy(MachineWireView*,
	psy_ui_Component* component);
static void machinewireview_onmousedown(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onmouseup(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onmousemove(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onmousedoubleclick(MachineWireView*, psy_ui_MouseEvent*);
static void machinewireview_onkeydown(MachineWireView*, psy_ui_KeyEvent*);
static void machinewireview_hittest(MachineWireView*, int x, int y);
static int machinewireview_hittestpan(MachineWireView*, int x, int y,
	uintptr_t slot, int* dx);
static int machinewireview_hittestcoord(MachineWireView*, int x, int y,
	int mode, uintptr_t slot, SkinCoord*);
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
static void machinewireview_onsongchanged(MachineWireView*, Workspace*,
	int flag, psy_audio_SongFile*);
static void machinewireview_buildmachineuis(MachineWireView*,
	psy_Table*);
static void machinewireview_applyproperties(MachineWireView*, psy_Properties*);
static void machinewireview_onshowparameters(MachineWireView*, Workspace*,
	uintptr_t slot);
static void machinewireview_onmachineworked(MachineWireView*,
	psy_audio_Machine*, uintptr_t slot, psy_audio_BufferContext*);
static void machinewireview_ontimer(MachineWireView*, int timerid);
static void machinewireview_preparedrawallmacvus(MachineWireView*);
static void machinewireview_onconfigchanged(MachineWireView*, Workspace*,
	psy_Properties*);
static void machinewireview_readconfig(MachineWireView*);
static void machinewireview_beforesavesong(MachineWireView*, Workspace*,
	psy_audio_SongFile*);
static void machinewireview_showwireview(MachineWireView*,
	psy_audio_Wire wire);
static void machinewireview_onwireframedestroyed(MachineWireView*,
	psy_ui_Component* sender);
static WireFrame* machinewireview_wireframe(MachineWireView*,
	psy_audio_Wire wire);
static void machinewireview_setcoords(MachineWireView*, psy_Properties*);
static void machinewireview_onsize(MachineWireView*, psy_ui_Component* sender,
	psy_ui_Size* size);
static psy_ui_Rectangle machinewireview_updaterect(MachineWireView* self, uintptr_t slot);
static void machineview_updatetext(MachineView*, Workspace* workspace);
static void machineview_onsongchanged(MachineView*, Workspace*, int flag, psy_audio_SongFile*);
static void machineview_onmousedown(MachineView*,
	psy_ui_Component* sender, psy_ui_MouseEvent*);
static void machineview_onmousedoubleclick(MachineView*,
	psy_ui_Component* sender, psy_ui_MouseEvent*);
static void machineview_onkeydown(MachineView*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void machineview_onfocus(MachineView*, psy_ui_Component* sender);
static void machineview_onskinchanged(MachineView*, Workspace*);
static void machineview_onlanguagechanged(MachineView*, Workspace* workspace);
static void selectsection(MachineView*, psy_ui_Component* sender, uintptr_t section);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(MachineWireView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_ondraw) machinewireview_ondraw;
		vtable.ontimer = (psy_ui_fp_ontimer) machinewireview_ontimer;
		vtable.onmousedown = (psy_ui_fp_onmousedown)
			machinewireview_onmousedown;
		vtable.onmouseup = (psy_ui_fp_onmouseup) machinewireview_onmouseup;
		vtable.onmousemove = (psy_ui_fp_onmousemove)
			machinewireview_onmousemove;
		vtable.onmousedoubleclick = (psy_ui_fp_onmousedoubleclick)
			machinewireview_onmousedoubleclick;
		vtable.onkeydown = (psy_ui_fp_onkeydown)
			machinewireview_onkeydown;
	}
}

void machineui_init(MachineUi* self, int x, int y, psy_audio_Machine* machine,
	uintptr_t slot, MachineSkin* skin, Workspace* workspace)
{	
	assert(machine);
	self->machine = machine;
	self->coords = 0;
	self->workspace = workspace;
	self->x = x;
	self->y = y;
	self->skin = skin;	
	self->mode = psy_audio_machine_mode(machine);	
	machineui_updatecoords(self);	
	self->volumedisplay = 0.f;
	self->volumemaxdisplay = 0.f;	
	self->volumemaxcounterlife = 0;	
	self->slot = slot;
	self->frame = 0;
	self->paramview = 0;
	self->vst2view = 0;
	self->restorename = 0;
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

psy_ui_Size machineui_size(MachineUi* self)
{	
	psy_ui_Size rv;

	if (self->coords) {
		rv.width = self->coords->background.destwidth;
		rv.height = self->coords->background.destheight;
	} else {
		rv.width = 200;
		rv.height = 20;		
	}
	return rv;
}

psy_ui_Rectangle machineui_position(MachineUi* self)
{
	psy_ui_Rectangle rv;

	if (self->coords) {
		psy_ui_setrectangle(&rv, self->x, self->y,
			self->coords->background.destwidth,
			self->coords->background.destheight);
	} else {
		psy_ui_setrectangle(&rv, self->x, self->y, 200, 20);
	}
	return rv;	
}

void machineui_editname(MachineUi* self, psy_ui_Edit* edit)
{
	if (self->machine) {
		psy_ui_Rectangle r;
		
		free(self->restorename);
		self->restorename = 
			psy_audio_machine_editname(self->machine)
				? strdup(psy_audio_machine_editname(self->machine)) : 0;
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
		r = machineui_position(self);
		r.left += self->coords->name.destx;
		r.top += self->coords->name.desty;
		r.right = r.left + self->coords->name.destwidth;
		r.bottom = r.top + self->coords->name.destheight;
		psy_ui_component_setposition(&edit->component, r.left, 
			r.top, r.right - r.left, r.bottom - r.top);
		psy_ui_component_show(&edit->component);
	}
}

void machineui_onkeydown(MachineUi* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{	
	if (ev->keycode == psy_ui_KEY_RETURN) {		
		psy_ui_component_hide(sender);
	} else
	if (ev->keycode == psy_ui_KEY_ESCAPE) {
		if (self->machine) {
			psy_audio_machine_seteditname(self->machine, self->restorename);
			free(self->restorename);
			self->restorename = 0;
		}
		psy_ui_component_hide(sender);
	}
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
	r.left += wireview->dx;
	r.right += wireview->dx;
	r.top += wireview->dy;
	r.bottom += wireview->dy;
	if (psy_ui_rectangle_intersect_rectangle(&g->clip, &r) &&  self->coords) {
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

			psy_ui_setrectangle(&clip, r.left + coords->name.destx,
				r.top + coords->name.desty, coords->name.destwidth,
				coords->name.destheight);
			psy_ui_textoutrectangle(g, r.left + coords->name.destx,
				r.top + coords->name.desty,
				psy_ui_ETO_CLIPPED, clip,
				editname, strlen(editname));
			skin_blitpart(g, &wireview->skin.skinbmp,
				r.left + slidercoord(&coords->pan, 
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

void machineui_drawvu(MachineUi* self, psy_ui_Graphics* g,  MachineWireView* wireview)
{				
	if (self->coords && self->mode != MACHMODE_MASTER) {
		machineui_drawvudisplay(self, g, wireview);
		machineui_drawvupeak(self, g, wireview);
	}
}

void machineui_drawvudisplay(MachineUi* self, psy_ui_Graphics* g, MachineWireView* wireview)
{
	SkinCoord* vu;		

	vu = &self->coords->vu0;
	psy_ui_drawbitmap(g, &wireview->skin.skinbmp,
		wireview->dx + self->x + vu->destx,
		wireview->dy + self->y + vu->desty,
		(int)(self->volumedisplay * vu->destwidth),
		vu->destheight,
		vu->srcx,
		vu->srcy);		
}

void machineui_drawvupeak(MachineUi* self, psy_ui_Graphics* g,  MachineWireView* wireview)
{
	SkinCoord* vupeak;
	SkinCoord* vu;

	vupeak = &self->coords->vupeak;
	vu = &self->coords->vu0;
	if (self->volumemaxdisplay > 0.01f) {
		psy_ui_drawbitmap(g, &wireview->skin.skinbmp,
			wireview->dx + self->x + vupeak->destx +
				(int)(self->volumemaxdisplay * vu->destwidth),
			wireview->dy + self->y + vupeak->desty,
			vupeak->destwidth, vupeak->destheight,
			vupeak->srcx, vupeak->srcy);
	}
}

void machineui_updatevolumedisplay(MachineUi* self)
{
	if (self->machine) {
		psy_audio_Buffer* memory;

		memory = psy_audio_machine_buffermemory(self->machine);
		if (memory) {
			self->volumedisplay = psy_audio_buffer_rmsdisplay(memory);
		} else {
			self->volumedisplay = (psy_dsp_amp_t)0.f;
		}
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
			psy_ui_Component* view = 0;

			self->frame = machineframe_alloc();
			machineframe_init(self->frame, parent, workspace_showparamviewaswindow(self->workspace),
				self->workspace);
			psy_signal_connect(&self->frame->component.signal_destroy, self,
				machineui_onframedestroyed);
			if (psy_audio_machine_haseditor(self->machine)) {
				Vst2View* vst2view;

				vst2view = vst2view_allocinit(&self->frame->notebook.component,
					self->machine, self->workspace);
				if (vst2view) {
					view = &vst2view->component;
				}
			} else {
				ParamView* paramview;

				paramview = paramview_allocinit(&self->frame->notebook.component,
					self->machine, self->workspace);
				if (paramview) {
					view = &paramview->component;
				}
			}
			if (view) {				
				machineframe_setview(self->frame, view, self->machine);
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

void machinewireview_init(MachineWireView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	psy_ui_FontInfo fontinfo;

	self->firstsize = 0;
	self->dx = 0;
	self->dy = 0;
	self->statusbar = 0;
	self->workspace = workspace;
	self->machines = &workspace->song->machines;
	self->drawvumeters = 1;
	self->wireframes = 0;
	self->randominsert = 1;
	self->addeffect = 0;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	// skin init
	psy_ui_bitmap_init(&self->skin.skinbmp);
	psy_ui_bitmap_loadresource(&self->skin.skinbmp, IDB_MACHINESKIN);
	psy_ui_fontinfo_init(&fontinfo, "Tahoma", 16);
	psy_ui_font_init(&self->skin.font, &fontinfo);
	machinewireview_initmachinecoords(self);	
	psy_table_init(&self->machineuis);			
	machinewireview_initmasterui(self);	
	psy_ui_component_setfont(&self->component, &self->skin.font);
	vtable_init(self);
	self->component.vtable = &vtable;
	machinewireview_connectuisignals(self);	
	self->dragslot = UINTPTR_MAX;
	self->dragmode = MACHINEWIREVIEW_DRAG_MACHINE;
	self->selectedslot = MASTER_INDEX;
	self->selectedwire.src = UINTPTR_MAX;
	self->selectedwire.dst = UINTPTR_MAX;	

	psy_signal_connect(&workspace->signal_songchanged, self,
		machinewireview_onsongchanged);	
	machinewireview_connectmachinessignals(self);
	psy_signal_connect(&workspace->signal_configchanged, self,
		machinewireview_onconfigchanged);
	psy_signal_connect(&workspace->signal_beforesavesong, self,
		machinewireview_beforesavesong);
	psy_signal_connect(&workspace->signal_showparameters, self,
		machinewireview_onshowparameters);
	psy_signal_connect(&self->component.signal_scroll, self,
		machinewireview_onscroll);
		psy_signal_connect(&self->component.signal_size, self,
		machinewireview_onsize);
	machinewireview_adjustscroll(self);
	psy_ui_edit_init(&self->editname, &self->component);
	psy_ui_component_hide(&self->editname.component);
	psy_ui_component_starttimer(&self->component, TIMERID_UPDATEVUMETERS, 50);
	if (workspace->machineviewtheme) {
		machinewireview_applyproperties(self, workspace->machineviewtheme);
	}
	self->firstsize = 1;	
}

void machinewireview_adjustscroll(MachineWireView* self)
{
	psy_ui_Rectangle bounds;
	psy_ui_Size size;	
	int maxstepx;
	int maxstepy;
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	bounds = machinewireview_bounds(self);
	size = psy_ui_component_size(&self->component);		
	self->component.scrollstepx = tm.tmAveCharWidth;
	self->component.scrollstepy = tm.tmHeight / 2;
	maxstepx = (int)((bounds.right - size.width + tm.tmAveCharWidth * 2)
		/ (float)self->component.scrollstepx + 0.5f);
	if (maxstepx <= 0) {
		maxstepx = 0;		
	}
	maxstepy = (int)((bounds.bottom - size.height + tm.tmHeight * 2) 
		/ (float)self->component.scrollstepy + 0.5f);
	if (maxstepy <= 0) {
		maxstepy = 0;		
	}
	if (-self->dx > self->component.scrollstepx * maxstepx) {
		self->dx = -self->component.scrollstepx * maxstepx;
	}
	if (-self->dy > self->component.scrollstepy * maxstepy) {
		self->dy = -self->component.scrollstepy * maxstepy;
	}
	psy_ui_component_sethorizontalscrollrange(&self->component, 0, maxstepx);
	psy_ui_component_setverticalscrollrange(&self->component, 0, maxstepy);		
}

void machinewireview_onscroll(MachineWireView* self, psy_ui_Component* sender,
	int stepx,	int stepy)
{
	self->dx += self->component.scrollstepx * stepx;
	self->dy += self->component.scrollstepy * stepy;
}

void machinewireview_connectuisignals(MachineWireView* self)
{
	psy_signal_connect(&self->component.signal_destroy, self,
		machinewireview_ondestroy);
}

void machinewireview_initmasterui(MachineWireView* self)
{	
	machineuis_insert(self, MASTER_INDEX, 0, 0, &self->skin);
}

void machinewireview_connectmachinessignals(MachineWireView* self)
{	
	psy_signal_connect(&self->machines->signal_slotchange, self,
		machinewireview_onmachineschangeslot);
	psy_signal_connect(&self->machines->signal_insert, self,
		machinewireview_onmachinesinsert);
	psy_signal_connect(&self->machines->signal_removed, self,
		machinewireview_onmachinesremoved);	
}

void machinewireview_ondestroy(MachineWireView* self, psy_ui_Component* component)
{
	psy_List* wireframenode;
	machineuis_removeall(self);
	psy_table_dispose(&self->machineuis);
	for (wireframenode = self->wireframes; wireframenode != 0;
			wireframenode = wireframenode->next) {
		WireFrame* frame;
		frame = (WireFrame*) wireframenode->entry;
		if (frame->wireview) {
			psy_ui_component_destroy(&frame->wireview->component);
			free(frame->wireview);
		}
		psy_ui_component_destroy(&frame->component);
		free(frame);
	}
	psy_list_free(self->wireframes);
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
		psy_ui_component_stoptimer(&self->component, TIMERID_UPDATEVUMETERS);
		if (self->drawvumeters) {
			psy_ui_component_starttimer(&self->component, TIMERID_UPDATEVUMETERS,
				50);
		}
		self->skin.drawmachineindexes = psy_properties_bool(mv,
			"drawmachineindexes", 1);
	}
}

void machinewireview_applyproperties(MachineWireView* self, psy_Properties* p)
{
	const char* machine_skin_name;

	self->skin.drawmachineindexes = workspace_showmachineindexes(self->workspace);
	self->skin.colour = psy_properties_int(p, "mv_colour", 0x00232323);
	self->skin.wirecolour = psy_properties_int(p, "mv_wirecolour", 0x005F5F5F);
	self->skin.selwirecolour = psy_properties_int(p, "mv_wirecolour", 0x007F7F7F);
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

void skincoord_setsource(SkinCoord* coord, int vals[4])
{
	coord->srcx = vals[0];
	coord->srcy = vals[1];
	coord->srcwidth = vals[2];
	coord->srcheight = vals[3];
	coord->destwidth = vals[2];
	coord->destheight = vals[3];
}

void skincoord_setdest(SkinCoord* coord, int vals[4])
{
	coord->destx = vals[0];
	coord->desty = vals[1];
	coord->range = vals[2];
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
					if (self->selectedwire.src == slot &&
							self->selectedwire.dst == entry->slot) {
						psy_ui_setcolor(g, self->skin.selwirecolour);
					} else {
						psy_ui_setcolor(g, self->skin.wirecolour);
					}
					psy_ui_drawline(g, 
						self->dx + out.left + (out.right - out.left) / 2,
						self->dy + out.top + (out.bottom - out.top) / 2,
						self->dx + in.left + (in.right - in.left) / 2,
						self->dy + in.top + (in.bottom - in.top) / 2);
					machinewireview_drawwirearrow(self, g, outmachineui,
						inmachineui);
				}
			}		
		}
	}
}

void machinewireview_drawwirearrow(MachineWireView* self, psy_ui_Graphics* g,
	MachineUi* outmachineui, MachineUi* inmachineui)
{
	psy_ui_Size out;
	psy_ui_Size in;	
	int x1,	y1;
	int x2, y2;		
	double phi;	
	psy_ui_Point center;
	psy_ui_Point a, b, c;	
	psy_ui_Point tri[4];
	int polysize;
	float deltaColR = ((self->skin.polycolour     & 0xFF) / 510.0f) + .45f;
	float deltaColG = ((self->skin.polycolour>>8  & 0xFF) / 510.0f) + .45f;
	float deltaColB = ((self->skin.polycolour>>16 & 0xFF) / 510.0f) + .45f;
	unsigned int polyInnards = RGB(192 * deltaColR, 192 * deltaColG,
		192 * deltaColB);

	out = machineui_size(outmachineui);
	in = machineui_size(inmachineui);
	x1 = self->dx + outmachineui->x + out.width/2;
	y1 = self->dy + outmachineui->y + out.height /2;
	x2 = self->dx + inmachineui->x + in.width/2;
	y2 = self->dy + inmachineui->y + in.height/2;

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

psy_ui_Point rotate_point(psy_ui_Point pt, double phi)
{
	psy_ui_Point rv;
	
	rv.x = (int) (cos(phi) * pt.x - sin(phi) * pt.y);
	rv.y = (int) (sin(phi) * pt.x + cos(phi) * pt.y);
	return rv;
}

psy_ui_Point move_point(psy_ui_Point pt, psy_ui_Point d)
{
	psy_ui_Point rv;
	
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
			psy_ui_Size machinesize;

			machinesize = machineui_size(machineui);
			psy_ui_setcolor(g, self->skin.wirecolour);
			psy_ui_drawline(g, 
				self->dx + machineui->x + machinesize.width/2,
				self->dy + machineui->y + machinesize.height/2,
				self->dx + self->mx,
				self->dy + self->my);		
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

int slidercoord(SkinCoord* coord, float value)
{	
	return (int)(value * coord->range);
}

void machineui_drawhighlight(MachineUi* self, psy_ui_Graphics* g,
	MachineWireView* wireview)
{	
	psy_ui_Rectangle r;
	static int d = 5; // the distance of the highlight from the machine

	r = machineui_position(self);
	r.left += wireview->dx;
	r.right += wireview->dx;
	r.top += wireview->dy;
	r.bottom += wireview->dy;
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
	if (self->firstsize && size->width > 0) {
		self->firstsize = 0;
		machinewireview_align(self);
	}
	machinewireview_adjustscroll(self);	
}

void machineview_align(MachineView* self)
{
	machinewireview_align(&self->wireview);
}

void machinewireview_align(MachineWireView* self)
{
	MachineUi* machineui;
	psy_ui_Size machinesize;
	psy_ui_Size size = psy_ui_component_size(&self->component);

	machineui = machineuis_at(self, MASTER_INDEX);
	if (machineui) {
		machinesize = machineui_size(machineui);
		machineui->x = (size.width - machinesize.width) / 2 ;
		machineui->y = (size.height - machinesize.height) / 2;
	}
}

void machinewireview_onmousedoubleclick(MachineWireView* self, psy_ui_MouseEvent* ev)
{
	self->mx = ev->x - self->dx;
	self->my = ev->y - self->dy;
	machinewireview_hittest(self, ev->x - self->dx, ev->y - self->dy);
	if (self->dragslot == UINTPTR_MAX) {
		self->selectedwire = machinewireview_hittestwire(self, ev->x - self->dx,
			ev->y - self->dy);
		if (self->selectedwire.dst != UINTPTR_MAX) {			
			machinewireview_showwireview(self, self->selectedwire);
			psy_ui_component_invalidate(&self->component);
			psy_ui_mouseevent_stoppropagation(ev);
		} else {
			self->randominsert = 0;
		}
	} else		 
	if (machinewireview_hittesteditname(self, ev->x - self->dx,
			ev->y - self->dy, self->dragslot)) {
		if (machineuis_at(self, self->dragslot)) {
			machineui_editname(machineuis_at(self, self->dragslot),
				&self->editname);
			psy_ui_mouseevent_stoppropagation(ev);
		}
	} else
	if (machinewireview_hittestcoord(self, ev->x - self->dx,
			ev->y - self->dy, MACHMODE_GENERATOR, self->dragslot,
			&self->skin.generator.solo) ||				
	    machinewireview_hittestcoord(self, ev->x - self->dx,
			ev->y - self->dy, MACHMODE_FX, self->dragslot,
			&self->skin.effect.bypass) ||	
	    machinewireview_hittestcoord(self, ev->x - self->dx,
			ev->y - self->dy, MACHMODE_GENERATOR, self->dragslot,
			&self->skin.generator.mute) ||
		machinewireview_hittestcoord(self, ev->x - self->dx,
			ev->y - self->dy, MACHMODE_FX, self->dragslot,
			&self->skin.effect.mute) ||						
	    machinewireview_hittestpan(self, ev->x - self->dx,
					ev->y - self->dy, self->dragslot, &self->mx)) {
			psy_ui_mouseevent_stoppropagation(ev);
	} else {
		workspace_showparameters(self->workspace, self->dragslot);
		psy_ui_mouseevent_stoppropagation(ev);
	}		
	self->dragslot = UINTPTR_MAX;	
}

void machinewireview_onmousedown(MachineWireView* self, psy_ui_MouseEvent* ev)
{
	self->mousemoved = FALSE;
	psy_ui_component_hide(&self->editname.component);
	psy_ui_component_setfocus(&self->component);
	self->mx = ev->x - self->dx;
	self->my = ev->y - self->dy;
	self->dragmode = MACHINEWIREVIEW_DRAG_NONE;
	machinewireview_hittest(self, ev->x - self->dx, ev->y - self->dy);
	if (self->dragslot == UINTPTR_MAX) {
		if (ev->button == 1) {
			self->selectedwire = machinewireview_hittestwire(self,
				ev->x - self->dx, ev->y - self->dy);
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
			if (self->dragslot != MASTER_INDEX) {
				self->selectedslot = self->dragslot;
				self->selectedwire.src = UINTPTR_MAX;
				self->selectedwire.dst = UINTPTR_MAX;
				machines_changeslot(self->machines, self->selectedslot);
			}			
			if (machinewireview_hittestcoord(self, ev->x - self->dx,
					ev->y - self->dy, MACHMODE_GENERATOR, self->dragslot,
					&self->skin.generator.solo)) {
				psy_audio_Machine* machine = machines_at(self->machines,
					self->dragslot);
				if (machine) {
					psy_audio_machines_solo(self->machines, self->dragslot);
					psy_ui_component_invalidate(&self->component);
				}
				self->dragslot = UINTPTR_MAX;
			} else
			if (machinewireview_hittestcoord(self, ev->x - self->dx,
					ev->y - self->dy, MACHMODE_FX, self->dragslot,
					&self->skin.effect.bypass)) {
				psy_audio_Machine* machine = machines_at(self->machines,
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
			if (machinewireview_hittestcoord(self, ev->x - self->dx,
					ev->y - self->dy, MACHMODE_GENERATOR, self->dragslot,
					&self->skin.generator.mute) ||
				machinewireview_hittestcoord(self, ev->x - self->dx,
					ev->y - self->dy, MACHMODE_FX, self->dragslot,
					&self->skin.effect.mute)) {
				psy_audio_Machine* machine = machines_at(self->machines,
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
			if (machinewireview_hittestpan(self, ev->x - self->dx,
					ev->y - self->dy, self->dragslot, &self->mx)) {
				self->dragmode = MACHINEWIREVIEW_DRAG_PAN;				
			} else  {
				MachineUi* machineui;

				machineui = machineuis_at(self, self->dragslot);
				if (machineui) {
					self->dragmode = MACHINEWIREVIEW_DRAG_MACHINE;				
					self->mx = ev->x - machineui->x - self->dx;
					self->my = ev->y - machineui->y - self->dy;
					psy_ui_component_capture(&self->component);
				}
			}
			
		} else
		if (ev->button == 2) {
			psy_audio_Machine* machine;

			machine = machines_at(self->machines, self->dragslot);
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
	int rv = 0;
	psy_audio_Machine* machine;
	psy_ui_Rectangle r;
	int offset;
	MachineCoords* coords;
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		int xm = x - machineui->x;	
		int ym = y - machineui->y;
		machine = machines_at(self->machines, slot);
		coords = machineui->coords;
		if (coords) {
			offset = (int) (psy_audio_machine_panning(machine) *
				coords->pan.range);
			psy_ui_setrectangle(&r,
				coords->pan.destx + offset,
				coords->pan.desty,
				coords->pan.destwidth,
				coords->pan.destheight);
			*dx = xm - r.left;
			rv = psy_ui_rectangle_intersect(&r, xm, ym);
		}
	}
	return rv;
}

int machinewireview_hittesteditname(MachineWireView* self, int x, int y,
	uintptr_t slot)
{
	int rv = 0;
	psy_audio_Machine* machine;
	psy_ui_Rectangle r;	
	MachineCoords* coords;
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		int xm = x - machineui->x;
		int ym = y - machineui->y;
		machine = machines_at(self->machines, slot);
		coords = machineui->coords;
		if (coords) {			
			psy_ui_setrectangle(&r, coords->name.destx, coords->name.desty,
				coords->name.destwidth, coords->name.destheight);			
			rv = psy_ui_rectangle_intersect(&r, xm, ym);
		}
	}
	return rv;
}

int machinewireview_hittestcoord(MachineWireView* self, int x, int y, int mode,
	uintptr_t slot, SkinCoord* coord)
{
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui && machineui->mode == mode) {
		psy_ui_Rectangle r = { 0, 0, 0, 0 };	
		int xm = x - machineui->x;	
		int ym = y - machineui->y;
			
		psy_ui_setrectangle(&r, coord->destx, coord->desty,
			coord->destwidth, coord->destheight);
		return psy_ui_rectangle_intersect(&r, xm, ym);
	}
	return 0;
}

psy_dsp_amp_t machinewireview_panvalue(MachineWireView* self, int x, int y,
	uintptr_t slot)
{
	psy_dsp_amp_t rv = 0.f;	
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		MachineCoords* coords;

		coords = machineui->coords;
		if (coords && coords->pan.range != 0) {
			rv =  (x - machineui->x - coords->pan.destx -
				self->mx) / (float)coords->pan.range;		
		}		
	}
	return rv;
}

void machinewireview_hittest(MachineWireView* self, int x, int y)
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
				psy_audio_machine_setpanning(machineui->machine, machinewireview_panvalue(
					self, ev->x - self->dx, ev->y - self->dy, self->dragslot));
				machineui_invalidate(machineui, self);
			}
		} else
		if (self->dragmode == MACHINEWIREVIEW_DRAG_MACHINE) {
			MachineUi* machineui;
			psy_ui_Rectangle r_old;
			psy_ui_Rectangle r_new;

			machineui = machineuis_at(self, self->dragslot);
			if (machineui) {
				r_old = machineui_position(machineui);
				psy_ui_rectangle_expand(&r_old, 5, 5, 5, 5);
				machineui->x = max(0, ev->x - self->mx - self->dx);
				machineui->y = max(0, ev->y - self->my - self->dy);
				if (self->statusbar && machineui->machine) {
					static char txt[128];
					psy_snprintf(txt, 128, "%s (%d, %d)",
						psy_audio_machine_editname(machineui->machine)
						? psy_audio_machine_editname(machineui->machine)
						: "",
						machineui->x,
						machineui->y);
					machineviewbar_settext(self->statusbar, txt);
				}
				r_new = machinewireview_updaterect(self, self->dragslot);
				psy_ui_rectangle_union(&r_new, &r_old);
				psy_ui_rectangle_expand(&r_new, 5, 5, 5, 5);
				psy_ui_rectangle_move(&r_new, self->dx, self->dy);
				psy_ui_component_invalidaterect(&self->component, &r_new);
			} else {
				self->dragmode = MACHINEWIREVIEW_DRAG_NONE;
			}
		} else
		if (self->dragmode >= MACHINEWIREVIEW_DRAG_NEWCONNECTION &&
				self->dragmode <= MACHINEWIREVIEW_DRAG_RIGHTCONNECTION) {			
			self->mx = ev->x - self->dx;
			self->my = ev->y - self->dy;
			psy_ui_component_invalidate(&self->component);			
		}		
	}
}

void machinewireview_onmouseup(MachineWireView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_releasecapture(&self->component);
	if (self->dragslot != UINTPTR_MAX) {
		if (self->dragmode == MACHINEWIREVIEW_DRAG_MACHINE) {
			machinewireview_adjustscroll(self);			
		} else
		if (self->dragmode >= MACHINEWIREVIEW_DRAG_NEWCONNECTION &&
				self->dragmode <= MACHINEWIREVIEW_DRAG_RIGHTCONNECTION) {
			if (self->mousemoved) {
				uintptr_t slot = self->dragslot;
				self->dragslot = UINTPTR_MAX;
				machinewireview_hittest(self, ev->x - self->dx,
					ev->y - self->dy);
				if (self->dragslot != UINTPTR_MAX) {
					if (self->dragmode != MACHINEWIREVIEW_DRAG_NEWCONNECTION) {
						machines_disconnect(self->machines, self->selectedwire.src,
							self->selectedwire.dst);
					}
					if (self->dragmode < MACHINEWIREVIEW_DRAG_RIGHTCONNECTION) {
						machines_connect(self->machines, slot, self->dragslot);
					} else {
						machines_connect(self->machines, self->dragslot, slot);
					}
				}
			} else
			if (ev->button == 2) {
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
			self->selectedwire.src != UINTPTR_MAX) {
		machines_disconnect(self->machines, self->selectedwire.src, 
			self->selectedwire.dst);
		psy_ui_component_invalidate(&self->component);
	} else 
	if (ev->keycode == psy_ui_KEY_DELETE && self->selectedslot != - 1 &&
			self->selectedslot != MASTER_INDEX) {		
		machines_remove(self->machines, self->selectedslot);
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
	
	rv.dst = UINTPTR_MAX;
	rv.src = UINTPTR_MAX;
	for (it = machines_begin(self->machines); it.curr != 0; 
			psy_tableiterator_inc(&it)) {
		psy_audio_MachineSockets* sockets;
		WireSocket* p;			
		uintptr_t slot = it.curr->key;
		int done = 0;
	
		sockets	= connections_at(&self->machines->connections, slot);
		if (sockets) {
			p = sockets->outputs;	
			while (p != NULL) {
				psy_audio_WireSocketEntry* entry =
					(psy_audio_WireSocketEntry*) p->entry;
				if (entry->slot != UINTPTR_MAX) {
					psy_ui_Size out;
					psy_ui_Size in;								
					int x1, x2, y1, y2;
					float m;
					int b;
					int y3;				
					MachineUi* inmachineui;
					MachineUi* outmachineui;

					inmachineui = machineuis_at(self, entry->slot);
					outmachineui = machineuis_at(self, slot);
					if (inmachineui && outmachineui) {
						out = machineui_size(outmachineui);
						in = machineui_size(inmachineui);

						x1 = outmachineui->x + out.width / 2;
						y1 = outmachineui->y + out.height / 2;
						x2 = inmachineui->x + in.width / 2;
						y2 = inmachineui->y + in.height / 2;							
						if (x2 - x1 != 0) {
							m = (y2 - y1) / (float)(x2 - x1);
							b = y1 - (int) (m * x1);
							y3 = (int)(m * x) + b;
						} else {											
							if (abs(x - x1) < 10 &&
								y >= y1 && y <= y2) {						
								rv.src = slot;
								rv.dst = entry->slot;												
							} 
							done = 1;
							break;
						}
						if (abs(y - y3) < 10) {					
							rv.src = slot;
							rv.dst = entry->slot;
							done = 1;
							break;
						}
					}
				}
				if (done) {
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
	self->selectedwire.src = UINTPTR_MAX;
	self->selectedwire.dst = UINTPTR_MAX;
	psy_ui_component_invalidate(&self->component);
	psy_ui_component_setfocus(&self->component);
}

void machinewireview_onmachinesinsert(MachineWireView* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = machines_at(self->machines, slot);
	if (machine) {
		MachineUi* machineui;

		machineui = machineuis_insert(self, slot, 0, 0, &self->skin);
		if (machineui && !self->randominsert) {
			int width;
			int height;

			width = machineui_position(machineui).right -
				machineui_position(machineui).left;
			height = machineui_position(machineui).bottom -
				machineui_position(machineui).top;
			machineui->x = max(0, self->mx - width / 2);
			machineui->y = max(0, self->my - height / 2);
		}
		psy_signal_connect(&machine->signal_worked, self, 
			machinewireview_onmachineworked);
		psy_ui_component_invalidate(&self->component);
		self->randominsert = 1;
	}
}

void machinewireview_onmachinesremoved(MachineWireView* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	machineuis_remove(self, slot);
	machinewireview_adjustscroll(self);
	psy_ui_component_invalidate(&self->component);
}

void machinewireview_beforesavesong(MachineWireView* self,
	Workspace* workspace, psy_audio_SongFile* songfile)
{
	if (songfile) {
		psy_TableIterator it;

		for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			MachineUi* machineui;
			psy_audio_MachineUi* songio_machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			songio_machineui = psy_audio_songfile_machineui(songfile, psy_tableiterator_key(&it));
			songio_machineui->x = machineui->x;
			songio_machineui->y = machineui->y;
		}
	}
}

void machinewireview_buildmachineuis(MachineWireView* self,
	psy_Table* machineuis)
{
	machineuis_removeall(self);
	if (machineuis) {
		psy_TableIterator it;

		for (it = psy_table_begin(machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_MachineUi* machineui;
			psy_audio_Machine* machine;

			machineui = (psy_audio_MachineUi*)psy_tableiterator_value(&it);
			machine = machines_at(self->machines, psy_tableiterator_key(&it));
			if (machine) {
				machineuis_insert(self, psy_tableiterator_key(&it),
					machineui->x, machineui->y, &self->skin);
				if (psy_tableiterator_key(&it) != MASTER_INDEX) {
					psy_signal_connect(&machine->signal_worked, self,
						machinewireview_onmachineworked);
				}
			}
		}
	} else {
		machineuis_insert(self, MASTER_INDEX,
			640, 480, &self->skin);
		machinewireview_align(self);
	}
}

void machinewireview_onsongchanged(MachineWireView* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
{	
	self->machines = &workspace->song->machines;	
	machinewireview_buildmachineuis(self, songfile ? &songfile->machineuis : 0);	
	machinewireview_connectmachinessignals(self);
	self->dx = 0;
	self->dy = 0;
	psy_ui_component_sethorizontalscrollposition(&self->component, 0);
	psy_ui_component_setverticalscrollposition(&self->component, 0);
	machinewireview_adjustscroll(self);
	psy_ui_component_invalidate(&self->component);	
}

void machinewireview_onshowparameters(MachineWireView* self, Workspace* sender,
	uintptr_t slot)
{	
	if (machineuis_at(self, slot)) {
		machineui_showparameters(machineuis_at(self, slot), &self->component);
	}
}

void machinewireview_onmachineworked(MachineWireView* self,
	psy_audio_Machine* machine, uintptr_t slot, psy_audio_BufferContext* bc)
{
	if (slot != MASTER_INDEX) {
		MachineUi* machineui;

		machineui = machineuis_at(self, slot);
		if (machineui) {			
			machineui_updatevolumedisplay(machineui);
		}
	}
}

void machinewireview_ontimer(MachineWireView* self, int timerid)
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
				psy_audio_Machine* machine;
				psy_ui_Component* dockparent;

				frame = machineui->frame;
				psy_ui_component_init(&temp, &self->component);
				view = frame->view;
				dockparent = psy_ui_component_parent(&frame->component);
				machine = frame->machine;
				psy_ui_component_setparent(frame->view, &temp);
				psy_signal_disconnectall(&view->signal_preferredsizechanged);
				frame->view = 0;
				psy_ui_component_destroy(&frame->component);
				frame = machineframe_alloc();
				machineframe_init(frame, &self->component, TRUE, self->workspace);
				psy_ui_component_insert(&frame->notebook.component, view, &frame->help.component);
				machineframe_setview(frame, view, machine);
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
				psy_audio_Machine* machine;

				frame = machineui->frame;
				psy_ui_component_init(&temp, &self->component);
				view = frame->view;
				machine = frame->machine;
				psy_ui_component_setparent(frame->view, &temp);
				frame->view = 0;
				psy_signal_disconnectall(&view->signal_preferredsizechanged);
				psy_ui_component_destroy(&frame->component);
				frame = machineframe_alloc();
				machineframe_init(frame, &self->component, FALSE, self->workspace);
				psy_ui_component_insert(&frame->notebook.component, view, &frame->help.component);
				machineframe_setview(frame, view, machine);
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
	machinewireview_preparedrawallmacvus(self);
}

void machinewireview_preparedrawallmacvus(MachineWireView* self)
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
	psy_ui_Rectangle r;	
	
	r = machineui_position(self);
	r.left += wireview->dx;
	r.right += wireview->dx;
	r.top += wireview->dy;
	r.bottom += wireview->dy;
	psy_ui_component_invalidaterect(&wireview->component, &r);
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
			wireframe = 0;
		}
	}
	if (wireframe != 0) {
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
	const char* path;
		
	path = psy_properties_readstring(plugininfo, "path", "");
	machine = machinefactory_makemachinefrompath(
		&self->workspace->machinefactory, 
		psy_properties_int(plugininfo, "type", UINTPTR_MAX),
		path,
		psy_properties_int(plugininfo, "shellidx", 0));
	if (machine) {		
		if (self->wireview.addeffect) {
			uintptr_t slot;

			slot = machines_append(self->wireview.machines, machine);		
			machines_disconnect(self->wireview.machines, self->wireview.selectedwire.src, self->wireview.selectedwire.dst);
			machines_connect(self->wireview.machines, self->wireview.selectedwire.src, slot);
			machines_connect(self->wireview.machines, slot, self->wireview.selectedwire.dst);
			machines_changeslot(self->wireview.machines, slot);
			self->wireview.addeffect = 0;
		} else
		if (self->newmachine.pluginsview.calledby == 10) {			
			machines_insert(self->wireview.machines,
				machines_slot(self->wireview.machines), machine);
		} else {			
			machines_changeslot(self->wireview.machines,
				machines_append(self->wireview.machines, machine));			
		}
		tabbar_select(&self->tabbar, 0);
	}	
}

MachineUi* machineuis_insert(MachineWireView* self, uintptr_t slot, int x, int y,
	MachineSkin* skin)
{	
	MachineUi* rv = 0;
	psy_audio_Machine* machine;

	machine = machines_at(self->machines, slot);
	if (machine) {
		if (psy_table_exists(&self->machineuis, slot)) {
			machineuis_remove(self, slot);
		}	
		rv = (MachineUi*) malloc(sizeof(MachineUi));
		machineui_init(rv, x, y, machine, slot, &self->skin,
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
	psy_TableIterator it;

	for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
		MachineUi* machineui;

		machineui = (MachineUi*)psy_tableiterator_value(&it);
		machineui_dispose(machineui);
		free(machineui);
	}
	psy_table_dispose(&self->machineuis);
	psy_table_init(&self->machineuis);
}

static psy_ui_ComponentVtable machineviewbar_vtable;
static int machineviewbar_vtable_initialized = 0;

static void machineviewbar_drawstatus(MachineViewBar*, psy_ui_Component* sender,
	psy_ui_Graphics*);
static void machineviewbar_onmixerconnectmodeclick(MachineViewBar*, psy_ui_Component* sender);

static void machineviewbar_vtable_init(MachineViewBar* self)
{
	if (!machineviewbar_vtable_initialized) {
		machineviewbar_vtable = *(self->component.vtable);
		machineviewbar_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			machineviewbar_onpreferredsize;
		machineviewbar_vtable_initialized = 1;
	}
}

void machineviewbar_init(MachineViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	psy_ui_component_init(&self->component, parent);
	machineviewbar_vtable_init(self);
	self->component.vtable = &machineviewbar_vtable;;
	psy_ui_component_enablealign(&self->component);	
	psy_ui_checkbox_init(&self->mixersend, &self->component);
	psy_ui_checkbox_settext(&self->mixersend,
		"Connect to Mixer Send/Return Input");
	psy_ui_checkbox_check(&self->mixersend);
	psy_signal_connect(&self->mixersend.signal_clicked, self,
		machineviewbar_onmixerconnectmodeclick);
	psy_ui_component_setalign(&self->mixersend.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_init(&self->status, &self->component);
	psy_ui_component_setalign(&self->status, psy_ui_ALIGN_CLIENT);
	psy_ui_component_doublebuffer(&self->status);
	psy_signal_connect(&self->status.signal_draw, self,
		machineviewbar_drawstatus);
	self->text = strdup("");
	psy_signal_connect(&workspace->signal_songchanged, self,
		machineviewbar_onsongchanged);
	self->workspace = workspace;
}

void machineviewbar_drawstatus(MachineViewBar* self, psy_ui_Component* sender,
	psy_ui_Graphics* g)
{
	psy_ui_Size size;
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size(&self->component);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolor(g, 0x00D1C5B6);
	psy_ui_textout(g, tm.tmAveCharWidth * 4, (size.height - tm.tmHeight) / 2, self->text, strlen(self->text));
}

void machineviewbar_ondestroy(MachineViewBar* self, psy_ui_Component* sender)
{
	free(self->text);
}

void machineviewbar_settext(MachineViewBar* self, const char* text)
{
	free(self->text);
	self->text = strdup(text);
	psy_ui_component_invalidate(&self->status);
}

void machineviewbar_onpreferredsize(MachineViewBar* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	rv->width = tm.tmAveCharWidth * 44 +
		psy_ui_component_preferredsize(&self->mixersend.component, rv).width;
	rv->height = (int)(tm.tmHeight);
}

void machineviewbar_onmixerconnectmodeclick(MachineViewBar* self, psy_ui_Component* sender)
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
			machines_isconnectasmixersend(&self->workspace->song->machines)) {
		psy_ui_checkbox_check(&self->mixersend);
	} else {
		psy_ui_checkbox_disablecheck(&self->mixersend);		
	}
}

void machineview_init(MachineView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setposition(&self->component, 0, 0, 0, 0);
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
	psy_signal_connect(&self->component.signal_mousedoubleclick, self,
		machineview_onmousedoubleclick);
	psy_signal_connect(&self->component.signal_mousedown, self,
		machineview_onmousedown);
	psy_signal_connect(&self->component.signal_keydown, self,
		machineview_onkeydown);
	psy_signal_connect(&self->component.signal_focus, self,
		machineview_onfocus);
	psy_signal_connect(&self->workspace->signal_skinchanged, self,
		machineview_onskinchanged);	
	machineview_updatetext(self, workspace);
	psy_signal_connect(&workspace->signal_languagechanged, self,
		machineview_onlanguagechanged);
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

void machineview_onmousedoubleclick(MachineView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	self->newmachine.pluginsview.calledby = 0;	
	tabbar_select(&self->tabbar, 1);
}

void machineview_onmousedown(MachineView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (ev->button == 2) {
		if (tabbar_selected(&self->tabbar) == 1) {
			tabbar_select(&self->tabbar, 0);
		}
	}
	psy_ui_mouseevent_stoppropagation(ev);
}

void machineview_onkeydown(MachineView* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_ESCAPE) {
		if (tabbar_selected(&self->tabbar) == 1) {
			tabbar_select(&self->tabbar, 0);			
		}
		psy_ui_keyevent_stoppropagation(ev);
	}
}

void machineview_applyproperties(MachineView* self, psy_Properties* p)
{
	machinewireview_applyproperties(&self->wireview, p);
}

void machineview_onfocus(MachineView* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(&self->wireview.component);
}

void machineview_onskinchanged(MachineView* self, Workspace* sender)
{			
	machineview_applyproperties(self, sender->machineviewtheme);
}

psy_ui_Rectangle machinewireview_bounds(MachineWireView* self)
{
	psy_ui_Rectangle rv;
	psy_TableIterator it;

	psy_ui_setrectangle(&rv, 0, 0, 0, 0);		
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
	} else {
		psy_ui_setrectangle(&rv, 0, 0, 0, 0);		
	}	
	return rv;
}

void selectsection(MachineView* self, psy_ui_Component* sender, uintptr_t section)
{
	tabbar_select(&self->tabbar, (int) section);
}

void machineview_onsongchanged(MachineView* self, Workspace* workspace,
	int flag, psy_audio_SongFile* songfile)
{
	tabbar_select(&self->tabbar, 0);
}

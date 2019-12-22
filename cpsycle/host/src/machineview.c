// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineview.h"
#include "wireview.h"
#include "skingraphics.h"
#include "skinio.h"
#include "resources/resource.h"
#include <math.h>
#include <dir.h>
#include <portable.h>

#define TIMERID_UPDATEVUMETERS 300

static void machineuis_insert(MachineWireView*, uintptr_t slot, int x, int y, MachineSkin*);
static MachineUi* machineuis_at(MachineWireView*, uintptr_t slot);
static void machineuis_remove(MachineWireView*, uintptr_t slot);
static void machineuis_removeall(MachineWireView*);

static void machineui_init(MachineUi*, int x, int y, psy_audio_Machine* machine, uintptr_t slot, MachineSkin*, Workspace*);
static void machineui_updatecoords(MachineUi*);
static void machineui_dispose(MachineUi*);
static ui_size machineui_size(MachineUi*);
static ui_rectangle machineui_position(MachineUi*);
static void machineui_draw(MachineUi*, MachineWireView*, ui_graphics*, uintptr_t slot);
static void machineui_drawvu(MachineUi*, MachineWireView*, ui_graphics*);
static void machineui_drawhighlight(MachineUi*, MachineWireView*, ui_graphics*);
static void machineui_showparameters(MachineUi*, ui_component* parent);
static void machineui_editname(MachineUi*, ui_edit* edit);
static void machineui_onkeydown(MachineUi*, ui_component* sender, KeyEvent*);
static void machineui_oneditchange(MachineUi*, ui_edit* sender);
static void machineui_oneditfocuslost(MachineUi*, ui_component* sender);
static int slidercoord(SkinCoord*, float value);

static void machinewireview_adjustscroll(MachineWireView*);
static void machinewireview_onscroll(MachineWireView*, ui_component* sender,
	int stepx,	int stepy);
static void machinewireview_initmasterui(MachineWireView*);
static void machinewireview_connectuisignals(MachineWireView*);
static void machinewireview_connectmachinessignals(MachineWireView*);
static void machinewireview_ondraw(MachineWireView*, ui_component* sender, ui_graphics*);
static void machinewireview_drawmachines(MachineWireView*, ui_graphics*);
static void drawmachineline(ui_graphics* g, int xdir, int ydir, int x, int y);
static void machinewireview_drawdragwire(MachineWireView*, ui_graphics*);
static void machinewireview_drawwires(MachineWireView*, ui_graphics*);
static void machinewireview_drawwire(MachineWireView*, ui_graphics*, uintptr_t slot, MachineUi*);
static void machinewireview_drawwirearrow(MachineWireView*, ui_graphics*, MachineUi* out, MachineUi* in);
static ui_point rotate_point(ui_point, double phi);
static ui_point move_point(ui_point pt, ui_point d);
static void machinewireview_ondestroy(MachineWireView*, ui_component* component);
static void machinewireview_onmousedown(MachineWireView*, ui_component* sender, MouseEvent*);
static void machinewireview_onmouseup(MachineWireView*, ui_component* sender, MouseEvent*);
static void machinewireview_onmousemove(MachineWireView*, ui_component* sender, MouseEvent*);
static void machinewireview_onmousedoubleclick(MachineWireView*, ui_component* sender, MouseEvent*);
static void machinewireview_onkeydown(MachineWireView*, ui_component* sender, KeyEvent*);
static void machinewireview_onkeyup(MachineWireView*, ui_component* sender, KeyEvent*);
static void machinewireview_hittest(MachineWireView*, int x, int y);
static int machinewireview_hittestpan(MachineWireView*, int x, int y, uintptr_t slot, int* dx);
static int machinewireview_hittestcoord(MachineWireView*, int x, int y, int mode, uintptr_t slot, SkinCoord*);
static psy_audio_Wire machinewireview_hittestwire(MachineWireView*, int x, int y);
static int machinewireview_hittesteditname(MachineWireView*, int x, int y, uintptr_t slot);
static psy_dsp_amp_t machinewireview_panvalue(MachineWireView*, int x, int y, uintptr_t slot);
static void machinewireview_onnewmachineselected(MachineView*, ui_component* sender, psy_Properties*);
static void machinewireview_initmachinecoords(MachineWireView*);
static void machinewireview_onmachineschangeslot(MachineWireView*, psy_audio_Machines*, uintptr_t slot);
static void machinewireview_onmachinesinsert(MachineWireView*, psy_audio_Machines*, uintptr_t slot);
static void machinewireview_onmachinesremoved(MachineWireView*, psy_audio_Machines*, uintptr_t slot);
static void machinewireview_onsongchanged(MachineWireView*, Workspace*);
static void machinewireview_updatemachineuis(MachineWireView*, psy_Properties* );
static void machinewireview_applyproperties(MachineWireView*, psy_Properties*);
static void machinewireview_onshowparameters(MachineWireView*, Workspace*, uintptr_t slot);
static void machinewireview_onmachineworked(MachineWireView*, psy_audio_Machine*, uintptr_t slot,
	psy_audio_BufferContext*);
static void machinewireview_ontimer(MachineWireView*, ui_component* sender, int timerid);
static void machinewireview_preparedrawallmacvus(MachineWireView*);
static void machinewireview_onconfigchanged(MachineWireView*, Workspace*, psy_Properties*);
static void machinewireview_readconfig(MachineWireView*);
static void machinewireview_beforesavesong(MachineWireView*, Workspace*);
static void machinewireview_showwireview(MachineWireView*, psy_audio_Wire wire);
static WireFrame* machinewireview_wireframe(MachineWireView* self, psy_audio_Wire wire);
static void machinewireview_setcoords(MachineWireView*, psy_Properties*);

static void machineview_onshow(MachineView*, ui_component* sender);
static void machineview_onhide(MachineView*, ui_component* sender);
static void machineview_onmousedoubleclick(MachineView*, ui_component* sender, int x, int y, int button);
static void machineview_onkeydown(MachineView*, ui_component* sender, KeyEvent*);
static void machineview_onkeyup(MachineView*, ui_component* sender, KeyEvent*);
static void machineview_onfocus(MachineView*, ui_component* sender);
static void machineview_onskinchanged(MachineView*, Workspace*, psy_Properties*);

void machineui_init(MachineUi* self, int x, int y, psy_audio_Machine* machine,
	uintptr_t slot, MachineSkin* skin, Workspace* workspace)
{	
	assert(machine);
	self->workspace = workspace;
	self->x = x;
	self->y = y;
	self->skin = skin;	
	self->mode = machine->vtable->mode(machine);	
	machineui_updatecoords(self);	
	self->volumedisplay = 0.f;	
	self->machine = machine;
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
		ui_component_destroy(&self->paramview->component);
		free(self->paramview);
	}
	if (self->frame) {
		ui_component_destroy(&self->frame->component);
		free(self->frame);
	}	
	free(self->restorename);
}

ui_size machineui_size(MachineUi* self)
{	
	ui_size rv;

	if (self->coords) {
		rv.width = self->coords->background.destwidth;
		rv.height = self->coords->background.destheight;
	} else {
		rv.width = 200;
		rv.height = 20;		
	}
	return rv;
}

ui_rectangle machineui_position(MachineUi* self)
{
	ui_rectangle rv;

	if (self->coords) {
		ui_setrectangle(&rv, self->x, self->y,
			self->coords->background.destwidth,
			self->coords->background.destheight);
	} else {
		ui_setrectangle(&rv, self->x, self->y, 200, 20);
	}
	return rv;	
}

void machineui_editname(MachineUi* self, ui_edit* edit)
{
	if (self->machine) {
		ui_rectangle r;
		
		free(self->restorename);
		self->restorename = 
			self->machine->vtable->editname(self->machine) 
				? strdup(self->machine->vtable->editname(self->machine)) : 0;
		psy_signal_disconnectall(&edit->component.signal_focuslost);
		psy_signal_disconnectall(&edit->component.signal_keydown);
		psy_signal_disconnectall(&edit->signal_change);
		psy_signal_connect(&edit->signal_change, self, machineui_oneditchange);
		psy_signal_connect(&edit->component.signal_keydown, self,
			machineui_onkeydown);		
		psy_signal_connect(&edit->component.signal_focuslost, self,
			machineui_oneditfocuslost);
		ui_edit_settext(edit, self->machine->vtable->editname(self->machine));
		r = machineui_position(self);
		r.left += self->coords->name.destx;
		r.top += self->coords->name.desty;
		r.right = r.left + self->coords->name.destwidth;
		r.bottom = r.top + self->coords->name.destheight;
		ui_component_setposition(&edit->component, r.left, 
			r.top, r.right - r.left, r.bottom - r.top);
		ui_component_show(&edit->component);
	}
}

void machineui_onkeydown(MachineUi* self, ui_component* sender, KeyEvent* keyevent)
{	
	if (keyevent->keycode == VK_RETURN) {		
		ui_component_hide(sender);
	} else
	if (keyevent->keycode == VK_ESCAPE) {
		if (self->machine) {
			self->machine->vtable->seteditname(self->machine,
				self->restorename);
			free(self->restorename);
			self->restorename = 0;
		}
		ui_component_hide(sender);
	}
}

void machineui_oneditchange(MachineUi* self, ui_edit* sender)
{
	if (self->machine) {
		self->machine->vtable->seteditname(self->machine, ui_edit_text(sender));
	}
}

void machineui_oneditfocuslost(MachineUi* self, ui_component* sender)
{
	ui_component_hide(sender);
}

void machineui_draw(MachineUi* self, MachineWireView* wireview, ui_graphics* g, 
	uintptr_t slot)
{	
	ui_rectangle r;

	r = machineui_position(self);
	r.left += wireview->dx;
	r.right += wireview->dx;
	r.top += wireview->dy;
	r.bottom += wireview->dy;
	if (ui_rectangle_intersect_rectangle(&g->clip, &r) &&  self->coords) {
		MachineCoords* coords;
		char editname[130];

		editname[0] = '\0';
		coords = self->coords;		
		if (self->machine->vtable->editname(self->machine)) {
			if (self->skin->drawmachineindexes) {
				psy_snprintf(editname, 130, "%02d:%s", slot, 
					self->machine->vtable->editname(self->machine));
			} else {
				psy_snprintf(editname, 130, "%s", 
					self->machine->vtable->editname(self->machine));
			}
		}
		ui_setbackgroundmode(g, TRANSPARENT);		
		skin_blitpart(g, &wireview->skin.skinbmp, r.left, r.top, &self->coords->background);
		if (self->mode == MACHMODE_FX) {			
			ui_settextcolor(g, wireview->skin.effect_fontcolour);
		} else {		
			ui_settextcolor(g, wireview->skin.generator_fontcolour);;
		}
		if (self->mode != MACHMODE_MASTER) {			
			ui_textout(g, r.left + coords->name.destx + 2, r.top + coords->name.desty + 2,
				editname, strlen(editname));
			skin_blitpart(g, &wireview->skin.skinbmp, r.left + slidercoord(&coords->pan, 
				self->machine->vtable->panning(self->machine)), r.top,
					&coords->pan);
			if (self->machine->vtable->muted(self->machine)) {
				skin_blitpart(g, &wireview->skin.skinbmp, r.left, r.top, &coords->mute);
			}
			if (self->machine->vtable->bypassed(self->machine)) {
				skin_blitpart(g, &wireview->skin.skinbmp, r.left, r.top, &coords->bypass);
			}
		}		
		if (wireview->drawvumeters) {
			machineui_drawvu(self, wireview, g);
		}
	}
}

void machineui_drawvu(MachineUi* self, MachineWireView* wireview, ui_graphics* g)
{				
	if (self->coords && self->mode != MACHMODE_MASTER) {	
		SkinCoord* vu;		

		vu = &self->coords->vu0;		
		ui_drawbitmap(g, &wireview->skin.skinbmp,
			wireview->dx + self->x + vu->destx,
			wireview->dy + self->y + vu->desty,
			(int)(self->volumedisplay * vu->destwidth),
			vu->destheight,
			vu->srcx, vu->srcy);		
	}
}

void machineui_showparameters(MachineUi* self, ui_component* parent)
{
	if (self->machine) {
		if (self->frame == 0) {			
			self->frame = (MachineFrame*) malloc(sizeof(MachineFrame));
			self->frame->component.hwnd = 0;
		}
		if (self->machine->vtable->haseditor(self->machine)) {
			int width;
			int height;

			machineframe_init(self->frame, parent);
			self->vst2view = (Vst2View*) malloc(sizeof(Vst2View));
			InitVst2View(self->vst2view, &self->frame->component, self->machine);
			machineframe_setview(self->frame, &self->vst2view->component,
				self->machine);
			self->machine->vtable->editorsize(self->machine, &width, &height);
			ui_component_resize(&self->frame->component, width, height + 28);			
		} else
		if (self->frame->component.hwnd == 0) {
			int width;
			int height;
			char txt[128];

			machineframe_init(self->frame, parent);

			if (self->machine && self->machine->vtable->info(self->machine)) {
				psy_snprintf(txt, 128, "%.2X : %s", self->slot,
					self->machine->vtable->info(self->machine)->ShortName);
			} else {
				ui_component_settitle(&self->frame->component, txt);
				psy_snprintf(txt, 128, "%.2X :", self->slot);
			}
			ui_component_settitle(&self->frame->component, txt);
			self->paramview = (ParamView*) malloc(sizeof(ParamView));
			InitParamView(self->paramview, &self->frame->component, self->machine,
				self->workspace);
			machineframe_setview(self->frame, &self->paramview->component,
				self->machine);		
			ParamViewSize(self->paramview, &width, &height);
			ui_component_resize(&self->frame->component, width, height + 28);
		}
		ui_component_show(&self->frame->component);
		//ui_component_setfocus(&self->paramview->component);
	}
}

void machinewireview_init(MachineWireView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{	
	ui_fontinfo fontinfo;

	self->dx = 0;
	self->dy = 0;
	self->statusbar = 0;
	self->workspace = workspace;
	self->machines = &workspace->song->machines;
	self->drawvumeters = 1;	
	ui_bitmap_init(&self->skin.skinbmp);
	ui_bitmap_loadresource(&self->skin.skinbmp, IDB_MACHINESKIN);	
	psy_table_init(&self->machineuis);	
	self->wireframes = 0;
	machinewireview_initmasterui(self);
	ui_component_init(&self->component, parent);
	ui_fontinfo_init(&fontinfo, "Tahoma", 80);
	ui_font_init(&self->skin.font, &fontinfo);
	ui_component_setfont(&self->component, &self->skin.font);
	machinewireview_connectuisignals(self);
	ui_component_move(&self->component, 0, 0);	
	self->dragslot = NOMACHINE_INDEX;
	self->dragmode = MACHINEWIREVIEW_DRAG_MACHINE;
	self->selectedslot = MASTER_INDEX;
	self->selectedwire.src = NOMACHINE_INDEX;
	self->selectedwire.dst = NOMACHINE_INDEX;
	self->component.doublebuffered = TRUE;
	machinewireview_initmachinecoords(self);
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
	machinewireview_adjustscroll(self);
	ui_edit_init(&self->editname, &self->component, 0);
	ui_component_hide(&self->editname.component);
	ui_component_starttimer(&self->component, TIMERID_UPDATEVUMETERS, 50);
}

void machinewireview_adjustscroll(MachineWireView* self)
{
	self->component.scrollstepx = 10;
	self->component.scrollstepy = 10;	
	ui_component_sethorizontalscrollrange(&self->component, 0, 200);
	ui_component_setverticalscrollrange(&self->component, 0, 200);
}

void machinewireview_onscroll(MachineWireView* self, ui_component* sender,
	int stepx,	int stepy)
{
	self->dx += self->component.scrollstepx * stepx;
	self->dy += self->component.scrollstepy * stepy;
}

void machinewireview_connectuisignals(MachineWireView* self)
{
	psy_signal_connect(&self->component.signal_destroy, self,
		machinewireview_ondestroy);
	psy_signal_connect(&self->component.signal_mousedown, self,
		machinewireview_onmousedown);
	psy_signal_connect(&self->component.signal_mouseup, self,
		machinewireview_onmouseup);
	psy_signal_connect(&self->component.signal_mousemove, self,
		machinewireview_onmousemove);
	psy_signal_connect(&self->component.signal_mousedoubleclick, self,
		machinewireview_onmousedoubleclick);
	psy_signal_connect(&self->component.signal_keydown, self,
		machinewireview_onkeydown);
	psy_signal_connect(&self->component.signal_keyup, self,
		machinewireview_onkeyup);
	psy_signal_connect(&self->component.signal_draw, self,
		machinewireview_ondraw);
	psy_signal_connect(&self->component.signal_timer, self,
		machinewireview_ontimer);
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

void machinewireview_ondestroy(MachineWireView* self, ui_component* component)
{
	psy_List* wireframenode;
	machineuis_removeall(self);
	psy_table_dispose(&self->machineuis);
	for (wireframenode = self->wireframes; wireframenode != 0;  wireframenode = wireframenode->next) {
		WireFrame* frame;
		frame = (WireFrame*) wireframenode->entry;
		if (frame->wireview) {
			ui_component_destroy(&frame->wireview->component);
			free(frame->wireview);
		}
		ui_component_destroy(&frame->component);
		free(frame);
	}
	psy_list_free(self->wireframes);
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
		{ 0, 156, 0, 7, 4, 20, 129, 4, 129 },		// vu0
		{ 108, 156, 1, 7, 6, 33, 82, 7, 82 },		// vupeak
		{ 0, 139, 6, 13, 6, 33, 6, 13, 82 },		// pan
		{ 23, 139, 17, 17, 117, 31, 17, 17, 0 },	// mute
		{ 40, 139, 17, 17, 98, 31, 17, 17, 0 },		// bypass
		{ 40, 139, 17, 17, 98, 31, 17, 17, 0 },		// solo
		{ 0, 0, 0, 0, 20, 3, 117, 15, 0 },			// name
	};		
	MachineCoords effect = {
		{ 0, 0, 138, 52, 0, 0, 138, 52, 0 },		// background
		{ 0, 163, 0, 7, 4, 20, 129, 4, 129 },		// vu0
		{ 96, 144, 6, 5, 0, 0, 0, 0, 0 },			// vupeak
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

void machinewireview_onconfigchanged(MachineWireView* self, Workspace* workspace, psy_Properties* property)
{	
	machinewireview_readconfig(self);
}

void machinewireview_readconfig(MachineWireView* self)
{
	psy_Properties* mv;
	
	mv = psy_properties_findsection(self->workspace->config, "visual.machineview");
	if (mv) {		
		self->drawvumeters = psy_properties_bool(mv, "drawvumeters", 1);
		ui_component_stoptimer(&self->component, TIMERID_UPDATEVUMETERS);
		if (self->drawvumeters) {
			ui_component_starttimer(&self->component, TIMERID_UPDATEVUMETERS, 50);
		}
		self->skin.drawmachineindexes = psy_properties_bool(mv, "drawmachineindexes", 1);
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
	ui_component_setbackgroundcolor(&self->component, self->skin.colour);
	machine_skin_name = psy_properties_readstring(p, "machine_skin",
		0);	
	if (machine_skin_name) {
		char path[_MAX_PATH];
		char filename[_MAX_PATH];

		strcpy(filename, machine_skin_name);
		strcat(filename, ".bmp");
		psy_dir_findfile(workspace_skins_directory(self->workspace),
			filename, path);
		if (path[0] != '\0') {
			ui_bitmap bmp;

			ui_bitmap_init(&bmp);
			if (ui_bitmap_load(&bmp, path) == 0) {
				ui_bitmap_dispose(&self->skin.skinbmp);
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



void machinewireview_ondraw(MachineWireView* self, ui_component* sender, ui_graphics* g)
{	
	machinewireview_drawwires(self, g);
	machinewireview_drawmachines(self, g);
	machinewireview_drawdragwire(self, g);
}

void machinewireview_drawwires(MachineWireView* self, ui_graphics* g)
{
	psy_TableIterator it;
	
	for (it = psy_table_begin(&self->machineuis); 
			!psy_tableiterator_equal(&it, psy_table_end()); 
				psy_tableiterator_inc(&it)) {
		machinewireview_drawwire(self, g, psy_tableiterator_key(&it),
			(MachineUi*)psy_tableiterator_value(&it));
	}	
}

void machinewireview_drawwire(MachineWireView* self, ui_graphics* g, uintptr_t slot,
	MachineUi* outmachineui)
{		
	psy_audio_MachineSockets* sockets;
	WireSocket* p;
	
	sockets	= connections_at(&self->machines->connections, slot);
	if (sockets) {
		for (p = sockets->outputs; p != NULL; p = p->next) {
			psy_audio_WireSocketEntry* entry = (psy_audio_WireSocketEntry*)p->entry;
			if (entry->slot != NOMACHINE_INDEX) {
				MachineUi* inmachineui;				

				inmachineui = machineuis_at(self, entry->slot);
				if (inmachineui && outmachineui) {
					ui_rectangle out;
					ui_rectangle in;

					out = machineui_position(inmachineui);
					in = machineui_position(outmachineui);
					if (self->selectedwire.src == slot &&
							self->selectedwire.dst == entry->slot) {
						ui_setcolor(g, self->skin.selwirecolour);
					} else {
						ui_setcolor(g, self->skin.wirecolour);
					}
					ui_drawline(g, 
						self->dx + out.left + (out.right - out.left) / 2,
						self->dy + out.top + (out.bottom - out.top) / 2,
						self->dx + in.left + (in.right - in.left) / 2,
						self->dy + in.top + (in.bottom - in.top) / 2);
					machinewireview_drawwirearrow(self, g, outmachineui, inmachineui);
				}
			}		
		}
	}
}

void machinewireview_drawwirearrow(MachineWireView* self, ui_graphics* g,
	MachineUi* outmachineui, MachineUi* inmachineui)
{
	ui_size out;
	ui_size in;	
	int x1,	y1;
	int x2, y2;		
	double phi;	
	ui_point center;
	ui_point a, b, c;	
	ui_point tri[4];
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
	
	ui_drawsolidpolygon(g, tri, 4, polyInnards, self->skin.wireaacolour);	
}

ui_point rotate_point(ui_point pt, double phi)
{
	ui_point rv;
	
	rv.x = (int) (cos(phi) * pt.x - sin(phi) * pt.y);
	rv.y = (int) (sin(phi) * pt.x + cos(phi) * pt.y);
	return rv;
}

ui_point move_point(ui_point pt, ui_point d)
{
	ui_point rv;
	
	rv.x = pt.x + d.x;
	rv.y = pt.y + d.y;
	return rv;
}

void machinewireview_drawdragwire(MachineWireView* self, ui_graphics* g)
{
	if (self->dragslot != NOMACHINE_INDEX &&
			self->dragmode == MACHINEWIREVIEW_DRAG_NEWCONNECTION) {
		MachineUi* machineui;

		machineui = machineuis_at(self, self->dragslot);
		if (machineui) {
			ui_size machinesize;

			machinesize = machineui_size(machineui);
			ui_setcolor(g, self->skin.wirecolour);
			ui_drawline(g, 
				self->dx + machineui->x + machinesize.width/2,
				self->dy + machineui->y + machinesize.height/2,
				self->dx + self->mx,
				self->dy + self->my);		
		}
	}
}

void machinewireview_drawmachines(MachineWireView* self, ui_graphics* g)
{
	psy_TableIterator it;
	
	for (it = psy_table_begin(&self->machineuis); it.curr != 0; 
			psy_tableiterator_inc(&it)) {				
		psy_audio_Machine* machine;

		machine = machines_at(self->machines, psy_tableiterator_key(&it));
		if (machine) {
			MachineUi* machineui;

			machineui = (MachineUi*)psy_tableiterator_value(&it);
			machineui_draw(machineui, self, g, psy_tableiterator_key(&it));
			if (self->selectedwire.src == NOMACHINE_INDEX && self->selectedslot == psy_tableiterator_key(&it)) {
				machineui_drawhighlight(machineui, self, g);	
			}
		}
	}
}

int slidercoord(SkinCoord* coord, float value)
{	
	return (int)(value * coord->range);
}

void machineui_drawhighlight(MachineUi* self, MachineWireView* wireview, ui_graphics* g)
{	
	ui_rectangle r;
	static int d = 5; // the distance of the highlight from the machine

	r = machineui_position(self);
	r.left += wireview->dx;
	r.right += wireview->dx;
	r.top += wireview->dy;
	r.bottom += wireview->dy;
	ui_setcolor(g, wireview->skin.wirecolour);
	drawmachineline(g, 1, 0, r.left - d, r.top - d);
	drawmachineline(g, 0, 1, r.left - d, r.top - d);
	drawmachineline(g, -1, 0, r.right + d, r.top - d);
	drawmachineline(g, 0, 1, r.right + d, r.top - d);
	drawmachineline(g, 0, -1, r.right + d, r.bottom + d);
	drawmachineline(g, -1, 0, r.right + d, r.bottom + d);
	drawmachineline(g, 1, 0, r.left - d, r.bottom + d);
	drawmachineline(g, 0, -1, r.left - d, r.bottom + d);	
}

void drawmachineline(ui_graphics* g, int xdir, int ydir, int x, int y)
{
	int hlength = 9; // the length of the selected machine highlight	

	ui_drawline(g, x, y, x + xdir * hlength, y + ydir * hlength);
}

void machineview_align(MachineView* self)
{
	machinewireview_align(&self->wireview);
}

void machinewireview_align(MachineWireView* self)
{
	MachineUi* machineui;
	ui_size machinesize;
	ui_size size = ui_component_size(&self->component);

	machineui = machineuis_at(self, MASTER_INDEX);
	if (machineui) {
		machinesize = machineui_size(machineui);
		machineui->x = (size.width - machinesize.width) / 2 ;
		machineui->y = (size.height - machinesize.height) / 2;
	}
}

void machinewireview_onmousedoubleclick(MachineWireView* self, ui_component* sender, MouseEvent* ev)
{
	self->mx = ev->x - self->dx;
	self->my = ev->y - self->dy;
	machinewireview_hittest(self, ev->x - self->dx, ev->y - self->dy);
	if (self->dragslot == NOMACHINE_INDEX) {
		self->selectedwire = machinewireview_hittestwire(self, ev->x - self->dx,
			ev->y - self->dy);
		if (self->selectedwire.dst != NOMACHINE_INDEX) {			
			machinewireview_showwireview(self, self->selectedwire);
			ui_component_invalidate(&self->component);
		} else {
			ui_component_propagateevent(sender);
		}
	} else		 
	if (machinewireview_hittesteditname(self, ev->x - self->dx,
			ev->y - self->dy, self->dragslot)) {
		if (machineuis_at(self, self->dragslot)) {
			machineui_editname(machineuis_at(self, self->dragslot), &self->editname);
		}
	} else {
		workspace_showparameters(self->workspace, self->dragslot);
	}
	self->dragslot = NOMACHINE_INDEX;
}

void machinewireview_onmousedown(MachineWireView* self, ui_component* sender, MouseEvent* ev)
{
	ui_component_hide(&self->editname.component);
	ui_component_setfocus(&self->component);
	self->mx = ev->x - self->dx;
	self->my = ev->y - self->dy;	
	machinewireview_hittest(self, ev->x - self->dx, ev->y - self->dy);
	if (self->dragslot == NOMACHINE_INDEX) {
		if (ev->button == 1) {
			self->selectedwire = machinewireview_hittestwire(self,
				ev->x - self->dx, ev->y - self->dy);
			ui_component_invalidate(&self->component);
		}
	} else {		
		if (ev->button == 1) {
			if (self->dragslot != MASTER_INDEX) {
				self->selectedslot = self->dragslot;
				self->selectedwire.src = NOMACHINE_INDEX;
				self->selectedwire.dst = NOMACHINE_INDEX;
				machines_changeslot(self->machines, self->selectedslot);
			}			
			if (machinewireview_hittestcoord(self, ev->x - self->dx,
					ev->y - self->dy, MACHMODE_GENERATOR, self->dragslot,
				&self->skin.generator.solo)) {
				psy_audio_Machine* machine = machines_at(self->machines, self->dragslot);
				if (machine) {
					if (machine->vtable->bypassed(machine)) {
						machine->vtable->unbypass(machine);
					} else {
						machine->vtable->bypass(machine);
					}
				}
			} else
			if (machinewireview_hittestcoord(self, ev->x - self->dx,
					ev->y - self->dy, MACHMODE_FX, self->dragslot,
					&self->skin.effect.bypass)) {
				psy_audio_Machine* machine = machines_at(self->machines, self->dragslot);
				if (machine) {
					if (machine->vtable->bypassed(machine)) {
						machine->vtable->unbypass(machine);
					} else {
						machine->vtable->bypass(machine);
					}
				}
			} else
			if (machinewireview_hittestcoord(self, ev->x - self->dx,
					ev->y - self->dy, MACHMODE_GENERATOR, self->dragslot,
					&self->skin.generator.mute) ||
				machinewireview_hittestcoord(self, ev->x - self->dx,
					ev->y - self->dy, MACHMODE_FX, self->dragslot,
					&self->skin.effect.mute)) {
				psy_audio_Machine* machine = machines_at(self->machines, self->dragslot);
				if (machine) {
					if (machine->vtable->muted(machine)) {
						machine->vtable->unmute(machine);
					} else {
						machine->vtable->mute(machine);
					}
				}
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
				}
			}
			
		} else
		if (ev->button == 2) {
			psy_audio_Machine* machine;

			machine = machines_at(self->machines, self->dragslot);
			if (machine && machine->vtable->numoutputs(machine) > 0) {
				self->dragmode = MACHINEWIREVIEW_DRAG_NEWCONNECTION;				
			} else {
				self->dragslot = NOMACHINE_INDEX;
			}			
		}
	}
}

int machinewireview_hittestpan(MachineWireView* self, int x, int y, uintptr_t slot, int* dx)
{
	int rv = 0;
	psy_audio_Machine* machine;
	ui_rectangle r;
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
			offset = (int) (machine->vtable->panning(machine) *
				coords->pan.range);
			ui_setrectangle(&r, coords->pan.destx + offset, coords->pan.desty,
					coords->pan.destwidth, coords->pan.destheight);
			*dx = xm - r.left;
			rv = ui_rectangle_intersect(&r, xm, ym);
		}
	}
	return rv;
}

int machinewireview_hittesteditname(MachineWireView* self, int x, int y, uintptr_t slot)
{
	int rv = 0;
	psy_audio_Machine* machine;
	ui_rectangle r;	
	MachineCoords* coords;
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui) {
		int xm = x - machineui->x;
		int ym = y - machineui->y;
		machine = machines_at(self->machines, slot);
		coords = machineui->coords;
		if (coords) {			
			ui_setrectangle(&r, coords->name.destx, coords->name.desty,
				coords->name.destwidth, coords->name.destheight);			
			rv = ui_rectangle_intersect(&r, xm, ym);
		}
	}
	return rv;
}

int machinewireview_hittestcoord(MachineWireView* self, int x, int y, int mode, uintptr_t slot, SkinCoord* coord)
{
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (machineui && machineui->mode != mode) {
		ui_rectangle r = { 0, 0, 0, 0 };	
		int xm = x - machineui->x;	
		int ym = y - machineui->y;
			
		ui_setrectangle(&r, coord->destx, coord->desty,
			coord->destwidth, coord->destheight);
		return ui_rectangle_intersect(&r, xm, ym);
	}
	return 0;
}

psy_dsp_amp_t machinewireview_panvalue(MachineWireView* self, int x, int y, uintptr_t slot)
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
		ui_rectangle position;	

		position = machineui_position((MachineUi*)psy_tableiterator_value(&it));
		if (ui_rectangle_intersect(&position, self->mx, self->my)) {
			self->dragslot = it.curr->key;
			break;	
		}
	}	
}

void machinewireview_onmousemove(MachineWireView* self, ui_component* sender, MouseEvent* ev)
{
	if (self->dragslot != NOMACHINE_INDEX) {
		if (self->dragmode == MACHINEWIREVIEW_DRAG_PAN) {
			psy_audio_Machine* machine;

			machine = machines_at(self->machines, self->dragslot);
			if (machine) {
				machine->vtable->setpanning(machine,machinewireview_panvalue(
					self, ev->x - self->dx, ev->y - self->dy, self->dragslot));
			}
		} else
		if (self->dragmode == MACHINEWIREVIEW_DRAG_MACHINE) {
			MachineUi* machineui;

			machineui = machineuis_at(self, self->dragslot);
			if (self->statusbar && machineui && machineui->machine) {
				char txt[128];

				machineui->x = ev->x - self->mx - self->dx;
				machineui->y = ev->y - self->my - self->dy;									
				psy_snprintf(txt, 128, "%s (%d, %d)",
					machineui->machine->vtable->editname(machineui->machine)
					? machineui->machine->vtable->editname(machineui->machine)
					: "",
					ev->x - self->mx - self->dx,
					ev->y - self->my - self->dy);
				ui_label_settext(&self->statusbar->label, txt);
				ui_component_invalidate(&self->statusbar->label.component);				
			}
		} else
		if (self->dragmode == MACHINEWIREVIEW_DRAG_NEWCONNECTION) {
			self->mx = ev->x - self->dx;
			self->my = ev->y - self->dy;			
		}
		ui_component_invalidate(&self->component);				
	}
}

void machinewireview_onmouseup(MachineWireView* self, ui_component* sender, MouseEvent* ev)
{	
	if (self->dragslot != NOMACHINE_INDEX) {
		if (self->dragmode == MACHINEWIREVIEW_DRAG_NEWCONNECTION) {
			uintptr_t outputslot = self->dragslot;
			self->dragslot = NOMACHINE_INDEX;
			machinewireview_hittest(self, ev->x - self->dx,
				ev->y - self->dy);
			if (self->dragslot != NOMACHINE_INDEX) {
				machines_connect(self->machines, outputslot, self->dragslot);				
			}
		}
	}
	self->dragslot = NOMACHINE_INDEX;
	ui_component_invalidate(&self->component);
}

void machinewireview_onkeydown(MachineWireView* self, ui_component* sender, KeyEvent* keyevent)
{		
	if (keyevent->keycode == VK_DELETE && self->selectedwire.src != NOMACHINE_INDEX) {
		machines_disconnect(self->machines, self->selectedwire.src, 
			self->selectedwire.dst);
		ui_component_invalidate(&self->component);
	} else 
	if (keyevent->keycode == VK_DELETE && self->selectedslot != - 1 && 
			self->selectedslot != MASTER_INDEX) {		
		machines_remove(self->machines, self->selectedslot);
		self->selectedslot = NOMACHINE_INDEX;
	} else 
	if (!keyevent->repeat) {
		ui_component_propagateevent(sender);
	}
}

void machinewireview_onkeyup(MachineWireView* self, ui_component* sender, KeyEvent* ev)
{
	ui_component_propagateevent(sender);
}

psy_audio_Wire machinewireview_hittestwire(MachineWireView* self, int x, int y)
{		
	psy_audio_Wire rv;
	psy_TableIterator it;
	
	rv.dst = NOMACHINE_INDEX;
	rv.src = NOMACHINE_INDEX;
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
				psy_audio_WireSocketEntry* entry = (psy_audio_WireSocketEntry*)p->entry;
				if (entry->slot != NOMACHINE_INDEX) {
					ui_size out;
					ui_size in;								
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

void machinewireview_onmachineschangeslot(MachineWireView* self, psy_audio_Machines* machines,
	uintptr_t slot)
{
	self->selectedslot = slot;
	self->selectedwire.src = NOMACHINE_INDEX;
	self->selectedwire.dst = NOMACHINE_INDEX;
	ui_component_invalidate(&self->component);
	ui_component_setfocus(&self->component);
}

void machinewireview_onmachinesinsert(MachineWireView* self, psy_audio_Machines* machines, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = machines_at(self->machines, slot);
	if (machine) {
		machineuis_insert(self, slot, 0, 0, &self->skin);
		psy_signal_connect(&machine->signal_worked, self, 
			machinewireview_onmachineworked);
		ui_component_invalidate(&self->component);
	}
}

void machinewireview_onmachinesremoved(MachineWireView* self, psy_audio_Machines* machines, uintptr_t slot)
{
	machineuis_remove(self, slot);
	ui_component_invalidate(&self->component);
}

void machinewireview_beforesavesong(MachineWireView* self, Workspace* workspace)
{
	if (workspace->properties) {		
		psy_Properties* p;
		
		p = psy_properties_find(workspace->properties, "machines");
		if (p) {
			for (p = p->children; p != 0; p = psy_properties_next(p)) {				
				int index;				
								
				index = psy_properties_int(p, "index", NOMACHINE_INDEX);
				if (index != NOMACHINE_INDEX) {
					MachineUi* machineui;
					
					machineui = machineuis_at(self, index);
					if (machineui) {
						psy_properties_write_int(p, "x", machineui->x);
						psy_properties_write_int(p, "y", machineui->y);						
					}
				}
			}			
		}
	}	
}

void machinewireview_updatemachineuis(MachineWireView* self, psy_Properties* machines)
{
	if (machines) {
		psy_audio_Machine* machine;
		psy_Properties* p;

		machineuis_removeall(self);
		p = machines->children;
		while (p) {
			if (p->item.key && strcmp(p->item.key, "machine") == 0) {				
				int x;
				int y;
				int index;				
																							
				index = psy_properties_int(p, "index", 0);
				x = psy_properties_int(p, "x", 0);
				y = psy_properties_int(p, "y", 0);				
				machine = machines_at(self->machines, index);
				if (machine) {					
					machineuis_insert(self, index, x, y, &self->skin);
					if (index != MASTER_INDEX) {					
						psy_signal_connect(&machine->signal_worked, self,
							machinewireview_onmachineworked);
					}
				}
			}
			p = psy_properties_next(p);
		}
	}
}

void machinewireview_onsongchanged(MachineWireView* self, Workspace* workspace)
{		
	self->machines = &workspace->song->machines;	
	machinewireview_updatemachineuis(self, psy_properties_find(workspace->properties,
		"machines"));	
	machinewireview_connectmachinessignals(self);
	ui_component_invalidate(&self->component);	
}

void machinewireview_onshowparameters(MachineWireView* self, Workspace* sender,
	uintptr_t slot)
{	
	if (machineuis_at(self, slot)) {
		machineui_showparameters(machineuis_at(self, slot), &self->component);
	}
}

void machinewireview_onmachineworked(MachineWireView* self, psy_audio_Machine* machine,
	uintptr_t slot, psy_audio_BufferContext* bc)
{
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (slot != MASTER_INDEX && machineui) {	
		if (buffercontext_rmsvolume(bc) != 0.f) {
			self = self;
		}
		machineui->volumedisplay = buffercontext_rmsvolume(bc) / 32767;				
	}
}

void machinewireview_ontimer(MachineWireView* self, ui_component* sender, int timerid)
{	
	psy_List* p;
	psy_List* q;
	
	for (p = self->wireframes; p != 0; p = q) {
		WireFrame* frame;

		frame = (WireFrame*) p->entry;
		q = p->next;
		if (frame->wireview && !wireview_wireexists(frame->wireview)) {			
			ui_component_destroy(&frame->wireview->component);
			free(frame->wireview);
			ui_component_destroy(&frame->component);
			free(frame);
			psy_list_remove(&self->wireframes, p);
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
		MachineUi* machineui;

		machineui = (MachineUi*) psy_tableiterator_value(&it);		
		if (psy_tableiterator_key(&it) != MASTER_INDEX) {
			ui_rectangle r;

			r = machineui_position(machineui);
			ui_component_invalidaterect(&self->component, &r);	
		}
	}	
}

void machinewireview_showwireview(MachineWireView* self, psy_audio_Wire wire)
{			
	WireFrame* wireframe;

	wireframe = machinewireview_wireframe(self, wire);
	if (!wireframe) {
		wireframe = (WireFrame*)malloc(sizeof(WireFrame));
		if (wireframe) {
			wireframe->component.hwnd = 0;			
			psy_list_append(&self->wireframes, wireframe);			
		}
	}		
	if (wireframe && wireframe->component.hwnd == 0) {
		WireView* wireview;

		wireframe_init(wireframe, &self->component, 0);
		wireview = (WireView*) malloc(sizeof(WireView));
		if (wireview) {
			wireview_init(wireview, &wireframe->component, wire,
				self->workspace);
			wireframe->wireview = wireview;						
		} else {
			ui_component_destroy(&wireframe->component);
			free(wireframe);
			wireframe = 0;
		}
	}
	if (wireframe != 0) {
		ui_component_show(&wireframe->component);
	}
}

WireFrame* machinewireview_wireframe(MachineWireView* self, psy_audio_Wire wire)
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

void machinewireview_onnewmachineselected(MachineView* self, ui_component* sender,
	psy_Properties* plugininfo)
{	
	psy_audio_Machine* machine;
	const char* path;
		
	path = psy_properties_readstring(plugininfo, "path", "");
	machine = machinefactory_makemachinefrompath(
		&self->workspace->machinefactory, 
		psy_properties_int(plugininfo, "type", NOMACHINE_INDEX), path);
	if (machine) {		
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


void machineuis_insert(MachineWireView* self, uintptr_t slot, int x, int y, MachineSkin* skin)
{	
	psy_audio_Machine* machine;

	machine = machines_at(self->machines, slot);
	if (machine) {
		MachineUi* machineui;

		if (psy_table_exists(&self->machineuis, slot)) {
			machineuis_remove(self, slot);
		}	
		machineui = (MachineUi*) malloc(sizeof(MachineUi));
		machineui_init(machineui, x, y, machine, slot, &self->skin, self->workspace);
		psy_table_insert(&self->machineuis, slot, machineui);
	}
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

void machineviewbar_init(MachineViewBar* self, ui_component* parent, Workspace* workspace)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makepx(0),
		ui_value_makepx(0), ui_value_makepx(0));
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_label_init(&self->label, &self->component);	
	ui_label_setcharnumber(&self->label, 40);			
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT,
		&margin));	
}

void machineview_init(MachineView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	ui_component_enablealign(&self->component);
	ui_notebook_init(&self->notebook, &self->component);	
	ui_component_setalign(&self->notebook.component, UI_ALIGN_CLIENT);
	psy_signal_connect(&self->component.signal_show, self, machineview_onshow);
	psy_signal_connect(&self->component.signal_hide, self, machineview_onhide);
	machinewireview_init(&self->wireview, &self->notebook.component, tabbarparent,
		workspace);
	newmachine_init(&self->newmachine, &self->notebook.component,
		self->workspace);	
	tabbar_init(&self->tabbar, tabbarparent);
	ui_component_setalign(&self->tabbar.component, UI_ALIGN_LEFT);
	ui_component_hide(&self->tabbar.component);
	tabbar_append(&self->tabbar, "Wires");
	tabbar_append(&self->tabbar, "New psy_audio_Machine");		
	ui_notebook_setpageindex(&self->notebook, 0);
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	psy_signal_connect(&self->newmachine.pluginsview.signal_selected, self,machinewireview_onnewmachineselected);
	psy_signal_connect(&self->component.signal_mousedoubleclick, self, machineview_onmousedoubleclick);
	psy_signal_connect(&self->component.signal_keydown, self, machineview_onkeydown);
	psy_signal_connect(&self->component.signal_keyup, self, machineview_onkeyup);
	psy_signal_connect(&self->component.signal_focus, self, machineview_onfocus);
	psy_signal_connect(&self->workspace->signal_skinchanged, self,
		machineview_onskinchanged);
}

void machineview_onshow(MachineView* self, ui_component* sender)
{	
	self->tabbar.component.visible = 1;
	ui_component_align(ui_component_parent(&self->tabbar.component));
	ui_component_show(&self->tabbar.component);
}

void machineview_onhide(MachineView* self, ui_component* sender)
{
	ui_component_hide(&self->tabbar.component);
}

void machineview_onmousedoubleclick(MachineView* self, ui_component* sender,
	int x, int y, int button)
{	
	self->newmachine.pluginsview.calledby = 0;
	tabbar_select(&self->tabbar, 1);
}

void machineview_onkeydown(MachineView* self, ui_component* sender, KeyEvent* keyevent)
{
	if (keyevent->keycode == VK_ESCAPE) {
		if (tabbar_selected(&self->tabbar) == 1) {
			tabbar_select(&self->tabbar, 0);			
		}
	} else {
		ui_component_propagateevent(sender);
	}
}

void machineview_onkeyup(MachineView* self, ui_component* sender, KeyEvent* ev)
{
	ui_component_propagateevent(sender);
}

void machineview_applyproperties(MachineView* self, psy_Properties* p)
{
	machinewireview_applyproperties(&self->wireview, p);
}

void machineview_onfocus(MachineView* self, ui_component* sender)
{
	ui_component_setfocus(&self->wireview.component);
}

void machineview_onskinchanged(MachineView* self, Workspace* sender,
	psy_Properties* properties)
{	
	machineview_applyproperties(self, properties);
}
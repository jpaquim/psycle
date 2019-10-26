// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineview.h"
#include "skingraphics.h"
#include "resources/resource.h"
#include <math.h>

#define TIMERID_UPDATEVUMETERS 300

void machineuis_insert(WireView*, int slot, int x, int y, MachineSkin*,
	const char* editname);
static MachineUi* machineuis_at(WireView*, int slot);
static void machineuis_remove(WireView*, int slot);
static void machineuis_removeall(WireView*);

static void machineui_init(MachineUi*, int x, int y, Machine* machine,
	MachineSkin*, const char* editname);
static void machineui_updatecoords(MachineUi*);
static void machineui_dispose(MachineUi*);
static ui_size machineui_size(MachineUi*);
static ui_rectangle machineui_position(MachineUi*);
static void machineui_draw(MachineUi*, WireView*, ui_graphics*, int slot);
static void machineui_drawvu(MachineUi*, WireView*, ui_graphics*);
static void machineui_drawhighlight(MachineUi*, WireView*, ui_graphics*);
static void machineui_showparameters(MachineUi*, ui_component* parent);
static int slidercoord(SkinCoord*, float value);

static void wireview_initmasterui(WireView*);
static void wireview_connectuisignals(WireView*);
static void wireview_connectmachinessignals(WireView*);
static void wireview_ondraw(WireView*, ui_component* sender, ui_graphics*);
static void wireview_drawmachines(WireView*, ui_graphics*);
static void drawmachineline(ui_graphics* g, int xdir, int ydir, int x, int y);
static void wireview_drawdragwire(WireView*, ui_graphics*);
static void wireview_drawwires(WireView*, ui_graphics*);
static void wireview_drawwire(WireView*, ui_graphics*, int slot, MachineUi*);
static void wireview_drawwirearrow(WireView*, ui_graphics*, MachineUi* out, MachineUi* in);
static ui_point rotate_point(ui_point, double phi);
static ui_point move_point(ui_point pt, ui_point d);
static void wireview_ondestroy(WireView*, ui_component* component);
static void wireview_onmousedown(WireView*, ui_component* sender, int x, int y, int button);
static void wireview_onmouseup(WireView*, ui_component* sender, int x, int y, int button);
static void wireview_onmousemove(WireView*, ui_component* sender,int x, int y, int button);
static void wireview_onmousedoubleclick(WireView*, ui_component* sender, int x, int y, int button);
static void wireview_onkeydown(WireView*, ui_component* sender, int keycode, int keydata);
static void wireview_hittest(WireView*, int x, int y);
static int wireview_hittestpan(WireView*, int x, int y, int slot, int* dx);
static int wireview_hittestcoord(WireView*, int x, int y, int mode, int slot, SkinCoord*);
static WireConnection wireview_hittestwire(WireView*, int x, int y);
static amp_t wireview_panvalue(WireView*, int x, int y, int slot);
static void wireview_onnewmachineselected(MachineView*, ui_component* sender, Properties*);
static void wireview_initmachinecoords(WireView*);
static void wireview_onmachineschangeslot(WireView*, Machines*, int slot);
static void wireview_onmachinesinsert(WireView*, Machines*, int slot);
static void wireview_onmachinesremoved(WireView*, Machines*, int slot);
static void wireview_onsongchanged(WireView*, Workspace*);
static void wireview_updatemachineuis(WireView*, Properties* );
static void wireview_applyproperties(WireView*, Properties*);
static void wireview_onshowparameters(WireView*, Machines*, int slot);
static void wireview_onmachineworked(WireView*, Machine*, unsigned int slot,
	BufferContext*);
static void wireview_ontimer(WireView*, ui_component* sender, int timerid);
static void wireview_preparedrawallmacvus(WireView*);
static void wireview_onconfigchanged(WireView*, Workspace*, Properties*);
static void wireview_readconfig(WireView*);

static void machineview_onshow(MachineView*, ui_component* sender);
static void machineview_onhide(MachineView*, ui_component* sender);
static void machineview_onmousedoubleclick(MachineView*, ui_component* sender, int x, int y, int button);
static void machineview_onkeydown(MachineView*, ui_component* sender, int keycode, int keydata);

void machineui_init(MachineUi* self, int x, int y, Machine* machine,
	MachineSkin* skin, const char* editname)
{	
	assert(machine);
	self->x = x;
	self->y = y;
	self->skin = skin;	
	self->mode = machine->mode(machine);
	if (self->mode == MACHMODE_MASTER) {
		self->mode = MACHMODE_MASTER;		
	} else {
		self->mode = (self->mode & 3) ? MACHMODE_GENERATOR : MACHMODE_FX;
	}	
	machineui_updatecoords(self);
	self->editname = _strdup(editname);
	self->volumedisplay = 0.f;	
	self->machine = machine;
	self->frame = 0;
	self->paramview = 0;
	self->vst2view = 0;
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
	free(self->editname);
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

void machineui_draw(MachineUi* self, WireView* wireview, ui_graphics* g, 
	int slot)
{
	int x;
	int y;

	x = self->x;
	y = self->y;
	if (ui_rectangle_intersect(&g->clip, x, y) && 
			self->coords) {
		MachineCoords* coords;
		char editname[130];

		coords = self->coords;		
		if (self->editname) {
			_snprintf(editname, 130, "%02d:%s", slot, self->editname);		
		}
		ui_setbackgroundmode(g, TRANSPARENT);		
		skin_blitpart(g, &wireview->skin.skinbmp, x, y, &self->coords->background);
		if (self->mode == MACHMODE_FX) {			
			ui_settextcolor(g, wireview->skin.effect_fontcolour);
		} else {		
			ui_settextcolor(g, wireview->skin.generator_fontcolour);;
		}
		if (self->mode != MACHMODE_MASTER) {			
			ui_textout(g, x + coords->name.destx + 2, y + coords->name.desty + 2,
				editname, strlen(editname));
			skin_blitpart(g, &wireview->skin.skinbmp, x + slidercoord(&coords->pan, 
				machine_panning(self->machine)), y, &coords->pan);
			if (machine_muted(self->machine)) {
				skin_blitpart(g, &wireview->skin.skinbmp, x, y, &coords->mute);
			}
			if (machine_bypassed(self->machine)) {
				skin_blitpart(g, &wireview->skin.skinbmp, x, y, &coords->bypass);
			}
		}		
		if (wireview->drawvumeters) {
			machineui_drawvu(self, wireview, g);
		}
	}
}

void machineui_drawvu(MachineUi* self, WireView* wireview, ui_graphics* g)
{				
	if (self->coords && self->mode != MACHMODE_MASTER) {	
		SkinCoord* vu;		

		vu = &self->coords->vu0;		
		ui_drawbitmap(g, &wireview->skin.skinbmp,
			self->x + vu->destx, self->y + vu->desty,
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
		if (self->machine->haseditor(self->machine)) {
			int width;
			int height;

			InitMachineFrame(self->frame, parent);
			self->vst2view = (Vst2View*) malloc(sizeof(Vst2View));
			InitVst2View(self->vst2view, &self->frame->component, self->machine);
			MachineFrameSetParamView(self->frame, &self->vst2view->component);
			self->machine->editorsize(self->machine, &width, &height);
			ui_component_resize(&self->frame->component, width, height + 28);			
		} else
		if (self->frame->component.hwnd == 0) {
			int width;
			int height;

			InitMachineFrame(self->frame, parent);
			self->paramview = (ParamView*) malloc(sizeof(ParamView));
			InitParamView(self->paramview, &self->frame->component, self->machine);
			MachineFrameSetParamView(self->frame, &self->paramview->component);		
			ParamViewSize(self->paramview, &width, &height);
			ui_component_resize(&self->frame->component, width, height + 28);
		}
		ui_component_show(&self->frame->component);
		//ui_component_setfocus(&self->paramview->component);
	}
}

void wireview_init(WireView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{	
	ui_fontinfo fontinfo;

	self->workspace = workspace;
	self->machines = &workspace->song->machines;
	self->drawvumeters = 1;		
	ui_bitmap_loadresource(&self->skin.skinbmp, IDB_MACHINESKIN);	
	table_init(&self->machineuis);	
	wireview_initmasterui(self);
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_fontinfo_init(&fontinfo, "Tahoma", 80);
	ui_font_init(&self->skin.font, &fontinfo);
	ui_component_setfont(&self->component, &self->skin.font);
	wireview_connectuisignals(self);
	ui_component_move(&self->component, 0, 0);	
	self->dragslot = -1;
	self->dragmode = WIREVIEW_DRAG_MACHINE;
	self->selectedslot = MASTER_INDEX;
	self->selectedwire.src = -1;
	self->selectedwire.dst = -1;
	self->component.doublebuffered = TRUE;
	wireview_initmachinecoords(self);
	signal_connect(&workspace->signal_songchanged, self, wireview_onsongchanged);	
	wireview_connectmachinessignals(self);
	signal_connect(&workspace->signal_configchanged, self, wireview_onconfigchanged);	
	SetTimer(self->component.hwnd, TIMERID_UPDATEVUMETERS, 50, 0);
}

void wireview_connectuisignals(WireView* self)
{
	signal_connect(&self->component.signal_destroy, self, wireview_ondestroy);
	signal_connect(&self->component.signal_mousedown, self, wireview_onmousedown);
	signal_connect(&self->component.signal_mouseup, self, wireview_onmouseup);
	signal_connect(&self->component.signal_mousemove, self, wireview_onmousemove);
	signal_connect(&self->component.signal_mousedoubleclick, self, wireview_onmousedoubleclick);
	signal_connect(&self->component.signal_keydown, self, wireview_onkeydown);	
	signal_connect(&self->component.signal_draw, self, wireview_ondraw);
	signal_connect(&self->component.signal_timer, self, wireview_ontimer);
}

void wireview_initmasterui(WireView* self)
{	
	machineuis_insert(self, MASTER_INDEX, 0, 0, &self->skin, "Master");
}

void wireview_connectmachinessignals(WireView* self)
{
	signal_connect(&self->machines->signal_slotchange, self, wireview_onmachineschangeslot);
	signal_connect(&self->machines->signal_insert, self, wireview_onmachinesinsert);
	signal_connect(&self->machines->signal_removed, self, wireview_onmachinesremoved);
	signal_connect(&self->machines->signal_showparameters, self, wireview_onshowparameters);
}

void wireview_ondestroy(WireView* self, ui_component* component)
{
	machineuis_removeall(self);
	table_dispose(&self->machineuis);
}

void wireview_initmachinecoords(WireView* self)
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
		{ 40, 139, 17, 17, 98, 31, 17, 17, 0 },		// solo
		{ 40, 139, 17, 17, 98, 31, 17, 17, 0 },		// bypass
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
	wireview_applyproperties(self, 0);	
}

void wireview_onconfigchanged(WireView* self, Workspace* workspace, Properties* property)
{
	if (property == workspace->config) {
		wireview_readconfig(self);
	} else
	if (strcmp(properties_key(property), "drawvumeters") == 0) {
		self->drawvumeters = properties_value(property);
	}
}

void wireview_readconfig(WireView* self)
{
	Properties* mv;
	
	mv = properties_findsection(self->workspace->config, "visual.machineview");
	if (mv) {		
		self->drawvumeters = properties_bool(mv, "drawvumeters", 1);
		KillTimer(self->component.hwnd, TIMERID_UPDATEVUMETERS);
		if (self->drawvumeters) {
			SetTimer(self->component.hwnd, TIMERID_UPDATEVUMETERS, 50, 0);			
		}
	}
}

void wireview_applyproperties(WireView* self, Properties* p)
{
	self->skin.colour = properties_int(p, "mv_colour", 0x00232323);
	self->skin.wirecolour = properties_int(p, "mv_wirecolour", 0x005F5F5F);
	self->skin.selwirecolour = properties_int(p, "mv_wirecolour", 0x007F7F7F);
	self->skin.polycolour = properties_int(p, "mv_wireaacolour2", 0x005F5F5F);
	self->skin.polycolour = properties_int(p, "mv_polycolour", 0x00B1C8B0);
	self->skin.generator_fontcolour = 
		properties_int(p, "mv_generator_fontcolour", 0x00B1C8B0); // 0x00B1C8B0
	self->skin.effect_fontcolour = 
		properties_int(p, "mv_effect_fontcolour", 0x00D1C5B6);
	self->skin.triangle_size = properties_int(p, "mv_triangle_size", 10);
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
}

void wireview_ondraw(WireView* self, ui_component* sender, ui_graphics* g)
{	
	wireview_drawwires(self, g);		
	wireview_drawmachines(self, g);	
	wireview_drawdragwire(self, g);
}

void wireview_drawwires(WireView* self, ui_graphics* g)
{
	TableIterator it;
	
	for (it = table_begin(&self->machineuis); 
			!tableiterator_equal(&it, table_end()); 
				tableiterator_inc(&it)) {
		wireview_drawwire(self, g, tableiterator_key(&it), 
			(MachineUi*)tableiterator_value(&it));
	}	
}

void wireview_drawwire(WireView* self, ui_graphics* g, int slot,
	MachineUi* outmachineui)
{		
	MachineSockets* sockets;
	WireSocket* p;
	
	sockets	= connections_at(&self->machines->connections, slot);
	if (sockets) {
		for (p = sockets->outputs; p != NULL; p = p->next) {
			WireSocketEntry* entry = (WireSocketEntry*)p->entry;
			if (entry->slot != -1) {				
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
						out.left + (out.right - out.left) / 2,
						out.top + (out.bottom - out.top) / 2,
						in.left + (in.right - in.left) / 2,
						in.top + (in.bottom - in.top) / 2);
					wireview_drawwirearrow(self, g, outmachineui, inmachineui);
				}
			}		
		}
	}
}

void wireview_drawwirearrow(WireView* self, ui_graphics* g,
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
	x1 = outmachineui->x + out.width/2;
	y1 = outmachineui->y + out.height /2;
	x2 = inmachineui->x + in.width/2;
	y2 = inmachineui->y + in.height/2;

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

void wireview_drawdragwire(WireView* self, ui_graphics* g)
{
	if (self->dragslot != -1 &&
			self->dragmode == WIREVIEW_DRAG_NEWCONNECTION) {
		MachineUi* machineui;

		machineui = machineuis_at(self, self->dragslot);
		if (machineui) {
			ui_size machinesize;

			machinesize = machineui_size(machineui);
			ui_setcolor(g, self->skin.wirecolour);
			ui_drawline(g, 
				machineui->x + machinesize.width/2,
				machineui->y + machinesize.height/2,
				self->mx,
				self->my);		
		}
	}
}

void wireview_drawmachines(WireView* self, ui_graphics* g)
{
	TableIterator it;
	
	for (it = table_begin(&self->machineuis); it.curr != 0; 
			tableiterator_inc(&it)) {				
		Machine* machine;

		machine = machines_at(self->machines, tableiterator_key(&it));
		if (machine) {
			MachineUi* machineui;

			machineui = (MachineUi*)tableiterator_value(&it);
			machineui_draw(machineui, self, g, tableiterator_key(&it));
			if (self->selectedslot == tableiterator_key(&it)) {
				machineui_drawhighlight(machineui, self, g);	
			}
		}
	}
}


int slidercoord(SkinCoord* coord, float value)
{	
	return (int)(value * coord->range);
}

void machineui_drawhighlight(MachineUi* self, WireView* wireview, ui_graphics* g)
{	
	ui_rectangle r;
	static int d = 5; // the distance of the highlight from the machine

	r = machineui_position(self);
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
	wireview_align(&self->wireview);
}

void wireview_align(WireView* self)
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

void wireview_onmousedown(WireView* self, ui_component* sender, int x, int y, int button)
{
	ui_component_setfocus(&self->component);
	self->mx = x;
	self->my = y;	
	wireview_hittest(self, x, y);
	if (self->dragslot == -1) {
		if (button == 1) {
			self->selectedwire = wireview_hittestwire(self, x, y);
			ui_invalidate(&self->component);
		}
	} else {		
		if (button == 1) {
			if (self->dragslot != MASTER_INDEX) {
				self->selectedslot = self->dragslot;				
				machines_changeslot(self->machines, self->selectedslot);
			}			
			if (wireview_hittestcoord(self, x, y, MACHMODE_GENERATOR, self->dragslot,
				&self->skin.generator.solo)) {
				
			} else
			if (wireview_hittestcoord(self, x, y, MACHMODE_FX, self->dragslot,
					&self->skin.effect.bypass)) {
				Machine* machine = machines_at(self->machines, self->dragslot);
				if (machine) {
					if (machine_bypassed(machine)) {
						machine_unbypass(machine);
					} else {
						machine_bypass(machine);
					}
				}
			} else
			if (wireview_hittestcoord(self, x, y, MACHMODE_GENERATOR, self->dragslot,
					&self->skin.generator.mute) ||
				wireview_hittestcoord(self, x, y, MACHMODE_FX, self->dragslot,
					&self->skin.effect.mute)) {
				Machine* machine = machines_at(self->machines, self->dragslot);
				if (machine) {
					if (machine_muted(machine)) {
						machine_unmute(machine);
					} else {
						machine_mute(machine);
					}
				}
			} else
			if (wireview_hittestpan(self, x, y, self->dragslot, &self->mx)) {
				self->dragmode = WIREVIEW_DRAG_PAN;				
			} else  {
				MachineUi* machineui;

				machineui = machineuis_at(self, self->dragslot);
				if (machineui) {
					self->dragmode = WIREVIEW_DRAG_MACHINE;				
					self->mx = x - machineui->x;
					self->my = y - machineui->y;
				}
			}
			
		} else
		if (button == 2) {
			Machine* machine;

			machine = machines_at(self->machines, self->dragslot);
			if (machine && machine->numoutputs(machine) > 0) {
				self->dragmode = WIREVIEW_DRAG_NEWCONNECTION;				
			} else {
				self->dragslot = -1;	
			}			
		}
	}
}

int wireview_hittestpan(WireView* self, int x, int y, int slot, int* dx)
{
	int rv = 0;
	Machine* machine;
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
			offset = (int) (machine_panning(machine) * coords->pan.range);	
			ui_setrectangle(&r, coords->pan.destx + offset, coords->pan.desty,
					coords->pan.destwidth, coords->pan.destheight);
			*dx = xm - r.left;
			rv = ui_rectangle_intersect(&r, xm, ym);
		}
	}
	return rv;
}

int wireview_hittestcoord(WireView* self, int x, int y, int mode, int slot, SkinCoord* coord)
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

amp_t wireview_panvalue(WireView* self, int x, int y, int slot)
{
	amp_t rv = 0.f;	
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

void wireview_hittest(WireView* self, int x, int y)
{	
	TableIterator it;
		
	for (it = table_begin(&self->machineuis); it.curr != 0; 
			tableiterator_inc(&it)) {
		ui_rectangle position;	

		position = machineui_position((MachineUi*)tableiterator_value(&it));
		if (ui_rectangle_intersect(&position, self->mx, self->my)) {
			self->dragslot = it.curr->key;
			break;	
		}
	}	
}

void wireview_onmousemove(WireView* self, ui_component* sender, int x, int y, int button)
{
	if (self->dragslot != -1) {
		if (self->dragmode == WIREVIEW_DRAG_PAN) {
			Machine* machine;

			machine = machines_at(self->machines, self->dragslot);
			if (machine) {
				machine_setpanning(machine, wireview_panvalue(self, x, y,
					self->dragslot));
			}
		} else
		if (self->dragmode == WIREVIEW_DRAG_MACHINE) {
			MachineUi* machineui;

			machineui = machineuis_at(self, self->dragslot);
			if (machineui) {
				machineui->x = x - self->mx;
				machineui->y = y - self->my;			
			}
		} else
		if (self->dragmode == WIREVIEW_DRAG_NEWCONNECTION) {
			self->mx = x;
			self->my = y;			
		}
		ui_invalidate(&self->component);		
	}
}

void wireview_onmouseup(WireView* self, ui_component* sender, int x, int y, int button)
{	
	if (self->dragslot != -1) {
		if (self->dragmode == WIREVIEW_DRAG_NEWCONNECTION) {
			int outputslot = self->dragslot;
			self->dragslot = -1;
			wireview_hittest(self, x, y);
			if (self->dragslot != -1) {								
				machines_connect(self->machines, outputslot, self->dragslot, 0);				
			}
		}
	}
	self->dragslot = -1;	
	ui_invalidate(&self->component);
}

void wireview_onmousedoubleclick(WireView* self, ui_component* sender, int x, int y, int button)
{
	self->mx = x;
	self->my = y;	
	wireview_hittest(self, x, y);
	if (self->dragslot == -1) {
		ui_component_propagateevent(sender);
	} else {
		machines_showparameters(self->machines, self->dragslot);		
	}
	self->dragslot = -1;
}

void wireview_onkeydown(WireView* self, ui_component* sender, int keycode, int keydata)
{	
	if (keycode == VK_DELETE && self->selectedwire.src != -1) {						
		machines_disconnect(self->machines, self->selectedwire.src, 
			self->selectedwire.dst);
		ui_invalidate(&self->component);
	} else 
	if (keycode == VK_DELETE && self->selectedslot != - 1 && 
			self->selectedslot != MASTER_INDEX) {		
		machines_remove(self->machines, self->selectedslot);
		self->selectedslot = -1;
	} else {
		ui_component_propagateevent(sender);
	}
}

WireConnection wireview_hittestwire(WireView* self, int x, int y)
{		
	WireConnection rv;
	TableIterator it;
	
	rv.dst = -1;
	rv.src = -1;
	for (it = machines_begin(self->machines); it.curr != 0; 
			tableiterator_inc(&it)) {
		MachineSockets* sockets;
		WireSocket* p;			
		int slot = it.curr->key;
		int done = 0;
	
		sockets	= connections_at(&self->machines->connections, slot);
		if (sockets) {
			p = sockets->outputs;	
			while (p != NULL) {
				WireSocketEntry* entry = (WireSocketEntry*)p->entry;
				if (entry->slot != -1) {					
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

						x1 = outmachineui->x + out.width / 2,
						y1 = outmachineui->y + out.height / 2,
						x2 = inmachineui->x + in.width / 2,
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

void wireview_onmachineschangeslot(WireView* self, Machines* machines,
	int slot)
{
	self->selectedslot = slot;
	ui_invalidate(&self->component);
	ui_component_setfocus(&self->component);
}

void wireview_onmachinesinsert(WireView* self, Machines* machines, int slot)
{
	Machine* machine;

	machine = machines_at(self->machines, slot);
	if (machine) {
		machineuis_insert(self, slot, 0, 0, &self->skin,
			machine->info(machine) ? machine->info(machine)->ShortName : "");
		signal_connect(&machine->signal_worked, self, 
			wireview_onmachineworked);
		ui_invalidate(&self->component);
	}
}

void wireview_onmachinesremoved(WireView* self, Machines* machines, int slot)
{
	machineuis_remove(self, slot);
	ui_invalidate(&self->component);
}

void wireview_updatemachineuis(WireView* self, Properties* machines)
{
	if (machines) {
		Machine* machine;
		Properties* p;

		machineuis_removeall(self);
		p = machines->children;
		while (p) {
			if (p->item.key && strcmp(p->item.key, "machine") == 0) {				
				int x;
				int y;
				int index;
				char* editname;
																							
				index = properties_int(p, "index", 0);
				x = properties_int(p, "x", 0);
				y = properties_int(p, "y", 0);
				properties_readstring(p, "editname", &editname, "");
				machine = machines_at(self->machines, index);
				if (machine) {					
					machineuis_insert(self, index, x, y, &self->skin,
						editname);
					if (index != MASTER_INDEX) {					
						signal_connect(&machine->signal_worked, self,
							wireview_onmachineworked);
					}
				}
			}
			p = properties_next(p);
		}
	}
}

void wireview_onsongchanged(WireView* self, Workspace* workspace)
{		
	self->machines = &workspace->song->machines;	
	wireview_updatemachineuis(self, properties_find(workspace->properties,
		"machines"));	
	wireview_connectmachinessignals(self);	
	ui_invalidate(&self->component);	
}

void wireview_onshowparameters(WireView* self, Machines* sender, int slot)
{	
	if (machineuis_at(self, slot)) {
		machineui_showparameters(machineuis_at(self, slot), &self->component);
	}
}

void wireview_onmachineworked(WireView* self, Machine* machine,
	unsigned int slot, BufferContext* bc)
{
	MachineUi* machineui;

	machineui = machineuis_at(self, slot);
	if (slot != MASTER_INDEX && machineui) {		
		machineui->volumedisplay = buffercontext_rmsvolume(bc) / 32767;				
	}
}

void wireview_ontimer(WireView* self, ui_component* sender, int timerid)
{	
	wireview_preparedrawallmacvus(self);
}

void wireview_preparedrawallmacvus(WireView* self)
{
	TableIterator it;
	
	for (it = table_begin(&self->machineuis);
			!tableiterator_equal(&it, table_end());
				tableiterator_inc(&it)) {		
		MachineUi* machineui;

		machineui = (MachineUi*) tableiterator_value(&it);		
		if (tableiterator_key(&it) != MASTER_INDEX) {
			ui_rectangle r;

			r = machineui_position(machineui);
			ui_invalidaterect(&self->component, &r);	
		}
	}	
}

void machineuis_insert(WireView* self, int slot,
	int x, int y, MachineSkin* skin, const char* editname)
{	
	Machine* machine;

	machine = machines_at(self->machines, slot);
	if (machine) {
		MachineUi* machineui;

		if (table_exists(&self->machineuis, slot)) {
			machineuis_remove(self, slot);
		}	
		machineui = (MachineUi*) malloc(sizeof(MachineUi));
		machineui_init(machineui, x, y, machine, &self->skin, editname);
		table_insert(&self->machineuis, slot, machineui);
	}
}

MachineUi* machineuis_at(WireView* self, int slot)
{
	return table_at(&self->machineuis, slot);
}

void machineuis_remove(WireView* self, int slot)
{
	MachineUi* machineui;

	machineui = (MachineUi*) table_at(&self->machineuis, slot);
	if (machineui) {
		machineui_dispose(machineui);
		free(machineui);
		table_remove(&self->machineuis, slot);
	}
}

void machineuis_removeall(WireView* self)
{
	TableIterator it;

	for (it = table_begin(&self->machineuis);
			!tableiterator_equal(&it, table_end());
				tableiterator_inc(&it)) {
		MachineUi* machineui;

		machineui = (MachineUi*)tableiterator_value(&it);
		machineui_dispose(machineui);
		free(machineui);
	}
	table_dispose(&self->machineuis);
	table_init(&self->machineuis);
}

void machineview_init(MachineView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);		
	ui_component_enablealign(&self->component);
	ui_notebook_init(&self->notebook, &self->component);	
	ui_component_setalign(&self->notebook.component, UI_ALIGN_CLIENT);
	signal_connect(&self->component.signal_show, self, machineview_onshow);
	signal_connect(&self->component.signal_hide, self, machineview_onhide);
	wireview_init(&self->wireview, &self->notebook.component, tabbarparent, workspace);
	InitNewMachine(&self->newmachine, &self->notebook.component, self->workspace);	
	InitTabBar(&self->tabbar, tabbarparent);
	ui_component_setposition(&self->tabbar.component, 450, 0, 160, 20);	
	ui_component_hide(&self->tabbar.component);
	tabbar_append(&self->tabbar, "Wires");
	tabbar_append(&self->tabbar, "New Machine");		
	ui_notebook_setpage(&self->notebook, 0);
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	signal_connect(&self->newmachine.pluginsview.signal_selected, self, wireview_onnewmachineselected);
	signal_connect(&self->component.signal_mousedoubleclick, self, machineview_onmousedoubleclick);
	signal_connect(&self->component.signal_keydown, self, machineview_onkeydown);
}

void machineview_onshow(MachineView* self, ui_component* sender)
{	
	ui_component_show(&self->tabbar.component);
}

void machineview_onhide(MachineView* self, ui_component* sender)
{
	ui_component_hide(&self->tabbar.component);
}

void machineview_onmousedoubleclick(MachineView* self, ui_component* sender,
	int x, int y, int button)
{	
	tabbar_select(&self->tabbar, 1);		
}

void machineview_onkeydown(MachineView* self, ui_component* sender, int keycode,
	int keydata)
{
	if (keycode == VK_ESCAPE) {
		if (tabbar_selected(&self->tabbar) == 1) {
			tabbar_select(&self->tabbar, 0);			
		}
	} else {
		ui_component_propagateevent(sender);
	}
}

void machineview_applyproperties(MachineView* self, Properties* p)
{
	wireview_applyproperties(&self->wireview, p);
}

void wireview_onnewmachineselected(MachineView* self, ui_component* sender,
	Properties* plugininfo)
{	
	Machine* machine;
	char* path;
		
	properties_readstring(plugininfo, "path", &path, "");
	machine = machinefactory_makefrompath(&self->workspace->machinefactory,
		properties_int(plugininfo, "type", -1), path);
	if (machine) {		
		if (self->newmachine.pluginsview.calledbygear) {
			machines_insert(self->wireview.machines,
				machines_slot(self->wireview.machines), machine);
		} else {
			machines_changeslot(self->wireview.machines,
				machines_append(self->wireview.machines, machine));
		}
		tabbar_select(&self->tabbar, 0);			
	}	
}

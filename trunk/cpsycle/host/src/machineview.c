// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "machineview.h"
#include "machines.h"
#include "machinefactory.h"
#include "workspace.h"
#include "resources/resource.h"
#include <math.h>

static void OnShow(MachineView*, ui_component* sender);
static void OnHide(MachineView*, ui_component* sender);
static void OnDraw(WireView*, ui_component* sender, ui_graphics* g);
static void Draw(WireView*, ui_graphics* g);
static void DrawBackground(WireView*, ui_graphics* g);
static void DrawTrackBackground(WireView*, ui_graphics* g, int track);
static void DrawMachines(WireView*, ui_graphics* g);
static void DrawMachine(WireView*, ui_graphics*, int slot, Machine*, int x, int y);
static void DrawMachineHighlight(WireView*, ui_graphics* g, int slot);
static void DrawNewConnectionWire(WireView*, ui_graphics* g);
static void DrawWires(WireView*, ui_graphics* g);
static void DrawWireArrow(WireView*, ui_graphics* g, int outputslot, int inputslot);
static void rotate_point(int x, int y, double phi, int* xr, int *yr);
static void OnSize(WireView*, ui_component* sender, int width, int height);
static void OnDestroy(WireView*, ui_component* component);
static void OnMouseDown(WireView*, ui_component* sender, int x, int y, int button);
static void OnMouseUp(WireView*, ui_component* sender, int x, int y, int button);
static void OnMouseMove(WireView*, ui_component* sender,int x, int y, int button);
static void OnMouseDoubleClick(WireView*, ui_component* sender, int x, int y, int button);
static void OnMachineViewMouseDoubleClick(MachineView*, ui_component* sender, int x, int y, int button);
static void OnKeyDown(WireView*, ui_component* sender, int keycode, int keydata);
static void OnMachineViewKeyDown(MachineView*, ui_component* sender, int keycode, int keydata);
static void HitTest(WireView*, int x, int y);
static int HitTestPan(WireView*, int x, int y, int slot, int* dx);
static float PanValue(WireView*, int x, int y, int slot);
static int OnEnumDrawMachine(WireView*, int slot, Machine*);
static int OnEnumDrawWires(WireView*, int slot, Machine*);
static int OnEnumHitTestMachine(WireView*, int slot, Machine*);
static void OnNewMachineSelected(MachineView*, ui_component* sender, Properties*);
static void InitMachineCoords(WireView*);
static void BlitSkinPart(WireView*, ui_graphics* g, int x, int y, SkinCoord*);
static int SliderCoord(SkinCoord*, float value);
static void MachineUiSize(WireView*, int mode, int* width, int* height);
static void OnMachinesChangeSlot(WireView*, Machines*, int slot);
static void OnMachinesInsert(WireView*, Machines*, int slot);
static void OnMachinesRemoved(WireView*, Machines*, int slot);
static int OnEnumFindWire(WireView*, int slot, Machine*);
static void OnSongChanged(WireView*, Workspace*);
static void UpdateMachineUis(WireView*, Properties* );
static void ConnectMachinesSignals(WireView*);
static void OnMachineViewSize(MachineView*, ui_component* sender, int width, int height);
static void WireViewApplyProperties(WireView*, Properties*);

void MachineUiSet(MachineUi* self, int x, int y, int mode, const char* editname)
{	
	self->x = x;
	self->y = y;
	self->mode = mode;
	if (self->editname) {
		free(self->editname);
	}
	self->editname = strdup(editname);
}

void InitWireView(WireView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{	
	ui_fontinfo fontinfo;

	self->workspace = workspace;
	self->cx = 0;
	self->cy = 0;	
	self->wirefound = 0;
	ui_bitmap_loadresource(&self->skin.skinbmp, IDB_MACHINESKIN);	
	memset(&self->machineuis, 0, sizeof(MachineUi[256]));
	MachineUiSet(&self->machineuis[0], 200, 200, MACHMODE_FX, 0);
	memset(&self->machine_frames, 0, sizeof(ui_component[256]));
	memset(&self->machine_paramviews, 0, sizeof(ParamView[256]));
	ui_component_init(&self->component, parent);
	ui_fontinfo_init(&fontinfo, "Tahoma", 80);
	ui_font_init(&self->skin.font, &fontinfo);
	ui_component_setfont(&self->component, &self->skin.font);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);	
	signal_connect(&self->component.signal_size, self, OnSize);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_mouseup, self, OnMouseUp);
	signal_connect(&self->component.signal_mousemove, self, OnMouseMove);
	signal_connect(&self->component.signal_mousedoubleclick, self,OnMouseDoubleClick);
	signal_connect(&self->component.signal_keydown, self, OnKeyDown);	
	signal_connect(&self->component.signal_draw, self, OnDraw);
	ui_component_move(&self->component, 0, 0);
	self->machines = &workspace->song->machines;
	self->dragslot = -1;
	self->dragmode = WIREVIEW_DRAG_MACHINE;
	self->selectedslot = 0;
	self->component.doublebuffered = TRUE;
	InitMachineCoords(self);	
			
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);	
	ConnectMachinesSignals(self);
}

void ConnectMachinesSignals(WireView* self)
{
	signal_connect(&self->machines->signal_slotchange, self, OnMachinesChangeSlot);
	signal_connect(&self->machines->signal_insert, self, OnMachinesInsert);
	signal_connect(&self->machines->signal_removed, self, OnMachinesRemoved);
}

void Draw(WireView* self, ui_graphics* g)
{
}

void OnDestroy(WireView* self, ui_component* component)
{	
}

void InitMachineCoords(WireView* self)
{	
	MasterCoords master = {
		{ 0, 52, 138, 35, 0, 0, 138, 35, 0 }	// background
	};
	GeneratorCoords generator = {
		{ 0, 87, 138, 52, 0, 0, 138, 52, 0 },		// background
		{ 0, 156, 0, 7, 4, 20, 129, 4, 129 },		// vu0
		{ 108, 156, 1, 7, 6, 33, 82, 7, 82 },		// vupeak
		{ 0, 139, 6, 13, 6, 33, 6, 13, 82 },		// pan
		{ 23, 139, 17, 17, 137, 4, 17, 17, 0 },		// mute
		{ 6, 139, 17, 17, 98, 31, 17, 17, 0 },		// solo
		{ 0, 0, 0, 0, 20, 3, 117, 15, 0 },			// name
	};		
	EffectCoords effect = {
		{ 0, 0, 138, 52, 0, 0, 138, 52, 0 },	// background
		{ 0, 144, 6, 5, 4, 20, 129, 4, 129 },		// vu0
		{ 96, 144, 6, 5, 0, 0, 0, 0, 0 },		// vupeak
		{ 57, 139, 6, 13, 6, 33, 6, 13, 82 },	// pan
		{ 7, 149, 7, 7, 117, 31, 7, 7, 0 },	// mute
		{ 14, 149, 7, 7, 98, 31, 7, 7, 0 },		// bypass
		{ 0, 0, 0, 0, 20, 3, 117, 15, 0 },			// name 
	};			
	self->skin.master = master;
	self->skin.generator = generator;
	self->skin.effect = effect;		
	WireViewApplyProperties(self, 0);
}

void MachineViewApplyProperties(MachineView* self, Properties* properties)
{
	WireViewApplyProperties(&self->wireview, properties);
}

void WireViewApplyProperties(WireView* self, Properties* properties)
{
	properties_readint(properties, "mv_colour", &self->skin.colour, 0x00232323);
	properties_readint(properties, "mv_wirecolour", &self->skin.wirecolour, 0x005F5F5F);
	properties_readint(properties, "mv_wireaacolour2", &self->skin.polycolour, 0x005F5F5F);
	properties_readint(properties, "mv_polycolour", &self->skin.generator_fontcolour, 0x00B1C8B0);
	properties_readint(properties, "mv_generator_fontcolour", &self->skin.generator_fontcolour, 0x00B1C8B0);
	properties_readint(properties, "mv_effect_fontcolour", &self->skin.effect_fontcolour, 0x00D1C5B6);	
	self->skin.wireaacolour 
		= ((((self->skin.wirecolour&0x00ff0000) + ((self->skin.colour&0x00ff0000)*4))/5)&0x00ff0000) +
		  ((((self->skin.wirecolour&0x00ff00) + ((self->skin.colour&0x00ff00)*4))/5)&0x00ff00) +
		  ((((self->skin.wirecolour&0x00ff) + ((self->skin.colour&0x00ff)*4))/5)&0x00ff);
	self->skin.wireaacolour2
		= (((((self->skin.wirecolour&0x00ff0000)) + ((self->skin.colour&0x00ff0000)))/2)&0x00ff0000) +
		  (((((self->skin.wirecolour&0x00ff00)) + ((self->skin.colour&0x00ff00)))/2)&0x00ff00) +
		  (((((self->skin.wirecolour&0x00ff)) + ((self->skin.colour&0x00ff)))/2)&0x00ff);
}


void OnDraw(WireView* self, ui_component* sender, ui_graphics* g)
{	
	DrawBackground(self, g);
	DrawWires(self, g);	
	DrawMachines(self, g);	
	DrawMachineHighlight(self, g, self->selectedslot);
	DrawNewConnectionWire(self, g);	
}

void DrawBackground(WireView* self, ui_graphics* g)
{
	ui_rectangle r;
	
	ui_setrectangle(&r, 0, 0, self->cx, self->cy);
	ui_drawsolidrectangle(g, r, self->skin.colour);	
}

void DrawWires(WireView* self, ui_graphics* g)
{
	self->g = g;	
	machines_enumerate(self->machines, self, OnEnumDrawWires);
}

int OnEnumDrawWires(WireView* self, int slot, Machine* machine)
{		
	MachineConnections* connections;
	MachineConnection* ptr;
	
	connections	= machines_connections(self->machines, slot);	
	ptr = connections->outputs;	
	while (ptr != NULL) {
		MachineConnectionEntry* entry = (MachineConnectionEntry*)ptr->entry;
		if (entry->slot != -1) 
		{
			Machine* outmachine;
			Machine* inmachine;

			outmachine = machines_at(self->machines, slot);
			if (machine) {
				inmachine  = machines_at(self->machines, entry->slot);
				if (inmachine) {
					int outwidth;
					int outheight;
					int inwidth;
					int inheight;
					MachineUiSize(self, outmachine->mode(outmachine), &outwidth, &outheight);
					MachineUiSize(self, inmachine->mode(inmachine), &inwidth, &inheight);
					ui_setcolor(self->g, self->skin.wirecolour);
					ui_drawline(self->g, 
						self->machineuis[slot].x + outwidth/2,
						self->machineuis[slot].y + outheight /2,
						self->machineuis[entry->slot].x + inwidth/2,
						self->machineuis[entry->slot].y + inheight/2);
					DrawWireArrow(self, self->g, slot, entry->slot);
				}
			}
		}
		ptr = ptr->next;
	}
	return 1;
}

void DrawWireArrow(WireView* self, ui_graphics* g, int outputslot, int inputslot)
{
	int outwidth;
	int outheight;
	int inwidth;
	int inheight;
	int x1;
	int y1;
	int x2;
	int y2;
	int centerx;
	int centery;
	double phi;

	ui_point a;
	ui_point b;
	ui_point c;
	ui_point ar;
	ui_point br;
	ui_point cr;

	Machine* outmachine;
	Machine* inmachine;

	outmachine = machines_at(self->machines, outputslot);
	inmachine = machines_at(self->machines, inputslot);
			
	if (!outmachine || !inmachine) {
		return;
	}

	MachineUiSize(self, outmachine->mode(outmachine), &outwidth, &outheight);
	MachineUiSize(self, inmachine->mode(inmachine), &inwidth, &inheight);

	x1 = self->machineuis[outputslot].x + outwidth/2;
	y1 = self->machineuis[outputslot].y + outheight /2;
	x2 = self->machineuis[inputslot].x + inwidth/2;
	y2 = self->machineuis[inputslot].y + inheight/2;
	
	centerx = (x2 - x1) / 2 + x1;
	centery = (y2 - y1) / 2 + y1;

	a.x = -5;
	a.y = 5;
	b.x = 5;
	b.y = 5;
	c.x = 0;
	c.y = -5;

	phi = atan2(x2 - x1, y1 - y2);

	rotate_point(a.x, a.y, phi, &ar.x, &ar.y);
	rotate_point(b.x, b.y, phi, &br.x, &br.y);
	rotate_point(c.x, c.y, phi, &cr.x, &cr.y);

	ui_drawline(g, centerx + ar.x, centery + ar.y, centerx + br.x, centery + br.y);
	ui_drawline(g, centerx + br.x, centery + br.y, centerx + cr.x, centery + cr.y);
	ui_drawline(g, centerx + cr.x, centery + cr.y, centerx + ar.x, centery + ar.y);
}

void rotate_point(int x, int y, double phi, int* xr, int *yr)
{
	*xr = (int) (cos(phi) * x - sin(phi) * y);
	*yr = (int) (sin(phi) * x + cos(phi) * y);
}

void DrawNewConnectionWire(WireView* self, ui_graphics* g)
{
	if (self->dragslot != -1 && self->dragmode == WIREVIEW_DRAG_NEWCONNECTION) {
		int width;
		int height;
		Machine* machine;

		machine = machines_at(self->machines, self->dragslot);
		if (machine) {
			MachineUiSize(self, machine->mode(machine), &width, &height);
			ui_setcolor(self->g, self->skin.wirecolour);
			ui_drawline(g, 
				self->machineuis[self->dragslot].x + width/2,
				self->machineuis[self->dragslot].y + height/2,
				self->mx,
				self->my);
		}
	}
}

void DrawMachines(WireView* self, ui_graphics* g) {
	self->g = g;
	machines_enumerate(self->machines, self, OnEnumDrawMachine);
}

int OnEnumDrawMachine(WireView* self, int slot, Machine* machine)
{
	DrawMachine(self, self->g, slot, machine, self->machineuis[slot].x,
		self->machineuis[slot].y);
	return 1;
}

void DrawMachine(WireView* self, ui_graphics* g, int slot, Machine* machine, int x, int y)
{  		
	char editname[130];
	if (self->machineuis[slot].editname) {
		_snprintf(editname, 130, "%02d:%s", slot, self->machineuis[slot].editname);		
	}
	ui_setbackgroundmode(g, TRANSPARENT);
	if (machine->mode(machine) == MACHMODE_GENERATOR) {
		BlitSkinPart(self, g, x, y, &self->skin.generator.background);
		ui_settextcolor(g, self->skin.generator_fontcolour);
		ui_textout(g, x + self->skin.generator.name.destx + 2, y + self->skin.generator.name.desty  + 2, editname, strlen(editname));
		BlitSkinPart(self, g,
			x + SliderCoord(&self->skin.generator.pan, machine->pan(machine)),
			y,
			&self->skin.generator.pan);
	} else
	if (machine->mode(machine) == MACHMODE_FX) {
		BlitSkinPart(self, g, x, y, &self->skin.effect.background);
		ui_settextcolor(g, self->skin.effect_fontcolour);
		ui_textout(g, x + self->skin.effect.name.destx + 2, y + self->skin.effect.name.desty  + 2, editname, strlen(editname));		
		BlitSkinPart(self, g,
			x + SliderCoord(&self->skin.effect.pan, machine->pan(machine)),
			y,
			&self->skin.effect.pan);
	} else
	if (machine->mode(machine) == MACHMODE_MASTER) {
		BlitSkinPart(self, g, x, y, &self->skin.master.background);
	}	
}

int SliderCoord(SkinCoord* coord, float value)
{	
	return (int)(value * coord->range);
}

void DrawMachineLine(ui_graphics* g, int xdir, int ydir, int x, int y)
{
	int hlength = 9; // the length of the selected machine highlight	

	ui_drawline(g, x, y, x + xdir*hlength, y + ydir*hlength);
}

void DrawMachineHighlight(WireView* self, ui_graphics* g, int slot)
{	
	if (slot != MASTER_INDEX) {
		int width;
		int height;
		int hdistance = 5; // the distance of the highlight from the machine	
		Machine* machine;

		machine = machines_at(self->machines, slot);
		if (machine) {
			MachineUi* ui = &self->machineuis[slot];
			MachineUiSize(self, machine->mode(machine), &width, &height);
			ui_setcolor(g, self->skin.wirecolour);
			DrawMachineLine(g, 1, 0, ui->x - hdistance, ui->y - hdistance);
			DrawMachineLine(g, 0, 1, ui->x - hdistance, ui->y - hdistance);
			DrawMachineLine(g, -1, 0, ui->x + width + hdistance, ui->y - hdistance);
			DrawMachineLine(g, 0, 1, ui->x + width + hdistance, ui->y - hdistance);
			DrawMachineLine(g, 0, -1, ui->x + width + hdistance, ui->y + height + hdistance);
			DrawMachineLine(g, -1, 0, ui->x + width + hdistance, ui->y + height + hdistance);
			DrawMachineLine(g, 1, 0, ui->x - hdistance, ui->y + height + hdistance);
			DrawMachineLine(g, 0, -1, ui->x - hdistance, ui->y + height + hdistance);
		}
	}
}

void BlitSkinPart(WireView* self, ui_graphics* g, int x, int y, SkinCoord* coord)
{
	ui_drawbitmap(g, &self->skin.skinbmp, x + coord->destx, y + coord->desty,
		coord->destwidth, coord->destheight, coord->srcx, coord->srcy);
}

void MachineUiSize(WireView* self, int mode, int* width, int* height)
{
	if (mode == MACHMODE_GENERATOR) {		
		*width = self->skin.generator.background.destwidth;
		*height = self->skin.generator.background.destheight;
	} else
	if (mode == MACHMODE_FX) {
		*width = self->skin.effect.background.destwidth;
		*height = self->skin.effect.background.destheight;
	} else
	if (mode == MACHMODE_MASTER) {
		*width = self->skin.master.background.destwidth;
		*height = self->skin.master.background.destheight;
	}	
}

void OnSize(WireView* self, ui_component* sender, int width, int height)
{
	self->cx = width;
	self->cy = height;	
}

void machineview_align(MachineView* self)
{
	wireview_align(&self->wireview);
}

void wireview_align(WireView* self)
{
	int machinewidth;
	int machineheight;
	ui_size size = ui_component_size(&self->component);

	MachineUiSize(self, MACHMODE_MASTER, &machinewidth, &machineheight);
	self->machineuis[MASTER_INDEX].x = (size.width - machinewidth) / 2 ;
	self->machineuis[MASTER_INDEX].y = (size.height - machineheight) / 2;
}

void OnMouseDown(WireView* self, ui_component* sender, int x, int y, int button)
{
	ui_component_setfocus(&self->component);
	self->mx = x;
	self->my = y;
	HitTest(self, x, y);
	if (self->dragslot != -1) {		
		if (button == 1) {
			if (self->dragslot != MASTER_INDEX) {
				self->selectedslot = self->dragslot;				
				machines_changeslot(self->machines, self->selectedslot);
			}
			if (HitTestPan(self, x, y, self->dragslot, &self->mx)) {
				self->dragmode = WIREVIEW_DRAG_PAN;				
			} else  {
				self->dragmode = WIREVIEW_DRAG_MACHINE;
				self->mx = x - self->machineuis[self->dragslot].x;
				self->my = y - self->machineuis[self->dragslot].y;			
			}
			
		} else
		if (button == 2) {
			self->dragmode = WIREVIEW_DRAG_NEWCONNECTION;		
		}
	}
}

void HitTest(WireView* self, int x, int y)
{	
	machines_enumerate(self->machines, self, OnEnumHitTestMachine);
}

int HitTestPan(WireView* self, int x, int y, int slot, int* dx)
{
	Machine* machine;
	ui_rectangle r = { 0, 0, 0, 0 };
	int offset;

	int xm = x - self->machineuis[slot].x;	
	int ym = y - self->machineuis[slot].y;
	machine = machines_at(self->machines, slot);	
	if (machine) {
		switch (self->machineuis[slot].mode) {
			case MACHMODE_GENERATOR:
				offset = (int) (machine->pan(machine) * self->skin.generator.pan.range);					
				ui_setrectangle(&r,
					self->skin.generator.pan.destx + offset,
						self->skin.generator.pan.desty,
						self->skin.generator.pan.destwidth,
						self->skin.generator.pan.destheight);			
			break;
			case MACHMODE_FX:
				offset = (int) (machine->pan(machine) * self->skin.generator.pan.range);
				ui_setrectangle(&r,
					self->skin.effect.pan.destx + offset,
						self->skin.effect.pan.desty,
						self->skin.effect.pan.destwidth,
						self->skin.effect.pan.destheight);
			break;
			default:		
			break;
		}	
	}
	*dx = xm - r.left;
	return ui_rectangle_intersect(&r, xm, ym);	
}

float PanValue(WireView* self, int x, int y, int slot)
{
	float rv;
	int range;
	int offset;
	int sliderwidth;
		
	switch (self->machineuis[slot].mode) {
		case MACHMODE_GENERATOR:
			range = self->skin.generator.pan.range;
			offset = self->skin.generator.pan.destx;
			sliderwidth = self->skin.generator.pan.destwidth;
		break;
		case MACHMODE_FX:
			range = self->skin.effect.pan.range;
			offset = self->skin.effect.pan.destx;
			sliderwidth = self->skin.effect.pan.destwidth;
		break;
		default:
			range = 0;
			offset = 0;
			sliderwidth = 0;
		break;
	}	
	if (range != 0) {
		rv =  (x - self->machineuis[slot].x - offset - self->mx) / (float)range;		
	} else {
		rv = 0.f;
	}
	return rv;
}

int OnEnumHitTestMachine(WireView* self, int slot, Machine* machine)
{
	int width;
	int height;

	MachineUiSize(self, machine->mode(machine), &width, &height);
	if (self->mx >= self->machineuis[slot].x &&
		self->my >= self->machineuis[slot].y &&
		self->mx < self->machineuis[slot].x + width &&
		self->my < self->machineuis[slot].y + height) {
			self->dragslot = slot;						
			return 0;
	}		
	return 1;
}

void OnMouseMove(WireView* self, ui_component* sender, int x, int y, int button)
{
	if (self->dragslot != -1) {
		if (self->dragmode == WIREVIEW_DRAG_PAN) {
			Machine* machine;

			machine = machines_at(self->machines, self->dragslot);
			if (machine) {
				machine->setpan(machine,
					PanValue(self, x, y,
						self->dragslot));
			}
		} else
		if (self->dragmode == WIREVIEW_DRAG_MACHINE) {
			self->machineuis[self->dragslot].x = x - self->mx;
			self->machineuis[self->dragslot].y = y - self->my;			
		} else
		if (self->dragmode == WIREVIEW_DRAG_NEWCONNECTION) {
			self->mx = x;
			self->my = y;			
		}
		ui_invalidate(&self->component);		
	}
}

void OnMouseUp(WireView* self, ui_component* sender, int x, int y, int button)
{	
	if (self->dragslot != -1) {
		if (self->dragmode == WIREVIEW_DRAG_NEWCONNECTION) {
			int outputslot = self->dragslot;
			self->dragslot = -1;
			HitTest(self, x, y);
			if (self->dragslot != -1) {								
				machines_connect(self->machines, outputslot, self->dragslot);				
			}
		}
	}
	self->dragslot = -1;	
	ui_invalidate(&self->component);
}


void OnMouseDoubleClick(WireView* self, ui_component* sender, int x, int y, int button)
{
	self->mx = x;
	self->my = y;	
	machines_enumerate(self->machines, self, OnEnumHitTestMachine);
	if (self->dragslot == -1) {
		self->component.propagateevent = 1;
	} else {
		if (self->machine_frames[self->dragslot].component.hwnd != 0) {
			ui_component_show(&self->machine_frames[self->dragslot].component);
		} else {
			MachineFrame* frame;
			ParamView* paramview;

			frame = &self->machine_frames[self->dragslot];
			InitMachineFrame(frame, &self->component);
			paramview = &self->machine_paramviews[self->dragslot];			
			InitParamView(paramview, &frame->component, machines_at(self->machines, self->dragslot));
			MachineFrameSetParamView(frame, paramview);
			ui_component_show(&frame->component);
			ui_component_setfocus(&paramview->component);
		}
	}
	self->dragslot = -1;
}

void OnKeyDown(WireView* self, ui_component* sender, int keycode, int keydata)
{	
	int state;

	state = GetKeyState (VK_LBUTTON);
	if (state < 0 && keycode == VK_DELETE) {
		self->wirefound = 0;
		machines_enumerate(self->machines, self, OnEnumFindWire);
		if (self->wirefound) {
			machines_disconnect(self->machines, self->wiresrc, self->wiredst);			
			ui_invalidate(&self->component);
		}
	} else 
	if (keycode == VK_DELETE && self->selectedslot != MASTER_INDEX) {
		machines_remove(self->machines, self->selectedslot);
	} else {
		ui_component_propagateevent(sender);
	}
}

int OnEnumFindWire(WireView* self, int slot, Machine* machine)
{		
	MachineConnections* connections;
	MachineConnection* ptr;
	
	connections	= machines_connections(self->machines, slot);	
	ptr = connections->outputs;	
	while (ptr != NULL) {
		MachineConnectionEntry* entry = (MachineConnectionEntry*)ptr->entry;
		if (entry->slot != -1) 
		{		
			Machine* outmachine;
			Machine* inmachine;

			outmachine = machines_at(self->machines, slot);
			if (machine) {
				inmachine  = machines_at(self->machines, entry->slot);
				if (inmachine) {
					int outwidth;
					int outheight;
					int inwidth;
					int inheight;
					int x1, x2, y1, y2;
					float m;
					int b;
					int y;

					MachineUiSize(self, outmachine->mode(outmachine), &outwidth, &outheight);
					MachineUiSize(self, inmachine->mode(inmachine), &inwidth, &inheight);
					
					x1 = self->machineuis[slot].x + outwidth/2,
					y1 = self->machineuis[slot].y + outheight /2,
					x2 = self->machineuis[entry->slot].x + inwidth/2,
					y2 = self->machineuis[entry->slot].y + inheight/2;
				
					self->wiresrc = slot;
					self->wiredst = entry->slot;

					if (x2 - x1 != 0) {
						m = (y2 - y1) / (float)(x2 - x1);
						b = y1 - (int) (m * x1);
						y = (int)(m * self->mx) + b;
					} else {											
						if (abs(self->mx - x1) < 10 &&
							self->my >= y1 && self->my <= y2) {	
							self->wirefound = 1;
							return 0;
						} else {
							return 1;
						}
					}
					if (abs(self->my - y) < 10) {						
						self->wirefound = 1;
						return 0;					
					}
				}
			}
		}
		ptr = ptr->next;
	}
	return 1;
}

void OnMachinesChangeSlot(WireView* self, Machines* machines, int slot)
{
	self->selectedslot = slot;
	ui_invalidate(&self->component);
	ui_component_setfocus(&self->component);
}

void OnMachinesInsert(WireView* self, Machines* machines, int slot)
{
	Machine* machine;

	machine = machines_at(self->machines, slot);
	if (machine) {
		MachineUiSet(&self->machineuis[slot], 0, 0,
			machine->mode(machine),
			machine->info(machine)
			? machine->info(machine)->ShortName
			: "");
	}
}

void OnMachinesRemoved(WireView* self, Machines* machines, int slot)
{
	ui_invalidate(&self->component);
}

void UpdateMachineUis(WireView* self, Properties* machines)
{
	if (machines) {
		Machine* machine;
		Properties* p;

		p = machines->children;
		while (p) {
			if (p->item.key && strcmp(p->item.key, "machine") == 0) {				
				int index;
				properties_readint(p, "machine", &index, 0);									
				if (p->children) {
					Properties* q;
					int x;
					int y;
					int mode;
					char* editname;

					machine = machines_at(self->machines, index);					
					mode = (machine) ? machine->mode(machine) : MACHMODE_FX;					
					
					q = p->children;
					properties_readint(q, "x", &x, 0);
					properties_readint(q, "y", &y, 0);					
					properties_readstring(q, "editname", &editname, "");
					MachineUiSet(&self->machineuis[index], x, y, mode, editname);	
				}				
			}
			p = p->next;
		}
	}
}

void InitMachineView(MachineView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_notebook_init(&self->notebook, &self->component);
	signal_connect(&self->component.signal_show, self, OnShow);
	signal_connect(&self->component.signal_hide, self, OnHide);
	InitWireView(&self->wireview, &self->notebook.component, tabbarparent, workspace);
	InitNewMachine(&self->newmachine, &self->notebook.component, self->workspace);
	signal_connect(&self->component.signal_size, self, OnMachineViewSize);
	InitTabBar(&self->tabbar, tabbarparent);
	ui_component_move(&self->tabbar.component, 450, 0);
	ui_component_resize(&self->tabbar.component, 160, 20);
	ui_component_hide(&self->tabbar.component);
	tabbar_append(&self->tabbar, "Wires");
	tabbar_append(&self->tabbar, "New Machine");	
	ui_notebook_setpage(&self->notebook, 0);
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	signal_connect(&self->newmachine.signal_selected, self, OnNewMachineSelected);
	signal_connect(&self->component.signal_mousedoubleclick, self,OnMachineViewMouseDoubleClick);
	signal_connect(&self->component.signal_keydown, self,OnMachineViewKeyDown);
}

void OnSongChanged(WireView* self, Workspace* workspace)
{	
	self->machines = &workspace->song->machines;	
	UpdateMachineUis(self, properties_find(workspace->properties, "machines"));	
	ConnectMachinesSignals(self);	
	ui_invalidate(&self->component);
}

void OnMachineViewSize(MachineView* self, ui_component* sender, int width, int height)
{
	ui_component_resize(&self->notebook.component, width, height);
}

void OnNewMachineSelected(MachineView* self, ui_component* sender,
	Properties* plugininfo)
{
	Machine* machine;
	
	machine = machinefactory_make(&self->workspace->machinefactory,
		properties_value(plugininfo), properties_key(plugininfo));
	if (machine) {		
		machines_changeslot(self->wireview.machines,
			machines_append(self->wireview.machines, machine));
		self->tabbar.selected = 0;
		ui_invalidate(&self->tabbar.component);
		ui_notebook_setpage(&self->notebook, 0);
	}
}

void OnShow(MachineView* self, ui_component* sender)
{	
	ui_component_show(&self->tabbar.component);
}

void OnHide(MachineView* self, ui_component* sender)
{
	ui_component_hide(&self->tabbar.component);
}

void OnMachineViewMouseDoubleClick(MachineView* self, ui_component* sender, int x, int y, int button)
{
	ui_notebook_setpage(&self->notebook, 1);
	self->tabbar.selected = 1;
	ui_invalidate(&self->tabbar.component);
}

void OnMachineViewKeyDown(MachineView* self, ui_component* sender, int keycode, int keydata)
{
	if (keycode == VK_ESCAPE) {
		if (self->tabbar.selected == 1) {
			ui_notebook_setpage(&self->notebook, 0);
			self->tabbar.selected = 0;
			ui_invalidate(&self->tabbar.component);
		}
	}
}
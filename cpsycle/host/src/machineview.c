// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineview.h"
#include "resources/resource.h"
#include <math.h>

#define TIMERID_UPDATEVUMETERS 300

static void machineui_init(MachineUi*, int x, int y, MachineCoords*, int mode,
	const char* editname);
static void machineui_dispose(MachineUi*);
static ui_size machineui_size(MachineUi*);
static ui_rectangle machineui_position(MachineUi*);

static void OnShow(MachineView*, ui_component* sender);
static void OnHide(MachineView*, ui_component* sender);
static void OnDraw(WireView*, ui_component* sender, ui_graphics*);
static void DrawBackground(WireView*, ui_graphics*);
static void DrawTrackBackground(WireView*, ui_graphics*, int track);
static void DrawMachines(WireView*, ui_graphics*);
static void DrawMachine(WireView*, ui_graphics*, int slot, Machine*, int x, int y);
static void DrawMachineVu(WireView* self, ui_graphics*, int slot, Machine* machine, int x, int y);
static void DrawMachineHighlight(WireView*, ui_graphics*, int slot);
static void DrawNewConnectionWire(WireView*, ui_graphics*);
static void DrawWires(WireView*, ui_graphics*);
static void DrawWire(WireView*, ui_graphics*, int slot, Machine*);
static void DrawWireArrow(WireView*, ui_graphics*, int outputslot, int inputslot);
static ui_point rotate_point(ui_point, double phi);
static ui_point move_point(ui_point pt, ui_point d);
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
static int HitTestCoord(WireView*, int x, int y, int mode, int slot, SkinCoord*);
static WireConnection HitTestWire(WireView*, int x, int y);
static float PanValue(WireView*, int x, int y, int slot);
static void InvalidateMachineVu(WireView*, int slot, Machine*);
static void OnNewMachineSelected(MachineView*, ui_component* sender, Properties*);
static void InitMachineCoords(WireView*);
static void BlitSkinPart(WireView*, ui_graphics*, int x, int y, SkinCoord*);
static int SliderCoord(SkinCoord*, float value);
static void OnMachinesChangeSlot(WireView*, Machines*, int slot);
static void OnMachinesInsert(WireView*, Machines*, int slot);
static void OnMachinesRemoved(WireView*, Machines*, int slot);
static void OnSongChanged(WireView*, Workspace*);
static void UpdateMachineUis(WireView*, Properties* );
static void ConnectMachinesSignals(WireView*);
static void WireViewApplyProperties(WireView*, Properties*);
static void OnShowParameters(WireView*, Machines*, int slot);
static void OnMachineWorked(WireView*, Machine*, unsigned int slot,
	BufferContext*);
static void OnTimer(WireView*, ui_component* sender, int timerid);
static void PrepareDrawAllMacVus(WireView*);
static void OnConfigChanged(WireView*, Workspace*, Properties*);
static void ReadConfig(WireView*);
static MachineCoords* machinecoords(WireView* self, int mode);

void machineui_init(MachineUi* self, int x, int y, MachineCoords* coords,
	int mode, const char* editname)
{	
	self->x = x;
	self->y = y;
	self->coords = coords;
	if (mode == MACHMODE_MASTER) {
		self->mode = MACHMODE_MASTER;		
	} else {
		self->mode = (mode & 3) ? MACHMODE_GENERATOR : MACHMODE_FX;
	}
	if (self->editname) {
		free(self->editname);
	}
	self->editname = _strdup(editname);
	self->volumedisplay = 0.f;
}

void machineui_dispose(MachineUi* self)
{
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

void InitWireView(WireView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{	
	ui_fontinfo fontinfo;

	self->workspace = workspace;	
	self->drawvumeters = 1;		
	ui_bitmap_loadresource(&self->skin.skinbmp, IDB_MACHINESKIN);	
	memset(&self->machineuis, 0, sizeof(MachineUi[256]));
	machineui_init(&self->machineuis[MASTER_INDEX],	200, 200,			
		machinecoords(self, MACHMODE_MASTER), MACHMODE_MASTER, 0);	
	memset(&self->machine_frames, 0, sizeof(ui_component[256]));
	memset(&self->machine_paramviews, 0, sizeof(ParamView[256]));
	ui_component_init(&self->component, parent);
	ui_fontinfo_init(&fontinfo, "Tahoma", 80);
	ui_font_init(&self->skin.font, &fontinfo);
	ui_component_setfont(&self->component, &self->skin.font);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_mouseup, self, OnMouseUp);
	signal_connect(&self->component.signal_mousemove, self, OnMouseMove);
	signal_connect(&self->component.signal_mousedoubleclick, self,OnMouseDoubleClick);
	signal_connect(&self->component.signal_keydown, self, OnKeyDown);	
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_timer, self, OnTimer);
	ui_component_move(&self->component, 0, 0);
	self->machines = &workspace->song->machines;
	self->dragslot = -1;
	self->dragmode = WIREVIEW_DRAG_MACHINE;
	self->selectedslot = MASTER_INDEX;
	self->selectedwire.src = -1;
	self->selectedwire.dst = -1;
	self->component.doublebuffered = TRUE;
	InitMachineCoords(self);			
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);	
	ConnectMachinesSignals(self);
	signal_connect(&workspace->signal_configchanged, self, OnConfigChanged);	
	SetTimer(self->component.hwnd, TIMERID_UPDATEVUMETERS, 50, 0);
}

void ConnectMachinesSignals(WireView* self)
{
	signal_connect(&self->machines->signal_slotchange, self, OnMachinesChangeSlot);
	signal_connect(&self->machines->signal_insert, self, OnMachinesInsert);
	signal_connect(&self->machines->signal_removed, self, OnMachinesRemoved);
	signal_connect(&self->machines->signal_showparameters, self, OnShowParameters);
}

void OnDestroy(WireView* self, ui_component* component)
{	
}

void InitMachineCoords(WireView* self)
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
	WireViewApplyProperties(self, 0);
}

void OnConfigChanged(WireView* self, Workspace* workspace, Properties* property)
{
	if (property == workspace->config) {
		ReadConfig(self);
	} else
	if (strcmp(properties_key(property), "drawvumeters") == 0) {
		self->drawvumeters = properties_value(property);
	}
}

void ReadConfig(WireView* self)
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

void MachineViewApplyProperties(MachineView* self, Properties* p)
{
	WireViewApplyProperties(&self->wireview, p);
}

void WireViewApplyProperties(WireView* self, Properties* p)
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
}

void OnDraw(WireView* self, ui_component* sender, ui_graphics* g)
{			
	DrawBackground(self, g);	
	DrawWires(self, g);		
	DrawMachines(self, g);		
	if (self->selectedslot != -1) {
		DrawMachineHighlight(self, g, self->selectedslot);
	}
	DrawNewConnectionWire(self, g);	
}

void DrawBackground(WireView* self, ui_graphics* g)
{
	ui_rectangle r;
	
	ui_setrectangle(&r, g->clip.left, g->clip.top, 
		g->clip.right - g->clip.left, g->clip.bottom - g->clip.top);
	ui_drawsolidrectangle(g, r, self->skin.colour);	
}

void DrawWires(WireView* self, ui_graphics* g)
{
	TableIterator it;
	
	for (it = machines_begin(self->machines); it.curr != 0; 
			tableiterator_inc(&it)) {
		DrawWire(self, g, it.curr->key, (Machine*)it.curr->value);
	}	
}

void DrawWire(WireView* self, ui_graphics* g, int slot, Machine* machine)
{		
	MachineConnections* connections;
	MachineConnection* p;
	
	connections	= machines_connections(self->machines, slot);		
	for (p = connections->outputs; p != NULL; p = p->next) {
		MachineConnectionEntry* entry = (MachineConnectionEntry*)p->entry;
		if (entry->slot != -1) {
			ui_rectangle out;
			ui_rectangle in;
			
			out = machineui_position(&self->machineuis[slot]);
			in = machineui_position(&self->machineuis[entry->slot]);
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
			DrawWireArrow(self, g, slot, entry->slot);			
		}		
	}
}

void DrawWireArrow(WireView* self, ui_graphics* g, int outputslot, int inputslot)
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

	out = machineui_size(&self->machineuis[outputslot]);
	in = machineui_size(&self->machineuis[inputslot]);
	x1 = self->machineuis[outputslot].x + out.width/2;
	y1 = self->machineuis[outputslot].y + out.height /2;
	x2 = self->machineuis[inputslot].x + in.width/2;
	y2 = self->machineuis[inputslot].y + in.height/2;
	
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

void DrawNewConnectionWire(WireView* self, ui_graphics* g)
{
	if (self->dragslot != -1 && self->dragmode == WIREVIEW_DRAG_NEWCONNECTION) {
		ui_size machinesize;	
				
		machinesize = machineui_size(&self->machineuis[self->dragslot]);
		ui_setcolor(g, self->skin.wirecolour);
		ui_drawline(g, 
			self->machineuis[self->dragslot].x + machinesize.width/2,
			self->machineuis[self->dragslot].y + machinesize.height/2,
			self->mx,
			self->my);		
	}
}

void DrawMachines(WireView* self, ui_graphics* g)
{
	TableIterator it;
	
	for (it = machines_begin(self->machines); it.curr != 0; 
			tableiterator_inc(&it)) {
		DrawMachine(self, g, it.curr->key, (Machine*)it.curr->value, 
			self->machineuis[it.curr->key].x, self->machineuis[it.curr->key].y);
	}
}

void DrawMachine(WireView* self, ui_graphics* g, int slot, Machine* machine, 
	int x, int y)
{		
	if (ui_rectangle_intersect(&g->clip, x, y) && 
			self->machineuis[slot].coords) {
		MachineCoords* coords;
		char editname[130];

		coords = self->machineuis[slot].coords;		
		if (self->machineuis[slot].editname) {
			_snprintf(editname, 130, "%02d:%s", slot, self->machineuis[slot].editname);		
		}
		ui_setbackgroundmode(g, TRANSPARENT);		
		BlitSkinPart(self, g, x, y, &self->machineuis[slot].coords->background);
		if (machine->mode(machine) == MACHMODE_FX) {			
			ui_settextcolor(g, self->skin.effect_fontcolour);
		} else {		
			ui_settextcolor(g, self->skin.generator_fontcolour);;
		}
		if (machine->mode(machine) != MACHMODE_MASTER) {			
			ui_textout(g, x + coords->name.destx + 2, y + coords->name.desty + 2,
				editname, strlen(editname));
			BlitSkinPart(self, g, x + SliderCoord(&coords->pan, 
				machine_panning(machine)), y, &coords->pan);
			if (machine_muted(machine)) {
				BlitSkinPart(self, g, x, y, &coords->mute);
			}
			if (machine_bypassed(machine)) {
				BlitSkinPart(self, g, x, y, &coords->bypass);
			}
		}		
		if (self->drawvumeters) {
			DrawMachineVu(self, g, slot, machine, x, y);
		}
	}
}

void DrawMachineVu(WireView* self, ui_graphics* g, int slot, Machine* machine,
	int x, int y)
{
	MachineCoords* coords = self->machineuis[slot].coords;
	if (coords && self->machineuis[slot].mode != MACHMODE_MASTER) {	
		SkinCoord* vu;
		int width;

		vu = &coords->vu0;		
		width = (int)(self->machineuis[slot].volumedisplay * vu->destwidth);
		ui_drawbitmap(g, &self->skin.skinbmp, x + vu->destx, y + vu->desty,
			width, vu->destheight, vu->srcx, vu->srcy);	
	}
}

void BlitSkinPart(WireView* self, ui_graphics* g, int x, int y,
	SkinCoord* coord)
{
	ui_drawbitmap(g, &self->skin.skinbmp, x + coord->destx, y + coord->desty,
		coord->destwidth, coord->destheight, coord->srcx, coord->srcy);
}

int SliderCoord(SkinCoord* coord, float value)
{	
	return (int)(value * coord->range);
}

void DrawMachineLine(ui_graphics* g, int xdir, int ydir, int x, int y)
{
	int hlength = 9; // the length of the selected machine highlight	

	ui_drawline(g, x, y, x + xdir * hlength, y + ydir * hlength);
}

void DrawMachineHighlight(WireView* self, ui_graphics* g, int slot)
{	
	if (slot != MASTER_INDEX) {
		ui_rectangle r;
		static int d = 5; // the distance of the highlight from the machine

		r = machineui_position(&self->machineuis[slot]);
		ui_setcolor(g, self->skin.wirecolour);
		DrawMachineLine(g, 1, 0, r.left - d, r.top - d);
		DrawMachineLine(g, 0, 1, r.left - d, r.top - d);
		DrawMachineLine(g, -1, 0, r.right + d, r.top - d);
		DrawMachineLine(g, 0, 1, r.right + d, r.top - d);
		DrawMachineLine(g, 0, -1, r.right + d, r.bottom + d);
		DrawMachineLine(g, -1, 0, r.right + d, r.bottom + d);
		DrawMachineLine(g, 1, 0, r.left - d, r.bottom + d);
		DrawMachineLine(g, 0, -1, r.left - d, r.bottom + d);		
	}
}

void machineview_align(MachineView* self)
{
	wireview_align(&self->wireview);
}

void wireview_align(WireView* self)
{
	ui_size machinesize;
	ui_size size = ui_component_size(&self->component);

	machinesize = machineui_size(&self->machineuis[MASTER_INDEX]);
	self->machineuis[MASTER_INDEX].x = (size.width - machinesize.width) / 2 ;
	self->machineuis[MASTER_INDEX].y = (size.height - machinesize.height) / 2;
}

void OnMouseDown(WireView* self, ui_component* sender, int x, int y, int button)
{
	ui_component_setfocus(&self->component);
	self->mx = x;
	self->my = y;	
	HitTest(self, x, y);
	if (self->dragslot == -1) {
		if (button == 1) {
			self->selectedwire = HitTestWire(self, x, y);
			ui_invalidate(&self->component);
		}
	} else {		
		if (button == 1) {
			if (self->dragslot != MASTER_INDEX) {
				self->selectedslot = self->dragslot;				
				machines_changeslot(self->machines, self->selectedslot);
			}			
			if (HitTestCoord(self, x, y, MACHMODE_GENERATOR, self->dragslot,
				&self->skin.generator.solo)) {
				
			} else
			if (HitTestCoord(self, x, y, MACHMODE_FX, self->dragslot,
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
			if (HitTestCoord(self, x, y, MACHMODE_GENERATOR, self->dragslot,
					&self->skin.generator.mute) ||
				HitTestCoord(self, x, y, MACHMODE_FX, self->dragslot,
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
			if (HitTestPan(self, x, y, self->dragslot, &self->mx)) {
				self->dragmode = WIREVIEW_DRAG_PAN;				
			} else  {
				self->dragmode = WIREVIEW_DRAG_MACHINE;
				self->mx = x - self->machineuis[self->dragslot].x;
				self->my = y - self->machineuis[self->dragslot].y;			
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

int HitTestPan(WireView* self, int x, int y, int slot, int* dx)
{
	int rv = 0;
	Machine* machine;
	ui_rectangle r;
	int offset;
	MachineCoords* coords;

	int xm = x - self->machineuis[slot].x;	
	int ym = y - self->machineuis[slot].y;
	machine = machines_at(self->machines, slot);
	coords = self->machineuis[slot].coords;
	if (coords) {
		offset = (int) (machine_panning(machine) * coords->pan.range);	
		ui_setrectangle(&r, coords->pan.destx + offset, coords->pan.desty,
					coords->pan.destwidth, coords->pan.destheight);
		*dx = xm - r.left;
		rv = ui_rectangle_intersect(&r, xm, ym);
	}
	return rv;
}

int HitTestCoord(WireView* self, int x, int y, int mode, int slot, SkinCoord* coord)
{
	if (self->machineuis[slot].mode != mode) {
		ui_rectangle r = { 0, 0, 0, 0 };	
		int xm = x - self->machineuis[slot].x;	
		int ym = y - self->machineuis[slot].y;
			
		ui_setrectangle(&r, coord->destx, coord->desty,
			coord->destwidth, coord->destheight);
		return ui_rectangle_intersect(&r, xm, ym);
	}
	return 0;
}

float PanValue(WireView* self, int x, int y, int slot)
{
	float rv;
	MachineCoords* coords;

	coords = self->machineuis[slot].coords;
	if (coords && coords->pan.range != 0) {
		rv =  (x - self->machineuis[slot].x - coords->pan.destx - self->mx) / 
			(float)coords->pan.range;		
	} else {
		rv = 0.f;
	}
	return rv;
}

void HitTest(WireView* self, int x, int y)
{	
	TableIterator it;
		
	for (it = machines_begin(self->machines); it.curr != 0; 
			tableiterator_inc(&it)) {
		ui_rectangle position;	

		position = machineui_position(&self->machineuis[it.curr->key]);
		if (ui_rectangle_intersect(&position, self->mx, self->my)) {
			self->dragslot = it.curr->key;
			break;	
		}
	}	
}

void OnMouseMove(WireView* self, ui_component* sender, int x, int y, int button)
{
	if (self->dragslot != -1) {
		if (self->dragmode == WIREVIEW_DRAG_PAN) {
			Machine* machine;

			machine = machines_at(self->machines, self->dragslot);
			if (machine) {
				machine_setpanning(machine, PanValue(self, x, y,
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
	HitTest(self, x, y);	
	if (self->dragslot == -1) {
		ui_component_propagateevent(sender);
	} else {
		machines_showparameters(self->machines, self->dragslot);		
	}
	self->dragslot = -1;
}

void OnKeyDown(WireView* self, ui_component* sender, int keycode, int keydata)
{	
	if (keycode == VK_DELETE && self->selectedwire.src != -1) {						
		machines_disconnect(self->machines, self->selectedwire.src, 
			self->selectedwire.dst);
		ui_invalidate(&self->component);
	} else 
	if (keycode == VK_DELETE && self->selectedslot != MASTER_INDEX) {		
		machines_remove(self->machines, self->selectedslot);		
	} else {
		ui_component_propagateevent(sender);
	}
}

WireConnection HitTestWire(WireView* self, int x, int y)
{		
	WireConnection rv;
	TableIterator it;
	
	rv.dst = -1;
	rv.src = -1;
	for (it = machines_begin(self->machines); it.curr != 0; 
			tableiterator_inc(&it)) {
		MachineConnections* connections;
		MachineConnection* p;			
		int slot = it.curr->key;
		int done = 0;
	
		connections	= machines_connections(self->machines, slot);	
		p = connections->outputs;	
		while (p != NULL) {
			MachineConnectionEntry* entry = (MachineConnectionEntry*)p->entry;
			if (entry->slot != -1) {					
				ui_size out;
				ui_size in;								
				int x1, x2, y1, y2;
				float m;
				int b;
				int y3;				

				out = machineui_size(&self->machineuis[slot]);
				in = machineui_size(&self->machineuis[entry->slot]);				

				x1 = self->machineuis[slot].x + out.width / 2,
				y1 = self->machineuis[slot].y + out.height / 2,
				x2 = self->machineuis[entry->slot].x + in.width / 2,
				y2 = self->machineuis[entry->slot].y + in.height / 2;							
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
			if (done) {
				break;
			}
			p = p->next;
		}
	}
	return rv;
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
		machineui_init(&self->machineuis[slot], 0, 0,
			machinecoords(self, machine->mode(machine)),
			machine->mode(machine),
			machine->info(machine) ? machine->info(machine)->ShortName : "");
		signal_connect(&machine->signal_worked, self, OnMachineWorked);
		ui_invalidate(&self->component);
	}
}

MachineCoords* machinecoords(WireView* self, int mode)
{
	MachineCoords* rv;

	switch (mode) {
		case MACHMODE_MASTER: 
			rv = &self->skin.master;
		break;	
		case MACHMODE_GENERATOR:
			rv = &self->skin.generator;
		break;
		default:
			rv = &self->skin.effect;
		break;
	}
	return rv;
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
				int x;
				int y;
				int mode;
				int index;
				char* editname;
																							
				index = properties_int(p, "index", 0);
				x = properties_int(p, "x", 0);
				y = properties_int(p, "y", 0);
				properties_readstring(p, "editname", &editname, "");
				machine = machines_at(self->machines, index);					
				mode = (machine) ? machine->mode(machine) : MACHMODE_FX;
				machineui_init(&self->machineuis[index], x, y, 
					machinecoords(self, machine->mode(machine)),
					mode, editname);
				if (machine && index != MASTER_INDEX) {					
					signal_connect(&machine->signal_worked, self,
						OnMachineWorked);
				}
			}
			p = properties_next(p);
		}
	}
}

void InitMachineView(MachineView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);		
	ui_component_enablealign(&self->component);
	ui_notebook_init(&self->notebook, &self->component);	
	ui_component_setalign(&self->notebook.component, UI_ALIGN_CLIENT);
	signal_connect(&self->component.signal_show, self, OnShow);
	signal_connect(&self->component.signal_hide, self, OnHide);
	InitWireView(&self->wireview, &self->notebook.component, tabbarparent, workspace);
	InitNewMachine(&self->newmachine, &self->notebook.component, self->workspace);	
	InitTabBar(&self->tabbar, tabbarparent);
	ui_component_setposition(&self->tabbar.component, 450, 0, 160, 20);	
	ui_component_hide(&self->tabbar.component);
	tabbar_append(&self->tabbar, "Wires");
	tabbar_append(&self->tabbar, "New Machine");		
	ui_notebook_setpage(&self->notebook, 0);
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	signal_connect(&self->newmachine.pluginsview.signal_selected, self, OnNewMachineSelected);
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

void OnNewMachineSelected(MachineView* self, ui_component* sender,
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

void OnShow(MachineView* self, ui_component* sender)
{	
	ui_component_show(&self->tabbar.component);
}

void OnHide(MachineView* self, ui_component* sender)
{
	ui_component_hide(&self->tabbar.component);
}

void OnMachineViewMouseDoubleClick(MachineView* self, ui_component* sender,
	int x, int y, int button)
{	
	tabbar_select(&self->tabbar, 1);		
}

void OnMachineViewKeyDown(MachineView* self, ui_component* sender, int keycode,
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

void OnShowParameters(WireView* self, Machines* sender, int slot)
{
	if (!machines_at(self->machines, slot)) {
		return;
	}
	if (self->machine_frames[slot].component.hwnd != 0) {
		ui_component_show(&self->machine_frames[slot].component);
	} else {
		MachineFrame* frame;
		ParamView* paramview;

		frame = &self->machine_frames[slot];
		InitMachineFrame(frame, &self->component);
		paramview = &self->machine_paramviews[slot];			
		InitParamView(paramview, &frame->component, 
			machines_at(self->machines, slot));
		MachineFrameSetParamView(frame, paramview);
		ui_component_show(&frame->component);
		ui_component_setfocus(&paramview->component);
	}	
}

void OnMachineWorked(WireView* self, Machine* machine, unsigned int slot,
	BufferContext* bc)
{
	self->machineuis[slot].volumedisplay = buffercontext_rmsvolume(bc) / 32767;
}

void OnTimer(WireView* self, ui_component* sender, int timerid)
{	
	PrepareDrawAllMacVus(self);
}

void PrepareDrawAllMacVus(WireView* self)
{
	TableIterator it;
	
	for (it = machines_begin(self->machines); it.curr != 0; 
			tableiterator_inc(&it)) {
		ui_rectangle r;

		r = machineui_position(&self->machineuis[it.curr->key]);
		ui_invalidaterect(&self->component, &r);	
	}	
}

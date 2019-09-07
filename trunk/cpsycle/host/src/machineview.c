// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "machineview.h"
#include "machines.h"
#include <machinefactory.h>
#include "resources/resource.h"

extern MachineCallback machinecallback;

static void OnDraw(MachineView* self, ui_component* sender, ui_graphics* g);
static void Draw(MachineView* self, ui_graphics* g);
static void DrawBackground(MachineView* self, ui_graphics* g);
static void DrawTrackBackground(MachineView* self, ui_graphics* g, int track);
static void DrawMachines(MachineView* self, ui_graphics* g);
static void DrawMachine(MachineView* self, ui_graphics* g, int slot, Machine* machine, int x, int y);
static void DrawMachineHighlight(MachineView* self, ui_graphics* g, int slot);
static void DrawNewConnectionWire(MachineView* self, ui_graphics* g);
static void DrawWires(MachineView* self, ui_graphics* g);
static void OnSize(MachineView* self, ui_component* sender, int width, int height);
static void OnDestroy(MachineView* self, ui_component* component);
static void OnMouseDown(MachineView* self, ui_component* sender, int x, int y, int button);
static void OnMouseUp(MachineView* self, ui_component* sender, int x, int y, int button);
static void OnMouseMove(MachineView* self, ui_component* sender,int x, int y, int button);
static void OnMouseDoubleClick(MachineView* self, ui_component* sender, int x, int y, int button);
static void OnKeyDown(MachineView* self, ui_component* sender, int keycode, int keydata);
static void HitTest(MachineView* self, int x, int y);
static int OnEnumDrawMachine(MachineView* self, int slot, Machine* machine);
static int OnEnumDrawWires(MachineView* self, int slot, Machine* machine);
static int OnEnumHitTestMachine(MachineView* self, int slot, Machine* machine);
static void OnPluginSelected(MachineView* self, CMachineInfo* plugin, const char* path);
static void InitMachineCoords(MachineView* self);
static void BlitSkinPart(MachineView* self, ui_graphics* g, int x, int y,
	SkinCoord* coord);
static void MachineUiSize(MachineView* self, int mode, int* width, int* height);
static void OnMachinesChangeSlot(MachineView* self, Machines* machines, int slot);

extern HINSTANCE appInstance;

void MachineUiSet(MachineUi* self, int x, int y, const char* editname)
{	
	self->x = x;
	self->y = y;
	if (self->editname) {
		free(self->editname);
	}
	self->editname = strdup(editname);
}

void InitMachineView(MachineView* self, ui_component* parent, MachineBar* machinebar, Player* player, Properties* properties)
{				
	self->cx = 0;
	self->cy = 0;
	self->machinebar = machinebar;
	self->skin.skinbmp.hBitmap = LoadBitmap (appInstance, MAKEINTRESOURCE(IDB_MACHINESKIN));	
	if (self->skin.hfont == NULL) {
		self->skin.hfont = ui_createfont("Tahoma", 12);
	}	
	memset(&self->machineuis, 0, sizeof(MachineUi[256]));
	MachineUiSet(&self->machineuis[0], 200, 200, 0);
	memset(&self->machine_frames, 0, sizeof(ui_component[256]));
	memset(&self->machine_paramviews, 0, sizeof(ParamView[256]));
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_destroy, self, OnDestroy);	
	signal_connect(&self->component.signal_size, self, OnSize);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_mouseup, self, OnMouseUp);
	signal_connect(&self->component.signal_mousemove, self, OnMouseMove);
	signal_connect(&self->component.signal_mousedoubleclick, self,OnMouseDoubleClick);
	signal_connect(&self->component.signal_keydown, self, OnKeyDown);	
	signal_connect(&self->component.signal_draw, self, OnDraw);
	ui_component_move(&self->component, 0, 0);
	self->player = player;			
	self->dragslot = -1;
	self->dragmode = MACHINEVIEW_DRAG_MACHINE;
	self->selectedslot = 0;
	InitNewMachine(&self->newmachine, &self->component, player, properties);
	ui_component_hide(&self->newmachine.component);
	self->newmachine.selected = OnPluginSelected;
	self->component.doublebuffered = TRUE;
	InitMachineCoords(self);
	self->machinebar = machinebar;	
	signal_connect(&self->player->song->machines.signal_slotchange, self, OnMachinesChangeSlot);
}

void Draw(MachineView* self, ui_graphics* g)
{
}

void OnDestroy(MachineView* self, ui_component* component)
{	
}

void InitMachineCoords(MachineView* self)
{	
	MasterCoords master = {
		{ 0, 0, 148, 47, 0, 0, 148, 47 }   /* background */
	};
	GeneratorCoords generator = {
		{ 0, 47, 148, 47, 0, 0, 148, 47 }, /* background */
		{ 0, 144, 6, 5, 8, 3, 96, 0 },	   /* vu0		 */	
		{ 96, 144, 6, 5, 0, 0, 0, 0 },	   /* vupeak	 */
		{ 21, 149, 24, 9, 3, 35, 117, 0 }, /* pan		 */
		{ 7, 149, 7, 7, 137, 4, 137, 17 }, /* mute	     */	
		{ 14, 149, 7, 7, 0, 0, 0, 0 },	   /* solo       */
		{ 0, 0, 0, 0, 49, 7, 0, 0 },	   /* name       */
	};		
	EffectCoords effect = {
		{ 0, 94, 148, 48, 0, 0, 147, 48 }, /* background */
		{ 0, 144, 6, 5, 8, 3, 96, 0 },	   /* vu0		 */	
		{ 96, 144, 6, 5, 0, 0, 0, 0 },	   /* vupeak	 */
		{ 21, 149, 24, 9, 3, 35, 117, 0 }, /* pan		 */
		{ 7, 149, 7, 7, 137, 4, 137, 17 }, /* mute	     */	
		{ 14, 149, 7, 7, 0, 0, 0, 0 },	   /* bypass     */
		{ 0, 0, 0, 0, 49, 7, 0, 0 },	   /* name       */
	};			
	self->skin.master = master;
	self->skin.generator = generator;
	self->skin.effect = effect;		
	MachineViewApplyProperties(self, 0);
}

void MachineViewApplyProperties(MachineView* self, Properties* properties)
{
	properties_readint(properties, "mv_colour", &self->skin.colour, 0x009a887c);
	properties_readint(properties, "mv_wirecolour", &self->skin.wirecolour, 0x00000000);
	properties_readint(properties, "mv_wireaacolour2", &self->skin.polycolour, 0x00400000);
	properties_readint(properties, "mv_polycolour", &self->skin.generator_fontcolour, 0x00ffffff);
	properties_readint(properties, "mv_generator_fontcolour", &self->skin.generator_fontcolour, 0x00000000);
	properties_readint(properties, "mv_generatort_fontcolour", &self->skin.generator_fontcolour, 0x00000000);	
	self->skin.wireaacolour 
		= ((((self->skin.wirecolour&0x00ff0000) + ((self->skin.colour&0x00ff0000)*4))/5)&0x00ff0000) +
		  ((((self->skin.wirecolour&0x00ff00) + ((self->skin.colour&0x00ff00)*4))/5)&0x00ff00) +
		  ((((self->skin.wirecolour&0x00ff) + ((self->skin.colour&0x00ff)*4))/5)&0x00ff);
	self->skin.wireaacolour2
		= (((((self->skin.wirecolour&0x00ff0000)) + ((self->skin.colour&0x00ff0000)))/2)&0x00ff0000) +
		  (((((self->skin.wirecolour&0x00ff00)) + ((self->skin.colour&0x00ff00)))/2)&0x00ff00) +
		  (((((self->skin.wirecolour&0x00ff)) + ((self->skin.colour&0x00ff)))/2)&0x00ff);
}


void OnDraw(MachineView* self, ui_component* sender, ui_graphics* g)
{	   	
	if (self->skin.hfont) {
		ui_setfont(g, self->skin.hfont);
	}
	DrawBackground(self, g);
	DrawWires(self, g);	
	DrawMachines(self, g);	
	DrawMachineHighlight(self, g, self->selectedslot);
	DrawNewConnectionWire(self, g);	
}

void DrawBackground(MachineView* self, ui_graphics* g)
{
	ui_rectangle r;
	
	ui_setrectangle(&r, 0, 0, self->cx, self->cy);
	ui_drawsolidrectangle(g, r, self->skin.colour);	
}

void DrawWires(MachineView* self, ui_graphics* g)
{
	self->g = g;	
	machines_enumerate(&self->player->song->machines, self, OnEnumDrawWires);
}

int OnEnumDrawWires(MachineView* self, int slot, Machine* machine)
{		
	MachineConnections* connections;
	MachineConnection* ptr;
	
	connections	= machines_connections(&self->player->song->machines, slot);	
	ptr = &connections->outputs;	
	while (ptr != NULL) {
		if (ptr->slot != -1) 
		{
			Machine* outmachine;
			Machine* inmachine;

			outmachine = machines_at(&self->player->song->machines, slot);
			if (machine) {
				inmachine  = machines_at(&self->player->song->machines, ptr->slot);
				if (inmachine) {
					int outwidth;
					int outheight;
					int inwidth;
					int inheight;
					MachineUiSize(self, outmachine->mode(outmachine), &outwidth, &outheight);
					MachineUiSize(self, inmachine->mode(inmachine), &inwidth, &inheight);
					ui_drawline(self->g, 
						self->machineuis[slot].x + outwidth/2,
						self->machineuis[slot].y + outheight /2,
						self->machineuis[ptr->slot].x + inwidth/2,
						self->machineuis[ptr->slot].y + inheight/2);
				}
			}
		}
		ptr = ptr->next;
	}
	return 1;
}

void DrawNewConnectionWire(MachineView* self, ui_graphics* g)
{
	if (self->dragslot != -1 && self->dragmode == MACHINEVIEW_DRAG_NEWCONNECTION) {
		int width;
		int height;
		Machine* machine;

		machine = machines_at(&self->player->song->machines, self->dragslot);
		if (machine) {
			MachineUiSize(self, machine->mode(machine), &width, &height);
			ui_drawline(g, 
				self->machineuis[self->dragslot].x + width/2,
				self->machineuis[self->dragslot].y + height/2,
				self->mx,
				self->my);
		}
	}
}

void DrawMachines(MachineView* self, ui_graphics* g) {
	self->g = g;
	machines_enumerate(&self->player->song->machines, self, OnEnumDrawMachine);
}

int OnEnumDrawMachine(MachineView* self, int slot, Machine* machine)
{
	DrawMachine(self, self->g, slot, machine, self->machineuis[slot].x,
		self->machineuis[slot].y);
	return 1;
}

void DrawMachine(MachineView* self, ui_graphics* g, int slot, Machine* machine, int x, int y)
{  		
	char editname[130];
	if (self->machineuis[slot].editname) {
		_snprintf(editname, 130, "%02d:%s", slot, self->machineuis[slot].editname);		
	}
	SetBkMode(g->hdc, TRANSPARENT);
	if (machine->mode(machine) == MACHMODE_GENERATOR) {		
		BlitSkinPart(self, g, x, y, &self->skin.generator.background);
		ui_textout(g, x + self->skin.generator.name.destx + 2, y + self->skin.generator.name.desty  + 2, editname, strlen(editname));
	} else
	if (machine->mode(machine) == MACHMODE_FX) {
		BlitSkinPart(self, g, x, y, &self->skin.effect.background);
		ui_textout(g, x + self->skin.effect.name.destx + 2, y + self->skin.effect.name.desty  + 2, editname, strlen(editname));		
	} else
	if (machine->mode(machine) == MACHMODE_MASTER) {
		BlitSkinPart(self, g, x, y, &self->skin.master.background);
	}		
}

void DrawMachineLine(ui_graphics* g, int xdir, int ydir, int x, int y)
{
	int hlength = 9; //the length of the selected machine highlight	

	ui_drawline(g, x, y, x + xdir*hlength, y + ydir*hlength);
}

void DrawMachineHighlight(MachineView* self, ui_graphics* g, int slot)
{	
	if (slot != 0) {
		int width;
		int height;
		int hdistance = 5; //the distance of the highlight from the machine	
		Machine* machine;

		machine = machines_at(&self->player->song->machines, slot);
		if (machine) {
			MachineUi* ui = &self->machineuis[slot];
			MachineUiSize(self, machine->mode(machine), &width, &height);		           
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

void BlitSkinPart(MachineView* self, ui_graphics* g, int x, int y, SkinCoord* coord)
{
	ui_drawbitmap(g, &self->skin.skinbmp, x + coord->destx, y + coord->desty,
		coord->destwidth, coord->destheight, coord->srcx, coord->srcy);
}

void MachineUiSize(MachineView* self, int mode, int* width, int* height)
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

void OnSize(MachineView* self, ui_component* sender, int width, int height)
{
	self->cx = width;
	self->cy = height;
	ui_component_resize(&self->newmachine.component, width, height);
}

void OnMouseDown(MachineView* self, ui_component* sender, int x, int y, int button)
{
	ui_component_setfocus(&self->component);
	self->mx = x;
	self->my = y;
	HitTest(self, x, y);
	if (self->dragslot != -1) {		
		if (button == 1) {
			if (self->dragslot != 0) {
				self->selectedslot = self->dragslot;
				if (self->machinebar) {
					machines_changeslot(&self->player->song->machines, self->selectedslot);					
				}
			}
			self->dragmode = MACHINEVIEW_DRAG_MACHINE;			
			self->mx = x - self->machineuis[self->dragslot].x;
			self->my = y - self->machineuis[self->dragslot].y;			
		} else
		if (button == 2) {
			self->dragmode = MACHINEVIEW_DRAG_NEWCONNECTION;		
		}
	}
}

void HitTest(MachineView* self, int x, int y)
{	
	machines_enumerate(&self->player->song->machines, self, OnEnumHitTestMachine);
}

int OnEnumHitTestMachine(MachineView* self, int slot, Machine* machine)
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

void OnMouseUp(MachineView* self, ui_component* sender, int x, int y, int button)
{	
	if (self->dragslot != -1) {
		if (self->dragmode == MACHINEVIEW_DRAG_NEWCONNECTION) {
			int outputslot = self->dragslot;
			self->dragslot = -1;
			HitTest(self, x, y);
			if (self->dragslot != -1) {				
				Machine* machine;
				machines_connect(&self->player->song->machines, outputslot, self->dragslot);
				machine = machines_at(&self->player->song->machines, outputslot);				
			}
		}
	}
	self->dragslot = -1;	
	ui_invalidate(&self->component);	
}

void OnMouseMove(MachineView* self, ui_component* sender, int x, int y, int button)
{
	if (self->dragslot != -1) {
		if (self->dragmode == MACHINEVIEW_DRAG_MACHINE) {
			self->machineuis[self->dragslot].x = x - self->mx;
			self->machineuis[self->dragslot].y = y - self->my;			
		} else
		if (self->dragmode == MACHINEVIEW_DRAG_NEWCONNECTION) {
			self->mx = x;
			self->my = y;			
		}
		ui_invalidate(&self->component);		
	}
}

void OnMouseDoubleClick(MachineView* self, ui_component* sender, int x, int y, int button)
{
	self->mx = x;
	self->my = y;	
	machines_enumerate(&self->player->song->machines, self, OnEnumHitTestMachine);
	if (self->dragslot == -1) {
		ui_component_show(&self->newmachine.component);
		ui_component_setfocus(&self->newmachine.component);
	} else {
		if (self->machine_frames[self->dragslot].component.hwnd != 0) {
			ui_component_show(&self->machine_frames[self->dragslot].component);
		} else {
			MachineFrame* frame;
			ParamView* paramview;

			frame = &self->machine_frames[self->dragslot];
			InitMachineFrame(frame, &self->component);
			paramview = &self->machine_paramviews[self->dragslot];			
			InitParamView(paramview, &frame->component, machines_at(&self->player->song->machines, self->dragslot));
			MachineFrameSetParamView(frame, paramview);
			ui_component_show(&frame->component);
			ui_component_setfocus(&paramview->component);
		}
	}
	self->dragslot = -1;
}

void OnPluginSelected(MachineView* self, CMachineInfo* info, const char* path)
{
	Machine* machine;
	
	machine = machinefactory_make(machinecallback, info, path);
	if (machine) {
		int slot;
		
		slot = machines_append(&self->player->song->machines, machine);
		MachineUiSet(&self->machineuis[slot], 0, 0, info->ShortName);
		ui_invalidate(&self->component);		
	}
}

void OnKeyDown(MachineView* self, ui_component* sender, int keycode, int keydata)
{	
	ui_component_propagateevent(sender);
}

void OnMachinesChangeSlot(MachineView* self, Machines* machines, int slot)
{
	self->selectedslot = slot;
	ui_invalidate(&self->component);
	ui_component_setfocus(&self->component);
}

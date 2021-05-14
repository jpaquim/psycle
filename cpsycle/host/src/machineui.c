// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineui.h"
// host
#include "skingraphics.h"
#include "paramview.h"
#include "wireview.h"
#include "effectui.h"
#include "generatorui.h"
#include "masterui.h"
// audio
#include <exclusivelock.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static void drawmachineline(psy_ui_Graphics*, psy_ui_RealPoint dir,
	psy_ui_RealPoint edge);

// VuValues
void vuvalues_init(VuValues* self)
{
	self->volumedisplay = (psy_dsp_amp_t)0.f;
	self->volumemaxdisplay = (psy_dsp_amp_t)0.f;
	self->volumemaxcounterlife = 0;
}

void vuvalues_update(VuValues* self, psy_audio_Buffer* buffer)
{		
	self->volumedisplay = (buffer)
		? psy_audio_buffer_rmsdisplay(buffer)
		: (psy_dsp_amp_t)0.f;
	vuvalues_tickcounter(self);
}

void vuvalues_tickcounter(VuValues* self)
{
	--self->volumemaxcounterlife;
	if ((self->volumedisplay > self->volumemaxdisplay)
		|| (self->volumemaxcounterlife <= 0)) {
		self->volumemaxdisplay = self->volumedisplay - 1 / 32768.f;
		self->volumemaxcounterlife = 60;
	}
}

// VuDisplay
// prototypes

static void vudisplay_drawdisplay(VuDisplay*, psy_ui_Graphics*);
static void vudisplay_drawpeak(VuDisplay*, psy_ui_Graphics*);

// implementation
void vudisplay_init(VuDisplay* self, MachineViewSkin* skin,
	MachineCoords* coords)
{
	self->skin = skin;
	self->coords = coords;	
	vuvalues_init(&self->vuvalues);
}

void vudisplay_update(VuDisplay* self, psy_audio_Buffer* buffer)
{
	vuvalues_update(&self->vuvalues, buffer);
}

void vudisplay_draw(VuDisplay* self, psy_ui_Graphics* g)
{
	assert(self);
	
	vudisplay_drawdisplay(self, g);
	vudisplay_drawpeak(self, g);	
}

void vudisplay_drawdisplay(VuDisplay* self, psy_ui_Graphics* g)
{		
	psy_ui_drawbitmap(g, &self->skin->skinbmp,
		psy_ui_realrectangle_make(
			psy_ui_realrectangle_topleft(&self->coords->vu0.dest),
			psy_ui_realsize_make(
				self->vuvalues.volumedisplay *
				psy_ui_realrectangle_width(&self->coords->vu0.dest),
				psy_ui_realrectangle_height(&self->coords->vu0.dest))),
		psy_ui_realrectangle_topleft(&self->coords->vu0.src));	
}

void vudisplay_drawpeak(VuDisplay* self, psy_ui_Graphics* g)
{
	if (self->vuvalues.volumemaxdisplay > 0.01f) {
		SkinCoord* vupeak;
		SkinCoord* vu;

		vupeak = &self->coords->vupeak;
		vu = &self->coords->vu0;
		psy_ui_drawbitmap(g, &self->skin->skinbmp,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					vu->dest.left + self->vuvalues.volumemaxdisplay *
					psy_ui_realrectangle_width(&vu->dest),
					vu->dest.top),
				psy_ui_realrectangle_size(&vupeak->src)),
			psy_ui_realrectangle_topleft(&vupeak->src));
	}
}

static bool vuupdate = FALSE;

// MachineUiCommon
// implementation
void machineuicommon_init(MachineUiCommon* self,
	uintptr_t slot, MachineViewSkin* skin,
	psy_ui_Component* view, ParamViews* paramviews,
	Workspace* workspace)
{
	self->machines = &workspace->song->machines;
	self->machine = psy_audio_machines_at(self->machines, slot);
	self->paramviews = paramviews;
	self->workspace = workspace;
	self->view = view;
	self->skin = skin;	
	self->mode = psy_audio_machine_mode(self->machine);
	self->coords = NULL;
	self->slot = slot;	
	self->restorename = NULL;
	self->machinepos = TRUE;
	self->drawmode = MACHINEUIMODE_BITMAP;
	self->dragmode = MACHINEVIEW_DRAG_NONE;	
}

void machineuicommon_move(MachineUiCommon* self, psy_ui_Point topleft)
{
	assert(self);
	
	if (self->machine && self->machinepos) {
		psy_ui_RealPoint topleftpx;

		topleftpx.x = psy_ui_value_px(&topleft.x,
			psy_ui_component_textmetric(self->view), NULL);
		topleftpx.y = psy_ui_value_px(&topleft.y,
			psy_ui_component_textmetric(self->view), NULL);
		psy_audio_machine_setposition(self->machine,
			topleftpx.x, topleftpx.y);
	}	
}

// static methods
void machineui_beginvuupdate(void)
{
	vuupdate = TRUE;
}

void machineui_endvuupdate(void)
{
	vuupdate = FALSE;
}

bool machineui_vuupdate(void)
{
	return vuupdate;
}

psy_ui_Component* machineui_create(psy_audio_Machine* machine, 
	uintptr_t slot, MachineViewSkin* skin, psy_ui_Component* parent,
	psy_ui_Component* view, ParamViews* paramviews, bool machinepos,
	MachineUiMode drawmode, Workspace* workspace)
{	
	psy_ui_Component* newui;
		
	newui = NULL;
	if (slot == psy_audio_MASTER_INDEX) {
		MasterUi* masterui;

		masterui = (MasterUi*)malloc(sizeof(MasterUi));
		if (masterui) {
			masterui_init(masterui, parent, skin, view, paramviews, workspace);
			masterui->intern.machinepos = machinepos;
			newui = &masterui->component;
		}
	} else if (slot >= 0x40 && slot <= 0x80) {
		EffectUi* effectui;

		effectui = (EffectUi*)malloc(sizeof(EffectUi));
		if (effectui) {
			effectui_init(effectui, parent, slot, skin, view, paramviews, workspace);
			effectui->intern.machinepos = machinepos;
			effectui_setdrawmode(effectui, drawmode);			
			newui = &effectui->component;
		}		
	} else {
		GeneratorUi* generatorui;

		generatorui = (GeneratorUi*)malloc(sizeof(GeneratorUi));
		if (generatorui) {
			generatorui_init(generatorui, parent, slot, skin, view, paramviews, workspace);
			generatorui->intern.machinepos = machinepos;
			generatorui->intern.drawmode = drawmode;
			newui = &generatorui->component;
		}
	}	
	if (newui) {
		newui->deallocate = TRUE;
	}
	return newui;	
}

void machineui_drawhighlight(psy_ui_Graphics* g, psy_ui_RealRectangle position)
{
	static double d = 5.0; // the distance of the highlight from the machine
	static psy_ui_RealPoint dirs[] = {
		{ 1.0, 0.0 }, { 0.0, 1.0 }, { -1.0, 0.0 }, { 0.0, -1.0 }
	};
	psy_ui_RealPoint edges[4];
	psy_ui_RealPoint origin;
	double width;
	double height;
	
	width = psy_ui_realrectangle_width(&position);
	height = psy_ui_realrectangle_height(&position);
	edges[0] = psy_ui_realpoint_make(-d, -d);
	edges[1] = psy_ui_realpoint_make(width + d, -d);
	edges[2] = psy_ui_realpoint_make(width + d, height + d);
	edges[3] = psy_ui_realpoint_make(-d, height + d);
	origin = psy_ui_origin(g);
	psy_ui_setorigin(g,
		psy_ui_realpoint_make(
			origin.x - position.left,
			origin.y - position.top));
	drawmachineline(g, dirs[0], edges[0]);
	drawmachineline(g, dirs[1], edges[0]);
	drawmachineline(g, dirs[1], edges[1]);
	drawmachineline(g, dirs[2], edges[1]);
	drawmachineline(g, dirs[2], edges[2]);
	drawmachineline(g, dirs[3], edges[2]);
	drawmachineline(g, dirs[3], edges[3]);
	drawmachineline(g, dirs[0], edges[3]);
	psy_ui_setorigin(g, origin);
}



void drawmachineline(psy_ui_Graphics* g, psy_ui_RealPoint dir,
	psy_ui_RealPoint edge)
{
	static intptr_t hlength = 9; // the length of the selected machine highlight	

	psy_ui_drawline(g, edge, psy_ui_realpoint_make(
		edge.x + dir.x * hlength, edge.y + dir.y * hlength));
}

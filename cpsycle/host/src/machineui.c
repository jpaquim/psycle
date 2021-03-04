// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineui.h"
// host
#include "skingraphics.h"
#include "paramview.h"
#include "wireview.h"
// audio
#include <exclusivelock.h>
// ui
#include <uiviewcomponentimp.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

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
	psy_ui_realrectangle_init(&self->position);
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

static void drawmachineline(psy_ui_Graphics*, psy_ui_RealPoint dir,
	psy_ui_RealPoint edge);

static bool vuupdate = FALSE;

// MachineUiCommon
// prototypes
static void machineuicommon_move(MachineUiCommon*, psy_ui_Point topleft);
// implementation
void machineuicommon_init(MachineUiCommon* self,
	uintptr_t slot, MachineViewSkin* skin,
	psy_ui_Component* view, psy_ui_Edit* editname, Workspace* workspace)
{
	self->machines = &workspace->song->machines;
	self->machine = psy_audio_machines_at(self->machines, slot);
	self->workspace = workspace;
	self->view = view;
	self->skin = skin;
	self->editname = editname;
	self->mode = psy_audio_machine_mode(self->machine);
	self->coords = NULL;
	self->slot = slot;
	self->machineframe = NULL;
	self->paramview = NULL;
	self->editorview = NULL;
	self->restorename = NULL;
	self->machinepos = TRUE;
	self->dragmode = MACHINEVIEW_DRAG_NONE;	
}

void machineuicommon_move(MachineUiCommon* self, psy_ui_Point topleft)
{
	assert(self);
	
	if (self->machine && self->machinepos) {
		psy_ui_RealPoint topleftpx;

		topleftpx.x = psy_ui_value_px(&topleft.x,
			psy_ui_component_textmetric(self->view));
		topleftpx.y = psy_ui_value_px(&topleft.y,
			psy_ui_component_textmetric(self->view));
		psy_audio_machine_setposition(self->machine,
			topleftpx.x, topleftpx.y);
	}	
}

// MasterUi
// prototypes
static void masterui_dispose(MasterUi*);
static void masterui_initsize(MasterUi*);
static void masterui_ondraw(MasterUi*, psy_ui_Graphics*);
static void masterui_drawbackground(MasterUi*, psy_ui_Graphics*);
static void masterui_onmousedoubleclick(MasterUi*, psy_ui_MouseEvent*);
static void masterui_drawhighlight(MasterUi*, psy_ui_Graphics*);
static void masterui_onframedestroyed(MasterUi*, psy_ui_Component* sender);
static void masterui_move(MasterUi*, psy_ui_Point topleft);
static void masterui_invalidate(MasterUi*);
static void masterui_onshowparameters(MasterUi*, Workspace* sender,
	uintptr_t slot);
static void masterui_showparameters(MasterUi*, psy_ui_Component* parent);
static void masterui_onpreferredsize(MasterUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable masterui_vtable;
static psy_ui_ComponentVtable masterui_super_vtable;
static bool masterui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* masterui_vtable_init(MasterUi* self)
{
	assert(self);

	if (!masterui_vtable_initialized) {
		masterui_vtable = *(self->component.vtable);
		masterui_super_vtable = masterui_vtable;
		masterui_vtable.dispose = (psy_ui_fp_component_dispose)masterui_dispose;
		masterui_vtable.ondraw = (psy_ui_fp_component_ondraw)masterui_ondraw;		
		masterui_vtable.onmousedoubleclick = (psy_ui_fp_component_onmouseevent)
			masterui_onmousedoubleclick;
		masterui_vtable.move = (psy_ui_fp_component_move)masterui_move;
		masterui_vtable.invalidate = (psy_ui_fp_component_invalidate)
			masterui_invalidate;
		masterui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			masterui_onpreferredsize;
		masterui_vtable_initialized = TRUE;
	}
	return &masterui_vtable;
}
// implementation
void masterui_init(MasterUi* self, psy_ui_Component* parent, MachineViewSkin* skin,
	psy_ui_Component* view, Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(workspace->song);
	assert(skin);
	assert(view);

	self->component.imp = (psy_ui_ComponentImp*)
		psy_ui_viewcomponentimp_allocinit(
			&self->component, (parent) ? parent->imp : NULL,
			view, "", 0, 0, 100, 100, 0, 0);
	psy_ui_component_init_imp(&self->component, view,
		self->component.imp);
	masterui_vtable_init(self);
	self->component.vtable = &masterui_vtable;
	machineuicommon_init(&self->intern, psy_audio_MASTER_INDEX, skin, view,
		NULL, workspace);
	self->intern.coords = &skin->master;
	self->intern.font = skin->effect_fontcolour;
	self->intern.bgcolour = psy_ui_colour_make(0x00333333);
	masterui_initsize(self);
	psy_signal_connect(&workspace->signal_showparameters, self,
		masterui_onshowparameters);
}

void masterui_dispose(MasterUi* self)
{
	assert(self);

	if (self->intern.paramview) {
		psy_ui_component_destroy(&self->intern.paramview->component);
		free(self->intern.paramview);
	}
	if (self->intern.machineframe) {
		psy_ui_component_destroy(&self->intern.machineframe->component);
		free(self->intern.machineframe);
	}
	free(self->intern.restorename);
	psy_signal_disconnect(&self->intern.workspace->signal_showparameters, self,
		masterui_onshowparameters);
	masterui_super_vtable.dispose(&self->component);
}

void masterui_initsize(MasterUi* self)
{
	psy_ui_RealPoint topleft;
	psy_ui_RealSize size;

	size = psy_ui_realrectangle_size(&self->intern.coords->background.dest);
	if (self->intern.machine && self->intern.machinepos) {
		psy_audio_machine_position(self->intern.machine, &topleft.x, &topleft.y);
	} else {
		topleft = psy_ui_realpoint_zero();
	}
	psy_ui_component_setposition(&self->component,
		psy_ui_rectangle_make(
			psy_ui_point_makepx(topleft.x, topleft.y),
			psy_ui_size_makepx(size.width, size.height)));	
}

void masterui_move(MasterUi* self, psy_ui_Point topleft)
{
	assert(self);

	masterui_super_vtable.move(&self->component, topleft);
	machineuicommon_move(&self->intern, topleft);	
}

psy_ui_RealRectangle masterui_coordposition(MasterUi* self, SkinCoord* coord)
{
	psy_ui_RealRectangle r;

	assert(self);

	r = psy_ui_component_position(&self->component);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			r.left + coord->dest.left,
			r.top + coord->dest.top),
		psy_ui_realrectangle_size(&coord->dest));
}

void masterui_ondraw(MasterUi* self, psy_ui_Graphics* g)
{
	assert(self);

	masterui_drawbackground(self, g);	
}

void masterui_drawbackground(MasterUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (!psy_ui_bitmap_empty(&self->intern.skin->skinbmp)) {
		skin_blitcoord(g, &self->intern.skin->skinbmp, psy_ui_realpoint_zero(),
			&self->intern.coords->background);
	} else {
		psy_ui_RealRectangle r;

		r = psy_ui_component_position(&self->component);
		psy_ui_drawsolidrectangle(g, r, self->intern.bgcolour);		
		psy_ui_textoutrectangle(g,
			psy_ui_realrectangle_topleft(&self->intern.coords->name.dest),
			psy_ui_ETO_CLIPPED, self->intern.coords->name.dest, "Master",
			strlen("Master"));		
	}
}

void masterui_showparameters(MasterUi* self, psy_ui_Component* parent)
{
	if (self->intern.machine) {
		if (!self->intern.machineframe) {
			self->intern.machineframe = machineframe_alloc();
			machineframe_init(self->intern.machineframe, parent, self->intern.workspace);
			psy_signal_connect(&self->intern.machineframe->component.signal_destroy,
				self, masterui_onframedestroyed);
			if (psy_audio_machine_haseditor(self->intern.machine)) {
				MachineEditorView* editorview;

				editorview = machineeditorview_allocinit(
					psy_ui_notebook_base(&self->intern.machineframe->notebook),
					self->intern.machine, self->intern.workspace);
				if (editorview) {
					machineframe_setview(self->intern.machineframe,
						&editorview->component, self->intern.machine);
				}
			} else {
				ParamView* paramview;

				paramview = paramview_allocinit(
					&self->intern.machineframe->notebook.component,
					self->intern.machine, self->intern.workspace);
				if (paramview) {
					machineframe_setparamview(self->intern.machineframe, paramview,
						self->intern.machine);
				}
			}
		}
		if (self->intern.machineframe) {
			psy_ui_component_show(&self->intern.machineframe->component);
		}
	}
}

void masterui_onframedestroyed(MasterUi* self, psy_ui_Component* sender)
{
	self->intern.machineframe = NULL;
}

void masterui_onmousedoubleclick(MasterUi* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {				
		masterui_showparameters(self, self->intern.view);		
		psy_ui_mouseevent_stoppropagation(ev);
	}
}

void masterui_invalidate(MasterUi* self)
{
	if (!vuupdate) {		
		masterui_super_vtable.invalidate(&self->component);
	}
}

void masterui_onshowparameters(MasterUi* self, Workspace* sender,
	uintptr_t slot)
{
	if (slot == self->intern.slot) {
		masterui_showparameters(self, self->intern.view);
	}
}

void masterui_onpreferredsize(MasterUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_RealSize sizepx;
	sizepx = psy_ui_realrectangle_size(&self->intern.coords->background.dest);
	*rv = psy_ui_size_makepx(sizepx.width, sizepx.height);
}

// GeneratorUi

// GeneratorUi
static void generatorui_dispose(GeneratorUi*);
static void generatorui_initsize(GeneratorUi*);
static int generatorui_hittestpan(GeneratorUi*, psy_ui_RealPoint, double* dx);
static bool generatorui_hittestcoord(GeneratorUi*, psy_ui_RealPoint,
	SkinCoord*);
static void generatorui_ondraw(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_draweditname(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_drawbackground(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_drawpanning(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_drawmute(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_drawsoloed(GeneratorUi*, psy_ui_Graphics*);
static bool generatorui_hittesteditname(GeneratorUi*, psy_ui_RealPoint);
static psy_ui_RealRectangle generatorui_coordposition(GeneratorUi*, SkinCoord*);
static void generatorui_oneditchange(GeneratorUi*, psy_ui_Edit* sender);
static void generatorui_oneditfocuslost(GeneratorUi*, psy_ui_Component* sender);
static psy_dsp_amp_t generatorui_panvalue(GeneratorUi*, double dx, uintptr_t slot);
static void generatorui_editname(GeneratorUi*, psy_ui_Edit*,
	psy_ui_RealPoint scrolloffset);
static void generatorui_onmousedown(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_onmouseup(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_onmousemove(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_onmousedoubleclick(GeneratorUi*, psy_ui_MouseEvent*);
static void generatorui_oneditkeydown(GeneratorUi*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void generatorui_drawvu(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_drawhighlight(GeneratorUi*, psy_ui_Graphics*);
static void generatorui_onframedestroyed(GeneratorUi*, psy_ui_Component* sender);
static void generatorui_move(GeneratorUi*, psy_ui_Point topleft);
static void generatorui_updatevolumedisplay(GeneratorUi*);
static void generatorui_invalidate(GeneratorUi*);
static void generatorui_onshowparameters(GeneratorUi*, Workspace* sender,
	uintptr_t slot);
static void generatorui_showparameters(GeneratorUi*, psy_ui_Component* parent);
static void generatorui_onpreferredsize(GeneratorUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable generatorui_vtable;
static psy_ui_ComponentVtable generatorui_super_vtable;
static bool generatorui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* generatorui_vtable_init(GeneratorUi* self)
{
	assert(self);

	if (!generatorui_vtable_initialized) {
		generatorui_vtable = *(self->component.vtable);
		generatorui_super_vtable = generatorui_vtable;
		generatorui_vtable.dispose = (psy_ui_fp_component_dispose)generatorui_dispose;
		generatorui_vtable.ondraw = (psy_ui_fp_component_ondraw)generatorui_ondraw;
		generatorui_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)generatorui_onmousedown;
		generatorui_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)generatorui_onmouseup;
		generatorui_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)generatorui_onmousemove;
		generatorui_vtable.onmousedoubleclick = (psy_ui_fp_component_onmouseevent)
			generatorui_onmousedoubleclick;
		generatorui_vtable.move = (psy_ui_fp_component_move)generatorui_move;
		generatorui_vtable.invalidate = (psy_ui_fp_component_invalidate)generatorui_invalidate;
		generatorui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			generatorui_onpreferredsize;
		generatorui_vtable_initialized = TRUE;
	}
	return &generatorui_vtable;
}
// implementation
void generatorui_init(GeneratorUi* self, psy_ui_Component* parent,
	uintptr_t slot, MachineViewSkin* skin,
	psy_ui_Component* view, psy_ui_Edit* editname, Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(workspace->song);
	assert(skin);
	assert(view);

	self->component.imp = (psy_ui_ComponentImp*)
		psy_ui_viewcomponentimp_allocinit(
			&self->component, (parent) ? parent->imp : NULL, view, "",
			0, 0, 100, 100, 0, 0);
	psy_ui_component_init_imp(&self->component, view,
		self->component.imp);
	generatorui_vtable_init(self);
	self->component.vtable = &generatorui_vtable;
	machineuicommon_init(&self->intern, slot, skin, view, editname, workspace);
	self->intern.coords = &skin->generator;
	self->intern.font = skin->generator_fontcolour;
	self->intern.bgcolour = psy_ui_colour_make(0x002f3E25);
	generatorui_initsize(self);
	psy_signal_connect(&workspace->signal_showparameters, self,
		generatorui_onshowparameters);
}

void generatorui_dispose(GeneratorUi* self)
{
	assert(self);

	if (self->intern.paramview) {
		psy_ui_component_destroy(&self->intern.paramview->component);
		free(self->intern.paramview);
	}
	if (self->intern.machineframe) {
		psy_ui_component_destroy(&self->intern.machineframe->component);
		free(self->intern.machineframe);
	}
	free(self->intern.restorename);
	psy_signal_disconnect(&self->intern.workspace->signal_showparameters, self,
		generatorui_onshowparameters);
	generatorui_super_vtable.dispose(&self->component);
}

void generatorui_initsize(GeneratorUi* self)
{
	psy_ui_RealPoint topleft;
	psy_ui_RealSize size;

	size = psy_ui_realrectangle_size(&self->intern.coords->background.dest);
	if (self->intern.machine && self->intern.machinepos) {
		psy_audio_machine_position(self->intern.machine, &topleft.x, &topleft.y);
	} else {
		topleft = psy_ui_realpoint_zero();
	}
	psy_ui_component_setposition(&self->component,
		psy_ui_rectangle_make(
			psy_ui_point_makepx(topleft.x, topleft.y),
			psy_ui_size_makepx(size.width, size.height)));
	vudisplay_init(&self->intern.vu, self->intern.skin, self->intern.coords);
	self->intern.vu.position = generatorui_coordposition(self, &self->intern.coords->vu0);
}

void generatorui_move(GeneratorUi* self, psy_ui_Point topleft)
{
	assert(self);

	generatorui_super_vtable.move(&self->component, topleft);
	machineuicommon_move(&self->intern, topleft);
	self->intern.vu.position = generatorui_coordposition(self,
		&self->intern.coords->vu0);
}

psy_ui_RealRectangle generatorui_coordposition(GeneratorUi* self, SkinCoord* coord)
{
	psy_ui_RealRectangle r;

	assert(self);

	r = psy_ui_component_position(&self->component);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			r.left + coord->dest.left,
			r.top + coord->dest.top),
		psy_ui_realrectangle_size(&coord->dest));
}

void generatorui_editname(GeneratorUi* self, psy_ui_Edit* edit,
	psy_ui_RealPoint scroll)
{
	assert(self);

	if (self->intern.machine) {
		psy_ui_RealRectangle r;

		psy_strreset(&self->intern.restorename,
			psy_audio_machine_editname(self->intern.machine));
		psy_signal_disconnectall(&edit->component.signal_focuslost);
		psy_signal_disconnectall(&edit->component.signal_keydown);
		psy_signal_disconnectall(&edit->signal_change);
		psy_signal_connect(&edit->signal_change, self, generatorui_oneditchange);
		psy_signal_connect(&edit->component.signal_keydown, self,
			generatorui_oneditkeydown);
		psy_signal_connect(&edit->component.signal_focuslost, self,
			generatorui_oneditfocuslost);
		psy_ui_edit_settext(edit, psy_audio_machine_editname(self->intern.machine));
		r = generatorui_coordposition(self, &self->intern.coords->name);
		psy_ui_realrectangle_move(&r, -scroll.x, -scroll.y);
		psy_ui_component_setposition(psy_ui_edit_base(edit),
			psy_ui_rectangle_make_px(&r));
		psy_ui_component_show(&edit->component);
	}
}

void generatorui_oneditkeydown(GeneratorUi* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{
	assert(self);

	switch (ev->keycode) {
	case psy_ui_KEY_ESCAPE:
		if (self->intern.machine) {
			psy_audio_machine_seteditname(self->intern.machine, self->intern.restorename);
			free(self->intern.restorename);
			self->intern.restorename = NULL;
		}
		psy_ui_component_hide(sender);
		psy_ui_keyevent_preventdefault(ev);
		psy_ui_component_invalidate(&self->component);
		break;
	case psy_ui_KEY_RETURN:
		psy_ui_component_hide(sender);
		psy_ui_keyevent_preventdefault(ev);
		psy_ui_component_invalidate(&self->component);
		break;
	default:
		break;
	}
	psy_ui_keyevent_stoppropagation(ev);
}

void generatorui_oneditchange(GeneratorUi* self, psy_ui_Edit* sender)
{
	assert(self);

	if (self->intern.machine) {
		psy_audio_machine_seteditname(self->intern.machine,
			psy_ui_edit_text(sender));
	}
}

void generatorui_oneditfocuslost(GeneratorUi* self, psy_ui_Component* sender)
{
	assert(self);

	psy_ui_component_hide(sender);
}

void generatorui_ondraw(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	generatorui_drawbackground(self, g);
	if (!vuupdate) {
		generatorui_draweditname(self, g);		
		generatorui_drawpanning(self, g);
		generatorui_drawmute(self, g);
		generatorui_drawsoloed(self, g);
		if (self->intern.machines &&
			self->intern.slot == psy_audio_machines_selected(
				self->intern.machines)) {
			generatorui_drawhighlight(self, g);		
		}
	}
	if (self->intern.skin->drawvumeters) {
		generatorui_drawvu(self, g);
	}
}

void generatorui_drawbackground(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (!psy_ui_bitmap_empty(&self->intern.skin->skinbmp)) {
		skin_blitcoord(g, &self->intern.skin->skinbmp, psy_ui_realpoint_zero(),
			&self->intern.coords->background);
	} else {
		psy_ui_RealRectangle r;

		r = psy_ui_component_position(&self->component);
		psy_ui_drawsolidrectangle(g, r, self->intern.bgcolour);		
	}
}

void generatorui_draweditname(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (self->intern.mode != psy_audio_MACHMODE_MASTER) {
		char editname[130];

		editname[0] = '\0';
		if (psy_audio_machine_editname(self->intern.machine)) {
			if (self->intern.skin->drawmachineindexes) {
				psy_snprintf(editname, 130, "%.2X:%s", (int)self->intern.slot,
					psy_audio_machine_editname(self->intern.machine));
			} else {
				psy_snprintf(editname, 130, "%s",
					psy_audio_machine_editname(self->intern.machine));
			}
		}
		if (psy_strlen(editname) > 0) {
			psy_ui_settextcolour(g, self->intern.font);
			psy_ui_textoutrectangle(g,
				psy_ui_realrectangle_topleft(&self->intern.coords->name.dest),
				psy_ui_ETO_CLIPPED, self->intern.coords->name.dest, editname,
				strlen(editname));
		}
	}
}

void generatorui_drawpanning(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	skin_blitcoord(g, &self->intern.skin->skinbmp,
		psy_ui_realpoint_make(
			skincoord_position(&self->intern.coords->pan,
				psy_audio_machine_panning(self->intern.machine)),
			0),
		&self->intern.coords->pan);
}

void generatorui_drawmute(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (psy_audio_machine_muted(self->intern.machine)) {
		skin_blitcoord(g, &self->intern.skin->skinbmp,
			psy_ui_realpoint_zero(),
			&self->intern.coords->mute);
	}
}

void generatorui_drawsoloed(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (psy_audio_machines_soloed(self->intern.machines) ==
			self->intern.slot) {
		skin_blitcoord(g, &self->intern.skin->skinbmp,
			psy_ui_realpoint_zero(),
			&self->intern.coords->solo);
	}
}

void generatorui_drawvu(GeneratorUi* self, psy_ui_Graphics* g)
{
	assert(self);
	
	generatorui_updatevolumedisplay(self);
	vudisplay_draw(&self->intern.vu, g);	
}

void generatorui_updatevolumedisplay(GeneratorUi* self)
{
	if (self->intern.machine) {
		vudisplay_update(&self->intern.vu,
			psy_audio_machine_buffermemory(self->intern.machine));
	}
}

void generatorui_showparameters(GeneratorUi* self, psy_ui_Component* parent)
{
	if (self->intern.machine) {
		if (!self->intern.machineframe) {
			self->intern.machineframe = machineframe_alloc();
			machineframe_init(self->intern.machineframe, parent, self->intern.workspace);
			psy_signal_connect(&self->intern.machineframe->component.signal_destroy,
				self, generatorui_onframedestroyed);
			if (psy_audio_machine_haseditor(self->intern.machine)) {
				MachineEditorView* editorview;

				editorview = machineeditorview_allocinit(
					psy_ui_notebook_base(&self->intern.machineframe->notebook),
					self->intern.machine, self->intern.workspace);
				if (editorview) {
					machineframe_setview(self->intern.machineframe,
						&editorview->component, self->intern.machine);
				}
			} else {
				ParamView* paramview;

				paramview = paramview_allocinit(
					&self->intern.machineframe->notebook.component,
					self->intern.machine, self->intern.workspace);
				if (paramview) {
					machineframe_setparamview(self->intern.machineframe, paramview,
						self->intern.machine);
				}
			}
		}
		if (self->intern.machineframe) {
			psy_ui_component_show(&self->intern.machineframe->component);
		}
	}
}

void generatorui_onframedestroyed(GeneratorUi* self, psy_ui_Component* sender)
{
	self->intern.machineframe = NULL;
}

void generatorui_onmousedown(GeneratorUi* self, psy_ui_MouseEvent* ev)
{	
	if (self->intern.slot != psy_audio_machines_selected(self->intern.machines)) {
		psy_audio_machines_select(self->intern.machines, self->intern.slot);
	}
	if (generatorui_hittestcoord(self, ev->pt,
			&self->intern.skin->generator.solo)) {
		psy_audio_machines_solo(self->intern.machines, self->intern.slot);
		psy_ui_mouseevent_stoppropagation(ev);
	} else if (generatorui_hittestcoord(self, ev->pt,
			&self->intern.skin->generator.mute)) {
		if (psy_audio_machine_muted(self->intern.machine)) {
			psy_audio_machine_unmute(self->intern.machine);
		} else {
			psy_audio_machine_mute(self->intern.machine);
		}
		psy_ui_mouseevent_stoppropagation(ev);
	} else if (generatorui_hittestpan(self, ev->pt, &self->intern.mx)) {
		self->intern.dragmode = MACHINEVIEW_DRAG_PAN;
		psy_ui_mouseevent_stoppropagation(ev);
	}
	if (!ev->bubble) {
		psy_ui_component_invalidate(&self->component);
	}
}

bool generatorui_hittesteditname(GeneratorUi* self, psy_ui_RealPoint pt)
{
	psy_ui_RealRectangle r;

	r = generatorui_coordposition(self, &self->intern.coords->name);
	return psy_ui_realrectangle_intersect(&r, pt);
}

bool generatorui_hittestcoord(GeneratorUi* self, psy_ui_RealPoint pt,
	SkinCoord* coord)
{
	assert(self);
	
	psy_ui_RealRectangle r;

	r = generatorui_coordposition(self, coord);
	return psy_ui_realrectangle_intersect(&r, pt);	
}

void generatorui_onmousemove(GeneratorUi* self, psy_ui_MouseEvent* ev)
{	
	if (self->intern.dragmode == MACHINEVIEW_DRAG_PAN) {
		psy_audio_machine_setpanning(self->intern.machine,
			generatorui_panvalue(self, ev->pt.x, self->intern.slot));
		psy_ui_component_invalidate(&self->component);
	}
}

int generatorui_hittestpan(GeneratorUi* self, psy_ui_RealPoint pt, double* dx)
{
	psy_ui_RealRectangle r;
	double offset;
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(&self->component);
	offset = psy_audio_machine_panning(self->intern.machine) *
		self->intern.coords->pan.range;
	r = skincoord_destposition(&self->intern.coords->pan);
	psy_ui_realrectangle_move(&r, offset, 0);
	*dx = pt.x - position.left - r.left;
	return psy_ui_realrectangle_intersect(&r,
		psy_ui_realpoint_make(pt.x - position.left,
			pt.y - position.top));
}

psy_dsp_amp_t generatorui_panvalue(GeneratorUi* self, double dx, uintptr_t slot)
{
	psy_dsp_amp_t rv = 0.f;
	MachineCoords* coords;
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(&self->component);
	coords = self->intern.coords;
	if (coords && coords->pan.range != 0) {
		rv = (psy_dsp_amp_t)(
			(dx - (double)position.left -
				coords->pan.dest.left - (double)self->intern.mx) /
			(double)coords->pan.range);
	}
	return rv;
}

void generatorui_onmouseup(GeneratorUi* self, psy_ui_MouseEvent* ev)
{
	self->intern.dragmode = MACHINEVIEW_DRAG_NONE;
}

void generatorui_onmousedoubleclick(GeneratorUi* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		psy_ui_RealPoint dragpt;

		if (generatorui_hittesteditname(self, ev->pt)) {
			if (self->intern.editname) {
				generatorui_editname(self, self->intern.editname,
					psy_ui_component_scrollpx(self->intern.view));
			}
		} else if (generatorui_hittestcoord(self, ev->pt,
				&self->intern.skin->generator.solo) ||
			generatorui_hittestcoord(self, ev->pt,
				&self->intern.skin->effect.bypass) ||
			generatorui_hittestcoord(self, ev->pt,
				&self->intern.skin->generator.mute) ||
			generatorui_hittestcoord(self, ev->pt,
				&self->intern.skin->effect.mute) ||
			generatorui_hittestpan(self, ev->pt, &dragpt.x)) {
		} else {
			generatorui_showparameters(self, self->intern.view);
		}
		psy_ui_mouseevent_stoppropagation(ev);
	}
}

void generatorui_drawhighlight(GeneratorUi* self, psy_ui_Graphics* g)
{
	static intptr_t d = 5; // the distance of the highlight from the machine
	static psy_ui_RealPoint dirs[] = {
		{ 1.0, 0.0 }, { 0.0, 1.0 }, { -1.0, 0.0 }, { 0.0, -1.0 }
	};
	psy_ui_RealPoint edges[4];
	double width;
	double height;
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(&self->component);
	width = psy_ui_realrectangle_width(&position);
	height = psy_ui_realrectangle_height(&position);
	edges[0] = psy_ui_realpoint_make(-d, -d);
	edges[1] = psy_ui_realpoint_make(width + d, -d);
	edges[2] = psy_ui_realpoint_make(width + d, height + d);
	edges[3] = psy_ui_realpoint_make(-d, height + d);
	psy_ui_setcolour(g, self->intern.skin->wirecolour);
	drawmachineline(g, dirs[0], edges[0]);
	drawmachineline(g, dirs[1], edges[0]);
	drawmachineline(g, dirs[1], edges[1]);
	drawmachineline(g, dirs[2], edges[1]);
	drawmachineline(g, dirs[2], edges[2]);
	drawmachineline(g, dirs[3], edges[2]);
	drawmachineline(g, dirs[3], edges[3]);
	drawmachineline(g, dirs[0], edges[3]);
}

void generatorui_invalidate(GeneratorUi* self)
{
	if (vuupdate) {		
		psy_ui_component_invalidaterect(self->intern.view,
			self->intern.vu.position);		
	} else {
		generatorui_super_vtable.invalidate(&self->component);
	}
}

void generatorui_onshowparameters(GeneratorUi* self, Workspace* sender,
	uintptr_t slot)
{
	if (slot == self->intern.slot) {
		generatorui_showparameters(self, self->intern.view);
	}
}

void generatorui_onpreferredsize(GeneratorUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_RealSize sizepx;
	sizepx = psy_ui_realrectangle_size(&self->intern.coords->background.dest);
	*rv = psy_ui_size_makepx(sizepx.width, sizepx.height);
}

// EffectUi
static void effectui_dispose(EffectUi*);
static void effectui_initsize(EffectUi*);
static int effectui_hittestpan(EffectUi*, psy_ui_RealPoint, double* dx);
static bool effectui_hittestcoord(EffectUi*, psy_ui_RealPoint, SkinCoord*);
static void effectui_ondraw(EffectUi*, psy_ui_Graphics*);
static void effectui_draweditname(EffectUi*, psy_ui_Graphics*);
static void effectui_drawbackground(EffectUi*, psy_ui_Graphics*);
static void effectui_drawpanning(EffectUi*, psy_ui_Graphics*);
static void effectui_drawmute(EffectUi*, psy_ui_Graphics*);
static void effectui_drawbypassed(EffectUi*, psy_ui_Graphics*);
static bool effectui_hittesteditname(EffectUi*, psy_ui_RealPoint);
static psy_ui_RealRectangle effectui_coordposition(EffectUi*, SkinCoord*);
static void effectui_oneditchange(EffectUi*, psy_ui_Edit* sender);
static void effectui_oneditfocuslost(EffectUi*, psy_ui_Component* sender);
static psy_dsp_amp_t effectui_panvalue(EffectUi*, double dx, uintptr_t slot);
static void effectui_editname(EffectUi*, psy_ui_Edit*,
	psy_ui_RealPoint scrolloffset);
static void effectui_onmousedown(EffectUi*, psy_ui_MouseEvent*);
static void effectui_onmouseup(EffectUi*, psy_ui_MouseEvent*);
static void effectui_onmousemove(EffectUi*, psy_ui_MouseEvent*);
static void effectui_onmousedoubleclick(EffectUi*, psy_ui_MouseEvent*);
static void effectui_oneditkeydown(EffectUi*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void effectui_drawvu(EffectUi*, psy_ui_Graphics*);
static void effectui_drawhighlight(EffectUi*, psy_ui_Graphics*);
static void effectui_onframedestroyed(EffectUi*, psy_ui_Component* sender);
static void effectui_move(EffectUi*, psy_ui_Point topleft);
static void effectui_updatevolumedisplay(EffectUi*);
static void effectui_invalidate(EffectUi*);
static void effectui_onshowparameters(EffectUi*, Workspace* sender,
	uintptr_t slot);
static void effectui_showparameters(EffectUi*, psy_ui_Component* parent);
static void effectui_onpreferredsize(EffectUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable effectui_vtable;
static psy_ui_ComponentVtable effectui_super_vtable;
static bool effectui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* effectui_vtable_init(EffectUi* self)
{
	assert(self);

	if (!effectui_vtable_initialized) {
		effectui_vtable = *(self->component.vtable);
		effectui_super_vtable = effectui_vtable;
		effectui_vtable.dispose = (psy_ui_fp_component_dispose)effectui_dispose;
		effectui_vtable.ondraw = (psy_ui_fp_component_ondraw)effectui_ondraw;
		effectui_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)effectui_onmousedown;
		effectui_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)effectui_onmouseup;
		effectui_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)effectui_onmousemove;
		effectui_vtable.onmousedoubleclick = (psy_ui_fp_component_onmouseevent)
			effectui_onmousedoubleclick;
		effectui_vtable.move = (psy_ui_fp_component_move)effectui_move;
		effectui_vtable.invalidate = (psy_ui_fp_component_invalidate)
			effectui_invalidate;
		effectui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			effectui_onpreferredsize;
		effectui_vtable_initialized = TRUE;
	}
	return &effectui_vtable;
}
// implementation
void effectui_init(EffectUi* self, psy_ui_Component* parent,
	uintptr_t slot, MachineViewSkin* skin,
	psy_ui_Component* view, psy_ui_Edit* editname, Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(workspace->song);
	assert(skin);
	assert(view);

	self->component.imp = (psy_ui_ComponentImp*)
		psy_ui_viewcomponentimp_allocinit(
			&self->component, (parent) ? parent->imp : NULL, view, "",
			0, 0, 100, 100, 0, 0);
	psy_ui_component_init_imp(&self->component, view,
		self->component.imp);
	effectui_vtable_init(self);
	self->component.vtable = &effectui_vtable;
	machineuicommon_init(&self->intern, slot, skin, view, editname, workspace);
	self->intern.coords = &skin->effect;
	self->intern.font = skin->effect_fontcolour;
	self->intern.bgcolour = psy_ui_colour_make(0x003E2f25);
	effectui_initsize(self);
	psy_signal_connect(&workspace->signal_showparameters, self,
		effectui_onshowparameters);
}

void effectui_dispose(EffectUi* self)
{
	assert(self);

	if (self->intern.paramview) {
		psy_ui_component_destroy(&self->intern.paramview->component);
		free(self->intern.paramview);
	}
	if (self->intern.machineframe) {
		psy_ui_component_destroy(&self->intern.machineframe->component);
		free(self->intern.machineframe);
	}
	free(self->intern.restorename);
	psy_signal_disconnect(&self->intern.workspace->signal_showparameters, self,
		effectui_onshowparameters);
	effectui_super_vtable.dispose(&self->component);
}

void effectui_initsize(EffectUi* self)
{
	psy_ui_RealPoint topleft;
	psy_ui_RealSize size;

	size = psy_ui_realrectangle_size(&self->intern.coords->background.dest);
	if (self->intern.machine && self->intern.machinepos) {
		psy_audio_machine_position(self->intern.machine, &topleft.x, &topleft.y);
	} else {
		topleft = psy_ui_realpoint_zero();
	}
	psy_ui_component_setposition(&self->component,
		psy_ui_rectangle_make(
			psy_ui_point_makepx(topleft.x, topleft.y),
			psy_ui_size_makepx(size.width, size.height)));
	vudisplay_init(&self->intern.vu, self->intern.skin, self->intern.coords);
	self->intern.vu.position = effectui_coordposition(self, &self->intern.coords->vu0);
}

void effectui_move(EffectUi* self, psy_ui_Point topleft)
{
	assert(self);

	effectui_super_vtable.move(&self->component, topleft);
	machineuicommon_move(&self->intern, topleft);
	self->intern.vu.position = effectui_coordposition(self,
		&self->intern.coords->vu0);
}

psy_ui_RealRectangle effectui_coordposition(EffectUi* self, SkinCoord* coord)
{
	psy_ui_RealRectangle r;

	assert(self);

	r = psy_ui_component_position(&self->component);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			r.left + coord->dest.left,
			r.top + coord->dest.top),
		psy_ui_realrectangle_size(&coord->dest));
}

void effectui_editname(EffectUi* self, psy_ui_Edit* edit,
	psy_ui_RealPoint scroll)
{
	assert(self);

	if (self->intern.machine) {
		psy_ui_RealRectangle r;

		psy_strreset(&self->intern.restorename,
			psy_audio_machine_editname(self->intern.machine));
		psy_signal_disconnectall(&edit->component.signal_focuslost);
		psy_signal_disconnectall(&edit->component.signal_keydown);
		psy_signal_disconnectall(&edit->signal_change);
		psy_signal_connect(&edit->signal_change, self, effectui_oneditchange);
		psy_signal_connect(&edit->component.signal_keydown, self,
			effectui_oneditkeydown);
		psy_signal_connect(&edit->component.signal_focuslost, self,
			effectui_oneditfocuslost);
		psy_ui_edit_settext(edit, psy_audio_machine_editname(self->intern.machine));
		r = effectui_coordposition(self, &self->intern.coords->name);
		psy_ui_realrectangle_move(&r, -scroll.x, -scroll.y);
		psy_ui_component_setposition(psy_ui_edit_base(edit),
			psy_ui_rectangle_make_px(&r));
		psy_ui_component_show(&edit->component);
	}
}

void effectui_oneditkeydown(EffectUi* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{
	assert(self);

	switch (ev->keycode) {
	case psy_ui_KEY_ESCAPE:
		if (self->intern.machine) {
			psy_audio_machine_seteditname(self->intern.machine, self->intern.restorename);
			free(self->intern.restorename);
			self->intern.restorename = NULL;
		}
		psy_ui_component_hide(sender);
		psy_ui_keyevent_preventdefault(ev);
		psy_ui_component_invalidate(&self->component);
		break;
	case psy_ui_KEY_RETURN:
		psy_ui_component_hide(sender);
		psy_ui_keyevent_preventdefault(ev);
		psy_ui_component_invalidate(&self->component);
		break;
	default:
		break;
	}
	psy_ui_keyevent_stoppropagation(ev);
}

void effectui_oneditchange(EffectUi* self, psy_ui_Edit* sender)
{
	assert(self);

	if (self->intern.machine) {
		psy_audio_machine_seteditname(self->intern.machine,
			psy_ui_edit_text(sender));
	}
}

void effectui_oneditfocuslost(EffectUi* self, psy_ui_Component* sender)
{
	assert(self);

	psy_ui_component_hide(sender);
}

void effectui_ondraw(EffectUi* self, psy_ui_Graphics* g)
{
	assert(self);

	effectui_drawbackground(self, g);
	if (!vuupdate) {
		effectui_draweditname(self, g);		
		effectui_drawpanning(self, g);
		effectui_drawmute(self, g);
		effectui_drawbypassed(self, g);		
		if (self->intern.machines &&
			self->intern.slot == psy_audio_machines_selected(
				self->intern.machines)) {
			effectui_drawhighlight(self, g);		
		}
	}
	if (self->intern.skin->drawvumeters) {
		effectui_drawvu(self, g);
	}
}

void effectui_drawbackground(EffectUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (!psy_ui_bitmap_empty(&self->intern.skin->skinbmp)) {
		skin_blitcoord(g, &self->intern.skin->skinbmp, psy_ui_realpoint_zero(),
			&self->intern.coords->background);
	} else {
		psy_ui_RealRectangle r;

		r = psy_ui_component_position(&self->component);
		psy_ui_drawsolidrectangle(g, r, self->intern.bgcolour);
	}
}

void effectui_draweditname(EffectUi* self, psy_ui_Graphics* g)
{
	assert(self);
	
	char editname[130];

	editname[0] = '\0';
	if (psy_audio_machine_editname(self->intern.machine)) {
		if (self->intern.skin->drawmachineindexes) {
			psy_snprintf(editname, 130, "%.2X:%s", (int)self->intern.slot,
				psy_audio_machine_editname(self->intern.machine));
		} else {
			psy_snprintf(editname, 130, "%s",
				psy_audio_machine_editname(self->intern.machine));
		}
	}
	if (psy_strlen(editname) > 0) {
		psy_ui_settextcolour(g, self->intern.font);
		psy_ui_textoutrectangle(g,
			psy_ui_realrectangle_topleft(&self->intern.coords->name.dest),
			psy_ui_ETO_CLIPPED, self->intern.coords->name.dest, editname,
			strlen(editname));
	}	
}

void effectui_drawpanning(EffectUi* self, psy_ui_Graphics* g)
{
	assert(self);

	skin_blitcoord(g, &self->intern.skin->skinbmp,
		psy_ui_realpoint_make(
			skincoord_position(&self->intern.coords->pan,
				psy_audio_machine_panning(self->intern.machine)),
			0),
		&self->intern.coords->pan);
}

void effectui_drawmute(EffectUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (psy_audio_machine_muted(self->intern.machine)) {
		skin_blitcoord(g, &self->intern.skin->skinbmp,
			psy_ui_realpoint_zero(),
			&self->intern.coords->mute);
	}
}

void effectui_drawbypassed(EffectUi* self, psy_ui_Graphics* g)
{
	assert(self);

	if (psy_audio_machine_bypassed(self->intern.machine)) {
		skin_blitcoord(g, &self->intern.skin->skinbmp,
			psy_ui_realpoint_zero(),
			&self->intern.coords->bypass);
	}
}

void effectui_drawvu(EffectUi* self, psy_ui_Graphics* g)
{
	assert(self);
	
	effectui_updatevolumedisplay(self);
	vudisplay_draw(&self->intern.vu, g);	
}

void effectui_updatevolumedisplay(EffectUi* self)
{
	if (self->intern.machine) {
		vudisplay_update(&self->intern.vu,
			psy_audio_machine_buffermemory(self->intern.machine));
	}
}

void effectui_showparameters(EffectUi* self, psy_ui_Component* parent)
{
	if (self->intern.machine) {
		if (!self->intern.machineframe) {
			self->intern.machineframe = machineframe_alloc();
			machineframe_init(self->intern.machineframe, parent, self->intern.workspace);
			psy_signal_connect(&self->intern.machineframe->component.signal_destroy,
				self, effectui_onframedestroyed);
			if (psy_audio_machine_haseditor(self->intern.machine)) {
				MachineEditorView* editorview;

				editorview = machineeditorview_allocinit(
					psy_ui_notebook_base(&self->intern.machineframe->notebook),
					self->intern.machine, self->intern.workspace);
				if (editorview) {
					machineframe_setview(self->intern.machineframe,
						&editorview->component, self->intern.machine);
				}
			} else {
				ParamView* paramview;

				paramview = paramview_allocinit(
					&self->intern.machineframe->notebook.component,
					self->intern.machine, self->intern.workspace);
				if (paramview) {
					machineframe_setparamview(self->intern.machineframe, paramview,
						self->intern.machine);
				}
			}
		}
		if (self->intern.machineframe) {
			psy_ui_component_show(&self->intern.machineframe->component);
		}
	}
}

void effectui_onframedestroyed(EffectUi* self, psy_ui_Component* sender)
{
	self->intern.machineframe = NULL;
}

void effectui_onmousedown(EffectUi* self, psy_ui_MouseEvent* ev)
{	
	if (self->intern.slot != psy_audio_machines_selected(self->intern.machines)) {
		psy_audio_machines_select(self->intern.machines, self->intern.slot);
	}
	if (effectui_hittestcoord(self, ev->pt,
			&self->intern.skin->effect.bypass)) {
		if (psy_audio_machine_bypassed(self->intern.machine)) {
			psy_audio_machine_unbypass(self->intern.machine);
		} else {
			psy_audio_machine_bypass(self->intern.machine);
		}
		psy_ui_mouseevent_stoppropagation(ev);
	} else if (effectui_hittestcoord(self, ev->pt,
			&self->intern.skin->generator.mute) ||
		effectui_hittestcoord(self, ev->pt,
			&self->intern.skin->effect.mute)) {
		if (psy_audio_machine_muted(self->intern.machine)) {
			psy_audio_machine_unmute(self->intern.machine);
		} else {
			psy_audio_machine_mute(self->intern.machine);
		}
		psy_ui_mouseevent_stoppropagation(ev);
	} else if (effectui_hittestpan(self, ev->pt, &self->intern.mx)) {
		self->intern.dragmode = MACHINEVIEW_DRAG_PAN;
		psy_ui_mouseevent_stoppropagation(ev);
	}
	if (!ev->bubble) {
		psy_ui_component_invalidate(&self->component);
	}
}

bool effectui_hittesteditname(EffectUi* self, psy_ui_RealPoint pt)
{
	psy_ui_RealRectangle r;

	r = effectui_coordposition(self, &self->intern.coords->name);
	return psy_ui_realrectangle_intersect(&r, pt);
}

bool effectui_hittestcoord(EffectUi* self, psy_ui_RealPoint pt,
	SkinCoord* coord)
{	
	psy_ui_RealRectangle r;

	assert(self);

	r = effectui_coordposition(self, coord);
	return psy_ui_realrectangle_intersect(&r, pt);	
}

void effectui_onmousemove(EffectUi* self, psy_ui_MouseEvent* ev)
{
	if (self->intern.dragmode == MACHINEVIEW_DRAG_PAN) {
		psy_audio_machine_setpanning(self->intern.machine,
			effectui_panvalue(self, ev->pt.x, self->intern.slot));
		psy_ui_component_invalidate(&self->component);
	}
}

int effectui_hittestpan(EffectUi* self, psy_ui_RealPoint pt, double* dx)
{
	psy_ui_RealRectangle r;
	double offset;
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(&self->component);
	offset = psy_audio_machine_panning(self->intern.machine) *
		self->intern.coords->pan.range;
	r = skincoord_destposition(&self->intern.coords->pan);
	psy_ui_realrectangle_move(&r, offset, 0);
	*dx = pt.x - position.left - r.left;
	return psy_ui_realrectangle_intersect(&r,
		psy_ui_realpoint_make(pt.x - position.left,
			pt.y - position.top));
}

psy_dsp_amp_t effectui_panvalue(EffectUi* self, double dx, uintptr_t slot)
{
	psy_dsp_amp_t rv = 0.f;
	MachineCoords* coords;
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(&self->component);
	coords = self->intern.coords;
	if (coords && coords->pan.range != 0) {
		rv = (psy_dsp_amp_t)(
			(dx - (double)position.left -
				coords->pan.dest.left - (double)self->intern.mx) /
			(double)coords->pan.range);
	}
	return rv;
}

void effectui_onmouseup(EffectUi* self, psy_ui_MouseEvent* ev)
{
	self->intern.dragmode = MACHINEVIEW_DRAG_NONE;
}

void effectui_onmousedoubleclick(EffectUi* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		psy_ui_RealPoint dragpt;

		if (effectui_hittesteditname(self, ev->pt)) {
			if (self->intern.editname) {
				effectui_editname(self, self->intern.editname,
					psy_ui_component_scrollpx(self->intern.view));
			}
		} else if (effectui_hittestcoord(self, ev->pt,
				&self->intern.skin->effect.bypass) ||
			effectui_hittestcoord(self, ev->pt,
				 &self->intern.skin->generator.mute) ||
			effectui_hittestcoord(self, ev->pt,
				&self->intern.skin->effect.mute) ||
			effectui_hittestpan(self, ev->pt, &dragpt.x)) {
		} else {
			effectui_showparameters(self, self->intern.view);
		}
		psy_ui_mouseevent_stoppropagation(ev);
	}
}

void effectui_drawhighlight(EffectUi* self, psy_ui_Graphics* g)
{
	static intptr_t d = 5; // the distance of the highlight from the machine
	static psy_ui_RealPoint dirs[] = {
		{ 1.0, 0.0 }, { 0.0, 1.0 }, { -1.0, 0.0 }, { 0.0, -1.0 }
	};
	psy_ui_RealPoint edges[4];
	double width;
	double height;
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(&self->component);
	width = psy_ui_realrectangle_width(&position);
	height = psy_ui_realrectangle_height(&position);
	edges[0] = psy_ui_realpoint_make(-d, -d);
	edges[1] = psy_ui_realpoint_make(width + d, -d);
	edges[2] = psy_ui_realpoint_make(width + d, height + d);
	edges[3] = psy_ui_realpoint_make(-d, height + d);
	psy_ui_setcolour(g, self->intern.skin->wirecolour);
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


void effectui_invalidate(EffectUi* self)
{
	if (vuupdate) {		
		psy_ui_component_invalidaterect(self->intern.view,
			self->intern.vu.position);		
	} else {
		effectui_super_vtable.invalidate(&self->component);
	}
}

void effectui_onshowparameters(EffectUi* self, Workspace* sender,
	uintptr_t slot)
{
	if (slot == self->intern.slot) {
		effectui_showparameters(self, self->intern.view);
	}
}

void effectui_onpreferredsize(EffectUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_RealSize sizepx;
	sizepx = psy_ui_realrectangle_size(&self->intern.coords->background.dest);
	*rv = psy_ui_size_makepx(sizepx.width, sizepx.height);
}

// SliderUi
// prototypes
static void sliderui_dispose(SliderUi*);
static void sliderui_ondraw(SliderUi*, psy_ui_Graphics*);
static void sliderui_invalidate(SliderUi*);
static void sliderui_onpreferredsize(SliderUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void sliderui_onmousedown(SliderUi*, psy_ui_MouseEvent*);
static void sliderui_onmouseup(SliderUi*, psy_ui_MouseEvent*);
static void sliderui_onmousemove(SliderUi*, psy_ui_MouseEvent*);

// vtable
static psy_ui_ComponentVtable sliderui_vtable;
static psy_ui_ComponentVtable sliderui_super_vtable;
static bool sliderui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* sliderui_vtable_init(SliderUi* self)
{
	assert(self);

	if (!sliderui_vtable_initialized) {
		sliderui_vtable = *(self->component.vtable);
		sliderui_super_vtable = sliderui_vtable;
		sliderui_vtable.dispose = (psy_ui_fp_component_dispose)sliderui_dispose;
		sliderui_vtable.ondraw = (psy_ui_fp_component_ondraw)sliderui_ondraw;		
		sliderui_vtable.invalidate = (psy_ui_fp_component_invalidate)
			sliderui_invalidate;
		sliderui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			sliderui_onpreferredsize;
		sliderui_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)sliderui_onmousedown;
		sliderui_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)sliderui_onmouseup;
		sliderui_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)sliderui_onmousemove;
		sliderui_vtable_initialized = TRUE;
	}
	return &sliderui_vtable;
}
// implementation
void sliderui_init(SliderUi* self, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam* param,
	Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(workspace->song);	
	assert(view);

	self->component.imp = (psy_ui_ComponentImp*)
		psy_ui_viewcomponentimp_allocinit(
			&self->component, (parent) ? parent->imp : NULL,
			view, "", 0, 0, 100, 100, 0, 0);
	psy_ui_component_init_imp(&self->component, view,
		self->component.imp);
	sliderui_vtable_init(self);
	self->component.vtable = &sliderui_vtable;	
	self->workspace = workspace;
	self->view = view;	
	self->skin = machineparamconfig_skin(
		psycleconfig_macparam(workspace_conf(workspace)));
	self->param = param;
	paramtweak_init(&self->paramtweak);
}

void sliderui_dispose(SliderUi* self)
{
	assert(self);
	
	sliderui_super_vtable.dispose(&self->component);
}

void sliderui_ondraw(SliderUi* self, psy_ui_Graphics* g)
{
	SliderDraw draw;
	psy_ui_RealSize size;

	size = mpfsize(self->skin, psy_ui_component_textmetric(self->view),
		MPF_SLIDER, FALSE);	
	size.width = 140;
	sliderdraw_init(&draw, self->skin, NULL, self->param, size, NULL, FALSE);
	sliderdraw_draw(&draw, g);	
}

void sliderui_invalidate(SliderUi* self)
{
	if (!vuupdate) {
		sliderui_super_vtable.invalidate(&self->component);
	}
}

void sliderui_onpreferredsize(SliderUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	psy_ui_RealSize size;

	size = mpfsize(self->skin, psy_ui_component_textmetric(self->view),
		MPF_SLIDER, FALSE);
	*rv = psy_ui_size_makepx(140.0, size.height);
}

void sliderui_onmousedown(SliderUi* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1 && self->param != NULL) {
		paramtweak_begin(&self->paramtweak, NULL, psy_INDEX_INVALID);
		self->paramtweak.param = self->param;
		paramtweak_onmousedown(&self->paramtweak, ev);
		psy_ui_component_capture(&self->component);
	}
}

void sliderui_onmousemove(SliderUi* self, psy_ui_MouseEvent* ev)
{
	if (self->paramtweak.param != NULL) {
		paramtweak_onmousemove(&self->paramtweak, ev);
		psy_ui_component_invalidate(&self->component);
	}
}

void sliderui_onmouseup(SliderUi* self, psy_ui_MouseEvent* ev)
{
	paramtweak_end(&self->paramtweak);
	psy_ui_component_releasecapture(&self->component);
	psy_ui_component_invalidate(&self->component);
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

psy_ui_Component* machineui_create(psy_audio_Machine* machine, 
	uintptr_t slot, MachineViewSkin* skin, psy_ui_Component* parent,
	psy_ui_Component* view,
	psy_ui_Edit* editname, bool machinepos, Workspace* workspace)
{	
	psy_ui_Component* newui;
		
	newui = NULL;
	if (slot == psy_audio_MASTER_INDEX) {
		MasterUi* masterui;

		masterui = (MasterUi*)malloc(sizeof(MasterUi));
		if (masterui) {
			masterui_init(masterui, parent, skin, view, workspace);
			masterui->intern.machinepos = machinepos;
			newui = &masterui->component;
		}
	} else if (slot >= 0x40 && slot <= 0x80) {
		EffectUi* effectui;

		effectui = (EffectUi*)malloc(sizeof(EffectUi));
		if (effectui) {
			effectui_init(effectui, parent, slot, skin, view, editname, workspace);
			effectui->intern.machinepos = machinepos;
			newui = &effectui->component;
		}
	} else {
		GeneratorUi* generatorui;

		generatorui = (GeneratorUi*)malloc(sizeof(GeneratorUi));
		if (generatorui) {
			generatorui_init(generatorui, parent, slot, skin, view, editname, workspace);
			generatorui->intern.machinepos = machinepos;
			newui = &generatorui->component;
		}
	}	
	if (newui) {
		newui->deallocate = TRUE;
	}
	return newui;	
}


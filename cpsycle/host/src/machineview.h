// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEVIEW)
#define MACHINEVIEW

#include "workspace.h"
#include <uinotebook.h>
#include <uiedit.h>
#include <uidef.h>
#include <player.h>
#include <plugincatcher.h>
#include "newmachine.h"
#include "machineframe.h"
#include "paramview.h"
#include "vst2view.h"
#include "machinebar.h"
#include "skincoord.h"
#include "tabbar.h"

typedef struct {
	SkinCoord background;
	SkinCoord vu0;
	SkinCoord vupeak;
	SkinCoord pan;
	SkinCoord mute;
	SkinCoord solo;
	SkinCoord bypass;	
	SkinCoord name;
} MachineCoords;

typedef struct {
	MachineCoords master;
	MachineCoords generator;
	MachineCoords effect;
	unsigned int colour;
	unsigned int wirecolour;
	unsigned int selwirecolour;
	unsigned int wireaacolour;
	unsigned int wireaacolour2;
	unsigned int polycolour;
	unsigned int wirewidth;
	unsigned int wireaa;
	unsigned int triangle_size;
	const char* generator_fontface;
	int generator_font_point;
	unsigned int generator_font_flags;	
	unsigned int generator_fontcolour;
	const char* effect_fontface;
	int effect_font_point;
	unsigned int effect_font_flags;	
	unsigned int effect_fontcolour;
	ui_bitmap skinbmp;	
	ui_font font;
	int drawmachineindexes;
} MachineSkin;

typedef struct {	
	int x;
	int y;	
	int mode;
	psy_dsp_amp_t volumedisplay;	
	MachineCoords* coords;
	MachineSkin* skin;
	psy_audio_Machine* machine;
	uintptr_t slot;
	MachineFrame* frame;
	ParamView* paramview;
	Vst2View* vst2view;
	char* restorename;
	Workspace* workspace;
} MachineUi;

enum {	
	MACHINEWIREVIEW_DRAG_MACHINE,
	MACHINEWIREVIEW_DRAG_NEWCONNECTION,
	MACHINEWIREVIEW_DRAG_CHANGECONNECTION,
	MACHINEWIREVIEW_DRAG_PAN,
};

typedef struct {
	ui_component component;	
	psy_audio_Machines* machines;
	psy_Table machineuis;
	psy_List* wireframes;
	int dx;
	int dy;
	int mx;
	int my;
	uintptr_t dragslot;
	int dragmode;
	uintptr_t selectedslot;	
	psy_audio_Wire dragwire;	
	psy_audio_Wire selectedwire;
	int drawvumeters;	
	psy_audio_PluginCatcher plugincatcher;
	MachineSkin skin;	   
	Workspace* workspace;	
	struct MachineViewBar* statusbar;
	ui_edit editname;	
} MachineWireView;

void machinewireview_init(MachineWireView*, ui_component* parent,
	ui_component* tabbarparent, Workspace*);
void machinewireview_align(MachineWireView*);

typedef struct MachineViewBar {
	ui_component component;
	ui_label label;	
} MachineViewBar;

void machineviewbar_init(MachineViewBar*, ui_component* parent, Workspace*);

typedef struct {
	ui_component component;
	TabBar tabbar;
	ui_notebook notebook;	
	MachineWireView wireview;
	NewMachine newmachine;
	Workspace* workspace;
} MachineView;

void machineview_init(MachineView*, ui_component* parent,
	ui_component* tabbarparent, Workspace*);
void machineview_applyproperties(MachineView*, psy_Properties*);
void machineview_align(MachineView*);

#endif

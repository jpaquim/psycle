// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEVIEW)
#define MACHINEVIEW

#include <windows.h>
#include "workspace.h"
#include <uinotebook.h>
#include <uidef.h>
#include <player.h>
#include <plugincatcher.h>
#include "newmachine.h"
#include "machineframe.h"
#include "paramview.h"
#include "machinebar.h"
#include "skincoord.h"
#include "tabbar.h"

typedef struct {
	SkinCoord background;	
} MasterCoords;

typedef struct {
	SkinCoord background;
	SkinCoord vu0;
	SkinCoord vupeak;
	SkinCoord pan;
	SkinCoord mute;
	SkinCoord solo;
	SkinCoord name;
} GeneratorCoords;

typedef struct {
	SkinCoord background;	
	SkinCoord vu0;
	SkinCoord vupeak;
	SkinCoord pan;
	SkinCoord mute;
	SkinCoord bypass;
	SkinCoord name;
} EffectCoords;

typedef struct {
	MasterCoords master;
	GeneratorCoords generator;
	EffectCoords effect;
	unsigned int colour;
	unsigned int wirecolour;
	unsigned int wireaacolour;
	unsigned int wireaacolour2;
	unsigned int polycolour;
	unsigned int wirewidth;
	unsigned int wireaa;
	const char* generator_fontface;
	int generator_font_point;
	unsigned int generator_font_flags;	
	unsigned int generator_fontcolour;
	const char* effect_fontface;
	int effect_font_point;
	unsigned int effect_font_flags;	
	unsigned int effect_fontcolour;
	ui_bitmap skinbmp;	
	HFONT hfont;
} MachineSkin;


typedef struct {
	int x;
	int y;
	char* editname;
} MachineUi;

void MachineUiSet(MachineUi* self, int x, int y, const char* editname);

enum {	
	WireView_DRAG_MACHINE,
	WireView_DRAG_NEWCONNECTION,
	WireView_DRAG_CHANGECONNECTION
};

typedef struct {
	ui_component component;	
	ui_graphics* g;
	int cx;
	int cy;
	Machines* machines;
	MachineUi machineuis[256];
	int mx;
	int my;
	int dragslot;
	int dragmode;
	int selectedslot;
	int wirefound;
	int wiresrc;
	int wiredst;
	MachineFrame machine_frames[256];
	ParamView machine_paramviews[256];
	PluginCatcher plugincatcher;
	MachineSkin skin;	   
	Workspace* workspace;
} WireView;

void InitWireView(WireView*, ui_component* parent,
	ui_component* tabbarparent, Workspace*);
void wireview_align(WireView*);

typedef struct {
	ui_component component;
	TabBar tabbar;
	ui_notebook notebook;	
	WireView wireview;
	NewMachine newmachine;
	Workspace* workspace;
} MachineView;

void InitMachineView(MachineView*, ui_component* parent,
	ui_component* tabbarparent, Workspace*);
void MachineViewApplyProperties(MachineView*, Properties*);
void machineview_align(MachineView*);

#endif
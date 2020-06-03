// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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
#include "machineeditorview.h"
#include "machinebar.h"
#include "skincoord.h"
#include "tabbar.h"

typedef struct {
	SkinCoord background;
	SkinCoord vu0;
	SkinCoord vupeak;
	SkinCoord pan;
	SkinCoord mute;
	SkinCoord bypass;
	SkinCoord solo;		
	SkinCoord name;
} MachineCoords;

typedef struct {
	MachineCoords master;
	MachineCoords generator;
	MachineCoords effect;
	unsigned int colour;
	unsigned int wirecolour;
	unsigned int selwirecolour;
	unsigned int hoverwirecolour;
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
	psy_ui_Bitmap skinbmp;	
	psy_ui_Font font;
	int drawmachineindexes;
} MachineSkin;

typedef struct {	
	int x;
	int y;	
	int mode;
	/// output level for display (0 .. 1.f)	
	psy_dsp_amp_t volumedisplay;
	/// output peak level for display (0 .. 1.f)
	psy_dsp_amp_t volumemaxdisplay;
	/// output peak level display time (refreshrate * 60)
	int volumemaxcounterlife;
	MachineCoords* coords;
	MachineSkin* skin;
	psy_audio_Machine* machine;
	uintptr_t slot;
	MachineFrame* frame;
	ParamView* paramview;
	MachineEditorView* editorview;
	char* restorename;
	Workspace* workspace;
} MachineUi;

enum {	
	MACHINEWIREVIEW_DRAG_NONE,
	MACHINEWIREVIEW_DRAG_MACHINE,
	MACHINEWIREVIEW_DRAG_NEWCONNECTION,
	MACHINEWIREVIEW_DRAG_LEFTCONNECTION,
	MACHINEWIREVIEW_DRAG_RIGHTCONNECTION,
	MACHINEWIREVIEW_DRAG_PAN,
};

typedef struct {
	psy_ui_Component component;	
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
	psy_audio_Wire hoverwire;
	int drawvumeters;	
	psy_audio_PluginCatcher plugincatcher;
	MachineSkin skin;	   
	Workspace* workspace;	
	struct MachineViewBar* statusbar;
	psy_ui_Edit editname;
	int firstsize;
	int randominsert;
	int addeffect;
	bool mousemoved;
	bool showwirehover;
} MachineWireView;

void machinewireview_init(MachineWireView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);
void machinewireview_align(MachineWireView*);

typedef struct MachineViewBar {
	psy_ui_Component component;
	psy_ui_CheckBox mixersend;
	psy_ui_Component status;
	char* text;
	Workspace* workspace;
} MachineViewBar;

void machineviewbar_init(MachineViewBar*, psy_ui_Component* parent, Workspace*);

typedef struct {
	psy_ui_Component component;
	TabBar tabbar;
	psy_ui_Notebook notebook;	
	MachineWireView wireview;
	NewMachine newmachine;
	Workspace* workspace;
} MachineView;

void machineview_init(MachineView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);
void machineview_applyproperties(MachineView*, psy_Properties*);
void machineview_align(MachineView*);

#endif

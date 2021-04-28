// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net


#if !defined(PROPERTIESVIEW)
#define PROPERTIESVIEW

// host
#include "inputdefiner.h"
#include <uitabbar.h>
#include "workspace.h"
// ui
#include <uiedit.h>
#include <uilabel.h>
#include <uiswitch.h>
#include <uiscroller.h>
#include <uinotebook.h>
// container
#include <hashtbl.h>
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// Displays psy_Property and allows to edit them.


struct PropertiesRenderLine;

typedef struct PropertiesRenderState {
	psy_Property* property;
	psy_Property* selected;
	psy_ui_Component* dummy;
	psy_ui_Edit* edit;
	struct PropertiesRenderLine* line;
	bool dialogbutton;
} PropertiesRenderState;

void propertiesrenderstate_init(PropertiesRenderState*);

typedef struct PropertiesRenderLine {
	// inherits
	psy_ui_Component component;
	psy_ui_Component col0;	
	psy_ui_Component col1;
	psy_ui_Component col2;
	psy_ui_Label key;
	psy_ui_Switch* check;
	psy_ui_Label* label;
	psy_ui_Button* dialogbutton;
	psy_ui_Component* colour;
	uintptr_t level;	
	psy_Property* property;
	uintptr_t numcols;
	PropertiesRenderState* state;
} PropertiesRenderLine;

void propertiesrenderline_init(PropertiesRenderLine*,
	psy_ui_Component* parent, psy_ui_Component* view,
	PropertiesRenderState*, psy_Property*,
	uintptr_t level, uintptr_t numcols);

PropertiesRenderLine* propertiesrenderline_alloc(void);
PropertiesRenderLine* propertiesrenderline_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	PropertiesRenderState*, psy_Property*,
	uintptr_t level, uintptr_t numcols);

void propertiesrenderline_update(PropertiesRenderLine*);
bool propertiesrenderline_updatecheck(PropertiesRenderLine*);
bool propertiesrenderline_updateintegerlabel(PropertiesRenderLine*);
bool propertiesrenderline_updatestringlabel(PropertiesRenderLine*);
bool propertiesrenderline_updatefontlabel(PropertiesRenderLine*);
bool propertiesrenderline_updateshortcut(PropertiesRenderLine*);
bool propertiesrenderline_updatecolour(PropertiesRenderLine*);

typedef struct PropertiesRenderer {
	// inherits
	psy_ui_Component component;
	psy_ui_Component client;
	// internal data
	// dummy used to calculate font size
	psy_ui_Component dummy;			
	int keyselected;		
	bool showkeyselection;
	psy_ui_Edit edit;
	InputDefiner inputdefiner;
	psy_Signal signal_changed;
	psy_Signal signal_selected;	
	uintptr_t numcols;
	uintptr_t currlinestatecount;		
	psy_ui_Colour valuecolour;
	psy_ui_Colour sectioncolour;
	psy_ui_Colour separatorcolour;
	psy_ui_Colour valueselcolour;
	psy_ui_Colour valueselbackgroundcolour;
	psy_ui_Component* currsection;	
	psy_Table sections;
	PropertiesRenderState state;
	uintptr_t mainsectionstyle;
	uintptr_t mainsectionheaderstyle;
	uintptr_t keystyle;
	uintptr_t keystyle_hover;
	// references
	psy_Property* properties;	
	Workspace* workspace;	
} PropertiesRenderer;

void propertiesrenderer_init(PropertiesRenderer*, psy_ui_Component* parent,
	psy_Property*, uintptr_t numcols, Workspace*);

void propertiesrenderer_setstyle(PropertiesRenderer*,
	uintptr_t mainsection,
	uintptr_t mainsectionheader,
	uintptr_t keystyle,
	uintptr_t keystyle_hover);

INLINE const psy_Property* propertiesrenderer_properties(const
	PropertiesRenderer* self)
{
	return self->properties;
}

INLINE psy_ui_Component* propertiesrenderer_base(PropertiesRenderer* self)
{
	return &self->component;
}

typedef struct PropertiesView {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_changed;
	psy_Signal signal_selected;
	// intern
	psy_ui_Component sectionfloated;
	psy_ui_Label floatdesc;
	psy_ui_Component viewtabbar;	
	psy_ui_TabBar tabbar;
	PropertiesRenderer renderer;
	psy_ui_Scroller scroller;
	bool preventscrollupdate;	
	// references
	Workspace* workspace;
} PropertiesView;

void propertiesview_init(PropertiesView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_Property*,
	uintptr_t numcols, Workspace*);

void propertiesview_reload(PropertiesView*);

INLINE psy_ui_Component* propertiesview_base(PropertiesView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PROPERTIESVIEW */

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
	struct PropertiesRenderLine* line;
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
	bool floated;
	psy_ui_Component* currsection;
	psy_Table sections;
	// references
	psy_Property* properties;
	psy_Property* selected;	
	// psy_Property* choiceproperty;
	Workspace* workspace;
	PropertiesRenderState state;	
} PropertiesRenderer;

void propertiesrenderer_init(PropertiesRenderer*, psy_ui_Component* parent,
	psy_Property*, uintptr_t numcols, Workspace*);

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
	psy_ui_Notebook notebook;
	psy_ui_Component client;
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

// float to side bar (see helpview, too)
// todo make it more general
void propertiesview_float(PropertiesView*, uintptr_t section, psy_ui_Component* dest);
void propertiesview_dock(PropertiesView*, uintptr_t section, psy_ui_Component* src);

INLINE psy_ui_Component* propertiesview_base(PropertiesView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PROPERTIESVIEW */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net


#if !defined(PROPERTIESVIEW)
#define PROPERTIESVIEW

#include <uicomponent.h>
#include <uiedit.h>
#include <properties.h>
#include <uiscroller.h>
#include "inputdefiner.h"
#include "tabbar.h"
#include "workspace.h"

#include <hashtbl.h>

// aim: Displays psy_Property and allows to edit them.

#define PROPERTIESRENDERER_NUMCOLS 3

typedef struct PropertiesRenderLineState {
	int cpy;
	int cpx;
	int level;
	int numlines;
	psy_Property* properties;
} PropertiesRenderLineState;

void propertiesrenderlinestate_init(PropertiesRenderLineState*);
void propertiesrenderlinestate_dispose(PropertiesRenderLineState*);

typedef struct {
	psy_ui_Component component;
	psy_Property* properties;
	psy_Property* selected;
	psy_Property* search;
	psy_Property* choiceproperty;
	psy_ui_Graphics* g;
	psy_ui_Rectangle selrect;
	int lastlevel;	
	int keyselected;
	int button;	
	int mx;
	int my;
	int cpy;
	int cpx;
	int lineheight;
	int textheight;
	int centery;
	int numblocklines;	
	int identwidth;
	float col_perc[PROPERTIESRENDERER_NUMCOLS];	
	int col_width[PROPERTIESRENDERER_NUMCOLS];	
	int col_start[PROPERTIESRENDERER_NUMCOLS];	
	psy_ui_Edit edit;
	InputDefiner inputdefiner;
	psy_Signal signal_changed;
	psy_Signal signal_selected;
	psy_ui_Value fixedwidth;
	bool usefixedwidth;
	PropertiesRenderLineState* linestate_clipstart;
	uintptr_t currlinestatecount;
	psy_Table linestates;	
	psy_ui_Color valuecolor;
	psy_ui_Color sectioncolor;
	psy_ui_Color separatorcolor;
	psy_ui_Color valueselcolor;
	psy_ui_Color valueselbackgroundcolor;
	Workspace* workspace;
} PropertiesRenderer;

void propertiesrenderer_init(PropertiesRenderer*, psy_ui_Component* parent,
	psy_Property*, Workspace*);

void propertiesrenderer_setfixedwidth(PropertiesRenderer*, psy_ui_Value width);

INLINE psy_ui_Component* propertiesrenderer_base(PropertiesRenderer* self)
{
	return &self->component;
}

typedef struct {
	psy_ui_Component component;
	psy_ui_Component viewtabbar;	
	TabBar tabbar;
	PropertiesRenderer renderer;
	psy_ui_Scroller scroller;
	psy_Signal signal_changed;
	psy_Signal signal_selected;
	Workspace* workspace;
} PropertiesView;

void propertiesview_init(PropertiesView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_Property*, Workspace*);

INLINE void propertiesview_setcolumnwidth(PropertiesView* self,
	float col0_perc, float col1_perc, float col2_perc)
{
	self->renderer.col_perc[0] = col0_perc;
	self->renderer.col_perc[1] = col1_perc;
	self->renderer.col_perc[2] = col2_perc;	
}

#endif /* PROPERTIESVIEW */

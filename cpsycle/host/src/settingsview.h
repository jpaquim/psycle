// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net


#if !defined(SETTINGSVIEW)
#define SETTINGSVIEW

#include <uicomponent.h>
#include <uiedit.h>
#include <properties.h>
#include "inputdefiner.h"
#include "tabbar.h"

// aim: Displays psy_Properties and allows to edit them.

#define PROPERTIESRENDERER_NUMCOLS 3

typedef struct {
	psy_ui_Component client;
	psy_Properties* properties;
	psy_ui_Graphics* g;
	int lastlevel;
	psy_Properties* selected;
	int keyselected;
	psy_Properties* search;
	psy_ui_Rectangle selrect;
	int button;
	int dy;
	int mx;
	int my;
	int cpy;
	int cpx;
	int currchoice;
	int choicecount;
	int lineheight;
	int numblocklines;	
	int identwidth;
	float col_perc[PROPERTIESRENDERER_NUMCOLS];	
	int col_width[PROPERTIESRENDERER_NUMCOLS];	
	int col_start[PROPERTIESRENDERER_NUMCOLS];
	psy_Properties* choiceproperty;
	psy_ui_Edit edit;
	InputDefiner inputdefiner;
	psy_Signal signal_changed;
	psy_Signal signal_selected;
} PropertiesRenderer;

void propertiesrenderer_init(PropertiesRenderer*, psy_ui_Component* parent,
	psy_Properties*);

typedef struct {
	psy_ui_Component component;
	psy_ui_Component viewtabbar;	
	TabBar tabbar;
	PropertiesRenderer renderer;
	psy_Signal signal_changed;
	psy_Signal signal_selected;
} PropertiesView;

void propertiesview_init(PropertiesView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_Properties*);

INLINE void propertiesview_setcolumnwidth(PropertiesView* self,
	float col0_perc, float col1_perc, float col2_perc)
{
	self->renderer.col_perc[0] = col0_perc;
	self->renderer.col_perc[1] = col1_perc;
	self->renderer.col_perc[2] = col2_perc;	
}

#endif

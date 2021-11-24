/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PROPERTIESVIEW)
#define PROPERTIESVIEW

/* host */
#include "inputdefiner.h"
#include <uitabbar.h>
#include "workspace.h"
/* ui */
#include <uicombobox.h>
#include <uitextinput.h>
#include <uilabel.h>
#include <uiswitch.h>
#include <uiscroller.h>
#include <uinotebook.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Displays and edits properties */

struct PropertiesRenderLine;

typedef struct PropertiesRenderState {	
	bool dialogbutton;
	psy_ui_Size size_col0;
	psy_ui_Size size_col2;
	uintptr_t numcols;
	bool preventmousepropagation;
	/* references */
	psy_Property* property; /* event bubble target property */
	psy_Property* selected; /* selected property*/
	struct PropertiesRenderLine* selectedline;
	psy_ui_TextInput* edit;
	psy_ui_Component* dummy; /* used to calculate font pt size */
	bool comboselect;
} PropertiesRenderState;

void propertiesrenderstate_init(PropertiesRenderState*, uintptr_t numcols,
	psy_ui_TextInput* edit, psy_ui_Component* dummy);

/* PropertiesRenderLine */
typedef struct PropertiesRenderLine {
	/* inherits */
	psy_ui_Component component;	
	/* internal */
	psy_ui_Label key;
	psy_ui_Switch* check;
	psy_ui_Label* label;
	psy_ui_Button* dialogbutton;
	psy_ui_ComboBox* combo;
	psy_ui_Component* colour;	
	psy_Property* property;	
	PropertiesRenderState* state;
} PropertiesRenderLine;

void propertiesrenderline_init(PropertiesRenderLine*,
	psy_ui_Component* parent, psy_ui_Component* view,
	PropertiesRenderState*, psy_Property*, uintptr_t level);

PropertiesRenderLine* propertiesrenderline_alloc(void);
PropertiesRenderLine* propertiesrenderline_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	PropertiesRenderState*, psy_Property*, uintptr_t level);

void propertiesrenderline_update(PropertiesRenderLine*);
bool propertiesrenderline_updatecheck(PropertiesRenderLine*);
bool propertiesrenderline_updateintegerlabel(PropertiesRenderLine*);
bool propertiesrenderline_updatestringlabel(PropertiesRenderLine*);
bool propertiesrenderline_updatefontlabel(PropertiesRenderLine*);
bool propertiesrenderline_updateshortcut(PropertiesRenderLine*);
bool propertiesrenderline_updatecolour(PropertiesRenderLine*);

typedef struct PropertiesRenderer {
	/* inherits */
	psy_ui_Component component;
	psy_ui_Component client;
	/* signals */
	psy_Signal signal_changed;
	psy_Signal signal_selected;
	/* internal */
	psy_ui_Component dummy;	/* used to calculate font pt size */	
	psy_ui_TextInput edit;
	InputDefiner inputdefiner;	
	uintptr_t currlinestatecount;		
	psy_ui_Component* curr;	
	PropertiesRenderLine* comboline;
	uintptr_t combolevel;
	psy_Table sections;	
	PropertiesRenderState state;
	uintptr_t mainsectionstyle;
	uintptr_t mainsectionheaderstyle;
	uintptr_t keystyle;
	uintptr_t keystyle_hover;
	uintptr_t linestyle_select;
	uintptr_t rebuild_level;
	/* references */
	psy_Property* properties;	
} PropertiesRenderer;

void propertiesrenderer_init(PropertiesRenderer*, psy_ui_Component* parent,
	psy_Property*, uintptr_t numcols);

void propertiesrenderer_setstyle(PropertiesRenderer*,
	uintptr_t mainsection,
	uintptr_t mainsectionheader,
	uintptr_t keystyle,
	uintptr_t keystyle_hover,
	uintptr_t linestyle_select);

INLINE const psy_Property* propertiesrenderer_properties(const
	PropertiesRenderer* self)
{
	return self->properties;
}

void propertiesrenderer_updateline(PropertiesRenderer*, PropertiesRenderLine*);
void propertiesrenderer_build(PropertiesRenderer*);
void propertiesrenderer_rebuild(PropertiesRenderer*,
	psy_Property* mainsection);

INLINE psy_ui_Component* propertiesrenderer_base(PropertiesRenderer* self)
{
	return &self->component;
}

typedef struct PropertiesView {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_changed;
	psy_Signal signal_selected;
	/* intern */
	psy_ui_Component sectionfloated;
	psy_ui_Label floatdesc;
	psy_ui_Component viewtabbar;	
	psy_ui_TabBar tabbar;
	PropertiesRenderer renderer;
	psy_ui_Scroller scroller;	
	bool maximizemainsections;
} PropertiesView;

void propertiesview_init(PropertiesView*, psy_ui_Component* parent,
	psy_ui_Component* view, psy_ui_Component* tabbarparent, psy_Property*,
	uintptr_t numcols, Workspace*);

void propertiesview_reload(PropertiesView*);
void propertiesview_mark(PropertiesView*, psy_Property*);
void propertiesview_select(PropertiesView*, psy_Property*);
psy_Property* propertiesview_selected(PropertiesView*);
void propertiesview_enablemousepropagation(PropertiesView*);
void propertiesview_prevent_maximize_mainsections(PropertiesView*);

INLINE psy_ui_Component* propertiesview_base(PropertiesView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PROPERTIESVIEW */

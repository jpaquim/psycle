/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PROPERTIESVIEW)
#define PROPERTIESVIEW

/* host */
#include "inputdefiner.h"
#include <uitabbar.h>
#include "workspace.h"
/* ui */
#include <uicombobox.h>
#include <uitextarea.h>
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
	psy_ui_Size size_col0;
	psy_ui_Size size_col2;
	uintptr_t numcols;
	bool prevent_mouse_propagation;
	/* references */	
	struct PropertiesRenderLine* property_line_changed;
} PropertiesRenderState;

void propertiesrenderstate_init(PropertiesRenderState*, uintptr_t numcols);


/* PropertiesRenderLine */

struct PropertiesRenderLine;

typedef struct PropertiesRenderLine {
	/* inherits */
	psy_ui_Component component;	
	/*signals */
	psy_Signal signal_selected;
	/* internal */	
	psy_ui_Switch* check;
	psy_ui_Label* label;
	psy_ui_TextArea* edit;
	InputDefiner* input_definer;
	psy_ui_ComboBox* combo;
	psy_ui_Component* colour;
	psy_Property* property;	
	/* references */
	PropertiesRenderState* state;
	struct PropertiesRenderLine* choice_line;
} PropertiesRenderLine;

void propertiesrenderline_init(PropertiesRenderLine*,
	psy_ui_Component* parent, PropertiesRenderState*, psy_Property*,
	uintptr_t level);

PropertiesRenderLine* propertiesrenderline_alloc(void);
PropertiesRenderLine* propertiesrenderline_allocinit(
	psy_ui_Component* parent,
	PropertiesRenderState*, psy_Property*, uintptr_t level);

void propertiesrenderline_update(PropertiesRenderLine*);
bool propertiesrenderline_update_bool(PropertiesRenderLine*);
bool propertiesrenderline_update_string(PropertiesRenderLine*);
bool propertiesrenderline_update_font(PropertiesRenderLine*);
bool propertiesrenderline_update_int(PropertiesRenderLine*);

typedef struct PropertiesRenderer {
	/* inherits */
	psy_ui_Component component;
	psy_ui_Component client;
	/* signals */
	psy_Signal signal_changed;
	psy_Signal signal_selected;
	/* internal */	
	uintptr_t currlinestatecount;		
	psy_ui_Component* curr;	
	PropertiesRenderLine* choiceline;
	uintptr_t choicelevel;
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

void propertiesrenderer_set_style(PropertiesRenderer*,
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

void propertiesrenderer_update_line(PropertiesRenderer*, PropertiesRenderLine*);
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
	bool maximize_main_sections;
} PropertiesView;

void propertiesview_init(PropertiesView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_Property*, uintptr_t numcols,
	InputHandler*);

void propertiesview_reload(PropertiesView*);
void propertiesview_mark(PropertiesView*, psy_Property*);
void propertiesview_select(PropertiesView*, psy_Property*);
psy_Property* propertiesview_selected(PropertiesView*);
void propertiesview_enable_mouse_propagation(PropertiesView*);
void propertiesview_prevent_maximize_main_sections(PropertiesView*);

INLINE psy_ui_Component* propertiesview_base(PropertiesView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PROPERTIESVIEW */

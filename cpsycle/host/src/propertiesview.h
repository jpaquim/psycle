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
#include <uilabel.h>
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
	uintptr_t mainsectionstyle;
	uintptr_t mainsectionheaderstyle;
	uintptr_t keystyle;
	uintptr_t keystyle_hover;
	uintptr_t linestyle_select;
	/* references */	
	psy_Table sections;
	struct PropertiesRenderLine* property_line_selected;
	psy_ui_Component* view;
} PropertiesRenderState;

void propertiesrenderstate_init(PropertiesRenderState*, uintptr_t numcols);
void propertiesrenderstate_dispose(PropertiesRenderState*);


/* PropertiesRenderLine */

typedef struct PropertiesRenderLine {
	/* inherits */
	psy_ui_Component component;	
	/* internal */
	psy_ui_Label* label;	
	InputDefiner* input_definer;	
	psy_ui_Component* colour;
	uintptr_t level;
	/* references */
	psy_Property* property;
	PropertiesRenderState* state;	
} PropertiesRenderLine;

void propertiesrenderline_init(PropertiesRenderLine*,
	psy_ui_Component* parent, PropertiesRenderState*, psy_Property*,
	uintptr_t level);

PropertiesRenderLine* propertiesrenderline_alloc(void);
PropertiesRenderLine* propertiesrenderline_allocinit(
	psy_ui_Component* parent, PropertiesRenderState*, psy_Property*,
	uintptr_t level);

typedef struct PropertiesRenderer {
	/* inherits */
	psy_ui_Component component;
	psy_ui_Component client;
	/* signals */
	psy_Signal signal_selected;
	/* internal */	
	uintptr_t currlinestatecount;			
	PropertiesRenderState state;	
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

void propertiesrenderer_build(PropertiesRenderer*);
void propertiesrenderer_maximize_sections(PropertiesRenderer*);

INLINE psy_ui_Component* propertiesrenderer_base(PropertiesRenderer* self)
{
	return &self->component;
}

typedef struct PropertiesView {
	/* inherits */
	psy_ui_Component component;
	/* signals */	
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
void propertiesview_prevent_maximize_main_sections(PropertiesView*);

INLINE psy_ui_Component* propertiesview_base(PropertiesView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PROPERTIESVIEW */

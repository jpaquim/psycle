/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(VIEWFRAME_H)
#define VIEWFRAME_H

/* audio */
#include "../../driver/eventdriver.h"
/* ui */
#include <uiframe.h>
#include <uiterminal.h>
#include <uisplitbar.h>
/* container */
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** EmptyViewPage
*/

typedef struct EmptyViewPage {
	/* inherits */
	psy_ui_Component component;
	psy_ui_Label label;
} EmptyViewPage;

void emptyviewpage_init(EmptyViewPage*, psy_ui_Component* parent);


/* ViewFrame */
typedef struct ViewFrame {
	psy_ui_Component component;
	psy_ui_Component pane;
	psy_ui_Component* view;	
	psy_ui_Component* splitter;
	psy_ui_Component* icons;
	psy_ui_Component* restore_parent;
	psy_ui_AlignType restore_align;
	uintptr_t restore_section;
	psy_EventDriver* kbd;	
} ViewFrame;

void viewframe_init(ViewFrame*, psy_ui_Component* parent,
	psy_ui_Component* page, psy_ui_Component* splitter,
	psy_ui_Component* icons, psy_EventDriver* kbd);

ViewFrame* viewframe_alloc(void);
ViewFrame* viewframe_allocinit(psy_ui_Component* parent,
	psy_ui_Component* page, psy_ui_Component* splitter,
	psy_ui_Component* icons, psy_EventDriver* kbd);

void viewframe_float(ViewFrame*, psy_ui_Component* component);
void viewframe_dock(ViewFrame*);

INLINE psy_ui_Component* viewframe_base(ViewFrame* self)
{
	return &self->component;
}


#ifdef __cplusplus
}
#endif

#endif /* VIEWFRAME_H */

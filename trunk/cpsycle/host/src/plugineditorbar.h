/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PLUGINEDITORBAR_H)
#define PLUGINEDITORBAR_H

/* host */
#include "zoombox.h"
/* ui */
#include <uicheckbox.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** PatternViewBar
**
** The bar displayed in the mainframe status bar, if the patternview is active
*/

struct PluginEditor;

typedef struct PluginEditorBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	ZoomBox zoombox;	
	psy_ui_Label line_desc;
	psy_ui_Label line;
	psy_ui_Label column_desc;
	psy_ui_Label column;
	/* references */
	struct PluginEditor* editor;
} PluginEditorBar;

void plugineditorbar_init(PluginEditorBar*, psy_ui_Component* parent);

void plugineditorbar_set_editor(PluginEditorBar*, struct PluginEditor*);
void plugineditorbar_idle(PluginEditorBar*);

INLINE psy_ui_Component* plugineditorbar_base(PluginEditorBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PLUGINEDITORBAR_H */

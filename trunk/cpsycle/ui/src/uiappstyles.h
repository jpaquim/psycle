/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_APPSTYLES_H
#define psy_ui_APPSTYLES_H

/* local */
#include "uistyle.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	psy_ui_STYLE_ROOT               0
#define	psy_ui_STYLE_LABEL              20
#define	psy_ui_STYLE_LABEL_HOVER        40
#define	psy_ui_STYLE_LABEL_SELECT       41
#define	psy_ui_STYLE_LABEL_DISABLED     42
#define	psy_ui_STYLE_EDIT               60
#define	psy_ui_STYLE_EDIT_FOCUS         61
#define	psy_ui_STYLE_BUTTON             80
#define psy_ui_STYLE_BUTTON_HOVER       81
#define psy_ui_STYLE_BUTTON_SELECT      82
#define psy_ui_STYLE_BUTTON_ACTIVE      83
#define psy_ui_STYLE_BUTTON_FOCUS       84
#define psy_ui_STYLE_TABBAR             100
#define psy_ui_STYLE_TAB                101	
#define psy_ui_STYLE_TAB_HOVER          102
#define psy_ui_STYLE_TAB_SELECT         103
#define psy_ui_STYLE_TAB_LABEL          104
#define psy_ui_STYLE_HEADER             120
#define psy_ui_STYLE_SCROLLPANE         140
#define psy_ui_STYLE_SCROLLTHUMB        141
#define psy_ui_STYLE_SCROLLTHUMB_HOVER  142
#define psy_ui_STYLE_SLIDERPANE         160
#define psy_ui_STYLE_SLIDERTHUMB        161
#define psy_ui_STYLE_SLIDERTHUMB_HOVER  162
#define psy_ui_STYLE_COMBOBOX           180
#define psy_ui_STYLE_COMBOBOX_HOVER     181
#define psy_ui_STYLE_COMBOBOX_SELECT    182
#define psy_ui_STYLE_SPLITTER           200
#define psy_ui_STYLE_SPLITTER_HOVER     201
#define psy_ui_STYLE_SPLITTER_SELECT    202
#define psy_ui_STYLE_PROGRESSBAR        220
#define psy_ui_STYLE_SWITCH             240
#define psy_ui_STYLE_SWITCH_HOVER       241
#define psy_ui_STYLE_SWITCH_SELECT      242

void psy_ui_appstyles_inittheme(psy_ui_Styles*, psy_ui_ThemeMode,
	bool keepfont);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_APPSTYLES_H */

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_APPSTYLES_H
#define psy_ui_APPSTYLES_H

/* local */
#include "uistyles.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	psy_ui_STYLE_ROOT                40000
#define	psy_ui_STYLE_LABEL               40020
#define	psy_ui_STYLE_LABEL_HOVER         40040
#define	psy_ui_STYLE_LABEL_SELECT        40041
#define	psy_ui_STYLE_LABEL_DISABLED      40042
#define	psy_ui_STYLE_EDIT                40060
#define	psy_ui_STYLE_EDIT_FOCUS          40061
#define	psy_ui_STYLE_BUTTON              40080
#define psy_ui_STYLE_BUTTON_HOVER        40081
#define psy_ui_STYLE_BUTTON_SELECT       40082
#define psy_ui_STYLE_BUTTON_ACTIVE       40083
#define psy_ui_STYLE_BUTTON_FOCUS        40084
#define psy_ui_STYLE_TABBAR              40100
#define psy_ui_STYLE_TAB                 40101	
#define psy_ui_STYLE_TAB_HOVER           40102
#define psy_ui_STYLE_TAB_SELECT          40103
#define psy_ui_STYLE_TAB_LABEL           40104
#define psy_ui_STYLE_HSCROLLPANE          40140
#define psy_ui_STYLE_HSCROLLTHUMB         40141
#define psy_ui_STYLE_HSCROLLTHUMB_HOVER   40142
#define psy_ui_STYLE_HSCROLLBUTTON        40143
#define psy_ui_STYLE_HSCROLLBUTTON_HOVER  40144
#define psy_ui_STYLE_HSCROLLBUTTON_ACTIVE 40145
#define psy_ui_STYLE_VSCROLLPANE          40150
#define psy_ui_STYLE_VSCROLLTHUMB         40151
#define psy_ui_STYLE_VSCROLLTHUMB_HOVER   40152
#define psy_ui_STYLE_VSCROLLBUTTON        40153
#define psy_ui_STYLE_VSCROLLBUTTON_HOVER  40154
#define psy_ui_STYLE_VSCROLLBUTTON_ACTIVE 40155
#define psy_ui_STYLE_SLIDERPANE          40160
#define psy_ui_STYLE_SLIDERTHUMB         40161
#define psy_ui_STYLE_SLIDERTHUMB_HOVER   40162
#define psy_ui_STYLE_COMBOBOX            40180
#define psy_ui_STYLE_COMBOBOX_HOVER      40181
#define psy_ui_STYLE_COMBOBOX_SELECT     40182
#define psy_ui_STYLE_COMBOBOX_TEXT       40183
#define psy_ui_STYLE_SPLITTER            40200
#define psy_ui_STYLE_SPLITTER_HOVER      40201
#define psy_ui_STYLE_SPLITTER_SELECT     40202
#define psy_ui_STYLE_PROGRESSBAR         40220
#define psy_ui_STYLE_SWITCH              40240
#define psy_ui_STYLE_SWITCH_HOVER        40241
#define psy_ui_STYLE_SWITCH_SELECT       40242
#define psy_ui_STYLE_LISTBOX             40260
#define psy_ui_STYLE_CHECKMARK           40280
#define psy_ui_STYLE_CHECKMARK_HOVER     40281
#define psy_ui_STYLE_CHECKMARK_SELECT    40282
#define psy_ui_STYLE_EDITOR              40300
#define psy_ui_STYLE_LIST_ITEM           40320
#define psy_ui_STYLE_LIST_ITEM_SELECT    40321

void psy_ui_appstyles_inittheme(psy_ui_Styles*, psy_ui_ThemeMode,
	bool keepfont);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_APPSTYLES_H */

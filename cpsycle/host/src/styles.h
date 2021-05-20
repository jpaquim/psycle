/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(STYLES_H)
#define STYLES_H

/* ui */
#include <uistyle.h>

#ifdef __cplusplus
extern "C" {
#endif
	
#define STYLE_SEQ_TAB                        30000
#define STYLE_SEQ_TAB_HOVER                  31001
#define STYLE_SEQ_TAB_SELECT                 31002
#define STYLE_SEQ_TAB_NEW		             31003
#define STYLE_SEQ_PROGRESS                   31004
#define STYLE_SEQEDT			             31020
#define STYLE_SEQEDT_ITEM                    31030
#define STYLE_SEQEDT_ITEM_HOVER              31031
#define STYLE_SEQEDT_ITEM_SELECTED           31032
#define STYLE_SEQEDT_EVENT                   31033
#define STYLE_SEQLISTVIEW                    31040
#define STYLE_SEQLISTVIEW_SELECT             31042
#define STYLE_SEQLISTVIEW_TRACK              31043
#define STYLE_SEQLISTVIEW_TRACK_SELECT       31044
#define STYLE_SEQVIEW_BUTTONS                31060
#define STYLE_SEQVIEW_BUTTON                 31080
#define STYLE_SEQVIEW_BUTTON_HOVER           31081
#define STYLE_SEQVIEW_BUTTON_SELECT          31082
#define STYLE_STATUSBAR                      32000
#define STYLE_MACHINEBOX                     32020
#define STYLE_ZOOMBOX                        32040
#define STYLE_MACHINEVIEW                    32060
#define STYLE_MACHINEVIEW_PROPERTIES         32061
#define STYLE_STEPSEQUENCER                  32080
#define STYLE_TOP                            33020
#define STYLE_TOPROWS                        33060
#define STYLE_TOPROW0                        34000
#define STYLE_TOPROW1                        34040
#define STYLE_TOPROW2                        34080
#define STYLE_TRACKSCOPES                    35000
#define STYLE_TRACKSCOPE                     35010
#define STYLE_SONGPROPERTIES                 35020
#define STYLE_SONGPROPERTIES_COMMENTS        35021
#define STYLE_SAMPLESVIEW                    35040
#define STYLE_SAMPLESVIEW_HEADER             35041
#define STYLE_SAMPLESVIEW_BUTTONS            35042
#define STYLE_ABOUT					         35060
#define STYLE_INSTRVIEW		                 35080
#define STYLE_INSTRVIEW_HEADER               35081
#define STYLE_INSTRVIEW_BUTTONS              35082
#define STYLE_PLAYBAR				         36000
#define STYLE_DURATION_TIME			         36020
#define STYLE_KEY                            36040
#define STYLE_KEY_HOVER                      36041
#define STYLE_KEY_SELECT                     36042
#define STYLE_KEY_ACTIVE                     36043
#define STYLE_KEY_SHIFT_SELECT               36044
#define STYLE_KEY_ALT_SELECT                 36045
#define STYLE_KEY_CTRL_SELECT                36046
#define STYLE_TABLEROW                       36050
#define STYLE_TABLEROW_HOVER                 36051
#define STYLE_TABLEROW_SELECT                36052
#define STYLE_INPUTDEFINER_FOCUS             36060
#define STYLE_NEWMACHINE_SECTIONS_HEADER     36080
#define STYLE_NEWMACHINE_SECTION             36100
#define STYLE_NEWMACHINE_SECTION_SELECTED    36101
#define STYLE_NEWMACHINE_SEARCHBAR           36120
#define STYLE_SEARCHFIELD                    36140
#define STYLE_SEARCHFIELD_SELECT             36141
#define STYLE_NEWMACHINE_RESCANBAR           36160
#define STYLE_NEWMACHINE_CATEGORYBAR         36180
#define STYLE_NEWMACHINE_SECTIONBAR          36200
#define STYLE_NEWMACHINE_SECTION_HEADER      36220
#define STYLE_NEWMACHINE_SORTBAR             36240
#define STYLE_NEWMACHINE_FILTERBAR           36260
#define STYLE_NEWMACHINE_DETAIL              36280
#define STYLE_PROPERTYVIEW_MAINSECTION       36300
#define STYLE_PROPERTYVIEW_MAINSECTIONHEADER 36320
#define STYLE_RECENTVIEW_MAINSECTION         36340
#define STYLE_RECENTVIEW_MAINSECTIONHEADER   36360
#define STYLE_HEADER                         36380
#define STYLE_MAINVIEWTOPBAR                 36400
#define STYLE_NAVBAR                         36420
#define STYLE_CLIPBOX                        36440
#define STYLE_CLIPBOX_SELECT                 36441
#define STYLE_LABELPAIR                      36460
#define STYLE_LABELPAIR_FIRST                36461
#define STYLE_LABELPAIR_SECOND               36462
#define STYLE_TERM_BUTTON                    36480
#define STYLE_TERM_BUTTON_WARNING            36485
#define STYLE_TERM_BUTTON_ERROR              36490
#define STYLE_GREET                          36500
#define STYLE_GREET_TOP                      36501

void inithoststyles(psy_ui_Styles*, psy_ui_ThemeMode);

#ifdef __cplusplus
}
#endif

#endif /* STYLES_H */

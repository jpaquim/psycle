// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(STYLES_H)
#define STYLES_H

#ifdef __cplusplus
extern "C" {
#endif
	
#define STYLE_SEQ_TAB                      1000
#define STYLE_SEQ_TAB_HOVER                1001
#define STYLE_SEQ_TAB_SELECT               1002
#define STYLE_SEQ_TAB_NEW		           1003
#define STYLE_SEQ_PROGRESS                 1004
#define STYLE_SEQEDT			           1020
#define STYLE_SEQEDT_ITEM                  1030
#define STYLE_SEQEDT_ITEM_HOVER            1031
#define STYLE_SEQEDT_ITEM_SELECTED         1032
#define STYLE_SEQEDT_EVENT                 1033
#define STYLE_SEQLISTVIEW                  1040
#define STYLE_SEQLISTVIEW_SELECT           1042
#define STYLE_SEQLISTVIEW_TRACK            1043
#define STYLE_SEQLISTVIEW_TRACK_SELECT     1044
#define STYLE_SEQVIEW_BUTTONS              1060
#define STYLE_SEQVIEW_BUTTON               1080
#define STYLE_SEQVIEW_BUTTON_HOVER         1081
#define STYLE_SEQVIEW_BUTTON_SELECT        1082
#define STYLE_STATUSBAR                    2000
#define STYLE_MACHINEBOX                   2020
#define STYLE_ZOOMBOX                      2040
#define STYLE_MACHINEVIEW                  2060
#define STYLE_STEPSEQUENCER                2080
#define STYLE_TOP                          3020
#define STYLE_TOPROWS                      3060
#define STYLE_TOPROW0                      4000
#define STYLE_TOPROW1                      4040
#define STYLE_TOPROW2                      4080
#define STYLE_TRACKSCOPES                  5000
#define STYLE_SONGPROPERTIES               5020
#define STYLE_SONGPROPERTIES_COMMENTS      5021
#define STYLE_SAMPLESVIEW                  5040
#define STYLE_ABOUT					       5060
#define STYLE_INSTRUMENTVIEW		       5080
#define STYLE_PLAYBAR				       6000
#define STYLE_DURATION_TIME			       6020
#define STYLE_KEY                          6040
#define STYLE_KEY_HOVER                    6041
#define STYLE_KEY_SELECT                   6042
#define STYLE_KEY_ACTIVE                   6043
#define STYLE_KEY_SHIFT                    6044
#define STYLE_KEY_ALT                      6045
#define STYLE_KEY_CTRL                     6046
#define STYLE_TABLEROW                     6050
#define STYLE_TABLEROW_HOVER               6051
#define STYLE_TABLEROW_SELECT              6052
#define STYLE_INPUTDEFINER_FOCUS           6060
#define STYLE_NEWMACHINE_SECTION           6080
#define STYLE_NEWMACHINE_SECTION_SELECTED  6081
#define STYLE_NEWMACHINE_SEARCHFIELD       6100
#define STYLE_NEWMACHINE_SECTION_HEADER    6120

struct psy_ui_Defaults;

void initdarkstyles(struct psy_ui_Defaults*);
void initlightstyles(struct psy_ui_Defaults*);

#ifdef __cplusplus
}
#endif

#endif /* STYLES_H */

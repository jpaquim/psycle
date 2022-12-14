/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(STYLES_H)
#define STYLES_H

/* ui */
#include <uistyles.h>

#ifdef __cplusplus
extern "C" {
#endif
	
typedef enum {
	STYLE_SEQ_PROGRESS = 31004,	
	STYLE_SEQEDT_TRACKS = 31020,
	STYLE_SEQEDT_ITEM = 31030,
	STYLE_SEQEDT_ITEM_HOVER = 31031,
	STYLE_SEQEDT_ITEM_SELECTED = 31032,
	STYLE_SEQEDT_SAMPLE = 31033,
	STYLE_SEQEDT_SAMPLE_HOVER = 31034,
	STYLE_SEQEDT_SAMPLE_SELECTED = 31035,
	STYLE_SEQEDT_MARKER = 31036,
	STYLE_SEQEDT_MARKER_HOVER = 31037,
	STYLE_SEQEDT_MARKER_SELECTED = 31038,
	STYLE_SEQEDT_EVENT = 31039,
	STYLE_SEQEDT_TRACK_NUMBER = 31040,
	STYLE_SEQEDT_DESCITEM_HOVER = 31041,
	STYLE_SEQEDT_DESCITEM_SELECTED = 31042,
	STYLE_SEQEDT_TOOLBAR = 31043,	
	STYLE_SEQEDT_RULER_TOP = 31045,
	STYLE_SEQEDT_RULER_BOTTOM = 31046,
	STYLE_SEQEDT_RULER_CURSOR = 31047,
	STYLE_SEQEDT_LEFT = 31048,
	STYLE_SEQEDT_LOOP = 31049,
	STYLE_SEQEDT_LOOP_HOVER = 31050,
	STYLE_SEQEDT_LOOP_ACTIVE = 31051,
	STYLE_SEQEDT_PROPERTIES = 31052,
	STYLE_SEQVIEW = 31059,
	STYLE_SEQLISTVIEW = 31060,
	STYLE_SEQLISTVIEW_SELECT = 31062,
	STYLE_SEQLISTVIEW_TRACK = 31063,
	STYLE_SEQLISTVIEW_TRACK_SELECT = 31064,
	STYLE_SEQLISTVIEW_FOCUS = 31065,	
	STYLE_SEQVIEW_BUTTONS = 31070,
	STYLE_SEQVIEW_BUTTON = 31080,
	STYLE_SEQVIEW_BUTTON_HOVER = 31081,
	STYLE_SEQVIEW_BUTTON_SELECT = 31082,	
	STYLE_STATUSBAR = 32000,
	STYLE_VIEWSTATUSBAR = 32010,	
	STYLE_ZOOMBOX = 32040,
	STYLE_ZOOMBOX_EDIT = 32041,	
	STYLE_STEPSEQUENCER_TILE = 32081,
	STYLE_STEPSEQUENCER_TILE_ACTIVE = 32082,
	STYLE_STEPSEQUENCER_TILE_SELECT = 32083,
	STYLE_TOP = 33020,
	STYLE_TOPROWS = 33060,	
	STYLE_TRACKSCOPES = 35000,
	STYLE_TRACKSCOPE = 35010,	
	STYLE_BUTTONS = 35042,
	STYLE_ABOUT = 35060,	
	STYLE_PLAYBAR = 36000,
	STYLE_DURATION_TIME = 36020,
	STYLE_KEY = 36040,
	STYLE_KEY_HOVER = 36041,
	STYLE_KEY_SELECT = 36042,
	STYLE_KEY_ACTIVE = 36043,
	STYLE_KEY_SHIFT_SELECT = 36044,
	STYLE_KEY_ALT_SELECT = 36045,
	STYLE_KEY_CTRL_SELECT = 36046,
	STYLE_TABLEROW = 36050,
	STYLE_TABLEROW_HOVER = 36051,
	STYLE_TABLEROW_SELECT = 36052,	
	STYLE_NEWMACHINE_SECTION = 36100,
	STYLE_NEWMACHINE_SECTION_SELECTED = 36101,	
	STYLE_SEARCHFIELD = 36140,
	STYLE_SEARCHFIELD_SELECT = 36141,
	STYLE_NEWMACHINE_RESCANBAR = 36160,
	STYLE_NEWMACHINE_SECTIONBAR = 36200,
	STYLE_NEWMACHINE_SECTION_HEADER = 36220,
	STYLE_NEWMACHINE_FILTERBAR = 36260,
	STYLE_NEWMACHINE_FILTERBAR_LABEL = 36270,
	STYLE_NEWMACHINE_DETAIL = 36280,
	STYLE_NEWMACHINE_DETAIL_GROUP = 36285,
	STYLE_PROPERTYVIEW_MAINSECTION = 36300,
	STYLE_PROPERTYVIEW_MAINSECTIONHEADER = 36320,
	STYLE_PROPERTYVIEW_SUBSECTION = 36321,
	STYLE_PROPERTYVIEW_SUBSECTIONHEADER = 36322,
	STYLE_PROPERTYVIEW_SECTION = 36323,
	STYLE_PROPERTYVIEW_SECTIONHEADER = 36324,	
	STYLE_RECENTVIEW_MAINSECTIONHEADER = 36360,	
	STYLE_RECENTVIEW_LINE = 36370,
	STYLE_RECENTVIEW_LINE_HOVER = 36371,
	STYLE_RECENTVIEW_LINE_SELECT = 36372,
	STYLE_HEADER = 36380,
	STYLE_MAINVIEWTOPBAR = 36400,
	STYLE_NAVBAR = 36420,
	STYLE_CLIPBOX = 36440,
	STYLE_CLIPBOX_SELECT = 36441,
	STYLE_MAIN_VU = 36450,
	STYLE_LABELPAIR = 36460,
	STYLE_LABELPAIR_FIRST = 36461,
	STYLE_LABELPAIR_SECOND = 36462,
	STYLE_TERM_BUTTON = 36480,
	STYLE_TERM_BUTTON_WARNING = 36485,
	STYLE_TERM_BUTTON_ERROR = 36490,
	STYLE_GREET = 36500,
	STYLE_GREET_TOP = 36501,	
	STYLE_LPB_NUMLABEL = 36580,
	STYLE_TIMEBAR_NUMLABEL = 36600,	
	STYLE_MACPARAM_TITLE = 36640,
	STYLE_MACPARAM_TOP = 36660,
	STYLE_MACPARAM_BOTTOM = 36680,
	STYLE_MACPARAM_TOP_ACTIVE = 36700,
	STYLE_MACPARAM_BOTTOM_ACTIVE = 36720,
	STYLE_MACPARAM_KNOB = 36740,
	STYLE_MACPARAM_SLIDER = 36760,
	STYLE_MACPARAM_SLIDERKNOB = 36780,
	STYLE_MACPARAM_SWITCHON = 36800,
	STYLE_MACPARAM_SWITCHOFF = 36820,
	STYLE_MACPARAM_CHECKON = 36840,
	STYLE_MACPARAM_CHECKOFF = 36860,
	STYLE_MACPARAM_VUON = 36880,
	STYLE_MACPARAM_VUOFF = 36900,	
	STYLE_MV_PROPERTIES = 36911,
	STYLE_MV_WIRES = 36912,
	STYLE_MV_STACK = 36913,
	STYLE_MV_NEWMACHINE = 36914,
	STYLE_MV_MASTER = 36920,
	STYLE_MV_MASTER_NAME = 36921,
	STYLE_MV_GENERATOR = 36940,
	STYLE_MV_GENERATOR_NAME = 36941,
	STYLE_MV_GENERATOR_MUTE = 36942,
	STYLE_MV_GENERATOR_MUTE_SELECT = 36943,
	STYLE_MV_GENERATOR_SOLO = 36944,
	STYLE_MV_GENERATOR_SOLO_SELECT = 36945,
	STYLE_MV_GENERATOR_VU = 36946,
	STYLE_MV_GENERATOR_VU0 = 36947,
	STYLE_MV_GENERATOR_VUPEAK = 36948,
	STYLE_MV_GENERATOR_PAN = 36949,
	STYLE_MV_GENERATOR_PAN_SLIDER = 36950,
	STYLE_MV_EFFECT = 36960,
	STYLE_MV_EFFECT_NAME = 36961,
	STYLE_MV_EFFECT_MUTE = 36962,
	STYLE_MV_EFFECT_MUTE_SELECT = 36963,
	STYLE_MV_EFFECT_BYPASS = 36964,
	STYLE_MV_EFFECT_BYPASS_SELECT = 36965,
	STYLE_MV_EFFECT_VU = 36966,
	STYLE_MV_EFFECT_VU0 = 36967,
	STYLE_MV_EFFECT_VUPEAK = 36968,
	STYLE_MV_EFFECT_PAN = 36969,
	STYLE_MV_EFFECT_PAN_SLIDER = 36970,
	STYLE_MV_ARROW = 36971,
	STYLE_MV_CHECK = 36972,
	STYLE_MV_KNOB = 36973,
	STYLE_MV_LABEL = 36974,
	STYLE_MV_HEADER = 36975,
	STYLE_MV_LEVEL = 36976,
	STYLE_MV_WIRE = 36980,
	STYLE_MV_WIRE_HOVER = 37000,
	STYLE_MV_WIRE_SELECT = 37020,
	STYLE_MV_WIRE_POLY = 37040,	
	STYLE_PATTERNVIEW = 37060,	
	STYLE_PV_TRACK_VIEW = 37070,
	STYLE_PV_TRACK_VIEW_SELECT = 37071,
	STYLE_PV_TRACK_HEADER = 37080,
	STYLE_PV_TRACK_HEADER_DIGITX0 = 37100,
	STYLE_PV_TRACK_HEADER_DIGIT0X = 37120,
	STYLE_PV_TRACK_HEADER_MUTE = 37140,
	STYLE_PV_TRACK_HEADER_MUTE_SELECT = 37141,
	STYLE_PV_TRACK_HEADER_SOLO = 37160,
	STYLE_PV_TRACK_HEADER_SOLO_SELECT = 37161,
	STYLE_PV_TRACK_HEADER_RECORD = 37180,
	STYLE_PV_TRACK_HEADER_RECORD_SELECT = 37181,
	STYLE_PV_TRACK_HEADER_PLAY = 37200,
	STYLE_PV_TRACK_HEADER_PLAY_SELECT = 37201,	
	STYLE_PV_TRACK_HEADER_TEXT = 37202,
	STYLE_PV_TRACK_CLASSIC_HEADER = 37220,
	STYLE_PV_TRACK_CLASSIC_HEADER_DIGITX0 = 37221,
	STYLE_PV_TRACK_CLASSIC_HEADER_DIGIT0X = 37222,
	STYLE_PV_TRACK_CLASSIC_HEADER_MUTE = 37223,
	STYLE_PV_TRACK_CLASSIC_HEADER_MUTE_SELECT = 37224,
	STYLE_PV_TRACK_CLASSIC_HEADER_SOLO = 37225,
	STYLE_PV_TRACK_CLASSIC_HEADER_SOLO_SELECT = 37226,
	STYLE_PV_TRACK_CLASSIC_HEADER_RECORD = 37227,
	STYLE_PV_TRACK_CLASSIC_HEADER_RECORD_SELECT = 37228,
	STYLE_PV_TRACK_CLASSIC_HEADER_PLAY = 37229,
	STYLE_PV_TRACK_CLASSIC_HEADER_PLAY_SELECT = 37230,
	STYLE_PV_TRACK_CLASSIC_HEADER_TEXT = 37231,
	STYLE_PV_TRACK_TEXT_HEADER = 37240,
	STYLE_PV_TRACK_TEXT_HEADER_DIGITX0 = 37241,
	STYLE_PV_TRACK_TEXT_HEADER_DIGIT0X = 37242,
	STYLE_PV_TRACK_TEXT_HEADER_MUTE = 37243,
	STYLE_PV_TRACK_TEXT_HEADER_MUTE_SELECT = 37244,
	STYLE_PV_TRACK_TEXT_HEADER_SOLO = 37245,
	STYLE_PV_TRACK_TEXT_HEADER_SOLO_SELECT = 37246,
	STYLE_PV_TRACK_TEXT_HEADER_RECORD = 37247,
	STYLE_PV_TRACK_TEXT_HEADER_RECORD_SELECT = 37248,
	STYLE_PV_TRACK_TEXT_HEADER_PLAY = 37249,
	STYLE_PV_TRACK_TEXT_HEADER_PLAY_SELECT = 37250,
	STYLE_PV_TRACK_TEXT_HEADER_TEXT = 37251,	
	STYLE_PV_ROW = 37260,
	STYLE_PV_ROW_SELECT = 37270,
	STYLE_PV_ROWBEAT = 37280,
	STYLE_PV_ROWBEAT_SELECT = 37290,
	STYLE_PV_ROW4BEAT = 37300,
	STYLE_PV_ROW4BEAT_SELECT = 37320,
	STYLE_PV_CURSOR = 37340,
	STYLE_PV_CURSOR_SELECT = 37360,
	STYLE_PV_PLAYBAR = 37380,
	STYLE_ENVELOPE = 37400,
	STYLE_ENVELOPE_POINT = 37401,
	STYLE_ENVELOPE_CURVE = 37402,
	STYLE_ENVELOPE_GRID = 37403,
	STYLE_ENVELOPE_SUSTAIN = 37404,
	STYLE_ENVELOPE_RULER = 37405,
	STYLE_DIALOG_BUTTONS = 37420,
	STYLE_DIALOG_BUTTON = 37421,
	STYLE_DIALOG_BUTTON_HOVER = 37422,
	STYLE_DIALOG_BUTTON_SELECT = 37423,
	STYLE_SCANTASK = 374440,	
	STYLE_FILEBOX_DIR_PANE = 374460,
	STYLE_FILEBOX_FILE_PANE = 374470,
	STYLE_FILEVIEW_DIRBAR = 374480,
	STYLE_FILEVIEW_FILTERS = 374490,
	STYLE_FILEVIEW_LINKS = 374500,	
	STYLE_FILEVIEW_OPTIONS = 374520,
	STYLE_GEAR = 374540,
	STYLE_SEPARATOR = 374560,
	STYLE_BOX = 374570,
	STYLE_CLIENT_VIEW = 374580,
	STYLE_SIDE_VIEW = 374590,
	STYLE_VIEW_HEADER = 374600,
	STYLE_TITLEBAR = 374610
} HostStyles;

void init_host_styles(psy_ui_Styles*, psy_ui_ThemeMode);
void init_psycletheme_styles(psy_ui_Styles*);
void init_machineview_styles(psy_ui_Styles*);
void init_machineparam_styles(psy_ui_Styles*);
void init_patternview_styles(psy_ui_Styles*);
void init_envelope_styles(psy_ui_Styles*);

#ifdef __cplusplus
}
#endif

#endif /* STYLES_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(STYLES_H)
#define STYLES_H

#ifdef __cplusplus
extern "C" {
#endif
	
#define STYLE_SEQ_TAB                 100
#define STYLE_SEQ_TAB_HOVER           101
#define STYLE_SEQ_TAB_SELECT          102
#define STYLE_SEQ_TAB_NEW		      103
#define STYLE_SEQ_PROGRESS            104
#define STYLE_SEQEDT			      120
#define STYLE_SEQEDT_ITEM             130
#define STYLE_SEQEDT_ITEM_SELECTED    131
#define STYLE_SEQEDT_EVENT            132
#define STYLE_SEQLISTVIEW             140
#define STYLE_SEQLISTVIEW_SELECT      142
#define STYLE_SEQVIEW_BUTTONS         160
#define STYLE_SEQVIEW_BUTTON          180
#define STYLE_SEQVIEW_BUTTON_HOVER    181
#define STYLE_SEQVIEW_BUTTON_SELECT   182
#define STYLE_STATUSBAR               200
#define STYLE_MACHINEBOX              220
#define STYLE_ZOOMBOX                 240
#define STYLE_MACHINEVIEW             260
#define STYLE_STEPSEQUENCER           280
#define STYLE_TOP                     320
#define STYLE_TOPROWS                 360
#define STYLE_TOPROW0                 400
#define STYLE_TOPROW1                 440
#define STYLE_TOPROW2                 480
#define STYLE_TRACKSCOPES             500
#define STYLE_SONGPROPERTIES          520
#define STYLE_SONGPROPERTIES_COMMENTS 521
#define STYLE_SAMPLESVIEW             540
#define STYLE_ABOUT					  560
#define STYLE_INSTRUMENTVIEW		  580
#define STYLE_PLAYBAR				  600
#define STYLE_DURATION_TIME			  620
#define STYLE_KEY                     640
#define STYLE_KEY_HOVER               641
#define STYLE_KEY_SELECT              642
#define STYLE_TABLEROW                640
#define STYLE_TABLEROW_HOVER          641
#define STYLE_TABLEROW_SELECT         642

struct psy_ui_Defaults;

void initdarkstyles(struct psy_ui_Defaults*);
void initlightstyles(struct psy_ui_Defaults*);

#ifdef __cplusplus
}
#endif

#endif /* STYLES_H */

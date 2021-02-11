// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(STYLES_H)
#define STYLES_H

#ifdef __cplusplus
extern "C" {
#endif
	
#define STYLE_SEQ_TAB              100
#define STYLE_SEQ_TAB_HOVER        101
#define STYLE_SEQ_TAB_SELECT       102
#define STYLE_SEQ_TAB_NEW		   103
#define STYLE_SEQ_PROGRESS         104
#define STYLE_SEQEDT_ITEM          120
#define STYLE_SEQEDT_ITEM_SELECTED 121
#define STYLE_SEQEDT_EVENT         122
#define STYLE_SEQLISTVIEW          140
#define STYLE_STATUSBAR            160
#define STYLE_MACHINEBOX           180
#define STYLE_ZOOMBOX              200

struct psy_ui_Defaults;

void initlightstyles(struct psy_ui_Defaults*);
void initdarkstyles(struct psy_ui_Defaults*);

#ifdef __cplusplus
}
#endif

#endif /* STYLES_H */

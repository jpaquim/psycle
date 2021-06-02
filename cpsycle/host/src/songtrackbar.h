/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SONGTRACKBAR_H)
#define SONGTRACKBAR_H

/* host */
#include "workspace.h"
/* ui */
#include "uicombobox.h"
#include "uilabel.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** SongTrackBar
**
** Sets the number of tracks for your patterns. This is a global setting
** affecting all patterns and sequence tracks. The player will only play
** the tracks upto this number.
*/

typedef struct SongTrackBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label headerlabel;
	psy_ui_ComboBox trackbox;
	/* references */
	Workspace* workspace;
} SongTrackBar;

void songtrackbar_init(SongTrackBar*, psy_ui_Component* parent, Workspace*);

#ifdef __cplusplus
}
#endif

#endif /* SONGTRACKBAR_H */

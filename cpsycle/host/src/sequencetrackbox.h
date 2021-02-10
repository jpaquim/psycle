// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQUENCETRACKBOX_H)
#define SEQUENCETRACKBOX_H

// host
#include "workspace.h"
// audio
#include <patterns.h>
#include <sequence.h>
// ui
#include <uibutton.h>
#include <uicheckbox.h>
#include <uiedit.h>
#include <uilabel.h>
#include <uiscroller.h>
#include <uisplitbar.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SEQUENCETRACKBOXEVENT_SELECT = 1,
	SEQUENCETRACKBOXEVENT_MUTE,
	SEQUENCETRACKBOXEVENT_SOLO,
	SEQUENCETRACKBOXEVENT_DEL	
} SequenceTrackBoxEvent;

typedef struct SequenceTrackBox {
	psy_ui_RealRectangle position;	
	uintptr_t trackindex;
	bool selected;
	bool hover;
	psy_ui_Colour colour;
	psy_ui_Colour colour_highlight;
	psy_ui_Colour colour_font;
	psy_ui_Colour colour_fonthighlight;	
	const psy_ui_TextMetric* tm;
	bool showname;
	// references
	psy_audio_SequenceTrack* track;
	psy_audio_Sequence* sequence;
} SequenceTrackBox;

void sequencetrackbox_init(SequenceTrackBox*,
	psy_ui_RealRectangle position, const psy_ui_TextMetric*,
	psy_audio_SequenceTrack*,
	psy_audio_Sequence*,
	uintptr_t trackindex, bool selected, bool hover);

void sequencetrackbox_draw(SequenceTrackBox*, psy_ui_Graphics*);
SequenceTrackBoxEvent sequencetrackbox_hittest(const SequenceTrackBox*,
	psy_ui_RealPoint);

#ifdef __cplusplus
}
#endif

#endif /* SEQUENCETRACKBOX_H */

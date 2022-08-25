/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PIANOGRID_H)
#define PIANOROLL_H

/* host */
#include "patternhostcmds.h"
#include "pianogriddraw.h"
#include "pianokeyboard.h"
#include "pianoruler.h"
#include "workspace.h"
#include "zoombox.h"
/* ui */
#include <uicombobox.h>
#include <uilabel.h>
#include <uiscroller.h>
/* audio */
#include <pattern.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** Pianogrid
*/

typedef struct Pianogrid {
	/* inherits */
	psy_ui_Component component;
	/* internal */	
	psy_audio_PatternEntry* hoverpatternentry;	
	psy_audio_SequenceCursor old_cursor;
	psy_audio_SequenceCursor dragcursor;
	bool cursoronnoterelease;
	PianoTrackDisplay trackdisplay;
	psy_audio_SequenceCursor last_drag_cursor;
	bool prevent_context_menu;
	bool edit_mode;
	bool preventscrollleft;
	psy_audio_PatternEvent play_event;
	/* references */
	KeyboardState* keyboardstate;
	PianoGridState* gridstate;
	Workspace* workspace;
} Pianogrid;

void pianogrid_init(Pianogrid*, psy_ui_Component* parent, KeyboardState*,
	PianoGridState*, InputHandler*, Workspace*);

void pianogrid_invalidate_line(Pianogrid*, intptr_t line);
void pianogrid_invalidate_lines(Pianogrid*, intptr_t line1, intptr_t line2);
void pianogrid_invalidate_cursor(Pianogrid*);
void pianogrid_invalidate_playbar(Pianogrid*);
void pianogrid_update_cursor(Pianogrid*, psy_audio_SequenceCursor);
void pianogrid_set_cursor(Pianogrid*, psy_audio_SequenceCursor);
void pianogrid_onpatternchange(Pianogrid*);
void pianogrid_settrackdisplay(Pianogrid*, PianoTrackDisplay);

INLINE PianoTrackDisplay pianogrid_trackdisplay(const Pianogrid* self)
{
	assert(self);

	return self->trackdisplay;
}

INLINE const psy_audio_BlockSelection* pianogrid_selection(const Pianogrid* self)
{
	assert(self);

	return &self->gridstate->pv->selection;
}

INLINE psy_ui_Component* pianogrid_base(Pianogrid* self)
{
	assert(self);

	return &self->component;
}

void pianogrid_startdragselection(Pianogrid*, psy_audio_SequenceCursor);
void pianogrid_dragselection(Pianogrid*, psy_audio_SequenceCursor);
bool pianogrid_keyhittest(Pianogrid*, psy_audio_PatternNode*,
	uintptr_t track, uint8_t cursorkey);
bool pianogrid_scrollright(Pianogrid*, psy_audio_SequenceCursor);
bool pianogrid_scrollleft(Pianogrid*, psy_audio_SequenceCursor);
bool pianogrid_scrollup(Pianogrid*, psy_audio_SequenceCursor);
bool pianogrid_scrolldown(Pianogrid*, psy_audio_SequenceCursor);
void pianogrid_prevline(Pianogrid*);
void pianogrid_prevlines(Pianogrid*, uintptr_t lines, bool wrap);
void pianogrid_prevkeys(Pianogrid*, uint8_t lines, bool wrap);
void pianogrid_advanceline(Pianogrid*);
void pianogrid_advancelines(Pianogrid*, uintptr_t lines, bool wrap);
void pianogrid_advancekeys(Pianogrid*, uint8_t lines, bool wrap);

#ifdef __cplusplus
}
#endif

#endif /* PIANOGRID_H */

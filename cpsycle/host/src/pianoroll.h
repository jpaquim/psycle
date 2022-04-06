/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PIANOROLL_H)
#define PIANOROLL_H

/* host */
#include "patternhostcmds.h"
#include "pianokeyboard.h"
#include "pianoruler.h"
#include "pianogrid.h"
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
** Pianoroll
**
** The Pianoroll is another way than the normal tracker view to enter notes.
** It displays a Pattern selected by the SeqView in a roll with a piano
** keyboard at the left and the time line to the right.
*/

/* PianoBar */
typedef struct PianoBar {
	/* inherits */
	psy_ui_Component component;
	ZoomBox zoombox_beatwidth;
	ZoomBox zoombox_keyheight;
	psy_ui_Label beats;
	psy_ui_Label keys;	
	psy_ui_Label tracks;
	psy_ui_Button tracks_all;
	psy_ui_Button track_curr;
	psy_ui_Button tracks_active;
	psy_ui_Button edit_mode;
	psy_ui_Button select_mode;
} PianoBar;

void pianobar_init(PianoBar*, psy_ui_Component* parent);

INLINE psy_ui_Component* pianobar_base(PianoBar* self)
{
	assert(self);

	return &self->component;
}

/* Pianoroll */
typedef struct Pianoroll {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component top;
	PianoRuler header;
	psy_ui_Component left;
	psy_ui_ComboBox keytype;
	Pianogrid grid;
	psy_ui_Scroller scroller;
	PianoBar bar;	
	KeyboardState keyboardstate;
	psy_ui_Component keyboardpane;
	PianoKeyboard keyboard;	
	PianoGridState gridstate;	
	uintptr_t opcount;
	bool sync_pattern;	
	uintptr_t chord_begin;
	/* references */
	Workspace* workspace;
} Pianoroll;

void pianoroll_init(Pianoroll*, psy_ui_Component* parent, PatternViewState*,
	Workspace*);
void pianoroll_scroll_to_order(Pianoroll*);
void pianoroll_updatescroll(Pianoroll*);
void pianoroll_makecmds(psy_Property* parent);
bool pianoroll_handlecommand(Pianoroll*, uintptr_t cmd);

/* block operations */
void pianoroll_navup(Pianoroll*);
void pianoroll_navdown(Pianoroll*);
void pianoroll_enter(Pianoroll*);
void pianoroll_rowclear(Pianoroll*);
void pianoroll_blockstart(Pianoroll*);
void pianoroll_blockend(Pianoroll*);

INLINE psy_ui_Component* pianoroll_base(Pianoroll* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PIANOROLL_H */

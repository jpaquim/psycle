/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(TRACKERVIEW)
#define TRACKERVIEW

/* host */
#include "trackercolumn.h"
#include "trackergridstate.h"
#include "patternhostcmds.h"
#include "trackerlinenumbers.h"
/* ui */
#include <uibutton.h>
#include <uilabel.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** TrackerView
**
**  The TrackerView is where you enter notes. It displays a Pattern selected by
**  the SeqView as a tracker grid.
*/

typedef struct TrackerGrid {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_colresize;
	/* internal */	
	psy_audio_SequenceCursor old_cursor;
	psy_audio_SequenceCursor last_drag_cursor;
	bool effcursor_always_down;	
	bool prevent_cursor;
	psy_Table columns;
	bool prevent_event_driver;
	psy_ui_RealSize size;
	psy_ui_RealSize line_size;
	bool down;
	/* references */
	TrackerState* state;	
	Workspace* workspace;
} TrackerGrid;

void trackergrid_init(TrackerGrid*, psy_ui_Component* parent,
	TrackerState*, InputHandler*, Workspace*);

void trackergrid_build(TrackerGrid*);
void trackergrid_show_empty_data(TrackerGrid*, int showstate);
void trackergrid_invalidate_playbar(TrackerGrid*);
void trackergrid_invalidate_line(TrackerGrid*, intptr_t line);
void trackergrid_invalidate_lines(TrackerGrid*, intptr_t line1, intptr_t line2);
void trackergrid_invalidate_cursor(TrackerGrid*);
void trackergrid_invalidate_internal_cursor(TrackerGrid*,
	psy_audio_SequenceCursor);
void trackergrid_center_on_cursor(TrackerGrid*);
void trackergrid_set_center_mode(TrackerGrid*, intptr_t mode);
void trackergrid_tweak(TrackerGrid*, int slot, uintptr_t tweak,
	float normvalue);

INLINE const psy_audio_BlockSelection* trackergrid_selection(
	const TrackerGrid* self)
{
	return &self->state->pv->selection;
}

bool trackergrid_handle_command(TrackerGrid*, intptr_t cmd);
/* block menu */
void trackergrid_block_start(TrackerGrid*);
void trackergrid_block_end(TrackerGrid*);

INLINE bool trackergrid_midline(TrackerGrid* self)
{
	return self->state->midline;
}

INLINE psy_ui_Component* trackergrid_base(TrackerGrid* self)
{
	assert(self);

	return &self->component;
}

INLINE bool trackergrid_check_update(const TrackerGrid* self)
{
	const psy_audio_Pattern* pattern;

	pattern = patternviewstate_pattern_const(self->state->pv);
	if (pattern) {
		bool rv;
		uintptr_t opcount;

		opcount = pattern->opcount;
		rv = (opcount != self->component.opcount);
		((TrackerGrid*)self)->component.opcount = opcount;
		return rv;
	}
	return FALSE;
}

/* TrackerView */

typedef struct TrackerView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Scroller scroller;
	TrackerGrid grid;
	TrackerLineNumberView lines;	
	psy_ui_ScrollBar hscroll;
	/* references */
	Workspace* workspace;
} TrackerView;

void trackerview_init(TrackerView*, psy_ui_Component* parent, TrackerState*,
	Workspace*);

INLINE psy_ui_Component* trackerview_base(TrackerView* self)
{
	assert(self);

	return &self->scroller.component;
}

#ifdef __cplusplus
}
#endif

#endif /* TRACKERVIEW */

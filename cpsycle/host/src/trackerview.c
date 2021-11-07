/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "trackerview.h"
/* local */
#include "cmdsnotes.h"
#include "patterncmds.h"
#include "skingraphics.h"
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static int keycodetoint(uint32_t keycode)
{
	if (keycode >= '0' && keycode <= '9') {
		return keycode - '0';
	} else if (keycode >= 'A' && keycode <= 'Z') {
		return keycode - 'A' + 10;
	}
	return -1;
}

static void enterdigit(int digit, int newval, unsigned char* val)
{
	if (digit == 0) {
		*val = (*val & 0x0F) | ((newval & 0x0F) << 4);
	} else if (digit == 1) {
		*val = (*val & 0xF0) | (newval & 0x0F);
	}
}

static void entervaluecolumn(psy_audio_PatternEntry* entry, intptr_t column,
	intptr_t value)
{
	psy_audio_PatternEvent* ev;

	assert(entry);

	ev = psy_audio_patternentry_front(entry);
	switch (column) {
		case TRACKER_COLUMN_INST:
			ev->inst = (uint16_t)value;
			break;
		case TRACKER_COLUMN_MACH:
			ev->mach = (uint8_t)value;
			break;
		case TRACKER_COLUMN_VOL:
			ev->vol = (uint16_t)value;
			break;
		case TRACKER_COLUMN_CMD:
			ev->cmd = (uint8_t)value;
			break;
		case TRACKER_COLUMN_PARAM:
			ev->parameter = (uint8_t)value;
			break;
		default:
			break;
	}
}

/* TrackerGrid */
/* prototypes */
static void trackergrid_ondestroy(TrackerGrid*);
static void trackergrid_init_signals(TrackerGrid*);
static void trackergrid_dispose_signals(TrackerGrid*);
static void trackergrid_ondraw(TrackerGrid*, psy_ui_Graphics*);
static void trackergrid_drawbackground(TrackerGrid* self, psy_ui_Graphics* g,
	const psy_audio_BlockSelection* clip);
static void trackergrid_updatetrackevents(TrackerGrid*,
	const psy_audio_BlockSelection* clip);
static void trackergrid_onmousedown(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousemove(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmouseup(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousedoubleclick(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_dragselection(TrackerGrid*, psy_audio_SequenceCursor);
static void trackergrid_onscroll(TrackerGrid*, psy_ui_Component* sender);
static void trackergrid_clearmidline(TrackerGrid*);
static bool trackergrid_movecursorwhenpaste(TrackerGrid*);
static void trackergrid_enterdigitcolumn(TrackerGrid*, psy_audio_PatternEntry*,
	psy_audio_PatternCursor, intptr_t digitvalue);
static void trackergrid_inputvalue(TrackerGrid*, intptr_t value, intptr_t digit);
static void trackergrid_prevtrack(TrackerGrid*);
static void trackergrid_nexttrack(TrackerGrid*);
static void trackergrid_prevline(TrackerGrid*);
static void trackergrid_advanceline(TrackerGrid*);
static void trackergrid_prevlines(TrackerGrid*, uintptr_t lines, bool wrap);
static void trackergrid_advancelines(TrackerGrid*, uintptr_t lines, bool wrap);
static void trackergrid_home(TrackerGrid*);
static void trackergrid_end(TrackerGrid*);
static void trackergrid_rowdelete(TrackerGrid*);
static void trackergrid_rowclear(TrackerGrid*);
static void trackergrid_prevcol(TrackerGrid*);
static void trackergrid_nextcol(TrackerGrid*);
static void trackergrid_selectall(TrackerGrid*);
static void trackergrid_selectcol(TrackerGrid*);
static void trackergrid_selectmachine(TrackerGrid*);
static void trackergrid_oninput(TrackerGrid*, psy_audio_Player*,
	psy_audio_PatternEvent*);
static void trackergrid_setdefaultevent(TrackerGrid*,
	psy_audio_Pattern* defaultpattern, psy_audio_PatternEvent*);
static void trackergrid_enablepatternsync(TrackerGrid*);
static void trackergrid_preventpatternsync(TrackerGrid*);
static void trackergrid_resetpatternsync(TrackerGrid*);
static void trackergrid_ongotocursor(TrackerGrid*, Workspace* sender,
	psy_audio_PatternCursor*);
static psy_dsp_big_beat_t trackergrid_currseqoffset(TrackerGrid*);
static psy_audio_OrderIndex trackergrid_checkupdatecursorseqoffset(
	TrackerGrid*, psy_audio_PatternCursor* rv);
static bool trackergrid_ontrackercmds(TrackerGrid*, InputHandler*);
static bool trackergrid_onnotecmds(TrackerGrid*, InputHandler* sender);
static bool trackergrid_onmidicmds(TrackerGrid*, InputHandler* sender);
static void trackergrid_onplaylinechanged(TrackerGrid*, Workspace* sender);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(TrackerGrid* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			trackergrid_ondestroy;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			trackergrid_ondraw;		
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			trackergrid_onmousedown;
		vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			trackergrid_onmousemove;
		vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			trackergrid_onmouseup;
		vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			trackergrid_onmousedoubleclick;		
		vtable_initialized = TRUE;
	}
	return &vtable;
}
/* implementation */
void trackergrid_init(TrackerGrid* self, psy_ui_Component* parent, psy_ui_Component* view,
	TrackConfig* trackconfig, TrackerState* state,
	Workspace* workspace)
{
	assert(self);

	/* init base component */
	psy_ui_component_init(&self->component, parent, view);
	psy_ui_component_setvtable(&self->component, vtable_init(self));	
	/* set references */
	self->workspace = workspace;	
	patterncmds_init(&self->cmds, workspace);
	psy_table_init(&self->columns);
	trackerstate_init(&self->defaultgridstate, trackconfig, workspace_song(workspace));	
	trackergrid_setsharedgridstate(self, state, trackconfig);	
	trackergrid_storecursor(self);
	/* setup base component */
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	psy_ui_component_doublebuffer(&self->component);	
	trackergrid_init_signals(self);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HEXPAND);
	psy_ui_component_setscrollstep_height(trackergrid_base(self),
		self->state->lineheight);
	/* init internal */
	self->syncpattern = TRUE;	
	self->state->midline = FALSE;	
	self->chordmode = FALSE;
	self->chordbegin = 0;
	self->wraparound = TRUE;
	self->state->showemptydata = FALSE;	
	self->movecursoronestep = FALSE;
	self->ft2home = TRUE;
	self->ft2delete = TRUE;
	self->effcursoralwaysdown = FALSE;	
	self->pgupdownstep = 4;
	self->preventscrolltop = FALSE;
	self->preventeventdriver = FALSE;
	self->notestabmode = psy_dsp_NOTESTAB_DEFAULT;
	inputhandler_connect(&workspace->inputhandler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_CMD, "tracker", psy_INDEX_INVALID,
		self, (fp_inputhandler_input)trackergrid_ontrackercmds);
	inputhandler_connect(&workspace->inputhandler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_CMD, "notes",
		psy_INDEX_INVALID, self, (fp_inputhandler_input)trackergrid_onnotecmds);
	inputhandler_connect(&workspace->inputhandler, INPUTHANDLER_VIEW,
		psy_EVENTDRIVER_MIDI, "", VIEW_ID_PATTERNVIEW, 
		self, (fp_inputhandler_input)trackergrid_onmidicmds);
	psy_audio_blockselection_init(&self->state->pv.selection);
	/* handle midline invalidation */
	psy_signal_connect(&self->component.signal_scroll, self,
		trackergrid_onscroll);		
	psy_signal_connect(&self->workspace->signal_gotocursor, self,
		trackergrid_ongotocursor);	
}

void trackergrid_ondestroy(TrackerGrid* self)
{
	assert(self);

	trackergrid_dispose_signals(self);
	psy_table_dispose(&self->columns);
	trackerstate_dispose(&self->defaultgridstate);	
}

void trackergrid_init_signals(TrackerGrid* self)
{
	assert(self);
	
	psy_signal_init(&self->signal_colresize);
}

void trackergrid_dispose_signals(TrackerGrid* self)
{
	assert(self);
	
	psy_signal_dispose(&self->signal_colresize);
}

void trackergrid_setsharedgridstate(TrackerGrid* self, TrackerState*
	state, TrackConfig* trackconfig)
{
	assert(self);
	
	if (state) {
		self->state = state;
		psy_signal_connect(&self->workspace->signal_playlinechanged, self,
			trackergrid_onplaylinechanged);
	} else {		
		self->state = &self->defaultgridstate;
	}
}

void trackergrid_ondraw(TrackerGrid* self, psy_ui_Graphics* g)
{
	psy_audio_BlockSelection clip;
		
	trackerstate_lineclip(self->state, &g->clip, &clip);
	trackerstate_clip(self->state, &g->clip, &clip);
	trackergrid_drawbackground(self, g, &clip);
	/* prepares entry draw done in trackergridcolumn */
	if (patternviewstate_pattern(&self->state->pv)) {
		trackergrid_updatetrackevents(self, &clip);	
	}
}

void trackergrid_drawbackground(TrackerGrid* self, psy_ui_Graphics* g,
	const psy_audio_BlockSelection* clip)
{
	psy_List* p;
	psy_List* q;
	uintptr_t track;
	psy_ui_RealSize clientsize;
	double scrolltop;	
	
	clientsize = psy_ui_component_clientsize_px(&self->component);
	scrolltop = psy_ui_component_scrolltop_px(&self->component);
	q = psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE);	
	for (p = q, track = 0; p != NULL; p = p->next, ++track) {
		psy_ui_Component* column;
		psy_ui_RealRectangle columnposition;		

		column = (psy_ui_Component*)p->entry;
		columnposition = psy_ui_component_position(column);
		psy_ui_drawsolidrectangle(g,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(columnposition.left, scrolltop),
				psy_ui_realsize_make(
					columnposition.right - columnposition.left,
					clientsize.height)),
			patternviewskin_separatorcolour(self->state->pv.skin, track,
				patternviewstate_numsongtracks(&self->state->pv)));
	}
	psy_list_free(q);
}


void trackergrid_updatetrackevents(TrackerGrid* self,
	const psy_audio_BlockSelection* clip)
{
	uintptr_t track;
	uintptr_t maxlines;	
	double offset;
	double seqoffset;
	double length;	
	psy_audio_SequenceTrackIterator ite;	
	uintptr_t line;	

	assert(self);
	
	trackereventtable_clearevents(&self->state->trackevents);
	ite.pattern = self->state->pv.pattern;
	ite.patternnode = NULL;
	ite.patterns = &self->workspace->song->patterns;
	seqoffset = 0.0;
	length = ite.pattern->length;
	offset = clip->topleft.cursor.offset;
	if (!self->state->pv.singlemode && patternviewstate_sequence(&self->state->pv)) {
		psy_audio_SequenceTrackNode* tracknode;
		psy_audio_Sequence* sequence;

		sequence = patternviewstate_sequence(&self->state->pv);
		tracknode = psy_list_at(sequence->tracks,
			self->state->pv.cursor.orderindex.track);
		if (!tracknode) {
			tracknode = sequence->tracks;
		}
	 	ite = psy_audio_sequence_begin(sequence,			
			tracknode ? (psy_audio_SequenceTrack*)tracknode->entry : NULL,
			offset);
	 	if (ite.sequencentrynode) {
			seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);	 		
	 		if (ite.pattern) {
	 			length = ite.pattern->length;
	 		}		
	 	}
	} else {
		ite.sequencentrynode = NULL;
		ite.patternnode = psy_audio_pattern_greaterequal(
			patternviewstate_pattern(&self->state->pv),
			(psy_dsp_big_beat_t)offset - seqoffset);
	}		
	line = trackerstate_beattoline(self->state, offset);	
	maxlines = trackerstate_numlines(self->state);
	while (offset <= clip->bottomright.cursor.offset && line < maxlines) {
		bool fill;
		
		fill = !(offset >= seqoffset && offset < seqoffset + length) || !ite.patternnode;		
		/* draw trackline */
		for (track = clip->topleft.cursor.track; track < clip->bottomright.cursor.track;
			++track) {
			bool hasevent = FALSE;
			
			while (!fill && ite.patternnode &&
					psy_audio_sequencetrackiterator_patternentry(&ite)->track <= track &&
					psy_dsp_testrange_e(
						psy_audio_sequencetrackiterator_offset(&ite),
						offset,
						trackerstate_bpl(self->state))) {
				psy_audio_PatternEntry* entry;

				entry = psy_audio_sequencetrackiterator_patternentry(&ite);
				if (entry->track == track) {
					psy_List** trackevents;

					trackevents = trackereventtable_track(&self->state->trackevents, entry->track);
					psy_list_append(trackevents, entry);
					psy_list_next(&ite.patternnode);
					hasevent = TRUE;
					break;
				}
				psy_list_next(&ite.patternnode);
			}
			if (!hasevent) {				
				psy_List** trackevents;

				trackevents = trackereventtable_track(&self->state->trackevents, track);
				psy_list_append(trackevents, NULL);
			} else if (ite.patternnode && ((psy_audio_PatternEntry*)(ite.patternnode->entry))->track <= track) {
				fill = TRUE;
			}			
		}
		/* skip remaining events of the line */
		while (ite.patternnode && (offset < seqoffset + length) && 
			(psy_audio_sequencetrackiterator_offset(&ite) + psy_dsp_epsilon * 2 <
				offset + trackerstate_bpl(self->state))) {
			psy_list_next(&ite.patternnode);
		}
		offset += trackerstate_bpl(self->state);
		if (offset >= seqoffset + length) {
			/* go to next seqentry or end draw */
			if (ite.sequencentrynode && ite.sequencentrynode->next) {
				psy_audio_sequencetrackiterator_inc_entry(&ite);
				seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);				
				offset = seqoffset;
				if (ite.pattern) {
					length = ite.pattern->length;
				} else {
					break;
				}
			} else {
				break;
			}
		}		
	}
	self->state->trackevents.seqoffset = seqoffset;
	self->state->trackevents.clip = *clip;	
	self->state->trackevents.currcursorline =
		trackerstate_beattoline(self->state,
			psy_audio_sequencecursor_offset(&self->state->pv.cursor));
	self->state->trackevents.currplaybarline =
		trackerstate_beattoline(self->state,
		self->workspace->lastplayposition -
		((self->state->pv.singlemode)
			? self->state->pv.cursor.seqoffset
			: 0.0));	
}

psy_audio_OrderIndex trackergrid_checkupdatecursorseqoffset(TrackerGrid* self,
		psy_audio_PatternCursor* rv)
{
	psy_audio_Sequence* sequence;
	
	sequence = patternviewstate_sequence(&self->state->pv);
	if (sequence && (rv->offset < 0 || rv->offset >=
			psy_audio_pattern_length(self->state->pv.pattern))) {
		uintptr_t order;
		psy_audio_SequenceEntry* entry;

		order = psy_audio_sequence_order(sequence,
			0, rv->offset /* + rv->seqoffset */ );
		if (order != psy_INDEX_INVALID) {
			entry = psy_audio_sequence_entry(sequence,
				psy_audio_orderindex_make(
				self->state->pv.cursor.orderindex.track, order));
			if (entry) {				
				// rv->offset -= (psy_audio_sequenceentry_offset(entry) - rv->seqoffset);
				// rv->seqoffset = psy_audio_sequenceentry_offset(entry);
			}
			return psy_audio_orderindex_make(
				self->state->pv.cursor.orderindex.track,
				order);
		}
	}
	return psy_audio_orderindex_zero();
}

psy_dsp_big_beat_t trackergrid_currseqoffset(TrackerGrid* self)
{
	psy_audio_SequenceEntry* entry;
	psy_audio_Sequence* sequence;

	sequence = patternviewstate_sequence(&self->state->pv);
	if (sequence) {
		entry = psy_audio_sequence_entry(sequence,
			self->state->pv.cursor.orderindex);
		if (entry) {
			return psy_audio_sequenceentry_offset(entry);
		}
	}
	return 0.0;
}

void trackergrid_prevtrack(TrackerGrid* self)
{	
	psy_audio_PatternCursorNavigator cursornavigator;

	assert(self);

	psy_audio_patterncursornavigator_init(&cursornavigator,
		&self->state->pv.cursor.cursor, patternviewstate_pattern(&self->state->pv),
		trackerstate_bpl(self->state), self->wraparound, 0);
	if (psy_audio_patterncursornavigator_prevtrack(&cursornavigator,
			patternviewstate_numsongtracks(&self->state->pv))) {
		trackergrid_scrollleft(self, self->state->pv.cursor);
		trackergrid_invalidatecursor(self);
	} else if (trackergrid_scrollright(self, self->state->pv.cursor)) {
		trackergrid_invalidatecursor(self);
	}	
}

void trackergrid_storecursor(TrackerGrid* self)
{
	assert(self);

	self->oldcursor = self->state->pv.cursor;
}

void trackergrid_nexttrack(TrackerGrid* self)
{
	psy_audio_PatternCursorNavigator cursornavigator;	

	psy_audio_patterncursornavigator_init(&cursornavigator,
		&self->state->pv.cursor.cursor, patternviewstate_pattern(&self->state->pv),
		trackerstate_bpl(self->state), self->wraparound, 0);
	if (psy_audio_patterncursornavigator_nexttrack(&cursornavigator,
			patternviewstate_numsongtracks(&self->state->pv))) {
		if (trackergrid_scrollleft(self, self->state->pv.cursor)) {
			trackergrid_invalidatecursor(self);
		}		
	} else {
		trackergrid_scrollright(self, self->state->pv.cursor);
		trackergrid_invalidatecursor(self);
	}
}

bool trackergrid_scrollup(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{
	intptr_t line;
	intptr_t topline;	
	double top;

	line = trackerstate_beattoline(self->state,
		psy_audio_sequencecursor_offset(&cursor));
	top = self->state->lineheightpx * line;	
	if (self->state->midline) {
		psy_ui_RealSize gridsize;		

		gridsize = psy_ui_component_scrollsize_px(&self->component);
		topline = (intptr_t)(gridsize.height / self->state->lineheightpx / 2.0);
	} else {
		topline = 0;
	}
	if (psy_ui_component_scrolltop_px(&self->component) +
			topline * self->state->lineheightpx > top) {
		intptr_t dlines;		
		
		dlines = (intptr_t)((psy_ui_component_scrolltop_px(&self->component) +
			topline * self->state->lineheightpx - top) /
			(self->state->lineheightpx));				
		self->state->cursorchanging = TRUE;		
		psy_ui_component_setscrolltop_px(&self->component,			
			psy_ui_component_scrolltop_px(&self->component) -
			psy_ui_component_scrollstep_height_px(&self->component) * dlines);
		return FALSE;
	}
	return TRUE;
}

bool trackergrid_scrolldown(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{
	intptr_t line;
	intptr_t visilines;	
	
	visilines = self->state->visilines;
	if (self->state->midline) {
		visilines /= 2;
	} else {
		--visilines;
	}
	line = trackerstate_beattoline(self->state,
		psy_audio_sequencecursor_offset(&cursor));
	if (visilines < line - psy_ui_component_scrolltop_px(&self->component) /
			self->state->lineheightpx) {
		intptr_t dlines;

		dlines = (intptr_t)
			(line - psy_ui_component_scrolltop_px(&self->component) /
			self->state->lineheightpx - visilines);
		self->state->cursorchanging = TRUE;
		psy_ui_component_setscrolltop_px(&self->component,			
			psy_ui_component_scrolltop_px(&self->component) +
			psy_ui_component_scrollstep_height_px(&self->component) * dlines);
		return FALSE;
	}
	return TRUE;
}

bool trackergrid_scrollleft(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{	
	psy_ui_Component* column;
	uintptr_t index;

	assert(self);
	
	column = psy_ui_component_intersect(trackergrid_base(self),
		psy_ui_realpoint_make(psy_ui_component_scrollleftpx(&self->component),
			0.0), &index);
	if (index != psy_INDEX_INVALID) {		
		if (index > cursor.cursor.track) {
			psy_ui_RealRectangle position;

			column = (psy_ui_Component*)psy_table_at(&self->columns, cursor.cursor.track);
			position = psy_ui_component_position(column);
			psy_ui_component_setscrollleft(&self->component,
				psy_ui_value_make_px(position.left));
			return FALSE;
		}
	}
	return TRUE;
}

bool trackergrid_scrollright(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{
	uintptr_t visitracks;
	uintptr_t tracks;
	psy_ui_RealSize size;	
	const psy_ui_TextMetric* tm;	
	intptr_t trackright;
	intptr_t trackleft;

	assert(self);

	size = psy_ui_component_clientsize_px(&self->component);
	tm = psy_ui_component_textmetric(&self->component);	
	trackleft = trackerstate_pxtotrack(self->state,
		psy_ui_component_scrollleftpx(&self->component));
	trackright = trackerstate_pxtotrack(self->state,
		size.width +
		psy_ui_component_scrollleftpx(&self->component));	
	visitracks = trackright - trackleft;
	tracks = cursor.cursor.track + 1;
	if (tracks > trackleft + visitracks) {		
		psy_ui_Component* column;

		column = (psy_ui_Component*)psy_table_at(&self->columns, tracks - visitracks);
		if (column) {
			psy_ui_RealRectangle position;
			 
			position = psy_ui_component_position(column);
			psy_ui_component_setscrollleft(&self->component,
				psy_ui_value_make_px(position.left));
		}
		return FALSE;
	}
	return TRUE;
}

void trackergrid_prevline(TrackerGrid* self)
{	
	assert(self);

	trackergrid_prevlines(self, workspace_cursorstep(self->workspace),
		self->wraparound);
}

void trackergrid_advanceline(TrackerGrid* self)
{
	assert(self);

	trackergrid_advancelines(self, workspace_cursorstep(self->workspace),
		self->wraparound);
}

void trackergrid_advancelines(TrackerGrid* self, uintptr_t lines, bool wrap)
{
	assert(self);

	if (patternviewstate_pattern(&self->state->pv)) {
		psy_audio_PatternCursorNavigator cursornavigator;
		bool restorewrap;

		restorewrap = wrap;
		if (!self->state->pv.singlemode) {
			wrap = TRUE;
		}
		psy_audio_patterncursornavigator_init(&cursornavigator,
			&self->state->pv.cursor.cursor,
			patternviewstate_pattern(&self->state->pv),
			trackerstate_bpl(self->state), wrap, 0);		
		if (psy_audio_patterncursornavigator_advancelines(&cursornavigator, lines)) {
			trackergrid_scrolldown(self, self->state->pv.cursor);
		} else if (!self->state->pv.singlemode) {
			psy_audio_OrderIndex index;

			self->state->pv.cursor.cursor.offset =
				psy_audio_pattern_length(self->state->pv.pattern);
			index = trackergrid_checkupdatecursorseqoffset(self,
				&self->state->pv.cursor.cursor);
			if (psy_audio_orderindex_valid(&index)) {
				self->preventscrolltop = TRUE;
				self->state->pv.cursor.orderindex = index;
				if (self->workspace && workspace_song(self->workspace)) {
					psy_audio_sequence_setcursor(
						psy_audio_song_sequence(workspace_song(self->workspace)),
						self->state->pv.cursor);
				}				
				self->preventscrolltop = FALSE;
			} else if (restorewrap) {
				self->state->pv.cursor.cursor.offset = 0;
				self->preventscrolltop = TRUE;
				self->state->pv.cursor.orderindex = psy_audio_orderindex_make(
					self->state->pv.cursor.orderindex.track, 0);
				if (self->workspace && workspace_song(self->workspace)) {
					psy_audio_sequence_setcursor(
						psy_audio_song_sequence(workspace_song(self->workspace)),
						self->state->pv.cursor);
				}
				self->preventscrolltop = FALSE;
				trackergrid_scrollup(self, self->state->pv.cursor);
				trackergrid_storecursor(self);
				return;
			} else {
				self->state->pv.cursor.cursor.offset =
					psy_audio_pattern_length(self->state->pv.pattern) -
					trackerstate_bpl(self->state);				
			}
			trackergrid_scrolldown(self, self->state->pv.cursor);
		} else {
			trackergrid_scrollup(self, self->state->pv.cursor);
		}		
		if (self->workspace && workspace_song(self->workspace)) {
			psy_audio_sequence_setcursor(
				psy_audio_song_sequence(workspace_song(self->workspace)),
				self->state->pv.cursor);
		}
		trackergrid_invalidatecursor(self);
	}
}

void trackergrid_prevlines(TrackerGrid* self, uintptr_t lines, bool wrap)
{
	assert(self);

	if (patternviewstate_pattern(&self->state->pv)) {
		psy_audio_PatternCursorNavigator cursornavigator;
		bool restorewrap;

		restorewrap = wrap;
		if (!self->state->pv.singlemode) {
			wrap = TRUE;
		}
		psy_audio_patterncursornavigator_init(&cursornavigator, &self->state->pv.cursor.cursor,
			patternviewstate_pattern(&self->state->pv), trackerstate_bpl(self->state), wrap, 0);
		if (!psy_audio_patterncursornavigator_prevlines(&cursornavigator, lines)) {
			trackergrid_scrollup(self, self->state->pv.cursor);
		} else if (!self->state->pv.singlemode) {
			psy_audio_OrderIndex index;			
			
			self->state->pv.cursor.cursor.offset = -trackerstate_bpl(self->state);
			index = trackergrid_checkupdatecursorseqoffset(self,
				&self->state->pv.cursor.cursor);
			if (psy_audio_orderindex_valid(&index)) {
				psy_audio_SequenceCursor cursor;

				cursor = self->state->pv.cursor;
				self->preventscrolltop = TRUE;
				cursor.orderindex = index;
				if (self->workspace && workspace_song(self->workspace)) {
					psy_audio_sequence_setcursor(
						psy_audio_song_sequence(workspace_song(self->workspace)),
						cursor);
				}
				self->preventscrolltop = FALSE;				
			} else if (restorewrap) {		
				psy_audio_SequenceCursor cursor;
				psy_audio_Sequence* sequence;

				sequence = patternviewstate_sequence(&self->state->pv);
				if (sequence) {
					self->preventscrolltop = TRUE;
					cursor = self->state->pv.cursor;
					cursor.orderindex = psy_audio_orderindex_make(
						self->state->pv.cursor.orderindex.track,
						psy_audio_sequence_track_size(
							sequence, 0) - 1);
					if (self->workspace && workspace_song(self->workspace)) {
						psy_audio_sequence_setcursor(
							psy_audio_song_sequence(workspace_song(self->workspace)),
							cursor);
					}
					self->preventscrolltop = FALSE;
					if (self->state->pv.pattern) {
						self->state->pv.cursor.cursor.offset =
							psy_audio_pattern_length(self->state->pv.pattern) -
							trackerstate_bpl(self->state);
					}
					trackergrid_scrolldown(self, self->state->pv.cursor);
					trackergrid_storecursor(self);
				}
				return;
			} else {
				self->state->pv.cursor.cursor.offset =
					psy_audio_pattern_length(self->state->pv.pattern) -
					trackerstate_bpl(self->state);
			}			
			trackergrid_scrollup(self, self->state->pv.cursor);
		} else {
			trackergrid_scrolldown(self, self->state->pv.cursor);
		}
		if (self->workspace && workspace_song(self->workspace)) {
			psy_audio_sequence_setcursor(
				psy_audio_song_sequence(workspace_song(self->workspace)),
				self->state->pv.cursor);
		}
		trackergrid_invalidatecursor(self);
	}
}

void trackergrid_home(TrackerGrid* self)
{
	assert(self);

	if (self->ft2home) {
		self->state->pv.cursor.cursor.offset = 0.0;
		trackergrid_scrollup(self, self->state->pv.cursor);
	} else {
		if (self->state->pv.cursor.cursor.column != 0) {
			self->state->pv.cursor.cursor.column = 0;
		} else {
			self->state->pv.cursor.cursor.track = 0;
			self->state->pv.cursor.cursor.column = 0;
		}
		trackergrid_scrollleft(self, self->state->pv.cursor);
	}
	if (self->workspace && workspace_song(self->workspace)) {
		psy_audio_sequence_setcursor(
			psy_audio_song_sequence(workspace_song(self->workspace)),
			self->state->pv.cursor);
	}
	trackergrid_invalidatecursor(self);
}

void trackergrid_end(TrackerGrid* self)
{
	assert(self);

	if (self->ft2home) {
		self->state->pv.cursor.cursor.offset = patternviewstate_pattern(&self->state->pv)->length - trackerstate_bpl(self->state);
		trackergrid_scrolldown(self, self->state->pv.cursor);
	} else {
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackerstate_trackdef(self->state, self->state->pv.cursor.cursor.track);
		columndef = trackdef_columndef(trackdef, self->state->pv.cursor.cursor.column);
		if (self->state->pv.cursor.cursor.track != patternviewstate_numsongtracks(&self->state->pv) - 1 ||
				self->state->pv.cursor.cursor.digit != columndef->numdigits - 1 ||
				self->state->pv.cursor.cursor.column != TRACKER_COLUMN_PARAM) {
			if (self->state->pv.cursor.cursor.column == TRACKER_COLUMN_PARAM &&
				self->state->pv.cursor.cursor.digit == columndef->numdigits - 1) {
				self->state->pv.cursor.cursor.track = patternviewstate_numsongtracks(&self->state->pv) - 1;
				trackdef = trackerstate_trackdef(self->state, self->state->pv.cursor.cursor.track);
				columndef = trackdef_columndef(trackdef, TRACKER_COLUMN_PARAM);
				self->state->pv.cursor.cursor.column = TRACKER_COLUMN_PARAM;
				self->state->pv.cursor.cursor.digit = columndef->numdigits - 1;
				trackergrid_scrollright(self, self->state->pv.cursor);
			} else {
				trackdef = trackerstate_trackdef(self->state, self->state->pv.cursor.cursor.track);
				columndef = trackdef_columndef(trackdef, TRACKER_COLUMN_PARAM);
				self->state->pv.cursor.cursor.column = TRACKER_COLUMN_PARAM;
				self->state->pv.cursor.cursor.digit = columndef->numdigits - 1;
			}			
		}
	}
	if (self->workspace && workspace_song(self->workspace)) {
		psy_audio_sequence_setcursor(
			psy_audio_song_sequence(workspace_song(self->workspace)),
			self->state->pv.cursor);
	}
	trackergrid_invalidatecursor(self);
}

void trackergrid_prevcol(TrackerGrid* self)
{
	bool invalidate;

	invalidate = TRUE;
	switch (trackerstate_prevcol(self->state, self->wraparound)) {
	case SCROLL_DIR_LEFT:
		invalidate = trackergrid_scrollleft(self, self->state->pv.cursor);
		break;
	case SCROLL_DIR_RIGHT:
		invalidate = trackergrid_scrollright(self, self->state->pv.cursor);
		break;
	default:;
	}
	if (self->workspace && workspace_song(self->workspace)) {
		psy_audio_sequence_setcursor(
			psy_audio_song_sequence(workspace_song(self->workspace)),
			self->state->pv.cursor);
	}
	if (invalidate) {
		trackergrid_invalidatecursor(self);
	}
}

void trackergrid_nextcol(TrackerGrid* self)
{
	bool invalidate;

	invalidate = TRUE;
	switch (trackerstate_nextcol(self->state, self->wraparound)) {
	case SCROLL_DIR_LEFT:
		invalidate = trackergrid_scrollleft(self, self->state->pv.cursor);
		break;
	case SCROLL_DIR_RIGHT:
		invalidate = trackergrid_scrollright(self, self->state->pv.cursor);
		break;
	default:;
	}
	if (self->workspace && workspace_song(self->workspace)) {
		psy_audio_sequence_setcursor(
			psy_audio_song_sequence(workspace_song(self->workspace)),
			self->state->pv.cursor);
	}
	if (invalidate) {
		trackergrid_invalidatecursor(self);
	}
}

void trackergrid_selectall(TrackerGrid* self)
{
	assert(self);

	trackerstate_selectall(self->state);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_selectcol(TrackerGrid* self)
{
	assert(self);

	trackerstate_selectcol(self->state);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_selectbar(TrackerGrid* self)
{
	assert(self);

	trackerstate_selectbar(self->state);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_selectmachine(TrackerGrid* self)
{
	assert(self);

	if (workspace_song(self->workspace)) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternEntry* entry;
		psy_audio_PatternNode* node;
		
		node = psy_audio_pattern_findnode_cursor(
			patternviewstate_pattern(&self->state->pv),
			self->state->pv.cursor.cursor, &prev);
		if (node) {
			psy_audio_PatternEvent* ev;

			entry = (psy_audio_PatternEntry*)node->entry;
			ev = psy_audio_patternentry_front(entry);
			psy_audio_machines_select(&workspace_song(self->workspace)->machines,
				ev->mach);
			psy_audio_instruments_select(&workspace_song(self->workspace)->instruments,
				psy_audio_instrumentindex_make(0, ev->inst));			
		}		
	}
}

void trackergrid_setdefaultevent(TrackerGrid* self,
	psy_audio_Pattern* defaults, psy_audio_PatternEvent* ev)
{
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;

	assert(self);

	node = psy_audio_pattern_findnode_cursor(defaults, self->state->pv.cursor.cursor,
		&prev);
	if (node) {
		psy_audio_PatternEvent* defaultevent;

		defaultevent = psy_audio_patternentry_front(psy_audio_patternnode_entry(node));
		if (defaultevent->inst != psy_audio_NOTECOMMANDS_INST_EMPTY) {
			ev->inst = defaultevent->inst;
		}
		if (defaultevent->mach != psy_audio_NOTECOMMANDS_psy_audio_EMPTY) {
			ev->mach = defaultevent->mach;
		}
		if (defaultevent->vol != psy_audio_NOTECOMMANDS_VOL_EMPTY) {
			ev->vol = defaultevent->vol;
		}
	}
}

void trackergrid_rowdelete(TrackerGrid* self)
{
	assert(self);

	if (self->state->pv.cursor.cursor.offset - trackerstate_bpl(self->state) >= 0) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternNode* p;
		psy_audio_PatternNode* q;
		psy_audio_PatternNode* node;

		if (self->ft2delete) {
			trackergrid_prevline(self);
		}
		node = psy_audio_pattern_findnode_cursor(
			patternviewstate_pattern(&self->state->pv),
			self->state->pv.cursor.cursor, &prev);
		if (node) {
			psy_audio_pattern_remove(patternviewstate_pattern(&self->state->pv), node);
			psy_audio_sequencer_checkiterators(
				&workspace_player(self->workspace)->sequencer,
				node);
		}
		p = (prev)
			? prev->next
			: psy_audio_pattern_begin(patternviewstate_pattern(&self->state->pv));
		for (; p != NULL; p = q) {
			psy_audio_PatternEntry* entry;

			q = p->next;
			entry = psy_audio_patternnode_entry(p);
			if (entry->track == self->state->pv.cursor.cursor.track) {
				psy_audio_PatternEvent event;
				psy_dsp_big_beat_t offset;
				uintptr_t track;
				psy_audio_PatternNode* node;
				psy_audio_PatternNode* prev;

				event = *psy_audio_patternentry_front(entry);
				offset = entry->offset;
				track = entry->track;
				psy_audio_pattern_remove(patternviewstate_pattern(&self->state->pv), p);
				psy_audio_sequencer_checkiterators(
					&workspace_player(self->workspace)->sequencer, p);
				offset -= (psy_dsp_big_beat_t)trackerstate_bpl(self->state);
				node = psy_audio_pattern_findnode(
					patternviewstate_pattern(&self->state->pv), track, offset,
					(psy_dsp_big_beat_t)trackerstate_bpl(self->state),
					&prev);
				if (node) {
					psy_audio_PatternEntry* entry;

					entry = (psy_audio_PatternEntry*)node->entry;
					*psy_audio_patternentry_front(entry) = event;
				} else {
					psy_audio_pattern_insert(
						patternviewstate_pattern(&self->state->pv), prev, track,
						offset, &event);
				}
			}
		}
	}
}

void trackergrid_rowclear(TrackerGrid* self)
{
	assert(self);

	if (self->state->pv.cursor.cursor.column == TRACKER_COLUMN_NOTE) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&removecommand_alloc(patternviewstate_pattern(&self->state->pv),
				trackerstate_bpl(self->state),
			self->state->pv.cursor,
			(self->state->synccursor)
				? self->workspace
				: NULL)->command);
		trackergrid_advanceline(self);		
	} else {
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackerstate_trackdef(self->state, self->state->pv.cursor.cursor.track);
		columndef = trackdef_columndef(trackdef, self->state->pv.cursor.cursor.column);
		trackergrid_inputvalue(self, columndef->emptyvalue, 0);
	}
}

bool trackergrid_onnotecmds(TrackerGrid* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	assert(self);

	cmd = inputhandler_cmd(sender);
	if (cmd.id != -1) {		
		psy_audio_PatternEvent ev;
		bool chord;
				
		chord = FALSE;
		trackergrid_preventpatternsync(self);
		if (cmd.id >= CMD_NOTE_OFF_C_0 && cmd.id < 255) {
			ev = psy_audio_player_patternevent(&self->workspace->player, (uint8_t)cmd.id);
			ev.note = CMD_NOTE_STOP;
			psy_audio_player_playevent(&self->workspace->player, &ev);
			return 1;
		} else if (cmd.id == CMD_NOTE_CHORD_END) {
			self->state->pv.cursor.cursor.track = self->chordbegin;
			trackergrid_scrollleft(self, self->state->pv.cursor);
			trackergrid_advanceline(self);			
			return 1;
		} else if (cmd.id >= CMD_NOTE_CHORD_C_0 && cmd.id < CMD_NOTE_STOP) {
			chord = TRUE;
			ev = psy_audio_player_patternevent(&self->workspace->player,
				(uint8_t)cmd.id - (uint8_t)CMD_NOTE_CHORD_C_0);
		} else if (cmd.id < 256) {			
			ev = psy_audio_player_patternevent(&self->workspace->player, (uint8_t)cmd.id);
			psy_audio_player_playevent(&self->workspace->player, &ev);
		}
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_alloc(patternviewstate_pattern(&self->state->pv),
				trackerstate_bpl(self->state),
				self->state->pv.cursor, ev,
				(self->state->synccursor) ? self->workspace : NULL)->command);
		if (chord != FALSE) {
			trackergrid_nexttrack(self);
		} else {
			trackergrid_advanceline(self);
		}
		if (ev.note < psy_audio_NOTECOMMANDS_RELEASE) {
			self->state->pv.cursor.cursor.key = ev.note;
			if (self->workspace && workspace_song(self->workspace)) {
				psy_audio_sequence_setcursor(
					psy_audio_song_sequence(workspace_song(self->workspace)),
					self->state->pv.cursor);
			}
		}
		trackergrid_enablepatternsync(self);
		return 1;
	}
	return 0;
}

bool trackergrid_onmidicmds(TrackerGrid* self, InputHandler* sender)
{
	if (self->preventeventdriver) {
		return 0;
	}
	if (!workspace_song(self->workspace)) {
		return 0;
	}
	if (!psy_audio_player_playing(&self->workspace->player) && 
			psy_audio_player_recordingnotes(&self->workspace->player)) {
		psy_EventDriverCmd cmd;				

		assert(self);

		cmd = inputhandler_cmd(sender);
		if (cmd.type == psy_EVENTDRIVER_MIDI) {
			psy_audio_PatternEvent ev;
			
			psy_audio_patternevent_clear(&ev);
			psy_audio_midiinput_workinput(&self->workspace->player.midiinput, cmd.midi,
				&self->workspace->song->machines, &ev);
			if (ev.note != psy_audio_NOTECOMMANDS_RELEASE ||
					psy_audio_player_recordingnoteoff(workspace_player(
						self->workspace))) {
				trackergrid_preventpatternsync(self);
				psy_undoredo_execute(&self->workspace->undoredo,
					&insertcommand_alloc(patternviewstate_pattern(&self->state->pv),
						trackerstate_bpl(self->state),
						self->state->pv.cursor, ev,
						(self->state->synccursor) ? self->workspace : NULL)->command);
				trackergrid_advanceline(self);
				if (ev.note < psy_audio_NOTECOMMANDS_RELEASE) {
					self->state->pv.cursor.cursor.key = ev.note;
					if (self->workspace && workspace_song(self->workspace)) {
						psy_audio_sequence_setcursor(
							psy_audio_song_sequence(workspace_song(self->workspace)),
							self->state->pv.cursor);
					}
				}
				trackergrid_enablepatternsync(self);
			}
			return 1;			
		}
	}
	return 0;
}

void trackergrid_inputvalue(TrackerGrid* self, intptr_t value, intptr_t digit)
{
	assert(self);

	if (patternviewstate_pattern(&self->state->pv) && value != -1) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternEntry* entry;
		psy_audio_PatternNode* node;
		psy_audio_PatternEntry newentry;
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackerstate_trackdef(self->state, self->state->pv.cursor.cursor.track);
		columndef = trackdef_columndef(trackdef, self->state->pv.cursor.cursor.column);
		psy_audio_patternentry_init(&newentry);
		node = psy_audio_pattern_findnode_cursor(
			patternviewstate_pattern(&self->state->pv),
			self->state->pv.cursor.cursor, &prev);
		if (node) {
			entry = (psy_audio_PatternEntry*)node->entry;
		} else {
			entry = &newentry;
		}
		if (digit) {
			trackergrid_enterdigitcolumn(self, entry, self->state->pv.cursor.cursor,
				value);
		} else {
			entervaluecolumn(entry, self->state->pv.cursor.cursor.column, value);
		}
		trackergrid_preventpatternsync(self);
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_alloc(patternviewstate_pattern(&self->state->pv),
				trackerstate_bpl(self->state),
				self->state->pv.cursor,
				*psy_audio_patternentry_front(entry),
				self->workspace)->command);
		if (self->effcursoralwaysdown) {
			trackergrid_advanceline(self);
		} else {
			if (!digit) {
				if (columndef->wrapclearcolumn == TRACKER_COLUMN_NONE) {
					trackergrid_nextcol(self);
				} else {
					self->state->pv.cursor.cursor.digit = 0;
					self->state->pv.cursor.cursor.column = columndef->wrapclearcolumn;
					trackergrid_advanceline(self);
				}
			} else if (self->state->pv.cursor.cursor.digit + 1 >= columndef->numdigits) {
				if (columndef->wrapeditcolumn == TRACKER_COLUMN_NONE) {
					trackergrid_nextcol(self);
				} else {
					self->state->pv.cursor.cursor.digit = 0;
					self->state->pv.cursor.cursor.column = columndef->wrapeditcolumn;
					trackergrid_advanceline(self);
				}
			} else {
				trackergrid_nextcol(self);
			}
		}
		trackergrid_invalidatecursor(self);
		trackergrid_enablepatternsync(self);
		psy_audio_patternentry_dispose(&newentry);
	}
}

void trackergrid_invalidatecursor(TrackerGrid* self)
{	
	assert(self);

	trackergrid_invalidateinternalcursor(self, self->oldcursor);
	trackergrid_invalidateinternalcursor(self, self->state->pv.cursor);
	trackergrid_storecursor(self);	
}

void trackergrid_invalidateinternalcursor(TrackerGrid* self,
	psy_audio_SequenceCursor cursor)
{
	psy_ui_Component* column;	

	column = psy_ui_component_at(trackergrid_base(self), cursor.cursor.track);
	if (column) {
		psy_ui_RealSize size;

		size = psy_ui_component_scrollsize_px(column);
		psy_ui_component_invalidaterect(column,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					0.0,
					trackerstate_beattopx(self->state,
						psy_audio_sequencecursor_offset(&cursor))),
				psy_ui_realsize_make(
					size.width,
					trackerstate_lineheight(self->state))));
	}
}

void trackergrid_invalidateline(TrackerGrid* self, psy_dsp_big_beat_t position)
{
	assert(self);

	if (!patternviewstate_pattern(&self->state->pv)) {
		return;
	}	
	if (!self->state->pv.singlemode ||
			psy_dsp_testrange(position, self->state->pv.cursor.seqoffset,
			psy_audio_pattern_length(patternviewstate_pattern(
				&self->state->pv)))) {
		psy_ui_RealSize size;		
		
		size = psy_ui_component_scrollsize_px(&self->component);		
		psy_ui_component_invalidaterect(&self->component,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(					
					psy_ui_component_scrollleftpx(&self->component),
					trackerstate_beattopx(self->state,
						position -
						((self->state->pv.singlemode)
						? self->state->pv.cursor.seqoffset
						: 0.0))),
				psy_ui_realsize_make(size.width, self->state->lineheightpx)));
		if (patternviewstate_pattern(&self->state->pv)) {
			trackergrid_preventpatternsync(self);
			patternviewstate_pattern(&self->state->pv)->opcount++;
			trackergrid_resetpatternsync(self);
		}
	}
}

void trackergrid_onscroll(TrackerGrid* self, psy_ui_Component* sender)
{
	assert(self);

	if (self->state->midline) {
		trackergrid_clearmidline(self);
	}
}

void trackergrid_clearmidline(TrackerGrid* self)
{
	psy_ui_RealSize size;

	assert(self);

	size = psy_ui_component_scrollsize_px(&self->component);	
	self->state->midline = FALSE;
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scrollleftpx(&self->component),
				((self->state->visilines) / 2 - 1) * self->state->lineheightpx +
					psy_ui_component_scrolltop_px(&self->component)),
			psy_ui_realsize_make(size.width, self->state->lineheightpx)));
	psy_ui_component_update(&self->component);
	self->state->midline = TRUE;
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scrollleftpx(&self->component),
				(self->state->visilines / 2 - 2) * self->state->lineheightpx +
				psy_ui_component_scrolltop_px(&self->component)),
			psy_ui_realsize_make(size.width, self->state->lineheightpx * 4)));		
}

void trackergrid_centeroncursor(TrackerGrid* self)
{
	intptr_t line;

	assert(self);

	line = trackerstate_beattoline(self->state,
		self->state->pv.cursor.cursor.offset);
	psy_ui_component_setscrolltop_px(&self->component,		
		-(self->state->visilines / 2 - line) *
			self->state->lineheightpx);
}

void trackergrid_setcentermode(TrackerGrid* self, int mode)
{
	assert(self);

	self->state->midline = mode;
	if (mode) {
		psy_ui_component_setoverflow(&self->component,
			(psy_ui_Overflow)(psy_ui_OVERFLOW_SCROLL | psy_ui_OVERFLOW_VSCROLLCENTER));
		trackergrid_centeroncursor(self);
	} else {
		psy_ui_component_setoverflow(&self->component,
			psy_ui_OVERFLOW_SCROLL);
		psy_ui_component_setscrolltop_px(&self->component, 0.0);			
	}
}

void trackergrid_onmousedown(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (self->state->trackconfig->colresize) {
		psy_signal_emit(&self->signal_colresize, self, 0);
	} else if (patternviewstate_pattern(&self->state->pv) && ev->button == 1) {
		if (!self->state->pv.singlemode) {
			psy_audio_OrderIndex index;

			index = trackergrid_checkupdatecursorseqoffset(self,
				&self->state->pv.dragselectionbase.cursor);
			if (psy_audio_orderindex_valid(&index)) {
				psy_audio_SequenceCursor cursor;

				self->preventscrolltop = TRUE;
				cursor = self->state->pv.cursor;
				cursor.orderindex = index;
				if (self->workspace && workspace_song(self->workspace)) {
					psy_audio_sequence_setcursor(
						psy_audio_song_sequence(workspace_song(self->workspace)),
						cursor);
				}
				self->preventscrolltop = FALSE;
			}
		}
		self->lastdragcursor = self->state->pv.dragselectionbase;
		psy_audio_blockselection_init_all(&self->state->pv.selection,
			self->state->pv.dragselectionbase.cursor, self->state->pv.dragselectionbase.cursor);
		psy_audio_blockselection_disable(&self->state->pv.selection);
		if (!psy_ui_component_hasfocus(&self->component)) {
			psy_ui_component_setfocus(&self->component);
		}
		psy_ui_component_capture(&self->component);		
	}
}

void trackergrid_onmousemove(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (ev->button == 1) {
		if (self->state->trackconfig->colresize) {
			psy_signal_emit(&self->signal_colresize, self, 0);
			psy_ui_mouseevent_stop_propagation(ev);
		} else {					
			if (!psy_audio_patterncursor_equal(&self->state->pv.dragcursor.cursor,
					&self->lastdragcursor.cursor)) {
				if (!psy_audio_blockselection_valid(&self->state->pv.selection)) {
					trackerstate_startdragselection(self->state,
						self->state->pv.dragcursor,
						trackerstate_bpl(self->state));
				} else {					
					trackergrid_dragselection(self, self->state->pv.dragcursor);
				}
				psy_ui_component_invalidate(&self->component);
				self->lastdragcursor = self->state->pv.dragcursor;
			}			
		}
	}
}

void trackergrid_dragselection(TrackerGrid* self, psy_audio_SequenceCursor cursor)
{
	int restoremidline = self->state->midline;
	
	self->state->midline = 0;
	trackerstate_dragselection(self->state, cursor,
		trackerstate_bpl(self->state));
	if (cursor.cursor.offset < self->lastdragcursor.cursor.offset) {
		trackergrid_scrollup(self, cursor);
	} else {
		trackergrid_scrolldown(self, cursor);
	}
	if (cursor.cursor.track < self->lastdragcursor.cursor.track) {
		trackergrid_scrollleft(self, cursor);
	} else {
		trackergrid_scrollright(self, cursor);
	}
	self->state->midline = restoremidline;
}

void trackergrid_onmouseup(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_releasecapture(&self->component);
	if (ev->button != 1) {
		return;
	}	
	if (self->state->trackconfig->colresize) {
		self->state->trackconfig->colresize = FALSE;
		psy_signal_emit(&self->signal_colresize, self, 0);		
	} else if (!psy_audio_blockselection_valid(&self->state->pv.selection)) {
		/* set cursor only, if no selection was made */
		if (!self->state->pv.singlemode) {
			psy_audio_OrderIndex index;
			
			index = trackergrid_checkupdatecursorseqoffset(self,
				&self->state->pv.dragselectionbase.cursor);
			if (psy_audio_orderindex_valid(&index)) {
				psy_audio_SequenceCursor cursor;
				
				self->preventscrolltop = TRUE;
				cursor = self->state->pv.cursor;
				cursor.orderindex = index;
				if (self->workspace && workspace_song(self->workspace)) {
					psy_audio_sequence_setcursor(
						psy_audio_song_sequence(workspace_song(self->workspace)),
						cursor);
				}
				self->preventscrolltop = FALSE;
			}			
		}
		self->state->pv.cursor = self->state->pv.dragselectionbase;
		if (!psy_audio_patterncursor_equal(&self->oldcursor.cursor, &self->state->pv.cursor.cursor)) {
			trackergrid_invalidatecursor(self);
		}
		if (self->workspace && workspace_song(self->workspace)) {
			psy_audio_sequence_setcursor(
				psy_audio_song_sequence(workspace_song(self->workspace)),
				self->state->pv.cursor);
		}
	}	
}

void trackergrid_onmousedoubleclick(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (ev->button == 1) {
		trackergrid_selectcol(self);
	}
}

void trackergrid_setpattern(TrackerGrid* self, psy_audio_Pattern* pattern)
{
	assert(self);
		
	patterncmds_setpattern(&self->cmds, pattern);
	patternviewstate_setpattern(&self->state->pv, pattern);	
	trackergrid_resetpatternsync(self);
	psy_ui_component_updateoverflow(trackergrid_base(self));	
	if (psy_audio_player_playing(workspace_player(self->workspace)) ||
			!trackerstate_cursorposition_valid(self->state)) {		
		self->state->pv.cursor.cursor.absolute = !self->state->pv.singlemode;
		if (!self->state->pv.singlemode) {
			if (!workspace_followingsong(self->workspace)) {
				psy_ui_component_setscrolltop_px(&self->component,
					trackerstate_beattopx(self->state,
						psy_audio_sequencecursor_offset_abs(&self->state->pv.cursor)));
			} else {
				self->state->pv.cursor.cursor.offset = 0;
			}
		} else {
			self->state->pv.cursor.cursor.offset = 0;
			psy_ui_component_setscrolltop_px(&self->component, 0.0);
		}		
	} else if (!self->state->pv.singlemode) {
		self->state->pv.cursor.cursor.absolute = !self->state->pv.singlemode;
		if (!self->preventscrolltop) {
			psy_ui_component_setscrolltop_px(&self->component,
				trackerstate_beattopx(self->state,
					self->state->pv.cursor.cursor.offset +
					psy_audio_sequencecursor_offset_abs(&self->state->pv.cursor)));
		}
	}
	if (self->state->midline) {
		trackergrid_centeroncursor(self);
	}
	psy_ui_component_align(&self->component);
}

void trackergrid_enablepatternsync(TrackerGrid* self)
{
	assert(self);
	
	trackergrid_resetpatternsync(self);
	self->syncpattern = TRUE;
}

void trackergrid_preventpatternsync(TrackerGrid* self)
{
	assert(self);

	trackergrid_resetpatternsync(self);
	self->syncpattern = FALSE;
}

void trackergrid_resetpatternsync(TrackerGrid* self)
{
	assert(self);

	if (patternviewstate_pattern(&self->state->pv)) {
		self->component.opcount = patternviewstate_pattern(
			&self->state->pv)->opcount;
	} else {
		self->component.opcount = 0;
	}
}

void trackergrid_changegenerator(TrackerGrid* self)
{
	assert(self);

	patterncmds_changemachine(&self->cmds, self->state->pv.selection);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_changeinstrument(TrackerGrid* self)
{
	assert(self);
	
	patterncmds_changeinstrument(&self->cmds, self->state->pv.selection);
	psy_ui_component_invalidate(&self->component);	
}

void trackergrid_blockcut(TrackerGrid* self)
{
	assert(self);

	trackergrid_blockcopy(self);
	trackergrid_blockdelete(self);	
}

void trackergrid_blockcopy(TrackerGrid* self)
{
	assert(self);

	patterncmds_blockcopy(&self->cmds, self->state->pv.selection);
}

void trackergrid_blockpaste(TrackerGrid* self)
{
	assert(self);

	patterncmds_blockpaste(&self->cmds,	self->state->pv.cursor, FALSE);
	trackergrid_movecursorwhenpaste(self);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blockmixpaste(TrackerGrid* self)
{
	assert(self);

	patterncmds_blockpaste(&self->cmds,	self->state->pv.cursor, TRUE);
	trackergrid_movecursorwhenpaste(self);
	psy_ui_component_invalidate(&self->component);	
}

bool trackergrid_movecursorwhenpaste(TrackerGrid* self)
{
	assert(self);

	if (patternviewconfig_ismovecursorwhenpaste(psycleconfig_patview(
			workspace_conf(self->workspace)))) {
		psy_audio_PatternCursor begin;
		psy_audio_PatternCursor end;

		begin = end = self->state->pv.cursor.cursor;
		end.track += self->workspace->patternpaste.maxsongtracks;
		end.offset += self->workspace->patternpaste.length;
		end.track = begin.track;
		if (end.offset >= psy_audio_pattern_length(patternviewstate_pattern(&self->state->pv))) {
			end.offset = psy_audio_pattern_length(patternviewstate_pattern(&self->state->pv)) -
				trackerstate_bpl(self->state);
		}		
		self->state->pv.cursor.cursor = end;
		if (self->workspace && workspace_song(self->workspace)) {
			psy_audio_sequence_setcursor(
				psy_audio_song_sequence(workspace_song(self->workspace)),
				self->state->pv.cursor);
		}
		self->oldcursor.cursor = end;
		return TRUE;
	}
	return FALSE;
}

void trackergrid_blockdelete(TrackerGrid* self)
{
	assert(self);
	
	patterncmds_blockdelete(&self->cmds, self->state->pv.selection);
	psy_ui_component_invalidate(&self->component);	
}

void trackergrid_blockstart(TrackerGrid* self)
{
	assert(self);

	self->state->pv.dragselectionbase = self->state->pv.cursor;
	trackerstate_startdragselection(self->state,
		self->state->pv.cursor,
		trackerstate_bpl(self->state));
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blockend(TrackerGrid* self)
{
	assert(self);

	trackergrid_dragselection(self, self->state->pv.cursor);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blockunmark(TrackerGrid* self)
{
	assert(self);

	psy_audio_blockselection_disable(&self->state->pv.selection);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blocktransposeup(TrackerGrid* self)
{
	assert(self);
	
	patterncmds_blocktranspose(&self->cmds, self->state->pv.selection,
		self->state->pv.cursor, +1);
}

void trackergrid_blocktransposedown(TrackerGrid* self)
{
	assert(self);

	patterncmds_blocktranspose(&self->cmds, self->state->pv.selection,
		self->state->pv.cursor, -1);
}

void trackergrid_blocktransposeup12(TrackerGrid* self)
{
	assert(self);

	patterncmds_blocktranspose(&self->cmds,	self->state->pv.selection,
		self->state->pv.cursor, 12);
}

void trackergrid_blocktransposedown12(TrackerGrid* self)
{
	assert(self);

	patterncmds_blocktranspose(&self->cmds,	self->state->pv.selection,
		self->state->pv.cursor, -12);
}

void trackergrid_enterdigitcolumn(TrackerGrid* self,
	psy_audio_PatternEntry* entry, psy_audio_PatternCursor cursor,
	intptr_t digitvalue)
{
	TrackDef* trackdef;

	assert(self);

	trackdef = trackerstate_trackdef(self->state, cursor.track);
	if (trackdef) {
		uintptr_t value;
		uintptr_t num;
		ptrdiff_t pos;
		uint8_t* data;

		value = trackdef_value(trackdef, cursor.column, entry);
		if (cursor.digit != 0xF && value == trackdef_emptyvalue(trackdef,
				cursor.column)) {
			value = 0;
		}
		num = trackdef_numdigits(trackdef, cursor.column);
		pos = num / 2 - cursor.digit / 2 - 1;
		data = (uint8_t*)&value + pos;
		enterdigit(cursor.digit % 2, (uint8_t)digitvalue, data);
		trackdef_setvalue(trackdef, cursor.column, entry, *((uintptr_t*)&value));
	}
}

bool trackergrid_ontrackercmds(TrackerGrid* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	cmd = inputhandler_cmd(sender);	
	return trackergrid_handlecommand(self, cmd.id);
}

bool trackergrid_handlecommand(TrackerGrid* self, intptr_t cmd)
{	
	assert(self);

	switch (cmd) {
	case CMD_NAVUP:
		if (self->movecursoronestep) {
			trackergrid_prevlines(self, 1, 0);
		} else {
			trackergrid_prevline(self);
		}
		return TRUE;
	case CMD_NAVPAGEUP:
		trackergrid_prevlines(self, self->pgupdownstep, 0);
		return TRUE;
	case CMD_NAVDOWN:
		if (self->movecursoronestep) {
			trackergrid_advancelines(self, 1, 0);
		} else {
			trackergrid_advanceline(self);
		}
		return TRUE;
	case CMD_NAVPAGEDOWN:
		trackergrid_advancelines(self, self->pgupdownstep, 0);
		return TRUE;
	case CMD_NAVLEFT:
		trackergrid_prevcol(self);
		return TRUE;
	case CMD_NAVRIGHT:
		trackergrid_nextcol(self);
		return TRUE;
	case CMD_NAVTOP:
		trackergrid_home(self);
		return TRUE;
	case CMD_NAVBOTTOM:
		trackergrid_end(self);
		return TRUE;
	case CMD_COLUMNPREV:
		trackergrid_prevtrack(self);
		return TRUE;
	case CMD_COLUMNNEXT:
		trackergrid_nexttrack(self);
		return TRUE;
	case CMD_BLOCKSTART:
		trackergrid_blockstart(self);
		return TRUE;
	case CMD_BLOCKEND:
		trackergrid_blockend(self);
		return TRUE;
	case CMD_BLOCKUNMARK:
		trackergrid_blockunmark(self);
		return TRUE;
	case CMD_BLOCKCUT:
		trackergrid_blockcut(self);
		return TRUE;
	case CMD_BLOCKCOPY:
		trackergrid_blockcopy(self);
		return TRUE;
	case CMD_BLOCKPASTE:
		trackergrid_blockpaste(self);
		return TRUE;
	case CMD_BLOCKMIX:
		trackergrid_blockmixpaste(self);
		return TRUE;
	case CMD_BLOCKDELETE:
		trackergrid_blockdelete(self);
		return TRUE;
	case CMD_TRANSPOSEBLOCKINC:
		trackergrid_blocktransposeup(self);
		return TRUE;
	case CMD_TRANSPOSEBLOCKDEC:
		trackergrid_blocktransposedown(self);
		return TRUE;
	case CMD_TRANSPOSEBLOCKINC12:
		trackergrid_blocktransposeup12(self);
		return TRUE;
	case CMD_TRANSPOSEBLOCKDEC12:
		trackergrid_blocktransposedown12(self);
		return TRUE;
	case CMD_ROWDELETE:
		trackergrid_rowdelete(self);			
		return TRUE;
	case CMD_ROWCLEAR:
		trackergrid_rowclear(self);			
		return TRUE;
	case CMD_SELECTALL:
		trackergrid_selectall(self);
		return TRUE;
	case CMD_SELECTCOL:
		trackergrid_selectcol(self);
		return TRUE;
	case CMD_SELECTBAR:
		trackergrid_selectbar(self);
		return TRUE;
	case CMD_SELECTMACHINE:
		trackergrid_selectmachine(self);
		psy_ui_component_setfocus(&self->component);
		return TRUE;
	case CMD_UNDO:
		workspace_undo(self->workspace);
		return TRUE;
	case CMD_REDO:
		workspace_redo(self->workspace);
		return TRUE;
	case CMD_DIGIT0:
	case CMD_DIGIT1:
	case CMD_DIGIT2:
	case CMD_DIGIT3:
	case CMD_DIGIT4:
	case CMD_DIGIT5:
	case CMD_DIGIT6:
	case CMD_DIGIT7:
	case CMD_DIGIT8:
	case CMD_DIGIT9:
	case CMD_DIGITA:
	case CMD_DIGITB:
	case CMD_DIGITC:
	case CMD_DIGITD:
	case CMD_DIGITE:
	case CMD_DIGITF:
		if (self->state->pv.cursor.cursor.column != TRACKER_COLUMN_NOTE) {
			int digit = (int)cmd - CMD_DIGIT0;
			if (digit != -1) {
				trackergrid_inputvalue(self, digit, 1);
			}
			return TRUE;
		}
		break;
	default:
		break;
	}
	return FALSE;
}

void trackergrid_tweak(TrackerGrid* self, int slot, uintptr_t tweak,
	float normvalue)
{
	assert(self);

	if (workspace_recordingtweaks(self->workspace)) {
		psy_audio_PatternEvent event;
		psy_audio_Machine* machine;
		int value;

		machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines, slot);
		assert(machine);
		value = 0; /* machine_parametervalue_scaled(machine, tweak, normvalue); */
		psy_audio_patternevent_init_all(&event,
			(unsigned char)(
				(keyboardmiscconfig_recordtweaksastws(&self->workspace->config.misc))
				? psy_audio_NOTECOMMANDS_TWEAKSLIDE
				: psy_audio_NOTECOMMANDS_TWEAK),
			psy_audio_NOTECOMMANDS_INST_EMPTY,
			(unsigned char)psy_audio_machines_selected(&workspace_song(self->workspace)->machines),
			psy_audio_NOTECOMMANDS_VOL_EMPTY,
			(unsigned char)((value & 0xFF00) >> 8),
			(unsigned char)(value & 0xFF));
		event.inst = (unsigned char)tweak;
		trackergrid_preventpatternsync(self);
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_alloc(patternviewstate_pattern(&self->state->pv), trackerstate_bpl(self->state),
				self->state->pv.cursor, event, self->workspace)->command);
		if (keyboardmiscconfig_advancelineonrecordtweak(&self->workspace->config.misc) &&
			!(workspace_followingsong(self->workspace) &&
				psy_audio_player_playing(workspace_player(self->workspace)))) {
			trackergrid_advanceline(self);
		}
		trackergrid_enablepatternsync(self);
	}
}

void trackergrid_onclientalign(TrackerGrid* self, psy_ui_Component* sender)
{	
	assert(self);
		
	printf("align\n");
	if (trackergrid_midline(self)) {		
		trackergrid_centeroncursor(self);				
	} 
}

void trackergrid_showemptydata(TrackerGrid* self, int showstate)
{
	assert(self);

	self->state->showemptydata = showstate;
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_ongotocursor(TrackerGrid* self, Workspace* sender,
	psy_audio_PatternCursor* cursor)
{			
	double y;
	psy_ui_RealSize clientsize;
	
	y = trackerstate_beattopx(self->state,
		cursor->offset -
		((self->state->pv.singlemode)
			? self->state->pv.cursor.seqoffset
			: 0.0));
	clientsize = psy_ui_component_clientsize_px(&self->component);
	if (y < psy_ui_component_scrolltop_px(&self->component) ||
		y > psy_ui_component_scrolltop_px(&self->component) + clientsize.height) {
		psy_ui_component_setscrolltop_px(&self->component,
			trackerstate_beattopx(self->state,
				psy_audio_sequencecursor_offset_abs(&self->state->pv.cursor)));				
	}
}

void trackergrid_build(TrackerGrid* self)
{
	uintptr_t t;
	
	psy_ui_component_clear(&self->component);
	psy_table_clear(&self->columns);
	for (t = 0; t < patternviewstate_numsongtracks(&self->state->pv); ++t) {
		psy_table_insert(&self->columns, t, (void*)	
			trackercolumn_allocinit(&self->component,
				(self->component.view) ? NULL : &self->component, t,
				self->state, self->workspace));
	}
	psy_ui_component_align(&self->component);
}

void trackergrid_onplaylinechanged(TrackerGrid* self, Workspace* sender)
{
	if (!workspace_followingsong(sender) &&
			psy_ui_component_drawvisible(&self->component)) {		
		trackergrid_invalidateline(self, sender->lastplayposition);		
		trackergrid_invalidateline(self, sender->currplayposition);		
	}	
}

/* TrackerView */

/* prototypes */
static void trackerviewview_onscrollpanealign(TrackerView* self,
	psy_ui_Component* sender);

/* implementation */
void trackerview_init(TrackerView* self, psy_ui_Component* parent,
	TrackConfig* trackconfig, TrackerState* state, Workspace* workspace)
{
	trackergrid_init(&self->grid, parent, NULL, trackconfig, state, workspace);
	psy_ui_component_setwheelscroll(&self->grid.component, 4);
	psy_ui_component_setoverflow(trackergrid_base(&self->grid),
		psy_ui_OVERFLOW_SCROLL);
	psy_ui_scroller_init(&self->scroller, &self->grid.component, parent, NULL);
	psy_signal_connect(&self->scroller.pane.signal_align, self,
		trackerviewview_onscrollpanealign);
	psy_ui_component_setbackgroundmode(&self->scroller.pane,
		psy_ui_NOBACKGROUND);
	psy_ui_component_setalign(&self->scroller.component,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->grid.component, psy_ui_ALIGN_FIXED);
}

void trackerviewview_onscrollpanealign(TrackerView* self,
	psy_ui_Component* sender)
{
	psy_ui_RealSize size;

	size = psy_ui_component_scrollsize_px(sender);
	self->grid.state->visilines = (intptr_t)(size.height /
		self->grid.state->lineheightpx);
}

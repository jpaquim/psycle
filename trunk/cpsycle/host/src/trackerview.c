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

static void entervaluecolumn(psy_audio_PatternEntry* entry, intptr_t column, intptr_t value)
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
	const psy_audio_PatternSelection* clip);
static void trackergrid_updatetrackevents(TrackerGrid*,
	const psy_audio_PatternSelection* clip);
static void trackergrid_onmousedown(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousemove(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmouseup(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousedoubleclick(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_dragselection(TrackerGrid*, psy_audio_PatternCursor);
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
	TrackConfig* trackconfig, TrackerGridState* gridstate, TrackerLineState* linestate,
	Workspace* workspace)
{
	assert(self);

	/* init base component */
	psy_ui_component_init(&self->component, parent, view);
	psy_ui_component_setvtable(&self->component, vtable_init(self));	
	/* set references */
	self->workspace = workspace;
	self->view = view;
	patterncmds_init(&self->cmds, workspace);
	psy_table_init(&self->columns);
	trackergridstate_init(&self->defaultgridstate, trackconfig, NULL, NULL);
	trackerlinestate_init(&self->defaultlinestate);
	trackergrid_setsharedgridstate(self, gridstate, trackconfig);
	trackergrid_setsharedlinestate(self, linestate);
	trackergrid_storecursor(self);
	/* setup base component */
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	psy_ui_component_doublebuffer(&self->component);	
	trackergrid_init_signals(self);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HEXPAND);
	/* init internal */
	self->syncpattern = TRUE;	
	self->gridstate->midline = FALSE;	
	self->chordmode = FALSE;
	self->chordbegin = 0;
	self->wraparound = TRUE;
	self->gridstate->showemptydata = FALSE;	
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
	psy_audio_patternselection_init(&self->gridstate->selection);
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
	trackergridstate_dispose(&self->defaultgridstate);
	trackerlinestate_dispose(&self->defaultlinestate);
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

void trackergrid_setsharedgridstate(TrackerGrid* self, TrackerGridState*
	gridstate, TrackConfig* trackconfig)
{
	assert(self);
	
	if (gridstate) {
		self->gridstate = gridstate;
	} else {		
		self->gridstate = &self->defaultgridstate;
	}
}

void trackergrid_setsharedlinestate(TrackerGrid* self, TrackerLineState*
	linestate)
{
	assert(self);

	if (linestate) {
		self->linestate = linestate;
	} else {		
		self->linestate = &self->defaultlinestate;
	}
}

void trackergrid_ondraw(TrackerGrid* self, psy_ui_Graphics* g)
{
	psy_audio_PatternSelection clip;
		
	trackerlinestate_clip(self->linestate, &g->clip, &clip);
	trackergridstate_clip(self->gridstate, &g->clip, &clip);
	trackergrid_drawbackground(self, g, &clip);
	/* prepares entry draw done in trackergridcolumn */
	if (trackergridstate_pattern(self->gridstate)) {		
		trackergrid_updatetrackevents(self, &clip);	
	}
}

void trackergrid_drawbackground(TrackerGrid* self, psy_ui_Graphics* g,
	const psy_audio_PatternSelection* clip)
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
			patternviewskin_separatorcolour(self->gridstate->skin, track,
				trackergridstate_numsongtracks(self->gridstate)));
	}
	psy_list_free(q);
}


void trackergrid_updatetrackevents(TrackerGrid* self,
	const psy_audio_PatternSelection* clip)
{
	uintptr_t track;
	uintptr_t maxlines;	
	double offset;
	double seqoffset;
	double length;	
	psy_audio_SequenceTrackIterator ite;	
	uintptr_t line;	

	assert(self);
	
	trackereventtable_clearevents(&self->gridstate->trackevents);
	ite.pattern = self->gridstate->pattern;
	ite.patternnode = NULL;
	ite.patterns = &self->workspace->song->patterns;
	seqoffset = 0.0;
	length = ite.pattern->length;
	offset = clip->topleft.offset;
	if (!self->linestate->singlemode && self->gridstate->sequence) {
		psy_audio_SequenceTrackNode* tracknode;

		tracknode = psy_list_at(self->gridstate->sequence->tracks,
			workspace_sequenceeditposition(self->workspace).track);
		if (!tracknode) {
			tracknode = self->gridstate->sequence->tracks;
		}
	 	ite = psy_audio_sequence_begin(self->gridstate->sequence,			
			tracknode, offset);
	 	if (ite.sequencentrynode) {
			seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);	 		
	 		if (ite.pattern) {
	 			length = ite.pattern->length;
	 		}		
	 	}
	} else {
		ite.sequencentrynode = NULL;
		ite.patternnode = psy_audio_pattern_greaterequal(
			trackergridstate_pattern(self->gridstate),
			(psy_dsp_big_beat_t)offset - seqoffset);
	}		
	line = trackerlinestate_beattoline(self->linestate, offset);	
	maxlines = trackerlinestate_numlines(self->linestate);
	while (offset <= clip->bottomright.offset && line < maxlines) {		
		bool fill;
		
		fill = !(offset >= seqoffset && offset < seqoffset + length) || !ite.patternnode;		
		/* draw trackline */
		for (track = clip->topleft.track; track < clip->bottomright.track;
			++track) {
			bool hasevent = FALSE;
			
			while (!fill && ite.patternnode &&
					psy_audio_sequencetrackiterator_patternentry(&ite)->track <= track &&
					psy_dsp_testrange_e(
						psy_audio_sequencetrackiterator_offset(&ite),
						offset,
						trackerlinestate_bpl(self->linestate))) {
				psy_audio_PatternEntry* entry;

				entry = psy_audio_sequencetrackiterator_patternentry(&ite);
				if (entry->track == track) {
					psy_List** trackevents;

					trackevents = trackereventtable_track(&self->gridstate->trackevents, entry->track);
					psy_list_append(trackevents, entry);
					psy_list_next(&ite.patternnode);
					hasevent = TRUE;
					break;
				}
				psy_list_next(&ite.patternnode);
			}
			if (!hasevent) {				
				psy_List** trackevents;

				trackevents = trackereventtable_track(&self->gridstate->trackevents, track);
				psy_list_append(trackevents, NULL);
			} else if (ite.patternnode && ((psy_audio_PatternEntry*)(ite.patternnode->entry))->track <= track) {
				fill = TRUE;
			}			
		}
		/* skip remaining events of the line */
		while (ite.patternnode && (offset < seqoffset + length) && 
			(psy_audio_sequencetrackiterator_offset(&ite) + psy_dsp_epsilon * 2 <
				offset + trackerlinestate_bpl(self->linestate))) {
			psy_list_next(&ite.patternnode);
		}
		offset += trackerlinestate_bpl(self->linestate);
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
	self->gridstate->trackevents.seqoffset = seqoffset;
	self->gridstate->trackevents.clip = *clip;	
	self->gridstate->trackevents.currcursorline =
		trackerlinestate_beattoline(self->linestate,
		self->gridstate->cursor.offset + self->gridstate->cursor.seqoffset);
	self->gridstate->trackevents.currplaybarline =
		trackerlinestate_beattoline(self->linestate,
		self->linestate->lastplayposition -
		((self->linestate->singlemode)
			? self->linestate->sequenceentryoffset
			: 0.0));	
}

psy_audio_OrderIndex trackergrid_checkupdatecursorseqoffset(TrackerGrid* self,
		psy_audio_PatternCursor* rv) {
	if (rv->offset < 0 || rv->offset >=
			psy_audio_pattern_length(self->linestate->pattern)) {
		uintptr_t order;
		psy_audio_SequenceEntry* entry;

		order = psy_audio_sequence_order(self->gridstate->sequence,
			0, rv->offset + rv->seqoffset);
		if (order != psy_INDEX_INVALID) {
			entry = psy_audio_sequence_entry(self->gridstate->sequence,
				psy_audio_orderindex_make(
					workspace_sequenceeditposition(self->workspace).track,
					order));
			if (entry) {				
				rv->offset -= (psy_audio_sequenceentry_offset(entry) - rv->seqoffset);
				rv->seqoffset = psy_audio_sequenceentry_offset(entry);
			}
			return psy_audio_orderindex_make(
				workspace_sequenceeditposition(self->workspace).track,
				order);
		}
	}
	return psy_audio_orderindex_zero();
}

psy_dsp_big_beat_t trackergrid_currseqoffset(TrackerGrid* self)
{
	psy_audio_SequenceEntry* entry;

	entry = psy_audio_sequence_entry(self->gridstate->sequence,
		workspace_sequenceeditposition(self->workspace));
	if (entry) {
		return psy_audio_sequenceentry_offset(entry);
	}
	return 0.0;
}

void trackergrid_prevtrack(TrackerGrid* self)
{	
	psy_audio_PatternCursorNavigator cursornavigator;

	assert(self);

	psy_audio_patterncursornavigator_init(&cursornavigator,
		&self->gridstate->cursor, trackergridstate_pattern(self->gridstate),
		trackerlinestate_bpl(self->linestate), self->wraparound, 0);
	if (psy_audio_patterncursornavigator_prevtrack(&cursornavigator,
			trackergridstate_numsongtracks(self->gridstate))) {
		trackergrid_scrollleft(self, self->gridstate->cursor);
		trackergrid_invalidatecursor(self);
	} else if (trackergrid_scrollright(self, self->gridstate->cursor)) {
		trackergrid_invalidatecursor(self);
	}	
}

void trackergrid_storecursor(TrackerGrid* self)
{
	assert(self);

	self->oldcursor = self->gridstate->cursor;
}

void trackergrid_nexttrack(TrackerGrid* self)
{
	psy_audio_PatternCursorNavigator cursornavigator;	

	psy_audio_patterncursornavigator_init(&cursornavigator,
		&self->gridstate->cursor, trackergridstate_pattern(self->gridstate),
		trackerlinestate_bpl(self->linestate), self->wraparound, 0);
	if (psy_audio_patterncursornavigator_nexttrack(&cursornavigator,
		trackergridstate_numsongtracks(self->gridstate))) {
		if (trackergrid_scrollleft(self, self->gridstate->cursor)) {
			trackergrid_invalidatecursor(self);
		}		
	} else {
		trackergrid_scrollright(self, self->gridstate->cursor);
		trackergrid_invalidatecursor(self);
	}
}

bool trackergrid_scrollup(TrackerGrid* self, psy_audio_PatternCursor cursor)
{
	intptr_t line;
	intptr_t topline;	
	double top;

	line = trackerlinestate_beattoline(self->linestate,
		cursor.offset + cursor.seqoffset);
	top = self->linestate->lineheightpx * line;	
	if (self->gridstate->midline) {
		psy_ui_RealSize gridsize;		

		gridsize = psy_ui_component_scrollsize_px(&self->component);
		topline = (intptr_t)(gridsize.height / self->linestate->lineheightpx / 2.0);
	} else {
		topline = 0;
	}
	if (psy_ui_component_scrolltop_px(&self->component) +
			topline * self->linestate->lineheightpx > top) {
		intptr_t dlines;		
		
		dlines = (intptr_t)((psy_ui_component_scrolltop_px(&self->component) +
			topline * self->linestate->lineheightpx - top) /
			(self->linestate->lineheightpx));				
		self->linestate->cursorchanging = TRUE;		
		psy_ui_component_setscrolltop_px(&self->component,			
			psy_ui_component_scrolltop_px(&self->component) -
			psy_ui_component_scrollstep_height_px(&self->component) * dlines);
		return FALSE;
	}
	return TRUE;
}

bool trackergrid_scrolldown(TrackerGrid* self, psy_audio_PatternCursor cursor)
{
	intptr_t line;
	intptr_t visilines;	
	
	visilines = self->linestate->visilines;
	if (self->gridstate->midline) {
		visilines /= 2;
	} else {
		--visilines;
	}
	line = trackerlinestate_beattoline(self->linestate,
		cursor.offset + cursor.seqoffset);	
	if (visilines < line - psy_ui_component_scrolltop_px(&self->component) /
			self->linestate->lineheightpx) {
		intptr_t dlines;

		dlines = (intptr_t)
			(line - psy_ui_component_scrolltop_px(&self->component) /
			self->linestate->lineheightpx - visilines);
		self->linestate->cursorchanging = TRUE;
		psy_ui_component_setscrolltop_px(&self->component,			
			psy_ui_component_scrolltop_px(&self->component) +
			psy_ui_component_scrollstep_height_px(&self->component) * dlines);
		return FALSE;
	}
	return TRUE;
}

bool trackergrid_scrollleft(TrackerGrid* self, psy_audio_PatternCursor cursor)
{	
	psy_ui_Component* column;
	uintptr_t index;

	assert(self);
	
	column = psy_ui_component_intersect(trackergrid_base(self),
		psy_ui_realpoint_make(psy_ui_component_scrollleftpx(&self->component),
			0.0), &index);
	if (index != psy_INDEX_INVALID) {		
		if (index > cursor.track) {
			psy_ui_RealRectangle position;

			column = (psy_ui_Component*)psy_table_at(&self->columns, cursor.track);
			position = psy_ui_component_position(column);
			psy_ui_component_setscrollleft(&self->component,
				psy_ui_value_make_px(position.left));
			return FALSE;
		}
	}
	return TRUE;
}

bool trackergrid_scrollright(TrackerGrid* self, psy_audio_PatternCursor cursor)
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
	trackleft = trackergridstate_pxtotrack(self->gridstate,
		psy_ui_component_scrollleftpx(&self->component));
	trackright = trackergridstate_pxtotrack(self->gridstate,
		size.width +
		psy_ui_component_scrollleftpx(&self->component));	
	visitracks = trackright - trackleft;
	tracks = cursor.track + 1;
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

	if (trackergridstate_pattern(self->gridstate)) {
		psy_audio_PatternCursorNavigator cursornavigator;
		bool restorewrap;

		restorewrap = wrap;
		if (!self->linestate->singlemode) {
			wrap = TRUE;
		}
		psy_audio_patterncursornavigator_init(&cursornavigator,
			&self->gridstate->cursor,
			trackergridstate_pattern(self->gridstate),
			trackerlinestate_bpl(self->linestate), wrap, 0);		
		if (psy_audio_patterncursornavigator_advancelines(&cursornavigator, lines)) {
			trackergrid_scrolldown(self, self->gridstate->cursor);
		} else if (!self->linestate->singlemode) {
			psy_audio_OrderIndex index;

			self->gridstate->cursor.offset =
				psy_audio_pattern_length(self->linestate->pattern);
			index = trackergrid_checkupdatecursorseqoffset(self,
				&self->gridstate->cursor);
			if (psy_audio_orderindex_valid(&index)) {
				self->preventscrolltop = TRUE;
				workspace_setseqeditposition(self->workspace,
					index);
				self->preventscrolltop = FALSE;
			} else if (restorewrap) {
				self->gridstate->cursor.offset = 0;
				self->preventscrolltop = TRUE;
				workspace_setseqeditposition(self->workspace,
					psy_audio_orderindex_make(
						workspace_sequenceeditposition(self->workspace).track, 0));
				self->preventscrolltop = FALSE;
				trackergrid_scrollup(self, self->gridstate->cursor);
				trackergrid_storecursor(self);
				return;
			} else {
				self->gridstate->cursor.offset =
					psy_audio_pattern_length(self->linestate->pattern) -
					trackerlinestate_bpl(self->linestate);				
			}
			trackergrid_scrolldown(self, self->gridstate->cursor);
		} else {
			trackergrid_scrollup(self, self->gridstate->cursor);
		}		
		trackergridstate_synccursor(self->gridstate);
		trackergrid_invalidatecursor(self);
	}
}

void trackergrid_prevlines(TrackerGrid* self, uintptr_t lines, bool wrap)
{
	assert(self);

	if (trackergridstate_pattern(self->gridstate)) {
		psy_audio_PatternCursorNavigator cursornavigator;
		bool restorewrap;

		restorewrap = wrap;
		if (!self->linestate->singlemode) {
			wrap = TRUE;
		}
		psy_audio_patterncursornavigator_init(&cursornavigator, &self->gridstate->cursor,
			trackergridstate_pattern(self->gridstate), trackerlinestate_bpl(self->linestate), wrap, 0);		
		if (!psy_audio_patterncursornavigator_prevlines(&cursornavigator, lines)) {
			trackergrid_scrollup(self, self->gridstate->cursor);
		} else if (!self->linestate->singlemode) {
			psy_audio_OrderIndex index;			
			
			self->gridstate->cursor.offset = -trackerlinestate_bpl(self->linestate);
			index = trackergrid_checkupdatecursorseqoffset(self,
				&self->gridstate->cursor);
			if (psy_audio_orderindex_valid(&index)) {
				psy_audio_PatternCursor cursor;

				cursor = self->gridstate->cursor;
				self->preventscrolltop = TRUE;
				workspace_setseqeditposition(self->workspace, index);
				self->preventscrolltop = FALSE;
				self->gridstate->cursor = cursor;
			} else if (restorewrap) {				
				self->preventscrolltop = TRUE;
				workspace_setseqeditposition(self->workspace,
					psy_audio_orderindex_make(
						workspace_sequenceeditposition(self->workspace).track,
						psy_audio_sequence_track_size(
							self->gridstate->sequence, 0) - 1));
				self->preventscrolltop = FALSE;
				if (self->linestate->pattern) {
					self->gridstate->cursor.offset =
						psy_audio_pattern_length(self->linestate->pattern) -
						trackerlinestate_bpl(self->linestate);
				}
				trackergrid_scrolldown(self, self->gridstate->cursor);
				trackergrid_storecursor(self);
				return;
			} else {
				self->gridstate->cursor.offset =
					psy_audio_pattern_length(self->linestate->pattern) -
					trackerlinestate_bpl(self->linestate);
			}			
			trackergrid_scrollup(self, self->gridstate->cursor);
		} else {
			trackergrid_scrolldown(self, self->gridstate->cursor);
		}
		trackergridstate_synccursor(self->gridstate);
		trackergrid_invalidatecursor(self);
	}
}

void trackergrid_home(TrackerGrid* self)
{
	assert(self);

	if (self->ft2home) {
		self->gridstate->cursor.offset = 0.0;
		trackergrid_scrollup(self, self->gridstate->cursor);
	} else {
		if (self->gridstate->cursor.column != 0) {
			self->gridstate->cursor.column = 0;
		} else {
			self->gridstate->cursor.track = 0;
			self->gridstate->cursor.column = 0;
		}
		trackergrid_scrollleft(self, self->gridstate->cursor);
	}
	trackergridstate_synccursor(self->gridstate);
	trackergrid_invalidatecursor(self);
}

void trackergrid_end(TrackerGrid* self)
{
	assert(self);

	if (self->ft2home) {
		self->gridstate->cursor.offset = trackergridstate_pattern(self->gridstate)->length - trackerlinestate_bpl(self->linestate);
		trackergrid_scrolldown(self, self->gridstate->cursor);
	} else {
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
		columndef = trackdef_columndef(trackdef, self->gridstate->cursor.column);
		if (self->gridstate->cursor.track != trackergridstate_numsongtracks(self->gridstate) - 1 ||
				self->gridstate->cursor.digit != columndef->numdigits - 1 ||
				self->gridstate->cursor.column != TRACKER_COLUMN_PARAM) {
			if (self->gridstate->cursor.column == TRACKER_COLUMN_PARAM &&
				self->gridstate->cursor.digit == columndef->numdigits - 1) {
				self->gridstate->cursor.track = trackergridstate_numsongtracks(self->gridstate) - 1;
				trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
				columndef = trackdef_columndef(trackdef, TRACKER_COLUMN_PARAM);
				self->gridstate->cursor.column = TRACKER_COLUMN_PARAM;
				self->gridstate->cursor.digit = columndef->numdigits - 1;
				trackergrid_scrollright(self, self->gridstate->cursor);
			} else {
				trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
				columndef = trackdef_columndef(trackdef, TRACKER_COLUMN_PARAM);
				self->gridstate->cursor.column = TRACKER_COLUMN_PARAM;
				self->gridstate->cursor.digit = columndef->numdigits - 1;
			}			
		}
	}
	trackergridstate_synccursor(self->gridstate);
	trackergrid_invalidatecursor(self);
}

void trackergrid_prevcol(TrackerGrid* self)
{
	bool invalidate;

	invalidate = TRUE;
	switch (trackergridstate_prevcol(self->gridstate, self->wraparound)) {
	case SCROLL_DIR_LEFT:
		invalidate = trackergrid_scrollleft(self, self->gridstate->cursor);
		break;
	case SCROLL_DIR_RIGHT:
		invalidate = trackergrid_scrollright(self, self->gridstate->cursor);
		break;
	default:;
	}
	trackergridstate_synccursor(self->gridstate);
	if (invalidate) {
		trackergrid_invalidatecursor(self);
	}
}

void trackergrid_nextcol(TrackerGrid* self)
{
	bool invalidate;

	invalidate = TRUE;
	switch (trackergridstate_nextcol(self->gridstate, self->wraparound)) {
	case SCROLL_DIR_LEFT:
		invalidate = trackergrid_scrollleft(self, self->gridstate->cursor);
		break;
	case SCROLL_DIR_RIGHT:
		invalidate = trackergrid_scrollright(self, self->gridstate->cursor);
		break;
	default:;
	}
	trackergridstate_synccursor(self->gridstate);
	if (invalidate) {
		trackergrid_invalidatecursor(self);
	}
}

void trackergrid_selectall(TrackerGrid* self)
{
	assert(self);

	trackergridstate_selectall(self->gridstate);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_selectcol(TrackerGrid* self)
{
	assert(self);

	trackergridstate_selectcol(self->gridstate);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_selectbar(TrackerGrid* self)
{
	assert(self);

	trackergridstate_selectbar(self->gridstate);
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
			trackergridstate_pattern(self->gridstate),
			self->gridstate->cursor, &prev);
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

	node = psy_audio_pattern_findnode_cursor(defaults, self->gridstate->cursor,
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

	if (self->gridstate->cursor.offset - trackerlinestate_bpl(self->linestate) >= 0) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternNode* p;
		psy_audio_PatternNode* q;
		psy_audio_PatternNode* node;

		if (self->ft2delete) {
			trackergrid_prevline(self);
		}
		node = psy_audio_pattern_findnode_cursor(
			trackergridstate_pattern(self->gridstate),
			self->gridstate->cursor, &prev);
		if (node) {
			psy_audio_pattern_remove(trackergridstate_pattern(self->gridstate), node);
			psy_audio_sequencer_checkiterators(
				&workspace_player(self->workspace)->sequencer,
				node);
		}
		p = (prev)
			? prev->next
			: psy_audio_pattern_begin(trackergridstate_pattern(self->gridstate));
		for (; p != NULL; p = q) {
			psy_audio_PatternEntry* entry;

			q = p->next;
			entry = psy_audio_patternnode_entry(p);
			if (entry->track == self->gridstate->cursor.track) {
				psy_audio_PatternEvent event;
				psy_dsp_big_beat_t offset;
				uintptr_t track;
				psy_audio_PatternNode* node;
				psy_audio_PatternNode* prev;

				event = *psy_audio_patternentry_front(entry);
				offset = entry->offset;
				track = entry->track;
				psy_audio_pattern_remove(trackergridstate_pattern(self->gridstate), p);
				psy_audio_sequencer_checkiterators(
					&workspace_player(self->workspace)->sequencer, p);
				offset -= (psy_dsp_big_beat_t)trackerlinestate_bpl(self->linestate);
				node = psy_audio_pattern_findnode(
					trackergridstate_pattern(self->gridstate), track, offset,
					(psy_dsp_big_beat_t)trackerlinestate_bpl(self->linestate),
					&prev);
				if (node) {
					psy_audio_PatternEntry* entry;

					entry = (psy_audio_PatternEntry*)node->entry;
					*psy_audio_patternentry_front(entry) = event;
				} else {
					psy_audio_pattern_insert(
						trackergridstate_pattern(self->gridstate), prev, track,
						offset, &event);
				}
			}
		}
	}
}

void trackergrid_rowclear(TrackerGrid* self)
{
	assert(self);

	if (self->gridstate->cursor.column == TRACKER_COLUMN_NOTE) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&removecommand_alloc(trackergridstate_pattern(self->gridstate),
				trackerlinestate_bpl(self->linestate),
			self->gridstate->cursor,
			(self->gridstate->synccursor)
				? self->workspace
				: NULL)->command);
		trackergrid_advanceline(self);		
	} else {
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
		columndef = trackdef_columndef(trackdef, self->gridstate->cursor.column);
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
			self->gridstate->cursor.track = self->chordbegin;
			trackergrid_scrollleft(self, self->gridstate->cursor);
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
			&insertcommand_alloc(trackergridstate_pattern(self->gridstate),
				trackerlinestate_bpl(self->linestate),
				self->gridstate->cursor, ev,
				(self->gridstate->synccursor) ? self->workspace : NULL)->command);
		if (chord != FALSE) {
			trackergrid_nexttrack(self);
		} else {
			trackergrid_advanceline(self);
		}
		if (ev.note < psy_audio_NOTECOMMANDS_RELEASE) {
			self->gridstate->cursor.key = ev.note;
			trackergridstate_synccursor(self->gridstate);
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
					&insertcommand_alloc(trackergridstate_pattern(self->gridstate),
						trackerlinestate_bpl(self->linestate),
						self->gridstate->cursor, ev,
						(self->gridstate->synccursor) ? self->workspace : NULL)->command);
				trackergrid_advanceline(self);
				if (ev.note < psy_audio_NOTECOMMANDS_RELEASE) {
					self->gridstate->cursor.key = ev.note;
					trackergridstate_synccursor(self->gridstate);
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

	if (trackergridstate_pattern(self->gridstate) && value != -1) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternEntry* entry;
		psy_audio_PatternNode* node;
		psy_audio_PatternEntry newentry;
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
		columndef = trackdef_columndef(trackdef, self->gridstate->cursor.column);
		psy_audio_patternentry_init(&newentry);
		node = psy_audio_pattern_findnode_cursor(
			trackergridstate_pattern(self->gridstate),
			self->gridstate->cursor, &prev);
		if (node) {
			entry = (psy_audio_PatternEntry*)node->entry;
		} else {
			entry = &newentry;
		}
		if (digit) {
			trackergrid_enterdigitcolumn(self, entry, self->gridstate->cursor,
				value);
		} else {
			entervaluecolumn(entry, self->gridstate->cursor.column, value);
		}
		trackergrid_preventpatternsync(self);
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_alloc(trackergridstate_pattern(self->gridstate),
				trackerlinestate_bpl(self->linestate),
				self->gridstate->cursor,
				*psy_audio_patternentry_front(entry),
				self->workspace)->command);
		if (self->effcursoralwaysdown) {
			trackergrid_advanceline(self);
		} else {
			if (!digit) {
				if (columndef->wrapclearcolumn == TRACKER_COLUMN_NONE) {
					trackergrid_nextcol(self);
				} else {
					self->gridstate->cursor.digit = 0;
					self->gridstate->cursor.column = columndef->wrapclearcolumn;
					trackergrid_advanceline(self);
				}
			} else if (self->gridstate->cursor.digit + 1 >= columndef->numdigits) {
				if (columndef->wrapeditcolumn == TRACKER_COLUMN_NONE) {
					trackergrid_nextcol(self);
				} else {
					self->gridstate->cursor.digit = 0;
					self->gridstate->cursor.column = columndef->wrapeditcolumn;
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
	trackergrid_invalidateinternalcursor(self, self->gridstate->cursor);
	trackergrid_storecursor(self);	
}

void trackergrid_invalidateinternalcursor(TrackerGrid* self,
	psy_audio_PatternCursor cursor)
{
	psy_ui_Component* column;	

	column = psy_ui_component_at(trackergrid_base(self), cursor.track);
	if (column) {
		psy_ui_RealSize size;

		size = psy_ui_component_scrollsize_px(column);
		psy_ui_component_invalidaterect(column,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					0.0,
					trackerlinestate_beattopx(self->linestate, cursor.offset +
						cursor.seqoffset)),
				psy_ui_realsize_make(
					size.width,
					trackerlinestate_lineheight(self->linestate))));
	}
}

void trackergrid_invalidateline(TrackerGrid* self, psy_dsp_big_beat_t position)
{
	assert(self);

	if (!trackergridstate_pattern(self->gridstate)) {
		return;
	}	
	if (!self->gridstate->singlemode ||
			psy_dsp_testrange(position, self->linestate->sequenceentryoffset,
			psy_audio_pattern_length(trackergridstate_pattern(
				self->gridstate)))) {
		psy_ui_RealSize size;		
		
		size = psy_ui_component_scrollsize_px(&self->component);		
		psy_ui_component_invalidaterect(&self->component,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(					
					psy_ui_component_scrollleftpx(&self->component),
					trackerlinestate_beattopx(self->linestate,
						position -
						((self->gridstate->singlemode)
						? self->linestate->sequenceentryoffset
						: 0.0))),
				psy_ui_realsize_make(size.width, self->linestate->lineheightpx)));
		if (trackergridstate_pattern(
			self->gridstate)) {
			trackergrid_preventpatternsync(self);
			trackergridstate_pattern(
				self->gridstate)->opcount++;
			trackergrid_resetpatternsync(self);
		}
	}
}

void trackergrid_onscroll(TrackerGrid* self, psy_ui_Component* sender)
{
	assert(self);

	if (self->gridstate->midline) {
		trackergrid_clearmidline(self);
	}
}

void trackergrid_clearmidline(TrackerGrid* self)
{
	psy_ui_RealSize size;

	assert(self);

	size = psy_ui_component_scrollsize_px(&self->component);	
	self->gridstate->midline = FALSE;
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scrollleftpx(&self->component),
				((self->linestate->visilines) / 2 - 1) * self->linestate->lineheightpx +
					psy_ui_component_scrolltop_px(&self->component)),
			psy_ui_realsize_make(size.width, self->linestate->lineheightpx)));
	psy_ui_component_update(&self->component);
	self->gridstate->midline = TRUE;
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scrollleftpx(&self->component),
				(self->linestate->visilines / 2 - 2) * self->linestate->lineheightpx +
				psy_ui_component_scrolltop_px(&self->component)),
			psy_ui_realsize_make(size.width, self->linestate->lineheightpx * 4)));		
}

void trackergrid_centeroncursor(TrackerGrid* self)
{
	intptr_t line;

	assert(self);

	line = trackerlinestate_beattoline(self->linestate,
		self->gridstate->cursor.offset);
	psy_ui_component_setscrolltop_px(&self->component,		
		-(self->linestate->visilines / 2 - line) *
			self->linestate->lineheightpx);
}

void trackergrid_setcentermode(TrackerGrid* self, int mode)
{
	assert(self);

	self->gridstate->midline = mode;
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

	if (self->gridstate->trackconfig->colresize) {
		psy_signal_emit(&self->signal_colresize, self, 0);
	} else if (trackergridstate_pattern(self->gridstate) && ev->button == 1) {		
		if (!self->linestate->singlemode) {
			psy_audio_OrderIndex index;

			index = trackergrid_checkupdatecursorseqoffset(self,
				&self->gridstate->dragselectionbase);
			if (psy_audio_orderindex_valid(&index)) {
				self->preventscrolltop = TRUE;
				workspace_setseqeditposition(self->workspace,
					index);
				self->preventscrolltop = FALSE;
			}
		}
		self->lastdragcursor = self->gridstate->dragselectionbase;
		psy_audio_patternselection_init_all(&self->gridstate->selection,
			self->gridstate->dragselectionbase, self->gridstate->dragselectionbase);
		psy_audio_patternselection_disable(&self->gridstate->selection);
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
		if (self->gridstate->trackconfig->colresize) {
			psy_signal_emit(&self->signal_colresize, self, 0);
		} else {					
			if (!psy_audio_patterncursor_equal(&self->gridstate->dragcursor,
					&self->lastdragcursor)) {
				if (!psy_audio_patternselection_valid(&self->gridstate->selection)) {
					trackergridstate_startdragselection(self->gridstate,
						self->gridstate->dragcursor,
						trackerlinestate_bpl(self->linestate));
				} else {					
					trackergrid_dragselection(self, self->gridstate->dragcursor);
				}
				psy_ui_component_invalidate(&self->component);
				self->lastdragcursor = self->gridstate->dragcursor;
			}			
		}
	}
}

void trackergrid_dragselection(TrackerGrid* self, psy_audio_PatternCursor cursor)
{
	int restoremidline = self->gridstate->midline;
	
	self->gridstate->midline = 0;
	trackergridstate_dragselection(self->gridstate, cursor,
		trackerlinestate_bpl(self->linestate));
	if (cursor.offset < self->lastdragcursor.offset) {
		trackergrid_scrollup(self, cursor);
	} else {
		trackergrid_scrolldown(self, cursor);
	}
	if (cursor.track < self->lastdragcursor.track) {
		trackergrid_scrollleft(self, cursor);
	} else {
		trackergrid_scrollright(self, cursor);
	}
	self->gridstate->midline = restoremidline;
}

void trackergrid_onmouseup(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_releasecapture(&self->component);
	if (ev->button != 1) {
		return;
	}	
	if (self->gridstate->trackconfig->colresize) {
		self->gridstate->trackconfig->colresize = FALSE;
		psy_signal_emit(&self->signal_colresize, self, 0);		
	} else if (!psy_audio_patternselection_valid(&self->gridstate->selection)) {
		/* set cursor only, if no selection was made */
		if (!self->linestate->singlemode) {
			psy_audio_OrderIndex index;
			
			index = trackergrid_checkupdatecursorseqoffset(self,
				&self->gridstate->dragselectionbase);
			if (psy_audio_orderindex_valid(&index)) {
				self->preventscrolltop = TRUE;
				workspace_setseqeditposition(self->workspace,
					index);
				self->preventscrolltop = FALSE;
			}			
		}
		self->gridstate->cursor = self->gridstate->dragselectionbase;
		if (!psy_audio_patterncursor_equal(&self->oldcursor, &self->gridstate->cursor)) {
			trackergrid_invalidatecursor(self);
		}
		trackergridstate_synccursor(self->gridstate);
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
	trackergridstate_setpattern(self->gridstate, pattern);	
	trackerlinestate_setpattern(self->linestate, pattern);
	trackergrid_resetpatternsync(self);
	psy_ui_component_updateoverflow(trackergrid_base(self));	
	if (psy_audio_player_playing(workspace_player(self->workspace)) ||
			!trackergridstate_cursorposition_valid(self->gridstate)) {		
		if (!self->gridstate->singlemode) {
			self->gridstate->cursor.seqoffset =
				trackergrid_currseqoffset(self);
			if (!workspace_followingsong(self->workspace)) {
				psy_ui_component_setscrolltop_px(&self->component,
					trackerlinestate_beattopx(self->linestate,
						self->gridstate->cursor.offset +
						self->gridstate->cursor.seqoffset));
			} else {
				self->gridstate->cursor.offset = 0;
			}
		} else {
			self->gridstate->cursor.offset = 0;
			psy_ui_component_setscrolltop_px(&self->component, 0.0);
		}		
	} else if (!self->gridstate->singlemode) {
		self->gridstate->cursor.offset = 0;
		self->gridstate->cursor.seqoffset =
			trackergrid_currseqoffset(self);		
		if (!self->preventscrolltop) {
			psy_ui_component_setscrolltop_px(&self->component,
				trackerlinestate_beattopx(self->linestate,
					self->gridstate->cursor.offset +
					self->gridstate->cursor.seqoffset));
		}
	}
	if (self->gridstate->midline) {
		trackergrid_centeroncursor(self);
	}
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

	if (trackergridstate_pattern(self->gridstate)) {
		self->component.opcount = trackergridstate_pattern(
			self->gridstate)->opcount;
	} else {
		self->component.opcount = 0;
	}
}

void trackergrid_changegenerator(TrackerGrid* self)
{
	assert(self);

	patterncmds_changemachine(&self->cmds, self->gridstate->selection);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_changeinstrument(TrackerGrid* self)
{
	assert(self);
	
	patterncmds_changeinstrument(&self->cmds, self->gridstate->selection);
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

	patterncmds_blockcopy(&self->cmds, self->gridstate->selection);
}

void trackergrid_blockpaste(TrackerGrid* self)
{
	assert(self);

	patterncmds_blockpaste(&self->cmds,	self->gridstate->cursor, FALSE);
	trackergrid_movecursorwhenpaste(self);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blockmixpaste(TrackerGrid* self)
{
	assert(self);

	patterncmds_blockpaste(&self->cmds,	self->gridstate->cursor, TRUE);
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

		begin = end = self->gridstate->cursor;
		end.track += self->workspace->patternpaste.maxsongtracks;
		end.offset += self->workspace->patternpaste.length;
		end.track = begin.track;
		if (end.offset >= psy_audio_pattern_length(trackergridstate_pattern(self->gridstate))) {
			end.offset = psy_audio_pattern_length(trackergridstate_pattern(self->gridstate)) -
				trackerlinestate_bpl(self->linestate);
		}		
		self->gridstate->cursor = end;
		trackergridstate_synccursor(self->gridstate);
		self->oldcursor = end;
		return TRUE;
	}
	return FALSE;
}

void trackergrid_blockdelete(TrackerGrid* self)
{
	assert(self);
	
	patterncmds_blockdelete(&self->cmds, self->gridstate->selection);
	psy_ui_component_invalidate(&self->component);	
}

void trackergrid_blockstart(TrackerGrid* self)
{
	assert(self);

	self->gridstate->dragselectionbase = self->gridstate->cursor;
	trackergridstate_startdragselection(self->gridstate,
		self->gridstate->cursor,
		trackerlinestate_bpl(self->linestate));
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blockend(TrackerGrid* self)
{
	assert(self);

	trackergrid_dragselection(self, self->gridstate->cursor);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blockunmark(TrackerGrid* self)
{
	assert(self);

	psy_audio_patternselection_disable(&self->gridstate->selection);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blocktransposeup(TrackerGrid* self)
{
	assert(self);
	
	patterncmds_blocktranspose(&self->cmds, self->gridstate->selection,
		self->gridstate->cursor, +1);
}

void trackergrid_blocktransposedown(TrackerGrid* self)
{
	assert(self);

	patterncmds_blocktranspose(&self->cmds, self->gridstate->selection,
		self->gridstate->cursor, -1);
}

void trackergrid_blocktransposeup12(TrackerGrid* self)
{
	assert(self);

	patterncmds_blocktranspose(&self->cmds,	self->gridstate->selection,
		self->gridstate->cursor, 12);
}

void trackergrid_blocktransposedown12(TrackerGrid* self)
{
	assert(self);

	patterncmds_blocktranspose(&self->cmds,	self->gridstate->selection,
		self->gridstate->cursor, -12);
}

void trackergrid_enterdigitcolumn(TrackerGrid* self,
	psy_audio_PatternEntry* entry, psy_audio_PatternCursor cursor,
	intptr_t digitvalue)
{
	TrackDef* trackdef;

	assert(self);

	trackdef = trackergridstate_trackdef(self->gridstate, cursor.track);
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
		if (self->gridstate->cursor.column != TRACKER_COLUMN_NOTE) {
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
			&insertcommand_alloc(trackergridstate_pattern(self->gridstate), trackerlinestate_bpl(self->linestate),
				self->gridstate->cursor, event, self->workspace)->command);
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

	self->gridstate->showemptydata = showstate;
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_ongotocursor(TrackerGrid* self, Workspace* sender,
	psy_audio_PatternCursor* cursor)
{			
	double y;
	psy_ui_RealSize clientsize;
	
	y = trackerlinestate_beattopx(self->linestate,
		cursor->offset -
		((self->gridstate->singlemode)
			? self->linestate->sequenceentryoffset
			: 0.0));
	clientsize = psy_ui_component_clientsize_px(&self->component);
	if (y < psy_ui_component_scrolltop_px(&self->component) ||
		y > psy_ui_component_scrolltop_px(&self->component) + clientsize.height) {
		psy_ui_component_setscrolltop_px(&self->component,
			trackerlinestate_beattopx(self->linestate,
				self->gridstate->cursor.offset +
				self->gridstate->cursor.seqoffset));
	}
}

void trackergrid_build(TrackerGrid* self)
{
	uintptr_t t;
	
	psy_ui_component_clear(&self->component);
	psy_table_clear(&self->columns);
	for (t = 0; t < trackergridstate_numsongtracks(self->gridstate); ++t) {
		psy_table_insert(&self->columns, t, (void*)
			trackercolumn_allocinit(&self->component,
				(self->view) ? self->view : &self->component, t,
				self->gridstate, self->linestate, self->workspace));
	}
	psy_ui_component_align(&self->component);
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequenceview.h"
// host
#include "styles.h"
#include "workspace.h"
// audio
#include <songio.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// SequenceListViewState
void sequencelistviewstate_init(SequenceListViewState* self,
	SequenceCmds* cmds)
{	
	assert(self);
	assert(cmds);

	self->cmds = cmds;

	self->trackwidth = psy_ui_value_make_ew(16.0);
	psy_ui_value_setroundmode(&self->trackwidth, psy_ui_ROUND_FLOOR);
	self->lineheight = psy_ui_value_make_eh(1.2);	
	psy_ui_value_setroundmode(&self->lineheight, psy_ui_ROUND_FLOOR);
	self->cmd = SEQLVCMD_NONE;
	self->cmd_orderindex = psy_audio_orderindex_zero();
	self->showpatternnames = FALSE;
}

// SequenceTrackHeaders
// prototypes
static void sequencetrackheaders_ondestroy(SequenceTrackHeaders*);
static void sequencetrackheaders_onmouseup(SequenceTrackHeaders*,
	psy_ui_MouseEvent*);
static void sequencetrackheaders_onnewtrack(SequenceTrackHeaders*,
	psy_ui_Button* sender);
static void sequencetrackheaders_ondeltrack(SequenceTrackHeaders*,
	TrackBox* sender);
// vtable
static psy_ui_ComponentVtable trackheaderviews_vtable;
static bool trackheaderviews_vtable_initialized = FALSE;

static void trackheaderview_vtable_init(SequenceTrackHeaders* self)
{
	if (!trackheaderviews_vtable_initialized) {
		trackheaderviews_vtable = *(self->component.vtable);
		trackheaderviews_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			sequencetrackheaders_ondestroy;		
		trackheaderviews_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			sequencetrackheaders_onmouseup;
		trackheaderviews_vtable_initialized = TRUE;
	}
}
// implemenetation
void sequencetrackheaders_init(SequenceTrackHeaders* self,
	psy_ui_Component* parent, SequenceListViewState* state)
{	
	// psy_ui_Margin spacing;

	psy_ui_component_init(&self->component, parent, NULL);
	// psy_ui_margin_init_all(&spacing,
	//	psy_ui_value_zero(), psy_ui_value_zero(), psy_ui_value_zero(),
	//	psy_ui_value_zero());
	// psy_ui_component_setspacing(&self->component, &spacing);
	trackheaderview_vtable_init(self);
	self->component.vtable = &trackheaderviews_vtable;
	self->state = state;	
	psy_ui_component_setminimumsize(&self->component,
		psy_ui_size_make_em(0.0, 2.0));
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_margin_zero());
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setoverflow(&self->component,
		psy_ui_OVERFLOW_HSCROLL);	
	psy_signal_init(&self->signal_trackselected);
	psy_ui_component_init(&self->client, &self->component, NULL);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_FIXED_RESIZE);
	psy_ui_component_setdefaultalign(&self->client,
		psy_ui_ALIGN_LEFT, psy_ui_margin_zero());
	psy_ui_component_setalignexpand(&self->client,
		psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setoverflow(&self->client,
		psy_ui_OVERFLOW_HSCROLL);	
	sequencetrackheaders_build(self);
}

void sequencetrackheaders_ondestroy(SequenceTrackHeaders* self)
{	
	psy_signal_dispose(&self->signal_trackselected);
}

void sequencetrackheaders_build(SequenceTrackHeaders* self)
{
	psy_audio_Sequence* sequence;

	psy_ui_component_clear(&self->client);	
	sequence = self->state->cmds->sequence;
	if (sequence) {
		psy_audio_SequenceTrackNode* t;
		uintptr_t c;
		psy_ui_Button* newtrack;

		for (t = sequence->tracks, c = 0; t != NULL;
			psy_list_next(&t), ++c) {
			SequenceTrackBox* sequencetrackbox;
			
			sequencetrackbox = sequencetrackbox_allocinit(&self->client,
				&self->component, self->state->cmds->sequence, c, NULL);
			if (sequencetrackbox) {
				psy_ui_component_setminimumsize(
					sequencetrackbox_base(sequencetrackbox),
					psy_ui_size_make(
						self->state->trackwidth,
						psy_ui_value_zero()));
				psy_signal_connect(&sequencetrackbox->trackbox.signal_close, self,
					sequencetrackheaders_ondeltrack);
			}
		}
		newtrack = psy_ui_button_allocinit(&self->client, &self->component);
		if (newtrack) {
			psy_ui_button_settext(newtrack, "seqview.new-trk");
			newtrack->stoppropagation = FALSE;
			psy_signal_connect(&newtrack->signal_clicked, self,
				sequencetrackheaders_onnewtrack);
		}
	}
	psy_ui_component_align(&self->client);
}

void sequencetrackheaders_onmouseup(SequenceTrackHeaders* self,
	psy_ui_MouseEvent* ev)
{	
	if (self->state->cmd == SEQLVCMD_NEWTRACK) {
		sequencecmds_appendtrack(self->state->cmds);		
	} else if (self->state->cmd == SEQLVCMD_DELTRACK) {
		sequencecmds_deltrack(self->state->cmds,
			self->state->cmd_orderindex.track);
	}
	self->state->cmd = SEQLVCMD_NONE;
}

void sequencetrackheaders_onnewtrack(SequenceTrackHeaders* self,
	psy_ui_Button* sender)
{
	self->state->cmd = SEQLVCMD_NEWTRACK;
}

void sequencetrackheaders_ondeltrack(SequenceTrackHeaders* self,
	TrackBox* sender)
{
	self->state->cmd = SEQLVCMD_DELTRACK;
	self->state->cmd_orderindex.track = trackbox_trackindex(sender);
}

// SequenceListTrack
// prototypes
static void sequencelisttrack_ondestroy(SequenceListTrack*);
static void sequencelisttrack_onpreferredsize(SequenceListTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void sequencelisttrack_ondraw(SequenceListTrack*, psy_ui_Graphics*);
static void sequencelisttrack_drawprogressbar(SequenceListTrack*,
	psy_ui_Graphics*, double x, double y,
	psy_audio_SequenceEntry*);
void sequencelisttrack_onmousedown(SequenceListTrack* self,
	psy_ui_MouseEvent*);
void sequencelisttrack_onmousedoubleclick(SequenceListTrack*,
	psy_ui_MouseEvent*);
static void sequencelisttrack_onsequenceselectionselect(SequenceListTrack*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void sequencelisttrack_onsequenceselectiondeselect(SequenceListTrack*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);

// vtable
static psy_ui_ComponentVtable sequencelisttrack_vtable;
static bool sequencelisttrack_vtable_initialized = FALSE;

static void sequencelisttrack_vtable_init(SequenceListTrack* self)
{
	if (!sequencelisttrack_vtable_initialized) {
		sequencelisttrack_vtable = *(self->component.vtable);
		sequencelisttrack_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			sequencelisttrack_ondestroy;
		sequencelisttrack_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			sequencelisttrack_ondraw;
		sequencelisttrack_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			sequencelisttrack_onpreferredsize;
		sequencelisttrack_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			sequencelisttrack_onmousedown;
		sequencelisttrack_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			sequencelisttrack_onmousedoubleclick;
		sequencelisttrack_vtable_initialized = TRUE;
	}
	self->component.vtable = &sequencelisttrack_vtable;
}

void sequencelisttrack_init(SequenceListTrack* self, psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t trackindex,
	psy_audio_SequenceTrack* track, SequenceListViewState* state)
{
	psy_ui_component_init(&self->component, parent, view);
	sequencelisttrack_vtable_init(self);	
	psy_ui_component_setstyletypes(&self->component,
		STYLE_SEQLISTVIEW_TRACK, psy_INDEX_INVALID,
		STYLE_SEQLISTVIEW_TRACK_SELECT, psy_INDEX_INVALID);
	self->state = state;
	self->trackindex = trackindex;
	self->track = track;
	psy_signal_connect(
		&state->cmds->workspace->sequenceselection.signal_select,
		self, sequencelisttrack_onsequenceselectionselect);
	psy_signal_connect(
		&state->cmds->workspace->sequenceselection.signal_deselect,
		self, sequencelisttrack_onsequenceselectiondeselect);
	if (state->cmds->workspace->sequenceselection.editposition.track ==
			trackindex) {
		psy_ui_component_addstylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

SequenceListTrack* sequencelisttrack_alloc(void)
{
	return (SequenceListTrack*)malloc(sizeof(SequenceListTrack));
}

SequenceListTrack* sequencelisttrack_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	uintptr_t trackindex, psy_audio_SequenceTrack* track,
	SequenceListViewState* state)
{
	SequenceListTrack* rv;

	rv = sequencelisttrack_alloc();
	if (rv) {
		sequencelisttrack_init(rv, parent, view, trackindex, track, state);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

void sequencelisttrack_ondestroy(SequenceListTrack* self)
{
	psy_signal_disconnect(
		&self->state->cmds->workspace->sequenceselection.signal_select,
		self, sequencelisttrack_onsequenceselectionselect);
	psy_signal_disconnect(
		&self->state->cmds->workspace->sequenceselection.signal_deselect,
		self, sequencelisttrack_onsequenceselectiondeselect);
}

void sequencelisttrack_ondraw(SequenceListTrack* self, psy_ui_Graphics* g)
{
	psy_List* p;
	uintptr_t c;
	double cpy = 0;
	char text[20];
	psy_ui_RealRectangle r;
	psy_ui_RealSize size;
	uintptr_t startrow;
	uintptr_t endrow;		
	double lineheightpx;	
	
	lineheightpx = psy_max(1.0, floor(psy_ui_value_px(&self->state->lineheight,
		psy_ui_component_textmetric(&self->component), NULL)));
	startrow = (uintptr_t)floor(psy_max(0, (g->clip.top / lineheightpx)));
	endrow = (uintptr_t)(floor(g->clip.bottom / lineheightpx + 0.5));
	size = psy_ui_component_size_px(&self->component);
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_settextcolour(g, psy_ui_colour_make(0));
	cpy = lineheightpx * startrow;
	p = psy_list_at(p = self->track->entries, startrow);
	for (c = startrow; p != NULL; psy_list_next(&p), ++c, cpy += lineheightpx) {
		psy_audio_SequenceEntry* sequenceentry;
		bool rowplaying = FALSE;

		sequenceentry = (psy_audio_SequenceEntry*)p->entry;
		rowplaying = psy_audio_player_playing(self->state->cmds->player) &&
			psy_audio_player_playlist_position(self->state->cmds->player) == c;
		if (self->state->showpatternnames) {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_sequenceentry_pattern(sequenceentry,
				self->state->cmds->patterns);
			if (pattern) {
				psy_snprintf(text, 20, "%02X: %s %4.2f", c,
					psy_audio_pattern_name(pattern),
					(float)sequenceentry->offset);
			} else {
				psy_snprintf(text, 20, "%02X:%02X(ERR) %4.2f", c,
					(int)psy_audio_sequenceentry_patternslot(sequenceentry),
					(float)sequenceentry->offset);
			}
		} else {
			psy_snprintf(text, 20, "%02X:%02X  %4.2f", c,
				(int)psy_audio_sequenceentry_patternslot(sequenceentry),
				(float)sequenceentry->offset);
		}
		if (rowplaying) {
			sequencelisttrack_drawprogressbar(self, g, 0, cpy, sequenceentry);
		}
		if (psy_audio_sequenceselection_isselected(
				&self->state->cmds->workspace->sequenceselection,
				psy_audio_orderindex_make(self->trackindex, c))) {
			if (!rowplaying) {
				psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x009B7800));
			} else {
				psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
			}
			if (psy_ui_componentstyle_style(&self->component.style, psy_ui_STYLESTATE_SELECT)) {
				psy_ui_settextcolour(g, psy_ui_componentstyle_style(
					&self->component.style, psy_ui_STYLESTATE_SELECT)->colour);
			}
			//self->foundselected = 1;
		} else if (rowplaying) {
			psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x00232323));
			psy_ui_settextcolour(g, psy_ui_style(STYLE_SEQ_PROGRESS)->colour);
		} else {
			psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x00232323));
			psy_ui_settextcolour(g, psy_ui_style(STYLE_SEQLISTVIEW)->colour);
		}
		psy_ui_textout(g, 0, cpy, text, psy_strlen(text));
	}
}

void sequencelisttrack_drawprogressbar(SequenceListTrack* self,
	psy_ui_Graphics* g, double x, double y,
	psy_audio_SequenceEntry* sequenceentry)
{
	psy_ui_RealRectangle r;
	psy_ui_RealSize size;

	size = psy_ui_component_size_px(&self->component);
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(x, y),
		psy_ui_realsize_make(
			psy_audio_player_playlist_rowprogress(self->state->cmds->player) * size.width,
			psy_ui_value_px(&self->state->lineheight,
				psy_ui_component_textmetric(&self->component), NULL)));
	psy_ui_drawsolidrectangle(g, r, psy_ui_style(STYLE_SEQ_PROGRESS)->backgroundcolour);
}

void sequencelisttrack_onpreferredsize(SequenceListTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{	
	rv->width = self->state->trackwidth;
	if (self->track) {
		const psy_ui_TextMetric* tm;

		tm = psy_ui_component_textmetric(&self->component);
		rv->height = psy_ui_value_make_px(
			floor(psy_ui_value_px(&self->state->lineheight, tm, NULL) *
			(double)psy_list_size(self->track->entries)));		
	} else {
		rv->height = psy_ui_value_zero();		
	}			
}

void sequencelisttrack_onmousedown(SequenceListTrack* self,
	psy_ui_MouseEvent* ev)
{
	if (self->track) {		
		if (self->track->entries) {
			self->state->cmd_orderindex.order = psy_min((uintptr_t)((ev->pt.y) /
				psy_ui_value_px(&self->state->lineheight,
					psy_ui_component_textmetric(&self->component), NULL)),
				psy_list_size(self->track->entries) - 1);
			self->state->cmd_orderindex.track = self->trackindex;
			workspace_setsequenceeditposition(self->state->cmds->workspace,
				self->state->cmd_orderindex);
		} else {
			self->state->cmd_orderindex.track = self->trackindex;
			self->state->cmd_orderindex.order = psy_INDEX_INVALID;
			workspace_setsequenceeditposition(self->state->cmds->workspace,
				self->state->cmd_orderindex);
		}
	}
}

void sequencelisttrack_onmousedoubleclick(SequenceListTrack* self,
	psy_ui_MouseEvent* ev)
{	
	if (self->state->cmd_orderindex.track <
			psy_audio_sequence_width(self->state->cmds->sequence)) {		
		self->state->cmd_orderindex.order = (uintptr_t)(ev->pt.y /
			psy_ui_value_px(&self->state->lineheight,
				psy_ui_component_textmetric(&self->component), NULL));
		workspace_setsequenceeditposition(self->state->cmds->workspace,
			self->state->cmd_orderindex);
		sequencecmds_changeplayposition(self->state->cmds);		
	}
}

void sequencelisttrack_onsequenceselectionselect(SequenceListTrack* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	if (index->track == self->trackindex) {
		psy_ui_component_addstylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

void sequencelisttrack_onsequenceselectiondeselect(SequenceListTrack* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	if (index->track == self->trackindex) {
		psy_ui_component_removestylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

// SequenceListView
// prototypes
static void sequencelistview_onpreferredsize(SequenceListView*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void sequencelistview_onmousedown(SequenceListView*,
	psy_ui_MouseEvent*);
static void sequencelistview_ontimer(SequenceListView*, uintptr_t timerid);
static void sequencelistview_onpatternnamechanged(SequenceListView*,
	psy_audio_Patterns*, uintptr_t slot);
static psy_ui_RealRectangle sequencelistview_rowrectangle(SequenceListView*,
	uintptr_t row);
static void sequencelistview_invalidaterow(SequenceListView*, uintptr_t row);
static void sequencelistview_build(SequenceListView*);
static void sequencelistview_changeplayposition(SequenceListView*);
// vtable
static psy_ui_ComponentVtable sequencelistview_vtable;
static psy_ui_ComponentVtable sequencelistview_super_vtable;
static bool sequencelistview_vtable_initialized = FALSE;

static void sequencelistview_vtable_init(SequenceListView* self)
{
	if (!sequencelistview_vtable_initialized) {
		sequencelistview_vtable = *(self->component.vtable);
		sequencelistview_super_vtable = *(self->component.vtable);
		sequencelistview_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			sequencelistview_onmousedown;		
		sequencelistview_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			sequencelistview_ontimer;
		sequencelistview_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			sequencelistview_onpreferredsize;
		sequencelistview_vtable_initialized = TRUE;
	}
	self->component.vtable = &sequencelistview_vtable;
}
// implementation
void sequencelistview_init(SequenceListView* self, psy_ui_Component* parent,
	SequenceListViewState* state, SequenceView* view)
{
	psy_ui_component_init(&self->component, parent, NULL);
	sequencelistview_vtable_init(self);	
	self->state = state;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 1);	
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_margin_zero());
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);	
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	self->lastplayposition = -1.f;
	self->lastplayrow = psy_INDEX_INVALID;
	self->showpatternnames = generalconfig_showingpatternnames(
		psycleconfig_general(workspace_conf(self->state->cmds->workspace)));
	self->refreshcount = 0;
	if (self->state->cmds->sequence && self->state->cmds->sequence->patterns) {
		psy_signal_connect(
			&self->state->cmds->sequence->patterns->signal_namechanged,
			self, sequencelistview_onpatternnamechanged);
	}
	psy_ui_component_setscrollstep(&self->component,
		psy_ui_size_make(self->state->trackwidth, self->state->lineheight));
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	psy_ui_component_setstyletypes(&self->component,
		STYLE_SEQLISTVIEW, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);
	sequencelistview_build(self);
	psy_ui_component_starttimer(&self->component, 0, 200); //STYLE_SEQLISTVIEW	
}

void sequencelistview_build(SequenceListView* self)
{
	psy_audio_SequenceTrackNode* p;	
	uintptr_t trackindex = 0;	
	
	psy_ui_component_clear(&self->component);
	for (p = self->state->cmds->sequence->tracks; p != NULL; p = p->next,
			++trackindex) {
		SequenceListTrack* track;

		track = sequencelisttrack_allocinit(&self->component,
			&self->component,
			trackindex, (psy_audio_SequenceTrack*)psy_list_entry(p),
			self->state);		
	}
	psy_ui_component_align(&self->component);
}

void sequencelistview_onpreferredsize(SequenceListView* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	sequencelistview_super_vtable.onpreferredsize(&self->component, limit, rv);	
	psy_ui_value_add(&rv->width, &self->state->trackwidth,
		psy_ui_component_textmetric(&self->component), NULL);	
}

void sequencelistview_showpatternnames(SequenceListView* self)
{
	self->showpatternnames = TRUE;	
	psy_ui_component_invalidate(&self->component);
}

void sequencelistview_showpatternslots(SequenceListView* self)
{
	self->showpatternnames = FALSE;	
	psy_ui_component_invalidate(&self->component);
}


void sequencelistview_onmousedown(SequenceListView* self,
	psy_ui_MouseEvent* ev)
{
}

void sequencelistview_ontimer(SequenceListView* self, uintptr_t timerid)
{
	if (psy_audio_player_playing(self->state->cmds->player)) {
		if (psy_audio_player_playlist_position(self->state->cmds->player) !=
				self->lastplayrow) {
			// invalidate previous row
			sequencelistview_invalidaterow(self, self->lastplayrow);
			self->lastplayrow = psy_audio_player_playlist_position(
				self->state->cmds->player);
			// next(curr) row is invalidated with row progress bar
		}
		if (self->refreshcount == 2) { // saves cpu not updating at every intervall
									   // todo: better check for player line change
			// invalidate row progress bar
			// takes care, too, that at row change the new row is invalidated
			if (psy_audio_player_playlist_position(self->state->cmds->player)
					!= psy_INDEX_INVALID) {
				sequencelistview_invalidaterow(self,
					psy_audio_player_playlist_position(self->state->cmds->player));
			}
			self->refreshcount = 0;
		}
		++self->refreshcount;
	} else if (self->lastplayrow != psy_INDEX_INVALID) {
		// invalidate if player is stopping to remove the progress bar
		sequencelistview_invalidaterow(self, self->lastplayrow);
		self->lastplayrow = psy_INDEX_INVALID;
	}
}

void sequencelistview_onpatternnamechanged(SequenceListView* self,
	psy_audio_Patterns* patterns, uintptr_t slot)
{
	psy_ui_component_invalidate(&self->component);
}

void sequencelistview_invalidaterow(SequenceListView* self, uintptr_t row)
{
	if (row != psy_INDEX_INVALID) {
		psy_ui_component_invalidaterect(&self->component,
			sequencelistview_rowrectangle(self, row));
	}
}

psy_ui_RealRectangle sequencelistview_rowrectangle(SequenceListView* self,
	uintptr_t row)
{
	psy_ui_RealSize size;
	double lineheightpx;
	
	size = psy_ui_component_size_px(&self->component);
	lineheightpx = psy_ui_value_px(&self->state->lineheight,
		psy_ui_component_textmetric(&self->component), NULL);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0.0, lineheightpx * row),		
		psy_ui_realsize_make(size.width, lineheightpx));
}

// SequenceViewDuration
// prototypes
static void sequenceduration_ontimer(SequenceViewDuration* self,
	psy_ui_Component* sender, uintptr_t id);
static void sequenceduration_ondestroy(SequenceViewDuration* self,
	psy_ui_Component* sender);	
// implementation
void sequenceduration_init(SequenceViewDuration* self,
	psy_ui_Component* parent, Workspace* workspace)
{	
	self->workspace = workspace;
	self->duration_ms = 0;
	self->duration_bts = 0.0;	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setspacing(&self->component,
		psy_ui_margin_make_em(0.5, 0.0, 0.5, 0.5));
	psy_ui_label_init_text(&self->desc, &self->component, NULL,
		"seqview.duration");
	psy_ui_component_setspacing(&self->desc.component,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));	
	psy_ui_component_setalign(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->duration, &self->component, NULL);	
	psy_ui_component_setalign(&self->duration.component, psy_ui_ALIGN_LEFT);	
	psy_ui_label_setcharnumber(&self->duration, 18.0);
	psy_ui_label_preventtranslation(&self->duration);
	psy_ui_component_setstyletypes(psy_ui_label_base(&self->duration),
		STYLE_DURATION_TIME, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);
	self->calcduration = FALSE;
	psy_signal_connect(&self->component.signal_timer, self,
		sequenceduration_ontimer);	
	psy_signal_connect(&self->component.signal_destroy, self,
		sequenceduration_ontimer);
	psy_ui_component_starttimer(&self->component, 0, 50);
	sequenceduration_update(self, FALSE);
}

void sequenceduration_ondestroy(SequenceViewDuration* self, psy_ui_Component* sender)
{	
	psy_ui_component_stoptimer(&self->component, 0);
	sequenceduration_stopdurationcalc(self);
}

void sequenceduration_stopdurationcalc(SequenceViewDuration* self)
{
	if (workspace_song(self->workspace) && self->calcduration) {
		psy_audio_sequence_endcalcdurationinmsresult(
			&workspace_song(self->workspace)->sequence);
		self->calcduration = FALSE;
	}
}

void sequenceduration_update(SequenceViewDuration* self, bool force)
{		
	if (workspace_song(self->workspace)) {
		psy_dsp_big_beat_t duration_bts;

		duration_bts = psy_audio_sequence_duration(
			&workspace_song(self->workspace)->sequence);
		if (self->duration_bts != duration_bts || force) {
			char text[64];

			self->duration_bts = duration_bts;
			if (self->calcduration) {
				self->duration_ms =
					psy_audio_sequence_endcalcdurationinmsresult(
						&workspace_song(self->workspace)->sequence);
				self->calcduration = FALSE;
			}
			if (!self->calcduration) {
				self->calcduration = TRUE;
				psy_audio_sequence_startcalcdurationinms(
					&workspace_song(self->workspace)->sequence);
				sequenceduration_ontimer(self, &self->component, 0);
			}
			psy_snprintf(text, 64, "--m--s %.2fb", (float)self->duration_bts);
			psy_ui_label_settext(&self->duration, text);
		}
	}
}

void sequenceduration_ontimer(SequenceViewDuration* self, psy_ui_Component* sender,
	uintptr_t id)
{
	if (self->calcduration && workspace_song(self->workspace)) {
		uintptr_t i;

		for (i = 0; i < 20; ++i) {
			if (!psy_audio_sequence_calcdurationinms(
					&workspace_song(self->workspace)->sequence)) {
				char text[64];

				self->duration_ms = psy_audio_sequence_endcalcdurationinmsresult(
					&workspace_song(self->workspace)->sequence);
				psy_snprintf(text, 64, " %02dm%02ds %.2fb",
					(int)(self->duration_ms / 60), ((int)self->duration_ms % 60),
					(float)self->duration_bts);
				psy_ui_label_settext(&self->duration, text);
				self->calcduration = FALSE;
				break;
			}
		}
	}
}

// SequenceView
// prototypes
static void sequenceview_onsongchanged(SequenceView*, Workspace*, int flag,
	psy_audio_Song* song);
static void sequenceview_onsequenceselectionchanged(SequenceView*,
	psy_audio_SequenceSelection*);
static void sequenceview_onsequencetrackreposition(SequenceView*,
	psy_audio_Sequence* sender, uintptr_t trackidx);
static void sequenceview_onsequencechanged(SequenceView*,
	psy_audio_Sequence* sender);
static void sequenceview_onconfigure(SequenceView*, GeneralConfig*,
	psy_Property*);
static void sequenceview_onscroll(SequenceView*, psy_ui_Component* sender);
// implementation
void sequenceview_init(SequenceView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin spacing;
		
	psy_ui_component_init(&self->component, parent, NULL);	
	sequencecmds_init(&self->cmds, workspace);
	// shared state
	sequencelistviewstate_init(&self->state, &self->cmds);			
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);	
	// sequence listview
	sequencelistview_init(&self->listview, &self->component,
		&self->state, self);	
	psy_ui_scroller_init(&self->scroller, &self->listview.component,
		&self->component, NULL);	
	psy_ui_margin_init_em(&spacing, 0.5, 0.5, 0.0, 1.0);
	psy_ui_component_setspacing(&self->scroller.component, spacing);
	psy_ui_scroller_setbackgroundmode(&self->scroller,
		psy_ui_SETBACKGROUND, psy_ui_SETBACKGROUND);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->listview.component,
		psy_ui_ALIGN_FIXED_RESIZE);
	// button bar
	sequencebuttons_init(&self->buttons, &self->component, &self->cmds);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);
	// spacer
	psy_ui_component_init_align(&self->spacer, &self->component, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_setpreferredsize(&self->spacer,
		psy_ui_size_make_em(0.0, 0.3));
	psy_ui_component_preventalign(&self->spacer);	
	// header
	sequencetrackheaders_init(&self->trackheader, &self->component,
		&self->state);
	psy_ui_component_setalign(&self->trackheader.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->listview.component.signal_scroll, self,
		sequenceview_onscroll);	
	// duration
	sequenceduration_init(&self->duration, &self->component, workspace);
	psy_ui_component_setalign(&self->duration.component, psy_ui_ALIGN_BOTTOM);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		sequenceview_onsongchanged);
	psy_signal_connect(&workspace->sequenceselection.signal_changed, self,
		sequenceview_onsequenceselectionchanged);
	if (self->cmds.sequence && self->cmds.sequence->patterns) {
		psy_signal_connect(&self->cmds.sequence->patterns->signal_namechanged,
			&self->listview,
			sequencelistview_onpatternnamechanged);
		psy_signal_connect(&self->cmds.sequence->signal_changed,
			self, sequenceview_onsequencechanged);
		psy_signal_connect(&self->cmds.sequence->signal_trackreposition,
			self, sequenceview_onsequencetrackreposition);
	}
	psy_signal_connect(
		&psycleconfig_general(workspace_conf(workspace))->signal_changed,
		self, sequenceview_onconfigure);	
}

void sequenceview_onscroll(SequenceView* self, psy_ui_Component* sender)
{
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(&self->listview.component);
	psy_ui_component_move(&self->trackheader.client,
		psy_ui_point_make_px(position.left, 0.0));
}

void sequenceview_clear(SequenceView* self)
{
	sequencecmds_clear(&self->cmds);	
}

void sequenceview_onsongchanged(SequenceView* self, Workspace* sender,
	int flag, psy_audio_Song* song)
{
	sequencecmds_update(&self->cmds);	
	if (workspace_song(sender)) {		
		if (self->cmds.sequence && self->cmds.patterns) {
			psy_signal_connect(
				&self->cmds.sequence->patterns->signal_namechanged,
				&self->listview, sequencelistview_onpatternnamechanged);
			psy_signal_connect(&self->cmds.sequence->signal_changed,
				self, sequenceview_onsequencechanged);
			psy_signal_connect(&self->cmds.sequence->signal_trackreposition,
				self, sequenceview_onsequencetrackreposition);
		}
	}
	sequenceduration_stopdurationcalc(&self->duration);
	sequencetrackheaders_build(&self->trackheader);
	sequencelistview_build(&self->listview);
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->listview.component);
	sequenceduration_update(&self->duration, TRUE);
}

void sequenceview_onsequenceselectionchanged(SequenceView* self,
	psy_audio_SequenceSelection* sender)
{		
	/*uintptr_t c = 0;
	double visilines;
	double listviewtop;
	psy_ui_RealSize listviewsize;
	
	c = sender->editposition.order;
	if (c == psy_INDEX_INVALID) {
		c = 0;
	}
	listviewsize = psy_ui_component_scrollsize_px(&self->listview.component);
	visilines = (listviewsize.height) /
		psy_ui_value_px(&self->state.lineheight,
			psy_ui_component_textmetric(&self->component));
	listviewtop = psy_ui_component_scrolltop_px(&self->listview.component) /
		psy_ui_value_px(&self->state.lineheight,
			psy_ui_component_textmetric(&self->component));
	if ((double)c < listviewtop) {
		psy_ui_component_setscrolltop(&self->listview.component,
			psy_ui_value_make_px(c *
				psy_ui_value_px(&self->state.lineheight,
					psy_ui_component_textmetric(&self->component))));				
	} else if ((double)c > listviewtop + visilines - 1) {
		psy_ui_component_setscrolltop(&self->listview.component,
			psy_ui_value_make_px((c - visilines + 1) * 
				psy_ui_value_px(&self->state.lineheight,
					psy_ui_component_textmetric(&self->component))));
	}
	*/
	psy_ui_component_invalidate(&self->listview.component);
	psy_ui_component_invalidate(&self->trackheader.component);
}

void sequenceview_onsequencechanged(SequenceView* self,
	psy_audio_Sequence* sender)
{		
	sequenceduration_stopdurationcalc(&self->duration);
	sequencetrackheaders_build(&self->trackheader);
	sequencelistview_build(&self->listview);
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->listview.component);
	psy_ui_component_align(&self->trackheader.component);
	sequenceduration_update(&self->duration, TRUE);
}

void sequenceview_onsequencetrackreposition(SequenceView* self,
	psy_audio_Sequence* sender, uintptr_t trackidx)
{
	sequenceduration_update(&self->duration, FALSE);	
	psy_ui_component_invalidate(&self->listview.component);
}

void sequenceview_onconfigure(SequenceView* self, GeneralConfig* config,
	psy_Property* property)
{
	if (generalconfig_showingpatternnames(config)) {
		self->state.showpatternnames = TRUE;
		self->state.trackwidth = psy_ui_value_make_ew(24.0);
		psy_ui_value_setroundmode(&self->state.trackwidth, psy_ui_ROUND_FLOOR);
		sequencetrackheaders_build(&self->trackheader);
		psy_ui_component_align(&self->trackheader.component);
		psy_ui_component_align(&self->listview.component);
	} else {
		self->state.showpatternnames = FALSE;
		self->state.trackwidth = psy_ui_value_make_ew(16.0);
		psy_ui_value_setroundmode(&self->state.trackwidth, psy_ui_ROUND_FLOOR);
		sequencetrackheaders_build(&self->trackheader);
		psy_ui_component_align(&self->trackheader.component);
		psy_ui_component_align(&self->listview.component);
	}	
	psy_ui_component_invalidate(&self->listview.component);	
}

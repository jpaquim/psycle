/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sequenceview.h"
/* host */
#include "trackercmds.h"
#include "styles.h"
#include "workspace.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#define COLMAX 13

/* SeqViewState */
void seqviewstate_init(SeqViewState* self, SequenceCmds* cmds)
{	
	assert(self);
	assert(cmds);

	self->cmds = cmds;
	self->trackwidth = psy_ui_value_make_ew(16.0);	
	self->lineheight = psy_ui_value_make_eh(1.2);	
	self->colwidth = 10.0;
	self->cmd = SEQLVCMD_NONE;
	self->cmd_orderindex = psy_audio_orderindex_zero();	
	self->col = 0;
	self->active = FALSE;
	self->showpatternnames = FALSE;
}

/* SeqviewTrackHeaders */
/* prototypes */
static void seqviewtrackheaders_ondestroy(SeqviewTrackHeaders*);
static void seqviewtrackheaders_onmouseup(SeqviewTrackHeaders*,
	psy_ui_MouseEvent*);
static void seqviewtrackheaders_onnewtrack(SeqviewTrackHeaders*,
	psy_ui_Button* sender);
static void seqviewtrackheaders_ondeltrack(SeqviewTrackHeaders*,
	TrackBox* sender);
/* vtable */
static psy_ui_ComponentVtable trackheaderviews_vtable;
static bool trackheaderviews_vtable_initialized = FALSE;

static void trackheaderview_vtable_init(SeqviewTrackHeaders* self)
{
	if (!trackheaderviews_vtable_initialized) {
		trackheaderviews_vtable = *(self->component.vtable);
		trackheaderviews_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqviewtrackheaders_ondestroy;		
		trackheaderviews_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			seqviewtrackheaders_onmouseup;
		trackheaderviews_vtable_initialized = TRUE;
	}
}
/* implemenetation */
void seqviewtrackheaders_init(SeqviewTrackHeaders* self,
	psy_ui_Component* parent, SeqViewState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	trackheaderview_vtable_init(self);
	self->component.vtable = &trackheaderviews_vtable;
	self->state = state;	
	psy_ui_component_setminimumsize(&self->component,
		psy_ui_size_make_em(0.0, 2.0));	
	psy_signal_init(&self->signal_trackselected);
	psy_ui_component_init(&self->client, &self->component, NULL);
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_FIXED_RESIZE);
	psy_ui_component_setdefaultalign(&self->client,
		psy_ui_ALIGN_LEFT, psy_ui_margin_zero());
	psy_ui_component_setalignexpand(&self->client, psy_ui_HEXPAND);
	psy_ui_component_setscrollstep(&self->client,
		psy_ui_size_make(self->state->trackwidth, self->state->lineheight));
	psy_ui_component_setoverflow(&self->client, psy_ui_OVERFLOW_HSCROLL);
	seqviewtrackheaders_build(self);
}

void seqviewtrackheaders_ondestroy(SeqviewTrackHeaders* self)
{	
	psy_signal_dispose(&self->signal_trackselected);
}

void seqviewtrackheaders_build(SeqviewTrackHeaders* self)
{
	psy_ui_component_clear(&self->client);		
	if (self->state->cmds->sequence) {
		psy_audio_SequenceTrackNode* t;
		uintptr_t c;
		psy_ui_Button* newtrack;

		for (t = self->state->cmds->sequence->tracks, c = 0; t != NULL;
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
				psy_signal_connect(&sequencetrackbox->trackbox.signal_close,
					self, seqviewtrackheaders_ondeltrack);
			}
		}
		newtrack = psy_ui_button_allocinit(&self->client, &self->component);
		if (newtrack) {
			psy_ui_button_settext(newtrack, "seqview.new-trk");
			newtrack->stoppropagation = FALSE;
			psy_signal_connect(&newtrack->signal_clicked, self,
				seqviewtrackheaders_onnewtrack);
		}
	}
	psy_ui_component_align(&self->client);
	psy_ui_component_align(&self->component);	
}

void seqviewtrackheaders_onmouseup(SeqviewTrackHeaders* self,
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

void seqviewtrackheaders_onnewtrack(SeqviewTrackHeaders* self,
	psy_ui_Button* sender)
{
	self->state->cmd = SEQLVCMD_NEWTRACK;
}

void seqviewtrackheaders_ondeltrack(SeqviewTrackHeaders* self,
	TrackBox* sender)
{
	self->state->cmd = SEQLVCMD_DELTRACK;
	self->state->cmd_orderindex.track = trackbox_trackindex(sender);
}

/* SeqViewTrack */
/* prototypes */
static void seqviewtrack_ondestroy(SeqViewTrack*);
static void seqviewtrack_onpreferredsize(SeqViewTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqviewtrack_ondraw(SeqViewTrack*, psy_ui_Graphics*);
static void seqviewtrack_drawentry(SeqViewTrack*, psy_ui_Graphics*,
	psy_audio_SequenceEntry*, uintptr_t row, psy_ui_RealPoint, psy_ui_Colour bg,
	bool drawcol);
static void seqviewtrack_textout_digit(SeqViewTrack*,
	psy_ui_Graphics*, const char* str, psy_ui_RealPoint,
	uintptr_t cursorcol);
static void seqviewtrack_drawprogressbar(SeqViewTrack*,
	psy_ui_Graphics*, psy_ui_RealPoint, psy_audio_SequenceEntry*);
void seqviewtrack_onmousedown(SeqViewTrack*, psy_ui_MouseEvent*);
void seqviewtrack_onmousedoubleclick(SeqViewTrack*, psy_ui_MouseEvent*);
static void seqviewtrack_onsequenceselect(SeqViewTrack*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqviewtrack_onsequencedeselect(SeqViewTrack*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
/* vtable */
static psy_ui_ComponentVtable seqviewtrack_vtable;
static bool seqviewtrack_vtable_initialized = FALSE;

static void seqviewtrack_vtable_init(SeqViewTrack* self)
{
	if (!seqviewtrack_vtable_initialized) {
		seqviewtrack_vtable = *(self->component.vtable);
		seqviewtrack_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqviewtrack_ondestroy;
		seqviewtrack_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqviewtrack_ondraw;
		seqviewtrack_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqviewtrack_onpreferredsize;
		seqviewtrack_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqviewtrack_onmousedown;
		seqviewtrack_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqviewtrack_onmousedoubleclick;		
		seqviewtrack_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqviewtrack_vtable;
}

void seqviewtrack_init(SeqViewTrack* self, psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t trackindex,
	psy_audio_SequenceTrack* track, SeqViewState* state)
{
	psy_ui_component_init(&self->component, parent, view);
	seqviewtrack_vtable_init(self);	
	psy_ui_component_setstyletype(&self->component,
		STYLE_SEQLISTVIEW_TRACK);
	psy_ui_component_setstyletype_select(&self->component,		
		STYLE_SEQLISTVIEW_TRACK_SELECT);	
	self->state = state;
	self->trackindex = trackindex;
	self->track = track;
	psy_signal_connect(
		&state->cmds->workspace->sequenceselection.signal_select,
		self, seqviewtrack_onsequenceselect);
	psy_signal_connect(
		&state->cmds->workspace->sequenceselection.signal_deselect,
		self, seqviewtrack_onsequencedeselect);
	if (state->cmds->workspace->sequenceselection.editposition.track ==
			trackindex) {
		psy_ui_component_addstylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

SeqViewTrack* seqviewtrack_alloc(void)
{
	return (SeqViewTrack*)malloc(sizeof(SeqViewTrack));
}

SeqViewTrack* seqviewtrack_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	uintptr_t trackindex, psy_audio_SequenceTrack* track,
	SeqViewState* state)
{
	SeqViewTrack* rv;

	rv = seqviewtrack_alloc();
	if (rv) {
		seqviewtrack_init(rv, parent, view, trackindex, track, state);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

void seqviewtrack_ondestroy(SeqViewTrack* self)
{
	psy_signal_disconnect(
		&self->state->cmds->workspace->sequenceselection.signal_select,
		self, seqviewtrack_onsequenceselect);
	psy_signal_disconnect(
		&self->state->cmds->workspace->sequenceselection.signal_deselect,
		self, seqviewtrack_onsequencedeselect);
}

void seqviewtrack_ondraw(SeqViewTrack* self, psy_ui_Graphics* g)
{
	psy_List* p;
	uintptr_t c;
	psy_ui_RealPoint cp;	
	uintptr_t startrow;
	uintptr_t endrow;		
	double lineheightpx;	
	const psy_ui_TextMetric* tm;
	psy_audio_OrderIndex first;
		
	tm = psy_ui_component_textmetric(&self->component);
	lineheightpx = psy_max(1.0, floor(psy_ui_value_px(&self->state->lineheight,
		tm, NULL)));
	self->state->colwidth = floor(tm->tmAveCharWidth * 1.4);	
	self->state->digitsize = psy_ui_realsize_make(self->state->colwidth,
		lineheightpx);
	startrow = (uintptr_t)floor(psy_max(0, (g->clip.top / lineheightpx)));
	endrow = (uintptr_t)(floor(g->clip.bottom / lineheightpx + 0.5));		
	psy_ui_settextcolour(g, psy_ui_colour_make(0));
	psy_ui_realpoint_init_all(&cp, 0.0, lineheightpx * startrow);
	p = psy_list_at(p = self->track->entries, startrow);	
	first = psy_audio_sequenceselection_first(
		&self->state->cmds->workspace->sequenceselection);
	for (c = startrow; p != NULL; psy_list_next(&p), ++c, cp.y += lineheightpx) {
		psy_audio_SequenceEntry* seqentry;
		bool rowplaying = FALSE;
		psy_ui_Colour bg;
		
		seqentry = (psy_audio_SequenceEntry*)p->entry;
		rowplaying = psy_audio_player_playing(self->state->cmds->player) &&
			psy_audio_player_playlist_position(self->state->cmds->player) == c;
		psy_ui_colour_init(&bg);
		if (rowplaying) {
			seqviewtrack_drawprogressbar(self, g, cp, seqentry);
		}
		if (psy_audio_sequenceselection_isselected(
				&self->state->cmds->workspace->sequenceselection,
				psy_audio_orderindex_make(self->trackindex, c))) {
			if (!rowplaying) {
				bg = psy_ui_colour_make(0x009B7800);
			} else {
				psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
			}
			if (psy_ui_componentstyle_style(&self->component.style, psy_ui_STYLESTATE_SELECT)) {
				psy_ui_settextcolour(g, psy_ui_componentstyle_style(
					&self->component.style, psy_ui_STYLESTATE_SELECT)->colour);
			}			
		} else if (rowplaying) {
			psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x00232323));
			psy_ui_settextcolour(g, psy_ui_style_const(STYLE_SEQ_PROGRESS)->colour);
		} else {
			psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x00232323));
			psy_ui_settextcolour(g, psy_ui_style_const(STYLE_SEQLISTVIEW)->colour);
		}		
		seqviewtrack_drawentry(self, g, seqentry, c, cp, bg,
			self->state->active && first.order == c &&
			first.track == self->trackindex);
	}
}

void seqviewtrack_drawentry(SeqViewTrack* self, psy_ui_Graphics* g,
	psy_audio_SequenceEntry* entry, uintptr_t row, psy_ui_RealPoint cp,
	psy_ui_Colour bg, bool drawcol)
{
	char text[256];		
	
	assert(entry);

	switch (entry->type) {
	case psy_audio_SEQUENCEENTRY_PATTERN: {
		psy_audio_Pattern* pattern;
		psy_audio_SequencePatternEntry* seqpatternentry;

		seqpatternentry = (psy_audio_SequencePatternEntry*)entry;
		pattern = psy_audio_sequencepatternentry_pattern(
			seqpatternentry,
			self->state->cmds->patterns);
		if (self->state->showpatternnames) {
			if (pattern) {
				psy_snprintf(text, 20, "%02X: %s %06.2f", row,
					psy_audio_pattern_name(pattern),
					(float)psy_audio_sequenceentry_offset(entry));
			} else {
				psy_snprintf(text, 20, "%02X:%02X(ERR) %06.2f", (int)row,
					(int)psy_audio_sequencepatternentry_patternslot(
						seqpatternentry),
					(float)psy_audio_sequenceentry_offset(entry));
			}
		} else {
			if (pattern) {
				psy_snprintf(text, 20, "%02X:%02X  %06.2f", row,
					(int)psy_audio_sequencepatternentry_patternslot(
						seqpatternentry),
					(float)psy_audio_sequenceentry_offset(entry));
			} else {
				psy_snprintf(text, 20, "%02X:%02XE %06.2f", row,
					(int)psy_audio_sequencepatternentry_patternslot(
						seqpatternentry),
					(float)psy_audio_sequenceentry_offset(entry));
			}
		}
		break; }
	case psy_audio_SEQUENCEENTRY_SAMPLE: {
		psy_audio_SequenceSampleEntry* seqsampleentry;

		seqsampleentry = (psy_audio_SequenceSampleEntry*)entry;
		psy_snprintf(text, 64, "%02X:%02X:%02X", row,
			(int)psy_audio_sequencesampleentry_samplesindex(seqsampleentry).slot,
			(int)psy_audio_sequencesampleentry_samplesindex(seqsampleentry).subslot);
		break; }
	case psy_audio_SEQUENCEENTRY_MARKER: {
		psy_audio_SequenceMarkerEntry* seqmarkerentry;

		seqmarkerentry = (psy_audio_SequenceMarkerEntry*)entry;
		if (seqmarkerentry->text) {
			psy_snprintf(text, 20, "%02X:%s", row,
				seqmarkerentry->text,
				(float)psy_audio_sequenceentry_offset(entry));
		} else {
			text[0] = '\0';
		}
		break; }
	default:
		text[0] = '\0';
		break;
	}

	if (bg.mode.set && !drawcol) {
		psy_ui_drawsolidrectangle(g,
			psy_ui_realrectangle_make(
			cp,
			psy_ui_realsize_make(
				psy_ui_value_px(&self->state->trackwidth,
					psy_ui_component_textmetric(&self->component), NULL),
				psy_ui_value_px(&self->state->lineheight,
					psy_ui_component_textmetric(&self->component), NULL))),
			bg);
	}
	seqviewtrack_textout_digit(self, g, text, cp, 
		(drawcol) ? self->state->col : psy_INDEX_INVALID);
}

void seqviewtrack_textout_digit(SeqViewTrack* self,
	psy_ui_Graphics* g, const char* str, psy_ui_RealPoint pt,
	uintptr_t cursorcol)
{
	uintptr_t numchars;
	uintptr_t digit;	
	psy_ui_RealPoint cp;
	
	numchars = psy_strlen(str);
	cp = pt;		
	for (digit = 0; digit < numchars; ++digit) {
		char digitstr[2];

		digitstr[0] = str[digit];
		digitstr[1] = '\n';
		if (digit == cursorcol) {			

			psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x009B7800));
			psy_ui_textoutrectangle(g,				
				cp, psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
				psy_ui_realrectangle_make(cp, self->state->digitsize),
				digitstr, 1);			
		} else {
			psy_ui_textout(g, cp.x, cp.y, digitstr, 1);
		}
		cp.x += self->state->colwidth;
	}
}

void seqviewtrack_drawprogressbar(SeqViewTrack* self, psy_ui_Graphics* g,
	psy_ui_RealPoint pt, psy_audio_SequenceEntry* sequenceentry)
{
	psy_ui_RealRectangle r;
	psy_ui_RealSize size;

	size = psy_ui_component_size_px(&self->component);
	r = psy_ui_realrectangle_make(pt, psy_ui_realsize_make(
		psy_audio_player_playlist_rowprogress(self->state->cmds->player)
			* size.width,
		psy_ui_value_px(&self->state->lineheight,
			psy_ui_component_textmetric(&self->component), NULL)));
	psy_ui_drawsolidrectangle(g, r,
		psy_ui_style_const(STYLE_SEQ_PROGRESS)->backgroundcolour);
}

void seqviewtrack_onpreferredsize(SeqViewTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{	
	rv->width = self->state->trackwidth;
	if (self->track) {
		const psy_ui_TextMetric* tm;

		tm = psy_ui_component_textmetric(&self->component);
		rv->height = psy_ui_value_make_px(
			psy_ui_value_px(&self->state->lineheight, tm, NULL) *
				(double)psy_list_size(self->track->entries));		
	} else {
		rv->height = psy_ui_value_zero();		
	}			
}

void seqviewtrack_onmousedown(SeqViewTrack* self, psy_ui_MouseEvent* ev)
{
	if (self->track) {		
		if (self->track->entries) {
			self->state->cmd_orderindex.order = psy_min((uintptr_t)((ev->pt.y) /
				psy_ui_value_px(&self->state->lineheight,
					psy_ui_component_textmetric(&self->component), NULL)),
				psy_list_size(self->track->entries) - 1);
			self->state->cmd_orderindex.track = self->trackindex;
			if (self->state->active) {
				self->state->col = (uintptr_t)(ev->pt.x / self->state->colwidth);
				self->state->col = psy_min(self->state->col, COLMAX - 1);
			}
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

void seqviewtrack_onmousedoubleclick(SeqViewTrack* self, psy_ui_MouseEvent* ev)
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

void seqviewtrack_onsequenceselect(SeqViewTrack* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	if (index->track == self->trackindex) {
		psy_ui_component_addstylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

void seqviewtrack_onsequencedeselect(SeqViewTrack* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	if (index->track == self->trackindex) {
		psy_ui_component_removestylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

/* SeqviewList */
/* prototypes */
static void seqviewlist_onpreferredsize(SeqviewList*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqviewlist_ontimer(SeqviewList*, uintptr_t timerid);
static void seqviewlist_onpatternnamechanged(SeqviewList*,
	psy_audio_Patterns*, uintptr_t slot);
static psy_ui_RealRectangle seqviewlist_rowrectangle(SeqviewList*,
	uintptr_t row);
static void seqviewlist_invalidaterow(SeqviewList*, uintptr_t row);
static void seqviewlist_build(SeqviewList*);
static void seqviewlist_changeplayposition(SeqviewList*);
static bool seqviewlist_oninput(SeqviewList*, InputHandler*);
static void seqviewlist_onfocus(SeqviewList*);
static void seqviewlist_onfocuslost(SeqviewList*);
static void seqviewlist_inputdigit(SeqviewList*, uint8_t value);
/* vtable */
static psy_ui_ComponentVtable seqviewlist_vtable;
static psy_ui_ComponentVtable seqviewlist_super_vtable;
static bool seqviewlist_vtable_initialized = FALSE;

static void seqviewlist_vtable_init(SeqviewList* self)
{
	if (!seqviewlist_vtable_initialized) {
		seqviewlist_vtable = *(self->component.vtable);
		seqviewlist_super_vtable = *(self->component.vtable);
		seqviewlist_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			seqviewlist_ontimer;
		seqviewlist_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqviewlist_onpreferredsize;		
		seqviewlist_vtable.onfocus =
			(psy_ui_fp_component_onfocus)
			seqviewlist_onfocus;
		seqviewlist_vtable.onfocuslost =
			(psy_ui_fp_component_onfocuslost)
			seqviewlist_onfocuslost;
		seqviewlist_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqviewlist_vtable;
}
/* implementation */
void seqviewlist_init(SeqviewList* self, psy_ui_Component* parent,
	SeqViewState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	seqviewlist_vtable_init(self);	
	self->state = state;
	self->lastplayposition = -1.f;
	self->lastplayrow = psy_INDEX_INVALID;
	self->refreshcount = 0;
	self->showpatternnames = generalconfig_showingpatternnames(
		psycleconfig_general(workspace_conf(self->state->cmds->workspace)));	
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 1);	
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_zero());
	psy_ui_component_setalignexpand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	if (self->state->cmds->sequence && self->state->cmds->sequence->patterns) {
		psy_signal_connect(
			&self->state->cmds->sequence->patterns->signal_namechanged,
			self, seqviewlist_onpatternnamechanged);
	}
	psy_ui_component_setscrollstep(&self->component,
		psy_ui_size_make(self->state->trackwidth, self->state->lineheight));
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	psy_ui_component_setstyletype(&self->component, STYLE_SEQLISTVIEW);	
	seqviewlist_build(self);
	inputhandler_connect(&self->state->cmds->workspace->inputhandler,
		INPUTHANDLER_FOCUS, psy_EVENTDRIVER_CMD, "tracker",
		psy_INDEX_INVALID, self,
		(fp_inputhandler_input)
		seqviewlist_oninput);
	psy_ui_component_starttimer(&self->component, 0, 200);
}

void seqviewlist_build(SeqviewList* self)
{
	psy_audio_SequenceTrackNode* p;	
	uintptr_t trackindex = 0;	
	
	psy_ui_component_clear(&self->component);
	for (p = self->state->cmds->sequence->tracks; p != NULL; p = p->next,
			++trackindex) {
		SeqViewTrack* track;

		track = seqviewtrack_allocinit(&self->component,
			&self->component,
			trackindex, (psy_audio_SequenceTrack*)psy_list_entry(p),
			self->state);		
	}
	psy_ui_component_align(&self->component);
}

void seqviewlist_onpreferredsize(SeqviewList* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	seqviewlist_super_vtable.onpreferredsize(&self->component, limit, rv);	
	psy_ui_value_add(&rv->width, &self->state->trackwidth,
		psy_ui_component_textmetric(&self->component), NULL);	
}

void seqviewlist_showpatternnames(SeqviewList* self)
{
	self->showpatternnames = TRUE;	
	psy_ui_component_invalidate(&self->component);
}

void seqviewlist_showpatternslots(SeqviewList* self)
{
	self->showpatternnames = FALSE;	
	psy_ui_component_invalidate(&self->component);
}

void seqviewlist_ontimer(SeqviewList* self, uintptr_t timerid)
{
	if (psy_audio_player_playing(self->state->cmds->player)) {
		if (psy_audio_player_playlist_position(self->state->cmds->player) !=
				self->lastplayrow) {
			/* invalidate previous row */
			seqviewlist_invalidaterow(self, self->lastplayrow);
			self->lastplayrow = psy_audio_player_playlist_position(
				self->state->cmds->player);
			/* next(curr) row is invalidated with row progress bar */
		}
		if (self->refreshcount == 2) { /* todo: check for player line change */
			/* invalidate row progress bar and at row change the new row */			
			if (psy_audio_player_playlist_position(self->state->cmds->player)
					!= psy_INDEX_INVALID) {
				seqviewlist_invalidaterow(self,
					psy_audio_player_playlist_position(self->state->cmds->player));
			}
			self->refreshcount = 0;
		}
		++self->refreshcount;
	} else if (self->lastplayrow != psy_INDEX_INVALID) {
		/* if player stops invalidate to remove the progress bar */
		seqviewlist_invalidaterow(self, self->lastplayrow);
		self->lastplayrow = psy_INDEX_INVALID;
	}
}

void seqviewlist_onpatternnamechanged(SeqviewList* self,
	psy_audio_Patterns* patterns, uintptr_t slot)
{
	psy_ui_component_invalidate(&self->component);
}

void seqviewlist_invalidaterow(SeqviewList* self, uintptr_t row)
{
	if (row != psy_INDEX_INVALID) {
		psy_ui_component_invalidaterect(&self->component,
			seqviewlist_rowrectangle(self, row));
	}
}

bool seqviewlist_oninput(SeqviewList* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	cmd = inputhandler_cmd(sender);
	if (cmd.id != -1) {
		psy_audio_OrderIndex first;

		first = psy_audio_sequenceselection_first(
			&self->state->cmds->workspace->sequenceselection);
		switch (cmd.id) {
		case CMD_NAVLEFT:
			if (self->state->col > 0) {
				--self->state->col;
				seqviewlist_invalidaterow(self, first.order);
			} else {
				self->state->col = COLMAX - 1;
				workspace_songposdec(self->state->cmds->workspace);
			}
			break;
		case CMD_NAVRIGHT:
			++self->state->col;
			if (self->state->col == COLMAX) {
				self->state->col = 0;
				workspace_songposinc(self->state->cmds->workspace);
			}
			seqviewlist_invalidaterow(self, first.order);
			break;
		case CMD_NAVDOWN:
			workspace_songposinc(self->state->cmds->workspace);
			break;
		case CMD_NAVUP:
			workspace_songposdec(self->state->cmds->workspace);
			break;
		case CMD_COLUMNPREV:
			if (first.track > 0) {
				uintptr_t rows;
					
				--first.track;		
				sequencecmds_update(self->state->cmds);
				rows = psy_audio_sequence_track_size(self->state->cmds->sequence, first.track);
				if (rows < first.order) {
					if (rows > 0) {
						first.order = rows - 1;
					} else {
						first.order = 0;
					}
				}
				workspace_setsequenceeditposition(self->state->cmds->workspace,
					first);
			}
			break;
		case CMD_COLUMNNEXT:				
			sequencecmds_update(self->state->cmds);
			if (first.track + 1 < psy_audio_sequence_width(self->state->cmds->sequence)) {
				uintptr_t rows;

				++first.track;
				rows = psy_audio_sequence_track_size(self->state->cmds->sequence, first.track);
				if (rows < first.order) {
					if (rows > 0) {
						first.order = rows - 1;
					} else {
						first.order = 0;
					}
				}
				workspace_setsequenceeditposition(self->state->cmds->workspace,
					first);
			}
			break;
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
		case CMD_DIGITF: {
			int digit = (int)cmd.id - CMD_DIGIT0;
			if (digit != -1) {
				seqviewlist_inputdigit(self, digit);
			}
			break; }
		default:
			break;
		}
	}
	return cmd.id != -1;
}

void seqviewlist_inputdigit(SeqviewList* self, uint8_t digit)
{
	psy_audio_SequenceEntry* entry;
	psy_audio_OrderIndex first;

	first = psy_audio_sequenceselection_first(
		&self->state->cmds->workspace->sequenceselection);
	sequencecmds_update(self->state->cmds);
	entry = psy_audio_sequence_entry(self->state->cmds->sequence, first);
	if (!entry) {
		return;
	}
	if (entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
		psy_audio_SequencePatternEntry* seqpatternentry;

		seqpatternentry = (psy_audio_SequencePatternEntry*)entry;
		/* pattern index */
		if (self->state->col == 3) {
			uintptr_t patidx;
			uint8_t curr;

			curr = seqpatternentry->patternslot & 0xF;
			patidx = seqpatternentry->patternslot & ~(0xFF);
			patidx = patidx | curr | (digit << 4);
			sequencecmds_changepattern(self->state->cmds,
				(int)patidx - (int)seqpatternentry->patternslot);
		} else if (self->state->col == 4) {
			uintptr_t patidx;
			uint8_t curr;

			curr = seqpatternentry->patternslot & 0xF0;
			patidx = seqpatternentry->patternslot & ~(0xFF);
			patidx = patidx | curr | (digit);
			sequencecmds_changepattern(self->state->cmds,
				(int)patidx - (int)seqpatternentry->patternslot);
		}
	}
}

void seqviewlist_onfocus(SeqviewList* self)
{
	psy_audio_OrderIndex first;
	
	seqviewlist_super_vtable.onfocus(&self->component);
	first = psy_audio_sequenceselection_first(
		&self->state->cmds->workspace->sequenceselection);
	self->state->active = TRUE;
	psy_ui_component_setborder(
		psy_ui_component_parent(psy_ui_component_parent(&self->component)),
		NULL);
	psy_ui_component_addstylestate(
		psy_ui_component_parent(psy_ui_component_parent(&self->component)),
		psy_ui_STYLESTATE_SELECT);
	seqviewlist_invalidaterow(self, first.order);
	self->state->cmds->workspace->seqviewactive = TRUE;
}

void seqviewlist_onfocuslost(SeqviewList* self)
{
	psy_audio_OrderIndex first;

	seqviewlist_super_vtable.onfocuslost(&self->component);
	first = psy_audio_sequenceselection_first(
		&self->state->cmds->workspace->sequenceselection);
	self->state->active = FALSE;
	seqviewlist_invalidaterow(self, first.order);
	self->state->cmds->workspace->seqviewactive = FALSE;
	psy_ui_component_setborder(
		psy_ui_component_parent(psy_ui_component_parent(&self->component)),
		psy_ui_component_border(&self->component));
	psy_ui_component_removestylestate(
		psy_ui_component_parent(psy_ui_component_parent(&self->component)),
		psy_ui_STYLESTATE_SELECT);
}


psy_ui_RealRectangle seqviewlist_rowrectangle(SeqviewList* self,
	uintptr_t row)
{
	psy_ui_RealSize size;
	double lineheightpx;
	
	size = psy_ui_component_size_px(&self->component);
	lineheightpx = psy_ui_value_px(&self->state->lineheight,
		psy_ui_component_textmetric(&self->component), NULL);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0.0, lineheightpx * row),		
		psy_ui_realsize_make(size.width, lineheightpx + 1));
}

/* SeqviewDuration */
/* prototypes */
static void seqviewduration_ontimer(SeqviewDuration* self,
	psy_ui_Component* sender, uintptr_t id);
static void seqviewduration_ondestroy(SeqviewDuration* self,
	psy_ui_Component* sender);	
/* implementation */
void seqviewduration_init(SeqviewDuration* self,
	psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	self->duration_ms = 0;
	self->duration_bts = 0.0;
	self->calcduration = FALSE;
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
	psy_ui_component_setstyletype(psy_ui_label_base(&self->duration),
		STYLE_DURATION_TIME);	
	psy_signal_connect(&self->component.signal_timer, self,
		seqviewduration_ontimer);	
	psy_signal_connect(&self->component.signal_destroy, self,
		seqviewduration_ontimer);
	psy_ui_component_starttimer(&self->component, 0, 50);
	seqviewduration_update(self, FALSE);
}

void seqviewduration_ondestroy(SeqviewDuration* self, psy_ui_Component* sender)
{	
	psy_ui_component_stoptimer(&self->component, 0);
	seqviewduration_stopdurationcalc(self);
}

void seqviewduration_stopdurationcalc(SeqviewDuration* self)
{
	if (workspace_song(self->workspace) && self->calcduration) {
		psy_audio_sequence_endcalcdurationinmsresult(
			&workspace_song(self->workspace)->sequence);
		self->calcduration = FALSE;
	}
}

void seqviewduration_update(SeqviewDuration* self, bool force)
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
				seqviewduration_ontimer(self, &self->component, 0);
			}
			psy_snprintf(text, 64, "--m--s %.2fb", (float)self->duration_bts);
			psy_ui_label_settext(&self->duration, text);
		}
	}
}

void seqviewduration_ontimer(SeqviewDuration* self, psy_ui_Component* sender,
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

/* SeqView */
/* prototypes */
static void seqview_onsongchanged(SeqView*, Workspace*, int flag,
	psy_audio_Song*);
static void seqview_onselectionchanged(SeqView*,
	psy_audio_SequenceSelection*);
static void seqview_ontrackreposition(SeqView*,
	psy_audio_Sequence* sender, uintptr_t trackidx);
static void seqview_onsequencechanged(SeqView*,
	psy_audio_Sequence* sender);
static void seqview_onconfigure(SeqView*, GeneralConfig*, psy_Property*);
static void seqview_onscroll(SeqView*, psy_ui_Component* sender);
static void seqview_rebuild(SeqView*);
static void seqview_oneditseqlist(SeqView*, psy_ui_Button* sender);
/* implementation */
void seqview_init(SeqView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	sequencecmds_init(&self->cmds, workspace);
	/* shared state */
	seqviewstate_init(&self->state, &self->cmds);
	/* sequence listview */
	seqviewlist_init(&self->listview, &self->component, &self->state);
	psy_ui_scroller_init(&self->scroller, &self->listview.component,
		&self->component, NULL);	
	psy_ui_component_setspacing(psy_ui_scroller_base(&self->scroller),
		psy_ui_margin_make_em(0.5, 0.5, 0.0, 1.0));
	psy_ui_scroller_setbackgroundmode(&self->scroller,
		psy_ui_SETBACKGROUND, psy_ui_SETBACKGROUND);
	psy_ui_component_setstyletype_select(&self->scroller.component,
		STYLE_SEQLISTVIEW_FOCUS);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->listview.component,
		psy_ui_ALIGN_FIXED_RESIZE);
	/* button bar */
	sequencebuttons_init(&self->buttons, &self->component, &self->cmds);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);
	/* spacer */
	psy_ui_component_init_align(&self->spacer, &self->component, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_setpreferredsize(&self->spacer,
		psy_ui_size_make_em(0.0, 0.3));	
	/* header */
	seqviewtrackheaders_init(&self->trackheader, &self->component,
		&self->state);
	psy_ui_component_setalign(&self->trackheader.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->listview.component.signal_scroll, self,
		seqview_onscroll);	
	/* duration*/
	seqviewduration_init(&self->duration, &self->component, workspace);
	psy_ui_component_setalign(&self->duration.component, psy_ui_ALIGN_BOTTOM);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		seqview_onsongchanged);
	/* focus */
	/*psy_ui_button_init_connect(&self->focus, &self->scroller.component, NULL,
		self, seqview_oneditseqlist);
	psy_ui_button_preventtranslation(&self->focus);
	psy_ui_button_settext(&self->focus, "Edit");	
	psy_ui_component_setalign(&self->focus.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setspacing(&self->focus.component, psy_ui_margin_make_em(
	  0.25, 0.0, 0.25, 0.0));*/
	psy_signal_connect(&workspace->sequenceselection.signal_changed, self,
		seqview_onselectionchanged);
	if (self->cmds.sequence && self->cmds.sequence->patterns) {
		psy_signal_connect(&self->cmds.sequence->patterns->signal_namechanged,
			&self->listview,
			seqviewlist_onpatternnamechanged);
		psy_signal_connect(&self->cmds.sequence->signal_changed,
			self, seqview_onsequencechanged);
		psy_signal_connect(&self->cmds.sequence->signal_trackreposition,
			self, seqview_ontrackreposition);
	}
	psy_signal_connect(
		&psycleconfig_general(workspace_conf(workspace))->signal_changed,
		self, seqview_onconfigure);	
}

void seqview_onscroll(SeqView* self, psy_ui_Component* sender)
{	
	psy_ui_component_setscrollleft(&self->trackheader.client,
		psy_ui_component_scrollleft(seqviewlist_base(&self->listview)));
}

void seqview_clear(SeqView* self)
{
	sequencecmds_clear(&self->cmds);
}

void seqview_onsongchanged(SeqView* self, Workspace* sender,
	int flag, psy_audio_Song* song)
{
	sequencecmds_update(&self->cmds);	
	if (workspace_song(sender)) {		
		if (self->cmds.sequence && self->cmds.patterns) {
			psy_signal_connect(
				&self->cmds.sequence->patterns->signal_namechanged,
				&self->listview, seqviewlist_onpatternnamechanged);
			psy_signal_connect(&self->cmds.sequence->signal_changed,
				self, seqview_onsequencechanged);
			psy_signal_connect(&self->cmds.sequence->signal_trackreposition,
				self, seqview_ontrackreposition);
		}
	}
	seqview_rebuild(self);
}

void seqview_rebuild(SeqView* self)
{
	seqviewduration_stopdurationcalc(&self->duration);
	seqviewtrackheaders_build(&self->trackheader);
	seqviewlist_build(&self->listview);			
	seqviewduration_update(&self->duration, TRUE);	
	psy_ui_component_align_full(&self->scroller.component);	
	psy_ui_component_invalidate(&self->scroller.component);	
}

void seqview_oneditseqlist(SeqView* self, psy_ui_Button* sender)
{
	psy_ui_component_setfocus(&self->listview.component);
}

void seqview_onselectionchanged(SeqView* self,
	psy_audio_SequenceSelection* sender)
{		
	uintptr_t c = 0;
	double visilines;
	double listviewtop;
	psy_ui_RealSize clientsize;
	
	c = sender->editposition.order;
	if (c == psy_INDEX_INVALID) {
		c = 0;
	}
	clientsize = psy_ui_component_clientsize_px(&self->listview.component);
	visilines = (clientsize.height) /
		psy_ui_value_px(&self->state.lineheight,
			psy_ui_component_textmetric(&self->component), NULL);
	listviewtop = psy_ui_component_scrolltop_px(&self->listview.component) /
		psy_ui_value_px(&self->state.lineheight,
			psy_ui_component_textmetric(&self->component), NULL);
	if ((double)c < listviewtop) {
		psy_ui_component_setscrolltop(&self->listview.component,
			psy_ui_value_make_px(c *
				psy_ui_value_px(&self->state.lineheight,
					psy_ui_component_textmetric(&self->component), NULL)));				
	} else if ((double)c > listviewtop + visilines - 1) {
		psy_ui_component_setscrolltop(&self->listview.component,
			psy_ui_value_make_px((c - visilines + 1) * 
				psy_ui_value_px(&self->state.lineheight,
					psy_ui_component_textmetric(&self->component), NULL)));
	}	
	psy_ui_component_invalidate(&self->listview.component);
	psy_ui_component_invalidate(&self->trackheader.component);
}

void seqview_onsequencechanged(SeqView* self,
	psy_audio_Sequence* sender)
{			
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_component_scrollsize(&self->component));
	seqviewduration_stopdurationcalc(&self->duration);
	seqviewtrackheaders_build(&self->trackheader);
	seqviewlist_build(&self->listview);
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_updateoverflow(&self->listview.component);
	psy_ui_component_invalidate(&self->listview.component);
	psy_ui_component_align(&self->trackheader.component);
	seqviewduration_update(&self->duration, TRUE);
}

void seqview_ontrackreposition(SeqView* self, psy_audio_Sequence* sender,
	uintptr_t trackindex)
{
	seqviewduration_update(&self->duration, FALSE);	
	psy_ui_component_invalidate(&self->listview.component);
}

void seqview_onconfigure(SeqView* self, GeneralConfig* config,
	psy_Property* property)
{
	if (generalconfig_showingpatternnames(config)) {
		self->state.showpatternnames = TRUE;
		self->state.trackwidth = psy_ui_value_make_ew(26.0);		
	} else {
		self->state.showpatternnames = FALSE;
		self->state.trackwidth = psy_ui_value_make_ew(18.0);		
	}
	seqviewtrackheaders_build(&self->trackheader);
	psy_ui_component_align(&self->trackheader.component);
	psy_ui_component_align(&self->listview.component);
	psy_ui_component_invalidate(&self->listview.component);	
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditorentry.h"
/* host */
#include "seqeditorstate.h"
#include "styles.h"
/* audio */
#include <exclusivelock.h>
#include <sequencecmds.h>
/* platform */
#include "../../detail/portable.h"

/* SeqEditorEntry */
/* prototypes */
static void seqeditentry_ondestroy(SeqEditEntry*);
static void seqeditentry_onpreferredsize(SeqEditEntry*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqeditentry_onmousemove(SeqEditEntry*, psy_ui_MouseEvent*);
static void seqeditentry_onsequenceselectionselect(SeqEditEntry*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditentry_onsequenceselectiondeselect(SeqEditEntry*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
/* vtable */
static psy_ui_ComponentVtable seqeditentry_vtable;
static bool seqeditentry_vtable_initialized = FALSE;

static void seqeditentry_vtable_init(SeqEditEntry* self)
{
	if (!seqeditentry_vtable_initialized) {
		seqeditentry_vtable = *(self->component.vtable);	
		seqeditentry_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqeditentry_ondestroy;
		seqeditentry_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqeditentry_onpreferredsize;		
		seqeditentry_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditentry_onmousemove;
	}
	self->component.vtable = &seqeditentry_vtable;
}
/* implementation */
void seqeditentry_init(SeqEditEntry* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceEntry* seqentry, psy_audio_OrderIndex seqpos,
	SeqEditState* state)
{
	assert(seqentry);

	psy_ui_component_init(&self->component, parent, view);
	seqeditentry_vtable_init(self);
	self->seqentry = seqentry;
	self->seqpos = seqpos;
	self->state = state;
	self->preventresize = FALSE;
	psy_signal_connect(&state->workspace->sequenceselection.signal_select,
		self, seqeditentry_onsequenceselectionselect);
	psy_signal_connect(&state->workspace->sequenceselection.signal_deselect,
		self, seqeditentry_onsequenceselectiondeselect);
	if (psy_audio_orderindex_equal(&self->seqpos,
		&self->state->workspace->sequenceselection.editposition)) {
		psy_ui_component_addstylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

void seqeditentry_ondestroy(SeqEditEntry* self)
{
	psy_signal_disconnect(
		&self->state->workspace->sequenceselection.signal_select,
		self, seqeditentry_onsequenceselectionselect);
	psy_signal_disconnect(
		&self->state->workspace->sequenceselection.signal_deselect,
		self, seqeditentry_onsequenceselectiondeselect);
}

void seqeditentry_onpreferredsize(SeqEditEntry* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	assert(self->seqentry);
	
	rv->width = psy_ui_value_make_px(seqeditstate_beattopx(self->state,
		psy_audio_sequenceentry_length(self->seqentry)));
	rv->height = self->state->lineheight;
}

void seqeditentry_startdrag(SeqEditEntry* self, psy_ui_MouseEvent* ev)
{	
	workspace_setseqeditposition(self->state->workspace,
		seqeditentry_seqpos(self));
	self->state->dragposition = seqeditstate_quantize(self->state,
		seqeditstate_pxtobeat(self->state, ev->pt.x));	
	self->state->dragseqpos = self->seqpos;
	self->state->seqentry = self->seqentry;
	if (ev->button == 1) {
		if (self->state->dragposition >= psy_audio_sequenceentry_length(
				self->seqentry) - 1.0) {
			self->state->dragstatus = SEQEDIT_DRAG_LENGTH;
			psy_ui_component_setcursor(&self->component,
				psy_ui_CURSORSTYLE_COL_RESIZE);
		} else {
			self->state->dragstatus = SEQEDIT_DRAG_START;
		}		
	} else if (ev->button == 2) {			
		self->state->dragstatus = SEQEDIT_DRAG_REMOVE;
	}
}

void seqeditentry_onmousemove(SeqEditEntry* self, psy_ui_MouseEvent* ev)
{
	if (!self->preventresize) {
		psy_dsp_big_beat_t position;

		position = seqeditstate_quantize(self->state,
			seqeditstate_pxtobeat(self->state, ev->pt.x));	
		if (position >= psy_audio_sequenceentry_length(self->seqentry) - 1.0) {
			psy_ui_component_setcursor(&self->component,
				psy_ui_CURSORSTYLE_COL_RESIZE);
		}
	}
}

void seqeditentry_onsequenceselectionselect(SeqEditEntry* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	if (psy_audio_orderindex_equal(&self->seqpos, index)) {
		psy_ui_component_addstylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

void seqeditentry_onsequenceselectiondeselect(SeqEditEntry* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	if (psy_audio_orderindex_equal(&self->seqpos, index)) {
		psy_ui_component_removestylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

/* SeqEditorPatternEntry */
/* prototypes */
static void seqeditpatternentry_ondraw(SeqEditPatternEntry*, psy_ui_Graphics*);
static void seqeditpatternentry_onmousedown(SeqEditPatternEntry*,
	psy_ui_MouseEvent*);
static void seqeditpatternentry_onmousedoubleclick(SeqEditPatternEntry*,
	psy_ui_MouseEvent*);	
/* vtable */
static psy_ui_ComponentVtable seqeditpatternentry_vtable;
static bool seqeditpatternentry_vtable_initialized = FALSE;

static void seqeditpatternentry_vtable_init(SeqEditPatternEntry* self)
{
	if (!seqeditpatternentry_vtable_initialized) {
		seqeditpatternentry_vtable = *(seqeditpatternentry_base(self)->vtable);
		seqeditpatternentry_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqeditpatternentry_ondraw;
		seqeditpatternentry_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditpatternentry_onmousedown;		
		seqeditpatternentry_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqeditpatternentry_onmousedoubleclick;
	}
	seqeditpatternentry_base(self)->vtable = &seqeditpatternentry_vtable;
}
/* implementation */
void seqeditpatternentry_init(SeqEditPatternEntry* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequencePatternEntry* entry,
	psy_audio_OrderIndex seqpos,
	SeqEditState* state)
{
	seqeditentry_init(&self->seqeditorentry, parent, view,
		&entry->entry, seqpos, state);
	seqeditpatternentry_vtable_init(self);
	self->sequenceentry = entry;
	psy_ui_component_setstyletypes(seqeditpatternentry_base(self),
		STYLE_SEQEDT_ITEM, STYLE_SEQEDT_ITEM_HOVER,
		STYLE_SEQEDT_ITEM_SELECTED, psy_INDEX_INVALID);	
}

SeqEditPatternEntry* seqeditpatternentry_alloc(void)
{
	return (SeqEditPatternEntry*)malloc(sizeof(SeqEditPatternEntry));
}

SeqEditPatternEntry* seqeditpatternentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequencePatternEntry* entry, psy_audio_OrderIndex seqpos,
	SeqEditState* state)
{
	SeqEditPatternEntry* rv;

	rv = seqeditpatternentry_alloc();
	if (rv) {
		seqeditpatternentry_init(rv, parent, view, entry, seqpos, state);
		psy_ui_component_deallocateafterdestroyed(
			seqeditpatternentry_base(rv));
	}
	return rv;
}

void seqeditpatternentry_ondraw(SeqEditPatternEntry* self, psy_ui_Graphics* g)
{
	if (self->sequenceentry) {		
		psy_audio_Pattern* pattern;

		pattern = psy_audio_sequencepatternentry_pattern(self->sequenceentry,
			&self->seqeditorentry.state->workspace->song->patterns);
		if (pattern) {
			psy_ui_RealSize size;
			const psy_ui_TextMetric* tm;
			psy_ui_RealRectangle clip;
			psy_ui_RealPoint topleft;
			char text[64];

			tm = psy_ui_component_textmetric(seqeditpatternentry_base(self));
			size = psy_ui_component_scrollsize_px(seqeditpatternentry_base(self));
			size.width -= 4;
			clip = psy_ui_realrectangle_make(psy_ui_realpoint_zero(), size);
			topleft = psy_ui_realpoint_make(4, 2);
			if (self->seqeditorentry.state->showpatternnames) {
				psy_snprintf(text, 64, "%s", psy_audio_pattern_name(pattern));
			} else {
				psy_snprintf(text, 64, "%02X:%02X",
					self->seqeditorentry.seqpos.order,
					(int)psy_audio_sequencepatternentry_patternslot(
						self->sequenceentry));
			}
			psy_ui_textoutrectangle(g, topleft,
				psy_ui_ETO_CLIPPED, clip, text, psy_strlen(text));
 		}
	}
}

void seqeditpatternentry_onmousedown(SeqEditPatternEntry* self,
	psy_ui_MouseEvent* ev)
{
	if (self->sequenceentry) {
		psy_audio_PatternCursor cursor;		
		psy_audio_OrderIndex seqeditpos;

		seqeditpos = seqeditstate_editposition(self->seqeditorentry.state);
		seqeditentry_startdrag(&self->seqeditorentry, ev);
		cursor = self->seqeditorentry.state->workspace->patterneditposition;
		cursor.offset = self->seqeditorentry.state->dragposition;
		if (ev->button == 1 && self->seqeditorentry.state->dragstatus == SEQEDIT_DRAG_START) {
			if ((workspace_currview(self->seqeditorentry.state->workspace).id ==
				VIEW_ID_PATTERNVIEW) &&
				psy_audio_orderindex_equal(&self->seqeditorentry.seqpos, &seqeditpos)) {
				workspace_setpatterncursor(self->seqeditorentry.state->workspace, cursor);
				cursor = self->seqeditorentry.state->workspace->patterneditposition;
				workspace_gotocursor(self->seqeditorentry.state->workspace, cursor);
			} else {
				cursor.offset = 0;
				workspace_setpatterncursor(self->seqeditorentry.state->workspace, cursor);
				cursor = self->seqeditorentry.state->workspace->patterneditposition;
				workspace_gotocursor(self->seqeditorentry.state->workspace, cursor);
			}
			if (workspace_currview(self->seqeditorentry.state->workspace).id !=
				VIEW_ID_PATTERNVIEW) {
				workspace_selectview(self->seqeditorentry.state->workspace,
					VIEW_ID_PATTERNVIEW,
					workspace_currview(self->seqeditorentry.state->workspace).section,
					0);
			}
		}
	}
}

void seqeditpatternentry_onmousedoubleclick(SeqEditPatternEntry* self,
	psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		sequencecmds_changeplayposition(self->seqeditorentry.state->cmds);
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

/* SeqEditSampleEntry */
/* prototypes */
static void seqeditsampleentry_ondestroy(SeqEditSampleEntry*);
static void seqeditsampleentry_onsamplechanged(SeqEditSampleEntry*,
	psy_audio_SequenceSampleEntry* sender);
static void seqeditsampleentry_onsampleentrydestroy(SeqEditSampleEntry*,
	psy_audio_SequenceSampleEntry* sender);
static void seqeditsampleentry_onmousedown(SeqEditSampleEntry*,
	psy_ui_MouseEvent*);
static void seqeditsampleentry_onmousedoubleclick(SeqEditSampleEntry*,
	psy_ui_MouseEvent*);
static void seqeditsampleentry_edit(SeqEditSampleEntry*);
static void seqeditsampleentry_updatetext(SeqEditSampleEntry*);
static void seqeditsampleentry_onalign(SeqEditSampleEntry*);
/* vtable */
static psy_ui_ComponentVtable seqeditsampleentry_vtable;
static bool seqeditsampleentry_vtable_initialized = FALSE;

static void seqeditsampleentry_vtable_init(
	SeqEditSampleEntry* self)
{
	if (!seqeditsampleentry_vtable_initialized) {
		seqeditsampleentry_vtable = *(seqeditsampleentry_base(self)->vtable);
		seqeditsampleentry_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqeditsampleentry_ondestroy;
		seqeditsampleentry_vtable.onalign =
			(psy_ui_fp_component_onalign)
			seqeditsampleentry_onalign;
		seqeditsampleentry_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditsampleentry_onmousedown;
		seqeditsampleentry_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqeditsampleentry_onmousedoubleclick;
	}
	seqeditsampleentry_base(self)->vtable = &seqeditsampleentry_vtable;
}
/* implementation */
void seqeditsampleentry_init(SeqEditSampleEntry* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceSampleEntry* entry,
	psy_audio_OrderIndex seqpos,
	SeqEditState* state)
{
	assert(entry);

	seqeditentry_init(&self->seqeditorentry, parent, view,
		&entry->entry, seqpos, state);
	seqeditsampleentry_vtable_init(self);
	self->seqeditorentry.preventresize = TRUE;	
	psy_ui_component_setstyletypes(seqeditsampleentry_base(self),
		STYLE_SEQEDT_SAMPLE, STYLE_SEQEDT_SAMPLE_HOVER,
		STYLE_SEQEDT_SAMPLE_SELECTED, psy_INDEX_INVALID);
	wavebox_init(&self->wavebox, seqeditsampleentry_base(self), state->workspace);
	wavebox_setnowavetext(&self->wavebox, "");
	psy_ui_component_buffer(&self->wavebox.component);
	psy_ui_component_setalign(&self->wavebox.component, psy_ui_ALIGN_CLIENT);
	psy_ui_label_init(&self->label, seqeditsampleentry_base(self), NULL);
	psy_ui_component_setalign(&self->label.component, psy_ui_ALIGN_TOP);
	self->sequenceentry = entry;	
	self->preventedit = TRUE;	
	seqeditsampleentry_updatesample(self);
	seqeditsampleentry_updatetext(self);
	psy_signal_connect(&self->sequenceentry->signal_samplechanged, self,
		seqeditsampleentry_onsamplechanged);
	psy_signal_connect(&self->sequenceentry->signal_beforedestroy, self,
		seqeditsampleentry_onsampleentrydestroy);
}

void seqeditsampleentry_ondestroy(SeqEditSampleEntry* self)
{
	if (self->sequenceentry) {
		psy_signal_disconnect(&self->sequenceentry->signal_samplechanged, self,
			seqeditsampleentry_onsamplechanged);
		psy_signal_disconnect(&self->sequenceentry->signal_beforedestroy, self,
			seqeditsampleentry_onsampleentrydestroy);
	}
}

void seqeditsampleentry_onsampleentrydestroy(SeqEditSampleEntry* self,
	psy_audio_SequenceSampleEntry* sender)
{
	self->sequenceentry = NULL;
}

SeqEditSampleEntry* seqeditsampleentry_alloc(void)
{
	return (SeqEditSampleEntry*)malloc(sizeof(SeqEditSampleEntry));
}

SeqEditSampleEntry* seqeditsampleentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceSampleEntry* entry, psy_audio_OrderIndex seqpos,
	SeqEditState* state)
{
	SeqEditSampleEntry* rv;

	rv = seqeditsampleentry_alloc();
	if (rv) {
		seqeditsampleentry_init(rv, parent, view, entry, seqpos, state);
		psy_ui_component_deallocateafterdestroyed(
			seqeditsampleentry_base(rv));
	}
	return rv;
}

void seqeditsampleentry_updatesample(SeqEditSampleEntry* self)
{	
	if (self->sequenceentry) {
		psy_audio_Sample* sample;

		sequencecmds_update(self->seqeditorentry.state->cmds);
		if (self->seqeditorentry.state->cmds->sequence &&
			self->seqeditorentry.state->cmds->sequence->samples) {
			sample = psy_audio_samples_at(
				self->seqeditorentry.state->cmds->sequence->samples,
				self->sequenceentry->sampleindex);
		} else {
			sample = NULL;
		}
		psy_ui_component_clearbuffer(&self->wavebox.component);
		wavebox_setsample(&self->wavebox, sample, 0);
	}
}

void seqeditsampleentry_onsamplechanged(SeqEditSampleEntry* self,
	psy_audio_SequenceSampleEntry* sender)
{	
	seqeditsampleentry_updatesample(self);
	seqeditsampleentry_updatetext(self);
}

void seqeditsampleentry_updatetext(SeqEditSampleEntry* self)	
{
	if (self->sequenceentry) {
		char text[64];
		
		if (self->sequenceentry->samplerindex == psy_INDEX_INVALID) {
			psy_snprintf(text, 64, "%02X:%02X:%02X",
				(int)self->seqeditorentry.seqpos.order,
				(int)psy_audio_sequencesampleentry_samplesindex(
					self->sequenceentry).slot,
				(int)psy_audio_sequencesampleentry_samplesindex(
					self->sequenceentry).subslot);
		} else {
			psy_snprintf(text, 64, "%02X:%02X:%02X:%02X",
				(int)self->seqeditorentry.seqpos.order,
				(int)psy_audio_sequencesampleentry_samplesindex(
					self->sequenceentry).slot,
				(int)psy_audio_sequencesampleentry_samplesindex(
					self->sequenceentry).subslot,
				(int)psy_audio_sequencesampleentry_samplerindex(
					self->sequenceentry));
		}
		psy_ui_label_settext(&self->label, text);
	}
}

void seqeditsampleentry_onmousedown(SeqEditSampleEntry* self,
	psy_ui_MouseEvent* ev)
{
	if (self->sequenceentry) {
		seqeditentry_startdrag(&self->seqeditorentry, ev);
		if (ev->button == 1 && self->seqeditorentry.state->dragstatus == SEQEDIT_DRAG_START) {
			self->seqeditorentry.state->dragstatus = SEQEDIT_DRAG_START;
			if (workspace_currview(self->seqeditorentry.state->workspace).id !=
					VIEW_ID_SAMPLESVIEW) {
				workspace_selectview(self->seqeditorentry.state->workspace,
					VIEW_ID_SAMPLESVIEW,
					workspace_currview(self->seqeditorentry.state->workspace).section,
					0);
			}			
		}
	}
}

void seqeditsampleentry_onmousedoubleclick(SeqEditSampleEntry* self,
	psy_ui_MouseEvent* ev)
{	
	psy_ui_mouseevent_stop_propagation(ev);
}

void seqeditsampleentry_onalign(SeqEditSampleEntry* self)
{
	psy_ui_RealSize bpmsize;
	psy_ui_RealSize waveboxsize;

	bpmsize = psy_ui_bitmap_size(&self->wavebox.component.bufferbitmap);
	waveboxsize = psy_ui_component_scrollsize_px(&self->wavebox.component);
	if (waveboxsize.width != bpmsize.width ||
			waveboxsize.height != bpmsize.height) {	
		psy_ui_component_clearbuffer(&self->wavebox.component);
	}
}

/* SeqEditMarkerEntry */
/* prototypes */
static void seqeditmarkerentry_ondraw(SeqEditMarkerEntry*,
	psy_ui_Graphics*);
static void seqeditmarkerentry_onmousedown(SeqEditMarkerEntry*,
	psy_ui_MouseEvent*);
static void seqeditmarkerentry_onmousedoubleclick(SeqEditMarkerEntry*,
	psy_ui_MouseEvent*);
static void seqeditmarkerentry_oneditaccept(SeqEditMarkerEntry*,
	psy_ui_Edit* sender);
static void seqeditmarkerentry_oneditreject(SeqEditMarkerEntry*,
	psy_ui_Edit* sender);
static void seqeditmarkerentry_edit(SeqEditMarkerEntry*);
/* vtable */
static psy_ui_ComponentVtable seqeditmarkerentry_vtable;
static bool seqeditmarkerentry_vtable_initialized = FALSE;

static void seqeditmarkerentry_vtable_init(SeqEditMarkerEntry* self)
{
	if (!seqeditmarkerentry_vtable_initialized) {
		seqeditmarkerentry_vtable = *(seqeditmarkerentry_base(self)->vtable);		
		seqeditmarkerentry_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqeditmarkerentry_ondraw;
		seqeditmarkerentry_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditmarkerentry_onmousedown;		
		seqeditmarkerentry_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqeditmarkerentry_onmousedoubleclick;
	}
	seqeditmarkerentry_base(self)->vtable = &seqeditmarkerentry_vtable;
}
/* implementation */
void seqeditmarkerentry_init(SeqEditMarkerEntry* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceMarkerEntry* entry,
	psy_audio_OrderIndex seqpos,
	SeqEditState* state)
{	
	assert(entry);

	seqeditentry_init(&self->seqeditorentry, parent, view, &entry->entry,
		seqpos, state);
	seqeditmarkerentry_vtable_init(self);
	psy_ui_component_setstyletypes(seqeditmarkerentry_base(self),
		STYLE_SEQEDT_MARKER, STYLE_SEQEDT_MARKER_HOVER,
		STYLE_SEQEDT_MARKER_SELECTED, psy_INDEX_INVALID);
	
	self->sequenceentry = entry;	
	self->preventedit = TRUE;	
	if (self->seqeditorentry.state->edit) {
		psy_signal_connect(&self->seqeditorentry.state->edit->signal_accept,
			self, seqeditmarkerentry_oneditaccept);
		psy_signal_connect(&self->seqeditorentry.state->edit->signal_reject,
			self, seqeditmarkerentry_oneditreject);
	}
}

SeqEditMarkerEntry* seqeditmarkerentry_alloc(void)
{
	return (SeqEditMarkerEntry*)malloc(sizeof(SeqEditMarkerEntry));
}

SeqEditMarkerEntry* seqeditmarkerentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceMarkerEntry* entry, psy_audio_OrderIndex seqpos,
	SeqEditState* state)
{
	SeqEditMarkerEntry* rv;

	rv = seqeditmarkerentry_alloc();
	if (rv) {
		seqeditmarkerentry_init(rv, parent, view, entry, seqpos, state);
		psy_ui_component_deallocateafterdestroyed(
			seqeditmarkerentry_base(rv));
	}
	return rv;
}

void seqeditmarkerentry_ondraw(SeqEditMarkerEntry* self, psy_ui_Graphics* g)
{
	if (self->sequenceentry && self->sequenceentry->text) {
		psy_ui_RealSize size;
		const psy_ui_TextMetric* tm;
		psy_ui_RealRectangle clip;
		psy_ui_RealPoint topleft;
		char text[64];

		tm = psy_ui_component_textmetric(seqeditmarkerentry_base(self));
		size = psy_ui_component_scrollsize_px(seqeditmarkerentry_base(self));
		size.width -= 4;
		clip = psy_ui_realrectangle_make(psy_ui_realpoint_zero(), size);
		topleft = psy_ui_realpoint_make(4, 2);
		psy_snprintf(text, 64, "%s", self->sequenceentry->text);			
		psy_ui_textoutrectangle(g, topleft,
			psy_ui_ETO_CLIPPED, clip, text, psy_strlen(text));		
	}
}

void seqeditmarkerentry_onmousedown(SeqEditMarkerEntry* self,
	psy_ui_MouseEvent* ev)
{	
	seqeditentry_startdrag(&self->seqeditorentry, ev);	
}

void seqeditmarkerentry_onmousedoubleclick(SeqEditMarkerEntry* self,
	psy_ui_MouseEvent* ev)
{
	seqeditmarkerentry_edit(self);
	psy_ui_mouseevent_stop_propagation(ev);
}

void seqeditmarkerentry_edit(SeqEditMarkerEntry* self)
{
	if (self->seqeditorentry.state->edit) {
		psy_ui_RealRectangle screenposition;
		psy_ui_RealRectangle viewscreenposition;		
		const psy_ui_TextMetric* tm;
		double centery;
		psy_ui_RealSize size;

		screenposition = psy_ui_component_screenposition(seqeditmarkerentry_base(self));
		viewscreenposition = psy_ui_component_screenposition(
			psy_ui_component_parent(&self->seqeditorentry.state->edit->component));
		tm = psy_ui_component_textmetric(seqeditmarkerentry_base(self));
		size = psy_ui_component_scrollsize_px(seqeditmarkerentry_base(self));
		centery = (size.height - tm->tmHeight) / 2;
		psy_ui_component_setposition(&self->seqeditorentry.state->edit->component,
			psy_ui_rectangle_make(
				psy_ui_point_make_px(
					screenposition.left - viewscreenposition.left,
					screenposition.top - viewscreenposition.top + centery),
				psy_ui_size_make_px(
					screenposition.right - screenposition.left,
					tm->tmHeight)));
		psy_ui_edit_settext(self->seqeditorentry.state->edit, self->sequenceentry->text);
		psy_ui_edit_setsel(self->seqeditorentry.state->edit, 0, -1);
		psy_ui_edit_enableinputfield(self->seqeditorentry.state->edit);
		self->preventedit = FALSE;
		psy_ui_component_show(&self->seqeditorentry.state->edit->component);
		psy_ui_component_setfocus(&self->seqeditorentry.state->edit->component);
	}
}

void seqeditmarkerentry_oneditaccept(SeqEditMarkerEntry* self,
	psy_ui_Edit* sender)
{
	if (!self->preventedit) {
		self->preventedit = TRUE;
		psy_strreset(&self->sequenceentry->text,
			psy_ui_edit_text(self->seqeditorentry.state->edit));
		psy_ui_component_hide(psy_ui_edit_base(sender));
	}
}

void seqeditmarkerentry_oneditreject(SeqEditMarkerEntry* self,
	psy_ui_Edit* sender)
{
	if (!self->preventedit) {
		self->preventedit = TRUE;
		psy_ui_component_hide(psy_ui_edit_base(sender));				
	}
}

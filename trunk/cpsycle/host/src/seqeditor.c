/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditor.h"
/* host */
#include "styles.h"

/* prototypes */
static void seqeditor_ondestroy(SeqEditor*);
static void seqeditor_onsongchanged(SeqEditor*, Workspace*, int flag,
	psy_audio_Song*);
static void seqeditor_updatesong(SeqEditor*);
static void seqeditor_build(SeqEditor*);
static void seqeditor_ontracksscroll(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_onconfigure(SeqEditor*, GeneralConfig* sender,
	psy_Property*);
static void seqeditor_onhzoom(SeqEditor*, ZoomBox* sender);
static void seqeditor_onvzoom(SeqEditor*, ZoomBox* sender);
static void seqeditor_updatescrollstep(SeqEditor*);
static void seqeditor_onclear(SeqEditor*, psy_audio_Sequence*);
static void seqeditor_ontrackinsert(SeqEditor*, psy_audio_Sequence*,
	uintptr_t trackidx);
static void seqeditor_ontrackremove(SeqEditor*, psy_audio_Sequence*,
	uintptr_t trackidx);
static void seqeditor_ontrackswap(SeqEditor*,
	psy_audio_Sequence* sender, uintptr_t first, uintptr_t second);
static void seqeditor_onmousemove(SeqEditor*, psy_ui_MouseEvent*);
static void seqeditor_onmouseup(SeqEditor*, psy_ui_MouseEvent*);
static void seqeditor_ontoggleexpand(SeqEditor*, psy_ui_Button* sender);
static void seqeditor_ontrackresize(SeqEditor*, psy_ui_Component* sender,
	uintptr_t trackid, double* height);
static void seqeditor_ontoggletimesig(SeqEditor*, psy_ui_Button* sender);
static void seqeditor_ontoggleloop(SeqEditor*, psy_ui_Button* sender);
static void seqeditor_onloopchange(SeqEditor*, SeqEditLoops* sender);
/* vtable */
static psy_ui_ComponentVtable seqeditor_vtable;
static bool seqeditor_vtable_initialized = FALSE;

static void seqeditor_vtable_init(SeqEditor* self)
{
	if (!seqeditor_vtable_initialized) {
		seqeditor_vtable = *(self->component.vtable);
		seqeditor_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqeditor_ondestroy;
		seqeditor_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditor_onmousemove;
		seqeditor_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			seqeditor_onmouseup;

		seqeditor_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqeditor_vtable;
}
/* implementation */
void seqeditor_init(SeqEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	seqeditor_vtable_init(self);	
	psy_ui_component_setstyletype(&self->component, STYLE_SEQEDT);	
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);
	psy_ui_edit_init(&self->edit, &self->component);
	psy_ui_component_hide(psy_ui_edit_base(&self->edit));
	sequencecmds_init(&self->cmds, workspace);
	seqeditstate_init(&self->state, &self->cmds, &self->edit, &self->component);	
	/* toolbar */
	seqedittoolbar_init(&self->toolbar, &self->component, &self->state);
	psy_ui_component_setalign(&self->toolbar.component, psy_ui_ALIGN_TOP);	
	/* spacer */
	psy_ui_component_init(&self->spacer, &self->component, NULL);
	psy_ui_component_setstyletype(&self->spacer, STYLE_SEQEDT_SPACER);
	psy_ui_component_setalign(&self->spacer, psy_ui_ALIGN_TOP);
	psy_ui_component_setpreferredsize(&self->spacer,
		psy_ui_size_make_em(0.0, 0.25));
	/* left */
	psy_ui_component_init(&self->left, &self->component, NULL);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);	
	psy_ui_component_setstyletype(&self->left, STYLE_SEQEDT_LEFT);
	/* SeqEditorHeaderBar */
	seqeditorheaderdescbar_init(&self->headerdescbar, &self->left);
	psy_ui_component_setalign(&self->headerdescbar.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->headerdescbar.hzoom.signal_changed, self,
		seqeditor_onhzoom);
	/* track description */
	psy_ui_component_init(&self->trackdescpane, &self->left, NULL);
	psy_ui_component_setalign(&self->trackdescpane, psy_ui_ALIGN_CLIENT);
	seqedittrackdesc_init(&self->trackdesc, &self->trackdescpane,
		&self->state, workspace);
	psy_signal_connect(&self->trackdesc.signal_resize, self,
		seqeditor_ontrackresize);
	psy_ui_component_setalign(&self->trackdesc.component,
		psy_ui_ALIGN_HCLIENT);	
	zoombox_init(&self->vzoom, &self->left);
	psy_ui_component_setalign(&self->vzoom.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->vzoom.signal_changed, self, seqeditor_onvzoom);	
	/* header */
	seqeditheader_init(&self->header, &self->component, &self->state);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->header.loops.signal_changed, self,
		seqeditor_onloopchange);
	/* connect expand */
	psy_signal_connect(&self->toolbar.expand.signal_clicked, self,
		seqeditor_ontoggleexpand);
	self->expanded = FALSE;
	/* tracks */
	seqeditortracks_init(&self->tracks, &self->component, &self->state,
		workspace);
	psy_ui_scroller_init(&self->scroller, &self->tracks.component,
		&self->component, NULL);	
	psy_ui_component_setalign(&self->tracks.component, psy_ui_ALIGN_FIXED);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
	psy_ui_component_setpreferredsize(&self->component, psy_ui_size_make(
		psy_ui_value_make_ew(20.0), psy_ui_value_make_ph(0.30)));
	seqeditor_updatesong(self);
	psy_signal_connect(&self->state.workspace->signal_songchanged, self,
		seqeditor_onsongchanged);
	psy_signal_connect(&self->tracks.component.signal_scroll, self,
		seqeditor_ontracksscroll);	
	psy_signal_connect(
		&psycleconfig_general(workspace_conf(workspace))->signal_changed,
		self, seqeditor_onconfigure);	
	psy_signal_connect(&self->toolbar.timesig.signal_clicked, self,
		seqeditor_ontoggletimesig);
	psy_signal_connect(&self->toolbar.loop.signal_clicked, self,
		seqeditor_ontoggleloop);
	seqeditor_updatescrollstep(self);
}

void seqeditor_ondestroy(SeqEditor* self)
{
	seqeditstate_dispose(&self->state);
}

void seqeditor_onsongchanged(SeqEditor* self, Workspace* workspace, int flag,
	psy_audio_Song* song)
{	
	seqeditor_updatesong(self);
}

void seqeditor_updatesong(SeqEditor* self)
{
	psy_audio_Song* song;

	song = self->state.workspace->song;
	if (song) {
		psy_ui_component_move(&self->trackdesc.component, psy_ui_point_zero());
		psy_ui_component_move(&self->tracks.component, psy_ui_point_zero());
		seqeditor_build(self);
		psy_ui_component_invalidate(&self->trackdesc.component);
		psy_ui_component_invalidate(&self->header.component);
		psy_ui_component_align(&self->tracks.component);
		psy_ui_component_invalidate(&self->tracks.component);
		psy_signal_connect(&song->sequence.signal_clear, self,
			seqeditor_onclear);
		psy_signal_connect(&song->sequence.signal_trackinsert, self,
			seqeditor_ontrackinsert);
		psy_signal_connect(&song->sequence.signal_trackswap, self,
			seqeditor_ontrackswap);
		psy_signal_connect(&song->sequence.signal_trackremove, self,
			seqeditor_ontrackremove);
	}
}

void seqeditor_updatescrollstep(SeqEditor* self)
{		
	psy_ui_component_setscrollstep_height(&self->tracks.component,
		seqeditstate_lineheight(&self->state));
	psy_ui_component_setscrollstep_height(&self->trackdesc.component,
		seqeditstate_lineheight(&self->state));
}

void seqeditor_onclear(SeqEditor* self, psy_audio_Sequence* sender)
{
	psy_ui_component_move(&self->trackdesc.component, psy_ui_point_zero());
	psy_ui_component_move(&self->tracks.component, psy_ui_point_zero());
	seqeditor_build(self);
	psy_ui_component_invalidate(&self->trackdesc.component);
	psy_ui_component_align(&self->tracks.component);
	psy_ui_component_invalidate(&self->tracks.component);
}

void seqeditor_ontrackinsert(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t trackidx)
{
	seqeditor_build(self);	
}

void seqeditor_ontrackswap(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t first, uintptr_t second)
{
	seqeditor_build(self);
	psy_ui_component_invalidate(&self->trackdesc.component);
	psy_ui_component_align(&self->tracks.component);
	psy_ui_component_invalidate(&self->tracks.component);
}

void seqeditor_ontrackremove(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t trackidx)
{
	seqeditor_build(self);	
}

void seqeditor_build(SeqEditor* self)
{	
	seqeditortracks_build(&self->tracks);
	seqedittrackdesc_build(&self->trackdesc);	
	psy_ui_component_align(&self->scroller.pane);	
	psy_ui_component_align(&self->trackdescpane);		
}

void seqeditor_ontracksscroll(SeqEditor* self, psy_ui_Component* sender)
{	
	psy_ui_component_setscrolltop(&self->trackdesc.component,
		psy_ui_component_scrolltop(&self->tracks.component));
	psy_ui_component_setscrollleft(&self->header.pane,
		psy_ui_component_scrollleft(&self->tracks.component));
}

void seqeditor_onconfigure(SeqEditor* self, GeneralConfig* sender,
	psy_Property* property)
{	
	self->state.showpatternnames = generalconfig_showingpatternnames(sender);
	psy_ui_component_invalidate(&self->tracks.component);
}

void seqeditor_onhzoom(SeqEditor* self, ZoomBox* sender)
{
	self->state.pxperbeat = sender->zoomrate * self->state.defaultpxperbeat;
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_align(&self->header.component);
}

void seqeditor_onvzoom(SeqEditor* self, ZoomBox* sender)
{
	self->state.lineheight = psy_ui_mul_value_real(
		self->state.defaultlineheight, zoombox_rate(sender));
	seqeditor_updatescrollstep(self);
	psy_ui_component_align(&self->scroller.pane);	
	psy_ui_component_align(&self->trackdescpane);
}

void seqeditor_onmousemove(SeqEditor* self, psy_ui_MouseEvent* ev)
{
	if (self->state.updatecursorposition) {
		psy_ui_component_invalidate(&self->header.component);
		self->state.updatecursorposition = FALSE;		
	}
}

void seqeditor_onmouseup(SeqEditor* self, psy_ui_MouseEvent* ev)
{
	if (self->state.cmd == SEQEDTCMD_NEWTRACK) {
		sequencecmds_appendtrack(self->state.cmds);
	} else if (self->state.cmd == SEQEDTCMD_DELTRACK) {
		sequencecmds_deltrack(self->state.cmds, self->state.dragseqpos.track);
	}	
	self->state.cmd = SEQEDTCMD_NONE;
}

void seqeditor_ontoggleexpand(SeqEditor* self, psy_ui_Button* sender)
{	
	self->expanded = !self->expanded;	
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_zero(), psy_ui_value_make_ph(
			(self->expanded) ? 0.75 : 0.3)));
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}

void seqeditor_ontrackresize(SeqEditor* self, psy_ui_Component* sender,
	uintptr_t trackid, double* height)
{
	psy_ui_Component* track;
	
	assert(height);

	track = psy_ui_component_at(&self->tracks.component, trackid);
	if (track) {		
		psy_ui_component_setpreferredheight(track,
			psy_ui_value_make_eh(*height));
		psy_ui_component_align(&self->scroller.pane);		
		psy_ui_component_align(&self->trackdescpane);
	}
}

void seqeditor_ontoggletimesig(SeqEditor* self, psy_ui_Button* sender)
{	
	psy_ui_Size size;

	if (psy_ui_component_visible(&self->header.timesig.component)) {
		seqeditheader_hidetimesig(&self->header);		
	} else {
		seqeditheader_showtimesig(&self->header);
	}
	size = psy_ui_component_preferredsize(&self->header.component, NULL);
	if (psy_ui_component_visible(&self->header.timesig.component)) {
		psy_ui_button_highlight(sender);
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_button_base(sender)));		
	} else {		
		psy_ui_button_disablehighlight(sender);
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_button_base(sender)));		
	}
	psy_ui_component_setpreferredheight(&self->headerdescbar.component,
		size.height);
	psy_ui_component_align(&self->left);
	psy_ui_component_align(&self->component);
}

void seqeditor_ontoggleloop(SeqEditor* self, psy_ui_Button* sender)
{
	psy_ui_Size size;

	if (psy_ui_component_visible(&self->header.loops.component)) {
		seqeditheader_hideloops(&self->header);
	} else {
		seqeditheader_showloops(&self->header);
	}
	size = psy_ui_component_preferredsize(&self->header.component, NULL);
	if (psy_ui_component_visible(&self->header.loops.component)) {
		psy_ui_button_highlight(sender);
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_button_base(sender)));		
	} else {		
		psy_ui_button_disablehighlight(sender);
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_button_base(sender)));		
	}
	psy_ui_component_setpreferredheight(&self->headerdescbar.component,
		size.height);
	psy_ui_component_align(&self->left);
	psy_ui_component_align(&self->component);
}

void seqeditor_onloopchange(SeqEditor* self, SeqEditLoops* sender)
{
	psy_ui_component_invalidate(&self->tracks.component);
}

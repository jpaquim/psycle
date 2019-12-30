// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequenceview.h"
#include <stdio.h>
#include <portable.h>
#include <exclusivelock.h>

#define TIMERID_SEQUENCEVIEW 2000

static void sequencelistview_ondraw(SequenceListView*, psy_ui_Component* sender,
	psy_ui_Graphics*);
static void sequencelistview_drawsequence(SequenceListView*, psy_ui_Graphics*);
static void sequencelistview_drawtrack(SequenceListView*, psy_ui_Graphics*,
	SequenceTrack* track, int trackindex, int x);
static void sequencelistview_computetextsizes(SequenceListView*);
static void sequencelistview_adjustscrollbars(SequenceListView*);
static void sequencelistview_onmousedown(SequenceListView*, 
	psy_ui_Component* sender, psy_ui_MouseEvent*);
static void sequencelistview_onscroll(SequenceListView*,
	psy_ui_Component* sender, int stepx, int stepy);
static void sequencelistview_ontimer(SequenceListView*,
	psy_ui_Component* sender, int timerid);
static void sequenceview_onnewentry(SequenceView*);
static void sequenceview_oninsertentry(SequenceView*);
static void sequenceview_oncloneentry(SequenceView*);
static void sequenceview_ondelentry(SequenceView*);
static void sequenceview_onincpattern(SequenceView*);
static void sequenceview_ondecpattern(SequenceView*);
static void sequenceview_onnewtrack(SequenceView*);
static void sequenceview_ondeltrack(SequenceView*);
static void sequenceview_onclear(SequenceView*);
static void sequenceview_oncut(SequenceView*);
static void sequenceview_oncopy(SequenceView*);
static void sequenceview_onpaste(SequenceView*);
static void sequenceview_onsingleselection(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onmultiselection(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onshowplaylist(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onfollowsong(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onrecordtweaks(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onmultichannelaudition(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onsongchanged(SequenceView*, Workspace*);
static void sequenceview_onsequenceselectionchanged(SequenceView*, Workspace*);
static void sequenceview_preferredsize(SequenceView*, ui_size* limit,
	ui_size* rv);

static void sequenceduration_update(SequenceViewDuration*);

static void sequencebuttons_onalign(SequenceButtons* self,
	psy_ui_Component* sender);
static void sequencebuttons_preferredsize(SequenceButtons*, ui_size* limit,
	ui_size* rv);
static psy_List* rowend(psy_List* p);


static void sequenceviewtrackheader_ondraw(SequenceViewTrackHeader*,
	psy_ui_Component* sender, psy_ui_Graphics*);

static psy_ui_ComponentVtable sequenceview_vtable;
static int sequenceview_vtable_initialized = 0;

static void sequenceview_vtable_init(SequenceView* self)
{
	if (!sequenceview_vtable_initialized) {
		sequenceview_vtable = *(self->component.vtable);
		sequenceview_vtable.preferredsize = (psy_ui_fp_preferredsize)
			sequenceview_preferredsize;
	}
}

static psy_ui_ComponentVtable sequencebuttons_vtable;
static int sequencebuttons_vtable_initialized = 0;

static void sequencebuttons_vtable_init(SequenceButtons* self)
{
	if (!sequencebuttons_vtable_initialized) {
		sequencebuttons_vtable = *(self->component.vtable);
		sequencebuttons_vtable.preferredsize = (psy_ui_fp_preferredsize)
			sequencebuttons_preferredsize;
	}
}

static int listviewmargin = 5;

void sequenceview_init(SequenceView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	self->workspace = workspace;
	self->sequence = &workspace->song->sequence;
	self->patterns = &workspace->song->patterns;
	self->selection = &workspace->sequenceselection;	
	ui_component_init(&self->component, parent);
	sequenceview_vtable_init(self);
	self->component.vtable = &sequenceview_vtable;
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	ui_component_enablealign(&self->component);
	playlisteditor_init(&self->playlisteditor, &self->component,
		workspace);
	ui_component_setalign(&self->playlisteditor.component, UI_ALIGN_TOP);
	ui_splitbar_init(&self->splitbar, &self->component);
	ui_component_setalign(&self->splitbar.component, UI_ALIGN_TOP);
	if (!workspace_showplaylisteditor(workspace)) {
		ui_component_hide(&self->playlisteditor.component);
		ui_component_hide(&self->splitbar.component);
	}	
	sequencelistview_init(&self->listview, &self->component, self,
		self->sequence, self->patterns, workspace);
	ui_component_setalign(&self->listview.component, UI_ALIGN_CLIENT);
	self->listview.player = &workspace->player;
	self->buttons.context = &self->listview;
	sequencebuttons_init(&self->buttons, &self->component);	
	ui_component_setalign(&self->buttons.component, UI_ALIGN_TOP);
	sequenceviewtrackheader_init(&self->trackheader, &self->component, self);
	ui_component_setalign(&self->trackheader.component, UI_ALIGN_TOP);	
	sequenceroptionsbar_init(&self->options, &self->component);
	if (workspace_showplaylisteditor(workspace)) {
		ui_checkbox_check(&self->options.showplaylist);
	}
	ui_component_setalign(&self->options.component, UI_ALIGN_BOTTOM);
	sequenceduration_init(&self->duration, &self->component, self->sequence);
	ui_component_setalign(&self->duration.component, UI_ALIGN_BOTTOM);

	psy_signal_connect(&self->buttons.newentry.signal_clicked, self,
		sequenceview_onnewentry);
	psy_signal_connect(&self->buttons.insertentry.signal_clicked, self,
		sequenceview_oninsertentry);
	psy_signal_connect(&self->buttons.cloneentry.signal_clicked, self,
		sequenceview_oncloneentry);
	psy_signal_connect(&self->buttons.delentry.signal_clicked, self,
		sequenceview_ondelentry);
	psy_signal_connect(&self->buttons.incpattern.signal_clicked, self,
		sequenceview_onincpattern);
	psy_signal_connect(&self->buttons.decpattern.signal_clicked, self,
		sequenceview_ondecpattern);
	psy_signal_connect(&self->buttons.newtrack.signal_clicked, self,
		sequenceview_onnewtrack);
	psy_signal_connect(&self->buttons.deltrack.signal_clicked, self,
		sequenceview_ondeltrack);
	psy_signal_connect(&self->buttons.clear.signal_clicked, self,
		sequenceview_onclear);
	psy_signal_connect(&self->buttons.cut.signal_clicked, self,
		sequenceview_oncut);
	psy_signal_connect(&self->buttons.copy.signal_clicked, self,
		sequenceview_oncopy);
	psy_signal_connect(&self->buttons.paste.signal_clicked, self,
		sequenceview_onpaste);
	psy_signal_connect(&self->buttons.singlesel.signal_clicked, self,
		sequenceview_onsingleselection);
	psy_signal_connect(&self->buttons.multisel.signal_clicked, self,
		sequenceview_onmultiselection);
	psy_signal_connect(&self->options.showplaylist.signal_clicked, self,
		sequenceview_onshowplaylist);
	psy_signal_connect(&self->options.followsong.signal_clicked, self,
		sequenceview_onfollowsong);
	psy_signal_connect(&self->options.recordtweaks.signal_clicked, self,
		sequenceview_onrecordtweaks);
	psy_signal_connect(&self->options.multichannelaudition.signal_clicked, self,
		sequenceview_onmultichannelaudition);
	psy_signal_connect(&workspace->signal_songchanged, self,
		sequenceview_onsongchanged);
	psy_signal_connect(&workspace->signal_sequenceselectionchanged, self,
		sequenceview_onsequenceselectionchanged);
	ui_component_resize(&self->component, 200, 0);	
}

void sequencebuttons_init(SequenceButtons* self, psy_ui_Component* parent)
{		
	ui_component_init(&self->component, parent);
	sequencebuttons_vtable_init(self);
	self->component.vtable = &sequencebuttons_vtable;
	ui_component_enablealign(&self->component);
	ui_button_init(&self->incpattern, &self->component);	
	ui_button_settext(&self->incpattern, "+");
	ui_button_init(&self->insertentry, &self->component);	
	ui_button_settext(&self->insertentry, "Ins");		
	ui_button_init(&self->decpattern, &self->component);	
	ui_button_settext(&self->decpattern, "-");		
	ui_button_init(&self->newentry, &self->component);	
	ui_button_settext(&self->newentry, "New");		
	ui_button_init(&self->cloneentry, &self->component);	
	ui_button_settext(&self->cloneentry, "Clone");		
	ui_button_init(&self->delentry, &self->component);	
	ui_button_settext(&self->delentry, "Del");		
	ui_button_init(&self->newtrack, &self->component);	
	ui_button_settext(&self->newtrack, "New Trk");
	ui_button_init(&self->deltrack, &self->component);	
	ui_button_settext(&self->deltrack, "Del Trk");
	ui_button_init(&self->clear, &self->component);
	ui_button_settext(&self->clear, "Clear");
	ui_button_init(&self->cut, &self->component);
	ui_button_settext(&self->cut, "");
	ui_button_init(&self->copy, &self->component);
	ui_button_settext(&self->copy, "Copy");
	ui_button_init(&self->paste, &self->component);
	ui_button_settext(&self->paste, "Paste");
	ui_button_init(&self->singlesel, &self->component);
	ui_button_settext(&self->singlesel, "SingleSel");
	ui_button_init(&self->multisel, &self->component);	
	ui_button_settext(&self->multisel, "MultiSel");	
	ui_button_highlight(&self->singlesel);
	ui_button_disablehighlight(&self->multisel);
	psy_signal_connect(&self->component.signal_align, self,
		sequencebuttons_onalign);	
}

void sequencebuttons_onalign(SequenceButtons* self, psy_ui_Component* sender)
{
	int numparametercols = 3;
	int numrows = 0;
	int colwidth = 0;
	int rowheight = 0;
	int cpx = 0;
	int cpy = 0;
	int c = 0;
	int margin = 5;
	ui_size size;
	psy_List* p;
	psy_List* q;
	
	size = ui_component_size(&self->component);
	size = ui_component_preferredsize(&self->component, &size);
	colwidth = size.width / numparametercols;
	p = q = ui_component_children(&self->component, 0);	
	numrows = (psy_list_size(p) / numparametercols) + 1;
	rowheight = size.height / numrows - margin;	
	for ( ; p != 0; p = p->next, ++c, cpx += colwidth + margin) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
		if (c >= numparametercols) {
			cpx = 0;
			cpy += rowheight + margin;
			c = 0;
		}		
		ui_component_setposition(component, cpx, cpy, colwidth, rowheight);
	}
	psy_list_free(q);
}

void sequencebuttons_preferredsize(SequenceButtons* self, ui_size* limit,
	ui_size* rv)
{	
	if (rv) {
		int numparametercols = 3;
		int margin = 5;
		int c = 0;
		int cpx = 0;
		int cpy = 0;
		int cpxmax = 0;
		int cpymax = 0;
		int colmax = 0;
		ui_size size;
		psy_List* p;
		psy_List* q;
		
		size = ui_component_size(&self->component);	
		for (p = q = ui_component_children(&self->component, 0); p != 0;
				p = p->next, ++c) {
			psy_ui_Component* component;
			ui_size componentsize;
			if (c >= numparametercols) {
				cpx = 0;
				cpy = cpymax;
				c = 0;
			}
			component = (psy_ui_Component*)p->entry;
			componentsize = ui_component_preferredsize(component, &size);
			if (colmax < componentsize.width + margin) {
				colmax = componentsize.width + margin;
			}
			cpx += componentsize.width + margin;		
			if (cpymax < cpy + componentsize.height + margin) {
				cpymax = cpy + componentsize.height + margin;
			}		
		}
		psy_list_free(q);
		cpxmax = numparametercols * colmax;	
		rv->width = cpxmax;
		rv->height = cpymax;
	}
}


void sequenceviewtrackheader_init(SequenceViewTrackHeader* self,
	psy_ui_Component* parent, SequenceView* view)
{
	self->view = view;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);	
	psy_signal_connect(&self->component.signal_draw, self,
		sequenceviewtrackheader_ondraw);
	ui_component_resize(&self->component, 0, 10);
}

void sequenceviewtrackheader_ondraw(SequenceViewTrackHeader* self,
	psy_ui_Component* sender, psy_ui_Graphics* g)
{
	SequenceTracks* p;	
	int cpx = 0;
	int centery;
	int lineheight = 1;
	int c = 0;
	ui_rectangle r;	

	centery = (ui_component_size(&self->component).height - lineheight) / 2;
	sequencelistview_computetextsizes(&self->view->listview);
	for (p = self->view->sequence->tracks; p != 0; p = p->next, 
			cpx += self->view->listview.trackwidth, ++c) {
		ui_setrectangle(&r, cpx, centery, 
			self->view->listview.trackwidth - 5,
			lineheight);
		if (self->view->listview.selectedtrack == c) {
			ui_drawsolidrectangle(g, r, 0x00B1C8B0);
		} else {
			ui_drawsolidrectangle(g, r, 0x00444444);
		}
	}	
}

void sequencelistview_init(SequenceListView* self, psy_ui_Component* parent,
	SequenceView* view, psy_audio_Sequence* sequence, psy_audio_Patterns* patterns,
	Workspace* workspace)
{	
	self->view = view;
	self->sequence = sequence;
	self->patterns = patterns;
	self->workspace = workspace;
	self->selection = &workspace->sequenceselection;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	psy_signal_connect(&self->component.signal_draw, self,
		sequencelistview_ondraw);
	psy_signal_connect(&self->component.signal_mousedown, self, 
		sequencelistview_onmousedown);
	psy_signal_connect(&self->component.signal_scroll, self,
		sequencelistview_onscroll);
	psy_signal_connect(&self->component.signal_timer, self,
		sequencelistview_ontimer);
	self->selected = 0;
	self->selectedtrack = 0;	
	self->lineheight = 12;
	self->trackwidth = 100;
	self->dx = 0;
	self->dy = 0;
	self->lastplayposition = -1.f;
	self->lastentry = 0;
	ui_component_starttimer(&self->component, TIMERID_SEQUENCEVIEW, 200);
}

void sequencelistview_ondraw(SequenceListView* self, psy_ui_Component* sender,
	psy_ui_Graphics* g)
{	
	sequencelistview_drawsequence(self, g);
}

void sequencelistview_drawsequence(SequenceListView* self, psy_ui_Graphics* g)
{
	SequenceTracks* p;	
	int cpx = 0;
	int c = 0;		
	self->foundselected = 0;
	sequencelistview_computetextsizes(self);
	for (p = self->sequence->tracks; p != 0; p = p->next, 
			cpx += self->trackwidth, ++c) {
		sequencelistview_drawtrack(self, g, (SequenceTrack*)p->entry, c, cpx +
			self->dx + listviewmargin);
	}
	if (!self->foundselected) {
		int cpy;
		ui_rectangle r;

		cpx = self->selectedtrack * self->trackwidth + self->dx +
			listviewmargin + 5;
		cpy = self->selected * self->lineheight + listviewmargin;
		ui_setrectangle(&r, cpx, cpy, self->trackwidth - 5 - 2 * listviewmargin,
			self->textheight);
		ui_drawsolidrectangle(g, r, 0x009B7800);		
	}
}

void sequencelistview_computetextsizes(SequenceListView* self)
{
	ui_textmetric tm;
	
	tm = ui_component_textmetric(&self->component);
	self->avgcharwidth = tm.tmAveCharWidth;
	self->lineheight = (int) (tm.tmHeight * 1.5);
	self->textheight = tm.tmHeight;
	self->trackwidth = tm.tmAveCharWidth * 16;
	self->identwidth = tm.tmAveCharWidth * 4;	
}

void sequencelistview_drawtrack(SequenceListView* self, psy_ui_Graphics* g, SequenceTrack* track, int trackindex, int x)
{
	psy_List* p;	
	unsigned int c = 0;
	int cpy = 0;	
	char buffer[20];
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);
		
	ui_setrectangle(&r, x, 0, self->trackwidth - 5, size.height);	
//	if (trackindex == self->selectedtrack) {		
//		ui_drawsolidrectangle(g, r, 0x00303030);
//	} else {
//		ui_drawsolidrectangle(g, r, 0x00272727);		
//	}
	
	// ui_drawline(g, r.left, r.bottom - 1, r.right, r.bottom - 1);
	ui_settextcolor(g, 0);
	p = track->entries;
	for (; p != 0; p = p->next, ++c, cpy += self->lineheight) {
		SequenceEntry* entry;
		int playing = 0;
		// psy_audio_Pattern* pattern;
		entry = (SequenceEntry*)p->entry;		

		if (player_playing(self->player)) {
			psy_audio_Pattern* pattern;

			pattern = patterns_at(self->patterns, entry->pattern);			
			if (pattern && 
					player_position(self->player) >= entry->offset &&
					player_position(self->player) < entry->offset +
						pattern->length) {
				playing = 1;
			}
		}		
		psy_snprintf(buffer,20, "%02X:%02X  %4.2f", c, entry->pattern, entry->offset);
		if ( self->selectedtrack == trackindex &&
			(self->selection->editposition.trackposition.tracknode == p
				 || (psy_list_findentry(self->selection->entries, entry))				 
				 )) {
			ui_setbackgroundcolor(g, 0x009B7800);
			ui_settextcolor(g, 0x00FFFFFF);
			self->foundselected = 1;				
		} else 
		if (playing) {
			ui_setbackgroundcolor(g, 0x00232323);
			ui_settextcolor(g, 0x00D1C5B6);
		} else {
			ui_setbackgroundcolor(g, 0x00232323);
			ui_settextcolor(g, 0x00CACACA);
		}
		ui_textout(g, x + 5, cpy + self->dy + listviewmargin, buffer, strlen(buffer));
	}	
}

void sequenceview_onnewentry(SequenceView* self)
{		
	psy_List* tracknode;
	
	tracknode = sequence_insert(self->sequence, self->selection->editposition, 
		patterns_append(self->patterns, pattern_allocinit()));	
	sequenceselection_seteditposition(self->selection,
		sequence_makeposition(self->sequence,
			self->selection->editposition.track,
			tracknode));
	workspace_setsequenceselection(self->workspace,
		self->workspace->sequenceselection);	
	sequenceduration_update(&self->duration);
	sequencelistview_adjustscrollbars(&self->listview);
	ui_component_invalidate(&self->component);
}

void sequenceview_oninsertentry(SequenceView* self)
{	
	SequencePosition editposition;
	SequenceEntry* entry;
	psy_List* tracknode;
	
	editposition = self->workspace->sequenceselection.editposition;
	entry = sequenceposition_entry(&editposition);			
	tracknode = sequence_insert(self->sequence, editposition,
		entry ? entry->pattern :0);
	sequenceselection_seteditposition(self->selection,
		sequence_makeposition(self->sequence,
			self->selection->editposition.track,
			tracknode));
	workspace_setsequenceselection(self->workspace, self->workspace->sequenceselection);
	sequenceduration_update(&self->duration);
	sequencelistview_adjustscrollbars(&self->listview);
}

void sequenceview_oncloneentry(SequenceView* self)
{		
	SequencePosition editposition;
	SequenceEntry* entry;
	
	editposition = self->workspace->sequenceselection.editposition;
	entry = sequenceposition_entry(&editposition);
	if (entry) {			
		psy_audio_Pattern* pattern;
		psy_List* tracknode;
				
		pattern = patterns_at(self->patterns, entry->pattern);
		if (pattern) {			
			tracknode = sequence_insert(self->sequence, editposition, 
				patterns_append(self->patterns, pattern_clone(pattern)));
			sequenceselection_seteditposition(self->selection,
				sequence_makeposition(self->sequence,
				self->selection->editposition.track,
				tracknode));			
			workspace_setsequenceselection(self->workspace,
				self->workspace->sequenceselection);
		}						
	}			
	sequenceduration_update(&self->duration);
}

void sequenceview_ondelentry(SequenceView* self)
{	
	SequencePosition editposition;
	SequenceTrackNode* tracknode;
	
	editposition = self->workspace->sequenceselection.editposition;
	tracknode = sequence_remove(self->sequence, editposition);
	if (tracknode) {						
		editposition = sequence_makeposition(self->sequence,
				self->selection->editposition.track,
				tracknode);
	} else {
		editposition.trackposition = sequence_last(self->sequence,
			editposition.track);				
	}
	if (editposition.track == self->sequence->tracks &&
			sequence_size(self->sequence, editposition.track) == 0) {
		SequencePosition position;
		
		position.trackposition = sequence_begin(self->sequence, editposition.track, 0);
		position.track = editposition.track;
		tracknode = sequence_insert(self->sequence, position, 0);
		editposition = sequence_makeposition(self->sequence,
				self->selection->editposition.track,
				tracknode);		
	}		
	sequenceselection_seteditposition(self->selection, editposition);
	workspace_setsequenceselection(self->workspace, 
		self->workspace->sequenceselection);
	sequenceduration_update(&self->duration);
	sequencelistview_adjustscrollbars(&self->listview);
}

void sequenceview_onincpattern(SequenceView* self)
{
	SequenceEntry* entry;
	SequencePosition editposition;

	editposition = self->workspace->sequenceselection.editposition;
	entry = sequenceposition_entry(&editposition);
	if (entry) {
		sequence_setpatternslot(self->sequence, editposition,
			entry->pattern + 1);
	}
	sequenceselection_seteditposition(self->selection,
		sequence_makeposition(self->sequence,
			self->selection->editposition.track,
			editposition.trackposition.tracknode));
	workspace_setsequenceselection(self->workspace,
		self->workspace->sequenceselection);
	sequenceduration_update(&self->duration);
}

void sequenceview_ondecpattern(SequenceView* self)
{
	SequenceEntry* entry;
	SequencePosition editposition;

	editposition = self->workspace->sequenceselection.editposition;
	entry = sequenceposition_entry(&editposition); 
	if (entry && entry->pattern > 0) {
		sequence_setpatternslot(self->sequence, editposition,
			entry->pattern - 1);
	}
	sequenceselection_seteditposition(self->selection,
		sequence_makeposition(self->sequence,
			self->selection->editposition.track,
			editposition.trackposition.tracknode));
	workspace_setsequenceselection(self->workspace,
		self->workspace->sequenceselection);
	sequenceduration_update(&self->duration);
}

void sequenceview_onnewtrack(SequenceView* self)
{	
	lock_enter();
	sequence_appendtrack(self->sequence, sequencetrack_allocinit());
	lock_leave();
	sequencelistview_adjustscrollbars(&self->listview);
	ui_component_invalidate(&self->component);	
}

void sequenceview_ondeltrack(SequenceView* self)
{	
	SequencePosition position;
	position = sequence_at(self->sequence, self->listview.selectedtrack,
		self->listview.selected);	
	lock_enter();
	sequence_removetrack(self->sequence, position.track);
	lock_leave();
	ui_component_invalidate(&self->component);	
	sequenceduration_update(&self->duration);
	sequencelistview_adjustscrollbars(&self->listview);
}

void sequenceview_onclear(SequenceView* self)
{
	SequencePosition sequenceposition;

	if (self->workspace->song) {
		lock_enter();	
		workspace_disposesequencepaste(self->workspace);
		sequence_clear(self->sequence);
		patterns_clear(&self->workspace->song->patterns);		
		patterns_insert(&self->workspace->song->patterns, 0, pattern_allocinit());
		sequenceposition.track = 
			sequence_appendtrack(self->sequence, sequencetrack_allocinit());
		sequenceposition.trackposition =
			sequence_begin(self->sequence, sequenceposition.track, 0);
		sequence_insert(self->sequence, sequenceposition, 0);
		sequenceselection_setsequence(self->selection, self->sequence);
		workspace_setsequenceselection(self->workspace, *self->selection);		
		ui_component_invalidate(&self->component);
		lock_leave();
	}
}

void sequenceview_oncut(SequenceView* self)
{		
}

void sequenceview_oncopy(SequenceView* self)
{	
	psy_List* p;
	
	workspace_disposesequencepaste(self->workspace);	
	for (p = self->selection->entries; p != 0; p = p->next) {
		SequenceEntry* entry;		

		entry = (SequenceEntry*) p->entry;		
		psy_list_append(&self->workspace->sequencepaste,
			sequenceentry_allocinit(entry->pattern, entry->offset));
	}
}

void sequenceview_onpaste(SequenceView* self)
{	
	SequencePosition position;
	psy_List* p;
	
	position = self->selection->editposition;	
	for (p = self->workspace->sequencepaste; p != 0; p = p->next) {
		SequenceEntry* entry;
		SequenceTrackNode* node;
		
		entry = (SequenceEntry*) p->entry;
		node = sequence_insert(self->sequence, position, entry->pattern);
		position = sequence_makeposition(self->sequence,
				self->selection->editposition.track,
				node);
	}
	ui_component_invalidate(&self->component);
}

void sequenceview_onsingleselection(SequenceView* self, psy_ui_Button* sender)
{
	ui_button_highlight(&self->buttons.singlesel);
	ui_button_disablehighlight(&self->buttons.multisel);
	self->listview.selection->selectionmode = SELECTIONMODE_SINGLE;
}

void sequenceview_onmultiselection(SequenceView* self, psy_ui_Button* sender)
{
	ui_button_highlight(&self->buttons.multisel);
	ui_button_disablehighlight(&self->buttons.singlesel);
	self->listview.selection->selectionmode = SELECTIONMODE_MULTI;
}

void sequenceview_onfollowsong(SequenceView* self, psy_ui_Button* sender)
{
	if (workspace_followingsong(self->workspace)) {
		workspace_stopfollowsong(self->workspace);
	} else {
		workspace_followsong(self->workspace);
	}
}

void sequenceview_onshowplaylist(SequenceView* self, psy_ui_Button* sender)
{
	ui_size size;

	size = ui_component_size(&self->component);
	if (ui_component_visible(&self->playlisteditor.component)) {
		ui_component_hide(&self->playlisteditor.component);
		ui_component_hide(&self->splitbar.component);
		psy_properties_write_bool(self->workspace->general,
			"showplaylisteditor", 0);
	} else {		
		self->playlisteditor.component.visible = 1;
		ui_component_show(&self->splitbar.component);
		ui_component_show(&self->playlisteditor.component);		
		psy_properties_write_bool(self->workspace->general,
			"showplaylisteditor", 1);
	}
	ui_component_align(&self->component);
}

void sequenceview_onrecordtweaks(SequenceView* self, psy_ui_Button* sender)
{
	if (workspace_recordingtweaks(self->workspace)) {
		workspace_stoprecordtweaks(self->workspace);
	} else {
		workspace_recordtweaks(self->workspace);
	}
}

void sequenceview_onmultichannelaudition(SequenceView* self, psy_ui_Button* sender)
{
	self->workspace->player.multichannelaudition =
		!self->workspace->player.multichannelaudition;
}

void sequencelistview_onmousedown(SequenceListView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	unsigned int selected;
	unsigned int selectedtrack;	

	sequencelistview_computetextsizes(self);
	selected = (ev->y - listviewmargin - self->dy) / self->lineheight;
	selectedtrack = (ev->x - self->dx) / self->trackwidth;
	if (selectedtrack < sequence_sizetracks(self->sequence)) {		
		sequenceselection_seteditposition(self->selection,
			sequence_at(self->sequence, selectedtrack, selected));		
		workspace_setsequenceselection(self->workspace, *self->selection);
	}
}

void sequencelistview_onscroll(SequenceListView* self, psy_ui_Component* sender, int stepx, int stepy)
{
	self->dx += (stepx * self->component.scrollstepx);
	self->dy += (stepy * self->component.scrollstepy);	
	ui_component_invalidate(&self->view->trackheader.component);
}

void sequencelistview_adjustscrollbars(SequenceListView* self)
{
	ui_size size;

	size = ui_component_size(&self->component);
	sequencelistview_computetextsizes(self);
	{ // vertical lines
		int num;
		int visi;
		
		num = sequence_maxtracksize(self->sequence);		
		visi = size.height / self->lineheight;
		self->component.scrollstepy = self->lineheight;
		if (visi - num >= 0) {
			self->dy = 0;		
		}	
		ui_component_setverticalscrollrange(&self->component, 0, num - visi);	
	}	
	{ // horizontal tracks
		int num;
		int visi;
		
		num = sequence_sizetracks(self->sequence);
		visi = size.width / self->trackwidth;
		self->component.scrollstepx = self->trackwidth;
		if (visi - num >= 0) {
			self->dx = 0;		
		}	
		ui_component_sethorizontalscrollrange(&self->component, 0, num - visi);	
	}
}

void sequenceview_onsongchanged(SequenceView* self, Workspace* workspace)
{
	self->sequence = &workspace->song->sequence;
	self->patterns = &workspace->song->patterns;	
//	signal_connect(&workspace->song->sequence.signal_editpositionchanged,
//		self, sequenceview_oneditpositionchanged);
	self->listview.sequence = &workspace->song->sequence;
	self->listview.patterns = &workspace->song->patterns;
	self->listview.selected = 0;
	self->duration.sequence = &workspace->song->sequence;
	sequencelistview_adjustscrollbars(&self->listview);
	ui_component_invalidate(&self->component);
}

void sequenceview_onsequenceselectionchanged(SequenceView* self, Workspace* sender)
{
	SequencePosition position;
	SequenceTracks* p;
	psy_List* q;
	int c = 0;

	position = sender->sequenceselection.editposition;	
	p = sender->song->sequence.tracks;
	while (p != 0) {
		if (position.track == p) {
			break;
		}
		++c;
		p = p->next;		
	}
	self->listview.selectedtrack = c;
	c = 0;
	if (p) {
		q = ((SequenceTrack*)p->entry)->entries;		
		while (q) {
			if (q == position.trackposition.tracknode) {				
				break;
			}
			++c;
			q = q->next;
		}		
	}
	self->listview.selected = c;
	ui_component_invalidate(&self->listview.component);
	ui_component_invalidate(&self->trackheader.component);
}

void sequenceview_preferredsize(SequenceView* self, ui_size* limit,
	ui_size* rv)
{	
	if (rv) {		
		*rv = ui_component_size(&self->component);		
	}
}

void sequenceduration_init(SequenceViewDuration* self, psy_ui_Component* parent,
	psy_audio_Sequence* sequence)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makeeh(0.5), ui_value_makepx(0),
		ui_value_makeeh(0.5), ui_value_makeew(1.0));
	self->sequence = sequence;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);	
	ui_label_init(&self->desc, &self->component);	
	ui_label_settext(&self->desc, "Duration");
	ui_label_setcharnumber(&self->desc, 10);
	ui_label_setstyle(&self->desc, 
		WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE);
	ui_component_setalign(&self->desc.component, UI_ALIGN_LEFT);
	ui_label_init(&self->duration, &self->component);
	ui_label_setstyle(&self->duration, 
		WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE);
	ui_component_setalign(&self->duration.component, UI_ALIGN_CLIENT);
	ui_label_setcharnumber(&self->duration, 10);	
	psy_list_free(ui_components_setmargin(
			ui_component_children(&self->component, 0),
			&margin));
	sequenceduration_update(self);
}

void sequenceduration_update(SequenceViewDuration* self)
{
	char text[40];
	
	psy_snprintf(text, 40, "%.2f", sequence_duration(self->sequence));
	ui_label_settext(&self->duration, text);
}

void sequencelistview_ontimer(SequenceListView* self, psy_ui_Component* sender, int timerid)
{			
	SequenceTrackIterator it;
	
	if (player_playing(self->player)) {
		it = sequence_begin(self->sequence, self->sequence->tracks, 
			player_position(self->player));
		if (it.tracknode && self->lastentry != it.tracknode->entry) {
			ui_component_invalidate(&self->component);
			self->lastentry = (SequenceEntry*) it.tracknode->entry;
		}
	} else
	if (self->lastentry) {
		ui_component_invalidate(&self->component);
		self->lastentry = 0;
	}	
}

void sequenceroptionsbar_init(SequencerOptionsBar* self, psy_ui_Component* parent)
{
	ui_margin margin;

	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_checkbox_init(&self->followsong, &self->component);
	ui_checkbox_settext(&self->followsong, "Follow Song");
	ui_checkbox_init(&self->shownames, &self->component);
	ui_checkbox_settext(&self->shownames, "Show pattern names");
	ui_checkbox_init(&self->showplaylist, &self->component);	
	ui_checkbox_settext(&self->showplaylist, "Show playlist");
	ui_checkbox_init(&self->recordtweaks, &self->component);
	ui_checkbox_settext(&self->recordtweaks, "Record tweaks");
	ui_checkbox_init(&self->multichannelaudition, &self->component);
	ui_checkbox_settext(&self->multichannelaudition, "Multichannel Audition");
	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makepx(0),
		ui_value_makeeh(0.2), ui_value_makeew(1.0));
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_TOP,
		&margin));	
}

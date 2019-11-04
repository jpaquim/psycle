// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequenceview.h"
#include <stdio.h>

#define TIMERID_SEQUENCEVIEW 2000

static void sequencelistview_ondraw(SequenceListView*, ui_component* sender, ui_graphics* g);
static void sequencelistview_drawsequence(SequenceListView*, ui_graphics*);
static void sequencelistview_drawtrack(SequenceListView*, ui_graphics*, SequenceTrack* track,
	int trackindex, int x);
static void sequencelistview_computetextsizes(SequenceListView*);
static void sequencelistview_adjustscrollbars(SequenceListView*);
static void sequencelistview_onmousedown(SequenceListView*, ui_component* sender, int x, int y, int button);
static void sequencelistview_onscroll(SequenceListView*, ui_component* sender, int cx, int cy);
static void sequencelistview_ontimer(SequenceListView*, ui_component* sender, int timerid);

static void sequenceview_onnewentry(SequenceView*);
static void sequenceview_oninsertentry(SequenceView*);
static void sequenceview_oncloneentry(SequenceView*);
static void sequenceview_ondelentry(SequenceView*);
static void sequenceview_onincpattern(SequenceView*);
static void sequenceview_ondecpattern(SequenceView*);
static void sequenceview_onnewtrack(SequenceView*);
static void sequenceview_ondeltrack(SequenceView*);
static void sequenceview_onsize(SequenceView*, ui_component* sender, ui_size*);
static void sequenceview_onsongchanged(SequenceView*, Workspace*);
static void sequenceview_oneditpositionchanged(SequenceView*, Sequence* sender);

static void sequenceduration_update(SequenceViewDuration*);

static void sequencebuttons_onalign(SequenceButtons* self, ui_component* sender);
static void sequencebuttons_onpreferredsize(SequenceButtons*, ui_component* sender, ui_size* limit, ui_size* rv);
static List* rowend(List* p);

static int listviewmargin = 5;

void sequenceview_init(SequenceView* self, ui_component* parent,
	Workspace* workspace)
{	
	self->workspace = workspace;
	self->sequence = &workspace->song->sequence;
	self->patterns = &workspace->song->patterns;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);	
	signal_connect(&self->component.signal_size, self, sequenceview_onsize);	
	sequencelistview_init(&self->listview, &self->component, 
		&workspace->song->sequence, &workspace->song->patterns);
	self->listview.player = &workspace->player;
	self->buttons.context = &self->listview;
	sequencebuttons_init(&self->buttons, &self->component);
	ui_component_resize(&self->buttons.component, 200, 70);
	sequenceduration_init(&self->duration, &self->component, self->sequence);
	ui_checkbox_init(&self->followsong, &self->component);
	ui_checkbox_settext(&self->followsong, "Follow Song");
	ui_checkbox_init(&self->shownames, &self->component);	
	ui_checkbox_settext(&self->shownames, "Show pattern names");
	signal_connect(&self->buttons.newentry.signal_clicked, self,
		sequenceview_onnewentry);
	signal_connect(&self->buttons.insertentry.signal_clicked, self,
		sequenceview_oninsertentry);
	signal_connect(&self->buttons.cloneentry.signal_clicked, self,
		sequenceview_oncloneentry);
	signal_connect(&self->buttons.delentry.signal_clicked, self,
		sequenceview_ondelentry);
	signal_connect(&self->buttons.incpattern.signal_clicked, self,
		sequenceview_onincpattern);
	signal_connect(&self->buttons.decpattern.signal_clicked, self,
		sequenceview_ondecpattern);
	signal_connect(&self->buttons.newtrack.signal_clicked, self,
		sequenceview_onnewtrack);
	signal_connect(&self->buttons.deltrack.signal_clicked, self,
		sequenceview_ondeltrack);
	signal_connect(&workspace->signal_songchanged, self,
		sequenceview_onsongchanged);
	signal_connect(&workspace->song->sequence.signal_editpositionchanged, self,
		sequenceview_oneditpositionchanged);
	ui_component_resize(&self->component, 150, 0);
}

void sequencebuttons_init(SequenceButtons* self, ui_component* parent)
{		
	ui_component_init(&self->component, parent);	
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
	signal_connect(&self->component.signal_align, self, sequencebuttons_onalign);
	signal_disconnectall(&self->component.signal_preferredsize);
	signal_connect(&self->component.signal_preferredsize, self,
		sequencebuttons_onpreferredsize);	
}

void sequencebuttons_onalign(SequenceButtons* self, ui_component* sender)
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
	List* p;
	List* q;
	
	size = ui_component_size(&self->component);
	size = ui_component_preferredsize(&self->component, &size);
	colwidth = size.width / numparametercols;
	p = q = ui_component_children(&self->component, 0);	
	numrows = (list_size(p) / numparametercols) + 1;
	rowheight = size.height / numrows - margin;	
	for ( ; p != 0; p = p->next, ++c, cpx += colwidth + margin) {
		ui_component* component;

		component = (ui_component*)p->entry;
		if (c >= numparametercols) {
			cpx = 0;
			cpy += rowheight + margin;
			c = 0;
		}		
		ui_component_setposition(component, 
			cpx,
			cpy,
			colwidth,
			rowheight);		
	}
	list_free(q);
}

void sequencebuttons_onpreferredsize(SequenceButtons* self, ui_component* sender, ui_size* limit, ui_size* rv)
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
		List* p;
		List* q;
		
		size = ui_component_size(&self->component);	
		for (p = q = ui_component_children(&self->component, 0); p != 0; p = p->next,
				++c) {
			ui_component* component;
			ui_size componentsize;
			if (c >= numparametercols) {
				cpx = 0;
				cpy = cpymax;
				c = 0;
			}
			component = (ui_component*)p->entry;
			componentsize = ui_component_preferredsize(component, &size);
			if (colmax < componentsize.width + margin) {
				colmax = componentsize.width + margin;
			}
			cpx += componentsize.width + margin;		
			if (cpymax < cpy + componentsize.height + margin) {
				cpymax = cpy + componentsize.height + margin;
			}		
		}
		list_free(q);
		cpxmax = numparametercols * colmax;	
		rv->width = cpxmax;
		rv->height = cpymax;
	}
}

void sequencelistview_init(SequenceListView* self, ui_component* parent,
	Sequence* sequence, Patterns* patterns)
{				
	self->sequence = sequence;
	self->patterns = patterns;
	ui_component_init(&self->component, parent);	
	self->component.doublebuffered = 1;
	signal_connect(&self->component.signal_draw, self, sequencelistview_ondraw);
	signal_connect(&self->component.signal_mousedown, self, 
		sequencelistview_onmousedown);
	signal_connect(&self->component.signal_scroll, self,
		sequencelistview_onscroll);
	signal_connect(&self->component.signal_timer, self,
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

void sequencelistview_ondraw(SequenceListView* self, ui_component* sender, ui_graphics* g)
{	
	sequencelistview_drawsequence(self, g);
}

void sequencelistview_drawsequence(SequenceListView* self, ui_graphics* g)
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
		ui_setbackgroundcolor(g, 0x00FF0000);
		ui_textout(g, self->selectedtrack*self->trackwidth,
			self->selected * self->lineheight + listviewmargin, "     ", 5);
	}
}

void sequencelistview_computetextsizes(SequenceListView* self)
{
	TEXTMETRIC tm;
	
	tm = ui_component_textmetric(&self->component);
	self->avgcharwidth = tm.tmAveCharWidth;
	self->lineheight = (int) (tm.tmHeight * 1.5);
	self->trackwidth = tm.tmAveCharWidth * 16;
	self->identwidth = tm.tmAveCharWidth * 4;	
}

void sequencelistview_drawtrack(SequenceListView* self, ui_graphics* g, SequenceTrack* track, int trackindex, int x)
{
	List* p;	
	unsigned int c = 0;
	int cpy = 0;	
	char buffer[20];
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);
		
	ui_setrectangle(&r, x, 0, self->trackwidth - 5, size.height);	
	if (trackindex == self->selectedtrack) {		
		ui_drawsolidrectangle(g, r, 0x00303030);
	} else {
		ui_drawsolidrectangle(g, r, 0x00272727);		
	}
	
	// ui_drawline(g, r.left, r.bottom - 1, r.right, r.bottom - 1);
	ui_settextcolor(g, 0);
	p = track->entries;
	for (; p != 0; p = p->next, ++c, cpy += self->lineheight) {
		SequenceEntry* entry;
		int playing = 0;
		// Pattern* pattern;
		entry = (SequenceEntry*)p->entry;		

		if (player_playing(self->player)) {
			Pattern* pattern;

			pattern = patterns_at(self->patterns, entry->pattern);			
			if (pattern && 
					player_position(self->player) >= entry->offset &&
					player_position(self->player) < entry->offset +
						pattern->length) {
				playing = 1;
			}
		}		
		_snprintf(buffer,20, "%02X:%02X  %4.2f", c, entry->pattern, entry->offset);
		if (self->selected == (int)c && self->selectedtrack == trackindex) {
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

void sequenceview_onsize(SequenceView* self, ui_component* sender, ui_size* size)
{		
	ui_size buttonssize = ui_component_preferredsize(&self->buttons.component, size);
	ui_size durationsize = ui_component_preferredsize(&self->duration.component, size);
		
	ui_component_setposition(&self->buttons.component,
		0, 0, size->width, buttonssize.height);
	ui_component_setposition(&self->listview.component, 
		0, buttonssize.height,
		size->width,
		size->height - buttonssize.height - durationsize.height - 40);
	sequencelistview_adjustscrollbars(&self->listview);
	ui_component_setposition(&self->duration.component, 
		0,
		size->height - durationsize.height - 40,
		size->width,
		durationsize.height);
	ui_component_setposition(&self->followsong.component, 
		0,
		size->height - 40,
		size->width,
		20);
	ui_component_setposition(&self->shownames.component, 
		0,
		size->height - 20,
		size->width,
		20);
}

void sequenceview_onnewentry(SequenceView* self)
{		
	sequence_insert(self->sequence, sequence_editposition(self->sequence), 
		patterns_append(self->patterns, pattern_allocinit()));	
	sequenceduration_update(&self->duration);
	sequencelistview_adjustscrollbars(&self->listview);
}

void sequenceview_oninsertentry(SequenceView* self)
{	
	SequencePosition editposition;
	SequenceEntry* entry;
	
	editposition = sequence_editposition(self->sequence);
	entry = sequenceposition_entry(&editposition);			
	sequence_insert(self->sequence, editposition, entry ? entry->pattern :0);
	sequenceduration_update(&self->duration);
	sequencelistview_adjustscrollbars(&self->listview);
}

void sequenceview_oncloneentry(SequenceView* self)
{		
	SequencePosition editposition;
	SequenceEntry* entry;
	
	editposition = sequence_editposition(self->sequence);
	entry = sequenceposition_entry(&editposition);
	if (entry) {			
		Pattern* pattern;
				
		pattern = patterns_at(self->patterns, entry->pattern);
		if (pattern) {			
			sequence_insert(self->sequence, editposition, 
				patterns_append(self->patterns, pattern_clone(pattern)));
		}						
	}			
	sequenceduration_update(&self->duration);
}

void sequenceview_ondelentry(SequenceView* self)
{	
	SequencePosition editposition;
	
	editposition = sequence_editposition(self->sequence);
	sequence_remove(self->sequence, editposition);
	if (editposition.track == self->sequence->tracks &&
			sequence_size(self->sequence, editposition.track) == 0) {
		SequencePosition position;
		
		position.trackposition = sequence_begin(self->sequence, editposition.track, 0);
		position.track = editposition.track;
		sequence_insert(self->sequence, position, 0);
	}	
	sequenceduration_update(&self->duration);
	sequencelistview_adjustscrollbars(&self->listview);
}

void sequenceview_onincpattern(SequenceView* self)
{
	SequenceEntry* entry;
	SequencePosition editposition;

	editposition = sequence_editposition(self->sequence);	
	entry = sequenceposition_entry(&editposition);
	if (entry) {
		sequence_setpatternslot(self->sequence,
			sequence_editposition(self->sequence), entry->pattern + 1);
	}
	sequenceduration_update(&self->duration);	
}

void sequenceview_ondecpattern(SequenceView* self)
{
	SequenceEntry* entry;
	SequencePosition editposition;

	editposition = sequence_editposition(self->sequence);	
	entry = sequenceposition_entry(&editposition); 
	if (entry && entry->pattern > 0) {
		sequence_setpatternslot(self->sequence,
			sequence_editposition(self->sequence), entry->pattern - 1);
	}
	sequenceduration_update(&self->duration);
}

void sequenceview_onnewtrack(SequenceView* self)
{	
	sequence_appendtrack(self->sequence, sequencetrack_allocinit());
	sequencelistview_adjustscrollbars(&self->listview);
	ui_invalidate(&self->component);	
}

void sequenceview_ondeltrack(SequenceView* self)
{	
	SequencePosition position;
	position = sequence_at(self->sequence, self->listview.selectedtrack,
		self->listview.selected);	
	sequence_removetrack(self->sequence, position.track);
	ui_invalidate(&self->component);	
	sequenceduration_update(&self->duration);
	sequencelistview_adjustscrollbars(&self->listview);
}

void sequencelistview_onmousedown(SequenceListView* self, ui_component* sender, int x, int y, int button)
{
	unsigned int selected;
	unsigned int selectedtrack;	

	sequencelistview_computetextsizes(self);
	selected = (y - listviewmargin - self->dy) / self->lineheight;
	selectedtrack = (x - self->dx) / self->trackwidth;
	if (selectedtrack < sequence_sizetracks(self->sequence)) {
		SequencePosition position;

		position = sequence_at(self->sequence, selectedtrack, selected);
		sequence_seteditposition(self->sequence, position);
	}
}

void sequencelistview_onscroll(SequenceListView* self, ui_component* sender, int cx, int cy)
{
	self->dx += cx;
	self->dy += cy;	
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
	signal_connect(&workspace->song->sequence.signal_editpositionchanged,
		self, sequenceview_oneditpositionchanged);
	self->listview.sequence = &workspace->song->sequence;
	self->listview.patterns = &workspace->song->patterns;
	self->listview.selected = 0;
	self->duration.sequence = &workspace->song->sequence;
	ui_invalidate(&self->component);
}

void sequenceview_oneditpositionchanged(SequenceView* self, Sequence* sequence)
{
	SequencePosition position;
	SequenceTracks* p;
	List* q;
	int c = 0;

	position = sequence_editposition(sequence);
	
	p = sequence->tracks;
	while (p != 0) {
		if (position.track == p) {
			break;
		}
		++c;
		p = p->next;		
	}
	self->listview.selectedtrack = c;
	if (p) {
		q = ((SequenceTrack*)p->entry)->entries;
		c = 0;
		while (q) {
			if (q == position.trackposition.tracknode) {
				break;
			}
			++c;
			q = q->next;
		}
		self->listview.selected = c;
	}
	ui_invalidate(&self->listview.component);
}

void sequenceduration_init(SequenceViewDuration* self, ui_component* parent,
	Sequence* sequence)
{
	self->sequence = sequence;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);	
	ui_label_init(&self->desc, &self->component);	
	ui_label_settext(&self->desc, "Duration");
	ui_label_setcharnumber(&self->desc, 10);
	ui_component_setalign(&self->desc.component, UI_ALIGN_LEFT);
	ui_label_init(&self->duration, &self->component);
	ui_label_setstyle(&self->duration, 
		WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE);
	ui_component_setalign(&self->duration.component, UI_ALIGN_CLIENT);
	ui_label_setcharnumber(&self->duration, 10);
	sequenceduration_update(self);
}

void sequenceduration_update(SequenceViewDuration* self)
{
	char text[40];
	
	_snprintf(text, 40, "%.2f", sequence_duration(self->sequence));
	ui_label_settext(&self->duration, text);
}

void sequencelistview_ontimer(SequenceListView* self, ui_component* sender, int timerid)
{			
	SequenceTrackIterator it;
	
	if (player_playing(self->player)) {
		it = sequence_begin(self->sequence, self->sequence->tracks, 
			player_position(self->player));
		if (it.tracknode && self->lastentry != it.tracknode->entry) {
			ui_invalidate(&self->component);
			self->lastentry = (SequenceEntry*) it.tracknode->entry;
		}
	} else
	if (self->lastentry) {
		ui_invalidate(&self->component);
		self->lastentry = 0;
	}
}

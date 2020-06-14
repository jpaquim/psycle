// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequenceview.h"
#include <stdio.h>
#include <string.h>
#include <patterns.h>
#include <exclusivelock.h>
#include <songio.h>
#include <uialigner.h>
#include <assert.h>

#include "../../detail/portable.h"

#define TIMERID_SEQUENCEVIEW 2000

static void sequencelistview_onsize(SequenceListView*, const psy_ui_Size*);
static void sequencelistview_ondraw(SequenceListView*, psy_ui_Graphics*);
static void sequencelistview_drawtrack(SequenceListView*, psy_ui_Graphics*,
	SequenceTrack*, int trackindex, int x);
static void sequencelistview_computetextsizes(SequenceListView*);
static void sequencelistview_adjustscroll(SequenceListView*);
static void sequencelistview_onmousedown(SequenceListView*, 
	psy_ui_MouseEvent*);
static void sequencelistview_onmousedoubleclick(SequenceListView*,
	psy_ui_MouseEvent*);
static void sequencelistview_onscroll(SequenceListView*,
	psy_ui_Component* sender, int stepx, int stepy);
static void sequencelistview_limitverticalscroll(SequenceListView*);
static void sequencelistview_limithorizontalscroll(SequenceListView*);
static void sequencelistview_ontimer(SequenceListView*, uintptr_t timerid);
static void sequencelistview_onpatternnamechanged(SequenceListView*,
	psy_audio_Patterns*, uintptr_t slot);

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
static void sequenceview_onshowpatternnames(SequenceView*, psy_ui_CheckBox* sender);
static void sequenceview_onfollowsong(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onfollowsongchanged(SequenceView*, Workspace* sender);
static void sequenceview_onrecordtweaks(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onmultichannelaudition(SequenceView*, psy_ui_Button* sender);
static void sequenceview_onsongchanged(SequenceView*, Workspace*, int flag, psy_audio_SongFile* songfile);
static void sequenceview_onsequenceselectionchanged(SequenceView*, Workspace*);
static void sequenceview_onpreferredsize(SequenceView*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void sequenceview_updateplayposition(SequenceView*);
static void sequenceview_changeplayposition(SequenceView*);

static void sequenceduration_update(SequenceViewDuration*);



static void sequenceviewtrackheader_ondraw(SequenceViewTrackHeader*,
	psy_ui_Component* sender, psy_ui_Graphics*);

static psy_ui_ComponentVtable sequenceview_vtable;
static int sequenceview_vtable_initialized = 0;

static void sequenceview_vtable_init(SequenceView* self)
{
	if (!sequenceview_vtable_initialized) {
		sequenceview_vtable = *(self->component.vtable);
		sequenceview_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			sequenceview_onpreferredsize;
		sequenceview_vtable_initialized = 1;
	}
}

static int listviewmargin = 5;

void sequenceview_init(SequenceView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	sequenceview_vtable_init(self);
	self->component.vtable = &sequenceview_vtable;
	self->start = 1;
	self->workspace = workspace;
	self->sequence = &workspace->song->sequence;
	self->patterns = &workspace->song->patterns;
	self->selection = &workspace->sequenceselection;		
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	psy_ui_component_enablealign(&self->component);
	playlisteditor_init(&self->playlisteditor, &self->component,
		workspace);
	psy_ui_component_setalign(&self->playlisteditor.component, psy_ui_ALIGN_TOP);
	psy_ui_splitbar_init(&self->splitbar, &self->component);
	psy_ui_component_setalign(&self->splitbar.component, psy_ui_ALIGN_TOP);
	if (!workspace_showplaylisteditor(workspace)) {
		psy_ui_component_hide(&self->playlisteditor.component);
		psy_ui_component_hide(&self->splitbar.component);
	}	
	sequencelistview_init(&self->listview, &self->component, self,
		self->sequence, self->patterns, workspace);
	psy_ui_component_setalign(&self->listview.component, psy_ui_ALIGN_CLIENT);
	self->listview.player = &workspace->player;
	self->buttons.context = &self->listview;
	sequencebuttons_init(&self->buttons, &self->component, workspace);	
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);
	sequenceviewtrackheader_init(&self->trackheader, &self->component, self);
	psy_ui_component_setalign(&self->trackheader.component, psy_ui_ALIGN_TOP);	
	sequenceroptionsbar_init(&self->options, &self->component, workspace);
	if (workspace_showplaylisteditor(workspace)) {
		psy_ui_checkbox_check(&self->options.showplaylist);
	}
	psy_ui_component_setalign(&self->options.component, psy_ui_ALIGN_BOTTOM);
	sequenceduration_init(&self->duration, &self->component, self->sequence, workspace);
	psy_ui_component_setalign(&self->duration.component, psy_ui_ALIGN_BOTTOM);

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
	psy_signal_connect(&self->options.shownames.signal_clicked, self,
		sequenceview_onshowpatternnames);
	psy_signal_connect(&self->options.recordtweaks.signal_clicked, self,
		sequenceview_onrecordtweaks);
	psy_signal_connect(&self->options.multichannelaudition.signal_clicked, self,
		sequenceview_onmultichannelaudition);
	psy_signal_connect(&workspace->signal_songchanged, self,
		sequenceview_onsongchanged);
	psy_signal_connect(&workspace->signal_sequenceselectionchanged, self,
		sequenceview_onsequenceselectionchanged);
	psy_signal_connect(&workspace->signal_followsongchanged, self,
		sequenceview_onfollowsongchanged);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(16),
		psy_ui_value_makepx(0)));
}

static void sequencebuttons_updatetext(SequenceButtons*);
static void sequencebuttons_onalign(SequenceButtons*);
static void sequencebuttons_onpreferredsize(SequenceButtons*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void sequencebuttons_onlanguagechanged(SequenceButtons*, Workspace* sender);

static psy_ui_ComponentVtable sequencebuttons_vtable;
static int sequencebuttons_vtable_initialized = 0;

static void sequencebuttons_vtable_init(SequenceButtons* self)
{
	if (!sequencebuttons_vtable_initialized) {
		sequencebuttons_vtable = *(self->component.vtable);
		sequencebuttons_vtable.onalign = (psy_ui_fp_onalign)
			sequencebuttons_onalign;
		sequencebuttons_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			sequencebuttons_onpreferredsize;
		sequencebuttons_vtable_initialized = 1;
	}
}

void sequencebuttons_init(SequenceButtons* self, psy_ui_Component* parent, Workspace* workspace)
{		
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	sequencebuttons_vtable_init(self);
	self->component.vtable = &sequencebuttons_vtable;
	psy_ui_component_enablealign(&self->component);
	psy_ui_button_init(&self->incpattern, &self->component);	
	psy_ui_button_init(&self->insertentry, &self->component);	
	psy_ui_button_init(&self->decpattern, &self->component);	
	psy_ui_button_init(&self->newentry, &self->component);		
	psy_ui_button_init(&self->cloneentry, &self->component);	
	psy_ui_button_init(&self->delentry, &self->component);	
	psy_ui_button_init(&self->newtrack, &self->component);	
	psy_ui_button_init(&self->deltrack, &self->component);	
	psy_ui_button_init(&self->clear, &self->component);
	psy_ui_button_init(&self->cut, &self->component);
	psy_ui_button_init(&self->copy, &self->component);
	psy_ui_button_init(&self->paste, &self->component);
	psy_ui_button_init(&self->singlesel, &self->component);
	psy_ui_button_init(&self->multisel, &self->component);		
	psy_ui_button_highlight(&self->singlesel);
	psy_ui_button_disablehighlight(&self->multisel);
	sequencebuttons_updatetext(self);
	psy_signal_connect(&self->workspace->signal_languagechanged, self,
		sequencebuttons_onlanguagechanged);
}

void sequencebuttons_updatetext(SequenceButtons* self)
{
	psy_ui_button_settext(&self->incpattern, "+");
	psy_ui_button_settext(&self->insertentry, "Ins");
	psy_ui_button_settext(&self->decpattern, "-");
	psy_ui_button_settext(&self->newentry,
		workspace_translate(self->workspace, "New"));
	psy_ui_button_settext(&self->cloneentry,
		workspace_translate(self->workspace, "Clone"));
	psy_ui_button_settext(&self->delentry,
		workspace_translate(self->workspace, "Del"));
	psy_ui_button_settext(&self->newtrack,
		workspace_translate(self->workspace, "New Trk"));
	psy_ui_button_settext(&self->deltrack,
		workspace_translate(self->workspace, "Del Trk"));
	psy_ui_button_settext(&self->clear,
		workspace_translate(self->workspace, "Clear"));
	psy_ui_button_settext(&self->cut, "");
	psy_ui_button_settext(&self->copy,
		workspace_translate(self->workspace, "Copy"));
	psy_ui_button_settext(&self->paste,
		workspace_translate(self->workspace, "Paste"));
	psy_ui_button_settext(&self->singlesel,
		workspace_translate(self->workspace, "SingleSel"));
	psy_ui_button_settext(&self->multisel,
		workspace_translate(self->workspace, "MultiSel"));
}

void sequencebuttons_onlanguagechanged(SequenceButtons* self,
	Workspace* sender)
{
	sequencebuttons_updatetext(self);
}

void sequencebuttons_onalign(SequenceButtons* self)
{
	int numparametercols = 3;
	int numrows = 0;
	int colwidth = 0;
	int rowheight = 0;
	int cpx = 0;
	int cpy = 0;
	int c = 0;
	int margin = 5;
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	psy_List* p;
	psy_List* q;
	
	size = psy_ui_component_size(&self->component);
	size = psy_ui_component_preferredsize(&self->component, &size);
	tm = psy_ui_component_textmetric(&self->component);
	colwidth = psy_ui_value_px(&size.width, &tm) / numparametercols;
	p = q = psy_ui_component_children(&self->component, 0);	
	numrows = (psy_list_size(p) / numparametercols) + 1;
	rowheight = psy_ui_value_px(&size.height, &tm) / numrows - margin;
	for ( ; p != NULL; p = p->next, ++c, cpx += colwidth + margin) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
		if (c >= numparametercols) {
			cpx = 0;
			cpy += rowheight + margin;
			c = 0;
		}		
		psy_ui_component_setposition(component,
			psy_ui_point_make(
				psy_ui_value_makepx(cpx),
				psy_ui_value_makepx(cpy)),
			psy_ui_size_make(
				psy_ui_value_makepx(colwidth),
				psy_ui_value_makepx(rowheight)));
	}
	psy_list_free(q);
}

void sequencebuttons_onpreferredsize(SequenceButtons* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
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
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		psy_List* p;
		psy_List* q;
		
		size = psy_ui_component_size(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		for (p = q = psy_ui_component_children(&self->component, 0); p != NULL;
				p = p->next, ++c) {
			psy_ui_Component* component;
			psy_ui_Size componentsize;
			if (c >= numparametercols) {
				cpx = 0;
				cpy = cpymax;
				c = 0;
			}
			component = (psy_ui_Component*)p->entry;
			componentsize = psy_ui_component_preferredsize(component, &size);
			if (colmax < psy_ui_value_px(&componentsize.width, &tm) + margin) {
				colmax = psy_ui_value_px(&componentsize.width, &tm) + margin;
			}
			cpx += psy_ui_value_px(&componentsize.width, &tm) + margin;
			if (cpymax < cpy + psy_ui_value_px(&componentsize.height, &tm) + margin) {
				cpymax = cpy + psy_ui_value_px(&componentsize.height, &tm) + margin;
			}		
		}
		psy_list_free(q);
		cpxmax = numparametercols * colmax;	
		rv->width = psy_ui_value_makepx(cpxmax);
		rv->height = psy_ui_value_makepx(cpymax);
	}
}

void sequenceviewtrackheader_init(SequenceViewTrackHeader* self,
	psy_ui_Component* parent, SequenceView* view)
{
	self->view = view;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_SET);
	psy_signal_connect(&self->component.signal_draw, self,
		sequenceviewtrackheader_ondraw);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_makepx(0),
		psy_ui_value_makeeh(1)));
}

void sequenceviewtrackheader_ondraw(SequenceViewTrackHeader* self,
	psy_ui_Component* sender, psy_ui_Graphics* g)
{
	SequenceTracks* p;	
	int cpx = 0;
	int centery;
	int lineheight = 1;
	int c = 0;
	psy_ui_Rectangle r;
	psy_ui_Size size;
	psy_ui_TextMetric tm;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	centery = (psy_ui_value_px(&size.height, &tm) - lineheight) / 2;
	sequencelistview_computetextsizes(&self->view->listview);
	for (p = self->view->sequence->tracks; p != NULL; p = p->next, 
			cpx += self->view->listview.trackwidth, ++c) {
		psy_ui_setrectangle(&r, cpx, centery, 
			self->view->listview.trackwidth - 5,
			lineheight);
		if (self->view->listview.selectedtrack == c) {
			psy_ui_drawsolidrectangle(g, r, 0x00B1C8B0);
		} else {
			psy_ui_drawsolidrectangle(g, r, 0x00444444);
		}
	}	
}

// sequencelistview

static psy_ui_ComponentVtable sequencelistview_vtable;
static int sequencelistview_vtable_initialized = 0;

static void sequencelistview_vtable_init(SequenceListView* self)
{
	if (!sequencelistview_vtable_initialized) {
		sequencelistview_vtable = *(self->component.vtable);
		sequencelistview_vtable.onsize = (psy_ui_fp_onsize)
			sequencelistview_onsize;
		sequencelistview_vtable.ondraw = (psy_ui_fp_ondraw)
			sequencelistview_ondraw;
		sequencelistview_vtable.onmousedown = (psy_ui_fp_onmousedown)
			sequencelistview_onmousedown;
		sequencelistview_vtable.onmousedoubleclick = (psy_ui_fp_onmousedoubleclick)
			sequencelistview_onmousedoubleclick;		
		sequencelistview_vtable.ontimer = (psy_ui_fp_ontimer)
			sequencelistview_ontimer;
		sequencelistview_vtable_initialized = 1;		
	}	
}

void sequencelistview_init(SequenceListView* self, psy_ui_Component* parent,
	SequenceView* view, psy_audio_Sequence* sequence,
	psy_audio_Patterns* patterns, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	sequencelistview_vtable_init(self);	
	self->component.vtable = &sequencelistview_vtable;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 1);
	self->view = view;
	self->sequence = sequence;
	self->patterns = patterns;
	self->workspace = workspace;
	self->selection = &workspace->sequenceselection;
	psy_signal_connect(&self->component.signal_scroll, self,
		sequencelistview_onscroll);
	self->selected = 0;
	self->selectedtrack = 0;	
	self->lineheight = 12;
	self->trackwidth = 100;
	self->dx = 0;
	self->dy = 0;
	self->lastplayposition = -1.f;
	self->lastentry = 0;
	self->showpatternnames = 0;
	if (self->sequence && self->sequence->patterns) {
		psy_signal_connect(&self->sequence->patterns->signal_namechanged, self,
			sequencelistview_onpatternnamechanged);
	}
	psy_ui_component_starttimer(&self->component, TIMERID_SEQUENCEVIEW, 200);
}

void sequencelistview_ondraw(SequenceListView* self, psy_ui_Graphics* g)
{
	SequenceTracks* p;	
	int cpx = 0;
	int c = 0;		
	self->foundselected = 0;
	sequencelistview_computetextsizes(self);
	for (p = self->sequence->tracks; p != NULL; p = p->next, 
			cpx += self->trackwidth, ++c) {
		sequencelistview_drawtrack(self, g, (SequenceTrack*)p->entry, c, cpx +
			self->dx + listviewmargin);
	}
	if (!self->foundselected) {
		int cpy;
		psy_ui_Rectangle r;

		cpx = self->selectedtrack * self->trackwidth + self->dx +
			listviewmargin + 5;
		cpy = self->selected * self->lineheight + listviewmargin;
		psy_ui_setrectangle(&r, cpx,
			cpy, self->trackwidth - 5 - 2 * listviewmargin,
			self->textheight);
		psy_ui_drawsolidrectangle(g, r, 0x009B7800);		
	}
}

void sequencelistview_onsize(SequenceListView* self, const psy_ui_Size* size)
{
	sequencelistview_adjustscroll(self);
}

void sequencelistview_computetextsizes(SequenceListView* self)
{
	psy_ui_TextMetric tm;
	
	tm = psy_ui_component_textmetric(&self->component);
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
	char text[20];
	psy_ui_Rectangle r;	
	psy_ui_Size size;
	psy_ui_TextMetric tm;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	psy_ui_setrectangle(&r, x, 0, self->trackwidth - 5, psy_ui_value_px(&size.height, &tm));
	psy_ui_settextcolor(g, 0);
	p = track->entries;
	for (; p != NULL; p = p->next, ++c, cpy += self->lineheight) {
		SequenceEntry* entry;
		int playing = 0;
		// psy_audio_Pattern* pattern;
		entry = (SequenceEntry*)p->entry;		

		if (psy_audio_player_playing(self->player)) {
			psy_audio_Pattern* pattern;

			pattern = patterns_at(self->patterns, entry->pattern);			
			if (pattern && 
					psy_audio_player_position(self->player) >= entry->offset &&
					psy_audio_player_position(self->player) < entry->offset +
					pattern->length) {
				playing = 1;
			}
		}
		if (self->showpatternnames) {
			psy_audio_Pattern* pattern;

			pattern = patterns_at(self->patterns, entry->pattern);
			if (pattern) {
				psy_snprintf(text, 20, "%02X: %s %4.2f", c, psy_audio_pattern_name(pattern), entry->offset);
			} else {
				psy_snprintf(text, 20, "%02X:%02X(ERR) %4.2f", c, entry->pattern, entry->offset);
			}
		} else {
			psy_snprintf(text, 20, "%02X:%02X  %4.2f", c, entry->pattern, entry->offset);
		}
		if ( self->selectedtrack == trackindex &&
			(self->selection->editposition.trackposition.tracknode == p
				 || (psy_list_findentry(self->selection->entries, entry))				 
				 )) {
			psy_ui_setbackgroundcolor(g, 0x009B7800);
			psy_ui_settextcolor(g, 0x00FFFFFF);
			self->foundselected = 1;				
		} else 
		if (playing) {
			psy_ui_setbackgroundcolor(g, 0x00232323);
			psy_ui_settextcolor(g, 0x00D1C5B6);
		} else {
			psy_ui_setbackgroundcolor(g, 0x00232323);
			psy_ui_settextcolor(g, 0x00CACACA);
		}
		psy_ui_textout(g, x + 5, cpy + self->dy + listviewmargin, text,
			strlen(text));
	}	
}

void sequencelistview_showpatternnames(SequenceListView* self)
{
	self->showpatternnames = 1;
	psy_ui_component_invalidate(&self->component);
}

void sequencelistview_showpatternslots(SequenceListView* self)
{
	self->showpatternnames = 0;
	psy_ui_component_invalidate(&self->component);
}

void sequenceview_onnewentry(SequenceView* self)
{		
	psy_List* tracknode;
	
	tracknode = sequence_insert(self->sequence, self->selection->editposition, 
		patterns_append(self->patterns, psy_audio_pattern_allocinit()));
	sequenceselection_seteditposition(self->selection,
		sequence_makeposition(self->sequence,
			self->selection->editposition.track,
			tracknode));
	workspace_setsequenceselection(self->workspace,
		self->workspace->sequenceselection);
	sequenceview_updateplayposition(self);
	sequenceduration_update(&self->duration);
	sequencelistview_adjustscroll(&self->listview);
	psy_ui_component_invalidate(&self->component);
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
	sequenceview_updateplayposition(self);
	workspace_setsequenceselection(self->workspace, self->workspace->sequenceselection);
	sequenceduration_update(&self->duration);
	sequencelistview_adjustscroll(&self->listview);
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
				patterns_append(self->patterns, psy_audio_pattern_clone(pattern)));
			sequenceselection_seteditposition(self->selection,
				sequence_makeposition(self->sequence,
				self->selection->editposition.track,
				tracknode));			
			workspace_setsequenceselection(self->workspace,
				self->workspace->sequenceselection);
			sequenceview_updateplayposition(self);
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
	sequenceview_updateplayposition(self);
	sequenceduration_update(&self->duration);
	sequencelistview_adjustscroll(&self->listview);
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
	sequenceview_updateplayposition(self);
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
	sequenceview_updateplayposition(self);
	sequenceduration_update(&self->duration);
}

void sequenceview_onnewtrack(SequenceView* self)
{	
	psy_audio_exclusivelock_enter();
	sequence_appendtrack(self->sequence, sequencetrack_allocinit());
	psy_audio_exclusivelock_leave();
	sequencelistview_adjustscroll(&self->listview);
	psy_ui_component_invalidate(&self->component);	
}

void sequenceview_ondeltrack(SequenceView* self)
{	
	SequencePosition position;
	position = sequence_at(self->sequence, self->listview.selectedtrack,
		self->listview.selected);	
	psy_audio_exclusivelock_enter();
	sequence_removetrack(self->sequence, position.track);
	psy_audio_exclusivelock_leave();
	psy_ui_component_invalidate(&self->component);	
	sequenceduration_update(&self->duration);
	sequencelistview_adjustscroll(&self->listview);
}

void sequenceview_onclear(SequenceView* self)
{
	SequencePosition sequenceposition;

	if (self->workspace->song) {
		psy_audio_exclusivelock_enter();	
		workspace_disposesequencepaste(self->workspace);
		sequence_clear(self->sequence);
		patterns_clear(&self->workspace->song->patterns);		
		patterns_insert(&self->workspace->song->patterns, 0,
			psy_audio_pattern_allocinit());
		sequenceposition.track = 
			sequence_appendtrack(self->sequence, sequencetrack_allocinit());
		sequenceposition.trackposition =
			sequence_begin(self->sequence, sequenceposition.track, 0);
		sequence_insert(self->sequence, sequenceposition, 0);
		sequenceselection_setsequence(self->selection, self->sequence);
		workspace_setsequenceselection(self->workspace, *self->selection);		
		sequenceview_updateplayposition(self);
		psy_ui_component_invalidate(&self->component);
		psy_audio_exclusivelock_leave();
	}
}

void sequenceview_oncut(SequenceView* self)
{		
}

void sequenceview_oncopy(SequenceView* self)
{	
	psy_List* p;
	
	workspace_disposesequencepaste(self->workspace);	
	for (p = self->selection->entries; p != NULL; p = p->next) {
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
	for (p = self->workspace->sequencepaste; p != NULL; p = p->next) {
		SequenceEntry* entry;
		SequenceTrackNode* node;
		
		entry = (SequenceEntry*) p->entry;
		node = sequence_insert(self->sequence, position, entry->pattern);
		position = sequence_makeposition(self->sequence,
				self->selection->editposition.track,
				node);
	}
	psy_ui_component_invalidate(&self->component);
}

void sequenceview_onsingleselection(SequenceView* self, psy_ui_Button* sender)
{
	psy_ui_button_highlight(&self->buttons.singlesel);
	psy_ui_button_disablehighlight(&self->buttons.multisel);
	self->listview.selection->selectionmode = SELECTIONMODE_SINGLE;
}

void sequenceview_onmultiselection(SequenceView* self, psy_ui_Button* sender)
{
	psy_ui_button_highlight(&self->buttons.multisel);
	psy_ui_button_disablehighlight(&self->buttons.singlesel);
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

void sequenceview_onfollowsongchanged(SequenceView* self, Workspace* sender)
{
	if (workspace_followingsong(sender)) {
		psy_ui_checkbox_check(&self->options.followsong);
	} else {
		psy_ui_checkbox_disablecheck(&self->options.followsong);
	}
}

void sequenceview_onshowplaylist(SequenceView* self, psy_ui_Button* sender)
{
	psy_ui_Size size;

	size = psy_ui_component_size(&self->component);
	if (psy_ui_component_visible(&self->playlisteditor.component)) {
		psy_ui_component_hide(&self->playlisteditor.component);
		psy_ui_component_hide(&self->splitbar.component);
		psy_properties_write_bool(self->workspace->general,
			"showplaylisteditor", 0);
	} else {		
		self->playlisteditor.component.visible = 1;
		psy_ui_component_show(&self->splitbar.component);
		psy_ui_component_show(&self->playlisteditor.component);		
		psy_properties_write_bool(self->workspace->general,
			"showplaylisteditor", 1);
	}
	psy_ui_component_align(&self->component);
}

void sequenceview_onshowpatternnames(SequenceView* self, psy_ui_CheckBox* sender)
{
	if (psy_ui_checkbox_checked(sender) != 0) {
		sequencelistview_showpatternnames(&self->listview);
	} else {
		sequencelistview_showpatternslots(&self->listview);
	}
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

void sequencelistview_onmousedown(SequenceListView* self,
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
		sequenceview_updateplayposition(self->view);
	}
}

void sequencelistview_onmousedoubleclick(SequenceListView* self,
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
		sequenceview_changeplayposition(self->view);
	}	
}

void sequencelistview_onscroll(SequenceListView* self,
	psy_ui_Component* sender, int stepx, int stepy)
{
	int olddy;

	olddy = self->dy;
	self->dx += (stepx * self->component.scrollstepx);
	self->dy += (stepy * self->component.scrollstepy);	
	sequencelistview_limitverticalscroll(self);
	if (olddy != self->dy) {
		psy_ui_component_invalidate(&self->view->trackheader.component);
	}
}

void sequencelistview_adjustscroll(SequenceListView* self)
{
	psy_ui_Size size;
	int smin, smax;

	psy_ui_component_verticalscrollrange(&self->component, &smin, &smax);
	size = psy_ui_component_size(&self->component);
	sequencelistview_computetextsizes(self);
	sequencelistview_limitverticalscroll(self);
	sequencelistview_limithorizontalscroll(self);
}

void sequencelistview_limitverticalscroll(SequenceListView* self)
{ 
	int num;
	int visi;
	int top;
	psy_ui_Size size;
	psy_ui_TextMetric tm;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);

	num = sequence_maxtracksize(self->sequence);
	visi = psy_ui_value_px(&size.height, &tm) / self->lineheight;
	if (visi > 0) {
		top = -self->dy / self->lineheight;
		self->component.scrollstepy = self->lineheight;
		if (visi - num >= 0) {
			self->dy = 0;
		} else
			if (top + visi > num) {
				self->dy = -(num - visi) * self->lineheight;
				if (self->dy > 0) {
					self->dy = 0;
				}
			}
		psy_ui_component_setverticalscrollrange(&self->component, 0, num - visi);
	}
}

void sequencelistview_limithorizontalscroll(SequenceListView* self)
{ // horizontal tracks
	int num;
	int visi;
	psy_ui_Size size;
	psy_ui_TextMetric tm;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	num = sequence_sizetracks(self->sequence);
	visi = psy_ui_value_px(&size.width, &tm) / self->trackwidth;
	self->component.scrollstepx = self->trackwidth;
	if (visi - num >= 0) {
		self->dx = 0;
	}
	psy_ui_component_sethorizontalscrollrange(&self->component, 0, num - visi);
}

void sequenceview_onsongchanged(SequenceView* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
{
	self->sequence = &workspace->song->sequence;
	self->patterns = &workspace->song->patterns;	
	self->listview.sequence = &workspace->song->sequence;
	self->listview.patterns = &workspace->song->patterns;
	self->listview.selected = 0;
	self->listview.dx = 0;
	self->listview.dy = 0;
	self->duration.sequence = &workspace->song->sequence;
	if (self->sequence && self->sequence->patterns) {
		psy_signal_connect(&self->sequence->patterns->signal_namechanged,
			&self->listview,
			sequencelistview_onpatternnamechanged);
	}
	sequencelistview_adjustscroll(&self->listview);
	sequenceduration_update(&self->duration);
	psy_ui_component_invalidate(&self->listview.component);
}

void sequenceview_onsequenceselectionchanged(SequenceView* self, Workspace* sender)
{
	SequencePosition position;
	SequenceTracks* p;
	psy_List* q;
	int c = 0;
	int visilines;
	int listviewtop;
	psy_ui_Size listviewsize;
	psy_ui_TextMetric tm;
	
	position = sender->sequenceselection.editposition;	
	p = sender->song->sequence.tracks;
	while (p != NULL) {
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
	listviewsize = psy_ui_component_size(&self->listview.component);
	tm = psy_ui_component_textmetric(&self->listview.component);
	visilines = (psy_ui_value_px(&listviewsize.height, &tm) - listviewmargin) / self->listview.lineheight;
	listviewtop = -self->listview.dy / self->listview.lineheight;
	if (c < listviewtop) {
		self->listview.dy = -c * self->listview.lineheight;
		listviewtop = -self->listview.dy / self->listview.lineheight;
		psy_ui_component_setverticalscrollposition(&self->listview.component,
			listviewtop);
	} else
	if (c > listviewtop + visilines - 1) {
		self->listview.dy = -(c - visilines + 1) * self->listview.lineheight;
		listviewtop = -self->listview.dy / self->listview.lineheight;
		psy_ui_component_setverticalscrollposition(&self->listview.component,
			listviewtop);
	}
	psy_ui_component_invalidate(&self->listview.component);
	psy_ui_component_invalidate(&self->trackheader.component);
}

void sequenceview_onpreferredsize(SequenceView* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	assert(rv);
	if (self->start) {		
		psy_ui_Aligner aligner;

		psy_ui_aligner_init(&aligner, &self->component);
		psy_ui_aligner_preferredsize(&aligner, limit, rv);
		self->start = 0;
	} else {
		*rv = psy_ui_component_size(&self->component);		
	}
}

// SequenceViewDuration
static void sequencedurationbar_updatetext(SequenceViewDuration* self);
static void sequencedurationbar_onlanguagechanged(SequenceViewDuration*, Workspace* sender);

void sequenceduration_init(SequenceViewDuration* self, psy_ui_Component* parent,
	psy_audio_Sequence* sequence, Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makeew(1.0));
	self->sequence = sequence;
	self->workspace = workspace;
	self->duration_ms = 0;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);	
	psy_ui_label_init(&self->desc, &self->component);	
	
	psy_ui_label_setcharnumber(&self->desc, 9);
	psy_ui_label_settextalignment(&self->desc, psy_ui_ALIGNMENT_CENTER_HORIZONTAL);
	psy_ui_component_setalign(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->duration, &self->component);
	psy_ui_label_settextalignment(&self->duration, psy_ui_ALIGNMENT_LEFT);
	psy_ui_component_setalign(&self->duration.component, psy_ui_ALIGN_CLIENT);
	psy_ui_label_setcharnumber(&self->duration, 10);	
	psy_list_free(psy_ui_components_setmargin(
		psy_ui_component_children(&self->component, 0),
		&margin));
	sequencedurationbar_updatetext(self);
	sequenceduration_update(self);
	psy_signal_connect(&self->workspace->signal_languagechanged, self,
		sequencedurationbar_onlanguagechanged);
}

void sequencedurationbar_updatetext(SequenceViewDuration* self)
{
	psy_ui_label_settext(&self->desc, workspace_translate(self->workspace,
		"sequencerview.Duration"));
}

void sequencedurationbar_onlanguagechanged(SequenceViewDuration* self, Workspace* sender)
{
	sequencedurationbar_updatetext(self);
}

void sequenceduration_update(SequenceViewDuration* self)
{
	char text[40];
	float songlength;
	
	songlength = psy_audio_sequence_calcdurationinms(self->sequence);
	psy_snprintf(text, 40, "%02dm%02ds %.2fb",
		(int)(songlength / 60), ((int)songlength % 60),
		sequence_duration(self->sequence));
	psy_ui_label_settext(&self->duration, text);
}

void sequencelistview_ontimer(SequenceListView* self, uintptr_t timerid)
{			
	SequenceTrackIterator it;
	
	if (psy_audio_player_playing(self->player)) {
		it = sequence_begin(self->sequence, self->sequence->tracks, 
			psy_audio_player_position(self->player));
		if (it.tracknode && self->lastentry != it.tracknode->entry) {
			psy_ui_component_invalidate(&self->component);
			self->lastentry = (SequenceEntry*) it.tracknode->entry;
		}
	} else
	if (self->lastentry) {
		psy_ui_component_invalidate(&self->component);
		self->lastentry = 0;
	}	
}

void sequencelistview_onpatternnamechanged(SequenceListView* self, psy_audio_Patterns* patterns,
	uintptr_t slot)
{
	psy_ui_component_invalidate(&self->component);
}

static void sequenceroptionsbar_onlanguagechanged(SequencerOptionsBar*, Workspace* sender);
static void sequenceroptionsbar_updatetext(SequencerOptionsBar*);

void sequenceroptionsbar_init(SequencerOptionsBar* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin margin;

	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_checkbox_init(&self->followsong, &self->component);	
	psy_ui_checkbox_init(&self->shownames, &self->component);	
	psy_ui_checkbox_init(&self->showplaylist, &self->component);		
	psy_ui_checkbox_init(&self->recordtweaks, &self->component);	
	psy_ui_checkbox_init(&self->multichannelaudition, &self->component);	
	psy_signal_connect(&self->workspace->signal_languagechanged, self,
		sequenceroptionsbar_onlanguagechanged);
	sequenceroptionsbar_updatetext(self);
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.2),
		psy_ui_value_makeew(1.0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		&margin));
}

void sequenceroptionsbar_updatetext(SequencerOptionsBar* self)
{
	psy_ui_checkbox_settext(&self->followsong,
		workspace_translate(self->workspace, "sequencerview.Follow Song"));
	psy_ui_checkbox_settext(&self->shownames,
		workspace_translate(self->workspace, "sequencerview.Show pattern names"));
	psy_ui_checkbox_settext(&self->showplaylist,
		workspace_translate(self->workspace, "sequencerview.Show playlist"));
	psy_ui_checkbox_settext(&self->recordtweaks,
		workspace_translate(self->workspace, "sequencerview.Record tweaks"));
	psy_ui_checkbox_settext(&self->multichannelaudition,
		workspace_translate(self->workspace, "sequencerview.Multichannel Audition"));
}

void sequenceroptionsbar_onlanguagechanged(SequencerOptionsBar* self, Workspace* sender)
{
	sequenceroptionsbar_updatetext(self);
}

void sequenceview_updateplayposition(SequenceView* self)
{	
	if (workspace_followingsong(self->workspace) &&
		psy_audio_player_playing(&self->workspace->player)) {
		sequenceview_changeplayposition(self);
	}
}

void sequenceview_changeplayposition(SequenceView* self)
{
	SequencePosition editposition;
	SequenceEntry* entry;
	psy_dsp_beat_t startposition;

	editposition = self->workspace->sequenceselection.editposition;
	entry = sequenceposition_entry(&editposition);
	startposition = entry->offset;
	psy_audio_exclusivelock_enter();
	psy_audio_player_stop(&self->workspace->player);
	psy_audio_player_setposition(&self->workspace->player, startposition);
	psy_audio_player_start(&self->workspace->player);
	psy_audio_exclusivelock_leave();
}


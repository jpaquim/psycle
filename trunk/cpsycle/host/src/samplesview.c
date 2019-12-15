// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "samplesview.h"
#include <instruments.h>

#include <exclusivelock.h>
#include <portable.h>
#include <songio.h>

#include <math.h>

/// Samples View
static void samplesview_onsampleboxchanged(SamplesView*, ui_component* sender);
static void samplesview_setsample(SamplesView*, SampleIndex slot);
static void samplesview_onloadsample(SamplesView*, ui_component* sender);
static void samplesview_onsavesample(SamplesView*, ui_component* sender);
static void samplesview_ondeletesample(SamplesView*, ui_component* sender);
static void samplesview_onduplicatesample(SamplesView*, ui_component* sender);
static void samplesview_onsongchanged(SamplesView*, Workspace*);
static void samplesview_oninstrumentslotchanged(SamplesView* self,
	Instrument* sender, int slot);
static uintptr_t samplesview_freesampleslot(SamplesView*, uintptr_t startslot,
	uintptr_t maxslots);
static void samplesview_onshow(SamplesView*, ui_component* sender);
static void samplesview_onhide(SamplesView*, ui_component* sender);
static void samplesview_onkeydown(SamplesView*, ui_component* sender, KeyEvent*);
/// Header View
static void samplesheaderview_init(SamplesHeaderView*, ui_component* parent,
	Instruments*, struct SamplesView*);
static void samplesheaderview_setsample(SamplesHeaderView*, Sample*);
static void OnInstrumentSlotChanged(SamplesView*, Instrument* sender, int slot);
static void OnPrevSample(SamplesHeaderView*, ui_component* sender);
static void OnNextSample(SamplesHeaderView*, ui_component* sender);
static void OnEditSampleName(SamplesHeaderView*, ui_edit* sender);
/// General Settings View
static void InitSamplesGeneralView(SamplesGeneralView*, ui_component* parent);
static void SetSampleSamplesGeneralView(SamplesGeneralView*, Sample* sample);
static void OnGeneralViewDescribe(SamplesGeneralView*, ui_slider*, char* txt);
static void GeneralViewFillPanDescription(SamplesGeneralView*, char* txt);
static void OnGeneralViewTweak(SamplesGeneralView*, ui_slider*, float value);
static void OnGeneralViewValue(SamplesGeneralView*, ui_slider*, float* value);
static void InitSamplesVibratoView(SamplesVibratoView*, ui_component* parent, Player* player);
static void SetSampleSamplesVibratoView(SamplesVibratoView*, Sample* sample);
/// Vibrato Settings View
static void OnVibratoViewDescribe(SamplesVibratoView*, ui_slider*, char* txt);
static void OnVibratoViewTweak(SamplesVibratoView*, ui_slider*, float value);
static void OnVibratoViewValue(SamplesVibratoView*, ui_slider*, float* value);
static void OnWaveFormChange(SamplesVibratoView*, ui_combobox* sender, int sel);
static WaveForms ComboBoxToWaveForm(int combobox_index);
static int WaveFormToComboBox(WaveForms waveform);
/// Waveloop Setting View
static void samplesloopview_init(SamplesLoopView*, ui_component* parent,
	SamplesView*);
static void samplesloopview_setsample(SamplesLoopView*, Sample* sample);
static int LoopTypeToComboBox(LoopType looptype);
static LoopType ComboBoxToLoopType(int combobox_index);
static void samplesloopview_onlooptypechange(SamplesLoopView*, ui_combobox* sender, int sel);
static void samplesloopview_onsustainlooptypechange(SamplesLoopView*,
	ui_combobox* sender, int selectedindex);
static void samplesloopview_looptypeenablepreventinput(SamplesLoopView*);
static void samplesloopview_oneditchangedloopstart(SamplesLoopView*, ui_edit* sender);
static void samplesloopview_oneditchangedloopend(SamplesLoopView*, ui_edit* sender);
static void samplesloopview_oneditchangedsustainstart(SamplesLoopView*, ui_edit* sender);
static void samplesloopview_oneditchangedsustainend(SamplesLoopView*, ui_edit* sender);

static void samplessongimportview_onloadsong(SamplesSongImportView*,
	ui_component* sender);
static void samplessongimportview_oncopy(SamplesSongImportView*,
	ui_component* sender);
static void samplessongimportview_ondestroy(SamplesSongImportView*,
	ui_component* sender);

void samplesviewbuttons_init(SamplesViewButtons* self, ui_component* parent)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(0.5),
		ui_value_makeeh(1.0), ui_value_makepx(0));
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	ui_button_init(&self->load, &self->component);
	ui_button_settext(&self->load, "Load");	
	ui_button_init(&self->save, &self->component);
	ui_button_settext(&self->save, "Save");	
	ui_button_init(&self->duplicate, &self->component);
	ui_button_settext(&self->duplicate, "Duplicate");
	ui_button_init(&self->del, &self->component);
	ui_button_settext(&self->del, "Delete");
	list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT, &margin));
}

void samplessongimportview_init(SamplesSongImportView* self, ui_component* parent,
	SamplesView* view, Workspace* workspace)
{	
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(0.5),
		ui_value_makeeh(1.0), ui_value_makepx(0));
	self->view = view;
	self->source = 0;
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_init(&self->header, &self->component);
	ui_component_enablealign(&self->header);
	ui_component_setalign(&self->header, UI_ALIGN_TOP);
	ui_label_init(&self->label, &self->header);
	ui_label_settext(&self->label, "Source");	
	ui_label_init(&self->songname, &self->header);
	ui_label_settext(&self->songname, "No song loaded");
	ui_label_setcharnumber(&self->songname, 30);	
	ui_button_init(&self->browse, &self->header);
	ui_button_settext(&self->browse, "Select a song");
	psy_signal_connect(&self->browse.signal_clicked, self,
		samplessongimportview_onloadsong);	
	list_free(ui_components_setalign(		
		ui_component_children(&self->header, 0),
		UI_ALIGN_LEFT,
		&margin));
	samplesbox_init(&self->samplesbox, &self->component, 0, 0);
	ui_component_setalign(&self->samplesbox.component,
		UI_ALIGN_CLIENT);
	ui_component_init(&self->bar, &self->component);
	ui_component_enablealign(&self->bar);
	ui_component_setalign(&self->bar, UI_ALIGN_LEFT);
	ui_button_init(&self->add, &self->bar);
	ui_button_settext(&self->add, "<- Copy");
	ui_component_setalign(&self->add.component, UI_ALIGN_TOP);
	psy_signal_connect(&self->add.signal_clicked, self,
		samplessongimportview_oncopy);
}

void samplessongimportview_ondestroy(SamplesSongImportView* self,
	ui_component* sender)
{
	if (self->source) {
		song_dispose(self->source);
		free(self->source);
	}
	self->source = 0;
}

void samplessongimportview_onloadsong(SamplesSongImportView* self,
	ui_component* sender)
{
	char path[MAX_PATH]	 = "";
	char title[MAX_PATH]	 = ""; 					
	static char filter[] = "All Songs (*.psy *.xm *.it *.s3m *.mod)" "\0*.psy;*.xm;*.it;*.s3m;*.mod\0"
				"Songs (*.psy)"				        "\0*.psy\0"
				"FastTracker II Songs (*.xm)"       "\0*.xm\0"
				"Impulse Tracker Songs (*.it)"      "\0*.it\0"
				"Scream Tracker Songs (*.s3m)"      "\0*.s3m\0"
				"Original Mod Format Songs (*.mod)" "\0*.mod\0";
	char  defaultextension[] = "PSY";
	int showsonginfo = 0;	
	*path = '\0'; 
	if (ui_openfile(&self->component, title, filter, defaultextension, 
		workspace_songs_directory(self->workspace), path)) {
		SongFile songfile;
		if (self->source) {
			song_dispose(self->source);
			free(self->source);
		}	
		self->source = song_allocinit(&self->workspace->machinefactory);
		songfile.song = self->source;
		songfile.file = 0;		
		songfile_load(&songfile, path);
		if (!songfile.err) {
			ui_label_settext(&self->songname,
				self->source->properties.title);
			samplesbox_setsamples(&self->samplesbox, &self->source->samples,
				&self->source->instruments);			
		} else {
			ui_label_settext(&self->songname,
				"No source song loaded");
		}
	}		
}

void samplessongimportview_oncopy(SamplesSongImportView* self,
	ui_component* sender) {
	SampleIndex src;
	SampleIndex dst;
	Sample* sample;
	Sample* samplecopy;
	Instrument* instrument;
	
	src = samplesbox_selected(&self->samplesbox);
	dst = samplesbox_selected(&self->view->samplesbox);	
	sample = samples_at(&self->source->samples, src);
	if (sample) {
		samplecopy = sample_clone(sample);
		samples_insert(&self->workspace->song->samples, samplecopy, dst);
		instrument = (Instrument*)malloc(sizeof(Instrument));		
		instrument_init(instrument);
		instrument_setname(instrument, sample_name(samplecopy));
		instruments_insert(&self->workspace->song->instruments, instrument,
			dst.slot);
		samplesview_setsample(self->view, dst);
		/*signal_prevent(&self->workspace->song->instruments.signal_slotchange,
			self->view, OnInstrumentSlotChanged);
		instruments_changeslot(&self->workspace->song->instruments, dstslot);
		signal_enable(&self->workspace->song->instruments.signal_slotchange, self,
			OnInstrumentSlotChanged);	*/
		ui_component_invalidate(&self->view->component);	
	}
}

void samplesview_init(SamplesView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{
	ui_margin margin;
	ui_margin waveboxmargin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(2.0),
		ui_value_makepx(0), ui_value_makepx(0));
	ui_margin_init(&waveboxmargin, ui_value_makeeh(0.5), ui_value_makeew(1.0),
		ui_value_makepx(0), ui_value_makeeh(0.5));
	self->workspace = workspace;	
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);	
	samplesheaderview_init(&self->header, &self->component,
		&workspace->song->instruments, self);
	ui_component_setalign(&self->header.component, UI_ALIGN_TOP);
	psy_signal_connect(&self->component.signal_keydown, self,
		samplesview_onkeydown);
	// left
	ui_component_init(&self->left, &self->component);
	ui_component_enablealign(&self->left);
	ui_component_setalign(&self->left, UI_ALIGN_LEFT);
	ui_component_setmargin(&self->left, &margin);
	samplesviewbuttons_init(&self->buttons, &self->left);
	ui_component_setalign(&self->buttons.component, UI_ALIGN_TOP);
	// tabbarparent
	tabbar_init(&self->clienttabbar, tabbarparent);
	ui_component_setalign(&self->clienttabbar.component, UI_ALIGN_LEFT);
	ui_component_hide(&self->clienttabbar.component);
	tabbar_append(&self->clienttabbar, "Properties");
	tabbar_append(&self->clienttabbar, "Song Import");
	tabbar_append(&self->clienttabbar, "Editor");	
	samplesbox_init(&self->samplesbox, &self->left,
		&workspace->song->samples, &workspace->song->instruments);	
	ui_component_setalign(&self->samplesbox.component,
		UI_ALIGN_CLIENT);
	psy_signal_connect(&self->buttons.load.signal_clicked, self,
		samplesview_onloadsample);	
	psy_signal_connect(&self->buttons.save.signal_clicked, self,
		samplesview_onsavesample);
	psy_signal_connect(&self->buttons.duplicate.signal_clicked, self,
		samplesview_onduplicatesample);
	psy_signal_connect(&self->buttons.del.signal_clicked, self,
		samplesview_ondeletesample);	
	psy_signal_connect(&self->component.signal_show, self,
		samplesview_onshow);
	psy_signal_connect(&self->component.signal_hide, self,
		samplesview_onhide);	
	// client
	ui_notebook_init(&self->clientnotebook, &self->component);	
	ui_component_setalign(&self->clientnotebook.component, UI_ALIGN_CLIENT);

	ui_component_init(&self->client, &self->clientnotebook.component);
	ui_component_enablealign(&self->client);
	tabbar_init(&self->tabbar, &self->client);
	ui_component_resize(&self->tabbar.component, 0, 20);
	ui_component_setalign(&self->tabbar.component, UI_ALIGN_TOP);
	ui_component_setmargin(&self->tabbar.component, &margin);
	tabbar_append(&self->tabbar, "General");
	tabbar_append(&self->tabbar, "Vibrato");

	ui_notebook_init(&self->notebook, &self->client);
	ui_component_enablealign(&self->notebook.component);
	ui_component_setalign(&self->notebook.component, UI_ALIGN_TOP);	
	ui_component_setbackgroundmode(&self->notebook.component, BACKGROUND_SET);	
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	InitSamplesGeneralView(&self->general, &self->notebook.component);
	ui_component_setalign(&self->general.component, UI_ALIGN_TOP);
	InitSamplesVibratoView(&self->vibrato, &self->notebook.component,
		&workspace->player);
	ui_component_setalign(&self->vibrato.component, UI_ALIGN_TOP);
	ui_notebook_setpageindex(&self->notebook, 0);	
	wavebox_init(&self->wavebox, &self->client);	
	ui_component_setalign(&self->wavebox.component, UI_ALIGN_CLIENT);
	ui_component_setmargin(&self->wavebox.component, &waveboxmargin);
	samplesloopview_init(&self->waveloop, &self->component, self);	
	ui_component_setalign(&self->waveloop.component, UI_ALIGN_BOTTOM);
	ui_component_setmargin(&self->waveloop.component, &margin);
	psy_signal_connect(&self->samplesbox.signal_changed, self,
		samplesview_onsampleboxchanged);
	psy_signal_connect(&workspace->song->instruments.signal_slotchange, self,
		samplesview_oninstrumentslotchanged);
	samplesview_setsample(self, sampleindex_make(0, 0));	
	samplessongimportview_init(&self->songimport,
		&self->clientnotebook.component, self, workspace);
	sampleeditor_init(&self->sampleeditor,
		&self->clientnotebook.component, workspace);	
	ui_notebook_setpageindex(&self->clientnotebook, 0);
	psy_signal_connect(&workspace->signal_songchanged, self,
		samplesview_onsongchanged);
	ui_notebook_setpageindex(&self->clientnotebook, 0);
	ui_notebook_connectcontroller(&self->clientnotebook,
		&self->clienttabbar.signal_change);
}

void samplesview_onsampleboxchanged(SamplesView* self, ui_component* sender)
{		
	SampleIndex index;
	
	index = samplesbox_selected(&self->samplesbox);
	samplesview_setsample(self, index);
	if (self->workspace->song) {
		psy_signal_prevent(&self->workspace->song->instruments.signal_slotchange,
			self, samplesview_oninstrumentslotchanged);		
		instruments_changeslot(&self->workspace->song->instruments,
			index.slot);
		psy_signal_enable(
			&self->workspace->song->instruments.signal_slotchange,
			self, samplesview_oninstrumentslotchanged);		
	}
	ui_component_invalidate(&self->wavebox.component);
}

void samplesview_oninstrumentslotchanged(SamplesView* self, Instrument* sender,
	int slot)
{
	SampleIndex index;
	
	index = samplesbox_selected(&self->samplesbox);
	if (index.slot != (uintptr_t) slot) {
		index.subslot = 0;
	}
	index.slot = slot;
	samplesview_setsample(self, index);
}

void samplesview_setsample(SamplesView* self, SampleIndex index)
{	
	Sample* sample;	

	sample = self->workspace->song
		? samples_at(&self->workspace->song->samples, index)
		: 0;
	wavebox_setsample(&self->wavebox, sample);
	sampleeditor_setsample(&self->sampleeditor, sample);
	samplesheaderview_setsample(&self->header, sample);
	SetSampleSamplesGeneralView(&self->general, sample);
	SetSampleSamplesVibratoView(&self->vibrato, sample);
	samplesloopview_setsample(&self->waveloop, sample);
	ui_listbox_setcursel(&self->samplesbox.samplelist, index.slot);
	ui_listbox_setcursel(&self->samplesbox.subsamplelist, index.subslot);
}

void samplesview_onloadsample(SamplesView* self, ui_component* sender)
{	
	if (self->workspace->song) {
		char path[MAX_PATH]	 = "";
		char title[MAX_PATH] = ""; 
		static char filter[] = "Wav Files (*.wav)\0*.wav\0" "IFF Samples (*.iff)\0*.iff\0" "All Files (*.*)\0*.*\0";
		char  defaultextension[] = "WAV";

		*path = '\0'; 
		if (ui_openfile(&self->component, title, filter, defaultextension, 0, path)) {			
			Sample* sample;
			SampleIndex index;
			Instrument* instrument;

			sample = sample_allocinit();
			sample_load(sample, path);
			index = samplesbox_selected(&self->samplesbox);
			samples_insert(&self->workspace->song->samples, sample, index);
			instrument = instrument_allocinit();
			instrument_setname(instrument, sample_name(sample));
			instruments_insert(&self->workspace->song->instruments, instrument,
				index.slot);
			samplesview_setsample(self, index);
			psy_signal_prevent(
				&self->workspace->song->instruments.signal_slotchange,
				self, samplesview_oninstrumentslotchanged);
			instruments_changeslot(&self->workspace->song->instruments, 
				index.slot);
			psy_signal_enable(
				&self->workspace->song->instruments.signal_slotchange,
				self, samplesview_oninstrumentslotchanged);
			ui_component_invalidate(&self->component);
		}
	}
}

void samplesview_onsavesample(SamplesView* self, ui_component* sender)
{	
	char path[MAX_PATH]	 = "";
	char title[MAX_PATH]	 = ""; 
	static char filter[]	 = "Wav Files (*.wav)\0*.wav\0" "IFF Samples (*.iff)\0*.iff\0" "All Files (*.*)\0*.*\0";
	char  defaultextension[] = "WAV";
	Sample* sample;

	*path = '\0'; 
	sample = self->wavebox.sample;
	if (sample && ui_savefile(&self->component, title, filter,
		defaultextension, 0, path)) {		
			sample_save(sample, path);		
	}	
}

void samplesview_ondeletesample(SamplesView* self, ui_component* sender)
{	
	if (self->workspace->song) {
		SampleIndex index;
			
		index = samplesbox_selected(&self->samplesbox);
		lock_enter();
		samples_remove(&self->workspace->song->samples, index);
		instruments_remove(&self->workspace->song->instruments,
			index.subslot);
		samplesview_setsample(self, index);
		lock_leave();
	}
}

void samplesview_onduplicatesample(SamplesView* self, ui_component* sender)
{	
	if (self->workspace->song) {
		SampleIndex src;
		SampleIndex dst;

		src = samplesbox_selected(&self->samplesbox);
		dst.slot = samplesview_freesampleslot(self, src.slot, 256);
		dst.subslot = 0;
		if (dst.slot != 256) {
			Sample* source;

			source = samples_at(&self->workspace->song->samples, src);
			if (source) {
				Sample* copy;
				Instrument* instrument;

				copy = sample_clone(source);
				samples_insert(&self->workspace->song->samples, copy, dst);
				instrument = instrument_allocinit();
				instrument_setname(instrument, sample_name(copy));
				instruments_insert(&self->workspace->song->instruments,
					instrument, dst.slot);
				samplesview_setsample(self, sampleindex_make(dst.slot, 0));
			}
		}
	}
}

uintptr_t samplesview_freesampleslot(SamplesView* self, uintptr_t startslot,
	uintptr_t maxslots)
{	
	uintptr_t rv = startslot;

	if (self->workspace->song) {
		int first = startslot != 0;

		while (samples_at(&self->workspace->song->samples,
				sampleindex_make(rv, 0)) != 0) {
			if (rv == maxslots) {
				if (first) {
					rv = 0;
				} else {
					rv = maxslots;
					break;
				}
			}
			++rv;
		}
	} else {
		rv = maxslots;
	}
	return rv;
}

void samplesview_onsongchanged(SamplesView* self, Workspace* workspace)
{	
	psy_signal_connect(&workspace->song->instruments.signal_slotchange, self,
		samplesview_oninstrumentslotchanged);	
	samplesbox_setsamples(&self->samplesbox, &workspace->song->samples,
		&workspace->song->instruments);
	samplesview_setsample(self, sampleindex_make(0, 0));
}

void samplesview_onshow(SamplesView* self, ui_component* sender)
{	
	self->clienttabbar.component.visible = 1;
	ui_component_align(ui_component_parent(&self->clienttabbar.component));
	ui_component_show(&self->clienttabbar.component);
}

void samplesview_onhide(SamplesView* self, ui_component* sender)
{
	ui_component_hide(&self->clienttabbar.component);
}

void samplesview_onkeydown(SamplesView* self, ui_component* sender, KeyEvent* ev)
{
	ui_component_propagateevent(&self->component);
}


void samplesheaderview_init(SamplesHeaderView* self, ui_component* parent,
	Instruments* instruments, struct SamplesView* view)
{
	ui_margin margin;

	self->view = view;
	self->instruments = instruments;
	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(0.5),
		ui_value_makeeh(0.5), ui_value_makepx(0));
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_label_init(&self->namelabel, &self->component);
	ui_label_settext(&self->namelabel, "Sample Name");
	ui_edit_init(&self->nameedit, &self->component, 0);		
	ui_edit_setcharnumber(&self->nameedit, 20);	
	psy_signal_connect(&self->nameedit.signal_change, self,
		OnEditSampleName);
	ui_button_init(&self->prevbutton, &self->component);
	ui_button_seticon(&self->prevbutton, UI_ICON_LESS);	
	psy_signal_connect(&self->prevbutton.signal_clicked, self,
		OnPrevSample);
	ui_button_init(&self->nextbutton, &self->component);
	ui_button_seticon(&self->nextbutton, UI_ICON_MORE);	
	psy_signal_connect(&self->nextbutton.signal_clicked, self,
		OnNextSample);
	ui_label_init(&self->srlabel, &self->component);
	ui_label_settext(&self->srlabel, "Sample Rate");	
	ui_edit_init(&self->sredit, &self->component, 0);	
	ui_edit_setcharnumber(&self->sredit, 8);
	ui_label_init(&self->numsamplesheaderlabel, &self->component);
	ui_label_settext(&self->numsamplesheaderlabel, "Samples");	
	ui_label_init(&self->numsampleslabel, &self->component);
	ui_label_setcharnumber(&self->numsampleslabel, 10);
	ui_label_init(&self->channellabel, &self->component);
	ui_label_settext(&self->channellabel, "");
	ui_label_setcharnumber(&self->channellabel, 6);						
	list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT,
			&margin));	
}

void samplesheaderview_setsample(SamplesHeaderView* self, Sample* sample)
{
	char text[20];

	self->sample = sample;
	ui_edit_settext(&self->nameedit, self->sample ? sample->name : "");	
	psy_snprintf(text, 20, "%d", self->sample ? self->sample->samplerate : 0);
	ui_edit_settext(&self->sredit, text);
	psy_snprintf(text, 20, "%d", self->sample ? self->sample->numframes : 0);
	ui_label_settext(&self->numsampleslabel, text);
	if (self->sample) {
		switch (buffer_numchannels(&self->sample->channels)) {
			case 0:
				psy_snprintf(text, 20, "");
			break;
			case 1:
				psy_snprintf(text, 20, "Mono");
			break;
			case 2:		
				psy_snprintf(text, 20, "Stereo");
			break;
			default:					
				psy_snprintf(text, 20, "%d Chs",
					buffer_numchannels(&self->sample->channels));
			break;
		}		
	} else {
		psy_snprintf(text, 20, "");
	}
	ui_label_settext(&self->channellabel, text);
	if (self->sample) {
		ui_component_enableinput(&self->component, 1);
	} else {
		ui_component_preventinput(&self->component, 1);
	}
}

void OnEditSampleName(SamplesHeaderView* self, ui_edit* sender)
{
	if (self->sample) {
		char text[40];
		SampleIndex index;

		index = samplesbox_selected(&self->view->samplesbox);
		sample_setname(self->sample, ui_edit_text(sender));		
		if (index.subslot == 0) {
			psy_snprintf(text, 20, "%02X:%s", 
			(int)index.slot, sample_name(self->sample));
			ui_listbox_setstring(&self->view->samplesbox.samplelist, text, 
				index.slot);
		}
		psy_snprintf(text, 20, "%02X:%s", 
			(int)index.subslot, sample_name(self->sample));
		ui_listbox_setstring(&self->view->samplesbox.subsamplelist, text,
			index.subslot);
	}
}

void OnPrevSample(SamplesHeaderView* self, ui_component* sender)
{
	instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) > 0 ?
		instruments_slot(self->instruments) - 1 : 0);
}

void OnNextSample(SamplesHeaderView* self, ui_component* sender)
{
	instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) < 255 ?
		instruments_slot(self->instruments) + 1 : 255);
}

void InitSamplesGeneralView(SamplesGeneralView* self, ui_component* parent)
{	
	ui_margin margin;
	ui_slider* sliders[] = {
		&self->defaultvolume,
		&self->globalvolume,
		&self->panposition,
		&self->samplednote,
		&self->pitchfinetune,
		0
	};	
	int i;
		
	self->sample = 0;
	ui_margin_init(&margin, ui_value_makeeh(1), ui_value_makepx(0),
		ui_value_makepx(0), ui_value_makepx(0));			
	self->notestabmode = psy_dsp_NOTESTAB_DEFAULT;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_slider_init(&self->defaultvolume, &self->component);
	ui_slider_settext(&self->defaultvolume, "Default Volume");
	ui_slider_init(&self->globalvolume, &self->component);
	ui_slider_settext(&self->globalvolume, "Global Volume");
	ui_slider_init(&self->panposition, &self->component);
	ui_slider_settext(&self->panposition, "Pan Position");
	ui_slider_init(&self->samplednote, &self->component);
	ui_slider_settext(&self->samplednote, "Sampled Note"); 
	ui_slider_init(&self->pitchfinetune, &self->component);
	ui_slider_settext(&self->pitchfinetune, "Pitch Finetune");	
	for (i = 0; sliders[i] != 0; ++i) {
		ui_component_resize(&sliders[i]->component, 0, 20);
		ui_component_setalign(&sliders[i]->component, UI_ALIGN_TOP);		
		ui_component_setmargin(&sliders[i]->component, &margin);		
		ui_slider_connect(sliders[i], self, OnGeneralViewDescribe,
			OnGeneralViewTweak, OnGeneralViewValue);		
	}
}

void SetSampleSamplesGeneralView(SamplesGeneralView* self, Sample* sample)
{
	self->sample = sample;
	if (self->sample) {
		ui_component_enableinput(&self->component, 1);
	} else {
		ui_component_preventinput(&self->component, 1);
	}
}

int map_1_128(float value) {
	return (int)(value * 128.f);
}

void OnGeneralViewTweak(SamplesGeneralView* self, ui_slider* slider,
	float value)
{
	if (self->sample) {			
		if (slider == &self->defaultvolume) {
			self->sample->defaultvolume = value;
		} else
		if (slider == &self->globalvolume) {		
			self->sample->globalvolume = (value * value) * 4.f;
		} else
		if (slider == &self->panposition) {
			self->sample->panfactor = value;
		} else
		if (slider == &self->samplednote) {
			self->sample->tune = (int)(value * 119.f) - 60;
		} else
		if (slider == &self->pitchfinetune) {
			self->sample->finetune = (int)(value * 200.f) - 100;
		}
	}
}

void OnGeneralViewValue(SamplesGeneralView* self, ui_slider* slider,
	float* value)
{	
	if (slider == &self->defaultvolume) {
		*value = self->sample ? self->sample->defaultvolume : 1.0f;
	} else 
	if (slider == &self->globalvolume) {
		if (self->sample) {			
			*value = (float)sqrt(self->sample->globalvolume) * 0.5f;
		} else {
			*value = 0.5f;
		}
	} else 	
	if (slider == &self->panposition) {
		*value = self->sample ? self->sample->panfactor : 0.5f;
	} else
	if (slider == &self->samplednote) {
		*value = self->sample ? (self->sample->tune + 60) / 119.f : 0.5f;
	} else
	if (slider == &self->pitchfinetune) {
		*value = self->sample ? self->sample->finetune / 200.f + 0.5f : 0.f;
	}
}

void OnGeneralViewDescribe(SamplesGeneralView* self, ui_slider* slider, char* txt)
{		
	if (slider == &self->defaultvolume) {		
		psy_snprintf(txt, 10, "C%02X", self->sample 
			? map_1_128(self->sample->defaultvolume)
			: 0x80);
	} else
	if (slider == &self->globalvolume) {		
		if (!self->sample) {
			psy_snprintf(txt, 10, "-inf. dB");
		} else
		if (self->sample->globalvolume == 0) {
			psy_snprintf(txt, 10, "-inf. dB");
		} else {
			float db = (float)(20 * log10(self->sample->globalvolume));
			psy_snprintf(txt, 10, "%.2f dB", db);
		}
	} else
	if (slider == &self->panposition) {		
		GeneralViewFillPanDescription(self, txt);		
	} else
	if (slider == &self->samplednote) {		
		psy_snprintf(txt, 10, "%s", self->sample
			? psy_dsp_notetostr((note_t)(self->sample->tune + 60), self->notestabmode)
			: psy_dsp_notetostr(60, self->notestabmode));		
	} else
	if (slider == &self->pitchfinetune) {
		psy_snprintf(txt, 10, "%d ct.", self->sample
			? self->sample->finetune
			: 0);
	}
}

void GeneralViewFillPanDescription(SamplesGeneralView* self, char* txt) {	

	if (!self->sample) {
		psy_snprintf(txt, 10, "|64|");
	} else
	if (self->sample->surround) {
		psy_snprintf(txt, 10, "SurrounD");
	} else {		
		int pos = (int)(self->sample->panfactor * 128.f);
		if (pos == 0) {
			psy_snprintf(txt, 128, "||%02d  ", pos);
		} else
		if (pos < 32) {
			psy_snprintf(txt, 128, "<<%02d  ", pos);
		} else
		if (pos < 64) {
			psy_snprintf(txt, 128, " <%02d< ", pos);
		} else 
		if (pos == 64) {
			psy_snprintf(txt, 128, " |%02d| ", pos); 
		} else
		if (pos <= 96) {
			psy_snprintf(txt, 128, " >%02d> ", pos);
		} else
		if (pos < 128) {
			psy_snprintf(txt, 128, "  %02d>>", pos);
		} else {
			psy_snprintf(txt, 128, "  %02d||", pos);
		}
	}	
}

void InitSamplesVibratoView(SamplesVibratoView* self, ui_component* parent, Player* player)
{
	ui_margin margin;
	int i;
	ui_slider* sliders[] = {
		&self->attack,
		&self->speed,
		&self->depth,		
	};	

	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);	
	self->sample = 0;
	self->player = player;	

	ui_label_init(&self->waveformheaderlabel, &self->component);
	ui_label_settext(&self->waveformheaderlabel, "Waveform");
	ui_component_setposition(&self->waveformheaderlabel.component, 5, 5, 100, 20);

	ui_combobox_init(&self->waveformbox, &self->component);	
	ui_combobox_addstring(&self->waveformbox, "Sinus");
	ui_combobox_addstring(&self->waveformbox, "Square");
	ui_combobox_addstring(&self->waveformbox, "RampUp");
	ui_combobox_addstring(&self->waveformbox, "RampDown");
	ui_combobox_addstring(&self->waveformbox, "Random");	
	ui_component_setposition(&self->waveformbox.component, 110, 5, 100, 20);
	ui_combobox_setcursel(&self->waveformbox, 0);
	psy_signal_connect(&self->waveformbox.signal_selchanged, self,
		OnWaveFormChange);

	ui_slider_init(&self->attack, &self->component);
	ui_slider_settext(&self->attack, "Attack");	
	ui_slider_init(&self->speed, &self->component);
	ui_slider_settext(&self->speed,"Speed");
	ui_slider_init(&self->depth, &self->component);
	ui_slider_settext(&self->depth, "Depth");
	ui_margin_init(&margin,
		ui_value_makepx(3),
		ui_value_makepx(3),
		ui_value_makepx(0),
		ui_value_makepx(3));
	for (i = 0; i < 3; ++i) {		
		ui_component_resize(&sliders[i]->component, 0, 20);		
		ui_component_setalign(&sliders[i]->component, UI_ALIGN_TOP);
		ui_component_setmargin(&sliders[i]->component, &margin);
		ui_slider_connect(sliders[i], self, OnVibratoViewDescribe,
			OnVibratoViewTweak, OnVibratoViewValue);		
	}	
	sliders[0]->component.margin.top.quantity.integer = 32;		
}

void SetSampleSamplesVibratoView(SamplesVibratoView* self, Sample* sample)
{
	self->sample = sample;
	if (self->sample) {
		ui_component_enableinput(&self->component, 1);
		ui_combobox_setcursel(&self->waveformbox,
			WaveFormToComboBox(self->sample->vibrato.type));
	} else {
		ui_component_preventinput(&self->component, 1);
		ui_combobox_setcursel(&self->waveformbox,
			WaveFormToComboBox(WAVEFORMS_SINUS));
	}
}

void OnVibratoViewTweak(SamplesVibratoView* self, ui_slider* slidergroup, float value)
{
	if (!self->sample) {
		return;
	}
	if (slidergroup == &self->attack) {
		self->sample->vibrato.attack = (unsigned char)(value * 255.f);
	} else
	if (slidergroup == &self->speed) {
		self->sample->vibrato.speed = (unsigned char)(value * 64.f);
	} else
	if (slidergroup == &self->depth) {
		self->sample->vibrato.depth = (unsigned char)(value * 32.f);
	}
}

void OnVibratoViewValue(SamplesVibratoView* self, ui_slider* slidergroup,
	float* value)
{	
	if (slidergroup == &self->attack) {
		*value = self->sample ? self->sample->vibrato.attack / 255.f : 0.f;
	} else 
	if (slidergroup == &self->speed) {
		*value = self->sample ? self->sample->vibrato.speed / 64.f : 0.f;
	} else
	if (slidergroup == &self->depth) {
		*value = self->sample ? self->sample->vibrato.depth / 32.f : 0.f;
	}
}

void OnVibratoViewDescribe(SamplesVibratoView* self, ui_slider* slidergroup, char* txt)
{		
	if (slidergroup == &self->attack) {		
		if (!self->sample) {
			psy_snprintf(txt, 10, "No Delay");
		} else 
		if (self->sample->vibrato.attack == 0) {
			psy_snprintf(txt, 10, "No Delay");
		} else {
			psy_snprintf(txt, 10, "%.0fms", (4096000.0f*256)
				/(self->sample->vibrato.attack*44100.f));
		}		
	} else
	if (slidergroup == &self->speed) {		
		if (!self->sample) {
			psy_snprintf(txt, 10, "off");
		} else
		if (self->sample->vibrato.speed == 0) {
			psy_snprintf(txt, 10, "off");			
		} else {		
			psy_snprintf(txt, 10, "%.0fms", (256000.0f*256) 
				/ (self->sample->vibrato.speed*44100.f));
		}
	} else
	if (slidergroup == &self->depth) {		
		if (!self->sample) {
			psy_snprintf(txt, 10, "off");
		} else
		if (self->sample->vibrato.depth == 0) {
			psy_snprintf(txt, 10, "off");			
		} else {
			psy_snprintf(txt, 10, "%d", self->sample->vibrato.depth);
		}
	}
}

void OnWaveFormChange(SamplesVibratoView* self, ui_combobox* sender, int sel)
{
	if (self->sample) {
		self->sample->vibrato.type = ComboBoxToWaveForm(sel);
	}
}

int WaveFormToComboBox(WaveForms waveform)
{
	int rv = 0;

	switch (waveform) {
		case WAVEFORMS_SINUS: rv = 0; break;		
		case WAVEFORMS_SQUARE: rv = 1; break;					
		case WAVEFORMS_SAWUP: rv = 2; break;					
		case WAVEFORMS_SAWDOWN: rv = 3; break;					
		case WAVEFORMS_RANDOM: rv = 4; break;					
		default:
		break;		
	}
	return rv;
}

WaveForms ComboBoxToWaveForm(int combobox_index)
{
	WaveForms rv = WAVEFORMS_SINUS;			
	
	switch (combobox_index) {					
		case 0: rv = WAVEFORMS_SINUS; break;
		case 1: rv = WAVEFORMS_SQUARE; break;
		case 2: rv = WAVEFORMS_SAWUP; break;
		case 3: rv = WAVEFORMS_SAWDOWN; break;
		case 4: rv = WAVEFORMS_RANDOM; break;
		default:
		break;				
	}
	return rv;
}

void samplesloopview_init(SamplesLoopView* self, ui_component* parent,
	SamplesView* view)
{
	ui_margin margin;
	ui_margin rowmargin;
	
	self->view = view;
	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(2),
		ui_value_makepx(0), ui_value_makepx(0));
	ui_margin_init(&rowmargin, ui_value_makeew(1.5), ui_value_makepx(0),
		ui_value_makeew(1.5), ui_value_makepx(0));
	self->sample = 0;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_init(&self->cont, &self->component);
	ui_component_enablealign(&self->cont);
	ui_component_setmargin(&self->cont, &rowmargin);
	ui_label_init(&self->loopheaderlabel, &self->cont);	
	ui_label_settext(&self->loopheaderlabel, "Continuous Loop");
	ui_label_setcharnumber(&self->loopheaderlabel, 18);	
	ui_combobox_init(&self->loopdir, &self->cont);
	ui_combobox_addstring(&self->loopdir, "Disabled");
	ui_combobox_addstring(&self->loopdir, "Forward");
	ui_combobox_addstring(&self->loopdir, "Bidirection");	
	ui_combobox_setcursel(&self->loopdir, 0);
	ui_combobox_setcharnumber(&self->loopdir, 10);
	ui_label_init(&self->loopstartlabel, &self->cont);
	ui_label_settext(&self->loopstartlabel, "Start ");	
	ui_edit_init(&self->loopstartedit, &self->cont, 0);
	ui_edit_setcharnumber(&self->loopstartedit, 10);
	ui_label_init(&self->loopendlabel, &self->cont);
	ui_label_settext(&self->loopendlabel, "End ");	
	ui_edit_init(&self->loopendedit, &self->cont, 0);
	ui_edit_setcharnumber(&self->loopendedit, 10);
	list_free(ui_components_setalign(
		ui_component_children(&self->cont, 0),
		UI_ALIGN_LEFT,
		&margin));
	ui_component_init(&self->sustain, &self->component);	
	ui_component_enablealign(&self->sustain);
	ui_label_init(&self->sustainloopheaderlabel, &self->sustain);
	ui_label_settext(&self->sustainloopheaderlabel, "Sustain Loop");
	ui_label_setcharnumber(&self->sustainloopheaderlabel, 18);
	ui_combobox_init(&self->sustainloopdir, &self->sustain);
	ui_combobox_addstring(&self->sustainloopdir, "Disabled");
	ui_combobox_addstring(&self->sustainloopdir, "Forward");
	ui_combobox_addstring(&self->sustainloopdir, "Bidirection");		
	ui_combobox_setcursel(&self->sustainloopdir, 0);
	ui_combobox_setcharnumber(&self->sustainloopdir, 10);
	ui_label_init(&self->sustainloopstartlabel, &self->sustain);
	ui_label_settext(&self->sustainloopstartlabel, "Start ");	
	ui_edit_init(&self->sustainloopstartedit, &self->sustain, 0);		
	ui_edit_setcharnumber(&self->sustainloopstartedit, 10);
	ui_label_init(&self->sustainloopendlabel, &self->sustain);	
	ui_label_settext(&self->sustainloopendlabel, "End ");	
	ui_edit_init(&self->sustainloopendedit, &self->sustain, 0);
	ui_edit_setcharnumber(&self->sustainloopendedit, 10);
	list_free(ui_components_setalign(
		ui_component_children(&self->sustain, 0),
		UI_ALIGN_LEFT,
		&margin));
	list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_TOP,
		0));
	psy_signal_connect(&self->loopdir.signal_selchanged, self,
		samplesloopview_onlooptypechange);
	psy_signal_connect(&self->sustainloopdir.signal_selchanged, self,
		samplesloopview_onsustainlooptypechange);
	psy_signal_connect(&self->loopstartedit.signal_change, self,
		samplesloopview_oneditchangedloopstart);
	psy_signal_connect(&self->loopendedit.signal_change, self,
		samplesloopview_oneditchangedloopend);
	psy_signal_connect(&self->sustainloopstartedit.signal_change, self,
		samplesloopview_oneditchangedsustainstart);
	psy_signal_connect(&self->sustainloopendedit.signal_change, self,
		samplesloopview_oneditchangedsustainend);
}

void samplesloopview_setsample(SamplesLoopView* self, Sample* sample)
{
	char tmp[40];

	self->sample = sample;
	if (self->sample) {
		ui_component_enableinput(&self->component, 1);
		sprintf(tmp, "%d", sample->loopstart);
		ui_edit_settext(&self->loopstartedit, tmp);
		sprintf(tmp, "%d", sample->loopend);
		ui_edit_settext(&self->loopendedit, tmp);
		sprintf(tmp, "%d", sample->sustainloopstart);
		ui_edit_settext(&self->sustainloopstartedit, tmp);
		sprintf(tmp, "%d", sample->sustainloopend);
		ui_edit_settext(&self->sustainloopendedit, tmp);
		ui_combobox_setcursel(&self->loopdir,
			LoopTypeToComboBox(self->sample->looptype));
		ui_combobox_setcursel(&self->sustainloopdir,
			LoopTypeToComboBox(self->sample->sustainlooptype));
	} else {
		ui_component_preventinput(&self->component, 1);
		sprintf(tmp, "%d", 0);
		ui_edit_settext(&self->loopstartedit, tmp);		
		ui_edit_settext(&self->loopendedit, tmp);		
		ui_edit_settext(&self->sustainloopstartedit, tmp);		
		ui_edit_settext(&self->sustainloopendedit, tmp);
		ui_combobox_setcursel(&self->loopdir,
			LoopTypeToComboBox(LOOP_DO_NOT));
		ui_combobox_setcursel(&self->sustainloopdir,
			LoopTypeToComboBox(LOOP_DO_NOT));		
	}
	samplesloopview_looptypeenablepreventinput(self);
}

int LoopTypeToComboBox(LoopType looptype)
{
	int rv = 0;

	switch (looptype) {
		case LOOP_DO_NOT: rv = 0; break;
		case LOOP_NORMAL: rv = 1; break;
		case LOOP_BIDI: rv = 2; break;
		default:
		break;
	}
	return rv;
}

void samplesloopview_onlooptypechange(SamplesLoopView* self, ui_combobox* sender, int sel)
{
	if (self->sample) {
		self->sample->looptype = ComboBoxToLoopType(sel);
		ui_component_invalidate(&self->view->wavebox.component);
		samplesloopview_looptypeenablepreventinput(self);
	}
}

void samplesloopview_onsustainlooptypechange(SamplesLoopView* self, ui_combobox* sender, int sel)
{
	if (self->sample) {
		self->sample->sustainlooptype = ComboBoxToLoopType(sel);
		ui_component_invalidate(&self->view->wavebox.component);
		samplesloopview_looptypeenablepreventinput(self);
	}
}

void samplesloopview_looptypeenablepreventinput(SamplesLoopView* self)
{
	if (self->sample) {
		if (self->sample->looptype == LOOP_DO_NOT) {
			ui_component_preventinput(&self->loopstartedit.component, 0);
			ui_component_preventinput(&self->loopendedit.component, 0);
		} else {
			ui_component_enableinput(&self->loopstartedit.component, 0);
			ui_component_enableinput(&self->loopendedit.component, 0);
		}
		if (self->sample->sustainlooptype == LOOP_DO_NOT) {
			ui_component_preventinput(&self->sustainloopstartedit.component, 0);
			ui_component_preventinput(&self->sustainloopendedit.component, 0);
		} else {
			ui_component_enableinput(&self->sustainloopstartedit.component, 0);
			ui_component_enableinput(&self->sustainloopendedit.component, 0);
		}
	} else {
		ui_component_preventinput(&self->loopstartedit.component, 0);
		ui_component_preventinput(&self->loopendedit.component, 0);
		ui_component_preventinput(&self->sustainloopstartedit.component, 0);
		ui_component_preventinput(&self->sustainloopendedit.component, 0);	
	}
}

LoopType ComboBoxToLoopType(int combobox_index)
{
	LoopType rv = 0;
			
	switch (combobox_index) {			
		case 0: rv = LOOP_DO_NOT; break;
		case 1: rv = LOOP_NORMAL; break;
		case 2: rv = LOOP_BIDI; break;
		default:
		break;
	}
	return rv;
}

void samplesloopview_oneditchangedloopstart(SamplesLoopView* self, ui_edit* sender)
{
	if (self->sample) {
		self->sample->loopstart = atoi(ui_edit_text(sender));
		ui_component_invalidate(&self->view->wavebox.component);
	}
}

void samplesloopview_oneditchangedloopend(SamplesLoopView* self, ui_edit* sender)
{				
	if (self->sample) {
		self->sample->loopend = atoi(ui_edit_text(sender));
		ui_component_invalidate(&self->view->wavebox.component);
	}
}

void samplesloopview_oneditchangedsustainstart(SamplesLoopView* self, ui_edit* sender)
{
	if (self->sample) {
		self->sample->sustainloopstart = atoi(ui_edit_text(sender));
		ui_component_invalidate(&self->view->wavebox.component);
	}
}

void samplesloopview_oneditchangedsustainend(SamplesLoopView* self, ui_edit* sender)
{
	if (self->sample) {
		self->sample->sustainloopend = atoi(ui_edit_text(sender));		
		ui_component_invalidate(&self->view->wavebox.component);
	}
}

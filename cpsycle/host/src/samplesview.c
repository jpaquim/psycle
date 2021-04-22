// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "samplesview.h"
// host
#include "styles.h"
// audio
#include <exclusivelock.h>
#include <instruments.h>
#include <songio.h>
// ui
#include <uiopendialog.h>
#include <uisavedialog.h>
// std
#include <math.h>
// file
#include <dir.h>
// platform
#include "../../detail/portable.h"

// SamplesViewButtons
// implementation
void samplesviewbuttons_init(SamplesViewButtons* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all_em(&margin, 0.0, 0.5, 1.0, 0.0);		
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	psy_ui_button_init_text(&self->load, &self->component, NULL, "file.load");
	psy_ui_button_init_text(&self->save, &self->component, NULL, "file.save");
	psy_ui_button_init_text(&self->duplicate, &self->component, NULL,
		"edit.duplicate");
	psy_ui_button_init_text(&self->del, &self->component, NULL, "edit.delete");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT, margin));	
}

// SamplesSongImportView
// prototypes
static void samplessongimportview_ondestroy(SamplesSongImportView*,
	psy_ui_Component* sender);
static void samplessongimportview_onloadsong(SamplesSongImportView*,
	psy_ui_Component* sender);
static void samplessongimportview_oncopy(SamplesSongImportView*,
	psy_ui_Component* sender);
static void samplessongimportview_onsamplesboxchanged(SamplesSongImportView*,
	psy_ui_Component* sender);
// implementation
void samplessongimportview_init(SamplesSongImportView* self, psy_ui_Component* parent,
	SamplesView* view, Workspace* workspace)
{	
	self->view = view;
	self->source = 0;
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_init(&self->header, &self->component, NULL);
	psy_ui_component_setalign(&self->header, psy_ui_ALIGN_TOP);
	psy_ui_component_setdefaultalign(&self->header, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make(psy_ui_value_make_px(0),
			psy_ui_value_make_ew(0.5), psy_ui_value_make_eh(1.0),
			psy_ui_value_make_px(0)));
	psy_ui_label_init_text(&self->label, &self->header, NULL, "Source");
	psy_ui_label_init_text(&self->songname, &self->header, NULL,
		"No song loaded");
	psy_ui_label_setcharnumber(&self->songname, 30);	
	psy_ui_button_init(&self->browse, &self->header, NULL);
	psy_ui_button_settext(&self->browse, "Select a song");
	psy_signal_connect(&self->browse.signal_clicked, self,
		samplessongimportview_onloadsong);		
	// bar
	psy_ui_component_init(&self->bar, &self->component, NULL);
	psy_ui_component_setalign(&self->bar, psy_ui_ALIGN_LEFT);		
	psy_ui_component_setminimumsize(&self->bar, psy_ui_size_make_em(12, 1));
	psy_ui_button_init(&self->add, &self->bar, NULL);
	psy_ui_button_settext(&self->add, "<- Copy");	
	psy_ui_component_setalign(&self->add.component, psy_ui_ALIGN_CENTER);
	psy_signal_connect(&self->add.signal_clicked, self,
		samplessongimportview_oncopy);
	// samplesbox
	samplesbox_init(&self->samplesbox, &self->component, NULL, workspace);
	psy_ui_component_setalign(&self->samplesbox.component,
		psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->samplesbox.signal_changed, self,
		samplessongimportview_onsamplesboxchanged);
	// samplebox
	wavebox_init(&self->samplebox, &self->component, workspace);
	psy_ui_component_setalign(&self->samplebox.component,
		psy_ui_ALIGN_BOTTOM);	
}

void samplessongimportview_ondestroy(SamplesSongImportView* self,
	psy_ui_Component* sender)
{
	if (self->source) {
		psy_audio_song_deallocate(self->source);
		self->source = NULL;
	}	
}

void samplessongimportview_onloadsong(SamplesSongImportView* self,
	psy_ui_Component* sender)
{
	psy_ui_OpenDialog dialog;
	
	psy_ui_opendialog_init_all(&dialog, 0, "Load Song",
		psy_audio_songfile_loadfilter(),
		psy_audio_songfile_standardloadfilter(),
		dirconfig_songs(&self->workspace->config.directories));
	if (psy_ui_opendialog_execute(&dialog)) {
		psy_audio_SongFile songfile;

		if (self->source) {
			psy_audio_song_deallocate(self->source);			
		}	
		self->source = psy_audio_song_allocinit(
			&self->workspace->machinefactory);
		psy_audio_songfile_init(&songfile);
		songfile.song = self->source;
		songfile.file = 0;		
		psy_audio_songfile_load(&songfile, psy_path_full(psy_ui_opendialog_path(&dialog)));
		if (!songfile.err) {
			psy_ui_label_settext(&self->songname,
				self->source->properties.title);
			samplesbox_setsamples(&self->samplesbox, &self->source->samples);			
		} else {
			psy_ui_label_settext(&self->songname,
				"No source song loaded");
		}
		psy_audio_songfile_dispose(&songfile);
	}
	psy_ui_opendialog_dispose(&dialog);
}

void samplessongimportview_oncopy(SamplesSongImportView* self,
	psy_ui_Component* sender) {
	psy_audio_SampleIndex src;
	psy_audio_SampleIndex dst;
	psy_audio_Sample* sample;
	psy_audio_Sample* samplecopy;
	psy_audio_Instrument* instrument;
	
	src = samplesbox_selected(&self->samplesbox);
	dst = samplesbox_selected(&self->view->samplesbox);	
	sample = psy_audio_samples_at(&self->source->samples, src);
	if (sample) {
		samplecopy = psy_audio_sample_clone(sample);
		psy_audio_samples_insert(&workspace_song(self->workspace)->samples, samplecopy,
			dst);
		instrument = (psy_audio_Instrument*)malloc(sizeof(psy_audio_Instrument));		
		psy_audio_instrument_init(instrument);
		psy_audio_instrument_setname(instrument, psy_audio_sample_name(samplecopy));
		psy_audio_instrument_setindex(instrument, dst.slot);

		psy_audio_instruments_insert(&workspace_song(self->workspace)->instruments, instrument,
			psy_audio_instrumentindex_make(0, dst.slot));
		samplesview_setsample(self->view, dst);
		/*signal_prevent(&workspace_song(self->workspace)->instruments.signal_slotchange,
			self->view, OnInstrumentSlotChanged);
		instruments_changeslot(&workspace_song(self->workspace)->instruments, dstslot);
		signal_enable(&workspace_song(self->workspace)->instruments.signal_slotchange, self,
			OnInstrumentSlotChanged);	*/
		psy_ui_component_invalidate(&self->view->component);
	}
}

void samplessongimportview_onsamplesboxchanged(SamplesSongImportView* self,
	psy_ui_Component* sender)
{
	psy_audio_SampleIndex index;
	psy_audio_Sample* sample;
	
	index = samplesbox_selected(&self->samplesbox);
	sample = self->source
		? psy_audio_samples_at(&self->source->samples, index)
		: 0;
	wavebox_setsample(&self->samplebox, sample, 0);
}

// Header View
// prototypes
static void samplesheaderview_onprevsample(SamplesHeaderView*, psy_ui_Component* sender);
static void samplesheaderview_onnextsample(SamplesHeaderView*, psy_ui_Component* sender);
static void samplesheaderview_oneditsamplename(SamplesHeaderView*, psy_ui_Edit* sender);

// implementation
void samplesheaderview_init(SamplesHeaderView* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments, struct SamplesView* view, Workspace* workspace)
{
	psy_ui_Margin margin;

	self->view = view;
	self->instruments = instruments;
	psy_ui_margin_init_all_em(&margin, 0.0, 0.5, 0.5, 0.0);
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_label_init_text(&self->namelabel, &self->component, NULL,
		"samplesview.samplename");	
	psy_ui_edit_init(&self->nameedit, &self->component);		
	psy_ui_edit_setcharnumber(&self->nameedit, 20);	
	psy_signal_connect(&self->nameedit.signal_change, self,
		samplesheaderview_oneditsamplename);
	psy_ui_button_init(&self->prevbutton, &self->component, NULL);
	psy_ui_button_seticon(&self->prevbutton, psy_ui_ICON_LESS);	
	psy_signal_connect(&self->prevbutton.signal_clicked, self,
		samplesheaderview_onprevsample);
	psy_ui_button_init(&self->nextbutton, &self->component, NULL);
	psy_ui_button_seticon(&self->nextbutton, psy_ui_ICON_MORE);	
	psy_signal_connect(&self->nextbutton.signal_clicked, self,
		samplesheaderview_onnextsample);
	psy_ui_label_init_text(&self->srlabel, &self->component, NULL,
		"samplesview.samplerate");	
	psy_ui_edit_init(&self->sredit, &self->component);	
	psy_ui_edit_setcharnumber(&self->sredit, 8);
	psy_ui_label_init_text(&self->numsamplesheaderlabel, &self->component,
		NULL, "samplesview.samples");
	psy_ui_label_init(&self->numsampleslabel, &self->component, NULL);
	psy_ui_label_preventtranslation(&self->numsampleslabel);
	psy_ui_label_setcharnumber(&self->numsampleslabel, 10);
	psy_ui_label_init(&self->channellabel, &self->component, NULL);
	psy_ui_label_preventtranslation(&self->channellabel);
	psy_ui_label_settext(&self->channellabel, "");
	psy_ui_label_setcharnumber(&self->channellabel, 7);	
	psy_ui_component_setalign_children(&self->component, psy_ui_ALIGN_LEFT);
}

void samplesheaderview_setsample(SamplesHeaderView* self, psy_audio_Sample* sample)
{
	char text[20];

	self->sample = sample;
	psy_signal_prevent(&self->nameedit.signal_change, self,
		samplesheaderview_oneditsamplename);
	psy_ui_edit_settext(&self->nameedit, self->sample ? sample->name : "");	
	psy_signal_enable(&self->nameedit.signal_change, self,
		samplesheaderview_oneditsamplename);
	psy_snprintf(text, 20, "%d", self->sample ? self->sample->samplerate : 0);
	psy_ui_edit_settext(&self->sredit, text);
	psy_snprintf(text, 20, "%d", self->sample ? self->sample->numframes : 0);
	psy_ui_label_settext(&self->numsampleslabel, text);
	if (self->sample) {
		switch (psy_audio_buffer_numchannels(&self->sample->channels)) {
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
					psy_audio_buffer_numchannels(&self->sample->channels));
			break;
		}		
	} else {
		psy_snprintf(text, 20, "");
	}
	psy_ui_label_settext(&self->channellabel, text);
	if (self->sample) {
		psy_ui_component_enableinput(&self->component, 1);
	} else {
		psy_ui_component_preventinput(&self->component, 1);
	}
}

void samplesheaderview_oneditsamplename(SamplesHeaderView* self, psy_ui_Edit* sender)
{
	if (self->sample) {
		char text[40];
		psy_audio_SampleIndex index;

		index = samplesbox_selected(&self->view->samplesbox);
		psy_audio_sample_setname(self->sample, psy_ui_edit_text(sender));
		if (index.subslot == 0) {
			psy_snprintf(text, 20, "%02X:%s", 
			(int)index.slot, psy_audio_sample_name(self->sample));
			psy_ui_listbox_settext(&self->view->samplesbox.samplelist, text, 
				index.slot);
		}
		psy_snprintf(text, 20, "%02X:%s", 
			(int)index.subslot, psy_audio_sample_name(self->sample));
		psy_ui_listbox_settext(&self->view->samplesbox.subsamplelist, text,
			index.subslot);
	}
}

void samplesheaderview_onprevsample(SamplesHeaderView* self, psy_ui_Component* sender)
{
	/*if (instruments_slot(self->instruments).subslot > 0) {
		instruments_changeslot(self->instruments,
	}
	instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) > 0 ?
		instruments_slot(self->instruments) - 1 : 0);*/
}

void samplesheaderview_onnextsample(SamplesHeaderView* self, psy_ui_Component* sender)
{
	/*instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) < 255 ?
		instruments_slot(self->instruments) + 1 : 255);*/
}

// GeneralView
// prototypes
static void generalview_setsample(SamplesGeneralView*, psy_audio_Sample*);
static void generalview_fillpandescription(SamplesGeneralView*, char* txt);
static void generalview_ondescribe(SamplesGeneralView*, psy_ui_Slider*, char* txt);
static void generalview_ontweak(SamplesGeneralView*, psy_ui_Slider*, float value);
static void generalview_onvalue(SamplesGeneralView*, psy_ui_Slider*, float* value);
static void generalview_updatetext(SamplesGeneralView*);
static void generalview_updatesliders(SamplesGeneralView*);
// implementation
void samplesgeneralview_init(SamplesGeneralView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_Margin margin;
	psy_ui_Slider* sliders[] = {
		&self->defaultvolume,
		&self->globalvolume,
		&self->panposition,
		&self->samplednote,
		&self->pitchfinetune,
		0
	};	
	int i;
		
	self->sample = NULL;	
	self->notestabmode = workspace_notetabmode(workspace);
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));	
	psy_ui_slider_init(&self->defaultvolume, &self->component, NULL);
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	psy_ui_margin_settop(&margin, psy_ui_value_make_eh(1.0));
	psy_ui_component_setmargin(&self->defaultvolume.component, margin);
	psy_ui_slider_init(&self->globalvolume, &self->component, NULL);
	psy_ui_slider_init(&self->panposition, &self->component, NULL);
	psy_ui_slider_init(&self->samplednote, &self->component, NULL);
	psy_ui_slider_init(&self->pitchfinetune, &self->component, NULL);
	for (i = 0; sliders[i] != 0; ++i) {		
		psy_ui_slider_setcharnumber(sliders[i], 16);
		psy_ui_slider_setvaluecharnumber(sliders[i], 10);
		psy_ui_slider_connect(sliders[i], self,
			(ui_slider_fpdescribe)generalview_ondescribe,
			(ui_slider_fptweak)generalview_ontweak,
			(ui_slider_fpvalue)generalview_onvalue);
	}
	generalview_updatetext(self);	
}

void generalview_updatetext(SamplesGeneralView* self)
{
	psy_ui_slider_settext(&self->defaultvolume, 
		"samplesview.default-volume");
	psy_ui_slider_settext(&self->globalvolume,
		"samplesview.global-volume");
	psy_ui_slider_settext(&self->panposition,
		"samplesview.pan-position");
	psy_ui_slider_settext(&self->samplednote,
		"samplesview.sampled-note");
	psy_ui_slider_settext(&self->pitchfinetune,
		"samplesview.pitch-finetune");
}

void generalview_updatesliders(SamplesGeneralView* self)
{
	psy_ui_slider_update(&self->defaultvolume);
	psy_ui_slider_update(&self->globalvolume);
	psy_ui_slider_update(&self->panposition);
	psy_ui_slider_update(&self->samplednote);
	psy_ui_slider_update(&self->pitchfinetune);
}

void generalview_setsample(SamplesGeneralView* self, psy_audio_Sample* sample)
{
	self->sample = sample;
	if (self->sample) {
		psy_ui_component_enableinput(&self->component, 1);
	} else {
		psy_ui_component_preventinput(&self->component, 1);
	}
	generalview_updatesliders(self);
}

int map_1_128(float value) {
	return (int)(value * 128.f);
}

void generalview_ontweak(SamplesGeneralView* self, psy_ui_Slider* slider,
	float value)
{
	if (self->sample) {			
		if (slider == &self->defaultvolume) {
			self->sample->defaultvolume = (uint16_t)(value * 128.f);
		} else
		if (slider == &self->globalvolume) {		
			self->sample->globalvolume = (value * value) * 4.f;
		} else
		if (slider == &self->panposition) {
			self->sample->panfactor = value;
		} else
		if (slider == &self->samplednote) {
			self->sample->zone.tune = (int)(value * 119.f) - 60;
		} else
		if (slider == &self->pitchfinetune) {
			self->sample->zone.finetune = (int)(value * 200.f) - 100;
		}
	}
}

void generalview_onvalue(SamplesGeneralView* self, psy_ui_Slider* slider,
	float* value)
{	
	if (slider == &self->defaultvolume) {
		*value = self->sample ? self->sample->defaultvolume / 128.f : 1.0f;
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
		*value = self->sample ? (self->sample->zone.tune + 60) / 119.f : 0.5f;
	} else
	if (slider == &self->pitchfinetune) {
		*value = self->sample ? self->sample->zone.finetune / 200.f + 0.5f : 0.f;
	}
}

void generalview_ondescribe(SamplesGeneralView* self, psy_ui_Slider* slider, char* txt)
{		
	if (slider == &self->defaultvolume) {		
		psy_snprintf(txt, 10, "C%02X", self->sample 
			? self->sample->defaultvolume
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
		generalview_fillpandescription(self, txt);		
	} else
	if (slider == &self->samplednote) {			
		psy_snprintf(txt, 10, "%s", self->sample
			? psy_dsp_notetostr((psy_dsp_note_t)(self->sample->zone.tune + 60),
				self->notestabmode)
			: psy_dsp_notetostr(60, self->notestabmode));
	} else
	if (slider == &self->pitchfinetune) {
		psy_snprintf(txt, 10, "%d ct.", self->sample
			? self->sample->zone.finetune
			: 0);
	}
}

void generalview_fillpandescription(SamplesGeneralView* self, char* txt) {	

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

/// psy_audio_Vibrato Settings View
// prototypes
static void vibratoview_setsample(SamplesVibratoView*, psy_audio_Sample*);
static void vibratoview_ondescribe(SamplesVibratoView*, psy_ui_Slider*, char* txt);
static void vibratoview_ontweak(SamplesVibratoView*, psy_ui_Slider*, float value);
static void vibratoview_onvalue(SamplesVibratoView*, psy_ui_Slider*, float* value);
static void vibratoview_onwaveformchange(SamplesVibratoView*, psy_ui_ComboBox* sender, int sel);
static psy_audio_WaveForms vibratoview_comboboxtowaveform(int combobox_index);
static int vibratoview_waveformtocombobox(psy_audio_WaveForms waveform);
// implementation
void samplesvibratoview_init(SamplesVibratoView* self, psy_ui_Component* parent, psy_audio_Player* player)
{
	psy_ui_Margin margin;
	int i;
	psy_ui_Slider* sliders[] = {
		&self->attack,
		&self->speed,
		&self->depth,		
	};	

	psy_ui_component_init(&self->component, parent, NULL);
	self->sample = 0;
	self->player = player;
	// header
	{
		psy_ui_Margin header_margin;

		psy_ui_margin_init_all_em(&header_margin, 0.0, 2.0, 0.0, 0.0);
		psy_ui_component_init(&self->header, &self->component, NULL);
		psy_ui_component_setalign(&self->header, psy_ui_ALIGN_TOP);		
		psy_ui_label_init(&self->waveformheaderlabel, &self->header, NULL);
		psy_ui_label_settext(&self->waveformheaderlabel, "Waveform");
		psy_ui_component_setalign(&self->waveformheaderlabel.component,
			psy_ui_ALIGN_LEFT);
		psy_ui_component_setmargin(&self->waveformheaderlabel.component,
			header_margin);
		psy_ui_combobox_init(&self->waveformbox, &self->header, NULL);
		psy_ui_combobox_setcharnumber(&self->waveformbox, 15);
		psy_ui_combobox_addtext(&self->waveformbox, "Sinus");
		psy_ui_combobox_addtext(&self->waveformbox, "Square");
		psy_ui_combobox_addtext(&self->waveformbox, "RampUp");
		psy_ui_combobox_addtext(&self->waveformbox, "RampDown");
		psy_ui_combobox_addtext(&self->waveformbox, "Random");
		psy_ui_component_setalign(&self->waveformbox.component,
			psy_ui_ALIGN_LEFT);
		psy_ui_combobox_setcursel(&self->waveformbox, 0);
		psy_signal_connect(&self->waveformbox.signal_selchanged, self,
			vibratoview_onwaveformchange);
	}
	psy_ui_slider_init(&self->attack, &self->component, NULL);
	psy_ui_slider_settext(&self->attack, "Attack");	
	psy_ui_slider_init(&self->speed, &self->component, NULL);
	psy_ui_slider_settext(&self->speed,"Speed");
	psy_ui_slider_init(&self->depth, &self->component, NULL);
	psy_ui_slider_settext(&self->depth, "Depth");
	psy_ui_margin_init_all_em(&margin, 1.0, 0.0, 0.0, 0.0);
	for (i = 0; i < 3; ++i) {				
		psy_ui_component_setalign(&sliders[i]->component, psy_ui_ALIGN_TOP);
		psy_ui_component_setmargin(&sliders[i]->component, margin);
		psy_ui_slider_setcharnumber(sliders[i], 16);
		psy_ui_slider_connect(sliders[i], self,
			(ui_slider_fpdescribe)vibratoview_ondescribe,
			(ui_slider_fptweak)vibratoview_ontweak,
			(ui_slider_fpvalue)vibratoview_onvalue);
	}
	margin = psy_ui_component_margin(&sliders[0]->component);
	margin.top = psy_ui_value_make_ew(2.0);	
	psy_ui_component_setmargin(&sliders[0]->component, margin);
}

void vibratoview_setsample(SamplesVibratoView* self, psy_audio_Sample* sample)
{
	self->sample = sample;
	if (self->sample) {
		psy_ui_component_enableinput(&self->component, 1);
		psy_ui_combobox_setcursel(&self->waveformbox,
			vibratoview_waveformtocombobox(self->sample->vibrato.type));
	} else {
		psy_ui_component_preventinput(&self->component, 1);
		psy_ui_combobox_setcursel(&self->waveformbox,
			vibratoview_waveformtocombobox(psy_audio_WAVEFORMS_SINUS));
	}
}

void vibratoview_ontweak(SamplesVibratoView* self, psy_ui_Slider* slidergroup, float value)
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

void vibratoview_onvalue(SamplesVibratoView* self, psy_ui_Slider* slidergroup,
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

void vibratoview_ondescribe(SamplesVibratoView* self, psy_ui_Slider* slidergroup, char* txt)
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

void vibratoview_onwaveformchange(SamplesVibratoView* self, psy_ui_ComboBox* sender,
	int sel)
{
	if (self->sample) {
		self->sample->vibrato.type = vibratoview_comboboxtowaveform(sel);
	}
}

int vibratoview_waveformtocombobox(psy_audio_WaveForms waveform)
{
	int rv = 0;

	switch (waveform) {
		case psy_audio_WAVEFORMS_SINUS: rv = 0; break;		
		case psy_audio_WAVEFORMS_SQUARE: rv = 1; break;					
		case psy_audio_WAVEFORMS_SAWUP: rv = 2; break;					
		case psy_audio_WAVEFORMS_SAWDOWN: rv = 3; break;					
		case psy_audio_WAVEFORMS_RANDOM: rv = 4; break;					
		default:
		break;		
	}
	return rv;
}

psy_audio_WaveForms vibratoview_comboboxtowaveform(int combobox_index)
{
	psy_audio_WaveForms rv = psy_audio_WAVEFORMS_SINUS;			
	
	switch (combobox_index) {					
		case 0: rv = psy_audio_WAVEFORMS_SINUS; break;
		case 1: rv = psy_audio_WAVEFORMS_SQUARE; break;
		case 2: rv = psy_audio_WAVEFORMS_SAWUP; break;
		case 3: rv = psy_audio_WAVEFORMS_SAWDOWN; break;
		case 4: rv = psy_audio_WAVEFORMS_RANDOM; break;
		default:
		break;				
	}
	return rv;
}

/// Waveloop Setting View
static void samplesloopview_setsample(SamplesLoopView*, psy_audio_Sample*);
static int LoopTypeToComboBox(psy_audio_SampleLoopType looptype);
static psy_audio_SampleLoopType ComboBoxToLoopType(int combobox_index);
static void samplesloopview_samplecontloopchanged(SamplesLoopView*, psy_audio_Sample* sender);
static void samplesloopview_samplesustainloopchanged(SamplesLoopView*, psy_audio_Sample* sender);
static void samplesloopview_onlooptypechange(SamplesLoopView*,
	psy_ui_ComboBox* sender, int sel);
static void samplesloopview_onsustainlooptypechange(SamplesLoopView*,
	psy_ui_ComboBox* sender, int selectedindex);
static void samplesloopview_looptypeenablepreventinput(SamplesLoopView*);
static void samplesloopview_oneditchangedloopstart(SamplesLoopView*,
	psy_ui_Edit* sender);
static void samplesloopview_oneditchangedloopend(SamplesLoopView*,
	psy_ui_Edit* sender);
static void samplesloopview_oneditchangedsustainstart(SamplesLoopView*,
	psy_ui_Edit* sender);
static void samplesloopview_oneditchangedsustainend(SamplesLoopView*,
	psy_ui_Edit* sender);
static void samplesloopview_ontimer(SamplesLoopView*, psy_ui_Component* sender, uintptr_t timerid);

void samplesloopview_init(SamplesLoopView* self, psy_ui_Component* parent,
	SamplesView* view)
{
	psy_ui_Margin margin;
	psy_ui_Margin rowmargin;
	
	self->view = view;
	psy_ui_margin_init_all_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_margin_init_all_em(&rowmargin, 1.5, 0.0, 1.5, 0.0);		
	self->sample = NULL;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_init(&self->cont, &self->component, NULL);
	psy_ui_component_setmargin(&self->cont, rowmargin);
	psy_ui_label_init(&self->loopheaderlabel, &self->cont, NULL);
	psy_ui_label_settext(&self->loopheaderlabel, "Continuous Loop");
	psy_ui_label_setcharnumber(&self->loopheaderlabel, 18);	
	psy_ui_combobox_init(&self->loopdir, &self->cont, NULL);
	psy_ui_combobox_addtext(&self->loopdir, "Disabled");
	psy_ui_combobox_addtext(&self->loopdir, "Forward");
	psy_ui_combobox_addtext(&self->loopdir, "Bidirection");
	psy_ui_combobox_setcursel(&self->loopdir, 0);
	psy_ui_combobox_setcharnumber(&self->loopdir, 12);
	psy_ui_label_init(&self->loopstartlabel, &self->cont, NULL);
	psy_ui_label_settext(&self->loopstartlabel, "Start ");	
	psy_ui_edit_init(&self->loopstartedit, &self->cont);
	psy_ui_edit_setcharnumber(&self->loopstartedit, 10);
	psy_ui_label_init(&self->loopendlabel, &self->cont, NULL);
	psy_ui_label_settext(&self->loopendlabel, "End ");	
	psy_ui_edit_init(&self->loopendedit, &self->cont);
	psy_ui_edit_setcharnumber(&self->loopendedit, 10);
	psy_ui_component_setalign_children(&self->cont, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin_children(&self->cont, margin);
	psy_ui_component_init(&self->sustain, &self->component, NULL);
	psy_ui_label_init(&self->sustainloopheaderlabel, &self->sustain, NULL);
	psy_ui_label_settext(&self->sustainloopheaderlabel, "Sustain Loop");
	psy_ui_label_setcharnumber(&self->sustainloopheaderlabel, 18);
	psy_ui_combobox_init(&self->sustainloopdir, &self->sustain, NULL);
	psy_ui_combobox_addtext(&self->sustainloopdir, "Disabled");
	psy_ui_combobox_addtext(&self->sustainloopdir, "Forward");
	psy_ui_combobox_addtext(&self->sustainloopdir, "Bidirection");
	psy_ui_combobox_setcursel(&self->sustainloopdir, 0);
	psy_ui_combobox_setcharnumber(&self->sustainloopdir, 12);
	psy_ui_label_init(&self->sustainloopstartlabel, &self->sustain, NULL);
	psy_ui_label_settext(&self->sustainloopstartlabel, "Start ");	
	psy_ui_edit_init(&self->sustainloopstartedit, &self->sustain);		
	psy_ui_edit_setcharnumber(&self->sustainloopstartedit, 10);
	psy_ui_label_init(&self->sustainloopendlabel, &self->sustain, NULL);
	psy_ui_label_settext(&self->sustainloopendlabel, "End ");	
	psy_ui_edit_init(&self->sustainloopendedit, &self->sustain);
	psy_ui_edit_setcharnumber(&self->sustainloopendedit, 10);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->sustain, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		margin));
	psy_ui_component_setalign_children(&self->component, psy_ui_ALIGN_TOP);		
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
	psy_signal_connect(&self->component.signal_timer, self,
		samplesloopview_ontimer);
	psy_audio_sampleloop_init(&self->currloop);
	psy_audio_sampleloop_init(&self->currsustainloop);
}

void samplesloopview_setsample(SamplesLoopView* self, psy_audio_Sample* sample)
{
	char tmp[40];

	self->sample = sample;
	if (self->sample) {
		psy_ui_component_starttimer(&self->component, 0, 200);
		psy_ui_component_enableinput(&self->component, 1);
		sprintf(tmp, "%d", (int)sample->loop.start);
		psy_ui_edit_settext(&self->loopstartedit, tmp);
		sprintf(tmp, "%d", (int)sample->loop.end);
		psy_ui_edit_settext(&self->loopendedit, tmp);
		sprintf(tmp, "%d", (int)sample->sustainloop.start);
		psy_ui_edit_settext(&self->sustainloopstartedit, tmp);
		sprintf(tmp, "%d", (int)sample->sustainloop.end);
		psy_ui_edit_settext(&self->sustainloopendedit, tmp);
		psy_ui_combobox_setcursel(&self->loopdir,
			LoopTypeToComboBox(self->sample->loop.type));		
		psy_ui_combobox_setcursel(&self->sustainloopdir,
			LoopTypeToComboBox(self->sample->sustainloop.type));
		self->currloop = self->sample->loop;
		self->currsustainloop = self->sample->sustainloop;
	} else {
		psy_ui_component_preventinput(&self->component, 1);
		sprintf(tmp, "%d", 0);
		psy_ui_edit_settext(&self->loopstartedit, tmp);		
		psy_ui_edit_settext(&self->loopendedit, tmp);		
		psy_ui_edit_settext(&self->sustainloopstartedit, tmp);		
		psy_ui_edit_settext(&self->sustainloopendedit, tmp);
		psy_ui_combobox_setcursel(&self->loopdir,
			LoopTypeToComboBox(psy_audio_SAMPLE_LOOP_DO_NOT));
		psy_ui_combobox_setcursel(&self->sustainloopdir,
			LoopTypeToComboBox(psy_audio_SAMPLE_LOOP_DO_NOT));
		psy_ui_component_stoptimer(&self->component, 0);
		psy_audio_sampleloop_init(&self->currloop);
		psy_audio_sampleloop_init(&self->currsustainloop);
	}
	samplesloopview_looptypeenablepreventinput(self);
}

int LoopTypeToComboBox(psy_audio_SampleLoopType looptype)
{
	int rv = 0;

	switch (looptype) {
		case psy_audio_SAMPLE_LOOP_DO_NOT: rv = 0; break;
		case psy_audio_SAMPLE_LOOP_NORMAL: rv = 1; break;
		case psy_audio_SAMPLE_LOOP_BIDI: rv = 2; break;
		default:
		break;
	}
	return rv;
}

void samplesloopview_ontimer(SamplesLoopView* self, psy_ui_Component* sender,
	uintptr_t timerid)
{
	if (!psy_audio_sampleloop_equal(&self->currloop, &self->sample->loop) ||
			!psy_audio_sampleloop_equal(&self->currsustainloop,
			&self->sample->sustainloop)) {			
		samplesloopview_setsample(self, self->sample);
	}		
}

void samplesloopview_onlooptypechange(SamplesLoopView* self,
	psy_ui_ComboBox* sender, int sel)
{
	if (self->sample) {
		self->sample->loop.type = ComboBoxToLoopType(sel);
		psy_ui_component_invalidate(&self->view->wavebox.component);
		psy_ui_component_invalidate(&self->view->sampleeditor.samplebox.component);
		samplesloopview_looptypeenablepreventinput(self);		
	}
}

void samplesloopview_onsustainlooptypechange(SamplesLoopView* self,
	psy_ui_ComboBox* sender, int sel)
{
	if (self->sample) {
		self->sample->sustainloop.type = ComboBoxToLoopType(sel);
		psy_ui_component_invalidate(&self->view->wavebox.component);
		psy_ui_component_invalidate(&self->view->sampleeditor.samplebox.component);
		samplesloopview_looptypeenablepreventinput(self);
	}
}

void samplesloopview_looptypeenablepreventinput(SamplesLoopView* self)
{
	if (self->sample) {
		if (self->sample->loop.type == psy_audio_SAMPLE_LOOP_DO_NOT) {
			psy_ui_component_preventinput(&self->loopstartedit.component, 0);
			psy_ui_component_preventinput(&self->loopendedit.component, 0);
		} else {
			psy_ui_component_enableinput(&self->loopstartedit.component, 0);
			psy_ui_component_enableinput(&self->loopendedit.component, 0);
		}
		if (self->sample->sustainloop.type == psy_audio_SAMPLE_LOOP_DO_NOT) {
			psy_ui_component_preventinput(&self->sustainloopstartedit.component, 0);
			psy_ui_component_preventinput(&self->sustainloopendedit.component, 0);
		} else {
			psy_ui_component_enableinput(&self->sustainloopstartedit.component, 0);
			psy_ui_component_enableinput(&self->sustainloopendedit.component, 0);
		}
	} else {
		psy_ui_component_preventinput(&self->loopstartedit.component, 0);
		psy_ui_component_preventinput(&self->loopendedit.component, 0);
		psy_ui_component_preventinput(&self->sustainloopstartedit.component, 0);
		psy_ui_component_preventinput(&self->sustainloopendedit.component, 0);	
	}
}

psy_audio_SampleLoopType ComboBoxToLoopType(int combobox_index)
{
	psy_audio_SampleLoopType rv = 0;
			
	switch (combobox_index) {			
		case 0: rv = psy_audio_SAMPLE_LOOP_DO_NOT; break;
		case 1: rv = psy_audio_SAMPLE_LOOP_NORMAL; break;
		case 2: rv = psy_audio_SAMPLE_LOOP_BIDI; break;
		default:
		break;
	}
	return rv;
}

void samplesloopview_oneditchangedloopstart(SamplesLoopView* self,
	psy_ui_Edit* sender)
{
	if (self->sample) {
		self->sample->loop.start = atoi(psy_ui_edit_text(sender));
		psy_ui_component_invalidate(&self->view->wavebox.component);
		psy_ui_component_invalidate(&self->view->sampleeditor.samplebox.component);
	}
}

void samplesloopview_oneditchangedloopend(SamplesLoopView* self,
	psy_ui_Edit* sender)
{				
	if (self->sample) {
		self->sample->loop.end = atoi(psy_ui_edit_text(sender));
		psy_ui_component_invalidate(&self->view->wavebox.component);
		psy_ui_component_invalidate(&self->view->sampleeditor.samplebox.component);
	}
}

void samplesloopview_oneditchangedsustainstart(SamplesLoopView* self,
	psy_ui_Edit* sender)
{
	if (self->sample) {
		self->sample->sustainloop.start = atoi(psy_ui_edit_text(sender));
		psy_ui_component_invalidate(&self->view->wavebox.component);
		psy_ui_component_invalidate(&self->view->sampleeditor.samplebox.component);
	}
}

void samplesloopview_oneditchangedsustainend(SamplesLoopView* self,
	psy_ui_Edit* sender)
{
	if (self->sample) {
		self->sample->sustainloop.end = atoi(psy_ui_edit_text(sender));		
		psy_ui_component_invalidate(&self->view->wavebox.component);
		psy_ui_component_invalidate(&self->view->sampleeditor.samplebox.component);
	}
}

// SamplesView
// prototypes
static void samplesview_onsamplesboxchanged(SamplesView*, psy_ui_Component* sender);
static void samplesview_onloadsample(SamplesView*, psy_ui_Component* sender);
static void samplesview_onsavesample(SamplesView*, psy_ui_Component* sender);
static void samplesview_ondeletesample(SamplesView*, psy_ui_Component* sender);
static void samplesview_onduplicatesample(SamplesView*, psy_ui_Component* sender);
static void samplesview_onsongchanged(SamplesView*, Workspace*, int flag,
	psy_audio_Song*);
static void samplesview_oninstrumentslotchanged(SamplesView* self,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex* slot);
static uintptr_t samplesview_freesampleslot(SamplesView*, uintptr_t startslot,
	uintptr_t maxslots);
static void samplesview_onsamplemodified(SamplesView*, SampleEditor* sender,
	psy_audio_Sample*);
static void samplesview_onconfigure(SamplesView*, Workspace*,
	psy_Property*);
static void samplesview_onresamplermethodchanged(SamplesView*,
	psy_ui_Component* sender, int index);
// implementation
void samplesview_init(SamplesView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	psy_ui_Margin margin;
	psy_ui_Margin waveboxmargin;
	psy_ui_Margin leftmargin;

	psy_ui_margin_init_all_em(&margin, 0.0, 2.0, 0.0, 0.0);		
	psy_ui_margin_init_all_em(&waveboxmargin, 0.5, 1.0, 0.0, 0.5);
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletypes(&self->component,
		STYLE_SAMPLESVIEW,
		psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID);
	psy_ui_margin_init_all_em(&leftmargin, 0.0, 0.0, 0.0, 3.0);
	samplesheaderview_init(&self->header, &self->component,
		&workspace->song->instruments, self, workspace);
	psy_ui_component_setmargin(&self->header.component, leftmargin);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	// left
	psy_ui_component_init(&self->left, &self->component, NULL);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(&self->left, leftmargin);
	samplesviewbuttons_init(&self->buttons, &self->left, workspace);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);
	// tabbarparent
	psy_ui_tabbar_init(&self->clienttabbar, tabbarparent);
	psy_ui_component_setalign(&self->clienttabbar.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(&self->clienttabbar.component);
	psy_ui_tabbar_append_tabs(&self->clienttabbar, "Properties", "Import", "Editor", NULL);
	samplesbox_init(&self->samplesbox, &self->left,
		&workspace->song->samples, workspace);
	psy_ui_component_setalign(&self->samplesbox.component,
		psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->buttons.load.signal_clicked, self,
		samplesview_onloadsample);
	psy_signal_connect(&self->buttons.save.signal_clicked, self,
		samplesview_onsavesample);
	psy_signal_connect(&self->buttons.duplicate.signal_clicked, self,
		samplesview_onduplicatesample);
	psy_signal_connect(&self->buttons.del.signal_clicked, self,
		samplesview_ondeletesample);
	// client	
	psy_ui_notebook_init(&self->clientnotebook, &self->component);
	psy_ui_component_setmargin(psy_ui_notebook_base(&self->clientnotebook),
		leftmargin);
	psy_ui_component_setalign(&self->clientnotebook.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_init(&self->client, &self->clientnotebook.component, NULL);
	psy_ui_tabbar_init(&self->tabbar, &self->client);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar), psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(psy_ui_tabbar_base(&self->tabbar), margin);
	psy_ui_tabbar_append(&self->tabbar, "General");
	psy_ui_tabbar_append(&self->tabbar, "Vibrato");
	psy_ui_notebook_init(&self->notebook, &self->client);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_TOP);
	psy_ui_component_setbackgroundmode(psy_ui_notebook_base(&self->notebook),
		psy_ui_SETBACKGROUND);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	// GeneralView
	samplesgeneralview_init(&self->general, psy_ui_notebook_base(
		&self->notebook), workspace);
	psy_ui_component_setalign(&self->general.component, psy_ui_ALIGN_TOP);
	// VibratoView
	samplesvibratoview_init(&self->vibrato, psy_ui_notebook_base(&self->notebook),
		&workspace->player);
	psy_ui_component_setalign(&self->vibrato.component, psy_ui_ALIGN_TOP);
	psy_ui_notebook_select(&self->notebook, 0);
	wavebox_init(&self->wavebox, &self->client, workspace);
	psy_ui_component_setalign(&self->wavebox.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setmargin(&self->wavebox.component, waveboxmargin);
	// LoopView
	samplesloopview_init(&self->waveloop, &self->component, self);
	psy_ui_component_setalign(&self->waveloop.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_setmargin(&self->waveloop.component, leftmargin);
	psy_signal_connect(&self->samplesbox.signal_changed, self,
		samplesview_onsamplesboxchanged);
	psy_signal_connect(&workspace->song->instruments.signal_slotchange, self,
		samplesview_oninstrumentslotchanged);
	// ImportView
	samplessongimportview_init(&self->songimport,
		&self->clientnotebook.component, self, workspace);
	// WaveEditorView
	sampleeditor_init(&self->sampleeditor, &self->clientnotebook.component,
		workspace);
	psy_ui_notebook_select(&self->clientnotebook, 0);
	psy_signal_connect(&workspace->signal_songchanged, self,
		samplesview_onsongchanged);
	psy_ui_notebook_select(&self->clientnotebook, 0);
	psy_ui_notebook_connectcontroller(&self->clientnotebook,
		&self->clienttabbar.signal_change);
	samplesview_setsample(self, psy_audio_sampleindex_make(0, 0));
	psy_signal_connect(&self->sampleeditor.signal_samplemodified, self,
		samplesview_onsamplemodified);
	psy_signal_connect(&self->workspace->signal_configchanged, self,
		samplesview_onconfigure);	
}

void samplesview_onconfigure(SamplesView* self, Workspace* sender,
	psy_Property* property)
{
	self->general.notestabmode = workspace_notetabmode(sender);
}

void samplesview_onsamplesboxchanged(SamplesView* self, psy_ui_Component* sender)
{
	psy_audio_SampleIndex index;

	index = samplesbox_selected(&self->samplesbox);
	samplesview_setsample(self, index);
	if (workspace_song(self->workspace)) {
		psy_signal_disconnect(&workspace_song(self->workspace)->instruments.signal_slotchange,
			self, samplesview_oninstrumentslotchanged);
		psy_audio_instruments_select(&workspace_song(self->workspace)->instruments,
			psy_audio_instrumentindex_make(0, index.slot));
		psy_signal_connect(
			&workspace_song(self->workspace)->instruments.signal_slotchange,
			self, samplesview_oninstrumentslotchanged);
	}
	psy_ui_component_invalidate(&self->wavebox.component);
}

void samplesview_oninstrumentslotchanged(SamplesView* self,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex* slot)
{
	psy_audio_SampleIndex index;

	index = samplesbox_selected(&self->samplesbox);
	if (index.slot != (uintptr_t)slot->subslot) {
		index.subslot = 0;
	}
	index.slot = slot->subslot;
	samplesview_setsample(self, index);
}

void samplesview_setsample(SamplesView* self, psy_audio_SampleIndex index)
{
	psy_audio_Sample* sample;

	sample = (workspace_song(self->workspace))
		? psy_audio_samples_at(&workspace_song(self->workspace)->samples, index)
		: 0;
	wavebox_setsample(&self->wavebox, sample, 0);
	sampleeditor_setsample(&self->sampleeditor, sample);
	samplesheaderview_setsample(&self->header, sample);
	generalview_setsample(&self->general, sample);
	vibratoview_setsample(&self->vibrato, sample);
	samplesloopview_setsample(&self->waveloop, sample);
	samplesbox_select(&self->samplesbox, index);
}

void samplesview_connectstatusbar(SamplesView* self)
{		
	psy_signal_connect(
		&self->sampleeditor.sampleeditortbar.visualrepresentation.signal_selchanged,
		self, samplesview_onresamplermethodchanged);
}


void samplesview_onloadsample(SamplesView* self, psy_ui_Component* sender)
{
	if (workspace_song(self->workspace)) {
		psy_ui_OpenDialog dialog;
		static char filter[] =
			"Wav Files (*.wav)|*.wav|"
			"IFF psy_audio_Samples (*.iff)|*.iff|"
			"All Files (*.*)|*.*";

		psy_ui_opendialog_init_all(&dialog, 0, "Load Sample", filter, "WAV",
			dirconfig_samples(&self->workspace->config.directories));
		if (psy_ui_opendialog_execute(&dialog)) {
			psy_audio_Sample* sample;
			psy_audio_SampleIndex index;
			psy_audio_Instrument* instrument;
			psy_audio_InstrumentEntry entry;			

			sample = psy_audio_sample_allocinit(0);			
			psy_audio_sample_load(sample, psy_ui_opendialog_path(&dialog));			
			index = samplesbox_selected(&self->samplesbox);
			psy_audio_samples_insert(&workspace_song(self->workspace)->samples, sample,
				index);
			instrument = psy_audio_instrument_allocinit();
			psy_audio_instrumententry_init(&entry);
			entry.sampleindex = index;
			psy_audio_instrument_addentry(instrument, &entry);
			psy_audio_instrument_setname(instrument, psy_audio_sample_name(sample));
			psy_audio_instruments_insert(&workspace_song(self->workspace)->instruments, instrument,
				psy_audio_instrumentindex_make(0, index.slot));
			samplesview_setsample(self, index);
			psy_signal_prevent(
				&workspace_song(self->workspace)->instruments.signal_slotchange,
				self, samplesview_oninstrumentslotchanged);
			psy_audio_instruments_select(&workspace_song(self->workspace)->instruments,
				psy_audio_instrumentindex_make(0, index.slot));
			psy_signal_enable(
				&workspace_song(self->workspace)->instruments.signal_slotchange,
				self, samplesview_oninstrumentslotchanged);
			psy_ui_component_invalidate(&self->component);
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void samplesview_onsavesample(SamplesView* self, psy_ui_Component* sender)
{
	psy_ui_SaveDialog dialog;
	static char filter[] =
		"Wav Files (*.wav)|*.wav|"
		"IFF psy_audio_Samples (*.iff)|*.iff|"
		"All Files (*.*)|*.*";

	psy_ui_savedialog_init_all(&dialog, 0,
		"Save Sample",
		filter,
		"WAV",
		dirconfig_samples(&self->workspace->config.directories));
	if (wavebox_sample(&self->wavebox) && psy_ui_savedialog_execute(&dialog)) {		
		psy_audio_sample_save(wavebox_sample(&self->wavebox),
			psy_ui_savedialog_path(&dialog));	
	}
	psy_ui_savedialog_dispose(&dialog);
}

void samplesview_ondeletesample(SamplesView* self, psy_ui_Component* sender)
{
	if (workspace_song(self->workspace)) {
		psy_audio_SampleIndex index;

		index = samplesbox_selected(&self->samplesbox);
		psy_audio_exclusivelock_enter();
		psy_audio_samples_remove(&workspace_song(self->workspace)->samples, index);
		psy_audio_instruments_remove(&workspace_song(self->workspace)->instruments,
			psy_audio_instrumentindex_make(0, index.subslot));
		samplesview_setsample(self, index);
		psy_audio_exclusivelock_leave();
	}
}

void samplesview_onduplicatesample(SamplesView* self, psy_ui_Component* sender)
{
	if (workspace_song(self->workspace)) {
		psy_audio_SampleIndex src;
		psy_audio_SampleIndex dst;

		src = samplesbox_selected(&self->samplesbox);
		dst.slot = samplesview_freesampleslot(self, src.slot, 256);
		dst.subslot = 0;
		if (dst.slot != 256) {
			psy_audio_Sample* source;

			source = psy_audio_samples_at(&workspace_song(self->workspace)->samples,
				src);
			if (source) {
				psy_audio_Sample* copy;
				psy_audio_Instrument* instrument;

				copy = psy_audio_sample_clone(source);
				psy_audio_samples_insert(&workspace_song(self->workspace)->samples, copy,
					dst);
				instrument = psy_audio_instrument_allocinit();
				psy_audio_instrument_setname(instrument, psy_audio_sample_name(copy));
				psy_audio_instrument_setindex(instrument, dst.slot);
				psy_audio_instruments_insert(&workspace_song(self->workspace)->instruments,
					instrument, psy_audio_instrumentindex_make(0, dst.slot));
				samplesview_setsample(self, psy_audio_sampleindex_make(dst.slot, 0));
			}
		}
	}
}

uintptr_t samplesview_freesampleslot(SamplesView* self, uintptr_t startslot,
	uintptr_t maxslots)
{
	uintptr_t rv = startslot;

	if (workspace_song(self->workspace)) {
		int first = startslot != 0;

		while (psy_audio_samples_at(&workspace_song(self->workspace)->samples,
			psy_audio_sampleindex_make(rv, 0)) != 0) {
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

void samplesview_onsamplemodified(SamplesView* self, SampleEditor* sender, psy_audio_Sample* sample)
{
	samplesheaderview_setsample(&self->header, sample);
	generalview_setsample(&self->general, sample);
	vibratoview_setsample(&self->vibrato, sample);
	samplesloopview_setsample(&self->waveloop, sample);
}

void samplesview_onsongchanged(SamplesView* self, Workspace* workspace, int flag,
	psy_audio_Song* song)
{
	psy_signal_connect(&workspace->song->instruments.signal_slotchange, self,
		samplesview_oninstrumentslotchanged);
	samplesbox_setsamples(&self->samplesbox, &workspace->song->samples);
	samplesview_setsample(self, psy_audio_sampleindex_make(0, 0));
}

void samplesview_onresamplermethodchanged(SamplesView* self,
	psy_ui_Component* sender, int index)
{
	wavebox_setquality(&self->wavebox, (psy_dsp_ResamplerQuality)index);
	sampleeditor_setquality(&self->sampleeditor, (psy_dsp_ResamplerQuality)index);
}

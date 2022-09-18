/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "samplesview.h"
/* host */
#include "styles.h"
/* audio */
#include <exclusivelock.h>
#include <instruments.h>
#include <songio.h>
/* ui */
#include <uiopendialog.h>
#include <uisavedialog.h>
/* std */
#include <math.h>
/* file */
#include <dir.h>
/* platform */
#include "../../detail/portable.h"

/* SamplesViewButtons */
/* implementation */
void samplesviewbuttons_init(SamplesViewButtons* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component, STYLE_SAMPLESVIEW_BUTTONS);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_button_init_text(&self->load, &self->component, "file.load");
	psy_ui_button_init_text(&self->save, &self->component, "file.save");
	psy_ui_button_init_text(&self->duplicate, &self->component,
		"edit.duplicate");
	psy_ui_button_init_text(&self->del, &self->component, "edit.delete");	
}

/* SamplesSongImportView */
/* prototypes */
static void samplessongimportview_on_destroyed(SamplesSongImportView*);
static void samplessongimportview_on_load_song_button(SamplesSongImportView*,
	psy_ui_Component* sender);
static void samplessongimportview_oncopy(SamplesSongImportView*,
	psy_ui_Component* sender);
static void samplessongimportview_onsamplesboxchanged(SamplesSongImportView*,
	psy_ui_Component* sender);
static void samplessongimportview_on_load(SamplesSongImportView*,
	psy_Property* sender);
static void samplessongimportview_load_song(SamplesSongImportView*,
	const char* path);	

/* vtable */
static psy_ui_ComponentVtable samplessongimportview_vtable;
static bool samplessongimportview_vtable_initialized = FALSE;

static void samplessongimportview_vtable_init(SamplesSongImportView* self)
{
	if (!samplessongimportview_vtable_initialized) {
		samplessongimportview_vtable = *(self->component.vtable);		
		samplessongimportview_vtable.on_destroyed =
			(psy_ui_fp_component)
			samplessongimportview_on_destroyed;		
		samplessongimportview_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component,
		&samplessongimportview_vtable);
}


/* implementation */
void samplessongimportview_init(SamplesSongImportView* self,
	psy_ui_Component* parent, SamplesView* view, Workspace* workspace)
{		
	psy_ui_component_init(&self->component, parent, NULL);
	samplessongimportview_vtable_init(self);
	self->view = view;
	self->source = 0;
	self->workspace = workspace;
	psy_ui_component_init(&self->header, &self->component, NULL);
	psy_ui_component_set_align(&self->header, psy_ui_ALIGN_TOP);
	psy_ui_component_set_default_align(&self->header, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_set_padding(&self->header,
		psy_ui_margin_make_em(0.5, 0.0, 0.5, 0.0));
	psy_ui_label_init_text(&self->label, &self->header, "samplesview.source");
	psy_ui_label_init_text(&self->songname, &self->header,
		"samplesview.nosongloaded");
	psy_ui_label_set_char_number(&self->songname, 30);	
	psy_ui_button_init(&self->browse, &self->header);
	psy_ui_button_set_text(&self->browse, "samplesview.songselect");
	psy_signal_connect(&self->browse.signal_clicked, self,
		samplessongimportview_on_load_song_button);		
	/* bar */
	psy_ui_component_init(&self->bar, &self->component, NULL);
	psy_ui_component_set_align(&self->bar, psy_ui_ALIGN_LEFT);		
	psy_ui_component_set_minimum_size(&self->bar, psy_ui_size_make_em(12, 1));
	psy_ui_button_init(&self->add, &self->bar);
	psy_ui_button_set_text(&self->add, "samplesview.copy");	
	psy_ui_component_set_align(&self->add.component, psy_ui_ALIGN_CENTER);
	psy_signal_connect(&self->add.signal_clicked, self,
		samplessongimportview_oncopy);
	 /* samplesbox */
	samplesbox_init(&self->samplesbox, &self->component, NULL, workspace);
	psy_ui_component_set_align(&self->samplesbox.component,
		psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->samplesbox.signal_changed, self,
		samplessongimportview_onsamplesboxchanged);
	/* samplebox */
	wavebox_init(&self->samplebox, &self->component, workspace);
	psy_ui_component_set_align(&self->samplebox.component,
		psy_ui_ALIGN_BOTTOM);
	psy_property_init_type(&self->song_load, "load", PSY_PROPERTY_TYPE_STRING);
	psy_property_connect(&self->song_load, self,
		samplessongimportview_on_load);
}

void samplessongimportview_on_destroyed(SamplesSongImportView* self)
{
	psy_property_dispose(&self->song_load);
	if (self->source) {
		psy_audio_song_deallocate(self->source);
		self->source = NULL;
	}	
}


void samplessongimportview_on_load_song_button(SamplesSongImportView* self,
	psy_ui_Component* sender)
{
	if (keyboardmiscconfig_ft2_file_view(psycleconfig_misc(
			workspace_conf(self->workspace)))) {
		fileview_set_callbacks(self->workspace->fileview,
			&self->song_load, NULL);
		workspace_select_view(self->workspace, viewindex_make_all(
			VIEW_ID_FILEVIEW, 0, 0, psy_INDEX_INVALID));
	} else {
		
		psy_ui_OpenDialog dialog;
		
		psy_ui_opendialog_init_all(&dialog, 0,
			psy_ui_translate("samplesview.loadsong"),
			psy_audio_songfile_loadfilter(),
			psy_audio_songfile_standardloadfilter(),
			dirconfig_songs(&self->workspace->config.directories));
		if (psy_ui_opendialog_execute(&dialog)) {
			samplessongimportview_load_song(self, 
				psy_path_full(psy_ui_opendialog_path(&dialog)));			
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void samplessongimportview_on_load(SamplesSongImportView* self,
	psy_Property* sender)
{
	samplessongimportview_load_song(self, psy_property_item_str(sender));
	workspace_select_view(self->workspace, viewindex_make_all(VIEW_ID_SAMPLESVIEW,
		0, 0, psy_INDEX_INVALID));
}

void samplessongimportview_load_song(SamplesSongImportView* self,
	const char* path)
{	
	psy_audio_SongFile songfile;

	if (self->source) {
		psy_audio_song_deallocate(self->source);			
	}	
	self->source = psy_audio_song_allocinit(
		&self->workspace->player.machinefactory);
	psy_audio_songfile_init(&songfile);
	songfile.song = self->source;
	songfile.file = 0;		
	psy_audio_songfile_load(&songfile, path);
	if (!songfile.err) {
		psy_ui_label_set_text(&self->songname,
			self->source->properties.title);
		samplesbox_setsamples(&self->samplesbox, &self->source->samples);			
	} else {
		psy_ui_label_set_text(&self->songname,
			"No source song loaded");
	}
	psy_audio_songfile_dispose(&songfile);	
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

/* Header View */

/* prototypes */
static void samplesheaderview_onprevsample(SamplesHeaderView*, psy_ui_Component* sender);
static void samplesheaderview_onnextsample(SamplesHeaderView*, psy_ui_Component* sender);
static void samplesheaderview_oneditsamplename(SamplesHeaderView*, psy_ui_TextArea* sender);

/* implementation */
void samplesheaderview_init(SamplesHeaderView* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments, struct SamplesView* view, Workspace* workspace)
{	
	self->view = view;
	self->instruments = instruments;	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component, STYLE_SAMPLESVIEW_HEADER);	
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	/* sample name */
	psy_ui_label_init_text(&self->namelabel, &self->component,
		"samplesview.samplename");	
	psy_ui_textarea_init_single_line(&self->nameedit, &self->component);	
	psy_ui_textarea_set_char_number(&self->nameedit, 20);
	psy_signal_connect(&self->nameedit.signal_change, self,
		samplesheaderview_oneditsamplename);
	/* sample navigation */
	psy_ui_button_init(&self->prevbutton, &self->component);
	psy_ui_button_set_icon(&self->prevbutton, psy_ui_ICON_LESS);	
	psy_signal_connect(&self->prevbutton.signal_clicked, self,
		samplesheaderview_onprevsample);
	psy_ui_button_init(&self->nextbutton, &self->component);
	psy_ui_button_set_icon(&self->nextbutton, psy_ui_ICON_MORE);	
	psy_signal_connect(&self->nextbutton.signal_clicked, self,
		samplesheaderview_onnextsample);
	/* samplerate */
	psy_ui_label_init_text(&self->srlabel, &self->component,
		"samplesview.samplerate");	
	psy_ui_textarea_init_single_line(&self->sredit, &self->component);	
	psy_ui_textarea_set_char_number(&self->sredit, 8);
	/* numsamples */
	psy_ui_label_init_text(&self->numsamplesheaderlabel, &self->component,
		"samplesview.samples");
	psy_ui_label_init(&self->numsampleslabel, &self->component);
	psy_ui_label_prevent_translation(&self->numsampleslabel);
	psy_ui_label_set_char_number(&self->numsampleslabel, 10);
	/* channels */
	psy_ui_label_init(&self->channellabel, &self->component);
	psy_ui_label_prevent_translation(&self->channellabel);
	psy_ui_label_set_text(&self->channellabel, "");
	psy_ui_label_set_char_number(&self->channellabel, 7);	
}

void samplesheaderview_setsample(SamplesHeaderView* self, psy_audio_Sample* sample)
{
	char text[20];

	self->sample = sample;
	psy_signal_prevent(&self->nameedit.signal_change, self,
		samplesheaderview_oneditsamplename);
	psy_ui_textarea_set_text(&self->nameedit, self->sample ? sample->name : "");
	psy_signal_enable(&self->nameedit.signal_change, self,
		samplesheaderview_oneditsamplename);
	psy_snprintf(text, 20, "%d", self->sample ? self->sample->samplerate : 0);
	psy_ui_textarea_set_text(&self->sredit, text);
	psy_snprintf(text, 20, "%d", self->sample ? self->sample->numframes : 0);
	psy_ui_label_set_text(&self->numsampleslabel, text);
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
	psy_ui_label_set_text(&self->channellabel, text);
	if (self->sample) {
		psy_ui_component_enable_input(&self->component, 1);
	} else {
		psy_ui_component_prevent_input(&self->component, 1);
	}
}

void samplesheaderview_oneditsamplename(SamplesHeaderView* self, psy_ui_TextArea* sender)
{
	if (self->sample) {
		char text[40];
		psy_audio_SampleIndex index;

		index = samplesbox_selected(&self->view->samplesbox);
		psy_audio_sample_setname(self->sample, psy_ui_textarea_text(sender));
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

/* GeneralView */

/* prototypes */
static void generalview_setsample(SamplesGeneralView*, psy_audio_Sample*);
static void generalview_fillpandescription(SamplesGeneralView*, char* txt);
static void generalview_ondescribe(SamplesGeneralView*, psy_ui_Slider*, char* txt);
static void generalview_ontweak(SamplesGeneralView*, psy_ui_Slider*, float value);
static void generalview_onvalue(SamplesGeneralView*, psy_ui_Slider*, float* value);
static void generalview_updatetext(SamplesGeneralView*);
static void generalview_updatesliders(SamplesGeneralView*);

/* implementation */
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
		
	assert(workspace);

	self->sample = NULL;	
	self->notes_tab_mode = patternviewconfig_notetabmode(
		&workspace->config.visual.patview);
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_padding(&self->component,
		psy_ui_margin_make_em(1.0, 0.0, 0.0, 0.0));
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_slider_init(&self->defaultvolume, &self->component);
	psy_ui_slider_setdefaultvalue(&self->defaultvolume, 1.0);
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());	
	psy_ui_slider_init(&self->globalvolume, &self->component);
	psy_ui_slider_init(&self->panposition, &self->component);
	psy_ui_slider_init(&self->samplednote, &self->component);
	psy_ui_slider_setdefaultvalue(&self->samplednote, 60.0 / 119.0);
	psy_ui_slider_init(&self->pitchfinetune, &self->component);
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
	psy_ui_slider_set_text(&self->defaultvolume, 
		"samplesview.default-volume");
	psy_ui_slider_set_text(&self->globalvolume,
		"samplesview.global-volume");
	psy_ui_slider_set_text(&self->panposition,
		"samplesview.pan-position");
	psy_ui_slider_set_text(&self->samplednote,
		"samplesview.sampled-note");
	psy_ui_slider_set_text(&self->pitchfinetune,
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
		psy_ui_component_enable_input(&self->component, 1);
	} else {
		psy_ui_component_prevent_input(&self->component, 1);
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
				self->notes_tab_mode)
			: psy_dsp_notetostr(60, self->notes_tab_mode));
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

/* SamplesVibratoView */
/* prototypes */
static void vibratoview_setsample(SamplesVibratoView*, psy_audio_Sample*);
static void vibratoview_ondescribe(SamplesVibratoView*, psy_ui_Slider*, char* txt);
static void vibratoview_ontweak(SamplesVibratoView*, psy_ui_Slider*, float value);
static void vibratoview_onvalue(SamplesVibratoView*, psy_ui_Slider*, float* value);
static void vibratoview_onwaveformchange(SamplesVibratoView*, psy_ui_ComboBox* sender, int sel);
static psy_audio_WaveForms vibratoview_comboboxtowaveform(int combobox_index);
static int vibratoview_waveformtocombobox(psy_audio_WaveForms waveform);
/* implementation */
void samplesvibratoview_init(SamplesVibratoView* self, psy_ui_Component* parent, psy_audio_Player* player)
{	
	uintptr_t i;
	psy_ui_Slider* sliders[] = {
		&self->attack,
		&self->speed,
		&self->depth,		
	};

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_padding(&self->component,
		psy_ui_margin_make_em(1.0, 0.0, 0.0, 0.0));
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	self->sample = 0;
	self->player = player;
	/* header */
	{
		psy_ui_Margin header_margin;

		psy_ui_margin_init_em(&header_margin, 0.0, 2.0, 0.0, 0.0);
		psy_ui_component_init(&self->header, &self->component, NULL);		
		psy_ui_component_set_align(&self->header, psy_ui_ALIGN_TOP);		
		psy_ui_label_init(&self->waveformheaderlabel, &self->header);
		psy_ui_label_set_text(&self->waveformheaderlabel, "Waveform");
		psy_ui_component_set_align(&self->waveformheaderlabel.component,
			psy_ui_ALIGN_LEFT);
		psy_ui_component_set_margin(&self->waveformheaderlabel.component,
			header_margin);
		psy_ui_combobox_init(&self->waveformbox, &self->header);
		psy_ui_combobox_set_char_number(&self->waveformbox, 15);
		psy_ui_combobox_add_text(&self->waveformbox, "Sinus");
		psy_ui_combobox_add_text(&self->waveformbox, "Square");
		psy_ui_combobox_add_text(&self->waveformbox, "RampUp");
		psy_ui_combobox_add_text(&self->waveformbox, "RampDown");
		psy_ui_combobox_add_text(&self->waveformbox, "Random");
		psy_ui_component_set_align(&self->waveformbox.component,
			psy_ui_ALIGN_LEFT);
		psy_ui_combobox_select(&self->waveformbox, 0);
		psy_signal_connect(&self->waveformbox.signal_selchanged, self,
			vibratoview_onwaveformchange);
	}
	psy_ui_slider_init(&self->attack, &self->component);
	psy_ui_slider_set_text(&self->attack, "Attack");	
	psy_ui_slider_init(&self->speed, &self->component);
	psy_ui_slider_set_text(&self->speed,"Speed");
	psy_ui_slider_init(&self->depth, &self->component);
	psy_ui_slider_set_text(&self->depth, "Depth");	
	for (i = 0; i < 3; ++i) {		
		psy_ui_slider_setcharnumber(sliders[i], 16);
		psy_ui_slider_setdefaultvalue(sliders[i], 0.0);
		psy_ui_slider_connect(sliders[i], self,
			(ui_slider_fpdescribe)vibratoview_ondescribe,
			(ui_slider_fptweak)vibratoview_ontweak,
			(ui_slider_fpvalue)vibratoview_onvalue);
	}	
}

void vibratoview_setsample(SamplesVibratoView* self, psy_audio_Sample* sample)
{
	self->sample = sample;
	if (self->sample) {
		psy_ui_component_enable_input(&self->component, 1);
		psy_ui_combobox_select(&self->waveformbox,
			vibratoview_waveformtocombobox(self->sample->vibrato.type));
	} else {
		psy_ui_component_prevent_input(&self->component, 1);
		psy_ui_combobox_select(&self->waveformbox,
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

/* Waveloop Setting View */
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
	psy_ui_TextArea* sender);
static void samplesloopview_oneditchangedloopend(SamplesLoopView*,
	psy_ui_TextArea* sender);
static void samplesloopview_oneditchangedsustainstart(SamplesLoopView*,
	psy_ui_TextArea* sender);
static void samplesloopview_oneditchangedsustainend(SamplesLoopView*,
	psy_ui_TextArea* sender);
static void samplesloopview_on_timer(SamplesLoopView*, psy_ui_Component* sender, uintptr_t timerid);

void samplesloopview_init(SamplesLoopView* self, psy_ui_Component* parent,
	SamplesView* view)
{	
	self->view = view;		
	self->sample = NULL;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());
	/* continious loop */
	psy_ui_component_init(&self->cont, &self->component, NULL);
	psy_ui_component_set_default_align(&self->cont, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_set_padding(&self->cont,
		psy_ui_margin_make_em(0.25, 0.0, 0.25, 0.0));
	psy_ui_label_init_text(&self->loopheaderlabel, &self->cont,
		"samplesview.cont-loop");	
	psy_ui_label_set_char_number(&self->loopheaderlabel, 18);	
	psy_ui_combobox_init(&self->loopdir, &self->cont);
	psy_ui_combobox_add_text(&self->loopdir,
		psy_ui_translate("samplesview.disabled"));
	psy_ui_combobox_add_text(&self->loopdir,
		psy_ui_translate("samplesview.forward"));
	psy_ui_combobox_add_text(&self->loopdir,
		psy_ui_translate("samplesview.bidirection"));
	psy_ui_combobox_select(&self->loopdir, 0);
	psy_ui_combobox_set_char_number(&self->loopdir, 14);
	psy_ui_label_init(&self->loopstartlabel, &self->cont);
	psy_ui_label_set_text(&self->loopstartlabel, "samplesview.start");
	psy_ui_textarea_init_single_line(&self->loopstartedit, &self->cont);	
	psy_ui_textarea_set_char_number(&self->loopstartedit, 10);
	psy_ui_label_init(&self->loopendlabel, &self->cont);
	psy_ui_label_set_text(&self->loopendlabel, "samplesview.end");
	psy_ui_textarea_init_single_line(&self->loopendedit, &self->cont);	
	psy_ui_textarea_set_char_number(&self->loopendedit, 10);
	/* sustain loop */
	psy_ui_component_init(&self->sustain, &self->component, NULL);
	psy_ui_component_set_default_align(&self->sustain, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_set_padding(&self->sustain,
		psy_ui_margin_make_em(0.25, 0.0, 0.25, 0.0));
	psy_ui_label_init(&self->sustainloopheaderlabel, &self->sustain);
	psy_ui_label_set_text(&self->sustainloopheaderlabel, "samplesview.sustain-loop");
	psy_ui_label_set_char_number(&self->sustainloopheaderlabel, 18);
	psy_ui_combobox_init(&self->sustainloopdir, &self->sustain);
	psy_ui_combobox_add_text(&self->sustainloopdir,
		psy_ui_translate("samplesview.disabled"));
	psy_ui_combobox_add_text(&self->sustainloopdir,
		psy_ui_translate("samplesview.forward"));
	psy_ui_combobox_add_text(&self->sustainloopdir,
		psy_ui_translate("samplesview.bidirection"));	
	psy_ui_combobox_select(&self->sustainloopdir, 0);
	psy_ui_combobox_set_char_number(&self->sustainloopdir, 14);
	psy_ui_label_init(&self->sustainloopstartlabel, &self->sustain);
	psy_ui_label_set_text(&self->sustainloopstartlabel, "samplesview.start");
	psy_ui_textarea_init_single_line(&self->sustainloopstartedit, &self->sustain);	
	psy_ui_textarea_set_char_number(&self->sustainloopstartedit, 10);
	psy_ui_label_init(&self->sustainloopendlabel, &self->sustain);
	psy_ui_label_set_text(&self->sustainloopendlabel, "samplesview.end");
	psy_ui_textarea_init_single_line(&self->sustainloopendedit, &self->sustain);	
	psy_ui_textarea_set_char_number(&self->sustainloopendedit, 10);
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
		samplesloopview_on_timer);
	psy_audio_sampleloop_init(&self->currloop);
	psy_audio_sampleloop_init(&self->currsustainloop);
}

void samplesloopview_setsample(SamplesLoopView* self, psy_audio_Sample* sample)
{
	char tmp[40];

	self->sample = sample;
	if (self->sample) {
		psy_ui_component_start_timer(&self->component, 0, 200);
		psy_ui_component_enable_input(&self->component, 1);
		sprintf(tmp, "%d", (int)sample->loop.start);
		psy_ui_textarea_set_text(&self->loopstartedit, tmp);
		sprintf(tmp, "%d", (int)sample->loop.end);
		psy_ui_textarea_set_text(&self->loopendedit, tmp);
		sprintf(tmp, "%d", (int)sample->sustainloop.start);
		psy_ui_textarea_set_text(&self->sustainloopstartedit, tmp);
		sprintf(tmp, "%d", (int)sample->sustainloop.end);
		psy_ui_textarea_set_text(&self->sustainloopendedit, tmp);
		psy_ui_combobox_select(&self->loopdir,
			LoopTypeToComboBox(self->sample->loop.type));		
		psy_ui_combobox_select(&self->sustainloopdir,
			LoopTypeToComboBox(self->sample->sustainloop.type));
		self->currloop = self->sample->loop;
		self->currsustainloop = self->sample->sustainloop;
	} else {
		psy_ui_component_prevent_input(&self->component, 1);
		sprintf(tmp, "%d", 0);
		psy_ui_textarea_set_text(&self->loopstartedit, tmp);
		psy_ui_textarea_set_text(&self->loopendedit, tmp);
		psy_ui_textarea_set_text(&self->sustainloopstartedit, tmp);
		psy_ui_textarea_set_text(&self->sustainloopendedit, tmp);
		psy_ui_combobox_select(&self->loopdir,
			LoopTypeToComboBox(psy_audio_SAMPLE_LOOP_DO_NOT));
		psy_ui_combobox_select(&self->sustainloopdir,
			LoopTypeToComboBox(psy_audio_SAMPLE_LOOP_DO_NOT));
		psy_ui_component_stop_timer(&self->component, 0);
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

void samplesloopview_on_timer(SamplesLoopView* self, psy_ui_Component* sender,
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
#if PSYCLE_USE_TK == PSYCLE_TK_X11
	/* todo seqfault in psy_ui_component_input_prevented X11 Imp */
	return;	
#endif
	if (self->sample) {
		if (self->sample->loop.type == psy_audio_SAMPLE_LOOP_DO_NOT) {
			psy_ui_component_prevent_input(&self->loopstartedit.component, 0);
			psy_ui_component_prevent_input(&self->loopendedit.component, 0);
		} else {
			psy_ui_component_enable_input(&self->loopstartedit.component, 0);
			psy_ui_component_enable_input(&self->loopendedit.component, 0);
		}
		if (self->sample->sustainloop.type == psy_audio_SAMPLE_LOOP_DO_NOT) {
			psy_ui_component_prevent_input(&self->sustainloopstartedit.component, 0);
			psy_ui_component_prevent_input(&self->sustainloopendedit.component, 0);
		} else {
			psy_ui_component_enable_input(&self->sustainloopstartedit.component, 0);
			psy_ui_component_enable_input(&self->sustainloopendedit.component, 0);
		}
	} else {
		psy_ui_component_prevent_input(&self->loopstartedit.component, 0);
		psy_ui_component_prevent_input(&self->loopendedit.component, 0);
		psy_ui_component_prevent_input(&self->sustainloopstartedit.component, 0);
		psy_ui_component_prevent_input(&self->sustainloopendedit.component, 0);	
	}
}

psy_audio_SampleLoopType ComboBoxToLoopType(int combobox_index)
{
	psy_audio_SampleLoopType rv;
	
	rv = psy_audio_SAMPLE_LOOP_DO_NOT;			
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
	psy_ui_TextArea* sender)
{
	if (self->sample) {
		self->sample->loop.start = atoi(psy_ui_textarea_text(sender));
		psy_ui_component_invalidate(&self->view->wavebox.component);
		psy_ui_component_invalidate(&self->view->sampleeditor.samplebox.component);
	}
}

void samplesloopview_oneditchangedloopend(SamplesLoopView* self,
	psy_ui_TextArea* sender)
{				
	if (self->sample) {
		self->sample->loop.end = atoi(psy_ui_textarea_text(sender));
		psy_ui_component_invalidate(&self->view->wavebox.component);
		psy_ui_component_invalidate(&self->view->sampleeditor.samplebox.component);
	}
}

void samplesloopview_oneditchangedsustainstart(SamplesLoopView* self,
	psy_ui_TextArea* sender)
{
	if (self->sample) {
		self->sample->sustainloop.start = atoi(psy_ui_textarea_text(sender));
		psy_ui_component_invalidate(&self->view->wavebox.component);
		psy_ui_component_invalidate(&self->view->sampleeditor.samplebox.component);
	}
}

void samplesloopview_oneditchangedsustainend(SamplesLoopView* self,
	psy_ui_TextArea* sender)
{
	if (self->sample) {
		self->sample->sustainloop.end = atoi(psy_ui_textarea_text(sender));		
		psy_ui_component_invalidate(&self->view->wavebox.component);
		psy_ui_component_invalidate(&self->view->sampleeditor.samplebox.component);
	}
}

/* SamplesView */
/* prototypes */
static void samplesview_ondestroyed(SamplesView*);
static void samplesview_onsamplesboxchanged(SamplesView*, psy_ui_Component* sender);
static void samplesview_onloadsamplebtn(SamplesView*, psy_ui_Component* sender);
static void samplesview_on_load_sample(SamplesView*, psy_Property* sender);
static void samplesview_load_sample(SamplesView*, const char* path);
static void samplesview_on_save_sample(SamplesView*, psy_Property* sender);
static void samplesview_onsavesamplebtn(SamplesView*, psy_ui_Component* sender);
static void samplesview_save_sample(SamplesView*, const char* path);
static void samplesview_ondeletesample(SamplesView*, psy_ui_Component* sender);
static void samplesview_onduplicatesample(SamplesView*, psy_ui_Component* sender);
static void samplesview_on_song_changed(SamplesView*,
	psy_audio_Player* sender);
static void samplesview_oninstrumentslotchanged(SamplesView*,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex* slot);
static uintptr_t samplesview_freesampleslot(SamplesView*, uintptr_t startslot,
	uintptr_t maxslots);
static void samplesview_onsamplemodified(SamplesView*, SampleEditor* sender,
	psy_audio_Sample*);
static void samplesview_onconfigure(SamplesView*, PatternViewConfig*,
	psy_Property* property);
static void samplesview_onresamplermethodchanged(SamplesView*,
	psy_ui_Component* sender, int index);
static void samplesview_onselectsection(SamplesView*, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options);

/* vtable */
static psy_ui_ComponentVtable samplesview_vtable;
static bool samplesview_vtable_initialized = FALSE;

static void samplesview_vtable_init(SamplesView* self)
{
	if (!samplesview_vtable_initialized) {
		samplesview_vtable = *(self->component.vtable);		
		samplesview_vtable.on_destroyed =
			(psy_ui_fp_component)
			samplesview_ondestroyed;		
		samplesview_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(samplesview_base(self), &samplesview_vtable);
}

/* implementation */
void samplesview_init(SamplesView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	psy_ui_Margin margin;
	psy_ui_Margin waveboxmargin;	
		
	psy_ui_component_init(&self->component, parent, NULL);
	samplesview_vtable_init(self);
	psy_ui_component_set_id(samplesview_base(self), VIEW_ID_SAMPLESVIEW);
	psy_ui_component_set_title(samplesview_base(self), "main.samples");
	psy_ui_margin_init_em(&waveboxmargin, 0.5, 1.0, 0.0, 0.5);
	self->workspace = workspace;	
	psy_ui_component_set_style_type(&self->component, STYLE_SAMPLESVIEW);	
	/* header */
	samplesheaderview_init(&self->header, &self->component,
		&workspace->song->instruments, self, workspace);	
	psy_ui_component_set_align(&self->header.component, psy_ui_ALIGN_TOP);
	/* left */
	psy_ui_component_init(&self->left, &self->component, NULL);
	psy_ui_component_set_align(&self->left, psy_ui_ALIGN_LEFT);	
	samplesviewbuttons_init(&self->buttons, &self->left, workspace);
	psy_ui_component_set_align(&self->buttons.component, psy_ui_ALIGN_TOP);
	/* tabbarparent */
	psy_ui_tabbar_init(&self->clienttabbar, tabbarparent);
	psy_ui_component_set_id(psy_ui_tabbar_base(&self->clienttabbar),
		VIEW_ID_SAMPLESVIEW);
	psy_ui_component_set_align(&self->clienttabbar.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(&self->clienttabbar.component);
	psy_ui_tabbar_append_tabs(&self->clienttabbar, "Properties", "Import",
		"Editor", NULL);
	samplesbox_init(&self->samplesbox, &self->left,
		&workspace->song->samples, workspace);
	psy_ui_component_set_align(&self->samplesbox.component,
		psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->buttons.load.signal_clicked, self,
		samplesview_onloadsamplebtn);
	psy_signal_connect(&self->buttons.save.signal_clicked, self,
		samplesview_onsavesamplebtn);
	psy_signal_connect(&self->buttons.duplicate.signal_clicked, self,
		samplesview_onduplicatesample);
	psy_signal_connect(&self->buttons.del.signal_clicked, self,
		samplesview_ondeletesample);
	/* client */
	/* right */
	psy_ui_component_init(&self->right, &self->component, NULL);
	psy_ui_component_set_align(&self->right, psy_ui_ALIGN_CLIENT);
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 0.0, 2.0);
	psy_ui_component_set_margin(&self->right, margin);
	psy_ui_notebook_init(&self->clientnotebook, &self->right);
	psy_ui_component_set_align(&self->clientnotebook.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_init(&self->client, &self->clientnotebook.component, NULL);
	psy_ui_tabbar_init(&self->tabbar, &self->client);
	psy_ui_component_set_align(psy_ui_tabbar_base(&self->tabbar), psy_ui_ALIGN_TOP);	
	psy_ui_tabbar_append(&self->tabbar, "General", psy_INDEX_INVALID,
		psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());
	psy_ui_tabbar_append(&self->tabbar, "Vibrato", psy_INDEX_INVALID,
		psy_INDEX_INVALID, psy_INDEX_INVALID, psy_ui_colour_white());
	psy_ui_notebook_init(&self->notebook, &self->client);
	psy_ui_component_set_margin(psy_ui_notebook_base(&self->notebook),
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.5));
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_TOP);	
	psy_ui_notebook_connect_controller(&self->notebook,
		&self->tabbar.signal_change);
	/* GeneralView */
	samplesgeneralview_init(&self->general, psy_ui_notebook_base(
		&self->notebook), workspace);
	psy_ui_component_set_align(&self->general.component, psy_ui_ALIGN_TOP);
	/* VibratoView */
	samplesvibratoview_init(&self->vibrato, psy_ui_notebook_base(&self->notebook),
		&workspace->player);
	psy_ui_component_set_align(&self->vibrato.component, psy_ui_ALIGN_TOP);
	psy_ui_notebook_select(&self->notebook, 0);
	wavebox_init(&self->wavebox, &self->client, workspace);
	psy_ui_component_set_align(&self->wavebox.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_margin(&self->wavebox.component, waveboxmargin);
	/* LoopView */
	samplesloopview_init(&self->waveloop, &self->right, self);
	psy_ui_component_set_align(&self->waveloop.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_margin(&self->waveloop.component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.5));
	psy_signal_connect(&self->samplesbox.signal_changed, self,
		samplesview_onsamplesboxchanged);
	psy_signal_connect(&workspace->song->instruments.signal_slotchange, self,
		samplesview_oninstrumentslotchanged);
	/* ImportView */
	samplessongimportview_init(&self->songimport,
		&self->clientnotebook.component, self, workspace);
	/* WaveEditorView */
	sampleeditor_init(&self->sampleeditor, &self->clientnotebook.component,
		workspace);
	psy_ui_notebook_select(&self->clientnotebook, 0);
	psy_signal_connect(&workspace->player.signal_song_changed, self,
		samplesview_on_song_changed);
	psy_ui_notebook_select(&self->clientnotebook, 0);
	psy_ui_notebook_connect_controller(&self->clientnotebook,
		&self->clienttabbar.signal_change);
	samplesview_setsample(self, psy_audio_sampleindex_make(0, 0));
	psy_signal_connect(&self->sampleeditor.signal_samplemodified, self,
		samplesview_onsamplemodified);	
	psy_ui_tabbar_select(&self->tabbar, 0);
	psy_signal_connect(&samplesview_base(self)->signal_selectsection, self,
		samplesview_onselectsection);
	psy_property_init_type(&self->sample_load, "load",
		PSY_PROPERTY_TYPE_STRING);
	psy_property_connect(&self->sample_load, self, samplesview_on_load_sample);
	psy_property_init_type(&self->sample_save, "save",
		PSY_PROPERTY_TYPE_STRING);
	psy_property_connect(&self->sample_save, self, samplesview_on_save_sample);
}

void samplesview_ondestroyed(SamplesView* self)
{	
	psy_property_dispose(&self->sample_load);
	psy_property_dispose(&self->sample_save);
}

void samplesview_onconfigure(SamplesView* self, PatternViewConfig* config,
	psy_Property* property)
{
	self->general.notes_tab_mode = patternviewconfig_notetabmode(config);
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
	workspace_song(self->workspace)->samples.selected = index;
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

void samplesview_onloadsamplebtn(SamplesView* self, psy_ui_Component* sender)
{
	if (keyboardmiscconfig_ft2_file_view(psycleconfig_misc(
			workspace_conf(self->workspace)))) {		
		psy_Property types;
		
		fileview_set_callbacks(self->workspace->fileview,
			&self->sample_load, NULL);
		psy_property_init_type(&types, "types", PSY_PROPERTY_TYPE_CHOICE);		
		psy_property_set_text(psy_property_set_id(
			psy_property_append_str(&types, "wav", "*.wav"),
			FILEVIEWFILTER_PSY), "Wave");
		psy_property_set_text(psy_property_set_id(
			psy_property_append_str(&types, "iff", "*.iff"),
			FILEVIEWFILTER_PSY), "IFF");
		fileview_set_filter(self->workspace->fileview, &types);
		psy_property_dispose(&types);
		workspace_select_view(self->workspace, viewindex_make_all(
			VIEW_ID_FILEVIEW, 0, 0, psy_INDEX_INVALID));
	} else {		
		psy_ui_OpenDialog dialog;
		static char filter[] =
			"Wav Files (*.wav)|*.wav|"
			"IFF psy_audio_Samples (*.iff)|*.iff|"
			"All Files (*.*)|*.*";
		
		psy_ui_opendialog_init_all(&dialog, 0, "Load Sample", filter, "WAV",
			dirconfig_samples(&self->workspace->config.directories));
		if (psy_ui_opendialog_execute(&dialog)) {
			samplesview_load_sample(self, psy_path_full(psy_ui_opendialog_path(
				&dialog)));			
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void samplesview_on_load_sample(SamplesView* self, psy_Property* sender)
{	
	samplesview_load_sample(self, psy_property_item_str(sender));	
	workspace_select_view(self->workspace, viewindex_make_all(VIEW_ID_SAMPLESVIEW,
		0, 0, psy_INDEX_INVALID));
}

void samplesview_load_sample(SamplesView* self, const char* path)
{
	if (workspace_song(self->workspace)) {		
		psy_audio_Sample* sample;
		psy_audio_SampleIndex index;
		psy_audio_Instrument* instrument;
		psy_audio_InstrumentEntry entry;			

		sample = psy_audio_sample_allocinit(0);			
		psy_audio_sample_load(sample, path);
		index = samplesbox_selected(&self->samplesbox);
		psy_audio_samples_insert(&workspace_song(self->workspace)->samples,
			sample, index);
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
}

void samplesview_onsavesamplebtn(SamplesView* self, psy_ui_Component* sender)
{
	if (keyboardmiscconfig_ft2_file_view(psycleconfig_misc(
			workspace_conf(self->workspace)))) {
		if (wavebox_sample(&self->wavebox)) {
			fileview_set_callbacks(self->workspace->fileview, NULL,
				&self->sample_save);
			workspace_select_view(self->workspace, viewindex_make_all(
				VIEW_ID_FILEVIEW, 0, 0, psy_INDEX_INVALID));			
		}
	} else {
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
				psy_path_full(psy_ui_savedialog_path(&dialog)));	
		}
		psy_ui_savedialog_dispose(&dialog);
	}
}

void samplesview_on_save_sample(SamplesView* self, psy_Property* sender)
{
	samplesview_save_sample(self, psy_property_item_str(sender));	
	workspace_select_view(self->workspace, viewindex_make_all(VIEW_ID_SAMPLESVIEW,
		0, 0, psy_INDEX_INVALID));
}

void samplesview_save_sample(SamplesView* self, const char* path)
{
	if (wavebox_sample(&self->wavebox)) {
		psy_audio_sample_save(wavebox_sample(&self->wavebox), path);
	}
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
	if (workspace_song(self->workspace)) {
		return psy_audio_samples_freeslot(
			&workspace_song(self->workspace)->samples, startslot, maxslots);		
	}
	return maxslots;
}

void samplesview_onsamplemodified(SamplesView* self, SampleEditor* sender, psy_audio_Sample* sample)
{
	samplesheaderview_setsample(&self->header, sample);
	generalview_setsample(&self->general, sample);
	vibratoview_setsample(&self->vibrato, sample);
	samplesloopview_setsample(&self->waveloop, sample);
}

void samplesview_on_song_changed(SamplesView* self, psy_audio_Player* sender)
{
	if (sender->song) {
		psy_signal_connect(&sender->song->instruments.signal_slotchange, self,
			samplesview_oninstrumentslotchanged);
		samplesbox_setsamples(&self->samplesbox, &sender->song->samples);
	} else {
		samplesbox_setsamples(&self->samplesbox, NULL);
	}
	samplesview_setsample(self, psy_audio_sampleindex_make(0, 0));
}

void samplesview_onresamplermethodchanged(SamplesView* self,
	psy_ui_Component* sender, int index)
{
	wavebox_setquality(&self->wavebox, (psy_dsp_ResamplerQuality)index);
	sampleeditor_setquality(&self->sampleeditor, (psy_dsp_ResamplerQuality)index);
}

void samplesview_onselectsection(SamplesView* self, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options)
{
	psy_ui_tabbar_select(&self->clienttabbar, section);
}

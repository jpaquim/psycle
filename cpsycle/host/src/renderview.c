// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "renderview.h"
#include <fileoutdriver.h>

#include <string.h>

static void renderview_ondestroy(RenderView*, psy_ui_Component* sender);
static void renderview_makeproperties(RenderView*);
static void renderview_onsettingsviewchanged(RenderView*, SettingsView* sender,
	psy_Properties*);
static void renderview_render(RenderView*);
static void renderview_onstoprendering(RenderView*, psy_AudioDriver* sender);

void renderview_init(RenderView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_signal_connect(&self->component.signal_destroy, self,
		renderview_ondestroy);
	psy_ui_component_enablealign(&self->component);
	renderview_makeproperties(self);
	settingsview_init(&self->view, &self->component, tabbarparent,
		self->properties);
	psy_signal_connect(&self->view.signal_changed, self,
		renderview_onsettingsviewchanged);
	psy_ui_component_setalign(&self->view.component, psy_ui_ALIGN_CLIENT);
	self->fileoutdriver = psy_audio_create_fileout_driver();
}

void renderview_ondestroy(RenderView* self, psy_ui_Component* sender)
{
	properties_free(self->properties);	
	self->fileoutdriver->deallocate(self->fileoutdriver);
}

void renderview_makeproperties(RenderView* self)
{
	psy_Properties* filesave;
	psy_Properties* savechoice;	
	psy_Properties* record;
	psy_Properties* recordchoice;
	psy_Properties* recordpatnum;
	psy_Properties* recordseqpos;
	psy_Properties* quality;
	psy_Properties* dither;
	psy_Properties* ditherpdf;
	psy_Properties* dithernoiseshape;
	psy_Properties* actions;

	self->properties = psy_properties_create();
	actions = psy_properties_settext(
		psy_properties_create_section(self->properties, "actions"),
		"Render");
	psy_properties_settext(
		psy_properties_append_action(actions, "savewave"),
		"Save wave");
	filesave = psy_properties_settext(
		psy_properties_create_section(self->properties, "filesave"),
		"File");
	psy_properties_settext(
		psy_properties_append_string(filesave, "filesave-outputpath", "Untitled.wav"),
		"Output Path");
	savechoice = psy_properties_settext(
		psy_properties_append_choice(filesave, "filesave-choice", 0),
		"Save each unmuted");
	psy_properties_settext(
		psy_properties_append_string(savechoice, "filesave-channel", ""),
		"input to master as a separated wav (wire number"
		" will be appended to filename)"
	);
	psy_properties_settext(
		psy_properties_append_string(savechoice, "filesave-track", ""),
		"track as a separated"
		"wav (track number will be appended to filename) ** may suffer from"
		"'delay bleed' - insert silence at the end of your file if this is a "
		"problem."
	);
	psy_properties_settext(
		psy_properties_append_string(savechoice, "filesave-generator", ""),
		"generator as a separated wav (generator number will"
		"be appended to filename) ** may suffer from 'delay bleed' - insert"
		" silence at the end of your file if this is a problem."
	);
	record = psy_properties_settext(
		psy_properties_create_section(self->properties, "record"),
		"Selection");	
	recordchoice = psy_properties_settext(
		psy_properties_append_choice(record, "record", 0),
		"Record");
	psy_properties_settext(
		psy_properties_append_string(recordchoice, "record-entiresong", ""),
		"the entire song"
	);
	recordpatnum = psy_properties_settext(
		psy_properties_append_string(recordchoice, "record-pattern", ""),
		"pattern"
	);
	psy_properties_settext(
		psy_properties_append_int(recordpatnum, "record-pattern-number", 0, 0, 256),
		"number"
	);
	recordseqpos = psy_properties_settext(
		psy_properties_append_string(recordchoice, "record-seqpos", ""),
		"sequence positions"
	);
	psy_properties_settext(
		psy_properties_append_int(recordseqpos, "record-seqpos-from", 0, 0, 256),
		"from"
	);
	psy_properties_settext(
		psy_properties_append_int(recordseqpos, "record-seqpos-to", 0, 0, 256),
		"to"
	);
	quality = psy_properties_settext(
		psy_properties_create_section(self->properties, "quality"),
		"Quality");	
	psy_properties_settext(
		psy_properties_append_int(quality, "quality-samplerate", 44100, 0, 96000),
		"Sample Rate"
	);
	psy_properties_settext(
		psy_properties_append_int(quality, "quality-bitdepth", 8, 8, 16),
		"Bit Depth"
	);
	psy_properties_settext(
		psy_properties_append_int(quality, "quality-channels", 2, 0, 2),
		"Channels"
	);
	dither = psy_properties_settext(
		psy_properties_create_section(self->properties, "dither"),
		"Dither");
	psy_properties_settext(
		psy_properties_append_bool(dither, "enable", FALSE),
		"Enable"
	);
	ditherpdf = psy_properties_settext(
		psy_properties_append_choice(dither, "pdf", 0),
		"Prob. Distribution");
	psy_properties_settext(
		psy_properties_append_string(ditherpdf, "triangular", ""),
		"Triangular");
	psy_properties_settext(
		psy_properties_append_string(ditherpdf, "rectangular", ""),
		"Rectangular");
	psy_properties_settext(
		psy_properties_append_string(ditherpdf, "gaussian", ""),
		"Gaussian");
	dithernoiseshape = psy_properties_settext(
		psy_properties_append_choice(dither, "noiseshape", 0),
		"Noise-shaping");
	psy_properties_settext(
		psy_properties_append_string(dithernoiseshape, "none", ""),
		"None");
	psy_properties_settext(
		psy_properties_append_string(dithernoiseshape, "highpass", ""),
		"High-Pass Contour");
}

void renderview_onsettingsviewchanged(RenderView* self, SettingsView* sender,
	psy_Properties* property)
{
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_ACTION) {
		if (strcmp(psy_properties_key(property), "savewave") == 0) {
			renderview_render(self);
		}
	}
}

void renderview_render(RenderView* self)
{	
	self->curraudiodriver = psy_audio_player_audiodriver(&self->workspace->player);
	self->curraudiodriver->close(self->curraudiodriver);
	psy_audio_player_setaudiodriver(&self->workspace->player, self->fileoutdriver);
	self->restoreloopmode = self->workspace->player.sequencer.looping;
	self->workspace->player.sequencer.looping = 0;
	self->restoredither = psy_dsp_dither_settings(&self->workspace->player.dither);
	self->restoredodither = self->workspace->player.dodither;
	if (psy_properties_bool(self->properties, "dither.enable", FALSE) != FALSE) {
		psy_Properties* pdf;
		psy_Properties* noiseshaping;
		psy_dsp_DitherPdf dither_pdf = psy_dsp_DITHER_PDF_TRIANGULAR;
		psy_dsp_DitherNoiseShape dither_noiseshape = psy_dsp_DITHER_NOISESHAPE_NONE;
		
		pdf = psy_properties_read(self->properties, "dither.pdf");
		if (pdf) {
			dither_pdf = (psy_dsp_DitherPdf)pdf->item.value.i;
		}
		noiseshaping = psy_properties_read(self->properties,
			"dither.noiseshape");
		if (noiseshaping) {
			dither_noiseshape = (psy_dsp_DitherNoiseShape)noiseshaping->item.value.i;
		}
		psy_audio_player_setdither(&self->workspace->player, 16, dither_pdf,
			dither_noiseshape);
		psy_audio_player_enabledither(&self->workspace->player);
	}
	psy_audio_player_setposition(&self->workspace->player, 0);
	psy_audio_player_start(&self->workspace->player);
	psy_signal_connect(&self->fileoutdriver->signal_stop, self,
		renderview_onstoprendering);
	self->fileoutdriver->open(self->fileoutdriver);
}

void renderview_onstoprendering(RenderView* self, psy_AudioDriver* sender)
{
	psy_audio_player_stop(&self->workspace->player);
	psy_audio_player_setaudiodriver(&self->workspace->player,
		self->curraudiodriver);
	self->workspace->player.sequencer.looping = self->restoreloopmode;	
	if (!self->restoredodither) {
		psy_audio_player_disabledither(&self->workspace->player);
	}
	psy_dsp_dither_setsettings(&self->workspace->player.dither,
		self->restoredither);	
	self->curraudiodriver->open(self->curraudiodriver);
	self->fileoutdriver->close(self->fileoutdriver);
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "renderview.h"
#include <fileoutdriver.h>
#include "../../driver/audiodriversettings.h"

#include <string.h>

#include "../../detail/trace.h"

static void renderview_ondestroy(RenderView*, psy_ui_Component* sender);
static void renderview_makeproperties(RenderView*);
static void renderview_onsettingsviewchanged(RenderView*, PropertiesView* sender,
	psy_Property*);
static void renderview_render(RenderView*);
static void renderview_onstoprendering(RenderView*, psy_AudioDriver* sender);
static void renderview_onfocus(RenderView*, psy_ui_Component* sender);

void renderview_init(RenderView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_signal_connect(&self->component.signal_destroy, self,
		renderview_ondestroy);
	psy_signal_connect(&self->component.signal_focus,
		self, renderview_onfocus);
	renderview_makeproperties(self);
	propertiesview_init(&self->view, &self->component, tabbarparent,
		self->properties, workspace);
	psy_signal_connect(&self->view.signal_changed, self,
		renderview_onsettingsviewchanged);
	psy_ui_component_setalign(&self->view.component, psy_ui_ALIGN_CLIENT);
	self->fileoutdriver = psy_audio_create_fileout_driver();	
}

void renderview_ondestroy(RenderView* self, psy_ui_Component* sender)
{
	psy_property_deallocate(self->properties);	
	psy_audiodriver_deallocate(self->fileoutdriver);
}

void renderview_makeproperties(RenderView* self)
{
	psy_Property* filesave;
	psy_Property* savechoice;	
	psy_Property* record;
	psy_Property* recordchoice;
	psy_Property* recordpatnum;
	psy_Property* recordseqpos;
	psy_Property* quality;
	psy_Property* channelchoice;
	psy_Property* dither;
	psy_Property* ditherpdf;
	psy_Property* dithernoiseshape;
	psy_Property* actions;

	self->properties = psy_property_allocinit_key(NULL);
	actions = psy_property_settext(
		psy_property_append_section(self->properties, "actions"),
		"render.render");
	psy_property_settext(
		psy_property_append_action(actions, "savewave"),
		"render.save-wave");
	filesave = psy_property_settext(
		psy_property_append_section(self->properties, "filesave"),
		"render.file");
	psy_property_settext(
		psy_property_append_string(filesave, "outputpath", "Untitled.wav"),
		"render.output-path");
	savechoice = psy_property_settext(
		psy_property_append_choice(filesave, "filesave-choice", 0),
		"render.save-each-unmuted");
	psy_property_settext(
		psy_property_append_string(savechoice, "filesave-channel", ""),
		"input to master as a separated wav (wire number"
		" will be appended to filename)"
	);
	psy_property_settext(
		psy_property_append_string(savechoice, "filesave-track", ""),
		"track as a separated"
		"wav (track number will be appended to filename) ** may suffer from"
		"'delay bleed' - insert silence at the end of your file if this is a "
		"problem"
	);
	psy_property_settext(
		psy_property_append_string(savechoice, "filesave-generator", ""),
		"generator as a separated wav (generator number will"
		"be appended to filename) ** may suffer from 'delay bleed' - insert"
		" silence at the end of your file if this is a problem"
	);
	record = psy_property_settext(
		psy_property_append_section(self->properties, "record"),
		"render.selection");	
	recordchoice = psy_property_settext(
		psy_property_append_choice(record, "record", 0),
		"render.record");
	psy_property_settext(
		psy_property_append_string(recordchoice, "record-entiresong", ""),
		"render.entire-song"
	);
	recordpatnum = psy_property_settext(
		psy_property_append_string(recordchoice, "record-pattern", ""),
		"render.pattern"
	);
	psy_property_settext(
		psy_property_append_int(recordpatnum, "record-pattern-number", 0, 0, 256),
		"render.number"
	);
	recordseqpos = psy_property_settext(
		psy_property_append_string(recordchoice, "record-seqpos", ""),
		"render.sequence-positions"
	);
	psy_property_settext(
		psy_property_append_int(recordseqpos, "record-seqpos-from", 0, 0, 256),
		"render.from"
	);
	psy_property_settext(
		psy_property_append_int(recordseqpos, "record-seqpos-to", 0, 0, 256),
		"render.to"
	);
	quality = psy_property_settext(
		psy_property_append_section(self->properties, "quality"),
		"render.quality");	
	psy_property_settext(
		psy_property_append_int(quality, "samplerate", 44100, 0, 96000),
		"render.samplerate"
	);
	psy_property_settext(
		psy_property_append_int(quality, "bitdepth", 16, 8, 16),
		"render.bitdepth"
	);
	channelchoice = psy_property_settext(
		psy_property_append_choice(quality, "channels",
			psy_AUDIODRIVERCHANNELMODE_STEREO),
		"render.channels"
	);
	psy_property_settext(
		psy_property_append_string(channelchoice, "mono_mix", ""),
		"Mono (Mix)");
	psy_property_settext(
		psy_property_append_string(channelchoice, "mono_left", ""),
		"Mono (Left");
	psy_property_settext(
		psy_property_append_string(channelchoice, "mono_right", ""),
		"Mono (Right)");
	psy_property_settext(
		psy_property_append_string(channelchoice, "stereo", ""),
		"Stereo");
	dither = psy_property_settext(
		psy_property_append_section(self->properties, "dither"),
		"Dither");
	psy_property_settext(
		psy_property_append_bool(dither, "enable", FALSE),
		"Enable"
	);
	ditherpdf = psy_property_settext(
		psy_property_append_choice(dither, "pdf", 0),
		"render.pdf");
	psy_property_settext(
		psy_property_append_string(ditherpdf, "triangular", ""),
		"render.triangular");
	psy_property_settext(
		psy_property_append_string(ditherpdf, "rectangular", ""),
		"render.rectangular");
	psy_property_settext(
		psy_property_append_string(ditherpdf, "gaussian", ""),
		"render.gaussian");
	dithernoiseshape = psy_property_settext(
		psy_property_append_choice(dither, "noiseshape", 0),
		"render.noise-shaping");
	psy_property_settext(
		psy_property_append_string(dithernoiseshape, "none", ""),
		"render.none");
	psy_property_settext(
		psy_property_append_string(dithernoiseshape, "highpass", ""),
		"render.high-pass-contour");
}

void renderview_onsettingsviewchanged(RenderView* self, PropertiesView* sender,
	psy_Property* property)
{
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_ACTION) {
		if (strcmp(psy_property_key(property), "savewave") == 0) {
			renderview_render(self);
		}
	}
}

void renderview_render(RenderView* self)
{	
	psy_Property* driverconfig;
	 
	self->curraudiodriver = psy_audio_player_audiodriver(workspace_player(self->workspace));
	psy_audiodriver_close(self->curraudiodriver);
	psy_audio_player_setaudiodriver(workspace_player(self->workspace), self->fileoutdriver);	
	driverconfig = psy_property_clone(psy_audiodriver_configuration(
		self->fileoutdriver));
	psy_property_set_str(driverconfig, "outputpath",
		psy_property_at_str(self->properties, "filesave.outputpath", "Untitled.wav"));
	psy_property_set_int(driverconfig, "samplerate",
		psy_property_at_int(self->properties, "quality.samplerate", 44100));
	psy_property_set_int(driverconfig, "bitdepth",
		psy_property_at_int(self->properties, "quality.bitdepth", 16));
	psy_property_set_int(driverconfig, "channels",
		psy_property_at_int(self->properties, "quality.channels",
			psy_AUDIODRIVERCHANNELMODE_STEREO));
	psy_audiodriver_configure(self->fileoutdriver, driverconfig);
	psy_property_deallocate(driverconfig);
	self->restoreloopmode = workspace_player(self->workspace)->sequencer.looping;
	workspace_player(self->workspace)->sequencer.looping = 0;
	self->restoredither = psy_dsp_dither_settings(&workspace_player(self->workspace)->dither);
	self->restoredodither = workspace_player(self->workspace)->dodither;
	if (psy_property_at_bool(self->properties, "dither.enable", FALSE) != FALSE) {
		psy_Property* pdf;
		psy_Property* noiseshaping;
		psy_dsp_DitherPdf dither_pdf = psy_dsp_DITHER_PDF_TRIANGULAR;
		psy_dsp_DitherNoiseShape dither_noiseshape = psy_dsp_DITHER_NOISESHAPE_NONE;
		
		pdf = psy_property_at(self->properties, "dither.pdf", PSY_PROPERTY_TYPE_NONE);
		if (pdf) {
			dither_pdf = (psy_dsp_DitherPdf)psy_property_item_int(pdf);
		}
		noiseshaping = psy_property_at(self->properties,
			"dither.noiseshape", PSY_PROPERTY_TYPE_NONE);
		if (noiseshaping) {
			dither_noiseshape = (psy_dsp_DitherNoiseShape)
				psy_property_item_int(noiseshaping);
		}
		psy_audio_player_setdither(workspace_player(self->workspace), 16, dither_pdf,
			dither_noiseshape);
		psy_audio_player_enabledither(workspace_player(self->workspace));
	}
	psy_audio_player_setposition(workspace_player(self->workspace), 0);
	psy_audio_player_start(workspace_player(self->workspace));
	psy_signal_connect(&self->fileoutdriver->signal_stop, self,
		renderview_onstoprendering);
	psy_audiodriver_open(self->fileoutdriver);
}

void renderview_onstoprendering(RenderView* self, psy_AudioDriver* sender)
{
	psy_audio_player_stop(workspace_player(self->workspace));
	psy_audio_player_setaudiodriver(workspace_player(self->workspace),
		self->curraudiodriver);
	workspace_player(self->workspace)->sequencer.looping = self->restoreloopmode;
	if (!self->restoredodither) {
		psy_audio_player_disabledither(workspace_player(self->workspace));
	}
	psy_dsp_dither_setsettings(&workspace_player(self->workspace)->dither,
		self->restoredither);	
	psy_audiodriver_open(self->curraudiodriver);
	psy_audiodriver_close(self->fileoutdriver);
}

void renderview_onfocus(RenderView* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(&self->view.component);
}
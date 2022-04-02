/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "renderview.h"
/* audio*/
#include <fileoutdriver.h>
/* driver */
#include "../../driver/audiodriversettings.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void renderview_on_destroy(RenderView*);
static void renderview_build(RenderView*);
static void renderview_onsettingsviewchanged(RenderView*,
	PropertiesView* sender, psy_Property*, uintptr_t* rebuild);
static void renderview_render(RenderView*);
static void renderview_onstoprendering(RenderView*, psy_AudioDriver* sender);
static void renderview_on_focus(RenderView*);
static void renderview_configure_setdefaultoutputpath(RenderView*);
static void renderview_configure_player_dither(RenderView*);
static void renderview_configure_player_record(RenderView*);
static psy_audio_SequencerPlayMode renderview_recordmode(const RenderView*);
static void renderview_readdriverconfiguration(RenderView*);
static void renderview_writedriverconfiguration(RenderView*);
/* vtable */
static psy_ui_ComponentVtable renderview_vtable;
static bool renderview_vtable_initialized = FALSE;

static void renderview_vtable_init(RenderView* self)
{
	if (!renderview_vtable_initialized) {
		renderview_vtable = *(self->component.vtable);
		renderview_vtable.on_destroy =
			(psy_ui_fp_component_event)
			renderview_on_destroy;
		renderview_vtable.on_focus =
			(psy_ui_fp_component_event)
			renderview_on_focus;
		renderview_vtable_initialized = TRUE;
	}
	self->component.vtable = &renderview_vtable;
}

/* implementation */
void renderview_init(RenderView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{		
	psy_ui_component_init(&self->component, parent, NULL);
	renderview_vtable_init(self);
	psy_ui_component_set_id(&self->component, VIEW_ID_RENDERVIEW);
	self->workspace = workspace;
	self->fileoutdriver = psy_audio_create_fileout_driver();
	renderview_build(self);
	propertiesview_init(&self->view, &self->component,
		tabbarparent, self->properties, 3, workspace);
	propertiesview_prevent_maximize_mainsections(&self->view);	
	psy_signal_connect(&self->view.signal_changed, self,
		renderview_onsettingsviewchanged);
	psy_ui_component_set_align(&self->view.component, psy_ui_ALIGN_CLIENT);
	renderprogressview_init(&self->progressview, &self->component, workspace);
	psy_ui_component_set_align(&self->progressview.component,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_hide(&self->progressview.component);
}

void renderview_on_destroy(RenderView* self)
{
	psy_property_deallocate(self->properties);	
	psy_audiodriver_deallocate(self->fileoutdriver);
}

void renderview_build(RenderView* self)
{	
	psy_Property* record;
	psy_Property* recordchoice;		
	psy_Property* actions;	
	
	self->properties = psy_property_allocinit_key(NULL);
	actions = psy_property_settext(
		psy_property_append_section(self->properties, "actions"),
		"render.render");
	psy_property_settext(
		psy_property_append_action(actions, "savewave"),
		"render.save-wave");	
	record = psy_property_settext(
		psy_property_append_section(self->properties, "record"),
		"render.record");	
	recordchoice = psy_property_settext(
		psy_property_append_choice(record, "selection", 0),
		"render.selection");
	psy_property_settext(
		psy_property_append_str(recordchoice, "record-entiresong", ""),
		"render.entire-song");
	psy_property_settext(
		psy_property_append_str(recordchoice, "record-songsel", ""),
		"render.songsel");
	self->driverconfigure = psy_property_settext(
		psy_property_append_section(self->properties, "configure"),
		"render.configure");
	renderview_readdriverconfiguration(self);
	renderview_configure_setdefaultoutputpath(self);
	renderview_writedriverconfiguration(self);
}

void renderview_readdriverconfiguration(RenderView* self)
{	
	psy_property_clear(self->driverconfigure);
	if (psy_audiodriver_configuration(self->fileoutdriver)) {
		psy_property_append_property(self->driverconfigure,
			psy_property_clone(psy_audiodriver_configuration(
				self->fileoutdriver)));
	}	
}

void renderview_writedriverconfiguration(RenderView* self)
{
	assert(self->fileoutdriver);

	if (psy_audiodriver_configuration(self->fileoutdriver)) {
		psy_Property* driversection = NULL;

		driversection = psy_property_find(self->driverconfigure,
			psy_property_key(psy_audiodriver_configuration(
				self->fileoutdriver)),
			PSY_PROPERTY_TYPE_NONE);
		if (driversection) {
			psy_audiodriver_configure(self->fileoutdriver, driversection);
		}
	}
}

void renderview_onsettingsviewchanged(RenderView* self, PropertiesView* sender,
	psy_Property* property, uintptr_t* rebuild)
{
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_ACTION) {
		if (strcmp(psy_property_key(property), "savewave") == 0) {
			renderview_render(self);
		}
	} else if (psy_property_insection(property, self->driverconfigure)) {
		renderview_writedriverconfiguration(self);
	}
}

void renderview_render(RenderView* self)
{	
	if (!workspace_song(self->workspace)) {
		return;
	}
	self->curraudiodriver = psy_audio_player_audiodriver(
		workspace_player(self->workspace));
	psy_audiodriver_close(self->curraudiodriver);
	psy_audio_player_setaudiodriver(
		workspace_player(self->workspace), self->fileoutdriver);	
	renderview_configure_player_dither(self);
	renderview_configure_player_record(self);	
	psy_audio_player_start(workspace_player(self->workspace));
	psy_signal_connect(&self->fileoutdriver->signal_stop, self,
		renderview_onstoprendering);
	psy_audiodriver_open(self->fileoutdriver);
	psy_ui_component_hide(&self->view.component);
	psy_ui_component_show_align(&self->progressview.component);
}

void renderview_configure_setdefaultoutputpath(RenderView* self)
{		
	const char* current_wave_rec_dir;
	char outputpath[4096];

	current_wave_rec_dir = dirconfig_songs(
		&self->workspace->config.directories);
	psy_snprintf(outputpath, 4096, "%s%s%s", current_wave_rec_dir,
		psy_SLASHSTR, "Untitled.wav");
	psy_property_set_str(self->driverconfigure, "fileout.outputpath",
		outputpath);
}

void renderview_configure_player_dither(RenderView* self)
{
	self->restoredither = psy_dsp_dither_settings(
		&workspace_player(self->workspace)->dither);
	self->restoredodither = workspace_player(self->workspace)->dodither;
	if (psy_property_at_bool(self->driverconfigure,
			"fileout.dither.enable", FALSE) != FALSE) {
		psy_Property* property;
		intptr_t bitdepth;
		psy_Property* pdf;
		psy_Property* noiseshaping;
		psy_dsp_DitherPdf dither_pdf = psy_dsp_DITHER_PDF_TRIANGULAR;
		psy_dsp_DitherNoiseShape dither_noiseshape =
			psy_dsp_DITHER_NOISESHAPE_NONE;

		pdf = psy_property_at(self->driverconfigure, "fileout.dither.pdf",
			PSY_PROPERTY_TYPE_NONE);
		if (pdf) {
			dither_pdf = (psy_dsp_DitherPdf)psy_property_item_int(pdf);
		}
		noiseshaping = psy_property_at(self->driverconfigure,
			"fileout.dither.noiseshape", PSY_PROPERTY_TYPE_NONE);
		if (noiseshaping) {
			dither_noiseshape = (psy_dsp_DitherNoiseShape)
				psy_property_item_int(noiseshaping);
		}
		bitdepth = 16;
		property = psy_property_at(self->driverconfigure, "fileout.bitdepth",
			PSY_PROPERTY_TYPE_CHOICE);
		if (property) {
			property = psy_property_at_choice(property);
			if (property) {				
				bitdepth = psy_property_item_int(property);				
			}
		}
		psy_audio_player_setdither(workspace_player(self->workspace),
			bitdepth, dither_pdf, dither_noiseshape);
		psy_audio_player_enabledither(workspace_player(self->workspace));
	}
}

void renderview_configure_player_record(RenderView* self)
{
	self->restoreloopmode = workspace_player(
		self->workspace)->sequencer.looping;
	workspace_player(self->workspace)->sequencer.looping = 0;
	psy_audio_player_setposition(workspace_player(self->workspace), 0);
	psy_audio_sequence_setplayselection(
		&workspace_song(self->workspace)->sequence,
		&self->workspace->song->sequence.sequenceselection);
	psy_audio_sequencer_setplaymode(
		&workspace_player(self->workspace)->sequencer,
		renderview_recordmode(self));
}

void renderview_onstoprendering(RenderView* self, psy_AudioDriver* sender)
{
	psy_audio_player_stop(workspace_player(self->workspace));
	psy_audio_player_setaudiodriver(workspace_player(self->workspace),
		self->curraudiodriver);
	workspace_player(self->workspace)->sequencer.looping =
		self->restoreloopmode;
	if (!self->restoredodither) {
		psy_audio_player_disabledither(workspace_player(self->workspace));
	}
	psy_dsp_dither_setsettings(&workspace_player(self->workspace)->dither,
		self->restoredither);	
	psy_audiodriver_open(self->curraudiodriver);
	psy_audiodriver_close(self->fileoutdriver);
	psy_ui_component_hide(&self->progressview.component);
	psy_ui_component_show_align(&self->view.component);
}

void renderview_on_focus(RenderView* self)
{
	psy_ui_component_set_focus(&self->view.component);
}

psy_audio_SequencerPlayMode renderview_recordmode(const RenderView* self)
{
	psy_Property* recordmode;

	recordmode = psy_property_at(self->properties, "record.selection",
		PSY_PROPERTY_TYPE_CHOICE);
	if (recordmode && psy_property_item_int(recordmode) == 1) {
		return psy_audio_SEQUENCERPLAYMODE_PLAYSEL;
	};
	return psy_audio_SEQUENCERPLAYMODE_PLAYALL;
}

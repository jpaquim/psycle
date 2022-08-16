/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
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
static void renderview_on_destroyed(RenderView*);
static void renderview_make(RenderView*);
static void renderview_make_driver_configuration(RenderView*);
static void renderview_on_render(RenderView*, psy_Property* sender);
static void renderview_on_stop_rendering(RenderView*, psy_AudioDriver* sender);
static void renderview_on_focus(RenderView*);
static void renderview_configure_set_default_output_path(RenderView*);
static void renderview_configure_player_dither(RenderView*);
static void renderview_configure_player_record(RenderView*);
static psy_audio_SequencerPlayMode renderview_record_mode(const RenderView*);

/* vtable */
static psy_ui_ComponentVtable renderview_vtable;
static bool renderview_vtable_initialized = FALSE;

static void renderview_vtable_init(RenderView* self)
{
	if (!renderview_vtable_initialized) {
		renderview_vtable = *(self->component.vtable);
		renderview_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			renderview_on_destroyed;
		renderview_vtable.on_focus =
			(psy_ui_fp_component_event)
			renderview_on_focus;
		renderview_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(renderview_base(self), &renderview_vtable);
}

/* implementation */
void renderview_init(RenderView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	assert(self);
	assert(workspace);

	psy_ui_component_init(&self->component, parent, NULL);
	renderview_vtable_init(self);
	psy_ui_component_set_id(&self->component, VIEW_ID_RENDERVIEW);
	self->workspace = workspace;
	self->fileout_driver = psy_audio_create_fileout_driver();
	self->restore_dither = psy_audio_player_dither_configuration(
		workspace_player(self->workspace));
	renderview_make(self);
	propertiesview_init(&self->view, &self->component,
		tabbarparent, self->properties, 3, FALSE,
		&workspace->inputhandler);
	psy_ui_component_set_id(&self->view.viewtabbar, VIEW_ID_RENDERVIEW);
	propertiesview_prevent_maximize_main_sections(&self->view);	
	psy_ui_component_set_align(&self->view.component, psy_ui_ALIGN_CLIENT);
	renderprogressview_init(&self->progress_view, &self->component, workspace);
	psy_ui_component_set_align(&self->progress_view.component,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_hide(&self->progress_view.component);
}

void renderview_on_destroyed(RenderView* self)
{
	psy_property_deallocate(self->properties);
	psy_audiodriver_deallocate(self->fileout_driver);
}

void renderview_make(RenderView* self)
{	
	psy_Property* record;
	psy_Property* recordchoice;		
	psy_Property* actions;	
	
	self->properties = psy_property_allocinit_key(NULL);
	actions = psy_property_set_text(psy_property_append_section(self->properties, 
		"actions"), "render.render");
	psy_property_connect(psy_property_set_text(
		psy_property_append_action(actions, "savewave"),
		"render.save-wave"), self, renderview_on_render);
	record = psy_property_set_text(psy_property_append_section(self->properties, 
		"record"), "render.record");	
	recordchoice = psy_property_set_text(
		psy_property_append_choice(record, "selection", 0),
		"render.selection");
	psy_property_set_text(
		psy_property_append_str(recordchoice, "record-entiresong", ""),
		"render.entire-song");
	psy_property_set_text(
		psy_property_append_str(recordchoice, "record-songsel", ""),
		"render.songsel");	
	renderview_make_driver_configuration(self);
	renderview_configure_set_default_output_path(self);	
}

void renderview_make_driver_configuration(RenderView* self)
{	
	self->driver_configure = psy_property_set_text(
		psy_property_append_section(self->properties, "configure"),
		"render.configure");
	self->driver_configure->item.disposechildren = FALSE;
	if (psy_audiodriver_configuration(self->fileout_driver)) {		
		psy_property_append_property(self->driver_configure,
			(psy_Property*)psy_audiodriver_configuration(self->fileout_driver));
	}	
}

void renderview_on_render(RenderView* self, psy_Property* sender)
{	
	if (!workspace_song(self->workspace)) {
		return;
	}
	self->curr_audio_driver = psy_audio_player_audiodriver(
		workspace_player(self->workspace));
	psy_audiodriver_close(self->curr_audio_driver);
	psy_audio_player_setaudiodriver(
		workspace_player(self->workspace), self->fileout_driver);	
	renderview_configure_player_dither(self);
	renderview_configure_player_record(self);	
	psy_audio_player_start(workspace_player(self->workspace));
	psy_signal_connect(&self->fileout_driver->signal_stop, self,
		renderview_on_stop_rendering);
	psy_audiodriver_open(self->fileout_driver);
	psy_ui_component_hide(&self->view.component);
	psy_ui_component_show_align(&self->progress_view.component);
}

void renderview_configure_set_default_output_path(RenderView* self)
{		
	const char* current_wave_rec_dir;
	char outputpath[4096];

	current_wave_rec_dir = dirconfig_songs(
		&self->workspace->config.directories);
	psy_snprintf(outputpath, 4096, "%s%s%s", current_wave_rec_dir,
		psy_SLASHSTR, "Untitled.wav");
	psy_property_set_str(self->driver_configure, "fileout.outputpath",
		outputpath);
}

void renderview_configure_player_dither(RenderView* self)
{
	self->restore_dither = psy_audio_player_dither_configuration(
		workspace_player(self->workspace));
	if (psy_property_at_bool(self->driver_configure,
			"fileout.dither.enable", FALSE) != FALSE) {
		psy_Property* property;
		intptr_t bitdepth;
		psy_Property* pdf;
		psy_Property* noiseshaping;
		psy_dsp_DitherPdf dither_pdf = psy_dsp_DITHER_PDF_TRIANGULAR;
		psy_dsp_DitherNoiseShape dither_noiseshape =
			psy_dsp_DITHER_NOISESHAPE_NONE;

		pdf = psy_property_at(self->driver_configure, "fileout.dither.pdf",
			PSY_PROPERTY_TYPE_NONE);
		if (pdf) {
			dither_pdf = (psy_dsp_DitherPdf)psy_property_item_int(pdf);
		}
		noiseshaping = psy_property_at(self->driver_configure,
			"fileout.dither.noiseshape", PSY_PROPERTY_TYPE_NONE);
		if (noiseshaping) {
			dither_noiseshape = (psy_dsp_DitherNoiseShape)
				psy_property_item_int(noiseshaping);
		}
		bitdepth = 16;
		property = psy_property_at(self->driver_configure, "fileout.bitdepth",
			PSY_PROPERTY_TYPE_CHOICE);
		if (property) {
			property = psy_property_at_choice(property);
			if (property) {				
				bitdepth = psy_property_item_int(property);				
			}
		}
		psy_audio_player_configure_dither(workspace_player(self->workspace),
			psy_dsp_dithersettings_make(TRUE, bitdepth, dither_pdf, dither_noiseshape));		
	}
}

void renderview_configure_player_record(RenderView* self)
{
	self->restore_loop_mode = workspace_player(
		self->workspace)->sequencer.looping;
	workspace_player(self->workspace)->sequencer.looping = 0;
	psy_audio_player_setposition(workspace_player(self->workspace), 0);
	psy_audio_sequence_setplayselection(
		&workspace_song(self->workspace)->sequence,
		&self->workspace->song->sequence.selection);
	psy_audio_sequencer_set_play_mode(
		&workspace_player(self->workspace)->sequencer,
		renderview_record_mode(self));
}

void renderview_on_stop_rendering(RenderView* self, psy_AudioDriver* sender)
{
	psy_audio_player_stop(workspace_player(self->workspace));
	psy_audio_player_setaudiodriver(workspace_player(self->workspace),
		self->curr_audio_driver);
	workspace_player(self->workspace)->sequencer.looping =
		self->restore_loop_mode;	
	psy_audio_player_configure_dither(workspace_player(self->workspace),	
		self->restore_dither);	
	psy_audiodriver_open(self->curr_audio_driver);
	psy_audiodriver_close(self->fileout_driver);
	psy_ui_component_hide(&self->progress_view.component);
	psy_ui_component_show_align(&self->view.component);
}

void renderview_on_focus(RenderView* self)
{
	psy_ui_component_set_focus(&self->view.component);
}

psy_audio_SequencerPlayMode renderview_record_mode(const RenderView* self)
{
	intptr_t mode;

	mode = psy_property_at_int(self->properties, "record.selection", 0);
	if (mode == 1) {
		return psy_audio_SEQUENCERPLAYMODE_PLAYSEL;
	}
	return psy_audio_SEQUENCERPLAYMODE_PLAYALL;
}

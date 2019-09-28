#include "songtrackbar.h"

static void OnSize(SongTrackBar*, ui_component* sender, int width, int height);
static void OnDestroy(SongTrackBar*, ui_component* component);
static void Buildtrackbox(SongTrackBar* self);
static void OnTrackBoxSelChange(SongTrackBar*, ui_component* sender, int sel);
static void OnSongTracksNumChanged(SongTrackBar* self, Workspace* workspace,
	unsigned int numsongtracks);
static void OnSongChanged(SongTrackBar*, Workspace*);

void InitSongTrackBar(SongTrackBar* self, ui_component* parent, Workspace* workspace)
{	
	self->workspace = workspace;
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);	
	ui_label_init(&self->headerlabel, &self->component);	
	ui_label_settext(&self->headerlabel, "Tracks");	
	ui_component_setposition(&self->headerlabel.component, 0, 0, 35, 20);	
	ui_combobox_init(&self->trackbox, &self->component);	
	ui_component_setposition(&self->trackbox.component, 40, 0, 60, 20);	
	Buildtrackbox(self);	
	signal_connect(&self->trackbox.signal_selchanged, self, OnTrackBoxSelChange);	
	signal_connect(&workspace->player.signal_numsongtrackschanged, self, OnSongTracksNumChanged);
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
}

void OnDestroy(SongTrackBar* self, ui_component* component)
{
}

void OnSize(SongTrackBar* self, ui_component* sender, int width, int height)
{	
}

void Buildtrackbox(SongTrackBar* self)
{
	int track;
	char text[20];

	for (track = 0; track < 65; ++track) {
		_snprintf(text, 20, "%d", track);		
		ui_combobox_addstring(&self->trackbox, text);
	}	
	ui_combobox_setcursel(&self->trackbox,
		player_numsongtracks(&self->workspace->player));
}

void OnTrackBoxSelChange(SongTrackBar* self, ui_component* sender, int sel)
{		
	player_setnumsongtracks(&self->workspace->player, sel);
}

void OnSongTracksNumChanged(SongTrackBar* self, Workspace* workspace,
	unsigned int numsongtracks)
{
	ui_combobox_setcursel(&self->trackbox, numsongtracks);
}

void OnSongChanged(SongTrackBar* self, Workspace* workspace)
{	
	ui_combobox_setcursel(&self->trackbox,
		player_numsongtracks(&workspace->player));
}

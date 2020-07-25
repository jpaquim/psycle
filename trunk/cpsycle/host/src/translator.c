// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/psyconf.h"

#include "translator.h"
#include "propertiesio.h"

#include "../../detail/portable.h"
#include "../../detail/os.h"

static void translator_initdefault(Translator*);
static void translator_definekeys(psy_Properties* lang);
static void translator_setproperties(Translator*, psy_Properties* lang);

void translator_init(Translator* self)
{
	self->lang = NULL;
	translator_initdefault(self);
}

void translator_initdefault(Translator* self)
{
	psy_Properties* lang;

	lang = psy_properties_create();
	translator_definekeys(lang);
	translator_setproperties(self, lang);
}

void translator_dispose(Translator* self)
{
	psy_properties_free(self->lang);
	self->lang = NULL;
}

bool translator_load(Translator* self, const char* path)
{	
	return propertiesio_load(self->lang, path, FALSE);
}

bool translator_test(Translator* self, const char* path, char* id)
{
	psy_Properties* lang;

	lang = psy_properties_create();
	if (propertiesio_load(lang, path, 1)) {
		psy_Properties* p;

		p = psy_properties_read(lang, "lang");
		if (p) {
			psy_snprintf(id, 256, "%s", psy_properties_valuestring(p));
			psy_properties_free(lang);
			return TRUE;
		}
	}
	psy_properties_free(lang);
	return FALSE;
}

void translator_setproperties(Translator* self, psy_Properties* lang)
{
	psy_properties_free(self->lang);
	self->lang = lang;
}

const char* translator_translate(Translator* self, const char* key)
{	
	if (self->lang) {
		const char* rv;

		rv = psy_properties_readstring(self->lang, key, key);
		if (rv == key && rv != NULL) {
			// removes section from defaulttext
			rv = strrchr(key, '.');
			rv = (rv != NULL)
				? rv + 1
				: key;
		}
		if (rv == NULL) {
			rv = "";
		}
		return rv;
	}
	return key;	
}

void translator_definekeys(psy_Properties* lang)
{
	psy_Properties* instrumentview;
	psy_Properties* machineview;
	psy_Properties* sequencerview;
	psy_Properties* newmachine;
	psy_Properties* samplesview;

	// filebar
	psy_properties_write_string(lang, "lang", "en");
	psy_properties_write_string(lang, "song", "Song");
	psy_properties_write_string(lang, "load", "Load");
	psy_properties_write_string(lang, "save", "Save");
	psy_properties_write_string(lang, "new", "New");
	// undobar
	psy_properties_write_string(lang, "undo", "Undo");
	psy_properties_write_string(lang, "redo", "Redo");
	// playbar
	psy_properties_write_string(lang, "loop", "Loop");
	psy_properties_write_string(lang, "record-notes", "Record Notes");
	psy_properties_write_string(lang, "play", "Play");
	psy_properties_write_string(lang, "stop", "Stop");
	// mainview
	psy_properties_write_string(lang, "help", "Help");
	psy_properties_write_string(lang, "about", "About");
	psy_properties_write_string(lang, "machines", "Machines");
	psy_properties_write_string(lang, "patterns", "Patterns");
	psy_properties_write_string(lang, "pattern", "Pattern");
	psy_properties_write_string(lang, "samples", "Samples");
	psy_properties_write_string(lang, "instrument", "Instrument");
	psy_properties_write_string(lang, "instruments", "Instruments");
	psy_properties_write_string(lang, "settings", "Settings");
	psy_properties_write_string(lang, "render", "Render");
	psy_properties_write_string(lang, "editor", "Editor");
	psy_properties_write_string(lang, "cpu", "CPU");
	psy_properties_write_string(lang, "midi", "MIDI");
	// sequenceview
	psy_properties_write_string(lang, "copy", "Copy");
	psy_properties_write_string(lang, "del", "Del");
	psy_properties_write_string(lang, "delete", "Delete");
	psy_properties_write_string(lang, "paste", "Paste");
	psy_properties_write_string(lang, "clear", "Clear");
	psy_properties_write_string(lang, "duplicate", "Duplicate");
	psy_properties_write_string(lang, "new-trk", "New Trk");
	psy_properties_write_string(lang, "del-trk", "eliminar trk");
	psy_properties_write_string(lang, "clone", "Clone");
	psy_properties_write_string(lang, "greetings", "Greetings");
	psy_properties_write_string(lang, "octave", "Octave");
	psy_properties_write_string(lang, "ok", "OK");
	psy_properties_write_string(lang, "all", "All");
	psy_properties_write_string(lang, "quality", "Quality");
	psy_properties_write_string(lang, "dither", "Dither");
	psy_properties_write_string(lang, "selection", "Selection");
	psy_properties_write_string(lang, "record", "Record");
	psy_properties_write_string(lang, "enable", "enable");
	psy_properties_write_string(lang, "from", "from");
	psy_properties_write_string(lang, "to", "to");
	psy_properties_write_string(lang, "number", "Number");
	psy_properties_write_string(lang, "save wave", "Save Wave");
	psy_properties_write_string(lang, "the entire song", "The Entire Song");
	psy_properties_write_string(lang, "favorites", "Favorites");
	psy_properties_write_string(lang, "contributors-credits", "Contributors / Credits");
	psy_properties_write_string(lang, "machine-view", "Machine View");
	psy_properties_write_string(lang, "pattern-view", "Pattern View");
	psy_properties_write_string(lang, "keyboard and misc", "Keyboard and misc");
	psy_properties_write_string(lang, "compatibility", "Compatibility");
	psy_properties_write_string(lang, "directories", "Directories");
	psy_properties_write_string(lang, "configure", "Configure");
	psy_properties_write_string(lang, "lines-per-beat", "lines per beat");
	psy_properties_write_string(lang, "german", "german");
	psy_properties_write_string(lang, "english", "english");
	psy_properties_write_string(lang, "spanish", "spanish");
	psy_properties_write_string(lang, "language", "Language");
	psy_properties_write_string(lang, "properties", "Properties");
	psy_properties_write_string(lang, "file", "File");
	psy_properties_write_string(lang, "parameters", "Parameters");
	psy_properties_write_string(lang, "exchange", "Exchange");
	psy_properties_write_string(lang, "show master", "Show Master");
	psy_properties_write_string(lang, "create-replace", "Create/Replace");
	psy_properties_write_string(lang, "effects", "Effects");
	psy_properties_write_string(lang, "generators", "Generators");
	psy_properties_write_string(lang, "waves", "Waves");
	psy_properties_write_string(lang, "event-input", "Event Input");
	psy_properties_write_string(lang, "audio-drivers", "Audio Drivers");
	psy_properties_write_string(lang, "jme-version-unknown",
		"Load new gamefx and blitz if version is unknown");
	// directories
	psy_properties_write_string(lang, "song-directory",
		"Song directory");
	psy_properties_write_string(lang, "samples-directory",
		"Samples directory");
	psy_properties_write_string(lang, "plug-in-directory",
		"Plug-in directory");
	psy_properties_write_string(lang, "lua-scripts-directory",
		"Lua-scripts directory");
	psy_properties_write_string(lang, "vst-directories",
		"Vst directories");
	psy_properties_write_string(lang, "vst64-directories",
		"Vst64 directories");
	psy_properties_write_string(lang, "ladspa-directories",
		"Ladspa directories");
	psy_properties_write_string(lang, "skin-directory",
		"Skin directory");
	// settings
	psy_properties_write_string(lang, "general", "General");
	psy_properties_write_string(lang, "visual", "Visual");
	psy_properties_write_string(lang, "background", "Background");
	psy_properties_write_string(lang, "font", "Font");
	psy_properties_write_string(lang, "name", "Name");
	psy_properties_write_string(lang, "show-about-at-startup", "Show About At Startup");
	psy_properties_write_string(lang, "show-song-info-on-load", "Show Song Info On Load");
	psy_properties_write_string(lang, "show-maximized-at-startup", "Show maximized at startup");
	psy_properties_write_string(lang, "show-playlist-editor", "Show Playlist-editor");
	psy_properties_write_string(lang, "show-sequencestepbar", "Show Sequencestepbar");
	psy_properties_write_string(lang, "save-recent-songs", "Save recent songs");
	psy_properties_write_string(lang, "play-song-after-load", "Play Song After Load");
	psy_properties_write_string(lang, "move-cursor-when-paste", "Move Cursor When Paste");
	psy_properties_write_string(lang, "theme", "Theme");
	psy_properties_write_string(lang, "show-as-window", "Show as Window");
	psy_properties_write_string(lang, "gear", "Gear Rack");
	psy_properties_write_string(lang, "tracks", "Tracks");
	psy_properties_write_string(lang, "process", "Process");
	psy_properties_write_string(lang, "no-wave-loaded", "No wave loaded");
	psy_properties_write_string(lang, "auowire", "Autowire");
	psy_properties_write_string(lang, "unselect-all", "Unselect all");
	psy_properties_write_string(lang, "remove-connection-with-right-click",
		"Remove connection with right click");
	psy_properties_write_string(lang, "sequence-positions",
		"Sequence positions");
	psy_properties_write_string(lang, "samplerate",
		"Samplerate");
	psy_properties_write_string(lang, "gaussian",
		"gaussian");
	psy_properties_write_string(lang, "bit-depth",
		"Bit depth");
	psy_properties_write_string(lang, "none",
		"none");
	psy_properties_write_string(lang, "noise-shaping",
		"noise-shaping");
	psy_properties_write_string(lang, "output path",
		"Output path");
	psy_properties_write_string(lang, "save-each-unmuted",
		"Save each unmuted");
	psy_properties_write_string(lang, "kbd",
		"Kbd");
	psy_properties_write_string(lang, "terminal",
		"Terminal");
	psy_properties_write_string(lang, "load-skin",
		"Load skin");
	psy_properties_write_string(lang, "default-skin",
		"Default skin");
	psy_properties_write_string(lang, "default-font",
		"Default Font");
	psy_properties_write_string(lang, "default-line",
		"Default Line");
	psy_properties_write_string(lang, "load-dial-bitmap",
		"Load dial bitmap");
	psy_properties_write_string(lang, "choose-font",
		"Choose Font");
	// instrumentview
	instrumentview = psy_properties_create_section(lang, "instrumentview");
	psy_properties_write_string(instrumentview, "instrument-name",
		"Instrument Name");
	psy_properties_write_string(instrumentview, "new-note-action",
		"New Note Action");
	psy_properties_write_string(instrumentview, "note-cut",
		"Note Cut");
	psy_properties_write_string(instrumentview, "note-release",
		"Note Release");
	psy_properties_write_string(instrumentview, "note-fadeout",
		"Note Fadeout");
	psy_properties_write_string(instrumentview, "none", "None");
	psy_properties_write_string(instrumentview, "play-sample-to-fit",
		"Play sample to fit");
	psy_properties_write_string(instrumentview, "pattern-rows",
		"Pattern rows");
	psy_properties_write_string(instrumentview, "global-volume",
		"Global volume");
	psy_properties_write_string(instrumentview,
		"amplitude-envelope", "Amplitude envelope");
	psy_properties_write_string(instrumentview,
		"attack", "Attack");
	psy_properties_write_string(instrumentview,
		"decay", "Decay");
	psy_properties_write_string(instrumentview,
		"sustain-level", "Sustain level");
	psy_properties_write_string(instrumentview,
		"release", "Release");
	psy_properties_write_string(instrumentview,
		"filter-type", "Filter type");
	psy_properties_write_string(instrumentview,
		"filter-envelope", "Filter envelope");
	psy_properties_write_string(instrumentview,
		"cut-off", "Cut-off");
	psy_properties_write_string(instrumentview,
		"res", "Res/bandw.");
	psy_properties_write_string(instrumentview,
		"mod", "Mod. Amount");
	psy_properties_write_string(instrumentview,
		"random-panning", "Random panning");
	psy_properties_write_string(instrumentview, "general", "General");
	psy_properties_write_string(instrumentview, "volume", "Volume");
	psy_properties_write_string(instrumentview, "pan", "Pan");
	psy_properties_write_string(instrumentview, "filter", "Filter");
	psy_properties_write_string(instrumentview, "pitch", "Pitch");
	machineview = psy_properties_create_section(lang, "machineview");
	psy_properties_write_string(machineview, "new-machine", "New Machine");
	psy_properties_write_string(machineview, "wires", "Wires");
	psy_properties_write_string(machineview, "no-machines-loaded", "No Machines Loaded");
	psy_properties_write_string(machineview, "no-machine", "No Machine");
	psy_properties_write_string(machineview, "connect-to-mixer-send-return-input",
		"Connect to Mixer-send/return-input");
	sequencerview = psy_properties_create_section(lang, "sequencerview");
	psy_properties_write_string(sequencerview, "follow-song", "Follow Song");
	psy_properties_write_string(sequencerview, "show-playlist", "Show Playlist");
	psy_properties_write_string(sequencerview, "show-pattern-names", "Show Pattern Names");
	psy_properties_write_string(sequencerview, "record-tweaks", "Record tweaks");
	psy_properties_write_string(sequencerview, "duration", "Duration");
	psy_properties_write_string(sequencerview, "ins", "Insert");	
	psy_properties_write_string(sequencerview, "new", "New");
	psy_properties_write_string(sequencerview, "clone", "Clone");
	psy_properties_write_string(sequencerview, "del", "Delete");
	psy_properties_write_string(sequencerview, "new-trk", "New Track");
	psy_properties_write_string(sequencerview, "del-trk", "Del Track");
	psy_properties_write_string(sequencerview, "clear", "Clear");
	psy_properties_write_string(sequencerview, "copy", "Copy");
	psy_properties_write_string(sequencerview, "paste", "Paste");
	psy_properties_write_string(sequencerview, "singlesel", "SingleSel");
	psy_properties_write_string(sequencerview, "multisel", "MultiSel");
	newmachine = psy_properties_create_section(lang, "newmachine");
	psy_properties_write_string(newmachine,
		"jme-version-unknown",
		"Load new gamefx and blitz if version is unknown");
	psy_properties_write_string(newmachine,
		"song-loading-compatibility",
		"Song loading compatibility");
	psy_properties_write_string(newmachine, "rescan", "Rescan");
	psy_properties_write_string(newmachine, "select-plugin-directories", "Select plugin directories");
	psy_properties_write_string(newmachine, "sort-by-favorite", "sort by favorite");
	psy_properties_write_string(newmachine, "sort-by-name", "Sort by name");
	psy_properties_write_string(newmachine, "sort-by-type", "Sort by type");
	psy_properties_write_string(newmachine, "sort-by-mode", "Sort by mode");
	psy_properties_write_string(newmachine, "select-plugin-to-view-description",
		"Select a plugin to view its description");
	samplesview = psy_properties_create_section(lang, "samplesview");
	psy_properties_write_string(samplesview, "samplename", "Samplename");
	psy_properties_write_string(samplesview, "samplerate", "Samplerate");
	psy_properties_write_string(samplesview, "samples", "Samples");
	psy_properties_write_string(samplesview, "groupsamples", "Group Samples");
	psy_properties_write_string(samplesview, "groupsfirstsample", "Groups first sample");
	psy_properties_write_string(samplesview, "default-volume", "Default volume");
	psy_properties_write_string(samplesview, "global-volume", "Global volume");
	psy_properties_write_string(samplesview, "pan-position", "Pan Position");
	psy_properties_write_string(samplesview, "sampled-note", "Sampled note");
	psy_properties_write_string(samplesview, "pitch-finetune", "Pitch finetune");
}
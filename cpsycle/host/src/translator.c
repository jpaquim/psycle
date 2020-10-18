// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/psyconf.h"

#include "translator.h"
// file
#include <propertiesio.h>
// platform
#include "../../detail/portable.h"

static void translator_definekeys(psy_Properties* lang);
static bool translator_hastranslation(Translator*, const char* key,
	const char* translation);
static const char* translator_remove_section(Translator*, const char* key);

void translator_init(Translator* self)
{
	psy_properties_init(&self->dictionary);
	translator_definekeys(&self->dictionary);
}

void translator_dispose(Translator* self)
{
	psy_properties_dispose(&self->dictionary);
}

void translator_reset(Translator* self)
{
	psy_properties_clear(&self->dictionary);
	translator_definekeys(&self->dictionary);
}

bool translator_load(Translator* self, const char* path)
{	
	translator_reset(self);
	return propertiesio_load(&self->dictionary, path, FALSE);
}

bool translator_test(Translator* self, const char* path, char* id)
{
	psy_Properties* lang;

	lang = psy_properties_create();
	if (propertiesio_load(lang, path, 1)) {
		psy_Properties* p;

		p = psy_properties_at(lang, "lang", PSY_PROPERTY_TYP_NONE);
		if (p) {
			psy_snprintf(id, 256, "%s", psy_properties_as_str(p));
			psy_properties_free(lang);
			return TRUE;
		}
	}
	psy_properties_free(lang);
	return FALSE;
}

const char* translator_translate(Translator* self, const char* key)
{	
	const char* rv;

	rv = psy_properties_at_str(&self->dictionary, key, key);
	if (!translator_hastranslation(self, rv, key)) {
		return translator_remove_section(self, rv);
	}
	return rv;
}

bool translator_hastranslation(Translator* self, const char* key,
	const char* translation)
{
	return translation != key;
}

const char* translator_remove_section(Translator* self, const char* key)
{
	const char* rv;

	if (key) {
		rv = strrchr(key, '.');
		rv = (rv != NULL)
			? rv + 1
			: key;
	} else {
		rv = NULL;
	}
	if (rv == NULL) {
		rv = "";
	}
	return rv;
}

void translator_definekeys(psy_Properties* lang)
{
	psy_Properties* section;	
		
	psy_properties_set_str(lang, "lang", "en");
	// filebar
	section = psy_properties_append_section(lang, "file");
	psy_properties_set_str(section, "song", "Song");
	psy_properties_set_str(section, "load", "Load");
	psy_properties_set_str(section, "save", "Save");
	psy_properties_set_str(section, "new", "New");
	psy_properties_set_str(section, "render", "Render");
	// undobar
	section = psy_properties_append_section(lang, "undo");
	psy_properties_set_str(section, "undo", "Undo");
	psy_properties_set_str(section, "redo", "Redo");
	// playbar
	section = psy_properties_append_section(lang, "play");
	psy_properties_set_str(section, "loop", "Loop");
	psy_properties_set_str(section, "record-notes", "Record Notes");
	psy_properties_set_str(section, "play", "Play");
	psy_properties_set_str(section, "stop", "Stop");
	// mainview
	section = psy_properties_append_section(lang, "main");
	psy_properties_set_str(section, "machines", "Machines");
	psy_properties_set_str(section, "patterns", "Patterns");
	psy_properties_set_str(section, "pattern", "Pattern");
	psy_properties_set_str(section, "samples", "Samples");
	psy_properties_set_str(section, "instrument", "Instrument");
	psy_properties_set_str(section, "instruments", "Instruments");
	psy_properties_set_str(section, "settings", "Settings");
	psy_properties_set_str(section, "properties", "Properties");
	psy_properties_set_str(section, "kbd", "Kbd");
	psy_properties_set_str(section, "terminal", "Terminal");	
	// Help
	section = psy_properties_append_section(lang, "help");
	psy_properties_set_str(section, "help", "Help");
	psy_properties_set_str(section, "about", "About");
	psy_properties_set_str(section, "greetings", "Greetings");
	psy_properties_set_str(section, "contributors-credits", "Contributors / Credits");
	psy_properties_set_str(section, "ok", "OK");
	// Machinebar
	section = psy_properties_append_section(lang, "machinebar");
	psy_properties_set_str(section, "gear", "Gear Rack");
	psy_properties_set_str(section, "editor", "Editor");
	psy_properties_set_str(section, "cpu", "CPU");
	psy_properties_set_str(section, "midi", "MIDI");
	// Trackbar
	section = psy_properties_append_section(lang, "trackbar");
	psy_properties_set_str(section,
		"tracks", "Tracks");
	// Octave
	section = psy_properties_append_section(lang, "octavebar");
	psy_properties_set_str(section,
		"octave", "Octave");
	// edit operations
	section = psy_properties_append_section(lang, "edit");
	psy_properties_set_str(section, "cut", "Cut");
	psy_properties_set_str(section, "copy", "Copy");
	psy_properties_set_str(section, "crop", "Crop");
	psy_properties_set_str(section, "delete", "Delete");
	psy_properties_set_str(section, "paste", "Paste");
	psy_properties_set_str(section, "mixpaste", "MixPaste");
	psy_properties_set_str(section, "clear", "Clear");
	psy_properties_set_str(section, "duplicate", "Duplicate");
	psy_properties_set_str(section, "clone", "Clone");
	// Lines per beat bar
	section = psy_properties_append_section(lang, "lpb");
	psy_properties_set_str(section, "lines-per-beat", "lines per beat");	
	section = psy_properties_append_section(section, "channelmapping");
	psy_properties_set_str(section, "autowire", "Autowire");
	psy_properties_set_str(section, "unselect-all", "Unselect all");
	psy_properties_set_str(section, "remove-connection-with-right-click",
		"Remove connection with right click");	
	// Render
	section = psy_properties_append_section(lang, "render");
	psy_properties_set_str(section,
		"render", "Render");
	psy_properties_set_str(section,
		"file", "File");
	psy_properties_set_str(section,
		"sequence-positions", "Sequence positions");
	psy_properties_set_str(section,
		"quality", "Quality");
	psy_properties_set_str(section,
		"dither", "Dither");
	psy_properties_set_str(section,
		"selection", "Selection");
	psy_properties_set_str(section,
		"record", "Record");
	psy_properties_set_str(section,
		"enable", "enable");
	psy_properties_set_str(section,
		"from", "from");
	psy_properties_set_str(section,
		"to", "to");
	psy_properties_set_str(section,
		"number", "Number");
	psy_properties_set_str(section,
		"save-wave", "Save Wave");
	psy_properties_set_str(section,
		"entire-song", "The Entire Song");
	psy_properties_set_str(section,
		"samplerate", "Samplerate");	
	psy_properties_set_str(section,
		"bitdepth", "Bit depth");
	psy_properties_set_str(section,
		"none", "none");
	psy_properties_set_str(section,
		"pdf",
		"Prob. Distribution");
	psy_properties_set_str(section,
		"triangular", "Triangular");
	psy_properties_set_str(section,
		"rectangular", "Rectangular");
	psy_properties_set_str(section,
		"gaussian", "gaussian");
	psy_properties_set_str(section,
		"noise-shaping", "noise-shaping");
	psy_properties_set_str(section,
		"high-pass-contour",
		"High - Pass Contour");
	psy_properties_set_str(section,
		"none",
		"none");
	psy_properties_set_str(section,
		"output-path", "Output path");
	psy_properties_set_str(section,
		"save-each-unmuted", "Save each unmuted");	
	// gear
	section = psy_properties_append_section(lang, "gear");
	psy_properties_set_str(section, "clone", "Clone");
	psy_properties_set_str(section, "delete", "Delete");
	psy_properties_set_str(section, "properties", "Properties");
	psy_properties_set_str(section, "parameters", "Parameters");
	psy_properties_set_str(section, "exchange", "Exchange");
	psy_properties_set_str(section, "show-master", "Show Master");
	psy_properties_set_str(section, "create-replace", "Create/Replace");
	psy_properties_set_str(section, "effects", "Effects");
	psy_properties_set_str(section, "generators", "Generators");
	psy_properties_set_str(section, "waves", "Waves");
	// settingsview
	section = psy_properties_append_section(lang, "settingsview");
	psy_properties_set_str(section, "general", "General");
	psy_properties_set_str(section, "visual", "Visual");
	psy_properties_set_str(section, "machineview", "Machine View");
	psy_properties_set_str(section, "patternview", "Pattern View");
	psy_properties_set_str(section, "keyboard-and-misc", "Keyboard and misc");
	psy_properties_set_str(section, "compatibility", "Compatibility");
	psy_properties_set_str(section, "language", "Language");
	psy_properties_set_str(section, "event-input", "Event Input");
	psy_properties_set_str(section, "audio-drivers", "Audio Drivers");
	psy_properties_set_str(section, "configure", "Configure");
	psy_properties_set_str(section, "jme-version-unknown",
		"Load new gamefx and blitz if version is unknown");	
	// settingsview/general
	psy_properties_set_str(section, "language", "Language");
	psy_properties_set_str(section, "de", "german");
	psy_properties_set_str(section, "en", "english");
	psy_properties_set_str(section, "es", "spanish");
	// settingsview/directories
	psy_properties_set_str(section,
		"directories", "Directories");
	psy_properties_set_str(section, "song-directory",
		"Song directory");
	psy_properties_set_str(section, "samples-directory",
		"Samples directory");
	psy_properties_set_str(section, "plug-in-directory",
		"Plug-in directory");
	psy_properties_set_str(section, "lua-scripts-directory",
		"Lua-scripts directory");
	psy_properties_set_str(section, "vst-directories",
		"Vst directories");
	psy_properties_set_str(section, "vst64-directories",
		"Vst64 directories");
	psy_properties_set_str(section, "ladspa-directories",
		"Ladspa directories");
	psy_properties_set_str(section, "skin-directory",
		"Skin directory");
	psy_properties_set_str(section,
		"theme", "Theme");	
	psy_properties_set_str(section,
		"background", "Background");
	psy_properties_set_str(section,
		"font", "Font");
	psy_properties_set_str(section,
		"name", "Name");
	psy_properties_set_str(section,
		"show-about-at-startup", "Show About At Startup");
	psy_properties_set_str(section,
		"show-song-info-on-load", "Show Song Info On Load");
	psy_properties_set_str(section,
		"show-maximized-at-startup", "Show maximized at startup");
	psy_properties_set_str(section,
		"show-playlist-editor", "Show Playlist-editor");
	psy_properties_set_str(section,
		"show-sequencestepbar", "Show Sequencestepbar");
	psy_properties_set_str(section,
		"save-recent-songs", "Save recent songs");
	psy_properties_set_str(section,
		"play-song-after-load", "Play Song After Load");
	psy_properties_set_str(section,
		"move-cursor-when-paste", "Move Cursor When Paste");
	psy_properties_set_str(section,
		"show-as-window", "Show as Window");	
	psy_properties_set_str(section,
		"load-skin", "Load skin");
	psy_properties_set_str(section,
		"default-skin", "Default skin");
	psy_properties_set_str(section,
		"default-font", "Default Font");
	psy_properties_set_str(section,
		"default-line", "Default Line");
	psy_properties_set_str(section,
		"load-dial-bitmap", "Load dial bitmap");
	psy_properties_set_str(section,
		"choose-font", "Choose Font");
	// settingsview/patternview
	psy_properties_set_str(section,
		"draw-empty-data",
		"Draw empty data");
	psy_properties_set_str(section, 
		"default-entries",
		"Default entries");
	psy_properties_set_str(section,
		"line-numbers",
		"Line numbers");
	psy_properties_set_str(section,
		"beat-offset",
		"Beat offset");
	psy_properties_set_str(section,
		"line-numbers-cursor",
		"Line numbers cursor");
	psy_properties_set_str(section,
		"line-numbers-in-hex",
		"Line numbers in HEX");
	psy_properties_set_str(section,
		"wide-instrument-column",
		"Wide instrument column");
	psy_properties_set_str(section,
		"pattern-track-scopes",
		"Pattern track scopes");
	psy_properties_set_str(section,
		"wrap-around",
		"Wrap Around");
	psy_properties_set_str(section,
		"center-cursor-on-screen",
		"Center cursor on screen");
	psy_properties_set_str(section,
		"bar-highlighting",
		"Bar highlighting: (beats/bar)");
	psy_properties_set_str(section,
		"a4-440hz",
		"A4 is 440Hz (Otherwise it is 220Hz)");
	// settingsview/machineview
	psy_properties_set_str(section,
		"draw-machine-indexes",
		"Draw Machine Indexes");
	psy_properties_set_str(section,
		"draw-vu-meters",
		"Draw VU Meters");
	psy_properties_set_str(section,
		"draw-wire-hover",
		"Draw VU Meters");
	// settingsview/machineview/theme
	psy_properties_set_str(section,
		"vu-background", "Vu BackGround");
	psy_properties_set_str(section,
		"vu-bar", "Vu Bar");
	psy_properties_set_str(section,
		"onclip", "OnClip");
	psy_properties_set_str(section,
		"generators-font-face", "Generators Font Face");
	psy_properties_set_str(section,
		"generators-font-point", "Generators Font Point");
	psy_properties_set_str(section,
		"effect-fontface", "effect_fontface");
	psy_properties_set_str(section,
		"effect-font-flags", "effect_font_flags");
	psy_properties_set_str(section,
		"background", "Background");
	psy_properties_set_str(section,
		"wirecolour", "Wires");
	psy_properties_set_str(section,
		"wirecolour2", "wirecolour2");
	psy_properties_set_str(section, 
		"polygons", "Polygons");
	psy_properties_set_str(section,
		"generators-font", "Generators Font");
	psy_properties_set_str(section,
		"effects-font", "Effects Font");
	psy_properties_set_str(section,
		"wire-width", "Wire width");
	psy_properties_set_str(section,
		"antialias-halo", "AntiAlias halo");
	psy_properties_set_str(section,
		"machine-background", "Machine Background");
	psy_properties_set_str(section,
		"polygon-size", "Polygon size");
	// settingsview/paramview
	psy_properties_set_str(section,
		"native-machine-parameter-window",
		"Native Machine Parameter Window");
	psy_properties_set_str(section,
		"load-dial-bitmap",
		"Load Dial Bitmap");
	// settingsview/paramview-theme
	psy_properties_set_str(section,
		"title-background", "Title Background");
	psy_properties_set_str(section,
		"title-font", "Title Font");
	psy_properties_set_str(section,
		"param-background", "Param Background");
	psy_properties_set_str(section,
		"param-font", "Param Font");
	psy_properties_set_str(section,
		"value-background", "Value Background");
	psy_properties_set_str(section,
		"value-font", "Value Font");
	psy_properties_set_str(section,
		"selparam-background", "SelParam Background");
	psy_properties_set_str(section,
		"selparam-font", "SelParam Font");
	psy_properties_set_str(section,
		"selvalue-background", "SelValue Background");
	psy_properties_set_str(section,
		"selvalue-font", "SelValue Font");
	// settingsview/keyboard/misc	
	psy_properties_set_str(section,
		"record-tws",
		"Record Mouse Tweaks as tws (Smooth tweaks)");
	psy_properties_set_str(section,
		"advance-line-on-record",
		"Advance Line On Record");
	// instrumentsbox
	section = psy_properties_append_section(lang, "instrumentsbox");
	psy_properties_set_str(section,
		"instrument-groups", "Instrument Groups");
	psy_properties_set_str(section,
		"group-instruments", "Group Instruments");	
	// instrumentview
	section = psy_properties_append_section(lang, "instrumentview");
	psy_properties_set_str(section,
		"instrument-name", "Instrument Name");
	psy_properties_set_str(section,
		"new-note-action", "New Note Action");
	psy_properties_set_str(section,
		"note-cut", "Note Cut");
	psy_properties_set_str(section,
		"note-release", "Note Release");
	psy_properties_set_str(section,
		"note-fadeout", "Note Fadeout");
	psy_properties_set_str(section,
		"none", "None");
	psy_properties_set_str(section,
		"play-sample-to-fit", "Play sample to fit");
	psy_properties_set_str(section,
		"pattern-rows", "Pattern rows");
	psy_properties_set_str(section,
		"global-volume", "Global volume");
	psy_properties_set_str(section,
		"amplitude-envelope", "Amplitude envelope");
	psy_properties_set_str(section,
		"attack", "Attack");
	psy_properties_set_str(section,
		"decay", "Decay");
	psy_properties_set_str(section,
		"sustain-level", "Sustain level");
	psy_properties_set_str(section,
		"release", "Release");
	psy_properties_set_str(section,
		"filter-type", "Filter type");
	psy_properties_set_str(section,
		"filter-envelope", "Filter envelope");
	psy_properties_set_str(section,
		"cut-off", "Cut-off");
	psy_properties_set_str(section,
		"res", "Res/bandw.");
	psy_properties_set_str(section,
		"mod", "Mod. Amount");
	psy_properties_set_str(section,
		"random-panning", "Random panning");
	psy_properties_set_str(section,
		"general", "General");
	psy_properties_set_str(section,
		"volume", "Volume");
	psy_properties_set_str(section,
		"pan", "Pan");
	psy_properties_set_str(section,
		"filter", "Filter");
	psy_properties_set_str(section,
		"pitch", "Pitch");
	// machineview
	section = psy_properties_append_section(lang, "machineview");
	psy_properties_set_str(section,
		"new-machine", "New Machine");
	psy_properties_set_str(section,
		"wires", "Wires");
	psy_properties_set_str(section,
		"no-machines-loaded", "No Machines Loaded");
	psy_properties_set_str(section,
		"no-machine", "No Machine");
	psy_properties_set_str(section,
		"connect-to-mixer-send-return-input",
		"Connect to Mixer-send/return-input");
	// PatternView
	section = psy_properties_append_section(lang, "patternview");
	psy_properties_set_str(section,
		"line", "Line");
	psy_properties_set_str(section,
		"defaults", "Defaults");
	psy_properties_set_str(section,
		"step", "Step");
	// SequenceView
	section = psy_properties_append_section(lang, "sequencerview");
	psy_properties_set_str(section,
		"follow-song", "Follow Song");
	psy_properties_set_str(section,
		"show-playlist", "Show Playlist");
	psy_properties_set_str(section,
		"show-pattern-names", "Show Pattern Names");
	psy_properties_set_str(section,
		"record-tweaks", "Record tweaks");
	psy_properties_set_str(section,
		"multichannel-audition", "Multichannel audition");
	psy_properties_set_str(section,
		"duration", "Duration");
	psy_properties_set_str(section,
		"ins", "Insert");
	psy_properties_set_str(section,
		"new", "New");
	psy_properties_set_str(section,
		"clone", "Clone");
	psy_properties_set_str(section,
		"del", "Delete");
	psy_properties_set_str(section,
		"new-trk", "New Track");
	psy_properties_set_str(section,
		"del-trk", "Del Track");
	psy_properties_set_str(section,
		"clear", "Clear");
	psy_properties_set_str(section,
		"copy", "Copy");
	psy_properties_set_str(section,
		"paste", "Paste");
	psy_properties_set_str(section,
		"singlesel", "SingleSel");
	psy_properties_set_str(section,
		"multisel", "MultiSel");
	section = psy_properties_append_section(lang, "newmachine");
	psy_properties_set_str(section,
		"all", "All");
	psy_properties_set_str(section,
		"favorites", "Favorites");
	psy_properties_set_str(section,
		"jme-version-unknown",
		"Load new gamefx and blitz if version is unknown");
	psy_properties_set_str(section,
		"song-loading-compatibility",
		"Song loading compatibility");
	psy_properties_set_str(section,
		"rescan", "Rescan");
	psy_properties_set_str(section,
		"select-plugin-directories", "Select plugin directories");
	psy_properties_set_str(section,
		"sort-by-favorite", "Sort by favorite");
	psy_properties_set_str(section,
		"sort-by-name", "Sort by name");
	psy_properties_set_str(section,
		"sort-by-type", "Sort by type");
	psy_properties_set_str(section,
		"sort-by-mode", "Sort by mode");
	psy_properties_set_str(section,
		"select-plugin-to-view-description",
		"Select a plugin to view its description");
	section = psy_properties_append_section(lang, "samplesview");
	psy_properties_set_str(section,
		"samplename", "Samplename");
	psy_properties_set_str(section,
		"samplerate", "Samplerate");
	psy_properties_set_str(section,
		"samples", "Samples");
	psy_properties_set_str(section,
		"groupsamples", "Group Samples");
	psy_properties_set_str(section,
		"groupsfirstsample", "Groups first sample");
	psy_properties_set_str(section,
		"default-volume", "Default volume");
	psy_properties_set_str(section,
		"global-volume", "Global volume");
	psy_properties_set_str(section,
		"pan-position", "Pan Position");
	psy_properties_set_str(section,
		"sampled-note", "Sampled note");
	psy_properties_set_str(section,
		"pitch-finetune", "Pitch finetune");
	psy_properties_set_str(section,
		"process", "Process");
	psy_properties_set_str(section,
		"no-wave-loaded", "No wave loaded");
}

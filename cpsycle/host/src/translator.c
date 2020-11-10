// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/psyconf.h"

#include "translator.h"
// file
#include <propertiesio.h>
// platform
#include "../../detail/portable.h"
// std
#include <assert.h>

static void translator_definekeys(psy_Property* lang);
static bool translator_hastranslation(const char* key,
	const char* translation);
static const char* translator_remove_section(Translator*, const char* key);

void translator_init(Translator* self)
{
	assert(self);
	psy_property_init(&self->dictionary);
	translator_definekeys(&self->dictionary);
}

void translator_dispose(Translator* self)
{
	assert(self);
	psy_property_dispose(&self->dictionary);
}

void translator_reset(Translator* self)
{
	assert(self);
	psy_property_clear(&self->dictionary);
	translator_definekeys(&self->dictionary);
}

bool translator_load(Translator* self, const char* path)
{	
	assert(self);
	translator_reset(self);
	return propertiesio_load(&self->dictionary, path, FALSE);
}

bool translator_test(const Translator* self, const char* path, char* id)
{
	psy_Property* lang;

	assert(self);
	lang = psy_property_allocinit_key(NULL);
	if (propertiesio_load(lang, path, 1)) {
		psy_Property* p;

		p = psy_property_at(lang, "lang", PSY_PROPERTY_TYPE_NONE);
		if (p) {
			psy_snprintf(id, 256, "%s", psy_property_as_str(p));
			psy_property_deallocate(lang);
			return TRUE;
		}
	}
	psy_property_deallocate(lang);
	return FALSE;
}

const char* translator_translate(Translator* self, const char* key)
{	
	const char* rv;

	assert(self);
	rv = psy_property_at_str(&self->dictionary, key, key);
	if (!translator_hastranslation(rv, key)) {
		return translator_remove_section(self, rv);
	}
	return rv;
}

bool translator_hastranslation(const char* key, const char* translation)
{
	return key != translation;	
}

const char* translator_remove_section(Translator* self, const char* key)
{
	const char* rv;

	assert(self);
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

void translator_definekeys(psy_Property* lang)
{
	psy_Property* section;	

	assert(lang);
	psy_property_set_str(lang, "lang", "en");
	// filebar
	section = psy_property_append_section(lang, "file");
	psy_property_set_str(section, "song", "Song");
	psy_property_set_str(section, "load", "Load");
	psy_property_set_str(section, "save", "Save");
	psy_property_set_str(section, "new", "New");
	psy_property_set_str(section, "render", "Render");
	// undobar
	section = psy_property_append_section(lang, "undo");
	psy_property_set_str(section, "undo", "Undo");
	psy_property_set_str(section, "redo", "Redo");
	// playbar
	section = psy_property_append_section(lang, "play");
	psy_property_set_str(section, "loop", "Loop");
	psy_property_set_str(section, "record-notes", "Record Notes");
	psy_property_set_str(section, "play", "Play");
	psy_property_set_str(section, "stop", "Stop");
	// mainview
	section = psy_property_append_section(lang, "main");
	psy_property_set_str(section, "machines", "Machines");
	psy_property_set_str(section, "patterns", "Patterns");
	psy_property_set_str(section, "pattern", "Pattern");
	psy_property_set_str(section, "samples", "Samples");
	psy_property_set_str(section, "instrument", "Instrument");
	psy_property_set_str(section, "instruments", "Instruments");
	psy_property_set_str(section, "settings", "Settings");
	psy_property_set_str(section, "properties", "Properties");
	psy_property_set_str(section, "kbd", "Kbd");
	psy_property_set_str(section, "terminal", "Terminal");	
	// Help
	section = psy_property_append_section(lang, "help");
	psy_property_set_str(section, "help", "Help");
	psy_property_set_str(section, "about", "About");
	psy_property_set_str(section, "greetings", "Greetings");
	psy_property_set_str(section, "contributors-credits", "Contributors / Credits");
	psy_property_set_str(section, "ok", "OK");
	// Machinebar
	section = psy_property_append_section(lang, "machinebar");
	psy_property_set_str(section, "gear", "Gear Rack");
	psy_property_set_str(section, "editor", "Editor");
	psy_property_set_str(section, "cpu", "CPU");
	psy_property_set_str(section, "midi", "MIDI");
	// Trackbar
	section = psy_property_append_section(lang, "trackbar");
	psy_property_set_str(section, "tracks", "Tracks");
	// Octave
	section = psy_property_append_section(lang, "octavebar");
	psy_property_set_str(section, "octave", "Octave");
	// edit operations
	section = psy_property_append_section(lang, "edit");
	psy_property_set_str(section, "cut", "Cut");
	psy_property_set_str(section, "copy", "Copy");
	psy_property_set_str(section, "crop", "Crop");
	psy_property_set_str(section, "delete", "Delete");
	psy_property_set_str(section, "paste", "Paste");
	psy_property_set_str(section, "mixpaste", "MixPaste");
	psy_property_set_str(section, "clear", "Clear");
	psy_property_set_str(section, "duplicate", "Duplicate");
	psy_property_set_str(section, "clone", "Clone");
	// Lines per beat bar
	section = psy_property_append_section(lang, "lpb");
	psy_property_set_str(section, "lines-per-beat", "lines per beat");	
	section = psy_property_append_section(section, "channelmapping");
	psy_property_set_str(section, "autowire", "Autowire");
	psy_property_set_str(section, "unselect-all", "Unselect all");
	psy_property_set_str(section, "remove-connection-with-right-click",
		"Remove connection with right click");	
	// Render
	section = psy_property_append_section(lang, "render");
	psy_property_set_str(section, "render", "Render");
	psy_property_set_str(section, "file", "File");
	psy_property_set_str(section,
		"sequence-positions", "Sequence positions");
	psy_property_set_str(section, "quality", "Quality");
	psy_property_set_str(section, "dither", "Dither");
	psy_property_set_str(section, "selection", "Selection");
	psy_property_set_str(section, "record", "Record");
	psy_property_set_str(section, "enable", "enable");
	psy_property_set_str(section, "from", "from");
	psy_property_set_str(section, "to", "to");
	psy_property_set_str(section, "number", "Number");
	psy_property_set_str(section, "save-wave", "Save Wave");
	psy_property_set_str(section, "entire-song", "The Entire Song");
	psy_property_set_str(section, "samplerate", "Samplerate");
	psy_property_set_str(section, "bitdepth", "Bit depth");
	psy_property_set_str(section, "none", "none");
	psy_property_set_str(section, "pdf", "Prob. Distribution");
	psy_property_set_str(section, "triangular", "Triangular");
	psy_property_set_str(section, "rectangular", "Rectangular");
	psy_property_set_str(section, "gaussian", "gaussian");
	psy_property_set_str(section, "noise-shaping", "noise-shaping");
	psy_property_set_str(section, "high-pass-contour", "High - Pass Contour");
	psy_property_set_str(section, "none", "none");
	psy_property_set_str(section, "output-path", "Output path");
	psy_property_set_str(section,
		"save-each-unmuted", "Save each unmuted");	
	// gear
	section = psy_property_append_section(lang, "gear");
	psy_property_set_str(section, "clone", "Clone");
	psy_property_set_str(section, "delete", "Delete");
	psy_property_set_str(section, "properties", "Properties");
	psy_property_set_str(section, "parameters", "Parameters");
	psy_property_set_str(section, "exchange", "Exchange");
	psy_property_set_str(section, "show-master", "Show Master");
	psy_property_set_str(section, "create-replace", "Create/Replace");
	psy_property_set_str(section, "effects", "Effects");
	psy_property_set_str(section, "instruments", "Instruments");
	psy_property_set_str(section, "generators", "Generators");
	psy_property_set_str(section, "waves", "Waves");
	// settingsview
	section = psy_property_append_section(lang, "settingsview");
	psy_property_set_str(section, "general", "General");
	psy_property_set_str(section, "visual", "Visual");
	psy_property_set_str(section, "machineview", "Machine View");
	psy_property_set_str(section, "patternview", "Pattern View");
	psy_property_set_str(section, "keyboard-and-misc", "Keyboard and misc");
	psy_property_set_str(section, "compatibility", "Compatibility");
	psy_property_set_str(section, "language", "Language");
	psy_property_set_str(section, "event-input", "Event Input");
	psy_property_set_str(section, "audio-drivers", "Audio Drivers");
	psy_property_set_str(section, "configure", "Configure");
	psy_property_set_str(section, "jme-version-unknown",
		"Load new gamefx and blitz if version is unknown");	
	// settingsview/general
	psy_property_set_str(section, "language", "Language");
	psy_property_set_str(section, "de", "german");
	psy_property_set_str(section, "en", "english");
	psy_property_set_str(section, "es", "spanish");
	// settingsview/directories
	psy_property_set_str(section,
		"directories", "Directories");
	psy_property_set_str(section, "song-directory",
		"Song directory");
	psy_property_set_str(section, "samples-directory",
		"Samples directory");
	psy_property_set_str(section, "plug-in-directory",
		"Plug-in directory");
	psy_property_set_str(section, "lua-scripts-directory",
		"Lua-scripts directory");
	psy_property_set_str(section, "vst-directories",
		"Vst directories");
	psy_property_set_str(section, "vst64-directories",
		"Vst64 directories");
	psy_property_set_str(section, "ladspa-directories",
		"Ladspa directories");
	psy_property_set_str(section, "skin-directory", "Skin directory");
	psy_property_set_str(section, "theme", "Theme");	
	psy_property_set_str(section, "background", "Background");
	psy_property_set_str(section, "font", "Font");
	psy_property_set_str(section, "name", "Name");
	psy_property_set_str(section, "version", "Version");
	psy_property_set_str(section,
		"show-about-at-startup", "Show About At Startup");
	psy_property_set_str(section,
		"show-song-info-on-load", "Show Song Info On Load");
	psy_property_set_str(section,
		"show-maximized-at-startup", "Show maximized at startup");
	psy_property_set_str(section,
		"show-playlist-editor", "Show Playlist-editor");
	psy_property_set_str(section,
		"show-sequencestepbar", "Show Sequencestepbar");
	psy_property_set_str(section,
		"save-recent-songs", "Save recent songs");
	psy_property_set_str(section,
		"play-song-after-load", "Play Song After Load");
	psy_property_set_str(section,
		"move-cursor-when-paste", "Move Cursor When Paste");
	psy_property_set_str(section,
		"show-as-window", "Show as Window");	
	psy_property_set_str(section, "load-skin", "Load skin");
	psy_property_set_str(section, "default-skin", "Default skin");
	psy_property_set_str(section, "default-font", "Default Font");
	psy_property_set_str(section, "default-line", "Default Line");
	psy_property_set_str(section, "load-dial-bitmap", "Load dial bitmap");
	psy_property_set_str(section, "choose-font", "Choose Font");
	// settingsview/patternview
	psy_property_set_str(section, "draw-empty-data", "Draw empty data");
	psy_property_set_str(section, "default-entries", "Default entries");
	psy_property_set_str(section, "line-numbers", "Line numbers");
	psy_property_set_str(section, "beat-offset", "Beat offset");
	psy_property_set_str(section,
		"line-numbers-cursor", "Line numbers cursor");
	psy_property_set_str(section,
		"line-numbers-in-hex", "Line numbers in HEX");
	psy_property_set_str(section,
		"wide-instrument-column",
		"Wide instrument column");
	psy_property_set_str(section,
		"pattern-track-scopes", "Pattern track scopes");
	psy_property_set_str(section, "wrap-around", "Wrap Around");
	psy_property_set_str(section,
		"center-cursor-on-screen", "Center cursor on screen");
	psy_property_set_str(section,
		"bar-highlighting", "Bar highlighting: (beats/bar)");
	psy_property_set_str(section,
		"a4-440hz", "A4 is 440Hz (Otherwise it is 220Hz)");
	// settingsview/machineview
	psy_property_set_str(section,
		"draw-machine-indexes", "Draw Machine Indexes");
	psy_property_set_str(section, "draw-vu-meters", "Draw VU Meters");
	psy_property_set_str(section, "draw-wire-hover", "Draw VU Meters");
	// settingsview/machineview/theme
	psy_property_set_str(section, "vu-background", "Vu BackGround");
	psy_property_set_str(section, "vu-bar", "Vu Bar");
	psy_property_set_str(section, "onclip", "OnClip");
	psy_property_set_str(section,
		"generators-font-face", "Generators Font Face");
	psy_property_set_str(section,
		"generators-font-point", "Generators Font Point");
	psy_property_set_str(section,
		"effect-fontface", "effect_fontface");
	psy_property_set_str(section,
		"effect-font-flags", "effect_font_flags");
	psy_property_set_str(section, "background", "Background");
	psy_property_set_str(section, "wirecolour", "Wires");
	psy_property_set_str(section, "wirecolour2", "wirecolour2");
	psy_property_set_str(section, "polygons", "Polygons");
	psy_property_set_str(section, "generators-font", "Generators Font");
	psy_property_set_str(section, "effects-font", "Effects Font");
	psy_property_set_str(section, "wire-width", "Wire width");
	psy_property_set_str(section, "antialias-halo", "AntiAlias halo");
	psy_property_set_str(section,
		"machine-background", "Machine Background");
	psy_property_set_str(section, "polygon-size", "Polygon size");
	// settingsview/paramview
	psy_property_set_str(section,
		"native-machine-parameter-window",
		"Native Machine Parameter Window");
	psy_property_set_str(section, "load-dial-bitmap", "Load Dial Bitmap");
	// settingsview/paramview-theme
	psy_property_set_str(section, "title-background", "Title Background");
	psy_property_set_str(section, "title-font", "Title Font");
	psy_property_set_str(section, "param-background", "Param Background");
	psy_property_set_str(section, "param-font", "Param Font");
	psy_property_set_str(section, "value-background", "Value Background");
	psy_property_set_str(section, "value-font", "Value Font");
	psy_property_set_str(section,
		"selparam-background", "SelParam Background");
	psy_property_set_str(section, "selparam-font", "SelParam Font");
	psy_property_set_str(section,
		"selvalue-background", "SelValue Background");
	psy_property_set_str(section, "selvalue-font", "SelValue Font");
	// settingsview/keyboard/misc	
	psy_property_set_str(section,
		"record-tws",
		"Record Mouse Tweaks as tws (Smooth tweaks)");
	psy_property_set_str(section,
		"advance-line-on-record", "Advance Line On Record");
	// instrumentsbox
	section = psy_property_append_section(lang, "instrumentsbox");
	psy_property_set_str(section,
		"instrument-groups", "Instrument Groups");
	psy_property_set_str(section, "group-instruments", "Group Instruments");
	// instrumentview
	section = psy_property_append_section(lang, "instrumentview");
	psy_property_set_str(section,
		"instrument-name", "Instrument Name");
	psy_property_set_str(section, "new-note-action", "New Note Action");
	psy_property_set_str(section, "note-cut", "Note Cut");
	psy_property_set_str(section, "note-release", "Note Release");
	psy_property_set_str(section, "note-fadeout", "Note Fadeout");
	psy_property_set_str(section, "none", "None");
	psy_property_set_str(section,
		"play-sample-to-fit", "Play sample to fit");
	psy_property_set_str(section, "pattern-rows", "Pattern rows");
	psy_property_set_str(section, "global-volume", "Global volume");
	psy_property_set_str(section,
		"amplitude-envelope", "Amplitude envelope");
	psy_property_set_str(section, "attack", "Attack");
	psy_property_set_str(section, "decay", "Decay");
	psy_property_set_str(section, "sustain-level", "Sustain level");
	psy_property_set_str(section, "release", "Release");
	psy_property_set_str(section, "filter-type", "Filter type");
	psy_property_set_str(section, "filter-envelope", "Filter envelope");
	psy_property_set_str(section, "cut-off", "Cut-off");
	psy_property_set_str(section, "res", "Res/bandw.");
	psy_property_set_str(section, "mod", "Mod. Amount");
	psy_property_set_str(section, "random-panning", "Random panning");
	psy_property_set_str(section, "general", "General");
	psy_property_set_str(section, "volume", "Volume");
	psy_property_set_str(section, "pan", "Pan");
	psy_property_set_str(section, "filter", "Filter");
	psy_property_set_str(section, "pitch", "Pitch");
	// machineview
	section = psy_property_append_section(lang, "machineview");
	psy_property_set_str(section, "new-machine", "New Machine");
	psy_property_set_str(section, "wires", "Wires");
	psy_property_set_str(section,
		"no-machines-loaded", "No Machines Loaded");
	psy_property_set_str(section, "no-machine", "No Machine");
	psy_property_set_str(section,
		"connect-to-mixer-send-return-input",
		"Connect to Mixer-send/return-input");
	// PatternView
	section = psy_property_append_section(lang, "patternview");
	psy_property_set_str(section, "line", "Line");
	psy_property_set_str(section, "defaults", "Defaults");
	psy_property_set_str(section, "step", "Step");
	// SequenceView
	section = psy_property_append_section(lang, "sequencerview");
	psy_property_set_str(section, "follow-song", "Follow Song");
	psy_property_set_str(section, "show-playlist", "Show Playlist");
	psy_property_set_str(section,
		"show-pattern-names", "Show Pattern Names");
	psy_property_set_str(section, "record-tweaks", "Record tweaks");
	psy_property_set_str(section,
		"multichannel-audition", "Multichannel audition");
	psy_property_set_str(section, "duration", "Duration");
	psy_property_set_str(section, "ins", "Insert");
	psy_property_set_str(section, "new", "New");
	psy_property_set_str(section, "clone", "Clone");
	psy_property_set_str(section, "del", "Delete");
	psy_property_set_str(section, "new-trk", "New Track");
	psy_property_set_str(section, "del-trk", "Del Track");
	psy_property_set_str(section, "clear", "Clear");
	psy_property_set_str(section, "copy", "Copy");
	psy_property_set_str(section, "paste", "Paste");
	psy_property_set_str(section, "singlesel", "SingleSel");
	psy_property_set_str(section, "multisel", "MultiSel");
	section = psy_property_append_section(lang, "newmachine");
	psy_property_set_str(section, "all", "All");
	psy_property_set_str(section, "favorites", "Favorites");
	psy_property_set_str(section,
		"jme-version-unknown",
		"Load new gamefx and blitz if version is unknown");
	psy_property_set_str(section,
		"song-loading-compatibility", "Song loading compatibility");
	psy_property_set_str(section, "rescan", "Rescan");
	psy_property_set_str(section,
		"select-plugin-directories", "Select plugin directories");
	psy_property_set_str(section,
		"sort-by-favorite", "Sort by favorite");
	psy_property_set_str(section, "sort-by-name", "Sort by name");
	psy_property_set_str(section, "sort-by-type", "Sort by type");
	psy_property_set_str(section, "sort-by-mode", "Sort by mode");
	psy_property_set_str(section,
		"select-plugin-to-view-description",
		"Select a plugin to view its description");
	section = psy_property_append_section(lang, "samplesview");
	psy_property_set_str(section, "samplename", "Samplename");
	psy_property_set_str(section, "samplerate", "Samplerate");
	psy_property_set_str(section, "samples", "Samples");
	psy_property_set_str(section, "groupsamples", "Group Samples");
	psy_property_set_str(section,
		"groupsfirstsample", "Groups first sample");
	psy_property_set_str(section, "default-volume", "Default volume");
	psy_property_set_str(section, "global-volume", "Global volume");
	psy_property_set_str(section, "pan-position", "Pan Position");
	psy_property_set_str(section, "sampled-note", "Sampled note");
	psy_property_set_str(section, "pitch-finetune", "Pitch finetune");
	psy_property_set_str(section, "process", "Process");
	psy_property_set_str(section, "no-wave-loaded", "No wave loaded");	
	section = psy_property_append_section(lang, "cmds");
	psy_property_set_str(section, "keymap", "Keymap");
	// General
	psy_property_set_str(section, "help", "Help");
	psy_property_set_str(section, "helpshortcut", "Kbd Help");
	psy_property_set_str(section, "editmachine", "Machines" );
	psy_property_set_str(section, "editpattern", "Patterns");
	psy_property_set_str(section, "addmachine", "Add Machine");
	psy_property_set_str(section, "playsong", "Play Song");
	psy_property_set_str(section, "playstart", "Play Start");
	psy_property_set_str(section, "playfrompos", "Play Pos");
	psy_property_set_str(section, "playstop", "stop");
	psy_property_set_str(section, "songposdec", "Seqpos dec");
	psy_property_set_str(section, "songposinc", "Seqpos Inc");
	psy_property_set_str(section, "maxpattern", "Max Pattern");
	psy_property_set_str(section, "infomachine", "gear");
	psy_property_set_str(section, "editinstr", "Instruments");
	psy_property_set_str(section, "editsample", "Samples");
	psy_property_set_str(section, "editwave", "Wave Edit");
	psy_property_set_str(section, "terminal", "Terminal");
	psy_property_set_str(section, "instrdec", "Current Instrument -1");
	psy_property_set_str(section, "instrinc", "Current Instrument +1");
	// Tracker
	psy_property_set_str(section, "navup", "Nav Up");
	psy_property_set_str(section, "navdown", "Nav Down");
	psy_property_set_str(section, "navleft", "Nav Left");
	psy_property_set_str(section, "navright", "Nav Right");
	psy_property_set_str(section, "navpageup", "Nav Up 16");
	psy_property_set_str(section, "navpagedown", "Nav Down 16");
	psy_property_set_str(section, "navtop", "Nav Top");
	psy_property_set_str(section, "navbottom", "Nav Bottom");
	psy_property_set_str(section, "columnprev", "Prev column");
	psy_property_set_str(section, "columnnext", "Next column");

	psy_property_set_str(section, "rowinsert", "Insert row");
	psy_property_set_str(section, "rowdelete", "Delete row");
	psy_property_set_str(section, "rowclear", "Clear row");

	psy_property_set_str(section, "blockstart", "Block Start");
	psy_property_set_str(section, "blockend", "Block End");
	psy_property_set_str(section, "blockunmark", "Block Unmark");
	psy_property_set_str(section, "blockcut", "Block cut");
	psy_property_set_str(section, "blockcopy", "Block copy");
	psy_property_set_str(section, "blockpaste", "Block paste");
	psy_property_set_str(section, "blockmix", "Block Mix");

	psy_property_set_str(section, "transposeblockinc", "Transpose Block +1");
	psy_property_set_str(section, "transposeblockdec", "Transpose Block -1");
	psy_property_set_str(section, "transposeblockinc12", "Transpose Block +12");
	psy_property_set_str(section, "transposeblockdec12", "Transpose Block -12");

	psy_property_set_str(section, "selectall", "Block Select All");
	psy_property_set_str(section, "selectcol", "Block Select Column");
	psy_property_set_str(section, "selectbar", "Block Select Bar");

	psy_property_set_str(section, "selectmachine",
		"Select Mac/Ins in Cursor Pos");
	psy_property_set_str(section, "undo", "Edit Undo");
	psy_property_set_str(section, "redo", "Edit Redo");
	psy_property_set_str(section, "followsong", "Follow Song");
}

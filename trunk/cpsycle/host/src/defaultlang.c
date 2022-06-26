/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "defaultlang.h"

/* container */
#include <propertiesio.h>
/* std */
#include <string.h>

static psy_Property* currsection = NULL;

static psy_Property* setsection(psy_Property* section)
{
	currsection = section;
	return section;
}

static void add(const char* key, const char* translation)
{
	assert(currsection);

	psy_property_set_str(currsection, key, translation);
}

/* prototypes */
static void make_translator_filebar(psy_Property* parent);
static void make_translator_undobar(psy_Property* parent);
static void make_translator_playbar(psy_Property* parent);
static void make_translator_playlist(psy_Property* parent);
static void make_translator_metronomebar(psy_Property* parent);
static void make_translator_mainview(psy_Property* parent);
static void make_translator_help(psy_Property* parent);
static void make_translator_greetings(psy_Property* parent);
static void make_translator_machinebar(psy_Property* parent);
static void make_translator_trackbar(psy_Property* parent);
static void make_translator_octavebar(psy_Property* parent);
static void make_translator_edit(psy_Property* parent);
static void make_translator_timebar(psy_Property* parent);
static void make_translator_linesperbeatbar(psy_Property* parent);
static void make_translator_channelmapping(psy_Property* parent);
static void make_translator_renderview(psy_Property* parent);
static void make_translator_export(psy_Property* parent);
static void make_translator_gear(psy_Property* parent);
static void make_translator_cpu(psy_Property* parent);
static void make_translator_songproperties(psy_Property* parent);
static void make_translator_settingsview(psy_Property* parent);
static void make_translator_instrumentsbox(psy_Property* parent);
static void make_translator_instrumentview(psy_Property* parent);
static void make_translator_machineview(psy_Property* parent);
static void make_translator_machineframe(psy_Property* parent);
static void make_translator_patternview(psy_Property* parent);
static void make_translator_transformpattern(psy_Property* parent);
static void make_translator_swingfillview(psy_Property* parent);
static void make_translator_sequenceview(psy_Property* parent);
static void make_translator_seqedit(psy_Property* parent);
static void make_translator_newmachine(psy_Property* parent);
static void make_translator_samplesview(psy_Property* parent);
static void make_translator_general(psy_Property* parent);
static void make_translator_tracker(psy_Property* parent);
static void make_translator_messages(psy_Property* parent);

/* implementation */
void make_translator_default(psy_Property* lang)
{
	psy_Property* section;

	assert(lang);

	section = setsection(lang);
	add("lang", "en");
	make_translator_filebar(lang);
	make_translator_undobar(lang);
	make_translator_playbar(lang);
	make_translator_playlist(lang);
	make_translator_metronomebar(lang);	
	make_translator_mainview(lang);
	make_translator_help(lang);
	make_translator_greetings(lang);
	make_translator_machinebar(lang);
	make_translator_trackbar(lang);
	make_translator_octavebar(lang);
	make_translator_edit(lang);
	make_translator_timebar(lang);
	make_translator_linesperbeatbar(lang);
	make_translator_channelmapping(lang);
	make_translator_renderview(lang);
	make_translator_export(lang);
	make_translator_gear(lang);
	make_translator_cpu(lang);
	make_translator_songproperties(lang);
	make_translator_settingsview(lang);
	make_translator_instrumentsbox(lang);
	make_translator_instrumentview(lang);
	make_translator_machineview(lang);
	make_translator_machineframe(lang);
	make_translator_patternview(lang);
	make_translator_transformpattern(lang);
	make_translator_swingfillview(lang);	
	make_translator_sequenceview(lang);
	make_translator_seqedit(lang);
	make_translator_newmachine(lang);
	make_translator_samplesview(lang);
	make_translator_general(lang);
	make_translator_tracker(lang);
	make_translator_messages(lang);
}

void make_translator_filebar(psy_Property* parent)
{	
	setsection(psy_property_append_section(parent, "file"));	
	add("showall", "Show all");
	add("file", "File:");
	add("song", "Song");
	add("diskop", "Disk op.");
	add("load", "Load");
	add("save", "Save");
	add("refresh", "Refresh");
	add("export", "Export");
	add("new", "New");
	add("render", "Render");
	add("loadsong", "Load Song");
	add("savesong", "Save Song");	
	add("plugin-filesave", "Plugin File Load");
	add("plugin-fileload", "Plugin File Save");
	add("exit", "Exit");
}

void make_translator_undobar(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "undo"));
	add("undo", "Undo");
	add("redo", "Redo");
}

void make_translator_playbar(psy_Property* parent)
{	
	setsection(psy_property_append_section(parent, "play"));
	add("loop", "Loop");
	add("record-notes", "Record Notes");
	add("play", "Play");
	add("stop", "Stop");
	add("song", "Song");
	add("sel", "Sel");
	add("beats", "Beats");
}

void make_translator_playlist(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "playlist"));
	add("play", "Play");
	add("stop", "Stop");
	add("delete", "Delete");
	add("clear", "Clear");
	add("recent-songs", "Recent Songs");
}

void make_translator_metronomebar(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "metronome"));
	add("metronome", "Metronome");
	add("configure", "(Configure)");	
	add("precount", "Precount");
}

void make_translator_mainview(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "main"));	
	add("machines", "Machines");
	add("patterns", "Patterns");
	add("pattern", "Pattern");
	add("samples", "Samples");
	add("instrument", "Instrument");
	add("instruments", "Instruments");
	add("scripts", "...");
	add("help", "Help");
	add("settings", "Settings");
	add("properties", "Properties");
	add("exit", "Exit");
	add("kbd", "Kbd");
	add("terminal", "Terminal");
	add("floated", "This view is floated");
}

void make_translator_help(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "help"));
	add("help", "Help");
	add("about", "About");
	add("greetings", "Greetings");
	add("licence", "Licence");
	add("contributors-credits", "Contributors / Credits");
	add("ok", "OK");
	add("extract", "Extract Help");
	add("combine", "Combine Help");
}

void make_translator_greetings(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "greetings"));
	add("thanks", "Thanks! / ");
	add("wantstothank",
		"Psycledelics, the Community, wants to thank the following people "
		"for their contributions in the developement of Psycle");
	add("showargurus", "Show Original Arguru's Greetings");
	add("showcurrent", "Show Current Greetings");
}

void make_translator_machinebar(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "machinebar"));
	add("gear", "Gear Rack");
	add("dock", "Param Rack");
	add("editor", "Editor");
	add("cpu", "CPU");
	add("midi", "MIDI");
}

void make_translator_trackbar(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "trackbar"));
	add("tracks", "Tracks");
}

void make_translator_octavebar(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "octavebar"));
	add("octave", "Octave");
}

void make_translator_edit(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "edit"));
	add("cut", "Cut");
	add("copy", "Copy");
	add("crop", "Crop");
	add("delete", "Delete");
	add("paste", "Paste");
	add("mixpaste", "MixPaste");
	add("clear", "Clear");
	add("duplicate", "Duplicate");
	add("clone", "Clone");
	add("transposeblockinc", "Transpose +1");
	add("transposeblockdec", "Transpose -1");
	add("transposeblockinc12", "Transpose +12");
	add("transposeblockdec12", "Transpose -12");
	add("transposeblockdec12", "Transpose -12");
	add("blockinterpolate", "Interpolate (Linear)");
	add("blockinterpolatecurve", "Interpolate (Curve)");
	add("blocksetinstr", "Change Instrument");
	add("blocksetmachine", "Change Machine");
	add("blockswingfill", "Swing Fill Block");
	add("trackswingfill", "Swing Fill Track");
	add("searchreplace", "Search and Replace");
	add("importpsb", "Import (psb)");
	add("exportpsb", "Export (psb)");
}

void make_translator_timebar(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "timebar"));
	add("tempo", "Tempo");
}

void make_translator_linesperbeatbar(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "lpb"));
	add("lines-per-beat", "Lines per beat");
}

void make_translator_channelmapping(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "channelmapping"));
	add("autowire", "Autowire");
	add("unselect-all", "Unselect all");
	add("remove-connection-with-right-click",
		"Remove connection with right click");
}

void make_translator_renderview(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "render"));
	add("render", "Render");
	add("configure", "Configure");
	add("file", "File");
	add("sequence-positions", "Sequence positions");
	add("quality", "Quality");
	add("dither", "Dither");
	add("selection", "Selection");
	add("record", "Record");
	add("enable", "enable");
	add("from", "from");
	add("to", "to");	
	add("save-wave", "Save Wave");
	add("entire-song", "The Entire Song");
	add("songsel", "The Song Selection");
	add("samplerate", "Samplerate");
	add("bitdepth", "Bit depth");
	add("none", "None");
	add("pdf", "Prob. Distribution");
	add("triangular", "Triangular");
	add("rectangular", "Rectangular");
	add("gaussian", "Gaussian");
	add("noise-shaping", "Noise-shaping");
	add("high-pass-contour", "High-Pass Contour");
	add("none", "none");
	add("output-path", "Output path");
	add("save-each-unmuted", "Save each unmuted");
}

void make_translator_export(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "export"));
	add("export-module", "Export Module");
	add("export-midifile", "Export Midi File");
	add("export-lyfile", "Export LilyPond File");
}

void make_translator_gear(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "gear"));
	add("clone", "Clone");
	add("delete", "Delete");
	add("properties", "Properties");
	add("parameters", "Parameters");
	add("exchange", "Exchange");
	add("show-master", "Show Master");
	add("connecttomaster", "Connect To Master");
	add("create-replace", "Create/Replace");
	add("mute-unmute", "Mute/Unmute");
	add("effects", "Effects");
	add("instruments", "Instruments");
	add("generators", "Generators");
	add("waves", "Waves");
}

void make_translator_cpu(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "cpu"));
	add("performance", "CPU Performance");
	add("resources", "Windows Resources");
	add("ram", "Physical Memory(RAM)");
	add("ram", "Memoria fisica");
	add("swap", "Page File (Swap)");
	add("vram", "Virtual Memory");
	add("audio-threads", "Audio threads");
	add("total", "Total (time)");
	add("machines", "Machines");
	add("routing", "Routing");	
}

void make_translator_songproperties(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "songproperties"));
	add("title", "Song Title");
	add("credits", "Credits");
	add("speed", "Speed");
	add("tempo", "Tempo");
	add("lpb", "Lines Per Beat");
	add("tpb", "Ticks Per Beat");
	add("etpb", "Extra Ticks Per Beat");
	add("realtempo", "Real Tempo");
	add("realtpb", "Real Ticks Per Beat");
	add("extcomments", "Extended Comments");
}

void make_translator_settingsview(psy_Property* parent)
{
	psy_Property* section;
	psy_Property* subsection;

	section = setsection(psy_property_append_section(parent, "settingsview"));
	/* settingsview */
	add("event-input", "Event Input");
	add("audio-drivers", "Audio Drivers");
	add("configure", "Configure");	
	add("compatibility", "Compatibility");
	add("choose-font", "Choose Font");	
	add("none", "None");
	add("from", "from");
	add("to", "to");
	/* settingsview global */
	setsection(psy_property_append_section(section, "global"));
	add("configuration", "Configuration");
	add("enable-audio", "Enable Audio");
	add("regenerate-plugincache", "Regenerate the plugin cache");		
	add("language", "Language");
	add("de", "german");
	add("en", "english");
	add("es", "spanish");
	add("importconfig", "Import MFC Ini File");
	/* settingsview-midicontrollers	*/
	setsection(psy_property_append_section(section, "midicontrollers"));
	add("controllers", "MIDI Controllers");
	add("add", "Add Controller Map");
	add("remove", "Remove");
	add("mapping", "Midi controllers mapping");
	add("macselect", "Select Generator/Instrument with");
	add("auxselect", "Select Instrument/Aux with");
	add("select-inpsycle", "Currently selected in Psycle");
	add("select-bybank", "Selected by bank change");
	add("select-byprogram", "Selected by program change");
	add("select-bychannel", "Selected by MIDI channel index");
	add("recordrawmidiasmcm", "Record Raw MIDI as MCM");
	add("mapping-map-velocity", "Map Velocity");
	add("mapping-map-pitchwheel", "Map Pitch Wheel");
	add("mapping-map-controller", "Map Controller");
	add("mapping-map-active", "active");		
	/* settingsview-metronome */
	setsection(psy_property_append_section(section, "metronome"));
	add("metronome", "Metronome");
	add("show", "Show Metronome Bar");
	add("machine", "Machine");
	add("note", "Note");
	/* settingsview-seqedit */
	setsection(psy_property_append_section(section, "seqeditor"));
	add("seqeditor", "Sequence Editor");	
	add("machine", "New Song Default Sampler (For current change song properties)");
	/* settingsview-directories */
	setsection(psy_property_append_section(section, "dirs"));
	add("dirs", "Directories");
	add("song", "Song directory");
	add("samples", "Samples directory");
	add("plugin32", "Plug-in32 directory");
	add("plugin64", "Plug-in64 directory");
	add("lua", "Lua-scripts directory");
	add("vst32", "Vst32 directories");
	add("vst64", "Vst64 directories");
	add("ladspa", "Ladspa directories");
	add("skin", "Skin directory");
	/* settingsview-theme */
	setsection(section);
	add("theme", "Theme");
	add("background", "Background");
	add("font", "Font");
	add("name", "Name");
	/* settingsview-general */
	setsection(psy_property_append_section(section, "general"));
	add("general", "General");
	add("version", "Version");
	add("show-about-at-startup", "Show About At Startup");
	add("show-song-info-on-load", "Show Song Info On Load");
	add("show-maximized-at-startup", "Show maximized at startup");
	add("show-playlist", "Show Playlist/Recentsongs");
	add("show-sequenceedit", "Show Sequenceeditor");
	add("show-sequencestepbar", "Show Sequencestepbar");
	add("save-recent-songs", "Save recent songs");
	add("play-song-after-load", "Play Song After Load");	
	add("show-pattern-names", "Show Pattern Names");
	/* settingsview-visual */
	setsection(psy_property_append_section(section, "visual"));
	add("visual", "Visual");
	add("load-skin", "Load skin");
	add("default-skin", "Default skin");
	add("default-font", "Default Font");
	add("default-line", "Default Line");
	add("load-dial-bitmap", "Load dial bitmap");
	add("apptheme", "App Theme");
	add("light", "light");
	add("dark", "dark");
	/* settingsview-patternview */
	subsection = setsection(psy_property_append_section(section, "pv"));
	add("patternview", "Pattern View");
	add("font", "Font");
	add("choose-font", "Choose Font");
	add("draw-empty-data", "Draw empty data");
	add("smoothscroll", "Scroll smooth");
	add("default-entries", "Default entries");
	add("displaysinglepattern", "Display Single Pattern");
	add("useheaderbitmap", "Use Pattern Header Bitmap");
	add("line-numbers", "Line numbers");
	add("beat-offset", "Beat offset");
	add("line-numbers-cursor", "Line numbers cursor");
	add("line-numbers-in-hex", "Line numbers in HEX");
	add("wide-instrument-column", "Wide instrument column");
	add("pattern-track-scopes", "Pattern track scopes");
	add("wrap-around", "Wrap Around");
	add("center-cursor-on-screen", "Center cursor on screen");
	add("bar-highlighting", "Bar highlighting: (beats/bar)");
	add("a4-440hz", "A4 is 440Hz (Otherwise it is 220Hz)");
	add("patterndisplay", "Display pattern in");
	add("tracker", "Tracker");
	add("piano", "Pianoroll");
	add("splitvertical", "both side by side");
	add("splithorizontal", "both one below the other");
	add("move-cursor-when-paste", "Move Cursor When Paste");	
	setsection(psy_property_append_section(subsection, "theme"));
	/* settingsview-patternview/theme */
	add("theme", "Theme");
	add("fontface", "Pattern Fontface");
	add("fontpoint", "Pattern Fontpoint");
	add("fontflags", "Pattern Fontflags");
	add("font_x", "Point X");
	add("font_y", "Point Y");
	add("headerskin", "Pattern Header Skin");
	add("separator", "Separator Left");
	add("separator2", "Separator Right");
	add("background", "BackGnd Left");
	add("background2", "BackGnd Right");	
	add("font", "Font Left");
	add("font2", "Font Right");
	add("fontcur", "Font Cur Left");
	add("fontcur2", "Font Cur Right");
	add("fontsel", "Font Sel Left");
	add("fontsel2", "Font Sel Right");
	add("fontplay", "Font Play Left");
	add("fontplay2", "Font Play Right");
	add("row", "Row Left");
	add("row2", "Row Right");
	add("rowbeat", "Beat Left");
	add("rowbeat2", "Beat Right");
	add("row4beat", "Bar Left");
	add("row4beat2", "Bar Right");
	add("selection", "Selection Left");
	add("selection2", "Selection Right");
	add("playbar", "Playbar Left");
	add("playbar2", "Playbar Right");
	add("cursor", "Cursor Left");
	add("cursor2", "Cursor Right");
	add("midline", "Midline Left");
	add("midline2", "Midline Right");
	/* settingsview-machineview	*/
	subsection = setsection(psy_property_append_section(section, "mv"));
	add("machineview", "Machine View");
	add("draw-machine-indexes", "Draw Machine Indexes");
	add("draw-vumeters", "Draw VU Meters");
	add("draw-wirehover", "Draw Wire Hover");
	add("draw-virtualgenerators", "Draw Virtual Generators");
	add("stackview", "Stackview");
	add("stackview-draw-smalleffects", "Draw Small Effects");
	/* settingsview-machineview-stackview */
	setsection(psy_property_append_section(subsection, "theme"));
	/* settingsview-machineview-theme */
	add("theme", "Theme");
	add("vu-background", "Vu BackGround");
	add("vu-bar", "Vu Bar");
	add("onclip", "OnClip");
	add("generators-font-face", "Generators Font Face");
	add("generators-font-point", "Generators Font Point");
	add("effect-fontface", "Effect Fontface");
	add("effect-font-point", "Effect Font Point");
	add("effect-font-flags", "Effect Font Flags");	
	add("background", "Background");
	add("wirecolour", "Wires Left");
	add("wirecolour2", "Wires Right");
	add("polygons", "Polygons");
	add("generators-font", "Generators Font");
	add("effects-font", "Effects Font");
	add("wire-width", "Wire width");
	add("antialias-halo", "AntiAlias halo");
	add("machine-background", "Machine Background");
	add("polygon-size", "Polygon size");
	add("machineskin", "Machineskin");
	/* settingsview-paramview */
	subsection = setsection(psy_property_append_section(section, "paramview"));
	add("native-machine-parameter-window",
		"Native Machine Parameter Window");
	add("font", "Font");
	add("load-dial-bitmap", "Load Dial Bitmap");
	add("default-skin", "Default skin");
	/* settingsview-paramview-theme */
	setsection(psy_property_append_section(subsection, "theme"));
	add("theme", "Theme");
	add("title-background", "Title Background");
	add("title-font", "Title Font");
	add("param-background", "Param Background");
	add("param-font", "Param Font");
	add("value-background", "Value Background");
	add("value-font", "Value Font");
	add("selparam-background", "SelParam Background");
	add("selparam-font", "SelParam Font");
	add("selvalue-background", "SelValue Background");
	add("selvalue-font", "SelValue Font");
	add("machinedialbmp", "Machine Dial Bitmap");
	/* settingsview-keyboard-misc */
	setsection(psy_property_append_section(section, "kbd"));
	add("kbd-misc", "Keyboard and misc");
	add("record-tws",
		"Record Mouse Tweaks as tws (Smooth tweaks)");
	add("advance-line-on-record", "Advance Line On Record");
	add("ctrl-play", "Right CTRL = play; Edit Toggle = stop");
	add("ft2-home", "FT2 Style Home / End Behaviour");
	add("ft2-delete", "FT2 Style Delete Behaviour");
	add("cursoralwayssdown", "Cursor always moves down in Effect Column");
	add("force-patstep1", "Force pattern step 1 when moving with cursors");
	add("pgupdowntype", "Page Up / Page Down step by");
	add("pgupdowntype", "Page Up / Page Down step by");
	add("pgupdowntype-one-beat", "one beat");
	add("pgupdowntype-one-bar", "one bar");
	add("pgupdowntype-lines", "lines");
	add("pgupdowntype-step-lines", "Page Up / Page Down step lines");
	add("misc", "Miscellaneous options");
	add("savereminder", "\"Save file?\" reminders on Load, New or Exit");
	add("numdefaultlines", "Default lines on new pattern");
	add("allowmultiinstances", "Allow multiple instances of Psycle");
	add("ft2-explorer", "Use FT2 Style Fileexplorer");
}

void make_translator_instrumentsbox(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "instrumentsbox"));
	add("instrument-groups", "Instrument Groups");
	add("group-instruments", "Group Instruments");
}

void make_translator_instrumentview(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "instrumentview"));
	add("instrument", "Instrument");
	add("instrument-name", "Instrument Name");
	add("new-note-action", "New Note Action");
	add("same", "but if the same");
	add("do", "do");
	add("note-cut", "Note Cut");
	add("note-release", "Note Release");
	add("note-fadeout", "Note Fadeout");
	add("none", "None");
	add("notemap", "Notemap");
	add("play-sample-to-fit", "Play sample to fit");
	add("pattern-rows", "Pattern rows");
	add("global-volume", "Global volume");
	add("amplitude-envelope", "Amplitude envelope");
	add("pan-envelope", "Pan envelope");
	add("pitch-envelope", "Pitch envelope");
	add("swing", "Swing (Randomize)");
	add("fadeout", "Fadeout");
	add("attack", "Attack");
	add("decay", "Decay");
	add("sustain-level", "Sustain level");
	add("release", "Release");
	add("filter-type", "Filter type");
	add("filter-envelope", "Filter envelope");
	add("cut-off", "Cut-off");
	add("res", "Res/bandw.");
	add("mod", "Mod. Amount");
	add("random-panning", "Random panning");
	add("general", "General");
	add("volume", "Volume");
	add("pan", "Pan");
	add("filter", "Filter");
	add("pitch", "Pitch");
	add("add", "Add");
	add("remove", "Remove");
	add("smplidx", "Smpl Idx");
	add("key", "Key");
	add("keylo", "Key Lo");
	add("keyhi", "Key Hi");
	add("keyfixed", "Key Fixed");
}

void make_translator_machineview(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "machineview"));
	add("new-machine", "New Machine");
	add("wires", "Wires");
	add("stack", "Stack");
	add("no-machines-loaded", "No Machines Loaded");
	add("no-machine", "No Machine");
	add("connect-to-mixer-send-return-input",
		"Connect to Mixer-send/return-input");
	add("delete", "Delete");
	add("editname", "Editname");
	add("mute", "Mute");
	add("pwr", "Pwr");
	setsection(psy_property_append_section(parent, "stackview"));
	add("inputs", "Inputs");
	add("effects", "Effects");
	add("outputs", "Outputs");
	add("volumes", "Volumes");
}

void make_translator_machineframe(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "machineframe"));
	add("about", "About");
	add("pwr", "Pwr");
	add("parameters", "Parameters");
	add("parammap", "Parammap");
	add("command", "Command");
	add("help", "Help");
	add("bus", "Bus");
	add("bank", "Bank");
	add("program", "Program");
	add("import", "Import");
	add("export", "Export");
	add("saveas", "Save as");
}

void make_translator_patternview(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "patternview"));
	add("line", "Line");
	add("defaults", "Defaults");
	add("step", "Step");
	add("patname", "Pattern Name");
	add("nopattern", "No Pattern");
	add("length", "Length");
	add("apply", "Apply");
	add("tracker", "Tracker");
	add("roll", "Pianoroll");
	add("horz", "Horizontal");
	add("vert", "Vertical");
	add("split", "Split");
	add("properties", "Properties");
	add("beats", "Beats");
	add("keyboard", "Keyboard");
	add("showtracks", "Show Tracks");
	add("all", "All");
	add("current", "Current");
	add("active", "Active");
}

void make_translator_transformpattern(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "transformpattern"));
	add("searchpattern", "Search pattern");
	add("note", "Note");
	add("instr", "Instrum/Aux");
	add("mac", "Machine");
	add("replacepattern", "Replace pattern");
	add("search", "Search");	
	add("replaceall", "Replace all");
	add("cancel", "Cancel");	
	add("searchon", "Search on");
	add("entiresong", "Entire song");
	add("currentpattern", "Current pattern");
	add("currentselection", "Current selection");
}

void make_translator_swingfillview(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "swingfill"));
	add("tempo", "Tempo(BPM)");
	add("bpm", "BPM");
	add("center", "Center");
	add("actual", "Actual");
	add("cycle", "Cycle Length(lines)");
	add("variance", "Variance(%)");
	add("phase", "Phase (degrees)");	
	add("cancel", "Cancel");
	add("apply", "Apply");
}

void make_translator_sequenceview(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "seqview"));
	add("follow-song", "Follow Song");
	add("show-playlist", "Show Playlist");
	add("show-pattern-names", "Show Pattern Names");
	add("record-noteoff", "Record note-off");
	add("record-tweak", "Record tweak");
	add("allow-notes-to_effect", "Allow notes to effect");
	add("multichannel-audition", "Multitrack playback on keypress");
	add("duration", "Duration");
	add("more", "Show More");
	add("less", "Show Less");
	add("ins", "Ins");
	add("new", "New");
	add("clone", "Clone");
	add("del", "Delete");
	add("new-trk", "+ New Track");
	add("del-trk", "Del Track");
	add("clear", "Clear");
	add("rename", "Rename");
	add("copy", "Copy");
	add("paste", "Paste");
	add("singlesel", "SingleSel");
	add("multisel", "MultiSel");	
	add("showseqeditor", "Show SequenceEditor");
	add("hideseqeditor", "Hide SequenceEditor");
	add("showstepsequencer", "Show Stepsequencer");
	add("hidestepsequencer", "Hide Stepsequencer");
}

void make_translator_seqedit(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "seqedit"));
	add("track", "Track");
	add("move", "Move");
	add("reorder", "Reorder");
	add("insert", "Insert");
	add("timesigs", "Timesigs");
	add("repetitions", "Repetitions");
	add("noitem", "No Item Selected");
	add("patternitem", "Pattern Item");
	add("sampleitem", "Sample Item");
	add("markeritem", "Marker Item");
	add("timesigitem", "TimeSig Item");
	add("loopitem", "Loop Item");
	add("position", "Position");
	add("length", "Length");
	add("end", "End");
	add("name", "Name");
	add("numerator", "Numerator");
	add("denominator", "Denominator");
}

void make_translator_newmachine(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "newmachine"));
	add("all", "All");
	add("favorites", "Favorites");
	add("jme-version-unknown",
		"Load new gamefx and blitz if version is unknown");
	add("song-loading-compatibility", "Song loading compatibility");
	add("rescan", "Rescan");
	add("in", "in");
	add("plugin-directories", "Plugin directories");
	add("filter", "Filter by");
	add("sort", "Sort by");
	add("favorite", "Favorite");
	add("favorites", "Favorites");
	add("name", "Name");
	add("type", "Type");
	add("mode", "Mode");
	add("sections", "Sections");
	add("section", "Section");
	add("mksection", "Mksection");
	add("remove", "Remove");
	add("clear", "Clear");
	add("copy", "Copy");
	add("delete", "Delete");
	add("select-plugin-to-view-description",
		"Select a plugin to view its description");
	add("select-first-section", "Select/Create first a section");
	add("select-first-plugin", "Select first a plugin");
	add("search-plugin", "Search Plugin");
	add("category", "Category");
	add("apiversion", "API Version");
	add("version", "Version");
	add("dllname", "DllName");
	add("anycategory", "Any Category");
	add("scanning", "Scanning");
	add("stop", "Stop");
	add("stopping", "Stopping");
}

void make_translator_samplesview(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "samplesview"));
	add("samplename", "Samplename");
	add("samplerate", "Samplerate");
	add("samples", "Samples");
	add("groupsamples", "Group Samples");
	add("groupsfirstsample", "Groups first sample");
	add("default-volume", "Default volume");
	add("global-volume", "Global volume");
	add("pan-position", "Pan Position");
	add("sampled-note", "Sampled note");
	add("pitch-finetune", "Pitch finetune");
	add("process", "Process");
	add("no-wave-loaded", "No wave loaded");
	add("cont-loop", "Continuous Loop");
	add("disabled", "Disabled");
	add("forward", "Forward");
	add("bidirection", "Bidirection");
	add("start", "Start");
	add("end", "End");
	add("sustain-loop", "Sustain Loop");
	add("play", "Play");
	add("stop", "Stop");
	add("select-together", "Select Channels Together");
	add("selstart", "Selection Start");
	add("selend", "Selection End");
	add("visual", "Visual");
	add("doublecont", "Double Cont Loop");
	add("doublesus", "Double Sus Loop");
	add("lines", "Lines");
	setsection(psy_property_append_section(parent, "cmds"));
	add("keymap", "Keymap");	
}

void make_translator_general(psy_Property* parent)
{
	add("help", "Help");
	add("helpshortcut", "Kbd Help");
	add("editmachine", "Machines");
	add("editpattern", "Patterns");
	add("addmachine", "Add Machine");
	add("playsong", "Play Song");
	add("playstart", "Play Start");
	add("playfrompos", "Play Pos");
	add("playstop", "stop");
	add("songposdec", "Seqpos dec");
	add("songposinc", "Seqpos Inc");
	add("maxpattern", "Max Pattern");
	add("infomachine", "gear");
	add("editinstr", "Instruments");
	add("editsample", "Samples");
	add("editwave", "Wave Edit");
	add("terminal", "Terminal");
	add("instrdec", "Current Instrument -1");
	add("instrinc", "Current Instrument +1");
}

void make_translator_tracker(psy_Property* parent)
{
	add("navup", "Nav Up");
	add("navdown", "Nav Down");
	add("navleft", "Nav Left");
	add("navright", "Nav Right");
	add("navpageup", "Nav Up 16");
	add("navpagedown", "Nav Down 16");
	add("navtop", "Nav Top");
	add("navbottom", "Nav Bottom");
	add("columnprev", "Prev column");
	add("columnnext", "Next column");

	add("rowinsert", "Insert row");
	add("rowdelete", "Delete row");
	add("rowclear", "Clear row");

	add("blockstart", "Block Start");
	add("blockend", "Block End");
	add("blockunmark", "Block Unmark");
	add("blockcut", "Block cut");
	add("blockcopy", "Block copy");
	add("blockpaste", "Block paste");
	add("blockmix", "Block Mix");

	add("transposeblockinc", "Transpose Block +1");
	add("transposeblockdec", "Transpose Block -1");
	add("transposeblockinc12", "Transpose Block +12");
	add("transposeblockdec12", "Transpose Block -12");

	add("selectall", "Block Select All");
	add("selectcol", "Block Select Column");
	add("selectbar", "Block Select Bar");

	add("selectmachine",
		"Select Mac/Ins in Cursor Pos");
	add("undo", "Edit Undo");
	add("redo", "Edit Redo");
	add("follow_song", "Follow Song");
}

void make_translator_messages(psy_Property* parent)
{
	setsection(psy_property_append_section(parent, "msg"));
	add("psyreq", "Psycle Request");
	add("seqclear", "Sequence Clear Request! Do you really want clear the sequenceand pattern data?");
	add("yes", "Yes");
	add("no", "No");
	add("cont", "Continue");
	add("psyexit", "Exit Psycle Request, but your Song is not saved!");
	add("saveexit", "Save and Exit");
	add("nosaveexit", "Exit (no save)");
	add("newsong", "New Song Request, but your Song is not saved!");
	add("savenew", "Save and Create New Song");
	add("nosavenew", "Create New Song (no save)");
	add("loadsong", "New Load Request, but your Song is not saved!");
	add("saveload", "Save and Load Song");
	add("nosaveload", "Load Song (no save)");
	add("audiostarting", "Audio starting");
	add("audiostarted", "Audio started");
}

#ifdef PSYCLE_DEFAULT_LANG
static int onenumproperties(void* context, psy_Property* property, uintptr_t level);

void save_translator_default(void)
{
	psy_Property* lang;
	char path[4096];

	strcpy(path, psy_dir_config());	
	strcat(path, "\\en.ini");

	lang = psy_property_allocinit_key(NULL);
	psy_property_set_comment(lang,
		"Psycle English Dictionary created by\r\n; " PSYCLE__BUILD__IDENTIFIER("\r\n; "));
	make_translator_default(lang);
	propertiesio_save(lang, path);
	psy_property_deallocate(lang);
}

void save_translator_template(void)
{
	psy_Property* lang;
	char path[4096];

	strcpy(path, psy_dir_config());
	strcat(path, "\\lang.ini");

	lang = psy_property_allocinit_key(NULL);
	psy_property_set_comment(lang,
		"Psycle Template Dictionary created by\r\n; " PSYCLE__BUILD__IDENTIFIER("\r\n; "));
	make_translator_default(lang);
	psy_property_enumerate((psy_Property*)lang, NULL,
		(psy_PropertyCallback)onenumproperties);
	propertiesio_save(lang, path);
	psy_property_deallocate(lang);
}

int onenumproperties(void* context, psy_Property* property, uintptr_t level)
{
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_STRING) {
		psy_property_set_item_str(property, "");
	}
	return 1;
}

#endif

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "defaultlang.h"


/* implementation */
void make_translator_default(psy_Dictionary* dict)
{
	assert(dict);
	
	psy_dictionary_set(dict, "lang", "en");
	/* [file] */
	psy_dictionary_set(dict, "file.showall", "Show all");
	psy_dictionary_set(dict, "file.file", "File:");
	psy_dictionary_set(dict, "file.song", "Song");
	psy_dictionary_set(dict, "file.diskop", "Disk op.");
	psy_dictionary_set(dict, "file.load", "Load");
	psy_dictionary_set(dict, "file.save", "Save");
	psy_dictionary_set(dict, "file.refresh", "Refresh");
	psy_dictionary_set(dict, "file.export", "Export");
	psy_dictionary_set(dict, "file.new", "New");
	psy_dictionary_set(dict, "file.render", "Render");
	psy_dictionary_set(dict, "file.loadsong", "Load Song");
	psy_dictionary_set(dict, "file.savesong", "Save Song");
	psy_dictionary_set(dict, "file.plugin-filesave", "Plugin File Load");
	psy_dictionary_set(dict, "file.plugin-fileload", "Plugin File Save");
	psy_dictionary_set(dict, "file.exit", "Exit");
	/* [undo] */
	psy_dictionary_set(dict, "undo.undo", "Undo");
	psy_dictionary_set(dict, "undo.redo", "Redo");
	/* [play] */	
	psy_dictionary_set(dict, "play.loop", "Loop");
	psy_dictionary_set(dict, "play.record-notes", "Record Notes");
	psy_dictionary_set(dict, "play.play", "Play");
	psy_dictionary_set(dict, "play.stop", "Stop");
	psy_dictionary_set(dict, "play.song", "Song");
	psy_dictionary_set(dict, "play.sel", "Sel");
	psy_dictionary_set(dict, "play.beats", "Beats");
	/* playlist */	
	psy_dictionary_set(dict, "playlist.play", "Play");
	psy_dictionary_set(dict, "playlist.stop", "Stop");
	psy_dictionary_set(dict, "playlist.delete", "Delete");
	psy_dictionary_set(dict, "playlist.clear", "Clear");
	psy_dictionary_set(dict, "playlist.recent-songs", "Recent Songs");
	/* metronome */	
	psy_dictionary_set(dict, "metronome.metronome", "Metronome");
	psy_dictionary_set(dict, "metronome.configure", "(Configure)");
	psy_dictionary_set(dict, "metronome.precount", "Precount");
	/* main */	
	psy_dictionary_set(dict, "main.machines", "Machines");
	psy_dictionary_set(dict, "main.patterns", "Patterns");
	psy_dictionary_set(dict, "main.pattern", "Pattern");
	psy_dictionary_set(dict, "main.samples", "Samples");
	psy_dictionary_set(dict, "main.instrument", "Instrument");
	psy_dictionary_set(dict, "main.instruments", "Instruments");
	psy_dictionary_set(dict, "main.scripts", "...");
	psy_dictionary_set(dict, "main.help", "Help");
	psy_dictionary_set(dict, "main.settings", "Settings");
	psy_dictionary_set(dict, "main.properties", "Properties");
	psy_dictionary_set(dict, "main.exit", "Exit");
	psy_dictionary_set(dict, "main.kbd", "Kbd");
	psy_dictionary_set(dict, "main.terminal", "Terminal");
	psy_dictionary_set(dict, "main.floated", "This view is floated");
	/* help */	
	psy_dictionary_set(dict, "help.help", "Help");
	psy_dictionary_set(dict, "help.about", "About");
	psy_dictionary_set(dict, "help.greetings", "Greetings");
	psy_dictionary_set(dict, "help.licence", "Licence");
	psy_dictionary_set(dict, "help.contributors-credits", "Contributors / Credits");
	psy_dictionary_set(dict, "help.ok", "OK");
	psy_dictionary_set(dict, "help.extract", "Extract Help");
	psy_dictionary_set(dict, "help.combine", "Combine Help");
	/* greetings */	
	psy_dictionary_set(dict, "greetings.thanks", "Thanks! / ");
	psy_dictionary_set(dict, "greetings.wantstothank",
		"Psycledelics, the Community, wants to thank the following people "
		"for their contributions in the developement of Psycle");
	psy_dictionary_set(dict, "greetings.showargurus", "Show Original Arguru's Greetings");
	psy_dictionary_set(dict, "greetings.showcurrent", "Show Current Greetings");
	/* machinebar */
	psy_dictionary_set(dict, "machinebar.gear", "Gear Rack");
	psy_dictionary_set(dict, "machinebar.dock", "Param Rack");
	psy_dictionary_set(dict, "machinebar.editor", "Editor");
	psy_dictionary_set(dict, "machinebar.cpu", "CPU");
	psy_dictionary_set(dict, "machinebar.midi", "MIDI");
	/* trackbar */	
	psy_dictionary_set(dict, "trackbar.tracks", "Tracks");
	/* octavebar */	
	psy_dictionary_set(dict, "octavebar.octave", "Octave");
	/* edit */	
	psy_dictionary_set(dict, "edit.cut", "Cut");
	psy_dictionary_set(dict, "edit.copy", "Copy");
	psy_dictionary_set(dict, "edit.crop", "Crop");
	psy_dictionary_set(dict, "edit.delete", "Delete");
	psy_dictionary_set(dict, "edit.paste", "Paste");
	psy_dictionary_set(dict, "edit.mixpaste", "MixPaste");
	psy_dictionary_set(dict, "edit.clear", "Clear");
	psy_dictionary_set(dict, "edit.duplicate", "Duplicate");
	psy_dictionary_set(dict, "edit.clone", "Clone");
	psy_dictionary_set(dict, "edit.transposeblockinc", "Transpose +1");
	psy_dictionary_set(dict, "edit.transposeblockdec", "Transpose -1");
	psy_dictionary_set(dict, "edit.transposeblockinc12", "Transpose +12");
	psy_dictionary_set(dict, "edit.transposeblockdec12", "Transpose -12");
	psy_dictionary_set(dict, "edit.transposeblockdec12", "Transpose -12");
	psy_dictionary_set(dict, "edit.blockinterpolate", "Interpolate (Linear)");
	psy_dictionary_set(dict, "edit.blockinterpolatecurve", "Interpolate (Curve)");
	psy_dictionary_set(dict, "edit.blocksetinstr", "Change Instrument");
	psy_dictionary_set(dict, "edit.blocksetmachine", "Change Machine");
	psy_dictionary_set(dict, "edit.blockswingfill", "Swing Fill Block");
	psy_dictionary_set(dict, "edit.trackswingfill", "Swing Fill Track");
	psy_dictionary_set(dict, "edit.searchreplace", "Search and Replace");
	psy_dictionary_set(dict, "edit.importpsb", "Import (psb)");
	psy_dictionary_set(dict, "edit.exportpsb", "Export (psb)");
	/* timebar */
	psy_dictionary_set(dict, "timebar.tempo", "Tempo");
	/* lpb */	
	psy_dictionary_set(dict, "lpb.lines-per-beat", "Lines per beat");
	/* channelmapping */
	psy_dictionary_set(dict, "channelmapping.autowire", "Autowire");
	psy_dictionary_set(dict, "channelmapping.unselect-all", "Unselect all");
	psy_dictionary_set(dict, "channelmapping.remove-connection-with-right-click",
		"Remove connection with right click");
	/* render */
	psy_dictionary_set(dict, "render.render", "Render");
	psy_dictionary_set(dict, "render.configure", "Configure");
	psy_dictionary_set(dict, "render.file", "File");
	psy_dictionary_set(dict, "render.sequence-positions", "Sequence positions");
	psy_dictionary_set(dict, "render.quality", "Quality");
	psy_dictionary_set(dict, "render.dither", "Dither");
	psy_dictionary_set(dict, "render.selection", "Selection");
	psy_dictionary_set(dict, "render.record", "Record");
	psy_dictionary_set(dict, "render.enable", "enable");
	psy_dictionary_set(dict, "render.from", "from");
	psy_dictionary_set(dict, "render.to", "to");	
	psy_dictionary_set(dict, "render.save-wave", "Save Wave");
	psy_dictionary_set(dict, "render.entire-song", "The Entire Song");
	psy_dictionary_set(dict, "render.songsel", "The Song Selection");
	psy_dictionary_set(dict, "render.samplerate", "Samplerate");
	psy_dictionary_set(dict, "render.bitdepth", "Bit depth");
	psy_dictionary_set(dict, "render.none", "None");
	psy_dictionary_set(dict, "render.pdf", "Prob. Distribution");
	psy_dictionary_set(dict, "render.triangular", "Triangular");
	psy_dictionary_set(dict, "render.rectangular", "Rectangular");
	psy_dictionary_set(dict, "render.gaussian", "Gaussian");
	psy_dictionary_set(dict, "render.noise-shaping", "Noise-shaping");
	psy_dictionary_set(dict, "render.high-pass-contour", "High-Pass Contour");
	psy_dictionary_set(dict, "render.none", "none");
	psy_dictionary_set(dict, "render.output-path", "Output path");
	psy_dictionary_set(dict, "render.save-each-unmuted", "Save each unmuted");
	/* export */
	psy_dictionary_set(dict, "export.export-module", "Export Module");
	psy_dictionary_set(dict, "export.export-midifile", "Export Midi File");
	psy_dictionary_set(dict, "export.export-lyfile", "Export LilyPond File");
	/* gear */
	psy_dictionary_set(dict, "gear.clone", "Clone");
	psy_dictionary_set(dict, "gear.delete", "Delete");
	psy_dictionary_set(dict, "gear.properties", "Properties");
	psy_dictionary_set(dict, "gear.parameters", "Parameters");
	psy_dictionary_set(dict, "gear.exchange", "Exchange");
	psy_dictionary_set(dict, "gear.show-master", "Show Master");
	psy_dictionary_set(dict, "gear.connecttomaster", "Connect To Master");
	psy_dictionary_set(dict, "gear.create-replace", "Create/Replace");
	psy_dictionary_set(dict, "gear.mute-unmute", "Mute/Unmute");
	psy_dictionary_set(dict, "gear.effects", "Effects");
	psy_dictionary_set(dict, "gear.instruments", "Instruments");
	psy_dictionary_set(dict, "gear.generators", "Generators");
	psy_dictionary_set(dict, "gear.waves", "Waves");
	/* cpu */	
	psy_dictionary_set(dict, "cpu.performance", "CPU Performance");
	psy_dictionary_set(dict, "cpu.resources", "Windows Resources");
	psy_dictionary_set(dict, "cpu.ram", "Physical Memory(RAM)");
	psy_dictionary_set(dict, "cpu.ram", "Memoria fisica");
	psy_dictionary_set(dict, "cpu.swap", "Page File (Swap)");
	psy_dictionary_set(dict, "cpu.vram", "Virtual Memory");
	psy_dictionary_set(dict, "cpu.audio-threads", "Audio threads");
	psy_dictionary_set(dict, "cpu.total", "Total (time)");
	psy_dictionary_set(dict, "cpu.machines", "Machines");
	psy_dictionary_set(dict, "cpu.routing", "Routing");	
	/* songproperties */
	psy_dictionary_set(dict, "songproperties.title", "Song Title");
	psy_dictionary_set(dict, "songproperties.credits", "Credits");
	psy_dictionary_set(dict, "songproperties.speed", "Speed");
	psy_dictionary_set(dict, "songproperties.tempo", "Tempo");
	psy_dictionary_set(dict, "songproperties.lpb", "Lines Per Beat");
	psy_dictionary_set(dict, "songproperties.tpb", "Ticks Per Beat");
	psy_dictionary_set(dict, "songproperties.etpb", "Extra Ticks Per Beat");
	psy_dictionary_set(dict, "songproperties.realtempo", "Real Tempo");
	psy_dictionary_set(dict, "songproperties.realtpb", "Real Ticks Per Beat");
	psy_dictionary_set(dict, "songproperties.extcomments", "Extended Comments");		
	/* settings */
	psy_dictionary_set(dict, "settings.event-input", "Event Input");
	psy_dictionary_set(dict, "settings.audio-drivers", "Audio Drivers");
	psy_dictionary_set(dict, "settings.configure", "Configure");	
	psy_dictionary_set(dict, "settings.compatibility", "Compatibility");
	psy_dictionary_set(dict, "settings.choose-font", "Choose Font");	
	psy_dictionary_set(dict, "settings.none", "None");
	psy_dictionary_set(dict, "settings.from", "from");
	psy_dictionary_set(dict, "settings.to", "to");
	/* settings.global */	
	psy_dictionary_set(dict, "settings.global.configuration", "Configuration");
	psy_dictionary_set(dict, "settings.global.enable-audio", "Enable Audio");
	psy_dictionary_set(dict, "settings.global.regenerate-plugincache", "Regenerate the plugin cache");		
	psy_dictionary_set(dict, "settings.global.language", "Language");
	psy_dictionary_set(dict, "settings.global.de", "german");
	psy_dictionary_set(dict, "settings.global.en", "english");
	psy_dictionary_set(dict, "settings.global.es", "spanish");
	psy_dictionary_set(dict, "settings.global.importconfig", "Import MFC Ini File");
	/* settings.midicontrollers	*/	
	psy_dictionary_set(dict, "settings.midicontrollers.controllers", "MIDI Controllers");
	psy_dictionary_set(dict, "settings.midicontrollers.add", "Add Controller Map");
	psy_dictionary_set(dict, "settings.midicontrollers.remove", "Remove Controller Map");
	psy_dictionary_set(dict, "settings.midicontrollers.select", "Select");
	psy_dictionary_set(dict, "settings.midicontrollers.mapping", "Midi controllers mapping");
	psy_dictionary_set(dict, "settings.midicontrollers.macselect", "Select Generator/Instrument with");
	psy_dictionary_set(dict, "settings.midicontrollers.auxselect", "Select Instrument/Aux with");
	psy_dictionary_set(dict, "settings.midicontrollers.select-inpsycle", "Currently selected in Psycle");
	psy_dictionary_set(dict, "settings.midicontrollers.select-bybank", "Selected by bank change");
	psy_dictionary_set(dict, "settings.midicontrollers.select-byprogram", "Selected by program change");
	psy_dictionary_set(dict, "settings.midicontrollers.select-bychannel", "Selected by MIDI channel index");
	psy_dictionary_set(dict, "settings.midicontrollers.recordrawmidiasmcm", "Record Raw MIDI as MCM");
	psy_dictionary_set(dict, "settings.midicontrollers.mapping-map-velocity", "Map Velocity");
	psy_dictionary_set(dict, "settings.midicontrollers.mapping-map-pitchwheel", "Map Pitch Wheel");
	psy_dictionary_set(dict, "settings.midicontrollers.mapping-map-controller", "Map Controller");
	psy_dictionary_set(dict, "settings.midicontrollers.mapping-map-active", "active");		
	/* settings.metronome */
	psy_dictionary_set(dict, "settings.metronome.metronome", "Metronome");
	psy_dictionary_set(dict, "settings.metronome.show", "Show Metronome Bar");
	psy_dictionary_set(dict, "settings.metronome.machine", "Machine");
	psy_dictionary_set(dict, "settings.metronome.note", "Note");
	/* settings.seqeditor */	
	psy_dictionary_set(dict, "settings.seqeditor.seqeditor", "Sequence Editor");	
	psy_dictionary_set(dict, "settings.seqeditor.machine", "New Song Default Sampler (For current change song properties)");
	/* settings.dirs */	
	psy_dictionary_set(dict, "settings.dirs.dirs", "Directories");
	psy_dictionary_set(dict, "settings.dirs.song", "Song directory");
	psy_dictionary_set(dict, "settings.dirs.samples", "Samples directory");
	psy_dictionary_set(dict, "settings.dirs.plugin32", "Plug-in32 directory");
	psy_dictionary_set(dict, "settings.dirs.plugin64", "Plug-in64 directory");
	psy_dictionary_set(dict, "settings.dirs.lua", "Lua-scripts directory");
	psy_dictionary_set(dict, "settings.dirs.vst32", "Vst32 directories");
	psy_dictionary_set(dict, "settings.dirs.vst64", "Vst64 directories");
	psy_dictionary_set(dict, "settings.dirs.ladspa", "Ladspa directories");
	psy_dictionary_set(dict, "settings.dirs.skin", "Skin directory");
	/* settings.theme */	
	psy_dictionary_set(dict, "settings.theme.theme", "Theme");
	psy_dictionary_set(dict, "settings.theme.background", "Background");
	psy_dictionary_set(dict, "settings.theme.font", "Font");
	psy_dictionary_set(dict, "settings.theme.name", "Name");
	/* settings.general */	
	psy_dictionary_set(dict, "settings.general.general", "General");
	psy_dictionary_set(dict, "settings.general.version", "Version");
	psy_dictionary_set(dict, "settings.general.show-about-at-startup", "Show About At Startup");
	psy_dictionary_set(dict, "settings.general.show-song-info-on-load", "Show Song Info On Load");
	psy_dictionary_set(dict, "settings.general.show-maximized-at-startup", "Show maximized at startup");
	psy_dictionary_set(dict, "settings.general.show-playlist", "Show Playlist/Recentsongs");
	psy_dictionary_set(dict, "settings.general.show-sequenceedit", "Show Sequenceeditor");
	psy_dictionary_set(dict, "settings.general.show-sequencestepbar", "Show Sequencestepbar");
	psy_dictionary_set(dict, "settings.general.save-recent-songs", "Save recent songs");
	psy_dictionary_set(dict, "settings.general.play-song-after-load", "Play Song After Load");	
	psy_dictionary_set(dict, "settings.general.show-pattern-names", "Show Pattern Names");
	/* settings.visual */	
	psy_dictionary_set(dict, "settings.visual.visual", "Visual");
	psy_dictionary_set(dict, "settings.visual.load-skin", "Load skin");
	psy_dictionary_set(dict, "settings.visual.default-skin", "Default skin");
	psy_dictionary_set(dict, "settings.visual.default-font", "Default Font");
	psy_dictionary_set(dict, "settings.visual.zoom", "Zoom");
	psy_dictionary_set(dict, "settings.visual.default-line", "Default Line");
	psy_dictionary_set(dict, "settings.visual.load-dial-bitmap", "Load dial bitmap");
	psy_dictionary_set(dict, "settings.visual.apptheme", "App Theme");
	psy_dictionary_set(dict, "settings.visual.light", "light");
	psy_dictionary_set(dict, "settings.visual.dark", "dark");
	/* setting.pv */	
	psy_dictionary_set(dict, "settings.pv.patternview", "Pattern View");
	psy_dictionary_set(dict, "settings.pv.font", "Font");
	psy_dictionary_set(dict, "settings.pv.choose-font", "Choose Font");
	psy_dictionary_set(dict, "settings.pv.draw-empty-data", "Draw empty data");
	psy_dictionary_set(dict, "settings.pv.smoothscroll", "Scroll smooth");
	psy_dictionary_set(dict, "settings.pv.default-entries", "Default entries");
	psy_dictionary_set(dict, "settings.pv.displaysinglepattern", "Display Single Pattern");
	psy_dictionary_set(dict, "settings.pv.useheaderbitmap", "Use Pattern Header Bitmap");
	psy_dictionary_set(dict, "settings.pv.line-numbers", "Line numbers");
	psy_dictionary_set(dict, "settings.pv.beat-offset", "Beat offset");
	psy_dictionary_set(dict, "settings.pv.line-numbers-cursor", "Line numbers cursor");
	psy_dictionary_set(dict, "settings.pv.line-numbers-in-hex", "Line numbers in HEX");
	psy_dictionary_set(dict, "settings.pv.wide-instrument-column", "Wide instrument column");
	psy_dictionary_set(dict, "settings.pv.pattern-track-scopes", "Pattern track scopes");
	psy_dictionary_set(dict, "settings.pv.wrap-around", "Wrap Around");
	psy_dictionary_set(dict, "settings.pv.center-cursor-on-screen", "Center cursor on screen");
	psy_dictionary_set(dict, "settings.pv.bar-highlighting", "Bar highlighting: (beats/bar)");
	psy_dictionary_set(dict, "settings.pv.a4-440hz", "A4 is 440Hz (Otherwise it is 220Hz)");
	psy_dictionary_set(dict, "settings.pv.patterndisplay", "Display pattern in");
	psy_dictionary_set(dict, "settings.pv.tracker", "Tracker");
	psy_dictionary_set(dict, "settings.pv.piano", "Pianoroll");
	psy_dictionary_set(dict, "settings.pv.splitvertical", "both side by side");
	psy_dictionary_set(dict, "settings.pv.splithorizontal", "both one below the other");
	psy_dictionary_set(dict, "settings.pv.move-cursor-when-paste", "Move Cursor When Paste");		
	/* settings.pv.theme */
	psy_dictionary_set(dict, "settings.pv.theme.theme", "Theme");
	psy_dictionary_set(dict, "settings.pv.theme.fontface", "Pattern Fontface");
	psy_dictionary_set(dict, "settings.pv.theme.fontpoint", "Pattern Fontpoint");
	psy_dictionary_set(dict, "settings.pv.theme.fontflags", "Pattern Fontflags");
	psy_dictionary_set(dict, "settings.pv.theme.font_x", "Point X");
	psy_dictionary_set(dict, "settings.pv.theme.font_y", "Point Y");
	psy_dictionary_set(dict, "settings.pv.theme.headerskin", "Pattern Header Skin");
	psy_dictionary_set(dict, "settings.pv.theme.separator", "Separator Left");
	psy_dictionary_set(dict, "settings.pv.theme.separator2", "Separator Right");
	psy_dictionary_set(dict, "settings.pv.theme.background", "BackGnd Left");
	psy_dictionary_set(dict, "settings.pv.theme.background2", "BackGnd Right");	
	psy_dictionary_set(dict, "settings.pv.theme.font", "Font Left");
	psy_dictionary_set(dict, "settings.pv.theme.font2", "Font Right");
	psy_dictionary_set(dict, "settings.pv.theme.fontcur", "Font Cur Left");
	psy_dictionary_set(dict, "settings.pv.theme.fontcur2", "Font Cur Right");
	psy_dictionary_set(dict, "settings.pv.theme.fontsel", "Font Sel Left");
	psy_dictionary_set(dict, "settings.pv.theme.fontsel2", "Font Sel Right");
	psy_dictionary_set(dict, "settings.pv.theme.fontplay", "Font Play Left");
	psy_dictionary_set(dict, "settings.pv.theme.fontplay2", "Font Play Right");
	psy_dictionary_set(dict, "settings.pv.theme.row", "Row Left");
	psy_dictionary_set(dict, "settings.pv.theme.row2", "Row Right");
	psy_dictionary_set(dict, "settings.pv.theme.rowbeat", "Beat Left");
	psy_dictionary_set(dict, "settings.pv.theme.rowbeat2", "Beat Right");
	psy_dictionary_set(dict, "settings.pv.theme.row4beat", "Bar Left");
	psy_dictionary_set(dict, "settings.pv.theme.row4beat2", "Bar Right");
	psy_dictionary_set(dict, "settings.pv.theme.selection", "Selection Left");
	psy_dictionary_set(dict, "settings.pv.theme.selection2", "Selection Right");
	psy_dictionary_set(dict, "settings.pv.theme.playbar", "Playbar Left");
	psy_dictionary_set(dict, "settings.pv.theme.playbar2", "Playbar Right");
	psy_dictionary_set(dict, "settings.pv.theme.cursor", "Cursor Left");
	psy_dictionary_set(dict, "settings.pv.theme.cursor2", "Cursor Right");
	psy_dictionary_set(dict, "settings.pv.theme.midline", "Midline Left");
	psy_dictionary_set(dict, "settings.pv.theme.midline2", "Midline Right");
	/* settings.mv */	
	psy_dictionary_set(dict, "settings.mv.machineview", "Machine View");
	psy_dictionary_set(dict, "settings.mv.draw-machine-indexes", "Draw Machine Indexes");
	psy_dictionary_set(dict, "settings.mv.draw-vumeters", "Draw VU Meters");
	psy_dictionary_set(dict, "settings.mv.draw-wirehover", "Draw Wire Hover");
	psy_dictionary_set(dict, "settings.mv.draw-virtualgenerators", "Draw Virtual Generators");
	psy_dictionary_set(dict, "settings.mv.stackview", "Stackview");
	psy_dictionary_set(dict, "settings.mv.stackview-draw-smalleffects", "Draw Small Effects");
	/* settings.mv.stackview */
	/* settings.mv.theme */
	psy_dictionary_set(dict, "settings.mv.theme.theme", "Theme");
	psy_dictionary_set(dict, "settings.mv.theme.vu-background", "Vu BackGround");
	psy_dictionary_set(dict, "settings.mv.theme.vu-bar", "Vu Bar");
	psy_dictionary_set(dict, "settings.mv.theme.onclip", "OnClip");
	psy_dictionary_set(dict, "settings.mv.theme.generators-font-face", "Generators Font Face");
	psy_dictionary_set(dict, "settings.mv.theme.generators-font-point", "Generators Font Point");
	psy_dictionary_set(dict, "settings.mv.theme.effect-fontface", "Effect Fontface");
	psy_dictionary_set(dict, "settings.mv.theme.effect-font-point", "Effect Font Point");
	psy_dictionary_set(dict, "settings.mv.theme.effect-font-flags", "Effect Font Flags");	
	psy_dictionary_set(dict, "settings.mv.theme.background", "Background");
	psy_dictionary_set(dict, "settings.mv.theme.wirecolour", "Wires Left");
	psy_dictionary_set(dict, "settings.mv.theme.wirecolour2", "Wires Right");
	psy_dictionary_set(dict, "settings.mv.theme.polygons", "Polygons");
	psy_dictionary_set(dict, "settings.mv.theme.generators-font", "Generators Font");
	psy_dictionary_set(dict, "settings.mv.theme.effects-font", "Effects Font");
	psy_dictionary_set(dict, "settings.mv.theme.wire-width", "Wire width");
	psy_dictionary_set(dict, "settings.mv.theme.antialias-halo", "AntiAlias halo");
	psy_dictionary_set(dict, "settings.mv.theme.machine-background", "Machine Background");
	psy_dictionary_set(dict, "settings.mv.theme.polygon-size", "Polygon size");
	psy_dictionary_set(dict, "settings.mv.theme.machineskin", "Machineskin");
	/* settings.paramview */	
	psy_dictionary_set(dict, "settings.paramview.native-machine-parameter-window",
		"Native Machine Parameter Window");
	psy_dictionary_set(dict, "settings.paramview.font", "Font");
	psy_dictionary_set(dict, "settings.paramview.load-dial-bitmap", "Load Dial Bitmap");
	psy_dictionary_set(dict, "settings.paramview.default-skin", "Default skin");
	/* settings.paramview.theme */	
	psy_dictionary_set(dict, "settings.paramview.theme.theme", "Theme");
	psy_dictionary_set(dict, "settings.paramview.theme.title-background", "Title Background");
	psy_dictionary_set(dict, "settings.paramview.theme.title-font", "Title Font");
	psy_dictionary_set(dict, "settings.paramview.theme.param-background", "Param Background");
	psy_dictionary_set(dict, "settings.paramview.theme.param-font", "Param Font");
	psy_dictionary_set(dict, "settings.paramview.theme.value-background", "Value Background");
	psy_dictionary_set(dict, "settings.paramview.theme.value-font", "Value Font");
	psy_dictionary_set(dict, "settings.paramview.theme.selparam-background", "SelParam Background");
	psy_dictionary_set(dict, "settings.paramview.theme.selparam-font", "SelParam Font");
	psy_dictionary_set(dict, "settings.paramview.theme.selvalue-background", "SelValue Background");
	psy_dictionary_set(dict, "settings.paramview.theme.selvalue-font", "SelValue Font");
	psy_dictionary_set(dict, "settings.paramview.theme.machinedialbmp", "Machine Dial Bitmap");
	/* settings.kbd */	
	psy_dictionary_set(dict, "settings.kbd.kbd-misc", "Keyboard and misc");
	psy_dictionary_set(dict, "settings.kbd.record-tws",
		"Record Mouse Tweaks as tws (Smooth tweaks)");
	psy_dictionary_set(dict, "settings.kbd.advance-line-on-record", "Advance Line On Record");
	psy_dictionary_set(dict, "settings.kbd.ctrl-play", "Right CTRL = play; Edit Toggle = stop");
	psy_dictionary_set(dict, "settings.kbd.ft2-home", "FT2 Style Home / End Behaviour");
	psy_dictionary_set(dict, "settings.kbd.ft2-delete", "FT2 Style Delete Behaviour");
	psy_dictionary_set(dict, "settings.kbd.cursoralwayssdown", "Cursor always moves down in Effect Column");
	psy_dictionary_set(dict, "settings.kbd.force-patstep1", "Force pattern step 1 when moving with cursors");
	psy_dictionary_set(dict, "settings.kbd.pgupdowntype", "Page Up / Page Down step by");
	psy_dictionary_set(dict, "settings.kbd.pgupdowntype", "Page Up / Page Down step by");
	psy_dictionary_set(dict, "settings.kbd.pgupdowntype-one-beat", "one beat");
	psy_dictionary_set(dict, "settings.kbd.pgupdowntype-one-bar", "one bar");
	psy_dictionary_set(dict, "settings.kbd.pgupdowntype-lines", "lines");
	psy_dictionary_set(dict, "settings.kbd.pgupdowntype-step-lines", "Page Up / Page Down step lines");
	psy_dictionary_set(dict, "settings.kbd.misc", "Miscellaneous options");
	psy_dictionary_set(dict, "settings.kbd.savereminder", "\"Save file?\" reminders on Load, New or Exit");
	psy_dictionary_set(dict, "settings.kbd.numdefaultlines", "Default lines on new pattern");
	psy_dictionary_set(dict, "settings.kbd.allowmultiinstances", "Allow multiple instances of Psycle");
	psy_dictionary_set(dict, "settings.kbd.ft2-explorer", "Use FT2 Style Fileexplorer");
	psy_dictionary_set(dict, "settings.kbd.followsong", "Follow Song");
	/* settings.io */	
	psy_dictionary_set(dict, "settings.io.input-output", "Input/Output");
	/* instrumentsbox */	
	psy_dictionary_set(dict, "instrumentsbox.instrument-groups", "Instrument Groups");
	psy_dictionary_set(dict, "instrumentsbox.group-instruments", "Group Instruments");
	/* instrumentview */
	psy_dictionary_set(dict, "instrumentview.instrument", "Instrument");
	psy_dictionary_set(dict, "instrumentview.instrument-name", "Instrument Name");
	psy_dictionary_set(dict, "instrumentview.new-note-action", "New Note Action");
	psy_dictionary_set(dict, "instrumentview.same", "but if the same");
	psy_dictionary_set(dict, "instrumentview.do", "do");
	psy_dictionary_set(dict, "instrumentview.note-cut", "Note Cut");
	psy_dictionary_set(dict, "instrumentview.note-release", "Note Release");
	psy_dictionary_set(dict, "instrumentview.note-fadeout", "Note Fadeout");
	psy_dictionary_set(dict, "instrumentview.none", "None");
	psy_dictionary_set(dict, "instrumentview.notemap", "Notemap");
	psy_dictionary_set(dict, "instrumentview.play-sample-to-fit", "Play sample to fit");
	psy_dictionary_set(dict, "instrumentview.pattern-rows", "Pattern rows");
	psy_dictionary_set(dict, "instrumentview.global-volume", "Global volume");
	psy_dictionary_set(dict, "instrumentview.amplitude-envelope", "Amplitude envelope");
	psy_dictionary_set(dict, "instrumentview.pan-envelope", "Pan envelope");
	psy_dictionary_set(dict, "instrumentview.pitch-envelope", "Pitch envelope");
	psy_dictionary_set(dict, "instrumentview.swing", "Swing (Randomize)");
	psy_dictionary_set(dict, "instrumentview.fadeout", "Fadeout");
	psy_dictionary_set(dict, "instrumentview.attack", "Attack");
	psy_dictionary_set(dict, "instrumentview.decay", "Decay");
	psy_dictionary_set(dict, "instrumentview.sustain-level", "Sustain level");
	psy_dictionary_set(dict, "instrumentview.release", "Release");
	psy_dictionary_set(dict, "instrumentview.filter-type", "Filter type");
	psy_dictionary_set(dict, "instrumentview.filter-envelope", "Filter envelope");
	psy_dictionary_set(dict, "instrumentview.cut-off", "Cut-off");
	psy_dictionary_set(dict, "instrumentview.res", "Res/bandw.");
	psy_dictionary_set(dict, "instrumentview.mod", "Mod. Amount");
	psy_dictionary_set(dict, "instrumentview.random-panning", "Random panning");
	psy_dictionary_set(dict, "instrumentview.general", "General");
	psy_dictionary_set(dict, "instrumentview.volume", "Volume");
	psy_dictionary_set(dict, "instrumentview.pan", "Pan");
	psy_dictionary_set(dict, "instrumentview.filter", "Filter");
	psy_dictionary_set(dict, "instrumentview.pitch", "Pitch");
	psy_dictionary_set(dict, "instrumentview.add", "Add");
	psy_dictionary_set(dict, "instrumentview.remove", "Remove");
	psy_dictionary_set(dict, "instrumentview.smplidx", "Smpl Idx");
	psy_dictionary_set(dict, "instrumentview.key", "Key");
	psy_dictionary_set(dict, "instrumentview.keylo", "Key Lo");
	psy_dictionary_set(dict, "instrumentview.keyhi", "Key Hi");
	psy_dictionary_set(dict, "instrumentview.keyfixed", "Key Fixed");
	/* machineview */
	psy_dictionary_set(dict, "machineview.new-machine", "New Machine");
	psy_dictionary_set(dict, "machineview.wires", "Wires");
	psy_dictionary_set(dict, "machineview.stack", "Stack");
	psy_dictionary_set(dict, "machineview.no-machines-loaded", "No Machines Loaded");
	psy_dictionary_set(dict, "machineview.no-machine", "No Machine");
	psy_dictionary_set(dict, "machineview.connect-to-mixer-send-return-input",
		"Connect to Mixer-send/return-input");
	psy_dictionary_set(dict, "machineview.delete", "Delete");
	psy_dictionary_set(dict, "machineview.editname", "Editname");
	psy_dictionary_set(dict, "machineview.mute", "Mute");
	psy_dictionary_set(dict, "machineview.pwr", "Pwr");
	/* stackview */
	psy_dictionary_set(dict, "stackview.inputs", "Inputs");
	psy_dictionary_set(dict, "stackview.effects", "Effects");
	psy_dictionary_set(dict, "stackview.outputs", "Outputs");
	psy_dictionary_set(dict, "stackview.volumes", "Volumes");
	/* machineframe */
	psy_dictionary_set(dict, "machineframe.about", "About");
	psy_dictionary_set(dict, "machineframe.pwr", "Pwr");
	psy_dictionary_set(dict, "machineframe.parameters", "Parameters");
	psy_dictionary_set(dict, "machineframe.parammap", "Parammap");
	psy_dictionary_set(dict, "machineframe.command", "Command");
	psy_dictionary_set(dict, "machineframe.help", "Help");
	psy_dictionary_set(dict, "machineframe.bus", "Bus");
	psy_dictionary_set(dict, "machineframe.bank", "Bank");
	psy_dictionary_set(dict, "machineframe.program", "Program");
	psy_dictionary_set(dict, "machineframe.import", "Import");
	psy_dictionary_set(dict, "machineframe.export", "Export");
	psy_dictionary_set(dict, "machineframe.saveas", "Save as");
	/* patternview */
	psy_dictionary_set(dict, "patternview.line", "Line");
	psy_dictionary_set(dict, "patternview.defaults", "Defaults");
	psy_dictionary_set(dict, "patternview.step", "Step");
	psy_dictionary_set(dict, "patternview.patname", "Pattern Name");
	psy_dictionary_set(dict, "patternview.nopattern", "No Pattern");
	psy_dictionary_set(dict, "patternview.length", "Length");
	psy_dictionary_set(dict, "patternview.apply", "Apply");
	psy_dictionary_set(dict, "patternview.tracker", "Tracker");
	psy_dictionary_set(dict, "patternview.roll", "Pianoroll");
	psy_dictionary_set(dict, "patternview.horz", "Horizontal");
	psy_dictionary_set(dict, "patternview.vert", "Vertical");
	psy_dictionary_set(dict, "patternview.split", "Split");
	psy_dictionary_set(dict, "patternview.properties", "Properties");
	psy_dictionary_set(dict, "patternview.beats", "Beats");
	psy_dictionary_set(dict, "patternview.keyboard", "Keyboard");
	psy_dictionary_set(dict, "patternview.showtracks", "Show Tracks");
	psy_dictionary_set(dict, "patternview.all", "All");
	psy_dictionary_set(dict, "patternview.current", "Current");
	psy_dictionary_set(dict, "patternview.active", "Active");
	/* transformpattern */
	psy_dictionary_set(dict, "transformpattern.searchpattern", "Search pattern");
	psy_dictionary_set(dict, "transformpattern.note", "Note");
	psy_dictionary_set(dict, "transformpattern.instr", "Instrum/Aux");
	psy_dictionary_set(dict, "transformpattern.mac", "Machine");
	psy_dictionary_set(dict, "transformpattern.replacepattern", "Replace pattern");
	psy_dictionary_set(dict, "transformpattern.search", "Search");	
	psy_dictionary_set(dict, "transformpattern.replaceall", "Replace all");
	psy_dictionary_set(dict, "transformpattern.cancel", "Cancel");	
	psy_dictionary_set(dict, "transformpattern.searchon", "Search on");
	psy_dictionary_set(dict, "transformpattern.entiresong", "Entire song");
	psy_dictionary_set(dict, "transformpattern.currentpattern", "Current pattern");
	psy_dictionary_set(dict, "transformpattern.currentselection", "Current selection");
	/* swingfill */
	psy_dictionary_set(dict, "swingfill.tempo", "Tempo(BPM)");
	psy_dictionary_set(dict, "swingfill.bpm", "BPM");
	psy_dictionary_set(dict, "swingfill.center", "Center");
	psy_dictionary_set(dict, "swingfill.actual", "Actual");
	psy_dictionary_set(dict, "swingfill.cycle", "Cycle Length(lines)");
	psy_dictionary_set(dict, "swingfill.variance", "Variance(%)");
	psy_dictionary_set(dict, "swingfill.phase", "Phase (degrees)");	
	psy_dictionary_set(dict, "swingfill.cancel", "Cancel");
	psy_dictionary_set(dict, "swingfill.apply", "Apply");
	/* seqview */
	psy_dictionary_set(dict, "seqview.follow-song", "Follow Song");
	psy_dictionary_set(dict, "seqview.show-playlist", "Show Playlist");
	psy_dictionary_set(dict, "seqview.show-pattern-names", "Show Pattern Names");
	psy_dictionary_set(dict, "seqview.record-noteoff", "Record note-off");
	psy_dictionary_set(dict, "seqview.record-tweak", "Record tweak");
	psy_dictionary_set(dict, "seqview.allow-notes-to_effect", "Allow notes to effect");
	psy_dictionary_set(dict, "seqview.multichannel-audition", "Multitrack playback on keypress");
	psy_dictionary_set(dict, "seqview.duration", "Duration");
	psy_dictionary_set(dict, "seqview.more", "Show More");
	psy_dictionary_set(dict, "seqview.less", "Show Less");
	psy_dictionary_set(dict, "seqview.ins", "Ins");
	psy_dictionary_set(dict, "seqview.new", "New");
	psy_dictionary_set(dict, "seqview.clone", "Clone");
	psy_dictionary_set(dict, "seqview.del", "Delete");
	psy_dictionary_set(dict, "seqview.new-trk", "+ New Track");
	psy_dictionary_set(dict, "seqview.del-trk", "Del Track");
	psy_dictionary_set(dict, "seqview.clear", "Clear");
	psy_dictionary_set(dict, "seqview.rename", "Rename");
	psy_dictionary_set(dict, "seqview.copy", "Copy");
	psy_dictionary_set(dict, "seqview.paste", "Paste");
	psy_dictionary_set(dict, "seqview.singlesel", "SingleSel");
	psy_dictionary_set(dict, "seqview.multisel", "MultiSel");	
	psy_dictionary_set(dict, "seqview.showseqeditor", "Show SequenceEditor");
	psy_dictionary_set(dict, "seqview.hideseqeditor", "Hide SequenceEditor");
	psy_dictionary_set(dict, "seqview.showstepsequencer", "Show Stepsequencer");
	psy_dictionary_set(dict, "seqview.hidestepsequencer", "Hide Stepsequencer");
	/* seqedit */
	psy_dictionary_set(dict, "seqedit.track", "Track");
	psy_dictionary_set(dict, "seqedit.move", "Move");
	psy_dictionary_set(dict, "seqedit.reorder", "Reorder");
	psy_dictionary_set(dict, "seqedit.insert", "Insert");
	psy_dictionary_set(dict, "seqedit.timesigs", "Timesigs");
	psy_dictionary_set(dict, "seqedit.repetitions", "Repetitions");
	psy_dictionary_set(dict, "seqedit.noitem", "No Item Selected");
	psy_dictionary_set(dict, "seqedit.patternitem", "Pattern Item");
	psy_dictionary_set(dict, "seqedit.sampleitem", "Sample Item");
	psy_dictionary_set(dict, "seqedit.markeritem", "Marker Item");
	psy_dictionary_set(dict, "seqedit.timesigitem", "TimeSig Item");
	psy_dictionary_set(dict, "seqedit.loopitem", "Loop Item");
	psy_dictionary_set(dict, "seqedit.position", "Position");
	psy_dictionary_set(dict, "seqedit.length", "Length");
	psy_dictionary_set(dict, "seqedit.end", "End");
	psy_dictionary_set(dict, "seqedit.name", "Name");
	psy_dictionary_set(dict, "seqedit.numerator", "Numerator");
	psy_dictionary_set(dict, "seqedit.denominator", "Denominator");
	/* newmachine */
	psy_dictionary_set(dict, "newmachine.all", "All");
	psy_dictionary_set(dict, "newmachine.favorites", "Favorites");
	psy_dictionary_set(dict, "newmachine.jme-version-unknown",
		"Load new gamefx and blitz if version is unknown");
	psy_dictionary_set(dict, "newmachine.song-loading-compatibility", "Song loading compatibility");
	psy_dictionary_set(dict, "newmachine.rescan", "Rescan");
	psy_dictionary_set(dict, "newmachine.in", "in");
	psy_dictionary_set(dict, "newmachine.plugin-directories", "Plugin directories");
	psy_dictionary_set(dict, "newmachine.filter", "Filter by");
	psy_dictionary_set(dict, "newmachine.sort", "Sort by");
	psy_dictionary_set(dict, "newmachine.favorite", "Favorite");
	psy_dictionary_set(dict, "newmachine.favorites", "Favorites");
	psy_dictionary_set(dict, "newmachine.name", "Name");
	psy_dictionary_set(dict, "newmachine.type", "Type");
	psy_dictionary_set(dict, "newmachine.mode", "Mode");
	psy_dictionary_set(dict, "newmachine.sections", "Sections");
	psy_dictionary_set(dict, "newmachine.section", "Section");
	psy_dictionary_set(dict, "newmachine.mksection", "Mksection");
	psy_dictionary_set(dict, "newmachine.remove", "Remove");
	psy_dictionary_set(dict, "newmachine.clear", "Clear");
	psy_dictionary_set(dict, "newmachine.copy", "Copy");
	psy_dictionary_set(dict, "newmachine.delete", "Delete");
	psy_dictionary_set(dict, "newmachine.select-plugin-to-view-description",
		"Select a plugin to view its description");
	psy_dictionary_set(dict, "newmachine.select-first-section", "Select/Create first a section");
	psy_dictionary_set(dict, "newmachine.select-first-plugin", "Select first a plugin");
	psy_dictionary_set(dict, "newmachine.search-plugin", "Search Plugin");
	psy_dictionary_set(dict, "newmachine.category", "Category");
	psy_dictionary_set(dict, "newmachine.apiversion", "API Version");
	psy_dictionary_set(dict, "newmachine.version", "Version");
	psy_dictionary_set(dict, "newmachine.dllname", "DllName");
	psy_dictionary_set(dict, "newmachine.anycategory", "Any Category");
	psy_dictionary_set(dict, "newmachine.scanning", "Scanning");
	psy_dictionary_set(dict, "newmachine.stop", "Stop");
	psy_dictionary_set(dict, "newmachine.stopping", "Stopping");
	/* samplesview */	
	psy_dictionary_set(dict, "samplesview.samplename", "Samplename");
	psy_dictionary_set(dict, "samplesview.samplerate", "Samplerate");
	psy_dictionary_set(dict, "samplesview.samples", "Samples");
	psy_dictionary_set(dict, "samplesview.groupsamples", "Group Samples");
	psy_dictionary_set(dict, "samplesview.groupsfirstsample", "Groups first sample");
	psy_dictionary_set(dict, "samplesview.default-volume", "Default volume");
	psy_dictionary_set(dict, "samplesview.global-volume", "Global volume");
	psy_dictionary_set(dict, "samplesview.pan-position", "Pan Position");
	psy_dictionary_set(dict, "samplesview.sampled-note", "Sampled note");
	psy_dictionary_set(dict, "samplesview.pitch-finetune", "Pitch finetune");
	psy_dictionary_set(dict, "samplesview.process", "Process");
	psy_dictionary_set(dict, "samplesview.no-wave-loaded", "No wave loaded");
	psy_dictionary_set(dict, "samplesview.cont-loop", "Continuous Loop");
	psy_dictionary_set(dict, "samplesview.disabled", "Disabled");
	psy_dictionary_set(dict, "samplesview.forward", "Forward");
	psy_dictionary_set(dict, "samplesview.bidirection", "Bidirection");
	psy_dictionary_set(dict, "samplesview.start", "Start");
	psy_dictionary_set(dict, "samplesview.end", "End");
	psy_dictionary_set(dict, "samplesview.sustain-loop", "Sustain Loop");
	psy_dictionary_set(dict, "samplesview.play", "Play");
	psy_dictionary_set(dict, "samplesview.stop", "Stop");
	psy_dictionary_set(dict, "samplesview.select-together", "Select Channels Together");
	psy_dictionary_set(dict, "samplesview.selstart", "Selection Start");
	psy_dictionary_set(dict, "samplesview.selend", "Selection End");
	psy_dictionary_set(dict, "samplesview.visual", "Visual");
	psy_dictionary_set(dict, "samplesview.doublecont", "Double Cont Loop");
	psy_dictionary_set(dict, "samplesview.doublesus", "Double Sus Loop");
	psy_dictionary_set(dict, "samplesview.lines", "Lines");
	/* cmds */	
	psy_dictionary_set(dict, "cmds.keymap", "Keymap");	
	psy_dictionary_set(dict, "cmds.help", "Help");
	psy_dictionary_set(dict, "cmds.helpshortcut", "Kbd Help");
	psy_dictionary_set(dict, "cmds.editmachine", "Machines");
	psy_dictionary_set(dict, "cmds.editpattern", "Patterns");
	psy_dictionary_set(dict, "cmds.addmachine", "Add Machine");
	psy_dictionary_set(dict, "cmds.playsong", "Play Song");
	psy_dictionary_set(dict, "cmds.playstart", "Play Start");
	psy_dictionary_set(dict, "cmds.playfrompos", "Play Pos");
	psy_dictionary_set(dict, "cmds.playstop", "stop");
	psy_dictionary_set(dict, "cmds.songposdec", "Seqpos dec");
	psy_dictionary_set(dict, "cmds.songposinc", "Seqpos Inc");
	psy_dictionary_set(dict, "cmds.maxpattern", "Max Pattern");
	psy_dictionary_set(dict, "cmds.infomachine", "gear");
	psy_dictionary_set(dict, "cmds.editinstr", "Instruments");
	psy_dictionary_set(dict, "cmds.editsample", "Samples");
	psy_dictionary_set(dict, "cmds.editwave", "Wave Edit");
	psy_dictionary_set(dict, "cmds.terminal", "Terminal");
	psy_dictionary_set(dict, "cmds.instrdec", "Current Instrument -1");
	psy_dictionary_set(dict, "cmds.instrinc", "Current Instrument +1");

	psy_dictionary_set(dict, "cmds.navup", "Nav Up");
	psy_dictionary_set(dict, "cmds.navdown", "Nav Down");
	psy_dictionary_set(dict, "cmds.navleft", "Nav Left");
	psy_dictionary_set(dict, "cmds.navright", "Nav Right");
	psy_dictionary_set(dict, "cmds.navpageup", "Nav Up 16");
	psy_dictionary_set(dict, "cmds.navpagedown", "Nav Down 16");
	psy_dictionary_set(dict, "cmds.navtop", "Nav Top");
	psy_dictionary_set(dict, "cmds.navbottom", "Nav Bottom");
	psy_dictionary_set(dict, "cmds.columnprev", "Prev column");
	psy_dictionary_set(dict, "cmds.columnnext", "Next column");

	psy_dictionary_set(dict, "cmds.rowinsert", "Insert row");
	psy_dictionary_set(dict, "cmds.rowdelete", "Delete row");
	psy_dictionary_set(dict, "cmds.rowclear", "Clear row");

	psy_dictionary_set(dict, "cmds.blockstart", "Block Start");
	psy_dictionary_set(dict, "cmds.blockend", "Block End");
	psy_dictionary_set(dict, "cmds.blockunmark", "Block Unmark");
	psy_dictionary_set(dict, "cmds.blockcut", "Block cut");
	psy_dictionary_set(dict, "cmds.blockcopy", "Block copy");
	psy_dictionary_set(dict, "cmds.blockpaste", "Block paste");
	psy_dictionary_set(dict, "cmds.blockmix", "Block Mix");

	psy_dictionary_set(dict, "cmds.transposeblockinc", "Transpose Block +1");
	psy_dictionary_set(dict, "cmds.transposeblockdec", "Transpose Block -1");
	psy_dictionary_set(dict, "cmds.transposeblockinc12", "Transpose Block +12");
	psy_dictionary_set(dict, "cmds.transposeblockdec12", "Transpose Block -12");

	psy_dictionary_set(dict, "cmds.selectall", "Block Select All");
	psy_dictionary_set(dict, "cmds.selectcol", "Block Select Column");
	psy_dictionary_set(dict, "cmds.selectbar", "Block Select Bar");

	psy_dictionary_set(dict, "cmds.selectmachine",
		"Select Mac/Ins in Cursor Pos");
	psy_dictionary_set(dict, "undo", "Edit Undo");
	psy_dictionary_set(dict, "redo", "Edit Redo");
	psy_dictionary_set(dict, "follow_song", "Follow Song");
	/* msg */
	psy_dictionary_set(dict, "msg.psyreq", "Psycle Request");
	psy_dictionary_set(dict, "msg.seqclear", "Sequence Clear Request! Do you really want clear the sequenceand pattern data?");
	psy_dictionary_set(dict, "msg.yes", "Yes");
	psy_dictionary_set(dict, "msg.no", "No");
	psy_dictionary_set(dict, "msg.cont", "Continue");
	psy_dictionary_set(dict, "msg.psyexit", "Exit Psycle Request, but your Song is not saved!");
	psy_dictionary_set(dict, "msg.saveexit", "Save and Exit");
	psy_dictionary_set(dict, "msg.nosaveexit", "Exit (no save)");
	psy_dictionary_set(dict, "msg.newsong", "New Song Request, but your Song is not saved!");
	psy_dictionary_set(dict, "msg.savenew", "Save and Create New Song");
	psy_dictionary_set(dict, "msg.nosavenew", "Create New Song (no save)");
	psy_dictionary_set(dict, "msg.loadsong", "New Load Request, but your Song is not saved!");
	psy_dictionary_set(dict, "msg.saveload", "Save and Load Song");
	psy_dictionary_set(dict, "msg.nosaveload", "Load Song (no save)");
	psy_dictionary_set(dict, "msg.audiostarting", "Audio starting");
	psy_dictionary_set(dict, "msg.audiostarted", "Audio started");
}

#ifdef PSYCLE_DEFAULT_LANG
static int onenumproperties(void* context, psy_Dictionary* property, uintptr_t level);

void save_translator_default(void)
{
	psy_Dictionary* lang;
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
	psy_Dictionary* lang;
	char path[4096];

	strcpy(path, psy_dir_config());
	strcat(path, "\\lang.ini");

	lang = psy_property_allocinit_key(NULL);
	psy_property_set_comment(lang,
		"Psycle Template Dictionary created by\r\n; " PSYCLE__BUILD__IDENTIFIER("\r\n; "));
	make_translator_default(lang);
	psy_property_enumerate((psy_Dictionary*)lang, NULL,
		(psy_PropertyCallback)onenumproperties);
	propertiesio_save(lang, path);
	psy_property_deallocate(lang);
}

int onenumproperties(void* context, psy_Dictionary* property, uintptr_t level)
{
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_STRING) {
		psy_property_set_item_str(property, "");
	}
	return 1;
}

#endif

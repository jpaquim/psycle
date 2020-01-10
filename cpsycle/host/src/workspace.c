// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "workspace.h"
#include "cmdproperties.h"
#include "skinio.h"
#include <exclusivelock.h>
#include <stdlib.h>
#include <string.h>
#include <songio.h>
#include <portable.h>
#include <operations.h>

#define PSYCLE_SONGS_DEFAULT_DIR "C:\\Programme\\Psycle\\Songs"
#define PSYCLE_SAMPLES_DEFAULT_DIR "C:\\Programme\\Psycle\\Samples"
#define PSYCLE_PLUGINS_DEFAULT_DIR "C:\\Programme\\Psycle\\PsyclePlugins"
#define PSYCLE_LUASCRIPTS_DEFAULT_DIR "C:\\Programme\\Psycle\\LuaScripts"
#define PSYCLE_VSTS32_DEFAULT_DIR "C:\\Programme\\Psycle\\VstPlugins"
#define PSYCLE_VSTS64_DEFAULT_DIR "C:\\Programme\\Psycle\\Vst64Plugins"
#define PSYCLE_SKINS_DEFAULT_DIR "C:\\Programme\\Psycle\\Skins"
#define PSYCLE_DOC_DEFAULT_DIR "C:\\Programme\\Psycle\\Docs"

static void workspace_initplayer(Workspace*);
static void workspace_initplugincatcherandmachinefactory(Workspace*);
static void workspace_initsignals(Workspace*);
static void workspace_disposesignals(Workspace*);
static void workspace_makeconfig(Workspace*);
static void workspace_makegeneral(Workspace*);
static void workspace_makenotes(Workspace*);
static void workspace_makevisual(Workspace*);
static void workspace_makepatternview(Workspace*, psy_Properties*);
static void workspace_makemachineview(Workspace* self, psy_Properties*);
static void workspace_makekeyboard(Workspace*);
static void workspace_makedirectories(Workspace*);
static void workspace_makenotes(Workspace*);
static void workspace_makeinputoutput(Workspace*);
static void workspace_makemidi(Workspace*);
static void workspace_makelang(Workspace*);
static void workspace_makelangen(Workspace*);
static void workspace_makelanges(Workspace*);
static void applysongproperties(Workspace*);
static psy_Properties* workspace_makeproperties(Workspace*);
static void workspace_makedriverlist(Workspace*);
static void workspace_makedriverconfigurations(Workspace*);
static void workspace_driverconfig(Workspace*);
static void workspace_mididriverconfig(Workspace*, int deviceid);
static void workspace_updatemididriverlist(Workspace*);
static const char* workspace_driverpath(Workspace*);
static const char* workspace_driverkey(Workspace*);
static const char* workspace_eventdriverpath(Workspace*);
static void workspace_configaudio(Workspace*);
static void workspace_configvisual(Workspace*);
static void workspace_configkeyboard(Workspace*);
static void workspace_setsong(Workspace*, psy_audio_Song*, int flag);
static void workspace_onloadprogress(Workspace*, psy_audio_Song*, int progress);
static void workspace_onscanprogress(Workspace*, psy_audio_PluginCatcher*, int progress);
static void workspace_onsequenceeditpositionchanged(Workspace*, SequenceSelection*);
static void workspace_updatenavigation(Workspace*);
/// Machinecallback
static MachineCallback machinecallback(Workspace*);
static unsigned int machinecallback_samplerate(Workspace*);
static int machinecallback_bpm(Workspace*);
static psy_dsp_beat_t machinecallback_beatspersample(Workspace*);
static psy_dsp_beat_t machinecallback_currbeatsperline(Workspace*);
static psy_audio_Samples* machinecallback_samples(Workspace*);
static psy_audio_Machines* machinecallback_machines(Workspace*);
static psy_audio_Instruments* machinecallback_instruments(Workspace*);
static void machinecallback_fileselect_load(Workspace*);
static void machinecallback_fileselect_save(Workspace*);
static void machinecallback_fileselect_directory(Workspace*);
static void machinecallback_output(Workspace*, const char* text);
/// terminal
static void workspace_onterminalwarning(Workspace*,
	psy_audio_SongFile* sender, const char* text);
static void workspace_onterminaloutput(Workspace*,
	psy_audio_SongFile* sender, const char* text);
static void workspace_onterminalerror(Workspace*,
	psy_audio_SongFile* sender, const char* text);

void history_init(History* self)
{
	self->container = 0;
	self->prevented = 0;
}

void history_dispose(History* self)
{
	psy_List* p;

	for (p = self->container; p != 0; p = p->next) {
		free(p->entry);
	}
	psy_list_free(self->container);
	self->container = 0;
	self->prevented = 0;
}

void history_clear(History* self)
{
	history_dispose(self);
}

void history_add(History* self, int viewid, int sequenceentryid)
{
	HistoryEntry* entry;

	entry = (HistoryEntry*) malloc(sizeof(HistoryEntry));	
	entry->viewid = viewid;
	entry->sequenceentryid = sequenceentryid;
	psy_list_append(&self->container, entry);
}

void workspace_init(Workspace* self, void* handle)
{	
	lock_init();
#ifdef SSE
	psy_dsp_sse2_init(&dsp);
#else
	psy_dsp_noopt_init(&dsp);
#endif
	self->octave = 4;	
	self->cursorstep = 1;
	self->followsong = 0;
	self->recordtweaks = 0;
	self->inputoutput = 0;
	self->midi = 0;
	self->mainhandle = handle;
	self->filename = strdup("Untitled.psy");
	self->lastentry = 0;
	self->maximizeview.maximized = 0;
	self->maximizeview.row0 = 1;
	self->maximizeview.row1 = 1;
	self->maximizeview.row2 = 1;
	self->currnavigation = 0;
	self->currview = 0;
	history_init(&self->history);
	workspace_makeconfig(self);	
	workspace_initplugincatcherandmachinefactory(self);
	self->song = song_allocinit(&self->machinefactory);
	self->songcbk = self->song;
	self->properties = workspace_makeproperties(self);
	sequenceselection_init(&self->sequenceselection, &self->song->sequence);
	sequence_setplayselection(&self->song->sequence, &self->sequenceselection);
	psy_signal_connect(&self->sequenceselection.signal_editpositionchanged, self,
		workspace_onsequenceeditpositionchanged);
	undoredo_init(&self->undoredo);	
	self->navigating = 0;
	workspace_initsignals(self);	
	workspace_initplayer(self);		
	self->patterneditposition.pattern = 0;	
	self->patterneditposition.column = 0;
	self->patterneditposition.digit = 0;
	self->patterneditposition.line = 0;
	self->patterneditposition.offset = 0;	
	self->patterneditposition.track = 0;
	pattern_init(&self->patternpaste);
	self->sequencepaste = 0;	
}

void workspace_initplugincatcherandmachinefactory(Workspace* self)
{
	plugincatcher_init(&self->plugincatcher, self->directories);
	psy_signal_connect(&self->plugincatcher.signal_scanprogress, self,
		workspace_onscanprogress);
	self->hasplugincache = plugincatcher_load(&self->plugincatcher);
	machinefactory_init(&self->machinefactory, machinecallback(self), 
		&self->plugincatcher);
}

void workspace_initsignals(Workspace* self)
{
	psy_signal_init(&self->signal_octavechanged);
	psy_signal_init(&self->signal_songchanged);
	psy_signal_init(&self->signal_configchanged);
	psy_signal_init(&self->signal_skinchanged);
	psy_signal_init(&self->signal_patterneditpositionchanged);
	psy_signal_init(&self->signal_sequenceselectionchanged);
	psy_signal_init(&self->signal_loadprogress);
	psy_signal_init(&self->signal_scanprogress);
	psy_signal_init(&self->signal_beforesavesong);
	psy_signal_init(&self->signal_showparameters);
	psy_signal_init(&self->signal_viewselected);
	psy_signal_init(&self->signal_parametertweak);
	psy_signal_init(&self->signal_terminal_error);
	psy_signal_init(&self->signal_terminal_out);
	psy_signal_init(&self->signal_terminal_warning);
}

void workspace_dispose(Workspace* self)
{	
	player_dispose(&self->player);	
	song_free(self->song);	
	self->song = 0;	
	self->songcbk = 0;
	properties_free(self->config);
	self->config = 0;
	properties_free(self->properties);
	self->properties = 0;
	properties_free(self->lang);
	self->lang = 0;
	free(self->filename);
	self->filename = 0;
	plugincatcher_dispose(&self->plugincatcher);
	machinefactory_dispose(&self->machinefactory);
	undoredo_dispose(&self->undoredo);
	history_dispose(&self->history);
	workspace_disposesignals(self);
	pattern_dispose(&self->patternpaste);	
	workspace_disposesequencepaste(self);
	properties_free(self->cmds);
	lock_dispose();
}

void workspace_disposesignals(Workspace* self)
{
	psy_signal_dispose(&self->signal_octavechanged);
	psy_signal_dispose(&self->signal_songchanged);
	psy_signal_dispose(&self->signal_configchanged);
	psy_signal_dispose(&self->signal_skinchanged);
	psy_signal_dispose(&self->signal_patterneditpositionchanged);
	psy_signal_dispose(&self->signal_sequenceselectionchanged);
	psy_signal_dispose(&self->signal_loadprogress);
	psy_signal_dispose(&self->signal_scanprogress);
	psy_signal_dispose(&self->signal_beforesavesong);
	psy_signal_dispose(&self->signal_showparameters);
	psy_signal_dispose(&self->signal_viewselected);
	psy_signal_dispose(&self->signal_parametertweak);
	psy_signal_dispose(&self->signal_terminal_error);
	psy_signal_dispose(&self->signal_terminal_out);
	psy_signal_dispose(&self->signal_terminal_warning);
}

void workspace_disposesequencepaste(Workspace* self)
{
	psy_List* p;

	for (p = self->sequencepaste; p != 0; p = p->next) {
		SequenceEntry* entry;		

		entry = (SequenceEntry*) p->entry;
		free(entry);
	}
	psy_list_free(self->sequencepaste);
	self->sequencepaste = 0;
}

void workspace_initplayer(Workspace* self)
{
	player_init(&self->player, self->song, (void*)
		ui_component_platformhandle(self->mainhandle));
	self->cmds = cmdproperties_create();
	eventdrivers_setcmds(&self->player.eventdrivers, self->cmds);
	workspace_driverconfig(self);
	workspace_updatemididriverlist(self);
	workspace_mididriverconfig(self, 0);
}

psy_Properties* workspace_driverconfiguration(Workspace* self)
{		
	return psy_properties_find(self->driverconfigurations,
		workspace_driverkey(self));	
}
	
void workspace_configaudio(Workspace* self)
{			
	player_loaddriver(&self->player, workspace_driverpath(self),
		workspace_driverconfiguration(self));	
	workspace_driverconfig(self);
}

void workspace_configvisual(Workspace* self)
{	
	psy_Properties* visual;

	visual = psy_properties_find(self->config, "visual");
	if (visual) {				
		workspace_changedefaultfontsize(self, 
			psy_properties_int(visual, "defaultfontsize", 80));
	}
}

void workspace_configkeyboard(Workspace* self)
{			
}

const char* workspace_driverpath(Workspace* self)
{
	psy_Properties* p;
	const char* rv = 0;

	if (p = psy_properties_read(self->inputoutput, "driver")) {	
		if (p = psy_properties_read_choice(p)) {		
			rv = psy_properties_valuestring(p);
		}
	}
	return rv;
}

const char* workspace_driverkey(Workspace* self)
{
	psy_Properties* p;
	const char* rv = 0;

	if (p = psy_properties_read(self->inputoutput, "driver")) {	
		if (p = psy_properties_read_choice(p)) {		
			rv = psy_properties_key(p);
		}
	}
	return rv;
}

const char* workspace_eventdriverpath(Workspace* self)
{
	psy_Properties* p;
	const char* rv = 0;

	p = psy_properties_read(self->midi, "driver");
	if (p) {
		int choice;		
		int count;
		
		choice = psy_properties_value(p);
		p = p->children;
		count = 0;
		while (p) {
			if (count == choice) {
				rv = psy_properties_valuestring(p);
				break;
			}
			p = psy_properties_next(p);
			++count;
		}
	}
	return rv;
}

void workspace_driverconfig(Workspace* self)
{		
	self->driverconfigure->item.disposechildren = 0;
	self->driverconfigure->children = self->player.driver->properties->children;	
}

void workspace_mididriverconfig(Workspace* self, int driverid)
{	
	EventDriver* eventdriver;
	
	eventdriver = player_eventdriver(&self->player, driverid);
	if (eventdriver) {	
		self->midiconfigure->item.disposechildren = 0;
		self->midiconfigure->children = eventdriver->properties ?
			eventdriver->properties->children : 0;
	} else {
		self->midiconfigure->children = 0;	
	}
}

void workspace_scanplugins(Workspace* self)
{		
	plugincatcher_scan(&self->plugincatcher);	
	plugincatcher_save(&self->plugincatcher);
}

void workspace_onscanprogress(Workspace* self, psy_audio_PluginCatcher* sender, int progress)
{
	psy_signal_emit(&self->signal_scanprogress, self, 1, progress);
}

void workspace_makeconfig(Workspace* self)
{	
	workspace_makelang(self);
	workspace_makegeneral(self);
	workspace_makevisual(self);	
	workspace_makekeyboard(self);
	workspace_makedirectories(self);
	workspace_makeinputoutput(self);
	workspace_makemidi(self);	
}

void workspace_makegeneral(Workspace* self)
{	
	psy_Properties* p;

	self->config = psy_properties_create();	
	self->general = psy_properties_create_section(self->config, "general");
	psy_properties_settext(self->general, "General");
	p = psy_properties_append_string(self->general, "version", "alpha");	
	psy_properties_settext(p, "Version");	
	p->item.hint = PSY_PROPERTY_HINT_HIDE;
	p = psy_properties_append_bool(self->general, "showaboutatstart", 1);
	psy_properties_settext(p, "Show About at Startup");
	p = psy_properties_append_bool(self->general, "showsonginfoonload", 1);
	psy_properties_settext(p, "Show song info on Load");
	p = psy_properties_append_bool(self->general, "showmaximizedatstart", 1);
	psy_properties_settext(p, "Show Maximized at Startup");
	p = psy_properties_append_bool(self->general, "showplaylisteditor", 0);
	psy_properties_settext(p, "Show Playlist Editor");
	p = psy_properties_append_bool(self->general, "showstepsequencer", 1);
	psy_properties_settext(p, "Show Sequencestepbar");
}

void workspace_makevisual(Workspace* self)
{
	psy_Properties* visual;
	
	visual = psy_properties_create_section(self->config, "visual");	
	psy_properties_settext(visual, "Visual");
	psy_properties_settext(
		psy_properties_append_action(visual, "loadskin"),
		"Load Skin");
	psy_properties_settext(
		psy_properties_append_int(visual, "defaultfontsize", 80, 0, 999),
		"Default font size");	
	workspace_makepatternview(self, visual);
	workspace_makemachineview(self, visual);
}

void workspace_makepatternview(Workspace* self, psy_Properties* visual)
{
	psy_Properties* pvc;	
	
	pvc = psy_properties_create_section(visual, "patternview");
	psy_properties_settext(pvc, "Pattern View");
	psy_properties_settext(
		psy_properties_append_bool(pvc, "drawemptydata", 0),
		"Draw empty data");
	psy_properties_settext(
		psy_properties_append_int(pvc, "fontsize", 80, 0, 999),
		"Font Size");
	psy_properties_settext(
		psy_properties_append_bool(pvc, "griddefaults", 1),
		"Default entries");
	psy_properties_settext(
		psy_properties_append_bool(pvc, "linenumbers", 1),
		"Line numbers");
	psy_properties_settext(
		psy_properties_append_bool(pvc, "beatoffset", 1),
		"Beat offset");
	psy_properties_settext(
		psy_properties_append_bool(pvc, "linenumberscursor", 1),
		"Line numbers cursor");
	psy_properties_settext(
		psy_properties_append_bool(pvc, "linenumbersinhex", 1),
		"Line numbers in HEX");
	psy_properties_settext(
		psy_properties_append_bool(pvc, "wideinstcolumn", 0),
		"Wide instrument column");
	psy_properties_settext(
		psy_properties_append_bool(pvc, "trackscopes", 1),
		"Pattern track scopes");
	psy_properties_settext(
		psy_properties_append_bool(pvc, "wraparound", 1),
		"Wrap Around");
	psy_properties_settext(
		psy_properties_append_bool(pvc, "centercursoronscreen", 1),
		"Center cursor on screen");
	psy_properties_settext(
		psy_properties_append_int(pvc, "beatsperbar", 4, 1, 16),
		"Bar highlighting: (beats/bar)");
	psy_properties_settext(
		psy_properties_append_bool(pvc, "notetab", 1),
		"A4 is 440Hz (Otherwise it is 220Hz)");	
}

void workspace_makemachineview(Workspace* self, psy_Properties* visual)
{
	psy_Properties* mvc;
	
	mvc = psy_properties_settext(
		psy_properties_create_section(visual, "machineview"),
		"Machine View");
	psy_properties_settext(
		psy_properties_append_bool(mvc, "drawmachineindexes", 1),
		"Draw psy_audio_Machine Indexes");
	psy_properties_settext(
		psy_properties_append_bool(mvc, "drawvumeters", 1),
		"Draw VU Meters");
}

void workspace_makekeyboard(Workspace* self)
{	
	self->keyboard = psy_properties_settext(
		psy_properties_create_section(self->config, "keyboard"),
		"Keyboard and Misc.");

	psy_properties_settext(
		psy_properties_append_bool(self->keyboard, 
		"recordtweaksastws", 0),
		"Record Mouse Tweaks as tws (Smooth tweaks)");	
	psy_properties_settext(
		psy_properties_append_bool(self->keyboard, 
		"advancelineonrecordtweak", 0),
		"Advance Line On Record");
}

void workspace_makedirectories(Workspace* self)
{	
	self->directories = psy_properties_settext(
		psy_properties_create_section(self->config, "directories"),
		"Directories");
	psy_properties_sethint(psy_properties_settext(
		psy_properties_append_string(
			self->directories,
			"songs",
			PSYCLE_SONGS_DEFAULT_DIR),
		"Song directory"),
		PSY_PROPERTY_HINT_EDITDIR);
	psy_properties_sethint(psy_properties_settext(
		psy_properties_append_string(
			self->directories,
			"samples",
			PSYCLE_SAMPLES_DEFAULT_DIR),
		"Samples directory"),
		PSY_PROPERTY_HINT_EDITDIR);
	psy_properties_sethint(psy_properties_settext(
		psy_properties_append_string(
			self->directories,
			"plugins",
			PSYCLE_PLUGINS_DEFAULT_DIR),
		"Plug-in directory"),
		PSY_PROPERTY_HINT_EDITDIR);
	psy_properties_sethint(psy_properties_settext(
		psy_properties_append_string(
			self->directories,
			"luascripts",
			PSYCLE_LUASCRIPTS_DEFAULT_DIR),
		"Lua scripts directory"),
		PSY_PROPERTY_HINT_EDITDIR);
	psy_properties_sethint(psy_properties_settext(
		psy_properties_append_string(
			self->directories,
			"vsts32",			
			PSYCLE_VSTS32_DEFAULT_DIR),
		"VST directories"),
		PSY_PROPERTY_HINT_EDITDIR);
	psy_properties_sethint(psy_properties_settext(
		psy_properties_append_string(
			self->directories,
			"vsts64",			
			PSYCLE_VSTS64_DEFAULT_DIR),
		"VST directories"),
		PSY_PROPERTY_HINT_EDITDIR);
	psy_properties_sethint(psy_properties_settext(
		psy_properties_append_string(
			self->directories,
			"skin",			
			"C:\\Programme\\Psycle\\Skins"),
		"Skin directory"),
		PSY_PROPERTY_HINT_EDITDIR);
}

void workspace_makeinputoutput(Workspace* self)
{		
	self->inputoutput = psy_properties_create_section(self->config, "inputoutput");
	workspace_makedriverlist(self);
	self->driverconfigure = psy_properties_settext(
		psy_properties_create_section(self->inputoutput, "configure"),
		"Configure");
	self->driverconfigure->item.save = 0;
	workspace_makedriverconfigurations(self);
}

void workspace_makedriverlist(Workspace* self)
{
	psy_Properties* drivers;

	psy_properties_settext(self->inputoutput, "Input/Output");
	// change number to set startup driver, if no psycle.ini found
	drivers = psy_properties_append_choice(self->inputoutput, "driver", 2); 
	psy_properties_settext(drivers, "Driver");
	psy_properties_append_string(drivers, "silent", "silentdriver");
#if defined(_DEBUG)
	psy_properties_append_string(drivers, "mme", "..\\driver\\mme\\Debug\\mme.dll");
	psy_properties_append_string(drivers, "directx", "..\\driver\\directx\\Debug\\directx.dll");
#else
	psy_properties_append_string(drivers, "mme", "..\\driver\\mme\\Release\\mme.dll");
	psy_properties_append_string(drivers, "directx", "..\\driver\\directx\\Release\\directx.dll");	
#endif
}

void workspace_makedriverconfigurations(Workspace* self)
{
	psy_Properties* drivers;
	psy_Properties* p;	
	
	self->driverconfigurations = psy_properties_sethint(
		psy_properties_create_section(self->inputoutput, "configurations"),
		PSY_PROPERTY_HINT_HIDE);
	
	drivers = psy_properties_read(self->inputoutput, "driver");
	if (drivers) {
		for (p = drivers->children; p != 0; p = psy_properties_next(p)) {			
			if (psy_properties_type(p) == PSY_PROPERTY_TYP_STRING) {
				const char* path;

				path = psy_properties_valuestring(p);
				if (path) {
					Library library;

					library_init(&library);
					library_load(&library, path);
					if (library.module) {
						pfndriver_create fpdrivercreate;

						fpdrivercreate = (pfndriver_create)
						library_functionpointer(&library, "driver_create");
						if (fpdrivercreate) {
							Driver* driver;							

							driver = fpdrivercreate();
							if (driver && driver->properties) {
								psy_Properties* driverconfig;
								psy_Properties* driversection;
								
								driverconfig = psy_properties_clone(driver->properties);
								driversection = psy_properties_create_section(
									self->driverconfigurations,
									psy_properties_key(p));	
								if (driverconfig) {									
									driversection->children = driverconfig->children;
									driverconfig->parent = driversection;
								}
								driver->dispose(driver);
							}
						}
					}
					library_dispose(&library);
				}
			}
		}
	}
}

void workspace_makemidi(Workspace* self)
{		
	psy_Properties* installed;
	self->midi = psy_properties_settext(
		psy_properties_create_section(self->config, "midicontrollers"),
		"MIDI Controllers");		
		

	psy_properties_settext(self->midi, "Event Input");
	// change number to set startup driver, if no psycle.ini found
	installed = psy_properties_append_choice(self->midi, "installeddriver", 0);
	psy_properties_settext(installed, "Input Drivers");
	psy_properties_append_string(installed, "kbd", "kbd");	
#if defined(_DEBUG)
	psy_properties_append_string(installed, "mmemidi", "..\\driver\\mmemidi\\Debug\\mmemidi.dll");	
#else
	psy_properties_append_string(installed, "mmemidi", "..\\driver\\mmemidi\\Release\\mmemidi.dll");
#endif
	psy_properties_settext(
		psy_properties_append_action(self->midi, "addeventdriver"),
		"Add to active drivers");
	psy_properties_settext(
		psy_properties_append_choice(self->midi, "mididriver", 0),
		"Active Drivers");		
	psy_properties_settext(
		psy_properties_append_action(self->midi, "removeeventdriver"),
			"Remove active driver");
	self->midiconfigure = psy_properties_settext(
		psy_properties_create_section(self->midi, "configure"),
		"Configure");
}

void workspace_updatemididriverlist(Workspace* self)
{
	int numdrivers;
	int i;
	psy_Properties* drivers;	
	
	drivers = psy_properties_read(self->midi, "mididriver");
	if (drivers) {
		psy_properties_clear(drivers);	
		numdrivers = player_numeventdrivers(&self->player);
		for (i = 0; i < numdrivers; ++i) {
			char idstr[40];
			EventDriver* eventdriver;

			eventdriver = player_eventdriver(&self->player, i);
			if (eventdriver) {
				const char* text;
				
				psy_snprintf(idstr, 40, "dev%d", i);
				text = psy_properties_readstring(eventdriver->properties, "name", idstr);
				psy_properties_settext(psy_properties_append_string(drivers, idstr, text), text);
			}
		}
	}
}

void workspace_makelang(Workspace* self)
{
	self->lang = psy_properties_create();
	workspace_makelangen(self);
//	workspace_makelanges(self);
}

void workspace_makelangen(Workspace* self)
{
	psy_properties_append_string(self->lang, "load", "Load");
	psy_properties_append_string(self->lang, "save", "Save");
	psy_properties_append_string(self->lang, "new", "New");
	psy_properties_append_string(self->lang, "undo", "Undo");
	psy_properties_append_string(self->lang, "redo", "Redo");
	psy_properties_append_string(self->lang, "play", "Play");
	psy_properties_append_string(self->lang, "sel", "Sel");
	psy_properties_append_string(self->lang, "stop", "Stop");
}

void workspace_makelanges(Workspace* self)
{
	psy_properties_write_string(self->lang, "load", "Cargar");
	psy_properties_write_string(self->lang, "save", "Guardar");
	psy_properties_write_string(self->lang, "new", "Nuevo");
	psy_properties_write_string(self->lang, "undo", "Deshacer");
	psy_properties_write_string(self->lang, "redo", "Rehacer");
	psy_properties_write_string(self->lang, "play", "Toca");
	psy_properties_write_string(self->lang, "stop", "Para");
}

const char* workspace_translate(Workspace* self, const char* key) {
	return psy_properties_readstring(self->lang, key, key);	
}

void workspace_configchanged(Workspace* self, psy_Properties* property, 
	psy_Properties* choice)
{
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_ACTION) {
		if (strcmp(psy_properties_key(property), "loadskin") == 0) {
			char path[MAX_PATH]	 = "";
			char title[MAX_PATH] = ""; 					
			static char filter[] = "Psycle Display psy_audio_Presets\0*.psv\0";
			char  defaultextension[] = "PSV";			
			
			if (ui_openfile(self->mainhandle, title, filter, defaultextension,
					workspace_skins_directory(self), path)) {
				workspace_loadskin(self, path);						
			}
		} else
		if (strcmp(psy_properties_key(property), "addeventdriver") == 0) {
			psy_Properties* p;

			p = psy_properties_read(self->midi, "installeddriver");
			if (p) {
				int id;
				int c = 0;
				
				id = psy_properties_value(p);				
				for (p = p->children; p != 0  && c != id; p = p->next, ++c);
				if (p) {
					player_loadeventdriver(&self->player,					
						psy_properties_valuestring(p));
					workspace_updatemididriverlist(self);
					p = psy_properties_read(self->midi, "mididriver");
					if (p) {
						p->item.value.i = player_numeventdrivers(&self->player) - 1;
						workspace_mididriverconfig(self, psy_properties_value(p));
					}
				}
			}
		} else
		if (strcmp(psy_properties_key(property), "removeeventdriver") == 0) {
			psy_Properties* p;

			p = psy_properties_read(self->midi, "mididriver");
			if (p) {
				int id;				
				
				id = psy_properties_value(p);
				player_removeeventdriver(&self->player, id);
				workspace_updatemididriverlist(self);
				--(p->item.value.i);
				if (p->item.value.i < 0) {
					p->item.value.i = 0;
				}
				workspace_mididriverconfig(self, psy_properties_value(p));	
			}
		}
	} else
	if (psy_properties_insection(property, self->driverconfigure)) {		
		psy_Properties* driversection;

		player_restartdriver(&self->player, 0);		
		driversection = psy_properties_find(self->driverconfigurations, 
			workspace_driverkey(self));
		if (driversection) {
			if (driversection->children) {
				psy_Properties* driverconfig;
				properties_free(driversection->children);

				driverconfig = 
					psy_properties_clone(self->player.driver->properties);
				if (driverconfig) {
					driversection->children = driverconfig->children;					
					if (driversection->children) {
						driversection->children->parent =
							driversection->children;
					}
				}
			}
		}		
	} else	
	if (choice && psy_properties_insection(property, self->midi)) {
		if (strcmp(psy_properties_key(choice), "mididriver") == 0) {
			workspace_mididriverconfig(self, psy_properties_value(choice));	
		} else {		
			psy_Properties* p;

			p = psy_properties_find(self->midi, "mididriver");
			if (p) {
				player_restarteventdriver(&self->player, psy_properties_value(p));
			}
		}
	} else
	if (psy_properties_insection(property, self->midi)) {
		psy_Properties* p;

		p = psy_properties_read(self->midi, "mididriver");
		if (p) {
			player_restarteventdriver(&self->player, psy_properties_value(p));
		}		
	} else
	if (choice && (strcmp(psy_properties_key(choice), "driver") == 0)) {
		player_reloaddriver(&self->player, psy_properties_valuestring(property),
			workspace_driverconfiguration(self));
		workspace_driverconfig(self);
	} else
	if (strcmp(psy_properties_key(property), "defaultfontsize") == 0) {
		workspace_changedefaultfontsize(self, psy_properties_value(property));
	} else
	if ((psy_properties_hint(property) == PSY_PROPERTY_HINT_INPUT)) {		
		workspace_configkeyboard(self);		
	} else
	if (strcmp(psy_properties_key(property), "drawvumeters") == 0) {
		if (psy_properties_value(property)) {
			player_setvumetermode(&self->player, VUMETER_RMS);
		} else {
			player_setvumetermode(&self->player, VUMETER_NONE);
		}
	}
	psy_signal_emit(&self->signal_configchanged, self, 1, property);
}

int workspace_showsonginfoonload(Workspace* self)
{				
	return psy_properties_bool(self->config, "general.showsonginfoonload", 1);
}

int workspace_showaboutatstart(Workspace* self)
{	
	return psy_properties_bool(self->config, "general.showaboutatstart", 1);	
}

int workspace_showmaximizedatstart(Workspace* self)
{
	return psy_properties_bool(self->config, "general.showmaximizedatstart", 1);	
}

int workspace_showplaylisteditor(Workspace* self)
{	
	return psy_properties_bool(self->config, "general.showplaylisteditor", 0);
}

int workspace_showstepsequencer(Workspace* self)
{	
	return psy_properties_bool(self->config, "general.showstepsequencer", 0);
}

int workspace_showgriddefaults(Workspace* self)
{	
	return psy_properties_bool(self->config, "visual.patternview.griddefaults", 1);	
}

int workspace_showlinenumbers(Workspace* self)
{	
	return psy_properties_bool(self->config, "visual.patternview.linenumbers", 1);	
}

int workspace_showbeatoffset(Workspace* self)
{	
	return psy_properties_bool(self->config, "visual.patternview.beatoffset", 1);	
}

int workspace_showlinenumbercursor(Workspace* self)
{	
	return psy_properties_bool(self->config, "visual.patternview.linenumberscursor", 1);
}

int workspace_showlinenumbersinhex(Workspace* self)
{
	return psy_properties_bool(self->config, "visual.patternview.linenumbersinhex", 1);
}

int workspace_showwideinstcolumn(Workspace* self)
{	
	return psy_properties_bool(self->config, "visual.patternview.wideinstcolumn", 1);
}

int workspace_showtrackscopes(Workspace* self)
{
	return psy_properties_bool(self->config, "visual.patternview.trackscopes", 1);
}

int workspace_wraparound(Workspace* self)
{
	return psy_properties_bool(self->config, "visual.patternview.wraparound", 1);
}

int workspace_showmachineindexes(Workspace* self)
{
	return psy_properties_bool(self->config, "visual.machineview.drawmachineindexes", 1);
}

void workspace_newsong(Workspace* self)
{			
	psy_audio_Song* song;	
	
	song = song_allocinit(&self->machinefactory);
	properties_free(self->properties);
	self->properties = workspace_makeproperties(self);
	free(self->filename);
	self->filename = strdup("Untitled.psy");
	workspace_setsong(self, song, WORKSPACE_NEWSONG);		
}

void workspace_loadsong(Workspace* self, const char* path)
{	
	psy_audio_Song* song;
	psy_audio_SongFile songfile;	
		
	song = song_allocinit(&self->machinefactory);
	if (song) {		
		psy_signal_connect(&song->signal_loadprogress, self,
			workspace_onloadprogress);	
		psy_signal_emit(&song->signal_loadprogress, self, 1, -1);
		psy_audio_songfile_init(&songfile);
		psy_signal_connect(&songfile.signal_warning, self,
			workspace_onterminalwarning);		
		psy_signal_connect(&songfile.signal_output, self,
			workspace_onterminaloutput);
		songfile.song = song;
		songfile.file = 0;		
		lock_enter();
		self->songcbk = song;
		if (psy_audio_songfile_load(&songfile, path) != PSY_OK) {
			self->songcbk = self->song;
			lock_leave();			
			song_free(song);
			psy_signal_emit(&self->signal_terminal_error, self, 1,
				songfile.serr);
			psy_audio_songfile_dispose(&songfile);
		} else {
			properties_free(self->properties);
			self->properties = songfile.workspaceproperties;
			psy_audio_songfile_dispose(&songfile);
			free(self->filename);
			self->filename = strdup(path);
			workspace_setsong(self, song, WORKSPACE_LOADSONG);
			psy_audio_songfile_dispose(&songfile);
			lock_leave();	
		}
		psy_signal_emit(&self->signal_terminal_out, self, 1,
			"ready\n");
	}
}

void workspace_onloadprogress(Workspace* self, psy_audio_Song* sender, int progress)
{
	psy_signal_emit(&self->signal_loadprogress, self, 1, progress);
}

void workspace_setsong(Workspace* self, psy_audio_Song* song, int flag)
{
	psy_audio_Song* oldsong;

	history_clear(&self->history);
	oldsong = self->song;
	player_stop(&self->player);
	lock_enter();	
	self->song = song;
	self->songcbk = song;
	player_setsong(&self->player, self->song);
	applysongproperties(self);
	sequenceselection_setsequence(&self->sequenceselection
		,&self->song->sequence);
	workspace_addhistory(self);
	psy_signal_emit(&self->signal_songchanged, self, 1, flag);
	self->lastentry = 0;
	workspace_disposesequencepaste(self);
	lock_leave();
	song_free(oldsong);	
}

void workspace_savesong(Workspace* self, const char* path)
{
	psy_audio_SongFile songfile;
	psy_audio_songfile_init(&songfile);
	songfile.file = 0;
	songfile.song = self->song;
	psy_signal_emit(&self->signal_beforesavesong, self, 0);
	songfile.workspaceproperties = self->properties;
	if (psy_audio_songfile_save(&songfile, path)) {
		psy_signal_emit(&self->signal_terminal_error, self, 1,
			songfile.serr);
	}
	psy_audio_songfile_dispose(&songfile);
	psy_signal_emit(&self->signal_terminal_out, self, 1,
				"ready\n");
}

void workspace_loadskin(Workspace* self, const char* path)
{
	psy_Properties* properties;
		
	properties = psy_properties_create();		
	skin_load(properties, path);
	psy_signal_emit(&self->signal_skinchanged, self, 1, properties);
	properties_free(properties);
}

psy_Properties* workspace_makeproperties(Workspace* self)
{
	psy_Properties* root;
	psy_Properties* machines;	
	psy_Properties* machine;
	
	root = psy_properties_create();
	machines = psy_properties_create_section(root, "machines");	
	machine = psy_properties_create_section(machines, "machine");	
	psy_properties_append_int(machine, "index", MASTER_INDEX, 0, 0);		
	psy_properties_append_int(machine, "x", 320, 0, 0);
	psy_properties_append_int(machine, "y", 200, 0, 0);
	return root;
}

void applysongproperties(Workspace* self)
{	
	if (self->song) {	
		player_setbpm(&self->player, self->song->properties.bpm);	
		player_setlpb(&self->player, self->song->properties.lpb);
	}
}

psy_Properties* workspace_pluginlist(Workspace* self)
{
	return self->plugincatcher.plugins;
}

void workspace_load_configuration(Workspace* self)
{		
	propertiesio_load(self->config, "psycle.ini", 0);	
	workspace_configaudio(self);	
	eventdrivers_restartall(&self->player.eventdrivers);
	workspace_updatemididriverlist(self);
	workspace_configvisual(self);
	workspace_configkeyboard(self);
	psy_signal_emit(&self->signal_configchanged, self, 1,self->config);
}

void workspace_save_configuration(Workspace* self)
{
	propertiesio_save(self->config, "psycle.ini");	
}

void workspace_setoctave(Workspace* self, int octave)
{
	self->octave = octave;
	psy_signal_emit(&self->signal_octavechanged, self, 1, octave);
}

int workspace_octave(Workspace* self)
{
	return self->octave;
}

void workspace_undo(Workspace* self)
{
	undoredo_undo(&self->undoredo);
}

void workspace_redo(Workspace* self)
{
	undoredo_redo(&self->undoredo);
}

void workspace_changedefaultfontsize(Workspace* self, int size)
{
	ui_fontinfo fontinfo;
	ui_font font;

	ui_fontinfo_init(&fontinfo, "Tahoma", size);
	ui_font_init(&font, &fontinfo);
	ui_replacedefaultfont(self->mainhandle, &font);
	ui_component_invalidate(self->mainhandle);
}

void workspace_setpatterneditposition(Workspace* self, PatternEditPosition editposition)
{	
	self->patterneditposition = editposition;
	self->patterneditposition.line = 
		(int) (editposition.offset * player_lpb(&self->player));
	psy_signal_emit(&self->signal_patterneditpositionchanged, self, 0);
}

PatternEditPosition workspace_patterneditposition(Workspace* self)
{
	return self->patterneditposition;
}

void workspace_setsequenceselection(Workspace* self,
		SequenceSelection selection)
{	
	self->sequenceselection = selection;	
	sequence_setplayselection(&self->song->sequence, &selection);
	psy_signal_emit(&self->signal_sequenceselectionchanged, self, 0);
	workspace_addhistory(self);
}

void workspace_addhistory(Workspace* self)
{
	if (!self->navigating && !self->history.prevented) {
		int sequencentryid = -1;

		if (self->sequenceselection.editposition.trackposition.tracknode) {
				SequenceEntry* entry;			
				entry = (SequenceEntry*)
					self->sequenceselection.editposition.trackposition.tracknode->entry;
				sequencentryid = entry->id;
		}
		history_add(&self->history, self->currview, sequencentryid);
		self->currnavigation = self->history.container->tail;
	}
}

SequenceSelection workspace_sequenceselection(Workspace* self)
{
	return self->sequenceselection;
}

void workspace_setcursorstep(Workspace* self, int step)
{
	self->cursorstep = step;
}

int workspace_cursorstep(Workspace* self)
{
	return self->cursorstep;
}

int workspace_hasplugincache(Workspace* self)
{
	return self->hasplugincache;
}

EventDriver* workspace_kbddriver(Workspace* self)
{
	return player_kbddriver(&self->player);
}

int workspace_followingsong(Workspace* self)
{
	return self->followsong;
}

void workspace_followsong(Workspace* self)
{
	self->followsong = 1;
}

void workspace_stopfollowsong(Workspace* self)
{
	self->followsong = 0;
}

void workspace_onsequenceeditpositionchanged(Workspace* self,
	SequenceSelection* selection)
{
	PatternEditPosition position;
	SequenceEntry* entry;

	if (selection->editposition.trackposition.tracknode) {
		entry = (SequenceEntry*) selection->editposition.trackposition.tracknode->entry;
		position.pattern = entry->pattern;
		position.column = 0;
		position.digit = 0;
		position.line = 0;
		position.offset = 0;		
		position.track = 0;
		workspace_setpatterneditposition(self, position);		
	}
}

void workspace_idle(Workspace* self)
{
	if (self->followsong) {
		SequenceTrackIterator it;
		
		if (player_playing(&self->player)) {
			it = sequence_begin(&self->song->sequence, 
				self->song->sequence.tracks,
				player_position(&self->player));
			if (it.tracknode && self->lastentry != it.tracknode->entry) {
				sequenceselection_seteditposition(&self->sequenceselection, 
						sequence_makeposition(&self->song->sequence,
							self->song->sequence.tracks, it.tracknode));
				self->history.prevented = 1;
				workspace_setsequenceselection(self, self->sequenceselection);
				self->history.prevented = 0;
				self->lastentry = (SequenceEntry*) it.tracknode->entry;		
			}
			if (self->lastentry) {				
				self->patterneditposition.line = (int) (
					(player_position(&self->player) -
					self->lastentry->offset) * player_lpb(&self->player));				
				self->patterneditposition.offset = 
					player_position(&self->player) - self->lastentry->offset;
				self->patterneditposition.offset = 
					self->patterneditposition.line / 
					(psy_dsp_beat_t) player_lpb(&self->player);
				workspace_setpatterneditposition(self, 
					self->patterneditposition);				
			}
		} else
		if (self->lastentry) {				
			self->patterneditposition.line = (int) (
				(player_position(&self->player) -
				self->lastentry->offset) * player_lpb(&self->player));				
			self->patterneditposition.offset = 
				player_position(&self->player) - self->lastentry->offset;
			self->patterneditposition.offset = 
				self->patterneditposition.line / 
				(psy_dsp_beat_t) player_lpb(&self->player);
			workspace_setpatterneditposition(self, 
				self->patterneditposition);
			self->lastentry = 0;
		}		
	}
}

void workspace_showparameters(Workspace* self, uintptr_t machineslot)
{
	psy_signal_emit(&self->signal_showparameters, self, 1, machineslot);
}

void workspace_selectview(Workspace* self, int view)
{
	self->currview = view;
	psy_signal_emit(&self->signal_viewselected, self, 1, view);
}

void workspace_parametertweak(Workspace* self, int slot, uintptr_t tweak, int value)
{
	psy_signal_emit(&self->signal_parametertweak, self, 3, slot, tweak, 
		value);
}

void workspace_recordtweaks(Workspace* self)
{
	self->recordtweaks = 1;
}

void workspace_stoprecordtweaks(Workspace* self)
{
	self->recordtweaks = 0;
}

int workspace_recordingtweaks(Workspace* self)
{
	return self->recordtweaks;
}

int workspace_recordtweaksastws(Workspace* self)
{
	return psy_properties_bool(self->keyboard, 
		"recordtweaksastws", 0);
}

int workspace_advancelineonrecordtweak(Workspace* self)
{
	return psy_properties_bool(self->keyboard, 
		"advancelineonrecordtweak", 0);
}

void workspace_onviewchanged(Workspace* self, int view)
{
	self->currview = view;
	workspace_addhistory(self);
}

void workspace_back(Workspace* self)
{
	if (self->currnavigation->prev) {		
		self->currnavigation = self->currnavigation->prev;
		workspace_updatenavigation(self);
	}
}

void workspace_forward(Workspace* self)
{
	if (self->currnavigation->next) {						
		self->currnavigation = self->currnavigation->next;	
		workspace_updatenavigation(self);
	}
}

void workspace_updatenavigation(Workspace* self)
{
	HistoryEntry* entry;

	entry = (HistoryEntry*) (self->currnavigation->entry);
	self->navigating = 1;
	if (self->currview != entry->viewid) {
		workspace_selectview(self, entry->viewid);
	}
	if (entry->sequenceentryid != -1 &&
			self->sequenceselection.editposition.trackposition.tracknode) {
		SequencePosition position;

		position = sequence_positionfromid(&self->song->sequence,
			entry->sequenceentryid);
		sequenceselection_seteditposition(&self->sequenceselection, position);
		workspace_setsequenceselection(self, self->sequenceselection);
	}
	self->navigating = 0;
}

const char* workspace_songs_directory(Workspace* self)
{
	return psy_properties_readstring(self->directories, "songs",
		PSYCLE_SONGS_DEFAULT_DIR);
}

const char* workspace_samples_directory(Workspace* self)
{
	return psy_properties_readstring(self->directories, "samples",
		PSYCLE_SAMPLES_DEFAULT_DIR);
}

const char* workspace_plugins_directory(Workspace* self)
{
	return psy_properties_readstring(self->directories, "plugins",
		PSYCLE_PLUGINS_DEFAULT_DIR);
}

const char* workspace_lusscripts_directory(Workspace* self)
{
	return psy_properties_readstring(self->directories, "luascripts",
		PSYCLE_LUASCRIPTS_DEFAULT_DIR);
}

const char* workspace_vsts32_directory(Workspace* self)
{
	return psy_properties_readstring(self->directories, "vsts32",
		PSYCLE_VSTS32_DEFAULT_DIR);
}

const char* workspace_vsts64_directory(Workspace* self)
{
	return psy_properties_readstring(self->directories, "vsts64",
		PSYCLE_VSTS64_DEFAULT_DIR);
}

const char* workspace_skins_directory(Workspace* self)
{
	return psy_properties_readstring(self->directories, "skins",
		PSYCLE_SKINS_DEFAULT_DIR);
}

const char* workspace_doc_directory(Workspace* self)
{
	return psy_properties_readstring(self->directories, "skins",
		PSYCLE_DOC_DEFAULT_DIR);
}

MachineCallback machinecallback(Workspace* self)
{
	MachineCallback rv;	

	rv.context = self;
	rv.samples = machinecallback_samples;
	rv.samplerate = machinecallback_samplerate;
	rv.bpm = machinecallback_bpm;
	rv.beatspersample = machinecallback_beatspersample;
	rv.currbeatsperline = machinecallback_currbeatsperline;
	rv.machines = machinecallback_machines;
	rv.instruments = machinecallback_instruments;
	rv.fileselect_load = machinecallback_fileselect_load;
	rv.fileselect_save = machinecallback_fileselect_save;
	rv.fileselect_directory = machinecallback_fileselect_directory;
	rv.output = machinecallback_output;
	return rv;
}

void machinecallback_fileselect_load(Workspace* self)
{
	char path[MAX_PATH]	 = "";
	char title[MAX_PATH] = ""; 					
	static char filter[] = "Psycle Display psy_audio_Presets\0*.psv\0";
	char  defaultextension[] = "PSV";			
			
	if (ui_openfile(self->mainhandle, title, filter, defaultextension,
			workspace_skins_directory(self), path)) {		
	}	
}

void machinecallback_fileselect_save(Workspace* self)
{
	char path[MAX_PATH]	 = "";
	char title[MAX_PATH] = ""; 					
	static char filter[] = "Psycle Display psy_audio_Presets\0*.psv\0";
	char  defaultextension[] = "PSV";			
			
	if (ui_savefile(self->mainhandle, title, filter, defaultextension,
			workspace_skins_directory(self), path)) {		
	}	
}

void machinecallback_fileselect_directory(Workspace* self)
{

}

psy_audio_Samples* machinecallback_samples(Workspace* self)
{
	return &self->songcbk->samples;
}

unsigned int machinecallback_samplerate(Workspace* self)
{
	return self->player.driver->samplerate(self->player.driver);
}

int machinecallback_bpm(Workspace* self)
{
	return (int) player_bpm(&self->player);
}

psy_dsp_beat_t machinecallback_beatspersample(Workspace* self)
{
	return sequencer_beatspersample(&self->player.sequencer);
}

psy_dsp_beat_t machinecallback_currbeatsperline(Workspace* self)
{
	return sequencer_currbeatsperline(&self->player.sequencer);
}

psy_audio_Machines* machinecallback_machines(Workspace* self)
{
	return self->songcbk ? &self->song->machines : 0;
}

psy_audio_Instruments* machinecallback_instruments(Workspace* self)
{
	return self->songcbk ? &self->song->instruments : 0;
}

void workspace_onterminalwarning(Workspace* self, psy_audio_SongFile* sender,
	const char* text)	
{
	psy_signal_emit(&self->signal_terminal_warning, self, 1, text);
}

void workspace_onterminalerror(Workspace* self, psy_audio_SongFile* sender,
	const char* text)	
{
	psy_signal_emit(&self->signal_terminal_error, self, 1, text);
}

void workspace_onterminaloutput(Workspace* self, psy_audio_SongFile* sender,
	const char* text)	
{
	psy_signal_emit(&self->signal_terminal_out, self, 1, text);
}

void machinecallback_output(Workspace* self, const char* text)
{
	psy_signal_emit(&self->signal_terminal_out, self, 1, text);
}
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "workspace.h"
#include <exclusivelock.h>
#include <stdlib.h>
#include <string.h>
#include <songio.h>

static void workspace_initplayer(Workspace*);
static void workspace_initplugincatcherandmachinefactory(Workspace*);
static void workspace_initsignals(Workspace*);
static void workspace_disposesignals(Workspace*);
static void workspace_makeconfig(Workspace*);
static void workspace_makegeneral(Workspace*);
static void workspace_makenotes(Workspace*);
static void workspace_makevisual(Workspace*);
static void workspace_makepatternview(Workspace*, Properties*);
static void workspace_makemachineview(Workspace* self, Properties*);
static void workspace_makekeyboard(Workspace*);
static void workspace_makedirectories(Workspace*);
static void workspace_makenotes(Workspace*);
static void workspace_makeinputoutput(Workspace*);
static void workspace_makemidi(Workspace*);
static void workspace_makelang(Workspace*);
static void workspace_makelangen(Workspace*);
static void workspace_makelanges(Workspace*);
static void applysongproperties(Workspace*);
static Properties* workspace_makeproperties(Workspace*);
static void workspace_setdriverlist(Workspace*);
static void workspace_driverconfig(Workspace*);
static void workspace_mididriverconfig(Workspace*, int deviceid);
static void workspace_updatemididriverlist(Workspace*);
static const char* workspace_driverpath(Workspace*);
static const char* workspace_eventdriverpath(Workspace*);
static void workspace_configaudio(Workspace*);
static void workspace_configvisual(Workspace*);
static void workspace_configkeyboard(Workspace*);
static void workspace_setsong(Workspace*, Song*, int flag);
static void workspace_changedefaultfontsize(Workspace*, int size);
static void workspace_onloadprogress(Workspace*, Song*, int progress);
static void workspace_onscanprogress(Workspace*, PluginCatcher*, int progress);

static MachineCallback machinecallback(Workspace*);
static unsigned int machinecallback_samplerate(Workspace*);
static int machinecallback_bpm(Workspace*);
static Samples* machinecallback_samples(Workspace*);
static Machines* machinecallback_machines(Workspace*);
static Instruments* machinecallback_instruments(Workspace*);

void workspace_init(Workspace* self, void* handle)
{	
	lock_init();
	self->octave = 4;	
	self->cursorstep = 1;
	self->followsong = 0;
	self->inputoutput = 0;
	self->midi = 0;
	self->mainhandle = handle;
	self->filename = strdup("Untitled.psy");
	workspace_makeconfig(self);
	workspace_initplugincatcherandmachinefactory(self);
	self->song = song_allocinit(&self->machinefactory);
	self->properties = workspace_makeproperties(self);	
	undoredo_init(&self->undoredo);
	workspace_initsignals(self);
	workspace_initplayer(self);
}

void workspace_initplugincatcherandmachinefactory(Workspace* self)
{
	plugincatcher_init(&self->plugincatcher, self->directories);
	signal_connect(&self->plugincatcher.signal_scanprogress, self,
		workspace_onscanprogress);
	self->hasplugincache = plugincatcher_load(&self->plugincatcher);
	machinefactory_init(&self->machinefactory, machinecallback(self), 
		&self->plugincatcher);
}

void workspace_initsignals(Workspace* self)
{
	signal_init(&self->signal_octavechanged);
	signal_init(&self->signal_songchanged);
	signal_init(&self->signal_configchanged);
	signal_init(&self->signal_editpositionchanged);
	signal_init(&self->signal_loadprogress);
	signal_init(&self->signal_scanprogress);
	signal_init(&self->signal_beforesavesong);
}

void workspace_dispose(Workspace* self)
{	
	player_dispose(&self->player);	
	song_free(self->song);	
	self->song = 0;	
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
	workspace_disposesignals(self);
	lock_dispose();
}

void workspace_disposesignals(Workspace* self)
{
	signal_dispose(&self->signal_octavechanged);
	signal_dispose(&self->signal_songchanged);
	signal_dispose(&self->signal_configchanged);
	signal_dispose(&self->signal_editpositionchanged);
	signal_dispose(&self->signal_loadprogress);
	signal_dispose(&self->signal_scanprogress);
	signal_dispose(&self->signal_beforesavesong);
}

void workspace_initplayer(Workspace* self)
{
	player_init(&self->player, self->song, (void*)self->mainhandle->hwnd);
	workspace_driverconfig(self);
	workspace_updatemididriverlist(self);
	workspace_mididriverconfig(self, 0);
}
	
void workspace_configaudio(Workspace* self)
{			
	player_loaddriver(&self->player, workspace_driverpath(self));
	workspace_driverconfig(self);	
}

void workspace_configvisual(Workspace* self)
{	
	Properties* visual;

	visual = properties_find(self->config, "visual");
	if (visual) {				
		workspace_changedefaultfontsize(self, 
			properties_int(visual, "defaultfontsize", 80));		
	}
}

void workspace_configkeyboard(Workspace* self)
{			
}

const char* workspace_driverpath(Workspace* self)
{
	Properties* p;
	const char* rv = 0;

	p = properties_read(self->inputoutput, "driver");
	if (p) {
		int choice;		
		int count;
		
		choice = properties_value(p);
		p = p->children;
		count = 0;
		while (p) {
			if (count == choice) {
				rv = properties_valuestring(p);
				break;
			}
			p = properties_next(p);
			++count;
		}
	}
	return rv;
}

const char* workspace_eventdriverpath(Workspace* self)
{
	Properties* p;
	const char* rv = 0;

	p = properties_read(self->midi, "driver");
	if (p) {
		int choice;		
		int count;
		
		choice = properties_value(p);
		p = p->children;
		count = 0;
		while (p) {
			if (count == choice) {
				rv = properties_valuestring(p);
				break;
			}
			p = properties_next(p);
			++count;
		}
	}
	return rv;
}

void workspace_driverconfig(Workspace* self)
{		
	self->driverconfigure->item.disposechildren = 0;
	self->driverconfigure->children = self->player.driver->properties;	
}

void workspace_mididriverconfig(Workspace* self, int driverid)
{	
	EventDriver* eventdriver;
	
	eventdriver = player_eventdriver(&self->player, driverid);
	if (eventdriver) {	
		self->midiconfigure->item.disposechildren = 0;
		self->midiconfigure->children = eventdriver->properties;	
	} else {
		self->midiconfigure->children = 0;	
	}
}

void workspace_scanplugins(Workspace* self)
{		
	plugincatcher_scan(&self->plugincatcher);	
	plugincatcher_save(&self->plugincatcher);
}

void workspace_onscanprogress(Workspace* self, PluginCatcher* sender, int progress)
{
	signal_emit(&self->signal_scanprogress, self, 1, progress);
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
	Properties* general;
	Properties* p;

	self->config = properties_create();	
	general = properties_createsection(self->config, "general");
	properties_settext(general, "General");
	p = properties_append_string(general, "version", "alpha");	
	properties_settext(p, "Version");	
	p->item.hint = PROPERTY_HINT_HIDE;
	p = properties_append_bool(general, "showaboutatstart", 1);
	properties_settext(p, "Show About at Startup");
	p = properties_append_bool(general, "showsonginfoonload", 1);
	properties_settext(p, "Show song info on Load");
	p = properties_append_bool(general, "showmaximizedatstart", 1);
	properties_settext(p, "Show Maximized at Startup");
}

void workspace_makevisual(Workspace* self)
{
	Properties* visual;
	Properties* p;
	// Visual
	visual = properties_createsection(self->config, "visual");	
	properties_settext(visual, "Visual");
	p = properties_append_int(visual, "defaultfontsize", 80, 0, 999);
	properties_settext(p, "Default font size");
	workspace_makepatternview(self, visual);
	workspace_makemachineview(self, visual);
}

void workspace_makepatternview(Workspace* self, Properties* visual)
{
	Properties* pvc;	
	
	pvc = properties_createsection(visual, "patternview");
	properties_settext(pvc, "Pattern View");
	properties_settext(
		properties_append_bool(pvc, "drawemptydata", 0),
		"Draw empty data");
	properties_settext(
		properties_append_int(pvc, "fontsize", 80, 0, 999),
		"Font Size");
	properties_settext(
		properties_append_bool(pvc, "linenumbers", 1),
		"Line numbers");
	properties_settext(
		properties_append_bool(pvc, "linenumberscursor", 1),
		"Line numbers cursor");
	properties_settext(
		properties_append_bool(pvc, "linenumbersinhex", 1),
		"Line numbers in HEX");
	properties_settext(
		properties_append_bool(pvc, "centercursoronscreen", 0),
		"Center cursor on screen");
	properties_settext(
		properties_append_int(pvc, "beatsperbar", 4, 1, 16),
		"Bar highlighting: (beats/bar)");
	properties_settext(
		properties_append_bool(pvc, "notetab", 1),
		"A4 is 440Hz (Otherwise it is 220Hz)");	
}

void workspace_makemachineview(Workspace* self, Properties* visual)
{
	Properties* mvc;
	
	mvc = properties_settext(
		properties_createsection(visual, "machineview"),
		"Machine View");
	properties_settext(
		properties_append_bool(mvc, "drawvumeters", 1),
		"Draw VU Meters");
}

void workspace_makekeyboard(Workspace* self)
{	
	self->keyboard = properties_settext(
		properties_createsection(self->config, "keyboard"),
		"Keyboard and Misc.");
}

void workspace_makedirectories(Workspace* self)
{	
	self->directories = properties_settext(
		properties_createsection(self->config, "directories"),
		"Directories");
	properties_sethint(properties_settext(
		properties_append_string(
			self->directories,
			"song",
			"C:\\Programme\\Psycle\\Songs"),
		"Song directory"),
		PROPERTY_HINT_EDITDIR);
	properties_sethint(properties_settext(
		properties_append_string(
			self->directories,
			"plugin",
			"C:\\Programme\\Psycle\\PsyclePlugins"),
		"Plug-in directory"),
		PROPERTY_HINT_EDITDIR);
	properties_sethint(properties_settext(
		properties_append_string(
			self->directories,
			"vst",			
			"C:\\Programme\\Psycle\\VstPlugins"),
		"VST directories"),
		PROPERTY_HINT_EDITDIR);
}

void workspace_makeinputoutput(Workspace* self)
{		
	self->inputoutput = properties_createsection(self->config, "inputoutput");
	workspace_setdriverlist(self);
	self->driverconfigure = properties_settext(
		properties_createsection(self->inputoutput, "configure"),
		"Configure");		
}

void workspace_makemidi(Workspace* self)
{		
	Properties* installed;
	self->midi = properties_settext(
		properties_createsection(self->config, "midicontrollers"),
		"MIDI Controllers");		
		

	properties_settext(self->midi, "Event Input");
	// change number to set startup driver, if no psycle.ini found
	installed = properties_append_choice(self->midi, "installeddriver", 0);
	properties_settext(installed, "Input Drivers");
	properties_append_string(installed, "kbd", "kbd");	
#if defined(_DEBUG)
	properties_append_string(installed, "mmemidi", "..\\driver\\mmemidi\\Debug\\mmemidi.dll");	
#else
	properties_append_string(installed, "mmemidi", "..\\driver\\mmemidi\\Release\\mmemidi.dll");
#endif
	properties_settext(
		properties_append_action(self->midi, "addeventdriver"),
		"Add to active drivers");
	properties_settext(
		properties_append_choice(self->midi, "mididriver", 0),
		"Active Drivers");		
	properties_settext(
		properties_append_action(self->midi, "removeeventdriver"),
			"Remove active driver");
	self->midiconfigure = properties_settext(
		properties_createsection(self->midi, "configure"),
		"Configure");
}

void workspace_setdriverlist(Workspace* self)
{
	Properties* drivers;

	properties_settext(self->inputoutput, "Input/Output");
	// change number to set startup driver, if no psycle.ini found
	drivers = properties_append_choice(self->inputoutput, "driver", 2); 
	properties_settext(drivers, "Driver");
	properties_append_string(drivers, "silent", "silentdriver");
#if defined(_DEBUG)
	properties_append_string(drivers, "mme", "..\\driver\\mme\\Debug\\mme.dll");
	properties_append_string(drivers, "directx", "..\\driver\\directx\\Debug\\directx.dll");
#else
	properties_append_string(drivers, "mme", "..\\driver\\mme\\Release\\mme.dll");
	properties_append_string(drivers, "directx", "..\\driver\\directx\\Release\\directx.dll");	
#endif
}

void workspace_updatemididriverlist(Workspace* self)
{
	int numdrivers;
	int i;
	Properties* drivers;	
	
	drivers = properties_read(self->midi, "mididriver");
	if (drivers) {
		properties_clear(drivers);	
		numdrivers = player_numeventdrivers(&self->player);
		for (i = 0; i < numdrivers; ++i) {
			char idstr[40];
			EventDriver* eventdriver;

			eventdriver = player_eventdriver(&self->player, i);
			if (eventdriver) {
				const char* text;
				
				_snprintf(idstr, 40, "dev%d", i);
				text = properties_readstring(eventdriver->properties, "name", idstr);
				properties_settext(properties_append_string(drivers, idstr, text), text);
			}
		}
	}
}

void workspace_makelang(Workspace* self)
{
	self->lang = properties_create();
	workspace_makelangen(self);
//	workspace_makelanges(self);
}

void workspace_makelangen(Workspace* self)
{
	properties_append_string(self->lang, "load", "Load");
	properties_append_string(self->lang, "save", "Save");
	properties_append_string(self->lang, "new", "New");
	properties_append_string(self->lang, "undo", "Undo");
	properties_append_string(self->lang, "redo", "Redo");
	properties_append_string(self->lang, "play", "Play");
	properties_append_string(self->lang, "stop", "Stop");
}

void workspace_makelanges(Workspace* self)
{
	properties_write_string(self->lang, "load", "Cargar");
	properties_write_string(self->lang, "save", "Guardar");
	properties_write_string(self->lang, "new", "Nuevo");
	properties_write_string(self->lang, "undo", "Deshacer");
	properties_write_string(self->lang, "redo", "Rehacer");
	properties_write_string(self->lang, "play", "Toca");
	properties_write_string(self->lang, "stop", "Para");
}

const char* workspace_translate(Workspace* self, const char* key) {
	return properties_readstring(self->lang, key, key);	
}

void workspace_configchanged(Workspace* self, Properties* property, 
	Properties* choice)
{
	if (properties_type(property) == PROPERTY_TYP_ACTION) {
		if (strcmp(properties_key(property), "addeventdriver") == 0) {
			Properties* p;

			p = properties_read(self->midi, "installeddriver");
			if (p) {
				int id;
				int c = 0;
				
				id = properties_value(p);				
				for (p = p->children; p != 0  && c != id; p = p->next, ++c);
				if (p) {
					player_loadeventdriver(&self->player,					
						properties_valuestring(p));
					workspace_updatemididriverlist(self);
					p = properties_read(self->midi, "mididriver");
					if (p) {
						p->item.value.i = player_numeventdrivers(&self->player) - 1;
						workspace_mididriverconfig(self, properties_value(p));
					}
				}
			}
		} else
		if (strcmp(properties_key(property), "removeeventdriver") == 0) {
			Properties* p;

			p = properties_read(self->midi, "mididriver");
			if (p) {
				int id;				
				
				id = properties_value(p);
				player_removeeventdriver(&self->player, id);
				workspace_updatemididriverlist(self);
				--(p->item.value.i);
				if (p->item.value.i < 0) {
					p->item.value.i = 0;
				}
				workspace_mididriverconfig(self, properties_value(p));	
			}
		}
	} else
	if (choice && properties_insection(property, self->midi)) {
		if (strcmp(properties_key(choice), "mididriver") == 0) {
			workspace_mididriverconfig(self, properties_value(choice));	
		} else {		
			Properties* p;

			p = properties_find(self->midi, "mididriver");
			if (p) {
				player_restarteventdriver(&self->player, properties_value(p));
			}
		}
	} else
	if (choice && (strcmp(properties_key(choice), "driver") == 0)) {
		player_reloaddriver(&self->player, properties_valuestring(property));		
		workspace_driverconfig(self);
	} else
	if (strcmp(properties_key(property), "defaultfontsize") == 0) {
		workspace_changedefaultfontsize(self, properties_value(property));
	} else
	if ((properties_hint(property) == PROPERTY_HINT_INPUT)) {		
		workspace_configkeyboard(self);		
	} else
	if (strcmp(properties_key(property), "drawvumeters") == 0) {
		if (properties_value(property)) {
			player_setvumetermode(&self->player, VUMETER_RMS);
		} else {
			player_setvumetermode(&self->player, VUMETER_NONE);
		}
	}
	signal_emit(&self->signal_configchanged, self, 1, property);
}

int workspace_showsonginfoonload(Workspace* self)
{				
	return properties_bool(self->config, "general.showsonginfoonload", 1);
}

int workspace_showaboutatstart(Workspace* self)
{	
	return properties_bool(self->config, "general.showaboutatstart", 1);	
}

int workspace_showmaximizedatstart(Workspace* self)
{
	return properties_bool(self->config, "general.showmaximizedatstart", 1);	
}

int workspace_showlinenumbers(Workspace* self)
{	
	return properties_bool(self->config, "visual.linenumbers", 1);	
}

void workspace_newsong(Workspace* self)
{			
	Song* song;	
	
	song = song_allocinit(&self->machinefactory);
	properties_free(self->properties);
	self->properties = workspace_makeproperties(self);
	free(self->filename);
	self->filename = strdup("Untitled.psy");
	workspace_setsong(self, song, WORKSPACE_NEWSONG);		
}

void workspace_loadsong(Workspace* self, const char* path)
{	
	Song* song;
	SongFile songfile;

	properties_free(self->properties);
	song = song_allocinit(&self->machinefactory);
	signal_connect(&song->signal_loadprogress, self, workspace_onloadprogress);
	songfile.song = song;
	songfile.file = 0;	
	songfile_load(&songfile, path);	
	self->properties = songfile.workspaceproperties;
	free(self->filename);
	self->filename = strdup(path);
	workspace_setsong(self, song, WORKSPACE_LOADSONG);
	
}

void workspace_onloadprogress(Workspace* self, Song* sender, int progress)
{
	signal_emit(&self->signal_loadprogress, self, 1, progress);
}

void workspace_setsong(Workspace* self, Song* song, int flag)
{
	Song* oldsong;

	oldsong = self->song;
	player_stop(&self->player);
	lock_enter();	
	self->song = song;
	player_setsong(&self->player, self->song);
	applysongproperties(self);
	signal_emit(&self->signal_songchanged, self, 1, flag);	
	lock_leave();
	song_free(oldsong);	
}

void workspace_savesong(Workspace* self, const char* path)
{
	SongFile songfile;
	songfile.file = 0;
	songfile.song = self->song;
	signal_emit(&self->signal_beforesavesong, self, 0);
	songfile.workspaceproperties = self->properties;
	songfile_save(&songfile, path);
}

Properties* workspace_makeproperties(Workspace* self)
{
	Properties* root;
	Properties* machines;	
	Properties* machine;
	
	root = properties_create();
	machines = properties_createsection(root, "machines");	
	machine = properties_createsection(machines, "machine");	
	properties_append_int(machine, "index", MASTER_INDEX, 0, 0);		
	properties_append_int(machine, "x", 320, 0, 0);
	properties_append_int(machine, "y", 200, 0, 0);
	return root;
}

void applysongproperties(Workspace* self)
{	
	double dTmp;

	properties_readdouble(self->song->properties, "bpm", &dTmp, 125.0);
	player_setbpm(&self->player, (float)dTmp);	
	player_setlpb(&self->player, properties_int(self->song->properties, 
		"lpb", 4));
}

Properties* workspace_pluginlist(Workspace* self)
{
	return self->plugincatcher.plugins;
}

void workspace_load_configuration(Workspace* self)
{	
	propertiesio_load(self->config, "psycle.ini", 0);	
	workspace_configaudio(self);
	workspace_updatemididriverlist(self);
	workspace_configvisual(self);
	workspace_configkeyboard(self);
	signal_emit(&self->signal_configchanged, self, 1,self->config);
}

void workspace_save_configuration(Workspace* self)
{
	propertiesio_save(self->config, "psycle.ini");	
}

void workspace_setoctave(Workspace* self, int octave)
{
	self->octave = octave;
	signal_emit(&self->signal_octavechanged, self, 1, octave);
}

int workspace_octave(Workspace* self)
{
	return self->octave;
}

void workspace_updatedriver(Workspace* self)
{
	player_restartdriver(&self->player);
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
	ui_invalidate(self->mainhandle);
}

void workspace_seteditposition(Workspace* self, EditPosition editposition)
{	
	self->editposition = editposition;
	self->editposition.line = 
		(int) (editposition.offset * player_lpb(&self->player));
	signal_emit(&self->signal_editpositionchanged, self, 0);
}

EditPosition workspace_editposition(Workspace* self)
{
	return self->editposition;
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

MachineCallback machinecallback(Workspace* self)
{
	MachineCallback rv;

	rv.context = self;
	rv.samples = machinecallback_samples;
	rv.samplerate = machinecallback_samplerate;
	rv.bpm = machinecallback_bpm;	
	rv.machines = machinecallback_machines;
	rv.instruments = machinecallback_instruments;
	return rv;
}

Samples* machinecallback_samples(Workspace* self)
{
	return &self->song->samples;
}

unsigned int machinecallback_samplerate(Workspace* self)
{
	return self->player.driver->samplerate(self->player.driver);
}

int machinecallback_bpm(Workspace* self)
{
	return (int) player_bpm(&self->player);
}

Machines* machinecallback_machines(Workspace* self)
{
	return self->song ? &self->song->machines : 0;
}

Instruments* machinecallback_instruments(Workspace* self)
{
	return self->song ? &self->song->instruments : 0;
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

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "workspace.h"
#include "cmdsnotes.h"
#include <stdlib.h>
#include <string.h>

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
static void workspace_makemidicontrollers(Workspace*);
static void workspace_makenoteinputs(Workspace*, Properties* notes);
static void workspace_makelang(Workspace*);
static void workspace_makelangen(Workspace*);
static void workspace_makelanges(Workspace*);
static int cmdnote(const char* key);
static void applysongproperties(Workspace*);
static Properties* workspace_makeproperties(Workspace*);
static void workspace_setdriverlist(Workspace*);
static void workspace_driverconfig(Workspace*);
static const char* workspace_driverpath(Workspace*);
static void workspace_configaudio(Workspace*);
static void workspace_configvisual(Workspace*);
static void workspace_configkeyboard(Workspace*);
static void workspace_setsong(Workspace*, Song*, int flag);
static void workspace_changedefaultfontsize(Workspace*, int size);

static MachineCallback machinecallback(Workspace*);
static unsigned int machinecallback_samplerate(Workspace*);
static int machinecallback_bpm(Workspace*);
static Samples* machinecallback_samples(Workspace*);
static Machines* machinecallback_machines(Workspace*);
static Instruments* machinecallback_instruments(Workspace*);

void workspace_init(Workspace* self)
{	
	self->octave = 4;
	signal_init(&self->signal_octavechanged);
	self->cursorstep = 1;	
	self->inputoutput = 0;
	workspace_makeconfig(self);
	plugincatcher_init(&self->plugincatcher, self->directories);
	self->hasplugincache = plugincatcher_load(&self->plugincatcher);
	machinefactory_init(&self->machinefactory, machinecallback(self), 
		&self->plugincatcher);
	self->song = (Song*) malloc(sizeof(Song));
	song_init(self->song, &self->machinefactory);	
	self->properties = workspace_makeproperties(self);		
	undoredo_init(&self->undoredo);
	signal_init(&self->signal_songchanged);
	signal_init(&self->signal_configchanged);
	signal_init(&self->signal_editpositionchanged);
}

void workspace_dispose(Workspace* self)
{	
	player_dispose(&self->player);	
	song_dispose(self->song);
	properties_free(self->config);
	properties_free(self->properties);
	properties_free(self->lang);
	signal_dispose(&self->signal_octavechanged);
	signal_dispose(&self->signal_songchanged);
	signal_dispose(&self->signal_configchanged);
	signal_dispose(&self->signal_editpositionchanged);
	plugincatcher_dispose(&self->plugincatcher);
	undoredo_dispose(&self->undoredo);
}

void workspace_initplayer(Workspace* self)
{
	player_init(&self->player, self->song, self->mainhandle->hwnd);
	workspace_driverconfig(self);
}
	
void workspace_configaudio(Workspace* self)
{			
	player_loaddriver(&self->player, workspace_driverpath(self));
	self->player.driver->open(self->player.driver);	
	workspace_driverconfig(self);	
}

void workspace_configvisual(Workspace* self)
{
	Properties* p;
	Properties* visual;

	visual = properties_find(self->config, "visual");
	if (visual) {
		p = properties_read(visual, "defaultfontsize");
		if (p) {
			workspace_changedefaultfontsize(self, properties_value(p));
		}
	}
}

void workspace_configkeyboard(Workspace* self)
{	
	Properties* notes;

	notes = properties_findsection(self->config, "keyboard.notes");
	if (notes) {
		inputs_dispose(&self->noteinputs);
		workspace_makenoteinputs(self, notes);		
	}
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

void workspace_driverconfig(Workspace* self)
{		
	self->driverconfigure->item.disposechildren = 0;
	self->driverconfigure->children = self->player.driver->properties;	
}

void workspace_scanplugins(Workspace* self)
{		
	plugincatcher_scan(&self->plugincatcher);
	plugincatcher_save(&self->plugincatcher);
}

void workspace_makeconfig(Workspace* self)
{	
	workspace_makelang(self);
	workspace_makegeneral(self);
	workspace_makevisual(self);	
	workspace_makekeyboard(self);
	workspace_makedirectories(self);
	workspace_makeinputoutput(self);
	workspace_makemidicontrollers(self);	
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
	workspace_makenotes(self);
}

void workspace_makenotes(Workspace* self)
{
	Properties* notes;
	Properties* p;

	notes = properties_createsection(self->keyboard, "notes");
	properties_settext(notes, "Notes");
	properties_append_int(notes, "cmd_note_c_0", encodeinput('Z', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_cs0", encodeinput('S', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_d_0", encodeinput('X', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_ds0", encodeinput('D', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_e_0", encodeinput('C', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_f_0", encodeinput('V', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_fs0", encodeinput('G', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_g_0", encodeinput('B', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_gs0", encodeinput('H', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_a_0", encodeinput('N', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_as0", encodeinput('J', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_b_0", encodeinput('M', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_c_1", encodeinput('Q', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_cs1", encodeinput('2', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_d_1", encodeinput('W', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_ds1", encodeinput('3', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_e_1", encodeinput('E', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_f_1", encodeinput('R', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_fs1", encodeinput('5', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_g_1", encodeinput('T', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_gs1", encodeinput('6', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_a_1", encodeinput('Y', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_as1", encodeinput('7', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_b_1", encodeinput('U', 0, 0), 0, 0);		
	properties_append_int(notes, "cmd_note_c_2", encodeinput('I', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_cs2", encodeinput('9', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_d_2", encodeinput('O', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_ds2", encodeinput('0', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_e_2", encodeinput('P', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_stop", encodeinput('1', 0, 0), 0, 0);
	// special
	properties_append_int(notes, "cmd_note_tweakm", encodeinput(192, 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_tweaks", encodeinput(192, 0, 1), 0, 0);
	for (p = notes->children; p != 0; p = properties_next(p)) {
		properties_sethint(p, PROPERTY_HINT_INPUT);
	}
	workspace_makenoteinputs(self, notes);
}

void workspace_makedirectories(Workspace* self)
{	
	self->directories = properties_settext(
		properties_createsection(self->config, "directories"),
		"Directories");
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

void workspace_makemidicontrollers(Workspace* self)
{
	Properties* midicontrollers;
		
	midicontrollers = properties_settext(
		properties_createsection(self->config, "midicontrollers"),
		"MIDI Controllers");
}

void workspace_makenoteinputs(Workspace* self, Properties* notes)
{
	Properties* p;

	inputs_init(&self->noteinputs);
	for (p = notes->children; p != 0; p = properties_next(p)) {
		int cmd;
		
		cmd = cmdnote(properties_key(p));
		if (cmd != -1) {
			inputs_define(&self->noteinputs, properties_value(p), cmd);
		}
	}
}

int cmdnote(const char* key)
{	
	if (strcmp("cmd_note_c_0", key) == 0) { return CMD_NOTE_C_0; } else
	if (strcmp("cmd_note_cs0", key) == 0) { return CMD_NOTE_CS0; } else
	if (strcmp("cmd_note_d_0", key) == 0) { return CMD_NOTE_D_0; } else
	if (strcmp("cmd_note_ds0", key) == 0) { return CMD_NOTE_DS0; } else
	if (strcmp("cmd_note_e_0", key) == 0) { return CMD_NOTE_E_0; } else
	if (strcmp("cmd_note_f_0", key) == 0) { return CMD_NOTE_F_0; } else
	if (strcmp("cmd_note_fs0", key) == 0) { return CMD_NOTE_FS0; } else
	if (strcmp("cmd_note_g_0", key) == 0) { return CMD_NOTE_G_0; } else
	if (strcmp("cmd_note_gs0", key) == 0) { return CMD_NOTE_GS0; } else
	if (strcmp("cmd_note_a_0", key) == 0) { return CMD_NOTE_A_0; } else
	if (strcmp("cmd_note_as0", key) == 0) { return CMD_NOTE_AS0; } else
	if (strcmp("cmd_note_b_0", key) == 0) { return CMD_NOTE_B_0; } else
	if (strcmp("cmd_note_c_1", key) == 0) { return CMD_NOTE_C_1; } else
	if (strcmp("cmd_note_cs1", key) == 0) { return CMD_NOTE_CS1; } else
	if (strcmp("cmd_note_d_1", key) == 0) { return CMD_NOTE_D_1; } else
	if (strcmp("cmd_note_ds1", key) == 0) { return CMD_NOTE_DS1; } else
	if (strcmp("cmd_note_e_1", key) == 0) { return CMD_NOTE_E_1; } else
	if (strcmp("cmd_note_f_1", key) == 0) { return CMD_NOTE_F_1; } else
	if (strcmp("cmd_note_fs1", key) == 0) { return CMD_NOTE_FS1; } else
	if (strcmp("cmd_note_g_1", key) == 0) { return CMD_NOTE_G_1; } else
	if (strcmp("cmd_note_gs1", key) == 0) { return CMD_NOTE_GS1; } else
	if (strcmp("cmd_note_a_1", key) == 0) { return CMD_NOTE_A_1; } else
	if (strcmp("cmd_note_as1", key) == 0) { return CMD_NOTE_AS1; } else
	if (strcmp("cmd_note_b_1", key) == 0) { return CMD_NOTE_B_1; } else
	if (strcmp("cmd_note_c_2", key) == 0) { return CMD_NOTE_C_2; } else
	if (strcmp("cmd_note_cs2", key) == 0) { return CMD_NOTE_CS2; } else
	if (strcmp("cmd_note_d_2", key) == 0) { return CMD_NOTE_D_2; } else
	if (strcmp("cmd_note_ds2", key) == 0) { return CMD_NOTE_DS2; } else
	if (strcmp("cmd_note_e_2", key) == 0) { return CMD_NOTE_E_2; } else
	if (strcmp("cmd_note_f_2", key) == 0) { return CMD_NOTE_F_2; } else
	if (strcmp("cmd_note_fs2", key) == 0) { return CMD_NOTE_FS2; } else
	if (strcmp("cmd_note_g_2", key) == 0) { return CMD_NOTE_G_2; } else
	if (strcmp("cmd_note_gs2", key) == 0) { return CMD_NOTE_GS2; } else
	if (strcmp("cmd_note_a_2", key) == 0) { return CMD_NOTE_A_2; } else
	if (strcmp("cmd_note_as2", key) == 0) { return CMD_NOTE_AS2; } else
	if (strcmp("cmd_note_b_2", key) == 0) { return CMD_NOTE_B_2; } else
	if (strcmp("cmd_note_c_3", key) == 0) { return CMD_NOTE_C_3; } else
	if (strcmp("cmd_note_cs3", key) == 0) { return CMD_NOTE_CS3; } else
	if (strcmp("cmd_note_d_3", key) == 0) { return CMD_NOTE_D_3; } else
	if (strcmp("cmd_note_ds3", key) == 0) { return CMD_NOTE_DS3; } else
	if (strcmp("cmd_note_e_3", key) == 0) { return CMD_NOTE_E_3; } else
	if (strcmp("cmd_note_f_3", key) == 0) { return CMD_NOTE_F_3; } else
	if (strcmp("cmd_note_fs3", key) == 0) { return CMD_NOTE_FS3; } else
	if (strcmp("cmd_note_g_3", key) == 0) { return CMD_NOTE_G_3; } else
	if (strcmp("cmd_note_gs3", key) == 0) { return CMD_NOTE_GS3; } else
	if (strcmp("cmd_note_a_3", key) == 0) { return CMD_NOTE_A_3; } else
	if (strcmp("cmd_note_as3", key) == 0) { return CMD_NOTE_AS3; } else
	if (strcmp("cmd_note_b_3", key) == 0) { return CMD_NOTE_B_3; } else
	if (strcmp("cmd_note_stop", key) == 0) { return CMD_NOTE_STOP; } else
	if (strcmp("cmd_note_tweakm", key) == 0) { return  CMD_NOTE_TWEAKM; } else	
	if (strcmp("cmd_note_midicc", key) == 0) { return CMD_NOTE_MIDICC; } else
	if (strcmp("cmd_note_tweaks", key) == 0) { return  CMD_NOTE_TWEAKS; }	
	return -1;
}

void workspace_makelang(Workspace* self)
{
	self->lang = properties_create();
	workspace_makelangen(self);
//	workspace_makelanges(self);
}

void workspace_makelangen(Workspace* self)
{
	properties_append_string(self->lang, "loadsong", "Load Song");
	properties_append_string(self->lang, "newsong", "New Song");
	properties_append_string(self->lang, "undo", "Undo");
	properties_append_string(self->lang, "redo", "Redo");
	properties_append_string(self->lang, "play", "Play");
	properties_append_string(self->lang, "stop", "Stop");
}

void workspace_makelanges(Workspace* self)
{
	properties_write_string(self->lang, "loadsong", "Carga Song");
	properties_write_string(self->lang, "newsong", "Nuevo Song");
	properties_write_string(self->lang, "undo", "Deshacer");
	properties_write_string(self->lang, "redo", "Rehacer");
	properties_write_string(self->lang, "play", "Toca");
	properties_write_string(self->lang, "stop", "Para");
}

const char* workspace_translate(Workspace* self, const char* key) {
	char* rv;

	properties_readstring(self->lang, key, &rv, key);
	return rv;
}

void workspace_setdriverlist(Workspace* self)
{
	Properties* drivers;

	properties_settext(self->inputoutput, "Input/Output");
	// change number to set startup driver, if no psycle.ini found
	drivers = properties_append_choice(self->inputoutput, "driver", 2); 
	properties_settext(drivers, "Driver");
	properties_append_string(drivers, "silent", "silentdriver");		
#if defined(DEBUG)
	properties_append_string(drivers, "mme", "..\\driver\\mme\\Debug\\mme.dll");
	properties_append_string(drivers, "directx", "..\\driver\\directx\\Debug\\directx.dll");
#else
	properties_append_string(drivers, "mme", "..\\driver\\mme\\Release\\mme.dll");
	properties_append_string(drivers, "directx", "..\\driver\\directx\\Release\\directx.dll");	
#endif
}

void workspace_configchanged(Workspace* self, Properties* property, Properties* choice)
{
	if (choice && (strcmp(properties_key(choice), "driver") == 0)) {
		player_reloaddriver(&self->player, properties_valuestring(property));		
		workspace_driverconfig(self);
	} else
	if (strcmp(properties_key(property), "defaultfontsize") == 0) {
		workspace_changedefaultfontsize(self, properties_value(property));
	} else
	if ((properties_hint(property) == PROPERTY_HINT_INPUT)) {		
		workspace_configkeyboard(self);		
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
	
	song = (Song*) malloc(sizeof(Song));
	song_init(song, &self->machinefactory);	
	properties_free(self->properties);
	self->properties = workspace_makeproperties(self);
	workspace_setsong(self, song, WORKSPACE_NEWSONG);		
}

void workspace_loadsong(Workspace* self, const char* path)
{	
	Song* song;

	properties_free(self->properties);
	song = malloc(sizeof(Song));
	song_init(song, &self->machinefactory);	
	song_load(song, path, &self->properties);
	workspace_setsong(self, song, WORKSPACE_LOADSONG);
}

void workspace_setsong(Workspace* self, Song* song, int flag)
{
	Song* oldsong;

	oldsong = self->song;
	suspendwork();	
	player_stop(&self->player);
	self->song = song;
	player_setsong(&self->player, self->song);
	applysongproperties(self);
	signal_emit(&self->signal_songchanged, self, 1, flag);
	song_dispose(oldsong);
	free(oldsong);			
	resumework();
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
	properties_load(self->config, "psycle.ini", 0);	
	workspace_configaudio(self);
	workspace_configvisual(self);
	workspace_configkeyboard(self);
	signal_emit(&self->signal_configchanged, self, 1,self->config);
}

void workspace_save_configuration(Workspace* self)
{
	properties_save(self->config, "psycle.ini");	
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

Inputs* workspace_noteinputs(Workspace* self)
{
	return &self->noteinputs;
}

void workspace_seteditposition(Workspace* self, EditPosition editposition)
{
	int line;
	double offset;

	offset = editposition.offset;
	line = (int) (offset * player_lpb(&self->player));
	self->editposition = editposition;
	self->editposition.line = line;
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
	return (int)player_bpm(&self->player);
}

Machines* machinecallback_machines(Workspace* self)
{
	return self->song ? &self->song->machines : 0;
}

Instruments* machinecallback_instruments(Workspace* self)
{
	return self->song ? &self->song->instruments : 0;
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "plugin.h"
// local
#include "pattern.h"
#include "plugin_interface.h"
#include "preset.h"
#include "presets.h"
#include "songio.h"
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

// psy_audio_PluginMachineParam
typedef struct psy_audio_PluginMachineParam {
	psy_audio_MachineParam machineparam;
	struct CMachineInterface* mi;
	struct CMachineInfo* cinfo;
	uintptr_t index;
} psy_audio_PluginMachineParam;

static void psy_audio_pluginmachineparam_init(psy_audio_PluginMachineParam*,
	struct CMachineInterface* mi,
	struct CMachineInfo* info,
	uintptr_t index);
static void psy_audio_pluginmachineparam_dispose(psy_audio_PluginMachineParam*);
psy_audio_PluginMachineParam* psy_audio_pluginmachineparam_alloc(void);

static void pluginparam_tweak(psy_audio_PluginMachineParam*, float val);
static void pluginparam_reset(psy_audio_PluginMachineParam*);
static float pluginparam_normvalue(psy_audio_PluginMachineParam*);
static void pluginparam_range(psy_audio_PluginMachineParam*,
	intptr_t* minval, intptr_t* maxval);
static int pluginparam_type(psy_audio_PluginMachineParam*);
static int pluginparam_label(psy_audio_PluginMachineParam*, char* text);
static int pluginparam_name(psy_audio_PluginMachineParam*, char* text);
static int pluginparam_describe(psy_audio_PluginMachineParam*, char* text);

static psy_audio_MachineParam* pluginparam_base(
	psy_audio_PluginMachineParam* self)
{
	return &(self->machineparam);
}

static MachineParamVtable pluginparam_vtable;
static int pluginparam_vtable_initialized = 0;

static void pluginparam_vtable_init(psy_audio_PluginMachineParam* self)
{
	if (!pluginparam_vtable_initialized) {
		pluginparam_vtable = *(self->machineparam.vtable);
		pluginparam_vtable.tweak = (fp_machineparam_tweak)pluginparam_tweak;
		pluginparam_vtable.reset = (fp_machineparam_reset)pluginparam_reset;
		pluginparam_vtable.normvalue = (fp_machineparam_normvalue)
			pluginparam_normvalue;
		pluginparam_vtable.range = (fp_machineparam_range)pluginparam_range;
		pluginparam_vtable.name = (fp_machineparam_name)pluginparam_name;
		pluginparam_vtable.label = (fp_machineparam_label)pluginparam_label;
		pluginparam_vtable.type = (fp_machineparam_type)pluginparam_type;
		pluginparam_vtable.describe = (fp_machineparam_describe)
			pluginparam_describe;
	}
}

void psy_audio_pluginmachineparam_init(psy_audio_PluginMachineParam* self,
	struct CMachineInterface* mi,
	CMachineInfo* cinfo,
	uintptr_t index)
{
	psy_audio_machineparam_init(&self->machineparam);
	pluginparam_vtable_init(self);
	self->machineparam.vtable = &pluginparam_vtable;
	self->mi = mi;
	self->cinfo = cinfo;
	self->index = index;
}

void psy_audio_pluginmachineparam_dispose(psy_audio_PluginMachineParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
	self->mi = NULL;
	self->cinfo = NULL;
	self->index = 0;
}

psy_audio_PluginMachineParam* psy_audio_pluginmachineparam_alloc(void)
{
	return (psy_audio_PluginMachineParam*) malloc(sizeof(psy_audio_PluginMachineParam));
}

psy_audio_PluginMachineParam* psy_audio_pluginmachineparam_allocinit(
	struct CMachineInterface* mi,
	CMachineInfo* cinfo,
	uintptr_t index)
{
	psy_audio_PluginMachineParam* rv;

	rv = psy_audio_pluginmachineparam_alloc();
	if (rv) {
		psy_audio_pluginmachineparam_init(rv, mi, cinfo, index);
	}
	return rv;
}

void pluginparam_tweak(psy_audio_PluginMachineParam* self, float value)
{
	int scaled;

	scaled = (int)(value * (self->cinfo->Parameters[self->index]->MaxValue  -
		self->cinfo->Parameters[self->index]->MinValue) + 0.5f) +
		self->cinfo->Parameters[self->index]->MinValue;
	mi_parametertweak(self->mi, (int)self->index, scaled);
}

void pluginparam_reset(psy_audio_PluginMachineParam* self)
{
	mi_parametertweak(self->mi, (int)self->index, self->cinfo->Parameters[self->index]->DefValue);
}

float pluginparam_normvalue(psy_audio_PluginMachineParam* self)
{
	return ((self->cinfo->Parameters[self->index]->MaxValue - self->cinfo->Parameters[self->index]->MinValue) != 0)
		? (mi_val(self->mi, (int)self->index) - self->cinfo->Parameters[self->index]->MinValue) /
		(float)(self->cinfo->Parameters[self->index]->MaxValue - self->cinfo->Parameters[self->index]->MinValue)
		: 0.f;
}

int pluginparam_describe(psy_audio_PluginMachineParam* self, char* text)
{
	return mi_describevalue(self->mi, text, (int)self->index, mi_val(self->mi,
		(int)self->index));
}

void pluginparam_range(psy_audio_PluginMachineParam* self, intptr_t* minval,
	intptr_t* maxval)
{		
	*minval = self->cinfo->Parameters[self->index]->MinValue;
	*maxval = self->cinfo->Parameters[self->index]->MaxValue;
}

int pluginparam_type(psy_audio_PluginMachineParam* self)
{
	return self->cinfo->Parameters[self->index]->Flags;
}

int pluginparam_name(psy_audio_PluginMachineParam* self, char* text)
{
	if (self->cinfo->Parameters[self->index]->Name) {
		psy_snprintf(text, 128, "%s", self->cinfo->Parameters[self->index]->Name);
	}
	return self->cinfo->Parameters[self->index]->Name != 0;
}

int pluginparam_label(psy_audio_PluginMachineParam* self, char* text)
{
	if (self->cinfo->Parameters[self->index]->Description) {
		psy_snprintf(text, 128, "%s", self->cinfo->Parameters[self->index]->Description);
	}
	return self->cinfo->Parameters[self->index]->Description != NULL;
}

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

// virtual methods
static void setcallback(psy_audio_Plugin*, psy_audio_MachineCallback);
static psy_audio_Machine* clone(psy_audio_Plugin*);
static int hostevent(psy_audio_Plugin*, int const eventNr, int val1, float val2);
static void generateaudio(psy_audio_Plugin*, psy_audio_BufferContext*);
static void seqtick(psy_audio_Plugin*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void stop(psy_audio_Plugin*);
static void newline(psy_audio_Plugin*);
static psy_audio_MachineInfo* info(psy_audio_Plugin*);
static psy_audio_MachineParam* parameter(psy_audio_Plugin*, uintptr_t param);
static unsigned int numparametercols(psy_audio_Plugin*);
static uintptr_t numparameters(psy_audio_Plugin*);
static void dispose(psy_audio_Plugin*);
static uintptr_t numinputs(psy_audio_Plugin*);
static uintptr_t numoutputs(psy_audio_Plugin*);
static int loadspecific(psy_audio_Plugin*, psy_audio_SongFile*,
	uintptr_t slot);
static int savespecific(psy_audio_Plugin*, psy_audio_SongFile*,
	uintptr_t slot);
static const char* modulepath(psy_audio_Plugin*);
static uintptr_t shellidx(psy_audio_Plugin*);
// data
static void putdata(psy_audio_Plugin*, uint8_t* data);
static uint8_t* data(psy_audio_Plugin*);
static uintptr_t datasize(psy_audio_Plugin*);
static void currentpreset(psy_audio_Plugin*, psy_audio_Preset*);
static void setpresets(psy_audio_Plugin*, psy_audio_Presets*);
static psy_audio_Presets* presets(psy_audio_Plugin*);
static bool acceptpresets(psy_audio_Plugin*);
static void command(psy_audio_Plugin*);
// private methods
static void disposeparameters(psy_audio_Plugin*);
static void tweakdefaults(psy_audio_Plugin*, CMachineInfo* info);
static void clearparameters(psy_audio_Plugin* self);
// programs
static void programname(psy_audio_Plugin*, uintptr_t bnkidx, uintptr_t prgidx, char* val);
static uintptr_t numprograms(psy_audio_Plugin*);
static void setcurrprogram(psy_audio_Plugin*, uintptr_t prgidx);
static uintptr_t currprogram(psy_audio_Plugin*);
static void bankname(psy_audio_Plugin*, uintptr_t bnkidx, char* val);
static uintptr_t numbanks(psy_audio_Plugin*);
static void setcurrbank(psy_audio_Plugin*, uintptr_t bnkidx);
static uintptr_t currbank(psy_audio_Plugin*);

static MachineVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_audio_Plugin* self)
{
	if (!vtable_initialized) {
		vtable = *(psy_audio_plugin_base(self)->vtable);
		vtable.setcallback = (fp_machine_setcallback)setcallback;
		vtable.clone = (fp_machine_clone)clone;
		vtable.hostevent = (fp_machine_hostevent)hostevent;
		vtable.seqtick = (fp_machine_seqtick)seqtick;
		vtable.stop = (fp_machine_stop) stop;
		vtable.newline = (fp_machine_newline)newline;
		vtable.info = (fp_machine_info) info;
		vtable.modulepath = (fp_machine_modulepath)modulepath;
		vtable.shellidx = (fp_machine_shellidx)shellidx;
		vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		vtable.numparameters = (fp_machine_numparameters)numparameters;
		vtable.parameter = (fp_machine_parameter)parameter;
		vtable.dispose = (fp_machine_dispose)dispose;
		vtable.generateaudio = (fp_machine_generateaudio)generateaudio;
		vtable.numinputs = (fp_machine_numinputs)numinputs;
		vtable.numoutputs = (fp_machine_numoutputs)numoutputs;
		vtable.putdata = (fp_machine_putdata)putdata;
		vtable.data = (fp_machine_data)data;
		vtable.datasize = (fp_machine_datasize)datasize;
		vtable.currentpreset = (fp_machine_currentpreset)currentpreset;
		vtable.setpresets = (fp_machine_setpresets)setpresets;
		vtable.presets = (fp_machine_presets)presets;
		vtable.acceptpresets = (fp_machine_acceptpresets)acceptpresets;
		vtable.command = (fp_machine_command)command;
		vtable.loadspecific = (fp_machine_loadspecific)loadspecific;
		vtable.savespecific = (fp_machine_savespecific)savespecific;
		vtable.programname = (fp_machine_programname)programname;
		vtable.numprograms = (fp_machine_numprograms)numprograms;
		vtable.setcurrprogram = (fp_machine_setcurrprogram)setcurrprogram;
		vtable.currprogram = (fp_machine_currprogram)currprogram;
		vtable.bankname = (fp_machine_bankname)bankname;
		vtable.numbanks = (fp_machine_numbanks)numbanks;
		vtable.setcurrbank = (fp_machine_setcurrbank)setcurrbank;
		vtable.currbank = (fp_machine_currbank)currbank;
		vtable_initialized = TRUE;
	}
}

void psy_audio_plugin_init(psy_audio_Plugin* self, psy_audio_MachineCallback* callback,
	const char* path, const char* root)
{
	GETINFO GetInfo;
		
	psy_audio_custommachine_init(&self->custommachine, callback);	
	vtable_init(self);
	psy_audio_plugin_base(self)->vtable = &vtable;
	psy_audio_logicalchannels_init(&self->logicalchannels);
	psy_table_init(&self->parameters);
	psy_library_init(&self->library);
	psy_library_setenv(&self->library, path, root);
	psy_library_load(&self->library, path);
	psy_library_restoreenv(&self->library);	
	self->mi = 0;
	self->plugininfo = 0;
	self->preventnewline = 0;
	self->presets = NULL;
	self->currprog = 0;
	GetInfo = (GETINFO)psy_library_functionpointer(&self->library, "GetInfo");
	if (!GetInfo) {
		psy_library_dispose(&self->library);		
	} else {		
		self->mi = mi_create(self->library.module);		
		if (!self->mi) {
			psy_library_dispose(&self->library);			
		} else {						
			CMachineInfo* pInfo = GetInfo();
			if (pInfo) {				
				mi_resetcallback(self->mi);
				mi_setcallback(self->mi, callback);
				mi_init(self->mi);
				tweakdefaults(self, pInfo);
				self->plugininfo = machineinfo_allocinit();												
				machineinfo_setnativeinfo(self->plugininfo, pInfo, psy_audio_PLUGIN,
					self->library.path, 0);								
				psy_audio_machine_seteditname(psy_audio_plugin_base(self),
					pInfo->ShortName);
				if (strcmp(self->plugininfo->shortname, "BexPhase!") == 0) {
					self->preventnewline = 1;
				}
			}
		}
	}
	if (!psy_audio_machine_editname(psy_audio_plugin_base(self))) {
		psy_audio_machine_seteditname(psy_audio_plugin_base(self), "Plugin");
	}
}

void tweakdefaults(psy_audio_Plugin* self, CMachineInfo* info)
{
	int gbp;

	for (gbp = 0; gbp < info->numParameters; ++gbp) {
		mi_parametertweak(self->mi, gbp, info->Parameters[gbp]->DefValue);		
	}
}

void dispose(psy_audio_Plugin* self)
{		
	if (self->library.module != 0 && self->mi) {
		mi_dispose(self->mi);
		psy_library_dispose(&self->library);
		self->mi = 0;		
	}			
	if (self->plugininfo) {
		machineinfo_dispose(self->plugininfo);
		free(self->plugininfo);
		self->plugininfo = 0;
	}
	if (self->presets) {
		psy_audio_presets_dispose(self->presets);
		free(self->presets);
	}
	disposeparameters(self);
	psy_audio_logicalchannels_dispose(&self->logicalchannels);
	psy_audio_custommachine_dispose(&self->custommachine);
}

void disposeparameters(psy_audio_Plugin* self)
{
	psy_table_disposeall(&self->parameters, (psy_fp_disposefunc)
		psy_audio_pluginmachineparam_dispose);
}

void clearparameters(psy_audio_Plugin* self)
{
	psy_table_disposeall(&self->parameters, (psy_fp_disposefunc)
		psy_audio_pluginmachineparam_dispose);	
	psy_table_init(&self->parameters);
}

void setcallback(psy_audio_Plugin* self, psy_audio_MachineCallback callback)
{
	if (self->mi) {
		mi_setcallback(self->mi, &callback);
	}
}

psy_audio_Machine* clone(psy_audio_Plugin* self)
{
	psy_audio_Plugin* rv;

	rv = malloc(sizeof(psy_audio_Plugin));
	if (rv) {
		psy_audio_plugin_init(rv, self->custommachine.machine.callback,
			self->library.path,
			self->library.root);
		if (rv->library.module) {
			psy_audio_Preset preset;
			double x;
			double y;

			psy_audio_preset_init(&preset);
			currentpreset(self, &preset);
			psy_audio_plugin_tweakpreset(rv, &preset);
			psy_audio_preset_dispose(&preset);
			psy_audio_machine_position(psy_audio_plugin_base(self), &x, &y);
			psy_audio_machine_setposition(psy_audio_plugin_base(rv), x, y);
		} else {
			psy_audio_machine_dispose(psy_audio_plugin_base(rv));
			free(rv);
			rv = 0;
		}
	}
	return rv ? &rv->custommachine.machine : 0;
}

int psy_audio_plugin_psycle_test(const char* path, const char* root, psy_audio_MachineInfo* info)
{	
	int rv = 0;

	if (path && strcmp(path, "") != 0) {
		GETINFO GetInfo;
		psy_Library library;		

		psy_library_init(&library);
		psy_library_setenv(&library, path, root);
		psy_library_load(&library, path);					
		GetInfo =(GETINFO)psy_library_functionpointer(&library, "GetInfo");
		if (GetInfo != NULL) {	
			CMachineInfo* nativeinfo = GetInfo();
			if (nativeinfo) {				
				machineinfo_setnativeinfo(info, nativeinfo, psy_audio_PLUGIN,
					library.path, 0);
				rv = 1;
			}
		}
		psy_library_restoreenv(&library);
		psy_library_dispose(&library);
	}
	return rv;	
}

void seqtick(psy_audio_Plugin* self, uintptr_t channel,
	const psy_audio_PatternEvent* ev)
{	
	uintptr_t physicalchannel;

	physicalchannel = psy_audio_logicalchannels_physical(
		&self->logicalchannels, channel);
	if (psy_audio_patternevent_has_volume(ev)) {
		mi_seqtick(self->mi, physicalchannel, ev->note, ev->inst & 0xFF, 0x0C, ev->vol);
		if (ev->parameter != 0 || ev->cmd != 0) {
			mi_seqtick(self->mi, physicalchannel, psy_audio_NOTECOMMANDS_EMPTY,
				psy_audio_NOTECOMMANDS_EMPTY, ev->cmd, ev->parameter);
		}
	} else
	if (ev->note == psy_audio_NOTECOMMANDS_MIDICC && ev->inst >= 0x80 && ev->inst < 0xFF) {		
		mi_midievent(self->mi, ev->inst & 0x0F, ev->inst & 0xF0,
			(ev->cmd << 8) + ev->parameter);			
	} else {
		mi_seqtick(self->mi, physicalchannel, ev->note, ev->inst & 0xFF, ev->cmd,
			ev->parameter);
	}
}

void generateaudio(psy_audio_Plugin* self, psy_audio_BufferContext* bc)
{
	mi_work(self->mi,
		psy_audio_buffer_at(bc->output, 0),
		psy_audio_buffer_at(bc->output, 1),
		bc->numsamples, bc->numsongtracks);	
}

int hostevent(psy_audio_Plugin* self, int const eventNr, int val1, float val2)
{
	return mi_hostevent(self->mi, eventNr, val1, val2);
}


void stop(psy_audio_Plugin* self)
{
	mi_stop(self->mi);
	psy_audio_logicalchannels_reset(&self->logicalchannels);
}


void newline(psy_audio_Plugin* self)
{	
	// prevent bexphase from crash
	if (!self->preventnewline) {
		mi_sequencertick(self->mi);	
	}	
}

psy_audio_MachineInfo* info(psy_audio_Plugin* self)
{		
	return self->plugininfo;
}

psy_audio_MachineParam* parameter(psy_audio_Plugin* self, uintptr_t id)
{
	psy_audio_MachineParam* param = NULL;

	if (!psy_table_exists(&self->parameters, id)) {
		GETINFO GetInfo;

		GetInfo = (GETINFO)psy_library_functionpointer(&self->library, "GetInfo");
		if (GetInfo != NULL) {
			CMachineInfo* info = GetInfo();

			param = pluginparam_base(psy_audio_pluginmachineparam_allocinit(
				self->mi, info, id));
			psy_table_insert(&self->parameters, id, (void*)param);
		}
	} else {
		param = (psy_audio_MachineParam*)psy_table_at(&self->parameters, id);
	}
	return param;
}

unsigned int numparametercols(psy_audio_Plugin* self)
{
	int rv = 0;
	GETINFO GetInfo;

	GetInfo = (GETINFO)psy_library_functionpointer(&self->library, "GetInfo");
	if (GetInfo != NULL) {
		CMachineInfo* info = GetInfo();
		if (info) {
			rv = info->numparametercols;
		}
	}
	return rv;
}

uintptr_t numparameters(psy_audio_Plugin* self)
{
	uintptr_t rv = 0;
	GETINFO GetInfo;

	GetInfo = (GETINFO)psy_library_functionpointer(&self->library, "GetInfo");
	if (GetInfo != NULL) {
		CMachineInfo* info = GetInfo();

		rv = info->numParameters;
	}
	return rv;
}

uintptr_t numinputs(psy_audio_Plugin* self)
{
	return info(self) ? (self->plugininfo->mode == psy_audio_MACHMODE_FX ? 2 : 0) : 0;
}

uintptr_t numoutputs(psy_audio_Plugin* self)
{
	return info(self) ? 2 : 0;
}

void putdata(psy_audio_Plugin* self, uint8_t* data)
{
	mi_putdata(self->mi, data);
}

uint8_t* data(psy_audio_Plugin* self)
{
	uint8_t* rv = 0;

	mi_getdata(self->mi, rv);
	return rv;
}

uintptr_t datasize(psy_audio_Plugin* self)
{
	return mi_getdatasize(self->mi);
}

void currentpreset(psy_audio_Plugin* self, psy_audio_Preset* preset)
{
	CMachineInfo* info;
	GETINFO GetInfo;

	psy_audio_preset_clear(preset);
	GetInfo = (GETINFO)psy_library_functionpointer(&self->library, "GetInfo");
	info = GetInfo();
	if (info) {
		uintptr_t gbp;		
		uintptr_t size;		
		unsigned char* pData;

		for (gbp = 0; gbp < (uintptr_t)info->numParameters; ++gbp) {
			psy_audio_preset_setvalue(preset, gbp, mi_val(self->mi, gbp));
		}		
		size = mi_getdatasize(self->mi);
		if (size > 0) {
			pData = (unsigned char*)malloc(size);
			mi_getdata(self->mi, pData);
			psy_audio_preset_putdata(preset, size, pData);
			free(pData);
		}
	}
}

int loadspecific(psy_audio_Plugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{	
	uint32_t size;
	int status;

	assert(self);

	clearparameters(self);
	// size of whole structure
	if (status = psyfile_read(songfile->file, &size, sizeof(size))) {
		return status;
	}
	if(size) {
		uint32_t numparams;
		uint32_t i;

		// size of vars
		if (status = psyfile_read(songfile->file, &numparams, sizeof(numparams))) {
			return status;
		}
		for (i = 0; i < numparams; ++i) {
			int32_t temp;
			psy_audio_MachineParam* param;
			
			if (status = psyfile_read(songfile->file, &temp, sizeof(temp))) {
				return status;
			}
			if (i < numparameters(self)) {
				intptr_t minval;
				intptr_t maxval;

				param = psy_audio_machine_parameter(psy_audio_plugin_base(self), i);
				if (param) {
					psy_audio_machine_parameter_range(psy_audio_plugin_base(self),
						param, &minval, &maxval);
					// skip out of range values (mfc-psycle behaviour), because
					// tweak will set them to min or maxval
					if (temp >= minval && temp <= maxval) {
						psy_audio_machine_parameter_tweak_scaled(
							psy_audio_plugin_base(self), param, temp);
					}
				}
			}
		}
		size -= sizeof(numparams) + sizeof(int) * numparams;
		if(size) {
			unsigned int size2 = 0;
			unsigned char* pData;
			
			size2 = mi_getdatasize(self->mi);
			// This way we guarantee that the plugin will have enough bytes,
			// even if it does not fit what it reads.
			pData = (unsigned char*)malloc(psy_max(size,size2));
			if (status = psyfile_read(songfile->file, pData, size)) { // Read internal data.			
				free(pData);
				return status;
			}
			mi_putdata(self->mi, pData); // Internal load
			free(pData);
		}						
	}
	return PSY_OK;
}			

int savespecific(psy_audio_Plugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t count;
	uint32_t size2;
	uint32_t size;
	uint32_t i;
	int status;

	count = (uint32_t)psy_audio_machine_numparameters(
		psy_audio_plugin_base(self));
	size2 = mi_getdatasize(self->mi);
	size = size2 + sizeof(count) + sizeof(int) * count;
	if (status = psyfile_write(songfile->file, &size, sizeof(size))) {
		return status;
	}
	if (status = psyfile_write(songfile->file, &count, sizeof(count))) {
		return status;
	}
	for (i = 0; i < count; ++i) {
		psy_audio_MachineParam* param;
		intptr_t scaled = 0;		

		param = psy_audio_machine_parameter(psy_audio_plugin_base(self), i);
		if (param) {
			scaled = psy_audio_machine_parameter_scaledvalue(
				psy_audio_plugin_base(self), param);			
		}
		if (status = psyfile_write_int32(songfile->file, (int32_t)scaled)) {
			return status;
		}
	}
	if (size2) {
		unsigned char* data;

		data = malloc(size2);
		mi_getdata(self->mi, data);
		if (status = psyfile_write(songfile->file, data, size2)) { //data chunk		
			free(data);
			return status;
		}
		free(data);
		data = NULL;
	}
	return PSY_OK;
}

void setpresets(psy_audio_Plugin* self, psy_audio_Presets* presets)
{
	if (self->presets) {
		psy_audio_presets_dispose(self->presets);
		free(self->presets);		
	}
	self->presets = presets;
}

psy_audio_Presets* presets(psy_audio_Plugin* self)
{
	return self->presets;
}

bool acceptpresets(psy_audio_Plugin* self)
{
	return TRUE;
}

void command(psy_audio_Plugin* self)
{
	mi_command(self->mi);
}

void programname(psy_audio_Plugin* self, uintptr_t bnkidx, uintptr_t prgidx, char* val)
{
	if (self->presets && bnkidx == 0) {
		psy_audio_Preset* preset;

		preset = psy_audio_presets_at(self->presets, prgidx);
		if (preset) {
			psy_snprintf(val, 256, "%s", psy_audio_preset_name(preset));
			return;
		}
	}
	val[0] = '\0';
}

uintptr_t numprograms(psy_audio_Plugin* self)
{
	if (self->presets) {
		return psy_audio_presets_size(self->presets);
	}
	return 0;
}

void setcurrprogram(psy_audio_Plugin* self, uintptr_t prgidx)
{
	self->currprog = prgidx;
	if (self->presets) {
		psy_audio_Preset* preset;

		preset = (psy_audio_Preset*)psy_table_at(
			&self->presets->container, self->currprog);
		if (preset) {
			psy_audio_plugin_tweakpreset(self, preset);
		}
	}
}

void psy_audio_plugin_tweakpreset(psy_audio_Plugin* self, psy_audio_Preset* preset)
{
	if (preset) {
		psy_TableIterator it;

		for (it = psy_table_begin(&preset->parameters);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_MachineParam* param;

			param = psy_audio_machine_tweakparameter(
				psy_audio_plugin_base(self), psy_tableiterator_key(&it));
			if (param) {
				psy_audio_machine_parameter_tweak_scaled(
					psy_audio_plugin_base(self), param,
					(intptr_t)psy_tableiterator_value(&it));
			}
		}
		if (preset->data && preset->datasize == datasize(self)) {
			putdata(self, preset->data);
		}
	}
}

uintptr_t currprogram(psy_audio_Plugin* self)
{
	return  (self->currprog != psy_INDEX_INVALID)
		? (int)self->currprog
		: 0;
}

void bankname(psy_audio_Plugin* self, uintptr_t bnkidx, char* val)
{
	if (bnkidx < numbanks(self)) {
		psy_snprintf(val, 256, "Internal %d", bnkidx + 1);
	} else {
		val[0] = '\0';
	}
}

uintptr_t numbanks(psy_audio_Plugin* self)
{
	return 1;
}

void setcurrbank(psy_audio_Plugin* self, uintptr_t bnkidx)
{	
}

uintptr_t currbank(psy_audio_Plugin* self)
{
	return 0;
}

const char* modulepath(psy_audio_Plugin* self)
{
	return self->library.path;
}

uintptr_t shellidx(psy_audio_Plugin* self)
{
	return 0;
}

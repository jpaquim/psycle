/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "lv2plugin.h"

#ifdef PSYCLE_USE_LV2

#include "../../detail/os.h"
#include <stdlib.h>
#include <operations.h>
#include "pattern.h"
#include "sequencer.h"
#include "plugin_interface.h"
#include "songio.h"
#include "preset.h"

#include "../../driver/eventdriver.h"

#include <dir.h>

#include "../../detail/portable.h"
#include "../../detail/trace.h"

#include "lv2/urid/urid.h"


/* LilvTestUriMap: Copyright 2007-2020 David Robillard <d@drobilla.net> */
static void
lilv_test_uri_map_init(LilvTestUriMap* const map)
{
  map->uris   = NULL;
  map->n_uris = 0;
}

static void
lilv_test_uri_map_clear(LilvTestUriMap* const map)
{
  for (uint32_t i = 0; i < map->n_uris; ++i) {
    free(map->uris[i]);
  }

  free(map->uris);
  map->uris   = NULL;
  map->n_uris = 0;
}

static LV2_URID
map_uri(LV2_URID_Map_Handle handle, const char* uri)
{
  LilvTestUriMap* map = (LilvTestUriMap*)handle;

  for (uint32_t i = 0; i < map->n_uris; ++i) {
    if (!strcmp(map->uris[i], uri)) {
      return i + 1;
    }
  }

  /* assert(serd_uri_string_has_scheme((const uint8_t*)uri)); */

  map->uris = (char**)realloc(map->uris, ++map->n_uris * sizeof(char*));
  map->uris[map->n_uris - 1] = psy_strdup(uri); /* exchanged lilv_strdup */
  return map->n_uris;
}

static const char*
unmap_uri(LV2_URID_Map_Handle handle, LV2_URID urid)
{
  LilvTestUriMap* map = (LilvTestUriMap*)handle;

  if (urid > 0 && urid <= map->n_uris) {
    return map->uris[urid - 1];
  }

  return NULL;
}
/* end LilvTestUriMap */


/* prototypes */
static const psy_audio_MachineInfo* info(psy_audio_LV2Plugin*);
static void generateaudio(psy_audio_LV2Plugin*, psy_audio_BufferContext*);
// Parameter
static psy_audio_MachineParam* parameter(psy_audio_LV2Plugin*,
	uintptr_t param);
static uintptr_t numparameters(psy_audio_LV2Plugin*);
static unsigned int numparametercols(psy_audio_LV2Plugin*);
static void dispose(psy_audio_LV2Plugin* self);
static uintptr_t numinputs(psy_audio_LV2Plugin*);
static uintptr_t numoutputs(psy_audio_LV2Plugin*);
static int loadspecific(psy_audio_LV2Plugin*, psy_audio_SongFile*,
	uintptr_t slot);
static int savespecific(psy_audio_LV2Plugin*, psy_audio_SongFile*,
	uintptr_t slot);
static int haseditor(psy_audio_LV2Plugin*);
static void seteditorhandle(psy_audio_LV2Plugin*, void* handle);
static void editorsize(psy_audio_LV2Plugin*, double* width, double* height);
static void editoridle(psy_audio_LV2Plugin*);
// static int makemachineinfo(AEffect* effect, psy_audio_MachineInfo*,
//	const char* path, int shellidx);
// typedef AEffect* (*PluginEntryProc)(audioMasterCallback audioMaster);
// static VstIntPtr VSTCALLBACK hostcallback(AEffect* effect, VstInt32 opcode,
//	VstInt32 index, VstIntPtr value, void* ptr, float opt);
// static PluginEntryProc getmainentry(psy_Library* library);
static void processevents(psy_audio_LV2Plugin*, psy_audio_BufferContext*);
static void generateaudio(psy_audio_LV2Plugin*, psy_audio_BufferContext*);
static void stop(psy_audio_LV2Plugin*);
// auxcolumns
static const char* auxcolumnname(psy_audio_LV2Plugin*, uintptr_t index);
static uintptr_t numauxcolumns(psy_audio_LV2Plugin*);

static psy_dsp_amp_range_t amprange(psy_audio_LV2Plugin* self)
{
	return PSY_DSP_AMP_RANGE_VST;
}

// programs
static void programname(psy_audio_LV2Plugin*, uintptr_t bnkidx,
	uintptr_t prgidx, char* val);
static uintptr_t numprograms(psy_audio_LV2Plugin*);
static void setcurrprogram(psy_audio_LV2Plugin*, uintptr_t prgidx);
static uintptr_t currprogram(psy_audio_LV2Plugin*);
static void bankname(psy_audio_LV2Plugin*, uintptr_t bnkidx, char* val);
static uintptr_t numbanks(psy_audio_LV2Plugin*);
static void setcurrbank(psy_audio_LV2Plugin*, uintptr_t bnkidx);
static uintptr_t currbank(psy_audio_LV2Plugin*);
static void currentpreset(psy_audio_LV2Plugin*, psy_audio_Preset*);
// static void lv2plugin_onfileselect(psy_audio_LV2Plugin*,
//	struct VstFileSelect*);
static void prepareparams(psy_audio_LV2Plugin*);
static void clearparams(psy_audio_LV2Plugin*);
static void disposeparameters(psy_audio_LV2Plugin*);
static void update_vsttimeinfo(psy_audio_LV2Plugin*);
static void preparebuffers(psy_audio_LV2Plugin*, psy_audio_BufferContext*);
static void getnuminputsoutputs(psy_audio_LV2Plugin*);

static LV2_URID uri_table_map(LV2_URID_Map_Handle handle, const char *uri);
static const char *uri_table_unmap(LV2_URID_Map_Handle handle, LV2_URID urid);



static const char* MIDI_CHAN_NAMES[16] = {
	"MIDI Channel 01", "MIDI Channel 02","MIDI Channel 03","MIDI Channel 04",
	"MIDI Channel 05","MIDI Channel 06","MIDI Channel 07","MIDI Channel 08",
	"MIDI Channel 09","MIDI Channel 10","MIDI Channel 11","MIDI Channel 12",
	"MIDI Channel 13","MIDI Channel 14","MIDI Channel 15","MIDI Channel 16"
};

/* vtable */
static MachineVtable vtable;
static int vtable_initialized = FALSE;

static void vtable_init(psy_audio_LV2Plugin* self)
{
	if (!vtable_initialized) {
		vtable = *(psy_audio_lv2plugin_base(self)->vtable);		
		vtable.generateaudio =
			(fp_machine_generateaudio)
			generateaudio;
		vtable.info = (fp_machine_info)info;
		vtable.parameter = (fp_machine_parameter)parameter;
		vtable.numparameters = (fp_machine_numparameters)numparameters;
		vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		vtable.dispose = (fp_machine_dispose)dispose;
		vtable.numinputs = (fp_machine_numinputs)numinputs;
		vtable.numoutputs = (fp_machine_numoutputs)numoutputs;
		vtable.loadspecific = (fp_machine_loadspecific)loadspecific;
		vtable.savespecific = (fp_machine_savespecific)savespecific;
		vtable.haseditor = (fp_machine_haseditor)haseditor;
		vtable.seteditorhandle = (fp_machine_seteditorhandle)seteditorhandle;
		vtable.editorsize = (fp_machine_editorsize)editorsize;
		vtable.editoridle = (fp_machine_editoridle)editoridle;
		vtable.amprange = (fp_machine_amprange)amprange;
		vtable.programname = (fp_machine_programname)programname;
		vtable.numprograms = (fp_machine_numprograms)numprograms;
		vtable.setcurrprogram = (fp_machine_setcurrprogram)setcurrprogram;
		vtable.currprogram = (fp_machine_currprogram)currprogram;
		vtable.bankname = (fp_machine_bankname)bankname;
		vtable.numbanks = (fp_machine_numbanks)numbanks;
		vtable.setcurrbank = (fp_machine_setcurrbank)setcurrbank;
		vtable.currbank = (fp_machine_currbank)currbank;
		vtable.currentpreset = (fp_machine_currentpreset)currentpreset;
		vtable.stop = (fp_machine_stop)stop;
		vtable.auxcolumnname = (fp_machine_auxcolumnname)auxcolumnname;
		vtable.numauxcolumns = (fp_machine_numauxcolumns)numauxcolumns;
		vtable_initialized = TRUE;
	}
}

int psy_audio_lv2plugin_init(psy_audio_LV2Plugin* self,
	psy_audio_MachineCallback* callback, const char* path,
	uintptr_t shell_index)
{	
	int status;
	const LilvPlugins* plugins;
	LilvNode*          uri;
	LilvNode*          name;
	LilvIter*          it;
	uintptr_t          i;
	char               path_with_slash[4096];
	
	assert(self);

	status = PSY_OK;
	psy_audio_custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	psy_audio_lv2plugin_base(self)->vtable = &vtable;
	psy_audio_machine_setcallback(psy_audio_lv2plugin_base(self), callback);
	psy_table_init(&self->parameters);	
	self->editorhandle = NULL;
	self->plugininfo = NULL;	
	self->world = lilv_world_new();
	psy_snprintf(path_with_slash, 4096, "%s"psy_SLASHSTR, path);	
	uri = lilv_new_file_uri(self->world, NULL, path_with_slash);
	psy_table_init(&self->parameters);	
	psy_table_init(&self->inportmap);
	psy_table_init(&self->outportmap);
	lilv_test_uri_map_init(&self->uri_table);
	self->map.handle = &self->uri_table;
    self->map.map = map_uri;
    self->map_feature.URI = LV2_URID_MAP_URI;
    self->map_feature.data = &self->map;
    self->unmap.handle = &self->uri_table;
    self->unmap.unmap  = unmap_uri;
    self->unmap_feature.URI = LV2_URID_UNMAP_URI;
	self->features[0] = &self->map_feature;
    self->features[1] = &self->unmap_feature;
    self->numInputs = 0;
    self->numOutputs = 0;
	self->input_port_class = lilv_new_uri(self->world, LV2_CORE__InputPort);
	self->output_port_class = lilv_new_uri(self->world, LV2_CORE__OutputPort);
	self->audio_port_class = lilv_new_uri(self->world, LV2_CORE__AudioPort);
	self->control_port_class = lilv_new_uri(self->world, LV2_CORE__ControlPort);	
	if (!uri) {
		status = PSY_ERRFILE;
		psy_audio_machine_bypass(psy_audio_lv2plugin_base(self));
		return status;
	}
	lilv_world_load_bundle(self->world, uri);
	lilv_world_load_specifications(self->world);
	lilv_world_load_plugin_classes(self->world);
	plugins = lilv_world_get_all_plugins(self->world);
	if (!plugins) {
		status = PSY_ERRFILE;
		psy_audio_machine_bypass(psy_audio_lv2plugin_base(self));
		return status;
	}
	i = 0;
	for (it = lilv_plugins_begin(plugins);
			!lilv_plugins_is_end(plugins, it);
			it = lilv_plugins_next(plugins, it),
			++i) {
		if (i == shell_index) {					
			self->plugin = lilv_plugins_get(plugins, it);
			assert(self->plugin);
			self->plugininfo = machineinfo_allocinit();
			machineinfo_set(
				self->plugininfo,
				lilv_node_as_string(lilv_plugin_get_author_name(self->plugin)),
				"", //const char* command,
				0, // int flags,
				psy_audio_MACHMODE_FX, //int mode,
				lilv_node_as_string(lilv_plugin_get_name(self->plugin)),
				lilv_node_as_string(lilv_plugin_get_name(self->plugin)),
				1, // short apiversion,
				0, // short plugversion,
				psy_audio_LV2,
				path,
				shell_index,
				"",
				lilv_node_as_string(lilv_plugin_get_project(self->plugin)),
				"",
				psy_INDEX_INVALID,
				"");			
			getnuminputsoutputs(self);
			self->instance = lilv_plugin_instantiate(self->plugin,
				psy_audio_machine_samplerate(psy_audio_lv2plugin_base(self)),
				self->features);
			if (self->instance) {
				prepareparams(self);				
				lilv_instance_activate(self->instance);						
			} else {
				status = PSY_ERRRUN;
				psy_audio_machine_bypass(psy_audio_lv2plugin_base(self));
			}			
			break;
		}
	}	
	
	if (!psy_audio_machine_editname(psy_audio_lv2plugin_base(self))) {
		psy_audio_machine_seteditname(psy_audio_lv2plugin_base(self),
			lilv_node_as_string(lilv_plugin_get_name(self->plugin)));
	}
	return status;
} 

void dispose(psy_audio_LV2Plugin* self)
{
	assert(self);
	
	lilv_node_free(self->control_port_class);
	lilv_node_free(self->audio_port_class);
	lilv_node_free(self->output_port_class);
	lilv_node_free(self->input_port_class);
	
	if (self->instance) {	
		lilv_instance_deactivate(self->instance);
		lilv_instance_free(self->instance);
		self->instance = NULL;
	}
	lilv_world_free(self->world);	
	if (self->plugininfo) {
		machineinfo_dispose(self->plugininfo);
		free(self->plugininfo);
		self->plugininfo = NULL;
	}
	// psy_table_dispose(&self->tracknote);
	disposeparameters(self);	
	psy_table_dispose(&self->inportmap);
	psy_table_dispose(&self->outportmap);
	lilv_test_uri_map_clear(&self->uri_table);
	// vsttimeinfo_deallocate(self->vsttimeinfo);	
		
	
	psy_audio_custommachine_dispose(&self->custommachine);
}

void prepareparams(psy_audio_LV2Plugin* self)
{
	//we're passing addresses within the vector to the plugin.. let's be sure they don't move around	
	int indexinput = 0;
	int indexoutput = 0;
	int indexpar = 0;
	unsigned int lPortIndex;
	unsigned int index;

	clearparams(self);
	index = 0;
	for (lPortIndex = 0; lPortIndex <  lilv_plugin_get_num_ports(self->plugin);
			++lPortIndex) {
		const LilvPort * port;		
		
		port = lilv_plugin_get_port_by_index(self->plugin, lPortIndex);
		if (port) {
			if (lilv_port_is_a(self->plugin, port, self->control_port_class)) {
				psy_audio_LV2Param* param;
		
				param = psy_audio_lv2param_allocinit(
					port,
					self->plugin,				
					lilv_node_as_string(lilv_port_get_name(self->plugin, port)),					
					index,
					lPortIndex);
				psy_table_insert(&self->parameters, index, (void*)
					param);
				++index;
				printf("connect %s %i", param->port_name, (int)lPortIndex);
				lilv_instance_connect_port(self->instance, lPortIndex,
					&param->value_);			
				indexpar++;
			}			
		}
	}	
}

void disposeparameters(psy_audio_LV2Plugin* self)
{
	assert(self);

	psy_table_dispose_all(&self->parameters, (psy_fp_disposefunc)
		psy_audio_lv2param_dispose);	
}

void clearparams(psy_audio_LV2Plugin* self)
{
	psy_table_dispose_all(&self->parameters, (psy_fp_disposefunc)
		psy_audio_lv2param_dispose);	
	psy_table_init(&self->parameters);
}

bool psy_audio_lv2plugin_test(const char* path, psy_audio_MachineInfo* rv,
	uintptr_t shell_index)
{
	bool success = FALSE;
	
	if (path && strcmp(path, "") != 0) {
		// psy_Library library;
		LilvWorld*         world;
		const LilvPlugin*  plugin;
		const LilvPlugins* plugins;
		LilvInstance*      instance;
		LilvNode*          uri;
		LilvNode*          name;
		LilvIter*          it;
		uintptr_t          i;
		char               path_with_slash[4096];
		
		world = lilv_world_new();
		psy_snprintf(path_with_slash, 4096, "%s"psy_SLASHSTR, path);
		uri = lilv_new_file_uri(world, NULL, path_with_slash);
		if (!uri) {
			lilv_world_free(world);
			return FALSE;
		}
		lilv_world_load_bundle(world, uri);
		lilv_world_load_specifications(world);
		lilv_world_load_plugin_classes(world);
		plugins = lilv_world_get_all_plugins(world);
		if (!plugins) {
			lilv_world_free(world);
			return FALSE;
		}
		i = 0;
		for (it = lilv_plugins_begin(plugins);
				!lilv_plugins_is_end(plugins, it);
				it = lilv_plugins_next(plugins, it),
				++i) {
			if (i == shell_index) {
				plugin = lilv_plugins_get(plugins, it);
				if (plugin) {										
					machineinfo_set(
						rv,
						lilv_node_as_string(lilv_plugin_get_author_name(plugin)),
						"", //const char* command,
						0, // int flags,
						psy_audio_MACHMODE_FX, //int mode,
						lilv_node_as_string(lilv_plugin_get_name(plugin)),
						lilv_node_as_string(lilv_plugin_get_name(plugin)),
						1, // short apiversion,
						0, // short plugversion,
						psy_audio_LV2,
						path,
						shell_index,
						"",
						lilv_node_as_string(lilv_plugin_get_project(plugin)),
						"",
						psy_INDEX_INVALID,
						"");
						success = TRUE;
				}
				break;
			}
		}		
		lilv_world_free(world);
	}
	return success;
}

void getnuminputsoutputs(psy_audio_LV2Plugin* self)
{
	assert(self);	
	assert(self->plugin);
		
	self->numInputs = psy_max(2, lilv_plugin_get_num_ports_of_class(
		self->plugin, self->input_port_class, NULL));
	self->numOutputs = psy_max(2, lilv_plugin_get_num_ports_of_class(
		self->plugin, self->output_port_class, NULL));
}

void preparebuffers(psy_audio_LV2Plugin* self, psy_audio_BufferContext* bc)
{
	//we're passing addresses within the vector to the plugin.. let's be sure they don't move around	
	int indexinput = 0;
	int indexoutput = 0;
	int indexpar = 0;
	uint32_t lPortIndex;
	
	assert(self->plugin);
	assert(self->instance);

	for (lPortIndex = 0; lPortIndex <  lilv_plugin_get_num_ports(self->plugin);
			++lPortIndex) {
		const LilvPort * port;		
		
		port = lilv_plugin_get_port_by_index(self->plugin, lPortIndex);
		if (port) {
			if (lilv_port_is_a(self->plugin, port, self->input_port_class)) {
				if (indexinput < 2) {
					lilv_instance_connect_port(self->instance,
						lPortIndex, psy_audio_buffer_at(bc->output,
						indexinput));
				//inportmap[indexinput] = lPortIndex;
				}
				indexinput++;						
			}
			if (lilv_port_is_a(self->plugin, port, self->output_port_class)) {
				if (indexoutput < 2) {
					lilv_instance_connect_port(self->instance,
						lPortIndex, psy_audio_buffer_at(bc->output,
						indexoutput));
				}
				indexoutput++;				
			}
		}
	}	
}

void generateaudio(psy_audio_LV2Plugin* self, psy_audio_BufferContext* bc)
{

	assert(self);	
	
	if (bc->numsamples > 0) {
		preparebuffers(self, bc);				
		lilv_instance_run(self->instance, (uint32_t)bc->numsamples);	
	}
}

void processevents(psy_audio_LV2Plugin* self, psy_audio_BufferContext* bc)
{		
	
}

void stop(psy_audio_LV2Plugin* self)
{		
}

const psy_audio_MachineInfo* info(psy_audio_LV2Plugin* self)
{	
	assert(self);

	return self->plugininfo;
}

uintptr_t numinputs(psy_audio_LV2Plugin* self)
{
	assert(self);

	return psy_max(2, self->numInputs);
}

uintptr_t numoutputs(psy_audio_LV2Plugin* self)
{
	assert(self);

	return psy_max(2, self->numOutputs);
}

psy_audio_MachineParam* parameter(psy_audio_LV2Plugin* self, uintptr_t param)
{
	assert(self);

	return (psy_audio_MachineParam*)psy_table_at(&self->parameters, param);
}

uintptr_t numparameters(psy_audio_LV2Plugin* self)
{
	assert(self);

	return psy_table_size(&self->parameters);
}

unsigned int numparametercols(psy_audio_LV2Plugin* self)
{
	assert(self);

	return 6;
}

int loadspecific(psy_audio_LV2Plugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	/*uint32_t size;
	unsigned char program;
	int status;

	assert(self);

	if (status = psyfile_read(songfile->file, &size, sizeof(size))) {
		return status;
	}
	if(size) {
		uint32_t count;

		if (status = psyfile_read(songfile->file, &program, sizeof program)) {
			return status;
		}
		if (status = psyfile_read(songfile->file, &count, sizeof count)) {
			return status;
		}
		size -= sizeof(program) + sizeof(count) + sizeof(float) * count;
		if (!size) {
			if (program < psy_audio_vstinterface_numprograms(&self->mi)) {
				uint32_t i;				
				
				psy_audio_vstinterface_beginprogram(&self->mi);
				psy_audio_vstinterface_setprogram(&self->mi, program);
				for(i = 0; i < count; ++i) {
					float temp;
				
					if (status = psyfile_read(songfile->file, &temp,
							sizeof(temp))) {
						return status;
					}
					psy_audio_vstinterface_setparametervalue(&self->mi, i,
						temp);
				}
				psy_audio_vstinterface_endprogram(&self->mi);				
			}
		} else {			
			psy_audio_vstinterface_beginprogram(&self->mi);
			psy_audio_vstinterface_setprogram(&self->mi, program);
			psy_audio_vstinterface_endprogram(&self->mi);
			if (psyfile_skip(songfile->file, sizeof(float) * count) == -1) {
				return PSY_ERRFILE;
			}
			if (psy_audio_vstinterface_hasprogramchunk(&self->mi)) {
				char * data;
				
				data = (char*)malloc(size);
				// Number of parameters
				if (status = psyfile_read(songfile->file, data, size)) {
					free(data);
					return status;
				}
				psy_audio_vstinterface_setchunkdata(&self->mi, FALSE, data,
					size);
				free(data);				
			} else {
				// there is a data chunk, but this machine does not want one.
				if (psyfile_skip(songfile->file, size) == -1) {
					return PSY_ERRFILE;
				}
				return PSY_OK;
			}
		}	
	}
	disposeparameters(self);
	psy_table_init(&self->parameters);
	initparameters(self);*/
	return PSY_OK;
}

int savespecific(psy_audio_LV2Plugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{	
	/*uint32_t count;
	uint8_t program;
	uint32_t size;
	uintptr_t chunksize;
	char* data;
	int status;

	assert(self);

	program = 0;
	chunksize = 0;	
	count = (uint32_t)numparameters(self);
	size = sizeof(program) + sizeof(count);
	data = psy_audio_vstinterface_chunkdata(&self->mi, FALSE, &chunksize);
	if (data) {
		count = 0;		
		size += (uint32_t)chunksize;
	} else {
		size += sizeof(float) * count;
	}
	if (status = psyfile_write(songfile->file, &size, sizeof(size))) {
		return status;
	}
	program = psy_audio_vstinterface_program(&self->mi);
	if (status = psyfile_write(songfile->file, &program, sizeof(program))) {
		return status;
	}
	if (status = psyfile_write(songfile->file, &count, sizeof count)) {
		return status;
	}

	if (data) {
		if (status = psyfile_write(songfile->file, data, (uint32_t)chunksize)) {
			return status;
		}
	} else {
		uint32_t i;

		for (i = 0; i < count; ++i) {
			float temp;
			
			temp = psy_audio_vstinterface_parametervalue(&self->mi, i);
			if (status = psyfile_write(songfile->file, &temp, sizeof(float))) {
				return status;
			}
		}
	}*/
	return PSY_OK;
}

int haseditor(psy_audio_LV2Plugin* self)
{
	assert(self);

	return 0; //psy_audio_vstinterface_haseditor(&self->mi);	
}

void seteditorhandle(psy_audio_LV2Plugin* self, void* handle)
{		
	assert(self);

	if (self->editorhandle && handle == NULL) {
		self->editorhandle = NULL;
		// psy_audio_vstinterface_closeeditor(&self->mi, handle);		
	} else {
		self->editorhandle = handle;
		// psy_audio_vstinterface_openeditor(&self->mi, handle);		
	}
}

void editorsize(psy_audio_LV2Plugin* self, double* width, double* height)
{
	struct ERect* r = 0;

	assert(self);

	/*if (self->mi.effect) {

		self->mi.effect->dispatcher(self->mi.effect, effEditGetRect, 0, 0, &r, 0);
		if (r != 0) {
			*width = (double)(r->right - r->left);
			*height = (double)(r->bottom - r->top);
			return;
		}
	}*/
	*width = 0.0;
	*height = 0.0;	
}

void editoridle(psy_audio_LV2Plugin* self)
{
	assert(self);

	if(self->editorhandle) {
		// psy_audio_vstinterface_editoridle(&self->mi);		
	}
}

void programname(psy_audio_LV2Plugin* self, uintptr_t bnkidx, uintptr_t prgidx, char* val)
{
	assert(self);

	if (prgidx != psy_INDEX_INVALID) {
		// self->mi.effect->dispatcher(self->mi.effect, effGetProgramNameIndexed,
		//	(VstInt32)(bnkidx * 128 + prgidx), -1, val, 0);
		val[0] = '\0';
	} else {
		val[0] = '\0';
	}
}

uintptr_t numprograms(psy_audio_LV2Plugin* self)
{
	assert(self);

	return 0; // psy_audio_vstinterface_numprograms(&self->mi);	
}

void setcurrprogram(psy_audio_LV2Plugin* self, uintptr_t prgidx)
{
	assert(self);

	// psy_audio_vstinterface_setprogram(&self->mi, prgidx);	
}

uintptr_t currprogram(psy_audio_LV2Plugin* self)
{
	assert(self);

	return 0; //psy_audio_vstinterface_program(&self->mi);	
}

void bankname(psy_audio_LV2Plugin* self, uintptr_t bnkidx, char* val)
{
	assert(self);

	if (bnkidx < numbanks(self)) {
		psy_snprintf(val, 256, "Internal %d", bnkidx + 1);
	} else {
		val[0] = '\0';
	}
}

uintptr_t numbanks(psy_audio_LV2Plugin* self)
{
	assert(self);

	return (numprograms(self) / 128) + 1;
}

void setcurrbank(psy_audio_LV2Plugin* self, uintptr_t bnkidx)
{
	assert(self);

	setcurrprogram(self, bnkidx * 128 + currprogram(self));
}

uintptr_t currbank(psy_audio_LV2Plugin* self)
{
	assert(self);

	return currprogram(self) / 128;
}

void currentpreset(psy_audio_LV2Plugin* self, psy_audio_Preset* preset)
{	
	uintptr_t gbp;
	uintptr_t chunksize;
	void* ptr;

	assert(self);

	/*for (gbp = 0; gbp < numparameters(self); ++gbp) {
		psy_audio_MachineParam* param;

		param = parameter(self, gbp);
		if (param) {
			float value;
			
			value = psy_audio_machineparam_normvalue(param);
			psy_audio_preset_setvalue(preset, gbp, (intptr_t)(value * 0xFFFF));
			preset->isfloat = TRUE;
		}		
	}
	ptr = psy_audio_vstinterface_chunkdata(&self->mi, TRUE, &chunksize);
	if (ptr) {
		psy_audio_preset_putdata(preset, (int)chunksize, ptr);
	}
	if (self->mi.effect) {
		preset->id = self->mi.effect->uniqueID;
		preset->magic = self->mi.effect->magic;
		preset->version = self->mi.effect->version;
	}*/
}

/*void lv2plugin_onfileselect(psy_audio_LV2Plugin* self,
	struct VstFileSelect* select)
{
	assert(self);

	if (!self->custommachine.machine.callback) {
		return;
	}
	switch (select->command) {
	case kVstFileLoad:
		self->custommachine.machine.callback->vtable->fileselect_load(
			self->custommachine.machine.callback, NULL, NULL);
		break;
	case kVstFileSave:
		self->custommachine.machine.callback->vtable->fileselect_save(
			self->custommachine.machine.callback, NULL, NULL);
		break;
	case kVstDirectorySelect:
		self->custommachine.machine.callback->vtable->fileselect_directory(
			self->custommachine.machine.callback);
		break;
	default:
		break;
	}
}*/


const char* auxcolumnname(psy_audio_LV2Plugin* self, uintptr_t index)
{
	if (index < 16) {
		return MIDI_CHAN_NAMES[index];
	}
	return "";
}

uintptr_t numauxcolumns(psy_audio_LV2Plugin* self)
{
	return 16;
}


#endif /* PSYCLE_USE_LV2 */

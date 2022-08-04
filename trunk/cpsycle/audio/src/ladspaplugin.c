/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "ladspaplugin.h"
/* local */
#include "pattern.h"
#include "plugin_interface.h"
#include "songio.h"
/* std */
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* platform */
#include "../../detail/portable.h"


/* implementation */
static psy_audio_Machine* clone(psy_audio_LadspaPlugin*);
static int hostevent(psy_audio_LadspaPlugin*, int const eventNr, int val1, float val2);
static void generateaudio(psy_audio_LadspaPlugin*, psy_audio_BufferContext*);
static void seqtick(psy_audio_LadspaPlugin*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void stop(psy_audio_LadspaPlugin*);
static void newline(psy_audio_LadspaPlugin*);
static psy_audio_MachineInfo* info(psy_audio_LadspaPlugin*);
static psy_audio_MachineParam* parameter(psy_audio_LadspaPlugin*, uintptr_t param);
static int parametertype(psy_audio_LadspaPlugin* self, uintptr_t par);
static uintptr_t numparametercols(psy_audio_LadspaPlugin*);
static uintptr_t numparameters(psy_audio_LadspaPlugin*);
static void dispose(psy_audio_LadspaPlugin*);
static uintptr_t numinputs(psy_audio_LadspaPlugin*);
static uintptr_t numoutputs(psy_audio_LadspaPlugin*);
static int loadspecific(psy_audio_LadspaPlugin*, psy_audio_SongFile*,
	uintptr_t slot);
static int savespecific(psy_audio_LadspaPlugin*, psy_audio_SongFile*,
	uintptr_t slot);
static void setcallback(psy_audio_LadspaPlugin*, psy_audio_MachineCallback);

LADSPA_Handle instantiate(const LADSPA_Descriptor* psDescriptor);
void preparebuffers(psy_audio_LadspaPlugin*, psy_audio_BufferContext*);
void prepareparams(psy_audio_LadspaPlugin*);
static void clearparams(psy_audio_LadspaPlugin*);

static psy_dsp_amp_range_t amprange(psy_audio_LadspaPlugin* self)
{
	return PSY_DSP_AMP_RANGE_VST;
}
/* vtable */
static MachineVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_audio_LadspaPlugin* self)
{
	if (!vtable_initialized) {
		vtable = *(psy_audio_ladspaplugin_base(self)->vtable);
		vtable.setcallback =
			(fp_machine_setcallback)
			setcallback;
		vtable.clone =
			(fp_machine_clone)
			clone;
		vtable.hostevent =
			(fp_machine_hostevent)
			hostevent;
		vtable.seqtick =
			(fp_machine_seqtick)
			seqtick;
		vtable.stop =
			(fp_machine_stop)
			stop;
		vtable.newline =
			(fp_machine_newline)
			newline;
		vtable.info =
			(fp_machine_info)
			info;
		vtable.numparametercols =
			(fp_machine_numparametercols)
			numparametercols;
		vtable.numparameters =
			(fp_machine_numparameters)
			numparameters;
		vtable.parameter =
			(fp_machine_parameter)
			parameter;
		vtable.dispose =
			(fp_machine_dispose)
			dispose;
		vtable.generateaudio =
			(fp_machine_generateaudio)
			generateaudio;
		vtable.numinputs =
			(fp_machine_numinputs)
			numinputs;
		vtable.numoutputs =
			(fp_machine_numoutputs)
			numoutputs;
		vtable.loadspecific =
			(fp_machine_loadspecific)
			loadspecific;
		vtable.savespecific =
			(fp_machine_savespecific)
			savespecific;
		vtable.amprange =
			(fp_machine_amprange)
			amprange;
		vtable_initialized = TRUE;
	}
	psy_audio_ladspaplugin_base(self)->vtable = &vtable;
}

/* implementation */
void psy_audio_ladspaplugin_init(psy_audio_LadspaPlugin* self,
	psy_audio_MachineCallback* callback, const char* path,
	uintptr_t shellidx)
{
	LADSPA_Descriptor_Function pfDescriptorFunction;

	psy_audio_custommachine_init(&self->custommachine, callback);
	vtable_init(self);	
	psy_audio_machine_setcallback(&self->custommachine.machine,
		callback);
	psy_table_init(&self->parameters);	
	psy_table_init(&self->inportmap);
	psy_table_init(&self->outportmap);
	psy_library_init(&self->library);
	psy_audio_ladspainterface_init(&self->mi, NULL, NULL);
	psy_library_load(&self->library, path);	
	self->plugininfo = 0;	
	pfDescriptorFunction = (LADSPA_Descriptor_Function)
		psy_library_functionpointer(&self->library, "ladspa_descriptor");					
	if (!pfDescriptorFunction) {
		psy_library_dispose(&self->library);
	} else {
		/*Step three: Get the descriptor of the selected plugin (a shared library can have
		several plugins*/
		const LADSPA_Descriptor* psDescriptor = pfDescriptorFunction((uint32_t)shellidx);
		if (psDescriptor) {
			LADSPA_Handle handle = instantiate(psDescriptor);
			if (handle) {
				self->handle = handle;
				self->psDescriptor = psDescriptor;
				psy_audio_ladspainterface_init(&self->mi, psDescriptor,
					handle);
				self->plugininfo = machineinfo_allocinit();								
				machineinfo_set(self->plugininfo,
					psDescriptor->Maker,
					"", //const char* command,
					0, // int flags,
					psy_audio_MACHMODE_FX,
					psDescriptor->Name,
					psDescriptor->Name, //const char* shortname,
					1, // short apiversion,
					0, // short plugversion,
					psy_audio_LADSPA,
					path, //const char* modulepath,
					shellidx,
					"",
					"LADSPA",
					"");
				// TODO: for LADSPA, it is more correct to use psDescriptor->Label to identify it.						
				psy_audio_machine_seteditname(psy_audio_ladspaplugin_base(self),
					self->plugininfo->shortname);
				prepareparams(self);
				psy_audio_ladspainterface_activate(&self->mi);				
			}
		}
	}	
}

LADSPA_Handle instantiate(const LADSPA_Descriptor* psDescriptor)
{
	if (LADSPA_IS_INPLACE_BROKEN(psDescriptor->Properties)) return 0;
	// Step four: Create (instantiate) the plugin, so that we can use it.
	return psDescriptor->instantiate(psDescriptor, 44100); //mcallback_->timeInfo().sampleRate());
}

void dispose(psy_audio_LadspaPlugin* self)
{	
	psy_audio_ladspainterface_deactivate(&self->mi);
	psy_audio_ladspainterface_cleanup(&self->mi);	
	psy_table_dispose_all(&self->parameters,
		(psy_fp_disposefunc)		
		psy_audio_ladspaparam_dispose);	
	psy_table_dispose(&self->inportmap);
	psy_table_dispose(&self->outportmap);
	if (self->library.module != 0 && self->handle) {
		// mi_dispose(self->mi);
		psy_library_dispose(&self->library);
		self->handle = 0;
	}
	if (self->plugininfo) {
		machineinfo_dispose(self->plugininfo);
		free(self->plugininfo);
		self->plugininfo = 0;
	}	
	psy_audio_custommachine_dispose(&self->custommachine);
}

psy_audio_Machine* clone(psy_audio_LadspaPlugin* self)
{
	psy_audio_LadspaPlugin* rv;

	rv = malloc(sizeof(psy_audio_LadspaPlugin));
	if (rv) {
		psy_audio_ladspaplugin_init(rv, self->custommachine.machine.callback,
			self->library.path, self->plugininfo->shellidx);
	}
	return rv ? &rv->custommachine.machine : 0;
}

int psy_audio_plugin_ladspa_test(const char* path, psy_audio_MachineInfo*
	machine_info, uintptr_t shellidx)
{
	int rv = 0;

	if (path && strcmp(path, "") != 0) {
		LADSPA_Descriptor_Function pfDescriptorFunction;
		psy_Library library;

		psy_library_init(&library);
		psy_library_load(&library, path);
		pfDescriptorFunction = (LADSPA_Descriptor_Function)
			psy_library_functionpointer(&library, "ladspa_descriptor");		
		if (pfDescriptorFunction != NULL) {
			const LADSPA_Descriptor* psDescriptor;
			
			psDescriptor = pfDescriptorFunction((uint32_t)shellidx);
			if (psDescriptor != NULL) {
				machineinfo_set(machine_info,
					psDescriptor->Maker,
					"", //const char* command,
					0, // int flags,
					psy_audio_MACHMODE_FX, //int mode,
					psDescriptor->Name, // const char* name,
					psDescriptor->Label, //const char* shortname,
					1, // short apiversion,
					0, // short plugversion,
					psy_audio_LADSPA, // int type,
					path, //const char* modulepath,
					shellidx,
					"",
					"LADSPA",
					"");
				// TODO: for LADSPA, it is more correct to use psDescriptor->Label to identify it.										
				rv = 1;
			}
		}
		psy_library_dispose(&library);
	}
	return rv;
}

void prepareparams(psy_audio_LadspaPlugin* self)
{
	//we're passing addresses within the vector to the plugin.. let's be sure they don't move around	
	int indexinput = 0;
	int indexoutput = 0;
	int indexpar = 0;
	unsigned int lPortIndex;
	unsigned int index;

	clearparams(self);
	index = 0;
	for (lPortIndex = 0; lPortIndex < self->psDescriptor->PortCount;
			++lPortIndex) {
		LADSPA_PortDescriptor iPortDescriptor =
		self->psDescriptor->PortDescriptors[lPortIndex];
		if (LADSPA_IS_PORT_CONTROL(iPortDescriptor)) {
			psy_audio_LadspaParam* param;
		
			param = psy_audio_ladspaparam_allocinit(
				iPortDescriptor,
				self->psDescriptor->PortRangeHints[lPortIndex],
				self->psDescriptor->PortNames[lPortIndex],
				index,
				lPortIndex);
			psy_table_insert(&self->parameters, index, (void*)
				param);
			++index;
			self->psDescriptor->connect_port(self->handle,
				lPortIndex, &param->value_);
//				ladspaparam_valueaddress(parameter));
			indexpar++;			
		}
	}	
}

void seqtick(psy_audio_LadspaPlugin* self, uintptr_t channel,
	const psy_audio_PatternEvent* ev)
{	
}

void stop(psy_audio_LadspaPlugin* self)
{
}

void generateaudio(psy_audio_LadspaPlugin* self,
	psy_audio_BufferContext* bc)
{
	preparebuffers(self, bc);
	psy_audio_ladspainterface_run(&self->mi, (uint32_t)bc->numsamples);
}

void preparebuffers(psy_audio_LadspaPlugin* self,
	psy_audio_BufferContext* bc)
{
	//we're passing addresses within the vector to the plugin.. let's be sure they don't move around	
	int indexinput = 0;
	int indexoutput = 0;
	int indexpar = 0;
	unsigned int lPortIndex;

	for (lPortIndex = 0; lPortIndex < self->psDescriptor->PortCount; ++lPortIndex) {
		LADSPA_PortDescriptor iPortDescriptor = self->psDescriptor->PortDescriptors[lPortIndex];
		if (LADSPA_IS_PORT_AUDIO(iPortDescriptor))
		{
			if (LADSPA_IS_PORT_INPUT(iPortDescriptor)) {
				if (indexinput < 2) {
					self->psDescriptor->connect_port(self->handle,
						lPortIndex, psy_audio_buffer_at(bc->output,
						indexinput));
				//inportmap[indexinput] = lPortIndex;
				}
				indexinput++;
				
			}
			else if (LADSPA_IS_PORT_OUTPUT(iPortDescriptor)) {
				if (indexoutput < 2) {
					self->psDescriptor->connect_port(self->handle, lPortIndex,
						psy_audio_buffer_at(bc->output, indexoutput));
				}
				indexoutput++;
			}
		}
	}	
}

int hostevent(psy_audio_LadspaPlugin* self, int const eventNr, int val1, float val2)
{	
	return 0;
}

void newline(psy_audio_LadspaPlugin* self)
{	
}

psy_audio_MachineInfo* info(psy_audio_LadspaPlugin* self)
{
	return self->plugininfo;
}

uintptr_t numinputs(psy_audio_LadspaPlugin* self)
{
	return (info(self) ?
		((self->plugininfo->mode == psy_audio_MACHMODE_FX) ? 2 : 0)
		: 0);
}

uintptr_t numoutputs(psy_audio_LadspaPlugin* self)
{
	return info(self) ? 2 : 0;
}

psy_audio_MachineParam* parameter(psy_audio_LadspaPlugin* self, uintptr_t param)
{
	assert(self);

	return (psy_audio_MachineParam*)psy_table_at(&self->parameters, param);
}

void setcallback(psy_audio_LadspaPlugin* self, psy_audio_MachineCallback callback)
{	
}

int loadspecific(psy_audio_LadspaPlugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	
	// size of whole structure
	psyfile_read(songfile->file, &size, sizeof(size));
	if (size)
	{
		uint32_t numparams;
		uint32_t i;
		// size of vars
		psyfile_read(songfile->file, &numparams, sizeof(numparams));
		for (i = 0; i < numparams; ++i) {
			float temp;
//			LadspaParam* param;

			psyfile_read(songfile->file, &temp, sizeof(temp));			
//			param = valueat(self, i);
//			if (param) {
//				ladspaparam_setrawvalue(param, temp);
//			}
		}
	}
	return PSY_OK;
}

void clearparams(psy_audio_LadspaPlugin* self)
{
	psy_table_dispose_all(&self->parameters, (psy_fp_disposefunc)
		psy_audio_ladspaparam_dispose);	
	psy_table_init(&self->parameters);
}

int savespecific(psy_audio_LadspaPlugin* self,
	psy_audio_SongFile* songfile, uintptr_t slot)
{
	/*uint32_t count;
	uint32_t size;
	uint32_t i;
	
	count = (uint32_t)psy_audio_machine_numparameters(psy_audio_ladspaplugin_base(self));
	size = sizeof(count) + sizeof(uint32_t) * count;
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write(songfile->file, &count, sizeof(count));
	for (i = 0; i < count; ++i) {
		float temp = 0.f;
		LadspaParam* param;

		param = valueat(self, i);
		if (param) {
			temp = ladspaparam_rawvalue(param);
		}		
		psyfile_write(songfile->file, &temp, sizeof temp);
	}*/
	return PSY_OK;
}

uintptr_t numparametercols(psy_audio_LadspaPlugin* self)
{	
	return (numparameters(self) / 12) + 1;	
}

uintptr_t numparameters(psy_audio_LadspaPlugin* self)
{
	return psy_table_size(&self->parameters);
}

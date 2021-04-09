// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "ladspaplugin.h"
#include "pattern.h"
#include "plugin_interface.h"
#include "songio.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../../detail/portable.h"

	
///\todo: Improve the case where no min/max limit is given by the plugin (Example: the amp.so doesn't have a max value).
void ladspaparam_init(LadspaParam* self, LADSPA_PortDescriptor descriptor,LADSPA_PortRangeHint hint, const char *newname)
{
	self->descriptor_ = descriptor;
	self->hint_ = hint;
	self->portName_ = newname ? strdup(newname) : strdup("");
	self->rangeMultiplier_ = 1;
	self->integer_ = FALSE;
	self->logarithmic_ = FALSE;

	if (LADSPA_IS_HINT_TOGGLED(hint.HintDescriptor)) {
		self->minVal_= 0;
		self->maxVal_ = 1;
		self->integer_= TRUE;
	}
	else {
		if (LADSPA_IS_HINT_BOUNDED_BELOW(hint.HintDescriptor)) {
			self->minVal_ = hint.LowerBound;
		} else {
			self->minVal_ = 0;
		}
		if (LADSPA_IS_HINT_BOUNDED_ABOVE(hint.HintDescriptor)) {
			self->maxVal_ = hint.UpperBound;
		} else {
			self->maxVal_ = 1;
		}
		if (LADSPA_IS_HINT_SAMPLE_RATE(hint.HintDescriptor)) {
			self->maxVal_ *= 44100; // Global::player().SampleRate();
		}

		if ( LADSPA_IS_HINT_LOGARITHMIC(hint.HintDescriptor) ){
			self->logarithmic_ = TRUE;
			// rangeMultiplier_ =   9 / (maxVal_ - minVal_);
			self->rangeMultiplier_ = (float)  (exp(1.0)-1) / (self->maxVal_ - self->minVal_);
		}
		else if ( LADSPA_IS_HINT_INTEGER(hint.HintDescriptor) ){
			self->integer_ = TRUE;
		} else {
			self->rangeMultiplier_ =   65535.0f / (self->maxVal_ - self->minVal_);
		}
	}
	ladspaparam_setdefault(self);
	//std::cout << "min/max/def" << minVal_ << "/" << maxVal_ << "/" << value_ << std::endl;
}

void ladspaparam_dispose(LadspaParam* self)
{
	free(self->portName_);
	self->portName_ = 0;
}

LadspaParam* ladspaparam_alloc(void)
{
	return (LadspaParam*)malloc(sizeof(LadspaParam));
}

LadspaParam* ladspaparam_allocinit(LADSPA_PortDescriptor descriptor, LADSPA_PortRangeHint hint, const char* newname)
{
	LadspaParam* rv;

	rv = ladspaparam_alloc();
	if (rv) {
		ladspaparam_init(rv, descriptor, hint, newname);
	}
	return rv;
}


void ladspaparam_setdefault(LadspaParam* self)
{
	LADSPA_Data fDefault=0.0f;
	uint32_t samplerate;

	samplerate = 44100;
	switch (self->hint_.HintDescriptor & LADSPA_HINT_DEFAULT_MASK) {
	case LADSPA_HINT_DEFAULT_NONE:
		break;
	case LADSPA_HINT_DEFAULT_MINIMUM:
		fDefault = self->hint_.LowerBound * (float)((LADSPA_IS_HINT_SAMPLE_RATE(self->hint_.HintDescriptor)) ? 
			(float)samplerate : 1.0f);
		break;
	case LADSPA_HINT_DEFAULT_LOW:
		if (LADSPA_IS_HINT_LOGARITHMIC(self->hint_.HintDescriptor)) {
			fDefault 
			= (LADSPA_Data) exp(log(self->hint_.LowerBound) * 0.75
			+ log(self->hint_.UpperBound) * 0.25) * (float)((LADSPA_IS_HINT_SAMPLE_RATE(self->hint_.HintDescriptor)) ?
				(float)samplerate : 1.0f);
		}
		else {
			fDefault 
			= (LADSPA_Data) (self->hint_.LowerBound * 0.75
			+ self->hint_.UpperBound * 0.25)* (float)((LADSPA_IS_HINT_SAMPLE_RATE(self->hint_.HintDescriptor)) ?
				(float)samplerate : 1.0f);
		}
		break;
	case LADSPA_HINT_DEFAULT_MIDDLE:
		if (LADSPA_IS_HINT_LOGARITHMIC(self->hint_.HintDescriptor)) {
			fDefault 
			= (LADSPA_Data) sqrt(self->hint_.LowerBound
			* self->hint_.UpperBound) * (float)((LADSPA_IS_HINT_SAMPLE_RATE(self->hint_.HintDescriptor)) ?
				(float)samplerate : 1.0f);
		}
		else {
			fDefault 
			= (LADSPA_Data) (0.5 * (self->hint_.LowerBound
			+ self->hint_.UpperBound) * (float)((LADSPA_IS_HINT_SAMPLE_RATE(self->hint_.HintDescriptor)) ?
				(float)samplerate : 1.0f));
		}
		break;
	case LADSPA_HINT_DEFAULT_HIGH:
		if (LADSPA_IS_HINT_LOGARITHMIC(self->hint_.HintDescriptor)) {
			fDefault 
			= (LADSPA_Data) exp(log(self->hint_.LowerBound) * 0.25
			+ log(self->hint_.UpperBound) * 0.75) * (float)((LADSPA_IS_HINT_SAMPLE_RATE(self->hint_.HintDescriptor)) ?
				(float)samplerate : 1.0f);
		}
		else {
			fDefault 
			= (LADSPA_Data) (self->hint_.LowerBound * 0.25
			+ self->hint_.UpperBound * 0.75) * (float)((LADSPA_IS_HINT_SAMPLE_RATE(self->hint_.HintDescriptor)) ?
				(float)samplerate : 1.0f);
		}
		break;
	case LADSPA_HINT_DEFAULT_MAXIMUM:
		fDefault = (LADSPA_Data) self->hint_.UpperBound * (float)((LADSPA_IS_HINT_SAMPLE_RATE(self->hint_.HintDescriptor)) ?
			(float)samplerate : 1.0f);
		break;
	case LADSPA_HINT_DEFAULT_0:
		fDefault=0.0f;
		break;
	case LADSPA_HINT_DEFAULT_1:
		fDefault=1.0f;
		break;
	case LADSPA_HINT_DEFAULT_100:
		fDefault=100.0f;
		break;
	case LADSPA_HINT_DEFAULT_440:
		fDefault=440.0f;
		break;
	default:
		break;
	}
	self->value_ = fDefault;
}
		
int LadspaParam_value(LadspaParam* self)
{ 
	return (int) ((self->integer_) ? self->value_ :
		// (logarithmic_) ? log10(1+((value_-minVal_)*rangeMultiplier_))*65535.0f:
		(self->logarithmic_) ?  log(1 + ((self->value_ - self->minVal_) * self->rangeMultiplier_)) * 65535.0f:
		(self->value_- self->minVal_)* self->rangeMultiplier_);
}

void LadspaParam_setValue(LadspaParam* self, int data)
{
	self->value_ = (LADSPA_Data)( (self->integer_) ? data :
		// (logarithmic_) ? minVal_ + (pow(10, data/65535.0f)-1)/ rangeMultiplier_ :
		(self->logarithmic_) ? self->minVal_ + (exp(data / 65535.0f) - 1) / self->rangeMultiplier_ :
		self->minVal_+ (data/ self->rangeMultiplier_));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

static psy_audio_Machine* clone(psy_audio_LadspaPlugin*);
static int hostevent(psy_audio_LadspaPlugin*, int const eventNr, int val1, float val2);
static void generateaudio(psy_audio_LadspaPlugin*, psy_audio_BufferContext*);
static void seqtick(psy_audio_LadspaPlugin*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void stop(psy_audio_LadspaPlugin*);
static void newline(psy_audio_LadspaPlugin*);
static psy_audio_MachineInfo* info(psy_audio_LadspaPlugin*);
static int parametertype(psy_audio_LadspaPlugin* self, uintptr_t par);
static uintptr_t numparametercols(psy_audio_LadspaPlugin*);
static uintptr_t numparameters(psy_audio_LadspaPlugin*);
static void parameterrange(psy_audio_LadspaPlugin*, uintptr_t param, int* minval,
	int* maxval);
static int parameterlabel(psy_audio_LadspaPlugin*, char* txt, uintptr_t param);
static int parametername(psy_audio_LadspaPlugin*, char* txt, uintptr_t param);
static void parametertweak(psy_audio_LadspaPlugin*, uintptr_t param, float val);
static int parameterlabel(psy_audio_LadspaPlugin*, char* txt, uintptr_t param);
static int parametername(psy_audio_LadspaPlugin*, char* txt, uintptr_t param);
static int describevalue(psy_audio_LadspaPlugin*, char* txt, uintptr_t param,
	int value);
static float parametervalue(psy_audio_LadspaPlugin*, uintptr_t param);
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
LadspaParam* valueat(psy_audio_LadspaPlugin*, uintptr_t index);
void prepareparams(psy_audio_LadspaPlugin*);
static void clearparams(psy_audio_LadspaPlugin*);

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_LadspaPlugin* self)
{
	if (!vtable_initialized) {
		vtable = *(psy_audio_ladspaplugin_base(self)->vtable);
		vtable.clone = (fp_machine_clone)clone;
		vtable.hostevent = (fp_machine_hostevent)hostevent;
		vtable.seqtick = (fp_machine_seqtick)seqtick;
		vtable.stop = (fp_machine_stop)stop;
		vtable.newline = (fp_machine_newline)
			newline;
		vtable.info = (fp_machine_info)info;
		vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		vtable.numparameters = (fp_machine_numparameters)numparameters;
		vtable.dispose = (fp_machine_dispose)dispose;
		vtable.generateaudio = (fp_machine_generateaudio)generateaudio;
		vtable.numinputs = (fp_machine_numinputs)numinputs;
		vtable.numoutputs = (fp_machine_numoutputs)numoutputs;
		vtable.loadspecific = (fp_machine_loadspecific)loadspecific;
		vtable.savespecific = (fp_machine_savespecific)savespecific;
		vtable.setcallback = (fp_machine_setcallback)setcallback;
		vtable_initialized = 1;
	}
}

void psy_audio_ladspaplugin_init(psy_audio_LadspaPlugin* self, psy_audio_MachineCallback* callback,
	const char* path, uintptr_t shellidx)
{
	LADSPA_Descriptor_Function pfDescriptorFunction;

	psy_audio_custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	psy_audio_ladspaplugin_base(self)->vtable = &vtable;
	psy_table_init(&self->values_);
	psy_table_init(&self->inportmap);
	psy_table_init(&self->outportmap);
	psy_library_init(&self->library);
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
				self->pluginHandle = handle;
				self->psDescriptor = psDescriptor;					
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
					"LADSPA");
				// TODO: for LADSPA, it is more correct to use psDescriptor->Label to identify it.						
				psy_audio_machine_seteditname(psy_audio_ladspaplugin_base(self),
					self->plugininfo->ShortName);
				prepareparams(self);
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
	psy_table_dispose(&self->values_);
	psy_table_dispose(&self->inportmap);
	psy_table_dispose(&self->outportmap);
	if (self->library.module != 0 && self->pluginHandle) {
		// mi_dispose(self->mi);
		psy_library_dispose(&self->library);
		self->pluginHandle = 0;
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

int psy_audio_plugin_ladspa_test(const char* path, psy_audio_MachineInfo* info, uintptr_t shellidx)
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
				machineinfo_set(info,
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
					"LADSPA");
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

	clearparams(self);
	for (lPortIndex = 0; lPortIndex < self->psDescriptor->PortCount; lPortIndex++) {
		LADSPA_PortDescriptor iPortDescriptor = self->psDescriptor->PortDescriptors[lPortIndex];
		if (LADSPA_IS_PORT_CONTROL(iPortDescriptor)) {
			LadspaParam* parameter = ladspaparam_allocinit(iPortDescriptor,
				self->psDescriptor->PortRangeHints[lPortIndex],
				self->psDescriptor->PortNames[lPortIndex]);			
			psy_table_insert(&self->values_, indexpar, parameter);
			self->psDescriptor->connect_port(self->pluginHandle, lPortIndex,
				ladspaparam_valueaddress(parameter));
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

void generateaudio(psy_audio_LadspaPlugin* self, psy_audio_BufferContext* bc)
{
	preparebuffers(self, bc);
	self->psDescriptor->run(self->pluginHandle, (uint32_t)bc->numsamples);	
}

void preparebuffers(psy_audio_LadspaPlugin* self, psy_audio_BufferContext* bc)
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
				self->psDescriptor->connect_port(self->pluginHandle, lPortIndex,
					psy_audio_buffer_at(bc->output, indexinput));
				//inportmap[indexinput] = lPortIndex;
				indexinput++;
			}
			else if (LADSPA_IS_PORT_OUTPUT(iPortDescriptor)) {
				self->psDescriptor->connect_port(self->pluginHandle, lPortIndex,
					psy_audio_buffer_at(bc->output, indexoutput));
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

void parametertweak(psy_audio_LadspaPlugin* self, uintptr_t param, float val)
{
	LadspaParam* ladspa_param;

	ladspa_param = valueat(self, param);
	if (ladspa_param) {
		ladspaparam_setrawvalue(ladspa_param, val);
	}	
}

int describevalue(psy_audio_LadspaPlugin* self, char* txt, uintptr_t param,
	int value)
{
	LadspaParam* ladspa_param;

	ladspa_param = valueat(self, param);
	if (ladspa_param) {
		LADSPA_PortRangeHintDescriptor iHintDescriptor = ladspaparam_hint(ladspa_param);
		float value = ladspaparam_rawvalue(ladspa_param);
		if (LADSPA_IS_HINT_TOGGLED(iHintDescriptor))
		{
			strcpy(txt, (value > 0.0) ? "on" : "off");
			return 1;
		}
		else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor))
		{
			sprintf(txt, "%.0f", value);
			return 1;
		}
		else
		{
			sprintf(txt, "%.4f", value);
			return 1;
		}
	}
	return 0;
}

float parametervalue(psy_audio_LadspaPlugin* self, uintptr_t param)
{
	LadspaParam* ladspa_param;

	ladspa_param = valueat(self, param);
	if (ladspa_param) {
		return ladspaparam_rawvalue(ladspa_param);
	}
	return 0.f;
}

uintptr_t numinputs(psy_audio_LadspaPlugin* self)
{
	return info(self) ? (self->plugininfo->mode == psy_audio_MACHMODE_FX ? 2 : 0) : 0;
}

uintptr_t numoutputs(psy_audio_LadspaPlugin* self)
{
	return info(self) ? 2 : 0;
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
			LadspaParam* param;

			psyfile_read(songfile->file, &temp, sizeof(temp));			
			param = valueat(self, i);
			if (param) {
				ladspaparam_setrawvalue(param, temp);
			}
		}
	}
	return PSY_OK;
}

LadspaParam* valueat(psy_audio_LadspaPlugin* self, uintptr_t index)
{	
	return (LadspaParam*)psy_table_at(&self->values_, index);
}

void clearparams(psy_audio_LadspaPlugin* self)
{
	psy_table_disposeall(&self->values_, (psy_fp_disposefunc)
		ladspaparam_dispose);	
	psy_table_init(&self->values_);
}

int savespecific(psy_audio_LadspaPlugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t count;
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
	}
	return PSY_OK;
}

int parametertype(psy_audio_LadspaPlugin* self, uintptr_t param)
{
	return MPF_STATE;
}

void parameterrange(psy_audio_LadspaPlugin* self, uintptr_t param, int* minval,
	int* maxval)
{
	LadspaParam* ladspa_param;

	ladspa_param = valueat(self, param);
	if (param) {
		*minval = (int) ladspaparam_minval(ladspa_param);
		*maxval = (int) ladspaparam_maxval(ladspa_param);
	} else {
		*minval = 0;
		*maxval = 0;
	}
}

int parameterlabel(psy_audio_LadspaPlugin* self, char* txt, uintptr_t param)
{	
	return 0;
}

int parametername(psy_audio_LadspaPlugin* self, char* txt, uintptr_t param)
{
	int rv = 0;
	LadspaParam* ladspa_param;

	ladspa_param = valueat(self, param);
	if (param) {
		strcpy(txt, ladspaparam_name(ladspa_param));
		rv = 1;
	}
	return rv;
}

uintptr_t numparametercols(psy_audio_LadspaPlugin* self)
{	
	return (numparameters(self) / 12) + 1;	
}

uintptr_t numparameters(psy_audio_LadspaPlugin* self)
{
	return psy_table_size(&self->values_);
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "lv2param.h"

#ifdef PSYCLE_USE_LV2

/* local */
#include "plugin_interface.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

/* prototypes */
static int lv2pluginparam_name(psy_audio_LV2Param*, char* text);
static int lv2pluginparam_label(psy_audio_LV2Param*, char* text);
static void lv2pluginparam_tweak(psy_audio_LV2Param*, float val);
static int lv2pluginparam_describe(psy_audio_LV2Param*, char* text);
static float lv2pluginparam_normvalue(psy_audio_LV2Param*);
static void lv2pluginparam_range(psy_audio_LV2Param*,
	intptr_t* minval, intptr_t* maxval);
static void lv2pluginparam_set_range_and_default(psy_audio_LV2Param*);

/* vtable */
static MachineParamVtable lv2pluginparam_vtable;
static bool lv2pluginparam_vtable_initialized = FALSE;

static void lv2pluginparam_vtable_init(psy_audio_LV2Param* self)
{
	if (!lv2pluginparam_vtable_initialized) {
		lv2pluginparam_vtable = *(self->custommachineparam.machineparam.vtable);
		lv2pluginparam_vtable.name =
			(fp_machineparam_name)
			lv2pluginparam_name;
		lv2pluginparam_vtable.label =
			(fp_machineparam_label)
			lv2pluginparam_label;
		lv2pluginparam_vtable.tweak =
			(fp_machineparam_tweak)
			lv2pluginparam_tweak;
		lv2pluginparam_vtable.normvalue =
			(fp_machineparam_normvalue)
			lv2pluginparam_normvalue;
		lv2pluginparam_vtable.range =
			(fp_machineparam_range)
			lv2pluginparam_range;
		lv2pluginparam_vtable.describe =
			(fp_machineparam_describe)
			lv2pluginparam_describe;
	}
	self->custommachineparam.machineparam.vtable = &lv2pluginparam_vtable;
}	

void psy_audio_lv2param_init(
	psy_audio_LV2Param* self,
	const LilvPort* port,
	const LilvPlugin* plugin,
	const char *newname,
	uintptr_t index,
	uintptr_t port_index)
{
	psy_audio_custommachineparam_init(&self->custommachineparam,
		"",
		"",
		MPF_STATE,
		0,
		0xFFFF);
	lv2pluginparam_vtable_init(self);	
	self->index = index;
	self->port_index = port_index;
	self->port = port;
	self->plugin = plugin;
	self->port_name = newname;
	lv2pluginparam_set_range_and_default(self);
}

void psy_audio_lv2param_dispose(psy_audio_LV2Param* self)
{
	psy_audio_custommachineparam_dispose(&self->custommachineparam);
}

psy_audio_LV2Param* psy_audio_lv2param_alloc(void)
{
	return (psy_audio_LV2Param*)malloc(sizeof(psy_audio_LV2Param));
}

psy_audio_LV2Param* psy_audio_lv2param_allocinit(
	const LilvPort* port,
	const LilvPlugin* plugin,
	const char *newname,	
	uintptr_t index,
	uintptr_t port_index)
{
	psy_audio_LV2Param* rv;

	rv = psy_audio_lv2param_alloc();
	if (rv) {
		psy_audio_lv2param_init(rv, port, plugin,
			newname, index, port_index);
	}
	return rv;
}

int lv2pluginparam_label(psy_audio_LV2Param* self, char* text)
{
	text[0] = '\0';
	// self->effect->dispatcher(self->effect, effGetParamLabel, (VstInt32)self->index, 0, text, 0);
	return *text != '\0';
}

int lv2pluginparam_name(psy_audio_LV2Param* self, char* text)
{
	text[0] = '\0';
	strcpy(text, self->port_name);
	// self->effect->dispatcher(self->effect, effGetParamName, (VstInt32)self->index, 0, text, 0);
	return *text != '\0';
}

void lv2pluginparam_tweak(psy_audio_LV2Param* self, float value)
{	
	float range;
	
	range = (self->maxval_ - self->minval_);	
	self->value_ = value * range + self->minval_;
}

int lv2pluginparam_describe(psy_audio_LV2Param* self, char* text)
{
	text[0] = '\0';
	
	/*if (LADSPA_IS_HINT_TOGGLED(self->hint.HintDescriptor)) {
		strcpy(text, (self->value_ > 0.0) ? "on" : "off");	
	} else if (LADSPA_IS_HINT_INTEGER(self->hint.HintDescriptor)) {
		sprintf(text, "%.0f", self->value_);		
	} else {
		sprintf(text, "%.4f", self->value_);
	}*/
	sprintf(text, "%.4f", self->value_);
	return *text != '\0';
}

float lv2pluginparam_normvalue(psy_audio_LV2Param* self)
{
	float range;
	
	range = (self->maxval_ - self->minval_);
	if (range != 0.0) {	
		return (self->value_ - self->minval_) / (range);
	}
	return 0.0;
}

void lv2pluginparam_range(psy_audio_LV2Param* self, intptr_t* minval,
	intptr_t* maxval)
{
	*minval = 0;
	*maxval = 0xFFFF;
}

void lv2pluginparam_set_range_and_default(psy_audio_LV2Param* self)
{
	 LilvNode* vdef;
	 LilvNode* vmin;
	 LilvNode* vmax;
	 
	 vdef = vmin = vmax = NULL;	 	 	
	 lilv_port_get_range(self->plugin, self->port, &vdef, &vmin, &vmax);
	 if (lilv_node_is_float(vmin)) {
		 self->minval_ = lilv_node_as_float(vmin);
	 } else {
		 self->minval_ = 0.f;
	 }
	 if (lilv_node_is_float(vmax)) {
		 self->maxval_ = lilv_node_as_float(vmax);
	 } else {
		 self->maxval_ = 1.f;
	 }
	 if (lilv_node_is_float(vdef)) {
		 self->value_ = lilv_node_as_float(vdef);
	 } else {
		 self->value_ = self->minval_;
	 }	 
     lilv_node_free(vdef);
     vdef = NULL;
     lilv_node_free(vmin);
     vmin = NULL;
     lilv_node_free(vmax);
     vmax = NULL;
     
     // self->minval_ *= 44100.0;
	 // self->maxval_ *= 44100.0;
	 
	/*LADSPA_Data fDefault=0.0f;
	
	switch (self->hint.HintDescriptor & LADSPA_HINT_DEFAULT_MASK) {
	case LADSPA_HINT_DEFAULT_NONE:
		fDefault = 0.f;
		break;
	case LADSPA_HINT_DEFAULT_MINIMUM:
		fDefault = self->hint.LowerBound;			
		break;
	case LADSPA_HINT_DEFAULT_LOW:
		if (LADSPA_IS_HINT_LOGARITHMIC(self->hint.HintDescriptor)) {
			fDefault = (LADSPA_Data)
				exp(log(self->hint.LowerBound) * 0.75 +
				log(self->hint.UpperBound) * 0.25);
		} else {
			fDefault  = (LADSPA_Data)
				(self->hint.LowerBound * 0.75 +
				self->hint.UpperBound * 0.25);
		}
		break;
	case LADSPA_HINT_DEFAULT_MIDDLE:
		if (LADSPA_IS_HINT_LOGARITHMIC(self->hint.HintDescriptor)) {
			fDefault  = (LADSPA_Data)
				sqrt(self->hint.LowerBound * self->hint.UpperBound);
		} else {
			fDefault = (LADSPA_Data)
				(0.5 * (self->hint.LowerBound + self->hint.UpperBound));
		}
		break;
	case LADSPA_HINT_DEFAULT_HIGH:
		if (LADSPA_IS_HINT_LOGARITHMIC(self->hint.HintDescriptor)) {
			fDefault = (LADSPA_Data)
				exp(log(self->hint.LowerBound) * 0.25 +
				log(self->hint.UpperBound) * 0.75);
		} else {
			fDefault = (LADSPA_Data)
				(self->hint.LowerBound * 0.25 +
				self->hint.UpperBound * 0.75);
		}
		break;
	case LADSPA_HINT_DEFAULT_MAXIMUM:
		fDefault = (LADSPA_Data)self->hint.UpperBound;
		break;
	case LADSPA_HINT_DEFAULT_0:
		fDefault = 0.0f;		
		break;
	case LADSPA_HINT_DEFAULT_1:
		fDefault = 1.0f;
		break;
	case LADSPA_HINT_DEFAULT_100:
		fDefault = 100.0f;
		break;
	case LADSPA_HINT_DEFAULT_440:
		fDefault = 440.0f;
		break;
	default:
		break;
	}	
	self->value_ = fDefault;	*/
}

#endif /* PSYCLE_USE_LV2 */






/*typedef struct {	
	LADSPA_PortDescriptor descriptor_;
	LADSPA_PortRangeHint hint_;
	char* portName_;
	LADSPA_Data value_;
	LADSPA_Data minVal_;
	LADSPA_Data maxVal_;
	float rangeMultiplier_;
	bool integer_;
	bool logarithmic_;	
} LV2Param;

void lv2param_init(LV2Param*, LADSPA_PortDescriptor, LADSPA_PortRangeHint, const char* newname);
void lv2param_dispose(LV2Param*);
LV2Param* lv2param_alloc(void);
LV2Param* lv2param_allocinit(LADSPA_PortDescriptor, LADSPA_PortRangeHint, const char* newname);
INLINE LADSPA_PortDescriptor lv2param_descriptor(LV2Param* self) { return self->descriptor_; }
INLINE LADSPA_PortRangeHintDescriptor lv2param_hint(LV2Param* self) { return self->hint_.HintDescriptor; }
INLINE const char* lv2param_name(LV2Param* self) { return self->portName_; }
INLINE LADSPA_Data lv2param_rawvalue(LV2Param* self) { return self->value_; }
INLINE LADSPA_Data* lv2param_valueaddress(LV2Param* self) { return &self->value_; }
int lv2param_value(LV2Param*);
void lv2param_setValue(LV2Param*, int data);
INLINE void lv2param_setrawvalue(LV2Param* self, LADSPA_Data data) { self->value_ = data; }
void lv2param_setdefault(LV2Param*);
INLINE LADSPA_Data lv2param_minval(LV2Param* self)
{ 
	return self->integer_ ? (self->minVal_ * self->rangeMultiplier_) : 0;
}
INLINE LADSPA_Data lv2param_maxval(LV2Param* self)
{
	return self->integer_ ? (self->maxVal_ - self->minVal_) : 65535;
}*/

/*	
// Parameter

///\todo: Improve the case where no min/max limit is given by the plugin (Example: the amp.so doesn't have a max value).
void lv2param_init(LV2Param* self, LADSPA_PortDescriptor descriptor,LADSPA_PortRangeHint hint, const char *newname)
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
	lv2param_setdefault(self);
	//std::cout << "min/max/def" << minVal_ << "/" << maxVal_ << "/" << value_ << std::endl;
}

void lv2param_dispose(LV2Param* self)
{
	free(self->portName_);
	self->portName_ = 0;
}

LV2Param* lv2param_alloc(void)
{
	return (LV2Param*)malloc(sizeof(LV2Param));
}

LV2Param* lv2param_allocinit(LADSPA_PortDescriptor descriptor, LADSPA_PortRangeHint hint, const char* newname)
{
	LV2Param* rv;

	rv = lv2param_alloc();
	if (rv) {
		lv2param_init(rv, descriptor, hint, newname);
	}
	return rv;
}


void lv2param_setdefault(LV2Param* self)
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
		
int LV2Param_value(LV2Param* self)
{ 
	return (int) ((self->integer_) ? self->value_ :
		// (logarithmic_) ? log10(1+((value_-minVal_)*rangeMultiplier_))*65535.0f:
		(self->logarithmic_) ?  log(1 + ((self->value_ - self->minVal_) * self->rangeMultiplier_)) * 65535.0f:
		(self->value_- self->minVal_)* self->rangeMultiplier_);
}

void LV2Param_setValue(LV2Param* self, int data)
{
	self->value_ = (LADSPA_Data)( (self->integer_) ? data :
		// (logarithmic_) ? minVal_ + (pow(10, data/65535.0f)-1)/ rangeMultiplier_ :
		(self->logarithmic_) ? self->minVal_ + (exp(data / 65535.0f) - 1) / self->rangeMultiplier_ :
		self->minVal_+ (data/ self->rangeMultiplier_));
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	



/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "ladspaparam.h"
/* local */
#include "plugin_interface.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

/* prototypes */
static int ladspapluginparam_name(psy_audio_LadspaParam*, char* text);
static int ladspapluginparam_label(psy_audio_LadspaParam*, char* text);
static void ladspapluginparam_tweak(psy_audio_LadspaParam*, float val);
static int ladspapluginparam_describe(psy_audio_LadspaParam*, char* text);
static float ladspapluginparam_normvalue(psy_audio_LadspaParam*);
static void ladspapluginparam_range(psy_audio_LadspaParam*,
	intptr_t* minval, intptr_t* maxval);
static void ladspapluginparam_set_default(psy_audio_LadspaParam* self);

/* vtable */
static MachineParamVtable ladspapluginparam_vtable;
static int ladspapluginparam_vtable_initialized = 0;

static void ladspapluginparam_vtable_init(psy_audio_LadspaParam* self)
{
	if (!ladspapluginparam_vtable_initialized) {
		ladspapluginparam_vtable = *(self->custommachineparam.machineparam.vtable);
		ladspapluginparam_vtable.name = (fp_machineparam_name)ladspapluginparam_name;
		ladspapluginparam_vtable.label = (fp_machineparam_label)
			ladspapluginparam_label;
		ladspapluginparam_vtable.tweak = (fp_machineparam_tweak)
			ladspapluginparam_tweak;
		ladspapluginparam_vtable.normvalue = (fp_machineparam_normvalue)
			ladspapluginparam_normvalue;
		ladspapluginparam_vtable.range = (fp_machineparam_range)
			ladspapluginparam_range;
		ladspapluginparam_vtable.describe = (fp_machineparam_describe)
			ladspapluginparam_describe;
	}
}	

void psy_audio_ladspaparam_init(
	psy_audio_LadspaParam* self,
	LADSPA_PortDescriptor descriptor,
	LADSPA_PortRangeHint hint,
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
	ladspapluginparam_vtable_init(self);
	self->custommachineparam.machineparam.vtable = &ladspapluginparam_vtable;	
	self->index = index;
	self->port_index = port_index;
	self->descriptor = descriptor;
	self->hint = hint;
	self->port_name = newname;		
	if (LADSPA_IS_HINT_BOUNDED_BELOW(self->hint.HintDescriptor)) {
		self->minval_ = self->hint.LowerBound;
	} else {
		self->minval_ = 0.0;
	}
	if (LADSPA_IS_HINT_BOUNDED_ABOVE(self->hint.HintDescriptor)) {
		self->maxval_ = self->hint.UpperBound;
	} else {
		self->maxval_ = 1.0;
	}	
	if (LADSPA_IS_HINT_SAMPLE_RATE(self->hint.HintDescriptor)) {
		self->minval_ *= 44100.0;
		self->maxval_ *= 44100.0;
	}
	ladspapluginparam_set_default(self);
}

void psy_audio_ladspaparam_dispose(psy_audio_LadspaParam* self)
{
	psy_audio_custommachineparam_dispose(&self->custommachineparam);
}

psy_audio_LadspaParam* psy_audio_ladspaparam_alloc(void)
{
	return (psy_audio_LadspaParam*)malloc(sizeof(psy_audio_LadspaParam));
}

psy_audio_LadspaParam* psy_audio_ladspaparam_allocinit(
	LADSPA_PortDescriptor descriptor,
	LADSPA_PortRangeHint hint,
	const char *newname,	
	uintptr_t index,
	uintptr_t port_index)
{
	psy_audio_LadspaParam* rv;

	rv = psy_audio_ladspaparam_alloc();
	if (rv) {
		psy_audio_ladspaparam_init(rv, descriptor, hint, newname,
			index, port_index);
	}
	return rv;
}

int ladspapluginparam_label(psy_audio_LadspaParam* self, char* text)
{
	text[0] = '\0';
	// self->effect->dispatcher(self->effect, effGetParamLabel, (VstInt32)self->index, 0, text, 0);
	return *text != '\0';
}

int ladspapluginparam_name(psy_audio_LadspaParam* self, char* text)
{
	text[0] = '\0';
	strcpy(text, self->port_name);
	// self->effect->dispatcher(self->effect, effGetParamName, (VstInt32)self->index, 0, text, 0);
	return *text != '\0';
}

void ladspapluginparam_tweak(psy_audio_LadspaParam* self, float value)
{		
	self->value_ = value * (self->maxval_ - self->minval_) -
		self->minval_; 
}

int ladspapluginparam_describe(psy_audio_LadspaParam* self, char* text)
{
	text[0] = '\0';
	
	if (LADSPA_IS_HINT_TOGGLED(self->hint.HintDescriptor)) {
		strcpy(text, (self->value_ > 0.0) ? "on" : "off");	
	} else if (LADSPA_IS_HINT_INTEGER(self->hint.HintDescriptor)) {
		sprintf(text, "%.0f", self->value_);		
	} else {
		sprintf(text, "%.4f", self->value_);
	}	
	// self->effect->dispatcher(self->effect, effGetParamDisplay, (VstInt32)self->index, 0,
	// 	text, 0);
	return *text != '\0';
}

float ladspapluginparam_normvalue(psy_audio_LadspaParam* self)
{
	if (self->maxval_ - self->minval_ != 0.0) {	
		return (self->value_ - self->minval_) /
			(self->maxval_ - self->minval_);
	}
	return 0.0;
}

void ladspapluginparam_range(psy_audio_LadspaParam* self, intptr_t* minval,
	intptr_t* maxval)
{
	*minval = 0;
	*maxval = 0xFFFF;
}

void ladspapluginparam_set_default(psy_audio_LadspaParam* self)
{
	LADSPA_Data fDefault=0.0f;
	
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
	self->value_ = fDefault;	
}







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
} LadspaParam;

void ladspaparam_init(LadspaParam*, LADSPA_PortDescriptor, LADSPA_PortRangeHint, const char* newname);
void ladspaparam_dispose(LadspaParam*);
LadspaParam* ladspaparam_alloc(void);
LadspaParam* ladspaparam_allocinit(LADSPA_PortDescriptor, LADSPA_PortRangeHint, const char* newname);
INLINE LADSPA_PortDescriptor ladspaparam_descriptor(LadspaParam* self) { return self->descriptor_; }
INLINE LADSPA_PortRangeHintDescriptor ladspaparam_hint(LadspaParam* self) { return self->hint_.HintDescriptor; }
INLINE const char* ladspaparam_name(LadspaParam* self) { return self->portName_; }
INLINE LADSPA_Data ladspaparam_rawvalue(LadspaParam* self) { return self->value_; }
INLINE LADSPA_Data* ladspaparam_valueaddress(LadspaParam* self) { return &self->value_; }
int ladspaparam_value(LadspaParam*);
void ladspaparam_setValue(LadspaParam*, int data);
INLINE void ladspaparam_setrawvalue(LadspaParam* self, LADSPA_Data data) { self->value_ = data; }
void ladspaparam_setdefault(LadspaParam*);
INLINE LADSPA_Data ladspaparam_minval(LadspaParam* self)
{ 
	return self->integer_ ? (self->minVal_ * self->rangeMultiplier_) : 0;
}
INLINE LADSPA_Data ladspaparam_maxval(LadspaParam* self)
{
	return self->integer_ ? (self->maxVal_ - self->minVal_) : 65535;
}*/

/*	
// Parameter

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
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	



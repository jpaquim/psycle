// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "psy2converter.h"
#include "psy2.h"
#include "machinefactory.h"
#include "song.h"
#include "songio.h"
#include "plugin.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <dir.h>
#include <math.h>
#include "constants.h"
#include "scale.h"
#include "../../detail/portable.h"

#if !defined DIVERSALIS__OS__MICROSOFT
#define _MAX_PATH 4096
#endif

typedef struct cpoint_t {
	int32_t x;
	int32_t y;
} cpoint_t;

typedef struct ConverterType {
	int type;
	const char* name;
} ConverterType;

static void convertertype_init(ConverterType* self, int type, const char* name)
{
	self->type = type;
	self->name = name;
}

static ConverterType* convertertype_alloc(void)
{
	return (ConverterType*)malloc(sizeof(ConverterType));
}

static ConverterType* convertertype_allocinit(int type, const char* name)
{
	ConverterType* rv;

	rv = convertertype_alloc();
	if (rv) {
		convertertype_init(rv, type, name);
	}
	return rv;
}

static void pluginnames_insert(PluginNames*, int type, const char* name, const char* convname);
static void readplugin(InternalMachinesConvert*, psy_audio_Machine* plugin, psy_audio_SongFile* songfile, int* index, int type, const char* name);
static void retweak_parameter(psy_audio_Song*, double samplerate, int type, const char* name, int* parameter, int* integral_value);
static void retweak_parameters(psy_audio_SongFile*, psy_audio_Machine* machine, int type, const char* name,
	int* parameters, int parameter_count, int parameter_offset);

static const char* convnames[] =
{
	"",
	"ring_modulator.dll",
	"distortion.dll",
	"",
	"delay.dll",
	"filter_2_poles.dll",
	"gainer.dll",
	"flanger.dll"
};

enum {
	master,
	ring_modulator,
	distortion,
	sampler,
	delay,
	filter_2_poles,
	gainer,
	flanger,
	nativeplug,
	vsti,
	vstfx,
	scope,
	dummy = 255
};

static const char* abass = "arguru bass.dll";
static const char* asynth = "arguru synth.dll";
static const char* asynth2 = "arguru synth 2.dll";
static const char* asynth21 = "synth21.dll";
static const char* asynth22 = "synth22.dll";

void pluginnames_init(PluginNames* self)
{
	psy_table_init(&self->container);
	pluginnames_insert(self, ring_modulator, "", convnames[ring_modulator]);
	pluginnames_insert(self, distortion, "", convnames[distortion]);
	pluginnames_insert(self, delay, "", convnames[delay]);
	pluginnames_insert(self, filter_2_poles, "", convnames[filter_2_poles]);
	pluginnames_insert(self, gainer, "", convnames[gainer]);
	pluginnames_insert(self, flanger, "", convnames[flanger]);
	pluginnames_insert(self, nativeplug, "arguru bass.dll", "arguru synth 2f.dll");
	pluginnames_insert(self, nativeplug, "arguru synth.dll", "arguru synth 2f.dll");
	pluginnames_insert(self, nativeplug, "arguru synth 2.dll", "arguru synth 2f.dll");
	pluginnames_insert(self, nativeplug, "synth21.dll", "arguru synth 2f.dll");
	pluginnames_insert(self, nativeplug, "synth22.dll", "arguru synth 2f.dll");
}

void pluginnames_dispose(PluginNames* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->container);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_Table* nametable;

		nametable = (psy_Table*)psy_tableiterator_value(&it);
		psy_table_dispose(nametable);
		free(nametable);
	}
	psy_table_dispose(&self->container);
}

void pluginnames_insert(PluginNames* self, int type, const char* name, const char* convname)
{
	psy_Table* nametable;

	nametable = (psy_Table*) psy_table_at(&self->container, type);
	if (!nametable) {
		nametable = (psy_Table*)malloc(sizeof(psy_Table));
		psy_table_init(nametable);
		psy_table_insert(&self->container, type, nametable);
	}
	psy_table_insert_strhash(nametable, name, (void*) convname);
}

bool pluginnames_exists(PluginNames* self, int type, const char* name)
{
	bool rv = FALSE;

	psy_Table* nametable;

	nametable = psy_table_at(&self->container, type);
	if (nametable) {
		rv = psy_table_exists_strhash(nametable, name);
	}
	return rv;
}

const char* pluginnames_convname(PluginNames* self, int type, const char* name)
{
	const char* rv = 0;

	psy_Table* nametable;

	nametable = psy_table_at(&self->container, type);
	if (nametable) {
		rv = (const char*) psy_table_at_strhash(nametable, name);
	}
	return rv;
}

void internalmachinesconvert_init(InternalMachinesConvert* self)
{
	pluginnames_init(&self->pluginnames);
	psy_table_init(&self->machine_converted_from);
}

void internalmachinesconvert_dispose(InternalMachinesConvert* self)
{
	pluginnames_dispose(&self->pluginnames);
	psy_table_disposeall(&self->machine_converted_from,
		(psy_fp_disposefunc)NULL);
}

psy_audio_Machine* internalmachinesconvert_redirect(
	InternalMachinesConvert* self,
	psy_audio_SongFile* songfile,
	int* index, int type, const char* name)
{
	psy_audio_Machine* machine;
	char sDllName[256];
	char plugincatchername[_MAX_PATH];
	const char* convname;
	char _editName[32];
	cpoint_t connectionpoint[MAX_CONNECTIONS];
	int32_t panning;
	psy_audio_MachineWires* machinewires;

	convname = pluginnames_convname(&self->pluginnames, type, name);
	assert(convname);
	psy_snprintf(sDllName, 256, "%s", convname);
	psy_strlwr(sDllName);
	plugincatcher_catchername(songfile->song->machinefactory->catcher,
		sDllName, plugincatchername, 0);
	machine = psy_audio_machinefactory_makemachine(songfile->song->machinefactory,
		MACH_PLUGIN, plugincatchername);

	psyfile_read(songfile->file, _editName, 16);
	_editName[15] = 0;
	if (machine) {
		psy_audio_machine_seteditname(machine, _editName);
	}
	if (type == nativeplug) {
		readplugin(self, machine, songfile, index, type, name);
	}
	machinewires = psy_audio_read_psy2machinewires(songfile->file);
	psy_audio_legacywires_insert(songfile->legacywires, *index, machinewires);
	psyfile_read(songfile->file, &connectionpoint, sizeof(connectionpoint));
	// numInputs and numOutputs
	psyfile_skip(songfile->file, 2 * sizeof(int32_t));
	psyfile_read(songfile->file, &panning, sizeof(panning));
	// Machine::SetPan(_panning);
	psyfile_skip(songfile->file, 40); // skips sampler data.
	switch (type) {
		case delay:
		{
			int32_t parameters[2];

			psyfile_read(songfile->file, parameters, sizeof(parameters));
			retweak_parameters(songfile, machine, type, name,
				parameters, sizeof parameters / sizeof * parameters, 5);
		}
		break;
		case flanger:
		{
			int32_t parameters[2];
		
			psyfile_read(songfile->file, parameters, sizeof(parameters));
			retweak_parameters(songfile, machine, type, name,
				parameters, sizeof parameters / sizeof * parameters, 7);
		}
		break;
		case gainer:
			psyfile_skip(songfile->file, sizeof(int32_t));
			{
				int parameters[1];

				psyfile_read(songfile->file, parameters, sizeof(parameters));
				retweak_parameters(songfile, machine, type, name,
					parameters, sizeof parameters / sizeof * parameters, 1);
			}
		break;
		default:
			psyfile_skip(songfile->file, 2 * sizeof(int32_t));
		break;
	}
	switch (type) {
		case distortion:
			{
			int32_t parameters[4];

			psyfile_read(songfile->file, parameters, sizeof(parameters));
			retweak_parameters(songfile, machine, type, name,
				parameters, sizeof parameters / sizeof * parameters, 1);
			}
		break;
		default:
			psyfile_skip(songfile->file, 4 * sizeof(int));
		break;
	}
	switch (type) {
	case ring_modulator:
	{
		unsigned char parameters_uint8[4];
		int parameters[4];
		int i;
		psyfile_read(songfile->file, &parameters_uint8[0], 2 * sizeof * parameters_uint8);
		psyfile_skip(songfile->file, sizeof(char));
		psyfile_read(songfile->file, &parameters_uint8[2], 2 * sizeof * parameters_uint8);
		for (i = 0; i < 4; ++i) {
			parameters[i] = parameters_uint8[i];
		}
		retweak_parameters(songfile, machine, type, name,
			parameters, sizeof parameters / sizeof * parameters, 1);
		psyfile_skip(songfile->file, 40);
	}
	break;
	case delay:
		psyfile_skip(songfile->file, 5);
		{
			int parameters[4];
			psyfile_read(songfile->file, &parameters[0], sizeof * parameters);
			psyfile_read(songfile->file, &parameters[2], sizeof * parameters);
			psyfile_read(songfile->file, &parameters[1], sizeof * parameters);
			psyfile_read(songfile->file, &parameters[3], sizeof * parameters);
			retweak_parameters(songfile, machine, type, name,
				parameters, sizeof parameters / sizeof * parameters, 1);
		}
		psyfile_skip(songfile->file, 24);
	break;
	case flanger:
		psyfile_skip(songfile->file, 4);
		{
			uint8_t parameters_uint8[1];
			int parameters[1];
			
			psyfile_read(songfile->file, parameters_uint8, sizeof(parameters_uint8));			
			parameters[0] = parameters_uint8[0];			
			retweak_parameters(songfile, machine, type, name,
				parameters, sizeof parameters / sizeof * parameters, 9);
		}
		{
			int parameters[6];
			psyfile_read(songfile->file, &parameters[0], sizeof * parameters);
			psyfile_skip(songfile->file, 4);
			psyfile_read(songfile->file, &parameters[3], sizeof * parameters);
			psyfile_read(songfile->file, &parameters[5], sizeof * parameters);
			psyfile_skip(songfile->file, 8);
			psyfile_read(songfile->file, &parameters[2], sizeof * parameters);
			psyfile_read(songfile->file, &parameters[1], sizeof * parameters);
			psyfile_read(songfile->file, &parameters[4], sizeof * parameters);
			retweak_parameters(songfile, machine, type, name,
				parameters, sizeof parameters / sizeof * parameters, 1);
		}
		psyfile_skip(songfile->file, 4);
	break;
	case filter_2_poles:
		psyfile_skip(songfile->file, 21);
		{
			int parameters[6];

			psyfile_read(songfile->file, &parameters[1], sizeof parameters - sizeof * parameters);
			psyfile_read(songfile->file, &parameters[0], sizeof * parameters);
			retweak_parameters(songfile, machine, type, name,
				parameters, sizeof parameters / sizeof * parameters, 1);
		}
		break;
	default:
		psyfile_skip(songfile->file, 45);
	}
	
	if (machine) {
		ConverterType* convertertype;

		convertertype = convertertype_allocinit(type, name);
		psy_table_insert(&self->machine_converted_from, (uintptr_t)machine, convertertype);
	}
	return machine;
}

void readplugin(InternalMachinesConvert* self, psy_audio_Machine* machine,
	psy_audio_SongFile* songfile, int* index, int type, const char* name)
{
	int numParameters;
	int* Vals;
	psy_audio_MachineParam* param;

	psyfile_read(songfile->file, &numParameters, sizeof(numParameters));
	Vals = malloc(sizeof(int32_t) * numParameters);
	psyfile_read(songfile->file, Vals, numParameters * sizeof(int));
	if (type == MACH_DUMMY) {
		//do nothing.
	} else if (strcmp(name, abass) == 0) {
		retweak_parameters(songfile, machine, type, name, Vals, 15, 0);
		param = psy_audio_machine_parameter(machine, 19);
		if (param) {
			psy_audio_machine_parameter_tweak_scaled(machine, param, 0);
		}
		retweak_parameters(songfile, machine, type, name, Vals + 15, 1, 15);
		if (numParameters > 16) {
			retweak_parameters(songfile, machine, type, name, Vals + 16, 2, 16);
		} else {
			param = psy_audio_machine_parameter(machine, 24);
			if (param) {
				psy_audio_machine_parameter_tweak_scaled(machine, param, 0);
			}
			param = psy_audio_machine_parameter(machine, 25);
			if (param) {
				psy_audio_machine_parameter_tweak_scaled(machine, param, 0);
			}
		}
	} else
	if (strcmp(name, asynth) == 0) {
		retweak_parameters(songfile, machine, type, name, Vals, numParameters, 0);
		param = psy_audio_machine_parameter(machine, 24);
		if (param) {
			psy_audio_machine_parameter_tweak_scaled(machine, param, 0);
		}
		param = psy_audio_machine_parameter(machine, 25);
		if (param) {
			psy_audio_machine_parameter_tweak_scaled(machine, param, 0);
		}
		param = psy_audio_machine_parameter(machine, 27);
		if (param) {
			psy_audio_machine_parameter_tweak_scaled(machine, param, 1);
		}
	} else if (strcmp(name, asynth2) == 0) {
		retweak_parameters(songfile, machine, type, name, Vals, numParameters, 0);
		param = psy_audio_machine_parameter(machine, 24);
		if (param) {
			psy_audio_machine_parameter_tweak_scaled(machine, param, 0);
		}
		param = psy_audio_machine_parameter(machine, 25);
		if (param) {
			psy_audio_machine_parameter_tweak_scaled(machine, param, 0);
		}
	} else  if (strcmp(name, asynth21) == 0) {
		//I am unsure which was the diference between asynth2 and asynth21 (need to chech sources in the cvs)
		retweak_parameters(songfile, machine, type, name, Vals, numParameters, 0);
		param = psy_audio_machine_parameter(machine, 24);
		if (param) {
			psy_audio_machine_parameter_tweak_scaled(machine, param, 0);
		}
		param = psy_audio_machine_parameter(machine, 25);
		if (param) {
			psy_audio_machine_parameter_tweak_scaled(machine, param, 0);
		}
	} else if (strcmp(name, asynth22) == 0) {
		retweak_parameters(songfile, machine, type, name, Vals, numParameters, 0);
	}
	free(Vals);
	Vals = 0;
}

void internalmachineconverter_retweak_song(InternalMachinesConvert* self, psy_audio_Song* song, double samplerate)
{
	/// \todo must each twk repeat the machine number ?
	// int previous_machines [MAX_TRACKS]; for(int i = 0 ; i < MAX_TRACKS ; ++i) previous_machines[i] = 255;
	psy_TableIterator it;

	for (it = psy_table_begin(&song->patterns.slots);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_Pattern* pattern;
		psy_audio_PatternNode* p;

		pattern = (psy_audio_Pattern*)psy_tableiterator_value(&it);
		if (psy_audio_pattern_empty(pattern)) {
			continue;
		}
		for (p = pattern->events; p != NULL; psy_list_next(&p)) {
			psy_audio_PatternEntry* entry;
			psy_audio_PatternEvent* event;

			entry = (psy_audio_PatternEntry*)psy_list_entry(p);
			event = psy_audio_patternentry_front(entry);

			if (event->note == psy_audio_NOTECOMMANDS_TWEAKEFFECT)
			{
				event->mach += 0x40;
				event->note = psy_audio_NOTECOMMANDS_TWEAK;
			}
			if (event->note == psy_audio_NOTECOMMANDS_TWEAK && event->mach < MAX_MACHINES)
			{
				if (psy_table_exists(&self->machine_converted_from, event->mach)) {
					ConverterType* convertertype;
					int parameter;
					int value;

					convertertype = psy_table_at(&self->machine_converted_from,
						event->mach);
					parameter = event->inst;
					value = ((event->cmd << 8) + event->parameter);

					retweak_parameter(song, samplerate,
						convertertype->type,
						convertertype->name,
						&parameter,
						&value);
					event->inst = parameter;
					event->cmd = value >> 8;
					event->parameter = 0xff & value;
				}				
			} else
			if (event->cmd == 0x0E && event->mach < MAX_MACHINES) {
				if (psy_table_exists(&self->machine_converted_from, event->mach)) {
					ConverterType* convertertype;
					int param;
					int value;

					convertertype = psy_table_at(&self->machine_converted_from,
						event->mach);		
					param = 25;
					value = event->parameter;
					retweak_parameter(song, samplerate,
						convertertype->type,
						convertertype->name,
						&param,
						&value);
					event->cmd = 0x0F;
					event->parameter = value;					
				}
			}			
		}
	}
}

void retweak_parameters(psy_audio_SongFile* songfile, psy_audio_Machine* machine, int type, const char* name,
	int* parameters, int parameter_count, int parameter_offset)
{
	int parameter = 0;

	for (; parameter < parameter_count; ++parameter) {
		int new_parameter;
		int new_value;
		psy_audio_MachineParam* param;
		new_parameter = parameter_offset + parameter;				
		new_value = parameters[parameter];
		retweak_parameter(songfile->song,
			psy_audio_machine_samplerate(machine),
			type, name, &new_parameter,
			&new_value);
		param = psy_audio_machine_parameter(machine, new_parameter);
		if (param) {
			psy_audio_machine_parameter_tweak_scaled(machine, param,  new_value);
		}
	}
}

void retweak_parameter(psy_audio_Song* song,
	double samplerate, 
	int type, const char* name, int* parameter,
	int* integral_value)
{
	typedef double Real;
	double value;
	const Real maximum = 0xffff;
	psy_dsp_Scale scale;
	double sr;
	double spr;
	double bpl;
	double bps;

	sr = samplerate;
	bps = (psy_audio_song_bpm(song) * (psy_dsp_beat_t) 1.f) /
		(sr * 60.0f);
	bpl = 1 / (psy_dsp_beat_t) song->properties.lpb;
	spr =  bpl * 1/bps;

	value = *integral_value;
	switch (type)
	{
	case gainer:
	{
		enum Parameters { gain };
		static const int parameters[] = { gain };
		*parameter = parameters[-- * parameter];
		switch (*parameter)
		{
		case gain:
			if (value < 1.0f) {
				value = 0;
			} else {
				psy_dsp_scale_init_exp(&scale, maximum, exp(-4.), exp(+4.));
				value = psy_dsp_scale_exp_apply_inverse(&scale, value / 0x100);
			}
			break;
		}
	}
	break;
	case distortion: {
		enum Parameters { input_gain, output_gain, positive_threshold, positive_clamp, negative_threshold, negative_clamp, symmetric };
		static const int parameters[] = { positive_threshold, positive_clamp, negative_threshold, negative_clamp };
		*parameter = parameters[-- * parameter];
		switch (*parameter)
		{
		case negative_threshold:
		case negative_clamp:
		case positive_threshold:
		case positive_clamp:
			value *= maximum / 0x100;
			break;
		}	
		break; }
	case delay: {
		enum Parameters { dry, wet, left_delay, left_feedback, right_delay, right_feedback };
		static const int parameters[] = { left_delay, left_feedback, right_delay, right_feedback, dry, wet };
		*parameter = parameters[--*parameter];
		switch (*parameter) {
			case left_delay:
			case right_delay:
				value *= (float)(2 * 3 * 4 * 5 * 7) / spr;
				break;
			case left_feedback:
			case right_feedback:
				value = (100 + value) * maximum / 200;
				break;
			case dry:
			case wet:
				value = (0x100 + value) * maximum / 0x200;
				break;
		}
	break;	}
	case flanger: {
		enum Parameters { delay, modulation_amplitude, modulation_radians_per_second, modulation_stereo_dephase, interpolation, dry, wet, left_feedback, right_feedback };
		static const int parameters[] = { delay, modulation_amplitude, modulation_radians_per_second, left_feedback, modulation_stereo_dephase, right_feedback, dry, wet, interpolation };
		*parameter = parameters[--*parameter];
		switch (*parameter) {
			case delay:
				value *= maximum / 0.1 / sr;
				break;
			case modulation_amplitude:
			case modulation_stereo_dephase:
				value *= maximum / 0x100;
				break;
			case modulation_radians_per_second:
				if (value < 1.0f) {
					value = 0;
				} else {
					psy_dsp_scale_init_exp(&scale, maximum, 0.0001 * psy_dsp_PI * 2, 100 * psy_dsp_PI * 2);
					value = psy_dsp_scale_exp_apply_inverse(&scale, value * 3e-9 * sr);
				}
				break;
			case left_feedback:
			case right_feedback:
				value = (100 + value) * maximum / 200;
				break;
			case dry:
			case wet:
				value = (0x100 + value) * maximum / 0x200;
				break;
			case interpolation:
				value = value != 0;
				break;
		}
	break;	}
	case filter_2_poles: {
		enum Parameters { response, cutoff_frequency, resonance, modulation_sequencer_ticks, modulation_amplitude, modulation_stereo_dephase };
		static const int parameters[] = { response, cutoff_frequency, resonance, modulation_sequencer_ticks, modulation_amplitude, modulation_stereo_dephase };
		*parameter = parameters[--*parameter];
		switch (*parameter) {
			case cutoff_frequency:
				if (value < 1.0f) {
					value = 0;
				} else {				
					psy_dsp_scale_init_exp(&scale, maximum, 15 * psy_dsp_PI, 22050 * psy_dsp_PI);
					value = psy_dsp_scale_exp_apply_inverse(&scale, asin(value / 0x100) * sr);
				}
				break;
			case modulation_sequencer_ticks:
				if (value < 1.0f) {
					value = 0;
				} else {
					psy_dsp_scale_init_exp(&scale, maximum, psy_dsp_PI * 2 / 10000, psy_dsp_PI * 2 * 2 * 3 * 4 * 5 * 7);
					value = psy_dsp_scale_exp_apply_inverse(&scale, value * 3e-8 * spr);
				}
				break;
			case resonance:
			case modulation_amplitude:
			case modulation_stereo_dephase:
				value *= maximum / 0x100;
				break;
		}
		break;	}
	case ring_modulator: {
		enum Parameters { am_radians_per_second, am_glide, fm_radians_per_second, fm_bandwidth };
		static const int parameters[] = { am_radians_per_second, am_glide, fm_radians_per_second, fm_bandwidth };
		*parameter = parameters[--*parameter];
		switch (*parameter)
		{
		case am_radians_per_second:
			if (value < 1.0f) {
				value = 0;
			} else {
				psy_dsp_scale_init_exp(&scale, maximum, 0.0001 * psy_dsp_PI * 2, 22050 * psy_dsp_PI * 2);
				value = psy_dsp_scale_exp_apply_inverse(&scale, value * 2.5e-3 * sr);
			}			
			break;
		case am_glide:
			if (value < 1.0f) {
				value = 0;
			} else {
				psy_dsp_scale_init_exp(&scale, maximum, 0.0001 * psy_dsp_PI * 2, 15 * 22050 * psy_dsp_PI * 2);
				value = psy_dsp_scale_exp_apply_inverse(&scale, value * 5e-6 * sr * sr);
			}
			break;
		case fm_radians_per_second:
			if (value < 1.0f) {
				value = 0;
			} else {
				psy_dsp_scale_init_exp(&scale, maximum, 0.0001 * psy_dsp_PI * 2, 100 * psy_dsp_PI * 2);
				value = psy_dsp_scale_exp_apply_inverse(&scale, value * 2.5e-5 * sr);
			}			
			break;
		case fm_bandwidth:
			if (value < 1.0f) {
				value = 0;
			} else {
				psy_dsp_scale_init_exp(&scale, maximum, 0.0001 * psy_dsp_PI * 2, 22050 * psy_dsp_PI * 2);
				value = psy_dsp_scale_exp_apply_inverse(&scale, value * 5e-4 * sr);
			}
			break;
		}
	break;	}
	case nativeplug: {
		if (strcmp(name, abass) == 0) {
			if (*parameter > 0 && *parameter < 15) {
				*parameter += 4;
			} else if (*parameter == 15) {
				*parameter += 5;
			} else if (*parameter > 15) {
				*parameter += 8;
			}
		} else if (strcmp(name, asynth) == 0) {
			if ((*parameter == 0 || *parameter == 1) && *integral_value == 4) {
				value = 5;
			}
			if (*parameter == 17) {
				value += 10.f;
			}
		} else if (strcmp(name, asynth2) == 0) {
			if ((*parameter == 0 || *parameter == 1) && *integral_value == 4) {
				value = 5;
			}
		} else if (strcmp(name, asynth21) == 0) {
			if ((*parameter == 0 || *parameter == 1) && *integral_value == 4) {
				value = 5;
			}
		} else if (strcmp(name, asynth22) == 0) {			
			if ((*parameter == 0 || *parameter == 1) && *integral_value == 4) {
				value = 5;
			}
			if (*parameter == 7 && integral_value == 0) {
				value = 1.f;
			} else if (*parameter == 25) {
				value = 256 - sqrt(16000.f - value * 16000.f / 127.f);
				*parameter += 1;
			} else if (*parameter == 26) {
				*parameter -= 1;
			}
		}
	}
	break;
	}
	*integral_value = (int)floor(value + (float)(0.5));
}

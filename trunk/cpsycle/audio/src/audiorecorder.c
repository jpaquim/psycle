// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "audiorecorder.h"
#include "plugin_interface.h"
#include "songio.h"

#include <operations.h>

#include <math.h>

#include "../../detail/portable.h"

const psy_audio_MachineInfo* psy_audio_audiorecorder_info(void)
{
	static psy_audio_MachineInfo const macinfo = {
		MI_VERSION,
		0x0250,
		0,
		psy_audio_MACHMODE_GENERATOR,
		"AudioRecorder"
			#ifndef NDEBUG
			" (debug build)"
			#endif
			,
		"AudioRecorder",
		"Psycledelics",
		"help",
		psy_audio_RECORDER,
		0,
		0,
		"",
		"",
		"",
		psy_INDEX_INVALID,
		""
	};
	return &macinfo;
}

static const psy_audio_MachineInfo* info(psy_audio_AudioRecorder* self) {
	return psy_audio_audiorecorder_info();
}
static int mode(psy_audio_AudioRecorder* self) { return psy_audio_MACHMODE_GENERATOR; }
static void work(psy_audio_AudioRecorder*, psy_audio_BufferContext*);
static uintptr_t numinputs(psy_audio_AudioRecorder* self) { return 0; }
static uintptr_t numoutputs(psy_audio_AudioRecorder* self) { return 2; }
static psy_dsp_amp_range_t amprange(psy_audio_AudioRecorder* self)
{
	return PSY_DSP_AMP_RANGE_NATIVE;
}
static int loadspecific(psy_audio_AudioRecorder*, psy_audio_SongFile*,
	uintptr_t slot);
static int savespecific(psy_audio_AudioRecorder*, psy_audio_SongFile*,
	uintptr_t slot);
// parameters
static int parametertype(psy_audio_AudioRecorder* self, uintptr_t param);
static unsigned int numparametercols(psy_audio_AudioRecorder*);
static uintptr_t numparameters(psy_audio_AudioRecorder*);
static void parameterrange(psy_audio_AudioRecorder*, uintptr_t param, int* minval,
	int* maxval);
static int parameterlabel(psy_audio_AudioRecorder*, char* txt, uintptr_t param);
static int parametername(psy_audio_AudioRecorder*, char* txt, uintptr_t param);
static void parametertweak(psy_audio_AudioRecorder*, uintptr_t param, float val);
static int describevalue(psy_audio_AudioRecorder*, char* txt, uintptr_t param,
	int value);
static float parametervalue(psy_audio_AudioRecorder*, uintptr_t param);
static void changeport(psy_audio_AudioRecorder*, int newport);

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_AudioRecorder* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;
		vtable.mode = (fp_machine_mode) mode;
		vtable.work = (fp_machine_work) work;
		vtable.info = (fp_machine_info) info;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable.amprange = (fp_machine_amprange) amprange;
		vtable.loadspecific = (fp_machine_loadspecific) loadspecific;
		vtable.savespecific = (fp_machine_savespecific) savespecific;
		vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		vtable.numparameters = (fp_machine_numparameters) numparameters;
		vtable_initialized = 1;
	}
}

void psy_audio_audiorecorder_init(psy_audio_AudioRecorder* self, psy_audio_MachineCallback* callback)
{		
	psy_audio_custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	self->custommachine.machine.vtable = &vtable;
	psy_audio_machine_seteditname(psy_audio_audiorecorder_base(self), "Recorder");
	self->_captureidx = -1;
	self->_gainvol = 1.f;
	if (psy_audio_machine_numcaptures(
		psy_audio_audiorecorder_base(self)) > 0) {
		changeport(self, 0);
	}	
}

int loadspecific(psy_audio_AudioRecorder* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	int32_t readcaptureidx;
	float gainvol;
	int status;

	// size of this part params to load
	if ((status = psyfile_read(songfile->file, &size, sizeof(size)))) {
		return status;
	}
	if ((status = psyfile_read(songfile->file, &readcaptureidx,
			sizeof(readcaptureidx)))) {
		return status;
	}
	if ((status = psyfile_read(songfile->file, &gainvol, sizeof(gainvol)))) {
		return status;
	}
	self->_gainvol = gainvol;
	self->_captureidx = readcaptureidx;
	// ChangePort(readcaptureidx);
	return PSY_OK;
}

int savespecific(psy_audio_AudioRecorder* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	int status;
	
	size = sizeof(self->_captureidx) + sizeof(self->_gainvol);
	// size of this part params to save
	if ((status = psyfile_write_uint32(songfile->file, size))) {
		return status;
	}
	if ((status = psyfile_write_int32(songfile->file, self->_captureidx))) {
		return status;
	}
	if ((status = psyfile_write_float(songfile->file, self->_gainvol))) {
		return status;
	}
	return PSY_OK;
}

// parameters
void parametertweak(psy_audio_AudioRecorder* self, uintptr_t param, float value)
{
	if (param == 0) {
		self->_gainvol = value * value * 4.f;
	} else
	if (param == 1) {
		int newport;
		//newport = machine_parametervalue_scaled(&self->custommachine.machine,
		//	param, value);
		changeport(self, newport);
	}
}

int describevalue(psy_audio_AudioRecorder* self, char* text, uintptr_t param,
	int value)
{
	if (param == 0) {
		psy_dsp_amp_t db = (psy_dsp_amp_t)(20 *
			log10(self->_gainvol));
		psy_snprintf(text, 10, "%.2f dB", db);
		return 1;
	} else if (param == 1) {
		if (value < psy_audio_machine_numcaptures(
			psy_audio_audiorecorder_base(self))) {
			psy_snprintf(text, 128, "%s", psy_audio_machine_capturename(
				psy_audio_audiorecorder_base(self), value));
			return 1;
		}
	}
	psy_snprintf(text, 128, "No psy_audio_Inputs Available");
	return 1;
}

float parametervalue(psy_audio_AudioRecorder* self, uintptr_t param)
{
	if (param == 0) {
		return (float)sqrt(self->_gainvol) * 0.5f;;
	} else
	if (param == 1) {
		//return machine_parametervalue_normed(&self->custommachine.machine,
		//	param, self->_captureidx);
	}
	return 0.f;
}

int parametertype(psy_audio_AudioRecorder* self, uintptr_t param)
{	
	return MPF_STATE;
}

void parameterrange(psy_audio_AudioRecorder* self, uintptr_t param, int* minval,
	int* maxval)
{
	if (param == 0) {
		*minval = 0;
		*maxval = 0xFFFF;
	} else
	if (param == 1) {
		*minval = 0;
		*maxval = psy_audio_machine_numcaptures(
			psy_audio_audiorecorder_base(self));
	} else {
		*minval = 0;
		*maxval = 0;
	}
}

int parameterlabel(psy_audio_AudioRecorder* self, char* text, uintptr_t param)
{
	if (param == 0) {
		psy_snprintf(text, 128, "Gain");
		return 1;
	} else
	if (param == 1) {
		psy_snprintf(text, 128, "Capture Port");
		return 1;
	}
	return 0;
}

int parametername(psy_audio_AudioRecorder* self, char* text, uintptr_t param)
{
	if (param == 0) {
		psy_snprintf(text, 128, "Gain");
		return 1;
	} else
	if (param == 1) {
		psy_snprintf(text, 128, "Capture Port");
		return 1;
	}
	return 0;
}

unsigned int numparametercols(psy_audio_AudioRecorder* self)
{
	return 1;
}

uintptr_t numparameters(psy_audio_AudioRecorder* self)
{	
	return 2;
}

void work(psy_audio_AudioRecorder* self, psy_audio_BufferContext* bc)
{
	if (!psy_audio_machine_bypassed(psy_audio_audiorecorder_base(self)) &&
		 !psy_audio_machine_muted(psy_audio_audiorecorder_base(self))) {
	
		float* left = 0;
		float* right = 0;
		psy_audio_machine_readbuffers(
			psy_audio_audiorecorder_base(self),
			self->_captureidx,
			&left, &right,
			bc->numsamples);
		if (left == NULL) {
			dsp.clear(bc->output->samples[0], bc->numsamples);
		} else {
			dsp.movmul(left, bc->output->samples[0], bc->numsamples, self->_gainvol);
		}
		if (right == NULL) {
			dsp.clear(bc->output->samples[1], bc->numsamples);
		} else {
			dsp.movmul(left, bc->output->samples[1], bc->numsamples, self->_gainvol);
		}
	}
}

void changeport(psy_audio_AudioRecorder* self, int newport)
{
	if (self->_captureidx != newport) {
		psy_audio_machine_removecapture(psy_audio_audiorecorder_base(self),
			self->_captureidx);
		psy_audio_machine_addcapture(psy_audio_audiorecorder_base(self),
			newport);
		self->_captureidx = newport;
	}
}

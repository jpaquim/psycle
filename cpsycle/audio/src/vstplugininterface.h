// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_VSTPLUGININTERFACE_H
#define psy_audio_VSTPLUGININTERFACE_H

#include "../../detail/psydef.h"

#ifdef PSYCLE_USE_VST2

// local
#include "machinedefs.h"
// dsp
#include "../../dsp/src/dsptypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AEffect;
struct VstEvents;
struct psy_audio_BufferContext;


typedef struct psy_audio_VstInterface {
	struct AEffect* effect;
} psy_audio_VstInterface;

void psy_audio_vstinterface_init(psy_audio_VstInterface*,
	struct AEffect* effect, void* user);

void psy_audio_vstinterface_open(psy_audio_VstInterface*);
void psy_audio_vstinterface_close(psy_audio_VstInterface*);
void psy_audio_vstinterface_setsamplerate(psy_audio_VstInterface*,
	psy_dsp_big_hz_t samplerate);
void psy_audio_vstinterface_setprocessprecision32(psy_audio_VstInterface*);
void psy_audio_vstinterface_setblocksize(psy_audio_VstInterface*,
	int32_t blocksize);
void psy_audio_vstinterface_mainschanged(psy_audio_VstInterface*);
void psy_audio_vstinterface_startprocess(psy_audio_VstInterface*);
void psy_audio_vstinterface_work(psy_audio_VstInterface*,
	struct psy_audio_BufferContext*);
void psy_audio_vstinterface_tick(psy_audio_VstInterface*, struct VstEvents*);
void psy_audio_vstinterface_beginprogram(psy_audio_VstInterface*);
void psy_audio_vstinterface_setprogram(psy_audio_VstInterface*, uintptr_t prgidx);
void psy_audio_vstinterface_endprogram(psy_audio_VstInterface*);
uint8_t psy_audio_vstinterface_program(psy_audio_VstInterface*);
uintptr_t psy_audio_vstinterface_numprograms(const psy_audio_VstInterface*);
bool psy_audio_vstinterface_hasprogramchunk(const psy_audio_VstInterface*);
psy_audio_MachineMode psy_audio_vstinterface_mode(psy_audio_VstInterface*);
uintptr_t psy_audio_vstinterface_numinputs(const psy_audio_VstInterface*);
uintptr_t psy_audio_vstinterface_numoutputs(const psy_audio_VstInterface*);
float psy_audio_vstinterface_parametervalue(const psy_audio_VstInterface*,
	uintptr_t index);
void psy_audio_vstinterface_setparametervalue(psy_audio_VstInterface*,
	uintptr_t index, float value);
uintptr_t psy_audio_vstinterface_numparameters(const psy_audio_VstInterface*);
void* psy_audio_vstinterface_chunkdata(psy_audio_VstInterface*,
	bool onlycurrprogram, uintptr_t* rv_size);
bool psy_audio_vstinterface_setchunkdata(psy_audio_VstInterface*,
	bool onlycurrprogram, void* data, uintptr_t size);
bool psy_audio_vstinterface_haseditor(const psy_audio_VstInterface*);
void psy_audio_vstinterface_openeditor(psy_audio_VstInterface*, void* handle);
void psy_audio_vstinterface_closeeditor(psy_audio_VstInterface*, void* handle);
void psy_audio_vstinterface_editoridle(psy_audio_VstInterface*);

#ifdef __cplusplus
}
#endif

#endif /* PSYCLE_USE_VST2 */

#endif /* psy_audio_VSTPLUGININTERFACE_H */

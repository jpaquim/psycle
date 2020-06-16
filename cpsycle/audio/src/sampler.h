// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SAMPLER_H
#define psy_audio_SAMPLER_H

#include "custommachine.h"
#include "instrument.h"
#include "sample.h"
#include "multiresampler.h"
#include <adsr.h>
#include <filter.h>
#include <hashtbl.h>
#include <valuemapper.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLER_DEFAULT_POLYPHONY	8
	
/*
	* = remembers its last value when called with param 00.
	t = slides/changes each tick. (or is applied in a specific tick != 0 )
	p = persistent ( a new note doesn't reset it )
	n = they need to appear next to a note.
*/
#define	SAMPLER_CMD_NONE                 0x00	
#define SAMPLER_CMD_PORTAMENTO_UP        0x01 // Portamento Up , Fine porta (01Fx), and Extra fine porta (01Ex)	(*t)
#define SAMPLER_CMD_PORTAMENTO_DOWN      0x02 // Portamento Down, Fine porta (02Fx), and Extra fine porta (02Ex) (*t)
#define SAMPLER_CMD_PORTA2NOTE           0x03 // Tone Portamento						(*tn)
#define SAMPLER_CMD_VIBRATO              0x04 // Do Vibrato							(*t)
#define SAMPLER_CMD_TONEPORTAVOL         0x05 // Tone Portament & Volume Slide		(*t)
#define SAMPLER_CMD_VIBRATOVOL           0x06 // Vibrato & Volume Slide				(*t)
#define SAMPLER_CMD_TREMOLO              0x07 // Tremolo							(*t)
#define SAMPLER_CMD_PANNING              0x08 // Set Panning Position				(p)
#define SAMPLER_CMD_PANNINGSLIDE         0x09 // Panning slide						(*t)
#define SAMPLER_CMD_SET_CHANNEL_VOLUME   0x0A // Set channel's volume				(p)
#define SAMPLER_CMD_CHANNEL_VOLUME_SLIDE 0x0B // channel Volume Slide up (0By0) down (0B0x), Fine slide up(0BFy) down(0BxF)	 (*tp)
#define SAMPLER_CMD_VOLUME               0x0C // Set Volume
#define SAMPLER_CMD_VOLUMESLIDE          0x0D // Volume Slide up (0Dy0), down (0D0x), Fine slide up(0DyF), down(0DFy)	 (*t)
#define SAMPLER_CMD_FINESLIDEUP          0x0F // Part of the value that indicates it is a fine slide up
#define SAMPLER_CMD_FINESLIDEDOWN        0xF0 // Part of the value that indicates it is a fine slide down
#define SAMPLER_CMD_EXTENDED             0x0E // Extend Command
#define SAMPLER_CMD_MIDI_MACRO           0x0F // Impulse Tracker MIDI macro			(p)
#define SAMPLER_CMD_ARPEGGIO             0x10 // Arpeggio							(*t)
#define SAMPLER_CMD_RETRIG               0x11 // Retrigger Note						(*t)
#define SAMPLER_CMD_FINE_VIBRATO         0x14 // Vibrato 4 times finer				(*t)
#define SAMPLER_CMD_TREMOR               0x17 // Tremor								(*t)
#define SAMPLER_CMD_PANBRELLO            0x18 // Panbrello							(*t)
#define SAMPLER_CMD_SET_ENV_POSITION     0x19 // Set Envelope Position
#define SAMPLER_CMD_SET_GLOBAL_VOLUME    0x1C // Sets Global Volume
#define SAMPLER_CMD_GLOBAL_VOLUME_SLIDE  0x1D // Slides Global Volume				(*t)
#define SAMPLER_CMD_SENDTOVOLUME         0x1E // Interprets this as a volume command	()
#define SAMPLER_CMD_OFFSET               0x90 // Set Sample Offset  , note!: 0x9yyy ! not 0x90yy (*n)		PORTAMENTO_UP = 0x01,// Portamento Up , Fine porta (01Fx), and Extra fine porta (01Ex)	(*t)

#define SAMPLER_CMD_E_GLISSANDO_TYPE     0x30 // E3     Set gliss control           (p)
#define SAMPLER_CMD_E_VIBRATO_WAVE       0x40 // E4     Set vibrato control         (p)
#define SAMPLER_CMD_E_PANBRELLO_WAVE     0x50 //                                    (p)
//0x60
#define SAMPLER_CMD_E_TREMOLO_WAVE       0x70 // E7     Set tremolo control         (p)
#define SAMPLER_CMD_E_SET_PAN            0x80 //                                    (p)
#define SAMPLER_CMD_E9                   0x90
//0xA0,
//0xB0,
#define SAMPLER_CMD_E_DELAYED_NOTECUT    0xC0 // EC     Note cut                    (t)
#define SAMPLER_CMD_E_NOTE_DELAY         0xD0 // ED     Note delay                  (tn)
#define SAMPLER_CMD_EE                   0xE0
#define SAMPLER_CMD_E_SET_MIDI_MACRO     0xF0 //                                    (p)


typedef enum psy_audio_SamplerCmdMode {
	// *= remembers its last value when called with param 00.
	psy_audio_SAMPLERCMDMODE_MEM0 = 1,
	// t = slides / changes each tick. (or is applied in a specific tick != 0)
	psy_audio_SAMPLERCMDMODE_TICK = 2,
	// p = persistent(a new note doesn't reset it )	
	psy_audio_SAMPLERCMDMODE_PERS = 4,
	// n = they need to appear next to a note.
	psy_audio_SAMPLERCMDMODE_NEXT = 8  
} psy_audio_SamplerCmdMode;

typedef struct SamplerCmd {
	int id;
	int patternid;
	int mode;
} psy_audio_SamplerCmd;

void psy_audio_samplercmd_init_all(psy_audio_SamplerCmd*,
	int id, int patternid, int mask);
void psy_audio_samplercmd_dispose(psy_audio_SamplerCmd*);

INLINE psy_audio_SamplerCmd psy_audio_samplercmd_make(int id, int patternid, int mask)
{
	psy_audio_SamplerCmd rv;

	rv.id = id;
	rv.patternid = patternid;
	rv.mode = mask;
	return rv;
}

INLINE bool psy_audio_samplercmd_hasticktime(const psy_audio_SamplerCmd* self)
{
	return ((self->mode & psy_audio_SAMPLERCMDMODE_TICK)
		== psy_audio_SAMPLERCMDMODE_TICK);
}

psy_audio_SamplerCmd* psy_audio_samplercmd_alloc(void);
psy_audio_SamplerCmd* psy_audio_samplercmd_allocinit_all(int id,
	int patternid, int mask);

typedef enum {
	INTERPOL_NONE = 0,
	INTERPOL_LINEAR = 1,
	INTERPOL_SPLINE = 2
} InterpolationType;

typedef enum {
	psy_audio_PANNING_LINEAR = 0,
	psy_audio_PANNING_TWOWAY = 1,
	psy_audio_PANNING_EQUALPOWER = 2
} psy_audio_SamplerPanningMode;

struct psy_audio_Sampler;

typedef struct psy_audio_SamplerMasterChannel {
	struct psy_audio_Sampler* sampler;
	// (0..1.0f) value used for Playback (channel volume)
	psy_dsp_amp_t volume;
	psy_audio_InfoMachineParam param_channel;
	psy_audio_VolumeMachineParam slider_param;
	psy_audio_IntMachineParam level_param;	
} psy_audio_SamplerMasterChannel;

typedef struct psy_audio_SamplerChannel {
	struct psy_audio_Sampler* sampler;
	// (0..1.0f) value used for Playback (channel volume)
	psy_dsp_amp_t volume;
	// (0..1.0f) value used for Playback (pan factor)
	float panfactor;
	int m_ChannelDefVolume;
	int m_DefaultCutoff;
	int m_DefaultRessonance;
	int m_DefaultPanFactor;  //  0..200 .  &0x100 == Surround. // value used for Storage and reset
	FilterType defaultfiltertype;
	psy_audio_InfoMachineParam param_channel;
	psy_audio_IntMachineParam filter_cutoff;
	psy_audio_IntMachineParam filter_res;
	psy_audio_IntMachineParam pan;
	psy_audio_VolumeMachineParam slider_param;
	psy_audio_IntMachineParam level_param;	
} psy_audio_SamplerChannel;

bool  psy_audio_samplerchannel_load(psy_audio_SamplerChannel*,
	struct psy_audio_SongFile*);
void psy_audio_samplerchannel_save(psy_audio_SamplerChannel*,
	struct psy_audio_SongFile*);

INLINE float psy_audio_samplerchannel_defaultpanfactorfloat(psy_audio_SamplerChannel* self)
{
	return (self->m_DefaultPanFactor & 0xFF) / 200.0f;
}

typedef struct {
	uintptr_t channelnum;
	psy_audio_SamplerChannel* channel;	
	struct psy_audio_Sampler* sampler;
	psy_audio_Instrument* instrument;
	psy_audio_Samples* samples;
	psy_dsp_ADSR env;
	psy_dsp_ADSR filterenv;
	Filter _filter;	
	psy_dsp_MultiResampler resampler;
	psy_List* positions;
	psy_dsp_amp_t vol;
	psy_dsp_amp_t pan;
	int usedefaultvolume;
	psy_audio_SamplerCmd effcmd;
	int effval;
	int dopan;
	int dooffset;
	uint8_t offset;
	int maxvolume;
	bool stopping;
	int _cutoff;
	float _coModify;
	double portaspeed;
	int period;
} psy_audio_SamplerVoice;

void psy_audio_samplervoice_init(psy_audio_SamplerVoice*,
	struct psy_audio_Sampler* sampler,
	psy_audio_Samples*,
	psy_audio_Instrument*,
	psy_audio_SamplerChannel* channel,
	uintptr_t channelnum, uintptr_t samplerate, int resamplingmethod,
	int maxvolume);
void psy_audio_samplervoice_dispose(psy_audio_SamplerVoice*);
psy_audio_SamplerVoice* psy_audio_samplervoice_alloc(void);
psy_audio_SamplerVoice* psy_audio_samplervoice_allocinit(struct psy_audio_Sampler*,
	psy_audio_Instrument*,
	psy_audio_SamplerChannel* channel,
	uintptr_t channelnum,
	uintptr_t samplerate);
void psy_audio_samplervoice_seqtick(psy_audio_SamplerVoice*,
	const psy_audio_PatternEvent*, double samplesprobeat);
void psy_audio_samplervoice_nna(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_noteon(psy_audio_SamplerVoice*,
	const psy_audio_PatternEvent*, double samplesprobeat);
void psy_audio_samplervoice_noteon_frequency(psy_audio_SamplerVoice*,
	double frequency);
void psy_audio_samplervoice_noteoff(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_fastnoteoff(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_work(psy_audio_SamplerVoice*, psy_audio_Buffer*,
	uintptr_t numsamples);
void psy_audio_samplervoice_release(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_fastrelease(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_clearpositions(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_setresamplerquality(psy_audio_SamplerVoice*,
	ResamplerType quality);
void psy_audio_samplervoice_tick(psy_audio_SamplerVoice*);

INLINE psy_dsp_amp_t psy_audio_samplervoice_volume(
	psy_audio_SamplerVoice* self, psy_audio_Sample* sample)
{
	psy_dsp_amp_t rv;
	psy_dsp_amp_t currrandvol = (psy_dsp_amp_t)1.f;

	// Since we have top +12dB in waveglobvolume and we have to clip randvol, we use the current globvol as top.
	// This isn't exactly what Impulse tracker did, but it's a reasonable compromise.
	// Instrument Global Volume [0..1.0f] Global volume affecting all samples of the instrument.
	rv = psy_audio_instrument_volume(self->instrument) * currrandvol * psy_audio_sample_volume(sample);
	if (rv > psy_audio_sample_volume(sample)) {
		rv = psy_audio_sample_volume(sample);
	}
	return rv;
}

typedef struct ZxxMacro {
	int32_t mode;
	int32_t value;
} ZxxMacro;

typedef void (*fp_samplerticktimer_ontick)(void* context);
typedef void (*fp_samplerticktimer_onwork)(void* context, psy_audio_BufferContext* bc);

typedef struct psy_audio_SamplerTickTimer {	
	uintptr_t counter;
	uintptr_t samplesprotick;
	int tickcount;
	fp_samplerticktimer_ontick tick;
	fp_samplerticktimer_onwork work;
	void* context;
} psy_audio_SamplerTickTimer;

void psy_audio_samplerticktimer_init(psy_audio_SamplerTickTimer*,
	void* context,
	fp_samplerticktimer_ontick,
	fp_samplerticktimer_onwork);
void psy_audio_samplerticktimer_reset(psy_audio_SamplerTickTimer*, uintptr_t samplesprotick);
void psy_audio_samplerticktimer_update(psy_audio_SamplerTickTimer*, uintptr_t amount,
	psy_audio_BufferContext* bc);

typedef struct psy_audio_Sampler {
	psy_audio_CustomMachine custommachine;		
	psy_List* voices;
	uintptr_t numvoices;
	int defaultspeed;	
	psy_Table lastinst;
	// psycle 0CFF, xm 0C80
	int maxvolume;
	// ps1 FALSE, sampulse TRUE
	int panpersistent;
	int xmsamplerload;
	psy_audio_IntMachineParam param_numvoices;
	psy_audio_ChoiceMachineParam param_resamplingmethod;
	psy_audio_ChoiceMachineParam param_defaultspeed;
	psy_audio_IntMachineParam param_maxvolume;
	psy_audio_ChoiceMachineParam param_panpersistent;
	psy_audio_IntMachineParam param_instrumentbank;
	psy_audio_CustomMachineParam param_blank;
	psy_audio_InfoMachineParam param_filter_cutoff;
	psy_audio_InfoMachineParam param_filter_res;
	psy_audio_InfoMachineParam param_pan;
	psy_audio_SamplerMasterChannel masterchannel;
	psy_audio_CustomMachineParam ignore_param;
	psy_audio_ChoiceMachineParam param_amigaslides;
	psy_Table channels;
	uint8_t basec;
	// Sampler PS1 with max amp = 0.5.
	psy_dsp_amp_t clipmax;
	// Instrument Bank 0: PS1 1: Sampulse
	int32_t instrumentbank;
	psy_audio_SamplerPanningMode panningmode;
	psy_List* cmds;
	psy_Table cmdmap;
	ResamplerType resamplerquality;
	psy_audio_SamplerTickTimer ticktimer;
	int samplecounter;
	int32_t amigaslides; // using linear or amiga slides.
	bool usefilters;
} psy_audio_Sampler;

void psy_audio_sampler_init(psy_audio_Sampler*, psy_audio_MachineCallback);
psy_audio_Sampler* psy_audio_sampler_alloc(void);
psy_audio_Sampler* psy_audio_sampler_allocinit(psy_audio_MachineCallback);
const psy_audio_MachineInfo* psy_audio_sampler_info(void);

INLINE psy_audio_Machine* psy_audio_sampler_base(psy_audio_Sampler* self)
{
	return &(self->custommachine.machine);
}

INLINE void psy_audio_sampler_defaultC4(psy_audio_Sampler* self, bool correct)
{
	self->basec = correct ? NOTECOMMANDS_MIDDLEC : 48;
}
			
INLINE bool psy_audio_sampler_isdefaultC4(psy_audio_Sampler* self)
{
	return self->basec == NOTECOMMANDS_MIDDLEC;
}

void psy_audio_sampler_setresamplerquality(psy_audio_Sampler* self,
	ResamplerType quality);

INLINE ResamplerType psy_audio_sampler_resamplerquality(psy_audio_Sampler* self)
{
	return self->resamplerquality;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SAMPLER_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_XMSAMPLERCHANNEL_H
#define psy_audio_XMSAMPLERCHANNEL_H

// local
#include "custommachine.h"
#include "instrument.h"
#include "sample.h"
#include "samplerdefs.h"
// dsp
#include <filter.h>
#include "multiresampler.h"
#include <valuemapper.h>
// container
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

// XMSampler::Channel Declaration

struct psy_audio_SongFile;
struct psy_audio_BufferContext;
struct psy_audio_PatternEvent;

struct psy_audio_XMSampler;
struct psy_audio_XMSamplerVoice;
struct psy_audio_SongFile;

typedef struct psy_audio_XMSamplerChannel {
	int m_Index;// Channel Index.
	struct psy_audio_XMSampler* m_pSampler;
	int m_InstrumentNo;///< ( 0 .. 255 )
	struct psy_audio_XMSamplerVoice* m_pForegroundVoice;

	int m_Note;
	double m_Period;

	float m_Volume;///<  (0..1.0f) value used for Playback (channel volume)
	int m_ChannelDefVolume;///< (0..200)   &0x100 = Mute. // value used for Storage and reset
	int m_LastVoiceVolume; // memory of note volume of last voice played.
	float m_LastVoiceRandVol; // memory of note volume of last voice played.
	bool m_bMute; // value used for playback.

	float m_PanFactor;// (0..1.0f) value used for Playback
	int m_DefaultPanFactor;  //  0..200 .  &0x100 == Surround. // value used for Storage and reset
	float m_LastVoicePanFactor;
	bool m_bSurround;// value used for playback (is channel set to surround?)

	int m_LastAmpEnvelopePosInSamples;
	int m_LastPanEnvelopePosInSamples;
	int m_LastFilterEnvelopePosInSamples;
	int m_LastPitchEnvelopePosInSamples;

	bool m_bGrissando;
	psy_audio_WaveForms m_VibratoType;///< vibrato type 
	psy_audio_WaveForms m_TremoloType;
	psy_audio_WaveForms m_PanbrelloType;


	int m_EffectFlags;

	int	m_PitchSlideSpeed;

	/// Global Volume Slide Speed
	float m_GlobalVolSlideSpeed;
	float m_ChanVolSlideSpeed;
	float m_PanSlideSpeed;

	int m_TremoloSpeed;
	int m_TremoloDepth;
	float m_TremoloDelta;
	int m_TremoloPos;

	// Panbrello
	int m_PanbrelloSpeed;
	int m_PanbrelloDepth;
	float m_PanbrelloDelta;
	int m_PanbrelloPos;

	/// Arpeggio  
	double m_ArpeggioPeriod[2];

	// Note Cut Command 
	int m_NoteCutTick;
	psy_List* m_DelayedNote; // std::vector<PatternEntry>

	int m_RetrigOperation;
	int m_RetrigVol;

	int m_PanSlideMem;
	int m_ChanVolSlideMem;
	int m_PitchSlideMem;
	int m_TremorMem;
	int m_TremorOnTime;
	int m_TremorOffTime;
	int m_VibratoDepthMem;
	int m_VibratoSpeedMem;
	int m_TremoloDepthMem;
	int m_TremoloSpeedMem;
	int m_PanbrelloDepthMem;
	int m_PanbrelloSpeedMem;
	int m_VolumeSlideMem;
	int m_GlobalVolSlideMem;
	int m_ArpeggioMem;
	int m_RetrigMem;
	int m_OffsetMem;
	//TODO: some way to let the user know the value set to this.
	int m_MIDI_Set;
	int m_Cutoff;
	int m_Ressonance;
	int m_DefaultCutoff;
	int m_DefaultRessonance;
	psy_dsp_FilterType m_DefaultFilterType;
	// paramview
	psy_audio_InfoMachineParam param_channel;
	psy_audio_IntMachineParam filter_cutoff;
	psy_audio_IntMachineParam filter_res;
	psy_audio_IntMachineParam pan;
	psy_audio_VolumeMachineParam slider_param;
	psy_audio_IntMachineParam level_param;
} psy_audio_XMSamplerChannel;

// mfc-psycle: constructor
void psy_audio_xmsamplerchannel_init(psy_audio_XMSamplerChannel*);
void psy_audio_xmsamplerchannel_dispose(psy_audio_XMSamplerChannel*);

bool  psy_audio_xmsamplerchannel_load(psy_audio_XMSamplerChannel*,
	struct psy_audio_SongFile*);
void psy_audio_xmsamplerchannel_save(psy_audio_XMSamplerChannel*,
	struct psy_audio_SongFile*);

void psy_audio_xmsamplerchannel_initparamview(psy_audio_XMSamplerChannel*);
// mfc-psycle: init
void psy_audio_xmsamplerchannel_initchannel(psy_audio_XMSamplerChannel*);
void psy_audio_xmsamplerchannel_effectinit(psy_audio_XMSamplerChannel*);
void psy_audio_xmsamplerchannel_restore(psy_audio_XMSamplerChannel*);

// Prepare the channel for the new effect (or execute if it's a one-shot one).
// This is executed on TrackerTick==0
void psy_audio_xmsamplerchannel_seteffect(psy_audio_XMSamplerChannel*,
	struct psy_audio_XMSamplerVoice*, int volcmd, int cmd, int parameter);

// Executes the slide/change effects. This is executed on TrackerTick!=0
void psy_audio_xmsamplerchannel_performfx(psy_audio_XMSamplerChannel*);

INLINE int psy_audio_xmsamplerchannel_effectflags(const psy_audio_XMSamplerChannel* self)
{
	return self->m_EffectFlags;
}

INLINE void psy_audio_xmsamplerchannel_seteffectflags(psy_audio_XMSamplerChannel* self, int value)
{
	self->m_EffectFlags = value;
}

// Effect-Related Object Functions

// Tick 0 commands
void psy_audio_xmsamplerchannel_globalvolslide(psy_audio_XMSamplerChannel*,
	int speed);
void psy_audio_xmsamplerchannel_panningslidespeed(psy_audio_XMSamplerChannel*,
	int speed);
void psy_audio_xmsamplerchannel_channelvolumeslidespeed(
	psy_audio_XMSamplerChannel*, int speed);
void psy_audio_xmsamplerchannel_panningslide(psy_audio_XMSamplerChannel*);
// default: int note = psy_audio_NOTECOMMANDS_empty
void psy_audio_xmsamplerchannel_pitchslide(psy_audio_XMSamplerChannel*,
	bool bUp, int speed, int note);
void psy_audio_xmsamplerchannel_volumeslide(psy_audio_XMSamplerChannel*,
	int speed);
void psy_audio_xmsamplerchannel_tremorparam(psy_audio_XMSamplerChannel*,
	int parameter);
// default: int depth = 0
void psy_audio_xmsamplerchannel_vibrato(psy_audio_XMSamplerChannel*,
	int speed, int depth);
void psy_audio_xmsamplerchannel_tremolo(psy_audio_XMSamplerChannel*,
	int speed, int depth);
void psy_audio_xmsamplerchannel_panbrello(psy_audio_XMSamplerChannel*,
	int speed, int depth);
void psy_audio_xmsamplerchannel_arpeggio(psy_audio_XMSamplerChannel*,
	int param);
void psy_audio_xmsamplerchannel_retrigger(psy_audio_XMSamplerChannel*,
	int parameter);
void psy_audio_xmsamplerchannel_delayednote(psy_audio_XMSamplerChannel*,
	psy_audio_PatternEvent data);

// Tick n commands.
void psy_audio_xmsamplerchannel_channelvolumeslide(psy_audio_XMSamplerChannel*);
void psy_audio_xmsamplerchannel_notecut(psy_audio_XMSamplerChannel*);
void psy_audio_xmsamplerchannel_notecuttick(psy_audio_XMSamplerChannel*, int ntick);
void psy_audio_xmsamplerchannel_stopbackgroundnotes(psy_audio_XMSamplerChannel*,
	psy_audio_NewNoteAction action);
double psy_audio_xmsamplerchannel_arpeggioperiod(const psy_audio_XMSamplerChannel*);

// Properties
INLINE int psy_audio_xmsamplerchannel_index(const psy_audio_XMSamplerChannel* self)
{
	return self->m_Index;
}

INLINE void psy_audio_xmsamplerchannel_setindex(psy_audio_XMSamplerChannel* self,
	int value)
{
	self->m_Index = value;
}

INLINE void psy_audio_xmsamplerchannel_setpsampler(psy_audio_XMSamplerChannel* self,
	struct psy_audio_XMSampler* pSampler)
{
	self->m_pSampler = pSampler;
}

INLINE int psy_audio_xmsamplerchannel_instrumentno(const psy_audio_XMSamplerChannel* self)
{
	return self->m_InstrumentNo;
}

INLINE void psy_audio_xmsamplerchannel_setinstrumentno(psy_audio_XMSamplerChannel* self, int no)
{
	self->m_InstrumentNo = no;
}

INLINE struct psy_audio_XMSamplerVoice* psy_audio_xmsamplerchannel_foregroundvoice(psy_audio_XMSamplerChannel* self)
{
	return self->m_pForegroundVoice;
}

void psy_audio_xmsamplerchannel_setforegroundvoice(psy_audio_XMSamplerChannel* self,
	struct psy_audio_XMSamplerVoice* pVoice);

INLINE int psy_audio_xmsamplerchannel_note(const psy_audio_XMSamplerChannel* self)
{
	return self->m_Note;
}

void psy_audio_xmsamplerchannel_setnote(psy_audio_XMSamplerChannel* self,
	int note);

INLINE double psy_audio_xmsamplerchannel_period(const psy_audio_XMSamplerChannel* self)
{
	return self->m_Period;
}

INLINE void psy_audio_xmsamplerchannel_setperiod(psy_audio_XMSamplerChannel* self,
	double value)
{
	self->m_Period = value;
}

INLINE float psy_audio_xmsamplerchannel_volume(const psy_audio_XMSamplerChannel* self)
{
	return self->m_Volume;
}

INLINE void psy_audio_xmsamplerchannel_setvolume(psy_audio_XMSamplerChannel* self, float value)
{
	self->m_Volume = value;
}

INLINE int psy_audio_xmsamplerchannel_defaultvolume(const psy_audio_XMSamplerChannel* self)
{
	return self->m_ChannelDefVolume & 0xFF;
}

INLINE bool psy_audio_xmsamplerchannel_defaultismute(const psy_audio_XMSamplerChannel* self)
{
	return self->m_ChannelDefVolume & 0x100;
}

INLINE void psy_audio_xmsamplerchannel_setdefaultismute(psy_audio_XMSamplerChannel* self, bool mute)
{
	if (mute) self->m_ChannelDefVolume |= 0x100;
	else self->m_ChannelDefVolume &= 0xFF;
	self->m_bMute = mute;
}

INLINE float psy_audio_xmsamplerchannel_defaultvolumefloat(const psy_audio_XMSamplerChannel* self)
{
	return (self->m_ChannelDefVolume & 0xFF) / 200.0f;
}

// default: bool updatecurrent = true
INLINE void psy_audio_xmsamplerchannel_setdefaultvolumefloat(psy_audio_XMSamplerChannel* self,
	float value, bool updatecurrent)
{
	if (psy_audio_xmsamplerchannel_defaultismute(self)) self->m_ChannelDefVolume = (int)((value * 200)) | 0x100;
	else self->m_ChannelDefVolume = (int)(value * 200);
	if (updatecurrent) psy_audio_xmsamplerchannel_setvolume(self,
		value);
}

// default: bool updatecurrent = true
INLINE void psy_audio_xmsamplerchannel_setdefaultvolume(psy_audio_XMSamplerChannel* self,
	int value, bool updatecurrent)
{
	if (psy_audio_xmsamplerchannel_defaultismute(self)) self->m_ChannelDefVolume = value | 0x100;
	else self->m_ChannelDefVolume = value;
	if (updatecurrent) {
		psy_audio_xmsamplerchannel_setvolume(self,
			psy_audio_xmsamplerchannel_defaultvolumefloat(self));
	}
}

INLINE int psy_audio_xmsamplerchannel_lastvoicevolume(const psy_audio_XMSamplerChannel* self)
{
	return self->m_LastVoiceVolume;
}

INLINE void psy_audio_xmsamplerchannel_setlastvoicevolume(psy_audio_XMSamplerChannel* self, int value)
{
	self->m_LastVoiceVolume = value;
}

INLINE int psy_audio_xmsamplerchannel_lastvoicerandvol(const psy_audio_XMSamplerChannel* self)
{
	return (int)self->m_LastVoiceRandVol;
}

INLINE void psy_audio_xmsamplerchannel_setlastvoicerandvol(psy_audio_XMSamplerChannel* self, float value)
{
	self->m_LastVoiceRandVol = value;
}

INLINE float psy_audio_xmsamplerchannel_panfactor(const psy_audio_XMSamplerChannel* self)
{
	return self->m_PanFactor;
}

void psy_audio_xmsamplerchannel_setpanfactor(psy_audio_XMSamplerChannel* self, float value);

INLINE int psy_audio_xmsamplerchannel_defaultpanfactor(const psy_audio_XMSamplerChannel* self)
{
	return self->m_DefaultPanFactor;
}

INLINE float psy_audio_xmsamplerchannel_defaultpanfactorfloat(const psy_audio_XMSamplerChannel* self)
{
	return (self->m_DefaultPanFactor & 0xFF) / 200.0f;
}

INLINE bool psy_audio_xmsamplerchannel_defaultissurround(const psy_audio_XMSamplerChannel* self)
{
	return (self->m_DefaultPanFactor & 0x100);
}

INLINE float psy_audio_xmsamplerchannel_lastvoicepanfactor(const psy_audio_XMSamplerChannel* self)
{
	return self->m_LastVoicePanFactor;
}

INLINE void psy_audio_xmsamplerchannel_setlastvoicepanfactor(psy_audio_XMSamplerChannel* self, float value)
{
	self->m_LastVoicePanFactor = value;
}

INLINE int psy_audio_xmsamplerchannel_lastampenvelopeposinsamples(const psy_audio_XMSamplerChannel* self)
{
	return self->m_LastAmpEnvelopePosInSamples;
}

INLINE void psy_audio_xmsamplerchannel_setlastampenvelopeposinsamples(psy_audio_XMSamplerChannel* self, int value)
{
	self->m_LastAmpEnvelopePosInSamples = value;
}

INLINE int psy_audio_xmsamplerchannel_lastpanenvelopeposinsamples(const psy_audio_XMSamplerChannel* self)
{
	return self->m_LastPanEnvelopePosInSamples;
}

INLINE void psy_audio_xmsamplerchannel_setlastpanenvelopeposinsamples(psy_audio_XMSamplerChannel* self, int value)
{
	self->m_LastPanEnvelopePosInSamples = value;
}

INLINE int psy_audio_xmsamplerchannel_lastfilterenvelopeposinsamples(const psy_audio_XMSamplerChannel* self)
{
	return self->m_LastFilterEnvelopePosInSamples;
}

INLINE void psy_audio_xmsamplerchannel_setlastfilterenvelopeposinsamples(psy_audio_XMSamplerChannel* self, int value)
{
	self->m_LastFilterEnvelopePosInSamples = value;
}

INLINE int psy_audio_xmsamplerchannel_lastpitchenvelopeposinsamples(const psy_audio_XMSamplerChannel* self)
{
	return self->m_LastPitchEnvelopePosInSamples;
}

INLINE void psy_audio_xmsamplerchannel_setlastpitchenvelopeposinsamples(psy_audio_XMSamplerChannel* self, int value)
{
	self->m_LastPitchEnvelopePosInSamples = value;
}

INLINE int psy_audio_xmsamplerchannel_offsetmem(const psy_audio_XMSamplerChannel* self)
{
	return self->m_OffsetMem;
}

INLINE void psy_audio_xmsamplerchannel_setoffsetmem(psy_audio_XMSamplerChannel* self, int value)
{
	self->m_OffsetMem = value;
}

INLINE bool psy_audio_xmsamplerchannel_issurround(const psy_audio_XMSamplerChannel* self)
{
	return self->m_bSurround;
}

INLINE void psy_audio_xmsamplerchannel_setissurround(psy_audio_XMSamplerChannel* self, int value)
{
	self->m_bSurround = value;
}

INLINE bool psy_audio_xmsamplerchannel_ismute(const psy_audio_XMSamplerChannel* self)
{
	return self->m_bMute;
}

INLINE int psy_audio_xmsamplerchannel_cutoff(const psy_audio_XMSamplerChannel* self)
{
	return self->m_Cutoff;
}

INLINE int psy_audio_xmsamplerchannel_ressonance(const psy_audio_XMSamplerChannel* self)
{
	return self->m_Ressonance;
}

INLINE int psy_audio_xmsamplerchannel_defaultcutoff(const psy_audio_XMSamplerChannel* self)
{
	return self->m_DefaultCutoff;
}

INLINE int psy_audio_xmsamplerchannel_defaultressonance(const psy_audio_XMSamplerChannel* self)
{
	return self->m_DefaultRessonance;
}

INLINE psy_dsp_FilterType psy_audio_xmsamplerchannel_defaultfiltertype(const psy_audio_XMSamplerChannel* self)
{
	return self->m_DefaultFilterType;
}

INLINE void psy_audio_xmsamplerchannel_setdefaultfiltertype(psy_audio_XMSamplerChannel* self, psy_dsp_FilterType value)
{
	self->m_DefaultFilterType = value;
}

INLINE bool psy_audio_xmsamplerchannel_isgrissando(const psy_audio_XMSamplerChannel* self)
{
	return self->m_bGrissando;
}

INLINE void psy_audio_xmsamplerchannel_setisgrissando(psy_audio_XMSamplerChannel* self, bool value)
{
	self->m_bGrissando = value;
}

INLINE void psy_audio_xmsamplerchannel_setvibratotype(psy_audio_XMSamplerChannel* self,
	psy_audio_WaveForms value)
{
	self->m_VibratoType = value;
}

INLINE psy_audio_WaveForms psy_audio_xmsamplerchannel_vibratotype(const psy_audio_XMSamplerChannel* self)
{
	return self->m_VibratoType;
}

INLINE void psy_audio_xmsamplerchannel_settremolotype(psy_audio_XMSamplerChannel* self, 
	psy_audio_WaveForms type)
{
	self->m_TremoloType = type;
}

INLINE psy_audio_WaveForms psy_audio_xmsamplerchannel_tremolotype(const psy_audio_XMSamplerChannel* self)
{
	return self->m_TremoloType;
}

INLINE void psy_audio_xmsamplerchannel_setpanbrellotype(psy_audio_XMSamplerChannel* self, psy_audio_WaveForms type) {
	self->m_PanbrelloType = type;
}

INLINE psy_audio_WaveForms psy_audio_xmsamplerchannel_panbrellotype(const psy_audio_XMSamplerChannel* self)
{
	return self->m_PanbrelloType;
}

INLINE bool psy_audio_xmsamplerchannel_isarpeggio(const psy_audio_XMSamplerChannel* self)
{
	return ((self->m_EffectFlags & XM_SAMPLER_EFFECT_ARPEGGIO) != 0);
}

INLINE bool psy_audio_xmsamplerchannel_isvibrato(const psy_audio_XMSamplerChannel* self)
{
	return (self->m_EffectFlags & XM_SAMPLER_EFFECT_VIBRATO) != 0;
}

INLINE bool psy_audio_xmsamplerchannel_isdelayed(const psy_audio_XMSamplerChannel* self)
{
	return self->m_DelayedNote != NULL;	
};

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_XMSAMPLERCHANNEL_H */

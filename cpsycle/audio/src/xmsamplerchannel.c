// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "xmsamplerchannel.h"

#include "xmsamplervoice.h"
#include "xmsampler.h"

#include "samplerdefs.h"
#include "machineparam.h"
#include "songio.h"
#include "plugin_interface.h"

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "../../detail/portable.h"
#include "../../detail/trace.h"
#include "constants.h"

void psy_audio_xmsamplerchannel_init(psy_audio_XMSamplerChannel* self)
{
	self->m_Index = 0;
	psy_audio_xmsamplerchannel_initchannel(self);
}

void psy_audio_xmsamplerchannel_initchannel(psy_audio_XMSamplerChannel* self)
{	
	self->m_Index = 0;

	self->m_InstrumentNo = psy_audio_NOTECOMMANDS_INST_EMPTY;
	self->m_pForegroundVoice = NULL;

	self->m_Note = psy_audio_NOTECOMMANDS_EMPTY;
	self->m_Period = 0;

	self->m_ChannelDefVolume = 200;// 0..200 , &0x100 = Mute.
	self->m_bMute = FALSE;

	self->m_DefaultPanFactor = 100;

	self->m_DefaultCutoff = 127;
	self->m_DefaultRessonance = 0;
	self->m_DefaultFilterType = F_NONE;

	self->m_DelayedNote = NULL;
	psy_audio_xmsamplerchannel_restore(self);
}

void psy_audio_xmsamplerchannel_dispose(psy_audio_XMSamplerChannel* self)
{

}

void psy_audio_xmsamplerchannel_effectinit(psy_audio_XMSamplerChannel* self)
{
	//Effects
	self->m_EffectFlags = 0;

	self->m_PitchSlideSpeed = 0;

	self->m_GlobalVolSlideSpeed = 0.0f;
	self->m_ChanVolSlideSpeed = 0.0f;
	self->m_PanSlideSpeed = 0.0f;

	self->m_TremoloSpeed = 0;
	self->m_TremoloDepth = 0;
	self->m_TremoloDelta = 0;
	self->m_TremoloPos = 0;

	self->m_PanbrelloSpeed = 0;
	self->m_PanbrelloDepth = 0;
	self->m_PanbrelloDelta = 0;
	self->m_PanbrelloPos = 0;

	self->m_bGrissando = FALSE;
	self->m_VibratoType = psy_audio_WAVEFORMS_SINUS;
	self->m_TremoloType = psy_audio_WAVEFORMS_SINUS;
	self->m_PanbrelloType = psy_audio_WAVEFORMS_SINUS;

	self->m_TremorOnTime = 0;
	self->m_TremorOffTime = 0;

	self->m_RetrigOperation = 0;
	self->m_RetrigVol = 0;

	self->m_ArpeggioPeriod[0] = 0.0;
	self->m_ArpeggioPeriod[1] = 0.0;

	self->m_NoteCutTick = 0;

	self->m_MIDI_Set = 0;

	//Memory
	self->m_PanSlideMem = 0;
	self->m_ChanVolSlideMem = 0;
	self->m_PitchSlideMem = 0;
	self->m_TremorMem = 0;
	self->m_VibratoDepthMem = 0;
	self->m_VibratoSpeedMem = 0;
	self->m_TremoloDepthMem = 0;
	self->m_TremoloSpeedMem = 0;
	self->m_PanbrelloDepthMem = 0;
	self->m_PanbrelloSpeedMem = 0;
	self->m_VolumeSlideMem = 0;
	self->m_ArpeggioMem = 0;
	self->m_RetrigMem = 0;
	self->m_OffsetMem = 0;
	self->m_GlobalVolSlideMem = 0;

}

void psy_audio_xmsamplerchannel_restore(psy_audio_XMSamplerChannel* self)
{
	self->m_InstrumentNo = psy_audio_NOTECOMMANDS_INST_EMPTY;
	self->m_pForegroundVoice = NULL;

	self->m_LastVoiceVolume = 0;
	self->m_LastVoiceRandVol = 1.0f;
	self->m_LastVoicePanFactor = 0.0f;
	self->m_LastAmpEnvelopePosInSamples = 0;
	self->m_LastPanEnvelopePosInSamples = 0;
	self->m_LastFilterEnvelopePosInSamples = 0;
	self->m_LastPitchEnvelopePosInSamples = 0;

	self->m_Volume = psy_audio_xmsamplerchannel_defaultvolumefloat(self);
	self->m_PanFactor = psy_audio_xmsamplerchannel_defaultpanfactorfloat(self);
	self->m_bSurround = psy_audio_xmsamplerchannel_defaultissurround(self);

	psy_audio_xmsamplerchannel_effectinit(self);
	self->m_Cutoff = self->m_DefaultCutoff;
	self->m_Ressonance = self->m_DefaultRessonance;
	psy_list_deallocate(&self->m_DelayedNote, NULL);	
}

void psy_audio_xmsamplerchannel_setforegroundvoice(psy_audio_XMSamplerChannel* self,
	struct psy_audio_XMSamplerVoice* pVoice)
{
	if (self->m_pForegroundVoice)
	{
		const psy_dsp_EnvelopeSettings* env;

		psy_audio_xmsamplerchannel_setlastvoicepanfactor(
			self, psy_audio_xmsamplervoice_panfactor(self->m_pForegroundVoice));
		psy_audio_xmsamplerchannel_setlastvoicevolume(
			self, psy_audio_xmsamplervoice_volume(self->m_pForegroundVoice));
		psy_audio_xmsamplerchannel_setlastvoicerandvol(
			self, psy_audio_xmsamplervoice_currrandvol(self->m_pForegroundVoice));

		env = xmenvelopecontroller_envelope(psy_audio_xmsamplervoice_amplitudeenvelope(self->m_pForegroundVoice));
		if (psy_dsp_envelopesettings_isenabled(env) && psy_dsp_envelopesettings_iscarry(env))
			psy_audio_xmsamplerchannel_setlastampenvelopeposinsamples(self,
				xmenvelopecontroller_getpositioninsamples(
					psy_audio_xmsamplervoice_amplitudeenvelope(self->m_pForegroundVoice)));
		else psy_audio_xmsamplerchannel_setlastampenvelopeposinsamples(self, 0);
		env = xmenvelopecontroller_envelope(psy_audio_xmsamplervoice_panenvelope(self->m_pForegroundVoice));
		if (psy_dsp_envelopesettings_isenabled(env) && psy_dsp_envelopesettings_iscarry(env))
			psy_audio_xmsamplerchannel_setlastpanenvelopeposinsamples(self,
				xmenvelopecontroller_getpositioninsamples(
					psy_audio_xmsamplervoice_panenvelope(self->m_pForegroundVoice)));
		else psy_audio_xmsamplerchannel_setlastpanenvelopeposinsamples(self, 0);
		env = xmenvelopecontroller_envelope(psy_audio_xmsamplervoice_filterenvelope(self->m_pForegroundVoice));
		if (psy_dsp_envelopesettings_isenabled(env) && psy_dsp_envelopesettings_iscarry(env))
			psy_audio_xmsamplerchannel_setlastfilterenvelopeposinsamples(self,
				xmenvelopecontroller_getpositioninsamples(
					psy_audio_xmsamplervoice_filterenvelope(self->m_pForegroundVoice)));
		else psy_audio_xmsamplerchannel_setlastfilterenvelopeposinsamples(self, 0);
		env = xmenvelopecontroller_envelope(psy_audio_xmsamplervoice_pitchenvelope(self->m_pForegroundVoice));
		if (psy_dsp_envelopesettings_isenabled(env) && psy_dsp_envelopesettings_iscarry(env))
			psy_audio_xmsamplerchannel_setlastpitchenvelopeposinsamples(self,
				xmenvelopecontroller_getpositioninsamples(
					psy_audio_xmsamplervoice_pitchenvelope(self->m_pForegroundVoice)));
		else psy_audio_xmsamplerchannel_setlastpitchenvelopeposinsamples(self, 0);
				
		if (self->m_pForegroundVoice != pVoice) {
			psy_audio_xmsamplervoice_setisbackground(
			self->m_pForegroundVoice, TRUE);
		}
	} else {
		psy_audio_xmsamplerchannel_setlastampenvelopeposinsamples(self, 0);
		psy_audio_xmsamplerchannel_setlastpanenvelopeposinsamples(self, 0);
		psy_audio_xmsamplerchannel_setlastfilterenvelopeposinsamples(self, 0);
		psy_audio_xmsamplerchannel_setlastpitchenvelopeposinsamples(self, 0);
	}
	self->m_pForegroundVoice = pVoice;
}

void psy_audio_xmsamplerchannel_setnote(psy_audio_XMSamplerChannel* self, int note)
{
	self->m_Note = note;
	if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
		self->m_Period = psy_audio_xmsamplervoice_notetoperiod(
			psy_audio_xmsamplerchannel_foregroundvoice(self), note, TRUE);
	}
}

void psy_audio_xmsamplerchannel_setpanfactor(psy_audio_XMSamplerChannel* self, float value)
{
	self->m_PanFactor = value;
	if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
		psy_audio_xmsamplervoice_setpanfactor(
			psy_audio_xmsamplerchannel_foregroundvoice(self),
			value);
	}
}

void psy_audio_xmsamplerchannel_seteffect(psy_audio_XMSamplerChannel* self,
	psy_audio_XMSamplerVoice* voice, int volcmd, int cmd, int parameter)
{
	int realSet = 0;
	int	realValue = 0;

	//1st check: Channel ( They can appear without an existing playing note and are persistent when a new one comes)
	switch (volcmd & 0xF0)
	{
	case XM_SAMPLER_CMD_VOL_PANNING:
		psy_audio_xmsamplerchannel_setpanfactor(self,
			(volcmd & 0x0F) / 15.0f);
		break;
	case XM_SAMPLER_CMD_VOL_PANSLIDELEFT:
		//this command is actually fine pan slide
		psy_audio_xmsamplerchannel_panningslidespeed(self,
			(volcmd & 0x0F) << 4);
		break;
	case XM_SAMPLER_CMD_VOL_PANSLIDERIGHT:
		//this command is actually fine pan slide
		psy_audio_xmsamplerchannel_panningslidespeed(self,
			volcmd & 0x0F);
		break;
	default:
		break;
	}

	switch (cmd)
	{
	case XM_SAMPLER_CMD_PANNING:
		psy_audio_xmsamplerchannel_setissurround(self,
			FALSE);
		psy_audio_xmsamplerchannel_setpanfactor(self,
			parameter / 255.0f);
		break;
	case XM_SAMPLER_CMD_SET_CHANNEL_VOLUME:
		psy_audio_xmsamplerchannel_setvolume(self,
			(parameter < 64) ? (parameter / 64.0f) : 1.0f);
		break;
	case XM_SAMPLER_CMD_PANNINGSLIDE:
		psy_audio_xmsamplerchannel_panningslidespeed(self, parameter);
		break;
	case XM_SAMPLER_CMD_CHANNEL_VOLUMESLIDE:
		// psy_audio_xmsamplerchannel_channelvolumeslide(self, parameter);
		break;
	case XM_SAMPLER_CMD_SET_GLOBAL_VOLUME:
		psy_audio_xmsampler_setglobalvolume(self->m_pSampler,
			parameter < 0x80 ? parameter : 0x80);
		break;
	case XM_SAMPLER_CMD_GLOBAL_VOLUME_SLIDE:
		psy_audio_xmsamplerchannel_globalvolslide(self,
			parameter);
		break;
	case XM_SAMPLER_CMD_EXTENDED:
		switch (parameter & 0xF0)
		{
		case XM_SAMPLER_CMD_E9:
			switch (parameter & 0x0F)
			{
			case XM_SAMPLER_CMD_E9_SURROUND_OFF:
				psy_audio_xmsamplerchannel_setissurround(self, FALSE);
				break;
			case XM_SAMPLER_CMD_E9_SURROUND_ON:
				psy_audio_xmsamplerchannel_setissurround(self, TRUE);
				break;
			case XM_SAMPLER_CMD_E9_REVERB_OFF:
				break;
			case XM_SAMPLER_CMD_E9_REVERB_FORCE:
				break;
			case XM_SAMPLER_CMD_E9_STANDARD_SURROUND:
				break;
			case XM_SAMPLER_CMD_E9_QUAD_SURROUND:
				break;
			case XM_SAMPLER_CMD_E9_GLOBAL_FILTER:
				break;
			case XM_SAMPLER_CMD_E9_LOCAL_FILTER:
				break;
			default:
				break;
			}
			break;
		case XM_SAMPLER_CMD_E_SET_PAN:
			// psy_audio_xmsamplerchannel_setpanfactor(XMSampler::E8VolMap[(parameter & 0xf)] / 64.0f);
			break;
		case XM_SAMPLER_CMD_E_SET_MIDI_MACRO:
			//\todo : implement. For now, it maps directly to internal Filter commands
			self->m_MIDI_Set = parameter & 0x0F;
			break;
		case XM_SAMPLER_CMD_E_GLISSANDO_TYPE:
			psy_audio_xmsamplerchannel_setisgrissando(self,
				parameter != 0);
			break;
		case XM_SAMPLER_CMD_E_VIBRATO_WAVE:
			if (parameter <= psy_audio_WAVEFORMS_RANDOM) {
			// 	VibratoType(static_cast<XMInstrument::WaveData<>::WaveForms::Type>(parameter));				
			} else {
				// psy_audio_xmsamplerchannel_setvibratotype(self, psy_audio_WAVEFORMS_SINUS);
			}
			break;
		case XM_SAMPLER_CMD_E_PANBRELLO_WAVE:
			if (parameter <= psy_audio_WAVEFORMS_RANDOM) {
			// 	PanbrelloType(static_cast<XMInstrument::WaveData<>::WaveForms::Type>(parameter));
			} else {
				// PanbrelloType(XMInstrument::WaveData<>::WaveForms::SINUS);
			}
			break;
		case XM_SAMPLER_CMD_E_TREMOLO_WAVE:
			if (parameter <= psy_audio_WAVEFORMS_RANDOM) {
			// 	TremoloType(static_cast<XMInstrument::WaveData<>::WaveForms::Type>(parameter));
			} else {
				// TremoloType(XMInstrument::WaveData<>::WaveForms::SINUS);
			}
			break;
		default:
			break;
		}
		break;
	case XM_SAMPLER_CMD_MIDI_MACRO:
		if (parameter < 0x80)
		{
			realSet = self->m_MIDI_Set;
			realValue = parameter;
		} else
		{
			//realSet = m_pSampler->GetMap(parameter - 0x80).mode;
			//srealValue = m_pSampler->GetMap(parameter - 0x80).value;
		}
		switch (realSet)
		{
		case 0:
			self->m_Cutoff = realValue;
			if (voice)
			{
			//	if (voice->psy_dsp_FilterType() == dsp::F_NONE) voice->psy_dsp_FilterType(m_DefaultFilterType);				
				psy_audio_xmsamplervoice_setcutoff(voice,
					self->m_Cutoff);
			}
			break;
		case 1:
			self->m_Ressonance = realValue;
			if (voice)
			{
				//if (voice->psy_dsp_FilterType() == dsp::F_NONE) voice->psy_dsp_FilterType(m_DefaultFilterType);
				psy_audio_xmsamplervoice_setressonance(voice,
					self->m_Ressonance);
			}
			break;
		case 2:
			//Set filter mode. OpenMPT only says 0..F lowpass and 10..1F highpass.
			// It also has a macro default setup where 0 and 8 set the lowpass and 10 an 18 set the highpass
			// From there, I adapted the following table for Psycle.
			if (realValue < 0x20)
			{
				if (realValue < 4) { //0..3
					self->m_DefaultFilterType = F_ITLOWPASS;
				} else if (realValue < 6) { //4..5
					self->m_DefaultFilterType = F_LOWPASS12;
				} else if (realValue < 8) { //6..7
					self->m_DefaultFilterType = F_BANDPASS12;
				} else if (realValue < 0xC) { //8..B
					self->m_DefaultFilterType = F_MPTLOWPASSE;
				} else if (realValue < 0xE) { //C..D
					self->m_DefaultFilterType = F_LOWPASS12E;
				} else if (realValue < 0x10) { //E..F
					self->m_DefaultFilterType = F_BANDPASS12E;
				} else if (realValue < 0x14) { //10..13
					self->m_DefaultFilterType = F_MPTHIGHPASSE;
				} else if (realValue < 0x16) { //14..15
					self->m_DefaultFilterType = F_HIGHPASS12;
				} else if (realValue < 0x18) { //16..17
					self->m_DefaultFilterType = F_BANDREJECT12;
				} else if (realValue < 0x1C) { //18..1B
					self->m_DefaultFilterType = F_MPTHIGHPASSE;
				} else if (realValue < 0x1E) { //1C..1D
					self->m_DefaultFilterType = F_HIGHPASS12E;
				} else { // 1E..1F
					self->m_DefaultFilterType = F_BANDREJECT12E;
				}
				if (voice) {
					psy_audio_xmsamplervoice_setfiltertype(voice,
						self->m_DefaultFilterType);
				}
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	// 2nd Check. Commands that require a voice.
	int slidval = 0;
	if (voice)
	{
		switch (volcmd & 0xF0)
		{
		case XM_SAMPLER_CMD_VOL_VOLUME0:
		case XM_SAMPLER_CMD_VOL_VOLUME1:
		case XM_SAMPLER_CMD_VOL_VOLUME2:
		case XM_SAMPLER_CMD_VOL_VOLUME3:
			// voice->Volume(volcmd << 1);
			break;
		case XM_SAMPLER_CMD_VOL_VOLSLIDEUP:
			// VolumeSlide((volcmd & 0x0F) << 4);
			break;
		case XM_SAMPLER_CMD_VOL_VOLSLIDEDOWN:
			// VolumeSlide(volcmd & 0x0F);
			break;
		case XM_SAMPLER_CMD_VOL_FINEVOLSLIDEUP:
			// voice->m_VolumeSlideSpeed = (volcmd & 0x0F) << 1;
			// voice->VolumeSlide();
			break;
		case XM_SAMPLER_CMD_VOL_FINEVOLSLIDEDOWN:
			// voice->m_VolumeSlideSpeed = -((volcmd & 0x0F) << 1);
			// voice->VolumeSlide();
			break;
		//	/*				case CMD_VOL::VOL_VIBRATO_SPEED:
		//						Vibrato(volcmd&0x0F,0); //\todo: vibrato_speed does not activate the vibrato if it isn't running.
		//						break;
		//	*/
/*
		case CMD_VOL::VOL_VIBRATO:
			Vibrato(0, (volcmd & 0x0F) << 2);
			break;
		case CMD_VOL::VOL_TONEPORTAMENTO:
			// Portamento to (Gx) affects the memory for Gxx and has the equivalent
			// slide given by this table:
			// SlideTable      DB      1, 4, 8, 16, 32, 64, 96, 128, 255
			if ((volcmd & 0x0F) == 0) slidval = 0;
			else if ((volcmd & 0x0F) == 1)  slidval = 1;
			else if ((volcmd & 0x0F) < 9) slidval = powf(2.0f, volcmd & 0x0F);
			else slidval = 255;
			PitchSlide(voice->Period() > voice->NoteToPeriod(Note()), slidval, Note());
			break;
		case CMD_VOL::VOL_PITCH_SLIDE_DOWN:
			// Pitch slide up/down affect E/F/(G)'s memory - a Pitch slide
			// up/down of x is equivalent to a normal slide by x*4
			PitchSlide(false, (volcmd & 0x0F) << 2);
			break;
		case CMD_VOL::VOL_PITCH_SLIDE_UP:
			PitchSlide(true, (volcmd & 0x0F) << 2);
			break;
		default:
			break;
		}

		switch (cmd)
		{
			// Class A: Voice ( They can apply to an already playing voice, or a new coming one).
		case CMD::VOLUME:
			voice->Volume(parameter);
			break;
		case CMD::SET_ENV_POSITION:
			if (voice->AmplitudeEnvelope().Envelope().IsEnabled())
				voice->AmplitudeEnvelope().SetPositionInSamples(parameter * Global::player().SamplesPerTick());
			if (voice->PanEnvelope().Envelope().IsEnabled())
				voice->PanEnvelope().SetPositionInSamples(parameter * Global::player().SamplesPerTick());
			if (voice->PitchEnvelope().Envelope().IsEnabled())
				voice->PitchEnvelope().SetPositionInSamples(parameter * Global::player().SamplesPerTick());
			if (voice->FilterEnvelope().Envelope().IsEnabled())
				voice->FilterEnvelope().SetPositionInSamples(parameter * Global::player().SamplesPerTick());
			break;
		case CMD::EXTENDED:
			switch (parameter & 0xF0)
			{
			case CMD_E::E9:
				switch (parameter & 0x0F)
				{
				case CMD_E9::E9_PLAY_FORWARD:
					voice->rWave().ChangeLoopDirection(WaveDataController<>::LoopDirection::FORWARD);
					break;
				case CMD_E9::E9_PLAY_BACKWARD:
					if (voice->rWave().Position() == 0)
					{
						voice->rWave().Position(voice->rWave().Length() - 1);
					}
					voice->rWave().ChangeLoopDirection(WaveDataController<>::LoopDirection::BACKWARD);
					break;
				}
				break;
			case CMD_E::EE:
				switch (parameter & 0x0F)
				{
				case CMD_EE::EE_SETNOTECUT:
					voice->NNA(XMInstrument::NewNoteAction::STOP);
					break;
				case CMD_EE::EE_SETNOTECONTINUE:
					voice->NNA(XMInstrument::NewNoteAction::CONTINUE);
					break;
				case CMD_EE::EE_SETNOTEOFF:
					voice->NNA(XMInstrument::NewNoteAction::NOTEOFF);
					break;
				case CMD_EE::EE_SETNOTEFADE:
					voice->NNA(XMInstrument::NewNoteAction::FADEOUT);
					break;
				case CMD_EE::EE_VOLENVOFF:
					voice->AmplitudeEnvelope().Stop();
					break;
				case CMD_EE::EE_VOLENVON:
					voice->AmplitudeEnvelope().Start();
					break;
				case CMD_EE::EE_PANENVOFF:
					voice->PanEnvelope().Stop();
					break;
				case CMD_EE::EE_PANENVON:
					voice->PanEnvelope().Start();
					break;
				case CMD_EE::EE_PITCHENVON:
					voice->PitchEnvelope().Stop();
					break;
				case CMD_EE::EE_PITCHENVOFF:
					voice->PitchEnvelope().Start();
					break;
				}
				break;
			case CMD_E::E_DELAYED_NOTECUT:
				NoteCut(parameter & 0x0F);
				break;
			}
			break;

			// Class B Channel ( Just like Class A, but remember its old value if it is called again with  00 as parameter  )

		case CMD::PORTAMENTO_UP:
			PitchSlide(true, parameter);
			break;
		case CMD::PORTAMENTO_DOWN:
			PitchSlide(false, parameter);
			break;
		case CMD::PORTA2NOTE:
			PitchSlide(voice->Period() > voice->NoteToPeriod(Note()), parameter, Note());
			break;
		case CMD::VOLUMESLIDE:
			VolumeSlide(parameter);
			break;
		case CMD::TONEPORTAVOL:
			VolumeSlide(parameter);
			PitchSlide(voice->Period() > voice->NoteToPeriod(Note()), 0, Note());
			break;
		case CMD::VIBRATOVOL:
			VolumeSlide(parameter);
			Vibrato(0);
			break;
		case CMD::VIBRATO:
			Vibrato(((parameter >> 4) & 0x0F), (parameter & 0x0F) << 2);
			break;
		case CMD::FINE_VIBRATO:
			Vibrato(((parameter >> 4) & 0x0F), (parameter & 0x0F));
			break;
		case CMD::TREMOR:
			Tremor(parameter);
			break;
		case CMD::TREMOLO:
			Tremolo((parameter >> 4) & 0x0F, (parameter & 0x0F));
			break;
		case CMD::RETRIG:
			Retrigger(parameter);
			break;
		case CMD::PANBRELLO:
			Panbrello((parameter >> 4) & 0x0F, (parameter & 0x0F));
			break;
		case CMD::ARPEGGIO:
			Arpeggio(parameter);
			break;
			*/
		}
	}
	//3rd check: It is not needed that the voice is playing, but it applies to the last instrument.
	/*if (InstrumentNo() != 255 && cmd == CMD::EXTENDED && (parameter & 0xF0) == CMD_E::EE)
	{
		switch (parameter & 0x0F)
		{
		case CMD_EE::EE_BACKGROUNDNOTECUT:
			StopBackgroundNotes(XMInstrument::NewNoteAction::STOP);
			break;
		case CMD_EE::EE_BACKGROUNDNOTEOFF:
			StopBackgroundNotes(XMInstrument::NewNoteAction::NOTEOFF);
			break;
		case CMD_EE::EE_BACKGROUNDNOTEFADE:
			StopBackgroundNotes(XMInstrument::NewNoteAction::FADEOUT);
			break;
		}
	}
	*/
}

// Add Here those commands that have an effect each tracker tick ( 1 and onwards) .
		// tick "0" is worked on in channel.SetEffect();
void psy_audio_xmsamplerchannel_performfx(psy_audio_XMSamplerChannel* self)
{
	int i;

	/*
	for (i = 0; i < psy_audio_xmsampler_numvoices(self->m_pSampler); i++)
	{
		if (m_pSampler->rVoice(i).ChannelNum() == m_Index && m_pSampler->rVoice(i).IsPlaying())  m_pSampler->rVoice(i).Tick();
	}
	if (ForegroundVoice()) // Effects that need a voice to be active.
	{
		if (ForegroundVoice()->IsAutoVibrato())
		{
			ForegroundVoice()->AutoVibrato();
		}
		if (EffectFlags() & EffectFlag::PITCHSLIDE)
		{
			ForegroundVoice()->PitchSlide();
		}
		if (EffectFlags() & EffectFlag::SLIDE2NOTE)
		{
			ForegroundVoice()->Slide2Note();
		}
		if (EffectFlags() & EffectFlag::VIBRATO)
		{
			ForegroundVoice()->Vibrato();
		}
		if (EffectFlags() & EffectFlag::TREMOLO)
		{
			ForegroundVoice()->Tremolo();
		}
		if (EffectFlags() & EffectFlag::PANBRELLO)
		{
			ForegroundVoice()->Panbrello();
		}
		if (EffectFlags() & EffectFlag::TREMOR)
		{
			ForegroundVoice()->Tremor();
		}
		if (EffectFlags() & EffectFlag::VOLUMESLIDE)
		{
			ForegroundVoice()->VolumeSlide();
		}
		if (EffectFlags() & EffectFlag::NOTECUT)
		{
			NoteCut();
		}
		if (EffectFlags() & EffectFlag::ARPEGGIO)
		{
			ForegroundVoice()->UpdateSpeed();
		}
		if (EffectFlags() & EffectFlag::RETRIG)
		{
			LastVoicePanFactor(ForegroundVoice()->PanFactor());

			if (m_RetrigOperation == 1)
			{
				int tmp = ForegroundVoice()->Volume() + m_RetrigVol;
				if (tmp < 0) tmp = 0;
				else if (tmp > 128) tmp = 128;
				LastVoiceVolume(tmp);
			} else if (m_RetrigOperation == 2)
			{
				int tmp = ForegroundVoice()->Volume() * m_RetrigVol;
				if (tmp < 0) tmp = 0;
				else if (tmp > 128) tmp = 128;
				LastVoiceVolume(tmp);
			}

			if (ForegroundVoice()->AmplitudeEnvelope().Envelope().IsEnabled())
				LastAmpEnvelopePosInSamples(ForegroundVoice()->AmplitudeEnvelope().GetPositionInSamples());
			if (ForegroundVoice()->PanEnvelope().Envelope().IsEnabled())
				LastPanEnvelopePosInSamples(ForegroundVoice()->PanEnvelope().GetPositionInSamples());
			if (ForegroundVoice()->FilterEnvelope().Envelope().IsEnabled())
				LastFilterEnvelopePosInSamples(ForegroundVoice()->FilterEnvelope().GetPositionInSamples());
			if (ForegroundVoice()->PitchEnvelope().Envelope().IsEnabled())
				LastPitchEnvelopePosInSamples(ForegroundVoice()->PitchEnvelope().GetPositionInSamples());
			ForegroundVoice()->Retrig();
		}
	}
	if (EffectFlags() & EffectFlag::CHANNELVOLSLIDE)
	{
		ChannelVolumeSlide();
	}
	if (EffectFlags() & EffectFlag::PANSLIDE)
	{
		PanningSlide();
	}
	if (EffectFlags() & EffectFlag::NOTEDELAY)
	{
		if (m_pSampler->CurrentTick() == m_NoteCutTick)
		{
			for (std::vector<PatternEntry>::iterator ite = m_DelayedNote.begin(); ite != m_DelayedNote.end(); ++ite) {
				m_pSampler->Tick(m_Index, &(*ite));
			}
			m_DelayedNote.clear();
		}
	}
	if (EffectFlags() & EffectFlag::GLOBALVOLSLIDE)
	{
		m_pSampler->SlideVolume(m_GlobalVolSlideSpeed);
	}
	*/
}

void psy_audio_xmsamplerchannel_globalvolslide(psy_audio_XMSamplerChannel* self,
	int speed)
{
	if (speed == 0) {
		if (self->m_GlobalVolSlideMem == 0) return;
		speed = self->m_GlobalVolSlideMem;
	} else self->m_GlobalVolSlideMem = speed;

	if (ISSLIDEUP(speed)) { // Slide up
		speed = GETSLIDEUPVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_GLOBALVOLSLIDE;
		self->m_GlobalVolSlideSpeed = speed;
		if (speed == 0xF) {
			//self->m_pSampler->SlideVolume(m_GlobalVolSlideSpeed);
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide down
		speed = GETSLIDEDOWNVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_GLOBALVOLSLIDE;
		self->m_GlobalVolSlideSpeed = -speed;
		if (speed == 0xF) {
			// m_pSampler->SlideVolume(m_GlobalVolSlideSpeed);
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide up
		self->m_GlobalVolSlideSpeed = GETSLIDEUPVAL(speed);
		// self->m_pSampler->SlideVolume(m_GlobalVolSlideSpeed);
	} else if ((speed & 0xF0) == 0xF0) { // FineSlide down
		self->m_GlobalVolSlideSpeed = -GETSLIDEDOWNVAL(speed);
		// self->m_pSampler->SlideVolume(m_GlobalVolSlideSpeed);
	}
}

void psy_audio_xmsamplerchannel_panningslidespeed(psy_audio_XMSamplerChannel* self,
	int speed)
{
	if (speed == 0) {
		if (self->m_PanSlideMem == 0) return;
		speed = self->m_PanSlideMem;
	} else self->m_PanSlideMem = speed;

	if (ISSLIDEUP(speed)) { // Slide Left
		speed = GETSLIDEUPVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_PANSLIDE;
		self->m_PanSlideSpeed = -speed / 64.0f;
		if (speed == 0xF) {
		//	PanningSlide();
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide Right
		speed = GETSLIDEDOWNVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_PANSLIDE;
		self->m_PanSlideSpeed = speed / 64.0f;
		if (speed == 0xF) {
		//	PanningSlide();
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide left
		self->m_PanSlideSpeed = -(GETSLIDEUPVAL(speed)) / 64.0f;
		// PanningSlide();
	} else if (ISFINESLIDEDOWN(speed)) { // FineSlide right
		self->m_PanSlideSpeed = GETSLIDEDOWNVAL(speed) / 64.0f;
		// PanningSlide();
	}
}

void psy_audio_xmsamplerchannel_pitchslide(psy_audio_XMSamplerChannel* self,
	bool bUp, int speed, int note)
{
	if (speed == 0) {
		if (self->m_PitchSlideMem == 0) return;
		speed = self->m_PitchSlideMem;
	} else self->m_PitchSlideMem = speed & 0xff;
	if (speed < 0xE0 || note != psy_audio_NOTECOMMANDS_EMPTY)	// Portamento , Fine porta ("f0", and Extra fine porta "e0" ) (*)
	{									// Porta to note does not have Fine.
		speed <<= 2;
		//if (psy_audio_xmsamplerchannel_foregroundvoice(self)) { psy_audio_xmsamplerchannel_foregroundvoice(self)->m_PitchSlideSpeed = bUp ? -speed : speed; }
		if (note != psy_audio_NOTECOMMANDS_EMPTY)
		{
			if (note != psy_audio_NOTECOMMANDS_RELEASE) {
// 				if (psy_audio_xmsamplerchannel_foregroundvoice(self)) { psy_audio_xmsamplerchannel_foregroundvoice(self)->m_Slide2NoteDestPeriod = psy_audio_xmsamplerchannel_foregroundvoice(self)->NoteToPeriod(note); }
				self->m_EffectFlags |= XM_SAMPLER_EFFECT_SLIDE2NOTE;
			}
		} else self->m_EffectFlags |= XM_SAMPLER_EFFECT_PITCHSLIDE;
	} else if (speed < 0xF0) {
		speed = speed & 0xf;
		if (psy_audio_xmsamplerchannel_foregroundvoice(self))
		{
			psy_audio_xmsamplerchannel_foregroundvoice(self)->m_PitchSlideSpeed = bUp ? -speed : speed;
			// psy_audio_xmsamplerchannel_foregroundvoice(self)->PitchSlide();
		}
	} else {
		speed = (speed & 0xf) << 2;
		if (psy_audio_xmsamplerchannel_foregroundvoice(self))
		{
			psy_audio_xmsamplerchannel_foregroundvoice(self)->m_PitchSlideSpeed = bUp ? -speed : speed;
			//psy_audio_xmsamplerchannel_foregroundvoice(self)->PitchSlide();
		}
	}
}

void psy_audio_xmsamplerchannel_volumeslide(psy_audio_XMSamplerChannel* self,
	int speed)
{
	if (speed == 0) {
		if (self->m_VolumeSlideMem == 0) return;
		speed = self->m_VolumeSlideMem;
	} else self->m_VolumeSlideMem = speed;

	if (ISSLIDEUP(speed)) { // Slide Up
		speed = GETSLIDEUPVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_VOLUMESLIDE;
		if (psy_audio_xmsamplerchannel_foregroundvoice(self))
		{
			psy_audio_xmsamplerchannel_foregroundvoice(self)->m_VolumeSlideSpeed = speed << 1;
			if (speed == 0xF) {
				// psy_audio_xmsamplervoice_volumeslide(
					//	psy_audio_xmsamplerchannel_foregroundvoice(self));
			}
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide Down
		speed = GETSLIDEDOWNVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_VOLUMESLIDE;
		if (psy_audio_xmsamplerchannel_foregroundvoice(self))
		{
			psy_audio_xmsamplerchannel_foregroundvoice(self)->m_VolumeSlideSpeed = -(speed << 1);
				if (speed == 0xF) {
					// psy_audio_xmsamplervoice_volumeslide(
					//	psy_audio_xmsamplerchannel_foregroundvoice(self));
				}
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide Up
		if (psy_audio_xmsamplerchannel_foregroundvoice(self))
		{
			psy_audio_xmsamplerchannel_foregroundvoice(self)->m_VolumeSlideSpeed = GETSLIDEUPVAL(speed) << 1;
			// psy_audio_xmsamplervoice_volumeslide(
					//	psy_audio_xmsamplerchannel_foregroundvoice(self));
		}
	} else if (ISFINESLIDEDOWN(speed)) { // FineSlide Down
		if (psy_audio_xmsamplerchannel_foregroundvoice(self))
		{
			psy_audio_xmsamplerchannel_foregroundvoice(self)->m_VolumeSlideSpeed = -(GETSLIDEDOWNVAL(speed) << 1);
			// psy_audio_xmsamplervoice_volumeslide(
					//	psy_audio_xmsamplerchannel_foregroundvoice(self));
		}
	}
}

void psy_audio_xmsamplerchannel_tremorparam(psy_audio_XMSamplerChannel* self,
	int parameter)
{
	if (parameter == 0) {
		if (self->m_TremorMem == 0) return;
		parameter = self->m_TremorMem;
	} else self->m_TremorMem = parameter;
	if (psy_audio_xmsamplerchannel_foregroundvoice(self))
	{
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_TremorOnTicks = ((parameter >> 4) & 0xF) + 1;
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_TremorOffTicks = (parameter & 0xF) + 1;
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_TremorTickChange = psy_audio_xmsamplerchannel_foregroundvoice(self)->m_TremorOnTicks;
	}
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_TREMOR;
}
void psy_audio_xmsamplerchannel_vibrato(psy_audio_XMSamplerChannel* self,
	int speed, int depth)
{
	if (depth == 0) {
		if (self->m_VibratoDepthMem == 0) return;
		depth = self->m_VibratoDepthMem;
	} else self->m_VibratoDepthMem = depth;

	if (speed == 0) {
		if (self->m_VibratoSpeedMem == 0) return;
		speed = self->m_VibratoSpeedMem;
	} else self->m_VibratoSpeedMem = speed;
	if (psy_audio_xmsamplerchannel_foregroundvoice(self))
	{
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_VibratoSpeed = speed << 2;
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_VibratoDepth = depth;
	}
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_VIBRATO;

}// XMSampler::Voice::Vibrato(const int depth,const int speed) ------------------------

void psy_audio_xmsamplerchannel_tremolo(psy_audio_XMSamplerChannel* self,
	int speed, int depth)
{
	if (depth == 0) {
		if (self->m_TremoloDepthMem == 0) return;
		depth = self->m_TremoloDepthMem;
	} else self->m_TremoloDepthMem = depth;

	if (speed == 0) {
		if (self->m_TremoloSpeedMem == 0) return;
		speed = self->m_TremoloSpeedMem;
	} else self->m_TremoloSpeedMem = speed;

	if (psy_audio_xmsamplerchannel_foregroundvoice(self))
	{
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_TremoloSpeed = speed << 2;
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_TremoloDepth = depth;
	}
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_TREMOLO;
}

void psy_audio_xmsamplerchannel_panbrello(psy_audio_XMSamplerChannel* self,
	int speed, int depth)
{
	if (depth == 0) {
		if (self->m_PanbrelloDepthMem == 0) return;
		depth = self->m_PanbrelloDepthMem;
	} else self->m_PanbrelloDepthMem = depth;

	if (speed == 0) {
		if (self->m_PanbrelloSpeedMem == 0) return;
		speed = self->m_PanbrelloSpeedMem;
	} else self->m_PanbrelloSpeedMem = speed;

	if (psy_audio_xmsamplerchannel_foregroundvoice(self))
	{
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_PanbrelloSpeed = speed << 2;
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_PanbrelloDepth = depth;
	}
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_PANBRELLO;
}

void psy_audio_xmsamplerchannel_arpeggio(psy_audio_XMSamplerChannel* self,
	int param)
{
	if (param != 0)
	{
		self->m_ArpeggioMem = param;
	} else param = self->m_ArpeggioMem;
	if (psy_audio_xmsamplerchannel_foregroundvoice(self))
	{
		// self->m_ArpeggioPeriod[0] = ForegroundVoice()->NoteToPeriod(Note() + ((param & 0xf0) >> 4));
		// self->m_ArpeggioPeriod[1] = ForegroundVoice()->NoteToPeriod(Note() + (param & 0xf));
	}
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_ARPEGGIO;
}

void psy_audio_xmsamplerchannel_retrigger(psy_audio_XMSamplerChannel* self,
	int parameter)
{
	int ticks, volumeModifier;
	int effretVol, effretMode;

	if (parameter == 0)
	{
		ticks = (self->m_RetrigMem & 0x0F);
		volumeModifier = (self->m_RetrigMem >> 4) & 0x0F;
	} else {
		ticks = (parameter & 0x0F);
		volumeModifier = (parameter >> 4) & 0x0F;
		self->m_RetrigMem = parameter;
	}
	switch (volumeModifier)
	{
	case 1://fallthrouhg
	case 2://fallthrouhg
	case 3://fallthrouhg
	case 4://fallthrouhg
	case 5: effretVol = -(int)pow(2., volumeModifier - 1); effretMode = 1; break;
	case 6: effretVol = 0.66666666f;	 effretMode = 2; break;
	case 7: effretVol = 0.5f;			 effretMode = 2; break;

	case 9://fallthrouhg
	case 10://fallthrouhg
	case 11://fallthrouhg
	case 12://fallthrouhg
	case 13: effretVol = (int)pow(2., volumeModifier - 9); effretMode = 1; break;
	case 14: effretVol = 1.5f; effretMode = 2; break;
	case 15: effretVol = 2.0f; effretMode = 2; break;

	case 0://fallthrouhg
	case 8:	//fallthrouhg
	default: effretVol = 0; effretMode = 0; break;
	}
	if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_RetrigTicks = ticks != 0 ? ticks : 1;
	}
	self->m_RetrigVol = effretVol;
	self->m_RetrigOperation = effretMode;
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_RETRIG;
}

void psy_audio_xmsamplerchannel_notecuttick(psy_audio_XMSamplerChannel* self,
	int ntick)
{
	self->m_NoteCutTick = ntick;
	if (ntick == 0) {
		if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
			psy_audio_xmsamplervoice_setvolume(
				psy_audio_xmsamplerchannel_foregroundvoice(self), 0);
		}		
		return;
	}
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_NOTECUT;
}

void psy_audio_xmsamplerchannel_delayednote(psy_audio_XMSamplerChannel* self,
	psy_audio_PatternEvent data)
{
	psy_audio_PatternEvent* ev;

	if (data.cmd == XM_SAMPLER_CMD_EXTENDED && (data.parameter & 0xf0) == XM_SAMPLER_CMD_E_NOTE_DELAY) {
		self->m_NoteCutTick = data.parameter & 0x0f;
		data.cmd = XM_SAMPLER_CMD_NONE;
		data.parameter = 0;
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_NOTEDELAY;
	}
	ev = psy_audio_patternevent_clone(&data);
	psy_list_append(&self->m_DelayedNote, ev);
}

void psy_audio_xmsamplerchannel_panningslide(psy_audio_XMSamplerChannel* self)
{
	self->m_PanFactor += self->m_PanSlideSpeed;
	if (self->m_PanFactor < 0.0f) {
		self->m_PanFactor = 0.0f;
	} else if (self->m_PanFactor > 1.0f) {
		self->m_PanFactor = 1.0f;
	}
	if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
		psy_audio_xmsamplervoice_setpanfactor(
			psy_audio_xmsamplerchannel_foregroundvoice(self), self->m_PanFactor);
	}	
}

void psy_audio_xmsamplerchannel_channelvolumeslide(psy_audio_XMSamplerChannel* self)
{
	self->m_Volume += self->m_ChanVolSlideSpeed;
	if (self->m_Volume < 0.0f) {
		self->m_Volume = 0.0f;
	} else if (self->m_Volume > 1.0f) {
		self->m_Volume = 1.0f;
	}
}

void psy_audio_xmsamplerchannel_notecut(psy_audio_XMSamplerChannel* self)
{
	if (psy_audio_xmsampler_currenttick(self->m_pSampler) == self->m_NoteCutTick)
	{
		if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
			psy_audio_xmsamplervoice_setvolume(
				psy_audio_xmsamplerchannel_foregroundvoice(self), 0);
		}
		self->m_EffectFlags &= ~XM_SAMPLER_EFFECT_NOTECUT;
	}
}

void psy_audio_xmsamplerchannel_stopbackgroundnotes(psy_audio_XMSamplerChannel* self,
	psy_audio_NewNoteAction action)
{
	int current;

	for (current = 0; current < psy_audio_xmsampler_numvoices(self->m_pSampler); current++)
	{
		if (psy_audio_xmsamplervoice_channelnum(psy_audio_xmsampler_rvoice(self->m_pSampler, current)) == self->m_Index &&
			psy_audio_xmsamplervoice_isplaying(psy_audio_xmsampler_rvoice(self->m_pSampler, current)))
		{
			switch (action)
			{
			case psy_audio_NNA_NOTEOFF:
				psy_audio_xmsamplervoice_noteoff(
					psy_audio_xmsampler_rvoice(self->m_pSampler, current));				
				break;
			case psy_audio_NNA_FADEOUT:
				psy_audio_xmsamplervoice_notefadeout(
					psy_audio_xmsampler_rvoice(self->m_pSampler, current));				
				break;
			case psy_audio_NNA_STOP:
				psy_audio_xmsamplervoice_noteofffast(
					psy_audio_xmsampler_rvoice(self->m_pSampler, current));				
				break;
			}
		}
	}
}

double psy_audio_xmsamplerchannel_arpeggioperiod(const psy_audio_XMSamplerChannel* self)
{
	int arpi = psy_audio_xmsampler_currenttick(self->m_pSampler) % 3;
	if (arpi >= 1) {
		return self->m_ArpeggioPeriod[arpi - 1];
	} else {
		return self->m_Period;
	}
}

bool  psy_audio_xmsamplerchannel_load(psy_audio_XMSamplerChannel* self,
	psy_audio_SongFile* songfile)
{
	char chan[8];
	int32_t size = 0;
	int32_t temp32;

	psyfile_read(songfile->file, chan, 4);
	chan[4] = '\0';
	psyfile_read(songfile->file, &size, sizeof(int32_t));
	if (strcmp(chan, "CHAN")) {
		return FALSE;
	}
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	///< (0..200)   &0x100 = Mute.
	self->m_ChannelDefVolume = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	//<  0..200 .  &0x100 = Surround.
	self->m_DefaultPanFactor = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	self->m_DefaultCutoff = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	self->m_DefaultRessonance = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	self->m_DefaultFilterType = (psy_dsp_FilterType)temp32;
	return TRUE;
}

void psy_audio_xmsamplerchannel_save(psy_audio_XMSamplerChannel* self,
	psy_audio_SongFile* songfile)
{
	int size;
	
	size = 5 * sizeof(int);
	psyfile_write(songfile->file, "CHAN", 4);
	psyfile_write_int32(songfile->file, size);
	psyfile_write_int32(songfile->file, self->m_ChannelDefVolume);
	psyfile_write_int32(songfile->file, self->m_DefaultPanFactor);
	psyfile_write_int32(songfile->file, self->m_DefaultCutoff);
	psyfile_write_int32(songfile->file, self->m_DefaultRessonance);		
	psyfile_write_uint32(songfile->file, (uint32_t)self->m_DefaultFilterType);
}

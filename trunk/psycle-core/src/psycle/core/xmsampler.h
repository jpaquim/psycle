// -*- mode:c++; indent-tabs-mode:t -*-
/***************************************************************************
	*   Copyright (C) 2007 Psycledelics   *
	*   psycle.sf.net   *
	*                                                                         *
	*   This program is free software; you can redistribute it and/or modify  *
	*   it under the terms of the GNU General Public License as published by  *
	*   the Free Software Foundation; either version 2 of the License, or     *
	*   (at your option) any later version.                                   *
	*                                                                         *
	*   This program is distributed in the hope that it will be useful,       *
	*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
	*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
	*   GNU General Public License for more details.                          *
	*                                                                         *
	*   You should have received a copy of the GNU General Public License     *
	*   along with this program; if not, write to the                         *
	*   Free Software Foundation, Inc.,                                       *
	*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
	***************************************************************************/
#ifndef XMSAMPLER_H
#define XMSAMPLER_H

#include "dsp.h"
#include "filter.h"
#include "xminstrument.h"
#include "machine.h"
#include <mutex>
/**
@author  Psycledelics
*/
namespace psy { namespace core {
class XMSampler : public Machine
{
public:

	static const int MAX_POLYPHONY = 64;///< max polyphony
	static const int MAX_INSTRUMENT = 255;///< max instrument
	static const std::uint32_t VERSION = 0x00010000;
	static const float SURROUND_THRESHOLD;

/*
* = remembers its last value when called with param 00.
t = slides/changes each tick. (or is applied in a specific tick != 0 )
p = persistent ( a new note doesn't reset it )
n = they need to appear next to a note.

Commands are Implemented in XMSampler::Channel::SetEffect() , and type "t" commands also need to be added to
XMSampler::Channel::PerformFX().
"n" type commands are implemented (partially or fully) in XMSampler::Tick(channel,event) (like sample offset)

*/
	struct CMD {
		enum Type {
		NONE				=	0x00,
		PORTAMENTO_UP		=	0x01,// Portamento Up , Fine porta (0x01fx, and Extra fine porta 01ex )	(*t)
		PORTAMENTO_DOWN		=	0x02,// Portamento Down, Fine porta (0x02fx, and Extra fine porta 02ex ) (*t)
		PORTA2NOTE			=	0x03,// Tone Portamento						(*tn)
		VIBRATO				=	0x04,// Do Vibrato							(*t)
		TONEPORTAVOL		=	0x05,// Tone Portament & Volume Slide		(*t)
		VIBRATOVOL			=	0x06,// Vibrato & Volume Slide				(*t)
		TREMOLO				=	0x07,// Tremolo								(*t)
		PANNING				=	0x08,// Set Panning Position				(p)
		PANNINGSLIDE		=	0x09,// PANNING SLIDE						(*t)
		SET_CHANNEL_VOLUME	=	0x0A,// Set channel's volume				(p)
		CHANNEL_VOLUME_SLIDE=	0x0B,// channel Volume Slide up (0dx0) down (0d0x), File slide up(0dxf) down(0dfx)	 (*tp)
		VOLUME				=	0x0C,// Set Volume
		VOLUMESLIDE			=	0x0D,// Volume Slide up (0dx0) down (0d0x), File slide up(0dxf) down(0dfx)	 (*t)
		EXTENDED			=	0x0E,// Extend Command
		MIDI_MACRO			=	0x0F,// Impulse Tracker MIDI macro			(p)
		ARPEGGIO			=	0x10,//	Arpeggio							(*t)
		RETRIG				=	0x11,// Retrigger Note						(*t)
		SET_GLOBAL_VOLUME	=	0x12,// Sets Global Volume
		GLOBAL_VOLUME_SLIDE =	0x13,// Slides Global Volume				(*t)
		FINE_VIBRATO		=	0x14,// Vibrato 4 times finer				(*t)
		SET_ENV_POSITION	=	0x15,// Set Envelope Position
							//0x16
		TREMOR				=	0x17,// Tremor								(*t)
		PANBRELLO			=	0x18,// Panbrello							(*t)
		OFFSET				=	0x90 // Set Sample Offset  , note!: 0x9yyy ! not 0x90yy (*n)
		};
	};
	struct CMD_E
	{
		enum Type {
		E_GLISSANDO_TYPE	=	0x30, //E3     Set gliss control			(p)
		E_VIBRATO_WAVE		=	0x40, //E4     Set vibrato control			(p)
								//0x50
		E_PANBRELLO_WAVE	=	0x60, //									(p)
		E_TREMOLO_WAVE		=	0x70, //E7     Set tremolo control			(p)
		E_SET_PAN			=	0x80,									//	(p)
		E9					=	0x90,
								//0xA0,
								//0xB0,
		E_DELAYED_NOTECUT	=	0xC0, //EC     Note cut						(t)
		E_NOTE_DELAY		=	0xD0, //ED     Note delay					(tn)
		EE					=	0xE0,
		E_SET_MIDI_MACRO	=	0xF0//										(p)
		};
	};
	struct CMD_E9
	{
		enum Type {
		E9_SURROUND_OFF		=	0x00,//										(p)
		E9_SURROUND_ON		=	0x01,//										(p)
		E9_REVERB_OFF		=	0x08,//										(p)
		E9_REVERB_FORCE		=	0x09,//										(p)
		E9_STANDARD_SURROUND=	0x0A,//										(p)
		E9_QUAD_SURROUND	=	0x0B,// (p)Select quad surround mode: this allows you to pan in the rear channels, especially useful for 4-speakers playback. Note that S9A and S9B do not activate the surround for the current channel, it is a global setting that will affect the behavior of the surround for all channels. You can enable or disable the surround for individual channels by using the S90 and S91 effects. In quad surround mode, the channel surround will stay active until explicitely disabled by a S90 effect
		E9_GLOBAL_FILTER	=	0x0C,// (p)Select global filter mode (IT compatibility). This is the default, when resonant filters are enabled with a Zxx effect, they will stay active until explicitely disabled by setting the cutoff frequency to the maximum (Z7F), and the resonance to the minimum (Z80).
		E9_LOCAL_FILTER		=	0x0D,// (p)Select local filter mode (MPT beta compatibility): when this mode is selected, the resonant filter will only affect the current note. It will be deactivated when a new note is being played.
		E9_PLAY_FORWARD		=	0x0E,// Play forward. You may use this to temporarily force the direction of a bidirectional loop to go forward.
		E9_PLAY_BACKWARD	=	0x0F // Play backward. The current instrument will be played backwards, or it will temporarily set the direction of a loop to go backward.
		};
	};
	struct CMD_EE
	{
		enum Type {
		EE_BACKGROUNDNOTECUT	=	0x00,
		EE_BACKGROUNDNOTEOFF	=	0x01,
		EE_BACKGROUNDNOTEFADE	=	0x02,
		EE_SETNOTECUT			=	0x03,
		EE_SETNOTECONTINUE		=	0x04,
		EE_SETNOTEOFF			=	0x05,
		EE_SETNOTEFADE			=	0x06,
		EE_VOLENVOFF			=	0x07,
		EE_VOLENVON				=	0x08,
		EE_PANENVOFF			=	0x09,
		EE_PANENVON				=	0x0A,
		EE_PITCHENVON			=	0x0B,
		EE_PITCHENVOFF			=	0x0C
		};
	};

	struct CMD_VOL
	{
		enum Type {
		VOL_VOLUME0			=	0x00, // 0x00..0x0F (63)  ||
		VOL_VOLUME1			=	0x10, // 0x10..0x1F (63)  || All are the same command.
		VOL_VOLUME2			=	0x20, // 0x20..0x2F (63)  ||
		VOL_VOLUME3			=	0x30, // 0x30..0x3F (63)  ||
		VOL_VOLSLIDEUP		=	0x40, // 0x40..0x4F (16)
		VOL_VOLSLIDEDOWN	=	0x50, // 0x50..0x5F (16)
		VOL_FINEVOLSLIDEUP	=	0x60, // 0x60..0x6F (16)
		VOL_FINEVOLSLIDEDOWN=	0x70, // 0x70..0x7F (16)
		VOL_PANNING			=	0x80, // 0x80..0x8F (16)
		VOL_PANSLIDELEFT	=	0x90, // 0x90..0x9F (16)
		VOL_PANSLIDERIGHT	=	0xA0, // 0xA0..0xAF (16)
		VOL_VIBRATO			=	0xB0, // 0xB0..0xBF (16) Linked to Vibrato Vy = 4xy
		VOL_TONEPORTAMENTO	=	0xC0, // 0xC0..0xCF (16) Linked to Porta2Note
		VOL_PITCH_SLIDE_UP	=	0xD0, // 0xD0..0xDF (16)
		VOL_PITCH_SLIDE_DOWN=	0xE0, // 0xE0..0xEF (16)
		// 0xFF -> Blank.
		};
	};

	struct ZxxMacro
	{
		int mode;
		int value;
	};

	class Channel;

//////////////////////////////////////////////////////////////////////////
//  XMSampler::WaveDataController Declaration
	//\todo: WaveDateController Needs to update the speed if sampleRate changes (but... would the samplerate change while
	//       there's a voice playing?)
	class WaveDataController
	{
	public:
		struct LoopDirection {
			enum Type {
				FORWARD = 0,
				BACKWARD
			};
		};

		virtual void Init(XMInstrument::WaveData* wave, const int layer);
		virtual void NoteOff(void);
		virtual void Work(float *pLeftw,float *pRightw, dsp::PRESAMPLERFN pResamplerWork)
		{
			//Process sample
			*pLeftw = pResamplerWork(
				pLeft() + (m_Position >> 32),
				m_Position >> 32, m_Position & 0xFFFFFFFF, Length());
			if (IsStereo())
			{
				*pRightw = pResamplerWork(
					pRight() + (m_Position >> 32),
					m_Position >> 32, m_Position & 0xFFFFFFFF, Length());
			}

			// Update Position
			if(CurrentLoopDirection() == LoopDirection::FORWARD)
			{
				m_Position+=Speed();
				const int curIntPos = m_Position >> 32;
				switch(m_CurrentLoopType)
				{
				case XMInstrument::WaveData::LoopType::NORMAL:

					if(curIntPos >= m_CurrentLoopEnd)
					{
						Position(m_CurrentLoopStart+(curIntPos-m_CurrentLoopEnd));
					}
					break;
				case XMInstrument::WaveData::LoopType::BIDI:

					if(curIntPos  >= m_CurrentLoopEnd)
					{
						Position(m_CurrentLoopEnd-(curIntPos-m_CurrentLoopEnd));
						CurrentLoopDirection(LoopDirection::BACKWARD);
					}
					break;
				case XMInstrument::WaveData::LoopType::DO_NOT:

					if (curIntPos >= Length()-1)
					{
						Playing(false);
					}
				default:
					break;
				}
			} else {
				m_Position-=Speed();
				const int curIntPos = m_Position >>32;
				switch(m_CurrentLoopType)
				{
				case XMInstrument::WaveData::LoopType::NORMAL:
				case XMInstrument::WaveData::LoopType::BIDI:

					if(curIntPos <= m_CurrentLoopStart)
					{
						Position(m_CurrentLoopStart+(m_CurrentLoopStart-curIntPos));
						CurrentLoopDirection(LoopDirection::FORWARD);
					}
				break;
				case XMInstrument::WaveData::LoopType::DO_NOT:

						if (curIntPos <= 0)
					{
						Playing(false);
					}
				default:
					break;
				}
			}
		}

		// Properties
		virtual const int Layer() { return m_Layer;}
		virtual XMInstrument::WaveData &Wave() { return *m_pWave; }

		virtual bool Playing(){ return m_Playing;}
		virtual void Playing(bool play){ m_Playing=play; }

		// Current sample position
		virtual const std::uint32_t  Position(){ return m_Position >>32;}
		virtual void Position(const	std::uint32_t value){
			if ( value < Length()) m_Position = value << 32;
			else m_Position = (Length()-1)<<32;
		}

		// Current sample Speed
		virtual const std::uint64_t Speed(){return m_Speed;}
		virtual void Speed(const double value){m_Speed = value * 4294967296.0f;} // 4294967296 is a left shift of 32bits

		virtual void CurrentLoopDirection(const int dir){m_LoopDirection = dir;}
		virtual const int CurrentLoopDirection(){return m_LoopDirection;}

		virtual const int LoopType(){return m_pWave->WaveLoopType();}
		virtual const int LoopStart(){return m_pWave->WaveLoopStart();}
		virtual const int LoopEnd(){ return m_pWave->WaveLoopEnd();}

		virtual const int SustainLoopType(){return m_pWave->WaveSusLoopType();}
		virtual const int SustainLoopStart(){return m_pWave->WaveSusLoopStart();}
		virtual const int SustainLoopEnd(){ return m_pWave->WaveSusLoopEnd();}

		virtual const int Length(){return m_pWave->WaveLength();}

		virtual const bool IsStereo(){ return m_pWave->IsWaveStereo();}

		// pointer to Start of Left sample
		virtual const short* pLeft(){return m_pL;}
		// pointer to Start of Right sample
		virtual const short* pRight(){return m_pR;}


	protected:
		int m_Layer;
		XMInstrument::WaveData *m_pWave;
		std::uint64_t m_Position;
		double m_Speed;
		bool m_Playing;

		int m_CurrentLoopType;
		int m_CurrentLoopEnd;
		int m_CurrentLoopStart;

		bool _stereo;
		int _length;

		int m_LoopDirection;
		short* m_pL;
		short* m_pR;
	};
/*
	class XDSPWaveController : public WaveDataController
	{
	public:
		XDSPWaveController(){};
		virtual ~XDSPWaveController();
		void Init(XMInstrument::WaveData* wave, const int layer);
		static void Workxdsp(int numSamples);
		void Speed(const double value);
	protected:
		void XMSampler::XDSPWaveController::RecreateResampler(void);
	private:
		int m_Speed1x;
	};
*/
//////////////////////////////////////////////////////////////////////////
//  XMSampler::EnvelopeController Declaration
	//\todo: Recall "CalcStep" after a SampleRate change, and also after a Tempo Change.
	class EnvelopeController {
	public:
		struct EnvelopeStage {
			enum Type {
				OFF		= 0,
				DOSTEP	= 1, // normal operation, follow the steps.
				HASLOOP	= 2, // Indicates that the envelope *has* a (normal) loop (not that it is playing it)
				HASSUSTAIN	= 4, // This indicates that the envelope *has* a sustain (not that it is playing it)
				RELEASE = 8  // Indicates that a Note-Off has been issued.
			};
		};
		// EnvelopeMode defines what the first value of a PointValue means
		// TICK = one tracker tick ( speed depends on the BPM )
		// MILIS = a millisecond. (independant of BPM).
/*		struct EnvelopeMode {
			enum Type {
				TICK=0,
				MILIS
			};
		};*/
		EnvelopeController(){};
		~EnvelopeController(){};

		void Init(XMInstrument::Envelope *pEnvelope = NULL, MachineCallbacks * callbacks = NULL);

//		const EnvelopeMode Mode() { return m_Mode; }
//		const Mode(EnvelopeMode _mode){ m_Mode=_mode; }

		void NoteOn();
		void NoteOff();

		void Work()
		{
			if(m_Stage&EnvelopeStage::DOSTEP)
			{
				if(++m_Samples >= m_NextEventSample) // m_NextEventSample is updated inside CalcStep()
				{
					m_PositionIndex++;
					if (m_Stage&EnvelopeStage::HASSUSTAIN && !(m_Stage&EnvelopeStage::RELEASE))
					{
						if (m_PositionIndex == m_pEnvelope->SustainEnd())
						{
							// if the begin==end, pause the envelope.
							if ( m_pEnvelope->SustainBegin() == m_pEnvelope->SustainEnd() )
									{
										m_Stage = EnvelopeStage::Type(m_Stage & ~EnvelopeStage::DOSTEP);
									}
							else { m_PositionIndex = m_pEnvelope->SustainBegin(); }
								}
							}
					else if (m_Stage&EnvelopeStage::HASLOOP)
							{
						if ( m_PositionIndex >= m_pEnvelope->LoopEnd())
						{
							if ( m_pEnvelope->LoopStart() == m_pEnvelope->LoopEnd() )
							{
								m_Stage = EnvelopeStage::Type(m_Stage & ~EnvelopeStage::DOSTEP);
							}
							else { m_PositionIndex = m_pEnvelope->LoopStart(); }
						}
					}
					if (m_Stage & EnvelopeStage::DOSTEP)
					{
						if( m_pEnvelope->GetTime(m_PositionIndex+1) == XMInstrument::Envelope::INVALID )
					{
						m_Stage = EnvelopeStage::OFF;
							//This ensures that the envelope is really inplace.
						m_PositionIndex = m_pEnvelope->NumOfPoints() - 1;
							CalcStep(m_PositionIndex,m_PositionIndex);
					}
						else CalcStep(m_PositionIndex,m_PositionIndex + 1);
					}
					else CalcStep(m_PositionIndex,m_PositionIndex);
				}
				else
				{
					m_ModulationAmount += m_Step;
				}
			}
		}

		///
		const XMInstrument::Envelope::ValueType ModulationAmount()
		{
			return m_ModulationAmount;
		}

		const EnvelopeStage::Type Stage(){return m_Stage;}
		void Stage(const EnvelopeStage::Type value){m_Stage = value;}
		XMInstrument::Envelope & Envelope(){return *m_pEnvelope;}
		inline void CalcStep(const int start,const int  end);
		void SetPosition(const int posi) { m_PositionIndex=posi-1; m_Stage= EnvelopeStage::Type(m_Stage|EnvelopeStage::DOSTEP); m_Samples= m_NextEventSample-1; } // m_Samples=m_NextEventSample-1 only forces a recalc when entering Work().
		int GetPosition(void) { return m_PositionIndex; }
		void SetPositionInSamples(const int samplePos);
		int GetPositionInSamples();
		void RecalcDeviation();
	private:
		inline float SRateDeviation() { return m_sRateDeviation; }

		int m_Samples;
		float m_sRateDeviation;
		int m_Mode;
		int m_PositionIndex;
		int m_NextEventSample;
		MachineCallbacks * m_pCallbacks;
		EnvelopeStage::Type m_Stage;

		XMInstrument::Envelope * m_pEnvelope;

		XMInstrument::Envelope::ValueType m_ModulationAmount;
		XMInstrument::Envelope::ValueType m_Step;
	};// EnvelopeController



/////////////////////////////////////////////////////////////////////////////////////
//  XMSampler::Voice Declaration
//	(This class could be called XMInstrumentController too, but "Voice" describes better how it is used.)

	class Voice
	{
	public:
		///
		friend class XMSampler::Channel;

		Voice(){
			Reset();
		}

		// Object Functions
		void Reset();
		void ResetEffects();

		void VoiceInit(int channelNum,int instrumentNum);
		void Work(int numSamples,float * pSampleL,float *pSamlpesR, dsp::Cubic& _resampler);

		// This one is Tracker Tick (Mod-tick)
		void Tick();
		// This one is Psycle's "Tick"
		void NewLine();

		void NoteOn(const std::uint8_t note,const std::int16_t playvol=-1,bool reset=true);
		void NoteOff();
		void NoteOffFast();
		void NoteFadeout();
		void UpdateFadeout();
		const XMInstrument::NewNoteAction::Type NNA() { return m_NNA;}
		void NNA(const XMInstrument::NewNoteAction::Type value){ m_NNA = value;}

		void ResetVolAndPan(std::int16_t playvol,bool reset=true);
		void UpdateSpeed();
		double PeriodToSpeed(int period);


		// Effect-Related Object Functions
		void PitchSlide()
		{
			m_Period += m_PitchSlideSpeed;
			UpdateSpeed();
		}
		void Slide2Note();
		void Vibrato();
		void Tremolo();
		void Panbrello();
		void Tremor();
		void VolumeSlide();
		void VolumeDown(const int value);
		void VolumeUp(const int value);
		void Retrig();

		// Do Auto Vibrato
		void AutoVibrato();
		bool IsAutoVibrato() { return m_AutoVibratoAmount!=0; }
		// Get Auto Vibrato Amount
		const double AutoVibratoAmount(){return m_AutoVibratoAmount;}


// Properties
		const int InstrumentNum(){ return _instrument;}
		void InstrumentNum(const int value){_instrument = value;}
		XMInstrument &rInstrument() { return *m_pInstrument;}
		void pInstrument(XMInstrument *p){m_pInstrument = p;}

		const int ChannelNum(){ return m_ChannelNum;}
		void ChannelNum(const int value){ m_ChannelNum = value;}
		void pChannel(XMSampler::Channel *p){m_pChannel = p;};
		XMSampler::Channel& rChannel(){return *m_pChannel;}

		void pSampler(XMSampler * const p){m_pSampler = p;}
		XMSampler * const pSampler(){return m_pSampler;}

		XMSampler::EnvelopeController& AmplitudeEnvelope(){return m_AmplitudeEnvelope;}
		XMSampler::EnvelopeController& FilterEnvelope(){return m_FilterEnvelope;}
		XMSampler::EnvelopeController& PitchEnvelope(){return m_PitchEnvelope;}
		XMSampler::EnvelopeController& PanEnvelope(){return m_PanEnvelope;}

		WaveDataController& rWave(){return m_WaveDataController;}

		const bool IsPlaying(){ return m_bPlay;}
		void IsPlaying(const bool value)
		{
			if ( value == false )
			{
				if ( rChannel().ForegroundVoice() == this)
				{
					rChannel().LastVoicePanFactor(m_PanFactor);
					//FIXME: warning! float to int!? i smell a bug
					rChannel().LastVoiceVolume(m_Volume);
					rChannel().LastAmpEnvelopePosInSamples(0);
					rChannel().LastPanEnvelopePosInSamples(0);
					rChannel().LastFilterEnvelopePosInSamples(0);
					rChannel().LastPitchEnvelopePosInSamples(0);
					rChannel().ForegroundVoice(NULL);
				}
			}
			m_bPlay = value;
		}

		const bool IsBackground() { return m_Background; }
		void IsBackground(const bool background){ m_Background = background; }

		const bool IsStopping() { return m_Stopping; }
		void IsStopping(const bool stop) { m_Stopping = stop; }

		// Volume of the current note.
		//fixme: m_Volume is float! what's going on here?
		const std::uint16_t Volume() { return m_Volume; }
		void Volume(const std::uint16_t vol)
		{
			m_Volume = vol;
			m_RealVolume = rWave().Wave().WaveGlobVolume() * rInstrument().GlobVol() * (vol/128.0f);
				//\todo :  rInstrument().RandomVolume() / 100.0f;
		}
		// Voice.RealVolume() returns the calculated volume out of "WaveData.WaveGlobVol() * Instrument.Volume() * Voice.NoteVolume()"
		const float RealVolume() { return (!m_bTremorMute)?(m_RealVolume+m_TremoloAmount):0; }
		void PanFactor(float pan)
		{
			m_PanFactor = pan;
			m_PanRange = 0.5f -(fabs(0.5-m_PanFactor)*2);
		}
		float PanFactor() { return m_PanFactor; }

		const int CutOff() { return m_CutOff; }
		void CutOff(int co)
		{
/*			m_CutOff = co;	m_Filter._cutoff = co;
			if ( m_Filter._type == dsp::F_NONE) { m_Filter._type =dsp::F_LOWPASS12; }
				m_Filter.Update();
*/
			m_CutOff = co; m_Filter.Cutoff(co);
		}

		const int Ressonance() { return m_Ressonance; }
		void Ressonance(int res)
		{
/*			m_Ressonance = res; m_Filter._q = res;
			if ( m_Filter._type == dsp::F_NONE) { m_Filter._type =dsp::F_LOWPASS12; }
			m_Filter.Update();
*/
			m_Ressonance = res; m_Filter.Ressonance(res);
		}

		void FilterType(dsp::FilterType ftype) { m_Filter.Type(ftype);}

		void Period(int newperiod) { m_Period = newperiod; UpdateSpeed(); }
		int Period() { return m_Period; }
		// convert note to period
		const double NoteToPeriod(const int note);
		// convert period to note
		const int PeriodToNote(const double period);

		double VibratoAmount() { return m_VibratoAmount; }

	protected:
		// Gets the delta between the points of the wavetables for tremolo/panbrello/vibrato
		int GetDelta(int wavetype,int wavepos);
		float PanRange() { return m_PanRange; }
		const bool IsTremorMute(){return m_bTremorMute;}
		void IsTremorMute(const bool value){m_bTremorMute = value;}


	private:

		int m_ChannelNum;
		XMSampler::Channel* m_pChannel;
		XMSampler *m_pSampler;

		int _instrument;// Instrument
		XMInstrument *m_pInstrument;
		XMInstrument::NewNoteAction::Type m_NNA;


		// Todo: do we need 4 controllers? wouldn't it be better 1 controller for all 4 envelopes?
		EnvelopeController m_FilterEnvelope;
		EnvelopeController m_AmplitudeEnvelope;
		EnvelopeController m_PanEnvelope;
		EnvelopeController m_PitchEnvelope;

		WaveDataController m_WaveDataController;
//		XDSPWaveController m_WaveDataController;

		dsp::ITFilter m_Filter;
		int m_CutOff;
		int m_Ressonance;
		float _coModify;

		bool m_bPlay;
		bool m_Background;
		bool m_Stopping;
		int m_Note;
		int m_Period;
		float m_Volume;
		float m_RealVolume;

		float m_PanFactor;
		float m_PanRange;

		int m_Slide2NoteDestPeriod;
		int m_PitchSlideSpeed;

		float m_VolumeFadeSpeed;
		float m_VolumeFadeAmount;

		int m_VolumeSlideSpeed;

		int m_VibratoSpeed;
		int m_VibratoDepth;
		int m_VibratoPos;
		double m_VibratoAmount;

		int m_TremoloSpeed;
		int m_TremoloDepth;
		float m_TremoloAmount;
		int m_TremoloPos;

		// Panbrello
		int m_PanbrelloSpeed;
		int m_PanbrelloDepth;
		float m_PanbrelloAmount;
		int m_PanbrelloPos;
		int m_PanbrelloRandomCounter;

		/// Tremor
		int m_TremorOnTicks;
		int m_TremorOffTicks;
		int m_TremorTickChange;
		bool m_bTremorMute;


		// Auto Vibrato
		double m_AutoVibratoAmount;
		int m_AutoVibratoDepth;
		int m_AutoVibratoPos;

		int m_RetrigTicks;

		static const int m_FineSineData[256];
		static const int m_FineRampDownData[256];
		static const int m_FineSquareTable[256];
		static const int m_RandomTable[256];
	};


	//////////////////////////////////////////////////////////////////////////
	//  XMSampler::Channel Declaration
	class Channel {
	public:
		struct EffectFlag
		{
			static const int VIBRATO		=	0x00000001;
			static const int PITCHSLIDE		=	0x00000002;
			static const int CHANNELVOLSLIDE =	0x00000004;
			static const int SLIDE2NOTE		=	0x00000008;
			static const int VOLUMESLIDE	=	0x00000010;
			static const int PANSLIDE		=	0x00000020;
			static const int TREMOLO		=	0x00000040;
			static const int ARPEGGIO		=	0x00000080;
			static const int NOTECUT		=	0x00000100;
			static const int PANBRELLO		=	0x00000200;
			static const int RETRIG 		=   0x00000400;
			static const int TREMOR			=	0x00000800;
			static const int NOTEDELAY		=	0x00001000;
			static const int GLOBALVOLSLIDE	=	0x00002000;
		};

		Channel()
		{
			m_Index = 0;
			Init();
		}
		bool Load(RiffFile& riffFile);
		void Save(RiffFile& riffFile);
		void Init();
		void EffectInit();
		void Restore();

		// Prepare the channel for the new effect (or execute if it's a one-shot one). This is executed on TrackerTick==0
		void SetEffect(Voice* voice,int volcmd,int cmd,int parameter);

		// Executes the slide/change effects. This is executed on TrackerTick!=0
		void PerformFx();

		const int EffectFlags(){return m_EffectFlags;}
		void EffectFlags(const int value){m_EffectFlags = value;}


// Effect-Related Object Functions

		// Tick 0 commands
		void GlobalVolSlide(int speed);
		void PanningSlide(int speed);
		void ChannelVolumeSlide(int speed);
		void PitchSlide(bool bUp,int speed,int note=notetypes::empty);
		void VolumeSlide(int speed);
		void Tremor(int parameter);
		void Vibrato(int speed,int depth = 0);
		void Tremolo(int speed,int depth);
		void Panbrello(int speed,int depth);
		void Arpeggio(const int param);
		void Retrigger(const int param);
		void NoteCut(const int ntick);
		void DelayedNote(PatternEvent data);

		// Tick n commands.
		void PanningSlide();
		void ChannelVolumeSlide();
		void NoteCut();
		void StopBackgroundNotes(XMInstrument::NewNoteAction::Type action);

		const double ArpeggioPeriod()
		{
			const int arpi = m_pSampler->CurrentTick()%3;
			if(arpi >= 1){
				return m_ArpeggioPeriod[arpi - 1];
			} else {
				return m_Period;
			}
		}


// Properties
		const int Index(){ return m_Index;}
		void Index(const int value){m_Index = value;}

		void pSampler(XMSampler * const pSampler){m_pSampler = pSampler;}

		const int InstrumentNo(){return m_InstrumentNo;}
		void InstrumentNo(const int no){m_InstrumentNo = no;}

		XMSampler::Voice* ForegroundVoice(){ return m_pForegroundVoice; }
		void ForegroundVoice(XMSampler::Voice* pVoice) { m_pForegroundVoice = pVoice; }

		const int Note(){ return m_Note;}
		void Note(const int note)
		{	m_Note = note;
			if (ForegroundVoice()) m_Period = ForegroundVoice()->NoteToPeriod(note);
		}
		const double Period(){return m_Period;}
		void Period(const double value){m_Period = value;}

		const float Volume(){return m_Volume;}
		void Volume(const float value){m_Volume = value;}
		inline const int DefaultVolume(){return m_ChannelDefVolume;}
		void DefaultVolume(const int value){
			m_ChannelDefVolume = value;
			if (DefaultIsMute() ) IsMute(true);
			Volume(DefaultVolumeFloat());
		}
		inline const float DefaultVolumeFloat() { return (m_ChannelDefVolume&0xFF)/200.0f; }
		void DefaultVolumeFloat(float value,bool ignoremute=false)
		{
			if ( DefaultIsMute() && !ignoremute ) m_ChannelDefVolume = int(value*200) | 0x100;
			else m_ChannelDefVolume = int(value*200);
		}
		inline const bool DefaultIsMute() { return m_ChannelDefVolume&0x100; }
		void DefaultIsMute(bool mute)
		{
			if (mute) m_ChannelDefVolume |= 0x100;
			else m_ChannelDefVolume &=0xFF;
		}
		const int LastVoiceVolume(){return m_LastVoiceVolume;}
		void LastVoiceVolume(const int value){m_LastVoiceVolume = value;}

		const float PanFactor(){return 	m_PanFactor;}
		void PanFactor(const float value){
			m_PanFactor = value;
			if ( ForegroundVoice()) ForegroundVoice()->PanFactor(value);
		}
		inline const int DefaultPanFactor() { return m_DefaultPanFactor; }
		void DefaultPanFactor(const int value){
			m_DefaultPanFactor = value;
			PanFactor(DefaultPanFactorFloat());
			if (DefaultIsSurround() ) IsSurround(true);
		}
		inline const float DefaultPanFactorFloat() { return (m_DefaultPanFactor&0xFF)/200.0f; }
		void DefaultPanFactorFloat(float value,bool ignoresurround=false)
		{
			if ( DefaultIsSurround() && !ignoresurround )  m_DefaultPanFactor = int(value*200) | 0x100;
			else m_DefaultPanFactor = int(value*200);
		}

		inline const bool DefaultIsSurround() { return (m_DefaultPanFactor&0x100); }
		void DefaultIsSurround(bool surr)
		{
			if (surr) m_DefaultPanFactor |= 0x100;
			else m_DefaultPanFactor &=0xFF;
		}

		const float LastVoicePanFactor(){return m_LastVoicePanFactor;}
		void LastVoicePanFactor(const float value){m_LastVoicePanFactor = value;}

		const int LastAmpEnvelopePosInSamples() { return m_LastAmpEnvelopePosInSamples; }
		void LastAmpEnvelopePosInSamples(const int value) { m_LastAmpEnvelopePosInSamples = value; }

		const int LastPanEnvelopePosInSamples() { return m_LastPanEnvelopePosInSamples; }
		void LastPanEnvelopePosInSamples(const int value) { m_LastPanEnvelopePosInSamples = value; }

		const int LastFilterEnvelopePosInSamples() { return m_LastFilterEnvelopePosInSamples; }
		void LastFilterEnvelopePosInSamples(const int value) { m_LastFilterEnvelopePosInSamples = value; }

		const int LastPitchEnvelopePosInSamples() { return m_LastPitchEnvelopePosInSamples; }
		void LastPitchEnvelopePosInSamples(const int value) { m_LastPitchEnvelopePosInSamples = value; }

		const int OffsetMem() { return m_OffsetMem; }
		void OffsetMem(const int value) { m_OffsetMem=value; }

		const bool IsSurround(){ return m_bSurround;}
		void IsSurround(const bool value){
			if ( value )
			{
				///\todo:
				/* if ( STANDARDSURROUND ) */ m_PanFactor = 0.5f;
			}
			m_bSurround = value;
		}
		const bool IsMute(){ return m_bMute;}
		void IsMute(const bool value){
			m_bMute = value;
		}

		const int Cutoff() { return m_Cutoff;}
		void Cutoff(const int cut) { m_Cutoff =cut;  if ( ForegroundVoice() ) ForegroundVoice()->CutOff(cut); }
		const int Ressonance() { return m_Ressonance;}
		void Ressonance(const int res) { m_Ressonance=res; if ( ForegroundVoice() ) ForegroundVoice()->Ressonance(res);}
		const dsp::FilterType FilterType() { return m_FilterType;}

		const int DefaultCutoff(){return m_DefaultCutoff;}
		void DefaultCutoff(const int value){m_DefaultCutoff = value; Cutoff(value);}
		const int DefaultRessonance(){return m_DefaultRessonance; }
		void DefaultRessonance(const int value){m_DefaultRessonance = value; Ressonance(value); }
		const dsp::FilterType DefaultFilterType(){return m_DefaultFilterType;}
		void DefaultFilterType(const dsp::FilterType value){m_DefaultFilterType = value; m_FilterType = value; }

		const bool IsGrissando(){return m_bGrissando;}
		void IsGrissando(const bool value){m_bGrissando = value;}
		void VibratoType(const int value){	m_VibratoType = value;}
		const int VibratoType() {return m_VibratoType;}
		void TremoloType(const int type){m_TremoloType = type;}
		const int TremoloType(){return m_TremoloType;}
		void PanbrelloType(const int type){m_PanbrelloType = type;}
		const int PanbrelloType(){return m_PanbrelloType;}

		const bool IsArpeggio() { return ((m_EffectFlags & EffectFlag::ARPEGGIO) != 0); }
		const bool IsVibrato(){return (m_EffectFlags & EffectFlag::VIBRATO) != 0;}
/*		void VibratoAmount(const double value){m_VibratoAmount = value;}
		const double VibratoAmount(){return m_VibratoAmount;}
*/

	private:


		int m_Index;// Channel Index.
		XMSampler *m_pSampler;
		int m_InstrumentNo;///< ( 0 .. 255 )
		XMSampler::Voice* m_pForegroundVoice;

		int m_Note;
		double m_Period;

		float m_Volume;///<  (0 - 1.0f)
		int m_ChannelDefVolume;///< (0..200)   &0x100 = Mute.
		int m_LastVoiceVolume;
		bool m_bMute;

		float m_PanFactor;// value used for Playback
		int m_DefaultPanFactor;  // value used for Storage //  0..200 .  &0x100 == Surround.
		float m_LastVoicePanFactor;
		bool m_bSurround;

		int m_LastAmpEnvelopePosInSamples;
		int m_LastPanEnvelopePosInSamples;
		int m_LastFilterEnvelopePosInSamples;
		int m_LastPitchEnvelopePosInSamples;

		bool m_bGrissando;
		int m_VibratoType;///< vibrato type
		int m_TremoloType;
		int m_PanbrelloType;


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
		PatternEvent m_DelayedNote;

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

		int m_MIDI_Set;
		int m_Cutoff;
		int m_Ressonance;
		dsp::FilterType m_FilterType;
		int m_DefaultCutoff;
		int m_DefaultRessonance;
		dsp::FilterType m_DefaultFilterType;
	};



	//////////////////////////////////////////////////////////////////////////
	//  XMSampler Declaration

	struct PanningMode {
		enum Type {
			Linear=0,
			TwoWay,
			EqualPower
		};
	};


	XMSampler(MachineCallbacks* callb,Machine::id_type id); friend class InternalHost;
	~XMSampler(){};

	virtual void Init(void);

	//These Tick() are "NewLine()" and NewEvent(). The API needs to be renamed.
	void Tick();
	virtual int GenerateAudioInTicks( int startSample, int numSamples );
	virtual void SetSampleRate(int sr);
	virtual void Stop(void);
	virtual void Tick(int channel, const PatternEvent & data );
	virtual MachineKey getMachineKey() const { return MachineKey::sampulse(); }
	virtual std::string GetName(void) const { return _psName; }

	virtual bool LoadPsy2FileFormat(RiffFile* pFile);
	virtual bool LoadSpecificChunk(RiffFile* riffFile, int version);
	virtual void SaveSpecificChunk(RiffFile* riffFile);

	MachineCallbacks* pCallbacks() { return callbacks; }

/*	Deprecated. See why in the body of "CalcBPMAndTick()"

	//Beats Per Minute
	void BPM (const int value){m_BPM = value;}
	const int BPM (){return m_BPM;}

	// MOD Tick
	void TicksPerRow(const int value){m_TicksPerRow = value;}
	const int TicksPerRow(){ return m_TicksPerRow;}

	/// BPM Speed
	void CalcBPMAndTick();
*/
	int Speed2LPB(int speed) { return 24/((speed==0)?6:speed); }
	int LPB2Speed(int lpb) { return 24/lpb; }
	Voice* GetCurrentVoice(int channelNum)
	{
		for(int current = 0;current < _numVoices;current++)
		{
			if ( (m_Voices[current].ChannelNum() == channelNum)  // Is this one an active note in this channel?
				&& m_Voices[current].IsPlaying() && !m_Voices[current].IsBackground())
			{
				//There can be only one foreground active voice for each channel, so we go out of the loop.
				return &m_Voices[current];
			}
		}
		return NULL;
	}
	Voice* GetFreeVoice()
	{
		//\todo : this function needs to be upgraded. This is a pretty simple allocation function.
		for (int voice = 0; voice < _numVoices; voice++)
		{
			if(!m_Voices[voice].IsPlaying()){
				return  &(m_Voices[voice]);
			}
		}
		return NULL;
	}
	int GetPlayingVoices(void)
	{
		int c=0;
		for (int i=0;i<MAX_POLYPHONY;i++)
		{
			if (m_Voices[i].IsPlaying()) c++;
		}
		return c;
	}

/// properties
	XMSampler::Channel& rChannel(const int index){ return m_Channel[index];}///< Channel
	Voice& rVoice(const int index) { return m_Voices[index];}///<

	const bool IsAmigaSlides(){ return m_bAmigaSlides;}
	void IsAmigaSlides(const bool value){ m_bAmigaSlides = value;}

	/// set current voice number
	const int NumVoices(){ return _numVoices;}
	/// get current voice number
	void NumVoices(const int value){_numVoices = value;}

	int GlobalVolume() { return m_GlobalVolume;}
	void GlobalVolume(const int value) { m_GlobalVolume= value;}
	void SlideVolume(const int value) {
		m_GlobalVolume += value;
		if ( m_GlobalVolume > 128 ) m_GlobalVolume = 128;
		else if ( m_GlobalVolume < 0) m_GlobalVolume = 0;
	}

	/// set resampler quality
	void ResamplerQuality(const dsp::ResamplerQuality value){
		_resampler.SetQuality(value);
	}

	const dsp::ResamplerQuality ResamplerQuality(){
		return _resampler.GetQuality();
	}
	const bool UseFilters(void) { return m_UseFilters; }
	void UseFilters(bool usefilters) { m_UseFilters = usefilters; }
	int PanningMode() { return m_PanningMode;}
	void PanningMode(const int value) { m_PanningMode= value;}

	void SetZxxMacro(int index,int mode, int val) { zxxMap[index].mode= mode; zxxMap[index].value=val; }
	ZxxMacro GetMap(int index) { return zxxMap[index]; }

	std::mutex & Mutex() { return m_Mutex; }

	const int SampleCounter(){return _sampleCounter;}// Sample pos since last linechange.
	void SampleCounter(const int value){_sampleCounter = value;}// ""

	void NextSampleTick(const int value){m_NextSampleTick = value;}// Sample Pos of the next (tracker) tick
	const int NextSampleTick(){ return m_NextSampleTick;}// ""

	void CurrentTick(const int value){m_TickCount = value;}// Current Tracker Tick number
	const int CurrentTick(){ return m_TickCount;}// ""

	int GetDeltaTick() { return m_DeltaTick; }

	static const float AmigaPeriod[XMInstrument::NOTE_MAP_SIZE];
protected:

	static std::string _psName;
	int _numVoices;

	Voice m_Voices[MAX_POLYPHONY];
	XMSampler::Channel m_Channel[MAX_TRACKS];
	dsp::Cubic _resampler;
	ZxxMacro zxxMap[128];


	void DeltaTick(const int value){m_DeltaTick = value;}
	const int DeltaTick(){return m_DeltaTick;}
	void WorkVoices(int sampleOffset, int numsamples);

private:

	bool m_bAmigaSlides;// Using Linear or Amiga Slides.
	bool m_UseFilters;
	int m_GlobalVolume;
	int m_PanningMode;
/*	int m_BPM;
	int m_TicksPerRow;	// Tracker Ticks. Also called "speed".
*/
	int m_TickCount;	// Current Tick number.
	int m_DeltaTick;	// Duration in Samples of a tracker tick.
	int m_NextSampleTick;// The sample position of the next Tracker Tick
	int _sampleCounter;	// Number of Samples since note start

	typedef std::scoped_lock<std::mutex> scoped_lock;
	std::mutex mutable m_Mutex;
};

}}
#endif


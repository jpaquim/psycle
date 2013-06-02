///\file
///\brief interface file for psycle::host::XMSampler.
#pragma once
#include <psycle/host/detail/project.hpp>
//#include "Global.hpp"
#include "Machine.hpp"
#include "XMInstrument.hpp"

#include <psycle/helpers/filter.hpp>
#include <psycle/helpers/resampler.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/cstdint.hpp>
#include <cstddef> // for std::ptrdiff_t

namespace psycle { namespace host {

class XMSampler : public Machine
{
public:

	static const int MAX_POLYPHONY = 64;///< max polyphony 
	static const std::uint32_t VERSION = 0x00010001;
	static const std::uint32_t VERSION_ONE = 0x00010000;

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
		PORTAMENTO_UP		=	0x01,// Portamento Up , Fine porta (01Fx, and Extra fine porta 01Ex )	(*t)
		PORTAMENTO_DOWN		=	0x02,// Portamento Down, Fine porta (02Fx, and Extra fine porta 02Ex ) (*t)
		PORTA2NOTE			=	0x03,// Tone Portamento						(*tn)
		VIBRATO				=	0x04,// Do Vibrato							(*t)
		TONEPORTAVOL		=	0x05,// Tone Portament & Volume Slide		(*t)
		VIBRATOVOL			=	0x06,// Vibrato & Volume Slide				(*t)
		TREMOLO				=	0x07,// Tremolo								(*t)
		PANNING				=	0x08,// Set Panning Position				(p)
		PANNINGSLIDE		=	0x09,// Panning slide						(*t)
		SET_CHANNEL_VOLUME	=	0x0A,// Set channel's volume				(p)
		CHANNEL_VOLUME_SLIDE=	0x0B,// channel Volume Slide up (0Dy0) down (0D0x), Fine slide up(0DFy) down(0DxF)	 (*tp)
		VOLUME				=	0x0C,// Set Volume
		VOLUMESLIDE			=	0x0D,// Volume Slide up (0Dy0), down (0D0x), Fine slide up(0DyF), down(0DFy)	 (*t)
		FINESLIDEUP         =   0x0F,//Part of the value that indicates it is a fine slide up
		FINESLIDEDOWN       =   0xF0,//Part of the value that indicates it is a fine slide down
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
		SENDTOVOLUME		=	0x1E,// Interprets this as a volume command	()
		OFFSET				=	0x90 // Set Sample Offset  , note!: 0x9yyy ! not 0x90yy (*n)
		};
	};
#define ISSLIDEUP(val) !((val)&0x0F)
#define ISSLIDEDOWN(val) !((val)&0xF0)
#define ISFINESLIDEUP(val) (((val)&0x0F)==CMD::FINESLIDEUP)
#define ISFINESLIDEDOWN(val) (((val)&0xF0)==CMD::FINESLIDEDOWN)
#define GETSLIDEUPVAL(val) (((val)&0xF0)>>4)
#define GETSLIDEDOWNVAL(val) ((val)&0x0F)

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

	static const char E8VolMap[16];

	class Channel;
	class Voice;

//////////////////////////////////////////////////////////////////////////
//  XMSampler::WaveDataController Declaration
	//\todo: WaveDateController Needs to update the speed if sampleRate changes (but... would the samplerate change while
	//       there's a voice playing?)
	class WaveDataController
	{
	public:
		typedef void (*WorkFunction)(WaveDataController& contr, float *pLeftw,float *pRightw);
		struct LoopDirection {
			enum Type {
				FORWARD = 0,
				BACKWARD
			};
		};
		WaveDataController():resampler_data(NULL){};
		virtual ~WaveDataController(){};

		virtual void Init(const XMInstrument::WaveData* const wave, int layer, const helpers::dsp::resampler & resampler);
		virtual void DisposeResampleData(const helpers::dsp::resampler& resampler);
		virtual void RecreateResampleData(const helpers::dsp::resampler& resampler);
		virtual void RefillBuffers();
		virtual void RefillBuffer(std::int16_t buffer[192], const std::int16_t* data);
		virtual void NoteOff(void);
		virtual int PreWork(int numSamples, WorkFunction* pWork);
		static void WorkMonoStatic(WaveDataController& contr,float *pLeftw,float *pRightw) { contr.WorkMono(pLeftw, pRightw);}
		inline void WorkMono(float *pLeftw,float *pRightw)
		{
			*pLeftw = resampler_work(m_pL, m_Position.LowPart, resampler_data);
			/* *pLeftw = resampler_work(m_pL, m_Position.HighPart, m_Position.LowPart, Length(), resampler_data);
			if ((m_Position.HighPart >= 14  && m_Position.HighPart <= 16)|| (m_Position.HighPart >= 491 && m_Position.HighPart <=493)) {
				TRACE("Pos: %d [%d,%d,%d,%d]: %f\n",m_Position.HighPart, *(m_pL-1),*(m_pL),*(m_pL+1),*(m_pL+2), *pLeftw);
			}*/
			const std::ptrdiff_t old = m_Position.HighPart;
			m_Position.QuadPart+=m_SpeedInternal;
			const std::ptrdiff_t diff = static_cast<std::ptrdiff_t>(m_Position.HighPart)-old;
			m_pL+=diff;
#ifndef NDEBUG
			if (static_cast<std::int32_t>(m_Position.HighPart) >= m_CurrentLoopEnd+17) {
				int i=0;
			}
#endif
		}
		static void WorkStereoStatic(WaveDataController& contr,float *pLeftw,float *pRightw) { contr.WorkStereo(pLeftw, pRightw);}
		inline void WorkStereo(float *pLeftw,float *pRightw)
		{
			//Process sample
			//todo: sinc resampling would benefit from having a stereo version of resampler_work
			*pLeftw  = resampler_work(m_pL, m_Position.LowPart, resampler_data);
			*pRightw = resampler_work(m_pR, m_Position.LowPart, resampler_data);
			/*
			*pLeftw  = resampler_work(m_pL, m_Position.HighPart, m_Position.LowPart, Length(), resampler_data);
			*pRightw = resampler_work(m_pR, m_Position.HighPart, m_Position.LowPart, Length(), resampler_data);
			*/
			const std::ptrdiff_t old = m_Position.HighPart;
			m_Position.QuadPart+=m_SpeedInternal;
			const std::ptrdiff_t diff = static_cast<std::ptrdiff_t>(m_Position.HighPart)-old;
			m_pL+=diff;
			m_pR+=diff;
#ifndef NDEBUG
			if (static_cast<std::int32_t>(m_Position.HighPart) >= m_CurrentLoopEnd+17) {
				int i=0;
			}
#endif
		}
		virtual void PostWork();

		// Properties
		inline int Layer() const { return m_Layer;}
		virtual const XMInstrument::WaveData &Wave() const { return *m_pWave; }

		inline bool Playing() const { return m_Playing;}
		virtual void Playing(bool play){ m_Playing=play; }

		// Current sample position 
		inline int  Position() const { return m_Position.HighPart;}
		virtual void Position(const	int value){ 
			if ( value < Length()) m_Position.HighPart = value;
			else m_Position.HighPart = Length()-1;
		}
		
		// Current sample Speed
		inline std::int64_t Speed() const {return m_Speed;}
		virtual void Speed(const helpers::dsp::resampler & resampler, const double value){
			m_Speed = static_cast<std::int64_t>(value * 4294967296.0); // 4294967296 is a left shift of 32bits
			resampler.UpdateSpeed(resampler_data, value);
			m_SpeedInternal = (CurrentLoopDirection() == LoopDirection::FORWARD) ? m_Speed : -1*m_Speed;
		}

		inline LoopDirection::Type CurrentLoopDirection() const {return m_CurrentLoopDirection;}
		virtual void CurrentLoopDirection(const LoopDirection::Type dir){m_CurrentLoopDirection = dir;}

		inline XMInstrument::WaveData::LoopType::Type LoopType() const {return m_pWave->WaveLoopType();}
		inline std::uint32_t LoopStart() const {return m_pWave->WaveLoopStart();}
		inline std::uint32_t LoopEnd() const { return m_pWave->WaveLoopEnd();}

		inline XMInstrument::WaveData::LoopType::Type SustainLoopType() const {return m_pWave->WaveSusLoopType();}
		inline std::uint32_t SustainLoopStart() const {return m_pWave->WaveSusLoopStart();}
		inline std::uint32_t SustainLoopEnd() const { return m_pWave->WaveSusLoopEnd();}

		inline std::uint32_t Length() const {return m_pWave->WaveLength();}

		inline bool IsStereo() const { return m_pWave->IsWaveStereo();}

		// pointer to Start of Left sample
		virtual const std::int16_t* pLeft() const {return m_pL;}
		// pointer to Start of Right sample
		virtual const std::int16_t* pRight() const {return m_pR;}


	protected:
		int m_Layer;
		const XMInstrument::WaveData *m_pWave;
		ULARGE_INTEGER m_Position;
		std::int64_t m_Speed;
		std::int64_t m_SpeedInternal;
		bool m_Playing;

		XMInstrument::WaveData::LoopType::Type m_CurrentLoopType;
		int m_CurrentLoopEnd;
		int m_CurrentLoopStart;
		LoopDirection::Type m_CurrentLoopDirection;

		const std::int16_t* m_pL;
		const std::int16_t* m_pR;

		helpers::dsp::resampler::work_unchecked_func_type resampler_work;
		void* resampler_data;

		std::int16_t lBuffer[64*3];
		std::int16_t rBuffer[64*3];
		//int requiredpre;  //Currently assumed to be the highest one, i.e. SINC sizes.
		//int requiredpost;
	};

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
				RELEASE = 8,  // Indicates that a Note-Off has been issued.
				PAUSED = 16 // Indicates that it is either paused by user ( EE commands) or by loop
			};
		};
		EnvelopeController(Voice& invoice):voice(invoice){};
		virtual ~EnvelopeController(){};

		void Init();
		void Init(const XMInstrument::Envelope& envelope) {
			m_pEnvelope = &envelope;
			Init();
		}

		void NoteOn();
		void NoteOff();
		void Pause();
		void Continue();

		inline void Work()
		{
			if(!(m_Stage&EnvelopeStage::PAUSED))
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
								m_Stage = EnvelopeStage::Type(m_Stage | EnvelopeStage::PAUSED);
							}
							else { m_PositionIndex = m_pEnvelope->SustainBegin(); }
						}
					}
					else if (m_Stage&EnvelopeStage::HASLOOP)
					{
						if ( m_PositionIndex >= m_pEnvelope->LoopEnd())
						{
							// if the begin==end, pause the envelope.
							if ( m_pEnvelope->LoopStart() == m_pEnvelope->LoopEnd() )
							{
								m_Stage = EnvelopeStage::Type(m_Stage | EnvelopeStage::PAUSED);
							}
							else { m_PositionIndex = m_pEnvelope->LoopStart(); }
						}
					}
					if (!(m_Stage&EnvelopeStage::PAUSED)) 
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
		void CalcStep(const int start,const int  end);
		void SetPositionInSamples(const int samplePos);
		inline int GetPositionInSamples() const {
			//TRACE("Requested Pos:%d. Idx:%d, Current Amount:%f\n",m_Samples,m_PositionIndex,m_ModulationAmount);
			//TRACE("-GET-Idx:%d, Step:%f, Current Amount:%f\n",m_PositionIndex,m_Step,m_ModulationAmount);
			return m_Samples;
		}
		void RecalcDeviation();
		inline XMInstrument::Envelope::ValueType ModulationAmount() const { return m_ModulationAmount; }

		inline const XMInstrument::Envelope & Envelope()const {return *m_pEnvelope;}
		inline EnvelopeStage::Type Stage() const {return m_Stage;}
		void Stage(const EnvelopeStage::Type value){m_Stage = value;}
		void SetPosition(const int posi) { 
			m_PositionIndex=posi-1; 
			Continue();
			m_Samples= m_NextEventSample-1;  // this forces a recalc when entering Work().
		}
		inline int GetPosition(void) const { return m_PositionIndex; }
	private:
		inline float SRateDeviation() const { return m_sRateDeviation; }

		int m_Samples;
		float m_sRateDeviation;
		int m_PositionIndex;
		int m_NextEventSample;
		EnvelopeStage::Type m_Stage;

		const XMInstrument::Envelope* m_pEnvelope;

		XMInstrument::Envelope::ValueType m_ModulationAmount;
		XMInstrument::Envelope::ValueType m_Step;
		Voice& voice;
	};// EnvelopeController



/////////////////////////////////////////////////////////////////////////////////////
//  XMSampler::Voice Declaration 
//	(This class could be called XMInstrumentController too, but "Voice" describes better how it is used.)

	class Voice
	{
	public:
		///
		friend class XMSampler::Channel;

		Voice():m_AmplitudeEnvelope(*this), m_PanEnvelope(*this), m_PitchEnvelope(*this),m_FilterEnvelope(*this) {
			Reset();
		}
		virtual ~Voice();

		// Object Functions
		void Reset();
		void ResetEffects();

		void VoiceInit(const XMInstrument& xins, int channelNum,int instrumentNum);
		void Work(int numSamples,float * pSamplesL,float *pSamplesR);

		// This one is Tracker Tick (Mod-tick)
		void Tick();
		// This one is Psycle's "NewLine"
		void NewLine();

		void NoteOn(const std::uint8_t note,const std::int16_t playvol=-1,bool reset=true);
		void NoteOff();
		void NoteOffFast();
		void NoteFadeout();
		void UpdateFadeout();
		XMInstrument::NewNoteAction::Type NNA() const { return m_NNA;}
		void NNA(const XMInstrument::NewNoteAction::Type value){ m_NNA = value;}
		
		void ResetVolAndPan(std::int16_t playvol,bool reset=true);
		void UpdateSpeed();
		double PeriodToSpeed(int period) const;


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
		bool IsAutoVibrato() const { return rWave().Wave().IsAutoVibrato(); }
		// Get Auto Vibrato Amount
		double AutoVibratoAmount() const {return m_AutoVibratoAmount;}


// Properties
		int InstrumentNum() const{ return _instrument;}
		void InstrumentNum(const int value){_instrument = value;}
//		XMInstrument &rInstrument() { return *m_pInstrument;}
		const XMInstrument &rInstrument() const { return *m_pInstrument;}
		void pInstrument(const XMInstrument * const p){m_pInstrument = p;}

		int ChannelNum() const { return m_ChannelNum;}
		void ChannelNum(const int value){ m_ChannelNum = value;}
		void pChannel(XMSampler::Channel * const p){m_pChannel = p;};
//		void pChannel(const XMSampler::Channel * const p){m_pChannel = p;};
		XMSampler::Channel& rChannel(){return *m_pChannel;}
		const XMSampler::Channel& rChannel() const {return *m_pChannel;}

		void pSampler(XMSampler * const p){m_pSampler = p;}
		XMSampler * const pSampler() {return m_pSampler;}
		const XMSampler * const pSampler() const {return m_pSampler;}

		XMSampler::EnvelopeController& AmplitudeEnvelope(){return m_AmplitudeEnvelope;}
		XMSampler::EnvelopeController& FilterEnvelope(){return m_FilterEnvelope;}
		XMSampler::EnvelopeController& PitchEnvelope(){return m_PitchEnvelope;}
		XMSampler::EnvelopeController& PanEnvelope(){return m_PanEnvelope;}

		WaveDataController& rWave() {return m_WaveDataController;}
		const WaveDataController& rWave() const {return m_WaveDataController;}
		void DisposeResampleData(helpers::dsp::resampler& resampler);
		void RecreateResampleData(helpers::dsp::resampler& resampler);

		bool IsPlaying() const { return m_bPlay;}
		void IsPlaying(const bool value)
		{ 
			if ( value == false )
			{
				if ( rChannel().ForegroundVoice() == this) 
				{
					rChannel().LastVoicePanFactor(m_PanFactor);
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

		bool IsBackground() const { return m_Background; }
		void IsBackground(const bool background){ m_Background = background; }

		bool IsStopping() const { return m_Stopping; }
		void IsStopping(const bool stop) { m_Stopping = stop; }

		// Volume of the current note.
		std::uint16_t Volume() const { return m_Volume; }
		void Volume(const std::uint16_t vol)
		{
			m_Volume = vol;
			m_RealVolume = rWave().Wave().WaveGlobVolume() * rInstrument().GlobVol() * (vol/128.0f);
				//\todo :  rInstrument().RandomVolume() / 100.0f;
		}
		// Voice.RealVolume() returns the calculated volume out of "WaveData.WaveGlobVol() * Instrument.Volume() * Voice.NoteVolume()"
		float RealVolume() const { return (!m_bTremorMute)?(m_RealVolume+m_TremoloAmount):0; }
		void PanFactor(float pan)
		{
			m_PanFactor = pan;
			m_PanRange = 2.f * (0.5-std::abs(pan-0.5));
		}
		float PanFactor() const { return m_PanFactor; }
		void IsSurround(bool surround) { m_Surround = surround; }
		bool IsSurround() const { return m_Surround; }


		int CutOff() const { return m_CutOff; }
		void CutOff(int co)
		{
			m_CutOff = co; 
			m_FilterIT.Cutoff(co);
			m_FilterClassic.Cutoff(co);
		}
		
		int Ressonance() const { return m_Ressonance; }
		void Ressonance(int res)
		{
			m_Ressonance = res; 
			m_FilterIT.Ressonance(res);
			m_FilterClassic.Ressonance(res);
		}

		void FilterType(dsp::FilterType ftype) {
			if (ftype==dsp::F_ITLOWPASS) {
				m_Filter = &m_FilterIT;
			}
			else {
				m_Filter = &m_FilterClassic;
			}
			m_Filter->Type(ftype); 
		}

		void Period(int newperiod) { m_Period = newperiod; UpdateSpeed(); }
		int Period() const { return m_Period; }
		// convert note to period
		double NoteToPeriod(const int note, bool correctNote=true) const;
		// convert period to note 
		int PeriodToNote(const double period) const;

		double VibratoAmount() const { return m_VibratoAmount; }

		inline int SampleRate() const { return m_pSampler->SampleRate(); }
	protected:
		// Gets the delta between the points of the wavetables for tremolo/panbrello/vibrato
		int GetDelta(int wavetype,int wavepos) const;
		float PanRange() const { return m_PanRange; }
		bool IsTremorMute() const {return m_bTremorMute;}
		void IsTremorMute(const bool value){m_bTremorMute = value;}


	private:

		int m_ChannelNum;
		XMSampler::Channel* m_pChannel;
		XMSampler * m_pSampler;

		int _instrument;// Instrument
		const XMInstrument * m_pInstrument;
		XMInstrument::NewNoteAction::Type m_NNA;


		EnvelopeController m_AmplitudeEnvelope;
		EnvelopeController m_PanEnvelope;
		EnvelopeController m_PitchEnvelope;
		EnvelopeController m_FilterEnvelope;

		WaveDataController m_WaveDataController;

		dsp::ITFilter m_FilterIT;
		dsp::Filter m_FilterClassic;
		dsp::Filter* m_Filter;
		int m_CutOff;
		int m_Ressonance;
		float _coModify;

		bool m_bPlay;
		bool m_Background;
		bool m_Stopping;
		int m_Note;
		int m_Period;
		int m_Volume;
		float m_RealVolume;
		//Volume ramping 
		float m_lVolCurr;
		float m_lVolDest;
		float m_rVolCurr;
		float m_rVolDest;


		float m_PanFactor;
		float m_PanRange;
		bool m_Surround;

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
		void Save(RiffFile& riffFile) const;
		void Init();
		void EffectInit();
		void Restore();

		// Prepare the channel for the new effect (or execute if it's a one-shot one). This is executed on TrackerTick==0
		void SetEffect(Voice* voice,int volcmd,int cmd,int parameter);

		// Executes the slide/change effects. This is executed on TrackerTick!=0
		void PerformFx();

		int EffectFlags() const {return m_EffectFlags;}
		void EffectFlags(const int value){m_EffectFlags = value;}


// Effect-Related Object Functions

		// Tick 0 commands
		void GlobalVolSlide(int speed);
		void PanningSlide(int speed);
		void ChannelVolumeSlide(int speed);
		void PitchSlide(bool bUp,int speed,int note=notecommands::empty);
		void VolumeSlide(int speed);
		void Tremor(int parameter);
		void Vibrato(int speed,int depth = 0);
		void Tremolo(int speed,int depth);
		void Panbrello(int speed,int depth);
		void Arpeggio(const int param);
		void Retrigger(const int param);
		void NoteCut(const int ntick);
		void DelayedNote(PatternEntry data);

		// Tick n commands.
		void PanningSlide();
		void ChannelVolumeSlide();
		void NoteCut();
		void StopBackgroundNotes(XMInstrument::NewNoteAction::Type action);

		double ArpeggioPeriod() const
		{
			const int arpi = m_pSampler->CurrentTick()%3;
			if(arpi >= 1){
				return m_ArpeggioPeriod[arpi - 1];
			} else {
				return m_Period;
			}
		}


// Properties
		int Index() const { return m_Index;}
		void Index(const int value){m_Index = value;}

		void pSampler(XMSampler * const pSampler){m_pSampler = pSampler;}

		int InstrumentNo() const {return m_InstrumentNo;}
		void InstrumentNo(const int no){m_InstrumentNo = no;}

		XMSampler::Voice* ForegroundVoice() { return m_pForegroundVoice; }
		const XMSampler::Voice* ForegroundVoice() const { return m_pForegroundVoice; }
		void ForegroundVoice(XMSampler::Voice* pVoice) {m_pForegroundVoice = pVoice;}

		int Note() const { return m_Note;}
		void Note(const int note)
		{	m_Note = note;
			if (ForegroundVoice()) {
				m_Period = ForegroundVoice()->NoteToPeriod(note);
			}
		}
		double Period() const {return m_Period;}
		void Period(const double value){m_Period = value;}

		float Volume() const {return m_Volume;}
		void Volume(const float value){m_Volume = value;}
		inline int DefaultVolume() const {return m_ChannelDefVolume;}
		void DefaultVolume(const int value){
			m_ChannelDefVolume = value; 
			if (DefaultIsMute() ) IsMute(true);
			Volume(DefaultVolumeFloat());
		}
		inline float DefaultVolumeFloat() const { return (m_ChannelDefVolume&0xFF)/200.0f; }
		void DefaultVolumeFloat(float value,bool ignoremute=false)
		{
			if ( DefaultIsMute() && !ignoremute ) m_ChannelDefVolume = int(value*200) | 0x100;
			else m_ChannelDefVolume = int(value*200);
		}
		inline bool DefaultIsMute() const { return m_ChannelDefVolume&0x100; }
		void DefaultIsMute(bool mute)
		{
			if (mute) m_ChannelDefVolume |= 0x100;
			else m_ChannelDefVolume &=0xFF;
		}
		int LastVoiceVolume() const {return m_LastVoiceVolume;}
		void LastVoiceVolume(const int value){m_LastVoiceVolume = value;}

		float PanFactor() const {return 	m_PanFactor;}
		void PanFactor(const float value){
			m_PanFactor = value;
			if ( ForegroundVoice()) ForegroundVoice()->PanFactor(value);
		}
		inline int DefaultPanFactor() const { return m_DefaultPanFactor; }
		void DefaultPanFactor(const int value){
			m_DefaultPanFactor = value;
			PanFactor(DefaultPanFactorFloat());
			if (DefaultIsSurround() ) IsSurround(true);
		}
		inline float DefaultPanFactorFloat() const { return (m_DefaultPanFactor&0xFF)/200.0f; }
		void DefaultPanFactorFloat(float value,bool ignoresurround=false)
		{
			if ( DefaultIsSurround() && !ignoresurround )  m_DefaultPanFactor = int(value*200) | 0x100;
			else m_DefaultPanFactor = int(value*200);
		}

		inline bool DefaultIsSurround() const { return (m_DefaultPanFactor&0x100); }
		void DefaultIsSurround(bool surr)
		{
			if (surr) m_DefaultPanFactor |= 0x100;
			else m_DefaultPanFactor &=0xFF;
		}

		float LastVoicePanFactor() const {return m_LastVoicePanFactor;}
		void LastVoicePanFactor(const float value){m_LastVoicePanFactor = value;}

		int LastAmpEnvelopePosInSamples() const { return m_LastAmpEnvelopePosInSamples; }
		void LastAmpEnvelopePosInSamples(const int value) { m_LastAmpEnvelopePosInSamples = value; }

		int LastPanEnvelopePosInSamples() const { return m_LastPanEnvelopePosInSamples; }
		void LastPanEnvelopePosInSamples(const int value) { m_LastPanEnvelopePosInSamples = value; }

		int LastFilterEnvelopePosInSamples() const { return m_LastFilterEnvelopePosInSamples; }
		void LastFilterEnvelopePosInSamples(const int value) { m_LastFilterEnvelopePosInSamples = value; }

		int LastPitchEnvelopePosInSamples() const { return m_LastPitchEnvelopePosInSamples; }
		void LastPitchEnvelopePosInSamples(const int value) { m_LastPitchEnvelopePosInSamples = value; }

		int OffsetMem() const { return m_OffsetMem; }
		void OffsetMem(const int value) { m_OffsetMem=value; }

		bool IsSurround() const { return m_bSurround;}
		void IsSurround(const bool value){
			m_bSurround = value;
			if ( ForegroundVoice()) ForegroundVoice()->IsSurround(value);
		}
		bool IsMute() const { return m_bMute;}
		void IsMute(const bool value){
			m_bMute = value;
		}

		int Cutoff() const { return m_Cutoff;}
		void Cutoff(const int cut) { m_Cutoff =cut;  if ( ForegroundVoice() ) ForegroundVoice()->CutOff(cut); }
		int Ressonance() const { return m_Ressonance;}
		void Ressonance(const int res) { m_Ressonance=res; if ( ForegroundVoice() ) ForegroundVoice()->Ressonance(res);}
		dsp::FilterType FilterType() const { return m_FilterType;}

		int DefaultCutoff() const {return m_DefaultCutoff;}
		void DefaultCutoff(const int value){m_DefaultCutoff = value; Cutoff(value);}
		int DefaultRessonance() const {return m_DefaultRessonance; }
		void DefaultRessonance(const int value){m_DefaultRessonance = value; Ressonance(value); }
		dsp::FilterType DefaultFilterType() const {return m_DefaultFilterType;}
		void DefaultFilterType(const dsp::FilterType value){m_DefaultFilterType = value; m_FilterType = value; }

		bool IsGrissando() const {return m_bGrissando;}
		void IsGrissando(const bool value){m_bGrissando = value;}
		void VibratoType(const int value){	m_VibratoType = value;}
		int VibratoType() const {return m_VibratoType;}
		void TremoloType(const int type){m_TremoloType = type;}
		int TremoloType() const {return m_TremoloType;}
		void PanbrelloType(const int type){m_PanbrelloType = type;}
		int PanbrelloType() const {return m_PanbrelloType;}

		bool IsArpeggio() const { return ((m_EffectFlags & EffectFlag::ARPEGGIO) != 0); }
		bool IsVibrato() const {return (m_EffectFlags & EffectFlag::VIBRATO) != 0;}
/*		void VibratoAmount(const double value){m_VibratoAmount = value;}
		double VibratoAmount() const {return m_VibratoAmount;}
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
		std::vector<PatternEntry> m_DelayedNote;

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


	XMSampler(int index);
	virtual ~XMSampler() {
		for (int i=0;i<MAX_POLYPHONY;i++) {
			rVoice(i).DisposeResampleData(_resampler);
		}
	}

	virtual void Init(void);
	
	virtual void NewLine();
	virtual int GenerateAudioInTicks(int startSample,  int numSamples);
	virtual void Stop(void);
	virtual bool playsTrack(const int track) const;
	virtual void Tick(int channel, PatternEntry* pData);
	virtual void PostNewLine();
	virtual float GetAudioRange() const { return 32768; }
	const char* const GetName(void) const { return _psName; }
	virtual void SetSampleRate(int sr);
	int SampleRate() const { return m_sampleRate; }
	virtual bool NeedsAuxColumn() { return true; }
	virtual const char* AuxColumnName(int idx) const;
	virtual int NumAuxColumnIndexes() { return XMInstrument::MAX_INSTRUMENT;}

	virtual bool Load(RiffFile* riffFile); // Old fileformat
	virtual bool LoadSpecificChunk(RiffFile* riffFile, int version);
	virtual void SaveSpecificChunk(RiffFile* riffFile);

	const Voice* GetCurrentVoice(int channelNum) const {
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
	Voice* GetFreeVoice(int channelNum)
	{
		//First, see if there's a free voice
		for (int voice = 0; voice < _numVoices; voice++)
		{
			if(!m_Voices[voice].IsPlaying()){
				return  &(m_Voices[voice]);
			}
		}
		//If there isn't, See if there are background voices in this channel
		int background = -1;
		for (int voice = 0; voice < _numVoices; voice++)
		{
			if(m_Voices[voice].IsBackground()){
				background = voice;
				if(m_Voices[voice].ChannelNum() == channelNum) {
					return  &(m_Voices[voice]);
				}
			}
		}
		//If still there isn't, See if there are background voices on other channels.
		//This could be improved in some sort of "older-first".
		if (background != -1) {
			return  &(m_Voices[background]);
		}
		return NULL;
	}
	int GetPlayingVoices(void) const
	{
		int c=0;
		for (int i=0;i<MAX_POLYPHONY;i++)
		{
			if (m_Voices[i].IsPlaying()) c++;
		}
		return c;
	}

/// properties
	XMSampler::Channel& rChannel(const int index){ return m_Channel[index];}
	const XMSampler::Channel& rChannel(const int index) const { return m_Channel[index];}
	Voice& rVoice(const int index){ return m_Voices[index];}

	bool IsAmigaSlides() const { return m_bAmigaSlides;}
	void IsAmigaSlides(const bool value){ m_bAmigaSlides = value;}

	/// set current voice number
	int NumVoices() const { return _numVoices;}
	/// get current voice number
	void NumVoices(const int value){_numVoices = value;}

	int GlobalVolume() const { return m_GlobalVolume;}
	void GlobalVolume(const int value) { m_GlobalVolume= value;}
	void SlideVolume(const int value) { 
		m_GlobalVolume += value;
		if ( m_GlobalVolume > 128 ) m_GlobalVolume = 128;
		else if ( m_GlobalVolume < 0) m_GlobalVolume = 0;
	}

	/// set resampler quality 
	void ResamplerQuality(const helpers::dsp::resampler::quality::type value){
		for (int i=0;i<MAX_POLYPHONY;i++) {
			rVoice(i).DisposeResampleData(_resampler);
		}
		_resampler.quality(value);
		for (int i=0;i<MAX_POLYPHONY;i++) {
			rVoice(i).RecreateResampleData(_resampler);
		}
	}

	helpers::dsp::resampler::quality::type ResamplerQuality() const {
		return _resampler.quality();
	}
	const helpers::dsp::resampler& Resampler() const {
		return _resampler;
	}
	bool UseFilters(void) const { return m_UseFilters; }
	void UseFilters(bool usefilters) { m_UseFilters = usefilters; }
	int PanningMode() const { return m_PanningMode;}
	void PanningMode(const int value) { m_PanningMode= value;}
	
	void SetZxxMacro(int index,int mode, int val) { zxxMap[index].mode= mode; zxxMap[index].value=val; }
	ZxxMacro GetMap(int index) const { return zxxMap[index]; }

	int SampleCounter() const {return _sampleCounter;}// Sample pos since last line change.
	void SampleCounter(const int value){_sampleCounter = value;}// ""

	void NextSampleTick(const int value){m_NextSampleTick = value;}// Sample Pos of the next (tracker) tick
	int NextSampleTick() const { return m_NextSampleTick;}// ""

	void CurrentTick(const int value){m_TickCount = value;}// Current Tracker Tick number
	int CurrentTick() const { return m_TickCount;}// ""

	static int XMSampler::CalcLPBFromSpeed(int trackerspeed, int &outextraticks);

	static const float AmigaPeriod[XMInstrument::NOTE_MAP_SIZE];
protected:

	static TCHAR* _psName;
	int _numVoices;

	Voice m_Voices[MAX_POLYPHONY];
	XMSampler::Channel m_Channel[MAX_TRACKS];
	psycle::helpers::dsp::cubic_resampler _resampler;
	ZxxMacro zxxMap[128];

	void WorkVoices(int numsamples);

private:

	bool m_bAmigaSlides;// Using Linear or Amiga Slides.
	bool m_UseFilters;
	int m_GlobalVolume;
	int m_PanningMode;
	int m_TickCount;	// Current Tick number.
	int m_NextSampleTick;// The sample position of the next Tracker Tick
	int _sampleCounter;	// Number of Samples since note start
	int m_sampleRate;
	std::vector<PatternEntry> multicmdMem;
};
}
}
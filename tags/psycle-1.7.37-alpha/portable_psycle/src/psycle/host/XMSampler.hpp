///\file
///\brief interface file for psycle::host::XMSampler.
#pragma once
#include "Machine.hpp"
#include "Filter.hpp"
#include "XMInstrument.hpp"

namespace psycle
{
	namespace host
	{
class XMSampler : public Machine
{
public:
	static const int MAX_POLYPHONY = 64;///< max polyphony 
	static const int MAX_INSTRUMENT = 255;///< max instrument
	static const compiler::uint32 VERSION = 0x00010000;

/*
Commands to implement as Global Commands:

SET_SPEED=				1,
SET_GLOBAL_VOLUME=		22,
GLOBAL_VOLUME_SLIDE=	23,


* = remembers its last value when called with param 00.
t = slides/changes each tick. (or is applied in a specific tick != 0 )
p = persistent ( a new note doesn't reset it )
n = they need to appear next to a note.

Commands are Implemented in XMSampler::Channel::SetEffect() , and type "t" commands also need to be added to
XMSampler::Channel::PerformFX().

*/
	struct CMD {
		enum{
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
		MIDI_MACRO			=	0x0F,// see MIDI.TXT						(p)
		ARPEGGIO			=	0x10,//	Arpeggio							(*t)
		RETRIG				=	0x11,// Retrigger Note						(*t)
							//0x12
							//0x13
		FINE_VIBRATO		=	0x14,// Vibrato 4 times finer				(*t)
		SET_ENV_POSITION	=	0x15,// Set Envelope Position
							//0x16
		TREMOR				=	0x17,// Tremor								(*t)
		PANBRELLO			=	0x18,// Panbrello							(*t)
		OFFSET_OLD			=	0x19,// it was 0x09 in old sampler			(n)
		RETRIG_OLD			=	0x1B,// it was 0x15 in old sampler			(n)
		OFFSET				=	0x90 // Set Sample Offset  , note!: 0x9yyy ! not 0x90yy
		};
	};
	struct CMD_E
	{
		enum{
		//\todo: Check XMInstrument::WaveData::WaveForms! IT is sine, square, sawdown and random
		E_GLISSANDO_TYPE	=	0x30, //E3     Set gliss control			(p)
		E_VIBRATO_WAVE		=	0x40, //E4     Set vibrato control			(p)
		//\todo : this one probably won't be implemented.
		E_FINETUNE			=	0x50, //E5     Set finetune
		E_SET_PANBRELLO_WAVE=	0x60, //									(p)
		E_TREMOLO_WAVE		=	0x70, //E7     Set tremolo control			(p)
		//\todo: remove this duplicated command? 
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
		enum{
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
		enum{
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

/*
// this is unfinished (as one can see by the strange numbers)
	struct CMD_VOL
	{
		enum{
		VOL_VOLUME			=	0x50 // 0x10..0x50 (64)
		VOL_FINEVOLUP		=	0x90; //(0!)        (16)
		VOL_FINEVOLDOWN		=	0x80//(0!)          (16)
		VOL_VOLSLIDEUP		=	0x70;        //     (16)
		VOL_VOLSLIDEDOWN	=	0x60;          //   (16)
		VOL_PITCH_SLIDE_DOWN=	0x69,   //(10)
		VOL_PITCH_SLIDE_UP	=	0x73,   //(10)
		VOL_PAN_POSITION	=	0x80     // (64)
		VOL_PANNING			=	0xC0;             //    (16)
		VOL_VIBRATO_SPEED	=	0xA0; // Linked to Vibrato Sx = 4xy (16)
		VOL_VIBRATO			=	0xB0;     // Linked to Vibrato Vy = 4xy (16)
		VOL_PANSLIDELEFT	=	0xD0;           //  (16)
		VOL_PANSLIDERIGHT	=	0xE0;        //     (16)
		VOL_TONEPORTAMENTO	=	0xF0; // Linked to Porta2Note (16)
		};
	};
*/

	class Channel;

//////////////////////////////////////////////////////////////////////////
//  XMSampler::WaveDataController Declaration
	//\todo: WaveDateController Needs to update the speed if sampleRate change (but... would the samplerate change while
	//       there's a voice playing?)
	class WaveDataController
	{
	public:
		struct LoopDirection{ 
			enum {
				FORWARD = 0,
				BACKWARD
			};
		};

		void Init(XMInstrument::WaveData* wave, const int layer);

		void Work(float *pLeftw,float *pRightw,dsp::PRESAMPLERFN pResamplerWork)
		{
			//Process sample
			*pLeftw = pResamplerWork(
				pLeft() + m_Position.HighPart,
				m_Position.HighPart, m_Position.LowPart, Length());
			if (IsStereo())
			{
				*pRightw = pResamplerWork(
					pRight() + m_Position.HighPart,
					m_Position.HighPart, m_Position.LowPart, Length());
			}

			// Update Position
			//\todo : What about sample vibrato? Should we put it here or in voice? (there is the pitch envelope)
			if(CurrentLoopDirection() == LoopDirection::FORWARD){
				m_Position.QuadPart+=Speed();
			} else {
				m_Position.QuadPart-=Speed();
			}

			// Loop handler
			//\todo : Implement Sustain Loop Control
			const int curIntPos = m_Position.HighPart;
			switch(LoopType())
			{
			case XMInstrument::WaveData::LoopType::NORMAL:

				if(curIntPos >= LoopEnd())
				{
					Position(LoopStart()+(curIntPos%LoopEnd()));
				}
				break;
			case XMInstrument::WaveData::LoopType::BIDI:

				if(CurrentLoopDirection() == LoopDirection::FORWARD)
				{
					if(curIntPos  >= LoopEnd())
					{
						Position(LoopEnd()-(curIntPos%LoopEnd()));
						CurrentLoopDirection(LoopDirection::BACKWARD);
					} 
				} else {
					if(curIntPos <= LoopStart())
					{
						Position(LoopStart()+(curIntPos%LoopEnd()));
						CurrentLoopDirection(LoopDirection::FORWARD);
					} 
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
		};

		// Properties
		const int Layer() { return m_Layer;};
		XMInstrument::WaveData &Wave() { return *m_pWave; };

		bool Playing(){ return m_Playing;};
		void Playing(bool play){ m_Playing=play; };

		// Current sample position 
		const __int64  Position(){ return m_Position.HighPart;};
		void Position(const	__int64 value){ 
			if ( value < Length()) m_Position.HighPart = value;
			else m_Position.HighPart = Length()-1;
		};
		
		// Current sample Speed
		const __int64 Speed(){return m_Speed;};
		void Speed(const double value){m_Speed = value * 4294967296.0f;}; // 4294967296 is a left shift of 32bits

		void CurrentLoopDirection(const int dir){m_LoopDirection = dir;};
		const int CurrentLoopDirection(){return m_LoopDirection;};

		const int LoopType(){return m_pWave->WaveLoopType();};
		const int LoopStart(){return m_pWave->WaveLoopStart();};
		const int LoopEnd(){ return m_pWave->WaveLoopEnd();};

		const int SustainLoopType(){return m_pWave->WaveSusLoopType();};
		const int SustainLoopStart(){return m_pWave->WaveSusLoopStart();};
		const int SustainLoopEnd(){ return m_pWave->WaveSusLoopEnd();};

		const int Length(){return m_pWave->WaveLength();};

		const bool IsStereo(){ return m_pWave->IsWaveStereo();};

		// pointer to Start of Left sample
		const short* pLeft(){return m_pL;};
		// pointer to Start of Right sample
		const short* pRight(){return m_pR;};


	private:
		int m_Layer;
		XMInstrument::WaveData *m_pWave;
		ULARGE_INTEGER m_Position;
		double m_Speed;
		bool m_Playing;

		bool _stereo;
		int _length;

		int m_LoopDirection;
		short* m_pL;
		short* m_pR;
	};


//////////////////////////////////////////////////////////////////////////
//  XMSampler::EnvelopeController Declaration
	//\todo: Recall "CalcStep" after a SampleRate change, and also after a Tempo Change.
	class EnvelopeController {
	public:
		enum EnvelopeStage
		{
			OFF		= 0,
			DOSTEP	= 1, // normal operation, follow the steps.
			HASLOOP	= 2, // Indicates that the envelope *has* a (normal) loop (not that it is playing it)
			HASSUSTAIN	= 4, // This indicates that the envelope *has* a sustain (not that it is playing it)
			RELEASE = 8  // Indicates that a Note-Off has been issued.
		};
		// EnvelopeMode defines what the first value of a PointValue means
		// TICK = one tracker tick ( speed depends on the BPM )
		// MILIS = a millisecond. (independant of BPM).
/*		struct EnvelopeMode {
			enum{
				TICK=0,
				MILIS
			};
		};*/
		EnvelopeController(){;};
		~EnvelopeController(){;};

		void Init(XMInstrument::Envelope *pEnvelope = NULL);

//		const EnvelopeMode Mode() { return m_Mode; }
//		const Mode(EnvelopeMode _mode){ m_Mode=_mode; }

		void NoteOn();
		void NoteOff();

		void Work()
		{
			if(m_Stage&EnvelopeStage::DOSTEP)
			{
				if(m_Samples++ >= m_NextEventSample) // m_NextEventSample is updated inside CalcStep()
				{
					m_PositionIndex++;
					//  continue with the sustain loop.
					if (m_PositionIndex == m_pEnvelope->SustainEnd() )
					{
						if (m_Stage&EnvelopeStage::HASSUSTAIN)
						{
							// if the begin==end, pause the envelope.
							if ( m_pEnvelope->SustainBegin() == m_PositionIndex )
							{
								m_Stage = EnvelopeStage(m_Stage & ~EnvelopeStage::DOSTEP);
								return;
							}
							else m_PositionIndex = m_pEnvelope->SustainBegin();
						}
						else if (m_Stage&EnvelopeStage::HASLOOP && m_pEnvelope->LoopEnd() <= m_PositionIndex)
						{
							m_PositionIndex = m_pEnvelope->LoopStart();
							if ( m_pEnvelope->LoopStart() == m_pEnvelope->LoopEnd() )
							{
								m_Stage = EnvelopeStage(m_Stage & ~EnvelopeStage::DOSTEP);
								return;
							}
						}
					}
					else if( m_PositionIndex == m_pEnvelope->LoopEnd() && !(m_Stage&EnvelopeStage::HASSUSTAIN))
					{
						if ( m_pEnvelope->LoopStart() == m_PositionIndex )
						{
							m_Stage = EnvelopeStage(m_Stage & ~EnvelopeStage::DOSTEP);
							return;
						}
						else m_PositionIndex = m_pEnvelope->LoopStart();
					}
					else if( m_pEnvelope->GetTime(m_PositionIndex+1) == XMInstrument::Envelope::INVALID )
					{
						m_Stage = EnvelopeStage::OFF;
						m_PositionIndex = m_pEnvelope->NumOfPoints() - 1;
						m_ModulationAmount = m_pEnvelope->GetValue(m_PositionIndex);
						return;
					}
					m_Samples = m_pEnvelope->GetTime(m_PositionIndex) * SRateDeviation();
					m_ModulationAmount = m_pEnvelope->GetValue(m_PositionIndex);
					CalcStep(m_PositionIndex,m_PositionIndex + 1);
				}
				else 
				{
					m_ModulationAmount += m_Step;
				}
			}
		};

		/// 
		const XMInstrument::Envelope::ValueType ModulationAmount()
		{
			return m_ModulationAmount;
		};
		
		const EnvelopeStage Stage(){return m_Stage;};
		void Stage(const EnvelopeStage value){m_Stage = value;};
		XMInstrument::Envelope & Envelope(){return *m_pEnvelope;};
		inline void CalcStep(const int start,const int  end);
		void SetPosition(const int posi) { m_PositionIndex=posi-1; m_Samples= m_NextEventSample; }
	private:
		inline float SRateDeviation() { return m_sRateDeviation; };
		void RecalcDeviation();

		int m_Samples;
		float m_sRateDeviation;
		int m_Mode;
		int m_PositionIndex;
		int m_NextEventSample;
		EnvelopeStage m_Stage;

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
		};

		// Object Functions
		void Reset();
		void ResetEffects();

		void VoiceInit(int channelNum,int instrumentNum);
		void Work(int numSamples,float * pSampleL,float *pSamlpesR,dsp::Cubic& _resampler);

		void NewLine();

		void NoteOn(const compiler::uint8 note,const compiler::sint16 playvol=-1,bool reset=true);
		void NoteOff();
		void NoteOffFast();
		void NoteFadeout();

		void ResetVolAndPan(compiler::sint16 playvol);
		void UpdateSpeed();
		double PeriodToSpeed(int period);


		// Effect-Related Object Functions
		void PitchSlide()
		{
			m_Period += m_PitchSlideSpeed;
			UpdateSpeed();
		};
		void Slide2Note();
		void Vibrato();
		void Tremolo();
		void Panbrello();
		void Tremor();
		void VolumeSlide();
		void VolumeDown(const int value);
		void VolumeUp(const int value);
		void Retrig();

		/// Do Auto Vibrato
		void AutoVibrato();
		bool IsAutoVibrato() { return m_AutoVibratoAmount!=0; };
		/// Get Auto Vibrato Amount
		const double AutoVibratoAmount(){return m_AutoVibratoAmount;};


// Properties
		const int InstrumentNum(){ return _instrument;};
		void InstrumentNum(const int value){_instrument = value;};
		XMInstrument &rInstrument() { return *m_pInstrument;};///< Get Instrument Pointer
		void pInstrument(XMInstrument *p){m_pInstrument = p;};///< Set Instrument Pointer

		const int ChannelNum(){ return m_ChannelNum;};
		void ChannelNum(const int value){ m_ChannelNum = value;};
		void pChannel(XMSampler::Channel *p){m_pChannel = p;}; 
		XMSampler::Channel& rChannel(){return *m_pChannel;};

		void pSampler(XMSampler * const p){m_pSampler = p;};///< 
		XMSampler * const pSampler(){return m_pSampler;};///< 

		XMSampler::EnvelopeController& AmplitudeEnvelope(){return m_AmplitudeEnvelope;};
		XMSampler::EnvelopeController& FilterEnvelope(){return m_FilterEnvelope;};
		XMSampler::EnvelopeController& PitchEnvelope(){return m_PitchEnvelope;};
		XMSampler::EnvelopeController& PanEnvelope(){return m_PitchEnvelope;};

		WaveDataController& rWave(){return m_WaveDataController;};

		const bool IsPlaying(){ return m_bPlay;};
		void IsPlaying(const bool value){ m_bPlay = value;};

		const bool IsBackground() { return m_Background; };
		void IsBackground(const bool background){ m_Background = background; };

		// Volume of the current note.
		const compiler::uint16 Volume() { return m_Volume; };
		void Volume(const compiler::uint16 vol)
		{
			m_Volume = vol;
			m_RealVolume = rWave().Wave().WaveGlobVolume() * rInstrument().GlobVol() * (vol/256.0f);
		};
		// Voice.RealVolume() returns the calculated volume out of "WaveData.WaveGlobVol() * Instrument.Volume() * Voice.NoteVolume()"
		const float RealVolume() { return (!m_bTremorMute)?(m_RealVolume+m_TremoloAmount):0; };
		void PanFactor(float pan)
		{
			m_PanFactor = pan;
			m_PanRange = 1.0f -(fabs(0.5-m_PanFactor)*2);
		}
		float PanFactor() { return m_PanFactor; }

		const int CutOff() { return m_CutOff; };
		void CutOff(int co)	{	m_CutOff = co;	m_Filter._cutoff = co;	m_Filter.Update();	};
		
		const int Ressonance() { return m_Ressonance; };
		void Ressonance(int res) {	m_Ressonance = res; m_Filter._q = res; m_Filter.Update(); };

		void Period(int newperiod) { m_Period = newperiod; UpdateSpeed(); };
		int Period() { return m_Period; }

		double VibratoAmount() { return m_VibratoAmount; };

	protected:
		// Gets the delta between the points of the wavetables for tremolo/panbrello/vibrato
		int GetDelta(int wavetype,int wavepos);	
		float PanRange() { return m_PanRange; };
		const bool IsTremorMute(){return m_bTremorMute;};
		void IsTremorMute(const bool value){m_bTremorMute = value;};


	private:

		int m_ChannelNum;
		XMSampler::Channel* m_pChannel;
		XMSampler *m_pSampler;

		int _instrument;///< Instrument
		XMInstrument *m_pInstrument;

		// Todo: do we need 4 controllers? wouldn't it be better 1 controller for all 4 envelopes?
		EnvelopeController m_FilterEnvelope;
		EnvelopeController m_AmplitudeEnvelope;
		EnvelopeController m_PanEnvelope;
		EnvelopeController m_PitchEnvelope;

		WaveDataController m_WaveDataController;

		dsp::Filter m_Filter;
		int m_CutOff;
		int m_Ressonance;
		float _coModify;

		bool m_bPlay;
		bool m_Background;
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

		int m_VibratoSpeed;///< vibrato speed 
		int m_VibratoDepth;///< vibrato depth
		int m_VibratoPos;///< vibrato position 
		double m_VibratoAmount;///< vibrato amount

		int m_TremoloSpeed;
		int m_TremoloDepth;
		float m_TremoloAmount;
		int m_TremoloPos;

		// Panbrello
		int m_PanbrelloSpeed;
		int m_PanbrelloDepth;
		float m_PanbrelloAmount;
		int m_PanbrelloPos;

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
		int m_RetrigVol;
		int m_RetrigOperation;


		static const int m_SinTable[64];///< Sin
		static const int m_RampDownTable[64];///< RampDown
		static const int m_SquareTable[64];///< Square
		static const int m_RandomTable[64];///< Random
		static const int m_ft2VibratoTable[256];///< Vibrato Table
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
		};

		Channel()
		{
			Init();
		};

		void Init();
		void EffectInit();
		void Restore();

		/// Prepare the channel for the new effect (or execute if it's a one-shot one). This is executed on TrackerTick==0
		void SetEffect(Voice* voice,int cmd,int parameter);

		/// Executes the slide/change effects. This is executed on TrackerTick!=0
		void PerformFx();

		const int EffectFlags(){return m_EffectFlags;};
		void EffectFlags(const int value){m_EffectFlags = value;};

		/// convert note to period
		const double NoteToPeriod(const int note,const int layer);
		/// convert period to note 
		const int PeriodToNote(const double period,const int layer);


// Effect-Related Object Functions

		// Tick 0 commands
		void PanningSlide(int speed);
		void ChannelVolumeSlide(int speed);
		void PitchSlide(bool bUp,int speed,int note=255);
		void VolumeSlide(int speed);
		void Tremor(int parameter);
		void Vibrato(int depth,int speed = 0);
		void Tremolo(int depth,int speed);
		void Panbrello(int depth,int speed);
		void Arpeggio(const int param,const int layer);
		void Retrigger(const int ticks,const int volumeModifier);
		void NoteCut(const int ntick);

		// Tick n commands.
		void PanningSlide();
		void ChannelVolumeSlide();
		void NoteCut();

		const double ArpeggioPeriod()
		{
			const int arpi = m_pSampler->CurrentTick()%3;
			if(arpi >= 1){
				return m_ArpeggioPeriod[arpi - 1];
			} else {
				return m_Period;
			}
		};
/*		/// Init Global Volume Slide
		void GlobalVolumeSlide(const int speed)
		{
			if(speed != 0){
				if((speed & 0xf0) != 0){
					m_GlobalVolumeSlideSpeed = (float)(speed & 0xf0 >> 4) * 0.00390625f;
				} else {
					m_GlobalVolumeSlideSpeed = -(float)(speed & 0xf) * 0.00390625f;
				}
			}

			m_EffectFlags |= EffectFlag::GLOBALVOLSLIDE;
		};
		/// Do Global Volume Slide
		void GLobalVolumeSlide(){
			float a;
			m_pSampler->GetWireVolume(0,a);
			m_pSampler->SetWireVolume(0,a + m_GlobalVolumeSlideSpeed);
		};
*/



// Properties
		const int Index(){ return m_Index;};
		void Index(const int value){m_Index = value;};

		void pSampler(XMSampler * const pSampler){m_pSampler = pSampler;};

		const int InstrumentNo(){return m_InstrumentNo;};
		void InstrumentNo(const int no){m_InstrumentNo = no;};

		XMSampler::Voice* ForegroundVoice(){ return m_pForegroundVoice; }
		void ForegroundVoice(XMSampler::Voice* pVoice) { m_pForegroundVoice = pVoice; }

		const int Note(){ return m_Note;};
		void Note(const int note,const int layer)
		{	m_Note = note;
			m_Period = NoteToPeriod(note,layer);
		};
		const double Period(){return m_Period;};
		void Period(const double value){m_Period = value;};

		const float Volume(){return m_Volume;};
		void Volume(const float value){m_Volume = value;};
		const int DefaultVolume(){return m_ChannelDefVolume;};
		void DefaultVolume(const int value){m_ChannelDefVolume = value; Volume(value/64.0f);};

		//\todo: Reset PanFactor to DeFaultPanFactor when Stopping.
		const float PanFactor(){return 	m_PanFactor;};
		void PanFactor(const float value){
			m_PanFactor = value;
			if ( ForegroundVoice()) ForegroundVoice()->PanFactor(value);
		};
		const int DefaultPanFactor(){return m_DefaultPanFactor;};
		void DefaultPanFactor(const int value){m_DefaultPanFactor = value; PanFactor(value/64.0f); };

		const bool IsSurround(){ return m_bSurround;};
		void IsSurround(const bool value){
			if ( value )
			{
				/* if ( STANDARDSURROUND ) */ m_PanFactor = 0.5f;
			}
			m_bSurround = value;
		};

		const bool IsGrissando(){return m_bGrissando;};
		void IsGrissando(const bool value){m_bGrissando = value;};
		void VibratoType(const int value){	m_VibratoType = value;};
		const int VibratoType() {return m_VibratoType;};
		void TremoloType(const int type){m_TremoloType = type;};
		const int TremoloType(){return m_TremoloType;};
		void PanbrelloType(const int type){m_PanbrelloType = type;};
		const int PanbrelloType(){return m_PanbrelloType;};

		const bool IsArpeggio() { return ((m_EffectFlags & EffectFlag::ARPEGGIO) != 0); };
		const bool IsVibrato(){return (m_EffectFlags & EffectFlag::VIBRATO) != 0;};
		void VibratoAmount(const double value){m_VibratoAmount = value;};
		const double VibratoAmount(){return m_VibratoAmount;};

		void Slide2NoteDestNote(const int note) { m_Slide2NoteDestNote = note; };
		const int Slide2NoteDestNote() { return m_Slide2NoteDestNote; };

	private:


		int m_Index;// Channel Index.
		XMSampler *m_pSampler;
		int m_InstrumentNo;///< ( 0 .. 255 )
		XMSampler::Voice* m_pForegroundVoice;

		int m_Note;
		double m_Period;

		float m_Volume;///<  (0 - 1.0f)
		int m_ChannelDefVolume;///< (0.0f - 64)

		float m_PanFactor;// value used for Playback
		int m_DefaultPanFactor; // value used for Storage //  0..64 .  80 == Surround. 127 = Mute.
		bool m_bSurround;

		bool m_bGrissando;
		int m_VibratoType;///< vibrato type 
		int m_TremoloType;
		int m_PanbrelloType;


		int m_EffectFlags;

		int	m_PitchSlideSpeed;
		int m_Slide2NoteDestNote;

		/// Global Volume Slide Speed
//		float m_GlobalVolumeSlideSpeed;
		float m_ChanVolSlideSpeed;
		float m_PanSlideSpeed;

		int m_VibratoSpeed;///< vibrato speed 
		int m_VibratoDepth;///< vibrato depth
		int m_VibratoPos;///< vibrato position 
		double m_VibratoAmount;///< vibrato amount
		double m_AutoVibratoAmount;///< vibrato amount

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
		int m_ArpeggioMem;

	};



	//////////////////////////////////////////////////////////////////////////
	//  XMSampler Declaration

	XMSampler(int index);

	virtual void Init(void);
	
	//This tick is "NewLine()". The API needs to be renamed.
	void Tick();
	/// numSamples
	virtual void Work(int numSamples);
	virtual void Stop(void);
	virtual void Tick(int channel, PatternEntry* pData);
	virtual TCHAR* GetName(void) { return _psName; };

	virtual bool Load(RiffFile& riffFile);
	virtual bool LoadSpecificFileChunk(RiffFile& riffFile, int version);
	virtual void SaveSpecificChunk(RiffFile& riffFile);

	void Update(void);

	void BPM (const int value){m_BPM = value;};///< Beat Per Minutes
	const int BPM (){return m_BPM;};///< Beat Per Minutes
	
	// MOD Tick 
	void TicksPerRow(const int value){m_TicksPerRow = value;};///< Get 1s 
	const int TicksPerRow(){ return m_TicksPerRow;};///< Set 1s 

	/// BPM Speed
	void CalcBPMAndTick();

	XMSampler::Channel& rChannel(const int index){ return m_Channel[index];};///< Channel 
	Voice& rVoice(const int index) { return m_Voices[index];};///< 

	XMInstrument & rInstrument(const int index){return m_Instruments[index];};
	XMInstrument::WaveData & SampleData(const int index){return m_rWaveLayer[index];};
	
	const bool IsLinearFreq(){ return m_bLinearFreq;};
	void IsLinearFreq(const bool value){ m_bLinearFreq = value;};

	/// set current voice number
	const int NumVoices(){ return _numVoices;};
	/// get current voice number
	void NumVoices(const int value){_numVoices = value;};

	/// set resampler quality 
	void ResamplerQuality(const dsp::ResamplerQuality value){
		_resampler._quality = value;
	}

	const dsp::ResamplerQuality ResamplerQuality(){
		return _resampler._quality;
	}

	boost::recursive_mutex & Mutex() {  return m_Mutex; };

	const int SampleCounter(){return _sampleCounter;};// Sample pos since last linechange.
	void SampleCounter(const int value){_sampleCounter = value;};// ""

	void NextSampleTick(const int value){m_NextSampleTick = value;};// Sample Pos of the next (tracker) tick
	const int NextSampleTick(){ return m_NextSampleTick;};// ""

	void CurrentTick(const int value){m_TickCount = value;};// Current Tracker Tick number
	const int CurrentTick(){ return m_TickCount;};// ""

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

protected:

	static TCHAR* _psName;
	int _numVoices;

	Voice m_Voices[MAX_POLYPHONY];
	XMSampler::Channel m_Channel[MAX_TRACKS];
	dsp::Cubic _resampler;
	
	void DeltaTick(const int value){m_DeltaTick = value;};
	const int DeltaTick(){return m_DeltaTick;};
	void WorkB(int numsamples);
private:

	int m_BPM;
	int m_TicksPerRow;	// Tracker Ticks. Also called "speed".
	int m_DeltaTick;	// Samples for a tracker tick.
//	int m_TickCount;	// Tracker Tick count inside current row.
	bool m_bLinearFreq; // Using Linear or Amiga Slides.

	int _sampleCounter;///< Number of Samples since note start
	int m_NextSampleTick;///< The sample position of the next Tracker Tick
	int m_TickCount;// Current Tick number.


	//\todo : This should not be independant for each xmsampler, but shared.
	XMInstrument m_Instruments[MAX_INSTRUMENT+1];
	XMInstrument::WaveData m_rWaveLayer[MAX_INSTRUMENT+1];
	boost::recursive_mutex m_Mutex;
};
}
}
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
//	static const int POINT = 16;///< envelope point
	static const int MAX_POLYPHONY = 64;///< max polyphony 
	static const int MAX_INSTRUMENT = 255;///< max instrument
	static const int LINEAR_FREQUECY = 1;///< Liner
	static const UINT VERSION = 0x00008000;
	/// \todo Sampling rate can't be a constant!
	static const int TICK_PER_WAVE = 44100  /* samples/sec */ * 60 /* sec */  / (4 /*row */ * 6 /* tick  */ /* * BPM */ ); 

	struct CMD {
	//	(*) = If the command byte is zero, the last nonzero byte for the command should be used.

		// [sartorius & bohan] was: static const compiler::uint8 ARPEGGIO		=		0x0; ///< Arpeggio
		static const compiler::uint8 NONE = 0;
		static const compiler::uint8 PORTAUP		=		0x01;///< Portamento Up		 (*)
		static const compiler::uint8 PORTADOWN	=		0x02;///< Portamento Down	 (*)
		static const compiler::uint8 PORTA2NOTE	=		0x03;///<  Tone Portamento	 (*)
		static const compiler::uint8 VIBRATO =			0x04;///< Do Vibrato		 (*)
		static const compiler::uint8 TONEPORTAVOL =		0x05;///< Tone Portament & Volume Slide (*)
		static const compiler::uint8 VIBRATOVOL =			0x06;///< Vibrato & Volume Slide (*)
		static const compiler::uint8 TREMOLO =			0x07;///< Tremolo			 (*)
		static const compiler::uint8 PANNING	=			0x08;///< Set Panning Position
		static const compiler::uint8 OFFSET	=			0x09;///< Set Sample Offset
		static const compiler::uint8 VOLUMESLIDE	=		0x0a;///< Volume Slide		 (*)
		static const compiler::uint8 POSITION_JUMP	=	0x0b;///< Position Jump
		static const compiler::uint8 VOLUME	=			0x0c;///< Set Volume
		static const compiler::uint8 PATTERN_BREAK	=	0x0d;///< Pattern Break
		static const compiler::uint8 EXTENDED		=		0x0e;///< Extend Command
		static const compiler::uint8 SETSPEED	=			0x0f;///< Set Speed or BPM
		static const compiler::uint8 SET_GLOBAL_VOLUME =	0x10;///< Set Global Volume
		static const compiler::uint8 GLOBAL_VOLUME_SLIDE = 0x11;///< Global Volume Slide (*)
		static const compiler::uint8 NOTE_OFF			 =	0x14;///< Note Off
		static const compiler::uint8 SET_ENV_POSITION =	0x15;///< Set Envelope Position
		static const compiler::uint8 PANNINGSLIDE =		0x19;///< PANNING SLIDE		 (*)
		static const compiler::uint8 RETRIG	=			0x1B;///< Retrigger Note	 (*)
		static const compiler::uint8 TREMOR	=			0x1D;///< Tremor
		static const compiler::uint8 EXTEND_XM_EFFECTS =	0x21;///< Extend XM Effects	
		static const compiler::uint8 PANBRELLO	=		0x22;///< Panbrello

/*  Extended Command
*
E1 (*) Fine porta up
E2 (*) Fine porta down
E3     Set gliss control
E4     Set vibrato control
E5     Set finetune
E6     Set loop begin/loop
E7     Set tremolo control
E9     Retrig note
EA (*) Fine volume slide up
EB (*) Fine volume slide down
EC     Note cut
ED     Note delay
EE     Pattern delay
 */

/* Extended XM Command
*
X1 (*) Extra fine porta up
X2 (*) Extra fine porta down
 */		// Extend Command

		/// Volume Column commands
 
		static const compiler::uint8 VOLSLIDEDOWN =		0x40;
		static const compiler::uint8 VOLSLIDEUP	=		0x41;
		static const compiler::uint8 FINEVOLDOWN	=		0x42;
		static const compiler::uint8 FINEVOLUP	=		0x43;
//		static const compiler::uint8 VIBRATO		= 0x1B;
		static const compiler::uint8 EXFINEPORTAMENT =	0x44;
		static const compiler::uint8 PANSLIDELEFT =		0x45;
		static const compiler::uint8 PANSLIDERIGHT = 0x46;
//		static const compiler::uint8 TONEPORTAMENTO = 0x1F;
		
		static const compiler::uint8 ARPEGGIO				= 0x47;
		static const compiler::uint8 GRISSANDO			= 0x48;
		static const compiler::uint8 TREMOLO_TYPE			= 0x49;
		static const compiler::uint8 FINE_PORTAMENTO_DOWN = 0x4A;
		static const compiler::uint8 FINE_PORTAMENTO_UP	= 0x4B;
		
		static const compiler::uint8 VIBRATO_TYPE = 0x4C;
		static const compiler::uint8 VIBRATO_SPEED = 0x4D;
		static const compiler::uint8 VIBRATO_DEPTH = 0x4E;
		static const compiler::uint8 VIBRATO2 = 0x4F;
		
		static const compiler::uint8 NOTE_CUT	= 0x50;

		static const compiler::uint8 EXTRA_FINE_PORTAMENTO_DOWN = 0x51;
		static const compiler::uint8 EXTRA_FINE_PORTAMENTO_UP	= 0x52;

		static const compiler::uint8 PANBRELLO_WAVE_FORM = 0x53;
		static const compiler::uint8 FINE_PATTERN_DELAY = 0x54;

		static const compiler::uint8 SURROUND_OFF = 0x55;
		static const compiler::uint8 SURROUND_ON = 0x56;

		static const compiler::uint8 REVERB_OFF = 0x57;
		static const compiler::uint8 REVERB_ON = 0x58;

		static const compiler::uint8 CENTER_SURROUND = 0x59;
		static const compiler::uint8 QUOAD_SURROUND  = 0x5A;

		static const compiler::uint8 GROBAL_FILTERS = 0x5B;
		static const compiler::uint8 LOCAL_FILTERS = 0x5C;

		static const compiler::uint8 PLAY_FORWARD = 0x5D;
		static const compiler::uint8 PLAY_BACKWARD = 0x5E;

		static const compiler::uint8 SET_HIGH_OFFSET = 0x5F;

		static const compiler::uint8 PATTERN_DELAY = 0x60;
		static const compiler::uint8 PATTERN_LOOP = 0x61;

		static const compiler::uint8 SET_FINE_TUNE = 0x62;
		static const compiler::uint8 SET_PANNING = 0x63;
		static const compiler::uint8 RETRIGGER_NOTE = 0x64;
		static const compiler::uint8 FINE_VOLSLIDE_UP = 0x65;
		static const compiler::uint8 FINE_VOLSLIDE_DOWN = 0x66;
		static const compiler::uint8 NOTE_DELAY = 0x67;
		static const compiler::uint8 SET_ACTIVE_MACRO = 0x69;

		static const compiler::uint8 EXT_NOTEOFF	=	0xc0;
		static const compiler::uint8 EXT_NOTEDELAY	= 0xd0;
		

	};


	class Channel;

	//////////////////////////////////////////////////////////////////////////
	//  XMSampler::WaveDataController Declaration
	class WaveDataController
	{
	public:
		struct LoopDirection{ 
			enum {
				FORWARD = 0,
				BACKWARD
			};
		};
		const short* pLeft(){return m_pL;};
		void pLeft(short * const value){m_pL = value;};
		
		const short* pRight(){return m_pR;};
		void pRight(short * const value){m_pR = value;};
		
		const bool IsStereo(){ return _stereo;};
		void IsStereo(const bool value){ _stereo = value;};
		
		const __int64  Position(){ return _pos;};
		void Position(const __int64 value){ _pos = value;};
		
		const double Speed(){return _speed;};
		void Speed(const double value){_speed = value;};

		const int LoopType(){return m_LoopType;};
		void LoopType(const int value){m_LoopType = value;};

		void CurrentLoopDirection(const int dir){m_LoopDirection = dir;};
		const int CurrentLoopDirection(){return m_LoopDirection;};

		const int LoopStart(){return _loopStart;};
		void LoopStart(const int value){_loopStart = value;};

		const int LoopEnd(){ return _loopEnd;};
		void  LoopEnd(const int value){ _loopEnd = value;};

		const int Length(){return _length;};
		void Length(const int value){_length = value;};

		const float Volume(){ return _vol;};
		void Volume(const float value){ _vol = value;};

		const float LVolumeDest() { return _lVolDest;};
		void LVolumeDest(const float value) { _lVolDest = value;};

		const float RVolumeDest() { return _rVolDest;};
		void RVolumeDest(const float value) { _rVolDest = value;};

		const float LVolumeCurr() { return _lVolCurr;};
		void LVolumeCurr(const float value) { _lVolCurr = value;};

		const float RVolumeCurr() { return _rVolCurr;};
		void RVolumeCurr(const float value) { _rVolCurr = value;};

		void Init(XMSampler *samp,int iInstIdx,Channel& channel,const int note);
		
		const int Layer() { return m_Layer;};
		void Layer(const int value){m_Layer = value;};


	private:
		short* m_pL;
		short* m_pR;
		bool _stereo;
		__int64 _pos;
		double _speed;
		bool _loop;
		int _loopStart;
		int _loopEnd;
		int _length;
		float _vol;
		float _lVolDest;
		float _rVolDest;
		float _lVolCurr;
		float _rVolCurr;

		int m_LoopType;
		int m_LoopDirection;
		int m_Layer;


	};
	//////////////////////////////////////////////////////////////////////////
	//  XMSampler::EnvelopeController Declaration

	class EnvelopeController {
	public:
		enum EnvelopeStage
		{
			OFF,
			DOSTEP,
			LOOP,
			SUSTAIN,
			RELEASE,
			END
		};

		EnvelopeController(){;};
		~EnvelopeController(){;};

		void Init(XMInstrument::Envelope *pEnvelope = NULL){
			if(pEnvelope != NULL){
				m_pEnvelope = pEnvelope;
			}
			m_Tick = 0;	
			m_PositionIndex = 0;
			m_ModulationAmount = 0;
			m_Step = 0;
			m_Stage = EnvelopeStage::OFF;
		};

		/// NoteOn EnvelopeStage
		void NoteOn(){
			m_Tick = 0;
			m_PositionIndex = 0;
			m_Stage = EnvelopeStage::DOSTEP;
			m_ModulationAmount = m_pEnvelope->GetValue(0);
			
			if(m_pEnvelope->SustainBegin() == 0)
			{
				m_Stage = EnvelopeStage::SUSTAIN;
				CalcStep(0,1);
				return;
			}

			if(m_pEnvelope->GetTime(1) == - 1){
				m_Stage = EnvelopeStage::END;
			} else {
				CalcStep(0,1);
			}
		};

		/// NoteOff EnvelopeStage
		void NoteOff()
		{
			if(m_Stage == EnvelopeStage::SUSTAIN || m_Stage == EnvelopeStage::DOSTEP){
				m_Stage = EnvelopeStage::RELEASE;
				CalcStep(m_PositionIndex,m_PositionIndex + 1);
			}

		};

		void Work()
		{
			if((m_Stage == EnvelopeStage::DOSTEP)|| (m_Stage == EnvelopeStage::RELEASE)){
				m_ModulationAmount += m_Step;

			} else {
				return;
			}

			m_Tick++;// 1Tick  1/44100sec

			if((m_Tick >= m_pEnvelope->GetTime(m_PositionIndex + 1)))
			{
				m_PositionIndex++;

				if(m_pEnvelope->LoopEnd() == m_PositionIndex){
					m_PositionIndex = m_pEnvelope->LoopStart();
					m_Tick = m_pEnvelope->GetTime(m_PositionIndex);
					m_ModulationAmount = m_pEnvelope->GetValue(m_PositionIndex);
					if(m_pEnvelope->LoopStart() != m_pEnvelope->LoopEnd()){
						CalcStep(m_PositionIndex,m_PositionIndex + 1);
					} else {
						m_Step = 0;
						m_Stage = EnvelopeStage::END;
					}

					return;
				}


				if(m_PositionIndex == m_pEnvelope->SustainBegin() && m_Stage == EnvelopeStage::DOSTEP){
					m_Stage = EnvelopeStage::SUSTAIN;
				}

				if((m_PositionIndex >= (m_pEnvelope->NumOfPoints() - 1)) || (m_PositionIndex > 15) || (m_pEnvelope->GetTime(m_PositionIndex) == -1)){
					m_Stage = EnvelopeStage::END;
					m_ModulationAmount = m_pEnvelope->GetValue(m_pEnvelope->NumOfPoints() - 1);
				} else {
					m_ModulationAmount = m_pEnvelope->GetValue(m_PositionIndex);
					CalcStep(m_PositionIndex,m_PositionIndex + 1);
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
	private:
		inline void CalcStep(const int start,const int  end);

		int m_Tick;
		int m_PositionIndex;
		EnvelopeStage m_Stage;

		XMInstrument::Envelope * m_pEnvelope;

		XMInstrument::Envelope::ValueType m_ModulationAmount;
		XMInstrument::Envelope::ValueType m_Step;
	};// EnvelopeController
	
	class Voice;

	//////////////////////////////////////////////////////////////////////////
	//  XMSampler::Channel Declaration
	class Channel {
	public:
		struct EffectFlag
		{
			static const int VIBRATO =			0x00000001;///< 
			static const int PORTAUP =			0x00000002;///< 
			static const int PORTADOWN =		0x00000004;///< 
			static const int PORTA2NOTE =		0x00000008;///< 
			static const int VOLUMESLIDE =		0x00000010;///< 
			static const int PANSLIDE =			0x00000020;///< 
			static const int TREMOLO =			0x00000040;///< 
			static const int ARPEGGIO =			0x00000080;///< 
			static const int NOTECUT =			0x00000100;///< 
			static const int PANBRELLO =		0x00000200;///<	Panbrello
			static const int GLOBALVOLSLIDE	=   0x00000400;///< GLobal Volume Slide
			static const int TREMOR			=	0x00000800;///< Tremor

		};

		Channel()
		{
			Init();
		};

		void Init(){
			m_InstrumentNo = 255;// 
			m_Volume = 0;
			m_ChannelVolume = 128;//
			m_Note = 255;// 
			m_PortamentoSpeed = 0;
			m_Porta2NoteDestPeriod= 0;
			m_FinePortamentoValue  = 0;
			m_bGrissando = false;
	
			m_EffectFlags = 0;
			m_VibratoSpeed = 0;
			m_VibratoDepth = 0;
			m_VibratoType = 0;// default: Sine Table
			m_VibratoPos = 0;
			
			m_TremoloSpeed = 0;
			m_TremoloDepth = 0;
			m_TremoloDelta = 0;
			m_TremoloPos = 0;
			m_TremoloType = 0;
			m_TremoloPos = 0;

			panFactor = 0.5f;
			m_PanSlideSpeed = 0.0f;
			m_bUp = false;
			m_VolumeSlideSpeed = 0;
			m_bLeft = false;
			m_Porta2NoteDestNote = 0;
			m_ChannelVolume = 0;
			m_Period = 0;
			m_bPeriodChange = false;
			m_Index = 0;
			m_VibratoAmount = 0;
			m_AutoVibratoAmount = 0;
			m_Note = 0;

			m_ArpeggioPeriod[0] = 0.0;
			m_ArpeggioPeriod[1] = 0.0;
			m_ArpeggioIndex = 0;

			m_NoteCutCounter = 0;

			m_PanbrelloSpeed = 0;
			m_PanbrelloDepth = 0;
			m_PanbrelloDelta = 0;
			m_PanbrelloPos = 0;
			m_PanbrelloType = 0;
			m_GlobalVolumeSlideSpeed = 0.0f;

			// Extra Fine Portamento Speed
			m_ExtraFinePortamentoSpeed = 0.0;

			// Tremor
			m_TremorOnTime = 0;
			m_TremorOffTime = 0;
			m_TremorCount = 0;
			m_bTremorMute = false;

			m_bSurround = false;

		};

		void EffectInit()
		{
			m_FinePortamentoValue = 0;
			m_VibratoPos = 0;
			m_TremoloPos = 0;
			m_TremoloDepth = 0;
			m_VibratoAmount = 0;
			m_AutoVibratoAmount = 0;
			m_PanbrelloPos = 0;
		
		};

		void pSampler(XMSampler * const pSampler){m_pSampler = pSampler;};

		const int InstrumentNo(){return m_InstrumentNo;};
		void InstrumentNo(const int no){m_InstrumentNo = no;};
		
		const int Index(){ return m_Index;};
		void Index(const int value){m_Index = value;};

		const float ChannelVolume(){return m_ChannelVolume;};
		void ChannelVolume(const float value){m_ChannelVolume = value;};
		
		const int Volume(){return m_Volume;};
		void Volume(const int value){m_Volume = value;};
		
		const int Note(){ return m_Note;};
		void Note(const int note,const int layer)
		{	m_Note = note;
			m_Period = NoteToPeriod(note,layer);
			m_bPeriodChange = true;
		};

		/// Note Cut command initialize
		void NoteCut(const int ntick){
			m_NoteCutCounter = ntick;
			if(ntick == 0){
				Volume(0);
				m_bVolumeChange = true;
				return;
			}
			m_EffectFlags |= EffectFlag::NOTECUT;
		};

		///  Do Note Cut
		void NoteCut()
		{
			m_NoteCutCounter--;
			if(m_NoteCutCounter == 0)
			{
				Volume(0);
				m_bVolumeChange = true;
				m_EffectFlags &= ~EffectFlag::NOTECUT;
			}
		};

		/// Perform effect at first tick
		void PerformEffect(Voice& voice,const int cmd,const int parameter);

		// Channel Command
		void VolumeDown(const int value){
			m_Volume -= value;
			if(m_Volume < 0){
				m_Volume = 0;
			}
		};

		void VolumeUp(const int value){
			m_Volume += value;
			if(m_Volume > 255){
				m_Volume = 255;
			}
		};

		void Porta2NoteDestPeriod(const double speed)
		{m_Porta2NoteDestPeriod = speed;};
		inline void Porta2NoteDestPeriod(const int note,const int layer);///
		const double Porta2NoteDestPeriod(){return m_Porta2NoteDestPeriod;};///

		void Porta2Note(const compiler::uint8 note,const int parameter,const int layer);
		void Porta2Note(XMSampler::Voice& voice);

		/// convert note to period
		const double NoteToPeriod(const int note,const int layer);
		/// convert period to note 
		const int PeriodToNote(const double period,const int layer);

		/// Get period
		const double Period(){return m_Period;};
		/// Set period
		void Period(const double value){m_Period = value;};


		void PortamentoUp(const int speed)
		{ 
			m_EffectFlags |= EffectFlag::PORTAUP;
			if(speed != 0){
				m_PortamentoSpeed = speed;
			}
		};

		void PortamentoUp()
		{
			m_Period -= m_PortamentoSpeed;
			m_bPeriodChange = true;
		};
		
		void PortamentoDown(const int speed)
		{	m_EffectFlags |= EffectFlag::PORTADOWN;
			if(speed != 0){
				m_PortamentoSpeed = speed;
			}
		};

		void PortamentoDown()
		{
			m_Period += m_PortamentoSpeed;
			
			m_bPeriodChange = true;

		};

		void FinePortamentoUp(const int value)
		{
			if(value != 0)
			{m_FinePortamentoValue = value;};

			m_Period -= m_FinePortamentoValue;
			m_bPeriodChange = true;

		};

		void FinePortamentoDown(const int value)
		{
			if(value != 0)
			{m_FinePortamentoValue = value;};
			m_Period += m_FinePortamentoValue;

			m_bPeriodChange = true;

		};

		const int EffectFlags(){return m_EffectFlags;};
		void EffectFlags(const int value){m_EffectFlags = value;};

		const bool IsGrissando(){return m_bGrissando;};
		void IsGrissando(const bool value){m_bGrissando = value;};
		const bool IsVibrato(){return (m_EffectFlags & EffectFlag::VIBRATO) > 0;};
		void IsVibrato(const bool value)
		{
			if(value){
				m_EffectFlags |= EffectFlag::VIBRATO;
			} else {
				m_EffectFlags &= !EffectFlag::VIBRATO;
			}
		};

		inline void Vibrato(const int depth,const int speed = 0);
		inline void Vibrato();

		void VibratoSpeed(const int value){	m_VibratoSpeed = value;};///
		const int VibratoSpeed() {return m_VibratoSpeed;};///

		void VibratoDepth(const int value){	m_VibratoDepth = value;};///
		const int VibratoDepth() {return m_VibratoDepth;};///

		void AutoVibratoAmount(const double value){m_AutoVibratoAmount = value;};
		const double AutoVibratoAmount(){return m_AutoVibratoAmount;};
		
		void VibratoAmount(const double value){m_VibratoAmount = value;};
		const double VibratoAmount(){return m_VibratoAmount;};


		void VibratoType(const int value){	m_VibratoType = value;};
		const int VibratoType() {return m_VibratoType;};

		void Tremolo(const int depth,const int speed)
		{
			if(depth) m_TremoloDepth = depth;
			if(speed) m_TremoloSpeed = speed;
			m_EffectFlags |= EffectFlag::TREMOLO;
		};

		void TremoloType(const int type){m_TremoloType = type;};
		const int TremoloType(){return m_TremoloType;};

		inline void Tremolo();
		const float TremoloDelta(){return m_TremoloDelta;};
		
		void VolumeSlide(const int speed,const bool bUp)
		{
			if(speed > 0){
				m_VolumeSlideSpeed = speed;
				m_bUp = bUp;
			}
			m_EffectFlags |= EffectFlag::VOLUMESLIDE;
		};
		
		void VolumeSlide()
		{
			if(m_bUp){
				VolumeUp(m_VolumeSlideSpeed);
			} else {
				VolumeDown(m_VolumeSlideSpeed);
			}
			m_bVolumeChange = true;
		};

		void PanningSlide(const int speed,const bool bLeft)
		{
			if(speed){
				m_PanSlideSpeed = (float)speed / 256.0f;
				m_bLeft = bLeft;
			}
			m_EffectFlags |= EffectFlag::PANSLIDE;
		};

		void PanningSlide(){
			if(m_bLeft){
				panFactor -= m_PanSlideSpeed;
				if(panFactor < 0.0f)
					panFactor = 0.0f;
			} else {
				panFactor += m_PanSlideSpeed;
				if(panFactor > 1.0f)
					panFactor = 1.0f;

			}
		};

		void PanbrelloType(const int type){m_PanbrelloType = type;};
		const int PanbrelloType(){return m_PanbrelloType;};

		/// Panbrello(Tick)
		void Panbrello(const int depth,const int speed)
		{
			if(depth) m_PanbrelloDepth = depth;
			if(speed) m_PanbrelloSpeed = speed;
			m_EffectFlags |= EffectFlag::PANBRELLO;
		};

		/// Panbrello(Work)
		inline void Panbrello();
		/// Panbrello
		const float PanbrelloDelta(){return m_PanbrelloDelta;};

		const float PanFactor(){return 	panFactor;};///< Pan Factor
		void PanFactor(const float value){panFactor = value;};///< Pan Factor

		void Porta2NoteDestNote(const int note){m_Porta2NoteDestNote = note;};
		const int Porta2NoteDestNote(){return m_Porta2NoteDestNote;};

		/// Arpeggio (tick)
		void Arpeggio(const int param,const int layer)
		{
			m_ArpeggioPeriod[0] = NoteToPeriod(Note() + param & 0xf,layer);
			m_ArpeggioPeriod[1] = NoteToPeriod(Note() + ((param & 0xf0) >> 4),layer);
			m_ArpeggioIndex = 0;
			m_EffectFlags |= EffectFlag::ARPEGGIO;
		};

		const bool IsArpeggio()
		{ 
			return ((m_EffectFlags & EffectFlag::ARPEGGIO) != 0);
		};
		
		const double ArpeggioPeriod()
		{  
			if(m_ArpeggioIndex >= 1){
				return m_ArpeggioPeriod[m_ArpeggioIndex - 1];
			} else {
				return m_Period;
			}
		};

		/// Arpeggio (work)
		void Arpeggio(){
			m_ArpeggioIndex++;
			if(m_ArpeggioIndex > 2){
				m_ArpeggioIndex = 0;
			}
			m_bPeriodChange = true;
		};
		
		
		/// Init Global Volume Slide
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

		/// Tremor 
		void Tremor(const int parameter)
		{
			if(parameter != 0){
				m_TremorOnTime = ((parameter >> 4) & 0xF) + 1;
				m_TremorOffTime = (parameter & 0xF) + 1;
			}
			m_EffectFlags |= EffectFlag::TREMOR;
			m_TremorCount = 0;
			m_bTremorMute = false;
		};

		/// Tremor 
		void Tremor()
		{
			
			if(	m_TremorCount >= m_TremorOnTime ){
				m_bTremorMute = true ;
				m_bVolumeChange = true;
			}
			
			m_TremorCount++ ;

			if( m_TremorCount >= m_TremorOnTime + m_TremorOffTime )
			{
				m_bTremorMute = false;
				m_bVolumeChange = true;
				m_TremorCount = 0 ;
			}
		};

		const bool IsTremorMute(){return m_bTremorMute;};
		void IsTremorMute(const bool value){m_bTremorMute = value;};

		const bool IsSurround(){ return m_bSurround;};
		void IsSurround(const bool value){ m_bSurround = value;};
		void SurroundOn()
		{
			panFactor = 0.5f;
			m_bVolumeChange = true;
			m_bSurround = true;
		};


		const bool IsPeriodChange(){return m_bPeriodChange;};
		void IsPeriodChange(const bool value){m_bPeriodChange = value;};
		
		const bool IsVolumeChange(){return m_bVolumeChange;};
		void IsVolumeChange(const bool value){m_bVolumeChange = value;};


	private:

		int m_InstrumentNo;///< ( 1 - 255 )
		int m_Volume;///<  (0 - 100)
		float m_ChannelVolume;///< (0.0f - 1.0f)
		
		int m_Note;///< 
		double m_Period;///< 
		bool m_bPeriodChange;
		
		int m_EffectFlags;

		int	m_PortamentoSpeed;

		int m_Porta2NoteDestNote;
		double m_Porta2NoteDestPeriod;

		int m_FinePortamentoValue;
		XMSampler *m_pSampler;
		bool m_bGrissando;

		int m_VibratoSpeed;///< vibrato speed 
		int m_VibratoDepth;///< vibrato depth
		int m_VibratoType;///< vibrato type 
		int m_VibratoPos;///< vibrato position 
		double m_VibratoAmount;///< vibrato amount
		double m_AutoVibratoAmount;///< vibrato amount

		int m_Index;///< 

		// 
		int m_TremoloSpeed;
		int m_TremoloDepth;
		float m_TremoloDelta;
		int m_TremoloPos;
		int m_TremoloType;

		// Panbrello
		int m_PanbrelloSpeed;
		int m_PanbrelloDepth;
		float m_PanbrelloDelta;
		int m_PanbrelloPos;
		int m_PanbrelloType;

		///
		float m_PanSlideSpeed;
		bool m_bLeft;

		///
		int m_VolumeSlideSpeed;
		///
		bool m_bUp;

		/// Arpeggio  
		double m_ArpeggioPeriod[2];
		int	m_ArpeggioIndex;
		
		bool m_bVolumeChange;
		
		/// Global Volume Slide Speed
		float m_GlobalVolumeSlideSpeed;

		/// Extra Fine Portamento Speed
		double m_ExtraFinePortamentoSpeed;
		
		/// Tremor 
		int m_TremorOnTime;
		int m_TremorOffTime;
		int m_TremorCount;
		bool m_bTremorMute;

		float panFactor;///< 

		// Note Cut Command 
		//	Channel volume is 0 after n ticks
		int m_NoteCutCounter;

		bool m_bSurround;

		// 
		static const int m_SinTable[64];///< Sin
		static const int m_RampDownTable[64];///< RampDown
		static const int m_SquareTable[64];///< Square
		static const int m_RandomTable[64];///< Random
	};


	//////////////////////////////////////////////////////////////////////////
	//  XMSampler::Voice Declaration
	class Voice
	{
	public:
		/// 
		Voice(){
			Init();
		};

		void Init(){

			m_AmplitudeEnvelope.Init();
			m_FilterEnvelope.Init();
			m_PitchEnvelope.Init();
			m_PanEnvelope.Init();

			_filter.Init();
			_cutoff = 0;
			_tickCounter = 0;
			_triggerNoteOff = 0;
			_triggerNoteDelay = 0;
			
			m_channel = -1;
			m_WaveDataController.LVolumeCurr(0.0f);
			m_WaveDataController.RVolumeCurr(0);
			m_WaveDataController.Volume(0.0f);

			effCmd = CMD::NONE;

			m_AutoVibratoAmount = 1.0;
			m_AutoVibratoPos = 0;
			m_AutoVibratoDepth = 0; 

			m_pChannel = NULL;
			m_bPlay = false;
			m_TickRemain = 0;

		};
		
		void VoiceInit(){
			_filter.Init();
			m_AutoVibratoAmount = 1.0;
			m_AutoVibratoPos = 0;
			m_AutoVibratoDepth = 0; 
			_triggerNoteOff = 0;
			_triggerNoteDelay = 0;
			m_bPlay = false;
		};

		void VoiceInit(const compiler::uint8 note);///< 
		void ClearEffectFlags() {m_EffectFlags = 0;};///< 
		
		const int TickCounter(){return _tickCounter;};///< Tick(Sample) 
		void TickCounter(const int value){_tickCounter = value;};///< Tick(Sample) 

		const int TriggerNoteOff(){return _triggerNoteOff;};
		void TriggerNoteOff(const int value){_triggerNoteOff = value;};
		
		const int TriggerNoteDelay(){return _triggerNoteDelay;};
		void TriggerNoteDelay(const int value){_triggerNoteDelay = value;};

		void NoteOn();
		void NoteOff();///< 
		void NoteOffFast();///< 
		

		void Retrigger(const int ticks,const int volumeModifier);
		
		void Work(int numSamples,float * pSampleL,float *pSamlpesR,dsp::Cubic& _resampler);
		
		const int Instrument(){ return _instrument;};
		void Instrument(const int value){_instrument = value;};
		
		const int Channel(){ return m_channel;};
		void Channel(const int value){ m_channel = value;};

		XMSampler::EnvelopeController& AmplitudeEnvelope(){return m_AmplitudeEnvelope;};
		XMSampler::EnvelopeController& FilterEnvelope(){return m_FilterEnvelope;};
		XMSampler::EnvelopeController& PitchEnvelope(){return m_PitchEnvelope;};
		XMSampler::EnvelopeController& PanEnvelope(){return m_PitchEnvelope;};
		
		WaveDataController& Wave(){return m_WaveDataController;};

//		void Work(const int numSamples,float * pSampleL,float *pSamlpesR);

		void pChannel(XMSampler::Channel *p){m_pChannel = p;}; 
		XMSampler::Channel& rChannel(){return *m_pChannel;};
		void Tick();

		void PerformFx();

		inline void SetWaveVolume(const int volume);

		void pSampler(XMSampler * const p){m_pSampler = p;};///< 
		XMSampler * const pSampler(){return m_pSampler;};///< 

		void NextTick(const int value){m_NextTick = value;};///< 
		const int NextTick(){ return m_NextTick;};///< 
		
		const bool IsPlaying(){ return m_bPlay;};///< 
		void IsPlaying(const bool value){ m_bPlay = value;};///< 
		
		XMInstrument *pInstrument() { return m_pInstrument;};///< Get Instrument Pointer
		void pInstrument(XMInstrument *p){m_pInstrument = p;};///< Set Instrument Pointer

		/// Do Auto Vibrato
		void AutoVibrato();
		/// Get Auto Vibrato Amount
		const double AutoVibratoAmount(){return m_AutoVibratoAmount;};

		
		void TickRemain(const int count){m_TickRemain = count;};
		const int TickRemain(){return m_TickRemain;};

		void PeriodToSpeed();

	private:
		
		bool m_bPlay;///
		int _instrument;///< Instrument

		XMInstrument *m_pInstrument;

		DWORD m_EffectFlags;///

	
		float m_VolumeFadeSpeed;
		float m_VolumeFadeAmount;

		int _tickCounter;///< Tick
		int m_NextTick;///< 

		int _triggerNoteOff;
		int _triggerNoteDelay;
		
	
		int effVal;//
		int effCmd;//

		EnvelopeController m_FilterEnvelope;///
		EnvelopeController m_AmplitudeEnvelope;///
		EnvelopeController m_PanEnvelope;///< 
		EnvelopeController m_PitchEnvelope;///< 
		
		WaveDataController m_WaveDataController;///< 
		
		dsp::Filter _filter;///< 
		
		int _cutoff;///< 
		float _coModify;///< 
		
		int m_channel;///< 
		
		XMSampler::Channel* m_pChannel;///< 
		XMSampler *m_pSampler;///< 
		int effretMode;
		int effretTicks;
		float effretVol;
		int effOld;

		// Auto Vibrato 
		double m_AutoVibratoAmount;
		int m_AutoVibratoDepth;
		int m_AutoVibratoPos;
		
		int m_TickRemain;

		static const int m_ft2VibratoTable[256];///< Vibrato Table
		static const int m_RandomTable[64];///< Random

	};


	//////////////////////////////////////////////////////////////////////////
	//  XMSampler Declaration

	XMSampler(int index);

	virtual void Init(void);
	
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


	XMSampler::Channel& rChannel(const int index){ return m_Channel[index];};///< Channel 
	Voice& rVoice(const int index) { return _voices[index];};///< 

	XMInstrument & Instrument(const int index){return m_Instruments[index];};
	XMInstrument::WaveData & SampleData(const int index){return m_rWaveLayer[index];};
	
	/// get which Frequency is Linear or Amiga ?
	const bool IsLinearFreq(){ return m_bLinearFreq;};
	/// set which Frequency is Linear or Amiga ?
	void IsLinearFreq(const bool value){ m_bLinearFreq = value;};

	/// set current voice number
	const int NumVoices(){ return _numVoices;};
	/// get current voice number
	void NumVoices(const int value){_numVoices = value;};

	/// set resampler quality 
	void ResamplerQuality(const dsp::ResamplerQuality value);
	const dsp::ResamplerQuality ResamplerQuality();

	/// BPM Speed
	void CalcBPMAndTick();
	
	boost::recursive_mutex & Mutex()
	{
		return m_Mutex;
	};

protected:

	static TCHAR* _psName;
	int _numVoices;

	Voice _voices[MAX_POLYPHONY];
	dsp::Cubic _resampler;

//	void VoiceWork(int numsamples, int voice);
	unsigned char lastInstrument[MAX_TRACKS];
	
	XMSampler::Channel m_Channel[MAX_TRACKS];

	static inline int alteRand(int x)
	{
		return (x * rand()) / 32768;
	};

	void DeltaTick(const int value){m_DeltaTick = value;};
	const int DeltaTick(){return m_DeltaTick;};
private:

	int m_BPM;
	int m_TicksPerRow;///< 
	int m_DeltaTick;///< Tick
	int m_TickCount;///< 1s 
	bool m_bLinearFreq;
	///
	XMInstrument m_Instruments[MAX_INSTRUMENT];
	XMInstrument::WaveData m_rWaveLayer[MAX_INSTRUMENT];
	boost::recursive_mutex m_Mutex;
};
}
}
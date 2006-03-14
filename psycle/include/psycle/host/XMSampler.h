#ifndef _XMSAMPLER_H
#define _XMSAMPLER_H
#pragma once
/** @file 
 *  @brief header file
 *  $Date$
 *  $Revision$
 */
#include "Machine.h"
#include "Filter.h"

//namespace SF {
namespace psycle
{
	namespace host
	{
class XMSampler : public Machine
{
public:
	static const int POINT = 16;///< envelope point
	static const int MAX_POLYPHONY = 64;///< max polyphony 
	static const int MAX_INSTRUMENT = 256;///< max instrument
	static const int LINEAR_FREQUECY = 1;///< Liner
	static const UINT VERSION = 0x00008000;
	/// \todo Sampling rate can't be a constant!
	static const int TICK_PER_WAVE = 44100  /* samples/sec */ * 60 /* sec */  / (4 /*row */ * 6 /* tick  */ /* * BPM */ ); 

	struct CMD {
		static const UCHAR NONE			=		0x0;///< Do Nothing
		static const UCHAR PORTAUP		=		0x01;///<�@Portamento Up
		static const UCHAR PORTADOWN	=		0x02;///<�@Portamento Down
		static const UCHAR PORTA2NOTE	=		0x03;///<  Tone Portamento
		static const UCHAR VIBRATO =			0x04;///< Do Vibrato
		static const UCHAR TONEPORTAVOL =		0x05;///< Tone Portament & Volume Slide 
		static const UCHAR VIBRATOVOL =			0x06;///< Vibrato & Volume Slide
		static const UCHAR TREMOLO =			0x07;///< Tremolo
		static const UCHAR PANNING	=			0x08;///< Set Panning Position
		static const UCHAR OFFSET	=			0x09;///< Set Sample Offset
		static const UCHAR VOLUMESLIDE	=		0x0a;///< Volume Slide
		static const UCHAR POSITION_JUMP	=	0x0b;///< Position Jump
		static const UCHAR VOLUME	=			0x0c;///< Set Volume
		static const UCHAR PATTERN_BREAK	=	0x0d;///< Pattern Break
		static const UCHAR EXTENDED		=		0x0e;///< Extend Command
		static const UCHAR SETSPEED	=			0x0f;///< Set Speed or BPM
		static const UCHAR SET_GLOBAL_VOLUME =	0x10;///< Set Global Volume
		static const UCHAR GLOBAL_VOLUME_SLIDE = 0x11;///< Global Volume Slide
		static const UCHAR NOTE_OFF			 =	0x14;///< Note Off
		static const UCHAR SET_ENV_POSITION =	0x15;///< Set Envelope Position
		static const UCHAR PANNINGSLIDE =		0x19;///< PANNING SLIDE
		static const UCHAR RETRIG	=			0x1B;///< Retrigger Note
		static const UCHAR TREMOR	=			0x1D;///< Retrigger Note
		static const UCHAR EXTEND_XM_EFFECTS =	0x21;///< Extend XM Effects	
		static const UCHAR PANBRELLO	=			0x22;///< Panbrello

		// Extend Command
		
		static const UCHAR VOLSLIDEDOWN =		0x40;
		static const UCHAR VOLSLIDEUP	=		0x41;
		static const UCHAR FINEVOLDOWN	=		0x42;
		static const UCHAR FINEVOLUP	=		0x43;
//		static const UCHAR VIBRATO		= 0x1B;
		static const UCHAR EXFINEPORTAMENT =	0x44;
		static const UCHAR PANSLIDELEFT =		0x45;
		static const UCHAR PANSLIDERIGHT = 0x46;
//		static const UCHAR TONEPORTAMENTO = 0x1F;
		
		static const UCHAR ARPEGGIO				= 0x47;
		static const UCHAR GRISSANDO			= 0x48;
		static const UCHAR TREMOLO_TYPE			= 0x49;
		static const UCHAR FINE_PORTAMENTO_DOWN = 0x4A;
		static const UCHAR FINE_PORTAMENTO_UP	= 0x4B;
		
		static const UCHAR VIBRATO_TYPE = 0x4C;
		static const UCHAR VIBRATO_SPEED = 0x4D;
		static const UCHAR VIBRATO_DEPTH = 0x4E;
		static const UCHAR VIBRATO2 = 0x4F;
		
		static const UCHAR NOTE_CUT	= 0x50;

		static const UCHAR EXTRA_FINE_PORTAMENTO_DOWN = 0x51;
		static const UCHAR EXTRA_FINE_PORTAMENTO_UP	= 0x52;

		static const UCHAR PANBRELLO_WAVE_FORM = 0x53;
		static const UCHAR FINE_PATTERN_DELAY = 0x54;

		static const UCHAR SURROUND_OFF = 0x55;
		static const UCHAR SURROUND_ON = 0x56;

		static const UCHAR REVERB_OFF = 0x57;
		static const UCHAR REVERB_ON = 0x58;

		static const UCHAR CENTER_SURROUND = 0x59;
		static const UCHAR QUOAD_SURROUND  = 0x5A;

		static const UCHAR GROBAL_FILTERS = 0x5B;
		static const UCHAR LOCAL_FILTERS = 0x5C;

		static const UCHAR PLAY_FORWARD = 0x5D;
		static const UCHAR PLAY_BACKWARD = 0x5E;

		static const UCHAR SET_HIGH_OFFSET = 0x5F;

		static const UCHAR PATTERN_DELAY = 0x60;
		static const UCHAR PATTERN_LOOP = 0x61;

		static const UCHAR SET_FINE_TUNE = 0x62;
		static const UCHAR SET_PANNING = 0x63;
		static const UCHAR RETRIGGER_NOTE = 0x64;
		static const UCHAR FINE_VOLSLIDE_UP = 0x65;
		static const UCHAR FINE_VOLSLIDE_DOWN = 0x66;
		static const UCHAR NOTE_DELAY = 0x67;
		static const UCHAR SET_ACTIVE_MACRO = 0x69;

		static const UCHAR EXT_NOTEOFF	=	0xc0;
		static const UCHAR EXT_NOTEDELAY	= 0xd0;
		

	};



	enum InterpolationType
	{
		INTERPOL_NONE = 0,
		INTERPOL_LINEAR = 1,
		INTERPOL_SPLINE = 2
	};
	class Channel;
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

		void Init(XMInstrument & instrument,Channel& channel,const int note);
		
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
			m_ModulationAmount = m_pEnvelope->Value(0);
			
			if(m_pEnvelope->SustainBegin() == 0)
			{
				m_Stage = EnvelopeStage::SUSTAIN;
				CalcStep(0,1);
				return;
			}

			if(m_pEnvelope->Point(1) == - 1){
				m_Stage = EnvelopeStage::END;
			} else {
				CalcStep(0,1);
			}
		};

		/// NoteOff EnvelopeStage
		void NoteOff()
		{
			/// �T�X�e�B���̏ꍇ�̓����[�X�Ɏ����Ă���
			if(m_Stage == EnvelopeStage::SUSTAIN || m_Stage == EnvelopeStage::DOSTEP){
				m_Stage = EnvelopeStage::RELEASE;
				CalcStep(m_PositionIndex,m_PositionIndex + 1);
			}

		};

		/// Tick���i�񂾎��̏���
		void Work()
		{
			// DOSTEP�̏ꍇ�A����Tick�������߂�
			if((m_Stage == EnvelopeStage::DOSTEP)|(m_Stage == EnvelopeStage::RELEASE)){
				m_ModulationAmount += m_Step;

			} else {
				return;
			}

			m_Tick++;// 1Tick��1/44100�b

			if((m_Tick >= m_pEnvelope->Point(m_PositionIndex + 1)))
			{
				m_PositionIndex++;

				if(m_pEnvelope->LoopEnd() == m_PositionIndex){
					m_PositionIndex = m_pEnvelope->LoopStart();
					m_Tick = m_pEnvelope->Point(m_PositionIndex);
					m_ModulationAmount = m_pEnvelope->Value(m_PositionIndex);
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

				if((m_PositionIndex >= (m_pEnvelope->NumOfPoints() - 1))  | (m_PositionIndex > 15) | (m_pEnvelope->Point(m_PositionIndex) == -1)){
					m_Stage = EnvelopeStage::END;
					m_ModulationAmount = m_pEnvelope->Value(m_pEnvelope->NumOfPoints() - 1);
				} else {
					m_ModulationAmount = m_pEnvelope->Value(m_PositionIndex);
					CalcStep(m_PositionIndex,m_PositionIndex + 1);
				}
			}

		};

		/// �G���x���[�v�R���g���[���̒l���擾����
		const ValueType ModulationAmount()
		{
			return m_ModulationAmount;
		};
		
		const EnvelopeStage Stage(){return m_Stage;};
		void Stage(const EnvelopeStage value){m_Stage = value;};
		XMInstrument::Envelope & Envelope(){return *m_pEnvelope;};
	private:
		/// 1Sample������̃G���x���[�v�����ʂ����߂�
		inline void CalcStep(const int start,const int  end);

		int m_Tick;
		int m_PositionIndex;
		EnvelopeStage m_Stage;

		XMInstrument::Envelope * m_pEnvelope;

		ValueType m_ModulationAmount;
		ValueType m_Step;
	};// EnvelopeController
	class Voice;
	/// �`�����l�� 
	class Channel {
	public:
		/// �G�t�F�N�g�t���O
		struct EffectFlag
		{
			static const int VIBRATO =			0x00000001;///< �r�u���[�g
			static const int PORTAUP =			0x00000002;///< �|���^�����g�A�b�v
			static const int PORTADOWN =		0x00000004;///< �|���^�����g�_�E��
			static const int PORTA2NOTE =		0x00000008;///< �g�[���|���^�����g
			static const int VOLUMESLIDE =		0x00000010;///< �{�����[���X���C�h�A�b�v
			static const int PANSLIDE =			0x00000020;///< �p���j���O�X���C�h��
			static const int TREMOLO =			0x00000040;///< �g������
			static const int ARPEGGIO =			0x00000080;///< �A���y�W�I
			static const int NOTECUT =			0x00000100;///<  �m�[�g�J�b�g
			static const int PANBRELLO =		0x00000200;///<	Panbrello
			static const int GLOBALVOLSLIDE	=   0x00000400;///< GLobal Volume Slide
			static const int TREMOR			=	0x00000800;///< Tremor

		};

		/// �R���X�g���N�^
		Channel()
		{
			Init();
		};

		void Init(){
						m_InstrumentNo = 255;// �C���X�g�������g����
			m_Volume = 0;
			m_ChannelVolume = 128;// �^��
			m_Note = 255;// �m�[�g����
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

			panFactor = -1.0f;
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

		/// �C���X�g�������g�l�擾
		const int InstrumentNo(){return m_InstrumentNo;};
		/// �C���X�g�������g�l�ݒ�
		void InstrumentNo(const int no){m_InstrumentNo = no;};
		
		/// �`�����l���{�����[���擾
		const float ChannelVolume(){return m_ChannelVolume;};
		/// �`�����l���{�����[���ݒ�
		void ChannelVolume(const float value){m_ChannelVolume = value;};
		
		/// �{�����[���擾
		const int Volume(){return m_Volume;};
		/// �{�����[���ݒ�
		void Volume(const int value){m_Volume = value;};
		
		/// �`�����l���m�[�g�ݒ�
		const int Note(){ return m_Note;};
		/// �`�����l���m�[�g�ݒ�
		void Note(const int note,const int layer)
		{	m_Note = note;
			m_Period = NoteToPeriod(note,layer);
			m_bPeriodChange = true;
		};


		// Channel Command
		/// �{�����[���_�E��
		void VolumeDown(const int value){
			m_Volume -= value;
			if(m_Volume < 0){
				m_Volume = 0;
			}
		};

		/// �{�����[���A�b�v
		void VolumeUp(const int value){
			m_Volume += value;
			if(m_Volume > 255){
				m_Volume = 255;
			}
		};

		/// �g�[���|���^�����g�X�s�[�h
		void Porta2NoteDestPeriod(const double speed)
		{m_Porta2NoteDestPeriod = speed;};
		inline void Porta2NoteDestPeriod(const int note,const int layer);///< �g�[���|���^�����g�̖ڕW�l Set
		const double Porta2NoteDestPeriod(){return m_Porta2NoteDestPeriod;};///< �g�[���|���^�����g�̖ڕW�l Get

		/// �g�[���|���^�����g�����ݒ�
		void Porta2Note(const UCHAR note,const int parameter,const int layer);
		/// �g�[���|���^�����g���s
		void Porta2Note(XMSampler::Voice& voice);

		/// �|���^�����g�A�b�v(�ŏ���Tick)
		void PortamentoUp(const int speed)
		{ 
			m_EffectFlags |= EffectFlag::PORTAUP;
			if(speed != 0){
				m_PortamentoSpeed = speed;
			}
		};

		/// �|���^�����g�A�b�v(Work)
		void PortamentoUp()
		{
			m_Period -= m_PortamentoSpeed;
			m_bPeriodChange = true;
		};
		
		/// �|���^�����g�_�E��(�ŏ���Tick)
		void PortamentoDown(const int speed)
		{	m_EffectFlags |= EffectFlag::PORTADOWN;
			if(speed != 0){
				m_PortamentoSpeed = speed;
			}
		};

		/// �|���^�����g�_�E��(Work)
		void PortamentoDown()
		{
			m_Period += m_PortamentoSpeed;
			
			m_bPeriodChange = true;

		};

		/// �t�@�C���|���^�����g�A�b�v
		void FinePortamentoUp(const int value)
		{
			if(value != 0)
			{m_FinePortamentoValue = value;};

			m_Period -= m_FinePortamentoValue;
			m_bPeriodChange = true;

		};

		/// �t�@�C���|���^�����g�_�E��
		void FinePortamentoDown(const int value)
		{
			if(value != 0)
			{m_FinePortamentoValue = value;};
			m_Period += m_FinePortamentoValue;

			m_bPeriodChange = true;

		};

		/// �`�����l���G�t�F�N�g�t���O Get
		const int EffectFlags(){return m_EffectFlags;};
		/// �`�����l���G�t�F�N�g�t���O Set
		void EffectFlags(const int value){m_EffectFlags = value;};

		void pSampler(XMSampler * const pSampler){m_pSampler = pSampler;};
		/// �O���b�T���h get state of grissando is playing? 
		const bool IsGrissando(){return m_bGrissando;};
		/// �O���b�T���h set state of grissando is playing 
		void IsGrissando(const bool value){m_bGrissando = value;};
		///�@�r�u���[�g���������Ă��邩�H Get
		const bool IsVibrato(){return (m_EffectFlags & EffectFlag::VIBRATO) > 0;};
		///	 �r�u���[�g���������Ă��邩�H Set
		void IsVibrato(const bool value)
		{
			if(value){
				m_EffectFlags |= EffectFlag::VIBRATO;
			} else {
				m_EffectFlags &= !EffectFlag::VIBRATO;
			}
		};

		/// �r�u���[�g�i�ŏ���Tick)
		inline void Vibrato(const int depth,const int speed = 0);
		/// �r�u���[�g�iWork�j
		inline void Vibrato();

		void VibratoSpeed(const int value){	m_VibratoSpeed = value;};///< �r�u���[�g���x Set
		const int VibratoSpeed() {return m_VibratoSpeed;};///< �r�u���[�g�X�s�[�h Get

		void VibratoDepth(const int value){	m_VibratoDepth = value;};///< �r�u���[�g�[�x Set
		const int VibratoDepth() {return m_VibratoDepth;};///< �r�u���[�g�[�x Get

		void AutoVibratoAmount(const double value){m_AutoVibratoAmount = value;};
		const double AutoVibratoAmount(){return m_AutoVibratoAmount;};
		
		void VibratoAmount(const double value){m_VibratoAmount = value;};
		const double VibratoAmount(){return m_VibratoAmount;};


		/** �r�u���[�g�^�C�v */
		void VibratoType(const int value){	m_VibratoType = value;};
		const int VibratoType() {return m_VibratoType;};

		/// �g������(Tick)
		void Tremolo(const int depth,const int speed)
		{
			if(depth) m_TremoloDepth = depth;
			if(speed) m_TremoloSpeed = speed;
			m_EffectFlags |= EffectFlag::TREMOLO;
		};

		/// �g�������^�C�v Tremolo Type
		void TremoloType(const int type){m_TremoloType = type;};
		const int TremoloType(){return m_TremoloType;};

		/// �g������(Work)
		inline void Tremolo();
		/// �g�������l
		const float TremoloDelta(){return m_TremoloDelta;};
		
		///�{�����[���X���C�h�A�b�v�����ݒ�
		void VolumeSlide(const int speed,const bool bUp)
		{
			if(speed > 0){
				m_VolumeSlideSpeed = speed;
				m_bUp = bUp;
			}
			m_EffectFlags |= EffectFlag::VOLUMESLIDE;
		};
		
		///�{�����[���X���C�h�A�b�v���s
		void VolumeSlide()
		{
			if(m_bUp){
				VolumeUp(m_VolumeSlideSpeed);
			} else {
				VolumeDown(m_VolumeSlideSpeed);
			}
			m_bVolumeChange = true;
		};

		/// �p���j���O�X���C�h��(Tick)
		void PanningSlide(const int speed,const bool bLeft)
		{
			if(speed){
				m_PanSlideSpeed = (float)speed / 256.0f;
				m_bLeft = bLeft;
			}
			m_EffectFlags |= EffectFlag::PANSLIDE;
		};

		/// �p���j���O�X���C�h��(Work)
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

		/// Panbrello�^�C�v Panbrello Type
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
		/// Panbrello�l
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
		
		/// convert note to period
		const double NoteToPeriod(const int note,const int layer);
		/// convert period to note 
		const int PeriodToNote(const double period,const int layer);

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




		/// Get period
		const double Period(){return m_Period;};
		/// Set period
		void Period(const double value){m_Period = value;};

		const int Index(){ return m_Index;};
		void Index(const int value){m_Index = value;};

		const bool IsPeriodChange(){return m_bPeriodChange;};
		void IsPeriodChange(const bool value){m_bPeriodChange = value;};
		
		const bool IsVolumeChange(){return m_bVolumeChange;};
		void IsVolumeChange(const bool value){m_bVolumeChange = value;};


		/// Perform effect at first tick
		void PerformEffect(Voice& voice,const int cmd,const int parameter);

	private:

		int m_InstrumentNo;///< ( 1 - 255 )
		int m_Volume;///<  (0 - 100)
		float m_ChannelVolume;///< (0.0f - 1.0f)
		
		int m_Note;///< ���݂̃m�[�g�l
		double m_Period;///< ���݂̃s���I�h
		bool m_bPeriodChange;
		
		int m_EffectFlags;

		int	m_PortamentoSpeed;

		int m_Porta2NoteDestNote;
		double m_Porta2NoteDestPeriod;

		int m_FinePortamentoValue;
		XMSampler *m_pSampler;
		bool m_bGrissando;

		// �r�u���[�g
		int m_VibratoSpeed;///< vibrato speed �r�u���[�g�X�s�[�h
		int m_VibratoDepth;///< vibrato depth �r�u���[�g�f�v�X
		int m_VibratoType;///< vibrato type �r�u���[�g�^�C�v
		int m_VibratoPos;///< vibrato position 
		double m_VibratoAmount;///< vibrato amount
		double m_AutoVibratoAmount;///< vibrato amount

		int m_Index;///< �C���X�^���X�̃C���f�b�N�X

		// �g������
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

		/// �p���j���O�X���C�h
		float m_PanSlideSpeed;
		bool m_bLeft;

		/// �{�����[���X���C�h
		int m_VolumeSlideSpeed;
		/// �{�����[���A�b�v���_�E����
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

		float panFactor;///< �p���̈ʒu(0.0f - 1.0f) 0.5�Ő^��

		// Note Cut Command 
		//	Channel volume is 0 after n ticks
		int m_NoteCutCounter;

		bool m_bSurround;

		// �r�u���[�g�p Sin �e�[�u�� 
		static const int m_SinTable[64];///< Sin
		static const int m_RampDownTable[64];///< RampDown
		static const int m_SquareTable[64];///< Square
		static const int m_RandomTable[64];///< Random
	};

	/// �{�C�X�N���X
	class Voice
	{
	public:
		/// �R���X�g���N�^
		Voice(){
			Init();
		};

		/// Voice �̏�����
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

		void VoiceInit(const UCHAR note);///< �m�[�g�I�����̏���
		void ClearEffectFlags() {m_EffectFlags = 0;};///< �G�t�F�N�g�t���O�̃N���A
		
		const int TickCounter(){return _tickCounter;};///< Tick(Sample) �J�E���^ Get
		void TickCounter(const int value){_tickCounter = value;};///< Tick(Sample) �J�E���^ Set

		const int TriggerNoteOff(){return _triggerNoteOff;};
		void TriggerNoteOff(const int value){_triggerNoteOff = value;};
		
		const int TriggerNoteDelay(){return _triggerNoteDelay;};
		void TriggerNoteDelay(const int value){_triggerNoteDelay = value;};

		// �m�[�g�I��
		void NoteOn();
		void NoteOff();///< �m�[�g�I�t
		void NoteOffFast();///< �m�[�g�J�b�g�H
		

		/// ���g���K�[
		void Retrigger(const int ticks,const int volumeModifier);
		
		/// ���T���v�����s����numSamples���̏������s���A�o�b�t�@�ɏ�������
		void Work(int numSamples,float * pSampleL,float *pSamlpesR,Cubic& _resampler);
		
		/// �C���X�g�������g
		const int Instrument(){ return _instrument;};
		void Instrument(const int value){_instrument = value;};
		
		/// �`�����l��
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

		/// �E�F�[�u�{�����[���̐ݒ�
		inline void SetWaveVolume(const int volume);

		void pSampler(XMSampler * const p){m_pSampler = p;};///< �e�ւ̃|�C���^Set
		XMSampler * const pSampler(){return m_pSampler;};///< �e�ւ̃|�C���^Get

		void NextTick(const int value){m_NextTick = value;};///< ����Tick�ւ̃T���v���lSet
		const int NextTick(){ return m_NextTick;};///< ����Tick�ւ̃T���v���l Get
		
		const bool IsPlaying(){ return m_bPlay;};///< ���t�����ǂ��� Get
		void IsPlaying(const bool value){ m_bPlay = value;};///< ���t�����ǂ��� Set
		
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
		
		bool m_bPlay;///< ���t�����ǂ����H
		int _instrument;///< Instrument �ԍ�

		XMInstrument *m_pInstrument;

		DWORD m_EffectFlags;/// �G�t�F�N�g�t���O

		// �{�����[���t�F�[�h
	
		float m_VolumeFadeSpeed;
		float m_VolumeFadeAmount;

		int _tickCounter;///< Tick �J�E���^
		int m_NextTick;///< ����Ticks

		int _triggerNoteOff;
		int _triggerNoteDelay;
		
		// �|���^�����g
	
		int effVal;//? ����͍���g�p���Ȃ�
		int effCmd;//? ����͍���g�p���Ȃ�

		EnvelopeController m_FilterEnvelope;///< �t�B���^�[�G���x���[�v�R���g���[��
		EnvelopeController m_AmplitudeEnvelope;///< �A���v�G���x���[�v�R���g���[��
		EnvelopeController m_PanEnvelope;///< �p���j���O�G���x���[�v�R���g���[��
		EnvelopeController m_PitchEnvelope;///< �s�b�`�G���x���[�v�R���g���[��
		
		WaveDataController m_WaveDataController;///< �T���v�����O�f�[�^�R���g���[��
		
		Filter _filter;///< �t�B���^
		
		int _cutoff;///< �J�b�g�I�t�l
		float _coModify;///< 
		
		int m_channel;///< �`�����l��
		
		XMSampler::Channel* m_pChannel;///< �`�����l���ւ̃|�C���^ 
		XMSampler *m_pSampler;///< �T���v���ւ̃|�C���^
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

	XMSampler(int index);

	virtual void Init(void);
	
	void Tick();
	/// numSamples���̏������s���A�o�b�t�@�ɏ�������
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
	
	// MOD Tick �G�~�����[�V�����p
	void TicksPerRow(const int value){m_TicksPerRow = value;};///< Get 1�s������̃e�B�b�N��
	const int TicksPerRow(){ return m_TicksPerRow;};///< Set 1�s������̃e�B�b�N��


	XMSampler::Channel& rChannel(const int index){ return m_Channel[index];};///< Channel�I�u�W�F�N�g�ւ̎Q��
	Voice& rVoice(const int index) { return _voices[index];};///< �`�����l���I�u�W�F�N�g�ւ̎Q��

	XMInstrument & Instrument(const int index){return m_Instruments[index];};
	
	/// get which Frequency is Linear or Amiga ?
	const bool IsLinearFreq(){ return m_bLinearFreq;};
	/// set which Frequency is Linear or Amiga ?
	void IsLinearFreq(const bool value){ m_bLinearFreq = value;};

	/// set current voice number
	const int NumVoices(){ return _numVoices;};
	/// get current voice number
	void NumVoices(const int value){_numVoices = value;};

	/// set resampler quality 
#if defined psycleWTL
	void ResamplerQuality(const ::ResamplerQuality value);
	/// get resampler quality 
	const ::ResamplerQuality ResamplerQuality();
#else
	void ResamplerQuality(const psycle::host::ResamplerQuality value);
	const psycle::host::ResamplerQuality ResamplerQuality();
#endif

	/// BPM Speed�����ɁA�v�Z����
	void CalcBPMAndTick();
	
	boost::recursive_mutex & Mutex()
	{
		return m_Mutex;
	};

protected:

	static TCHAR* _psName;
	int _numVoices;

	Voice _voices[MAX_POLYPHONY];
	Cubic _resampler;

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
	int m_TicksPerRow;///< XM�ł����Ƃ���̃X�s�[�h
	int m_DeltaTick;///< Tick�̕ω���
	int m_TickCount;///< 1�s��TickCounrer
	bool m_bLinearFreq;
	/// ���F�f�[�^ Instrument Data
	XMInstrument m_Instruments[MAX_INSTRUMENT];
	boost::recursive_mutex m_Mutex;
};
}
}
#endif
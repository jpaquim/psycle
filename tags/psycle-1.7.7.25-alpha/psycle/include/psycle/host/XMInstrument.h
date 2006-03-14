#ifndef _XMXMInstrument_H
#define _XMXMInstrument_H
#pragma unmanaged
/** @file
 *  @brief header file
 *  XMInstrument は XMSampler::Voiceをどのように制御するかを指定する
 *  XMInstrumentはパラメータクラスであり、XMSampler::Voiceはそれをもとに音源をコントロールします。
 *  $Date$
 *  $Revision$
 */
#if defined(_WINAMP_PLUGIN_)
//	#include <afxmt.h>
#endif // _WINAMP_PLUGIN_

#include "Constants.h"
#include "FileIO.h"
//namespace SF {
namespace psycle
{
	namespace host
	{
	typedef float ValueType;
	class XMInstrument
	{
	public:
		/// 1つのInstrumentでいくつSampleがAssignできるか
		static const int MAX_ASSIGNNABLE_SAMPLE = 32;
		/// Note最大数
		static const int MAX_NOTES = 120;

		/** Envelope Class
		 *  エンベロープの形を定義します。
		 *	EnvelopeGenaratorがエンベロープを定義したEnvelopeにしたがって値をコントロールします。
		 *　エンベロープの仕様:
		 *　Envelope Pointは16points
		 *  Envelope 1 Tick = 1/44100 sec (XMファイル読み込み時に変換）
		 *  Envelope Value は 0.0 - 1.0f
		 */
		class Envelope {

		public:
			// static constants 
			static const int MAX_POINT = 16;///< エンベロープポイントの最大値
			static const int INVALID = -1;
			typedef std::pair<int,ValueType> PointValue;
			typedef std::vector< PointValue > Points;	
			// constructor & destructor
			explicit Envelope()
			{	Init();
			};

			// copy Constructor
			Envelope(const Envelope& other)
			{
				Init();
				operator=(other);
			}

			~Envelope(){;};

			// Init
			void Init(){
				m_Enabled = false;
				m_SustainBegin = INVALID;
				m_SustainEnd = INVALID;
				m_LoopStart = INVALID;
				m_LoopEnd = INVALID;

//				m_NumOfPoints = 0;
				
			}

			// property 
			/// エンベロープが有効か
			const bool IsEnabled(){ return m_Enabled;};
			/// エンベロープが有効か
			void IsEnabled(const bool value){ m_Enabled = value;};

			
			/** エンベロープのTickを取得する 
			 * @param index エンベロープのインデックス
			 * @return エンベロープのTick値。indexが範囲外の場合 -1を返す。*/
			const int Point(const int index)
			{	
				if(index >= 0 && index < (int)m_Points.size()){
					return m_Points[index].first;
				}
				return INVALID;
			};

			const int Point(const int index,const int value)
			{ 
				ATLASSERT(index >= 0 && index < (int)m_Points.size());
				m_Points[index].first = value;
				return PointAndValue(index,value,m_Points[index].second);
			};
			
			const ValueType Value(const int index)
			{ //return m_Value[index];
				ATLASSERT(index >= 0 && index < (int)m_Points.size());
				return m_Points[index].second;
			};

			void Value(const int index,const ValueType value)
			{
				ATLASSERT(index >= 0 && index < (int)m_Points.size());
				m_Points[index].second = value;
			};
			
			/// Point値と値をセットする
			const int PointAndValue(const int index,const int point,const ValueType value);
			
			/// Point値と値を挿入する
			const int Insert(const int point,const ValueType value);
			
			/// Point値と値を代入する
			void Append(const int point,const ValueType value)
			{
				PointValue _value;
				_value.first = point;
				_value.second = value;
				m_Points.push_back(_value);
			};

			/// indexの位置のPointとValueを削除する
			void Delete(const int index)
			{
				ATLASSERT(index < (int)m_Points.size());
				if(index < (int)m_Points.size())
				{
					m_Points.erase(m_Points.begin() + index);
					if(index == m_SustainBegin || index == m_SustainEnd)
					{
						m_SustainBegin = INVALID;
						m_SustainEnd = INVALID;
					} else {
						if(m_SustainBegin > index)
						{
							m_SustainBegin--;
						}
						if(m_SustainEnd > index)
						{
							m_SustainEnd--;
						}
					}

					if(index == m_LoopStart || index == m_LoopEnd)
					{
						m_LoopStart = INVALID;
						m_LoopEnd = INVALID;
					} else {
						if(m_LoopStart > index)
						{
							m_LoopStart--;
						}
						if(m_LoopEnd > index)
						{
							m_LoopEnd--;
						}
					}

				}
			};

			/// Point,Valueのクリア
			void Clear()
			{
				m_Points.clear();
			};

			const int SustainBegin(){ return m_SustainBegin;};
			void SustainBegin(const int value){m_SustainBegin = value;};

			const int SustainEnd(){ return m_SustainEnd;};
			void SustainEnd(const int value){m_SustainEnd = value;};

			const int LoopStart(){return m_LoopStart;};
			void LoopStart(const int value){m_LoopStart = value;};

			const int LoopEnd(){return m_LoopEnd;};
			void LoopEnd(const int value){m_LoopEnd = value;};

			const int NumOfPoints(){ return m_Points.size();};

			void Load(RiffFile& riffFile,const UINT version);
			void Save(RiffFile& riffFile,const UINT version);

			/// = 演算子
			Envelope& operator=(const Envelope& other){
				
				if(this == &other){ return *this;};

				m_Enabled = other.m_Enabled;
				m_Points.clear();

				for(Points::const_iterator it = other.m_Points.begin();it != other.m_Points.end();it++)
				{
					m_Points.push_back(*it);
				}
				m_SustainBegin = other.m_SustainBegin;
				m_SustainEnd = other.m_SustainEnd; 
				m_LoopStart = other.m_LoopStart;
				m_LoopEnd = other.m_LoopEnd;

				return *this;
			};

		private:
			
			bool m_Enabled;///< Envelope is enable or disable
			//? パラメータについては、floatにするかどうか考える
			Points m_Points; 
			//int m_Points[MAX_POINT]; ///< Position [in Samples at 44.1KHz]
			//ValueType m_Value[MAX_POINT];///< Value [0 - 1.0f]
			int m_SustainBegin;///< Sustain Start Point
			int m_SustainEnd;///< Sustain End Point
			int m_LoopStart;///< Loop Start Point
			int m_LoopEnd;///< Loop End Point
			
			//int m_NumOfPoints;///< Number Of Envelope Points
		};// class Envelope


		/// Wave Data 保持クラス
		class WaveData {
		public:
			/** Wave Loop Type */
			struct LoopType {
				enum {
					DO_NOT = 0x0,///< Do Nothing
					NORMAL = 0x1,///< normal Start --> End ,Start --> End ...
					BIDI = 0x2	 ///< bidirectional Start --> End, End --> Start ...
				};
			};

			/// Constructor
			WaveData()
			{
				m_pWaveDataL = m_pWaveDataR = NULL;
				Init();
			};

			/// Initialize
			void Init(){
				DeleteWaveData();
				m_WaveLength = 0;
				m_WaveVolume = 0;
				m_WaveLoopStart = 0;
				m_WaveLoopEnd = 0;
				m_WaveTune = 0;
				m_WaveFineTune = 0;	
				m_WaveLoopType = false;
				m_WaveStereo = false;;
				m_WaveLoopType = LoopType::DO_NOT;
			};

			/// Destructor
			~WaveData(){
				if(m_pWaveDataL)  delete [] m_pWaveDataL;
				if(m_pWaveDataR)  delete [] m_pWaveDataR;
			};
			
			const UINT WaveLength(){ return m_WaveLength;};
			void WaveLength (const UINT value){m_WaveLength = value;};

			const unsigned short WaveVolume(){ return m_WaveVolume;};
			void WaveVolume(const unsigned short value){m_WaveVolume = value;};
			
			const UINT WaveLoopStart(){ return m_WaveLoopStart;};
			void WaveLoopStart(const UINT value){m_WaveLoopStart = value;};

			const UINT WaveLoopEnd(){ return m_WaveLoopEnd;};
			void WaveLoopEnd(const UINT value){m_WaveLoopEnd = value;};

			const int WaveTune(){return m_WaveTune;};
			void WaveTune(const int value){m_WaveTune = value;};

			const int WaveFineTune(){return m_WaveFineTune;};
			void WaveFineTune(const int value){m_WaveFineTune = value;};

			const int WaveLoopType(){ return m_WaveLoopType;};
			void WaveLoopType(const int value){ m_WaveLoopType = value;};

			const bool IsWaveStereo(){ return m_WaveStereo;};
			void IsWaveStereo(const bool value){ m_WaveStereo = value;};

			const TCHAR * WaveName(){ return m_WaveName;};

			const signed short * pWaveDataL(){ return m_pWaveDataL;};
			const signed short * pWaveDataR(){ return m_pWaveDataR;};
			
			signed short WaveDataL(const int index) const { return (*(m_pWaveDataL + index));};
			signed short WaveDataR(const int index) const { return (*(m_pWaveDataR + index));};
			
			void WaveDataL(const int index,const signed short value){ *(m_pWaveDataL + index) = value;};
			void WaveDataR(const int index,const signed short value){ *(m_pWaveDataR + index) = value;};

			void DeleteWaveData(){
				if(m_pWaveDataL != NULL){
					delete []  m_pWaveDataL;
					m_pWaveDataL = NULL;
				}

				if(m_pWaveDataR != NULL){
					delete [] m_pWaveDataR;
					m_pWaveDataR = NULL;
				}
			};

			void AllocWaveData(const int iLen,const bool bStereo)
			{
				DeleteWaveData();
				if(bStereo){
					m_pWaveDataR = new signed short[iLen];
					m_pWaveDataL = new signed short[iLen];
					m_WaveStereo = true;
				} else {
					m_pWaveDataL = new signed short[iLen];
					m_pWaveDataR = NULL;
					m_WaveStereo = false;
				}
				m_WaveLength  = iLen;
			};

			void Load(RiffFile& riffFile,const UINT version);
			void Save(RiffFile& riffFile,const UINT version);

			/// Wave Data Copy Operator
			void operator= (const WaveData& source)
			{
				Init();
				m_WaveLength = source.m_WaveLength;
				m_WaveVolume = source.m_WaveLength;
				m_WaveLoopStart = source.m_WaveLoopStart;
				m_WaveLoopEnd = source.m_WaveLoopEnd;
				m_WaveTune = source.m_WaveTune;
				m_WaveFineTune = source.m_WaveFineTune;	
				m_WaveLoopType = source.m_WaveLoopType;
				m_WaveStereo = source.m_WaveStereo;
				_tcscpy(m_WaveName,source.m_WaveName);

				AllocWaveData(source.m_WaveLength,source.m_WaveStereo);
			
				memcpy(m_pWaveDataL,source.m_pWaveDataL,source.m_WaveLength * sizeof(short));
				if(source.m_WaveStereo){
					memcpy(m_pWaveDataR,source.m_pWaveDataR,source.m_WaveLength * sizeof(short));
				}

			};

		private:

			UINT m_WaveLength;
			unsigned short m_WaveVolume;
			UINT m_WaveLoopStart;
			UINT m_WaveLoopEnd;
			int m_WaveTune;
			int m_WaveFineTune;	
			int m_WaveLoopType;
			bool m_WaveStereo;
			TCHAR m_WaveName[32];
			signed short *m_pWaveDataL;
			signed short *m_pWaveDataR;

		};// WaveData()

		/// コンストラクタ
		XMInstrument();
		/// デストラクタ
		~XMInstrument();

		void Init();
		void DeleteLayer(int c);
		/// インストルメント部分のロード
		void Load(RiffFile& riffFile,const UINT version);
		/// インストルメント部分のセーブ
		void Save(RiffFile& riffFile,const UINT version);

		// Property //
		
		// New Note Action (New?)
		const int NNA() { return m_NNA;};///< Get NNA
		void NNA(const int value){ m_NNA = value;};///< Set NNA
		
		Envelope* const AmpEnvelope(){ return &m_AmpEnvelope;};
		Envelope* const FilterEnvelope(){ return &m_FilterEnvelope;};
		
		const int FilterCutoff(){ return m_FilterCutoff;};
		void FilterCutoff(const int value){m_FilterCutoff = value;};

		const int FilterResonance() { return m_FilterResonance;};
		void FilterResonance(const int value){m_FilterResonance = value;};

		const int FilterEnvAmount() { return m_FilterEnvAmount;};
		void FilterEnvAmount(const int value){ m_FilterEnvAmount = value;};
		
		const int FilterType(){ return m_FilterType;};
		void FilterType(const int value){ m_FilterType = value;};

		Envelope* const PanEnvelope(){return &m_PanEnvelope;};
		Envelope* const PitchEnvelope(){return &m_PitchEnvelope;};

		const bool IsRandomPanning(){return  m_RandomPanning;};///< Random Panning
		void IsRandomPanning(const bool value){m_RandomPanning = value;};///< Random Panning

		const bool IsRandomCutoff(){return m_RandomCutoff;};///< Random CutOff
		void IsRandomCutoff(const bool value){m_RandomCutoff = value;};

		const bool IsRandomResonance(){return m_RandomResonance;};///< Random Resonance
		void IsRandomResonance(const bool value){m_RandomResonance = value;};
		
		const bool IsRandomSampleStart(){return m_RandomSampleStart;};///< Random Sample Start
		void IsRandomSampleStart(const bool value){m_RandomSampleStart = value;};
		

#if defined psycleWTL
		SF::string& Name(){return m_Name;};
		void Name(const SF::string& name);
#else
		std::string& Name(){return m_Name;};
		void Name(const std::string& name);
#endif	

		// 
		WaveData& rWaveData(const int index){ return m_WaveData[index];};

	
		const bool IsLoop(){ return m_Loop;};
		void IsLoop(const bool value){m_Loop = value;};

		const int Lines(){ return m_Lines;};
		void Lines(const int value){ m_Lines = value;};
	
		const float Pan() { return m_InitPan;};
		void Pan(const float pan) { m_InitPan = pan;};

		/// Get Volume Fade Enable or Disable
		const bool IsVolumeFade() { return m_bVolumeFade;};
		/// Set Volume Fade Enable or Disable
		void IsVolumeFade(const bool value){m_bVolumeFade = value;};

		/// Get Volume Fade Speed 
		const float VolumeFadeSpeed() { return m_VolumeFadeSpeed;};
		/// Set Volume Fade Speed
		void VolumeFadeSpeed(const float value){ m_VolumeFadeSpeed = value;};

		const int AutoVibratoType(){return m_AutoVibratoType;};
		const int AutoVibratoSweep(){return m_AutoVibratoSweep;};
		const int AutoVibratoDepth(){return m_AutoVibratoDepth;};
		const int AutoVibratoRate(){return m_AutoVibratoRate;};

		void AutoVibratoType(const int value){m_AutoVibratoType = value ;};
		void AutoVibratoSweep(const int value){m_AutoVibratoSweep = value ;};
		void AutoVibratoDepth(const int value){m_AutoVibratoDepth = value ;};
		void AutoVibratoRate(const int value){m_AutoVibratoRate = value ;};

		const bool IsAutoVibrato(){return m_AutoVibratoDepth && m_AutoVibratoRate;};

		const bool IsEnabled(){ return m_bEnabled;};
		void IsEnabled(const bool value){ m_bEnabled = value;};

		const int NoteToSample(const int note){return m_AssignNoteToSample[note];};
		void NoteToSample(const int note,const int sampleNo){m_AssignNoteToSample[note] = sampleNo;};

		void operator= (const XMInstrument & other)
		{
			
			for(int i = 0;i < MAX_ASSIGNNABLE_SAMPLE;i++)
			{
				m_WaveData[i] = other.m_WaveData[i];
			}

			m_Loop = other.m_Loop;
			m_Lines = other.m_Lines;

			m_NNA = other.m_NNA;
			

			m_AmpEnvelope = other.m_AmpEnvelope;
			m_FilterEnvelope = other.m_FilterEnvelope;
			m_FilterCutoff = other.m_FilterCutoff;

			m_FilterResonance = other.m_FilterResonance;
			m_FilterEnvAmount = other.m_FilterEnvAmount;
			m_FilterType = other.m_FilterType;

			// Paninng
			m_InitPan = other.m_InitPan;
			m_PanEnvelope = other.m_PanEnvelope;

			// Pitch Envelope
			m_PitchEnvelope = other.m_PitchEnvelope;

			// LFO

			m_RandomPanning = other.m_RandomPanning;
			m_RandomCutoff = other.m_RandomCutoff;
			m_RandomResonance = other.m_RandomResonance;
			m_RandomSampleStart = other.m_RandomSampleStart;

			m_Name = other.m_Name;

			m_bVolumeFade = other.m_bVolumeFade;
			m_VolumeFadeSpeed = other.m_VolumeFadeSpeed;

			// Auto Vibrato
			m_AutoVibratoType = other.m_AutoVibratoType;
			m_AutoVibratoSweep = other.m_AutoVibratoSweep;
			m_AutoVibratoDepth = other.m_AutoVibratoDepth;
			m_AutoVibratoRate = other.m_AutoVibratoRate;

			// 
			m_bEnabled = other.m_bEnabled;

			memcpy(m_AssignNoteToSample,other.m_AssignNoteToSample,sizeof(int) * MAX_NOTES);

		};

	private:
		WaveData m_WaveData[MAX_ASSIGNNABLE_SAMPLE];

		//////////////////////////////////////////////////////////////////
		// Loop stuff

		bool m_Loop;
		int m_Lines;

		//////////////////////////////////////////////////////////////////
		// NNA values overview:
		//
		// 0 = Note Cut			[Fast Release 'Default']
		// 1 = Note Release		[Release Stage]
		// 2 = Note Continue	[No NNA]
		int m_NNA;
		
		//////////////////////////////////////////////////////////////////
		// Amplitude Envelope overview:
		//
		//int ENV_AT;	// Attack Time [in Samples at 44.1Khz]
		//int ENV_DT;	// Decay Time [in Samples at 44.1Khz]
		//int ENV_SL;	// Sustain Level [in %]
		//int ENV_RT;	// Release Time [in Samples at 44.1Khz]
		
		Envelope m_AmpEnvelope;
		
		// Filter 
		/*int ENV_F_AT;	// Attack Time [in Samples at 44.1Khz]
		int ENV_F_DT;	// Decay Time [in Samples at 44.1Khz]
		int ENV_F_SL;	// Sustain Level [0..128]
		int ENV_F_RT;	// Release Time [in Samples at 44.1Khz]

		int ENV_F_CO;	// Cutoff Frequency [0-127]
		int ENV_F_RQ;	// Resonance [0-127]
		int ENV_F_EA;	// EnvAmount [-128,128]
		int ENV_F_TP;	// Filter Type [0-4]
		*/

		Envelope m_FilterEnvelope;
		int m_FilterCutoff;///< Cutoff Frequency [0-127]
		int m_FilterResonance;///< Resonance [0-127]
		int m_FilterEnvAmount;///< EnvAmount [-128,128]
		int m_FilterType;///< Filter Type [0-4]


		// Paninng
		float m_InitPan;
		Envelope m_PanEnvelope;///

		// Pitch Envelope
		Envelope m_PitchEnvelope;

		// LFO

		bool m_RandomPanning;///< Random Panning
		bool m_RandomCutoff;///< Random CutOff
		bool m_RandomResonance;///< Random Resonance
		bool m_RandomSampleStart;///< Random SampleStart

#if defined psycleWTL
		SF::string m_Name;
#else
		std::string m_Name;
#endif
		bool m_bVolumeFade;
		float m_VolumeFadeSpeed;

		// Auto Vibrato
		int m_AutoVibratoType;
		int m_AutoVibratoSweep;
		int m_AutoVibratoDepth;
		int m_AutoVibratoRate;

		// 
		bool m_bEnabled;

		int m_AssignNoteToSample[MAX_NOTES];/// table of assign note to sample number
		
	};
	}
}// namespace SF

#endif

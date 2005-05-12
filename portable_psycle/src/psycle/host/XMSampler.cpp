#include <project.private.hpp>
#include "XMInstrument.hpp"
#include "XMSampler.hpp"
#include "Player.hpp"
#include "Song.hpp"
#include "FileIO.hpp"
#include "Configuration.hpp"
#pragma unmanaged

namespace psycle
{
	namespace host
	{
		TCHAR* XMSampler::_psName = _T("Sampulse");

		const int XMSampler::Voice::m_FineSineData[256] = {
			0,  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
			24, 26, 27, 29, 30, 32, 33, 34, 36, 37, 38, 39, 41, 42, 43, 44,
			45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
			59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 64, 64,
			64, 64, 64, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 60, 60,
			59, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,
			45, 44, 43, 42, 41, 39, 38, 37, 36, 34, 33, 32, 30, 29, 27, 26,
			24, 23, 22, 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2,
			0, -2, -3, -5, -6, -8, -9,-11,-12,-14,-16,-17,-19,-20,-22,-23,
			-24,-26,-27,-29,-30,-32,-33,-34,-36,-37,-38,-39,-41,-42,-43,-44,
			-45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,-56,-57,-58,-59,
			-59,-60,-60,-61,-61,-62,-62,-62,-63,-63,-63,-64,-64,-64,-64,-64,
			-64,-64,-64,-64,-64,-64,-63,-63,-63,-62,-62,-62,-61,-61,-60,-60,
			-59,-59,-58,-57,-56,-56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,
			-45,-44,-43,-42,-41,-39,-38,-37,-36,-34,-33,-32,-30,-29,-27,-26,
			-24,-23,-22,-20,-19,-17,-16,-14,-12,-11, -9, -8, -6, -5, -3, -2
		};

		const int XMSampler::Voice::m_FineRampDownData[256] = {
			64, 63, 63, 62, 62, 61, 61, 60, 60, 59, 59, 58, 58, 57, 57, 56,
			56, 55, 55, 54, 54, 53, 53, 52, 52, 51, 51, 50, 50, 49, 49, 48,
			48, 47, 47, 46, 46, 45, 45, 44, 44, 43, 43, 42, 42, 41, 41, 40,
			40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32,
			32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24,
			24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
			16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,
			8,  7,  7,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  0,
			0, -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6, -7, -7, -8,
			-8, -9, -9,-10,-10,-11,-11,-12,-12,-13,-13,-14,-14,-15,-15,-16,
			-16,-17,-17,-18,-18,-19,-19,-20,-20,-21,-21,-22,-22,-23,-23,-24,
			-24,-25,-25,-26,-26,-27,-27,-28,-28,-29,-29,-30,-30,-31,-31,-32,
			-32,-33,-33,-34,-34,-35,-35,-36,-36,-37,-37,-38,-38,-39,-39,-40,
			-40,-41,-41,-42,-42,-43,-43,-44,-44,-45,-45,-46,-46,-47,-47,-48,
			-48,-49,-49,-50,-50,-51,-51,-52,-52,-53,-53,-54,-54,-55,-55,-56,
			-56,-57,-57,-58,-58,-59,-59,-60,-60,-61,-61,-62,-62,-63,-63,-64
		};
		const int XMSampler::Voice::m_FineSquareTable[256] =	{
			64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
			64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
			64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
			64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
			64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
			64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
			64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
			64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
			-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
			-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
			-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
			-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
			-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
			-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
			-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
			-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 
		};


		// Random wave table
		//\todo: convert this table to IT compatible.
		const int XMSampler::Voice::m_RandomTable[256] =	{
			98 ,-127 ,-43 ,88 ,102 ,41 ,-65 ,-94 ,125 ,20 ,-71 ,-86 ,-70 ,-32 ,-16 ,-96 ,
			17 ,72 ,107 ,-5 ,116 ,-69 ,-62 ,-40 ,10 ,-61 ,65 ,109 ,-18 ,-38 ,-13 ,-76 ,
			-23 ,88 ,21 ,-94 ,8 ,106 ,21 ,-112 ,6 ,109 ,20 ,-88 ,-30 ,9 ,-127 ,118 ,
			42 ,-34 ,89 ,-4 ,-51 ,-72 ,21 ,-29 ,112 ,123 ,84 ,-101 ,-92 ,98 ,-54 ,-95 ,
			98 ,-127 ,-43 ,88 ,102 ,41 ,-65 ,-94 ,125 ,20 ,-71 ,-86 ,-70 ,-32 ,-16 ,-96 ,
			17 ,72 ,107 ,-5 ,116 ,-69 ,-62 ,-40 ,10 ,-61 ,65 ,109 ,-18 ,-38 ,-13 ,-76 ,
			-23 ,88 ,21 ,-94 ,8 ,106 ,21 ,-112 ,6 ,109 ,20 ,-88 ,-30 ,9 ,-127 ,118 ,
			42 ,-34 ,89 ,-4 ,-51 ,-72 ,21 ,-29 ,112 ,123 ,84 ,-101 ,-92 ,98 ,-54 ,-95 ,
			98 ,-127 ,-43 ,88 ,102 ,41 ,-65 ,-94 ,125 ,20 ,-71 ,-86 ,-70 ,-32 ,-16 ,-96 ,
			17 ,72 ,107 ,-5 ,116 ,-69 ,-62 ,-40 ,10 ,-61 ,65 ,109 ,-18 ,-38 ,-13 ,-76 ,
			-23 ,88 ,21 ,-94 ,8 ,106 ,21 ,-112 ,6 ,109 ,20 ,-88 ,-30 ,9 ,-127 ,118 ,
			42 ,-34 ,89 ,-4 ,-51 ,-72 ,21 ,-29 ,112 ,123 ,84 ,-101 ,-92 ,98 ,-54 ,-95 ,
			98 ,-127 ,-43 ,88 ,102 ,41 ,-65 ,-94 ,125 ,20 ,-71 ,-86 ,-70 ,-32 ,-16 ,-96 ,
			17 ,72 ,107 ,-5 ,116 ,-69 ,-62 ,-40 ,10 ,-61 ,65 ,109 ,-18 ,-38 ,-13 ,-76 ,
			-23 ,88 ,21 ,-94 ,8 ,106 ,21 ,-112 ,6 ,109 ,20 ,-88 ,-30 ,9 ,-127 ,118 ,
			42 ,-34 ,89 ,-4 ,-51 ,-72 ,21 ,-29 ,112 ,123 ,84 ,-101 ,-92 ,98 ,-54 ,-95 
		};



		// calculated table from the following formula:
		// period =  pow(2.0,double(15.74154-(note/12.0f)));
		// 15.74154 comes from  5 + 10.7415, being 5 = the middle octave, and
		// 10.74154 = log2(2*7159090.5/8363) , being 7159090.5 the Amiga Clock Speed and 8363 the middle C sample rate.
		// why multiplied by 2? well.. it should be a 4 (the amiga period was shifted two octaves up in newer trackers),
		// so i am not sure. The rest of the logic is perfectly clear.
		// 5 also represents the middle octave, and pow(2,10.74146698) = 1712 ( middle C period )

		const float XMSampler::AmigaPeriod[XMInstrument::NOTE_MAP_SIZE] = {
			54787,	51712,	48809,	46070,	43484,	41044,	38740,	36566,	34514,	32576,	30748,	29022,
			27393,	25856,	24405,	23035,	21742,	20522,	19370,	18283,	17257,	16288,	15374,	14511,
			13697,	12928,	12202,	11517,	10871,	10261,	9685,	9141,	8628,	8144,	7687,	7256,
			6848,	6464,	6101,	5759,	5436,	5130,	4843,	4571,	4314,	4072,	3844,	3628,
			3424,	3232,	3051,	2879,	2718,	2565,	2421,	2285,	2157,	2036,	1922,	1814,
			1712,	1616,	1525,	1440,	1359,	1283,	1211,	1143,	1079,	1018,	961,	907,
			856,	808,	763,	720,	679,	641,	605,	571,	539,	509,	480,	453,
			428,	404,	381,	360,	340,	321,	303,	286,	270,	255,	240,	227,
			214,	202,	191,	180,	170,	160,	151,	143,	135,	127,	120,	113,
			107,	101,	95,		90,		85,		80,		76,		71,		67,		63,		60,		57 

		};
		// Original table
/*		const int XMSampler::AmigaPeriod[XMInstrument::NOTE_MAP_SIZE] = {
			54784,	51712,	48768,	46080,	43392,	40960,	38656,	36480,	34432,	32512,	30720,	29008,
			27392,	25856,	24384,	23040,	21696,	20480,	19328,	18240,	17216,	16256,	15360,	14504,
			13696,	12928,	12192,	11520,	10848,	10240,	9664,	9120,	8608,	8128,	7680,	7252,
			6848,	6464,	6096,	5760,	5424,	5120,	4832,	4560,	4304,	4064,	3840,	3626,
			3424,	3232,	3048,	2880,	2712,	2560,	2416,	2280,	2152,	2032,	1920,	1813,
			1712,	1616,	1524,	1440,	1356,	1280,	1208,	1140,	1076,	1016,	960,	906,
			856,	808,	762,	720,	678,	640,	604,	570,	538,	508,	480,	453,
			428,	404,	381,	360,	339,	320,	302,	285,	269,	254,	240,	226,
			214,	202,	190,	180,	170,	160,	151,	143,	135,	127,	120,	113,
			107,	101,	95,		90,		85,		80,		75,		71,		67,		63,		60,		56 
		};
*/

//////////////////////////////////////////////////////////////////////////
//	XMSampler::WaveDataController Implementation
		void XMSampler::WaveDataController::Init(XMInstrument::WaveData* wave, const int layer)
		{
			m_Layer = layer;
			m_pWave = wave;
			m_Position.QuadPart=0;
			m_Speed=0;
			m_Playing=false;

			m_pL = const_cast<short *>(wave->pWaveDataL());
			m_pR = const_cast<short *>(wave->pWaveDataR());

			_stereo=wave->IsWaveStereo();
			_length=wave->WaveLength();

			if ( SustainLoopType() != XMInstrument::WaveData::LoopType::DO_NOT)
			{
				m_CurrentLoopType = SustainLoopType();
				m_CurrentLoopStart = SustainLoopStart();
				m_CurrentLoopEnd = SustainLoopEnd();

			} else {
				m_CurrentLoopType = LoopType();
				m_CurrentLoopStart = LoopStart();
				m_CurrentLoopEnd = LoopEnd();
			}

			CurrentLoopDirection(LoopDirection::FORWARD);
		}
		void XMSampler::WaveDataController::NoteOff(void)
		{
			if ( SustainLoopType() != XMInstrument::WaveData::LoopType::DO_NOT)
			{
				m_CurrentLoopType = LoopType();
				m_CurrentLoopStart = LoopStart();
				m_CurrentLoopEnd = LoopEnd();
			}
		}


//////////////////////////////////////////////////////////////////////////
//      XMSampler::EnvelopeController Implementation
		void XMSampler::EnvelopeController::Init(XMInstrument::Envelope *pEnvelope)
		{
			if(pEnvelope != NULL){
				m_pEnvelope = pEnvelope;
			}
			m_Samples = 0;
//			m_Mode = EnvelopeMode::TICK;
			m_PositionIndex = 0;
			m_ModulationAmount = 0;
			m_Step = 0;
			m_NextEventSample=0;
			m_Stage = EnvelopeStage::OFF;
		}

		/// NoteOn EnvelopeStage
		// Explanation:
		//	First, the stage is set to off. Then, if the envelope is enabled and there are points, 
		//  we check if it has sustain or normal loop points, enabling the corresponding flags in m_Stage
		//  Second, if there are more points in the envelope, let's enable the joy!
		void XMSampler::EnvelopeController::NoteOn()
		{
			m_Samples = 0;
			m_PositionIndex = -1;
			m_NextEventSample = 0;
			m_Stage = EnvelopeStage::OFF;

			// if there are no points, there is nothing to do.
			if ( m_pEnvelope->NumOfPoints() > 0 && m_pEnvelope->IsEnabled())
			{
				m_ModulationAmount = m_pEnvelope->GetValue(0);
				if ( m_pEnvelope->SustainBegin() != XMInstrument::Envelope::INVALID )
				{
					m_Stage = EnvelopeStage(m_Stage | EnvelopeStage::HASSUSTAIN);
				}
				if (m_pEnvelope->LoopStart() != XMInstrument::Envelope::INVALID )
				{
					m_Stage = EnvelopeStage(m_Stage | EnvelopeStage::HASLOOP);
				}

				if(m_pEnvelope->GetTime(1) != XMInstrument::Envelope::INVALID )
				{ 
					// Since we do not make a CalcStep, m_NextEventSample=0 and the first "Work()" call enters
					// the check of what to do, forcing a new decision.
					m_Stage = EnvelopeStage(m_Stage | EnvelopeStage::DOSTEP);
				}
			} else { m_ModulationAmount = 1.0f; }

		}

		/// NoteOff EnvelopeStage
		// Explanation:
		// First we check if the envelope is active. If it is, we release the envelope.
		// Then if the envelope state is paused, we check if it was because of a sustain loop and in that case
		// we reenable it. The next work call, it will recheck the status and decide what does it need to do.
		void XMSampler::EnvelopeController::NoteOff()
		{
			if ( m_Stage != EnvelopeStage::OFF && !(m_Stage&EnvelopeStage::RELEASE))
			{
				m_Stage = EnvelopeStage(m_Stage | EnvelopeStage::RELEASE);

				// If we are paused, check why
				if (!(m_Stage&EnvelopeStage::DOSTEP))
				{
					if ( m_Stage&EnvelopeStage::HASSUSTAIN)
					{
						m_Stage = EnvelopeStage(m_Stage | EnvelopeStage::DOSTEP);
						m_Samples--;
						m_PositionIndex--;
					}
				}
				// disable the sustain flag.
				m_Stage = EnvelopeStage(m_Stage & ~EnvelopeStage::HASSUSTAIN);
			}
		}

		void XMSampler::EnvelopeController::RecalcDeviation()
		{
//			if ( m_Mode == EnvelopeMode::TICK )
//			{
			m_sRateDeviation = (Global::pPlayer->SampleRate() *60) / (24 * Global::pPlayer->bpm);
//			} else if ( m_Mode == EnvelopeMode::MILIS ) {
//				m_sRateDeviation = (Global::pPlayer->SampleRate() / 1000.0f;
//			}
		}

		void XMSampler::EnvelopeController::CalcStep(const int start,const int  end)
		{
			const XMInstrument::Envelope::ValueType ystep = (m_pEnvelope->GetValue(end) - m_pEnvelope->GetValue(start));
			const XMInstrument::Envelope::ValueType xstep = (m_pEnvelope->GetTime(end) - m_pEnvelope->GetTime(start));
			RecalcDeviation();
			m_NextEventSample = m_pEnvelope->GetTime(end)* SRateDeviation();
			m_Step = ystep / (xstep * SRateDeviation());
		}


//////////////////////////////////////////////////////////////////////////
//	XMSampler::Voice  Implementation 
		void XMSampler::Voice::Reset()
		{
			m_ChannelNum = -1;
			m_pChannel = NULL;

			_instrument = -1;
			m_pInstrument = NULL;

			m_AmplitudeEnvelope.Init();
			m_FilterEnvelope.Init();
			m_PitchEnvelope.Init();
			m_PanEnvelope.Init();

			m_Filter.Reset();
			m_CutOff = 127;
			m_Ressonance = 0;
			_coModify = 0;


			m_bPlay =false;
			m_Background = false;
			m_Period=0;
			m_Note = 255;
			m_Volume = 128;
			m_RealVolume = 1.0f;

			m_PanFactor=0.5f;
			m_PanRange=1;

			ResetEffects();
		}

		void XMSampler::Voice::ResetEffects()
		{
			m_Slide2NoteDestPeriod=0;
			m_PitchSlideSpeed=0;

			m_VolumeFadeSpeed = 0;
			m_VolumeFadeAmount = 1;

			m_VolumeSlideSpeed=0;

			m_VibratoSpeed=0;
			m_VibratoDepth=0;
			m_VibratoPos=0;
			m_VibratoAmount =0.0;

			m_TremoloSpeed=0;
			m_TremoloDepth=0;
			m_TremoloAmount =0.0f;
			m_TremoloPos=0;

			// Panbrello
			m_PanbrelloSpeed=0;
			m_PanbrelloDepth=0;
			m_PanbrelloAmount=0.0f;
			m_PanbrelloPos=0;
			// Tremor
			m_TremorOnTicks = 0;
			m_TremorOffTicks = 0;
			m_TremorTickChange = 0;
			m_bTremorMute = false;

			m_AutoVibratoAmount = 0.0;
			m_AutoVibratoPos = 0;
			m_AutoVibratoDepth = 0; 

			m_RetrigTicks=0;
			m_RetrigVol=0;
			m_RetrigOperation=0;

		}
		void XMSampler::Voice::VoiceInit(int channelNum, int instrumentNum)
		{
			IsBackground(false);
			srand(0);
			m_ChannelNum = channelNum;
			pChannel(&pSampler()->rChannel(channelNum));
			InstrumentNum(instrumentNum);
			XMInstrument & _inst = pSampler()->rInstrument(instrumentNum);
			m_pInstrument = &_inst;

			// Envelopes
			m_AmplitudeEnvelope.Init(_inst.AmpEnvelope());
			m_PanEnvelope.Init(_inst.PanEnvelope());
			m_PitchEnvelope.Init(_inst.PitchEnvelope());
			m_FilterEnvelope.Init(_inst.FilterEnvelope());

//			m_Filter.Init();
			m_Filter.Reset();
			m_Filter.SampleRate(Global::pPlayer->SampleRate());

			//\todo: add the missing  Random options
/*			if (_inst.RandomCutoff())
			{
				CutOff(_inst.FilterCutoff()* (float)rand() * _inst.RandomCutoff() / 3276800.0f);
			}
			else*/ if (_inst.FilterCutoff() < 127 || _inst.FilterResonance() > 0)
			{
				CutOff(_inst.FilterCutoff());
				Ressonance(_inst.FilterResonance());
				FilterType(_inst.FilterType());
			} else if ( rChannel().Cutoff() < 127 || rChannel().Ressonance() > 0)
			{
				FilterType(rChannel().FilterType());
				CutOff(rChannel().Cutoff());
				Ressonance(rChannel().Ressonance());
			}
			else
			{
				CutOff(127);Ressonance(0);
			}

			
/*			if (_inst.RandomResonance())
			{
				m_Filter._q = _inst.FilterResonance() * (float)rand()* _inst.RandomResonance() / 3276800.f;
			}
			else
			{
				m_Filter._q = _inst.FilterResonance();
			}
*/
			_coModify = (float)_inst.FilterEnvAmount()/128.0f;
//			m_Filter.Update();

			ResetEffects();
			
		}// XMSampler::Voice::VoiceInit) 

		void XMSampler::Voice::Work(int numSamples,float * pSamplesL,float * pSamplesR,dsp::Cubic& _resampler)
		{
			dsp::PRESAMPLERFN pResamplerWork;
			pResamplerWork = _resampler._pWorkFn;

			float left_output = 0.0f;
			float right_output = 0.0f;

			if (Global::_pSong->IsInvalided())
			{
				IsPlaying(false);
				return;
			}
			while (numSamples)
			{
			//////////////////////////////////////////////////////////////////////////
			//  Step 1 : Get the unprocessed wave data.

				m_WaveDataController.Work(&left_output,&right_output,pResamplerWork);

			//////////////////////////////////////////////////////////////////////////
			//  Step 2 : Process the Envelopes.

				// Amplitude Envelope 
				// Voice::RealVolume() returns the calculated volume out of "WaveData.WaveGlobVol() * Instrument.Volume() * Voice.NoteVolume()"

				float volume = RealVolume() * rChannel().Volume();
				if(m_AmplitudeEnvelope.Envelope().IsEnabled()){
					m_AmplitudeEnvelope.Work();
					volume *= m_AmplitudeEnvelope.ModulationAmount();
					if (m_AmplitudeEnvelope.Stage() == EnvelopeController::EnvelopeStage::OFF && m_VolumeFadeSpeed == 0)
					{
						NoteFadeout();
					}
				}
				// Volume Fade Out
				if(m_VolumeFadeSpeed > 0.0f)
				{
					UpdateFadeout();
					if ( m_VolumeFadeAmount <= 0) { IsPlaying(false); return; }
					volume *= m_VolumeFadeAmount;
				}
				
				// Panning Envelope 
				// (actually, the correct word for panning is panoramization. 
				//  "panning" comes from the diminutive "pan")
				float rvol =  m_PanFactor + m_PanbrelloAmount;
				if(m_PanEnvelope.Envelope().IsEnabled()){
					m_PanEnvelope.Work();

					// PanFactor() contains the pan calculated at note start ( pan of note, wave pan, instrument pan, pitchpan sep, and channel pan)
					// PanRange() is a Range delimiter, which is set when at voice init and when a panning command comes in.
					rvol += (m_PanEnvelope.ModulationAmount()*PanRange());
				}

				float lvol = 1.0f - rvol;

				// Filter section
				if (m_Filter.Type() != dsp::F_NONE)
				{
					if(m_FilterEnvelope.Envelope().IsEnabled()){
						m_FilterEnvelope.Work();
						int tmpCO = int(m_CutOff * m_FilterEnvelope.ModulationAmount());
						if (tmpCO < 0) { tmpCO = 0; }
						else if (tmpCO > 127) { tmpCO = 127; }
						m_Filter.Cutoff(tmpCO);
					}
					if ( m_pSampler->UseFilters() )
					{
						if (m_WaveDataController.IsStereo())
						{
							m_Filter.WorkStereo(left_output, right_output);
						}
						else
						{
							m_Filter.Work(left_output);
						}
					}
				}

				// Picth Envelope. Currently, the pitch envelope Amount is only updated on NewLine().
				if(m_PitchEnvelope.Envelope().IsEnabled()){
					m_PitchEnvelope.Work();
				}

				

			//////////////////////////////////////////////////////////////////////////
			//  Step 3: Add the processed data to the sampler's buffer.

				// Monoaural output‚ copy left to right output.
				if(!m_WaveDataController.IsStereo()){
					right_output = left_output;
				}

				left_output *= lvol * volume;
				right_output *= rvol * volume;
				
				*pSamplesL++ += left_output;
				if(m_pChannel->IsSurround()){
					*pSamplesR++ -= right_output;
				} else {
					*pSamplesR++ += right_output;
				}

				if (!m_WaveDataController.Playing()) {
					if ( rChannel().ForegroundVoice() == this) rChannel().ForegroundVoice(NULL);
					IsPlaying(false); return; 
				}

				numSamples--;
			}
		}
		// This one is Tracker-Tick (Mod-Tick).
		void XMSampler::Voice::Tick()
		{
			UpdateSpeed();
		}
		void XMSampler::Voice::NewLine()
		{
			m_bTremorMute = false;
			m_VibratoAmount = 0;
			UpdateSpeed();
			m_AmplitudeEnvelope.RecalcDeviation();
			m_PanEnvelope.RecalcDeviation();
			m_PitchEnvelope.RecalcDeviation();
			m_FilterEnvelope.RecalcDeviation();
		}

		void XMSampler::Voice::NoteOn(const compiler::uint8 note,const compiler::sint16 playvol,bool reset)
		{
			int wavelayer = rInstrument().NoteToSample(note).second;
			if ( pSampler()->SampleData(wavelayer).WaveLength() == 0 ) return;

			m_WaveDataController.Init(&(pSampler()->SampleData(wavelayer)),wavelayer);
			m_Note = note;
			m_Period=NoteToPeriod(rInstrument().NoteToSample(note).first);
			m_NNA = rInstrument().NNA();
			//\todo : add pInstrument().LinesMode

			//\todo: implement autovibrato.
			if ( rWave().Wave().IsAutoVibrato())
			{
			}

			if ( reset ) ResetVolAndPan(playvol);

			if(m_AmplitudeEnvelope.Envelope().IsEnabled()){
				m_AmplitudeEnvelope.NoteOn();
			}

			if(m_PanEnvelope.Envelope().IsEnabled()){
				m_PanEnvelope.NoteOn();
			}

			if(m_FilterEnvelope.Envelope().IsEnabled()){
				m_FilterEnvelope.NoteOn();
			}

			if(m_PitchEnvelope.Envelope().IsEnabled()){
				m_PitchEnvelope.NoteOn();
			}
			//Important, put it after m_PitchEnvelope.NoteOn();
			UpdateSpeed();
			m_WaveDataController.Playing(true);
			IsPlaying(true);
		}
		void XMSampler::Voice::ResetVolAndPan(compiler::sint16 playvol)
		{
			if ( playvol != -1)
			{
				Volume(playvol);
			} else { 
				Volume(rWave().Wave().WaveVolume());
			}
			// Impulse Tracker panning had the following pan controls: All these are bypassed if
			// a panning command is explicitely put in a channel.
			// Note : m_pChannel->PanFactor() returns the panFactor of the last panning command (if any) or
			// in its absence, the pan position of the channel.
			if ( rWave().Wave().PanEnabled() ) m_PanFactor = rWave().Wave().PanFactor();
			else if ( rInstrument().PanEnabled() ) m_PanFactor = rInstrument().Pan();
			else m_PanFactor = m_pChannel->PanFactor();

			//\todo : pitchpansep is in the range -32..32, being 8=one step (0..64) each seminote.
			m_PanFactor += (m_Note-rInstrument().PitchPanCenter())*rInstrument().PitchPanSep()/512.0f;
			m_PanFactor += (float)(rand()-16384.0f) * rInstrument().RandomPanning() / 1638400.0f;

			if ( m_PanFactor > 1.0f ) m_PanFactor = 1.0f;
			else if ( m_PanFactor < 0.0f ) m_PanFactor = 0.0f;

			m_PanRange = 1.0f -(fabs(0.5-m_PanFactor)*2);

		}

		void XMSampler::Voice::NoteOff()
		{
			if(!IsPlaying()){
				return;
			}

			if(m_AmplitudeEnvelope.Envelope().IsEnabled())
			{
				m_AmplitudeEnvelope.NoteOff();
			} else if ( rInstrument().VolumeFadeSpeed() >0.0f )
			{
				NoteFadeout();
			}else {
				NoteOffFast();
				return;
			}
			
			m_PanEnvelope.NoteOff();
			m_FilterEnvelope.NoteOff();
			m_PitchEnvelope.NoteOff();
			m_WaveDataController.NoteOff();
		}
		void XMSampler::Voice::NoteOffFast()
		{
			if(!IsPlaying()){
				return;
			}

			if(m_AmplitudeEnvelope.Envelope().IsEnabled()){
				m_AmplitudeEnvelope.NoteOff();
			}
			// Fade Out Volume
			m_VolumeFadeSpeed = 64.0f;
			m_VolumeFadeAmount = 1.0f;
			
			m_PanEnvelope.NoteOff();
			m_FilterEnvelope.NoteOff();
			m_PitchEnvelope.NoteOff();
			m_WaveDataController.NoteOff();
		}

		//\todo : see if this function really does what Impulse Tracker means by "NNA fadeout"
		void XMSampler::Voice::NoteFadeout()
		{
			m_VolumeFadeSpeed = m_pInstrument->VolumeFadeSpeed()/(65535.0f*m_pSampler->DeltaTick());
			m_VolumeFadeAmount = 1.0f;
			if ( RealVolume() * rChannel().Volume() == 0.0f ) IsPlaying(false);
			else if ( m_AmplitudeEnvelope.Envelope().IsEnabled() && m_AmplitudeEnvelope.ModulationAmount() == 0.0f) IsPlaying(false);
		}
		void XMSampler::Voice::UpdateFadeout()
		{
			if ( RealVolume() == 0.0f ) IsPlaying(false);
			m_VolumeFadeAmount -= m_VolumeFadeSpeed;
			if( m_VolumeFadeAmount <= 0){
				IsPlaying(false);
			}
		}

		void XMSampler::Voice::Slide2Note()
		{
			if(m_Slide2NoteDestPeriod > m_Period)
			{
				m_Period += m_PitchSlideSpeed;
				if(m_Period > m_Slide2NoteDestPeriod ){
					m_Period = m_Slide2NoteDestPeriod;
				}
				UpdateSpeed();
			}
			else if(m_Slide2NoteDestPeriod < m_Period)
			{	// m_PitchSlide is signed. (+/-)
				m_Period += m_PitchSlideSpeed;
				if(m_Period < m_Slide2NoteDestPeriod){
					m_Period = m_Slide2NoteDestPeriod;
				}
				UpdateSpeed();
			}
		}// Porta2Note() -------------------------------------------------------

		void XMSampler::Voice::Vibrato()
		{
			int vdelta = GetDelta(rChannel().VibratoType(),m_VibratoPos);

			vdelta = vdelta * m_VibratoDepth;
			m_VibratoAmount=(double)vdelta / 32.0f;
			m_VibratoPos = (m_VibratoPos - m_VibratoSpeed) & 0xFF;
			UpdateSpeed();

		}// Vibrato() -------------------------------------

		void XMSampler::Voice::Tremolo()
		{

			int vdelta = GetDelta(rChannel().TremoloType(),m_TremoloPos);

			vdelta = (vdelta * m_TremoloDepth);
			m_TremoloAmount = vdelta / 2048.0f;
			m_TremoloPos = (m_TremoloPos + m_TremoloSpeed) & 0xFF;


		};// Tremolo() -------------------------------------------

		void XMSampler::Voice::Panbrello()
		{
/*
\todo:
The random pan position can be achieved by setting the
waveform to 3 (ie. a S53 command). In this case *ONLY*, the
speed actually is interpreted as a delay in frames before
another random value is found. so S14 will be a very QUICK
panbrello, and S44 will be a slower panbrello.
*/
			int vdelta = GetDelta(rChannel().PanbrelloType(),m_PanbrelloPos);

			vdelta = vdelta * m_PanbrelloDepth;
			m_PanbrelloAmount = vdelta / 2048.0f; // 64*16*2
			m_PanbrelloPos = (m_PanbrelloPos + m_PanbrelloSpeed) & 0xFF;


		}// Panbrello() -------------------------------------------

		void XMSampler::Voice::Tremor()
		{
			//\todo: according to Impulse Tracker, this command uses its own counter, so being with
			// speed 3, we can specify the command I41 ( x/y > speed), which, with the current implementation, doesn't work,
			if ( pSampler()->CurrentTick() >= m_TremorTickChange ) 
			{
				if ( m_bTremorMute )
				{
					m_TremorTickChange = pSampler()->CurrentTick()+m_TremorOnTicks;
					m_bTremorMute = false;
				}
				else
				{
					m_TremorTickChange = pSampler()->CurrentTick()+m_TremorOffTicks;
					m_bTremorMute = true;
				}
			}
		}

		void XMSampler::Voice::VolumeSlide()
		{
			if(m_VolumeSlideSpeed >0){
				VolumeUp(m_VolumeSlideSpeed);
			} else {
				VolumeDown(m_VolumeSlideSpeed);
			}
		}
		void XMSampler::Voice::VolumeDown(const int value){
			int vol = Volume() +value;
			if(vol < 0){
				vol = 0;
			}
			Volume(vol);
		}
		void XMSampler::Voice::VolumeUp(const int value){
			int vol = Volume() +value;
			if(vol > 255){
				vol = 255;
			}
			Volume(vol);	
		}

		void XMSampler::Voice::Retrig()
		{

		}
		void XMSampler::Voice::AutoVibrato()
		{
			/*			
			int vdelta = GetDelta(_inst.AutoVibratoType(),m_AutoVibratoPos & 0x3F);

			if(_inst.AutoVibratoSweep())
			{
				m_AutoVibratoDepth += (_inst.AutoVibratoDepth() << 8) / _inst.AutoVibratoSweep();
				if((m_AutoVibratoDepth >> 8) > _inst.AutoVibratoDepth()){
					m_AutoVibratoDepth = _inst.AutoVibratoDepth() << 8;
				}
			} else {
				m_AutoVibratoDepth = _inst.AutoVibratoDepth() << 8;
			}

			vdelta = ((vdelta * m_AutoVibratoDepth) >> 14);

			rChannel().AutoVibratoAmount((double)vdelta);

			m_AutoVibratoPos = (m_AutoVibratoPos + _inst.AutoVibratoRate());
			*/

		}//XMSampler::Voice::AutoVibrato() -------------------------------------


		int XMSampler::Voice::GetDelta(int wavetype,int wavepos)
		{
			switch (wavetype)
			{
			case XMInstrument::WaveData::WaveForms::SAWDOWN:
				return m_FineRampDownData[wavepos];
			case XMInstrument::WaveData::WaveForms::SAWUP:
				return m_FineRampDownData[0x3F - wavepos];
			case XMInstrument::WaveData::WaveForms::SQUARE:
				return m_FineSquareTable[wavepos];
			case XMInstrument::WaveData::WaveForms::RANDOM:
				return m_RandomTable[wavepos];
			case XMInstrument::WaveData::WaveForms::SINUS:
			default:
				return m_FineSineData[wavepos];
			}
		}

		void XMSampler::Voice::UpdateSpeed()
		{
			double _period = m_Period;

			if(rChannel().IsArpeggio()){
				_period = rChannel().ArpeggioPeriod();
			}

			if(rChannel().IsGrissando())
			{
				int _note = PeriodToNote(_period);
				_period = NoteToPeriod(_note);
			}
			_period = _period + AutoVibratoAmount() + VibratoAmount() - (m_PitchEnvelope.ModulationAmount()*1024.0f);
			if ( _period > 65535) _period = 65535;
			else if ( _period < 32 ) _period = 32;

			rWave().Speed(PeriodToSpeed(_period));
		}

		double XMSampler::Voice::PeriodToSpeed(int period)
		{
			if(m_pSampler->IsAmigaSlides()){
				// amiga period mode
				// 14317456 = 8363Hz* 1712(center-period), got from xm-form.txt
				// In fs3mdoc there's the value 14317056 , which i assume wrong.
				// and still, in fmoddoc there's the value 7159090.5, which attains for
				//  856 (center-period/2) * 8363.42....Hz.
				// So the question is if mod is incorrect or XM is rounded.
				return ( 14317456  / period ) / (double)Global::pPlayer->SampleRate();
			} else {
				// Linear Frequency
				// 8363*2^((5*12*64 - Period) / (12*64))
				// 8363=Hz for Middle-C note
				// 12*64 = 12 notes * 64 finetune steps.
				// 5 = 10-middle_C ( if C-4 is middle_C, then, 6*12*64, if C-3, then 7*12*64, etc..)
				return	pow(2.0,
							(3840 - period ) /768.0
						)
						* 8363 / (double)Global::pPlayer->SampleRate();
			}
		}

		const double XMSampler::Voice::NoteToPeriod(const int note)
		{
			XMInstrument::WaveData& _wave = m_pSampler->m_rWaveLayer[rWave().Layer()];

			if(m_pSampler->IsAmigaSlides())
			{
				// Amiga Period . Nonstandard table, but *maybe* more accurate.
				double speedfactor =  pow(2.0,(_wave.WaveTune()+(_wave.WaveFineTune()/256.0))/12.0);
//				double c5speed =  8363.0*speedfactor;
				return AmigaPeriod[note]/speedfactor;
			} else {
				// 7680 = 12notes*10octaves*64fine.
				return 7680 - ((double)(note + _wave.WaveTune()) * 64.0)
					- ((double)(_wave.WaveFineTune()) * 0.25); // 0.25 since the range is +-256 for XMSampler as opposed to +-128 for FT.
			}
		};

		const int XMSampler::Voice::PeriodToNote(const double period)
		{
			XMInstrument::WaveData& _wave = m_pSampler->m_rWaveLayer[rWave().Layer()];

			if(m_pSampler->IsAmigaSlides()){
				// f1
				//period =  pow(2.0,double(15.74154-(note/12.0f)))
				// log2(period) = 15.74154 - (note+wavetune+(finetune/256.0f))/12.0f
				// note = (15.74154 - log2(period))*12 - tune - (fine/256)
				//f2
				//period = pow(2.0,(116.898 - ((double)(note + _wave.WaveTune()) + ((double)_wave.WaveFineTune() / 128.0))/12.0) * 32;
				//log2(period/32) = (116.898 - (double)note - (double)_wave.WaveTune() + ((double)_wave.WaveFineTune() / 128.0))/12.0;
				//log2(period/32)*12 =  116.898 - (double)note - (double)_wave.WaveTune() + ((double)_wave.WaveFineTune() / 128.0)
				//note = 116.898 - (double)_wave.WaveTune() + ((double)_wave.WaveFineTune() / 128.0) - (log2(period/32)*12); 
				int _note = (int)(116.898 - (double)_wave.WaveTune() - ((double)_wave.WaveFineTune() / 256.0) 
					-(12.0 * log((double)period / 32.0)/(0.301029995f /*log(2)*/ )));
				return _note+12;
			} else {
				// period = ((10.0 * 12.0 * 64.0 - ((double)note + (double)_wave.WaveTune()) * 64.0)
				//	- (_wave.WaveFineTune() / 256.0) * 64.0);
				// period / 64.0 = 10.0 * 12.0  - ((double)note + (double)_wave.WaveTune()) - _wave.WaveFineTune() / 256.0;
				// note = (int)(10.0 * 12.0  - (double)_wave.WaveTune() - _wave.WaveFineTune() / 256.0 - period / 64.0 + 0.5);

				return (int)(120 - (double)_wave.WaveTune() - ((double)_wave.WaveFineTune() / 256.0)  - (period / 64.0)); // Apparently,  (int)(x.5) rounds to x+1, so no need for +0.5
			}
		}

//////////////////////////////////////////////////////////////////////////
// Channel Class Implementation 
//////////////////////////////////////////////////////////////////////////
		void XMSampler::Channel::Init()
		{
			m_InstrumentNo = 255;
			m_pForegroundVoice = NULL;

			m_Note = 255;
			m_Period = 0;

			m_Volume = 1.0f;
			m_ChannelDefVolume = 64;//

			m_PanFactor = 0.5f;
			m_DefaultPanFactor = 32;
			m_bSurround = false;

			m_bGrissando = false;
			m_VibratoType = XMInstrument::WaveData::WaveForms::SINUS;
			m_TremoloType = XMInstrument::WaveData::WaveForms::SINUS;
			m_PanbrelloType = XMInstrument::WaveData::WaveForms::SINUS;

			m_PanSlideMem = 0;
			m_ChanVolSlideMem = 0;
			m_PitchSlideMem = 0;
			m_TremorMem = 0;
			m_TremorOnTime = 0;
			m_TremorOffTime = 0;
			m_VibratoDepthMem = 0;
			m_VibratoSpeedMem = 0;
			m_TremoloDepthMem = 0;
			m_TremoloSpeedMem = 0;
			m_PanbrelloDepthMem = 0;
			m_PanbrelloSpeedMem = 0;
			m_VolumeSlideMem = 0;
			m_ArpeggioMem = 0;

			m_EffectFlags = 0;

			m_PitchSlideSpeed = 0;
			m_Slide2NoteDestNote = 0;

			m_GlobalVolSlideSpeed = 0.0f;
			m_ChanVolSlideSpeed = 0.0f;
			m_PanSlideSpeed = 0.0f;

/*			m_VibratoSpeed = 0;
			m_VibratoDepth = 0;
			m_VibratoPos = 0;
			m_VibratoAmount = 0;
			m_AutoVibratoAmount = 0;
*/
			m_TremoloSpeed = 0;
			m_TremoloDepth = 0;
			m_TremoloDelta = 0;
			m_TremoloPos = 0;

			m_PanbrelloSpeed = 0;
			m_PanbrelloDepth = 0;
			m_PanbrelloDelta = 0;
			m_PanbrelloPos = 0;

			m_ArpeggioPeriod[0] = 0.0;
			m_ArpeggioPeriod[1] = 0.0;

			m_NoteCutTick = 0;

			m_MIDI_Set = 0;
			m_DefaultCutoff= 127;
			m_DefaultRessonance =0;
			m_DefaultFilterType = dsp::F_NONE;

		}

		void XMSampler::Channel::EffectInit()
		{
/*			m_VibratoPos = 0;
			m_TremoloPos = 0;
			m_TremoloDepth = 0;
			m_VibratoAmount = 0;
			m_AutoVibratoAmount = 0;
			m_PanbrelloPos = 0;
*/
		}
		void XMSampler::Channel::Restore()
		{
			m_Volume = m_ChannelDefVolume/64.0f;

			if ( m_DefaultPanFactor != 80 ) m_PanFactor = (m_DefaultPanFactor&0x7F)/64.0f;
			else if ( m_DefaultPanFactor == 80) m_bSurround = true;

			m_PanSlideMem = 0;
			m_ChanVolSlideMem = 0;
			m_PitchSlideMem = 0;
			m_TremorMem = 0;
			m_TremorOnTime = 0;
			m_TremorOffTime = 0;
			m_VibratoDepthMem = 0;
			m_VibratoSpeedMem = 0;
			m_TremoloDepthMem = 0;
			m_TremoloSpeedMem = 0;
			m_PanbrelloDepthMem = 0;
			m_PanbrelloSpeedMem = 0;
			m_VolumeSlideMem = 0;
			m_ArpeggioMem = 0;
			m_GlobalVolSlideMem = 0;
			m_Cutoff = m_DefaultCutoff;
			m_Ressonance = m_DefaultRessonance;
			m_FilterType = m_DefaultFilterType;

		}
		void XMSampler::Channel::SetEffect(Voice* voice,int volcmd,int cmd,int parameter)
		{
			int realSet=0;
			int	realValue=0;

			//1st check: Channel ( They can appear without an existing playing note and are persistent when a new one comes)
			switch(volcmd&0xF0)
			{
			case CMD_VOL::VOL_PANNING:
				PanFactor((volcmd&0x0F)/15.0f);
				break;
			case CMD_VOL::VOL_PANSLIDELEFT:
				PanningSlide((volcmd&0x0F)<<4);
				break;
			case CMD_VOL::VOL_PANSLIDERIGHT:
				PanningSlide(volcmd&0x0F);
				break;
			default:
				break;
			}

			switch(cmd)
			{
			case CMD::PANNING:
				PanFactor(parameter/255.0f);
				break;
			case CMD::SET_CHANNEL_VOLUME:
				Volume((parameter<64)?(parameter/64.0f):1.0f);
				break;
			case CMD::PANNINGSLIDE:
				PanningSlide(parameter);
				break;
			case CMD::CHANNEL_VOLUME_SLIDE:
				ChannelVolumeSlide(parameter);
				break;
			case CMD::SET_GLOBAL_VOLUME:
				m_pSampler->GlobalVolume(parameter<0x80?parameter:0x80);
				break;
			case CMD::GLOBAL_VOLUME_SLIDE:
				GlobalVolSlide(parameter);
				break;
			case CMD::EXTENDED:
				switch(parameter&0xF0)
				{
				case CMD_E::E9:
					switch(parameter&0x0F)
					{
					case CMD_E9::E9_SURROUND_OFF:
						IsSurround(false);
						break;
					case CMD_E9::E9_SURROUND_ON:
						IsSurround(true);
						break;
					case CMD_E9::E9_REVERB_OFF:
						break;
					case CMD_E9::E9_REVERB_FORCE:
						break;
					case CMD_E9::E9_STANDARD_SURROUND:
						break;
					case CMD_E9::E9_QUAD_SURROUND:
						break;
					case CMD_E9::E9_GLOBAL_FILTER:
						break;
					case CMD_E9::E9_LOCAL_FILTER:
						break;
					default:
						break;
					}
					break;
				case CMD_E::E_SET_PAN:
					PanFactor((parameter&0xf)/15.0f);
					break;
				case CMD_E::E_SET_MIDI_MACRO:
					m_MIDI_Set = parameter;
					//\todo : implement.
					break;
				case CMD_E::E_GLISSANDO_TYPE:
					IsGrissando(parameter != 0);
					break;
				case CMD_E::E_VIBRATO_WAVE:
					VibratoType(parameter);
					break;
				case CMD_E::E_PANBRELLO_WAVE:
					PanbrelloType(parameter);
					break;
				case CMD_E::E_TREMOLO_WAVE:
					TremoloType(parameter);
					break;
				default:
					break;
				}
				break;
			case CMD::MIDI_MACRO:
				if ( parameter < 0x80)
				{
					realSet = m_MIDI_Set;
					realValue = parameter;
				}
				else
				{
					realSet = m_pSampler->GetMap(parameter-0x80).mode;
					realValue = m_pSampler->GetMap(parameter-0x80).value;
				}
				switch(realSet)
				{
				case 0:
					m_Cutoff=realValue;
					if ( m_FilterType == dsp::F_NONE) m_FilterType = dsp::F_LOWPASS12;
					if ( voice ) 
					{
						voice->FilterType(m_FilterType);
						voice->CutOff(m_Cutoff);
					}
					break;
				case 1:
					m_Ressonance=realValue;
					if ( m_FilterType == dsp::F_NONE) m_FilterType = dsp::F_LOWPASS12;
					if ( voice )
					{
						voice->FilterType(m_FilterType);
						voice->Ressonance(m_Ressonance);
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
			if ( voice )
			{
				switch(volcmd&0xF0)
				{
				case CMD_VOL::VOL_VOLUME0:
				case CMD_VOL::VOL_VOLUME1:
				case CMD_VOL::VOL_VOLUME2:
				case CMD_VOL::VOL_VOLUME3:
					voice->Volume(volcmd<<1);
					break;
				case CMD_VOL::VOL_VOLSLIDEUP:
					VolumeSlide((volcmd&0x0F)<<4);
					break;
				case CMD_VOL::VOL_VOLSLIDEDOWN:
					VolumeSlide(volcmd&0x0F);
					break;
				case CMD_VOL::VOL_PITCH_SLIDE_DOWN:
					PitchSlide(false,volcmd&0x0F);
					break;
				case CMD_VOL::VOL_PITCH_SLIDE_UP:
					PitchSlide(true,volcmd&0x0F);
					break;
				case CMD_VOL::VOL_VIBRATO_SPEED:
					Vibrato(volcmd&0x0F,0); //\todo: vibrato_speed does not activate the vibrato if it isn't running.
					break;
				case CMD_VOL::VOL_VIBRATO:
					Vibrato(0,(volcmd & 0x0F)<<2);
					break;
				case CMD_VOL::VOL_TONEPORTAMENTO:
					PitchSlide(voice->Period()>voice->NoteToPeriod(Slide2NoteDestNote()),volcmd&0x0F,Slide2NoteDestNote());
					break;
				default:
					break;
				}

				switch(cmd)
				{
				// Class A: Voice ( They can apply to an already playing voice, or a new coming one).
				case CMD::VOLUME:
					voice->Volume(parameter);
					break;
				case CMD::SET_ENV_POSITION:
					voice->AmplitudeEnvelope().SetPosition(parameter);
					voice->PanEnvelope().SetPosition(parameter);
					voice->PitchEnvelope().SetPosition(parameter);
					voice->FilterEnvelope().SetPosition(parameter);
					break;
				case CMD::EXTENDED:
					switch(parameter&0xF0)
					{
					case CMD_E::E9:
						switch(parameter&0x0F)
						{
						case CMD_E9::E9_PLAY_FORWARD:
							voice->rWave().CurrentLoopDirection(WaveDataController::LoopDirection::FORWARD);
							break;
						case CMD_E9::E9_PLAY_BACKWARD:
							voice->rWave().CurrentLoopDirection(WaveDataController::LoopDirection::BACKWARD);
							break;
						}
						break;
					case CMD_E::EE:
						switch(parameter&0x0F)
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
						case CMD_EE::EE_BACKGROUNDNOTECUT:
							// search bacground notes on this channel and notecut them
							break;
						case CMD_EE::EE_BACKGROUNDNOTEOFF:
							// search bacground notes on this channel and noteoff them
							break;
						case CMD_EE::EE_BACKGROUNDNOTEFADE:
							// search bacground notes on this channel and notefade them
							break;
						}
						break;
					case CMD_E::E_DELAYED_NOTECUT:
						NoteCut(parameter&0x0F);
						break;
					}
					break;

				// Class B Channel ( Just like Class A, but remember its old value if it is called again with  00 as parameter  )

				case CMD::PORTAMENTO_UP:
					PitchSlide(true,parameter);
					break;
				case CMD::PORTAMENTO_DOWN:
					PitchSlide(false,parameter);
					break;
				case CMD::PORTA2NOTE:
					PitchSlide(voice->Period()>voice->NoteToPeriod(Slide2NoteDestNote()),parameter,Slide2NoteDestNote());
					break;
				case CMD::VOLUMESLIDE:
					VolumeSlide(parameter);
					break;
				case CMD::TONEPORTAVOL:
					VolumeSlide(parameter);
					PitchSlide(voice->PeriodToNote(voice->Period())<Slide2NoteDestNote(),0,Slide2NoteDestNote());
					break;
				case CMD::VIBRATOVOL:
					VolumeSlide(parameter);
					Vibrato(0);
					break;
				case CMD::VIBRATO:
					Vibrato(((parameter >> 4) & 0x0F),(parameter & 0x0F)<<2);
					break;
				case CMD::FINE_VIBRATO:
					Vibrato(((parameter >> 4) & 0x0F),(parameter & 0x0F));
					break;
				case CMD::TREMOR:
					Tremor(parameter);
					break;
				case CMD::TREMOLO:
					Tremolo((parameter>> 4) & 0x0F,(parameter & 0x0F));
					break;
				case CMD::RETRIG:
					Retrigger((parameter & 0x0F),(parameter>> 4) & 0x0F);
					break;
				case CMD::PANBRELLO:
					Panbrello((parameter>> 4) & 0x0F,(parameter & 0x0F));
					break;
				case CMD::ARPEGGIO:
					Arpeggio(parameter);
					break;
				}
			}
			//3nd check: It is not needed that the voice is playing, but it applies to the last instrument.
			if ( InstrumentNo() != 255 && cmd == CMD::EXTENDED && (parameter&0xF0) == CMD_E::EE)
			{
				switch(parameter&0x0F)
				{
				case CMD_EE::EE_VOLENVOFF:
					m_pSampler->rInstrument(InstrumentNo()).AmpEnvelope()->IsEnabled(false);
					break;
				case CMD_EE::EE_VOLENVON:
					m_pSampler->rInstrument(InstrumentNo()).AmpEnvelope()->IsEnabled(true);
					break;
				case CMD_EE::EE_PANENVOFF:
					m_pSampler->rInstrument(InstrumentNo()).PanEnvelope()->IsEnabled(false);
					break;
				case CMD_EE::EE_PANENVON:
					m_pSampler->rInstrument(InstrumentNo()).PanEnvelope()->IsEnabled(true);
					break;
				case CMD_EE::EE_PITCHENVON:
					m_pSampler->rInstrument(InstrumentNo()).PitchEnvelope()->IsEnabled(false);
					break;
				case CMD_EE::EE_PITCHENVOFF:
					m_pSampler->rInstrument(InstrumentNo()).PitchEnvelope()->IsEnabled(true);
					break;
				}
			}
		}

		// Add Here those commands that have an effect each tracker tick ( 1 and onwards) .
		// tick "0" is worked on in channel.SetEffect();
		void XMSampler::Channel::PerformFx()
		{
			for ( int i=0;i< m_pSampler->NumVoices();i++)
			{
				if ( m_pSampler->rVoice(i).ChannelNum() == m_Index && m_pSampler->rVoice(i).IsPlaying())  m_pSampler->rVoice(i).Tick();
			}
			if(ForegroundVoice()) // Effects that need a voice to be active.
			{
				if(ForegroundVoice()->IsAutoVibrato())
				{
					ForegroundVoice()->AutoVibrato();
				}
				if(EffectFlags() & EffectFlag::PITCHSLIDE)
				{
					ForegroundVoice()->PitchSlide();
				}
				if(EffectFlags() & EffectFlag::SLIDE2NOTE)
				{
					ForegroundVoice()->Slide2Note();
				}
				if(EffectFlags() & EffectFlag::VIBRATO)
				{
					ForegroundVoice()->Vibrato();
				}
				if(EffectFlags() & EffectFlag::TREMOLO)
				{
					ForegroundVoice()->Tremolo();
				}
				if(EffectFlags() & EffectFlag::PANBRELLO)
				{
					ForegroundVoice()->Panbrello();
				}
				if(EffectFlags() & EffectFlag::TREMOR)
				{
					ForegroundVoice()->Tremor();
				}
				if(EffectFlags() & EffectFlag::VOLUMESLIDE)
				{
					ForegroundVoice()->VolumeSlide();
				}
				if(EffectFlags() & EffectFlag::NOTECUT)
				{
					NoteCut();
				}
				if(EffectFlags() & EffectFlag::ARPEGGIO)
				{
					ForegroundVoice()->UpdateSpeed();
				}
				if(EffectFlags() & EffectFlag::RETRIG)
				{
					ForegroundVoice()->Retrig();
				}
			}
			if(EffectFlags() & EffectFlag::CHANNELVOLSLIDE)
			{
				ChannelVolumeSlide();
			}
			if(EffectFlags()  & EffectFlag::PANSLIDE)
			{
				PanningSlide();
			}
			if(EffectFlags() & EffectFlag::NOTEDELAY)
			{	
				if(m_pSampler->CurrentTick() == m_NoteCutTick)
				{
					m_pSampler->Tick(m_Index,&m_DelayedNote);
				}
			}
			if(EffectFlags() & EffectFlag::GLOBALVOLSLIDE)
			{
				m_pSampler->SlideVolume(m_GlobalVolSlideSpeed);
			}

		}

		void XMSampler::Channel::GlobalVolSlide(int speed)
		{
			if(speed == 0){
				if ( m_GlobalVolSlideMem == 0 ) return;
				speed = m_GlobalVolSlideMem;
			}
			else m_GlobalVolSlideMem = speed;

			if ( (speed & 0x0F) == 0 ){ // Slide up
				speed = (speed & 0xF0)>>4;
				m_EffectFlags |= EffectFlag::GLOBALVOLSLIDE;
				m_GlobalVolSlideSpeed = speed/128.0f;
				if (speed == 0xF ) m_pSampler->SlideVolume(m_GlobalVolSlideSpeed);
			}
			else if ( (speed & 0xF0) == 0 )  { // Slide down
				speed = (speed & 0x0F);
				m_EffectFlags |= EffectFlag::GLOBALVOLSLIDE;
				m_GlobalVolSlideSpeed = -speed/128.0f;
				if (speed == 0xF ) m_pSampler->SlideVolume(m_GlobalVolSlideSpeed);
			}
			else if ( (speed & 0x0F) == 0xF ) { // FineSlide up
				m_GlobalVolSlideSpeed = ((speed & 0xF0)>>4);
				m_pSampler->SlideVolume(m_GlobalVolSlideSpeed);
			} 
			else if ( (speed & 0xF0) == 0xF0 ) { // FineSlide down
				m_GlobalVolSlideSpeed = -(speed & 0x0F);
				m_pSampler->SlideVolume(m_GlobalVolSlideSpeed);
			}
		};
		void XMSampler::Channel::PanningSlide(int speed)
		{
			if(speed == 0){
				if ( m_PanSlideMem == 0 ) return;
				speed = m_PanSlideMem;
			}
			else m_PanSlideMem = speed;

			if ( (speed & 0x0F) == 0 ){ // Slide Left
				speed = (speed & 0xF0)>>4;
				m_EffectFlags |= EffectFlag::PANSLIDE;
				m_PanSlideSpeed = -speed/64.0f;
				if (speed == 0xF ) PanningSlide();
			}
			else if ( (speed & 0xF0) == 0 )  { // Slide Right
				speed = (speed & 0x0F);
				m_EffectFlags |= EffectFlag::PANSLIDE;
				m_PanSlideSpeed = speed/64.0f;
				if (speed == 0xF ) PanningSlide();
			}
			else if ( (speed & 0x0F) == 0xF ) { // FineSlide left
				m_PanSlideSpeed = -((speed & 0xF0)>>4)/64.0f;
				PanningSlide();
			} 
			else if ( (speed & 0xF0) == 0xF0 ) { // FineSlide right
				m_PanSlideSpeed = (speed & 0x0F)/64.0f;
				PanningSlide();
			}
		};
		void XMSampler::Channel::ChannelVolumeSlide(int speed)
		{
			if(speed == 0){
				if ( m_ChanVolSlideMem == 0 ) return;
				speed = m_ChanVolSlideMem;
			}
			else m_ChanVolSlideMem = speed;

			if ( (speed & 0x0F) == 0 ){ // Slide up
				speed = (speed & 0xF0)>>4;
				m_EffectFlags |= EffectFlag::CHANNELVOLSLIDE;
				m_ChanVolSlideSpeed = speed/64.0f;
				if (speed == 0xF ) ChannelVolumeSlide();
			}
			else if ( (speed & 0xF0) == 0 )  { // Slide down
				speed = (speed & 0x0F);
				m_EffectFlags |= EffectFlag::CHANNELVOLSLIDE;
				m_ChanVolSlideSpeed = -speed/64.0f;
				if (speed == 0xF ) ChannelVolumeSlide();
			}
			else if ( (speed & 0x0F) == 0xF ) { // FineSlide up
				m_ChanVolSlideSpeed = ((speed & 0xF0)>>4)/64.0f;
				ChannelVolumeSlide();
			} 
			else if ( (speed & 0xF0) == 0xF0 ) { // FineSlide down
				m_ChanVolSlideSpeed = -(speed & 0x0F)/64.0f;
				ChannelVolumeSlide();
			}
		};
		void XMSampler::Channel::PitchSlide(bool bUp,int speed,int note)
		{
			if ( speed == 0 ) {
				if ( m_PitchSlideMem == 0 ) return;
				speed = m_PitchSlideMem;
			}
			else m_PitchSlideMem = speed &0xff;

			if ( speed < 0xE0 || note !=255)	// Portamento , Fine porta ("f0", and Extra fine porta "e0" ) (*)
			{									// Porta to note does not have Fine.
				speed<<=2;
				ForegroundVoice()->m_PitchSlideSpeed= bUp?-speed:speed;
				if ( note != 255 ) 
				{
					ForegroundVoice()->m_Slide2NoteDestPeriod = ForegroundVoice()->NoteToPeriod(note);
					m_EffectFlags |= EffectFlag::SLIDE2NOTE;
				}
				else m_EffectFlags |= EffectFlag::PITCHSLIDE;
			} else if ( speed < 0xF0 ) {
				speed= speed&0xf;
				ForegroundVoice()->m_PitchSlideSpeed= bUp?-speed:speed;
				ForegroundVoice()->PitchSlide();
			} else  {
				speed= (speed&0xf)<<2;
				ForegroundVoice()->m_PitchSlideSpeed= bUp?-speed:speed;
				ForegroundVoice()->PitchSlide();
			}
		}

		void XMSampler::Channel::VolumeSlide(int speed)
		{
			if(speed == 0){
				if ( m_VolumeSlideMem == 0 ) return;
				speed = m_VolumeSlideMem;
			}
			else m_VolumeSlideMem = speed;

			if ( (speed & 0x0F) == 0 ){ // Slide Up
				speed = (speed & 0xF0)>>4;
				m_EffectFlags |= EffectFlag::VOLUMESLIDE;
				ForegroundVoice()->m_VolumeSlideSpeed = speed<<1;
				if (speed == 0xF ) ForegroundVoice()->VolumeSlide();
			}
			else if ( (speed & 0xF0) == 0 )  { // Slide Down
				speed = (speed & 0x0F);
				m_EffectFlags |= EffectFlag::VOLUMESLIDE;
				ForegroundVoice()->m_VolumeSlideSpeed = -(speed<<1);
				if (speed == 0xF ) ForegroundVoice()->VolumeSlide();
			}
			else if ( (speed & 0x0F) == 0xF ) { // FineSlide Up
				ForegroundVoice()->m_VolumeSlideSpeed = (speed & 0xF0)>>3;
				ForegroundVoice()->VolumeSlide();
			} 
			else if ( (speed & 0xF0) == 0xF0 ) { // FineSlide Down
				ForegroundVoice()->m_VolumeSlideSpeed = -((speed & 0x0F)<<1);
				ForegroundVoice()->VolumeSlide();
			}
		};

		void XMSampler::Channel::Tremor(int parameter)
		{
			if(parameter == 0){
				if (m_TremorMem == 0 ) return;
				parameter = m_TremorMem;
			}
			else m_TremorMem = parameter;

			ForegroundVoice()->m_TremorOnTicks = ((parameter >> 4) & 0xF) + 1;
			ForegroundVoice()->m_TremorOffTicks = (parameter & 0xF) + 1;
			ForegroundVoice()->m_TremorTickChange = ForegroundVoice()->m_TremorOnTicks;
			m_EffectFlags |= EffectFlag::TREMOR;
		};
		void XMSampler::Channel::Vibrato(int speed,int depth)
		{
			if(depth == 0){
				if ( m_VibratoDepthMem == 0 ) return;
				depth = m_VibratoDepthMem;
			}
			else m_VibratoDepthMem = depth;

			if(speed == 0){
				if ( m_VibratoSpeedMem == 0 ) return;
				speed = m_VibratoSpeedMem;
			}
			else m_VibratoSpeedMem = speed;

			ForegroundVoice()->m_VibratoSpeed=speed<<2;
			ForegroundVoice()->m_VibratoDepth=depth;
			m_EffectFlags |= EffectFlag::VIBRATO;

		}// XMSampler::Voice::Vibrato(const int depth,const int speed) ------------------------
		void XMSampler::Channel::Tremolo(int speed,int depth)
		{
			if(depth == 0){
				if ( m_TremoloDepthMem == 0 ) return;
				depth = m_TremoloDepthMem;
			}
			else m_TremoloDepthMem = depth;

			if(speed == 0){
				if ( m_TremoloSpeedMem == 0 ) return;
				speed = m_TremoloSpeedMem;
			}
			else m_TremoloSpeedMem = speed;

			ForegroundVoice()->m_TremoloSpeed=speed<<2;
			ForegroundVoice()->m_TremoloDepth=depth;
			m_EffectFlags |= EffectFlag::TREMOLO;
		};
		void XMSampler::Channel::Panbrello(int speed,int depth)
		{
			if(depth == 0){
				if ( m_PanbrelloDepthMem == 0 ) return;
				depth = m_PanbrelloDepthMem;
			}
			else m_PanbrelloDepthMem = depth;

			if(speed == 0){
				if ( m_PanbrelloSpeedMem == 0 ) return;
				speed = m_PanbrelloSpeedMem;
			}
			else m_PanbrelloSpeedMem = speed;

			ForegroundVoice()->m_PanbrelloSpeed=speed<<2;
			ForegroundVoice()->m_PanbrelloDepth=depth;
			m_EffectFlags |= EffectFlag::PANBRELLO;
		};

		void XMSampler::Channel::Arpeggio(int param)
		{
			if ( param != 0 )
			{
				m_ArpeggioMem = param;
			}
			else param = m_ArpeggioMem;
			m_ArpeggioPeriod[0] = ForegroundVoice()->NoteToPeriod(Note() + ((param & 0xf0) >> 4));
			m_ArpeggioPeriod[1] = ForegroundVoice()->NoteToPeriod(Note() + (param & 0xf));
			m_EffectFlags |= EffectFlag::ARPEGGIO;
		};
		void XMSampler::Channel::Retrigger(const int ticks,const int volumeModifier)
		{
			int effretVol,effretMode;
			switch (volumeModifier) 
			{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5: effretVol = -(int)pow(2,volumeModifier-1); effretMode=1; break;
			case 6: effretVol = 0.66666666f;	 effretMode=2; break;
			case 7: effretVol = 0.5f;			 effretMode=2; break;
			case 9:
			case 10:
			case 11:
			case 12:
			case 13: effretVol = (int)pow(2,volumeModifier - 9); effretMode=1; break;
			case 14: effretVol = 1.5f;effretMode = 2; break;
			case 15: effretVol = 2.0f;effretMode = 2; break;
			case 0:
			case 8:	
			default: effretVol = 0; effretMode=0; break;
			}
			ForegroundVoice()->m_RetrigTicks = ticks;
			ForegroundVoice()->m_RetrigVol = effretVol;
			ForegroundVoice()->m_RetrigOperation = effretMode;
			m_EffectFlags &= EffectFlag::RETRIG;
		}
		void XMSampler::Channel::NoteCut(const int ntick){
			m_NoteCutTick = ntick;
			if(ntick == 0){
				Volume(0);
				return;
			}
			m_EffectFlags |= EffectFlag::NOTECUT;
		}
		void XMSampler::Channel::DelayedNote(PatternEntry data)
		{
			m_NoteCutTick=data._parameter&0x0f;
			data._cmd=XMSampler::CMD::NONE;
			data._parameter=0;
			m_DelayedNote=data;
			m_EffectFlags |= EffectFlag::NOTEDELAY;
		}

		void XMSampler::Channel::PanningSlide(){
			m_PanFactor += m_PanSlideSpeed;
			if(m_PanFactor < 0.0f) {	m_PanFactor = 0.0f;}
			else if(m_PanFactor > 1.0f){m_PanFactor = 1.0f;}
			if (ForegroundVoice()) ForegroundVoice()->PanFactor(m_PanFactor);
		}
		void XMSampler::Channel::ChannelVolumeSlide(){
			m_Volume += m_ChanVolSlideSpeed;
			if(m_Volume < 0.0f){	m_Volume = 0.0f;}
			else if(m_Volume > 1.0f){m_Volume = 1.0f;}
		}
		void XMSampler::Channel::NoteCut()
		{
			if(m_pSampler->CurrentTick() == m_NoteCutTick)
			{
				if (ForegroundVoice()) ForegroundVoice()->Volume(0);
				m_EffectFlags &= ~EffectFlag::NOTECUT;

			}
		}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///    XMSampler 

		XMSampler::XMSampler(int index)
		{
			_macIndex = index;
			_numPars = 0;
			_type = MACH_XMSAMPLER;
			_mode = MACHMODE_GENERATOR;
			_stprintf(_editName, _T(_psName));

			_resampler.SetQuality(dsp::R_LINEAR);

			m_GlobalVolume = 128;
			_sampleCounter = 0;
			m_NextSampleTick = 0;
			m_TickCount = 0;
			m_bAmigaSlides = false;
			m_UseFilters = true;

			int i;
			for (i = 0; i < XMSampler::MAX_POLYPHONY; i++)
			{
				m_Voices[i].pSampler(this);
				m_Voices[i].Reset();
			}

			for (i = 0; i < MAX_TRACKS; i++)
			{
				m_Channel[i].pSampler(this);
				m_Channel[i].Index(i);
			}

			for(i = 0; i < 128;i++)
			{
				zxxMap[i].mode=0;
				zxxMap[i].value=0;
			}
		}

		void XMSampler::Init(void)
		{
			Machine::Init();

			_numVoices = XMSampler::MAX_POLYPHONY;
			int i;
			for (i=0; i < _numVoices; i++)
			{
				m_Voices[i].Reset();
			}

			for(i = 0; i < MAX_TRACKS;i++)
			{
				m_Channel[i].Init();
			}
		}
		void XMSampler::SetSampleRate(int sr)
		{
			//\todo
			//update deltatick
			//update envelopes
			//
		}

		void XMSampler::Tick()
		{
			boost::recursive_mutex::scoped_lock _lock(m_Mutex);
			SampleCounter(0);
			m_TickCount=0;

			m_DeltaTick = Global::pPlayer->SampleRate() * 60
				/ (Global::pPlayer->bpm * 24/*ticksPerBeat*/);

			NextSampleTick(m_DeltaTick);

			for (int channel=0;channel<MAX_TRACKS;channel++)
			{
				rChannel(channel).EffectFlags(0);
			}

			for (int voice = 0; voice < _numVoices ; voice++)
			{
				if ( m_Voices[voice].IsPlaying()) m_Voices[voice].NewLine();
			}
		}


		void XMSampler::Tick(int channelNum,PatternEntry* pData)
		{
			boost::recursive_mutex::scoped_lock _lock(m_Mutex);

			if (Global::_pSong->IsInvalided()) { return; }

			// don't process twk , twf of Mcm Commands
			if ( pData->_note > 120 )
			{
#if !defined PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
				if ((pData->_cmd == 0 && pData->_volume == 255) || pData->_note != 255 )return; // Return in everything but commands!
	#else
				if (pData->_cmd == 0 || pData->_note != 255 )return; // Return in everything but commands!
	#endif
#endif
			}

			// define some variables to ease the case checking.
			bool bInstrumentSet = (pData->_inst < 255);
#if !defined PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
			bool bPortaEffect = ((pData->_cmd == CMD::PORTA2NOTE) || ((pData->_volume&0xF0) == CMD_VOL::VOL_TONEPORTAMENTO));
	#else
			bool bPortaEffect = (pData->_cmd == CMD::PORTA2NOTE);
	#endif
#endif
			bool bPorta2Note = (pData->_note < 120) && bPortaEffect;
			bool bNoteOn = (pData->_note < 120) && !bPorta2Note;


			Voice* currentVoice = NULL;
			Voice* newVoice = NULL;
			XMSampler::Channel& thisChannel = rChannel(channelNum);


			// STEP A: Look for an existing (foreground) playing voice in the current channel.
			currentVoice = GetCurrentVoice(channelNum);
//			currentVoice = thisChannel.ForegroundVoice();
			if ( currentVoice )
			{
				// Is a new note coming? Then apply the NNA to the playing one.
				if (bNoteOn)
				{	
					//\todo: Implement DCType (Duplicate check type)
					switch (currentVoice->NNA())
					{
					case XMInstrument::NewNoteAction::STOP:
						currentVoice->NoteOffFast();
						currentVoice->IsBackground(true);
						break;
					case XMInstrument::NewNoteAction::NOTEOFF:
						currentVoice->NoteOff();
						currentVoice->IsBackground(true);
						break;
					case XMInstrument::NewNoteAction::FADEOUT:
						currentVoice->NoteFadeout();
						currentVoice->IsBackground(true);
						break;
					}
				} else if(pData->_note == 120 ){
					currentVoice->NoteOff();
				}
			}
			else if ( bPorta2Note )
			{
				// If there is a Porta2Note command, but there is no voice playing, this is converted to a noteOn.
				bPorta2Note=false; bNoteOn = true;
			}
			
			// STEP B: Get a Voice to work with, and initialize it if needed.
			if(bNoteOn)
			{
				thisChannel.Slide2NoteDestNote(pData->_note);
				if (( pData->_cmd == CMD::EXTENDED) && ((pData->_parameter & 0xf0) == CMD_E::E_NOTE_DELAY))
				{
					thisChannel.DelayedNote(*pData);
				}
				else
				{
					newVoice = GetFreeVoice();
					if ( newVoice )
					{
						if(bInstrumentSet){
							thisChannel.InstrumentNo(pData->_inst);
						} else if(thisChannel.InstrumentNo() == 255)
						{	//this is a note to an undefined instrument. we can't continue.
							return;
						}
						//\todo: if the instrument is not set, some initializations do not take effect.
						newVoice->VoiceInit(channelNum,thisChannel.InstrumentNo());
						thisChannel.EffectInit();

						XMInstrument & _inst = m_Instruments[newVoice->InstrumentNum()];
						int _layer = _inst.NoteToSample(pData->_note).second;
						int twlength = m_rWaveLayer[_layer].WaveLength();
						if(twlength > 0)
						{
							thisChannel.ForegroundVoice(newVoice);
#if !defined PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
							if ( pData->_volume<0x40) newVoice->NoteOn(pData->_note,pData->_volume<<2);
							else newVoice->NoteOn(pData->_note);
	#else
							newVoice->NoteOn(pData->_note);
	#endif
#endif
							thisChannel.Note(pData->_note);

							// Add Here any command that is limited to the scope of the new note.
							// An offset is a good candidate, but a volume is not (volume can apply to an existing note)
							if (pData->_cmd == CMD::OFFSET)
							{
								int offset = ((pData->_cmd&0x0F) << 16) +pData->_parameter<<8;
								if ( offset < twlength)
								{
									newVoice->rWave().Position(offset);
								}
								else { newVoice->rWave().Position(0);	}
							}else{ newVoice->rWave().Position(0); }
						}
						else 
						{
							bNoteOn=false;
							newVoice = NULL;
							if ( pData->_cmd == 0 ) return;
						}
					}
					else
					{
						// This is a noteon command, but we are out of voices. We will try to process the effect.
						bNoteOn=false;
						if ( pData->_cmd == 0 ) return;
					}
				}
			} else {
				newVoice = currentVoice;
				if ( bPorta2Note ) 
				{
					thisChannel.Slide2NoteDestNote(pData->_note);
				}
				if (bInstrumentSet && currentVoice)
				{
					currentVoice->ResetVolAndPan(-1);
					currentVoice->rWave().Playing(true);
					currentVoice->IsPlaying(true);
				}
			}

			//\todo : portamento to note, if the note corresponds to a new sample, the sample gets changed
			//		  and the position reset to 0.
			// Effect Command
#if !defined PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN
			thisChannel.SetEffect(newVoice,pData->_volume,pData->_cmd,pData->_parameter);
	#else
			thisChannel.SetEffect(newVoice,255,pData->_cmd,pData->_parameter);
	#endif
#endif
		}

		void XMSampler::Work(int numSamples)
		{
			boost::recursive_mutex::scoped_lock _lock(m_Mutex);

			CPUCOST_INIT(cost);
			int i;

			if (!_mute)
			{
				int _songtracks = Global::_pSong->SongTracks();
				int ns = numSamples;
				int nextevent;

				while (ns)
				{
					nextevent = ns + 1;

					for (i = 0; i < _songtracks; i++)
					{
						if (TriggerDelay[i]._cmd)
						{
							if (TriggerDelayCounter[i] < nextevent)
							{
								nextevent = TriggerDelayCounter[i];
							}
						}
					}

					if (nextevent > ns)
					{
						for (i = 0; i < _songtracks; i++)
						{
							// come back to this
							if (TriggerDelay[i]._cmd)
							{
								TriggerDelayCounter[i] -= ns;
							}
						}
						WorkVoices(ns);
						ns = 0;
					}
					else
					{
						if (nextevent)
						{
							ns -= nextevent;
							WorkVoices(nextevent);
						}

						for (i = 0; i < _songtracks; i++)
						{
							// come back to this
							if (TriggerDelay[i]._cmd == PatternCmd::NOTE_DELAY)
							{
								if (TriggerDelayCounter[i] == nextevent)
								{
									// do event
									Tick(i,&TriggerDelay[i]);
									TriggerDelay[i]._cmd = 0;
								}
								else
								{
									TriggerDelayCounter[i] -= nextevent;
								}
							}
							else if (TriggerDelay[i]._cmd == PatternCmd::RETRIGGER)
							{
								if (TriggerDelayCounter[i] == nextevent)
								{
									// do event
									Tick(i,&TriggerDelay[i]);
									TriggerDelayCounter[i] 
									= (RetriggerRate[i] * Global::pPlayer->SamplesPerRow()) / 256;
								}
								else
								{
									TriggerDelayCounter[i] -= nextevent;
								}
							}
							else if (TriggerDelay[i]._cmd == PatternCmd::RETR_CONT)
							{
								if (TriggerDelayCounter[i] == nextevent)
								{
									// do event
									Tick(i,&TriggerDelay[i]);
									TriggerDelayCounter[i] = (RetriggerRate[i]*Global::pPlayer->SamplesPerRow())/256;
									int parameter = TriggerDelay[i]._parameter&0x0f;
									if (parameter < 9)
									{
										RetriggerRate[i]+= 4*parameter;
									}
									else
									{
										RetriggerRate[i]-= 2 * (16 - parameter);
										if (RetriggerRate[i] < 16)
										{
											RetriggerRate[i] = 16;
										}
									}
								}
								else
								{
									TriggerDelayCounter[i] -= nextevent;
								}
							}
							else if (TriggerDelay[i]._cmd == PatternCmd::ARPEGGIO)
							{
								if (TriggerDelayCounter[i] == nextevent)
								{
									PatternEntry entry =TriggerDelay[i];
									switch(ArpeggioCount[i])
									{
									case 0: 
										Tick(i,&TriggerDelay[i]);
										ArpeggioCount[i]++;
										break;
									case 1:
										entry._note+=((TriggerDelay[i]._parameter&0xF0)>>4);
										Tick(i,&entry);
										ArpeggioCount[i]++;
										break;
									case 2:
										entry._note+=(TriggerDelay[i]._parameter&0x0F);
										Tick(i,&entry);
										ArpeggioCount[i]=0;
										break;
									}
									TriggerDelayCounter[i] = Global::pPlayer->SamplesPerRow()*Global::pPlayer->tpb/24;
								}
								else
								{
									TriggerDelayCounter[i] -= nextevent;
								}
							}
						}
					}
				}

				Machine::SetVolumeCounter(numSamples);

				if ( Global::pConfig->autoStopMachines )
				{
					if (_volumeCounter < 8.0f)	{
						_volumeCounter = 0.0f;
						_volumeDisplay = 0;
						_stopped = true;
					}
					else _stopped=false;
				}
			}

			CPUCOST_CALC(cost, numSamples);
			_cpuCost += cost;
			_worked = true;
		}// XMSampler::Work()

		void XMSampler::WorkVoices(int numsamples)
		{
			float* psamL = _pSamplesL;
			float* psamR = _pSamplesR;
			int tmpsamples = numsamples;
			//////////////////////////////////////////////////////////////////////////
			//  If there is a tick change in this "numsamples" period, process it.
			if ( _sampleCounter + numsamples > NextSampleTick())
			{
				// Work the remaining samples
				int remainingticks = NextSampleTick()-_sampleCounter;
				for (int voice = 0; voice < _numVoices; voice++)
				{
					//VoiceWork(ns, voice);
					if(m_Voices[voice].IsPlaying()){
						m_Voices[voice].Work(remainingticks,_pSamplesL,_pSamplesR,_resampler);
					}
				}
				// Do the Tick jump.
				m_NextSampleTick += DeltaTick();
				m_TickCount++;
				for (int channel=0; channel<MAX_TRACKS; channel++)
				{
					rChannel(channel).PerformFx();
				}
				psamL+=remainingticks;
				psamR+=remainingticks;
				numsamples-=remainingticks;
				_sampleCounter+=remainingticks;
			}
			// Process the samples for each voice.
			if ( numsamples != 0 )
			{
				for (int voice = 0; voice < _numVoices; voice++)
				{
					//VoiceWork(ns, voice);
					if(m_Voices[voice].IsPlaying()){
						m_Voices[voice].Work(numsamples,psamL,psamR,_resampler);
					}
				}
			}
			// Doing this here is faster than in voice, because it is done once per voice then.
			float multip = m_GlobalVolume/128.0f;
			psamL = _pSamplesL;
			psamR = _pSamplesR;
			for (int i=0; i<tmpsamples;i++)
			{
				*psamL = *(psamL++)*multip;
				*psamR = *(psamR++)*multip;
			}
			_sampleCounter+=numsamples;
		}

		void XMSampler::Stop(void)
		{
			//\todo: check that all needed variables/objects are reset.
			int i;
			for (i = 0; i < _numVoices; i++)
			{
				m_Voices[i].NoteOffFast();
			}
			for (i = 0; i < MAX_TRACKS; i++)
			{
				rChannel(i).Restore();
			}
		}// XMSampler::Stop(void)

/*		// Deprecated function.
		While this allows a good reproduction of the files that use arbitrary Speed values ( TicksPerRow()),
		it does so messing with the BeatsPerMin. Since the objective of importing a Module is to modify and
		"Modularize" it, this trick could potentially mess with delays or other sincronized plugins, so the only
		good thing to do is let the user fix it by himself and just approximate it to LinesPerBeat.

		void XMSampler::CalcBPMAndTick()
		{
			int tmp = 24 / ((TicksPerRow() == 0)?6:TicksPerRow());
			if (tmp*TicksPerRow() == 24)
			{
				Global::_pSong->LinesPerBeat(tmp);
				Global::_pSong->BeatsPerMin(BPM());
			}
			else
			{
				Global::_pSong->LinesPerBeat(4);
				Global::_pSong->BeatsPerMin(6 * BPM() / TicksPerRow() );
			}

			int t= Global::pConfig->_pOutputDriver->_samplesPerSec * 60;
			int v=Global::_pSong->BeatsPerMin();
			int z=Global::_pSong->LinesPerBeat();
			Global::pPlayer->SamplesPerRow(	t / (v * z) );
			m_DeltaTick = t / (Global::_pSong->BeatsPerMin() * 24);
		}
*/
		bool XMSampler::Load(RiffFile& riffFile)
		{
			ASSERT(false);
			//The function "Load()" is used for Songs made with Psycle earlier than 1.7
			// It cannot happen that one of those has this new Sampler.
			return true;
		}


		void XMSampler::SaveSpecificChunk(RiffFile* riffFile)
		{
			int temp;
			//		UINT size = 2 * sizeof(temp);
			//		riffFile.Write(&size,sizeof(size));
			riffFile->Write(VERSION);
			riffFile->Write(_numVoices); // numSubtracks
			switch (_resampler.GetQuality())
			{
			case dsp::R_NONE:
				temp = 0;
				break;
			case dsp::R_SPLINE:
				temp = 2;
				break;
			case dsp::R_LINEAR:
			default:
				temp = 1;
				break;
			}
			riffFile->Write(temp); // quality

			riffFile->Write(m_bAmigaSlides);

			// Instrument Data Save
			int numInstruments = 0;		
			for(int i = 0;i < MAX_INSTRUMENT;i++){
				if(m_Instruments[i].IsEnabled()){
					numInstruments++;
				}
			}

			riffFile->Write(numInstruments);

			for(int i = 0;i < MAX_INSTRUMENT;i++){
				if(m_Instruments[i].IsEnabled()){
					riffFile->Write(i);
					m_Instruments[i].Save(riffFile,XMSampler::VERSION);
				}
			}

		}//void SaveSpecificChunk(RiffFile& riffFile) 

		bool XMSampler::LoadSpecificChunk(RiffFile* riffFile, int version)
		{

			int temp;
			UINT _version;
			riffFile->Read(_version); 

			riffFile->Read(_numVoices); // numSubtracks
			riffFile->Read(temp); // quality

			switch (temp)
			{
			case 2:
				_resampler.SetQuality(dsp::R_SPLINE);
				break;
			case 0:
				_resampler.SetQuality(dsp::R_NONE);
				break;
			default:
			case 1:
				_resampler.SetQuality(dsp::R_LINEAR);
				break;
			}

			riffFile->Read(m_bAmigaSlides);

			// Instrument Data Load
			int numInstruments;
			riffFile->Read(numInstruments);
			int idx;
			for(int i = 0;i < numInstruments;i++){
				riffFile->Read(idx);
				m_Instruments[idx].Load(riffFile,_version);
				m_Instruments[idx].IsEnabled(true);
			}

			return TRUE;
		}
	}
}
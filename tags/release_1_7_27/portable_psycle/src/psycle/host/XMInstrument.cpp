/** @file
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */

#include "stdafx.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "Configuration.h"
//#include "IPsySongLoader.h"
//#include "IPsySongSaver.h"
#include "XMInstrument.h"
#include "DataCompression.h"
#include "FileIO.h"

// constructor
#pragma unmanaged

//namespace SF {
namespace psycle
{
	namespace host
	{

		XMInstrument::XMInstrument()
		{
			// clear everythingout

			Init();
			
		}

		// destructor

		XMInstrument::~XMInstrument()
		{

		}

		// other functions

		// delete XMInstrument

		void XMInstrument::Init()
		{
			m_Lines = 16;
			m_FilterType = 4;
			m_NNA = 0;
			m_InitPan = 0.5f;
			m_Loop = false;
			m_AutoVibratoType = 0;
			m_AutoVibratoSweep = 0;
			m_AutoVibratoDepth = 0;
			m_AutoVibratoRate = 0;
			m_bVolumeFade = false;
			m_VolumeFadeSpeed = 0;

			m_RandomPanning = false;///< Random Panning
			m_RandomCutoff = false;///< Random CutOff
			m_RandomResonance = false;///< Random Resonance
			m_RandomSampleStart = false;///< Random SampleStart

			m_Name = _T("");

			m_bVolumeFade = false;
			m_VolumeFadeSpeed = 0;

			// Auto Vibrato
			m_AutoVibratoType = 0;
			m_AutoVibratoSweep = 0;
			m_AutoVibratoDepth = 0;
			m_AutoVibratoRate = 0;

			m_bEnabled = false;

			for (int c = 0; c < MAX_WAVES; c++)
			{
				m_WaveData[c].DeleteWaveData();
			}

			for(int i = 0;i < MAX_NOTES;i++){
				m_AssignNoteToSample[i] = 0;
			}
			/*
			// Reset envelope
			ENV_AT = 1; // 16
			ENV_DT = 1; // 16386
			ENV_SL = 100; // 64
			ENV_RT = 16; // OVERLAPTIME
			
			ENV_F_AT = 16;
			ENV_F_DT = 16384;
			ENV_F_SL = 64;
			ENV_F_RT = 16384;
			
			ENV_F_CO = 64;
			ENV_F_RQ = 64;
			ENV_F_EA = 128;
			ENV_F_TP = 4;
			
			_loop = false;
			_lines = 16;
			
			_NNA = 0; // NNA set to Note Cut [Fast Release]
			
			_pan = 128;
			_RPAN = false;
			_RCUT = false;
			_RRES = false;
			
			for (int c=0; c<MAX_WAVES; c++)
			{
				DeleteLayer(c);
			}
			*/
			//_stprintf(_sName,_T("Sample 無し"));
		}

		// delete layer

		void XMInstrument::DeleteLayer(int c)
		{
		//		_stprintf(waveName[c],_T("Sample 無し"));
		/*	
			if(waveLength[c]>0)
			{
				delete waveDataL[c];
				if(waveStereo[c])
				{
					delete waveDataR[c];
				}
				waveLength[c] = 0;
			}

			waveDataL[c] = NULL;
			waveDataR[c] = NULL;
			
			waveStereo[c]=false;
			waveLoopStart[c]=0;
			waveLoopEnd[c]=0;
			waveLoopType[c]=0;
			waveVolume[c]=100;
			waveFinetune[c]=0;
			waveTune[c]=0;
			*/
		}

		// check for empty XMInstrument

		////	const bool XMInstrument::Empty()
		//	{
		//		/*
		//		for (int i = 0; i < MAX_WAVES; i++)
		//		{
		//			if (waveLength[i] > 0)
		//			{
		//				return FALSE;
		//			}
		//		}
		//		*/
		//		return TRUE;
		//	}

		// load XMInstrument

		void XMInstrument::Load(RiffFile& riffFile,const UINT version)
		{
			int i;
			Init();
			// assume version 0 for now
			riffFile.Read(&m_Loop,sizeof(m_Loop));
			riffFile.Read(&m_Lines,sizeof(m_Lines));
			riffFile.Read(&m_NNA,sizeof(m_NNA));

			
			
			for(i = 0;i < MAX_NOTES;i++){
				NoteToSample(i,riffFile.ReadInt());
			}
			
			m_AmpEnvelope.Load(riffFile,version);
			m_FilterEnvelope.Load(riffFile,version);
			m_PanEnvelope.Load(riffFile,version);
			m_PitchEnvelope.Load(riffFile,version);

			riffFile.Read(m_FilterCutoff);
			riffFile.Read(m_FilterType);
			riffFile.Read(m_FilterResonance);
			riffFile.Read(m_FilterEnvAmount);


			riffFile.Read(m_InitPan);
			riffFile.Read(m_RandomPanning);
			riffFile.Read(m_RandomResonance);
			riffFile.Read(m_RandomSampleStart);

			riffFile.Read(m_VolumeFadeSpeed);
			
			TCHAR _name[128];
			riffFile.ReadStringA2T(_name,sizeof(_name));
			m_Name = _name;

			int numwaves;
			riffFile.Read(numwaves);

			for (i = 0; i < numwaves; i++)
			{

		//			char Header[5];

		//			riffFile.Read((void *)(&Header),4);
		//			Header[4] = 0;

		//			if (strcmp(Header,"WAVE")==0)
		//			{
					UINT index;
					riffFile.Read(index);
					m_WaveData[index].Load(riffFile,version);
		//			}
			}
		}

		// save XMInstrument

		void XMInstrument::Save(RiffFile& riffFile,const UINT version)
		{
			int i;
			riffFile.Write(m_Loop);
			riffFile.Write(m_Lines);
			riffFile.Write(m_NNA);

			for(i = 0;i < MAX_NOTES;i++){
				riffFile.Write(NoteToSample(i));
			}
			
			m_AmpEnvelope.Save(riffFile,version);
			m_FilterEnvelope.Save(riffFile,version);
			m_PanEnvelope.Save(riffFile,version);
			m_PitchEnvelope.Save(riffFile,version);

			riffFile.Write(m_FilterCutoff);
			riffFile.Write(m_FilterType);
			riffFile.Write(m_FilterResonance);
			riffFile.Write(m_FilterEnvAmount);


			riffFile.Write(m_InitPan);
			riffFile.Write(m_RandomPanning);
			riffFile.Write(m_RandomResonance);
			riffFile.Write(m_RandomSampleStart);

			riffFile.Write(m_VolumeFadeSpeed);

			CT2A _name(m_Name.data());
			riffFile.Write(_name,strlen(_name) + 1);

			// now we have to write out the waves, but only the valid ones

			int numwaves = 0;
			for (i = 0; i < MAX_WAVES; i++)
			{
				if (m_WaveData[i].WaveLength() > 0)
				{
					numwaves++;
				}
			}

			riffFile.Write(numwaves);
			
			for (i = 0; i < MAX_WAVES; i++)
			{
				if (m_WaveData[i].WaveLength() > 0)
				{

					UINT index = i;
					
		//				riffFile.Write("WAVE",4);
					riffFile.Write(index);
					m_WaveData[i].Save(riffFile,version);
				}
			}
		}

		void XMInstrument::Envelope::Load(RiffFile& riffFile,const UINT version)
		{
			riffFile.Read(m_Enabled);
			int _num_of_points = 0;
			if(m_Enabled){
				riffFile.Read(_num_of_points);
				riffFile.Read(m_LoopStart);
				riffFile.Read(m_LoopEnd);
				riffFile.Read(m_SustainBegin);
				riffFile.Read(m_SustainEnd);
				for(int i = 0;i < _num_of_points ;i++)
				{
					std::pair<int,ValueType> _value;

					riffFile.Read(_value.first);
					riffFile.Read(_value.second);

					//_value.first = m_Points[i];
					//_value.second = m_Value[i];

					m_Points.push_back(_value);
				}
			}

		}

		void XMInstrument::Envelope::Save(RiffFile& riffFile,const UINT version)
		{
			riffFile.Write(m_Enabled);
			if(m_Enabled){
				riffFile.Write(m_Points.size());
				riffFile.Write(m_LoopStart);
				riffFile.Write(m_LoopEnd);
				riffFile.Write(m_SustainBegin);
				riffFile.Write(m_SustainEnd);

				for(UINT i = 0;i < m_Points.size() ;i++){
					riffFile.Write(m_Points[i].first);// point
					riffFile.Write(m_Points[i].second);//value
				}
			}

		}

		void XMInstrument::WaveData::Load(RiffFile& riffFile,const UINT version)
		{	
			UINT size;
			riffFile.Read(size);

			riffFile.Read(m_WaveLength);
			riffFile.Read(m_WaveVolume);
			riffFile.Read(m_WaveLoopStart);
			riffFile.Read(m_WaveLoopEnd);
					
			riffFile.Read(m_WaveTune);
			riffFile.Read(m_WaveFineTune);
			riffFile.Read(m_WaveLoopType);
			riffFile.Read(m_WaveStereo);
					
			riffFile.ReadStringA2T(m_WaveName,32);
					
			riffFile.Read(size);

			byte* pData;
					
			pData = new byte[size];
			riffFile.Read((void *)pData,size);
			SoundDesquash(pData,&m_pWaveDataL);
			delete pData;
			
			if (m_WaveStereo)
			{
				riffFile.Read(size);
				pData = new byte[size];
				riffFile.Read(pData,size);
				SoundDesquash(pData,&m_pWaveDataR);
				delete pData;
			}

		}

		void XMInstrument::WaveData::Save(RiffFile& riffFile,const UINT version)
		{
			byte * pData1;
			byte * pData2;

			UINT size1(SoundSquash(m_pWaveDataL,&pData1,m_WaveLength));
			UINT size2(0);

			//  test for accuracy of compress/decompress - it's 100%
			//SoundDesquash(pData1,&waveDataR[i]);
			//for (int c = 0; c < waveLength[i]; c++)
			//{
			//	if (waveDataL[i][c] != waveDataR[i][c])
			//	{
			//		int z = 2000; // error
			//	}
			//}
			

			if (m_WaveStereo)
			{
				size2 = SoundSquash(m_pWaveDataR,&pData2,m_WaveLength);
			}

			CT2A _wave_name(m_WaveName);

			UINT size = sizeof(m_WaveLength)
						+sizeof(m_WaveVolume)
						+sizeof(m_WaveLoopStart)
						+sizeof(m_WaveLoopEnd)
						+sizeof(m_WaveTune)
						+sizeof(m_WaveFineTune)
						+sizeof(m_WaveStereo)
						+strlen(_wave_name) + 1
						+size1
						+size2;

			riffFile.Write(size);

			riffFile.Write(m_WaveLength);
			riffFile.Write(m_WaveVolume);
			riffFile.Write(m_WaveLoopStart);
			riffFile.Write(m_WaveLoopEnd);

			riffFile.Write(m_WaveTune);
			riffFile.Write(m_WaveFineTune);
			riffFile.Write(m_WaveLoopType);
			riffFile.Write(m_WaveStereo);

			riffFile.Write(_wave_name,strlen(_wave_name) + 1);

			riffFile.Write(size1);
			riffFile.Write((void*)pData1,size1);
			delete pData1;
			
			if (m_WaveStereo)
			{
				riffFile.Write(size2);
				riffFile.Write((void*)pData2,size2);
				delete pData2;
			}
		}

#if defined WTL
		void XMInstrument::Name(const SF::string& name)
#else
		void XMInstrument::Name(const std::string& name)
#endif
		{
			m_Name = name; 		
		};

		/** ***** [bohan] iso-(10)646 encoding only please! *****
			* @param index Envelopeのindex
			* @param point Envelopeの位置
			* @param value Envelopeの値 
			* @return ***** [bohan] iso-(10)646 encoding only please! ******/
		const int  XMInstrument::Envelope::PointAndValue(const int index,const int point,const ValueType value)
		{
			ATLASSERT(index < (int)m_Points.size());

			if(index < (int)m_Points.size())
			{
				m_Points[index].first = point;
				m_Points[index].second = value;

				// point***** [bohan] iso-(10)646 encoding only please! *****

				if(index > 0 && (index + 1) < (int)m_Points.size())
				{	
					if(point >= m_Points[index - 1].first 
						&& point <= m_Points[index + 1].first)
					{
						return index;
					}
				}

				if(index == ((int)m_Points.size() - 1))
				{
					if(point >= (int)(m_Points[index - 1].first))
					{
						return index;
					}

				}

				if(index == 0 && (index + 1) < (int)(m_Points.size()))
				{
					if(point <= m_Points[index + 1].first)
					{
						return index;
					}
				}

				// ***** [bohan] iso-(10)646 encoding only please! *****
				if(index == 0 && (index + 1) == ((int)(m_Points.size()) - 1))
				{
					return index;
				}

				if(index > 0)
				{
					// point***** [bohan] iso-(10)646 encoding only please! *****
					int _new_index = index;
					if(point < m_Points[index - 1].first){
						// ***** [bohan] iso-(10)646 encoding only please! *****
						do {
							_new_index--;
							if(point > m_Points[_new_index].first)
							{ 
								
								PointValue _point = m_Points[index];
								_new_index++;
								m_Points.erase(m_Points.begin() + index);
								m_Points.insert(m_Points.begin() + _new_index,_point);


								// Sustain,Loop***** [bohan] iso-(10)646 encoding only please! *****
								// ***** [bohan] iso-(10)646 encoding only please! *****
								// pt var
								// 10 -----------------
								// 09 -----------------
								// 08 index             --+
								// 07 -----------------   +
								// 06 -----------------   +
								// 05 ***** [bohan] iso-(10)646 encoding only please! *****          <-+ 
								// 04 _new_index
								// 03 -----------------
								// 02 -----------------
								// 01 -----------------
								// 00 -----------------
								// ***** [bohan] iso-(10)646 encoding only please! *****
								// pt 元pt var
								// 10 10 -----------------
								// 09 09 -----------------
								// 08 07 index             --+
								// 07 06 -----------------   +
								// 06 05 -----------------   +
								// 05 08 挿入位置--------- <-+ 
								// 04 04 _new_index
								// 03 03 -----------------
								// 02 02 -----------------
								// 01 01 -----------------
								// 00 00 -----------------
								//  1. index > Sustain,Loop ***** [bohan] iso-(10)646 encoding only please! *****
								//  2. ***** [bohan] iso-(10)646 encoding only please! ***** <= Sustain,Loop ***** [bohan] iso-(10)646 encoding only please! *****
								//  3. ***** [bohan] iso-(10)646 encoding only please! ***** < Sustain,Loop ***** [bohan] iso-(10)646 encoding only please! *****
								//  4. index == Sustain,Loop ***** [bohan] iso-(10)646 encoding only please! *****
								
								if((_new_index <= m_SustainBegin) && (m_SustainBegin < index))
								{
									m_SustainBegin++;
								} else if(index == m_SustainBegin) {
									m_SustainBegin = _new_index;
								}

								if((_new_index <= m_SustainEnd) && (m_SustainEnd < index)){
									m_SustainEnd++;
								} else if(index == m_SustainEnd){ 
									m_SustainEnd = _new_index;
								}

								if((_new_index <= m_LoopStart) && (m_LoopStart < index))
								{
									m_LoopStart++;
								}
								else if(index == m_LoopStart)
								{
									m_LoopStart = _new_index;
								}

								if((_new_index <= m_LoopEnd) &&	(m_LoopEnd  < index))
								{
									m_LoopEnd++;
								} else if(index == m_LoopEnd)
								{
									m_LoopStart = _new_index;
								}

								break;

							}


						} while(_new_index > 0);

						// _new_index == ***** [bohan] iso-(10)646 encoding only please! *****
						if(_new_index == 0){
							PointValue _point = m_Points[index];
							m_Points.erase(m_Points.begin() + index);
							m_Points.insert(m_Points.begin() + (UINT)_new_index,_point);
							
							if(m_SustainBegin < index){
								m_SustainBegin++;
							} if(m_SustainBegin == index) {
								m_SustainBegin = _new_index;
							}
							
							if(m_SustainEnd < index){
								m_SustainEnd++;
							} else if(m_SustainEnd == index){
								m_SustainBegin = _new_index;
							}

							if(m_LoopStart != -1 && m_LoopStart < index){
								m_LoopStart++;
							} else if(m_LoopStart == index) {
								m_LoopStart = _new_index;
							}

							if(m_LoopEnd < index)
							{
								m_LoopEnd++;
							} else if(m_LoopEnd == index) {
								m_LoopEnd = _new_index;
							}
							
						}

						// ***** [bohan] iso-(10)646 encoding only please! *****
						if(m_SustainBegin != INVALID && m_SustainEnd != INVALID)
						{
							if(m_SustainBegin > m_SustainEnd){
								m_SustainBegin = INVALID;
								m_SustainEnd = INVALID;
							}
						}

						// ループ開始が、ループ終了より大きい場合は無効にする
						if(m_LoopStart != INVALID && m_LoopEnd != INVALID)
						{
							if(m_LoopStart > m_LoopEnd){
								m_LoopStart = INVALID;
								m_LoopEnd = INVALID;
							}
						}
						return _new_index;
					}
				}

				// pointが１つ後の座標より大きい場合は入れ替える
				if(point > m_Points[index + 1].first)
				{
					int _new_index = 0;
					for(_new_index = index + 2;_new_index < (int)m_Points.size();_new_index++)
					{
						if(point < m_Points[_new_index].first)
						{
							PointValue _point = m_Points[index];
							m_Points.erase(m_Points.begin() + (UINT)index);
							m_Points.insert(m_Points.begin() + (UINT)_new_index - 1,_point);

							// Sustain,Loopポイントの修正
							// <挿入前>
							// pt var
							// 10 -----------------
							// 09 _new_index
							// 08 挿入位置          <-+
							// 07 -----------------   +
							// 06 -----------------   +
							// 05 -----------------   + 
							// 04 index             --+
							// 03 -----------------
							// 02 -----------------
							// 01 -----------------
							// 00 -----------------
							// <挿入後>
							// pt 元pt var
							// 10 10 -----------------
							// 09 09 _new_index
							// 08 04 挿入位置　        <-+
							// 07 08 -----------------   +
							// 06 07 -----------------   +
							// 05 06 -----------------   + 
							// 04 05 index             --+
							// 03 03 -----------------
							// 02 02 -----------------
							// 01 01 -----------------
							// 00 00 -----------------
							//  1. index < Sustain,Loop ***** [bohan] iso-(10)646 encoding only please! ***** < _new_index ***** [bohan] iso-(10)646 encoding only please! *****
							//  2. _new_index <= Sustain,Loop ***** [bohan] iso-(10)646 encoding only please! *****
							//  3. index > Sustain,Loop ***** [bohan] iso-(10)646 encoding only please! *****

							if((index < m_SustainBegin) && (m_SustainBegin < _new_index))
							{
								m_SustainBegin--;
							} else if(m_SustainBegin == index)
							{
								m_SustainBegin = _new_index - 1;
							}

							if((index < m_SustainEnd) && (m_SustainEnd < _new_index))
							{
								m_SustainEnd--;

							} else if(m_SustainEnd == index)
							{
								m_SustainEnd = _new_index - 1;
							}

							if((index < m_LoopStart) &&	(m_LoopStart < _new_index))
							{
								m_LoopStart--;
							} else if(m_LoopStart == index)
							{
								m_LoopStart = _new_index - 1;
							}

							if((index < m_LoopEnd) && (m_LoopEnd  < _new_index))
							{
								m_LoopEnd--;
							} else if(m_LoopEnd == index)
							{
								m_LoopEnd = _new_index - 1;
							}
							// ループより抜けて、終了
							break;

						}

					}

					if(_new_index == (int)(m_Points.size())){
						// _new_index が　最大値の場合
						PointValue _point = m_Points[index];
						m_Points.erase(m_Points.begin() + (UINT)index);
						m_Points.push_back(_point);
						if((index < m_SustainBegin))
						{
							m_SustainBegin--;
						} else if(m_SustainBegin == index){
							m_SustainBegin = _new_index - 1;
						}


						if((index < m_SustainEnd))
						{
							m_SustainEnd--;
						} else if(m_SustainEnd == index){
							m_SustainEnd = _new_index - 1;
						}

						if((index < m_LoopStart))
						{
							m_LoopStart--;
						} else if(m_LoopStart == index){
							m_LoopStart = _new_index - 1;
						}

						if((index < m_LoopEnd))
						{
							m_LoopEnd--;
						} else if(m_LoopEnd == index){
							m_LoopEnd = _new_index - 1;
						}
					}

					// ***** [bohan] iso-(10)646 encoding only please! *****
					if(m_SustainBegin != INVALID && m_SustainEnd != INVALID)
					{
						if(m_SustainBegin > m_SustainEnd){
							m_SustainBegin = INVALID;
							m_SustainEnd = INVALID;
						}
					}

					// ***** [bohan] iso-(10)646 encoding only please! *****
					if(m_LoopStart != INVALID && m_LoopEnd != INVALID)
					{
						if(m_LoopStart > m_LoopEnd){
							m_LoopStart = INVALID;
							m_LoopEnd = INVALID;
						}
					}
					return _new_index - 1;
				}
				
			}
			return INVALID;
		}

		/** Point***** [bohan] iso-(10)646 encoding only please! *****
			* @param index Envelope***** [bohan] iso-(10)646 encoding only please! *****
			* @param point Envelope***** [bohan] iso-(10)646 encoding only please! *****
			* @param value Envelope***** [bohan] iso-(10)646 encoding only please! *****
			* @return ***** [bohan] iso-(10)646 encoding only please! *****index */
		const int XMInstrument::Envelope::Insert(const int point,const ValueType value)
		{

			int _new_index = 0;
			for(_new_index = 0;_new_index < (int)m_Points.size();_new_index++)
			{
				if(point < m_Points[_new_index].first)
				{
					PointValue _point;
					_point.first = point;
					_point.second = value;

					m_Points.insert(m_Points.begin() + (UINT)_new_index,_point);

					// Sustain,Loop***** [bohan] iso-(10)646 encoding only please! *****
					// ***** [bohan] iso-(10)646 encoding only please! *****
					// pt var
					// 10 -----------------
					// 09 _new_index
					// 08 挿入位置          <-+
					// 07 -----------------   +
					// 06 -----------------   +
					// 05 -----------------   + 
					// 04 index             --+
					// 03 -----------------
					// 02 -----------------
					// 01 -----------------
					// 00 -----------------
					// <挿入後>
					// pt 元pt var
					// 10 10 -----------------
					// 09 09 _new_index
					// 08 04 挿入位置　        <-+
					// 07 08 -----------------   +
					// 06 07 -----------------   +
					// 05 06 -----------------   + 
					// 04 05 index             --+
					// 03 03 -----------------
					// 02 02 -----------------
					// 01 01 -----------------
					// 00 00 -----------------
					//  1. index < Sustain,Loop ***** [bohan] iso-(10)646 encoding only please! *****
					//  2. _new_index <= Sustain,Loop***** [bohan] iso-(10)646 encoding only please! *****
					//  3. index > Sustain,Loop ***** [bohan] iso-(10)646 encoding only please! *****

					if(m_SustainBegin >= _new_index)
					{
						m_SustainBegin++;
					}

					if(m_SustainEnd >= _new_index)
					{
						m_SustainEnd++;
					}

					if(m_LoopStart >= _new_index)
					{
						m_LoopStart++;
					}

					if(m_LoopEnd >= _new_index)
					{
						m_LoopEnd++;
					}
					// ***** [bohan] iso-(10)646 encoding only please! *****
					break;

				}

			}

			if(_new_index == (int)(m_Points.size())){
				// _new_index ***** [bohan] iso-(10)646 encoding only please! *****
				PointValue _point;
				_point.first = point;
				_point.second = value;
				m_Points.push_back(_point);
				// ***** [bohan] iso-(10)646 encoding only please! *****
			}
			return _new_index;

		}
	}

}// namespace SF

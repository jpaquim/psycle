/** @file
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */

#include <project.private.hpp>
#include "Configuration.hpp"
//#include "IPsySongLoader.h"
//#include "IPsySongSaver.h"
#include "XMInstrument.hpp"
#include "Filter.hpp"
#include "DataCompression.hpp"
#include "FileIO.hpp"

// constructor
#pragma unmanaged

namespace psycle
{
	namespace host
	{
//////////////////////////////////////////////////////////////////////////
//  XMInstrument::WaveData Implementation.

		void XMInstrument::WaveData::Load(RiffFile& riffFile,const UINT version)
		{	
			compiler::uint32 size1,size2;

			CT2A _wave_name("");
			riffFile.ReadStringA2T(_wave_name,32);
			m_WaveName=_wave_name;

			riffFile.Read(m_WaveLength);
			riffFile.Read(m_WaveGlobVolume);
			riffFile.Read(m_WaveDefVolume);

			riffFile.Read(m_WaveLoopStart);
			riffFile.Read(m_WaveLoopEnd);
			riffFile.Read(&m_WaveLoopType,sizeof(m_WaveLoopType));

			riffFile.Read(m_WaveSusLoopStart);
			riffFile.Read(m_WaveSusLoopEnd);
			riffFile.Read(&m_WaveSusLoopType,sizeof(m_WaveSusLoopType));

			riffFile.Read(m_WaveTune);
			riffFile.Read(m_WaveFineTune);

			riffFile.Read(m_WaveStereo);
			riffFile.Read(m_PanEnabled);
			riffFile.Read(m_PanFactor);

			riffFile.Read(m_VibratoRate);
			riffFile.Read(m_VibratoSweep);
			riffFile.Read(m_VibratoDepth);
			riffFile.Read(m_VibratoType);

			riffFile.Read(size1);
			byte* pData;
			pData = new byte[size1];
			riffFile.Read((void *)pData,size1);
			SoundDesquash(pData,&m_pWaveDataL);
			
			if (m_WaveStereo)
			{
				delete pData;
				riffFile.Read(size2);
				pData = new byte[size2];
				riffFile.Read(pData,size2);
				SoundDesquash(pData,&m_pWaveDataR);
			}
			delete pData;

		}

		void XMInstrument::WaveData::Save(RiffFile& riffFile,const UINT version)
		{
			byte * pData1;
			byte * pData2;
			UINT size1= SoundSquash(m_pWaveDataL,&pData1,m_WaveLength);
			UINT size2(0);

			if (m_WaveStereo)
			{
				size2 = SoundSquash(m_pWaveDataR,&pData2,m_WaveLength);
			}

			CT2A _wave_name(m_WaveName.c_str());
			UINT size = sizeof(XMInstrument::WaveData)
				-sizeof(m_pWaveDataL)
				-sizeof(m_pWaveDataR)
				+(2*sizeof(size1))
				+size1
				+size2;

			riffFile.Write("SMPD");
			riffFile.Write(size);

			riffFile.Write(_wave_name,strlen(_wave_name) + 1);

			riffFile.Write(m_WaveLength);
			riffFile.Write(m_WaveGlobVolume);
			riffFile.Write(m_WaveDefVolume);

			riffFile.Write(m_WaveLoopStart);
			riffFile.Write(m_WaveLoopEnd);
			riffFile.Write(&m_WaveLoopType,sizeof(m_WaveLoopType));

			riffFile.Write(m_WaveSusLoopStart);
			riffFile.Write(m_WaveSusLoopEnd);
			riffFile.Write(&m_WaveSusLoopType,sizeof(m_WaveSusLoopType));

			riffFile.Write(m_WaveTune);
			riffFile.Write(m_WaveFineTune);

			riffFile.Write(m_WaveStereo);
			riffFile.Write(m_PanEnabled);
			riffFile.Write(m_PanFactor);

			riffFile.Write(m_VibratoRate);
			riffFile.Write(m_VibratoSweep);
			riffFile.Write(m_VibratoDepth);
			riffFile.Write(m_VibratoType);

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


//////////////////////////////////////////////////////////////////////////
//  XMInstrument::Envelope Implementation.

		/** 
		* @param pointIndex : Current point index.
		* @param pointTime  : Desired point Time.
		* @param value		: Desired point Value.
		* @return			: New point index.
		*/
		const int  XMInstrument::Envelope::SetTimeAndValue(const int pointIndex,const int pointTime,const ValueType pointVal)
		{
			ASSERT(pointIndex < (int)m_Points.size());
			if(pointIndex < (int)m_Points.size())
			{
				int prevtime,nextime;
				m_Points[pointIndex].first = pointTime;
				m_Points[pointIndex].second = pointVal;

				prevtime=(pointIndex == 0)?m_Points[pointIndex].first:m_Points[pointIndex-1].first;
				nextime=(pointIndex == (int)(m_Points.size())-1 )?m_Points[pointIndex].first:m_Points[pointIndex+1].first;
					
				// Initialization done. Check if we have to move the point to a new index:


				// Is the Time already between the previous and next?
				if(    pointTime >= m_Points[pointIndex - 1].first 
					&& pointTime <= m_Points[pointIndex + 1].first)
				{
					return pointIndex;
				}

				// Else, we have some work to do.
				int	new_index = pointIndex;

				// If we have to move it backwards:
				if (pointTime < prevtime)
				{
					// Let's go backwards....
					do {
						new_index--; 
						// ... until we find a smaller time.
						if (pointTime > m_Points[new_index].first)
						{
							new_index++;
							break;
						}
					} while(new_index>0);

					// We have reached the end, either because of the "break", or because of the loop.
					// In any case, the new_index has the point where the new point should go, so move it.
				}
				// If we have to move it forward:
				else /*if ( pointTime > nextime)  <-- It is the only case left*/
				{
					// Let's go forward....
					do {
						new_index++;
						// ... until we find a smaller time.
						if (pointTime < m_Points[new_index].first)
						{
							new_index--;
							break;
						}
					} while(new_index<(int)m_Points.size()-1);

					// We have reached the end, either because of the "break", or because of the loop.
					// In any case, the new_index has the point where the new point should go, so move it.
				}

				PointValue _point = m_Points[pointIndex];
				m_Points.erase(m_Points.begin() + pointIndex);
				m_Points.insert(m_Points.begin() + new_index,_point);

				// Ok, point moved. Let's see if this change has affected the Sustain and Loop points:

				// we have moved forward
				if ( new_index > pointIndex )
				{
					// In this scenario, it can happen that we have to move Sustain and/or Loop backwards.
					if (m_SustainBegin > pointIndex && m_SustainBegin <= new_index)
					{
						m_SustainBegin--;
					}
					if (m_SustainEnd > pointIndex && m_SustainEnd <= new_index)
					{
						m_SustainEnd--;
					}
					if (m_LoopStart > pointIndex && m_LoopStart <= new_index)
					{
						m_LoopStart--;
					}
					if (m_LoopEnd > pointIndex && m_LoopEnd <= new_index)
					{
						m_LoopEnd--;
					}
				}
				// else, we have moved backwards
				else /*if ( new_index < pointIndex ) <-- It is the only case left*/
				{
					// In this scenario, it can happen that we have to move Sustain and/or Loop forward.
					if (m_SustainBegin < pointIndex && m_SustainBegin >= new_index)
					{
						m_SustainBegin++;
					}
					if (m_SustainEnd < pointIndex && m_SustainEnd >= new_index)
					{
						m_SustainEnd++;
					}
					if (m_LoopStart < pointIndex && m_LoopStart >= new_index)
					{
						m_LoopStart++;
					}
					if (m_LoopEnd < pointIndex && m_LoopEnd >= new_index)
					{
						m_LoopEnd++;
					}
				}
				return new_index;
			}
			return INVALID;
		}
		/** 
		* @param pointTime  : Point Time.
		* @param value		: Point Value.
		* @return			: New point index.
		*/
		const int XMInstrument::Envelope::Insert(const int pointTime,const ValueType pointVal)
		{
			int _new_index;
			for(_new_index = 0;_new_index < (int)m_Points.size();_new_index++)
			{
				if(pointTime < m_Points[_new_index].first)
				{
					PointValue _point;
					_point.first = pointTime;
					_point.second = pointVal;

					m_Points.insert(m_Points.begin() + (UINT)_new_index,_point);

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
					break;
				}
			}
			// If we have reached the end without finding a suitable point to insert, then this
			// one should go at the end.
			if(_new_index == (int)(m_Points.size()))
			{
				PointValue _point;
				_point.first = pointTime;
				_point.second = pointVal;
				m_Points.push_back(_point);
			}
			return _new_index;
		}
		/** 
		* @param pointIndex : point index to be deleted.
		*/
		void XMInstrument::Envelope::Delete(const int pointIndex)
		{
			ASSERT(pointIndex < (int)m_Points.size());
			if(pointIndex < (int)m_Points.size())
			{
				m_Points.erase(m_Points.begin() + pointIndex);
				if(pointIndex == m_SustainBegin || pointIndex == m_SustainEnd)
				{
					m_SustainBegin = INVALID;
					m_SustainEnd = INVALID;
				}
				else {
					if(m_SustainBegin > pointIndex)
					{
						m_SustainBegin--;
					}
					if(m_SustainEnd > pointIndex)
					{
						m_SustainEnd--;
					}
				}

				if(pointIndex == m_LoopStart || pointIndex == m_LoopEnd)
				{
					m_LoopStart = INVALID;
					m_LoopEnd = INVALID;
				}
				else {
					if(m_LoopStart > pointIndex)
					{
						m_LoopStart--;
					}
					if(m_LoopEnd > pointIndex)
					{
						m_LoopEnd--;
					}
				}
			}
		}
	
		// Loading Procedure
		void XMInstrument::Envelope::Load(RiffFile& riffFile,const UINT version)
		{
			riffFile.Read(m_Enabled);
			riffFile.Read(m_Carry);
			riffFile.Read(m_LoopStart);
			riffFile.Read(m_LoopEnd);
			riffFile.Read(m_SustainBegin);
			riffFile.Read(m_SustainEnd);

			int _num_of_points = 0;
			riffFile.Read(_num_of_points);
			for(int i = 0;i < _num_of_points ;i++)
			{
				PointValue _value;

				riffFile.Read(_value.first);
				riffFile.Read(_value.second);
				m_Points.push_back(_value);
			}
		}
		// Saving Procedure
		void XMInstrument::Envelope::Save(RiffFile& riffFile,const UINT version)
		{
			riffFile.Write(m_Enabled);
			riffFile.Write(m_Carry);
			riffFile.Write(m_LoopStart);
			riffFile.Write(m_LoopEnd);
			riffFile.Write(m_SustainBegin);
			riffFile.Write(m_SustainEnd);
			riffFile.Write(m_Points.size());

			for(UINT i = 0;i < m_Points.size() ;i++)
			{
				riffFile.Write(m_Points[i].first);// point
				riffFile.Write(m_Points[i].second);//value
			}
		}

//////////////////////////////////////////////////////////////////////////
//   XMInstrument Implementation
		XMInstrument::XMInstrument()
		{
			Init();
		}

		// destructor
		XMInstrument::~XMInstrument()
		{
			// No need to delete anything, since we don't allocate memory explicitely.
		}

		// other functions
		void XMInstrument::Init()
		{
			m_bEnabled = false;

			m_Name = _T("");

			m_Lines = 16;

			m_GlobVol = 1.0f;
			m_VolumeFadeSpeed = 0;

			m_PanEnabled=false;
			m_InitPan = 0.5f;
			m_PitchPanCenter = 60;
			m_PitchPanSep = 0;

			m_FilterCutoff = 127;
			m_FilterResonance = 0;
			m_FilterEnvAmount = 0;
			m_FilterType = dsp::F_NONE;

			m_RandomVolume = 0;
			m_RandomPanning = 0;
			m_RandomCutoff = 0;
			m_RandomResonance = 0;

			m_NNA = NewNoteAction::STOP;
			m_DCT = DCType::DCT_NONE;
			m_DCA = DCAction::DCA_STOP;

			NotePair npair;
			npair.second=255;
			for(int i = 0;i < NOTE_MAP_SIZE;i++){
				npair.first=i;
				m_AssignNoteToSample[i] = npair;
			}

			m_AmpEnvelope.Init();
			m_PanEnvelope.Init();
			m_PitchEnvelope.Init();
			m_FilterEnvelope.Init();

		}

		// load XMInstrument
		void XMInstrument::Load(RiffFile& riffFile,const UINT version)
		{
			m_bEnabled = true;

			CT2A _name("");
			riffFile.ReadStringA2T(_name,32);
			m_Name=_name;

			riffFile.Read(m_Lines);

			riffFile.Read(m_GlobVol);
			riffFile.Read(m_VolumeFadeSpeed);

			riffFile.Read(m_InitPan);
			riffFile.Read(m_PanEnabled);
			riffFile.Read(m_PitchPanCenter);
			riffFile.Read(&m_PitchPanSep,sizeof(compiler::sint8));

			riffFile.Read(m_FilterCutoff);
			riffFile.Read(m_FilterResonance);
			riffFile.Read(m_FilterEnvAmount);
			riffFile.Read(&m_FilterType,sizeof(m_FilterType));

			riffFile.Read(m_RandomVolume);
			riffFile.Read(m_RandomPanning);
			riffFile.Read(m_RandomCutoff);
			riffFile.Read(m_RandomResonance);

			riffFile.Read(&m_NNA,sizeof(m_NNA));
			riffFile.Read(&m_DCT,sizeof(m_DCT));
			riffFile.Read(&m_DCA,sizeof(m_DCA));

			NotePair npair;
			for(int i = 0;i < NOTE_MAP_SIZE;i++){
				riffFile.Read(&npair,sizeof(npair));
				NoteToSample(i,npair);
			}

			m_AmpEnvelope.Load(riffFile,version);
			m_PanEnvelope.Load(riffFile,version);
			m_PitchEnvelope.Load(riffFile,version);
			m_FilterEnvelope.Load(riffFile,version);
		}

		// save XMInstrument
		void XMInstrument::Save(RiffFile& riffFile,const UINT version)
		{
			int i;
			int size = sizeof(XMInstrument)
				-sizeof(m_AmpEnvelope)
				-sizeof(m_PanEnvelope)
				-sizeof(m_PitchEnvelope)
				-sizeof(m_FilterEnvelope);

			riffFile.Write("INST");
			riffFile.Write(size);

			CT2A _name(m_Name.c_str());
			riffFile.Write(_name,strlen(_name) + 1);

			riffFile.Write(m_bEnabled);

			riffFile.Write(m_Lines);

			riffFile.Write(m_GlobVol);
			riffFile.Write(m_VolumeFadeSpeed);

			riffFile.Write(m_InitPan);
			riffFile.Write(m_PanEnabled);
			riffFile.Write(m_PitchPanCenter);
			riffFile.Write(m_PitchPanSep);

			riffFile.Write(m_FilterCutoff);
			riffFile.Write(m_FilterResonance);
			riffFile.Write(m_FilterEnvAmount);
			riffFile.Write(&m_FilterType,sizeof(m_FilterType));

			riffFile.Write(m_RandomVolume);
			riffFile.Write(m_RandomPanning);
			riffFile.Write(m_RandomCutoff);
			riffFile.Write(m_RandomResonance);

			riffFile.Write(&m_NNA,sizeof(m_NNA));
			riffFile.Write(&m_DCT,sizeof(m_DCT));
			riffFile.Write(&m_DCA,sizeof(m_DCA));

			NotePair npair;
			for(i = 0;i < NOTE_MAP_SIZE;i++){
				riffFile.Write(&npair,sizeof(npair));
				NoteToSample(i,npair);
			}

			m_AmpEnvelope.Save(riffFile,version);
			m_FilterEnvelope.Save(riffFile,version);
			m_PanEnvelope.Save(riffFile,version);
			m_PitchEnvelope.Save(riffFile,version);
		}

	} //namespace host
}// namespace psycle

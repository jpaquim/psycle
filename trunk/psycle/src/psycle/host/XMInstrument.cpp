/** @file
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */

#include <psycle/host/detail/project.private.hpp>
#include "XMInstrument.hpp"
#include <psycle/helpers/filter.hpp>
#include <psycle/helpers/datacompression.hpp>
#include <psycle/helpers/math/clip.hpp>
#include "FileIO.hpp"
#include <cassert>
namespace psycle
{
	namespace host
	{
//////////////////////////////////////////////////////////////////////////
//  XMInstrument::WaveData Implementation.
		template <class T> 
		void XMInstrument::WaveData<T>::Init(){
			DeleteWaveData();
			m_WaveName= "";
			m_WaveLength = 0;
			m_WaveGlobVolume = 1.0f; // Global volume ( global multiplier )
			m_WaveDefVolume = 128; // Default volume ( volume at which it starts to play. corresponds to 0Cxx/volume command )
			m_WaveLoopStart = 0;
			m_WaveLoopEnd = 0;
			m_WaveLoopType = LoopType::DO_NOT;
			m_WaveSusLoopStart = 0;
			m_WaveSusLoopEnd = 0;
			m_WaveSusLoopType = LoopType::DO_NOT;
			m_WaveSampleRate = 44100;
			m_WaveTune = 0;
			m_WaveFineTune = 0;	
			m_WaveStereo = false;
			m_PanFactor = 0.5f;
			m_PanEnabled = false;
			m_Surround = false;
			m_VibratoAttack = 0;
			m_VibratoSpeed = 0;
			m_VibratoDepth = 0;
			m_VibratoType = 0;
		}
        template void XMInstrument::WaveData<short>::Init();

		template <class T> 
		void XMInstrument::WaveData<T>::DeleteWaveData(){
			if ( m_pWaveDataL)
			{
				delete[] m_pWaveDataL;
				m_pWaveDataL=0;
				if (m_WaveStereo)
				{
					delete[] m_pWaveDataR;
					m_pWaveDataR=0;
				}
			}
			m_WaveLength = 0;
		}

		template <class T> 
		void XMInstrument::WaveData<T>::AllocWaveData(const int iLen,const bool bStereo)
		{
			DeleteWaveData();
			m_pWaveDataL = new T[iLen];
			m_pWaveDataR = bStereo?new T[iLen]:NULL;
			m_WaveStereo = bStereo;
			m_WaveLength  = iLen;
			// "bigger than" insted of "bigger or equal", because that means interpolate between loopend and loopstart
			if(m_WaveLoopEnd > m_WaveLength) {m_WaveLoopEnd=m_WaveLength;}
			if(m_WaveSusLoopEnd > m_WaveLength) {m_WaveSusLoopEnd=m_WaveLength;} 
		}

		template <class T> 
		void XMInstrument::WaveData<T>::ConvertToMono()
		{
			if (!m_WaveStereo) return;
			for (std::uint32_t c = 0; c < m_WaveLength; c++)
			{
				m_pWaveDataL[c] = ( m_pWaveDataL[c] + m_pWaveDataR[c] ) / 2;
			}
			m_WaveStereo = false;
			delete[] m_pWaveDataR;
			m_pWaveDataR=0;
		}

		template <class T> 
		void XMInstrument::WaveData<T>::ConvertToStereo()
		{
			if (m_WaveStereo) return;
			m_pWaveDataR = new T[m_WaveLength];
			for (std::uint32_t c = 0; c < m_WaveLength; c++)
			{
				m_pWaveDataR[c] = m_pWaveDataL[c];
			}
			m_WaveStereo = true;
		}

		template <class T> 
		void XMInstrument::WaveData<T>::InsertAt(std::uint32_t insertPos, const WaveData& wave)
		{
			T* oldLeft = m_pWaveDataL;
			T* oldRight = NULL;
			m_pWaveDataL = new T[m_WaveLength+wave.WaveLength()];

			std::memcpy(m_pWaveDataL, oldLeft, insertPos*sizeof(short));
			std::memcpy(&m_pWaveDataL[insertPos], wave.pWaveDataL(), wave.WaveLength()*sizeof(short));
			std::memcpy(&m_pWaveDataL[insertPos+wave.WaveLength()], 
				&oldLeft[insertPos], 
				(m_WaveLength - insertPos)*sizeof(short));

			if(m_WaveStereo)
			{
				oldRight = m_pWaveDataR;
				m_pWaveDataR = new T[m_WaveLength+wave.WaveLength()];
				std::memcpy(m_pWaveDataR, oldRight, insertPos*sizeof(short));
				std::memcpy(&m_pWaveDataR[insertPos], wave.pWaveDataR(), wave.WaveLength()*sizeof(short));
				std::memcpy(&m_pWaveDataR[insertPos+wave.WaveLength()], 
					&oldRight[insertPos], 
					(m_WaveLength - insertPos)*sizeof(short));
			}
			m_WaveLength += wave.WaveLength();
			delete[] oldLeft;
			delete[] oldRight;
		}

		template <class T> 
		void XMInstrument::WaveData<T>::ModifyAt(std::uint32_t modifyPos, const WaveData& wave)
		{
			std::memcpy(&m_pWaveDataL[modifyPos], wave.pWaveDataL(), std::min(wave.WaveLength(), m_WaveLength)*sizeof(short));
			if(m_WaveStereo)
			{
				std::memcpy(&m_pWaveDataR[modifyPos], wave.pWaveDataR(), std::min(wave.WaveLength(), m_WaveLength)*sizeof(short));
			}
		}

		template <class T> 
		void XMInstrument::WaveData<T>::DeleteAt(std::uint32_t deletePos, std::uint32_t length)
		{
			T* oldLeft = m_pWaveDataL;
			T* oldRight = NULL;
			m_pWaveDataL = new T[m_WaveLength-length];

			std::memcpy(m_pWaveDataL, oldLeft, deletePos*sizeof(short));
			std::memcpy(&m_pWaveDataL[deletePos], &oldLeft[deletePos+length], 
				(m_WaveLength - deletePos - length)*sizeof(short));

			if(m_WaveStereo)
			{
				oldRight = m_pWaveDataR;
				m_pWaveDataR = new T[m_WaveLength-length];
				std::memcpy(m_pWaveDataR, oldRight, deletePos*sizeof(short));
				std::memcpy(&m_pWaveDataR[deletePos], &oldRight[deletePos+length], 
					(m_WaveLength - deletePos - length)*sizeof(short));
			}
			m_WaveLength -= length;
			// "bigger than" insted of "bigger or equal", because that means interpolate between loopend and loopstart
			if(m_WaveLoopEnd > m_WaveLength) {m_WaveLoopEnd=m_WaveLength;}
			if(m_WaveSusLoopEnd > m_WaveLength) {m_WaveSusLoopEnd=m_WaveLength;} 

			delete[] oldLeft;
			delete[] oldRight;
		}

		template <class T> 
		void XMInstrument::WaveData<T>::Mix(const WaveData& waveIn, float buf1Vol, float buf2Vol)
		{
			if (m_WaveLength<=0 || waveIn.WaveLength()<0) return;
			T* oldLeft = m_pWaveDataL;
			T* oldRight = m_pWaveDataR;
			bool increased=false;

			if (waveIn.WaveLength() > m_WaveLength) {
				m_pWaveDataL = new T[waveIn.WaveLength()];
				std::memcpy(m_pWaveDataL,oldLeft,m_WaveLength);
				if(m_WaveStereo)
				{
					m_pWaveDataR = new T[waveIn.WaveLength()];
					std::memcpy(m_pWaveDataR,oldRight,m_WaveLength);
				}
				m_WaveLength = waveIn.WaveLength();
				increased=true;
			}

			if (m_WaveStereo) {
				for( int i(0); i<waveIn.WaveLength(); i++ )
				{
					m_pWaveDataL[i] = m_pWaveDataL[i] * buf1Vol + waveIn.pWaveDataL()[i] * buf2Vol;
					m_pWaveDataR[i] = m_pWaveDataR[i] * buf1Vol + waveIn.pWaveDataR()[i] * buf2Vol;
				}
				for( int i(waveIn.WaveLength()); i<m_WaveLength; ++i )
				{
					m_pWaveDataL[i] *= buf1Vol;
					m_pWaveDataR[i] *= buf1Vol;
				}
			}
			else {
				for( int i(0); i<waveIn.WaveLength(); i++ )
				{
					m_pWaveDataL[i] = m_pWaveDataL[i] * buf1Vol + waveIn.pWaveDataL()[i] * buf2Vol;
				}
				for( int i(waveIn.WaveLength()); i<m_WaveLength; ++i )
				{
					m_pWaveDataL[i] *= buf1Vol;
				}
			}
			if (increased) {
				delete[] oldLeft;
				delete[] oldRight;
			}
		}

		template <class T> 
		void XMInstrument::WaveData<T>::Silence(int silStart, int silEnd) 
		{
			if(silStart<0) silStart=0;
			if(silEnd<=0) silEnd=m_WaveLength;
			if(silStart>=m_WaveLength||silEnd>m_WaveLength||silStart==silEnd) return;
			std::memset(&m_pWaveDataL[silStart], 0, silEnd-silStart);
			if (m_WaveStereo) {
				std::memset(&m_pWaveDataR[silStart], 0, silEnd-silStart);
			}
		}

		//Fade - fades an audio buffer from one volume level to another.
		template <class T> 
		void XMInstrument::WaveData<T>::Fade(int fadeStart, int fadeEnd, float startVol, float endVol)
		{
			if(fadeStart<0) fadeStart=0;
			if(fadeEnd<=0) fadeEnd=m_WaveLength;
			if(fadeStart>=m_WaveLength||fadeEnd>m_WaveLength||fadeStart==fadeEnd) return;

			float slope = (endVol-startVol)/(float)(fadeEnd-fadeStart);
			if (m_WaveStereo) {
				for(int i(fadeStart);i<fadeEnd;++i) {
					m_pWaveDataL[i] *= startVol+i*slope;
					m_pWaveDataR[i] *= startVol+i*slope;
				}
			}
			else {
				for(int i(fadeStart);i<fadeEnd;++i) {
					m_pWaveDataL[i] *= startVol+i*slope;
				}
			}
		}

		//Amplify - multiplies an audio buffer by a given factor.  buffer can be inverted by passing
		//	a negative value for vol.
		template <class T> 
		void XMInstrument::WaveData<T>::Amplify(int ampStart, int ampEnd, float vol)
		{
			if(ampStart<0) ampStart=0;
			if(ampEnd<=0) ampEnd=m_WaveLength;
			if(ampStart>=m_WaveLength||ampEnd>m_WaveLength||ampStart==ampEnd) return;
			//Todo: Templatize
			if (m_WaveStereo) {
				for(int i(ampStart);i<ampEnd;++i) {
					m_pWaveDataL[i] = math::rint_clip<std::int16_t>(m_pWaveDataL[i] * vol);
					m_pWaveDataR[i] = math::rint_clip<std::int16_t>(m_pWaveDataR[i] * vol);
				}
			}
			else { 
				for(int i(ampStart);i<ampEnd;++i) {
					m_pWaveDataL[i] = math::rint_clip<std::int16_t>(m_pWaveDataL[i] * vol);
				}
			}
		}

		template <class T> 
		void XMInstrument::WaveData<T>::WaveSampleRate(const std::uint32_t value){
			m_WaveSampleRate = value;
		}

		template <class T> 
		int XMInstrument::WaveData<T>::Load(RiffFile& riffFile)
		{	
			std::uint32_t size1,size2;
			
			char temp[8];
			std::uint32_t filevers;
			int size=0;
			riffFile.Read(temp,4); temp[4]='\0';
			riffFile.Read(size);
			if (strcmp(temp,"SMPD")) return size+8;

			riffFile.Read(filevers);
			if (filevers == 0 || filevers > 0x1F) {
				riffFile.Seek(riffFile.GetPos()-sizeof(filevers));
				filevers = 0;
			}

			CT2A _wave_name("");
			riffFile.ReadStringA2T(_wave_name,32);
			m_WaveName=_wave_name;

			riffFile.Read(m_WaveLength);
			riffFile.Read(m_WaveGlobVolume);
			riffFile.Read(m_WaveDefVolume);

			riffFile.Read(m_WaveLoopStart);
			riffFile.Read(m_WaveLoopEnd);
			{ std::uint32_t i(0); riffFile.Read(i); m_WaveLoopType = static_cast<LoopType::Type>(i); }

			riffFile.Read(m_WaveSusLoopStart);
			riffFile.Read(m_WaveSusLoopEnd);
			{ std::uint32_t i(0); riffFile.Read(i); m_WaveSusLoopType = static_cast<LoopType::Type>(i); }
			// "bigger than" insted of "bigger or equal", because that means interpolate between loopend and loopstart
			if(m_WaveLoopEnd > m_WaveLength) {m_WaveLoopEnd=m_WaveLength;}
			if(m_WaveSusLoopEnd > m_WaveLength) {m_WaveSusLoopEnd=m_WaveLength;} 

			if(filevers == 1) {
				riffFile.Read(m_WaveSampleRate);
			}
			else {
				m_WaveSampleRate=8363;
			}
			riffFile.Read(m_WaveTune);
			riffFile.Read(m_WaveFineTune);

			riffFile.Read(m_WaveStereo);
			riffFile.Read(m_PanEnabled);
			riffFile.Read(m_PanFactor);
			if(filevers == 1) {
				riffFile.Read(m_Surround);
			}
			else if (m_PanFactor > 1.0f) {
				m_Surround = true;
				m_PanFactor = m_PanFactor-=1.0f;
			} else { m_Surround = false; }

			riffFile.Read(m_VibratoAttack);
			riffFile.Read(m_VibratoAttack);
			riffFile.Read(m_VibratoDepth);
			riffFile.Read(m_VibratoType);

			riffFile.Read(size1);
			unsigned char * pData = new unsigned char[size1];
			riffFile.Read((void*)pData,size1);
			SoundDesquash(pData, &m_pWaveDataL);
			delete[] pData;
			
			if (m_WaveStereo)
			{
				riffFile.Read(size2);
				pData = new unsigned char[size2];
				riffFile.Read(pData,size2);
				SoundDesquash(pData, &m_pWaveDataR);
				delete[] pData;
			}
			return size+8;
		}

		template <class T> 
		bool XMInstrument::WaveData<T>::SoundDesquash(uint8_t const * pSourcePos, int16_t ** pDestination) {
			return DataCompression::SoundDesquash( pSourcePos, pDestination);
		}

		template <class T> 
        bool XMInstrument::WaveData<T>::SoundDesquash(uint8_t const * pSourcePos, float ** pDestination) {
			assert(0); // not implemented yet
			return false;
		}
		
		template <class T> 
		std::size_t XMInstrument::WaveData<T>::SoundSquash(int16_t const * pSource, uint8_t ** pDestination, std::size_t size) const {
			return DataCompression::SoundSquash( pSource, pDestination, size);
		}

		template <class T> 
        std::size_t XMInstrument::WaveData<T>::SoundSquash(float const * pSource, uint8_t ** pDestination, std::size_t size) const {
			assert(0); // not implemented yet
			return 0;
		}

		template <class T> 
		void XMInstrument::WaveData<T>::Save(RiffFile& riffFile) const
		{
			unsigned char * pData1(0);
			unsigned char * pData2(0);
			std::uint32_t size1= static_cast<uint32_t>(SoundSquash(m_pWaveDataL,&pData1,m_WaveLength));
			std::uint32_t size2(0);

			if (m_WaveStereo)
			{
				size2 = static_cast<uint32_t>(SoundSquash(m_pWaveDataR,&pData2,m_WaveLength));
			}

			CT2A _wave_name(m_WaveName.c_str());

			int size = 0;
			riffFile.Write("SMPD",4);
			riffFile.Write(size);
			size_t filepos = riffFile.GetPos();
			riffFile.Write(WAVEVERSION);

			riffFile.Write(_wave_name, std::strlen(_wave_name) + 1);

			riffFile.Write(m_WaveLength);
			riffFile.Write(m_WaveGlobVolume);
			riffFile.Write(m_WaveDefVolume);

			riffFile.Write(m_WaveLoopStart);
			riffFile.Write(m_WaveLoopEnd);
			{ std::uint32_t i = m_WaveLoopType; riffFile.Write(i); }

			riffFile.Write(m_WaveSusLoopStart);
			riffFile.Write(m_WaveSusLoopEnd);
			{ std::uint32_t i = m_WaveSusLoopType; riffFile.Write(i); }

			riffFile.Write(m_WaveSampleRate);
			riffFile.Write(m_WaveTune);
			riffFile.Write(m_WaveFineTune);

			riffFile.Write(m_WaveStereo);
			riffFile.Write(m_PanEnabled);
			riffFile.Write(m_PanFactor);
			riffFile.Write(m_Surround);

			riffFile.Write(m_VibratoAttack);
			riffFile.Write(m_VibratoSpeed);
			riffFile.Write(m_VibratoDepth);
			riffFile.Write(m_VibratoType);

			riffFile.Write(size1);
			riffFile.Write((void*)pData1,size1);
			delete[] pData1;
			
			if (m_WaveStereo)
			{
				riffFile.Write(size2);
				riffFile.Write((void*)pData2,size2);
				delete[] pData2;
			}
			size_t endpos = riffFile.GetPos();
			riffFile.Seek(filepos-4);
			{ std::uint32_t i = static_cast<uint32_t>(endpos - filepos); riffFile.Write(i); }
			riffFile.Seek(endpos);
		}


	template class XMInstrument::WaveData<std::int16_t>;
	template class XMInstrument::WaveData<float>;

//////////////////////////////////////////////////////////////////////////
//  XMInstrument::Envelope Implementation.

		void XMInstrument::Envelope::Init()
		{	m_Enabled = false;
			m_Carry = false;
			m_SustainBegin = INVALID;
			m_SustainEnd = INVALID;
			m_LoopStart = INVALID;
			m_LoopEnd = INVALID;
			m_Mode = Mode::TICK;
			m_Points.clear();
		}
		/** 
		* @param pointIndex : Current point index.
		* @param pointTime  : Desired point Time.
		* @param value		: Desired point Value.
		* @return			: New point index.
		*/
		int XMInstrument::Envelope::SetTimeAndValue(const unsigned int pointIndex,const int pointTime,const ValueType pointVal)
		{
			assert(pointIndex < m_Points.size());
			if(pointIndex < m_Points.size())
			{
				int prevtime,nextime;
				m_Points[pointIndex].first = pointTime;
				m_Points[pointIndex].second = pointVal;

				prevtime=(pointIndex == 0)?m_Points[pointIndex].first:m_Points[pointIndex-1].first;
				nextime=(pointIndex == m_Points.size()-1 )?m_Points[pointIndex].first:m_Points[pointIndex+1].first;
					
				// Initialization done. Check if we have to move the point to a new index:


				// Is the Time already between the previous and next?
				if( pointTime >= prevtime && pointTime <= nextime)
				{
					return pointIndex;
				}

				// Else, we have some work to do.
				unsigned int new_index = pointIndex;

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
					} while(new_index<m_Points.size()-1);

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
		unsigned int XMInstrument::Envelope::Insert(const int pointTime,const ValueType pointVal)
		{
			unsigned int _new_index;
			for(_new_index = 0;_new_index < (int)m_Points.size();_new_index++)
			{
				if(pointTime < m_Points[_new_index].first)
				{
					PointValue _point;
					_point.first = pointTime;
					_point.second = pointVal;

					m_Points.insert(m_Points.begin() + _new_index,_point);

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
			if(_new_index == m_Points.size())
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
		void XMInstrument::Envelope::Delete(const unsigned int pointIndex)
		{
			assert(pointIndex < m_Points.size());
			if(pointIndex < m_Points.size())
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
	
		/// Loading Procedure
		void XMInstrument::Envelope::Load(RiffFile& riffFile,const std::uint32_t version)
		{
			riffFile.Read(m_Enabled);
			riffFile.Read(m_Carry);
			{
				std::int32_t i32(0);
				riffFile.Read(i32); m_LoopStart = i32;
				riffFile.Read(i32); m_LoopEnd = i32;
				riffFile.Read(i32); m_SustainBegin = i32;
				riffFile.Read(i32); m_SustainEnd = i32;
			}
			{
				uint32_t num_of_points = 0; riffFile.Read(num_of_points);
				for(int i = 0; i < num_of_points; i++){
					PointValue value;
					riffFile.Read(value.first); // point
					riffFile.Read(value.second); // value
					m_Points.push_back(value);
				}
			}
			//TODO: Update version to add mode
		}

		/// Saving Procedure
		void XMInstrument::Envelope::Save(RiffFile& riffFile, const std::uint32_t version) const
		{
			// Envelopes don't neeed ID and/or version. they are part of the instrument chunk.
			riffFile.Write(m_Enabled);
			riffFile.Write(m_Carry);
			{
				int32_t i;
				i = m_LoopStart; riffFile.Write(i);
				i = m_LoopEnd; riffFile.Write(i);
				i = m_SustainBegin; riffFile.Write(i);
				i = m_SustainEnd; riffFile.Write(i);
			}
			{ uint32_t s = static_cast<uint32_t>(m_Points.size()); riffFile.Write(s); }

			for(std::size_t i = 0; i < m_Points.size(); i++)
			{
				riffFile.Write(m_Points[i].first); // point
				riffFile.Write(m_Points[i].second); // value
			}
			//TODO: Update version to add mode
		}


//////////////////////////////////////////////////////////////////////////
//   XMInstrument Implementation
		XMInstrument::XMInstrument()
		{
			Init();
		}

		/// destructor
		XMInstrument::~XMInstrument()
		{
			// No need to delete anything, since we don't allocate memory explicitely.
		}

		/// other functions
		void XMInstrument::Init()
		{
			m_bEnabled = false;

			m_Name = "";

			m_Lines = 16;

			m_GlobVol = 1.0f;
			m_VolumeFadeSpeed = 0;

			m_PanEnabled=false;
			m_InitPan = 0.5f;
			m_Surround = false;
			m_NoteModPanCenter = notecommands::middleC;
			m_NoteModPanSep = 0;

			m_FilterCutoff = 127;
			m_FilterResonance = 0;
			m_FilterEnvAmount = 0;
			m_FilterType = dsp::F_NONE;

			m_RandomVolume = 0;
			m_RandomPanning = 0;
			m_RandomCutoff = 0;
			m_RandomResonance = 0;

			m_NNA = NewNoteAction::STOP;
			m_DCT = DupeCheck::NONE;
			m_DCA = NewNoteAction::STOP;

			SetDefaultNoteMap();

			m_AmpEnvelope.Init();
			m_PanEnvelope.Init();
			m_PitchEnvelope.Init();
			m_FilterEnvelope.Init();

		}
		void XMInstrument::SetDefaultNoteMap() {
			NotePair npair;
			npair.second=255;
			for(int i = 0;i < NOTE_MAP_SIZE;i++){
				npair.first=i;
				m_AssignNoteToSample[i] = npair;
			}
		}

		/// load XMInstrument
		int XMInstrument::Load(RiffFile& riffFile)
		{
			char temp[8];
			int size=0;
			riffFile.Read(temp,4); temp[4]='\0';
			riffFile.Read(size);
			if (strcmp(temp,"INST")) return size+8;

			//\todo: add version
			//riffFile.Read(version);

			m_bEnabled = true;
			CT2A _name("");
			riffFile.ReadStringA2T(_name,32);
			m_Name=_name;

			riffFile.Read(m_Lines);

			riffFile.Read(m_GlobVol);
			riffFile.Read(m_VolumeFadeSpeed);

			riffFile.Read(m_InitPan);
			riffFile.Read(m_PanEnabled);
			//TODO: ADD surround
			riffFile.Read(m_NoteModPanCenter);
			riffFile.Read(m_NoteModPanSep);

			riffFile.Read(m_FilterCutoff);
			riffFile.Read(m_FilterResonance);
			riffFile.Read(m_FilterEnvAmount);
			{ std::uint32_t i(0); riffFile.Read(i); m_FilterType = static_cast<dsp::FilterType>(i); }

			riffFile.Read(m_RandomVolume);
			riffFile.Read(m_RandomPanning);
			riffFile.Read(m_RandomCutoff);
			riffFile.Read(m_RandomResonance);

			{
				std::uint32_t i(0);
				riffFile.Read(i); m_NNA = static_cast<NewNoteAction::Type>(i);
				riffFile.Read(i); m_DCT = static_cast<DupeCheck::Type>(i);
				riffFile.Read(i); m_DCA = static_cast<NewNoteAction::Type>(i);
			}

			NotePair npair;
			for(int i = 0;i < NOTE_MAP_SIZE;i++){
				riffFile.Read(npair.first);
				riffFile.Read(npair.second);
				NoteToSample(i,npair);
			}

			int version = 0;
			m_AmpEnvelope.Load(riffFile,version);
			m_PanEnvelope.Load(riffFile,version);
			m_FilterEnvelope.Load(riffFile,version);
			m_PitchEnvelope.Load(riffFile,version);
			return size+8;//Size of the whole data, including chunk header.
		}

		// save XMInstrument
		void XMInstrument::Save(RiffFile& riffFile) const
		{
			if ( ! m_bEnabled ) return;

			int i;
			int size = 0;
			riffFile.Write("INST",4);
			riffFile.Write(size);
			size_t filepos = riffFile.GetPos();
			//\todo : add version.

			CT2A _name(m_Name.c_str());
			riffFile.Write(_name,strlen(_name) + 1);

//			riffFile.Write(m_bEnabled);

			riffFile.Write(m_Lines);

			riffFile.Write(m_GlobVol);
			riffFile.Write(m_VolumeFadeSpeed);

			riffFile.Write(m_InitPan);
			riffFile.Write(m_PanEnabled);
			//TODO: ADD surround
			riffFile.Write(m_NoteModPanCenter);
			riffFile.Write(m_NoteModPanSep);

			riffFile.Write(m_FilterCutoff);
			riffFile.Write(m_FilterResonance);
			riffFile.Write(m_FilterEnvAmount);
			{ std::uint32_t i = m_FilterType; riffFile.Write(i); }

			riffFile.Write(m_RandomVolume);
			riffFile.Write(m_RandomPanning);
			riffFile.Write(m_RandomCutoff);
			riffFile.Write(m_RandomResonance);
			{
				std::uint32_t i;
				i = m_NNA; riffFile.Write(i);
				i = m_DCT; riffFile.Write(i);
				i = m_DCA; riffFile.Write(i);
			}


			NotePair npair;
			for(i = 0;i < NOTE_MAP_SIZE;i++){
				npair = NoteToSample(i);
				riffFile.Write(npair.first);
				riffFile.Write(npair.second);
			}

			int version = 0;
			m_AmpEnvelope.Save(riffFile,version);
			m_PanEnvelope.Save(riffFile,version);
			m_FilterEnvelope.Save(riffFile,version);
			m_PitchEnvelope.Save(riffFile,version);
			size_t endpos = riffFile.GetPos();
			riffFile.Seek(filepos-4);
			{ std::uint32_t i = static_cast<uint32_t>(endpos - filepos); riffFile.Write(i); }
			riffFile.Seek(endpos);
		}

	} //namespace host
}// namespace psycle

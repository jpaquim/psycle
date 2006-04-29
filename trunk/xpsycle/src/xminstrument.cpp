/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
 *   natti@linux   *
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
#include "xminstrument.h"

void XMInstrument::WaveData::Load(DeSerializer* file)
{ /// todo port
   uint32_t size1,size2;

   //\todo: add version
   //riffFile->Read(version);

   // CT2A _wave_name(""); ???
   /*riffFile->ReadStringA2T(_wave_name,32);
			m_WaveName=_wave_name;

			riffFile->Read(m_WaveLength);
			riffFile->Read(m_WaveGlobVolume);
			riffFile->Read(m_WaveDefVolume);

			riffFile->Read(m_WaveLoopStart);
			riffFile->Read(m_WaveLoopEnd);
			riffFile->Read(&m_WaveLoopType,sizeof(m_WaveLoopType));

			riffFile->Read(m_WaveSusLoopStart);
			riffFile->Read(m_WaveSusLoopEnd);
			riffFile->Read(&m_WaveSusLoopType,sizeof(m_WaveSusLoopType));

			riffFile->Read(m_WaveTune);
			riffFile->Read(m_WaveFineTune);

			riffFile->Read(m_WaveStereo);
			riffFile->Read(m_PanEnabled);
			riffFile->Read(m_PanFactor);

			riffFile->Read(m_VibratoAttack);
			riffFile->Read(m_VibratoSpeed);
			riffFile->Read(m_VibratoDepth);
			riffFile->Read(m_VibratoType);

			riffFile->Read(size1);
			byte* pData;
			pData = new byte[size1];
			riffFile->Read((void *)pData,size1);
			SoundDesquash(pData,&m_pWaveDataL);
			
			if (m_WaveStereo)
			{
				delete pData;
				riffFile->Read(size2);
				pData = new byte[size2];
				riffFile->Read(pData,size2);
				SoundDesquash(pData,&m_pWaveDataR);
			}
			delete pData;
 */
}

void XMInstrument::WaveData::Save(Serializer* file)
{ /// todo port
/*			byte * pData1(0);
			byte * pData2(0);
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

			riffFile->Write("SMPD");
			riffFile->Write(size);
			//\todo: add version

			riffFile->Write(_wave_name,strlen(_wave_name) + 1);

			riffFile->Write(m_WaveLength);
			riffFile->Write(m_WaveGlobVolume);
			riffFile->Write(m_WaveDefVolume);

			riffFile->Write(m_WaveLoopStart);
			riffFile->Write(m_WaveLoopEnd);
			riffFile->Write(&m_WaveLoopType,sizeof(m_WaveLoopType));

			riffFile->Write(m_WaveSusLoopStart);
			riffFile->Write(m_WaveSusLoopEnd);
			riffFile->Write(&m_WaveSusLoopType,sizeof(m_WaveSusLoopType));

			riffFile->Write(m_WaveTune);
			riffFile->Write(m_WaveFineTune);

			riffFile->Write(m_WaveStereo);
			riffFile->Write(m_PanEnabled);
			riffFile->Write(m_PanFactor);

			riffFile->Write(m_VibratoAttack);
			riffFile->Write(m_VibratoSpeed);
			riffFile->Write(m_VibratoDepth);
			riffFile->Write(m_VibratoType);

			riffFile->Write(size1);
			riffFile->Write((void*)pData1,size1);
			delete pData1;
			
			if (m_WaveStereo)
			{
				riffFile->Write(size2);
				riffFile->Write((void*)pData2,size2);
				delete pData2;
			}*/
}


XMInstrument::XMInstrument()
{
}


XMInstrument::~XMInstrument()
{
}



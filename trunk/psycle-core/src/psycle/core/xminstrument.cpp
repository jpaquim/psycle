/***************************************************************************
	*   Copyright (C) 2007 by Psycledelics     *
	*   psycle.sf.net   *
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
	*   xxxxxxxxxxxxxxxxxxxxxxx
	*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
	*   WRONG ADDRESS
	*   xxxxxxxxxxxxxxxxxxxxxxx
	***************************************************************************/
//#include <psycle/core/psycleCorePch.hpp>

#include <cassert>

#include "xminstrument.h"

namespace psy { namespace core {

XMInstrument::XMInstrument()
{
}


XMInstrument::~XMInstrument()
{
}

XMInstrument::WaveData::WaveData() {
  m_pWaveDataL = m_pWaveDataR = NULL;
  Init();
};

void XMInstrument::WaveData::Init(){
  DeleteWaveData();
  m_WaveLength = 0;
  m_WaveGlobVolume = 1.0f; // Global volume ( global multiplier )
  m_WaveDefVolume = 128; // Default volume ( volume at which it starts to play. corresponds to 0Cxx/volume command )
  m_WaveLoopStart = 0;
  m_WaveLoopEnd = 0;
  m_WaveLoopType = DO_NOT;
  m_WaveSusLoopStart = 0;
  m_WaveSusLoopEnd = 0;
  m_WaveSusLoopType = DO_NOT;
  //todo: Add SampleRate functionality, and change WaveTune's one.
  // This means modifying the functions PeriodToSpeed (for linear slides) and NoteToPeriod (for amiga slides)
  m_WaveSampleRate = 8363;
  m_WaveTune = 0;
  m_WaveFineTune = 0;
  m_WaveStereo = false;
  m_PanFactor = 0.5f;
  m_PanEnabled = false;
  m_VibratoAttack = 0;
  m_VibratoSpeed = 0;
  m_VibratoDepth = 0;
  m_VibratoType = 0;
}

XMInstrument::WaveData::~WaveData(){
  DeleteWaveData();
};

void XMInstrument::WaveData::DeleteWaveData(){
  delete [] m_pWaveDataL; m_pWaveDataL = NULL;
  delete [] m_pWaveDataR; m_pWaveDataR = NULL;
}

void XMInstrument::WaveData::AllocWaveData(const int iLen,const bool bStereo){
  DeleteWaveData();
  m_pWaveDataL = new signed short[iLen];
  m_pWaveDataR = bStereo?new signed short[iLen]:NULL;
  m_WaveStereo = bStereo;
  m_WaveLength  = iLen;
}

void XMInstrument::WaveData::operator= (const XMInstrument::WaveData::WaveData& source){
  Init();
  m_WaveName = source.m_WaveName;
  m_WaveLength = source.m_WaveLength;
  m_WaveGlobVolume = source.m_WaveGlobVolume;
  m_WaveDefVolume = source.m_WaveDefVolume;
  m_WaveLoopStart = source.m_WaveLoopStart;
  m_WaveLoopEnd = source.m_WaveLoopEnd;
  m_WaveLoopType = source.m_WaveLoopType;
  m_WaveSusLoopStart = source.m_WaveSusLoopStart;
  m_WaveSusLoopEnd = source.m_WaveSusLoopEnd;
  m_WaveSusLoopType = source.m_WaveSusLoopType;
  m_WaveTune = source.m_WaveTune;
  m_WaveFineTune = source.m_WaveFineTune;
  m_WaveStereo = source.m_WaveStereo;
  m_VibratoAttack = source.m_VibratoAttack;
  m_VibratoSpeed = source.m_VibratoSpeed;
  m_VibratoDepth = source.m_VibratoDepth;
  m_VibratoType = source.m_VibratoType;
  
  AllocWaveData(source.m_WaveLength,source.m_WaveStereo);
  
  memcpy(m_pWaveDataL,source.m_pWaveDataL,source.m_WaveLength * sizeof(short));
  if(source.m_WaveStereo){
    memcpy(m_pWaveDataR,source.m_pWaveDataR,source.m_WaveLength * sizeof(short));
  }
}



const std::string XMInstrument::WaveData::WaveName(){ return m_WaveName;};
void XMInstrument::WaveData::WaveName(std::string newname){ m_WaveName = newname;};

uint32_t  XMInstrument::WaveData::WaveLength(){ return m_WaveLength;};
void XMInstrument::WaveData::WaveLength (const uint32_t value){m_WaveLength = value;};

const float XMInstrument::WaveData::WaveGlobVolume()const{ return m_WaveGlobVolume;};
void XMInstrument::WaveData::WaveGlobVolume(const float value){m_WaveGlobVolume = value;};
const uint16_t XMInstrument::WaveData::WaveVolume(){ return m_WaveDefVolume;};
void XMInstrument::WaveData::WaveVolume(const uint16_t value){m_WaveDefVolume = value;};

const float XMInstrument::WaveData::PanFactor(){ return m_PanFactor;};// Default position for panning ( 0..1 ) 0left 1 right. Bigger than XMSampler::SURROUND_THRESHOLD -> Surround!
void XMInstrument::WaveData::PanFactor(const float value){m_PanFactor = value;};
bool XMInstrument::WaveData::PanEnabled(){ return m_PanEnabled;};
void XMInstrument::WaveData::PanEnabled(bool pan){ m_PanEnabled=pan;};

const uint32_t XMInstrument::WaveData::WaveLoopStart(){ return m_WaveLoopStart;};
void XMInstrument::WaveData::WaveLoopStart(const uint32_t value){m_WaveLoopStart = value;};
const uint32_t XMInstrument::WaveData::WaveLoopEnd(){ return m_WaveLoopEnd;};
void XMInstrument::WaveData::WaveLoopEnd(const uint32_t value){m_WaveLoopEnd = value;};
const XMInstrument::WaveData::LoopType XMInstrument::WaveData::WaveLoopType(){ return m_WaveLoopType;}
void XMInstrument::WaveData::WaveLoopType(const XMInstrument::WaveData::LoopType value){ m_WaveLoopType = value;};

const uint32_t XMInstrument::WaveData::WaveSusLoopStart(){ return m_WaveSusLoopStart;};
void XMInstrument::WaveData::WaveSusLoopStart(const uint32_t value){m_WaveSusLoopStart = value;};
const uint32_t XMInstrument::WaveData::WaveSusLoopEnd(){ return m_WaveSusLoopEnd;};
void XMInstrument::WaveData::WaveSusLoopEnd(const uint32_t value){m_WaveSusLoopEnd = value;};
const XMInstrument::WaveData::LoopType XMInstrument::WaveData::WaveSusLoopType(){ return m_WaveSusLoopType;};
void XMInstrument::WaveData::WaveSusLoopType(const XMInstrument::WaveData::LoopType value){ m_WaveSusLoopType = value;};

const int16_t XMInstrument::WaveData::WaveTune(){return m_WaveTune;};
void XMInstrument::WaveData::WaveTune(const int16_t value){m_WaveTune = value;};
const int16_t XMInstrument::WaveData::WaveFineTune(){return m_WaveFineTune;};
void XMInstrument::WaveData::WaveFineTune(const int16_t value){m_WaveFineTune = value;};
const uint32_t XMInstrument::WaveData::WaveSampleRate(){return m_WaveSampleRate;};
void XMInstrument::WaveData::WaveSampleRate(const uint32_t value){m_WaveSampleRate = value;};

const bool XMInstrument::WaveData::IsWaveStereo(){ return m_WaveStereo;};
void XMInstrument::WaveData::IsWaveStereo(const bool value){ m_WaveStereo = value;};

const uint8_t XMInstrument::WaveData::VibratoType(){return m_VibratoType;};
const uint8_t XMInstrument::WaveData::VibratoSpeed(){return m_VibratoSpeed;};
const uint8_t XMInstrument::WaveData::VibratoDepth(){return m_VibratoDepth;};
const uint8_t XMInstrument::WaveData::VibratoAttack(){return m_VibratoAttack;};

void XMInstrument::WaveData::VibratoType(const uint8_t value){m_VibratoType = value ;};
void XMInstrument::WaveData::VibratoSpeed(const uint8_t value){m_VibratoSpeed = value ;};
void XMInstrument::WaveData::VibratoDepth(const uint8_t value){m_VibratoDepth = value ;};
void XMInstrument::WaveData::VibratoAttack(const uint8_t value){m_VibratoAttack = value ;};

const bool XMInstrument::WaveData::IsAutoVibrato(){return m_VibratoDepth && m_VibratoSpeed;};

const signed short * XMInstrument::WaveData::pWaveDataL(){ return m_pWaveDataL;};
const signed short * XMInstrument::WaveData::pWaveDataR(){ return m_pWaveDataR;};

signed short XMInstrument::WaveData::WaveDataL(const uint32_t index) const { assert(index<m_WaveLength); return (*(m_pWaveDataL + index));};
signed short XMInstrument::WaveData::WaveDataR(const uint32_t index) const { assert(index<m_WaveLength); return (*(m_pWaveDataR + index));};

void XMInstrument::WaveData::WaveDataL(const uint32_t index,const signed short value){ assert(index<m_WaveLength); *(m_pWaveDataL + index) = value;};
void XMInstrument::WaveData::WaveDataR(const uint32_t index,const signed short value){ assert(index<m_WaveLength); *(m_pWaveDataR + index) = value;};

}}


// -*- mode:c++; indent-tabs-mode:t -*-
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

XMInstrument::WaveData::WaveData()
:
	m_pWaveDataL(),
	m_pWaveDataR()
{
	Init();
}

XMInstrument::WaveData::WaveData(unsigned int samples, bool bStereo)
:
	m_pWaveDataL(),
	m_pWaveDataR()
{
	Init();
	AllocWaveData(samples, bStereo);
}

XMInstrument::WaveData::WaveData(XMInstrument::WaveData const & source)
:
	m_pWaveDataL(),
	m_pWaveDataR()
{
	*this = source;
}

void XMInstrument::WaveData::Init() {
	DeleteWaveData();
	m_WaveLength = 0;
	m_WaveGlobVolume = 1.0f;
	m_WaveDefVolume = 128;
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

XMInstrument::WaveData::~WaveData() {
	DeleteWaveData();
}

void XMInstrument::WaveData::DeleteWaveData() {
	delete [] m_pWaveDataL; m_pWaveDataL = 0;
	delete [] m_pWaveDataR; m_pWaveDataR = 0;
}

void XMInstrument::WaveData::AllocWaveData(unsigned int samples, bool bStereo) {
	DeleteWaveData();
	m_pWaveDataL = new std::int16_t[samples];
	m_pWaveDataR = bStereo ? new std::int16_t[samples] : 0;
	m_WaveStereo = bStereo;
	m_WaveLength = samples;
}

XMInstrument::WaveData const & XMInstrument::WaveData::operator=(XMInstrument::WaveData const & source) {
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

	std::memcpy(m_pWaveDataL, source.m_pWaveDataL, source.m_WaveLength * sizeof *m_pWaveDataL);
	if(source.m_WaveStereo)
		std::memcpy(m_pWaveDataR, source.m_pWaveDataR, source.m_WaveLength * sizeof *m_pWaveDataR);

	return *this;
}
}}

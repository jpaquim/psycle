// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "instpreview.h"
#include "instrument.h"
#include <cstdint>

namespace psy { namespace core {

void InstPreview::Work(float * pInSamplesL, float * pInSamplesR, int numSamples) {
	if(!m_pInstrument) return;

	float * pSamplesL = pInSamplesL;
	float * pSamplesR = pInSamplesR;
	--pSamplesL;
	--pSamplesR;
	
	std::int16_t * wl = m_pInstrument->waveDataL;
	std::int16_t * wr = m_pInstrument->waveDataR;
	float ld = 0;
	float rd = 0;
	
	do {
		ld = (*(wl + m_pos)) * m_vol;
		if(m_pInstrument->waveStereo) rd = (*(wr + m_pos)) * m_vol;
		else rd = ld;
			
		*(++pSamplesL) += ld;
		*(++pSamplesR) += rd;
			
		if(++m_pos>=m_pInstrument->waveLength) {
			Stop();
			return;
		}
		if(m_bLoop && m_pInstrument->waveLoopEnd == m_pos) {
			m_pos=m_pInstrument->waveLoopStart;
		}
	} while(--numSamples);
}

void InstPreview::Play(unsigned long startPos/* = 0 */) {
	if(!m_pInstrument) return;

	m_bLoop = m_pInstrument->waveLoopType;
	if(startPos < m_pInstrument->waveLength) {
		m_bPlaying = true;
		m_pos = startPos;
	}
}

void InstPreview::Stop() {
	m_bPlaying = false;
}

void InstPreview::Release() {
	m_bLoop = false;
}

}}

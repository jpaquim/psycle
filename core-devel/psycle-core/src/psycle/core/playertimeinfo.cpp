// -*- mode:c++; indent-tabs-mode:t -*-
/**************************************************************************
*   Copyright 2007 Psycledelics http://psycle.sourceforge.net             *
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
#include <psycle/core/psycleCorePch.hpp>
#include "playertimeinfo.h"
#include <cassert>

namespace psy { namespace core {

PlayerTimeInfo::PlayerTimeInfo( )
:
	playBeatPos_(0.0),
	samplePos_(0),
	ticks_(4),
	isTicks_(true),
	bpm_(125.0),
	sampleRate_(44100)
{
	recalcSPB();
	recalcSPT();
}

PlayerTimeInfo::~ PlayerTimeInfo( )
{
}

void PlayerTimeInfo::setPlayBeatPos( double pos )
{
	assert(pos >= 0);
	playBeatPos_ = pos;
}

void PlayerTimeInfo::setSamplePos( int pos )
{
	assert(pos >= 0);
	samplePos_ = pos;
}

void PlayerTimeInfo::setTicksSpeed( int ticks, bool isticks )
{
	assert(ticks > 0);
	ticks_ = ticks;
	isTicks_ = isticks;
	recalcSPT();
}

void PlayerTimeInfo::setBpm( double bpm )
{
	assert(bpm > 0);
	bpm_ = bpm;
	recalcSPB();
	recalcSPT();
}

void PlayerTimeInfo::setSampleRate( int rate )
{
	assert(rate > 0);
	sampleRate_ = rate;
	recalcSPB();
	recalcSPT();
}

void PlayerTimeInfo::recalcSPB( )
{
	samplesPerBeat_ = (sampleRate_*60) / bpm_;
	assert(samplesPerBeat_ > 0);
}

void PlayerTimeInfo::recalcSPT( )
{
	if ( isTicks_ )
	{
		samplesPerTick_ = samplesPerBeat() / ticks_;
	}
	else
	{
		samplesPerTick_ =  samplesPerBeat() * ticks_ / 24.0f;
	}
	assert(samplesPerTick_ > 0);
}

}}

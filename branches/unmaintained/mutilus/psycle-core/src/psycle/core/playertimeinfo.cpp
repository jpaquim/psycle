/***************************************************************************
	*   Copyright (C) 2007 Psycledelics     *
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
	*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
	***************************************************************************/
#include <psycle/core/psycleCorePch.hpp>

#include "playertimeinfo.h"

/// class for play time informations

namespace psy
{
	namespace core
	{
		PlayerTimeInfo::PlayerTimeInfo( )
			: playBeatPos_(0.0),
				samplePos_(0),
				lpb_(4),
				bpm_(125.0),
				sampleRate_(44100),
				samplesPerBeat_(0),
				samplesPerRow_(0)
		{
		recalcSPB();
		recalcSPR();
		}

		PlayerTimeInfo::~ PlayerTimeInfo( )
		{
		}

		void PlayerTimeInfo::setPlayBeatPos( double pos )
		{
		assert(pos >= 0);
			playBeatPos_ = pos;
		}

		double PlayerTimeInfo::playBeatPos( ) const
		{
			return playBeatPos_;
		}

		void PlayerTimeInfo::setSamplePos( int pos )
		{
		assert(pos >= 0);
			samplePos_ = pos;
		}

		int PlayerTimeInfo::samplePos( ) const
		{
			return samplePos_;
		}

		void PlayerTimeInfo::setLinesPerBeat( int lines )
		{
		assert(lines > 0);
			lpb_ = lines;
			recalcSPR();
		}

		int PlayerTimeInfo::linesPerBeat( ) const
		{
			return lpb_;
		}

		void PlayerTimeInfo::setBpm( double bpm )
		{
		assert(bpm > 0);
			bpm_ = bpm;
			recalcSPB();
			recalcSPR();
		}

		double PlayerTimeInfo::bpm( ) const
		{
			return bpm_;
		}

		void PlayerTimeInfo::setSampleRate( int rate )
		{
		assert(rate > 0);
			sampleRate_ = rate;
			recalcSPB();
			recalcSPR();
		}

		int PlayerTimeInfo::sampleRate( ) const
		{
			return sampleRate_;
		}

		float PlayerTimeInfo::samplesPerBeat( ) const
		{
			return samplesPerBeat_;
		}

		float PlayerTimeInfo::samplesPerRow( ) const
		{
			return samplesPerRow_;
		}

		void PlayerTimeInfo::recalcSPB( )
		{
			samplesPerBeat_ = (sampleRate_*60) / bpm_;
		assert(samplesPerBeat_ > 0);
		}

		void PlayerTimeInfo::recalcSPR( )
		{
			samplesPerRow_ = (sampleRate_*60)/(bpm_ * lpb_);
		assert(samplesPerRow_ > 0);
		}

	} // end of hostnamespace
} // end of psycle namespace
	

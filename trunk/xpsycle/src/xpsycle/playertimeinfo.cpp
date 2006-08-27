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
#include "playertimeinfo.h"

/// class for play time informations

namespace psycle
{
	namespace host
	{


		PlayerTimeInfo::PlayerTimeInfo( )
			: playBeatPos_(0.0),
				samplePos_(0),
				lpb_(4)
		{
		}

		PlayerTimeInfo::~ PlayerTimeInfo( )		
		{
		}

		void PlayerTimeInfo::setPlayBeatPos( double pos )
		{
			playBeatPos_ = pos;
		}

		double PlayerTimeInfo::playBeatPos( ) const
		{
			return playBeatPos_;
		}

		void PlayerTimeInfo::setSamplePos( int pos )
		{
			samplePos_ = pos;
		}

		int PlayerTimeInfo::samplePos( ) const
		{
			return samplePos_;
		}

		void PlayerTimeInfo::setLinesPerBeat( int lines )
		{
			lpb_ = lines;
		}

		int PlayerTimeInfo::linesPerBeat( ) const
		{
			return lpb_;
		}


	} // end of hostnamespace
}

 // end of psycle namespace
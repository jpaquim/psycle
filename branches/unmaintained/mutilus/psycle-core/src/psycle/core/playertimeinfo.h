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
#ifndef PLAYERTIMEINFO_H
#define PLAYERTIMEINFO_H

/**
@author  Psycledelics  
*/

/// class for play time informations

namespace psy
{
	namespace core
	{

		class PlayerTimeInfo {
		public:
				PlayerTimeInfo();
				~PlayerTimeInfo();

				/// the sequence position currently being played in beats
				void setPlayBeatPos( double pos );
				double playBeatPos() const;

				/// the current master sample position
				void setSamplePos( int pos );
				int samplePos() const;

				/// for old psycle machines 
				void setLinesPerBeat( int lines );
				int linesPerBeat() const;

				/// the current beats per minute at which to play the song.
				/// can be changed from the song itself using commands.
				void setBpm( double bpm );
				double bpm() const;

				void setSampleRate( int rate );
				int sampleRate( ) const;

				float samplesPerBeat() const;
				float samplesPerRow() const;

				
		private:

				double playBeatPos_;
				int samplePos_;
				int lpb_;
				double bpm_;
				int sampleRate_;
				float samplesPerBeat_;
				float samplesPerRow_;

				void recalcSPB();
				void recalcSPR();

		};

	} // end of host namespace
} // end of psycle namespace

#endif

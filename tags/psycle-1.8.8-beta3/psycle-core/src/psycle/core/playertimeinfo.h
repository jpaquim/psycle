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
#ifndef PSYCLE__CORE__PLAYER_TIME_INFO__INCLUDED
#define PSYCLE__CORE__PLAYER_TIME_INFO__INCLUDED
#pragma once

namespace psy { namespace core {

/// class for play time informations
///\author Psycledelics
class PlayerTimeInfo {
	public:
			PlayerTimeInfo();
			~PlayerTimeInfo();

			/// the sequence position currently being played in beats
			void setPlayBeatPos( double pos );
			double playBeatPos() const { return playBeatPos_; }

			/// the current master sample position
			void setSamplePos( int pos );
			int samplePos() const { return samplePos_; }

			/// for legacy sincronization and duration of some commands.
			void setTicksSpeed( int ticks , bool isticks);
			int ticksSpeed() const { return ticks_; }
			bool isTicks() const { return isTicks_; }

			/// the current beats per minute at which to play the song.
			/// can be changed from the song itself using commands.
			void setBpm( double bpm );
			double bpm() const { return bpm_; }

			void setSampleRate( int rate );
			int sampleRate( ) const { return sampleRate_; }

			float samplesPerBeat() const { return samplesPerBeat_; }
			float samplesPerTick() const { return samplesPerTick_; }
			
	private:
			double playBeatPos_;
			int samplePos_;
			int ticks_;
			bool isTicks_;
			double bpm_;
			int sampleRate_;
			float samplesPerBeat_;
			float samplesPerTick_;

			void recalcSPB();
			void recalcSPT();
};

}}

#endif

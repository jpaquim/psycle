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
#ifndef PSYCLE__CORE__PATTERN_LINE_INCLUDED
#define PSYCLE__CORE__PATTERN_LINE_INCLUDED
#pragma once

#include "patternevent.h"

#include <map>
#include <string>

namespace psy { namespace core {

/**
@author  Psycledelics  
*/
class PatternLine {
	public:
		
		typedef std::map<std::uint8_t, NoteEvent>::iterator noteiterator;
		typedef std::map<std::uint8_t, NoteEvent>::const_iterator notec_iterator;
		typedef std::map<std::uint8_t, TweakEvent>::iterator tweakiterator;
		typedef std::map<std::uint8_t, TweakEvent>::const_iterator tweakc_iterator;

		PatternLine() {;}

		void setNoteEvent(std::uint8_t,NoteEvent& pevent);
		void replaceNoteEvent(std::uint8_t,NoteEvent& pevent);
		void remNoteEvent(std::uint8_t);
		NoteEvent & noteEvent(std::uint8_t index);
		const NoteEvent & noteEvent(std::uint8_t index) const;
//		std::uint8_t noteEvents() { return noteMap.count(); }

		void setTweak(std::uint8_t, TweakEvent& pevent);
		void replacetweak(std::uint8_t,TweakEvent& pevent);
		void remTweak(std::uint8_t);
		TweakEvent & tweak(std::uint8_t index);
		const TweakEvent & tweak(std::uint8_t index) const;
//		std::size_t tweaks() { return tweakMap.count(); } const;

		///\todo why is this virtual?
		/// answer: because we once wanted to make subclasses of patternlines. specifically for tweaks and globals.
		/// Not sure if this is needed now.
		virtual bool empty() const { return noteMap.empty() && tweakMap.empty(); }

		std::string toXml( float pos ) const;

	private:
		static NoteEvent emptyevent;
		static TweakEvent emptytweak;

		/// A line can contain two types of information, notes/commands, and tweaks. (tweaks will be also wire volumes)
		/// pattern holds a tweakInfoMap and trackInfoMap maps that contain information about the track, and the tweaks
		/// of the pattern.
		/// noteMap and tweakMap store the actual data (noteMap holds events, and tweakMap holds the value of the tweak).
		/// the key is used to indicate the track at which is located (for tweakMap, it does not correspond to the physical track)
		std::map<std::uint8_t, NoteEvent> noteMap;
		std::map<std::uint8_t, TweakEvent> tweakMap;
};

}}
#endif

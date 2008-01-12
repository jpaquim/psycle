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
/*!
* \brief
* Pattern Line storage information class.
* 
* A PatternLine stores all the events (notes and tweaks) that happen at a specific time (line) inside a pattern.
* 
* \remarks
* noteMap and tweakMap store the actual data (noteMap holds events, and tweakMap holds the value of the tweak).
* the key is used to indicate the track at which is located (for tweakMap, it corresponds to the Pattern::tweakInfoMap index.)
* 
* \see
* Pattern.h
*/
class PatternLine {
	public:
		
		typedef std::map<std::uint8_t, NoteEvent>::iterator noteiterator;
		typedef std::map<std::uint8_t, NoteEvent>::const_iterator notec_iterator;
		typedef std::map<std::uint8_t, TweakEvent>::iterator tweakiterator;
		typedef std::map<std::uint8_t, TweakEvent>::const_iterator tweakc_iterator;

		PatternLine() {;}

		///\todo: These functions are not fully usable as they are. This is an area of study right now
		///{
		inline iterator upper_bound(const double& _Keyval) { return noteMap.upper_bound(_Keyval); }
		inline const_iterator upper_bound(const double& _Keyval) const { return noteMap.upper_bound(_Keyval); }
		inline iterator lower_bound(const double& _Keyval) { return noteMap.lower_bound(_Keyval); }
		inline const_iterator lower_bound(const double& _Keyval) const{ return noteMap.lower_bound(_Keyval); }
		inline iterator begin() { noteMap.begin(); }
		inline const_iterator begin() const { noteMap.begin(); }
		inline iterator end() { noteMap.end(); }
		inline const_iterator end() const { noteMap.end(); }
		inline reverse_iterator rbegin() { noteMap.rbegin(); }
		inline const_reverse_iterator rbegin() const { noteMap.rbegin(); }
		inline reverse_iterator rend() { noteMap.rend(); }
		inline const_reverse_iterator rend() const { noteMap.rend(); }
		///}

		void setNoteEvent(std::uint8_t,NoteEvent& pevent);
		void replaceNoteEvent(std::uint8_t,NoteEvent& pevent);
		void remNoteEvent(std::uint8_t);
		inline NoteEvent & noteEvent(std::uint8_t index);
		inline const NoteEvent & noteEvent(std::uint8_t index) const;
		///\todo: verify the necessity of this function as a public one
		inline bool isnotesempty() { return noteMap.empty(); }
//		inline std::uint8_t noteEvents() { return noteMap.count(); }

		void setTweak(std::uint8_t, TweakEvent& pevent);
		void replacetweak(std::uint8_t,TweakEvent& pevent);
		void remTweak(std::uint8_t);
		inline TweakEvent & tweak(std::uint8_t index);
		inline const TweakEvent & tweak(std::uint8_t index) const;
		///\todo: verify the necessity of this function as a public one
		inline bool istweaksempty() { return tweakMap.empty(); }
//		inline std::size_t tweaks() { return tweakMap.count(); } const;

		/**
		* checks if the structure is in an empty state
		* @return a boolean with the empty state value.
		*/
		inline bool empty() const { return noteMap.empty() && tweakMap.empty(); }

		/**
		* generates an xml output of the data, in order to save it.
		* @return an std::string containing the xml output.
		*/
		std::string toXml( float pos ) const;

	private:
		static NoteEvent emptyevent; ///< Empty event.
		static TweakEvent emptytweak;///< Empty tweak.

		std::map<std::uint8_t, NoteEvent> noteMap;///< Map with the notes in this line. The index corresponds to the track.
		std::map<std::uint8_t, TweakEvent> tweakMap;///< Map with the tweaks in this line. The index corresponds to the Pattern::tweakInfoMap index.
};

}}
#endif

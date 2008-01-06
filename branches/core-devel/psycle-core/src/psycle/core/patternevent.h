/*!
 * <File comment goes here!!>
 * 
 * Copyright (c) 2005 by <your name/ organization here>
 */
/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/
#ifndef PSYCLE__CORE__PATTERN_EVENT__INCLUDED
#define PSYCLE__CORE__PATTERN_EVENT__INCLUDED
#pragma once

#include <cstdint>
#include <vector>

namespace psy { namespace core {

/**
@author  Psycledelics  
*/
/*!
 * \brief
 * Note Event information class.
 * 
 * A NoteEvent contains the event information for notes sent to the machines.
 * 
 * \remarks
 * Note that the machine index is not set in the patternEvent. It is common for the same track and is stored by the pattern in a TrackInfo struct
 * 
 * \see
 * patternline.h|Pattern.h
 */
class NoteEvent
{
	public:
		typedef std::pair<std::uint8_t,std::uint8_t> PcmType;
		typedef std::map<std::uint8_t,PcmType> PcmListType;

		NoteEvent();

		///\todo: use notes from 128 to 254 for a "custom scale", and store the custom scale as song information.
		/// these notes will have to be sent in a special way (VST allow finetuning. For native it needs to be implemented)
		inline void setNote(std::uint8_t value) { note_ = value; }
		inline std::uint8_t note() const { return note_; }

		inline void setVolume(std::uint8_t volume) { volume_ = volume; }
		inline std::uint8_t volume() const { return volume_; }

		void setParamCmd(std::uint8_t index,PcmType parm);
		void remParamCmd(std::uint8_t index);
		const inline PcmType & paramCmd(std::uint8_t index) const;

		inline bool empty() const { return note_ == 255 && volume_ == 255 && paraCmdList_.empty(); }

		std::string toXml(int track) const;

	private:
		std::uint8_t note_;
		std::uint8_t volume_;
		PcmListType paraCmdList_;
		static PcmType emptyCmd_;
};

class TweakEvent
{
public:
	TweakEvent();

	inline void setValue(std::uint16_t value) { value_ = value; }
	inline std::uint8_t value() const { return value_; }

	inline bool empty() const { return value_ == 0; }

	std::string toXml(int track) const;

private:
	std::uint16_t value_;
};

}}
#endif

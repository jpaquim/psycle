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
 * Note that the machine index is not set in the NoteEvent. It is common for the same track and is stored by the pattern in a TrackInfo struct
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

		/**
		* setter for the note value
		* @param value an std::uint8_t containing the note value.
		*/
		inline void setNote(std::uint8_t value) { note_ = value; }
		/**
		* getter for the note value
		* @return an std::uint8_t with the note value
		*/
		inline std::uint8_t note() const { return note_; }

		/**
		* setter for the volume value
		* @param value an std::uint8_t containing the volume value.
		*/
		inline void setVolume(std::uint8_t volume) { volume_ = volume; }
		/**
		* getter for the note value
		* @return an std::uint8_t with the volume value
		*/
		inline std::uint8_t volume() const { return volume_; }

		/**
		* setter for a command. If the index doesn't exists, it adds it.
		* @param index an std::uint8_t containing the index position of the command
		* @param parm a PcmType with the command value.
		*/
		void setParamCmd(std::uint8_t index,PcmType parm);
		/**
		* deletes a command
		* @param index an std::uint8_t containing the index position to remove.
		*/
		void remParamCmd(std::uint8_t index);
		/**
		* getter for a command.
		* @param index an std::uint8_t containing the index position to get.
		* @return a PcmType containing the command, or emptyCmd_.
		*/
		const inline PcmType & paramCmd(std::uint8_t index) const;

		/**
		* checks if the structure is in an empty state
		* @return a boolean with the empty state value.
		*/
		inline bool empty() const { return note_ == 255 && volume_ == 255 && paraCmdList_.empty(); }

		/**
		* generates an xml output of the data, in order to save it.
		* @return an std::string containing the xml output.
		*/
		std::string toXml(int track) const;

	private:
		std::uint8_t note_;///< value of the note. 0 to 119 is c-0 to b-9. 120 is off, from 128 to 254 are custom-scale note values.
		std::uint8_t volume_;///< value for volume. Range undefined for now. Idea: use 0.1db steps. Else, the usual 00..FF
		PcmListType paraCmdList_;///< List (in fact, a map) of extra command columns, with its values.
		static PcmType emptyCmd_;///< Empty command.
};

/*!
* \brief
* Tweak Event information class.
* 
* A TweakEvent contains the value of an even of tweak type.
* 
* \remarks
* Note that the machine index is not set in the TweakEvent. It is common for the same track and is stored by the pattern in a TrackInfo struct
* Also, the parameter value is stored in a TweakTrackInfo struct, stored by the pattern.
* The types of Tweaks are defined in the TweakTrackInfo class.
* 
* \see
* patternline.h|Pattern.h
*/
class TweakEvent
{
public:
	TweakEvent();

	/**
	* setter for the value of the tweak
	* @param value an std::uint16_t containing the tweak value.
	*/
	inline void setValue(std::uint16_t value) { value_ = value; }
	/**
	* getter for the tweak value
	* @return an std::uint16_t with the tweak value
	*/
	inline std::uint8_t value() const { return value_; }

	/**
	* checks if the structure is in an empty state
	* @return a boolean with the empty state value.
	*/
	inline bool empty() const { return value_ == 0; }

	/**
	* generates an xml output of the data, in order to save it.
	* @return an std::string containing the xml output.
	*/
	std::string toXml(int track) const;

private:
	std::uint16_t value_;///< value at which to set the tweak.
};

}}
#endif

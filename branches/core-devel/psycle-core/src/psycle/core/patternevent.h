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
#include "commands.h"

namespace psy { namespace core {

class CommandEvent
{
public:
	CommandEvent();
	CommandEvent(std::uint8_t cmd,std::uint8_t param):command_(cmd),param_(param){}
	/**
	* setter for a command.
	@param value an std::uint8_t containing the parameter.
	*/
	inline void setParam(std::uint8_t param) { param_ = param; }
	/**
	* getter for a command.
	* @return the value of the parameterm
	*/
	inline std::uint8_t param() const { return param_; }

	/**
	* setter for a command.
	@param value an std::uint8_t containing the command.
	*/
	inline void setCmd(std::uint8_t command) { command_ = command; }
	/**
	* getter for a command.
	* @return the value of the command.
	*/
	inline std::uint8_t command() const { return command_; }

	inline bool empty() const {	return command_ == 0 && param_ == 0; }

	/**
	* generates an xml output of the data, in order to save it.
	* @return an std::string containing the xml output.
	*/
	std::string toXml(int track) const;

private:
	std::uint8_t command_;///< value of the command. There should be a set of them defined globally.
	std::uint8_t param_;///< value for the command.

};

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
 * TrackInfo|TrackEvent|Pattern
 */
class NoteEvent
{
	public:
		NoteEvent():note_(notetype::empty),volume_(255);
		NoteEvent(std::uint8_t note,std::uint8_t vol)
		:note_(note),volume_(vol){}

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
		* checks if the structure is in an empty state
		* @return a boolean with the empty state value.
		*/
		inline bool empty() const {	return note_ == notetypes::empty && volume_ == 255;	}

		/**
		* generates an xml output of the data, in order to save it.
		* @return an std::string containing the xml output.
		*/
		std::string toXml(int track) const;

	private:
		std::uint8_t note_;///< value of the note. 0 to 119 is c-0 to b-9. 120 is off, from 128 to 254 are custom-scale note values.
		std::uint8_t volume_;///< value for volume. Range 0 = 0, 9F = full, A0 to AF volume slide up, B0 to BF slide down, C0 to CF fine vol slide up, D0 to DF fine vol slide down, E0 to EF delay.
};

/*!
* \brief
* Tweak Event information class.
* 
* A TweakEvent contains the value of an event of tweak type.
* 
* \remarks
* Note that the machine index is not set in the TweakEvent. It is common for the same track and is stored by the pattern in a TrackInfo struct
* Also, the parameter value is stored in a TweakTrackInfo struct, stored by the pattern.
* The types of Tweaks are defined in the TweakTrackInfo class.
* 
* \see
* TweakTrackInfo|TrackInfo|TrackEvent|Pattern
*/
class TweakEvent
{
public:
	TweakEvent();
	TweakEvent(std::uint16_t val):value_(val)set_(true){}

	/**
	* setter for the value of the tweak
	* @param value an std::uint16_t containing the tweak value.
	*/
	inline void setValue(std::uint16_t value) { value_ = value; set_(true);}
	/**
	* getter for the tweak value
	* @return an std::uint16_t with the tweak value
	*/
	inline std::uint8_t value() const { return value_; }

	/**
	* checks if the structure is in an empty state
	* @return a boolean with the empty state value.
	*/
	inline bool empty() const { return !set; }

	/**
	* generates an xml output of the data, in order to save it.
	* @return an std::string containing the xml output.
	*/
	std::string toXml(int track) const;

private:
	std::uint16_t value_;///< value at which to set the tweak.
	bool set_;
};

class TrackEvent
{
public:
	TrackEvent();
	
	inline void SetNote(std::uint8_t index, NoteEvent &note)
	{
		if (index >= notes.size()) AddNote(index,note);
		else notes[index]=note;
	}
	inline NoteEvent& note(std::uint8_t index)
	{
		if (index >= notes.size()) return emptyevent;
		else return notes[index];
	}
	void RemoveNote(std::uint8_t index);

	inline void SetCommand(std::uint8_t index, CommandEvent &command)
	{
		if (index >= commands.size()) AddCommand(index,command);
		else commands[index]=command;
	}
	inline CommandEvent& command(std::uint8_t index)
	{
		if (index >= commands.size()) return emptycommand;
		else return commands[index];
	}
	void RemoveCommand(std::uint8_t index);

	inline void SetTweak(std::uint8_t index, TweakEvent &tweak)
	{
		if (index >= tweaks.size()) AddTweak(index,tweak);
		else tweaks[index]=tweak;
	}
	inline TweakEvent& tweak(std::uint8_t index)
	{
		if (index >= tweaks.size()) return emptytweak;
		else return tweaks[index];
	}
	void RemoveTweak(std::uint8_t index);

	/**
	* generates an xml output of the data, in order to save it.
	* @return an std::string containing the xml output.
	*/
	std::string toXml(int track) const;


private:
	static NoteEvent emptyevent; ///< Empty event.
	static CommandEvent emptycommand;///< Empty command.
	static TweakEvent emptytweak;///< Empty tweak.

	void AddNote(std::uint8_t index,NoteEvent &note);
	void AddCommand(std::uint8_t index,CommandEvent& cmd);
	void AddTweak(std::uint8_t index,TweakEvent& tweak);
	std::vector<NoteEvent> notes;
	std::vector<CommandEvent> commands;
	std::vector<TweakEvent> tweaks;

	// Pointer to TrackInfo ??? There's no way to know which one it is right now.

};
}}
#endif

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

class TrackInfo;

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
 * TrackInfo|PatternEvent|Pattern
 */
class NoteEvent
{
public:
	NoteEvent():note_(notetypes::empty),volume_(255) {}
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
	std::uint8_t volume_;///< value for volume. Range 0 = 0, 9F = full, A0 to AF volume slide up, B0 to BF slide down, C0 to CF fine vol slide up, D0 to DF fine vol slide down, E0 to EF delay. FF = empty
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
* TweakTrackInfo|TrackInfo|PatternEvent|Pattern
*/
class TweakEvent
{
public:
	TweakEvent();
	TweakEvent(std::uint16_t val):value_(val),set_(true){}

	/**
	* setter for the value of the tweak
	* @param value an std::uint16_t containing the tweak value.
	*/
	inline void setValue(std::uint16_t value) { value_ = value; set_ = true;}
	/**
	* getter for the tweak value
	* @return an std::uint16_t with the tweak value
	*/
	inline std::uint8_t value() const { return value_; }

	/**
	* checks if the structure is in an empty state
	* @return a boolean with the empty state value.
	*/
	inline bool empty() const { return !set_; }

	/**
	* generates an xml output of the data, in order to save it.
	* @return an std::string containing the xml output.
	*/
	std::string toXml(int track) const;

private:
	std::uint16_t value_;///< value at which to set the tweak.
	bool set_;
};

class ClassicTweakEvent
{
public:
	ClassicTweakEvent(){};
	ClassicTweakEvent(NoteEvent &note,CommandEvent &command)
		:tweaktype_(note.note())
		,param_(note.volume())
		,value_((command.command()<<8)|command.param()){}
	ClassicTweakEvent(std::uint8_t tweaktype, std::uint8_t param,std::uint16_t value)
		:tweaktype_(tweaktype),param_(param),value_(value){}
	
	inline void setType(std::uint8_t type) { tweaktype_ = type; }
	inline std::uint8_t type() const { return tweaktype_; }

	inline void setParameter(std::uint8_t param) { param_ = param; }
	inline std::uint8_t parameter() const { return param_; }

	inline void setValue(std::uint16_t value) { value_ = value; }
	inline std::uint16_t value() const { return value_; }

	inline NoteEvent getAsNote() const { NoteEvent thisevent(tweaktype_,param_); return thisevent; }
	inline CommandEvent getAsCommand() const { CommandEvent thisevent((value_&0xFF00)>>8,value_&0x00FF); return thisevent; }

private:
	std::uint8_t tweaktype_;
	std::uint8_t param_;
	std::uint16_t value_;
};

class ClassicEvent
{
public:
	ClassicEvent():instr_(instrumenttypes::empty) {};
	ClassicEvent(NoteEvent &note, std::uint16_t instr=instrumenttypes::empty):instr_(instr) { addNote(0,note); }
	ClassicEvent(CommandEvent &command, std::uint16_t instr=instrumenttypes::empty):instr_(instr) { addCommand(0,command); }
	ClassicEvent(NoteEvent &note,CommandEvent &command, std::uint16_t instr=instrumenttypes::empty):instr_(instr) { addNote(0,note); addCommand(0,command); }
	ClassicEvent(ClassicTweakEvent &tweak, std::uint16_t instr=instrumenttypes::empty):instr_(instr) { addNote(0,tweak.getAsNote()); addCommand(0,tweak.getAsCommand()); }

	inline void setInstrument(std::uint16_t instr) { instr_ = instr; }
	inline std::uint16_t instrument() { return instr_; }

	inline void setNote(std::uint8_t index, NoteEvent &note)
	{
		if (index >= notes.size()) addNote(index,note);
		else notes[index]=note;
	}
	inline const NoteEvent& note(std::uint8_t index) const
	{
		if (index >= notes.size()) return emptyevent;
		else return notes[index];
	}
	void removeNote(std::uint8_t index);

	inline void setCommand(std::uint8_t index, CommandEvent &command)
	{
		if (index >= commands.size()) addCommand(index,command);
		else commands[index]=command;
	}
	inline const CommandEvent& command(std::uint8_t index) const
	{
		if (index >= commands.size()) return emptycommand;
		else return commands[index];
	}
	void removeCommand(std::uint8_t index);

	void setTweak(std::uint8_t index, ClassicTweakEvent &tweak);
	const ClassicTweakEvent tweak(std::uint8_t index) const;

	inline bool empty() const { return notes.empty() && commands.empty() && instr_ == instrumenttypes::empty; }

	/**
	* generates an xml output of the data, in order to save it.
	* @return an std::string containing the xml output.
	*/
	///\todo: check if "track" is needed (depends on how we save ClassicPattern) 
	std::string toXml(double position,int track) const;


private:
	void addNote(std::uint8_t index,NoteEvent note);
	void addCommand(std::uint8_t index,CommandEvent cmd);

	std::vector<NoteEvent> notes;
	std::vector<CommandEvent> commands;
	std::uint16_t instr_;

	static NoteEvent emptyevent; ///< Empty event.
	static CommandEvent emptycommand;///< Empty command.
};



class PatternEvent
{
public:
	PatternEvent():track_(0){};
	PatternEvent(TrackInfo* track){ track_ = track; }
	PatternEvent(NoteEvent &note, TrackInfo* track=0):track_(track) { AddNote(0,note); }
	PatternEvent(CommandEvent &command, TrackInfo* track=0):track_(track) { AddCommand(0,command); }
	PatternEvent(NoteEvent &note,CommandEvent &command, TrackInfo* track=0):track_(track) { AddNote(0,note); AddCommand(0,command); }
	PatternEvent(TweakEvent &tweak, TrackInfo* track=0):track_(track) { AddTweak(0,tweak); }

	inline void SetTrack(TrackInfo* track) { track_ = track; }
	inline TrackInfo* track() const { return track_; }

	inline void SetNote(std::uint8_t index, NoteEvent &note)
	{
		if (index >= notes.size()) AddNote(index,note);
		else notes[index]=note;
	}
	inline const NoteEvent& note(std::uint8_t index) const
	{
		if (index >= notes.size()) return emptyevent;
		else return notes[index];
	}
//	inline const NoteEvent& note(std::uint8_t index) const { return note(index); }
	void RemoveNote(std::uint8_t index);

	inline void SetCommand(std::uint8_t index, CommandEvent &command)
	{
		if (index >= commands.size()) AddCommand(index,command);
		else commands[index]=command;
	}
	inline const CommandEvent& command(std::uint8_t index) const
	{
		if (index >= commands.size()) return emptycommand;
		else return commands[index];
	}
//	inline const CommandEvent& command(std::uint8_t index) const { return command(index); }
	void RemoveCommand(std::uint8_t index);

	inline void SetTweak(std::uint8_t index, TweakEvent &tweak)
	{
		if (index >= tweaks.size()) AddTweak(index,tweak);
		else tweaks[index]=tweak;
	}
	inline const TweakEvent& tweak(std::uint8_t index) const
	{
		if (index >= tweaks.size()) return emptytweak;
		else return tweaks[index];
	}
//	inline const TweakEvent& tweak(std::uint8_t index) const { return tweak(index); }
	void RemoveTweak(std::uint8_t index);

	inline bool empty() const { return notes.empty() && commands.empty() && tweaks.empty(); }

	/**
	* generates an xml output of the data, in order to save it.
	* @return an std::string containing the xml output.
	*/
	std::string toXml(double position, int track) const;


private:
	void AddNote(std::uint8_t index,NoteEvent &note);
	void AddCommand(std::uint8_t index,CommandEvent& cmd);
	void AddTweak(std::uint8_t index,TweakEvent& tweak);

	std::vector<NoteEvent> notes;
	std::vector<CommandEvent> commands;
	std::vector<TweakEvent> tweaks;
	TrackInfo* track_;

	static NoteEvent emptyevent; ///< Empty event.
	static CommandEvent emptycommand;///< Empty command.
	static TweakEvent emptytweak;///< Empty tweak.

};

}}
#endif

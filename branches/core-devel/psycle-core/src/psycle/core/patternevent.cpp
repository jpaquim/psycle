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
#include <psycle/core/psycleCorePch.hpp>
#include "patternevent.h"
#include <sstream>

namespace psy { namespace core {
	
NoteEvent PatternEvent::emptyevent;
CommandEvent PatternEvent::emptycommand;
TweakEvent PatternEvent::emptytweak;

std::string CommandEvent::toXml( int index ) const
{
	std::ostringstream xml;
	xml << "<CommandEvent index='" << index;
	xml << std::hex << "' command='" << (int) command_
		<< std::hex << "' param='" << (int) param_;
	xml <<"' />\n";

}

std::string NoteEvent::toXml( int index ) const
{
	std::ostringstream xml;
	xml << "<NoteEvent index='" << index;
		if (note_ != notetypes::empty) xml << std::hex << "' note='" << (int) note_;
		if (volume_ != 255) xml << std::hex << "' volume='" << (int) volume_;
	return xml.str();
}


/************************************************************************/
// TweakEvent
std::string TweakEvent::toXml( int index ) const
{
	std::ostringstream xml;
	xml << "<TweakEvent index='" << index
		<< std::hex << "' value='" << (int) value_
		<<"' />\n";
	return xml.str();
}


/************************************************************************/
// ClassicEvent
void ClassicEvent::setTweak(std::uint8_t index, ClassicTweakEvent &tweak)
{
	if (index >= notes.size()) 
	{
		AddNote(index,tweak.getAsNote());
		setCommand(0,tweak.getAsCommand());
	}
	else
	{
		notes[index]=tweak.getAsNote();
		setCommand(0,tweak.getAsCommand());
	}
}
const ClassicTweakEvent ClassicEvent::tweak(std::uint8_t index) const
{
	if (index >= notes.size() || commands.empty())
	{
		ClassicTweakEvent thisevent;
		return thisevent;
	}
	else 
	{
		ClassicTweakEvent(notes[index],commands[0]) thisevent;
		return thisevent;
	}
}

void ClassicEvent::removeNote(std::uint8_t index)
{
	if (index >= notes.size()) return;

	int size = notes.size()-1;
	for (int i=index; i<size; i++)
	{
		notes[index]=notes[index+1];
	}
	notes.pop_back();
}
void ClassicEvent::removeCommand(std::uint8_t index)
{
	if (index >= commands.size()) return;

	int size = commands.size()-1;
	for (int i=index; i<size; i++)
	{
		commands[index]=commands[index+1];
	}
	commands.pop_back();
}

void ClassicEvent::addNote(std::uint8_t index,NoteEvent &note)
{
	int size = notes.size()+1;
	while (size < index) { notes.push_back(NoteEvent()); }
	notes[index]=note;
}
void ClassicEvent::addCommand(std::uint8_t index,CommandEvent &cmd)
{
	int size = commands.size()+1;
	while (size < index) { commands.push_back(CommandEvent()); }
	commands[index]=cmd;
}

std::string ClassicEvent::toXml( double position, int track ) const
{
	std::ostringstream xml;
	xml << "<ClassicEvent position='" << position << "' track='" << track << "' instrument='" << instr_;
	xml << "'>\n";
	for(int i=0;i<notes.size();i++) if (!notes[i].empty() ) xml << notes[i].toXml(i);
	for(int i=0;i<commands.size();i++) if (!commands[i].empty() )x ml << commands[i].toXml(i);
	xml <<"</ClassicEvent>\n";
	return xml.str();
}
/************************************************************************/
// PatternEvent
void PatternEvent::removeNote(std::uint8_t index)
{
	if (index >= notes.size()) return;
	
	int size = notes.size()-1;
	for (int i=index; i<size; i++)
	{
		notes[index]=notes[index+1];
	}
	notes.pop_back();
}
void PatternEvent::removeCommand(std::uint8_t index)
{
	if (index >= commands.size()) return;

	int size = commands.size()-1;
	for (int i=index; i<size; i++)
	{
		commands[index]=commands[index+1];
	}
	commands.pop_back();
}
void PatternEvent::removeTweak(std::uint8_t index)
{
	if (index >= tweaks.size()) return;

	int size = tweaks.size()-1;
	for (int i=index; i<size; i++)
	{
		tweaks[index]=tweaks[index+1];
	}
	tweaks.pop_back();
}


void PatternEvent::addNote(std::uint8_t index,NoteEvent &note)
{
	int size = notes.size()+1;
	while (size < index) { notes.push_back(NoteEvent()); }
	notes[index]=note;
}
void PatternEvent::addCommand(std::uint8_t index,CommandEvent &cmd)
{
	int size = commands.size()+1;
	while (size < index) { commands.push_back(CommandEvent()); }
	commands[index]=cmd;
}
void PatternEvent::addTweak(std::uint8_t index,TweakEvent &tweak)
{
	int size = tweaks.size()+1;
	while (size < index) { tweaks.push_back(TweakEvent()); }
	tweaks[index]=tweak;
}

std::string PatternEvent::toXml( double position, int track ) const
{
	std::ostringstream xml;
	xml << "<PatternEvent position='" << position << "' track='" << track;
	xml << "'>\n";
	for(int i=0;i<notes.size();i++) if (!notes[i].empty() ) xml << notes[i].toXml(i);
	for(int i=0;i<commands.size();i++) if (!commands[i].empty() )x ml << commands[i].toXml(i);
	for(int i=0;i<tweaks.size();i++) if (!tweaks[i].empty() ) xml << tweaks[i].toXml(i);
	xml <<"</PatternEvent>\n";
	return xml.str();
}

}}

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
#include "patternevent.h"

PatternEvent::PatternEvent() :
  _note(255),
  _inst(255),
  _mach(0),
  _cmd(0),
  _parameter(0)
{

}


PatternEvent::~PatternEvent()
{
}

void PatternEvent::setNote( std::uint8_t value )
{
  _note = value;
}

std::uint8_t PatternEvent::note( ) const
{
  return _note;
}

void PatternEvent::setInstrument( std::uint8_t instrument )
{
  _inst = instrument;
}

std::uint8_t PatternEvent::instrument( ) const
{
  return _inst;
}

void PatternEvent::setMachine( std::uint8_t machine )
{
  _mach = machine;
}

std::uint8_t PatternEvent::machine( ) const
{
  return _mach;
}

void PatternEvent::setCommand( std::uint8_t command )
{
  _cmd = command;
}

std::uint8_t PatternEvent::command( ) const
{
  return _cmd;
}

void PatternEvent::setParameter( std::uint8_t parameter )
{
  _parameter = parameter;
}

std::uint8_t PatternEvent::parameter( ) const
{
  return _parameter;
}

psycle::host::PatternEntry * PatternEvent::entry( )
{
  return (psycle::host::PatternEntry*) this;
}

psycle::host::PatternEntry * PatternEvent::entry( ) const
{
  return (psycle::host::PatternEntry*) this;
}



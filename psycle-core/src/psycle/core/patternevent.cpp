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
#include "psycleCorePch.hpp"

#include "patternevent.h"

namespace psy
{
	namespace core
	{
		
		
		PatternEvent::PatternEvent() :
			_note(255),
			_inst(255),
			_mach(255),
			_cmd(0),
			_parameter(0),
			_volume(255),
			_sharp(1)
		{
			for (int i = 0; i < 10; i++) 
			paraCmdList_.push_back( PcmType() );
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

		void PatternEvent::setVolume(std::uint8_t vol) {
			_volume = vol;
		}

		std::uint8_t PatternEvent::volume() const {
			return _volume;
		}

		void PatternEvent::setSharp( bool on ) {
			_sharp = on;
		}

		bool PatternEvent::isSharp() const {
			return _sharp;
		}

		PatternEvent::PcmListType & PatternEvent::paraCmdList() {
			return paraCmdList_;
		}

		std::string PatternEvent::toXml( int track ) const
		{
			std::ostringstream xml;
			xml << "<patevent track='" << track << std::hex << "' note='" << (int)_note << std::hex << +"' mac='" << (int)_mach << std::hex
					<< "' inst='" << (int)_inst << std::hex << "' cmd='" << (int)_cmd << std::hex
					<< "' param='" << (int)_parameter << std::hex << "' sharp='" << (int) _sharp <<"' />" << std::endl;

			return xml.str();
		}

		bool PatternEvent::empty( ) const
		{
			return (_note == 255 && _inst == 255 && _mach == 255 && _cmd == 0 && _parameter == 0);
		}

	} // end of host namespace
}//end of psycle namespace

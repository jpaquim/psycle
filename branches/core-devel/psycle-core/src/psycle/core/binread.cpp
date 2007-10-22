/***************************************************************************
*   Copyright (C) 2007 Psycledelics,                                      *
*   psycle.sf.net                                                         *
*                                                                         *
*   based on binread from sondar                                          *
*   Copyright (C) 2007 by Stefan Nattkemper                               *
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

#include "binread.h"

namespace psy {
	namespace core {

		BinRead::BinRead( std::istream & in ) 
			: in_( in  ) 
		{
			//platform_ = testPlatform();
		}

		BinRead::~BinRead() {
		}

		std::uint16_t BinRead::readUInt2LE() {
			unsigned char buf[2];
			in_.read( reinterpret_cast<char*>(&buf), 2 );
      return buf[0] | (buf[1] << 8);
		}

		std::uint16_t BinRead::readUInt2BE() {
			unsigned char buf[2];
			in_.read( reinterpret_cast<char*>(&buf), 2 );
      return buf[1] | (buf[0] << 8);
		}

    std::uint32_t BinRead::readUInt4BE() {
			unsigned char buf[4];
			in_.read( reinterpret_cast<char*>(&buf), 4 );
      return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3] ;
		}
		
    std::uint32_t BinRead::readUInt4LE() {
			unsigned char buf[4];
			in_.read( reinterpret_cast<char*>(&buf), 4 );
      return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24) ; 
		}
		
    std::int16_t BinRead::readInt2LE() {
			return static_cast<std::int16_t>( readUInt2LE() );
		}
		
    std::int16_t BinRead::readInt2BE() {
			return static_cast<std::int16_t>( readUInt2BE() );
		}

    std::int32_t BinRead::readInt4LE() {
			return static_cast<int32_t>( readUInt4LE() );
		}
    std::int32_t BinRead::readInt4BE() {
			return static_cast<int32_t>( readUInt4BE() );
		}

		void BinRead::readUIntArray4LE( std::uint32_t data[], int count ) {
			for ( int i = 0; i < count; ++i ) {
				data[i] = readUInt4LE();
			}
		}

    void BinRead::readIntArray4LE( std::int32_t data[], int count ) {
			for ( int i = 0; i < count; ++i ) {
				data[i] = readInt4LE();
			}
		}

		void BinRead::read( char * data, std::streamsize const & bytes ) {
			in_.read(reinterpret_cast<char*>(data) ,bytes);
		}

    /*
		BinRead::BinPlatform BinRead::testPlatform() {
			BinPlatform order;
			unsigned long u = 0x01;
			if ( sizeof(long) == 4 && sizeof(int) == 4 ) {    
				char const* p = reinterpret_cast< char const* >( &u ) ;
				if ( p[0] == '1' ) 
					order = BinRead::byte4BE;
				else
					order = BinRead::byte4LE;
			} else if ( sizeof(long) == 8 && sizeof(int) == 4 )  {
				char const* p = reinterpret_cast< char const* >( &u ) ;
				if ( p[0] == '1' ) 
					order = BinRead::byte8BE;
				else
					order = BinRead::byte8LE;
			}
			else assert(0); // Platform not supported.
			return order;
		}

		unsigned int BinRead::swap4( unsigned int value )
		{
			return \
				((value >> 24) & 0x000000ff) |
				((value >>  8) & 0x0000ff00) |
				((value <<  8) & 0x00ff0000) |
				((value << 24) & 0xff000000);
		}
    */

		bool BinRead::eof() const {
			return in_.eof();
		}

		bool BinRead::bad() const {
			return in_.bad();
		}
    /*
		BinRead::BinPlatform BinRead::platform() const {
			return platform_;
		}
    */

	}
}

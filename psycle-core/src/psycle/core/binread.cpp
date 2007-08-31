/***************************************************************************
*   Copyright (C) 2007 Psycledelics    *
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
#include <psycle/core/psycleCorePch.hpp>

#include "binread.h"

namespace psy {
	namespace core {

		BinRead::BinRead( std::istream & in ) 
			: in_( in  ) 
		{
			platform_ = testPlatform();
		}

		BinRead::~BinRead() {
		}

		unsigned int BinRead::readUInt4LE() {
			unsigned char buf[4];
			in_.read( reinterpret_cast<char*>(&buf), 4 );      
			switch ( platform_ ) {
				case byte4LE : return buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24; 
				break;
				case byte4BE : return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3] << 0 ; 
				break;
				case byte8LE : return buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24 | buf[4] << 32 | buf[5] << 40 | buf[6] << 48 | buf[7] << 56;
				break;
				case byte8BE : return buf[0] << 56 | buf[1] << 48 | buf[2] << 40 | buf[3] << 32 | buf[4] << 24 | buf[5] << 16 | buf[6] << 8 | buf[7];
				break;
			}
			return 0; // cannot handle platform
		}

		int BinRead::readInt4LE() {
			return static_cast<int>( readUInt4LE() );
		}

		void BinRead::readUIntArray4LE( unsigned int data[], int count ) {
			for ( int i = 0; i < count; ++i ) {
				data[i] = readUInt4LE();
			}
		}

			void BinRead::readIntArray4LE( int data[], int count ) {
			for ( int i = 0; i < count; ++i ) {
				data[i] = static_cast<int>( readUInt4LE() );
			}
		}

		void BinRead::read( char * data, std::streamsize const & bytes ) {
			in_.read(reinterpret_cast<char*>(data) ,bytes);
		}

		BinRead::BinPlatform BinRead::testPlatform() {
			BinPlatform order;
			unsigned int u = 0x01;
			if ( sizeof(int) == 4 ) {    
				char const* p = reinterpret_cast< char const* >( &u ) ;
				if ( p[0] == '1' ) 
					order = BinRead::byte4BE;
				else
					order = BinRead::byte4LE;
			} else
				if ( sizeof(int) == 8 )  {
					char const* p = reinterpret_cast< char const* >( &u ) ;
					if ( p[0] == '1' ) 
						order = BinRead::byte8BE;
					else
						order = BinRead::byte8LE;
				}
				return order;
		}

		unsigned int BinRead::swap4( unsigned int value )
		{
			return  ((value  & 0xFF000000) >> 24) |
				(((value & 0x00FF0000) >> 16) << 8) |
				(((value & 0x0000FF00) >> 8)  << 16) |
				((value  & 0x000000FF) << 24);
		}

		bool BinRead::eof() const {
			return in_.eof();
		}

		bool BinRead::bad() const {
			return in_.bad();
		}

		BinRead::BinPlatform BinRead::platform() const {
			return platform_;
		}

	}
}

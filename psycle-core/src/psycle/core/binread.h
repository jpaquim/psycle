/***************************************************************************
*   Copyright (C) 2007 Psycledelics					  *
*   psycle.sf.net							  *
*
*   based on binread from sondar					  *
*   Copyright (C) 2007 Stefan Nattkemper				  *
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
#ifndef BINREAD_H
#define BINREAD_H

#include <istream>

namespace psy {
	namespace core {

		class BinRead {      
		public:

			enum BinPlatform { byte4LE, byte4BE, byte8LE, byte8BE };

			BinRead( std::istream & in );
			~BinRead();

			short readInt2LE();
			unsigned short readUInt2LE();
	
			short readInt2BE();
			unsigned short readUInt2BE();
		
			unsigned int readUInt4LE();
			unsigned int readUInt4BE();
			
			int readInt4LE();
			void readUIntArray4LE( unsigned int data[], int count );
			void readIntArray4LE( int data[], int size );

			void read( char * data, std::streamsize const & bytes );

			bool eof() const;
			bool bad() const;

			BinPlatform platform() const;

		private:

			BinPlatform platform_;
			std::istream & in_;

			BinPlatform testPlatform();
			unsigned int swap4( unsigned int value );

		};

	}
}

#endif

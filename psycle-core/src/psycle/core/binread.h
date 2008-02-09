/* -*- mode:c++, indent-tabs-mode:t -*- */
/***************************************************************************
*   Copyright (C) 2007 Psycledelics                                       *
*   psycle.sf.net                                                         *
*                                                                         *
*   based on binread from sondar                                          *
*   Copyright (C) 2007 Stefan Nattkemper                                  *
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
#include <cstdint>

namespace psy { namespace core {

class BinRead {
	public:

		enum BinPlatform { byte4LE, byte4BE, byte8LE, byte8BE };

		BinRead( std::istream & in );
		~BinRead();

		std::int16_t readInt2LE();
		std::uint16_t readUInt2LE();
	
		std::int16_t readInt2BE();
		std::uint16_t readUInt2BE();
		
		std::int32_t readInt4LE(); 
		std::uint32_t readUInt4LE();

		std::int32_t readInt4BE();
		std::uint32_t readUInt4BE();
			
		void readUIntArray4LE( std::uint32_t data[], int count );
		void readIntArray4LE( std::int32_t data[], int count );

		void read( char * data, std::streamsize const & bytes );

		bool eof() const;
		bool bad() const;

		//BinPlatform platform() const;

	private:

		std::istream & in_;

		#if 0
			BinPlatform platform_;
			BinPlatform testPlatform();
			std::uint32_t swap4( std::uint32_t value );
		#endif
};

}}

#endif

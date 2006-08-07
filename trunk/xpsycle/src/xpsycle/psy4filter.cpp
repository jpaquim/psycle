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
#include "psy4filter.h"
#include "fileio.h"
#include <ngrs/nxmlparser.h>

namespace psycle {
	namespace host {


		Psy4Filter::Psy4Filter()
			: PsyFilter()
		{
		}


		Psy4Filter::~Psy4Filter()
		{
		}

		int psycle::host::Psy4Filter::version( ) const
		{
			return 4;
		}

		bool Psy4Filter::testFormat( const std::string & fileName )
		{
			NXmlParser parser;
			isPsy4 = false;
			parser.tagParse.connect(this,&Psy4Filter::onDetectFilterTag);
			parser.parseFile(fileName);
			return isPsy4;
		}

		void Psy4Filter::onDetectFilterTag( const std::string & tagName )
		{
			if (tagName == "psy4") isPsy4 = true;
		}

		bool Psy4Filter::load( const std::string & fileName, Song & song )
		{
			std::cout << "psy4filter detected for load" << std::endl;
			NXmlParser parser;
			parser.tagParse.connect(this,&Psy4Filter::onTagParse);
			parser.parseFile(fileName);
			return isPsy4;
			return false;
		}

		void Psy4Filter::onTagParse( const std::string & tagName )
		{
			if (tagName == "category") {

			}
		}

		bool Psy4Filter::save( const std::string & fileName, const Song & song )
		{
			_stream.open(fileName.c_str (), std::ios_base::out | std::ios_base::trunc |std::ios_base::binary);
			if (!_stream.is_open ()) return false;
			_stream.seekg (0, std::ios::beg);
			

			std::ostringstream xml;
			xml << "<psy4>" << std::endl;
			xml << "<info>" << std::endl;
			xml << "<name   text='" << song.name()    << "' />" << std::endl;
			xml << "<author text='" << song.author()  << "' />" << std::endl;;
			xml << "<coment text='" << song.comment() << "' />" << std::endl;;
			xml << "</info>" << std::endl;
			xml << "<global>" << std::endl;
			xml << "</global>" << std::endl;
			xml << song.patternSequence().patternData().toXml();
			xml << "<machines>" << std::endl;
			xml << "</machines>" << std::endl;
			xml << "<instruments>" << std::endl;
			xml << "</instruments>" << std::endl;
			xml << "</psy4>" << std::endl;

			_stream << xml.str() << std::endl;
			_stream.close();
		}

	}
}

/***************************************************************************
	*   Copyright (C) 2006 by  Psycledelics     *
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
#include "psyfilter.h"

#include "psy2filter.h"
#include "psy3filter.h"
#include "psy4filter.h"

namespace psy
{
	namespace core
	{

		std::vector<PsyFilter*> PsyFilter::filters;
		int PsyFilter::c = 0;


		PsyFilter::PsyFilter()
		{
			if (c==0) {
				c++;
				filters.push_back( Psy2Filter::Instance() );
				filters.push_back( Psy3Filter::Instance() );
				filters.push_back( Psy4Filter::Instance() );
			}
		}


		PsyFilter::~PsyFilter()
		{
		}

		bool PsyFilter::load( const std::string & fileName, Song & song )
		{
			return false;
		}

		bool PsyFilter::save( const std::string & fileName, const Song & song )
		{
			return false;
		}

		std::string  PsyFilter::filePostfix() const {
			return "psy";
		}

		bool PsyFilter::testFormat( const std::string & fileName )
		{
			return false;
		}

		int PsyFilter::version( ) const
		{
			return 0;
		}

		bool PsyFilter::loadSong( const std::string & fileName, Song & song )
		{
			if ( fileIsReadable( fileName ) ) {
				std::vector<PsyFilter*>::iterator it = filters.begin();
				for (  ; it < filters.end(); it++) {
					PsyFilter* filter = *it;
					if ( filter->testFormat(fileName) ) {
						return filter->load(fileName,song);
						break;
					}
				}
			}
			return false;
		}

		bool PsyFilter::saveSong( const std::string & fileName, Song & song, int version )
		{
			std::vector<PsyFilter*>::iterator it = filters.begin();
			for (  ; it < filters.end(); it++) {
				PsyFilter* filter = *it;
				if ( filter->version() == version ) {
					// check postfix
					std::string newFileName = fileName;
					int dotPos = fileName.rfind(".");
					if ( dotPos == std::string::npos ) 
						// append postfix
						newFileName = fileName + "." + filter->filePostfix();

					return filter->save(newFileName,song);
					break;
				}
			}
			return false;
		}


		bool PsyFilter::fileIsReadable( const std::string & file )
		{
			std::ifstream _stream (file.c_str (), std::ios_base::in | std::ios_base::binary);
			if (!_stream.is_open ()) return false;
			return true;
		}


		std::string PsyFilter::replaceIllegalXmlChr( const std::string & text, bool strict )
		{
			std::string xml = text;

			// replace ampersand
			unsigned int search_pos = 0;
			while ( ( search_pos = xml.find("&", search_pos) ) != std::string::npos )
			xml.replace(search_pos++, 1, "&amp;" );

			// replace less than
			while ( ( search_pos = xml.find("<") ) != std::string::npos )
			xml.replace(search_pos, 1, "&lt;" );
				
			if ( strict ) {
				// replace greater than
				while ( ( search_pos = xml.find(">") ) != std::string::npos )
				xml.replace(search_pos, 1, "&gt;" );
				// replace apostrophe
				while ( ( search_pos = xml.find("'") ) != std::string::npos )
				xml.replace(search_pos, 1, "&apos;" );
				// replace quotation mark
				while ( ( search_pos = xml.find("\"") ) != std::string::npos ) 
				xml.replace(search_pos, 1, "&quot;" );
			}
			return xml;
		}


	} // end of host namespace
} // end of psycle namespace

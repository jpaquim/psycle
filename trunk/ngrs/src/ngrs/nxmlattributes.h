/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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

#ifndef NXMLATTRIBUTES_H
#define NXMLATTRIBUTES_H

/*
 * No description
 */
 
#include <string>
#include <map>

namespace ngrs {

  struct NXmlPos {
    std::string::size_type pos;
    std::string::size_type len;
    int err;
    int type;     
  };    

  class NXmlAttributes
  {
  public:

    NXmlAttributes( );

    NXmlAttributes( const std::string & tag_header );

    ~NXmlAttributes();

    void reset( const std::string & tag_header );
    std::string value( const std::string & name ) const;

    const std::string & tagName() const;

  private:

    std::string tagName_;    
    std::map< std::string, std::string > attrib_;

    void parseHeader( const std::string & text );    	
    std::string::size_type getNextAttribute( const std::string & text, std::string::size_type pos );

  };

}

#endif // NXMLATTRIBUTES_H

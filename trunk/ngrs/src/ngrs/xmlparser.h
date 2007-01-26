/***************************************************************************
 *    Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#ifndef XMLPARSER_H
#define XMLPARSER_H

#include "sigslot.h"
#include <string>
#include "xmlattributes.h"

#ifdef __unix__
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLString.hpp>

#endif
/**
@author  Stefan Nattkemper
*/

namespace ngrs {

  class XmlParser{
  public:
    XmlParser();

    ~XmlParser();

    sigslot::signal2< const XmlParser &, const std::string & > tagParse;

    int parseFile( const std::string & fileName );
    int parseString( const std::string & text );
    std::string getAttribValue(const std::string & name) const;

#ifdef __unix__
    const XERCES_CPP_NAMESPACE_QUALIFIER  Attributes*   attrs;
#endif

    void onTagParse( const std::string& tagName );

  private:

    XmlAttributes attribs;
    NXmlPos getNextTag( int pos, const std::string & text ) const;    

  };

}

#endif

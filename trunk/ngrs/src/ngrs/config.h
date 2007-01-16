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
#ifndef NCONFIG_H
#define NCONFIG_H

/**
@author Stefan
*/

#include "skin.h"
#include "object.h"
#include <map>

namespace ngrs {

  class Border;
  class XmlParser;


  class Config : public Object {

  public:
    Config();

    ~Config();

    Skin skin( const std::string & identifier );

  private:

    std::string lastId;

    std::map<std::string, Skin> skinMap;
    Skin* findSkin( const std::string & id );
    Color attrsToColor( const XmlParser & parser );

    int loadXmlConfig( const std::string & configName );
    void onTagParse( const XmlParser & parser, const std::string & tagName );

  };

}

#endif

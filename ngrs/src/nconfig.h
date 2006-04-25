/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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

#include "nskin.h"
#include <map>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <nobject.h>

class NBorder;


class XERCES_CPP_NAMESPACE_QUALIFIER Attributes;


class NConfig : public NObject {

public:
    NConfig();

    ~NConfig();

    void setSkin(NSkin * skin, const std::string & identifier);

    void loadXmlConfig(const std::string & configName);

    std::map<std::string, NSkin> skinMap;
    std::map<std::string, std::string> pathMap;

    std::string findPath(const std::string & id);
    NSkin* findSkin(const std::string & id);

    std::string getAttribValue(const std::string & name);

    signal1<const std::string &> tagParse;

    const XERCES_CPP_NAMESPACE_QUALIFIER  Attributes*   attrs;

private:


};


#endif

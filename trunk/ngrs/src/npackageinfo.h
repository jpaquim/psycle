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
#ifndef NPACKAGEINFO_H
#define NPACKAGEINFO_H

#include <map>
#include <string>
#include <vector>

/**
@author Stefan Nattkemper
*/
class NPackageInfo{
public:
    NPackageInfo();

    ~NPackageInfo();

    std::string name() const;

    std::vector<std::string> categories() const;
    std::vector<std::string> factoryNamesByCategory(const std::string & categoryName) const;

private:

    std::map<std::string, std::vector<std::string> > packageMap;

    std::string packageName;
};

// the types of the class factories
typedef NPackageInfo* create_p();
typedef void destroy_p(NPackageInfo*);


#endif

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
#include "ngrs/npackageinfo.h"
#include <iostream>

NPackageInfo::NPackageInfo()
{
  std::vector<std::string> standards;
    standards.push_back("Label");
    standards.push_back("Edit");
    standards.push_back("Button");
    standards.push_back("ScrollBar");
    standards.push_back("CheckBox");
    standards.push_back("GroupBox");
    standards.push_back("ComboBox");
    standards.push_back("ListBox");
    standards.push_back("TogglePanel");
    standards.push_back("Panel");
  packageMap[std::string("Standard")] = standards;

  std::vector<std::string> additional;
    additional.push_back("Image");
  packageMap[std::string("Additional")] = additional;

  std::vector<std::string> dialogs;
    dialogs.push_back("FileDialog");
  packageMap[std::string("Dialogs")] = dialogs;

  packageName = "ngrs_pkg";
}


NPackageInfo::~NPackageInfo()
{
}

extern "C" NPackageInfo* createPackageInfo() {
    return new NPackageInfo();
}

extern "C" void destroyNPackageInfo(NPackageInfo* p) {
    delete p;
}

std::vector< std::string > NPackageInfo::categories( ) const
{

  std::vector<std::string> stringList;

  for (std::map<std::string , std::vector<std::string> >::const_iterator i(packageMap.begin()) ; i != packageMap.end() ; ++i) stringList.push_back(i->first);

  return stringList;
}

std::vector< std::string > NPackageInfo::factoryNamesByCategory( const std::string & categoryName ) const
{
  std::map<std::string , std::vector<std::string> >::const_iterator itr = packageMap.begin();

  if ( (itr = packageMap.find(categoryName)) == packageMap.end() )
    return std::vector<std::string>(); // empty list
  else
    return itr->second;
}

std::string NPackageInfo::name( ) const
{
  return packageName;
}


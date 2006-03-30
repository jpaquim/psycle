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
#ifndef NFILE_H
#define NFILE_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

/**
@author Stefan
*/
class NFile{
public:
    NFile();

    ~NFile();

    static std::string readFile(const std::string & filename);
    static std::vector<std::string> fileList(const std::string path);
    static std::vector<std::string> dirList(const std::string path);
    static std::vector<std::string> parentDirList(const std::string path);
    static std::string workingDir();
    static std::string parentWorkingDir();
    static void cd( const std::string & path );
    static void cdHome();
    static std::string replaceTilde(std::string path);

};

#endif

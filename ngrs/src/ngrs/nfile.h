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

/**
@author Stefan
*/

const int nFiles = 1;
const int nDirs  = 2;

class NFile{
public:
    NFile();

    ~NFile();

    static std::string readFile(const std::string & filename);
    static std::vector<std::string> fileList( const std::string & path, int fMode = nFiles | nDirs );   
    static std::string workingDir();
    static bool fileIsReadable(const std::string & file);
    static std::string parentWorkingDir();
    static void cd( const std::string & path );
    static void cdHome();
    static std::string home();
    static std::string replaceTilde(const std::string & path);
    static std::string env(const std::string & envName);
    static std::string extractFileNameFromPath(const std::string & fileName);

		// replaces entitys`s for xml writing
		// There are 5 predefined entity references in XML:
 		// &lt; 	< 	less than 
		// &gt; 	> 	greater than
    // &amp; 	& 	ampersand 
    // &apos; 	' 	apostrophe
    // &quot; 	" 	quotation mark
    // Only the characters "<" and "&" are strictly illegal in XML. Apostrophes, quotation marks and greater than signs are legal. strict = true  replaces all.
		static std::string replaceIllegalXmlChr( const std::string & text, bool strict = true );
};

#endif
